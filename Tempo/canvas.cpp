#include "LOpenGL.h"
#include <stdio.h>
#include <iomanip>
#include <vector>
#include "canvas.h"
#include "constants.h"
#include "image_loader.h"
#include "kinect_controller.h"
using std::vector;

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
  for (int i = 0; i < KINECT_DEPTH_HEIGHT*KINECT_DEPTH_WIDTH*4; i++) {
    depthData[i] = (BYTE)0;
  }

  // Start SDL
  SDL_Init(SDL_INIT_EVERYTHING);
  // SDL_SWSURFACE implies that the surface is set up in software memory.
  putenv(strdup("SDL_VIDEO_CENTERED=1"));
  screen = SDL_SetVideoMode(width, height, SCREEN_BPP, SDL_OPENGL);
  //Initialize SDL ttf. Must be called prior to all SDL_ttf functions.
  TTF_Init();
  // AA
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); // On/off
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // Level
  // Set the caption on the window.
  SDL_WM_SetCaption("Tempo", NULL);
  // Other GL attributes
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // Initialize Projection Matrix
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  double s = 1; // hacky as fuck redux

  glFrustum( -s*SCALE*width/2, s*SCALE*width/2, s*SCALE*height/2, -s*SCALE*height/2, s*SCALE*Z_NEAR, SCALE*Z_FAR);

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

void Canvas::draw(float shiftZ, std::vector<Cube*>& obstacles, float progressPct, std::vector<int>& closeCubes, Cube::ColourSet currentColour, int comboLevel) {
  // Reset and clear
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //glEnable(GL_CULL_FACE); // commenting this out makes cubes more identifible due to random bug
  glEnable(GL_DEPTH_TEST);

  /** Draw 3D stuff **/
  glPushMatrix();
  glTranslatef(0, 0, shiftZ);
  drawObstacles(obstacles);
  drawGrid(closeCubes, obstacles);
  glPopMatrix();

  glPushMatrix();
  glPopMatrix();

  /** Draw 2D overlays **/
#ifndef USE_MAC_INCLUDES
  glPushMatrix();
  drawPlayer(currentColour, comboLevel);
  glPopMatrix();
#endif

  glPushMatrix();
  drawProgress(progressPct);
  glPopMatrix();

  glPushMatrix();
  drawCombo(currentColour);
  glPopMatrix();

  // Draw ui text
  float padding = 5.0f;
  float x = 20.0f;
  //fpsText->renderText(width, height, x, 10, fpsString);
  //comboLevelText->renderText(width, height, x, height - 100 - padding, comboLevelString);
  //pointsText->renderText(width, height, x, height - 50 - padding, pointsString);
  pointsText->renderText(width, height, width/4, height - 90, pointsString);

  // Update screen
  SDL_GL_SwapBuffers();
}


