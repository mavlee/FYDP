#ifndef CANVAS_H
#define CANVAS_H
#include "LOpenGL.h"
#include <list>
#include "objects.h"

// class used for drawing shit
class Canvas {
private:
	int width;
	int height;
	SDL_Surface *screen;

	void drawSkybox(int width, int height, float shiftZ);
    void drawObstacles(std::list<Cube*> obstacles);

public:
	Canvas(int width, int height);

	void initCanvas(); // inits GL
	void cleanupCanvas(); // cleans up GL
	void draw(float shiftZ, std::list<Cube*> obstacles);
};

#endif
