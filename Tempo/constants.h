#ifndef CONSTANTS
#define CONSTANTS

const int COLOR_MODE_CYAN = 0;
const int COLOR_MODE_MULTI = 1;
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 800;
const int SCREEN_BPP = 32;
const float SCREEN_TILT = -20.f;
const float Z_FAR = 30000.0f;
const float Z_NEAR = 1000.f;
const float FPS_CAP = 1000.f / 60;
const float PEAK_THRESHOLD = 0.f;

const int NUM_ROWS = 4;
const int NUM_COLUMNS = 6;

const float SHAPE_X = 100.f;
const float SHAPE_Y = 100.f;
const float SHAPE_Z = 50.f;
const float OFFSET_FROM_CAMERA = Z_NEAR + 200.f;

const float SHIFT_INTERVAL_PER_SECOND = 500.f;

const int KINECT_DEPTH_WIDTH = 320;
const int KINECT_DEPTH_HEIGHT = 240;

// Parameters used to stretch the open gl texture
const int PLAYER_DRAW_WIDTH = KINECT_DEPTH_WIDTH*2;
const int PLAYER_DRAW_HEIGHT = KINECT_DEPTH_HEIGHT*3.5;

const int TOTAL_LIFE_COUNT = 10;

const int START_DELAY = 5000;

// Time before pausing when player is missing (ms)
const int MISSING_PLAYER_DELAY = 5000;
#endif
