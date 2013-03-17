#include "inc/sound/sound_includes.h"
#include "music_handler.h"
#include <cmath>
#include <string>
#include <exception>

using namespace std;

MusicHandler::MusicHandler() {
  // check the correct BASS was loaded
  if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
    char msg[256];
    sprintf(msg, "Wrong BASS version: %d, expected %d\n", HIWORD(BASS_GetVersion()), BASSVERSION);
    error(msg);
    //throw exception(msg);
    throw msg;
  }

  // enable floating-point DSP
  BASS_SetConfig(BASS_CONFIG_FLOATDSP,TRUE);
  // initialize - default device
  if (!BASS_Init(-1,SAMPLE_RATE,0,NULL,NULL)) {
    char msg[100] = "Error initialising BASS!";
    error(msg);
    //throw exception(msg);
    throw msg;
  }

  // check for floating-point capability
  floatable = BASS_StreamCreate(SAMPLE_RATE, 2, BASS_SAMPLE_FLOAT, NULL, 0);
  if (floatable) {
    BASS_StreamFree(floatable);
    floatable=BASS_SAMPLE_FLOAT;
  }

  // init fields
  musicFilename = "";
  playbackChan = 0;
  numChans = 2;
  peakData.resize(NUM_BANDS);
}

MusicHandler::~MusicHandler() {
  reset();
  BASS_Free();
}

void MusicHandler::reset() {
  musicFilename = "";
  playbackChan = 0;
  numChans = 2;

  for (int i = 0; i < peakData.size(); i++) {
    peakData[i].clear();
  }
  intensityData.clear();

  BASS_StreamFree(playbackChan);
}

int MusicHandler::setMusicFile(string filename) {
  if (playbackChan != 0) {
    reset();
  }

  printf("Set file to %s\n", filename.c_str());

  // check if file exists
  ifstream ifile(filename.c_str());
  if (!ifile) {
    error("File does not exist");
    return 1;
  }
  musicFilename = filename;

  // perform analysis on file
  int ret;
  ret = analyze();
  if (ret != 0) {
    reset();
    return ret;
  }

  // prepare playback channel
  ret = preparePlayback();
  if (ret != 0) {
    reset();
    return ret;
  }

  return 0;
}

string MusicHandler::getMusicFile() {
  return musicFilename;
}

// returns 0 if sucessful
int MusicHandler::analyze() {
  DWORD decodeChan;	// the channel... HMUSIC or HSTREAM

  // consider decoding stream in mono, since it's faster
  if (!(decodeChan=BASS_StreamCreateFile(FALSE, musicFilename.c_str(), 0, 0, BASS_SAMPLE_FLOAT|BASS_STREAM_DECODE|floatable))){
    // not playable
    char msg[256];
    sprintf(msg, "File %s not playable!", musicFilename.c_str());
    error(msg);
    return 1;
  }

  // get number of channels
  BASS_CHANNELINFO channel_info;
  if (!BASS_ChannelGetInfo(decodeChan,&channel_info)) {
    error("Error getting channel info.");
    return 1;
  }

  // get data
  DWORD ret = 0;
  vector<float *> FFT_data;
  int num_samples = 0;
  while (true) {
    float *buf = new float[BUF_SIZE];
    FFT_data.push_back(buf);
    ret = BASS_ChannelGetData(decodeChan, FFT_data.back(), BASS_DATA_FFT1024);
    if (-1 == ret) {
      break;
    }
    num_samples += ret;
  }
  // BASS_ChannelGetData actually returns num bytes read from source
  numChans = channel_info.chans;
  num_samples /= numChans;
  num_samples /= 4; // 32-bit float samples...?
  if (BASS_ERROR_ENDED != BASS_ErrorGetCode()) {
    error("Error getting data from file");
    BASS_Free();
    return 1;
  }
  printf("%d total samples\n", num_samples);
  printf("%d total channels\n", numChans);
  printf("%d m %d s\n", (num_samples/SAMPLE_RATE)/60, ((int) (num_samples*1.0/SAMPLE_RATE + 0.5)) % 60);

  vector<vector<float> > energies(NUM_BANDS);
  for (int v = 0; v < NUM_BANDS; v++) {
    energies[v].resize(FFT_data.size() - 1, 0);
    for (int i = 0; i < energies[v].size(); i++) {
      for (int j = 0 + v*BUF_SIZE/NUM_BANDS; j < (v+1)*BUF_SIZE/NUM_BANDS; j++) {
        energies[v][i] += 0.25*FFT_data[i][j];
      }
    }
  }

  for (int i = 0; i < energies[0].size(); i++) {
    float total = 0;
    for (int v = 0; v < NUM_BANDS; v++) {
      total += energies[v][i];
    }
    intensityData.push_back(1000 * total / NUM_BANDS);
  }

  vector<vector<float> > average_energies(NUM_BANDS);
  for (int v = 0; v < NUM_BANDS; v++) {
    average_energies[v].resize(FFT_data.size() - 1, 0);
    for (int i = 0; i < average_energies[v].size(); i++) {
      for (int j = max(0, i - SAMPLE_HISTORY); j < i; j++) {
        average_energies[v][i] += 1.0 / min(i + 1, SAMPLE_HISTORY) * energies[v][j];
      }
    }
  }

  // find peaks
  for (int v = 0; v < NUM_BANDS; v++) {
    peakData[v].resize(FFT_data.size() - 1, 0);
  }
  for (int i = 0; i < energies[0].size(); i++) {
    for (int v = 0; v < NUM_BANDS; v++) {
      if (energies[v][i] - 4 * average_energies[v][i] > 0) {
        peakData[v][i] = energies[v][i] / (average_energies[v][i]);
      } else {
        peakData[v][i] = 0;
      }
    }
  }

  // clean up memory
  for (int i = 0; i < FFT_data.size(); i++) {
    delete [] FFT_data[i];
    FFT_data[i] = NULL;
  }

  return 0;
}

