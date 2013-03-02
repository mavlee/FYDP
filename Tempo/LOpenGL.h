/*This source code copyrighted by Lazy Foo' Productions (2004-2013)
and may not be redistributed without written permission.*/
//Version: 001

#ifndef LOPENGL_H
#define LOPENGL_H

#include <string>
#include <sstream>
#include <stdio.h>

#ifdef USE_MAC_INCLUDES

#include <gl.h>
#include <glu.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_opengl.h"

#else

#include <GL/gl.h>
#include <GL/glu.h>
#include <sdl/SDL.h>
#include <sdl/SDL_image.h>
#include <sdl/SDL_ttf.h>
#include <sdl/SDL_opengl.h>

#endif

#endif
