#include "LText.h"
#include <SDL_ttf.h>

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
	SDL_Color mer = { 0, 169, 255 };
	color = mer;
	surface = SDL_SetVideoMode( width, height, SCREEN_BPP, SDL_OPENGL );
}

Text::~Text() {
}

void Text::renderText(int width, int height, GLfloat x, GLfloat y, std::string text) {
	surface = SDL_DisplayFormatAlpha(TTF_RenderText_Solid(font, text.c_str(), color));
	SDL_Rect area;
	area.x = 0;
	area.y = 0;
	area.w = surface->w;
	area.h = surface->h;

	SDL_Surface *temp = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA, surface->w, surface->h, SCREEN_BPP, 0x00000ff, 0x0000ff00, 0x00ff0000, 0x000000ff);
	SDL_BlitSurface(surface, &area, temp, NULL);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, temp->pixels);
	/*SDL_GL_Enter2DMode(width, height);
	// Generate a texture object handle
	glGenTextures(1, &texture);

	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, texture);*/

	// Set the texture's stretching properties
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	glBegin(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		glTexCoord2d(0, 0); glVertex3f(0, 0, 0);
		glTexCoord2d(1, 0); glVertex3f(0 + surface->w, 0, 0);
		glTexCoord2d(1, 1); glVertex3f(0 + surface->w, 0 + surface->h, 0);
		glTexCoord2d(0, 1); glVertex3f(0, 0 + surface->h, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(temp);
	
	// Edit the texture object's image data using the information SDL_Surface gives us
	/*glTexImage2D( GL_TEXTURE_2D, 0, surface->format->BytesPerPixel, surface->w, surface->h, 0,
                      GL_RGB, GL_UNSIGNED_BYTE, surface->pixels );
*/
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