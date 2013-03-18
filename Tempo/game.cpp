#include <stdio.h>
#include <string>
#include <fstream>
#include <cmath>
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
#include "kinect_controller.h"

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
  kinectConnected = false;

  canvas = new Canvas(width, height);
  musicHandler = new MusicHandler();
  // TODO remove after player collision has been done
  // also, this has nothing to do with the near plane
  playerCube = new Cube(0.f, 0.f, -OFFSET_FROM_CAMERA, SHAPE_X, SHAPE_Y, SHAPE_Z, Cube::NO_COLOUR);

  string song = selectMusicFileDialog();
  while (!strcmp(song.c_str(), "\0")) {
    song = selectMusicFileDialog();
  }
  reset(song);
}

Game::~Game() {
  for (int i = 0; i < obstacles.size(); i++) {
    delete obstacles[i];
    obstacles[i] = NULL;
  }
  obstacles.clear();
  if (canvas) delete canvas;
  if (playerCube) delete playerCube;
  if (musicHandler) delete musicHandler;
}

// resets the game so a new game can be started
void Game::reset(string song) {
  // Init text
  points = 0;
  comboLevel = 1;
  canvas->setFPSText(0);
  canvas->setPointsText(0);
  canvas->setComboLevelText(0);

  progressPct = 0.0f;
  currentColour = Cube::NO_COLOUR;

  isPaused = false;
  restart = false;
  finished = false;

  //TODO remove after player cube gone
  cameraX = 0.f;
  cameraY = 0.f;
  gProjectionScale = 1.f;

  // clear obstacles
  prevObstacle = 0;
  for (int i = 0; i < obstacles.size(); i++) {
    delete obstacles[i];
    obstacles[i] = NULL;
  }
  obstacles.clear();
  closeCubes.clear();
  shiftZ = 0.f;
  lastUpdate = 0;
  frames = 0;

  // Arbitrary numbers for colours
  prevC1 = 1000;
  prevC2 = 1000;

  if (strcmp(song.c_str(), "\0")) {
    printf("New song must be chosen");
  }
  musicHandler->setMusicFile(song);
  musicData = musicHandler->getPeakData();
  musicIntensityData = musicHandler->getIntensityData();
  generateGameFeatures();

  timer.start();
  musicStarted = false;
}

// the game's main loop
int Game::execute() {
  SDL_Event event;
  quitGame = false;
  int lastHasPlayer = 0;

  while (!quitGame) {
    while (SDL_PollEvent(&event)) {
      if (SDL_QUIT == event.type) {
        quitGame = true;
      } else {
        handleEvent(event);
      }
    }

#ifndef USE_MAC_INCLUDES
    // Pauses the music when there is no player
    if (musicStarted && kinectConnected) {
      if (!hasPlayer() && !isPaused) {
        if (lastHasPlayer == 0) {
          lastHasPlayer = timer.get_ticks();
        } else if (timer.get_ticks() - lastHasPlayer
          >= MISSING_PLAYER_DELAY) {
            musicHandler->pause();
            isPaused = true;
        }
      } else if (hasPlayer() && isPaused) {
        musicHandler->play();
        isPaused = false;
        lastHasPlayer = 0;
      }
    }
#endif

    update();
    draw();
  }

  return 0;
}

