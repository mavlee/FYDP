#ifndef USE_MAC_INCLUDES

#include "image_loader.h"
#include "il/il.h"
#include "il/ilu.h"
#include <stdio.h>
#include <string>

ILuint imgID = 0;

bool initImageLoader() {
	bool result = false;
	// initialize DevIL
	ilInit();
	// Set DevIL clear colour to transparent white
	ilClearColour(255, 255, 255, 000);
	ILenum ilError = ilGetError();
	if (ilError != IL_NO_ERROR) {
		printf("Error initializing DevIL with the error: %s\n", iluErrorString(ilError));
	} else {
		result = true;
	}
	return result;
}

bool loadImage(char* filename, int &width, int &height, bool &hasAlpha, GLubyte **outData) {
	bool result = false;

	ilGenImages(1, &imgID);
	ilBindImage(imgID);

	printf("%s\n", filename);
	ILboolean loadSuccess = ilLoadImage(filename);
	if (loadSuccess == IL_FALSE) {
		printf("Image loading failed completely\n");
		char* skyboxPathAlternate = "C:/FYDP/Tempo/res/images/skybox.png";
		loadSuccess = ilLoadImage("C:/FYDP/Tempo/res/images/skybox.png");
	} else {
		printf("Image loaded successfully from relative path\n");
	}

	if (loadSuccess == IL_TRUE) {
		printf("Image loaded successfully");
		if (ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE)) {
			glGenTextures(1, &imgID);
			glBindTexture(GL_TEXTURE_2D, imgID);
			//Generate texture
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, outData );

			//Set texture parameters
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

			glBindTexture(GL_TEXTURE_2D, NULL);
			GLenum error = glGetError();
			if (error != GL_NO_ERROR) {
				printf("Error loading texture from image: %s\n", gluErrorString(error));
			} else {
				result = true;
			}
			outData = (GLubyte**) ilGetData();
			width = ilGetInteger(IL_IMAGE_WIDTH);
			height = ilGetInteger(IL_IMAGE_HEIGHT);
			hasAlpha = false;
		}
	} else {
		ILenum ilError = ilGetError();
		printf("Error initializing DevIL with the error: %s\n", iluErrorString(ilError));
	}
	return result;
}

void clearImage() {
	glDeleteTextures(1, &imgID);
	ilDeleteImages(1, &imgID);
}

#endif
