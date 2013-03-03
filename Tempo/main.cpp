#include "timer.h"
#include "canvas.h"
#include "game.h"
#include "constants.h"
#include "util.h"

int main( int argc, char* args[] ) {
#ifndef USE_MAC_INCLUDES
  OpenConsole();
#endif

  Game *game;
  game = new Game(SCREEN_WIDTH, SCREEN_HEIGHT);
  SDL_Event event;

  // FPS counter and regulator code
  Timer timer;
  Timer clock;
  Timer fpsTimer;

  int frame = 0;

  timer.start();
  clock.start();
  fpsTimer.start();

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

	while (fpsTimer.get_ticks() < FPS_CAP) {
		// wait until it's appropriate to update the screen.
	}
	// reset timer now
	fpsTimer.start();

    frame++;
    if (timer.get_ticks() > 1000) {
      game->update(frame, (clock.get_ticks() / 1000.f));
      timer.start();
    } else {
      game->update();
    }
    game->draw();
  }

  delete game;

  return 0;
}

