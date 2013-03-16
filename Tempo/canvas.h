#ifndef CANVAS_H
#define CANVAS_H
#include "LOpenGL.h"
#include <list>
#include "objects.h"
#include "constants.h"
#include "LText.h"

#ifdef USE_MAC_INCLUDES
typedef unsigned char BYTE;
#endif

// class used for drawing shit
class Canvas {
private:
  int width;
  int height;
  SDL_Surface *screen;
  GLuint playerDepthId;

  Text *scoreText;

  void drawSkybox(int width, int height, float shiftZ);
  void drawObstacles(std::list<Cube*> obstacles);
  void drawPlayer();

public:
  BYTE depthData[KINECT_DEPTH_WIDTH*KINECT_DEPTH_HEIGHT*4];
  Canvas(int width, int height);
  ~Canvas();

  void initCanvas(); // inits GL
  void cleanupCanvas(); // cleans up GL
  void draw(float shiftZ, std::list<Cube*> obstacles);

  void drawHighscore(int points, int* highscores, bool highscoreAchieved);
};

#endif
