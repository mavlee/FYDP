#include "surfaceHelper.h"

SDL_Surface *loadImage(std::string filename) {
  SDL_Surface *loadedImage = NULL;
  SDL_Surface *optimizedImage = NULL;

  loadedImage = IMG_Load(filename.c_str());

  // If the image isn't null, stick the image onto another surface. doing this wil ensure that the loaded image is in the same format as the screen.
  if (loadedImage != NULL) {
    optimizedImage = SDL_DisplayFormat(loadedImage);
    SDL_FreeSurface(loadedImage);

    // If the surface was optimized
    if (optimizedImage != NULL) {
      // Color key surface
      SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB( optimizedImage->format, 0xFF, 0xFF, 0xFF) );
    }
  }
  return optimizedImage;
}

void applySurface(int x, int y, SDL_Surface *source, SDL_Surface *dest, SDL_Rect *clip) {
  // SDL_Rect represents a rectangle, has x, y, width, height.
  SDL_Rect offset;

  offset.x = x;
  offset.y = y;

  SDL_BlitSurface(source, clip, dest, &offset);
}