// based on whatever music analysis gives us, generate game features
void Game::generateGameFeatures() {
  // temporary vector to keep track of peak locations
  vector<vector<vector<int> > >peakMarker(musicData[0].size(), vector<vector<int> >(NUM_ROWS, vector<int>(NUM_COLUMNS, 0)));

  // TODO: rename variables when i'm more awake
  int last = 0;
  int last_superpeak = -1000;
  int last_event = -1000;
  float maxholyshittotal = 0;
  Cube* obstacle;
  for (int i = 0; i < musicData[0].size(); i++) {
    // used to get the peak value at an instant
    float maxValue = 0;
    int maxValueIndex = -1;
    // used to count number of peaks
    int count = 0;
    // used to count number of superpeaks
    int wallCounter = 0;
    int twoColumnCounter = 0;
    int horizontalBarCounter = 0;
    int eventCounter = 0;
    float holyshittotal = 0;
    if (i - last_superpeak > 1400) maxholyshittotal = 0;
    for (int v = 0; v < NUM_BANDS; v++) {
      if (musicData[v][i] > maxValue) {
        maxValue = musicData[v][i];
        maxValueIndex = v;
      }
    }
    if (maxValueIndex > -1) {
      for (int v = 0; v < NUM_BANDS; v++) {
        if (musicData[v][i] > 200 && i > SAMPLE_HISTORY*2) {
          //printf("holyshit of %f on sample %d with intensity %f\n", musicData[v][i], i, musicIntensityData[i]);
          wallCounter++;
          holyshittotal += musicData[v][i] * musicIntensityData[i];
        } else if (musicData[v][i] > 50 && i > SAMPLE_HISTORY*2) {
          //printf("minor holyshit of %f on sample %d\n", musicData[v][i], i);
          eventCounter++;
        }
        // individual piece
        if (musicData[v][i] / maxValue > 0.9 && count < 1) {
          if (musicIntensityData[i] < 0.5) {
            //printf("low intensity on sample %d with intensity %f\n", i, musicIntensityData[i]);
            continue;
          }

          int rand_r = rand() % 4;
          int rand_c = 0;
          if (rand() % 2 == 1) {
            rand_c = 5;
          }
          peakMarker[i][rand_r][rand_c] = 1;
          count++;
        }
      }
    }
    // wall
    if (wallCounter > 1) {
      if (i - last_superpeak < 500 && holyshittotal > maxholyshittotal) {
        //printf("erase holyshit sample %d\n", last_superpeak);
        for (int r = 0; r < NUM_ROWS; r++) {
          for (int c = 0; c < NUM_COLUMNS; c++) {
            peakMarker[last_superpeak][r][c] = 0;
          }
        }
        int rand_r = rand() % 4;
        int rand_c = 0;
        if (rand() % 2 == 1) {
          rand_c = 5;
        }
        peakMarker[last_superpeak][rand_r][rand_c] = 1;
      }
      if ((i - last_superpeak < 500 && holyshittotal > maxholyshittotal) || i - last_superpeak > 1400) {
        //printf("holyshit on sample %d\n", i);
        for (int j = i-10; j < i; j++) {
          for (int r = 0; r < NUM_ROWS; r++) {
            for (int c = 0; c < NUM_COLUMNS; c++) {
              peakMarker[j][r][c] = 0;
            }
          }
        }
        for (int r = 0; r < NUM_ROWS; r++) {
          for (int c = 0; c < NUM_COLUMNS; c++) {
            peakMarker[i][r][c] = 1;
          }
        }
        last_superpeak = i;
        maxholyshittotal = holyshittotal;
      }
    } else if (eventCounter > 0 && i - last_event > 0) {
      int eventType = rand() % 8;
      int rand_c, rand_r, rand_c2, start;
      last_event = i;
      switch (eventType) {
        // wall with 2 column gap
        case 0:
          rand_c = rand() % 5;
          for (int r = 0; r < NUM_ROWS; r++)
            for (int c = 0; c < NUM_COLUMNS; c++)
              peakMarker[i][r][c] = 1;
          for (int r = 0; r < NUM_ROWS; r++) {
            peakMarker[i][r][rand_c] = 0;
            peakMarker[i][r][rand_c+1] = 0;
          }
          break;
        // horizontal bar
        case 1:
          rand_r = 0;
          if (rand() % 2 == 1)
            rand_r = 3;
          for (int c = 0; c < NUM_COLUMNS; c++)
            peakMarker[i][rand_r][c] = 1;
          break;
        // two columns
        case 2:
          rand_c = rand() % 6;
          rand_c2 = (rand_c + 1) % 6;
          for (int r = 0; r < NUM_ROWS; r++) {
            peakMarker[i][r][rand_c] = 1;
            peakMarker[i][r][rand_c2] = 1;
          }
          break;
        // 2 bottom left to top right diagonals
        case 3:
          peakMarker[i][0][2] = 1;
          peakMarker[i][1][1] = 1;
          peakMarker[i][2][0] = 1;
          peakMarker[i][0][5] = 1;
          peakMarker[i][1][4] = 1;
          peakMarker[i][2][3] = 1;
          peakMarker[i][3][2] = 1;
          break;
        // 2 top left to bottom right diagonals
        case 4:
          peakMarker[i][1][0] = 1;
          peakMarker[i][2][1] = 1;
          peakMarker[i][3][2] = 1;
          peakMarker[i][0][2] = 1;
          peakMarker[i][1][3] = 1;
          peakMarker[i][2][4] = 1;
          peakMarker[i][3][5] = 1;
          break;
        // weird t shape thing
        case 5:
          if (rand() % 2 == 1) {
            rand_c = rand() % 4;
            for (int r = 0; r < NUM_ROWS; r++)
              peakMarker[i][r][rand_c] = 1;
            peakMarker[i][1][rand_c+1] = 1;
            peakMarker[i][1][rand_c+2] = 1;
          } else {
            rand_c = rand() % 4 + 2;
            for (int r = 0; r < NUM_ROWS; r++)
              peakMarker[i][r][rand_c] = 1;
            peakMarker[i][1][rand_c-1] = 1;
            peakMarker[i][1][rand_c-2] = 1;
          }
          break;
        // diagonal box hax
        case 6:
          if (rand() % 2 == 1) {
            peakMarker[i][0][2] = 1;
            peakMarker[i][1][1] = 1;
            peakMarker[i][2][0] = 1;
            peakMarker[i][3][1] = 1;
            peakMarker[i][3][3] = 1;
            peakMarker[i][0][4] = 1;
            peakMarker[i][2][4] = 1;
            peakMarker[i][1][5] = 1;
          } else {
            peakMarker[i][0][1] = 1;
            peakMarker[i][1][0] = 1;
            peakMarker[i][2][1] = 1;
            peakMarker[i][3][2] = 1;
            peakMarker[i][0][3] = 1;
            peakMarker[i][1][4] = 1;
            peakMarker[i][2][5] = 1;
            peakMarker[i][3][4] = 1;
          }
          break;
        // half side and top bar
        case 7:
          start = 0;
          if (rand() % 2 == 1)
            start = 3;
          for (int c = start; c < NUM_COLUMNS; c++)
            for (int r = 0; r < NUM_ROWS; r++)
              peakMarker[i][r][c] = 1;
          for (int c = 0; c < NUM_COLUMNS; c++)
            peakMarker[i][0][c] = 1;
          break;
        default:
          break;
      }
    }
  }

  averageIntensity.resize(musicData[0].size());
  for (int i = 0; i < musicData[0].size(); i++) {
    for (int j = i; j < min(i+100,(int)musicData[0].size()); j++) {
      averageIntensity[i] += musicIntensityData[j] / min(100.0, double(musicData[0].size() - i));
    }
  }
  for (int i = 0; i < musicData[0].size(); i++) {
    if (averageIntensity[i] < 0.3) {
      averageIntensity[i] = 0.3;
    }
  }

  //vector<float> distances(musicData[0].size(), OFFSET_FROM_CAMERA);
  distances.resize(musicData[0].size());//, OFFSET_FROM_CAMERA);
  for (int i = 0; i < musicData[0].size(); i++) {
    distances[i] = 0;
  }
  for (int i = 0; i < musicData[0].size(); i++) {
    distances[i] += OFFSET_FROM_CAMERA;
    //distances[i] += i*1.0*SHIFT_INTERVAL_PER_SECOND/musicHandler->getPeakDataPerSec();
    for (int j = i; j < musicData[0].size(); j++) {
      distances[j] += averageIntensity[i]*SHIFT_INTERVAL_PER_SECOND/musicHandler->getPeakDataPerSec();
    }
  }

  for (int i = 0; i < musicData[0].size(); i++) {
    int color = 0;
    for (int r = 0; r < NUM_ROWS; r++) {
      for (int c = 0; c < NUM_COLUMNS; c++) {
        if (peakMarker[i][r][c] > 0 && (i - last > 10 || i == last)) {
          if (!color)
            color = generateColour();
          //obstacle = new Cube(c, r, -(OFFSET_FROM_CAMERA + i*1.0*SHIFT_INTERVAL_PER_SECOND/musicHandler->getPeakDataPerSec()), SHAPE_X, SHAPE_Y, SHAPE_Z, Cube::ColourSet(peakMarker[i][r][c]));
          obstacle = new Cube(c, r, -distances[i], SHAPE_X, SHAPE_Y, SHAPE_Z, Cube::ColourSet(color));
          obstacles.push_back(obstacle);
          last = i;
        }
      }
    }
  }
}

