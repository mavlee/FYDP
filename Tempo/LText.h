#ifndef LTEXT_H
#define LTEXT_H

#include "LOpenGL.h"
#include "surfaceHelper.h"

class Text {
	public:
		// Constructor
		Text(int width, int hewight);

		// Deconstructor
		~Text();

		void renderText(int width, int height, GLfloat x, GLfloat y, std::string text);

		SDL_Color color;

	private:
		TTF_Font *font;
		SDL_Surface *surface;
		GLuint texture;
};

#endif
