#ifndef CANVAS_H
#define CANVAS_H

#include "LOpenGL.h"

// class used for drawing shit
class Canvas {
	private:
    int width;
    int height;
    SDL_Surface *screen;

	public:
		Canvas(int width, int height);

    void initCanvas(); // inits GL
    void cleanupCanvas(); // cleans up GL
		void draw();
};

#endif
