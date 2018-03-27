#include "LODGenerator.h"


using namespace std;


//////////////////////////////////////////////////////////////////////////////////////////////
// analyses all arguments obtained from command line
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT ParseArgs( int argc, _TCHAR* argv[], SETTINGS *sets )
{
	DWORD	state = 0;
	HRESULT	hr;
	UINT	u;
	float	f;

#define WRONGPARAMS { cout << "ERROR: Invalid program parameters. Run with -h option for help.\n"; return 1; }
	for ( int i = 1; i < argc; i++ )
	{
		CString		str( argv[i] );
		if ( str.GetLength() < 2 ) continue;

		switch ( str[1] )
		{
		case 'f':
			sets->fiName = str.Right( str.GetLength() - 2 );
			break;
		case 'F':
			sets->foName = str.Right( str.GetLength() - 2 );
			break;
		case 'a':
			hr = sscanf( argv[i], "-a%u", &u );
			if ( hr != 1 ) WRONGPARAMS;
			sets->maxFaceAngle = u;
			break;
		case 'p':
			hr = sscanf( argv[i], "-p%f", &f );
			if ( hr != 1 ) WRONGPARAMS;
			sets->maxPlaneDistance = f;
			break;
		case 'e':
			hr = sscanf( argv[i], "-e%f", &f );
			if ( hr != 1 ) WRONGPARAMS;
			sets->maxEdgeDistance = f;
			break;
		case 'c':
			hr = sscanf( argv[i], "-c%f", &f );
			if ( hr != 1 ) WRONGPARAMS;
			sets->triangulationCriterion = f;
			break;
		case 'h':
			PrintHelp();
			return 1;
		case 'v':
			sets->PerformValidation = true;
			return 1;
		}
	}
#undef WRONGPARAMS

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// restores default settings
//////////////////////////////////////////////////////////////////////////////////////////////
void LoadDefaults( SETTINGS *sets )
{
	sets->fiName = DEFAULT_INPUT_FILE_NAME;
	sets->foName = DEFAULT_OUTPUT_FILE_NAME;
	sets->maxEdgeDistance = DEFAULT_MAX_EDGE_DISTANCE;
	sets->maxPlaneDistance = DEFAULT_MAX_PLANE_DISTANCE;
	sets->maxFaceAngle = DEFAULT_MAX_FACE_ANGLE;
	sets->triangulationCriterion = DEFAULT_TRIANGULATION_CRITERION;
	sets->PerformValidation = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// prints a help information to console
//////////////////////////////////////////////////////////////////////////////////////////////
void PrintHelp()
{
	printf( "\n How to use LODGenerator v%d.%d %s\n", LODG_MAJOR_VERSION, LODG_MINOR_VERSION, LODG_VERSION_DESCRIPTOR );
	printf( "=======================================\n" );
	printf( "This program is used to generate less complex model then that one you already have. The final quality can be affected by options and program parameters which are:\n\n" );

	printf( "\t-h\n\t\tCalls up for this help. The program won't run.\n" );

	printf( "\t-a#\n\t\tSpecifies a maximal angle between face normals that \n\t\tsigns an egde between two faces as \"border\"." );
	printf( "\n\t\tThis value is an integer value in interval <1,179>.\n\t\tDefault value is %d degrees.\n", DEFAULT_MAX_FACE_ANGLE );

	printf( "\t-fNAME\n\t\tNAME specifies the name of input file." );
	printf( "\n\t\tDefault value is '%s'.\n", DEFAULT_INPUT_FILE_NAME );

	printf( "\t-FNAME\n\t\tNAME specifies the name of output file." );
	printf( "\n\t\tDefault value is '%s'.\n", DEFAULT_OUTPUT_FILE_NAME );

	printf( "\t-p#\n\t\tMaximal distance of a vertex to its \"average plane\" \n\t\tto be deleted from the mesh." );
	printf( "\n\t\tThis value is 4B floating point positive value." );
	printf( "\n\t\tDefault value is %f.\n", DEFAULT_MAX_PLANE_DISTANCE );

	printf( "\t-e#\n\t\tMaximal distance of a vertex on border edge\n\t\t from the new edge to be deleted from the mesh." );
	printf( "\n\t\tThis value is 4B floating point positive value." );
	printf( "\n\t\tDefault value is %f.\n", DEFAULT_MAX_EDGE_DISTANCE );

	printf( "\t-c#\n\t\tAn optimal value that is used to check triangulation quality." );
	printf( "\n\t\tThis value is 4B floating point value." );
	printf( "\n\t\tDefault value is %f.\n", DEFAULT_TRIANGULATION_CRITERION );

	printf( "\t-v\n\t\tPerforms a test for mesh validation.\nNo other actions are done in that case.\n" );

	printf( "\n" );
}

//////////////////////////////////////////////////////////////////////////////////////////////
// loads the mesh from specified file to a LPD3DXMESH structure
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT LoadMesh( LPCTSTR fileName, CModel *model )
{
	HRESULT	hr;
	
	hr = model->LoadFromXFile( fileName );
	if ( hr ) return hr;

	return ERRNOERROR;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// main function
//////////////////////////////////////////////////////////////////////////////////////////////
int _tmain( int argc, _TCHAR* argv[] )
{
	HRESULT		hr;
	SETTINGS	sets;
	CModel		oldModel, newModel;


	printf( "You are using LOD Generator of version %d.%d %s.\n", LODG_MAJOR_VERSION, LODG_MINOR_VERSION, LODG_VERSION_DESCRIPTOR );

	// parse configuration settings
	LoadDefaults( &sets );
	if ( hr = ParseArgs( argc, argv, &sets ) ) return hr;


	hr = LoadMesh( sets.fiName, &oldModel );
	if ( hr ) 
	{
		cout << "There was an error while reading the input file ('" << sets.fiName << "').\n";	
		return hr;
	}


	if ( sets.PerformValidation )
	{
		if ( oldModel.IsValid() ) cout << "The specified model is valid.\n";
		else cout << "The specified model is invalid!\n";
		goto END;
	}



	oldModel.Reduce(	&newModel,
						sets.maxFaceAngle,
						sets.maxEdgeDistance,
						sets.maxPlaneDistance,
						sets.triangulationCriterion
					);


	hr = oldModel.SaveToXFile( sets.foName );
	if ( hr )
	{
		cout << "The model could not be saved.\n";
		return hr;
	}
	else cout << "Reduced model was successfully saved to file '" << sets.fiName << "' .\n";


END:
	cout << "Press any key to end this program.\n";
	
	_getch();

	return 0;
}