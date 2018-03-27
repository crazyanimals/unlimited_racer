// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__AE78B9E2_A5B8_11D4_A1FB_00500C0076C8__INCLUDED_)
#define AFX_STDAFX_H__AE78B9E2_A5B8_11D4_A1FB_00500C0076C8__INCLUDED_

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers


typedef char dInt8;
typedef unsigned char dUnsigned8;

typedef short dInt16;
typedef unsigned short dUnsigned16;

typedef int dInt32;
typedef unsigned dUnsigned32;
typedef unsigned int dUnsigned32;


#ifdef _MSC_VER
	#include <windows.h>
	#include <crtdbg.h>
	#include <gl/glaux.h>
//	#include <gl/glut.h>
	#include <malloc.h>
#else
	#define _NEWTON_USE_LIB
	#include <Carbon/Carbon.h>
	#include <glut/glux.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "dList.h"
#include "dvector.h"
#include "dmatrix.h"

#include "../../NewtonSDK/Newton.h"


#ifdef _NEWTON_DOUBLE_PRECISION__
	#define glMultMatrix(x) glMultMatrixd(x)
#else
	#define glMultMatrix(x) glMultMatrixf(x)
#endif



#ifdef _MSC_VER
	#pragma warning (disable: 4100) //unreferenced formal parameter
	#pragma warning (disable: 4505) //unreferenced local function has been removed
	#pragma warning (disable: 4201) //nonstandard extension used : nameless struct/union
	#pragma warning (disable: 4127) //conditional expression is constant

#else
	#define _ASSERTE(x) assert(x)
	#define min(a,b) (a < b ? a : b)
	#define max(a,b) (a > b ? a : b)
	
#endif


#endif 
