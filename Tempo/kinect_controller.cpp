#include "kinect_controller.h"
#ifndef USE_MAC_INCLUDES

HANDLE hNextSkeletonEvent;
HANDLE hNextDepthEvent;
HANDLE hEventStopKinect;

INuiSensor *kinectSensor;

list<USHORT*> averageQueue;


HANDLE depthStream;
typedef struct {
  BYTE depthData[(width/2)*(height/2)*4];
} *pDepthData;

pDepthData depthData;

USHORT playerId;

/*
Initialize kinect sensor, sets it up to receive skeletal data
*/
HRESULT kinectinit() {
  HRESULT hr = E_FAIL;
  int numSensors;
  DWORD kinectFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON;

  hNextDepthEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  hNextSkeletonEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  hEventStopKinect = CreateEvent(NULL, TRUE, FALSE, NULL);

  // Check for availabe Kinect sensor
  if (NuiGetSensorCount(&numSensors) < 0 || numSensors < 1) {
    printf("Sensor not connected\n");
    return E_FAIL;
  }
  if (NuiCreateSensorByIndex (0, &kinectSensor) < 0) {
    printf("Sensor not connected\n");
    return E_FAIL;
  }

  // Initialize sensor
  hr = kinectSensor->NuiInitialize(kinectFlags);
  if (FAILED(hr)) {
    printf("Kinect initialization error\n");
    return hr;
  }

  // Initialize depth stream
  hr = kinectSensor->NuiImageStreamOpen (
    NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
    NUI_IMAGE_RESOLUTION_320x240,
    0,
    2,
    hNextDepthEvent,
    &depthStream);

  if (FAILED(hr)) {
    printf("Failed to start depth stream\n");
    return hr;
  }

  hr = kinectSensor->NuiSkeletonTrackingEnable(hNextSkeletonEvent, 0);
  if (FAILED(hr))
  {
    printf("Failed to start skeletal stream\n");
    return hr;
  }

  playerId = 0;

  return hr;
}

// Generate skeletal locations in pixels
PixelSkeletalData GetSkeletalPixelLocation(NUI_SKELETON_DATA skeleton) {
  PixelSkeletalData skeletalData = {0};
  for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++) {
    Vector4 jointPos = skeleton.SkeletonPositions[i];
    Vector4 drawPos = {0};
    float max_XZ = jointPos.z*tan(65/2 * 3.14/180);
    float max_YZ = jointPos.z*tan(50/2 * 3.14/180);
    drawPos.x = (jointPos.x/max_XZ) * width/2 + width/2;
    drawPos.y = -(jointPos.y/max_YZ) * height/2 + height/2;
    skeletalData.skPixelLocation[i].x = drawPos.x;
    skeletalData.skPixelLocation[i].y = drawPos.y;
  }

  return skeletalData;
}

// Process skeletal data
bool ProcessSkeletalEvent() {
  // Wait for skeletal event to trigger
  PixelSkeletalData skeletalData = {0};
  NUI_SKELETON_FRAME skeletonFrame = {0};

  // Gets skeleton frame
  if (SUCCEEDED(kinectSensor->NuiSkeletonGetNextFrame(0, &skeletonFrame))) {
    for (int i = 0; i < NUI_SKELETON_COUNT; i++) {

      // Save the Id of the first player found
      // Only change Ids when the found player exits the field of view
      if (skeletonFrame.SkeletonData[i].eTrackingState != NUI_SKELETON_NOT_TRACKED
        && playerId == 0) {
          playerId = i + 1;
      } else if (playerId > 0
        && skeletonFrame.SkeletonData[playerId - 1].eTrackingState ==
        NUI_SKELETON_NOT_TRACKED) {
          playerId = 0;
      }
    }
  } else {
    return false;
  }
  return true;
}

