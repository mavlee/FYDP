#ifndef MUSIC_HANDLER_H
#define MUSIC_HANDLER_H

#include "inc/sound/sound_includes.h"

#define BUF_SIZE 512
#define THRESHOLD_WINDOW_SIZE 15
#define THRESHOLD_MULTIPLIER 1.5

using namespace std;

class MusicHandler {
  public:
    MusicHandler();
    ~MusicHandler();
    int analyze();

  private:
    void to_csv(char* name, vector<float> vec);
    void error(char msg[]);
};

#endif
