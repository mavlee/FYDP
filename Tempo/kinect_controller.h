#ifndef __KINECT_CONTROLLER_H__
#ifndef USE_MAC_INCLUDES
#define __KINECT_CONTROLLER_H__

#include <tchar.h>
#include <math.h>

#include <Windows.h>
#include <objbase.h>

#include <strsafe.h>
#include <list>

#include "NuiApi.h"
#include "NuiImageCamera.h"
#include "NuiSensor.h"

using namespace std;

#define HORZ_ANGLE 65
#define VERT_ANGLE 50

const int width = 640;
const int height = 480;

const int depthWidth = 320;
const int depthHeight = 240;

// frequency threshold for inner and outer band around pixel
const int innerBandThreshhold = 2;
const int outerBandThreshhold = 4;

// number of previous frames to sample
const int queueLength = 9;

typedef struct {
  float x;
  float y;
  } PixelCoordinates;

typedef struct {
  PixelCoordinates skPixelLocation[20];
  } PixelSkeletalData;

enum PLAYER_COLOUR {
  PLAYER_RED,
  PLAYER_GREEN,
  PLAYER_BLUE,
  PLAYER_WHITE,
  PLAYER_BLACK
};

/*
  Entry point for the kinect polling thread
  Parameters should include an array of event types that should trigger SDL events
  TODO: define this data structure
  Function loops and sends SDL events off
  When finished, cleans up the kinect
*/
DWORD WINAPI KinectProcessThread(LPVOID lpParam);

HRESULT kinectinit();

void ShutdownKinect(HANDLE hKinectProcess);

void changePlayerColour(PLAYER_COLOUR colour);

bool hasPlayer();

#endif
#endif
