#ifndef GAME_H
#define GAME_H

#include "canvas.h"
#include "LOpenGL.h"
#include "LText.h"
#include <list>
#include "objects.h"
#include "music_handler.h"
#include "timer.h"

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
    vector<vector<float> > musicData;
    float shiftZ;
    Cube* playerCube;
    std::list<Cube*> obstacles;
    double lastPeakTime;

    // private functions
    void generateGameFeatures();
    void drawObstacles();
    bool checkForCollisions();
    void updateScore();
    void reset();

  public:
    Game(int width, int height, std::string musicFile);
    ~Game();

    void draw();
    void update();
    void handleKeys(int key, int* movementKeyDown);
};

#endif
