/*
	Globals:		declarations of frequently and globally used macros, functions and classes
	Creation Date:	16.10.2003
	Last Update:	7.1.2007
	Author:			Roman Margold
*/

#pragma once

// version info
#define UR_MAJOR_VERSION_NO		1
#define UR_MINOR_VERSION_NO		0
#define UR_VERSION_STRING		"v1.00"


#define D3D_DEBUG_INFO

#include "stdafx.h"


// macros
//#define MY_DEBUG_VERSION
//#define DEBUG_LEVEL				0 // 0 - 4, 4 means all debug msgs, 0 means no msgs; globally, msgs are written when less than or eq. with DEBUGLEVEL


// constants
#define CONFIGFILENAME			"options.cfg"
#define KEYCONFIGFILENAME		"KeyBinding.cfg"


// graphic global macro and constants
#define	ONE_METER				100.0f // specifies a basic unit of coordinate system
#define ONE_CENTIMETER			(ONE_METER * 0.01f)
#define ONE_KILOMETER			(ONE_METER * 1000.0f)

#define METER_2_CENTIMETER(m)	((m)*100.0f)
#define CENTIMETER_2_METER(cm)	((cm)*0.01f)

#define	MAX_CAR_WHEELS			8

#define FIELD_OF_VIEW_Y			((float) D3DX_PI / 4)
#define NEAR_CLIPPING_PLANE		10.0f
#define FAR_CLIPPING_PLANE		30000.0f*ONE_METER
#define TERRAIN_LOD_COUNT		6 // maximal count of LODs for terrain plates
#define MODEL_LOD_COUNT			10 // maximal count of LODs for models

#define TERRAIN_PLATE_HEIGHT_M	2.5f
#define TERRAIN_PLATE_WIDTH_M	10.0f
#define TERRAIN_PLATE_HEIGHT	(TERRAIN_PLATE_HEIGHT_M * ONE_METER) // height of one terrain level
#define TERRAIN_PLATE_WIDTH		(TERRAIN_PLATE_WIDTH_M * ONE_METER) // width of one terrain plate (field)

#define TERRAIN_REAL_LEVELS_COUNT		8  // count of terrain levels 
#define TERRAIN_VISIBLE_LEVELS_COUNT	(TERRAIN_REAL_LEVELS_COUNT + 6) // count of terrain levels that should be checked in visibility testing and so on
																	// this should be some levels above TERRAIN_REAL_LEVELS_COUNT because of higher models
#define DEG90					((float) D3DX_PI / 2) // angle of 90 degrees
#define DEG180					((float) D3DX_PI) // angle of 180 degrees
#define DEG45					((float) D3DX_PI / 4) // angle of 45 degrees
#define DEG270					(-DEG90) // angle of 270 degrees


// directions specified by integer value
#define DIRECTION_N		0
#define DIRECTION_E		1
#define DIRECTION_S		2
#define DIRECTION_W		3

// controls macro
#define CONTROL_MAX_VALUE	99

// max text lengths
#define MAX_MAP_NAME_LENGTH	30

// multiplayer macros
#define MAX_OPPONENTS		7
#define MAX_PLAYERS			8


// globally used macros
#define SAFE_DELETE(p)						{ if (p) { delete (p); (p) = NULL; } }
#define SAFE_RELEASE(p)						{ if (p) { (p)->Release(); (p) = NULL; } }
#define SAFE_DESTROY(p)						{ if (p) { (p)->Destroy(); (p) = NULL; } }
#define SAFE_FREE(p)						{ if (p) { free( p ); (p) = NULL; } }
#define SAFE_DELETE_ARRAY(p)				{ if (p) { delete[] (p); (p) = NULL; } }
#define SAFE_FCLOSE(f)						{ if (f) { fclose( f ); (f) = NULL; } } 
#define SAFE_CLOSE_HANDLE(h)				{ if ( (h) != INVALID_HANDLE_VALUE ) { CloseHandle(h); h = INVALID_HANDLE_VALUE; } }
#define __SAFE_NEW(p,_class,fce,ret)		{ p = new _class; if(!p)__ERRORMSG(ERROUTOFMEMORY,fce,"",ret); }
#define SAFE_NEW(p,_class,fce)				{ p = new _class; if(!p)ERRORMSG(ERROUTOFMEMORY,fce,""); }
#define SAFE_NEW_ARRAY(p,_class,size,fce)	SAFE_NEW(p,_class[size],fce)




#define __SETTLE_BETWEEN(a,b,c)	{ if ((a) < (b)) (a) = (b); if ((a) > (c)) (a) = (c); }
#define SIGNUM(x)				( ( (x) < 0 ) ? -1 : ( (x) > 0 ? 1 : 0 ) )
#define LIES_BETWEEN(x,min,max)			( ( (x) >= (min) ) && ( (x) <= (max) ) )
#define LIES_AROUND(x,number,epsilon)	( ( (x) >= ((number) - (epsilon)) ) && ( (x) <= ((number) + (epsilon)) ) )


// compiler messages
#define __A2S(arg) #arg
#define __A2S1(arg) __A2S(arg)
#define COMPILER_WARN(msg)		__FILE__ "("__A2S1(__LINE__)") : warning : "msg
#define COMPILER_ERR(msg)		__FILE__ "("__A2S1(__LINE__)") : error : "msg


const GUID g_guidApp = {0x1bd4cb3b,0xac38,0x2d9b,{0x55, 0x12, 0xae, 0x69, 0xad, 0x38, 0xa2, 0x76 } };

