#include "LOpenGL.h"
#include <stdio.h>
#include "canvas.h"
#include "game.h"
#include "constants.h"
#include "objects.h"
#include "LText.h"
#include <string>

Text *text;
Text *pointsText;

float shiftZ = 0.f;

Game::Game(int width, int height) {
  canvasWidth = width;
  canvasHeight = height;
  canvas = new Canvas(width, height);
  canvas->initCanvas();

  // Instantiate components displayed on the screen
  analyzeMusic();
  generateGameFeatures();
  text = new Text(width, height);
  pointsText = new Text(width, height);

  // color stuff and camera
  gColorMode = COLOR_MODE_CYAN;

  gProjectionScale = 1.f;
  cameraX = 0.f;
  cameraY = 0.f;

  points = 0;
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
	obstacle = new Cube(-150.f, 0.f, -(Z_NEAR + 5000.f), 100.f, 100.f, 100.f, Cube::Multi);
	obstacles.push_back(obstacle);

	obstacle = new Cube(150.f, 0.f, -Z_FAR, 100.f, 100.f, 100.f, Cube::Multi);
	obstacles.push_back(obstacle);
}

// this probably shouldn't be void in the end, some tamper with points somehow too
void Game::checkForCollisions() {

}

// draw the obstacles
void Game::drawObstacles() {
  for (std::list<Cube*>::iterator i = obstacles.begin(); i != obstacles.end(); i++) {
    (*i)->draw();
  }
}

void Game::draw() {
  // TODO
  //
  // clean this up - canvas.prepForDrawing() maybe
  // and canvas.finishedDrawing()
  // should be using playerCube.draw() ??
  // draw fps stuff at the end

  // Clear color buffer & depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();
  glTranslatef(cameraX, cameraY, 0);
  glTranslatef(0, 0, shiftZ);

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
  drawObstacles();
  glPopMatrix();

  std::stringstream fps_caption;
  fps_caption << "Average FPS: " << avgFps;
  text->renderText(canvasWidth, canvasHeight, 0, canvasHeight - 50, fps_caption.str());
  std::stringstream points_caption;
  points_caption << "Points: " << points;
  pointsText->renderText(canvasWidth, canvasHeight, 0, canvasHeight - 100, points_caption.str());

  // Update screen
  SDL_GL_SwapBuffers();
}

void Game::update(int nFrames, float timeElapsed) {
  if (timeElapsed != 1.0f) {
	  avgFps = nFrames / timeElapsed;
  }
  // TODO
  // update player cube position
  // update camera position
  // check for collision
  checkForCollisions();
  // calculate score
  points += 1;

  shiftZ -= SHIFT_INTERVAL;
  printf("player cube position: %f\n", shiftZ);

  glTranslatef(0, 0, SHIFT_INTERVAL);
}

// WASD should move the playerCube, not the camera
void Game::handleKeys(int key, int* movementKeyDown) {
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
		*movementKeyDown = 1;
      translation = true;
      cameraY -= 16.f;
      break;
    case SDLK_s:
		*movementKeyDown = 1;
      translation = true;
      cameraY += 16.f;
      break;
    case SDLK_a:
		*movementKeyDown = 1;
      translation = true;
      cameraX -= 16.f;
      break;
    case SDLK_d:
		*movementKeyDown = 1;
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
