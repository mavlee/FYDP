#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <string>
#include <sstream>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;

SDL_Surface *loadImage(std::string filename);
void applySurface(int x, int y, SDL_Surface *source, SDL_Surface *dest, SDL_Rect *clip = NULL);