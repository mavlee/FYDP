#include "surfaceHelper.h"

SDL_Surface *dots = NULL;

//Array of clips from the sprite map (dots.png) to be blitted
SDL_Rect clip[4];

/*
 *	Set the boundaries for clipping the sprite map.
 */
void setClipBounds() {
  clip[0].x = 0;
  clip[0].y = 0;
  clip[0].w = 100;
  clip[0].h = 100;

  clip[1].x = 100;
  clip[1].y = 0;
  clip[1].w = 100;
  clip[1].h = 100;

  clip[2].x = 0;
  clip[2].y = 100;
  clip[2].w = 100;
  clip[2].h = 100;

  clip[3].x = 100;
  clip[3].y = 100;
  clip[3].w = 100;
  clip[3].h = 100;
}

SDL_Rect *getClipBounds() {
  return clip;
}

bool initClipper() {
  // Load the sprite map
  dots = loadImage("../res/images/dots.png");

  // If there was an problem loading the sprite map
  if( dots == NULL ) {
    return false;
  }

  setClipBounds();

  return true;
}

SDL_Surface* getSpriteMap() {
  if (dots == NULL) {
    if (!initClipper()) {
      return NULL;
    }
  }
  return dots;
}

void cleanUpClipper() {
  SDL_FreeSurface(dots);
}
