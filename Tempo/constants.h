#ifndef CONSTANTS
#define CONSTANTS

#include "colours.h"

const int COLOR_MODE_CYAN = 0;
const int COLOR_MODE_MULTI = 1;
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 800;
const int SCREEN_BPP = 32;
const float SCREEN_TILT = -20.f;
const float SCALE = 1; // hacky as fuck
const float Z_FAR = 50000.0f;
const float Z_NEAR = 1000.f;
const float FPS_CAP = 1000.f / 60;
const float PEAK_THRESHOLD = 0.f;

const int NUM_ROWS = 4;
const int NUM_COLUMNS = 6;

const float SHAPE_X = SCALE*100.f;
const float SHAPE_Y = SCALE*100.f;
const float SHAPE_Z = SCALE*15.f;
const float OFFSET_FROM_CAMERA = SCALE*(Z_NEAR);

const float SHIFT_INTERVAL_PER_SECOND = 500.f;

const int KINECT_DEPTH_WIDTH = 320;
const int KINECT_DEPTH_HEIGHT = 240;

const int MAX_LEVEL = 4;

// Parameters used to stretch the open gl texture
const int PLAYER_DRAW_WIDTH = KINECT_DEPTH_WIDTH*2;
const int PLAYER_DRAW_HEIGHT = KINECT_DEPTH_HEIGHT*3.5;

const int START_DELAY = 5000;

// Time before pausing when player is missing (ms)
const int MISSING_PLAYER_DELAY = 2000;

#endif
