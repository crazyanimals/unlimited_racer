#pragma once

#include "stdafx.h"
#include "..\globals\Configuration.h"
#include <vector>
#include <complex>
#include "..\GameMods\ModWorkshop.h"


#define ERR_NOSTART -1
#define ERR_NOWAYPOINT -2

#define EDITOR_NAME "Map editor:"
#define TERRAIN_TYPE_ROVINA 0

#define TERRAIN_TYPE_ROHSTRED	1
#define TERRAIN_TYPE_STENASTRED	2
#define TERRAIN_TYPE_ROHVNITRNISTRED 3

#define TERRAIN_TYPE_ROHVRCH	4
#define TERRAIN_TYPE_STENAVRCH	5
#define TERRAIN_TYPE_ROHVNITRNIVRCH 6

#define TERRAIN_TYPE_ROHPATA	7
#define TERRAIN_TYPE_STENAPATA 8
#define TERRAIN_TYPE_ROHVNITRNIPATA 9

#define TERRAIN_TYPE_ROH1	10
#define TERRAIN_TYPE_STENA1 11
#define TERRAIN_TYPE_ROHVNITRNI1 12

#define MAX_OBJECT_SIZE 10
#define MAX_MAP_SIZE 200
#define MAX_HEIGHT 9

#define MY_RED 0
#define MY_GREEN 1
#define MY_BLUE 2


#define MAX_ZOOM 48
#define MIN_ZOOM 5
#define WAYPOINT_ERR -1000000
#define TRANSPARENT_COLOR 0xc536fa
#define COLOR_STEP 22
#define COLOR_GROUND RGB(0,79,40)
#define COLOR_WAYPOINT RGB(64,64,255)
#define COLOR_PATHEDIT	RGB(0,0,255)
#define COLOR_PATHEDIT_FONT RGB(200,200,255)
#define COLOR_RECTANGLE_SELECT RGB(255,255,70)
#define COLOR_ADD(i) RGB(0,(i)*COLOR_STEP,0)

#define STARTPOS_R 150
#define TEXTURESET_WALL_PATH  "wall\\"
#define TEXTURESET_TERRAIN_PATH "terrain\\"
#define TEXTURE_MAPLOGO_PATH ""

#define HELP_FILE "support\\Limited Editor.html"
#define MAP_EXT gProp.mExts["urmap"]
#define TERRAIN_EXT gProp.mExts["terrain"]
#define TREE_FILE "objects.tree"
#define OBJECT_EXT gProp.mExts["object"]
#define CAR_EXT gProp.mExts["car"]
#define SKYSYSTEM_EXT gProp.mExts["sk"]
#define TEXTURESET_EXT gProp.mExts["textset"]
#define GRASS_EXT gProp.mExts["grass"]
#define DHLP_EXT gProp.mExts["dhlp"]

class globalProperties{



	
	public:
		
		void createInterpolatedImage(COLORREF colors[4],CImage &img);
		
		int pictureViewSizeX;
		int pictureViewSizeY;
		int mapMaxSizeX ;	
		int mapMaxSizeY ;
		
	//	CString sExt[]={ ".map",".terrain",".object",".car",".sk",".textset",".
		

		CMapStringToString mExts;
		CMapStringToString mPaths;

		COLORREF colors[MAX_HEIGHT];
		CImage plates[MAX_HEIGHT][2][2][2][2];
		UINT crTransparent;
		UINT 	numberOfTerrainsPlates;
		std::vector<CString> types; 
		CMODWorkshop	ModWorkshop;
		
		int	loadProp();

	private:
		BYTE getColor(COLORREF col,int barva);
};

extern globalProperties gProp;
