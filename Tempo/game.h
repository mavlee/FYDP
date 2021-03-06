#ifndef GAME_H
#define GAME_H

#include "canvas.h"
#include "LOpenGL.h"
#include "LText.h"
#include <vector>
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
  float avgFps;

  // For handling music
  MusicHandler* musicHandler;

  // Game Timer
  Timer timer;
  int lastUpdate;
  int frames;

  // data about the game
  int points;
  int comboLevel;
  float progressPct;
  Cube::ColourSet currentColour;
  bool kinectConnected;

  bool musicStarted;
  int isPaused;

  // after game variables
  bool restart;
  bool finished;
  bool highscoreAchieved;
  int* highscores;
  bool quitGame;

  vector<vector<float> > musicData;
  vector<float> musicIntensityData;
  vector<float> averageIntensity;
  vector<float> distances;
  float shiftZ;
  Cube* playerCube;

  vector<Cube*> obstacles;
  vector<int> closeCubes;

  int prevObstacle;

  // Used for obstacle generation
  int generateColour();
  int prevC1;
  int prevC2;

  // private functions
  void reset(string song);

  void handleEvent(SDL_Event& event);
  void draw();
  void update();

  void generateGameFeatures();
  void updateScore();

  void pause();
  void resume();

  vector<int> checkCollisions();

public:
  BYTE depthData[KINECT_DEPTH_WIDTH*KINECT_DEPTH_HEIGHT*4];
  Canvas *canvas;

  Game(int width, int height);
  ~Game();

  int execute();
  void enableKinect();
};

#endif
