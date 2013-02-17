#include "LText.h"
#include FT_BITMAP_H
#include <SDL_ttf.h>

FT_Library Text::mLibrary;

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