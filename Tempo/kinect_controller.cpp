#include "kinect_controller.h"

HANDLE hNextSkeletonEvent;
HANDLE hNextDepthEvent;
HANDLE hEventStopKinect;

INuiSensor *kinectSensor;

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
    const USHORT* dataEnd = curr + ((width/2)*(height/2));

    while (curr < dataEnd && playerId != 0) {
      USHORT depth     = *curr;
      USHORT realDepth = NuiDepthPixelToDepth(depth);
      BYTE intensity   = 255;
      USHORT player    = NuiDepthPixelToPlayerIndex(depth);


      // Only colour in the player, only 1 player
      if (player == playerId) {
          for (int i = index; i < index + 4; i++)
            depthData->depthData[i] = intensity;
          depthData->depthData[index+3] = (BYTE)50;
          }
        else {
          for (int i = index; i < index + 4; i++)
            depthData->depthData[i] = (BYTE)50;
          }
      index += 4;
      curr += 1;                                                         
    }    
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
    eventId = WaitForMultipleObjects(numEvents, hEvents, FALSE, 100);

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