// Samples around empty pixels, checks the values around it and picks
// the most frequent value and fills the pixel in with that value
USHORT* FilterDepthArray (USHORT* depthArray, int iWidth, int iHeight) {
  USHORT* smoothedDepthArray = new USHORT[width*height/4];
  for (int i = 0; i < width*height/4; i++)
    smoothedDepthArray[i] = 0;

  int widthBound = iWidth - 1;
  int heightBound = iHeight -1;

  for (int rowIndex = 0; rowIndex < 240; rowIndex++) {
    for (int colIndex = 0; colIndex < 320; colIndex++) {
      int cellIndex = colIndex + rowIndex * 320;

      if (depthArray[cellIndex] == 0) {
        int x = cellIndex % 320;
        int y = (cellIndex - x) / 320;

        USHORT* filterCollection = new USHORT[24,2];
        for (int i = 0; i < 24; i++)
          for (int j = 0; j < 2; j++)
            filterCollection[i, j] = 0;

        int innerBandCount = 0;
        int outerBandCount = 0;

        // Loop in a 5x5 grid around the empty pixel to sample values
        for (int yi = -2; yi < 3; yi++) {
          for (int xi = -2; xi < 3; xi++) {
            if (xi != 0 || yi != 0) {
              int xSearch = x + xi;
              int ySearch = y + yi;

              if (xSearch >= 0 && xSearch <= widthBound && ySearch >= 0 && ySearch <= heightBound) {
                int index = xSearch + (ySearch * iWidth);

                // Keep a count of values it finds
                if (depthArray[index] != 0) {
                  for (int i = 0; i < 24; i++) {
                    if (filterCollection[i, 0] == depthArray[index]) {
                      filterCollection[i, 1]++;
                      break;
                    } else if (filterCollection[i, 0] == 0) {
                      filterCollection[i, 0] = depthArray[index];
                      filterCollection[i, 1]++;
                      break;
                    }
                  }

                  if (yi != 2 && yi != -2 && xi != 2  && xi != -2)
                    innerBandCount++;
                  else
                    outerBandCount++;
                }
              }
            }
          }
        }

        if (innerBandCount >= innerBandThreshhold || outerBandCount >= outerBandThreshhold) {
          short frequency = 0;
          short depth = 0;

          for (int i = 0; i < 24; i++) {
            if (filterCollection[i, 0] == 0) {
              break;
            }
            if (filterCollection[i, 1] > frequency) {
              depth = filterCollection[i, 0];
              frequency = filterCollection[i, 1];
            }
          }

          smoothedDepthArray[cellIndex] = depth;
        } else {
          smoothedDepthArray[cellIndex] = depthArray[cellIndex];
        }
      } else {
        smoothedDepthArray[cellIndex] = depthArray[cellIndex];
      }
    }
  }

  if (depthArray)
    delete depthArray;

  return smoothedDepthArray;
}

// Filters the array with a weighted average of previous frames
USHORT* AverageDepthArray (USHORT* depthArray) {
  averageQueue.push_back(depthArray);

  while (averageQueue.size() > queueLength) {
    averageQueue.pop_front();
  }

  int sumDepthArray[depthWidth*depthHeight];
  USHORT* averageDepthArray = new USHORT[depthWidth*depthHeight];

  for (int i = 0; i < depthWidth * depthHeight; i++) {
    sumDepthArray[i] = 0;
    averageDepthArray[i] = 0;
  }

  int denominator = 0;
  int count = 1;

  list<USHORT*>::const_iterator iterator;

  for (iterator = averageQueue.begin(); iterator != averageQueue.end(); ++iterator) {
    for (int rowIndex = 0; rowIndex < 240; rowIndex++) {
      for (int colIndex = 0; colIndex < 320; colIndex++) {
        int index = colIndex + (rowIndex * 320);
        sumDepthArray[index] += (*iterator)[index] * count;
      }
    }
    denominator += count;
    count++;
  }

  for (int rowIndex = 0; rowIndex < 240; rowIndex++) {
    for (int colIndex = 0; colIndex < 320; colIndex++) {
      int index = colIndex + (rowIndex * 320);
      averageDepthArray[index] = (USHORT)(sumDepthArray[index]/denominator);
    }
  }

  if (depthArray)
    delete depthArray;

  return averageDepthArray;
}

