#include "LOpenGL.h"
#include <stdio.h>
#include "canvas.h"
#include "game.h"
#include "constants.h"
#include "objects.h"
#include "LText.h"

int gColorMode = COLOR_MODE_CYAN;

GLfloat gProjectionScale = 1.f;
GLfloat cameraX = 0.f;
GLfloat cameraY = 0.f;
float angle = 0.f;

Text *text;

float avgFps;

Game::Game(int width, int height) {
  canvasWidth = width;
  canvasHeight = height;
  canvas = new Canvas(width, height);
  canvas->initCanvas();

  // Instantiate components displayed on the screen
  initCube();
  text = new Text(width, height);
}

Game::~Game() {
  canvas->cleanupCanvas();
  delete canvas;
}

// Wien's stuff goes here
void Game::analyzeMusic() {

}

// based on whatever music analysis gives us, generate game features
void Game::generateGameFeatures() {

}

void Game::draw() {
  // Clear color buffer & depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();

  glTranslatef(canvasWidth/2 + cameraX, canvasHeight/2 + cameraY, 0);
  glRotatef(angle, 0.f, 1.f, 1.f);

  int i, j;
  Cube cube = getCube();
  int currentVer;
  // Render the cube
  glBegin( GL_QUADS );
  for (i = 0; i < cube.nFaces; i++) {
	  for (j = 0; j < 4; j++) {
		  currentVer = cube.face[i].ver[j];

		  // back face
		  if (gColorMode == COLOR_MODE_MULTI) {
			glColor3fv(cube.ver[currentVer].col);
		  } else {
			glColor3f( 0.f, 1.f, 1.f);
		  }
		  glVertex3fv(cube.ver[currentVer].pos);
	  }
  }
  glEnd();
  glTranslatef(-(canvasWidth/2 + cameraX), -(canvasHeight/2 + cameraY), 0);
  
  glPopMatrix();

  std::stringstream fps_caption;
  fps_caption << "Average FPS: " << avgFps;
  text->renderText(canvasWidth, canvasHeight, 0, canvasHeight - 50, fps_caption.str());

  // Update screen
  SDL_GL_SwapBuffers();
}

void Game::update(int nFrames, float timeElapsed) {
  angle += 0.1f;
  if (timeElapsed != 1.0f) {
	avgFps = nFrames / timeElapsed;
  }
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
      //glTranslatef((gProjectionScale - 1.f) * canvasWidth / 2.f,
      //    (gProjectionScale - 1.f) * canvasHeight / 2.f, 0.f);
      break;
    default:
      break;
  }
}
