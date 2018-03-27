#pragma once


// for omitting some security MSVC 2005 warnings
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif


#define WIN32_LEAN_AND_MEAN        // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <atlstr.h>
#include <d3d9.h>
#include <d3dx9math.h>
#include <CString>
#include "RM_Errors.h"
#include "..\Globals\GlobalsIncludes.h"
