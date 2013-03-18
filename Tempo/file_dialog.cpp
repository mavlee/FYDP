#include "file_dialog.h"

#ifndef USE_MAC_INCLUDES
OPENFILENAME ofn;       // common dialog box structure
#endif
char filePath[260];

std::string selectMusicFileDialog() {
#ifndef USE_MAC_INCLUDES
  *filePath = 0;
  // Initialize OPENFILENAME
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hInstance = NULL;
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = filePath;
  // Set lpstrFile[0] to '\0' so that GetOpenFileName does not
  // use the contents of szFile to initialize itself.
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = sizeof(filePath);
  ofn.lpstrFilter = "Music File (*.mp3)\0 *.mp3\0\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR ;

  // Display the Open dialog box.
  GetOpenFileName(&ofn);

  return filePath;
#else
  return "res/music/pumpedupkicks.mp3";
#endif
}

