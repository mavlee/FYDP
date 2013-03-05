#include "timer.h"
#include "canvas.h"
#include "game.h"
#include "constants.h"
#include "util.h"
#include "kinect_controller.h"

int main( int argc, char* args[] ) {
#ifndef USE_MAC_INCLUDES
  OpenConsole();
#endif

  //std::string musicFile = selectMusicFileDialog();
  //printf("File in main.cpp: %s\n", musicFile);
  Game *game;
  //game = new Game(SCREEN_WIDTH, SCREEN_HEIGHT, musicFile);
  game = new Game(SCREEN_WIDTH, SCREEN_HEIGHT, "");
  SDL_Event event;

#ifndef USE_MAC_INCLUDES
  HANDLE hKinectProcess;
  if (FAILED(kinectinit())) {
    printf("Failed to initialize Kinect\n");
  }else {
    printf("Kinect Sensor started\n");
    hKinectProcess = CreateThread( NULL, 0, KinectProcessThread,
      (void*)game->canvas->depthData, 0, NULL );
  }
#endif

  bool eventTriggered = false;
  int *movementKeyDown = new int;
  *movementKeyDown = 0;
  int currentKey = 0;

  while (!eventTriggered) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          eventTriggered = true;
          break;
        case SDL_KEYDOWN:
          currentKey = event.key.keysym.sym;
          game->handleKeys(currentKey, movementKeyDown);
          break;
        case SDL_KEYUP:
          *movementKeyDown = 0;
          break;
        default:
          break;
      }
    }

    if (*movementKeyDown == 1) {
      game->handleKeys(currentKey, movementKeyDown);
    }

    game->update();
  }

  delete game;
#ifndef USE_MAC_INCLUDES
  ShutdownKinect(hKinectProcess);
#endif
  return 0;
}
