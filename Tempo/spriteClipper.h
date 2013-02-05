#include "surfaceHelper.h"
/*
 *	Set the boundaries for clipping the sprite map.
 */
void setClipBounds();

SDL_Rect *getClipBounds();

bool initClipper();

SDL_Surface* getSpriteMap();

void cleanUpClipper();