#include <stdio.h>
#include <string>
#include "LOpenGL.h"
#include "canvas.h"
#include "game.h"
#include "objects.h"
#include "LText.h"
#include "inc/sound/sound_includes.h"
#include "music_handler.h"
#include "util.h"
#include "file_dialog.h"
#include "highscore.h"

#define LEFT 0
#define RIGHT 1

#define LEFT_KEY        SDLK_a
#define RIGHT_KEY       SDLK_d
#define COLOUR_MODE_KEY SDLK_q
#define PERSPECTIVE_KEY SDLK_e
#define PAUSE_KEY       ' '
#define RESTART_KEY     SDLK_r

Game::Game(int width, int height) {
  canvasWidth = width;
  canvasHeight = height;

  canvas = new Canvas(width, height);
  musicHandler = new MusicHandler();
  fpsText = new Text(width, height);
  comboLevelText = new Text(width, height);
  pointsText = new Text(width, height);
  // TODO this has nothing to do with the near plane
  playerCube = new Cube(0.f, 0.f, -(Z_NEAR + 200.f), 100.f, 100.f, 100.f, Cube::Multi);

  reset();
}

Game::~Game() {
  canvas->cleanupCanvas();
  if (canvas) delete canvas;
  if (playerCube) delete playerCube;
  if (musicHandler) delete musicHandler;
  if (fpsText) delete fpsText;
  if (comboLevelText) delete comboLevelText;
  if (pointsText) delete pointsText;
}

// resets the game so a new game can be started
void Game::reset() {
  delete canvas;
  canvas = new Canvas(canvasWidth, canvasHeight);
  points = 0;
  combo = 0;
  comboLevel = 1;
  lifeRemaining = TOTAL_LIFE_COUNT;

  isPaused = false;
  dirKeyPressed[LEFT] = dirKeyPressed[RIGHT] = false;
  restart = false;
  finished = false;

  cameraX = 0.f;
  cameraY = 0.f;
  gColorMode = COLOR_MODE_CYAN;
  gProjectionScale = 1.f;

  // clear obstacles
  obstacles.clear();

  shiftZ = 0.f;

  lastUpdate = 0;
  frames = 0;

  musicHandler->setMusicFile(selectMusicFileDialog());
  musicData = musicHandler->getPeakData();
  generateGameFeatures();

  timer.start();
  musicHandler->play();
}

// the game's main loop
int Game::execute() {
  SDL_Event event;
  bool quitGame = false;

  while (!quitGame) {
    while (SDL_PollEvent(&event)) {
      if (SDL_QUIT == event.type) {
        quitGame = true;
      } else {
        handleEvent(event);
      }
    }

    update();
    draw();
  }

  return 0;
}

// based on whatever music analysis gives us, generate game features
void Game::generateGameFeatures() {
  int last = -50;
  Cube* obstacle;
  for (int b = 0; b < musicData.size(); b++) {
    last = -50;
    for (int i = 0; i < musicData[b].size(); i++) {
      if (musicData[b][i] > PEAK_THRESHOLD && i - last > 0) {
        //float pos = SCREEN_WIDTH/2.f*(-1 + rand()%3);
        // TODO: this has nothing to do with the near plane
        float pos = -NUM_BANDS/2*125.f + b*125;
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
    lifeRemaining--;
  }
  if (combo % 1000 == 0 && combo > 0) {
    comboLevel++;
  }
  points += 1 * comboLevel;
}

void Game::draw() {
  if (!finished) {
  // TODO
  //
  // clean this up - canvas.prepForDrawing() maybe
  // and canvas.finishedDrawing()
  // should be using playerCube.draw() ??

  canvas->draw(shiftZ, obstacles, lifeRemaining);
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
  } else {
    canvas->drawHighscore(points, highscores, highscoreAchieved, lifeRemaining);
  }

  // Update screen
  SDL_GL_SwapBuffers();
}

void Game::update() {
  if (!finished) {
    frames++;
    int diff = timer.get_ticks() - lastUpdate;

    // TODO
    // update player cube position
    // check for collision
    if (checkForCollisions()) {
      printf("Collision\n");
    }

    // calculate score
    // TODO: calculate score according to time, and not the frequency that frames are drawn
    updateScore();

    // Update positions
    int xDir = 0;
    if (dirKeyPressed[LEFT]) xDir = -1;
    if (dirKeyPressed[RIGHT]) xDir = 1;
    cameraX += 1600.f * diff / 1000.f * xDir;

    shiftZ -= SHIFT_INTERVAL_PER_SECOND * diff / 1000;
    glTranslatef(0, 0, SHIFT_INTERVAL_PER_SECOND * diff / 1000);
    lastUpdate += diff;
  }

  // Check for end of song
  if (musicHandler->getPositionInSec() == musicHandler->getLengthInSec() || lifeRemaining == 0) {
    if (!finished) {
      string fileName = musicHandler->getMusicFile();
      fileName = fileName.substr(fileName.find_last_of('\\') + 1);
      fileName = fileName.substr(0, fileName.find(".mp3"));
      fileName = fileName + ".high";
      highscores = readHighScores(fileName);
      highscoreAchieved = insertHighScore(points);
      int i = 0;
      writeHighScores(fileName);
      finished = true;
    }
    if (restart) {
      reset();
    }
  }
}

void Game::handleEvent(SDL_Event& event) {
  switch (event.type) {

  case SDL_KEYDOWN:
    switch (event.key.keysym.sym) {
    case LEFT_KEY:
      dirKeyPressed[LEFT] = true;
      dirKeyPressed[RIGHT] = false;
      break;
    case RIGHT_KEY:
      dirKeyPressed[LEFT] = false;
      dirKeyPressed[RIGHT] = true;
      break;
    case PAUSE_KEY:
      isPaused = !isPaused;
      break;
    case COLOUR_MODE_KEY:
      //Toggle color mode
      if (gColorMode == COLOR_MODE_CYAN) {
        gColorMode = COLOR_MODE_MULTI;
      } else {
        gColorMode = COLOR_MODE_CYAN;
      }
      break;
    case PERSPECTIVE_KEY:
      // Cycle through projection scales
      if (gProjectionScale == 1.f) {
        // Zoom out
        gProjectionScale = 2.f;
      } else if( gProjectionScale == 2.f ) {
        // Zoom in
        gProjectionScale = 0.5f;
      } else if( gProjectionScale == 0.5f ) {
        // Regular zoom
        gProjectionScale = 1.f;
      }
      // Update projection matrix
      // TODO: move to canvas
      glMatrixMode( GL_PROJECTION );
      glLoadIdentity();
      glFrustum(-canvasWidth / 2 * gProjectionScale, canvasWidth / 2 * gProjectionScale,
        canvasHeight / 2 * gProjectionScale, -canvasHeight / 2 + gProjectionScale,
        Z_NEAR / gProjectionScale, Z_FAR / gProjectionScale);
      break;
    case RESTART_KEY:
      restart = true;
      break;
    }
    break;

  case SDL_KEYUP:
    switch (event.key.keysym.sym) {
    case LEFT_KEY:
      dirKeyPressed[LEFT] = false;
      break;
    case RIGHT_KEY:
      dirKeyPressed[RIGHT] = false;
      break;
    }
    break;
  }
}
