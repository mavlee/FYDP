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

char* rectPath = "res/images/rect.png";
bool rectLoaded = false;
int rectWidth;
int rectHeight;
GLuint* rectTexture;
GLuint RECTANGLE = 3;

Canvas::Canvas(int width, int height) {
  this->width = width;
  this->height = height;
  screen = NULL;
  initCanvas();
}

Canvas::~Canvas() {
  delete skyboxTexture;
  delete rectTexture;
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
    if (loadImage(rectPath, width, height, hasAlpha, rectTexture, &RECTANGLE)) {
      rectLoaded = true;
      rectWidth = width;
      rectHeight = height;
    }
  }

  // Load player silhouette
  glGenTextures(1, &playerDepthId);
  glBindTexture(GL_TEXTURE_2D, playerDepthId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (GLvoid*) depthData);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Set the caption on the window.
  SDL_WM_SetCaption("Tempo", NULL);
}

void Canvas::cleanupCanvas() {
  clearImage(&SKYBOX);
  clearImage(&RECTANGLE);
  // Quit SDL. Also handles cleanup of the screen object.
  SDL_Quit();
}

void Canvas::draw(float shiftZ, std::list<Cube*> obstacles) {
  // Clear color buffer & depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw the skybox before anything else is drawn.
  if (skyboxLoaded) {
    drawSkybox(skyboxWidth, skyboxHeight, shiftZ);
  }

  glPushMatrix();
  drawObstacles(obstacles);
  glPopMatrix();

  glPushMatrix();
  drawPlayer();
  glPopMatrix();
}

void Canvas::drawPlayer() {
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_2D,GL_TEXTURE_ENV_MODE,GL_MODULATE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glBindTexture(GL_TEXTURE_2D, playerDepthId);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (GLvoid*)depthData);
  glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-SCREEN_WIDTH/2, -SCREEN_HEIGHT/2, 1200.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 1200.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 1200.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(SCREEN_WIDTH/2, -SCREEN_HEIGHT/2, 1200.0f);
  glEnd();
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

  double e = 0.001;

  glBindTexture(GL_TEXTURE_2D, SKYBOX);
  glBegin(GL_QUADS);
  // Four sides
  // Front
  glTexCoord2f( 1.0/4.0, 2.0/3.0 ); glVertex3f(          -size/2,          -size/2,  -size );
  glTexCoord2f( 2.0/4.0, 2.0/3.0 ); glVertex3f(           size/2,          -size/2,  -size );
  glTexCoord2f( 2.0/4.0, 1.0/3.0 ); glVertex3f(           size/2,           size/2,  -size );
  glTexCoord2f( 1.0/4.0, 1.0/3.0 ); glVertex3f(          -size/2,           size/2,  -size );

  // Left
  glTexCoord2f( 0.0/4.0, 2.0/3.0 ); glVertex3f(  -SCREEN_WIDTH/2,          -size/2,    0.0 );
  glTexCoord2f( 1.0/4.0, 2.0/3.0 ); glVertex3f(  -SCREEN_WIDTH/2,          -size/2,  -size );
  glTexCoord2f( 1.0/4.0, 1.0/3.0 ); glVertex3f(  -SCREEN_WIDTH/2,           size/2,  -size );
  glTexCoord2f( 0.0/4.0, 1.0/3.0 ); glVertex3f(  -SCREEN_WIDTH/2,           size/2,    0.0 );

  // Right
  glTexCoord2f( 2.0/4.0, 2.0/3.0 ); glVertex3f(   SCREEN_WIDTH/2,          -size/2,  -size );
  glTexCoord2f( 3.0/4.0, 2.0/3.0 ); glVertex3f(   SCREEN_WIDTH/2,          -size/2,    0.0 );
  glTexCoord2f( 3.0/4.0, 1.0/3.0 ); glVertex3f(   SCREEN_WIDTH/2,           size/2,    0.0 );
  glTexCoord2f( 2.0/4.0, 1.0/3.0 ); glVertex3f(   SCREEN_WIDTH/2,           size/2,  -size );

  // Bottom
  glTexCoord2f( 1.0/4.0, 3.0/3.0 ); glVertex3f(          -size/2, -SCREEN_HEIGHT/2,    0.0 );
  glTexCoord2f( 2.0/4.0, 3.0/3.0 ); glVertex3f(           size/2, -SCREEN_HEIGHT/2,    0.0 );
  glTexCoord2f( 2.0/4.0, 2.0/3.0 ); glVertex3f(           size/2, -SCREEN_HEIGHT/2,  -size );
  glTexCoord2f( 1.0/4.0, 2.0/3.0 ); glVertex3f(          -size/2, -SCREEN_HEIGHT/2,  -size );

  // Top
  glTexCoord2f( 1.0/4.0, 1.0/3.0 ); glVertex3f(          -size/2,  SCREEN_HEIGHT/2,  -size );
  glTexCoord2f( 2.0/4.0, 1.0/3.0 ); glVertex3f(           size/2,  SCREEN_HEIGHT/2,  -size );
  glTexCoord2f( 2.0/4.0, 0.0/3.0 ); glVertex3f(           size/2,  SCREEN_HEIGHT/2,    0.0 );
  glTexCoord2f( 1.0/4.0, 0.0/3.0 ); glVertex3f(          -size/2,  SCREEN_HEIGHT/2,    0.0 );

  // We never rotate the screen. No need to draw the back surface.

  glEnd();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}

void Canvas::drawObstacles(std::list<Cube*> obstacles) {
  for (std::list<Cube*>::iterator i = obstacles.begin(); i != obstacles.end(); i++) {
    if (rectLoaded) {
      (*i)->draw(&RECTANGLE);
    } else {
      (*i)->draw();
    }
  }
}
