// Ref http://www.cs.princeton.edu/~edwardz/tutorials/kinect/kinect1.html

#include <Windows.h>
#include <Ole2.h>

#include <gl/GL.h>
#include <gl/GLU.h>
#include <gl/glut.h>

#include <NuiApi.h>
#include <NuiImageCamera.h>
#include <NuiSensor.h>

#include <math.h>

#include <queue>

using namespace std;

typedef queue<int> INTQUEUE;


#define width 640
#define height 480

// OpenGL
GLuint textureId;                             // ID of texture that contains kinect RGB data
GLubyte data[width*height*4];                 // BGRA array containing the texture data
GLubyte depthData[(width/2)*(height/2)*4];

// Kinect
HANDLE depthStream;                             // Kinect's RGB camera;
HANDLE rgbStream;
INuiSensor* sensor;                           // Kinect Sensor
NUI_SKELETON_DATA* skeletonData;                  // skeleton frame;
HANDLE hNextSkeletonEvent;
HANDLE hNextDepthPlayerEvent;

USHORT playerGlobal = 0;
USHORT playerId = 0;

// Something useful for colouring players
static const int g_IntensityShiftByPlayerR[] = { 1, 2, 0, 2, 0, 0, 2, 0 };
static const int g_IntensityShiftByPlayerG[] = { 1, 2, 2, 0, 2, 0, 0, 1 };
static const int g_IntensityShiftByPlayerB[] = { 1, 0, 2, 2, 0, 2, 0, 2 };

void draw(void);

void printData(float, float, char*);

/*
* Initializes Kinect sensor.  First find the attached sensor, then
* initialize and prepare to read data from it
*/
bool initKinect()  {
  int numSensors;
  if (NuiGetSensorCount(&numSensors) < 0 || numSensors < 1) return false;
  if (NuiCreateSensorByIndex (0, &sensor) < 0) return false;
  HRESULT hr;
  hNextSkeletonEvent = NULL;
  skeletonData = new NUI_SKELETON_DATA;

  // Initialize sensor
  hr = sensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_COLOR|NUI_INITIALIZE_FLAG_USES_SKELETON );

  hNextDepthPlayerEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

  hr = sensor->NuiImageStreamOpen(
    NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,                     // Type of camera
    NUI_IMAGE_RESOLUTION_320x240,             // Resolution
    0,                                        // Image stream mode, (near flag)
    2,                                        // Number of frames to buffer
    hNextDepthPlayerEvent,                    // Event handler
    &depthStream);

  hr = sensor->NuiImageStreamOpen(
    NUI_IMAGE_TYPE_COLOR,                     // Type of camera
    NUI_IMAGE_RESOLUTION_640x480,             // Resolution
    0,                                        // Image stream mode, (near flag)
    2,                                        // Number of frames to buffer
    NULL,                                     // Event handler
    &rgbStream);

  hNextSkeletonEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

  hr = sensor->NuiSkeletonTrackingEnable(
    hNextSkeletonEvent,
    0);
  return sensor;
  } 

void DrawTrackedSkeletonJoints(NUI_SKELETON_DATA skeleton) {
  for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++) {
    Vector4 jointPos = skeleton.SkeletonPositions[i];
    Vector4 drawPos = {0};
    float max_XZ = jointPos.z*tan(65/2 * 3.14/180);
    float max_YZ = jointPos.z*tan(50/2 * 3.14/180);
    drawPos.x = (jointPos.x/max_XZ) * 320 + 320;
    drawPos.y = -(jointPos.y/max_YZ) * height/2 + height/2;
    printData(drawPos.x, drawPos.y, "x");
    }
  }

void DrawSkeletonPosition(NUI_SKELETON_DATA skeleton) {
  Vector4 skeletonPos = skeleton.Position;
  printData(skeletonPos.x, skeletonPos.y, "o");
  }