void Canvas::drawPlayer(Cube::ColourSet colour, int comboLevel) {
  // Set attributes for texture drawing
  glMatrixMode(GL_MODELVIEW);
  glPushAttrib(GL_ENABLE_BIT);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);

  // Set texture settings
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Set colour according to HP
  const float MIN_COLOUR = 0.4;
  if (comboLevel > MAX_LEVEL) {
    comboLevel = MAX_LEVEL;
  }
  //glColor4f(cubeColours[colour][0][0], cubeColours[colour][0][1], cubeColours[colour][0][2], 0.2f + 0.5f * comboLevel / MAX_LEVEL);
  glColor4f(1, 1, 1, 0.5);

  float kinectAspect = 1.f*KINECT_DEPTH_WIDTH/KINECT_DEPTH_HEIGHT;

  float z = OFFSET_FROM_CAMERA;
  int drawHeight = height*(z/Z_NEAR);
  int drawWidth = kinectAspect * drawHeight;

  glBindTexture(GL_TEXTURE_2D, playerDepthId);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (GLvoid*)depthData);
  glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(drawWidth/2, drawHeight/2, -z);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-drawWidth/2, drawHeight/2, -z);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-drawWidth/2, -drawHeight/2, -z);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(drawWidth/2, -drawHeight/2, -z);
  glEnd();

  // borders for player area
  glColor4f(0.4, 0.4, 0.4, 0.4);
  const float border = 10.f;
  glBegin(GL_QUADS);
    glVertex3f(drawWidth/2, drawHeight/2, -z);
    glVertex3f(drawWidth/2, drawHeight/2 + border, -z);
    glVertex3f(-drawWidth/2, drawHeight/2 + border, -z);
    glVertex3f(-drawWidth/2, drawHeight/2, -z);
  glEnd();

  glBegin(GL_QUADS);
    glVertex3f(-drawWidth/2, drawHeight/2, -z);
    glVertex3f(-drawWidth/2, -drawHeight/2, -z);
    glVertex3f(-drawWidth/2 - border, -drawHeight/2, -z);
    glVertex3f(-drawWidth/2 - border, drawHeight/2, -z);
  glEnd();

  glBegin(GL_QUADS);
    glVertex3f(-drawWidth/2, -drawHeight/2, -z);
    glVertex3f(-drawWidth/2, -drawHeight/2 - border, -z);
    glVertex3f(drawWidth/2, -drawHeight/2 - border, -z);
    glVertex3f(drawWidth/2, -drawHeight/2, -z);
  glEnd();

  glBegin(GL_QUADS);
    glVertex3f(drawWidth/2, drawHeight/2, -z);
    glVertex3f(drawWidth/2, -drawHeight/2, -z);
    glVertex3f(drawWidth/2 + border, -drawHeight/2, -z);
    glVertex3f(drawWidth/2 + border, drawHeight/2, -z);
  glEnd();

  // Reset attributes
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();
}

