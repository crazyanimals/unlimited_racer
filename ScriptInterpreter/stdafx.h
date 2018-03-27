#pragma once


// for omitting some security MSVC 2005 warnings
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <atlstr.h>
#include <math.h>
#include <vector>
#include <map>
#include <stack>
#include <list>