int Game::generateColour() {
  return rand() % 3 + 1;
  int newColour = rand() % 4 + 1;
  while (newColour == prevC1 || newColour == prevC2) {
    newColour = rand() % 3 + 1;
  }
  prevC1 = prevC2;
  prevC2 = newColour;
  return newColour;
}

void Game::updateScore() {
  vector<int> collidedObjects = checkCollisions();
  if (collidedObjects.size() > 0) {
    vector<int> collidedWrongColour;
    for (int i = 0; i < collidedObjects.size(); i++) {
      if (obstacles[collidedObjects[i]]->colour == currentColour) {
        comboLevel++;
      } else {
        comboLevel = 1;
        collidedWrongColour.push_back(collidedObjects[i]);
      }
      points += 100 * comboLevel;
    }
    if (collidedWrongColour.size() > 0) {
      if (collidedWrongColour.size() == 1) {
        currentColour = obstacles[collidedWrongColour[0]]->colour;
      } else {
        Cube::ColourSet firstColour = obstacles[collidedWrongColour[0]]->colour;
        bool sameColour = true;
        for (int i = 0; i < collidedWrongColour.size(); i++) {
          if (obstacles[collidedWrongColour[i]]->colour != firstColour) {
            sameColour = false;
            break;
          }
        }
        currentColour = obstacles[collidedWrongColour[0]]->colour;
      }
    }
  }
}