void Canvas::drawObstacles(vector<Cube*>& obstacles) {
  for (vector<Cube*>::iterator i = obstacles.begin(); i != obstacles.end(); i++) {
    (*i)->draw();
  }
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

  glColor4fv(TEXT_COLOUR);
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
  glColor4fv(PROGRESS_BAR);
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

void Canvas::drawCombo(Cube::ColourSet colour) {
  float padding = 10.0f;
  float size = 40.0f;

  float kinectAspect = 1.f*KINECT_DEPTH_WIDTH/KINECT_DEPTH_HEIGHT;
  int drawWidth = kinectAspect * height;
  int sideBarWidth = (width - drawWidth)/2 - padding;

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

  //glColor4f(cubeColours[colour][0][0], cubeColours[colour][0][1], cubeColours[colour][0][2], 0.2f + 0.5f * comboLevel / MAX_LEVEL);
  glColor4f(cubeColours[colour][0][0], cubeColours[colour][0][1], cubeColours[colour][0][2], 1);
  int xOff[2];
  xOff[0] = 0;
  xOff[1] = width - sideBarWidth;

  comboLevel = min(comboLevel, MAX_LEVEL);
  for (int y = -comboLevel+1; y < comboLevel; y++) {
    glBegin(GL_QUADS);
      glVertex3f(0, height/2-size + y*(2*size + padding), 0);
      glVertex3f(sideBarWidth, height/2-size + y*(2*size + padding), 0);
      glVertex3f(sideBarWidth, height/2+size + y*(2*size + padding), 0);
      glVertex3f(0, height/2+size + y*(2*size + padding), 0);
    glEnd();
    glBegin(GL_QUADS);
      glVertex3f(width - sideBarWidth, height/2+size + y*(2*size + padding), 0);
      glVertex3f(width - sideBarWidth, height/2-size + y*(2*size + padding), 0);
      glVertex3f(width, height/2-size + y*(2*size + padding), 0);
      glVertex3f(width, height/2+size + y*(2*size + padding), 0);
    glEnd();
  }

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

void Canvas::drawGrid(std::vector<int>& closeCubes, std::vector<Cube*>& obstacles) {
  glMatrixMode(GL_MODELVIEW);
  glPushAttrib(GL_ENABLE_BIT);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glEnable(GL_BLEND);

  float padding = 0.0f;
  float w = 1.0*SHAPE_X;
  float h = 1.0*SHAPE_Y;
  for (int i = 0; i < obstacles.size(); i++) {
    float border = 8.0f;
    glColor4f(1, 1, 1, 0.7);

    float z = obstacles[i]->zFront + 1;
    int r = obstacles[i]->r;
    int c = obstacles[i]->c;
    float centreX = obstacles[i]->centre.x;
    float centreY = obstacles[i]->centre.y;
    for (int j = 0; j < closeCubes.size(); j++) {
      if (closeCubes[j] == i) {
        border *= 2;
        glColor4f(1, 1, 1, 0.9);
        break;
      }
    }
    // top bar
    glBegin(GL_QUADS);
      glVertex3f(centreX - w/2, padding + centreY + SHAPE_Y/2, z);
      glVertex3f(centreX - w/2, padding + centreY + SHAPE_Y/2 - border, z);
      glVertex3f(centreX + w/2, padding + centreY + SHAPE_Y/2 - border, z);
      glVertex3f(centreX + w/2, padding + centreY + SHAPE_Y/2, z);
    glEnd();

    // bottom bar
    glBegin(GL_QUADS);
      glVertex3f(centreX - w/2, centreY - padding - SHAPE_Y/2, z);
      glVertex3f(centreX + w/2, centreY - padding - SHAPE_Y/2, z);
      glVertex3f(centreX + w/2, centreY - padding - SHAPE_Y/2 + border, z);
      glVertex3f(centreX - w/2, centreY - padding - SHAPE_Y/2 + border, z);
    glEnd();

    // left bar
    glBegin(GL_QUADS);
      glVertex3f(centreX - padding - SHAPE_X/2, centreY - h/2, z);
      glVertex3f(centreX - padding - SHAPE_X/2 + border, centreY - h/2, z);
      glVertex3f(centreX - padding - SHAPE_X/2 + border, centreY + h/2, z);
      glVertex3f(centreX - padding - SHAPE_X/2, centreY + h/2, z);
    glEnd();

    // right bar
    glBegin(GL_QUADS);
      glVertex3f(centreX + padding + SHAPE_X/2, centreY + h/2, z);
      glVertex3f(centreX + padding + SHAPE_X/2 - border, centreY + h/2, z);
      glVertex3f(centreX + padding + SHAPE_X/2 - border, centreY - h/2, z);
      glVertex3f(centreX + padding + SHAPE_X/2, centreY - h/2, z);
    glEnd();
  }

  // Reset attributes, projection matrix
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glDisable(GL_BLEND);

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glPopAttrib();
}

void Canvas::drawHighscore(int points, int* highscores, bool highscoreAchieved) {
  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  float x = SCREEN_WIDTH / 2 - 150;
  float y = 150;

  std::stringstream highscore_caption;
  if (highscoreAchieved) {
    highscore_caption << "New Highscore! Score: " << points;
    scoreText->renderText(SCREEN_WIDTH, SCREEN_HEIGHT, x , y      , highscore_caption.str());
  } else {
    highscore_caption << "Score: " << points;
    scoreText->renderText(SCREEN_WIDTH, SCREEN_HEIGHT, x , y      , highscore_caption.str());
  }
  int i = 0;
  for (i; i < 10; i++) {
    std::stringstream highscore_line;
    highscore_line <<  "Record " << std::setfill(' ') << std::setw(3) << i + 1 << ":       " << highscores[i];
    scoreText->renderText(SCREEN_WIDTH, SCREEN_HEIGHT, x, y + (i + 1) * 50, highscore_line.str());
  }
  std::stringstream restart_line;
  restart_line <<  "Press \'r\' to play again.";
  scoreText->renderText(SCREEN_WIDTH, SCREEN_HEIGHT, x , y + (11) * 50, restart_line.str());

  glPopMatrix();

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
  pointsCaption << points;
  pointsString = pointsCaption.str();
}

void Canvas::setComboLevelText(int comboLevel) {
  this->comboLevel = comboLevel;
  std::stringstream comboCaption;
  comboCaption << "Combo Level: " << comboLevel;
  comboLevelString = comboCaption.str();
}
