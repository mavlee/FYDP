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
  ret = analyze2();
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
  numChans = channel_info.chans;
  num_samples /= numChans;
  num_samples /= 4; // 32-bit float samples...?
  if (BASS_ERROR_ENDED != BASS_ErrorGetCode()) {
    error("Error getting data from file");
    BASS_Free();
    return 1;
  }
  printf("%d total samples\n", num_samples);
  printf("%d m %d s\n", (num_samples/SAMPLE_RATE)/60, ((int) (num_samples*1.0/SAMPLE_RATE + 0.5)) % 60);

  // calculate spectral flux
  vector<vector<float> > spectral_flux (NUM_BANDS);
  for (int v = 0; v < NUM_BANDS; v++) {
    spectral_flux[v].resize(FFT_data.size() - 1, 0);
    for (int i = 0; i < spectral_flux[v].size(); i++) {
      for (int j = 0 + v*BUF_SIZE/NUM_BANDS; j < (v+1)*BUF_SIZE/NUM_BANDS; j++) {
        spectral_flux[v][i] += abs(FFT_data[i+1][j] - FFT_data[i][j]);
        //spectral_flux[v][i] += max(FFT_data[i+1][j] - FFT_data[i][j], 0.f);
      }
    }
  }
  toCsv("spectral_flux.csv", spectral_flux);

  // calculate threshold
  vector<vector<float> > threshold(NUM_BANDS);
  for (int v = 0; v < NUM_BANDS; v++) {
    threshold[v].resize(FFT_data.size() - 1, 0);
    for (int i = 0; i < threshold[v].size(); i++) {
      int start = max(0, i - THRESHOLD_WINDOW_SIZE);
      int end = min(int(spectral_flux[0].size() - 1), i + THRESHOLD_WINDOW_SIZE);
      float mean = 0;
      for (int j = start; j <= end; j++) {
        mean += spectral_flux[v][j];
      }
      mean /= (end - start);
      threshold[v][i] =  mean * THRESHOLD_MULTIPLIER;
    }
  }
  toCsv("threshold.csv", threshold);

  // apply threshold to spectral flux
  vector<vector<float> > prunned_spectral_flux(NUM_BANDS);
  for (int v = 0; v < NUM_BANDS; v++) {
    prunned_spectral_flux[v].resize(FFT_data.size() - 1, 0);
    for (int i = 0; i < prunned_spectral_flux[v].size(); i++) {
      if( threshold[v][i] <= spectral_flux[v][i] )
        prunned_spectral_flux[v][i] = spectral_flux[v][i] - threshold[v][i];
      else
        prunned_spectral_flux[v][i] = 0;
    }
  }
  toCsv("prunned_spectral_flux.csv", prunned_spectral_flux);

  // find peaks
  for (int v = 0; v < NUM_BANDS; v++) {
    peakData[v].resize(FFT_data.size() - 2, 0);
    for (int i = 0; i < prunned_spectral_flux[v].size() - 1; i++) {
      if (prunned_spectral_flux[v][i] > prunned_spectral_flux[v][i+1]) {
        peakData[v][i] = prunned_spectral_flux[v][i];
      } else {
        peakData[v][i] = 0;
      }
    }
  }
  toCsv("peaks.csv", peakData);

  // clean up memory
  for (int i = 0; i < FFT_data.size(); i++) {
    delete [] FFT_data[i];
    FFT_data[i] = NULL;
  }

  return 0;
}

int MusicHandler::analyze2() {
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
        energies[v][i] += 8.0/512*FFT_data[i][j];
      }
    }
  }

  vector<vector<float> > average_energies(NUM_BANDS);
  for (int v = 0; v < NUM_BANDS; v++) {
    average_energies[v].resize(FFT_data.size() - 1, 0);
    for (int i = 0; i < average_energies[v].size(); i++) {
      for (int j = i-43; j < i; j++) {
        average_energies[v][i] += 1.0/min(i+1,43)*energies[v][j];
      }
    }
  }

  // find peaks
  /*
  for (int v = 0; v < NUM_BANDS; v++) {
    peakData[v].resize(FFT_data.size() - 1, 0);
    for (int i = 0; i < energies[v].size(); i++) {
      if (energies[v][i] > 4*average_energies[v][i]) {
        peakData[v][i] = 1;
      } else {
        peakData[v][i] = 0;
      }
    }
  }
  */

  for (int v = 0; v < NUM_BANDS; v++) {
    peakData[v].resize(FFT_data.size() - 1, 0);
  }
  for (int i = 0; i < energies[0].size(); i++) {
    int value = 0;
    int m = -1;
    for (int v = 0; v < NUM_BANDS; v++) {
      if (energies[v][i] - 3*average_energies[v][i] > value) {
        value = energies[v][i] - average_energies[v][i];
        m = v;
      }
      peakData[v][i] = 0;
    }
    if (m > -1) {
      peakData[m][i] = 1;
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
