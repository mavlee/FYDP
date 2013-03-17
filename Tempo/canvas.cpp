#include "LOpenGL.h"
#include <stdio.h>
#include "canvas.h"
#include "constants.h"
#include "image_loader.h"
#include "kinect_controller.h"

char* skyboxPath = "res/images/skybox.png";
bool skyboxLoaded = false;
int skyboxWidth;
int skyboxHeight;
GLuint* skyboxTexture;
GLuint SKYBOX = 2;

char* rectPath = "res/images/rect.png";
bool rectLoaded = false;
int rectWidth;
int rectHeight;
GLuint* rectTexture;
GLuint RECTANGLE = 3;

char* lifePath = "res/images/heart.png";
bool lifeLoaded = false;
int lifeWidth;
int lifeHeight;
GLuint* lifeTexture;
GLuint LIFE = 4;

char* pbPath = "res/images/progress_bar.png";
bool pbLoaded = false;
int pbWidth;
int pbHeight;
GLuint* pbTexture;
GLuint PB = 5;

char* proPath = "res/images/progress_indicator.png";
bool proLoaded = false;
int proWidth;
int proHeight;
GLuint* proTexture;
GLuint PRO = 6;

Canvas::Canvas(int width, int height) {
  this->width = width;
  this->height = height;
  screen = NULL;
  initCanvas();
  scoreText = new Text(width, height);
  lifeText = new Text(width, height);
  fpsText = new Text(width, height);
  comboLevelText = new Text(width, height);
  pointsText = new Text(width, height);
  // Init text
  setFPSText(0);
  setPointsText(0);
  setComboLevelText(0);
}

Canvas::~Canvas() {
  cleanupCanvas();
  if (skyboxTexture) delete skyboxTexture;
  if (rectTexture) delete rectTexture;
  if (scoreText) delete scoreText;
  if (lifeText) delete lifeText;
  if (fpsText) delete fpsText;
  if (comboLevelText) delete comboLevelText;
  if (pointsText) delete pointsText;
}

void Canvas::initCanvas() {
  // Initialize player texture
  for (int i = 0; i < KINECT_DEPTH_HEIGHT*KINECT_DEPTH_WIDTH*4; i++)
    depthData[i] = i % 4 == 3? (BYTE)100: (BYTE)255;

  // Start SDL
  SDL_Init(SDL_INIT_EVERYTHING);
  // SDL_SWSURFACE implies that the surface is set up in software memory.
  screen = SDL_SetVideoMode(width, height, SCREEN_BPP, SDL_OPENGL);
  //Initialize SDL ttf. Must be called prior to all SDL_ttf functions.
  TTF_Init();

  // Initialize Projection Matrix
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glFrustum( -width/2, width/2, height/2, -height/2, Z_NEAR, Z_FAR);

  // Initialize Modelview Matrix
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  // Initialize clear color
  glClearColor( 0.1f, 0.1f, 0.1f, 1.f );

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
    if (loadImage(rectPath, width, height, hasAlpha, rectTexture, &RECTANGLE)) {
      rectLoaded = true;
      rectWidth = width;
      rectHeight = height;
    }
    if (loadImage(lifePath, width, height, hasAlpha, rectTexture, &LIFE)) {
      lifeLoaded = true;
      lifeWidth = width;
      lifeHeight = height;
    }
    if (loadImage(pbPath, width, height, hasAlpha, rectTexture, &PB)) {
      pbLoaded = true;
      pbWidth = width;
      pbHeight = height;
    }
    if (loadImage(proPath, width, height, hasAlpha, rectTexture, &PRO)) {
      proLoaded = true;
      proWidth = width;
      proHeight = height;
    }
  }

  // Load player silhouette
  glGenTextures(1, &playerDepthId);
  glBindTexture(GL_TEXTURE_2D, playerDepthId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (GLvoid*) depthData);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Set the caption on the window.
  SDL_WM_SetCaption("Tempo", NULL);
}

void Canvas::cleanupCanvas() {
  clearImage(&SKYBOX);
  clearImage(&RECTANGLE);
  clearImage(&LIFE);
  clearImage(&PB);
  clearImage(&PRO);
  // Quit SDL. Also handles cleanup of the screen object.
  SDL_Quit();
}

