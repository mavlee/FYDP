#include "LOpenGL.h"
#include <stdio.h>
#include "canvas.h"

Canvas::Canvas(int width, int height) {
  this->width = width;
  this->height = height;
  screen = NULL;
  initCanvas();
}

void Canvas::initCanvas() {
  // Start SDL
  if (SDL_Init( SDL_INIT_EVERYTHING ) == -1 ) {
    //return false;
  }

  // SDL_SWSURFACE implies that the surface is set up in software memory.
  screen = SDL_SetVideoMode(width, height, SCREEN_BPP, SDL_OPENGL);
  if (screen == NULL) {
    //return false;
  }

  //Initialize SDL ttf. Must be called prior to all SDL_ttf functions.
  if (TTF_Init() == -1) {
    //return false;
  }

  /*
  if (!initClipper()) {
    return false;
  }
  */

  /*
  if (!initGL()) {
    return false;
  }
  */

  // Initialize Projection Matrix
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0.0, width, height, 0.0, 100.0, -100.0);

  // Initialize Modelview Matrix
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  glPushMatrix();

  // Initialize clear color
  glClearColor( 0.f, 0.f, 0.f, 1.f );

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  //Check for error
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    printf( "Error initializing OpenGL! %s\n", gluErrorString( error ) );
  }

  // Set the caption on the window.
  SDL_WM_SetCaption("Tempo", NULL);
}

void Canvas::cleanupCanvas() {
  // Quit SDL. Also handles cleanup of the screen object.
  SDL_Quit();
}

void Canvas::draw() {

}
