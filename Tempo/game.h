#ifndef GAME_H
#define GAME_H

#include "canvas.h"
#include "LOpenGL.h"
#include <list>
#include "objects.h"

class Game {
  private:
    int canvasWidth;
    int canvasHeight;
    int gameWidth;
    int gameHeight;
    int gameDepth;
    Canvas *canvas;

    // random stuff for color and camera
    GLfloat cameraX;
    GLfloat cameraY;
    GLfloat gProjectionScale;
    int gColorMode;
    float avgFps;

    Cube* playerCube;

    std::list<Cube*> obstacles;
    void drawObstacles();

  public:
    Game(int width, int height);
    ~Game();

    void analyzeMusic();
    void generateGameFeatures();

    void draw();
    void update(int nFrames = 0, float timeElapsed = 1.0f);
	//void update();
    void handleKeys(int key);
};

#endif