void Canvas::draw(float shiftZ, std::list<Cube*> obstacles, int lifeRemaining, float progressPct, Cube::ColourSet currentColour) {
  // Reset and clear
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //glEnable(GL_CULL_FACE); // commenting this out makes cubes more identifible due to random bug
  glEnable(GL_DEPTH_TEST);

  // Draw the skybox before anything else is drawn.
#ifndef USE_MAC_INCLUDES
  if (skyboxLoaded) {
    //drawSkybox(skyboxWidth, skyboxHeight, shiftZ);
  }
#endif

  /** Draw 3D stuff **/
  glPushMatrix();
  glTranslatef(0, 0, shiftZ);
  drawObstacles(obstacles);
  glPopMatrix();

  /** Draw 2D overlays **/
#ifndef USE_MAC_INCLUDES
  glPushMatrix();
  drawPlayer(lifeRemaining);
  glPopMatrix();
#endif

  glPushMatrix();
  //drawLife(lifeRemaining);
  glPopMatrix();

  glPushMatrix();
  drawProgress(progressPct);
  glPopMatrix();

  // Draw ui text
  fpsText->renderText(width, height, 10, 10, fpsString);
  comboLevelText->renderText(width, height, 10, height - 100, comboLevelString);
  pointsText->renderText(width, height, 10, height - 50, pointsString);

  // Update screen
  SDL_GL_SwapBuffers();
}

