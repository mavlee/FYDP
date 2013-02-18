#include "spriteClipper.h"
#include "timer.h"
#include "canvas.h"
#include "game.h"
#include "constants.h"
#include "util.h"

int main( int argc, char* args[] ) {
  Game *game;
  game = new Game(SCREEN_WIDTH, SCREEN_HEIGHT);
  SDL_Event event;

  OpenConsole();

  // FPS counter and regulator code
  Timer timer;
  Timer clock;

  int frame = 0;

  timer.start();
  clock.start();

  bool eventTriggered = false;

  while (!eventTriggered) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          eventTriggered = true;
          break;
        case SDL_KEYDOWN:
          game->handleKeys(event.key.keysym.sym);
          break;
        default:
          break;
      }
    }

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

