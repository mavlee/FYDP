#include "LOpenGL.h"
#include <stdio.h>
#include "canvas.h"
#include "game.h"
#include "constants.h"
#include "objects.h"
#include "LText.h"

//float angle = 0.f;

Text *text;

Game::Game(int width, int height) {
  canvasWidth = width;
  canvasHeight = height;
  canvas = new Canvas(width, height);
  canvas->initCanvas();

  // Instantiate components displayed on the screen
  analyzeMusic();
  generateGameFeatures();
  text = new Text(width, height);

  // color stuff and camera
  gColorMode = COLOR_MODE_CYAN;

  gProjectionScale = 1.f;
  cameraX = 0.f;
  cameraY = 0.f;
}

Game::~Game() {
  canvas->cleanupCanvas();
  delete canvas;
  delete playerCube;
}

// Wien's stuff goes here
void Game::analyzeMusic() {

}

// based on whatever music analysis gives us, generate game features
void Game::generateGameFeatures() {
  // TODO: do this dynamically
  // this is filled with some static cubes for now
	playerCube = new Cube(0.f, 0.f, -(Z_NEAR + 200.f), 100.f, 100.f, 100.f, Cube::Multi);

	Cube* obstacle;
	obstacle = new Cube(-150.f, -50.f, -Z_FAR, 100.f, 100.f, 100.f, Cube::Multi);
	obstacles.push_back(obstacle);

	obstacle = new Cube(150.f, 200.f, -Z_FAR, 100.f, 100.f, 100.f, Cube::Multi);
	obstacles.push_back(obstacle);
}

// draw the obstacles
void Game::drawObstacles() {
  for (std::list<Cube*>::iterator i = obstacles.begin(); i != obstacles.end(); i++) {
    (*i)->draw();
    (*i)->shiftZ += 2.f;
  }
}

void Game::draw() {
  // Clear color buffer & depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();

  glTranslatef(0, 0, -(Z_NEAR + 200.f));
  //glRotatef(angle, 0.f, 1.f, 1.f);
  glTranslatef(0, 0, (Z_NEAR + 200.f));

  int i, j;
  int currentVer;
  // Render the cube
  glBegin( GL_QUADS );
  for (i = 0; i < playerCube->nFaces; i++) {
    for (j = 0; j < 4; j++) {
      currentVer = playerCube->face[i].ver[j];

      // back face
      if (gColorMode == COLOR_MODE_MULTI) {
        glColor3fv(playerCube->ver[currentVer].col);
      } else {
        glColor3f( 0.f, 1.f, 1.f);
      }
      glVertex3fv(playerCube->ver[currentVer].pos);
    }
  }
  glEnd();

  glPopMatrix();

  // Obstacles
  glPushMatrix();

  glTranslatef(-cameraX, -cameraY, 0);

  drawObstacles();

  glPopMatrix();

  std::stringstream fps_caption;
  fps_caption << "Average FPS: " << avgFps;
  text->renderText(canvasWidth, canvasHeight, 0, canvasHeight - 50, fps_caption.str());

  // Update screen
  SDL_GL_SwapBuffers();
}

void Game::update(int nFrames, float timeElapsed) {
  //angle += 0.1f;
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
        // Regular zoom
        gProjectionScale = 1.f;
      }

      // Update projection matrix
      glMatrixMode( GL_PROJECTION );
      glLoadIdentity();
	  glFrustum(-canvasWidth / 2 * gProjectionScale, canvasWidth / 2 * gProjectionScale,
		  canvasHeight / 2 * gProjectionScale, -canvasHeight / 2 + gProjectionScale,
		  Z_NEAR / gProjectionScale, Z_FAR / gProjectionScale);
      break;
    default:
      break;
  }
}
