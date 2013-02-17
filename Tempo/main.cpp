#include "spriteClipper.h"
#include "timer.h"
#include "canvas.h"
#include "game.h"
#include "constants.h"

int main( int argc, char* args[] ) {
  Game *game;
  game = new Game(SCREEN_WIDTH, SCREEN_HEIGHT);
  SDL_Event event;

  // FPS counter and regulator code
  //Timer clock;
  //Timer fps;

  int frame = 0;

  //clock.start();
  //fps.start();

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

	/*frame++;
	if (clock.get_ticks() > 1000) {
		game->update(frame);
	} else {*/
		game->update();
	//}
    game->draw();
  }

  delete game;

  return 0;
}
