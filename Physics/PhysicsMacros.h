
#pragma once

#define FRAMES 10
#define VELOCITY_LIMIT ONE_CENTIMETER
#define VELOCITY_LIMIT_ABS 0.1f*ONE_CENTIMETER
#define ANGULAR_LIMIT D3DX_PI/100.0f
#define ANGULAR_LIMIT_ABS 0.05f

#define GRAVITY -10.0f

#define A2S(arg) #arg
#define A2S1(arg) A2S(arg)
#define WARNING_MSG(msg) __FILE__ "("A2S1(__LINE__)") : warning : "msg
#define ERROR_MSG(msg) __FILE__ "("A2S1(__LINE__)") : error : "msg

#define WHEEL_COLLITION_ID	0x100
#define CHASIS_COLLITION_ID	0x050

#define GEARBOX_AUTOMATIC	0
#define GEARBOX_MANUAL		1

#define WORLD_LEFT		0.0f
#define WORLD_FRONT		0.0f
#define WORLD_BOTTOM	0.0f
//#define WORLD_RIGHT	sizeX*TERRAIN_PLATE_WIDTH_M
//#define WORLD_BACK	sizeY*TERRAIN_PLATE_WIDTH_M
#define WORLD_TOP		100.0f

#define TERRAIN_LOD_PHYSICS	3

#define PLATE_SIZE		100
#define UPDATE_STEP		1/60.0f


/*
#define KEY_UPSHIFT		0x00000001
#define KEY_DOWNSHIFT	0x00000002
//#define KEY_CYCLEVIEW	0x00000004
#define KEY_DRIVERVIEW	0x00000008
#define KEY_HANDBRAKE	0x00000010
#define KEY_GAMEPAUSE	0x00000020
#define KEY_RESTART		0x00000040
*/

enum eGameKeysBitmap {
	KEY_UPSHIFT = 1,
	KEY_DOWNSHIFT =		KEY_UPSHIFT << 1,
	KEY_HANDBRAKE =		KEY_UPSHIFT << 2,
	KEY_GAMEPAUSE =		KEY_UPSHIFT << 3,
	KEY_RESTART =		KEY_UPSHIFT << 4,
	KEY_DRIVERVIEW =	KEY_UPSHIFT << 5
};

#define CONTROL_VALUE(key)				(((float)(key))/CONTROL_MAX_VALUE)
#define DEG2RAD(angle)					((angle)*D3DX_PI/180.0f)
#define CLAMP_BETWEEN(value,min,max)	(((value)>(max))?(max):(((value)<(min))?(min):(value)))	// nefunguje

#define FUNGUJICI_CLAMP_BETWEEN(value,min,max)	\
			{							\
			if ((value) > (max))			\
				(value) = (max);			\
			else if ((value) < (min))		\
				(value) = (min);			\
			}

#define CLAMP_TO(value,value1,value2)	(((value)<(((value1)+(value2))*0.5f))?(value1):(value2))
#define AIR_DENSITY		1.29f


#define MAX_GEARS_COUNT			7
#define MAX_CURVE_POINTS_COUNT	8