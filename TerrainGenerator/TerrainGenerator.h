/*
	TerrainGenerator:	Console application which is generating terrain plates and writing them to file.
	Creation Date:		22.12.2003
	Last Update:		28.4.2006
	Author:				Roman Margold

	Terrain file fomrat:
		1] Header block
		2] Vertex coords block
		3] TexCoords block
		4] Normals block

		1] Header block
			1.line>	SizeX	SizeY
			2.line>	SegCountX	SegCountY
			3.line> Options (specifies, whether the normal and texture data are also stored)
		2] Vertex coords block
			each line contains x, y and z coordinates of one vertex
			the first is the one on [-x/2,-z/2] coords and then follows those with same Z and greater X coord
			there is an empty line after each that sequence and then follows same sequence with greater Z coord
		3] TexCoords block
			same sequences as in Vertex coords, but there is only TU and TV coords on one line
		4] Normals block
			same as Vertex block, but one line specifies a normal vector (3 floats) to single vertex

TODO: Souradnice textury jsou generovany pouze v zavislosti na Xove a Zove souradnici, tedy nezavisi na vysce (Y)
		

Typy ploch:

typ   | plocha
------+-------------------------------------------------------------------------
00    | rovina
01    | vnejsi  roh kopce vysky 1 (diagonala i obe hrany maji profil curved)
02    | vnitrni roh kopce vysky 1 (diagonala i obe hrany maji profil curved)
03-18 | stena kopce
19-34 | vnejsi roh kopce s diagonalou "top". 
35-50 | vnejsi roh kopce s diagonalou "middle". 
51-66 | vnejsi roh kopce s diagonalou "bottom". 
67-82 | vnitrni roh kopce s diagonalou "top". 
83-98 | vnitrni roh kopce s diagonalou "middle".
99-114| vnitrni roh kopce s diagonalou "bottom". 

Poznamky: 
U vnejsiho rohu (outercorner) urcujeme profily hran u X=segsx a Z=segsz (v tomto poradi).
U vnitrniho rohu (innercorner) urcujeme profily hran u X=0 a Z=0.
U steny kopce urcujeme profily hran u X=0 a X=segsx.

Jednotlive dvojice profilu hran lze urcit pomoci SideType((typ-prvni_typ_teto_plochy)/4) a SideType((typ-prvni_typ_teto_plochy)%4). 
Priklad - typ 42:
35<=42<=50, tedy jde o vnejsi roh kopce s diagonalou middle
SideType((42-35)/4) = SideType(1) = middle - tedy u hrany X=segsx je profil "middle" (prostredek kopce)
SideType((42-35)%4) = SideType(3) = curved - tedy u hrany Z=segsz je profil "curved" (hrana jednotkoveho kopce)
*/

