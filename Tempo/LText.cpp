#include "LText.h"
#include <math.h>
#include "constants.h"

Text::Text(int width, int height) {
	// TODO: just don't render text if the font can't be found
	font = TTF_OpenFont("C:/FYDP/Tempo/res/fonts/EunjinNakseo.ttf", 28);
	if (!font) {
		font = TTF_OpenFont("res/fonts/EunjinNakseo.ttf", 28);
	}
	// TODO: i don't know how to declare color directly...
	SDL_Color mer = { 255, 169, 255, 0 };
	color = mer;
	surface = SDL_SetVideoMode( width, height, SCREEN_BPP, SDL_OPENGL );
}

Text::~Text() {
	if (!font) {
		TTF_CloseFont(font);
		TTF_Quit();
	}
}

void Text::renderText(int width, int height, GLfloat x, GLfloat y, std::string text) {
	if (font == NULL) {
		return;
	}
	surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);

	GLint nColors = surface->format->BytesPerPixel;
	GLenum texture_format;
	if (nColors == 4) {
		if (surface->format->Rmask == 0x000000ff) {
			texture_format = GL_RGBA;
		} else {
			texture_format = GL_BGRA_EXT;
		}
	} else if (nColors == 3) {
		if (surface->format->Rmask == 0x000000ff) {
			texture_format = GL_RGB;
		} else {
			texture_format = GL_BGR_EXT;
		}
	} else {
		// this should never happen
		// TODO: throw an exception
	}

	glPushAttrib(GL_ENABLE_BIT);

	// Generate a texture object handle
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// Set the texture's stretching properties
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	glTexImage2D(GL_TEXTURE_2D, 0, surface->format->BytesPerPixel, surface->w, surface->h, 0, texture_format, GL_UNSIGNED_BYTE, surface->pixels);

	glBindTexture(GL_TEXTURE_2D, texture);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glOrtho( 0, width, height, 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f( 0.0f, 0.6f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, 0);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + surface->h, 0);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x + surface->w, y + surface->h, 0);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x + surface->w, y, 0);
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	SDL_FreeSurface(surface);

	glDeleteTextures(1, &texture);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}
