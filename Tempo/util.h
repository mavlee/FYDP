/*
Utility functions
*/

#ifndef __UTIL_H__
#define __UTIL_H__

#ifndef USE_MAC_INCLUDES

#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

/* Displays a message box with an error message.  To use, call
  ErrorHandler(TEXT("Your message here"));
  */
void ErrorHandler(LPTSTR lpszFunction);

/* Attaches a console to our GUI application, just call this once */
void OpenConsole();

#endif


#endif
