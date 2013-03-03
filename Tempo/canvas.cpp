#include "LOpenGL.h"
#include <stdio.h>
#include "canvas.h"
#include "constants.h"
#include "image_loader.h"

char* skyboxPath = "res/images/skybox.png";
bool skyboxLoaded = false;
int skyboxWidth;
int skyboxHeight;
GLuint* skyboxTexture;
GLuint SKYBOX = 2;

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

  // Initialize Projection Matrix
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  glRotatef(SCREEN_TILT, 1, 0, 0);

  float offset = 0.0f;
  glFrustum( -width/2, width/2, height/2 + offset, -height/2 + offset, Z_NEAR, Z_FAR);

  //glRotatef(-40.0f, 1, 0, 0);

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
  } else {
    printf( "Initialized OpenGL!\n");

    initImageLoader();

    //TODO: load skybox texture
    int width, height;
    bool hasAlpha;
    if (loadImage(skyboxPath, width, height, hasAlpha, skyboxTexture, &SKYBOX)) {
      skyboxLoaded = true;
      skyboxWidth = width;
      skyboxHeight = height;
    }
  }

  // Set the caption on the window.
  SDL_WM_SetCaption("Tempo", NULL);
}

void Canvas::cleanupCanvas() {
  clearImage(&SKYBOX);
  // Quit SDL. Also handles cleanup of the screen object.
  SDL_Quit();
}

void Canvas::draw(float shiftZ) {
  // Clear color buffer & depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw the skybox before anything else is drawn.
  if (skyboxLoaded) {
    drawSkybox(skyboxWidth, skyboxHeight, shiftZ);
  }
}

void Canvas::drawSkybox(int width, int height, float shiftZ) {

  // Initialize Projection Matrix
  glMatrixMode( GL_PROJECTION );
  // Save current matrix.
  glPushMatrix();
  glLoadIdentity();

  glFrustum( -1, 1, -1, 1, 1, Z_FAR);

  glMatrixMode(GL_MODELVIEW);

  glPushAttrib(GL_ENABLE_BIT);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  //Set texture parameters
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

  double size;
  if (width >= height) {
    size = width;
  } else {
    size = height;
  }

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  //glRotatef(90.0, 1.0, 1.0, 0.0);
  //glTranslatef(0, 0, shiftZ);

  //glRotatef(-SCREEN_TILT, 1, 0, 0);

  double e = 0.001;

  glBindTexture(GL_TEXTURE_2D, SKYBOX);
  glBegin(GL_QUADS);
    // Four sides
    // Front
    glTexCoord2f( 1.0/4.0, 2.0/3.0 ); glVertex3f(   -size/2, -size/2,  -size );
    glTexCoord2f( 2.0/4.0, 2.0/3.0 ); glVertex3f(    size/2, -size/2,  -size );
    glTexCoord2f( 2.0/4.0, 1.0/3.0 ); glVertex3f(    size/2,  size/2,  -size );
    glTexCoord2f( 1.0/4.0, 1.0/3.0 ); glVertex3f(   -size/2,  size/2,  -size );

    // Left
    glTexCoord2f( 0.0/4.0, 2.0/3.0 ); glVertex3f(  -SCREEN_WIDTH/2, -size/2,    0.0 );
    glTexCoord2f( 1.0/4.0, 2.0/3.0 ); glVertex3f(  -SCREEN_WIDTH/2, -size/2,  -size );
    glTexCoord2f( 1.0/4.0, 1.0/3.0 ); glVertex3f(  -SCREEN_WIDTH/2,  size/2,  -size );
    glTexCoord2f( 0.0/4.0, 1.0/3.0 ); glVertex3f(  -SCREEN_WIDTH/2,  size/2,    0.0 );

    // Right
    glTexCoord2f( 2.0/4.0, 2.0/3.0 ); glVertex3f(   SCREEN_WIDTH/2, -size/2,  -size );
    glTexCoord2f( 3.0/4.0, 2.0/3.0 ); glVertex3f(   SCREEN_WIDTH/2, -size/2,    0.0 );
    glTexCoord2f( 3.0/4.0, 1.0/3.0 ); glVertex3f(   SCREEN_WIDTH/2,  size/2,    0.0 );
    glTexCoord2f( 2.0/4.0, 1.0/3.0 ); glVertex3f(   SCREEN_WIDTH/2,  size/2,  -size );

    // Bottom
    glTexCoord2f( 1.0/4.0, 3.0/3.0 ); glVertex3f(  -size/2, -SCREEN_HEIGHT/2,    0.0 );
    glTexCoord2f( 2.0/4.0, 3.0/3.0 ); glVertex3f(   size/2, -SCREEN_HEIGHT/2,    0.0 );
    glTexCoord2f( 2.0/4.0, 2.0/3.0 ); glVertex3f(   size/2, -SCREEN_HEIGHT/2,  -size );
    glTexCoord2f( 1.0/4.0, 2.0/3.0 ); glVertex3f(  -size/2, -SCREEN_HEIGHT/2,  -size );

    // Top
    glTexCoord2f( 1.0/4.0, 1.0/3.0 ); glVertex3f(  -size/2,  SCREEN_HEIGHT/2,  -size );
    glTexCoord2f( 2.0/4.0, 1.0/3.0 ); glVertex3f(   size/2,  SCREEN_HEIGHT/2,  -size );
    glTexCoord2f( 2.0/4.0, 0.0/3.0 ); glVertex3f(   size/2,  SCREEN_HEIGHT/2,    0.0 );
    glTexCoord2f( 1.0/4.0, 0.0/3.0 ); glVertex3f(  -size/2,  SCREEN_HEIGHT/2,    0.0 );

    // We never rotate the screen. No need to draw the back surface.

  glEnd();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}
