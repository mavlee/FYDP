#include "LOpenGL.h"
#include <stdio.h>
#include "canvas.h"
#include "constants.h"
#include "image_loader.h"

char* skyboxPath = "res/images/skybox.png";
bool skyboxLoaded = false;
int skyboxWidth;
int skyboxHeight;
GLuint* skyboxTexture;
GLuint SKYBOX = 2;

Canvas::Canvas(int width, int height) {
    this->width = width;
    this->height = height;
    screen = NULL;
    initCanvas();
}

void Canvas::initCanvas() {
    // Start SDL
    if (SDL_Init( SDL_INIT_EVERYTHING ) == -1 ) {
        //return false;
    }

    // SDL_SWSURFACE implies that the surface is set up in software memory.
    screen = SDL_SetVideoMode(width, height, SCREEN_BPP, SDL_OPENGL);
    if (screen == NULL) {
        //return false;
    }

    //Initialize SDL ttf. Must be called prior to all SDL_ttf functions.
    if (TTF_Init() == -1) {
        //return false;
    }

    // Initialize Projection Matrix
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    glRotatef(-20.0f, 1, 0, 0);

    float offset = 0.0f;
    glFrustum( -width/2, width/2, height/2 + offset, -height/2 + offset, Z_NEAR, Z_FAR);

    
    //glRotatef(-40.0f, 1, 0, 0);

    // Initialize Modelview Matrix
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glPushMatrix();

    // Initialize clear color
    glClearColor( 0.f, 0.f, 0.f, 1.f );

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    //Check for error
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf( "Error initializing OpenGL! %s\n", gluErrorString( error ) );
    } else {
        printf( "Initialized OpenGL!\n");

        initImageLoader();

        //TODO: load skybox texture
        int width, height;
        bool hasAlpha;
        if (loadImage(skyboxPath, width, height, hasAlpha, skyboxTexture, &SKYBOX)) {
            skyboxLoaded = true;
            skyboxWidth = width;
            skyboxHeight = height;
        }
    }

    // Set the caption on the window.
    SDL_WM_SetCaption("Tempo", NULL);
}

void Canvas::cleanupCanvas() {
    clearImage();
    // Quit SDL. Also handles cleanup of the screen object.
    SDL_Quit();
}

void Canvas::draw() {
    // Clear color buffer & depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw the skybox before anything else is drawn.
    if (skyboxLoaded) {
        drawSkybox(skyboxWidth, skyboxHeight);
    }
}

void Canvas::drawSkybox(int width, int height) {
    /*	GLuint mTextureID;

    //Generate texture ID
    //    glGenTextures( 1, &mTextureID );

    //Bind texture ID
    glBindTexture( GL_TEXTURE_2D, mTextureID );

    //Generate texture
    //    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, skyboxTexture );

    //Set texture parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    //Unbind texture
    //    glBindTexture( GL_TEXTURE_2D, NULL );

    //Check for error
    GLenum error = glGetError();
    if( error != GL_NO_ERROR ) {
    printf( "Error drawing skybox from %p pixels! %s\n", skyboxTexture, gluErrorString( error ) );
    }
    */
    // Save current matrix.
    glPushMatrix();

    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_2D);

    //Set texture parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    double size;
    if (width >= height) {
        size = width;
    } else {
        size = height;
    }

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glRotatef(-90.0, 1.0, 0.0, 0.0);

    double e = 0.001;

    glBindTexture(GL_TEXTURE_2D, SKYBOX);
    glBegin(GL_QUADS);
    // Four sides
    glTexCoord2f(1.0/4.0 - e, 2.0/3.0 - e); glVertex3f(  0.0f,  size,  size/2.0 );
    glTexCoord2f(0.0/4.0 + e, 2.0/3.0 - e); glVertex3f(  0.0f,  0.0f,  size/2.0 );
    glTexCoord2f(0.0/4.0 + e, 1.0/3.0 + e); glVertex3f(  0.0f,  0.0f, -size/2.0 );
    glTexCoord2f(1.0/4.0 - e, 1.0/3.0 + e); glVertex3f(  0.0f,  size, -size/2.0 );
    glEnd();

    glPopAttrib();
    glPopMatrix();
}
