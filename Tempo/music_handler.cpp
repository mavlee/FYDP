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
  if (!BASS_Init(-1,44100,0,NULL,NULL)) {
    char msg[100] = "Error initialising BASS!";
    error(msg);
    //throw exception(msg);
    throw msg;
  }

  // check for floating-point capability
  floatable = BASS_StreamCreate(44100, 2, BASS_SAMPLE_FLOAT, NULL, 0);
  if (floatable) {
    BASS_StreamFree(floatable);
    floatable=BASS_SAMPLE_FLOAT;
  }

  // init fields
  musicFilename = "";
  playbackChan = 0;
  peakData.resize(NUM_BANDS);
}

MusicHandler::~MusicHandler() {
  reset();
}

void MusicHandler::reset() {
  musicFilename = "";
  playbackChan = 0;

  for (int i = 0; i < peakData.size(); i++) {
      peakData[i].clear();
  }

  BASS_Free();
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
  num_samples /= channel_info.chans;
  num_samples /= 4; // 32-bit float samples...?
  if (BASS_ERROR_ENDED != BASS_ErrorGetCode()) {
    error("Error getting data from file");
    BASS_Free();
    return 1;
  }
  printf("%d total samples\n", num_samples);
  printf("%d m %d s\n", (num_samples/44100)/60, ((int) (num_samples/44100.0 + 0.5)) % 60);

  // calculate spectral flux
  vector<float> spectral_flux (FFT_data.size() - 1, 0);
  for (int i = 0; i < spectral_flux.size(); i++) {
    for (int j = 0; j < BUF_SIZE; j++) {
      spectral_flux[i] += abs(FFT_data[i+1][j] - FFT_data[i][j]);
    }
  }
  toCsv("spectral_flux.csv", spectral_flux);

  // calculate threshold
  vector<float> threshold(FFT_data.size() - 1, 0);
  for (int i = 0; i < threshold.size(); i++) {
    int start = max(0, i - THRESHOLD_WINDOW_SIZE);
    int end = min(int(spectral_flux.size() - 1), i + THRESHOLD_WINDOW_SIZE);
    float mean = 0;
    for (int j = start; j <= end; j++) {
      mean += spectral_flux[j];
    }
    mean /= (end - start);
    threshold[i] =  mean * THRESHOLD_MULTIPLIER;
  }
  toCsv("threshold.csv", threshold);

  // apply threshold to spectral flux
  vector<float> prunned_spectral_flux (FFT_data.size() - 1, 0);
  for (int i = 0; i < prunned_spectral_flux.size(); i++) {
    if( threshold[i] <= spectral_flux[i] )
      prunned_spectral_flux[i] = spectral_flux[i] - threshold[i];
    else
      prunned_spectral_flux[i] = 0;
  }
  toCsv("prunned_spectral_flux.csv", prunned_spectral_flux);

  // find peaks
  peakData[0].resize(FFT_data.size() - 2, 0);
  for (int i = 0; i < prunned_spectral_flux.size() - 1; i++) {
    if (prunned_spectral_flux[i] > prunned_spectral_flux[i+1]) {
      peakData[0][i] = prunned_spectral_flux[i];
    } else {
      peakData[0][i] = 0;
    }
  }
  toCsv("peaks.csv", peakData[0]);

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

int MusicHandler::getPosition() {
    QWORD pos = BASS_ChannelGetPosition(playbackChan, BASS_POS_BYTE);
    printf("pos: %ld, pos2sec: %Lf\n", pos, BASS_ChannelBytes2Seconds(playbackChan, pos));
    return pos;
}

/** Helper fcns**/
void MusicHandler::toCsv (string name, vector<float> vec) {
  ofstream file;
  file.open(name.c_str());
  for (vector<float>::iterator it = vec.begin(); it != vec.end(); ++it) {
    file << *it << endl;
  }
  file.close();
}

void MusicHandler::error(string msg) {
  printf("Music handler error: %s.\nError code: %d", msg.c_str(), BASS_ErrorGetCode());
  //printf("Music handler error: %s.\n", msg);
}
