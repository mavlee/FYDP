#ifndef USE_MAC_INCLUDES

#include <il/il.h>
#include <il/ilu.h>
#include "LOpenGL.h"

bool initImageLoader();
bool loadImage(char* imagePath, int &width, int &height, bool &hasAlpha, GLuint* texture, GLuint* textureId);
void clearImage();

#endif
