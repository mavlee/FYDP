#include "spriteClipper.h"
#include "LUtil.h"
#include "timer.h"
#include "canvas.h"
#include "game.h"

int main( int argc, char* args[] ) {
  Game *game;
  game = new Game(640, 480);
  SDL_Event event;

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

    game->update();
    game->draw();
  }

  delete game;

  return 0;
}
