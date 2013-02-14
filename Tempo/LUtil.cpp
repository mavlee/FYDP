/*This source code copyrighted by Lazy Foo' Productions (2004-2013)
and may not be redistributed without written permission.*/
//Version: 001

#include "LUtil.h"

int gColorMode = COLOR_MODE_CYAN;

GLfloat gProjectionScale = 1.f;
GLfloat cameraX = 0.f;
GLfloat cameraY = 0.f;
float angle = 0.f;

bool initGL()
{
    //Initialize Projection Matrix
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
	glOrtho( 0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, 100.0, -100.0);

    //Initialize Modelview Matrix
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

	glPushMatrix();

    //Initialize clear color
    glClearColor( 0.f, 0.f, 0.f, 1.f );

	//glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);

    //Check for error
    GLenum error = glGetError();
    if( error != GL_NO_ERROR )
    {
        printf( "Error initializing OpenGL! %s\n", gluErrorString( error ) );
        return false;
    }

    return true;
}

void update()
{
	angle += 0.05f;
	if (angle > 360) {
	//	angle -= 360.f;
	}
}

void render()
{
    //Clear color buffer
    glClear( GL_COLOR_BUFFER_BIT );

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPushMatrix();

	glTranslatef(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f, 0.f);
	glRotatef(angle, 0.f, 1.f, 1.f);

    //Render quad
    glBegin( GL_QUADS );
		// back face
		if (gColorMode == COLOR_MODE_MULTI) {
			glColor3f( 1.f, 0.f, 0.f);
		} else {
			glColor3f( 0.f, 1.f, 1.f);
		}
		glVertex3f( -50.f, -50.f, -50.f);
		glVertex3f(  50.f, -50.f, -50.f);
	    glVertex3f(  50.f,  50.f, -50.f);
		glVertex3f( -50.f,  50.f, -50.f);
		// front face
		if (gColorMode == COLOR_MODE_MULTI) {
			glColor3f( 0.f, 1.f, 0.f);
		}
		glVertex3f( -50.f, -50.f,  50.f);
		glVertex3f(  50.f, -50.f,  50.f);
		glVertex3f(  50.f,  50.f,  50.f);
		glVertex3f( -50.f,  50.f,  50.f);
		// left face
		if (gColorMode == COLOR_MODE_MULTI) {
			glColor3f( 0.f, 0.f, 1.f);
		}
		glVertex3f( -50.f, -50.f, -50.f);
		glVertex3f( -50.f, -50.f,  50.f);
	    glVertex3f( -50.f,  50.f, -50.f);
		glVertex3f( -50.f,  50.f,  50.f);
		// right face
		if (gColorMode == COLOR_MODE_MULTI) {
			glColor3f( 1.f, 1.f, 0.f);
		}
		glVertex3f(  50.f, -50.f, -50.f);
		glVertex3f(  50.f, -50.f,  50.f);
	    glVertex3f(  50.f,  50.f, -50.f);
		glVertex3f(  50.f,  50.f,  50.f);
		// top face
		if (gColorMode == COLOR_MODE_MULTI) {
			glColor3f( 0.f, 1.f, 1.f);
		}
		glVertex3f( -50.f,  50.f, -50.f);
		glVertex3f( -50.f,  50.f,  50.f);
	    glVertex3f(  50.f,  50.f, -50.f);
		glVertex3f(  50.f,  50.f,  50.f);
		// bottom face
		if (gColorMode == COLOR_MODE_MULTI) {
			glColor3f( 1.f, 0.f, 1.f);
		}
		glVertex3f( -50.f, -50.f, -50.f);
		glVertex3f( -50.f, -50.f,  50.f);
	    glVertex3f(  50.f, -50.f, -50.f);
		glVertex3f(  50.f, -50.f,  50.f);
    glEnd();

    //Update screen
    SDL_GL_SwapBuffers();
}

void handleKeys(int key) {
	bool translation = false;
    switch(key) {
		case SDLK_q:
			//Toggle color mode
			if(gColorMode == COLOR_MODE_CYAN) {
				gColorMode = COLOR_MODE_MULTI;
			} else {
				gColorMode = COLOR_MODE_CYAN;
			}
			break;
		case SDLK_w:
			translation = true;
			cameraY -= 16.f;
			break;
		case SDLK_s:
			translation = true;
			cameraY += 16.f;
			break;
		case SDLK_a:
			translation = true;
			cameraX -= 16.f;
			break;
		case SDLK_d:
			translation = true;
			cameraX += 16.f;
			break;
		case SDLK_e:
			//Cycle through projection scales
			if( gProjectionScale == 1.f )
			{
				//Zoom out
				gProjectionScale = 2.f;
			}
			else if( gProjectionScale == 2.f )
			{
				//Zoom in
				gProjectionScale = 0.5f;
			}
			else if( gProjectionScale == 0.5f )
			{
				//Regular zoom
				gProjectionScale = 1.f;
			}

			//Update projection matrix
			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			glOrtho( 0.0, SCREEN_WIDTH * gProjectionScale, SCREEN_HEIGHT * gProjectionScale, 0.0, 1.0, -1.0 );
			glTranslatef((gProjectionScale - 1.f) * SCREEN_WIDTH / 2.f, 
						 (gProjectionScale - 1.f) * SCREEN_HEIGHT / 2.f, 0.f);
			break;
		default:
			break;
    }
	if (translation) {
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glLoadIdentity();

		glTranslatef(cameraX, cameraY, 0.f);

		glPushMatrix();
	}
}