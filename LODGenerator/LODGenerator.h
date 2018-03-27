/*
	LODGenerator:		Console application that is getting a mesh (3D model) on its input and generates another mesh 
						of the same model but less quality (that is less LOD) as an ouput.
						It gets at least four important parameters that specifies the quality and appearance of the output model:
							- fi - maximal face angle - specifies a maximal angle between face normals that signs an egde between two faces as "border"
							- plane distance - maximal distance of a vertex to its "average plane" to be deleted from mesh
							- edge distance - maximal distance of a vertex on border edge from the new edge to be deleted from mesh
							- optimal criterion to create edge when making hole triangulation
						Enough information is written below...
	Creation Date:		30.3.2004
	Last Update:		1.4.2004
	Author:				Roman Margold


	Program parameters:
		-a#		maximal face angle
		-p#		maximal plane distance (before the vertex is cutted out)
		-e#		maximal edfe distance (before the vertex is cutted out)
		-c#		optimal criterion used by triangulation
		-fNAME	input file name
		-FNAME	output file name
		-v		specifies that the user want to perform mesh validation, in that case, no other actions are done
*/

#pragma once


#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "Model.h"



#define LODG_MAJOR_VERSION		0
#define LODG_MINOR_VERSION		9
#define LODG_VERSION_DESCRIPTOR	"prealpha"

// default values
#define DEFAULT_MAX_FACE_ANGLE			30 // in degrees
#define DEFAULT_MAX_PLANE_DISTANCE		1.5f
#define DEFAULT_MAX_EDGE_DISTANCE		1.5f
#define DEFAULT_TRIANGULATION_CRITERION	0.1f
#define DEFAULT_INPUT_FILE_NAME			"mesh.x"
#define DEFAULT_OUTPUT_FILE_NAME		"newmesh.x"


// program parameters structure
struct SETTINGS
{
	CString				fiName; // input file name
	CString				foName; // output file name
	UINT				maxFaceAngle; // minimal face angle 
	float				maxPlaneDistance; // maximal distance of a vertex to the plane to be cutted of
	float				maxEdgeDistance; // maximal distance of a vertex to border edge to be cutted of
	float				triangulationCriterion; // optimal triangulation criterion
	bool				PerformValidation; // specifies whether the user want to perform mesh validation
} ;



// function prototypes
HRESULT			ParseArgs( int argc, _TCHAR* argv[], SETTINGS *sets );
void			LoadDefaults( SETTINGS *sets );
int				_tmain( int argc, _TCHAR* argv[] );
void			PrintHelp();