void SkeletonFrameReady(NUI_SKELETON_FRAME *skeletonFrame) {
  DWORD temp;
  for (int i = 0; i <   NUI_SKELETON_COUNT; i++) {
    const NUI_SKELETON_DATA &skeleton = skeletonFrame->SkeletonData[i];
    if (skeleton.eTrackingState != NUI_SKELETON_NOT_TRACKED){
      *skeletonData = skeleton;

      }
    if (playerId == 0 && skeleton.eTrackingState != NUI_SKELETON_NOT_TRACKED) {
      playerId = i + 1;
      } else if (playerId > 0 && 
        skeletonFrame->SkeletonData[playerId - 1].eTrackingState == 
        NUI_SKELETON_NOT_TRACKED ) {
          playerId = 0;
        }

      //switch (skeleton.eTrackingState) {
      //  case NUI_SKELETON_TRACKED:
      //    DrawTrackedSkeletonJoints(skeleton);
      //     printData(200.0, 220.0, "have joints");
      //    break;

      //  case NUI_SKELETON_POSITION_ONLY:
      //    DrawSkeletonPosition(skeleton);
      //   
      //    break;
      //}
    }
  }

void getSkeletonData() {
  if (WAIT_OBJECT_0 == WaitForSingleObject(hNextSkeletonEvent, 0)) {
    NUI_SKELETON_FRAME skeletonFrame = {0};

    if (SUCCEEDED(sensor->NuiSkeletonGetNextFrame(0, &skeletonFrame))) {
      SkeletonFrameReady(&skeletonFrame);
      }
    }
  }




void getKinectDepthData (GLubyte *dest) {
  NUI_IMAGE_FRAME imageFrame;
  NUI_LOCKED_RECT LockedRect;
  int index;
  bool playerInFrame[6] = {false, false, false, false, false, false};

  if (WAIT_OBJECT_0 == WaitForSingleObject(hNextDepthPlayerEvent, 0)) {
    HRESULT hr 
      = sensor->NuiImageStreamGetNextFrame(depthStream, 0, &imageFrame);

    if (FAILED(hr)) return;

    INuiFrameTexture *texture = imageFrame.pFrameTexture;
    texture->LockRect(0, &LockedRect, NULL, 0);

    // Check pitch to see if frame is empty
    if (LockedRect.Pitch != 0 ) {
      USHORT* curr = (USHORT*) LockedRect.pBits;
      const USHORT* dataEnd = curr + ((width/2)*(height/2));
      index = 0;



      //while (curr < dataEnd) {
      //  USHORT player = NuiDepthPixelToPlayerIndex(*curr);
      //  if (player > 0 ) {
      //    if (playerId == 0 || player > playerId)
      //      playerId = player;
      //    
      //    if (!playerInFrame[player])
      //      playerInFrame[player] = true;
      //    break;
      //    }
      //  curr++;
      //  }

      //if (playerId > 0)
      //  if (!playerInFrame[playerId] )
      //    playerId = 0;

      //curr = (USHORT*) LockedRect.pBits;

      while (curr < dataEnd && playerId != 0) {
        USHORT depth     = *curr;
        USHORT realDepth = NuiDepthPixelToDepth(depth);
        BYTE intensity = 255;
        USHORT player    = NuiDepthPixelToPlayerIndex(depth);


        // Only colour in the player, only 1 player
        if (player == playerId) {
          for (int i = index; i < index + 4; i++)
            dest[i] = intensity;

          //dest[index + 4] = intensity >> 2;
          //dest[index + 5] = intensity >> 2;
          //dest[index + 6] = intensity;

          }
        else {
          for (int i = index; i < index + 4; i++)
            dest[i] = 0;
          }
        index += 4;
        curr += 1;                                                                
        }    
      }
    texture->UnlockRect(0);
    sensor->NuiImageStreamReleaseFrame(depthStream, &imageFrame);
    }
  }

