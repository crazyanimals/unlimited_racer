// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


// for omitting some security MSVC 2005 warnings
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif


#define WIN32_LEAN_AND_MEAN	
#ifndef WINVER				
#define WINVER 0x0500		
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#ifndef DIRECTINPUT_VERSION 
#define DIRECTINPUT_VERSION  0x0800
#endif
// Windows headers
#include <windows.h>


// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <mmsystem.h>
#include <vector>

// DirectX headers
#include <d3d9.h>
#include <d3dx9math.h>
#include <dxerr9.h>
#include <dxfile.h>
#include <xact.h>

#include <atlstr.h>