void Canvas::drawPlayer(int lifeRemaining) {
  // Initialize Projection Matrix
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, width, height, 0, 0, 1);

  // Set attributes for texture drawing
  glMatrixMode(GL_MODELVIEW);
  glPushAttrib(GL_ENABLE_BIT);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  // Set texture settings
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Set colour according to HP
  const float MIN_COLOUR = 0.4;
  float colour = (1.f - MIN_COLOUR) * 1.f*lifeRemaining/TOTAL_LIFE_COUNT;
  if (lifeRemaining >= 1) {
    glColor4f(colour, 0, 0, 0.7);
  } else {
    glColor4f(colour, colour, colour, 0.7);
  }

  float kinectAspect = 1.f*KINECT_DEPTH_HEIGHT/KINECT_DEPTH_HEIGHT;
  int drawHeight = height;
  int drawWidth = kinectAspect * drawHeight;
  int xOffset = (width - drawWidth)/2;

  glBindTexture(GL_TEXTURE_2D, playerDepthId);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (GLvoid*)depthData);
  glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(xOffset + drawWidth, height, 0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(xOffset, height, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(xOffset, 0, 0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(xOffset + drawWidth, 0, 0);
  glEnd();

  // Reset attributes, projection matrix
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}

void Canvas::drawSkybox(int width, int height, float shiftZ) {

  // Initialize Projection Matrix
  glMatrixMode( GL_PROJECTION );
  // Save current matrix.
  glPushMatrix();
  glLoadIdentity();

  glFrustum( -1, 1, -1, 1, 1, Z_FAR);

  glMatrixMode(GL_MODELVIEW);

  glPushAttrib(GL_ENABLE_BIT);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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

  double e = 0.001;

  glBindTexture(GL_TEXTURE_2D, SKYBOX);
  glBegin(GL_QUADS);
  // Four sides
  // Front
  glTexCoord2f( 1.0/4.0, 2.0/3.0 ); glVertex3f(          -size/2,          -size/2,  -size );
  glTexCoord2f( 2.0/4.0, 2.0/3.0 ); glVertex3f(           size/2,          -size/2,  -size );
  glTexCoord2f( 2.0/4.0, 1.0/3.0 ); glVertex3f(           size/2,           size/2,  -size );
  glTexCoord2f( 1.0/4.0, 1.0/3.0 ); glVertex3f(          -size/2,           size/2,  -size );

  // Left
  glTexCoord2f( 0.0/4.0, 2.0/3.0 ); glVertex3f(  -SCREEN_WIDTH/2,          -size/2,    0.0 );
  glTexCoord2f( 1.0/4.0, 2.0/3.0 ); glVertex3f(  -SCREEN_WIDTH/2,          -size/2,  -size );
  glTexCoord2f( 1.0/4.0, 1.0/3.0 ); glVertex3f(  -SCREEN_WIDTH/2,           size/2,  -size );
  glTexCoord2f( 0.0/4.0, 1.0/3.0 ); glVertex3f(  -SCREEN_WIDTH/2,           size/2,    0.0 );

  // Right
  glTexCoord2f( 2.0/4.0, 2.0/3.0 ); glVertex3f(   SCREEN_WIDTH/2,          -size/2,  -size );
  glTexCoord2f( 3.0/4.0, 2.0/3.0 ); glVertex3f(   SCREEN_WIDTH/2,          -size/2,    0.0 );
  glTexCoord2f( 3.0/4.0, 1.0/3.0 ); glVertex3f(   SCREEN_WIDTH/2,           size/2,    0.0 );
  glTexCoord2f( 2.0/4.0, 1.0/3.0 ); glVertex3f(   SCREEN_WIDTH/2,           size/2,  -size );

  // Bottom
  glTexCoord2f( 1.0/4.0, 3.0/3.0 ); glVertex3f(          -size/2, -SCREEN_HEIGHT/2,    0.0 );
  glTexCoord2f( 2.0/4.0, 3.0/3.0 ); glVertex3f(           size/2, -SCREEN_HEIGHT/2,    0.0 );
  glTexCoord2f( 2.0/4.0, 2.0/3.0 ); glVertex3f(           size/2, -SCREEN_HEIGHT/2,  -size );
  glTexCoord2f( 1.0/4.0, 2.0/3.0 ); glVertex3f(          -size/2, -SCREEN_HEIGHT/2,  -size );

  // Top
  glTexCoord2f( 1.0/4.0, 1.0/3.0 ); glVertex3f(          -size/2,  SCREEN_HEIGHT/2,  -size );
  glTexCoord2f( 2.0/4.0, 1.0/3.0 ); glVertex3f(           size/2,  SCREEN_HEIGHT/2,  -size );
  glTexCoord2f( 2.0/4.0, 0.0/3.0 ); glVertex3f(           size/2,  SCREEN_HEIGHT/2,    0.0 );
  glTexCoord2f( 1.0/4.0, 0.0/3.0 ); glVertex3f(          -size/2,  SCREEN_HEIGHT/2,    0.0 );

  // We never rotate the screen. No need to draw the back surface.

  glEnd();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}

void Canvas::drawObstacles(std::list<Cube*> obstacles) {
  for (std::list<Cube*>::iterator i = obstacles.begin(); i != obstacles.end(); i++) {
    if (rectLoaded) {
      //(*i)->draw(&RECTANGLE);
      (*i)->draw();
    } else {
      (*i)->draw();
    }
  }
}

void Canvas::drawLife(int lifeRemaining) {
  const float padding = 20.0f;
  const float x = 0.0f;
  const float y = 0.0f;
  const double size = 50;

  // TODO remove
  //std::stringstream life;
  //life << "Life Remaining: ";
  //lifeText->renderText(width, height, x, y + padding, life.str());

  // Initialize Projection Matrix
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, width, height, 0, 0, 1);

  // Set attributes for ortho texture drawing
  glMatrixMode(GL_MODELVIEW);
  glPushAttrib(GL_ENABLE_BIT);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glEnable(GL_BLEND);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  //Set texture stretching parameters
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

  glBindTexture(GL_TEXTURE_2D, LIFE);
  glBegin(GL_QUADS);
  for (int i = 0; i < lifeRemaining; i++) {
    glTexCoord2f( 1.0f, 0.0f ); glVertex3f( x + 235 + size * (i+1) + padding * i , y + padding + 0        , 0.f );
    glTexCoord2f( 0.0f, 0.0f ); glVertex3f( x + 235 + size * (i  ) + padding * i , y + padding + 0        , 0.f );
    glTexCoord2f( 0.0f, 1.0f ); glVertex3f( x + 235 + size * (i  ) + padding * i , y + padding + 0 + size , 0.f );
    glTexCoord2f( 1.0f, 1.0f ); glVertex3f( x + 235 + size * (i+1) + padding * i , y + padding + 0 + size , 0.f );
  }
  glEnd();

  // Reset attributes, projection matrix
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glDisable(GL_BLEND);

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}

void Canvas::drawProgress(float progressPct) {
  float padding = 20.0f;
  float x = 500.0f;
  float y = height - 50.0f;
  float border = 4.0f;

  // Initialize Projection Matrix
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, width, height, 0, 0, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glEnable(GL_BLEND);

  glColor4f(0.3f, 0.3f, 0.3f, 0.9f);
  // top bar
  glBegin(GL_QUADS);
    glVertex3f(width/4.f - border, y + padding, 0);
    glVertex3f(3.f*width/4.f + border, y + padding, 0);
    glVertex3f(3.f*width/4.f + border, y + padding + border, 0);
    glVertex3f(width/4.f - border, y + padding + border, 0);
  glEnd();

  // bottom bar
  glBegin(GL_QUADS);
    glVertex3f(width/4.f - border, y - border, 0);
    glVertex3f(3.f*width/4.f + border, y - border, 0);
    glVertex3f(3.f*width/4.f + border, y, 0);
    glVertex3f(width/4.f - border, y, 0);
  glEnd();

  // left bar
  glBegin(GL_QUADS);
    glVertex3f(width/4.f - border, y, 0);
    glVertex3f(width/4.f, y, 0);
    glVertex3f(width/4.f, y + padding, 0);
    glVertex3f(width/4.f - border, y + padding, 0);
  glEnd();

  // right bar
  glBegin(GL_QUADS);
    glVertex3f(3.f*width/4.f, y, 0);
    glVertex3f(3.f*width/4.f + border, y, 0);
    glVertex3f(3.f*width/4.f + border, y + padding, 0);
    glVertex3f(3.f*width/4.f, y + padding, 0);
  glEnd();

  // progress bar
  glColor4f(0.4f, 0.4f, 0.4f, 0.9f);
  glBegin(GL_QUADS);
    glVertex3d(width/4.f, y, 0);
    glVertex3d(width/4.f + width/2.f*progressPct, y, 0);
    glVertex3d(width/4.f + width/2.f*progressPct, y + padding, 0);
    glVertex3d(width/4.f, y + padding, 0);
  glEnd();

  // Reset attributes, projection matrix
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glDisable(GL_BLEND);

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}


void Canvas::drawHighscore(int points, int* highscores, bool highscoreAchieved, int lifeRemaining) {
	glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  std::stringstream highscore_caption;
  if (lifeRemaining == 0) {
    std::stringstream game_over;
    game_over << "Game Over";
    scoreText->renderText(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH / 2, 100, game_over.str());
  }
  if (highscoreAchieved) {
    highscore_caption << "New Highscore! Score: " << points;
    scoreText->renderText(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH / 2, 150, highscore_caption.str());
  } else {
    highscore_caption << "Score: " << points;
    scoreText->renderText(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH / 2, 150, highscore_caption.str());
  }
  int i = 0;
  for (i; i < 10; i++) {
    std::stringstream highscore_line;
    highscore_line <<  "Record " << i + 1 << ":       " << highscores[i];
    scoreText->renderText(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH / 2, 150 + (i + 1) * 50, highscore_line.str());
  }
  std::stringstream restart_line;
  restart_line <<  "Press \'r\' to play again.";
  scoreText->renderText(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH / 2, 150 + (11) * 50, restart_line.str());

  // Update screen
  SDL_GL_SwapBuffers();
}


void Canvas::setFPSText(float fps) {
  std::stringstream fpsCaption;
  fpsCaption << fps;
  fpsString = fpsCaption.str();
}

void Canvas::setPointsText(int points) {
  std::stringstream pointsCaption;
  pointsCaption << "Points: " << points;
  pointsString = pointsCaption.str();
}

void Canvas::setComboLevelText(int comboLevel) {
  std::stringstream comboCaption;
  comboCaption << "Combo Level: " << comboLevel;
  comboLevelString = comboCaption.str();
}