// Process depth frame event
bool ProcessDepthEvent() {
  NUI_IMAGE_FRAME imageFrame;
  bool processedFrame = true;
  int index = 0;

  HRESULT hr = kinectSensor->NuiImageStreamGetNextFrame(
    depthStream,
    0,
    &imageFrame);

  if (FAILED(hr))
    return false;

  INuiFrameTexture *frameTexture = imageFrame.pFrameTexture;
  NUI_LOCKED_RECT LockedRect;

  frameTexture->LockRect(0, &LockedRect, NULL, 0);

  if (LockedRect.Pitch != 0 ) {
    USHORT* curr = (USHORT*) LockedRect.pBits;
    USHORT* shortDepthArray = new USHORT[width*height/4];
    const USHORT* dataEnd = curr + ((width/2)*(height/2));


    for (int i = 0; i < width*height/4; i++)
      shortDepthArray[i] = 0;

    int i = 0;

    while (curr < dataEnd && playerId != 0) {
      USHORT player    = NuiDepthPixelToPlayerIndex(*curr);
      shortDepthArray[i] = NuiDepthPixelToDepth(*curr);
      curr++;
      i++;
    }

        shortDepthArray = FilterDepthArray(shortDepthArray, width/2, height/2);

    shortDepthArray = AverageDepthArray(shortDepthArray);

    curr = curr - 76800;
    i = 0;

    while (curr < dataEnd && playerId != 0) {
      USHORT player = NuiDepthPixelToPlayerIndex(*curr);
      shortDepthArray[i] = shortDepthArray[i] << 3 | player;
      curr++;
      i++;
    }



    for (int i = 0; i < width*height/4 && playerId != 0; i++) {
      USHORT player = NuiDepthPixelToPlayerIndex(shortDepthArray[i]);
      if (player == playerId) {
        for (int j = index; j < index + 4; j++)
          depthData->depthData[j] = (BYTE)255;
        //depthData->depthData[i+3] = (BYTE)50;
      } else {
        for (int j = index; j < index + 4; j++)
          depthData->depthData[j] = 0;
      }
      index += 4;
    }

    if (shortDepthArray)
      delete shortDepthArray;
  }
  frameTexture->UnlockRect(0);
  kinectSensor->NuiImageStreamReleaseFrame(depthStream, &imageFrame);


  return true;
}

void ShutdownKinect(HANDLE hKinectProcess) {
  if (hEventStopKinect != NULL)  {
    SetEvent(hEventStopKinect);

    if (hKinectProcess != NULL) {
      WaitForSingleObject(hKinectProcess, INFINITE);
      CloseHandle(hKinectProcess);
    }
    CloseHandle(hEventStopKinect);
  }

  if (kinectSensor) {
    kinectSensor->NuiShutdown();
  }

  if (hNextDepthEvent && hNextDepthEvent != INVALID_HANDLE_VALUE) {
    CloseHandle(hNextDepthEvent);
    hNextDepthEvent = NULL;
  }

  if (hNextSkeletonEvent && hNextSkeletonEvent != INVALID_HANDLE_VALUE) {
    CloseHandle(hNextSkeletonEvent);
    hNextSkeletonEvent = NULL;
  }
}


DWORD WINAPI KinectProcessThread(LPVOID lpParam) {
  printf ("Starting kinect thread..\n");
  depthData = (pDepthData)lpParam;
  const int numEvents = 3;
  HANDLE hEvents[numEvents] = {hEventStopKinect, hNextDepthEvent, hNextSkeletonEvent};
  int eventId;
  bool run = true;
  printf("Beginning to wait for events\n");
  while (run) {
    eventId = WaitForMultipleObjects(numEvents, hEvents, FALSE, 0);

    if (eventId == WAIT_TIMEOUT)
      continue;

    if (eventId == WAIT_OBJECT_0) {
      printf("Thread shutdown\n");
      run = false;
      break;
    }

    if (WaitForSingleObject(hNextDepthEvent, 0) == WAIT_OBJECT_0)
      ProcessDepthEvent();

    if (WaitForSingleObject(hNextSkeletonEvent, 0) == WAIT_OBJECT_0)
      ProcessSkeletalEvent();
  }
  printf("Kinect exit\n");
  return 0;
}

#endif
