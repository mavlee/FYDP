#include "LOpenGL.h"
#include <stdio.h>
#include "canvas.h"
#include "game.h"

const int COLOR_MODE_CYAN = 0;
const int COLOR_MODE_MULTI = 1;
int gColorMode = COLOR_MODE_CYAN;

GLfloat gProjectionScale = 1.f;
GLfloat cameraX = 0.f;
GLfloat cameraY = 0.f;
float angle = 0.f;

Game::Game(int width, int height) {
  canvasWidth = width;
  canvasHeight = height;
  canvas = new Canvas(width, height);
  canvas->initCanvas();
}

Game::~Game() {
  canvas->cleanupCanvas();
  delete canvas;
}

void Game::draw() {
  // Clear color buffer
  glClear( GL_COLOR_BUFFER_BIT );

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glPushMatrix();

  glTranslatef(canvasWidth / 2.f, canvasHeight / 2.f, 0.f);
  glRotatef(angle, 0.f, 1.f, 1.f);

  // Render quad
  glBegin( GL_QUADS );
  // back face
  if (gColorMode == COLOR_MODE_MULTI) {
    glColor3f( 1.f, 0.f, 0.f);
  } else {
    glColor3f( 0.f, 1.f, 1.f);
  }

  glVertex3f( -50.f, -50.f, -50.f);
  glVertex3f(  50.f, -50.f, -50.f);
  glVertex3f(  50.f,  50.f, -50.f);
  glVertex3f( -50.f,  50.f, -50.f);

  // front face
  if (gColorMode == COLOR_MODE_MULTI) {
    glColor3f( 0.f, 1.f, 0.f);
  }
  glVertex3f( -50.f, -50.f,  50.f);
  glVertex3f(  50.f, -50.f,  50.f);
  glVertex3f(  50.f,  50.f,  50.f);
  glVertex3f( -50.f,  50.f,  50.f);

  // left face
  if (gColorMode == COLOR_MODE_MULTI) {
    glColor3f( 0.f, 0.f, 1.f);
  }
  glVertex3f( -50.f, -50.f, -50.f);
  glVertex3f( -50.f, -50.f,  50.f);
  glVertex3f( -50.f,  50.f, -50.f);
  glVertex3f( -50.f,  50.f,  50.f);

  // right face
  if (gColorMode == COLOR_MODE_MULTI) {
    glColor3f( 1.f, 1.f, 0.f);
  }
  glVertex3f(  50.f, -50.f, -50.f);
  glVertex3f(  50.f, -50.f,  50.f);
  glVertex3f(  50.f,  50.f, -50.f);
  glVertex3f(  50.f,  50.f,  50.f);

  // top face
  if (gColorMode == COLOR_MODE_MULTI) {
    glColor3f( 0.f, 1.f, 1.f);
  }
  glVertex3f( -50.f,  50.f, -50.f);
  glVertex3f( -50.f,  50.f,  50.f);
  glVertex3f(  50.f,  50.f, -50.f);
  glVertex3f(  50.f,  50.f,  50.f);

  // bottom face
  if (gColorMode == COLOR_MODE_MULTI) {
    glColor3f( 1.f, 0.f, 1.f);
  }
  glVertex3f( -50.f, -50.f, -50.f);
  glVertex3f( -50.f, -50.f,  50.f);
  glVertex3f(  50.f, -50.f, -50.f);
  glVertex3f(  50.f, -50.f,  50.f);
  glEnd();

  // Update screen
  SDL_GL_SwapBuffers();
}

void Game::update() {
  angle += 0.05f;
}

void Game::handleKeys(int key) {
  bool translation = false;
  switch (key) {
    case SDLK_q:
      //Toggle color mode
      if (gColorMode == COLOR_MODE_CYAN) {
        gColorMode = COLOR_MODE_MULTI;
      } else {
        gColorMode = COLOR_MODE_CYAN;
      }
      break;
    case SDLK_w:
      translation = true;
      cameraY -= 16.f;
      break;
    case SDLK_s:
      translation = true;
      cameraY += 16.f;
      break;
    case SDLK_a:
      translation = true;
      cameraX -= 16.f;
      break;
    case SDLK_d:
      translation = true;
      cameraX += 16.f;
      break;
    case SDLK_e:
      // Cycle through projection scales
      if (gProjectionScale == 1.f) {
        // Zoom out
        gProjectionScale = 2.f;
      } else if( gProjectionScale == 2.f ) {
        // Zoom in
        gProjectionScale = 0.5f;
      }
      else if( gProjectionScale == 0.5f ) {
        //Regular zoom
        gProjectionScale = 1.f;
      }

      //Update projection matrix
      glMatrixMode( GL_PROJECTION );
      glLoadIdentity();
      glOrtho( 0.0, canvasWidth * gProjectionScale, canvasHeight * gProjectionScale, 0.0, 1.0, -1.0 );
      glTranslatef((gProjectionScale - 1.f) * canvasWidth / 2.f,
          (gProjectionScale - 1.f) * canvasHeight / 2.f, 0.f);
      break;
    default:
      break;
  }
  if (translation) {
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glLoadIdentity();

    glTranslatef(cameraX, cameraY, 0.f);
    glPushMatrix();
  }
}
