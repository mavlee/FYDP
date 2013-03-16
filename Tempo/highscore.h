#ifndef HIGHSCORE_H
#define HIGHSCORE_H

#include <string>

//using namespace std;

int* readHighScores(std::string musicFile);

void writeHighScores(std::string musicFile);

bool insertHighScore(int score);

#endif HIGHSCORE_H