#ifndef GAME_H
#define GAME_H

#include "canvas.h"
#include "LOpenGL.h"
#include "LText.h"
#include <list>
#include "objects.h"

class Game {
  private:
    // board and camera stuff
    int canvasWidth;
    int canvasHeight;
    int gameWidth;
    int gameHeight;
    int gameDepth;
    Canvas *canvas;

    // random stuff for color and camera
    // and drawing in general
    GLfloat cameraX;
    GLfloat cameraY;
    GLfloat gProjectionScale;
    int gColorMode;
    float avgFps;
    Text *text;
    Text *pointsText;

    // player and obstacles
    int points;
    Cube* playerCube;
    std::list<Cube*> obstacles;

    // private functions
    void analyzeMusic();
    void generateGameFeatures();
    void drawObstacles();
    void checkForCollisions();

  public:
    Game(int width, int height);
    ~Game();

    void draw();
    void update(int nFrames = 0, float timeElapsed = 1.0f);
    void handleKeys(int key, int* movementKeyDown);
};

#endif
