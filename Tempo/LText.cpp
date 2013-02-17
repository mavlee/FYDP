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

bool Text::loadFreeType(std::string path, GLuint pixelSize) {
	bool result = true;
	FT_Error error = NULL;

	#ifndef TEXT
		error = FT_Init_FreeType(&mLibrary);
		if (error) {
			result = false;
		}
	#endif

	GLuint cellW = 0;
	GLuint cellH = 0;
	int maxBearing = 0;
	int minHang = 0;

	// Contains info about the metrics of a glyph (character image)
	FT_Glyph_Metrics metrics[256];

	return result;
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