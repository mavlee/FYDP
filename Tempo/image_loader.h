#ifndef USE_MAC_INCLUDES

#include "LOpenGL.h"

bool initImageLoader();
bool loadImage(char* filename, int &width, int &height, bool &hasAlpha, GLubyte **outData);
void clearImage();

#endif