/*
Sousedni pole pro normaly:
01 - W,S rovina
02 - N,E - rovina
03 - stena kopce vysky 1 : N,S - rovina
4-8 - top stena : S sikmina, N rovina
9-13 - middle : N,S - sikmina
14-18 - bottom: S - rovina, N - sikmina
19-34 : W,S - sikmina
35-50 : W,S - sikmy
51-66 : W,S - rovny
67-82 : NE - rovny
83-98 : NE - sikmy
99-114 : NE - sikmy
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\Globals.h"
#include "VertexNormals.h"
#include "FileHeader.h"


// include ScriptInterpreter
#ifdef SCRIPTINTERPRETER_EXPORTS
#undef SCRIPTINTERPRETER_EXPORTS
#endif
#include "..\ScriptInterpreter\ScriptInterpreter.h"
#include "..\ScriptInterpreter\Variables.h"
#include "GeneratingFunctions.h"


#define PLATE_TYPES_KNOWN		115
#define DEFAULT_FILE_NAME		"unnamed.terrain"
#define DEFAULT_SIZE_X			TERRAIN_PLATE_WIDTH
#define DEFAULT_SIZE_Z			TERRAIN_PLATE_WIDTH
#define DEFAULT_SEGS_X			8
#define DEFAULT_SEGS_Z			8
#define DEFAULT_LEVEL_HEIGHT	TERRAIN_PLATE_HEIGHT
#define DEFAULT_PLATE_TYPE		0
#define DEFAULT_SAVE_NORMALS	true
#define DEFAULT_SAVE_TCOORDS	true


#define TG_MINOR_VERSION		5
#define TG_MAJOR_VERSION		2
#define TG_VERSION_DESCRIPTOR	"full"


#define SI_MINOR_VERSION_NEEDED	0
#define SI_MAJOR_VERSION_NEEDED	1


// saving options
#define TGOPT_POSITION		0x00000001
#define TGOPT_TEXTCOORD		0x00000002
#define TGOPT_NORMAL		0x00000004


// normal masks:
#define NMASK_X		(D3DXVECTOR3(1.0f, 0.0f, 0.0f))
#define NMASK_Y		(D3DXVECTOR3(0.0f, 1.0f, 0.0f))
#define NMASK_Z		(D3DXVECTOR3(0.0f, 0.0f, 1.0f))



struct VERTEX
{
	float			x, y, z;
	float			tu, tv;
	CVertexNormals	normal;
} ;


struct SETTINGS
{
	CString	fName; // output file name
	CString	batchFName; // name of batch file that contains params to batch run
	int		PlaneType; // type of plate - specifying the function that will be used to generate it
	float	SizeX, SizeZ; // width of terrain's projection to a XZ plane
	float	LevelHeight;
	int		SegCountX, SegCountZ; // count of segments in both directions
	bool	SaveTextureCoords, SaveNormals; // specifies whether to save texture coordinates and normals
} ;


// methods
int _tmain(int argc, _TCHAR* argv[]); // main procedure
HRESULT ParseArgs(int argc, _TCHAR* argv[], SETTINGS *sets); // analyses all arguments obtained from command line
void LoadDefaults(SETTINGS *sets); // restores default settings
void PrintConfig(SETTINGS *sets); // prints the configuration to console
void PrintHelp(); // prints a help information to console
HRESULT CheckParams(SETTINGS *sets); // checks whether the program parameters are valid
HRESULT Init(FILE **f, VERTEX **pBuf, LPCTSTR fName, int segsx, int segsz); // inits vertex block - allocates memory and calls constructors
void DeInit(FILE **f, VERTEX **pBuf); // deinits all structures, close files

HRESULT CALLBACK ExternalCallback(int ParamCount, CString *params[]); // callback function used as a "ExternalFunctionCall" by ScriptInterpreter

HRESULT Generator(SETTINGS *sets); // gets your settings and generates and writes a terrain to a file - this is the complex solution
HRESULT GenerateTerrain( VERTEX *pBuf, int type, float sizex, float sizez, int segsx, int segsz, float LevelHeight ); // generates terrain, returns it in passed buffer
void GenerateNormals( VERTEX *pBuf, int type, float sizex, float sizez, int segsx, int segsz, float LevelHeight ); // add normals to each vertex for every adjacent face (square); this doesnt compute the resultant normal!
void SetBorderNormal( VERTEX *pBuf, int type, float sizex, float sizez, int segsx, int segsz, 
						float LevelHeight, int direct, D3DXVECTOR3 * normal );

HRESULT Write(FILE *f, VERTEX *pBuf, float sizex, float sizez, int segsx, int segsz, DWORD options); // writes a generated terrain to specified file
HRESULT WriteTexCoords(FILE *f, VERTEX *pBuf, int segsx, int segsz); // writes a generated terrain to specified file
HRESULT WriteNormals(FILE *f, VERTEX *pBuf, int segsx, int segsz); // writes a generated terrain to specified file

float GetFunctionValue(int type, int X, int Z, int segsx, int segsz, float LevelHeight); // generates terrain

bool FileExistsCheck( CString fileName ); // performs a check for file existence and asks the user for overwrite permission

bool IsPlanar( int type ); // returns true if all vertices lies in one plane (don't have to be horizontal)
bool IsConvex( int type ); // returns true if at least a part of the function is convex


