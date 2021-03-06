#ifndef CANVAS_H
#define CANVAS_H
#include "LOpenGL.h"
#include <vector>
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
  Text *lifeText;
  Text *fpsText;
  Text *comboLevelText;
  Text *pointsText;
  std::string fpsString;
  std::string comboLevelString;
  std::string pointsString;
  int comboLevel;

  void drawSkybox(int width, int height, float shiftZ);
  void drawObstacles(std::vector<Cube*>& obstacles);
  void drawPlayer(Cube::ColourSet, int comboLevel);
  void drawProgress(float progressPct);
  void drawGrid(std::vector<int>& closeCubes, std::vector<Cube*>& obstacles);
  void drawCombo(Cube::ColourSet clr);

public:
  BYTE depthData[KINECT_DEPTH_WIDTH*KINECT_DEPTH_HEIGHT*4];
  Canvas(int width, int height);
  ~Canvas();

  void initCanvas(); // inits GL
  void cleanupCanvas(); // cleans up GL

  void draw(float shiftZ, std::vector<Cube*>& obstacles, float progressPct, std::vector<int>& closeCubes, Cube::ColourSet currentColour, int comboLevel);
  void drawHighscore(int points, int* highscores, bool highscoreAchieved);

  void setFPSText(float fps);
  void setComboLevelText(int comboLevel);
  void setPointsText(int points);
};

#endif
