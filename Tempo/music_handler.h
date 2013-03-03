#ifndef MUSIC_HANDLER_H
#define MUSIC_HANDLER_H

#include "inc/sound/sound_includes.h"
#include <string>

#define BUF_SIZE 512
#define THRESHOLD_WINDOW_SIZE 15
#define THRESHOLD_MULTIPLIER 1.8
#define NUM_BANDS 1

using namespace std;

class MusicHandler {
  public:
    MusicHandler();
    ~MusicHandler();
    int setMusicFile(string filename);

    int getNumBands();
    const vector<vector<float> >& getPeakData();

    void play();
    void pause();
    void setPosition(QWORD pos);
    double getPositionInSec();

  private:
    void toCsv(string name, vector<float> vec);
    void error(string msg);
    int analyze();
    int preparePlayback();
    void reset();

    string musicFilename;
    DWORD floatable; // floating-point channel support?
    vector<vector<float> > peakData;
    DWORD playbackChan;	// the channel... HMUSIC or HSTREAM
};

#endif
