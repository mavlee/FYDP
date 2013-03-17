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
  fpsText = new Text(width, height);
  comboLevelText = new Text(width, height);
  pointsText = new Text(width, height);
  // TODO remove
  // also, this has nothing to do with the near plane
  playerCube = new Cube(0.f, 0.f, -OFFSET_FROM_CAMERA, SHAPE_X, SHAPE_Y, SHAPE_Z, Cube::Player);
  //playerCube = new Cube(0.f, 0.f, -(Z_NEAR + 200.f), 100.f, 100.f, 100.f, Cube::Multi);
  //playerCube = new Cube(0.f, 0.f, -(Z_NEAR + 200.f), 1800.f, 300.f, 100.f, Cube::Multi);

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

  string song = selectMusicFileDialog();
  while (!strcmp(song.c_str(), "/0")) {
    song = selectMusicFileDialog();
  }
  reset(song);
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
void Game::reset(string song) {
  if (canvas) delete canvas;
  canvas = new Canvas(canvasWidth, canvasHeight);
  points = 0;
  combo = 0;
  comboLevel = 1;
  lifeRemaining = TOTAL_LIFE_COUNT;
  progressPct = 0.0f;

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

  if (strcmp(song.c_str(), "/0")) {
    printf("New song must be chosen");
  }
  musicHandler->setMusicFile(song);
  musicData = musicHandler->getPeakData();
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
  int last = -50;
  Cube* obstacle;
  for (int i = 0; i < musicData[0].size(); i++) {
    for (int b = 0; b < NUM_BANDS; b++) {
      if (musicData[b][i] > PEAK_THRESHOLD && (i - last > 10 || i == last)) {
        float pos = -NUM_BANDS/2*125.f + b*125;
        //float y = (-b%2) * 125;
        //float x = -NUM_BANDS/4*125.f + b/2*125;
        float y = -b%4 * 125;
        float x = -NUM_BANDS/8*125.f + b/4*125;
        //obstacle = new Cube(pos, 0, -(Z_NEAR + 200.f + i*1.0*SHIFT_INTERVAL_PER_SECOND/musicHandler->getPeakDataPerSec()), 100.f, 100.f, 100.f, Cube::Multi);
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

    // Render the cube
    glPushMatrix();
    glTranslatef(cameraX, cameraY, 0);
    glTranslatef(0, 0, shiftZ);
    playerCube->draw();

    // Current method to indicate colour to hit. Integrate colour into something else later
    glBegin(GL_QUADS);
    glColor3fv(cubeColours[currentColour][0]); glVertex3f(  SCREEN_WIDTH/2       , -SCREEN_HEIGHT/2 , -1100.0f );
    glColor3fv(cubeColours[currentColour][0]); glVertex3f(  SCREEN_WIDTH/2 - 100 , -SCREEN_HEIGHT/2 , -1100.0f );
    glColor3fv(cubeColours[currentColour][0]); glVertex3f(  SCREEN_WIDTH/2 - 100 ,  SCREEN_HEIGHT/2 , -1100.0f );
    glColor3fv(cubeColours[currentColour][0]); glVertex3f(  SCREEN_WIDTH/2       ,  SCREEN_HEIGHT/2 , -1100.0f );
    glEnd();
    glPopMatrix();

    // Render text
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
        int xDir = 0;
        if (dirKeyPressed[LEFT]) xDir = -1;
        if (dirKeyPressed[RIGHT]) xDir = 1;
        cameraX += 1600.f * diff / 1000.f * xDir;

        shiftZ -= SHIFT_INTERVAL_PER_SECOND * diff / 1000;
        glTranslatef(0, 0, SHIFT_INTERVAL_PER_SECOND * diff / 1000);
        lastUpdate += diff;
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
