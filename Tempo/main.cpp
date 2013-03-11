#include "game.h"
#include "constants.h"
#include "util.h"

int main( int argc, char* args[] ) {
#ifndef USE_MAC_INCLUDES
  OpenConsole();
#endif
  Game theGame(SCREEN_WIDTH, SCREEN_HEIGHT);
  return theGame.execute();
}
