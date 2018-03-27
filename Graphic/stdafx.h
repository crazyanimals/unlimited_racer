/*
	stdafx:	precompiled header
	Creation Date:	20.10.2003
	Author:			Roman Margold
*/

#pragma once

#define D3D_DEBUG_INFO

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

// windows headers
#include <windows.h>
#include <atlstr.h>
#include <winuser.h>

// C Run-Time headers
#include <stdio.h>
#include <vector>
#include <list>
#include <map>
#include <math.h>
#include <string>
#include <fstream>
#include <algorithm>


// DX headers
#include <d3d9.h>
#include <d3dx9core.h>
#include <d3dx9math.h>
#include <dxerr9.h>
#include "..\DXUtils\d3dfont.h"

