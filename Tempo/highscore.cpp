#include "highscore.h"
#include <iostream>
#include <fstream>

int highscores[10];

int* readHighScores(std::string musicFile) {
  std::ifstream ifile(musicFile.c_str());
  if (!ifile) {
    return highscores;
  } else {
    std::string scoresString;
    getline(ifile, scoresString);
    // break by comma, parse as int
    std::string temp;
    int i = 0;
    for (i; i < 10; i++) {
      temp = scoresString.substr(0, scoresString.find(','));
      scoresString = scoresString.substr(scoresString.find(',') + 1);
      highscores[i] = atoi(temp.c_str());
    }
  }
  ifile.close();
  return highscores;
}

void writeHighScores(std::string musicFile) {
  std::ofstream ofile;
  ofile.open(musicFile.c_str());
  int i = 0;
  for (i; i < 10; i++) {
    ofile << highscores[i] << ',';
  }
  ofile << std::endl;
  ofile.close();
}

bool insertHighScore(int score) {
  int i = 0;
  for (i; i < 10; i++) {
    if (score >= highscores[i]) {
      int j = 9;
      for (j; j >= i; j--) {
        highscores[j + 1] = highscores[j];
      }
      highscores[i] = score;
      return true;
    }
  }
  return false;
}
