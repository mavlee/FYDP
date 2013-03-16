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
}

Canvas::~Canvas() {
  delete skyboxTexture;
  delete rectTexture;
  if (scoreText) delete scoreText;
  if (lifeText) delete lifeText;
}

void Canvas::initCanvas() {
  // Initialize player texture
  for (int i = 0; i < KINECT_DEPTH_HEIGHT*KINECT_DEPTH_WIDTH*4; i++)
    depthData[i] = 0;

  // Start SDL
  SDL_Init(SDL_INIT_EVERYTHING);
  // SDL_SWSURFACE implies that the surface is set up in software memory.
  screen = SDL_SetVideoMode(width, height, SCREEN_BPP, SDL_OPENGL);
  //Initialize SDL ttf. Must be called prior to all SDL_ttf functions.
  TTF_Init();

  // Initialize Projection Matrix
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  glRotatef(SCREEN_TILT, 1, 0, 0);

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

void Canvas::draw(float shiftZ, std::list<Cube*> obstacles, int lifeRemaining, float progressPct) {
  // Clear color buffer & depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw the skybox before anything else is drawn.
  if (skyboxLoaded) {
    drawSkybox(skyboxWidth, skyboxHeight, shiftZ);
  }

  glPushMatrix();
  drawObstacles(obstacles);
  glPopMatrix();

  glPushMatrix();
  drawPlayer();
  glPopMatrix();

  glPushMatrix();
  drawLife(lifeRemaining);
  glPopMatrix();

  glPushMatrix();
  drawProgress(progressPct);
  glPopMatrix();
}

void Canvas::drawPlayer() {
  // Initialize Projection Matrix
  glMatrixMode( GL_PROJECTION );
  // Save current matrix.
  glLoadIdentity();

  glFrustum( -1, 1, -1, 1, 1, Z_FAR);

  glMatrixMode(GL_MODELVIEW);
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glClearColor(0,0,0,0);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_2D,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glBindTexture(GL_TEXTURE_2D, playerDepthId);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (GLvoid*)depthData);
  glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(PLAYER_DRAW_WIDTH, PLAYER_DRAW_HEIGHT, -1100.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-PLAYER_DRAW_WIDTH, PLAYER_DRAW_HEIGHT, -1100.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-PLAYER_DRAW_WIDTH, -PLAYER_DRAW_HEIGHT, -1100.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(PLAYER_DRAW_WIDTH, -PLAYER_DRAW_HEIGHT, -1100.0f);
  glEnd(); 
  glMatrixMode(GL_MODELVIEW);

  glMatrixMode(GL_PROJECTION);
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
      (*i)->draw(&RECTANGLE);
    } else {
      (*i)->draw();
    }
  }
}

void Canvas::drawLife(int lifeRemaining) {
  float padding = 20.0f;
  float x = 0.0f;
  float y = 0.0f;

  std::stringstream life;
  life << "Life Remaining: ";
  lifeText->renderText(SCREEN_WIDTH, SCREEN_HEIGHT, x, y + padding, life.str());

  // Initialize Projection Matrix
  glMatrixMode( GL_PROJECTION );
  // Save current matrix.
  glPushMatrix();
  glLoadIdentity();

  glFrustum( 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, Z_NEAR, Z_FAR);

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

  double size = 50;

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  int i = 0;

  glBindTexture(GL_TEXTURE_2D, LIFE);
  glBegin(GL_QUADS);
  for (i; i < lifeRemaining; i++) {
    glTexCoord2f( 1.0f, 0.0f ); glVertex3f( x + 235 + size * (i+1) + padding * i , y + padding + 0        , -1100.0f );
    glTexCoord2f( 0.0f, 0.0f ); glVertex3f( x + 235 + size * (i  ) + padding * i , y + padding + 0        , -1100.0f );
    glTexCoord2f( 0.0f, 1.0f ); glVertex3f( x + 235 + size * (i  ) + padding * i , y + padding + 0 + size , -1100.0f );
    glTexCoord2f( 1.0f, 1.0f ); glVertex3f( x + 235 + size * (i+1) + padding * i , y + padding + 0 + size , -1100.0f );
  }
  glEnd();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}

void Canvas::drawHighscore(int points, int* highscores, bool highscoreAchieved, int lifeRemaining) {
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
}

void Canvas::drawProgress(float progressPct) {
  float padding = 20.0f;
  float x = 500.0f;
  float y = 500.0f;

  std::stringstream progress;
  progress << "Progress";
  lifeText->renderText(SCREEN_WIDTH, SCREEN_HEIGHT, x, y + padding, progress.str());

  // Initialize Projection Matrix
  glMatrixMode( GL_PROJECTION );
  // Save current matrix.
  glPushMatrix();
  glLoadIdentity();

  glFrustum( 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, Z_NEAR, Z_FAR);

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

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  int i = 0;
  // this border is predefined by the image files and the offset required to place one on top of the other.
  float border = 12.0f;

  glBindTexture(GL_TEXTURE_2D, PB);
  glBegin(GL_QUADS);
    glTexCoord2f( 1.0f, 0.0f ); glVertex3f( x + pbWidth * 2/3 , y + padding + pbHeight * 2 , -1100.0f );
    glTexCoord2f( 0.0f, 0.0f ); glVertex3f( x - pbWidth * 1/3 , y + padding + pbHeight * 2 , -1100.0f );
    glTexCoord2f( 0.0f, 1.0f ); glVertex3f( x - pbWidth * 1/3 , y + padding + pbHeight * 3 , -1100.0f );
    glTexCoord2f( 1.0f, 1.0f ); glVertex3f( x + pbWidth * 2/3 , y + padding + pbHeight * 3 , -1100.0f );
  glEnd();

  glBindTexture(GL_TEXTURE_2D, PRO);
  glBegin(GL_QUADS);
    glTexCoord2f( 1.0f, 0.0f ); glVertex3f( x - pbWidth * 1/3 + pbWidth * progressPct + border , y + padding + pbHeight * 2 + border , -1100.0f );
    glTexCoord2f( 0.0f, 0.0f ); glVertex3f( x - pbWidth * 1/3                         + border , y + padding + pbHeight * 2 + border , -1100.0f );
    glTexCoord2f( 0.0f, 1.0f ); glVertex3f( x - pbWidth * 1/3                         + border , y + padding + pbHeight * 3 - border , -1100.0f );
    glTexCoord2f( 1.0f, 1.0f ); glVertex3f( x - pbWidth * 1/3 + pbWidth * progressPct + border , y + padding + pbHeight * 3 - border , -1100.0f );
  glEnd();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}