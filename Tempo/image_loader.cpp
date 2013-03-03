#include "image_loader.h"
#include "constants.h"

bool initImageLoader() {
    //Initialize DevIL
    ilInit();
    ilClearColour( 255, 255, 255, 000 );

    //Check for error
    ILenum ilError = ilGetError();
    if(ilError != IL_NO_ERROR) {
        printf( "Error initializing DevIL! %s\n", iluErrorString( ilError ) );
        return false;
    }
}

bool loadImage(char* imagePath, int &width, int &height, bool &hasAlpha, GLuint* texture, GLuint* textureId) {
    bool result = false;

    //Generate and set current image ID
    ILuint imgID = 0;
    ilGenImages(1, &imgID);
    ilBindImage(imgID);

    //Load image
    ILboolean success = ilLoadImage(imagePath);
    if (success == IL_FALSE) {
        printf("Unable to load %s\n", imagePath);
        success = ilLoadImage("C:/FYDP/Tempo/res/images/skybox.png");
    }

    //Image loaded successfully
    if(success == IL_TRUE)
    {
        //Convert image to RGBA
        success = ilConvertImage( IL_RGBA, IL_UNSIGNED_BYTE );
        if( success == IL_TRUE )
        {
            //Generate texture ID
            glGenTextures( 1, textureId );

            //Bind texture ID
            glBindTexture( GL_TEXTURE_2D, *textureId );

            width = (GLuint)ilGetInteger(IL_IMAGE_WIDTH);
            height = (GLuint)ilGetInteger(IL_IMAGE_HEIGHT);
            hasAlpha = true;
            texture = (GLuint*) ilGetData();

            //Generate texture
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture );

            //Set texture parameters
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

            //Unbind texture
            glBindTexture( GL_TEXTURE_2D, NULL );

            //Check for error
            GLenum error = glGetError();
            if( error != GL_NO_ERROR ) {
                printf( "Error loading texture from %p pixels! %s\n", texture, gluErrorString(error) );
            }
            result = true;
        }

        //Delete file from memory
        ilDeleteImages( 1, &imgID );
    }

    //Report error
    if(!result) {
        printf( "Completely failed to load %s\n", imagePath );
    }

    return result;
}

void clearImage() {
}