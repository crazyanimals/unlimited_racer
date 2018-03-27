/*
TODO: 1] Batch run - pridat option -b, pri nemz se zada jmeno souboru, ve kterem budou prikazy k davkovemu spusteni. To bude dobre k tomu,
aby se pri zmene nektereho parametru nemusely slozite generovat vsechny tereny znovu.
TODO: 2] Nastaveni normal kraje, nebo jmena souboru prilehlych poli. Jedno ci druhe by melo definovat normaly vrcholu u kraje, ktere tak ovlivni
osvetleni krajnich segmentu, jenz nyni jsou definovany jen samy sebou.
*/

#include "TerrainGenerator.h"


using namespace std;



//////////////////////////////////////////////////////////////////////////////////////////////
//
// analyses all arguments obtained from command line
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT ParseArgs( int argc, _TCHAR* argv[], SETTINGS *sets )
{
	DWORD	state = 0;
	HRESULT	hr;
	UINT	u;
	float	f;
	char	str[512];

#define WRONGPARAMS { cout << "Error: Wrong usage. Run with -h for help.\n"; return 1; }
	for ( int i = 1; i < argc; i++ )
	{
		switch ( argv[i][1] )
		{
		case 'f':
			sets->fName = argv[argc-1];
			break;
		case 'x':
			hr = sscanf( argv[i], "-x%f", &f );
			if ( hr != 1 ) WRONGPARAMS;
			sets->SizeX = f;
			break;
		case 'z':
			hr = sscanf( argv[i], "-z%f", &f );
			if ( hr != 1 ) WRONGPARAMS;
			sets->SizeZ = f;
			break;
		case 'y':
			hr = sscanf( argv[i], "-y%f", &f );
			if ( hr != 1 ) WRONGPARAMS;
			sets->LevelHeight = f;
			break;
		case 'X':
			hr = sscanf( argv[i], "-X%lu", &u );
			if ( hr != 1 ) WRONGPARAMS;
			sets->SegCountX = u;
			break;
		case 'Z':
			hr = sscanf( argv[i], "-Z%lu", &u );
			if ( hr != 1 ) WRONGPARAMS;
			sets->SegCountZ = u;
			break;
		case 't':
			hr = sscanf( argv[i], "-t%lu", &u );
			if ( hr != 1 ) WRONGPARAMS;
			sets->PlaneType = u;
			break;
		case 'N':
			if ( argv[i][0] == '-' ) sets->SaveNormals = false;
			else if ( argv[i][0] == '+' ) sets->SaveNormals = true;
			break;
		case 'T':
			if ( argv[i][0] == '-' ) sets->SaveTextureCoords = false;
			else if ( argv[i][0] == '+' ) sets->SaveTextureCoords = true;
			break;
		case 'b':
			hr = sscanf( argv[i], "-b%s", str );
			if ( hr != 1 ) WRONGPARAMS;
			sets->batchFName = str;
			break;
		case 'h':
		case '?':
			PrintHelp();
			return 1;
		}
	}
#undef WRONGPARAMS

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// checks whether the program parameters are valid
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CheckParams( SETTINGS *sets )
{
#define WRONGPARAM( msg ) { cout << "Error: "; cout << msg; cout << " Run the program with option -h for help.\n"; return 1; } 
	
	if ( sets->PlaneType >= PLATE_TYPES_KNOWN || sets->PlaneType < 0 ) WRONGPARAM( "Wrong terrain type." );
	if ( sets->SegCountX <= 0 || sets->SegCountZ <= 0 ) WRONGPARAM( "Segment count not allowed." );
	if ( sets->SizeX <= 0 || sets->SizeZ <= 0 ) WRONGPARAM( "Wrong field size." );
	if ( sets->LevelHeight == 0 ) WRONGPARAM( "Wrong level height." );

#undef WRONGPARAM

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// prints a help information to console
//
//////////////////////////////////////////////////////////////////////////////////////////////
void PrintHelp()
{
	printf( "\nUsage of TerrainGenerator v%d.%d %s\n", TG_MAJOR_VERSION, TG_MINOR_VERSION, TG_VERSION_DESCRIPTOR );
	printf( "===========================================\n" );
	printf( "Generates geometry of terrain fields. Can also generate normal data and texture coordinates.\n" );
	printf( "Parameters\n" );
	printf( "\t-h / -?\n\t\tCalls this help.\n" );
	printf( "\t-tX\n\t\tDefines field type, that is predeclared function used for terrain geometry." );
	printf( "\n\t\tX must be natural number from interval <1,%lu>.\n\t\tDefault value %lu.\n", PLATE_TYPES_KNOWN, DEFAULT_PLATE_TYPE );
	printf( "\t-f\n\t\tOutput file name will be specified as a parameter." );
	printf( "\n\t\tIn such case, the last parameter MUST BE the output file name." );
	printf( "\n\t\t'%s' by default.\n", DEFAULT_FILE_NAME );
	printf( "\t-bS\n\t\tS specifies the batch file name." );
	printf( "\n\t\tThis file will be interpreted by ScriptInterpreter as a batch." );
	printf( "\n\t\tBatch file is not used by default.\n" );
	printf( "\t+T\n\t\tAlso generates normal data." );
	printf( "\n\t\tOn by default.\n" );
	printf( "\t-T\n\t\tNormal data won't be generated.\n" );
	printf( "\t+N\n\t\tTexture coordinates (u,v) will be generated besides world coordinates (x,y,z)." );
	printf( "\n\t\tOn by default.\n" );
	printf( "\t-N\n\t\tTexture coordinates won't be generated.\n" );
	printf( "\t-xX\n\t\tThe width (X) of the field. X is positive real number." );
	printf( "\n\t\tDefault value is %f.\n", DEFAULT_SIZE_X );
	printf( "\t-zX\n\t\tThe depth (Z) of the field. X is real positive number, the same as width, by common." );
	printf( "\n\t\tDefault value: %f.\n", DEFAULT_SIZE_Z );
	printf( "\t-yX\n\t\tLevel height. X is non-zero real number." );
	printf( "\n\t\tDefault %f.\n", DEFAULT_LEVEL_HEIGHT );
	printf( "\t-XA\n\t\tCount of segments in X axis (width). A must be natural number." );
	printf( "\n\t\t%lu by default.\n", DEFAULT_SEGS_X );
	printf( "\t-ZA\n\t\tCount of segments in Z axis (depth). A must be natural number." );
	printf( "\n\t\t%lu by default.\n", DEFAULT_SEGS_Z );

	printf( "\n" );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// generates terrain
//
//////////////////////////////////////////////////////////////////////////////////////////////
float GetFunctionValue( int type, int X, int Z, int segsx, int segsz, float LevelHeight ) {
    PlaneType pt;
    SideType st1, st2, stDiagonal;
    switch (type) {
        case 0 : return 0; break; // zero plane
        case 1 : return GetTerrainY(outercorner, curved, curved, X, Z, segsx, segsz, LevelHeight, curved); break;
        case 2 : return GetTerrainY(innercorner, curved, curved, X, Z, segsx, segsz, LevelHeight, curved); break;
    }

    type -= 3;
    if (type < 16) // hillside
        return GetTerrainY(hillside, SideType(type/4), SideType(type%4), X, Z, segsx, segsz, LevelHeight);

    type -= 16;
    pt         = PlaneType(type / 48); type %= 48;
    stDiagonal = SideType (type / 16); type %= 16;
    st1        = SideType (type /  4); type %=  4;
    st2        = SideType (type     );

    return GetTerrainY(pt, st1, st2, X, Z, segsx, segsz, LevelHeight, stDiagonal);
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// generates terrain
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT GenerateTerrain(	VERTEX *pBuf, 
							int type, 
							float sizex, float sizez, 
							int segsx, int segsz, 
							float LevelHeight )
{
	float	deltaTU, deltaTV;
	float	deltaX, deltaZ;
	float	X, Y, Z;
	float	U, V;
	
	deltaX = sizex / (float) segsx;
	deltaZ = sizez / (float) segsz;
	deltaTU = 1.0f / (float) segsx;
	deltaTV = 1.0f / (float) segsz;

	Z = -sizez / 2;
	V = 1;
	for ( int i = 0; i <= segsz; i++ )
	{
		X = -sizex / 2;
		U = 0;
		for ( int j = 0; j <= segsx; j++ )
		{
			Y = GetFunctionValue( type, j, i, segsx, segsz, LevelHeight );
			pBuf[i*(segsx + 1) + j].x = X;
			pBuf[i*(segsx + 1) + j].y = Y;
			pBuf[i*(segsx + 1) + j].z = Z;
			pBuf[i*(segsx + 1) + j].tu = U;
			pBuf[i*(segsx + 1) + j].tv = V;
			X += deltaX;
			U += deltaTU;
		}
		V -= deltaTV;
		Z += deltaZ;
	}

	GenerateNormals( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight );

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns true if all vertices lies in one plane (don't have to be horizontal)
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool IsPlanar( int type )
{
	return !type || type == 8; //zero plane or hillside with two "middle" profiles
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns true if at least a part of the function is convex
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool IsConvex( int type ) {
	if (!type) return false; // zero plane
    if (type < 3) return true; // curved corners
    type -= 3;
    if (type < 16) // hill side
//PERFORMANCE COMMENT        return (type/4 == bottom || type/4 == curved || type%4 == bottom || type%4 == curved);
        return false;
    else type -= 16;
    if (type >= 48) return true; // type out of range, < 48 are corners
    if (type / 16 == bottom) return true; type %= 16; // bottom profile on diagonal
//PERFORMANCE COMMENT    if (type / 4  == bottom || type / 4 == curved) return true; type %= 4;
//PERFORMANCE COMMENT    if (type      == bottom || type     == curved) return true; 
    return false;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// writes a generated terrain to specified file
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT WriteTexCoords( FILE *f, VERTEX *pBuf, int segsx, int segsz )
{
	for ( int i = 0; i <= segsz; i++ )
	{
		for ( int j = 0; j <= segsx; j++ )
		{
			D3DXVECTOR2 coord;

			coord.x = pBuf[i*(segsx + 1) + j].tu;
			coord.y = pBuf[i*(segsx + 1) + j].tv;
			fwrite( &coord, sizeof( D3DXVECTOR2 ), 1, f );
		}
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// writes a generated terrain to specified file
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT WriteNormals( FILE *f, VERTEX *pBuf, int segsx, int segsz )
{
	for ( int i = 0; i <= segsz; i++ )
	{
		for ( int j = 0; j <= segsx; j++ )
		{
			D3DXVECTOR3	normal;
			normal = pBuf[i*(segsx + 1) + j].normal.GetNormal(); // compute resultant normal
			fwrite( &normal, sizeof( normal ), 1, f );
		}
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// writes a generated terrain to specified file
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT Write( FILE *f, VERTEX *pBuf, float sizex, float sizez, int segsx, int segsz, DWORD options )
{
	HRESULT			hr = 0;
	TG_FILEHEADER	fh;

	if ( !f || !pBuf ) return 1;

	fh.fSizeX = sizex;
	fh.fSizeZ = sizez;
	fh.iSegsX = segsx;
	fh.iSegsZ = segsz;
	fh.iOptions = options;

	if ( fwrite( &fh, sizeof( fh ), 1, f ) != 1 )
		return 1;
	
	if ( !options ) return 0;

	for ( int i = 0; i <= segsz; i++ )
	{
		for ( int j = 0; j <= segsx; j++ )
		{
			fwrite( &D3DXVECTOR3( pBuf[i*(segsx + 1) + j].x, pBuf[i*(segsx + 1) + j].y, pBuf[i*(segsx + 1) + j].z ),
				sizeof( D3DXVECTOR3 ), 1, f );
		}
	}

	if ( options & 2 ) hr = WriteTexCoords( f, pBuf, segsx, segsz );
	if ( hr ) return hr;
	if ( options & 4 ) hr = WriteNormals( f, pBuf, segsx, segsz );
	if ( hr ) return hr;

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// add normals to each vertex for every adjacent face (square)
// this doesnt compute the resultant normal!
// specify terrain type for generating appropriate border normals
//
//////////////////////////////////////////////////////////////////////////////////////////////
void GenerateNormals(	VERTEX *pBuf, 
						int type, 
						float sizex, float sizez, 
						int segsx, int segsz, 
						float LevelHeight )
{
	D3DXVECTOR3	norm1, norm2; // face normals
	D3DXVECTOR3	norm2w; // face normals
	D3DXVECTOR3 v1, v2, v3, v4; // vertices of one square in clocwise order, v1 is left-bottom
	int			ind1, ind2, ind3, ind4;
	D3DXVECTOR3	vec1, vec2;

	
	for ( int i = 0; i < segsz; i++ )
	{
		for ( int j = 0; j < segsx; j++ )
		{
			// compute indices
			ind1 = i * (segsx + 1) + j;
			ind2 = (i + 1) * (segsx + 1) + j;
			ind3 = ind2 + 1;
			ind4 = ind1 + 1;

			// pick up vertex coordinates
			v1 = D3DXVECTOR3( pBuf[ind1].x, pBuf[ind1].y, pBuf[ind1].z );
			v2 = D3DXVECTOR3( pBuf[ind2].x, pBuf[ind2].y, pBuf[ind2].z );
			v3 = D3DXVECTOR3( pBuf[ind3].x, pBuf[ind3].y, pBuf[ind3].z );
			v4 = D3DXVECTOR3( pBuf[ind4].x, pBuf[ind4].y, pBuf[ind4].z );

			// make up face normals
			vec1 = v1 - v2;
			vec2 = v3 - v2;
			D3DXVec3Cross( &norm1, &vec2, &vec1 );

			vec1 = v3 - v4;
			vec2 = v1 - v4;
			D3DXVec3Cross( &norm2, &vec2, &vec1 );

			// add computed normals to vertices of both faces
			pBuf[ind1].normal.AddNormal( &norm1 );
			pBuf[ind2].normal.AddNormal( &norm1 );
			pBuf[ind2].normal.AddNormal( &norm1 );
			pBuf[ind3].normal.AddNormal( &norm1 );

			pBuf[ind3].normal.AddNormal( &norm2 );
			pBuf[ind4].normal.AddNormal( &norm2 );
			pBuf[ind4].normal.AddNormal( &norm2 );
			pBuf[ind1].normal.AddNormal( &norm2 );
		}
	}


	// GENERATE BORDER NORMALS
	
	/*
		Sousedni pole pro normaly:
		01 - W,S rovina
		02 - NE - rovina

3,6,15 - R S
4,5,7-12,16 SS
13-14,17 SR
18 RR
		3-4,7 - N rovina, S sikmy
		5-6,10-11,14-18 - N,S rovny
		8 - N,S sikmy
		9,12-13 - N sikmy, S rovny
		19-34 : W,S - sikmina
		35-50 : W,S - sikmy
		51-66 : W,S - rovny
		67-82 : NE - rovny
		83-114 : NE - sikmy
	*/
	norm1 = D3DXVECTOR3( 0, 1, 0 ); // plane plate
	D3DXVec3Normalize( &norm2, &D3DXVECTOR3( 0, sizez, -LevelHeight ) ); // inclined plate
	D3DXVec3Normalize( &norm2w, &D3DXVECTOR3( -LevelHeight, sizez, 0 ) ); // inclined plate

	if ( type == 1 ) { 
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_W, &norm1 );
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_S, &norm1 );
	}
	if ( type == 2 ) { 
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_N, &norm1 );
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_E, &norm1 );
	}
	if ( type == 3 || type == 6 || type == 15 ) { 
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_N, &norm1 );
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_S, &norm2 );
	}
	if ( type == 4 || type == 5 || (type >= 7 && type <= 12) || type == 16 ) { 
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_N, &norm2 );
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_S, &norm2 );
	}
	if ( type == 13 || type == 14 || type == 17 ) { 
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_N, &norm2 );
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_S, &norm1 );
	}
	if ( type == 18 ) { 
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_N, &norm1 );
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_S, &norm1 );
	}
	if ( type >= 19 && type <= 34 ) { 
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_W, &norm2w );
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_S, &norm2 );
	}
	if ( type >= 35 && type <= 50 ) { 
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_W, &norm2w );
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_S, &norm2 );
	}
	if ( type >= 51 && type <= 66 ) { 
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_W, &norm1 );
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_S, &norm1 );
	}
	if ( type >= 67 && type <= 82 ) { 
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_N, &norm1 );
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_E, &norm1 );
	}
	if ( type >= 83 && type <= 114 ) { 
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_N, &norm2 );
		SetBorderNormal( pBuf, type, sizex, sizez, segsx, segsz, LevelHeight, DIRECTION_E, &norm2w );
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// changes the normal of border vertices
//
// 'direct' parameter specifies which side should be changed (north, east, west or south)
// 'normal' specifies the target normal vector to set
// 'pBuf' is a valid pointer to 2D vertex array
// 'segsx' and 'segsz' define dimension of the array
//
//////////////////////////////////////////////////////////////////////////////////////////////
void SetBorderNormal( 	VERTEX *pBuf, 
						int type, 
						float sizex, float sizez, 
						int segsx, int segsz, 
						float LevelHeight,
						int direct,
						D3DXVECTOR3 * normal )
{
	int multi, count, base;

	switch ( direct % 4 )
	{
	case DIRECTION_N: 
		count = segsx;
		base = segsz * (segsx + 1);
		multi = 1;
		break;
	case DIRECTION_S:
		count = segsx;
		base = 0;
		multi = 1;
		break;
	case DIRECTION_E: 
		count = segsz;
		base = segsx;
		multi = segsx + 1;
		break;
	case DIRECTION_W:
		count = segsz;
		base = 0;
		multi = segsx + 1;
		break;
	}
	
	
	for ( int i = 0; i <= count; i++ )
	{
		pBuf[base + i * multi].normal.SetNormal( normal );
	}

};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// inits vertex block - allocates memory and calls constructors
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT Init( FILE **f, VERTEX **pBuf, LPCTSTR fName, int segsx, int segsz )
{
	VERTEX *	vertices = *pBuf;
	
	// create vertex field
	vertices = (VERTEX *) malloc( sizeof(VERTEX) * ( segsx + 1 ) * ( segsz + 1 ) );
	if ( !vertices )
	{
		cout << "Error : Not enough memory.\n";
		return 3;
	}

	
	// initialize normals objects
	for ( int i = 0; i <= segsz; i++ )
		for ( int j = 0; j <= segsx; j++ )
		{
			vertices[i * (segsx + 1) + j].normal = CVertexNormals();
		}

	
	// check for file existence
	if ( FileExistsCheck( fName ) )
	{
		*f = NULL;
		return -1;
	}


	*f = fopen( fName, "wb" );
	if ( !(*f) )
	{
		cout << "Error : Can't create file.\n";
		free( vertices );
		return 2;
	}

	*pBuf = vertices;

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// performs a check for file existence and asks the user for overwrite permission
// returns true if the file exists and can't be overwritten, otherwise returns false
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool FileExistsCheck( CString fileName )
{
	FILE *	f;
	char	znak;
	HRESULT	hr;

	// open specified file
	hr = fopen_s( &f, fileName, "rt" );
	if ( !hr )
	{
		fclose( f );
		cout << "File '";
		cout << (LPCTSTR) fileName ;
		cout << "' exists. Do you want to overwrite? (Y/N)" ;
		znak = toupper( _getch() );
		printf( "%c\n", znak );
		if ( znak != 'Y' ) return true;
	}

	return false;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// deinits all structures, close files
//
//////////////////////////////////////////////////////////////////////////////////////////////
void DeInit( FILE **f, VERTEX **pBuf )
{
	if ( *f ) fclose( *f );
	if ( *pBuf ) SAFE_FREE( *pBuf );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// prints the configuration to console
//
//////////////////////////////////////////////////////////////////////////////////////////////
void PrintConfig( SETTINGS *sets )
{
	printf( "File: %s\n", sets->fName );
	printf( "\tTerrain type: %lu\n\tLevel height: %f\n", sets->PlaneType, sets->LevelHeight );
	printf( "\tSize X,Z: %f, %f\n\tSegments count X,Z: %lu, %lu\n",
			sets->SizeX,
			sets->SizeZ,
			sets->SegCountX,
			sets->SegCountZ );
	printf( "\tSaved data: vertices" );
	if ( sets->SaveNormals ) printf( ", normals" );
	if ( sets->SaveTextureCoords ) printf( ", texture coordinates." );
	printf( ".\n" );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// restores default settings
//
//////////////////////////////////////////////////////////////////////////////////////////////
void LoadDefaults( SETTINGS *sets )
{
	sets->batchFName = "";
	sets->fName = DEFAULT_FILE_NAME;
	sets->LevelHeight = DEFAULT_LEVEL_HEIGHT;
	sets->PlaneType = DEFAULT_PLATE_TYPE;
	sets->SaveNormals = DEFAULT_SAVE_NORMALS;
	sets->SaveTextureCoords = DEFAULT_SAVE_TCOORDS;
	sets->SegCountX = DEFAULT_SEGS_X;
	sets->SegCountZ = DEFAULT_SEGS_Z;
	sets->SizeX = DEFAULT_SIZE_X;
	sets->SizeZ = DEFAULT_SIZE_Z;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// gets your settings and generates and writes a terrain to a file
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT Generator( SETTINGS *sets )
{
	FILE *		fOut = NULL; // output file
	DWORD		Options = 1;
	HRESULT		hr;
	VERTEX *	vertices = NULL;

	
	if ( hr = CheckParams( sets ) ) return hr;
	if ( sets->SaveTextureCoords ) Options += 2;
	if ( sets->SaveNormals ) Options += 4;
	PrintConfig( sets );

	
	// allocates memory and opens a file
	if ( (hr = Init( &fOut, &vertices, sets->fName, sets->SegCountX, sets->SegCountZ )) > 0 ) return hr;
	if ( hr == -1 )
	{
		cout << "Terrain hasn't been saved. Original file has been kept.\n";
		DeInit( &fOut, &vertices );
		return 0;
	}
	
	
	// generate terrain
	GenerateTerrain( vertices, sets->PlaneType, sets->SizeX, sets->SizeZ, sets->SegCountX, sets->SegCountZ, sets->LevelHeight );

	
	// write terrain to file
	if ( Write( fOut, vertices, sets->SizeX, sets->SizeZ, sets->SegCountX, sets->SegCountZ, Options ) )
	{
		cout << "An error occured during data write.\n";
		DeInit( &fOut, &vertices );
		return 1;
	}

	
	// close program
	DeInit( &fOut, &vertices );

	cout << "Terrain successfully generated and saved to file '";
	cout << LPCTSTR(sets->fName);
	cout << "'.\n";

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function used as a "ExternalFunctionCall" by ScriptInterpreter
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CALLBACK ExternalCallback( CString &retVal, int ParamCount, CString params[] )
{
	CVariable			var( "a" );
	static bool			firstRun = true;			
	static SETTINGS		sets;
	static FILE *		fOut = NULL; // output file
	HRESULT				hr;
	static VERTEX *		vertices = NULL;


	retVal = "0";

	// load default if this method is runnig first time
	if ( firstRun )
	{
		LoadDefaults( &sets );
		firstRun = false;
	}

	if ( params[0] == "settings" )
	{
		if ( ParamCount < 9 ) return ERR_NOTENOUGHPARAMETERS;
		if ( ParamCount > 9 ) return ERR_TOOMANYPARAMETERS;

		sets.fName = params[1];
		
		var.SetValue( params[2] );
		sets.PlaneType = var.GetInt();

		var.SetValue( params[3] );
		sets.LevelHeight = var.GetFloat();

		var.SetValue( params[4] );
		sets.SegCountX = var.GetInt();

		var.SetValue( params[5] );
		sets.SegCountZ = var.GetInt();

		var.SetValue( params[6] );
		sets.SizeX = var.GetFloat();

		var.SetValue( params[7] );
		sets.SizeZ = var.GetFloat();

		var.SetValue( params[8] );
		sets.SaveNormals = var.GetInt() != 0;

		var.SetValue( params[9] );
		sets.SaveTextureCoords = var.GetInt() != 0;
	}
	
	
	// apply self-generation
	if ( params[0] == "generate" ) retVal = Generator( &sets ) ? "1" : "0";

	
	// various settings commands
	if ( params[0] == "setFName" || params[0] == "setType" || params[0] == "setHeight" 
		|| params[0] == "setSegsX" || params[0] == "setSegsZ" || params[0] == "setSizeX" 
		|| params[0] == "setSizeZ" || params[0] == "setNormals" || params[0] == "setTextureCoords" )
	{
	
		if ( ParamCount < 1 ) return ERR_NOTENOUGHPARAMETERS;
		if ( ParamCount > 1 ) return ERR_TOOMANYPARAMETERS;

		var.SetValue( params[1] );

		if ( params[0] == "setFName" )			sets.fName = var.GetString();
		if ( params[0] == "setType" )			sets.PlaneType = var.GetInt();
		if ( params[0] == "setHeight" )			sets.LevelHeight = var.GetFloat();
		if ( params[0] == "setSegsX" )			sets.SegCountX = var.GetInt();
		if ( params[0] == "setSegsZ" )			sets.SegCountZ = var.GetInt();
		if ( params[0] == "setSizeX" )			sets.SizeX = var.GetFloat();
		if ( params[0] == "setSizeZ" )			sets.SizeZ = var.GetFloat();
		if ( params[0] == "setNormals" )		sets.SaveNormals = var.GetInt() != 0;
		if ( params[0] == "setTextureCoords" )	sets.SaveTextureCoords = var.GetInt() != 0;
	}

	
	// create a new directory
	if ( params[0] == "mkdir" )
	{
		if ( ParamCount < 1 ) return ERR_NOTENOUGHPARAMETERS;
		if ( ParamCount > 1 ) return ERR_TOOMANYPARAMETERS;

		if ( _mkdir( params[1] ) )
		{
			if ( errno == EEXIST ) cout << "Directory already exists!\n";
			else
			{
				cout << "Can't create directory.\n";
				retVal = "1";
				return 1;
			}
		}
	}


	// create a new surface definition file
	// is called with parameters: filename, type, pathToPlates, lodcount
	// default name for plates is used: #.surface
	if ( params[0] == "createSurfaceDef" )
	{ 
		int		count, lod, type;
		FILE *	f;
		CString	str;
		
		if ( ParamCount < 4 ) return ERR_NOTENOUGHPARAMETERS;
		if ( ParamCount > 4 ) return ERR_TOOMANYPARAMETERS;

		if ( FileExistsCheck( params[1] ) )
		{
			cout << "Surface definition hasn't been saved.\n";
			retVal = "1";
			return 1;
		}

		hr = fopen_s( &f, params[1], "wt" );
		if ( hr )
		{
			cout << "Can't create surface definition file.\n";
			retVal = "1";
			return 1;
		}

		var.SetValue( params[4] );
		count = var.GetInt(); 
		fprintf_s( f, "%d\n", count );
		var.SetValue( params[2] );
		type = var.GetInt();
		
		lod = 1;
		for ( int i = 0; i < count; i++ )
		{
			str = params[3] + "\\";
			var.SetValue( lod );
			str += var.GetString() + ".surface";
			fprintf_s( f, "%s\n", str );
			lod *= 2;
		}

		fprintf_s( f, "%d\n%d\n", IsPlanar( type ) ? 1 : 0, IsConvex( type ) ? 0 : 1 );
		fclose( f );

		cout << "Surface definition saved into file '" << params[1] << "'.\n";
	}

	

	// write terrain to file
	if ( params[0] == "write" )
	{
		DWORD	Options = 1;
		if ( sets.SaveTextureCoords ) Options += 2;
		if ( sets.SaveNormals ) Options += 4;
		
		GenerateNormals(vertices, sets.PlaneType, sets.SizeX, sets.SizeZ, sets.SegCountX, sets.SegCountZ, sets.LevelHeight );
		if ( Write( fOut, vertices, sets.SizeX, sets.SizeZ, sets.SegCountX, sets.SegCountZ, Options ) )
		{
			cout << "An error occured during data write.\n";
			DeInit( &fOut, &vertices );
			retVal = "1";
			return 1;
		}
	}


	// allocates memory and opens a file
	if ( params[0] == "init" )
	{
		if ( hr = CheckParams( &sets ) )
		{
			retVal = "1";
			return hr;
		}
		PrintConfig( &sets );
		if ( hr ) retVal = "1";
		if ( (hr = Init( &fOut, &vertices, sets.fName, sets.SegCountX, sets.SegCountZ )) > 0 ) return hr;
	}

	
	// close the file and deinit structures
	if ( params[0] == "release" )
	{
		if ( !fOut )
		{
			cout << "Terrain wasn't saved.\n";
			retVal = "1";
		}
		else
		{
			cout << "Terrain successfully generated and saved to file '" << LPCTSTR(sets.fName);
			cout << "'.\n";
		}
        DeInit( &fOut, &vertices );
	}


	// giving the computed Y value for selected point
	if ( params[0] == "computeY" )
	{
		if ( ParamCount < 3 ) return ERR_NOTENOUGHPARAMETERS;
		if ( ParamCount > 3 ) return ERR_TOOMANYPARAMETERS;
		
		float	X, Y, Z;
		float	U, V;
		int		i, j, index;
	
		var.SetValue( params[1] );
		j = var.GetInt();
		var.SetValue( params[2] );
		i = var.GetInt();
		var.SetValue( params[3] );
		Y = var.GetFloat();
		
		X = (float) j * (sets.SizeX / (float) sets.SegCountX) - (sets.SizeX / 2);
		Z = (float) i * (sets.SizeZ / (float) sets.SegCountZ) - (sets.SizeZ / 2);
		U = (float) j / (float) sets.SegCountX;
		V = (float) (1 - i) / (float) sets.SegCountZ;

		index = i * ( sets.SegCountX + 1 ) + j; 
		vertices[index].x = X;
		vertices[index].y = Y;
		vertices[index].z = Z;
		vertices[index].tu = U;
		vertices[index].tv = V;
	}


	return ERR_NOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads the ScriptInterpreter dll and runs the script as a batch to generate terrain(s)
//
// gets the file name of the batch to interpret
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT BatchGeneration( LPCTSTR batchFileName )
{
	HRESULT				hr;
	int					minVer, majVer;
	CString				verDesc, retValue, errDes;
	UINT				lineNo;


	// check the DLL version
	GetDLLVersion( majVer, minVer, verDesc );

	printf( "You are using TerrainGenerator version %d.%d %s.\n", TG_MAJOR_VERSION, TG_MINOR_VERSION, TG_VERSION_DESCRIPTOR );

	printf( "ScriptInterpreter version %d.%d %s loaded.\n", majVer, minVer, (LPCTSTR)verDesc );
	
	if ( SI_MAJOR_VERSION_NEEDED > majVer || SI_MAJOR_VERSION_NEEDED == majVer && SI_MINOR_VERSION_NEEDED > minVer )
	{
		printf( "Wrong version of ScriptInterpreteru. %d.%d needed at least.\n",	SI_MAJOR_VERSION_NEEDED, SI_MINOR_VERSION_NEEDED );
		return 4;
	}

	cout << "Going to use a script from file '" << (LPCTSTR) batchFileName;
	cout << "' as a batch.\n";


	// interpret script
	hr = InterpretScript( batchFileName, ExternalCallback, retValue );
	switch ( hr )
	{
	case ERR_FILENOTFOUND:
		printf( "Can't open script file.\n" );
		return 5;
	case ERR_WRONGINTERPRETVERSION:
		printf( "Script file needs never version of ScriptInterpreteru.\n" );
		return 6;
	}

		
	// check for possible error
	if ( CInterpretingError::PickError( hr, errDes, lineNo ) )
		printf( "An error occured during the script interpretation on line %d: %s\n", lineNo, (LPCTSTR) errDes );
		
	//printf( "Returned value: %s\n", (LPCTSTR) retValue );

	return ERR_NOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// main procedure
//
//////////////////////////////////////////////////////////////////////////////////////////////
int _tmain( int argc, _TCHAR* argv[] )
{
	HRESULT				hr;
	SETTINGS			sets;


	// parse configuration settings
	LoadDefaults( &sets );
	if ( hr = ParseArgs( argc, argv, &sets ) ) return hr;
	
	
	// the batch will be applied
	if ( sets.batchFName != "" )
	{
		if ( hr = BatchGeneration( sets.batchFName ) ) return hr;
	}
	else // standard generating (without script) used
	{
		if ( hr = Generator( &sets ) ) return hr;
	}


	//cout << "Zmackni libovolnou klavesu pro ukonceni programu.\n";
	
	//_getch();

	return 0;
}

