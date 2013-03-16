#ifndef GAME_H
#define GAME_H

#include "canvas.h"
#include "LOpenGL.h"
#include "LText.h"
#include <list>
#include "objects.h"
#include "music_handler.h"
#include "timer.h"
#include "constants.h"

class Game {
private:
  // board and camera stuff
  int canvasWidth;
  int canvasHeight;
  int gameWidth;
  int gameHeight;
  int gameDepth;


  // random stuff for color and camera
  // and drawing in general
  GLfloat cameraX;
  GLfloat cameraY;
  GLfloat gProjectionScale;
  int gColorMode;
  float avgFps;
  Text *fpsText;
  Text *comboLevelText;
  Text *pointsText;

  // For handling music
  MusicHandler* musicHandler;

  // Game Timer
  Timer timer;
  int lastUpdate;
  int frames;

  // data about the game
  int points;
  int comboLevel;
  int combo;
  int lifeRemaining;

  int isPaused;
  bool dirKeyPressed[2];

  // after game variables
  bool restart;
  bool finished;
  bool highscoreAchieved;
  int* highscores;

  vector<vector<float> > musicData;
  float shiftZ;
  Cube* playerCube;
  std::list<Cube*> obstacles;

  // private functions
  void reset();

  void handleEvent(SDL_Event& event);
  void draw();
  void update();

  void generateGameFeatures();
  bool checkForCollisions();
  void updateScore();

public:
  BYTE depthData[KINECT_DEPTH_WIDTH*KINECT_DEPTH_HEIGHT*4];
  Canvas *canvas;

  Game(int width, int height);
  ~Game();

  int execute();
};

#endif
