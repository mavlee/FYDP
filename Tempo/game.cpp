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
#define QUIT_KEY        SDLK_q
#define PERSPECTIVE_KEY SDLK_e
#define PAUSE_KEY       ' '
#define RESTART_KEY     SDLK_r
#define STOP_KEY        SDLK_ESCAPE

Game::Game(int width, int height) {
  canvasWidth = width;
  canvasHeight = height;

  canvas = new Canvas(width, height);
  musicHandler = new MusicHandler();
  // TODO remove after player collision has been done
  // also, this has nothing to do with the near plane
  playerCube = new Cube(0.f, 0.f, -OFFSET_FROM_CAMERA, SHAPE_X, SHAPE_Y, SHAPE_Z, Cube::Player);

  string song = selectMusicFileDialog();
  while (!strcmp(song.c_str(), "/0")) {
    song = selectMusicFileDialog();
  }
  reset(song);
}

Game::~Game() {
  if (canvas) delete canvas;
  if (playerCube) delete playerCube;
  if (musicHandler) delete musicHandler;
}

// resets the game so a new game can be started
void Game::reset(string song) {
  points = 0;
  combo = 0;
  comboLevel = 1;
  lifeRemaining = TOTAL_LIFE_COUNT;
  progressPct = 0.0f;

  isPaused = false;
  restart = false;
  finished = false;

  //TODO remove after player cube gone
  cameraX = 0.f;
  cameraY = 0.f;
  gProjectionScale = 1.f;

  // clear obstacles
  obstacles.clear();
  shiftZ = 0.f;
  lastUpdate = 0;
  frames = 0;

  if (strcmp(song.c_str(), "/0")) {
    printf("New song must be chosen");
  }
  musicHandler->setMusicFile(song);
  musicData = musicHandler->getPeakData();
  musicIntensityData = musicHandler->getIntensityData();
  generateGameFeatures();

  timer.start();
  starting = true;
}

// the game's main loop
int Game::execute() {
  SDL_Event event;
  quitGame = false;

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
  // temporary vector to keep track of peak locations
  vector<vector<int> > peakMarker(NUM_BANDS);
  for (int v = 0; v < NUM_BANDS; v++) {
    peakMarker[v].resize(musicData[0].size(), 0);
  }

  int last = 0;
  int last_superpeak = -1000;
  Cube* obstacle;
  for (int i = 0; i < musicData[0].size(); i++) {
    // used to get the peak value at an instant
    float value = 0;
    int m = -1;
    // used to count number of peaks
    int count = 0;
    // used to count number of superpeaks
    int count2 = 0;
    for (int v = 0; v < NUM_BANDS; v++) {
      if (musicData[v][i] > value) {
        value = musicData[v][i];
        m = v;
      }
      peakMarker[v][i] = 0;
    }
    if (m > -1) {
      for (int v = 0; v < NUM_BANDS; v++) {
        if (musicData[v][i] > 400 && i > SAMPLE_HISTORY*2) {
          printf("holyshit of %f on sample %d\n", musicData[v][i] / musicIntensityData[i], i);
          count2++;
        }
        if (musicData[v][i] / value > 0.9 && count < 2) {
          peakMarker[v][i] = 1;
          count++;
        }
      }
    }
    if (count2 > 1 && i - last_superpeak > 1400) {
      printf("holyshit on sample %d\n", i);
      for (int c = i-10; c < i; c++) {
        for (int v = 0; v < NUM_BANDS; v++) {
          peakMarker[v][c] = 0;
        }
      }
      for (int v = 0; v < NUM_BANDS; v++) {
        peakMarker[v][i] = 1;
      }
      last_superpeak = i;
    }
  }

  for (int i = 0; i < musicData[0].size(); i++) {
    for (int b = 0; b < NUM_BANDS; b++) {
      //if (musicData[b][i] > PEAK_THRESHOLD && (i - last > 10 || i == last)) {
      if (peakMarker[b][i] == 1 && (i - last > 10 || i == last)) {
        int x = b / 4;
        int y = b % 4;
        obstacle = new Cube(x, y, -(OFFSET_FROM_CAMERA + i*1.0*SHIFT_INTERVAL_PER_SECOND/musicHandler->getPeakDataPerSec()), SHAPE_X, SHAPE_Y, SHAPE_Z, Cube::ColourSet(rand() % 4 + 1));
        obstacles.push_back(obstacle);
        last = i;
      }
    }
  }
}

bool Game::checkForNegativeCollisions() {
  for (std::list<Cube*>::const_iterator iterator = obstacles.begin(), end = obstacles.end(); iterator != end; ++iterator) {
    if (!(*iterator)->collided) {
      if ((*iterator)->colour != currentColour) {
        if ((*iterator)->zFront > shiftZ + playerCube->zBack && (*iterator)->zBack < shiftZ + playerCube->zBack ) {
          if (((*iterator)->wRight > playerCube->wLeft + cameraX && (*iterator)->wLeft < playerCube->wLeft + cameraX)
            || ((*iterator)->wLeft < playerCube->wRight + cameraX && (*iterator)->wRight > playerCube->wRight + cameraX)) {
              (*iterator)->collided = true;
              printf("Collision detected at:\nCurrent Depth: %f\nCurrent Left: %f\nCurrent Right: %f\nDepth: %f\nLeft: %f\nRight: %f\n", shiftZ - playerCube->zBack, playerCube->wLeft + cameraX, playerCube->wRight + cameraX, (*iterator)->zFront, (*iterator)->wLeft, (*iterator)->wRight);
              return true;
          }
        }
      }
    }
  }

  return false;
}

