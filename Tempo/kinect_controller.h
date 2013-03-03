#ifndef __KINECT_CONTROLLER_H__
#ifndef USE_MAC_INCLUDES 
#define __KINECT_CONTROLLER_H__

#include <tchar.h>
#include <strsafe.h>
#include <math.h>

#include <Windows.h>
#include <objbase.h>

#include "NuiApi.h"
#include "NuiImageCamera.h"
#include "NuiSensor.h"

#define HORZ_ANGLE 65
#define VERT_ANGLE 50

#define width 640
#define height 480

#define depthWidth 320
#define depthHeight 240

typedef struct {
  float x;
  float y;
  } PixelCoordinates;

typedef struct {
  PixelCoordinates skPixelLocation[20];
  } PixelSkeletalData;
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

#endif
#endif