void getKinectColorData (GLubyte *dest) {
  NUI_IMAGE_FRAME imageFrame;
  NUI_LOCKED_RECT LockedRect;

  if (sensor->NuiImageStreamGetNextFrame(rgbStream, 0, &imageFrame) < 0) return;

  INuiFrameTexture *texture = imageFrame.pFrameTexture;
  texture->LockRect(0, &LockedRect, NULL, 0);

  // Check pitch to see if frame is empty
  if (LockedRect.Pitch != 0 ) {
    const BYTE* curr = (const BYTE*) LockedRect.pBits;
    const BYTE* dataEnd = curr + (width*height)*4;

    while (curr < dataEnd) {
      *dest++ = *curr++;
      }
    }
  texture->UnlockRect(0);
  sensor->NuiImageStreamReleaseFrame(rgbStream, &imageFrame);
  }

void getKinectSkeleton(NUI_SKELETON_DATA *skeleton) {
  NUI_SKELETON_FRAME *pSkeletonFrame = new NUI_SKELETON_FRAME;
  NuiSkeletonGetNextFrame(0, pSkeletonFrame);
  *skeleton = pSkeletonFrame->SkeletonData[0];
  }


bool init(int argc, char* argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(width,height);
  glutCreateWindow("Kinect SDK Tutorial");
  glutDisplayFunc(draw);
  glutIdleFunc(draw);
  return true;
  }

void printData( float x, float y, char* data) {
  int l = strlen(data);
  glRasterPos2i(x,y);
  glRasterPos2f(x, y);

  for (int i = 0; i < l; i++)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, data[i]);
  }

void printSkeleton(NUI_SKELETON_DATA* skeletonData) {
  for (int i = 0; i < 20; i++) {
    if (skeletonData->eSkeletonPositionTrackingState[i] != NUI_SKELETON_POSITION_NOT_TRACKED) {
      Vector4 pos = skeletonData->SkeletonPositions[i];
      printData(pos.x, pos.y, "o");
      }
    }
  }

void drawKinectData() {
  glBindTexture(GL_TEXTURE_2D, textureId);
  getKinectColorData(data);
  getKinectDepthData(depthData);

  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width/2, height/2, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (GLvoid*)depthData);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(0, 0, 0);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(width*2, 0, 0);
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(width*2, height*2, 0.0f);
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(0, height*2, 0.0f);
  glEnd();


  //skeletonData = new NUI_SKELETON_DATA;
  //getKinectSkeleton(skeletonData);
  //if (skeletonData->eTrackingState == NUI_SKELETON_TRACKED) {
  //  printData(200.0, 100.0, "tracking skeleton");
  //  printSkeleton(skeletonData);
  //  } else if (skeletonData->eTrackingState == NUI_SKELETON_POSITION_ONLY) {
  //    printData(skeletonData->Position.x * 100, skeletonData->Position.y*100, "X");
  //  }

  getSkeletonData();
  if (skeletonData != NULL)
    if (skeletonData->eTrackingState != NUI_SKELETON_NOT_TRACKED)
      DrawTrackedSkeletonJoints(*skeletonData);
  printData(600, 400, "hello world");
  }

void draw() {
  drawKinectData();
  glutSwapBuffers();
  }

void shutdownKinect() {
  //NuiCameraElevationSetAngle (0l);
  NuiShutdown();
  }

void execute() {
  atexit(shutdownKinect);
  glutMainLoop();
  }



int main(int argc, char* argv[]) {
  if (!init(argc, argv)) return 1;
  if (!initKinect()) return 1;
  //NuiCameraElevationSetAngle (20l);

  // Initialize textures
  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (GLvoid*) data);
  glBindTexture(GL_TEXTURE_2D, 0);

  // OpenGL setup
  glClearColor(0,0,0,0);
  glClearDepth(1.0f);
  glEnable(GL_TEXTURE_2D);

  // Camera setup
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, width, height, 0, 1, -1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Main loop
  execute();
  return 0;
  }

