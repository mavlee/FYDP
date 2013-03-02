#include "inc/sound/sound_includes.h"
#include "music_handler.h"
#include <cmath>
#include <string>

using namespace std;

MusicHandler::MusicHandler() {
  musicFilename = "res/music/test.mp3";
}

MusicHandler::~MusicHandler() {
}

void MusicHandler::setMusicFile(string filename) {
  musicFilename = filename;
}

void MusicHandler::to_csv (string name, vector<float> vec) {
  ofstream file;
  file.open(name.c_str());
  for (vector<float>::iterator it = vec.begin(); it != vec.end(); ++it) {
    file << *it << endl;
  }
  file.close();
}

void MusicHandler::error(string msg) {
  cout << msg << endl;
}

// returns 0 for successful run, 1 otherwise
int MusicHandler::analyze() {
  cout << "start" << endl;

  // check the correct BASS was loaded
  if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
    error("Wrong BASS version!");
    return 1;
  }

  DWORD floatable; // floating-point channel support?
  DWORD chan;	// the channel... HMUSIC or HSTREAM

  // enable floating-point DSP
  BASS_SetConfig(BASS_CONFIG_FLOATDSP,TRUE);
  // initialize - default device
  if (!BASS_Init(-1,44100,0,NULL,NULL)) {
    error("Error initialising BASS!");
    return 1;
  }

  // check for floating-point capability
  floatable = BASS_StreamCreate(44100, 2, BASS_SAMPLE_FLOAT, NULL, 0);
  if (floatable) {
    BASS_StreamFree(floatable);
    floatable=BASS_SAMPLE_FLOAT;
  }

  // consider decoding stream in mono, since it's faster
  if (!(chan=BASS_StreamCreateFile(FALSE, musicFilename.c_str(), 0, 0, BASS_SAMPLE_FLOAT|BASS_STREAM_DECODE|floatable))){
    // not playable
    char msg[256];
    cout << "ERROR" << endl;
    sprintf(msg, "File %s not playable!", musicFilename.c_str());
    error(msg);
    return 1;
  }

  // get number of channels
  BASS_CHANNELINFO channel_info;
  if (!BASS_ChannelGetInfo(chan,&channel_info)) {
    error("Error getting channel info.");
    return 1;
  }
  unsigned int chans = channel_info.chans;

  // get data
  DWORD ret = 0;
  vector<float *> FFT_data;
  int num_samples = 0;
  cout << "processing data\n";
  while (true) {
    float *buf = new float[BUF_SIZE];
    FFT_data.push_back(buf);
    ret = BASS_ChannelGetData(chan, FFT_data.back(), BASS_DATA_FFT1024);
    if (-1 == ret) {
      break;
    }
    num_samples += ret;
  }
  // BASS_ChannelGetData actually returns num bytes read from source
  num_samples /= chans;
  num_samples /= 4; // 32-bit float samples...?
  if (BASS_ERROR_ENDED != BASS_ErrorGetCode()) {
    error("Error getting data from file");
    BASS_Free();
    return 1;
  }
  cout << num_samples << " total samples\n";
  cout << (num_samples/44100)/60 << " m " << ((int) (num_samples/44100.0 + 0.5)) % 60 << " s\n";

  // calculate spectral flux
  vector<float> spectral_flux (FFT_data.size() - 1, 0);
  for (int i = 0; i < spectral_flux.size(); i++) {
    for (int j = 0; j < BUF_SIZE; j++) {
      spectral_flux[i] += abs(FFT_data[i+1][j] - FFT_data[i][j]);
    }
  }
  to_csv("spectral_flux.csv", spectral_flux);

  // calculate threshold
  vector<float> threshold(FFT_data.size() - 1, 0);
  for (int i = 0; i < spectral_flux.size(); i++) {
    int start = max(0, i - THRESHOLD_WINDOW_SIZE);
    int end = min(int(spectral_flux.size() - 1), i + THRESHOLD_WINDOW_SIZE);
    float mean = 0;
    for (int j = start; j <= end; j++) {
      mean += spectral_flux[j];
    }
    mean /= (end - start);
    threshold[i] =  mean * THRESHOLD_MULTIPLIER;
  }
  to_csv("threshold.csv", threshold);

  // apply threshold to spectral flux
  vector<float> prunned_spectral_flux (FFT_data.size() - 1, 0);
  for (int i = 0; i < threshold.size(); i++) {
    if( threshold[i] <= spectral_flux[i] )
      prunned_spectral_flux[i] = spectral_flux[i] - threshold[i];
    else
      prunned_spectral_flux[i] = 0;
  }
  to_csv("prunned_spectral_flux.csv", prunned_spectral_flux);

  // find peaks
  vector<float> peaks(FFT_data.size() - 2, 0);
  for (int i = 0; i < prunned_spectral_flux.size() - 1; i++) {
    if (prunned_spectral_flux[i] > prunned_spectral_flux[i+1]) {
      peaks[i] = prunned_spectral_flux[i];
    } else {
      peaks[i] = 0;
    }
  }
  to_csv("peaks.csv", peaks);

  cout << "done\n";

  BASS_Free();
  return 0;
}
