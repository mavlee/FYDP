#include "game.h"
#include "constants.h"
#include "util.h"
#include "kinect_controller.h"

int main( int argc, char* args[] ) {
#ifndef USE_MAC_INCLUDES
  OpenConsole();
#endif
  Game theGame(SCREEN_WIDTH, SCREEN_HEIGHT);

#ifndef USE_MAC_INCLUDES
  HANDLE hKinectProcess = NULL;
  if (FAILED(kinectinit())) {
    printf("Failed to initialize Kinect\n");
  } else {
    printf("Kinect Sensor started\n");
    hKinectProcess = CreateThread( NULL, 0, KinectProcessThread,
      (void*)theGame.canvas->depthData, 0, NULL );
  }
#endif

  theGame.execute();

#ifndef USE_MAC_INCLUDES
  if (hKinectProcess)
    ShutdownKinect(hKinectProcess);
#endif

  return 0;
}
