#ifndef PCH_H
#define PCH_H

// Standard includes.
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Windows API.
#include <Windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <ShlObj_core.h>
#include <commctrl.h>

// Enable visual styles.
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#endif