bool Game::checkForBonusCollisions() {
  for (std::list<Cube*>::const_iterator iterator = obstacles.begin(), end = obstacles.end(); iterator != end; ++iterator) {
    if (!(*iterator)->collided) {
      if ((*iterator)->colour == currentColour) {
        if ((*iterator)->zFront > shiftZ + playerCube->zBack && (*iterator)->zBack < shiftZ + playerCube->zBack ) {
          if (((*iterator)->wRight > playerCube->wLeft + cameraX && (*iterator)->wLeft < playerCube->wLeft + cameraX)
            || ((*iterator)->wLeft < playerCube->wRight + cameraX && (*iterator)->wRight > playerCube->wRight + cameraX)) {
              (*iterator)->collided = true;
              printf("Collision detected at:\nCurrent Depth: %f\nCurrent Left: %f\nCurrent Right: %f\nDepth: %f\nLeft: %f\nRight: %f\n", shiftZ - playerCube->zBack, playerCube->wLeft + cameraX, playerCube->wRight + cameraX, (*iterator)->zFront, (*iterator)->wLeft, (*iterator)->wRight);
              return true;
          }
        }
      }
    }
  }

  return false;
}

void Game::updateScore() {
  bool collision = checkForNegativeCollisions();

  combo++;
  if (collision) {
    combo = 0;
    comboLevel = 1;
    lifeRemaining--;
  }
  if (combo % 1000 == 0 && combo > 0) {
    comboLevel++;
  }
  collision = checkForBonusCollisions();
  points += (1 + 100 * collision) * comboLevel;
}

void Game::draw() {
  if (!finished) {
    canvas->draw(shiftZ, obstacles, lifeRemaining, progressPct, currentColour);

    // Current method to indicate colour to hit. Integrate colour into something else later
    // TODO remove
    glBegin(GL_QUADS);
      glColor3fv(cubeColours[currentColour][0]); glVertex3f(  SCREEN_WIDTH/2       , -SCREEN_HEIGHT/2 , -1100.0f );
      glColor3fv(cubeColours[currentColour][0]); glVertex3f(  SCREEN_WIDTH/2 - 100 , -SCREEN_HEIGHT/2 , -1100.0f );
      glColor3fv(cubeColours[currentColour][0]); glVertex3f(  SCREEN_WIDTH/2 - 100 ,  SCREEN_HEIGHT/2 , -1100.0f );
      glColor3fv(cubeColours[currentColour][0]); glVertex3f(  SCREEN_WIDTH/2       ,  SCREEN_HEIGHT/2 , -1100.0f );
    glEnd();
    glPopMatrix();


  } else {
    canvas->drawHighscore(points, highscores, highscoreAchieved, lifeRemaining);
  }
}

void Game::update() {
  if (isPaused) {
    lastUpdate = timer.get_ticks();
    return;
  }

  if (!finished) {
    frames++;
    int diff = timer.get_ticks() - lastUpdate;

    if (timer.get_ticks() < START_DELAY) {
      //return;
    } else {
      if (starting) {
        musicHandler->play();
        starting = false;
        lastUpdate = timer.get_ticks();
      } else {
        // calculate score
        // TODO: calculate score according to time, and not the frequency that frames are drawn
        updateScore();

        // Update positions
        shiftZ += SHIFT_INTERVAL_PER_SECOND * diff / 1000;
        lastUpdate += diff;

        // Update text
        canvas->setFPSText(frames * 1.0 / (timer.get_ticks()/1000));
        canvas->setPointsText(points);
        canvas->setComboLevelText(comboLevel);
      }
    }

    progressPct = musicHandler->getPositionInSec() / musicHandler->getLengthInSec();

    // change colour every 5%
    if (progressPct >= lastColourChange + 0.05f) {
      Cube::ColourSet newColour = Cube::ColourSet(rand() % 4 + 1);
      while (currentColour == newColour) {
        newColour = Cube::ColourSet(rand() % 4 + 1);
      }
      currentColour = newColour;
      lastColourChange = progressPct;
    }
  }

  // Check for end of song
  if (musicHandler->getPositionInSec() == musicHandler->getLengthInSec() || lifeRemaining == 0) {
    if (lifeRemaining == 0) {
      musicHandler->pause();
    }
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
    string song = selectMusicFileDialog();
    while (!strcmp(song.c_str(), "/0")) {
      song = selectMusicFileDialog();
    }
    reset(song);
  }
}

void Game::handleEvent(SDL_Event& event) {
  switch (event.type) {

  case SDL_KEYDOWN:
    switch (event.key.keysym.sym) {
    case PAUSE_KEY:
      isPaused = !isPaused;
      if (isPaused) {
        musicHandler->pause();
      } else {
        musicHandler->play();
      }
      break;
    case QUIT_KEY:
      quitGame = true;
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
    case STOP_KEY:
      musicHandler->pause();
      timer.pause();
      string song = selectMusicFileDialog();
      if (!strcmp(song.c_str(), "/0")) {
        reset(song);
      } else {
        // User pressed cancel, move along
        musicHandler->play();
        timer.unpause();
      }
      break;
    }
    break;

  case SDL_KEYUP:
    switch (event.key.keysym.sym) {
    }
    break;
  }
}
