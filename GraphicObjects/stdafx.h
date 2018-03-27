#pragma once


// for omitting some security MSVC 2005 warnings
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0500		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif
// windows headers
#include <windows.h>
#include <atlstr.h>

// C Run-Time headers
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <set>
#include <list>
#include <string>
#include <fstream>


// DX headers
#include <d3d9.h>
#include <d3dx9math.h>
#include <dxerr9.h>
#include <d3d9types.h>