void Game::draw() {
  if (!finished) {
    canvas->draw(shiftZ, obstacles, progressPct, closeCubes, currentColour, comboLevel);
  } else {
    canvas->drawHighscore(points, highscores, highscoreAchieved);
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
      shiftZ = averageIntensity[0]*SHIFT_INTERVAL_PER_SECOND*(timer.get_ticks() - START_DELAY)/1000.f;
    } else {
      if (!musicStarted) {
        musicHandler->play();
        musicStarted = true;
        lastUpdate = timer.get_ticks();
      } else {

        // calculate score
        // TODO: calculate score according to time, and not the frequency that frames are drawn
        updateScore();

        // Update positions
        //shiftZ += SHIFT_INTERVAL_PER_SECOND * diff / 1000;
        //shiftZ += musicIntensityData[musicHandler->getPositionInSec()*43] * SHIFT_INTERVAL_PER_SECOND * diff / 1000;
        //shiftZ += averageIntensity[lastUpdate*43/1000] * SHIFT_INTERVAL_PER_SECOND * diff / 1000;
        shiftZ += averageIntensity[musicHandler->getPositionInSec()*43] * SHIFT_INTERVAL_PER_SECOND * diff / 1000;
        //shiftZ += distances[(lastUpdate+diff)*43.0/1000] - distances[lastUpdate*43.0/1000];
        lastUpdate += diff;

        // Update text
        canvas->setFPSText(frames * 1.0 / (timer.get_ticks()/1000));
        canvas->setPointsText(points);
        canvas->setComboLevelText(comboLevel);
      }
    }

    // Close cubes
    int i = prevObstacle;
    closeCubes.clear();
    while(abs(obstacles[i]->zFront) < shiftZ + OFFSET_FROM_CAMERA + 250.0) {
      closeCubes.push_back(i);
      i++;
    }

    progressPct = musicHandler->getPositionInSec() / musicHandler->getLengthInSec();
  }

  // Check for end of song
  if (musicHandler->getPositionInSec() == musicHandler->getLengthInSec()) {
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
      string song = selectMusicFileDialog();
      while (!strcmp(song.c_str(), "\0")) {
        song = selectMusicFileDialog();
      }
      reset(song);
    }
  }
}

