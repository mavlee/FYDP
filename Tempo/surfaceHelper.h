#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_opengl.h"
#include <string>
#include <sstream>

#ifndef SDL_HELPER
#define SDL_HELPER

const int SCREEN_BPP = 32;

SDL_Surface *loadImage(std::string filename);
void applySurface(int x, int y, SDL_Surface *source, SDL_Surface *dest, SDL_Rect *clip = NULL);

#endif