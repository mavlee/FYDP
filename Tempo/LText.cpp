#include "LText.h"
#include <SDL_ttf.h>
#include <math.h>

bool Text::initText() {
	bool result = true;

	#ifndef TEXT
	FT_Error error = FT_Init_FreeType(&mLibrary);
	if (error) {
		result = false;
	}
	#endif

	return result;
}

Text::Text(int width, int height) {
	font = TTF_OpenFont("C:/FYDP/res/fonts/EunjinNakseo.ttf", 28);
	// TODO: i don't know how to declare color directly...
	SDL_Color mer = { 0, 169, 255, 0 };
	color = mer;
	surface = SDL_SetVideoMode( width, height, SCREEN_BPP, SDL_OPENGL );
}

Text::~Text() {
}

int round(double x) {
	return (int)(x + 0.5);
}

int nextpoweroftwo(int x) {
	double logbase2 = log((float) x) / log(2.f);
	return round(pow(2,ceil(logbase2)));
}

void Text::renderText(int width, int height, GLfloat x, GLfloat y, std::string text) {
	surface = SDL_DisplayFormatAlpha(TTF_RenderUTF8_Blended(font, text.c_str(), color));
	SDL_Rect area;
	area.x = 0;
	area.y = 0;
	area.w = nextpoweroftwo(surface->w);
	area.h = nextpoweroftwo(surface->h);

	SDL_Surface *temp = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA, area.w, area.h, SCREEN_BPP, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	SDL_BlitSurface(surface, &area, temp, NULL);

	GLint nColors = surface->format->BytesPerPixel;
	GLenum texture_format;
	if (nColors == 4) {
		if (surface->format->Rmask == 0x000000ff) {
			texture_format = GL_RGBA;
		} else {
			texture_format = GL_BGRA;
		}
	} else if (nColors == 3) {
		if (surface->format->Rmask == 0x000000ff) {
			texture_format = GL_RGB;
		} else {
			texture_format = GL_BGR;
		}
	} else {
		// this should never happen
		// TODO: throw an exception
	}

	// Generate a texture object handle
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, texture_format, area.w, area.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, temp->pixels);

	// Set the texture's stretching properties
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, texture);
	glColor3f( 0.0f, 0.6f, 1.0f);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, 0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x + area.w, y, 0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x + area.w, y + area.h, 0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + area.h, 0);
	glEnd();

	glFinish();

	glDisable(GL_TEXTURE_2D);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(temp);

	glDeleteTextures(1, &texture);

	//SDL_GL_Leave2DMode();
}

void SDL_GL_Enter2DMode(int width, int height) {
	glPushAttrib(GL_ENABLE_BIT);
	//glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glOrtho(0.0, (GLdouble) width, (GLdouble) height, 0.0, 0.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void SDL_GL_Leave2DMode() {
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glPopAttrib();
}