// returns 0 if sucessful
int MusicHandler::preparePlayback() {
  if (!(playbackChan=BASS_StreamCreateFile(FALSE, musicFilename.c_str(), 0, 0, BASS_SAMPLE_FLOAT|floatable))){
    // not playable
    char msg[256];
    sprintf(msg, "File %s not playable!", musicFilename.c_str());
    error(msg);
    return 1;
  }

  return 0;
}

const vector<vector<float> >& MusicHandler::getPeakData() {
  return peakData;
}

const vector<float>& MusicHandler::getIntensityData() {
  return intensityData;
}

int MusicHandler::getNumBands() {
  return NUM_BANDS;
}

/** Playback **/
void MusicHandler::play() {
  BASS_ChannelPlay(playbackChan,FALSE);
}

void MusicHandler::pause() {
  BASS_ChannelPause(playbackChan);
}

void MusicHandler::setPosition(QWORD pos) {
  BASS_ChannelSetPosition(playbackChan, pos, BASS_POS_BYTE);
}

double MusicHandler::getPositionInSec() {
  QWORD pos = BASS_ChannelGetPosition(playbackChan, BASS_POS_BYTE);
  double pos2sec = BASS_ChannelBytes2Seconds(playbackChan, pos);
  return pos2sec;
}

double MusicHandler::getLengthInSec() {
  QWORD pos = BASS_ChannelGetLength(playbackChan, BASS_POS_BYTE);
  double pos2sec = BASS_ChannelBytes2Seconds(playbackChan, pos);
  return pos2sec;
}

double MusicHandler::getPeakDataPerSec() {
    return 1.0 * SAMPLE_RATE / BUF_SIZE / numChans;
}

/** Helper fcns**/
void MusicHandler::toCsv (string name, vector<vector<float> > vec) {
  ofstream file;
  file.open(name.c_str());
  for (int samples = 0; samples < vec[0].size(); samples++) {
    for (int bands = 0; bands < vec.size(); bands++) {
        file << vec[bands][samples];
        if (bands != vec.size() - 1) {
            file << ", ";
        }
    }
    file << endl;
  }
  file.close();
}

void MusicHandler::error(string msg) {
  printf("Music handler error: %s.\nError code: %d", msg.c_str(), BASS_ErrorGetCode());
  //printf("Music handler error: %s.\n", msg);
}
