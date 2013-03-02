#ifndef MUSIC_HANDLER_H
#define MUSIC_HANDLER_H

#include "inc/sound/sound_includes.h"
#include <string>

#define BUF_SIZE 512
#define THRESHOLD_WINDOW_SIZE 15
#define THRESHOLD_MULTIPLIER 1.5

using namespace std;

class MusicHandler {
  public:
    MusicHandler();
    ~MusicHandler();
    int analyze();
    void setMusicFile(string filename);

  private:
    void to_csv(string name, vector<float> vec);
    void error(string msg);
    string musicFilename;
};

#endif
