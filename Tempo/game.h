#include "canvas.h"

class Game {
  private:
    int canvasWidth;
    int canvasHeight;
    int gameWidth;
    int gameHeight;
    int gameDepth;
    Canvas canvas;

  public:
    Game();

    void draw();
    void handleKeys(int key);
};
