#ifndef MUSIC_HANDLER_H
#define MUSIC_HANDLER_H

#include "inc/sound/sound_includes.h"
#include <string>

#define BUF_SIZE 512 // dependent on the kind of FFT data used
#define THRESHOLD_WINDOW_SIZE 10
#define THRESHOLD_MULTIPLIER 4.0
#define NUM_BANDS 16
#define SAMPLE_RATE 44100
#define SAMPLE_HISTORY 20 // corresponds to about half a second at the moment

using namespace std;

class MusicHandler {
  public:
    MusicHandler();
    ~MusicHandler();
    int setMusicFile(string filename);
    string getMusicFile();

    int getNumBands();
    const vector<vector<float> >& getPeakData();
    const vector<float>& getIntensityData();

    void play();
    void pause();
    void setPosition(QWORD pos);
    double getPositionInSec();
    double getLengthInSec();
    double getPeakDataPerSec();

  private:
    void toCsv(string name, vector<vector<float> > vec);
    void error(string msg);
    int analyze();
    int preparePlayback();
    void reset();

    string musicFilename;
    DWORD floatable; // floating-point channel support?
    vector<vector<float> > peakData;
    vector<float> intensityData;
    DWORD playbackChan;	// the channel... HMUSIC or HSTREAM
    DWORD numChans;
};

#endif
