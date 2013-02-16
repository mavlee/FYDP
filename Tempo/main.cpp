#include "spriteClipper.h"
#include "LUtil.h"
#include "timer.h"

SDL_Surface *screen = NULL;
SDL_Surface *imageSurface = NULL;
SDL_Surface *debugInfoSurface = NULL;

//This is the font that's going to be used.
TTF_Font *font = NULL;
SDL_Color textColor = { 255, 255, 255 };

bool init() {
  // Start SDL
  if (SDL_Init( SDL_INIT_EVERYTHING ) == -1 ) {
    return false;
  }

  // SDL_SWSURFACE implies that the surface is set up in software memory.
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_OPENGL);
  if (screen == NULL) {
    return false;
  }

  //Initialize SDL ttf. Must be called prior to all SDL_ttf functions.
  if (TTF_Init() == -1) {
    return false;
  }

  if (!initClipper()) {
    return false;
  }

  if (!initGL()) {
    return false;
  }

  // Set the caption on the window.
  SDL_WM_SetCaption("Tempo", NULL);
  return true;
}

void clean_up() {
  SDL_FreeSurface(imageSurface);
  SDL_FreeSurface(debugInfoSurface);

  TTF_CloseFont(font);
  TTF_Quit();

  cleanUpClipper();

  // Quit SDL. Also handles cleanup of the screen object.
  SDL_Quit();
}

int main( int argc, char* args[] ) {
  SDL_Event event;

  bool eventTriggered = false;

  if (init() == false) {
    return 1;
  }

  glClear(GL_COLOR_BUFFER_BIT);

  while (!eventTriggered) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          eventTriggered = true;
          break;
        case SDL_KEYDOWN:
          handleKeys(event.key.keysym.sym);
          break;
        default:
          break;
      }
    }

    update();
    render();
  }

  clean_up();

  return 0;
}
