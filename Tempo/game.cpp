#include "LOpenGL.h"
#include <stdio.h>
#include "canvas.h"
#include "game.h"
#include "constants.h"
#include "objects.h"
#include "LText.h"
#include <string>
#include "inc/sound/sound_includes.h"
#include "music_handler.h"

Game::Game(int width, int height, std::string musicFile) {
  canvasWidth = width;
  canvasHeight = height;
  canvas = new Canvas(width, height);
  musicHandler = new MusicHandler();
  fpsText = new Text(width, height);
  comboLevelText = new Text(width, height);
  pointsText = new Text(width, height);

  // player cube
  playerCube = new Cube(0.f, 0.f, -(Z_NEAR + 200.f), 100.f, 100.f, 100.f, Cube::Multi);

  /*
  if (strcmp(musicFile.c_str(), "") != 0) {
    musicHandler->setMusicFile("C:\\FYDP\\Tempo\\res\\music\\clocks.mp3");
  } else {
    //musicHandler->setMusicFile("res/music/clocks.mp3");
    if (strcmp(musicFile.c_str(), "C:\\FYDP\\Tempo\\res\\music\\clocks.mp3") != 0) {
      printf("this is the right file");
      musicHandler->setMusicFile(musicFile);
    } else {
      printf("reverting back to clocks\n");
      musicHandler->setMusicFile("C:\\FYDP\\Tempo\\res\\music\\clocks.mp3");
    }
  }
  */

  reset();
}

Game::~Game() {
  canvas->cleanupCanvas();
  delete canvas;
  delete playerCube;
  delete musicHandler;
  delete fpsText;
  delete comboLevelText;
  delete pointsText;
}

// resets the game so a new game can be started
void Game::reset() {
  delete canvas;
  canvas = new Canvas(canvasWidth, canvasHeight);
  points = 0;
  combo = 0;
  comboLevel = 1;

  cameraX = 0.f;
  cameraY = 0.f;
  gColorMode = COLOR_MODE_CYAN;
  gProjectionScale = 1.f;

  // clear obstacles
  obstacles.clear();

  shiftZ = 0.f;
  lastPeakTime = 0;

  lastUpdate = 0;
  frames = 0;

  musicHandler->setMusicFile("res/music/simpletest.mp3");
  musicData = musicHandler->getPeakData();
  generateGameFeatures();

  timer.start();
  musicHandler->play();
}

// based on whatever music analysis gives us, generate game features
void Game::generateGameFeatures() {
  int last = -50;
  Cube* obstacle;
  for (int b = 0; b < musicData.size(); b++) {
    last = -50;
    for (int i = 0; i < musicData[b].size(); i++) {
      if (musicData[b][i] > PEAK_THRESHOLD && i - last > 0) {
        float pos = SCREEN_WIDTH/2.f*(-1 + rand()%3);
        obstacle = new Cube(pos, 0.f, -(Z_NEAR + 200.f + i*1.0*SHIFT_INTERVAL_PER_SECOND/musicHandler->getPeakDataPerSec()), 100.f, 100.f, 100.f, Cube::Multi);
        obstacles.push_back(obstacle);
        last = i;
      }
    }
  }
}

// this probably shouldn't be void in the end, some tamper with points somehow too
bool Game::checkForCollisions() {
  for (std::list<Cube*>::const_iterator iterator = obstacles.begin(), end = obstacles.end(); iterator != end; ++iterator) {
    if (!(*iterator)->collided) {
      if ((*iterator)->zNear + Z_NEAR > shiftZ + playerCube->zFar + Z_NEAR && (*iterator)->zFar + Z_NEAR < shiftZ + playerCube->zFar + Z_NEAR) {
        if (((*iterator)->wRight > playerCube->wLeft + cameraX && (*iterator)->wLeft < playerCube->wLeft + cameraX)
          || ((*iterator)->wLeft < playerCube->wRight + cameraX && (*iterator)->wRight > playerCube->wRight + cameraX)) {
            (*iterator)->collided = true;
            printf("Collision detected at:\nCurrent Depth: %f\nCurrent Left: %f\nCurrent Right: %f\nDepth: %f\nLeft: %f\nRight: %f\n", shiftZ - playerCube->zFar, playerCube->wLeft + cameraX, playerCube->wRight + cameraX, (*iterator)->zNear, (*iterator)->wLeft, (*iterator)->wRight);
            return true;
        }
      }
    }
  }

  return false;
}

void Game::updateScore() {
  bool collision = checkForCollisions();

  combo++;
  if (collision) {
    combo = 0;
    comboLevel = 1;
  }
  if (combo % 1000 == 0) {
    comboLevel++;
  }
  points += 1 * comboLevel;
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

  /*
  * Someone move all the drawing magic into canvas.
  */
  canvas->draw(shiftZ, obstacles);

  // Render the cube
  glPushMatrix();
  glTranslatef(cameraX, cameraY, 0);
  glTranslatef(0, 0, shiftZ);

  int i, j;
  int currentVer;
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

  std::stringstream fps_caption;
  fps_caption << "Average FPS: " << frames * 1.0 / (timer.get_ticks()/1000);
  fpsText->renderText(canvasWidth, canvasHeight, 0, canvasHeight - 50, fps_caption.str());
  std::stringstream points_caption;
  points_caption << "Points: " << points;
  pointsText->renderText(canvasWidth, canvasHeight, 0, canvasHeight - 100, points_caption.str());
  std::stringstream combo_caption;
  combo_caption << "Combo Level: " << comboLevel;
  pointsText->renderText(canvasWidth, canvasHeight, 0, canvasHeight - 150, combo_caption.str());

  // Update screen
  SDL_GL_SwapBuffers();
}

void Game::update() {
  frames++;
  int diff = timer.get_ticks() - lastUpdate;

  // TODO
  // update player cube position
  // check for collision
  bool col = checkForCollisions();

  if (col) {
    printf("Collision\n");
  }

  double pos = musicHandler->getPositionInSec();
  if (musicData[0][int(pos*musicHandler->getPeakDataPerSec())] > PEAK_THRESHOLD && pos - lastPeakTime > 0.1) {
    lastPeakTime = pos;
    /*
    cout << "time " << pos << endl;
    for (std::list<Cube*>::const_iterator iterator = obstacles.begin(), end = obstacles.end(); iterator != end; ++iterator) {
      cout << ((*iterator)->zNear + (*iterator)->zFar) / 2 - shiftZ << endl;
    }
    */
  }

  // calculate score
  updateScore();

  shiftZ -= SHIFT_INTERVAL_PER_SECOND * diff / 1000;
  glTranslatef(0, 0, SHIFT_INTERVAL_PER_SECOND * diff / 1000);
  lastUpdate += diff;

  draw();

  if (musicHandler->getPositionInSec() == musicHandler->getLengthInSec()) {
    reset();
  }
}

// WASD should move the playerCube, not the camera
void Game::handleKeys(int key, int* movementKeyDown) {
  bool translation = false;
  switch (key) {
  case ' ':
    musicHandler->pause();
    break;
  case SDLK_q:
    //Toggle color mode
    if (gColorMode == COLOR_MODE_CYAN) {
      gColorMode = COLOR_MODE_MULTI;
    } else {
      gColorMode = COLOR_MODE_CYAN;
    }
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