void Game::handleEvent(SDL_Event& event) {
  switch (event.type) {

  case SDL_KEYDOWN:
    switch (event.key.keysym.sym) {
    case PAUSE_KEY:
      isPaused = !isPaused;
      if (isPaused) {
        pause();
      } else {
        resume();
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
        gProjectionScale = 0.25f;
      } else if( gProjectionScale == 0.25f ) {
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
      pause();
      string song = selectMusicFileDialog();
      if (strcmp(song.c_str(), "\0")) {
        reset(song);
      } else {
        // User pressed cancel, move along
        resume();
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

void Game::pause() {
  timer.pause();
  if (musicStarted) musicHandler->pause();
}

void Game::resume() {
  timer.unpause();
  if (musicStarted) musicHandler->play();
}

void Game::enableKinect() {
  kinectConnected = true;
}

vector<int> Game::checkCollisions() {
  vector<int> collidedCubes;
  float shrinkRatio = 240.f/SCREEN_HEIGHT;
  float playerZ = shiftZ + OFFSET_FROM_CAMERA;

  // note: use negative when dealing with cube coordinates

  // cube crossed intersection plane?
  if (-(obstacles[prevObstacle]->zFront) > playerZ) {
    return collidedCubes;
  }

  // have intersection? check for collisions
  // if cube back is less than shiftZ, it's behind the the collision plane
  // while loop is opposite of above if statement
  // basically, when the front of the cube has passed the collision plane (zFront < shiftZ) & zBack has not passed the collision plane (zBack > shiftZ), then check if the cube intersects with the player shadow

  int cubeIndex = prevObstacle;
  while(-obstacles[cubeIndex]->zFront < playerZ) {
    // cube past intersection plane?
    if (-obstacles[cubeIndex]->zBack < playerZ || obstacles[cubeIndex]->collided) {
      if (prevObstacle < obstacles.size() - 1)
        prevObstacle++;
    } else {
      Cube *cube = obstacles[cubeIndex];
      float x = cube->centre.x/SCALE;
      float y = cube->centre.y/SCALE;
      float x_scaled = x * shrinkRatio;
      float y_scaled = y * shrinkRatio;
      int pixelX = (int)x_scaled + 160;
      int pixelY = (int)y_scaled + 120;
      int cornerX = pixelX - cube->width/2/SCALE*shrinkRatio;
      int cornerY = pixelY - cube->height/2/SCALE*shrinkRatio;
      float cubeTexture[320*240];

      for (int i = 0; i < 320; i++) {
        for (int j = 0; j < 240; j++) {
          int index = j*320 + i;
          if (i >= cornerX && i <= cornerX + cube->width/SCALE*shrinkRatio) {
            if (j >= cornerY && j <= cornerY + cube->height/SCALE*shrinkRatio) {
              cubeTexture[index] = 1;
            } else {
              cubeTexture[index] = 0;
            }
          } else {
            cubeTexture[index] = 0;
          }
        }
      }

      float playerTexture[320*240];
      for (int i = 0; i < 320; i++) {
        for (int j = 0; j < 240; j++) {
          int index = j*320 + i;
          playerTexture[index] = canvas->depthData[index*4];
          if (playerTexture[index] + cubeTexture[index] > 255) {
            obstacles[cubeIndex]->collided = true;
            collidedCubes.push_back(cubeIndex);
            printf("collision with cube %d\n", cubeIndex);
            break;
          }
        }
        if (obstacles[cubeIndex]->collided)
          break;
      }

      // now this cube is sure to be crossing intersection; check with player shadow
      // if collide, add index, increment prevObstacle, mark as collided
    }
    if (cubeIndex < obstacles.size()- 1)
      cubeIndex++;
    else
      break;
  }

  return collidedCubes;
}
