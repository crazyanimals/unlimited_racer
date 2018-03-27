#include "stdafx.h"
#include "GrObjectTerrain.h"

using namespace graphic;

const WORD			CGrObjectTerrain::MAPBB_FVF = D3DFVF_XYZ;		// FV format of map's bounding box
const WORD			CGrObjectTerrain::MAPBBTRANS_FVF = D3DFVF_XYZRHW;		// FV format of map's transformed bounding box
const D3DXMATRIX	CGrObjectTerrain::IdentityMatrix = D3DXMATRIX( 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f ); // identity matrix for free use :o)


//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrObjectTerrain::CGrObjectTerrain()
{
	// call inherited constructor

	// sets all properties to default values
	PreInitThis();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrObjectTerrain::~CGrObjectTerrain()
{
	Free();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// private method called by constructor
// this initializes all internal structures to default values
// !!! this doesn't call inherited PreInit() method
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrain::PreInitThis()
{
	pFields = NULL;
	pVisibilityBitField = NULL;
	pLeftBorder= NULL;
	pRightBorder=NULL;
	pLeftBorderBackup=NULL;
	pRightBorderBackup=NULL;
	pBorderIBuffer = NULL;
	pBorderVBuffer = NULL;
	pBorderTexture = NULL;
	ridTerrain = -1;

	ZeroMemory(	&StandardMaterial, sizeof( StandardMaterial ) );
	StandardMaterial.Diffuse.r = 1;
	StandardMaterial.Diffuse.g = 1;
	StandardMaterial.Diffuse.b = 1;
	StandardMaterial.Diffuse.a = 1;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this initializes all internal structures to default values
// !!! also calls PreInit() method of all ancestors
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrain::PreInit()
{
	// calls inherited PreInit() method
	this->CGrObjectBase::PreInit();

	// init own data
	PreInitThis();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// releases all structures created by Init() or other calls
// this is called by destructor, but could be used separately for this object's reuse
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrain::Free()
{
	if ( ridTerrain >= 0 ) ResourceManager->ReleaseResource( ridTerrain );
	
	//if ( pFields )
	//	for ( DWORD i = 0; i < SegsCountZ; i++ )
	//		for ( DWORD j = 0; j < SegsCountX; j++ ) 
	//			SAFE_DELETE( pFields[i * SegsCountX + j].TerrainPlate )

	SAFE_DELETE_ARRAY( pFields );
	SAFE_FREE( pLeftBorder );
	SAFE_FREE( pRightBorder );
	SAFE_FREE( pLeftBorderBackup );
	SAFE_FREE( pRightBorderBackup );
	SAFE_FREE( pVisibilityBitField );
	SAFE_RELEASE( pBorderIBuffer );
	SAFE_RELEASE( pBorderVBuffer );

	// call inherited dealocator
	this->CGrObjectBase::Free();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads a terrain from file
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectTerrain::Init(	BOOL Static,
								CString fileName,
								UINT LODcount )
{
	HRESULT					hr;
	resManNS::RESOURCEID				resID;
	resManNS::__Terrain			*	resTerrain;
	resManNS::__TerrainPlateInfo	*	resPlateInfo;
	float					posX, posZ;
	UINT					index;


	// use inherited initializator
	hr = this->CGrObjectBase::Init( Static );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGrObjectTerrain::Init()", "An error occurs while initializing global parts of the object." );

	// sets all properties to default values
	PreInitThis();


	// load terrain resource
	this->uiLODCount = LODcount;
	resID = ResourceManager->LoadResource( fileName, RES_Terrain, &LODcount );
	if ( resID < 1 ) ERRORMSG( ERRGENERIC, "CGrObjectTerrain::Init()", "Could not load terrain object." );

	resTerrain = ResourceManager->GetTerrain( resID );
	if ( !resTerrain ) ERRORMSG( ERRNOTFOUND, "CGrObjectTerrain::Init()", "Could not obtain terrain object." );
	ridTerrain = resID;

	
	// we must copy all important data from terrain resource stored in ResourceManager to more practical structures

	// allocate memory for all fields
	pFields = new TERRAINFIELD[resTerrain->uiSizeX * resTerrain->uiSizeY];
	//pFields = (LPTERRAINFIELD) malloc( sizeof(TERRAINFIELD) * resTerrain->uiSizeX * resTerrain->uiSizeY );
	if ( !pFields )	ERRORMSG( ERROUTOFMEMORY, "CGrObjectTerrain::Init()", "Could not allocate memory for terrain." );
	

	posZ = ( resTerrain->uiSizeY - 0.5f ) * TERRAIN_PLATE_WIDTH ;

	// compute precise positions and fill fields structures
	for ( int iz = (int) resTerrain->uiSizeY - 1; iz >= 0; iz-- )
	{
		posX = TERRAIN_PLATE_WIDTH / 2;

		for ( UINT ix = 0; ix < resTerrain->uiSizeX; ix++ )
		{
			index = resTerrain->uiSizeX * iz + ix;

			resPlateInfo = &resTerrain->pTerrainPlateInfo[index];

			//pFields[index].TerrainPlate = new CGrObjectTerrainPlate; // create new TerrainPlate object
			//if ( !pFields[index].TerrainPlate )
			//{
			//	SAFE_FREE( pFields );
			//	ERRORMSG( ERROUTOFMEMORY, "CGrObjectTerrain::Init()", "Could not allocate memory for terrain." );
			//}
			
			// init TerrainPlate object
			hr = pFields[index].TerrainPlate.Init( Static, "", LODcount );
			if ( hr ) 
			{
				SAFE_FREE( pFields );
				ERRORMSG( ERRGENERIC, "CGrObjectTerrain::Init()", "Could not initialise terrain plate." );
			}

			pFields[index].TextureID = resTerrain->pTextureIDs[resPlateInfo->uiTextureIndex];
			pFields[index].Height = resPlateInfo->uiHeight;
			pFields[index].Rotation = resPlateInfo->uiPlateRotation;
			pFields[index].TextureRotation = resPlateInfo->uiTextureRotation;

			// set properties for the plate
			pFields[index].TerrainPlate.SetPlate( resTerrain->pGrPlateIDs[resPlateInfo->uiGrPlateIndex] );
			pFields[index].TerrainPlate.SetTexture( pFields[index].TextureID );
			pFields[index].TerrainPlate.InitialWorldPlacement( D3DXVECTOR3(	posX,
																				(float) pFields[index].Height * TERRAIN_PLATE_HEIGHT,
																				posZ ),
																D3DXVECTOR3(	0,
																				(float) pFields[index].Rotation * DEG90,
																				0 ),
																D3DXVECTOR3( 1, 1, 1 ) );
			pFields[index].TerrainPlate.SetTextureRotation( (float) pFields[index].TextureRotation * DEG90 );
			posX += TERRAIN_PLATE_WIDTH;
		}

		posZ -= TERRAIN_PLATE_WIDTH;
	}
	
	
	// store terrain size
	SegsCountX = resTerrain->uiSizeX;
	SegsCountZ = resTerrain->uiSizeY;

	
	// init auxiliary structures
	pLeftBorder = (int *) malloc( sizeof( int ) * SegsCountZ );
	pRightBorder = (int *) malloc( sizeof( int ) * SegsCountZ );
	pLeftBorderBackup = (int *) malloc( sizeof( int ) * SegsCountZ );
	pRightBorderBackup = (int *) malloc( sizeof( int ) * SegsCountZ );

	pVisibilityBitField = (char *) malloc( ( (SegsCountX * SegsCountZ) / 8 ) + 1 );

	if ( !pLeftBorder || !pRightBorder || !pLeftBorderBackup || !pRightBorderBackup || !pVisibilityBitField ) 
		ERRORMSG( ERROUTOFMEMORY, "CGrObjectTerrain::Init()", "Unsuccessful attampt to create auxiliary data structures." );

	for ( int i = 0; i < (int) SegsCountZ; i++ )
	{
		pLeftBorderBackup[i] = 100000;
		pRightBorderBackup[i] = -100000;
	}


	
	// set visibility parameters to every field
	for ( UINT i = 0, j = 0, index = 0; i < SegsCountX * SegsCountZ; i++, j++ )
	{
		if ( j == 8 )
		{
			index++;
			j = 0;
		}
		pFields[i].TerrainPlate.SetVisibilityParams( pVisibilityBitField + index, 128 >> j );
	}



	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns world coordinates of a vertex placed in given map position
// specifiy platex and platez as map coordinates of the terrain field and use index to specify
//   which terrain plate corner do you want to obtain:
// 1 - 2
// |    |
// 0 - 3
//
// have no return value other than the output vector parameter
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrain::GetBorderVertex( int platex, int platez, int index, D3DXVECTOR3 * pos )
{
	resManNS::__GrSurface * PlateData; // terrain plate - rendering data
	int					ind;
	CGrObjectTerrainPlate::VERTEX_DX * pVertexBuffer; // pointer to all plate vertices
	WORD			*	pIndexBuffer; // pointer to all plate indices
	

	ind = (4 + index - pFields[platez * SegsCountX + platex].Rotation) % 4;
	if ( ind == 3 ) ind = 4;

	PlateData = ResourceManager->GetGrSurface( GetPlate( platex, platez )->GetLODResourceID( 0 ) );
		
	PlateData->pVertices->Lock( 0, 0, (void **) &pVertexBuffer, 0 );
	PlateData->pIndices->Lock( 0, 0, (void **) &pIndexBuffer, 0 );
				
	D3DXVec3TransformCoord( pos, &pVertexBuffer[pIndexBuffer[ind]].pos, &GetPlate( platex, platez )->GetWorldMat() );

	PlateData->pVertices->Unlock();
	PlateData->pIndices->Unlock();

};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// generates the geometry for terrain borders; must not be called before Init()
// on success returns 0, otherwise returns nonzero error code 
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectTerrain::InitBorders()
{
	HRESULT					hr;
	WORD				*	pIndices;
	BORDERVERTEX		*	pVertices;
	resManNS::__Terrain	*	resTerrain; // terrain plate - rendering data
	resManNS::__TextureSet *resTextSet;
	resManNS::__Texture	*	resTexture;
	int						index;



	// INIT STRUCTURES
	uiBorderVertCount = 3 * 2 * ( SegsCountX + SegsCountZ ) + 3;
	uiBorderIndCount = 2 * ( 2 + 4 * ( SegsCountX + SegsCountZ ) );

	resTerrain = ResourceManager->GetTerrain( ridTerrain );
	if ( !resTerrain ) ERRORMSG( ERRGENERIC, "CGrObjectTerrain::InitBorders()", "Unable to obtain terrain resource." );
	resTextSet = ResourceManager->GetTextureSet( resTerrain->ridWallTextureSet );
	if ( !resTextSet ) ERRORMSG( ERRGENERIC, "CGrObjectTerrain::InitBorders()", "Resource missing." );
	resTexture = ResourceManager->GetTexture( resTextSet->pTextureIDs[0] );
	if ( !resTexture ) ERRORMSG( ERRGENERIC, "CGrObjectTerrain::InitBorders()", "Resource missing." );
	pBorderTexture = resTexture->texture;

	
	// create vertex buffer
	hr = D3DDevice->CreateVertexBuffer( uiBorderVertCount * sizeof( BORDERVERTEX ), 0, 
										BorderVertexFVF, D3DPOOL_MANAGED, &pBorderVBuffer, NULL );
	if ( hr ) ERRORMSG( hr, "CGrObjectTerrain::InitBorders()", "Unable to create terrain border vertex buffer." );

	// create index buffer
	hr = D3DDevice->CreateIndexBuffer( uiBorderIndCount * sizeof( *pIndices ), 0,
										D3DFMT_INDEX16, D3DPOOL_MANAGED, &pBorderIBuffer, NULL );
	if ( hr ) ERRORMSG( hr, "CGrObjectTerrain::InitBorders()", "Unable to create terrain border index buffer." );
	
	
	// lock both buffers
	hr = pBorderVBuffer->Lock( 0, 0, (void **) &pVertices, 0 );
	if ( hr ) ERRORMSG( hr, "CGrObjectTerrain::InitBorders()", "Unable to lock terrain border vertex buffer." );

	hr = pBorderIBuffer->Lock( 0, 0, (void **) &pIndices, 0 );
	if ( hr ) ERRORMSG( hr, "CGrObjectTerrain::InitBorders()", "Unable to lock terrain border index buffer." );




	// FILL VERTEX BUFFER WITH GEOMETRY DATA
	index = 0;

	// process distant horizontal line
	for ( UINT x = 0; x < SegsCountX; x++ )
	{
		GetBorderVertex( x, SegsCountZ - 1, 1, &pVertices[index].Pos );
		pVertices[index+2] = pVertices[index+1] = pVertices[index];
		pVertices[index  ].Pos.y -= 30 * ONE_CENTIMETER;
		pVertices[index+1].Pos.y += BORDER_WALL_HEIGHT;
		pVertices[index+2].Pos.y += BORDER_WALL_HEIGHT;
		pVertices[index+2].Pos.z += BORDER_WALL_THICKNESS;

		pVertices[index  ].Normal = D3DXVECTOR3( 0, 1, -1 );
		pVertices[index+1].Normal = D3DXVECTOR3( 0, 1, -1 );
		pVertices[index+2].Normal = D3DXVECTOR3( 0, 1, 0 );
		pVertices[index  ].TexCoord = D3DXVECTOR2( (float) index / 2, 1 );
		pVertices[index+1].TexCoord = D3DXVECTOR2( (float) index / 2, 0.1f );
		pVertices[index+2].TexCoord = D3DXVECTOR2( (float) index / 2, 0 );
		index += 3;
	}
	
	pVertices[2].Pos.x -= BORDER_WALL_THICKNESS;


	// process right vertical line
	for ( UINT z = SegsCountZ; z > 0; z-- )
	{
		GetBorderVertex( SegsCountX - 1, z - 1, 2, &pVertices[index].Pos );
		pVertices[index+2] = pVertices[index+1] = pVertices[index];
		pVertices[index  ].Pos.y -= 30 * ONE_CENTIMETER;
		pVertices[index+1].Pos.y += BORDER_WALL_HEIGHT;
		pVertices[index+2].Pos.y += BORDER_WALL_HEIGHT;
		pVertices[index+2].Pos.x += BORDER_WALL_THICKNESS;

		pVertices[index  ].Normal = D3DXVECTOR3( -1, 1, 0 );
		pVertices[index+1].Normal = D3DXVECTOR3( -1, 1, 0 );
		pVertices[index+2].Normal = D3DXVECTOR3( 0, 1, 0 );
		pVertices[index  ].TexCoord = D3DXVECTOR2( (float) index / 2, 1 );
		pVertices[index+1].TexCoord = D3DXVECTOR2( (float) index / 2, 0.1f );
		pVertices[index+2].TexCoord = D3DXVECTOR2( (float) index / 2, 0 );
		index += 3;
	}
	
	pVertices[SegsCountX * 3 + 2].Pos.z += BORDER_WALL_THICKNESS;


	// process near horizontal line
	for ( UINT x = SegsCountX; x > 0; x-- )
	{
		GetBorderVertex( x - 1, 0, 3, &pVertices[index].Pos );
		pVertices[index+2] = pVertices[index+1] = pVertices[index];
		pVertices[index  ].Pos.y -= 30 * ONE_CENTIMETER;
		pVertices[index+1].Pos.y += BORDER_WALL_HEIGHT;
		pVertices[index+2].Pos.y += BORDER_WALL_HEIGHT;
		pVertices[index+2].Pos.z -= BORDER_WALL_THICKNESS;

		pVertices[index  ].Normal = D3DXVECTOR3( 0, 1, 1 );
		pVertices[index+1].Normal = D3DXVECTOR3( 0, 1, 1 );
		pVertices[index+2].Normal = D3DXVECTOR3( 0, 1, 0 );
		pVertices[index  ].TexCoord = D3DXVECTOR2( (float) index / 2, 1 );
		pVertices[index+1].TexCoord = D3DXVECTOR2( (float) index / 2, 0.1f );
		pVertices[index+2].TexCoord = D3DXVECTOR2( (float) index / 2, 0 );
		index += 3;
	}
	
	pVertices[(SegsCountX + SegsCountZ) * 3 + 2].Pos.x += BORDER_WALL_THICKNESS;


	// process left vertical line
	for ( UINT z = 0; z < SegsCountZ; z++ )
	{
		GetBorderVertex( 0, z, 0, &pVertices[index].Pos );
		pVertices[index+2] = pVertices[index+1] = pVertices[index];
		pVertices[index  ].Pos.y -= 30 * ONE_CENTIMETER;
		pVertices[index+1].Pos.y += BORDER_WALL_HEIGHT;
		pVertices[index+2].Pos.y += BORDER_WALL_HEIGHT;
		pVertices[index+2].Pos.x -= BORDER_WALL_THICKNESS;

		pVertices[index  ].Normal = D3DXVECTOR3( 1, 1, 0 );
		pVertices[index+1].Normal = D3DXVECTOR3( 1, 1, 0 );
		pVertices[index+2].Normal = D3DXVECTOR3( 0, 1, 0 );
		pVertices[index  ].TexCoord = D3DXVECTOR2( (float) index / 2, 1 );
		pVertices[index+1].TexCoord = D3DXVECTOR2( (float) index / 2, 0.1f );
		pVertices[index+2].TexCoord = D3DXVECTOR2( (float) index / 2, 0 );
		index += 3;
	}

	pVertices[(2 * SegsCountX + SegsCountZ) * 3 + 2].Pos.z -= BORDER_WALL_THICKNESS;

	// the last quad has different texture coordinates
	for ( int i = 0; i < 3; i++ ) pVertices[index+i] = pVertices[i];
	pVertices[index  ].TexCoord = D3DXVECTOR2( (float) index / 2, 1 );
	pVertices[index+1].TexCoord = D3DXVECTOR2( (float) index / 2, 0.1f );
	pVertices[index+2].TexCoord = D3DXVECTOR2( (float) index / 2, 0 );




	// FILL INDEX BUFFER WITH PROPER INDICES

	// wall itself
	index = 0;
	for ( UINT i = 0; i <= 2 * (SegsCountX + SegsCountZ); i++ )
	{
		pIndices[index++] = 3 * i;
		pIndices[index++] = 3 * i + 1;
	}


	// ledge
	for ( UINT i = 0; i <= 2 * (SegsCountX + SegsCountZ); i++ )
	{
		pIndices[index++] = 3 * i + 1;
		pIndices[index++] = 3 * i + 2;
	}



	pBorderVBuffer->Unlock();
	pBorderIBuffer->Unlock();


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// says whether the specified point in screen space is visible, depends on the viewPort settings
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGrObjectTerrain::PointIsInVisibleScreen( FLOAT x, FLOAT y, D3DVIEWPORT9 *viewPort )
{
	return ( (x >= viewPort->X) && (x < viewPort->X + viewPort->Width) && 
			(y >= viewPort->Y) && (y < viewPort->Y + viewPort->Height) );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// help sorting function
//
//////////////////////////////////////////////////////////////////////////////////////////////
int SortByYCoord( const void * v1, const void * v2 )
{
	if ( ((RAY *) v1)->intersectPoint.y < ((RAY *) v2)->intersectPoint.y ) return -1;
	else if ( ((RAY *) v1)->intersectPoint.y > ((RAY *) v2)->intersectPoint.y ) return 1;
	else return 0;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// makes all fields lying on line between ray1 and ray2 visible
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrain::MakeLineVisible( RAY * ray1, RAY * ray2 )
{
#define GETFIELDINDEX(x)		( (int) (x / TERRAIN_PLATE_WIDTH) )
#define UPDATEBORDERS(y,x)		{ if ( pLeftBorder[y] > (x) ) pLeftBorder[y] = (x); if ( pRightBorder[y] < (x) ) pRightBorder[y] = (x); }

	FLOAT		dx, dy, diff;
	FLOAT		y1, y2, x1, x2;
	FLOAT		x0, y, x;
	RAY		*	pomRay;

	
	// rearrange rays so that ray1.y < ray2.y
	if ( ray1->iPosZ > ray2->iPosZ )
	{
		pomRay = ray1;
		ray1 = ray2;
		ray2 = pomRay;
	}

	x1 = ray1->intersectPoint.x;
	x2 = ray2->intersectPoint.x;
	y1 = ray1->intersectPoint.z;
	y2 = ray2->intersectPoint.z;
	
	dx = x2 - x1;
	dy = y2 - y1;


	// solve pathological cases
	if ( (ray1->iPosZ - ray2->iPosZ) == 0 ) // y2 ~ y1
	{
		if ( (ray1->iPosX - ray2->iPosX) == 0 ) // x2 ~ x1
		{
			if ( ( ray1->iPosZ >= 0 ) && ( ray1->iPosZ < (int) SegsCountZ ) ) UPDATEBORDERS( ray1->iPosZ, ray1->iPosX );
			return;
		}
		if ( ( ray1->iPosZ >= 0 ) && ( ray1->iPosZ < (int) SegsCountZ ) ) 
		{
			UPDATEBORDERS( ray1->iPosZ, ray1->iPosX );
			UPDATEBORDERS( ray1->iPosZ, ray2->iPosX );
		}
		return;
	}


	// equation of linear function will be used: Ax = (dx/dy) * Ay - x0  (A is a 2D point)
	// where (dx / dy) is a differential:
	diff = dx / dy;
	x0 = diff * y1 - x1; // precompute x0 on the base of knowledge x1 and y1

	for ( int i = ray1->iPosZ + 1; i < ray2->iPosZ; )
	{
		if ( i < 0 ) { i = 0; continue; }
		if ( i >= (int) SegsCountZ ) break;
		
		y = (FLOAT) i * TERRAIN_PLATE_WIDTH;
		x = diff * y - x0;
		UPDATEBORDERS( i, GETFIELDINDEX(x) );

		y = (FLOAT) ( i + 1 ) * TERRAIN_PLATE_WIDTH;
		x = diff * y - x0;
		UPDATEBORDERS( i, GETFIELDINDEX(x) );

		i++;
	}

	// line end points weren't processed, process them separately:
	if ( ray1->iPosZ >= 0 && ray1->iPosZ < (int) SegsCountZ )
	{
		y = (FLOAT) (ray1->iPosZ + 1) * TERRAIN_PLATE_WIDTH;
		x = diff * y - x0;
		UPDATEBORDERS( ray1->iPosZ, GETFIELDINDEX(x) );
		UPDATEBORDERS( ray1->iPosZ, GETFIELDINDEX(x1) );
	}
	if ( ray2->iPosZ >= 0 && ray2->iPosZ < (int) SegsCountZ )
	{
		y = (FLOAT) ray2->iPosZ * TERRAIN_PLATE_WIDTH;
		x = diff * y - x0;
		UPDATEBORDERS( ray2->iPosZ, GETFIELDINDEX(x) );
		UPDATEBORDERS( ray2->iPosZ, GETFIELDINDEX(x2) );
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// takes three vertices from rays field and fills the triangle of the map fields they 
// determines with Visible value
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrain::MakeTriangleVisible( RAY * rays )
{
	MakeLineVisible( &rays[0], &rays[1] );
	MakeLineVisible( &rays[1], &rays[2] );
	MakeLineVisible( &rays[2], &rays[0] );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// gets a general plane and finds the intersection line of this plane and a horizontal plane
// of specified y coordinate
// point1 and point2 are output vectors that specifies two points lying on the intersection line
//
// function returns zero when the intersection line exists (not necessary)
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectTerrain::PlaneSurfaceIntersection(	D3DXVECTOR3 * point1,
													D3DXVECTOR3 * point2,
													D3DXPLANE * plane,
													FLOAT y )
{
	FLOAT		e;

	// plane equation:   ax + by + cz + d = 0
	// equation of an intersection line of this plane and specific horizontal plane:
	//   ax + cz + e = 0 ,  where   e = by + d   is constant
	// we have to choose two points of this line, so for the simplicity take points
	//    P1 = [x1,y,0]  and  P2 = [0,y,z2]	  (y is still constant)
	// they can be computed from the line equation:   a*x1 + c*0 + e = 0,  or better:
	//    x1 = -e / a = -(by + d) / a  ,   and
	//    z2 = -e / c = -(by + d) / c  .

	if ( abs( plane->a ) < 0.0001 || abs( plane->c ) < 0.0001 ) return ERRINVALIDPARAMETER;
	
	point1->y = point2->y = y;
	point1->z = 0;
	point2->x = 0;

	e = - ( plane->b * y + plane->d );

	point1->x = - e / plane->a;
	point2->z = - e / plane->c;

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// tests the horizontal ray starting at point1 and directing to point2 (but not ending there)
//  for intersection with map borders, but not with whole lines, just the proper line segment.
// So it should finally find 0, 1 or 2 intersection points.
// If one intersection point is found, a point that is two times farther from the point1 
//  is returned as intersector param.
// If two points are found, the farther one is choosen, but a pointe that is two times
//  farther from point1 is returned as intersector.
// If no intersection point is found, intersector parameter will be filled by coordinates of 
//  point that lies on the ray but is 10 times farther from point1 than point2. This is because
//  we have to be sure to cover whole map even if borders are untouched (not likely to happen).
//
// The return value is a count of intersection points found.
//
//////////////////////////////////////////////////////////////////////////////////////////////
int CGrObjectTerrain::RayIntersectionWithMapBorders(	D3DXVECTOR3 * intersector,
														D3DXVECTOR3 * point1,
														D3DXVECTOR3 * point2 )
{
	FLOAT		a, b, c;
	FLOAT		maxx, maxz;
	int			iInterCount;
	D3DXVECTOR3	inters[2], vecs[2];
	FLOAT		dX1, dZ1, dX2, dZ2;


	// express the ray by line equation:   ax + bz + c = 0  (y is missing here because we are in plane)
	a = point2->z - point1->z;
	b = point1->x - point2->x;
	c = - ( point1->x * a + point1->z * b );
	
	// four map borders, test them one by one;
	// but first, init and precompute some values
	iInterCount = 0;
	maxx = SegsCountX * TERRAIN_PLATE_WIDTH;
	maxz = SegsCountZ * TERRAIN_PLATE_WIDTH;
	inters[0].y = inters[1].y = point1->y;
	dX1 = point2->x - point1->x;
	dZ1 = point2->z - point1->z;

	if ( abs( a ) > 0.001 ) // prevent division by zero
	{
		// test for intersection with nearer transverse border
		inters[iInterCount].x = - c / a;
		if ( inters[iInterCount].x >= 0 && inters[iInterCount].x <= maxx ) // line intersects the map border segment
		{
			inters[iInterCount].z = 0;
			dX2 = inters[iInterCount].x - point1->x;
			dZ2 = inters[iInterCount].z - point1->z;
			// test for proper ray orientation
			if ( ( SIGNUM(dX1) == SIGNUM(dX2) ) && ( SIGNUM(dZ1) == SIGNUM(dZ2) ) ) iInterCount++;
		}

		// test for intersection with farther transverse border
		inters[iInterCount].x = - ( b * maxz + c ) / a;
		if ( inters[iInterCount].x >= 0 && inters[iInterCount].x <= maxx ) // line intersects the map border segment
		{
			inters[iInterCount].z = maxz;
			dX2 = inters[iInterCount].x - point1->x;
			dZ2 = inters[iInterCount].z - point1->z;
			// test for proper ray orientation
			if ( ( SIGNUM(dX1) == SIGNUM(dX2) ) && ( SIGNUM(dZ1) == SIGNUM(dZ2) ) ) iInterCount++;
		}
	}

	if ( iInterCount >= 2 ) goto HopHereAndOut; // two intersect points already found, there couldn't be any more

	if ( abs( b ) > 0.001 ) // test for zero deletion
	{
		// test for intersection with left lengthwise border
		inters[iInterCount].z = - c / b;
		if ( inters[iInterCount].z >= 0 && inters[iInterCount].z <= maxz ) // line intersects the map border segment
		{
			inters[iInterCount].x = 0;
			dX2 = inters[iInterCount].x - point1->x;
			dZ2 = inters[iInterCount].z - point1->z;
			// test for proper ray orientation
			if ( ( SIGNUM(dX1) == SIGNUM(dX2) ) && ( SIGNUM(dZ1) == SIGNUM(dZ2) ) ) iInterCount++;
		}

		if ( iInterCount >= 2 ) goto HopHereAndOut; // two intersect points already found, there couldn't be any more
		
		// test for intersection with farther transverse border
		inters[iInterCount].z = - ( a * maxx + c ) / b;
		if ( inters[iInterCount].z >= 0 && inters[iInterCount].z <= maxz ) // line intersects the map border segment
		{
			inters[iInterCount].x = maxx;
			dX2 = inters[iInterCount].x - point1->x;
			dZ2 = inters[iInterCount].z - point1->z;
			// test for proper ray orientation
			if ( ( SIGNUM(dX1) == SIGNUM(dX2) ) && ( SIGNUM(dZ1) == SIGNUM(dZ2) ) ) iInterCount++;
		}
	}

	
HopHereAndOut: 
	// if no intersection points were found
	if ( !iInterCount )
	{
		*intersector = 10 * (*point2 - *point1) + *point1;
	}

	if ( iInterCount == 1 )
	{
		*intersector = 2 * (inters[0] - *point1) + *point1;
	}

	if ( iInterCount == 2 )
	{
		vecs[0] = inters[0] - *point1;
		vecs[1] = inters[1] - *point1;
		if ( D3DXVec3Length( &vecs[0] ) < D3DXVec3Length( &vecs[1] ) ) *intersector = 2 * (inters[1] - *point1) + *point1;
		else *intersector = 2 * (inters[0] - *point1) + *point1;
	}
	

	return iInterCount;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this method is used when some ray didn't intercept the surface
// In such case a new ray is created that divides the angle between some proper ray and the
//  wrong one. If this new ray intercepts the surface plane it is returned as proper ray,
//  if this new one is still not proper, the same process is applied (half angle) and this
//  runs as long as needed. Stops when a proper ray intercepting the surface plane is found.
//  Note that such ray has to be found since there is a small neighbourhood of the proper
//  ray where any ray should be proper. We have just find this neighbourhood by "binary search".
//
// There is no return value since it "should" work and return a proper ray in all cases :)
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrain::CreateAdditionalIntersectionPoint(	RAY * rayOk,
															RAY * rayNotOk,
															D3DXPLANE * plane,
															D3DXVECTOR3 * cameraPos )
{
	D3DXVECTOR3		diff = D3DXVECTOR3( 1.0f, 1.0f, 1.0f );

	// in some situations, the floating point format is not precise enough and the computation never ends
	//   this advanced condition should stop it after some time
	while ( !rayNotOk->bCrossed && ( ABS(diff.x) >= TVC_MIN_DIFFERENCE || ABS(diff.y) >= TVC_MIN_DIFFERENCE 
		|| ABS(diff.z) >= TVC_MIN_DIFFERENCE) )
	{
		diff = (rayNotOk->coord - rayOk->coord) / 2;
		rayNotOk->coord = diff + rayOk->coord;
		if ( D3DXPlaneIntersectLine( &rayNotOk->intersectPoint, plane, cameraPos, &rayNotOk->coord ) )
		{
			// an intersection point was found, but we have to discover whether it's in view frustum
			//   which means whether it is on the right side of the ray (not behind camera) 
			if ( SIGNUM( cameraPos->y - rayNotOk->intersectPoint.y ) != SIGNUM( cameraPos->y - rayNotOk->coord.y ) )
				rayNotOk->iDirection = 1;   // I----C----P - wrong direction - intersection point behind camera
			else if ( SIGNUM( cameraPos->y - rayNotOk->coord.y ) ) rayNotOk->iDirection = -1; // C---P---I - properly oriented ray
			else rayNotOk->iDirection = 0; // C---P---I, but parallel with surface plane
			
			if ( rayNotOk->iDirection == -1 ) rayNotOk->bCrossed = true; // only rays with proper orientation are used
			else rayNotOk->bCrossed = false;
		}
		else rayNotOk->bCrossed = false;
	}

	RayIntersectionWithMapBorders( &rayNotOk->intersectPoint, &rayOk->intersectPoint, &rayNotOk->intersectPoint );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
UINT CGrObjectTerrain::TerrainVisibilityForSinglePlane(	RAY * rays,
														D3DXMATRIX * mat,
														D3DXVECTOR3 * cameraPos,
														FLOAT planeY )
{
	D3DXPLANE			plane; // horizontal map plane
	UINT				uiCrossedCount;


	D3DXPlaneFromPointNormal( &plane, &D3DXVECTOR3( 0, planeY, 0 ), &D3DXVECTOR3( 0, 1, 0 ) );


	// compute the intersection points of rays with the plane
	uiCrossedCount = 0;
	for ( int i = 0; i < 4; i++ ) 
	{
		if ( D3DXPlaneIntersectLine( &rays[i].intersectPoint, &plane, cameraPos, &rays[i].coord ) )
		{
			// an intersection point was found, but we have to discover whether it's in view frustum
			//   which means whether it is on the right side of the ray (not behind camera) 
			if ( SIGNUM( cameraPos->y - rays[i].intersectPoint.y ) != SIGNUM( cameraPos->y - rays[i].coord.y ) )
				rays[i].iDirection = 1;  // I----C----P - wrong direction - intersection point behind camera
			else if ( SIGNUM( cameraPos->y - rays[i].coord.y ) ) rays[i].iDirection = -1; // C---P---I - properly oriented ray
			else rays[i].iDirection = 0; // C---P---I, but parallel with surface plane
			
			if ( rays[i].iDirection == -1 ) rays[i].bCrossed = true; // only rays with proper orientation are used
			else rays[i].bCrossed = false;
		}
		else rays[i].bCrossed = false;

		if ( rays[i].bCrossed ) uiCrossedCount++;
		rays[i].bOriginal = rays[i].bCrossed;
	}



	// TODO: udelat specielni osetreni pripadu s chybejicimi 3 vrcholy
	switch ( uiCrossedCount )
	{
	case 0: return uiCrossedCount; // get out when no ray crossed the plane
	case 1: return uiCrossedCount; 
	case 2:
		// the higher limit in the following cycle should work since modulo is used
		for ( int i = 0; i < 4; i++ ) 
		{
			// this one is good and the next is bad, so find a proper one!
			if ( rays[i%4].bOriginal && !rays[(i+1)%4].bOriginal ) CreateAdditionalIntersectionPoint( &rays[i%4], &rays[(i+1)%4], &plane, cameraPos );
			if ( rays[(i+1)%4].bOriginal && !rays[i%4].bOriginal ) CreateAdditionalIntersectionPoint( &rays[(i+1)%4], &rays[i%4], &plane, cameraPos );
		}
		break;
	case 3:
		for ( int i = 0; i < 4; i++ ) 
		{
			// this one is good and the next is bad, so find a proper one!
			if ( rays[i%4].bOriginal && !rays[(i+1)%4].bOriginal ) CreateAdditionalIntersectionPoint( &rays[i%4], &rays[(i+1)%4], &plane, cameraPos );
		}
		break;
	case 4: break;
	}



	// process those four found interception points to determine terrain visibility
	if ( uiCrossedCount != 1 )
	{
		for ( int i = 0; i < 4; i++ )
		{
			rays[i].iPosX = rays[i].intersectPoint.x >= 0 ? (int) ( rays[i].intersectPoint.x / TERRAIN_PLATE_WIDTH ) : (int) ( rays[i].intersectPoint.x / TERRAIN_PLATE_WIDTH ) - 1;
			rays[i].iPosZ = rays[i].intersectPoint.z >= 0 ? (int) ( rays[i].intersectPoint.z / TERRAIN_PLATE_WIDTH ) : (int) ( rays[i].intersectPoint.z / TERRAIN_PLATE_WIDTH ) - 1;
		}

		rays[4] = rays[0];

		MakeTriangleVisible( rays ); // processes the 0-1-2 triangle
		MakeTriangleVisible( &rays[2] ); // processes the 2-3-0 triangle
	}



	return uiCrossedCount;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// determines the terrain fields visibility
// for transformation uses actual VIEW and PROJECTION matrices and identity matrix as WORLD matrix
// you must specify view frustum parameters (Field of View and near and far clipping planes)
// and camera parameters
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectTerrain::ComputeTerrainVisibility(	D3DVIEWPORT9 *viewPort,
													D3DXMATRIX * viewMatrix,
													FLOAT fovX,
													FLOAT fovY,
													FLOAT ZNear,
													FLOAT ZFar )
{
	RAY					rays[5*TERRAIN_VISIBLE_LEVELS_COUNT];
	D3DXMATRIX			mat; // inverse of view matrix
	FLOAT				width, height; // width and height of the view rectangle on near clipping plane
	D3DXVECTOR3			cameraPos;
	UINT				index;



	SetWholeMapInvisible();


	// set the view frustum vertice coordinates in camera space
	width = 2 * tanf( fovX / 2 ) * ZNear;
	height = 2 * tanf( fovY / 2 ) * ZNear;

    rays[0].cameraPos = D3DXVECTOR3( -width/2, -height/2, ZNear );
	rays[1].cameraPos = D3DXVECTOR3(  width/2, -height/2, ZNear );
	rays[2].cameraPos = D3DXVECTOR3(  width/2,  height/2, ZNear );
	rays[3].cameraPos = D3DXVECTOR3( -width/2,  height/2, ZNear );
	

	cameraPos = D3DXVECTOR3( 0, 0, 0 );


	// compute inverse of the view matrix
	D3DXMatrixInverse( &mat, NULL, viewMatrix );

	// place frustum points from camera space into world space
	D3DXVec3TransformCoord( &cameraPos, &cameraPos, &mat );
	for ( int i = 0; i < 4; i++ ) D3DXVec3TransformCoord( &rays[i].coord, &rays[i].cameraPos, &mat );

	if ( cameraPos.y < 0 ) return ERRNOERROR; // no field is visible when camera is under surface


	for ( int i = 1; i < TERRAIN_VISIBLE_LEVELS_COUNT; i++ )
	{
        rays[5*i] = rays[0];
		rays[5*i+1] = rays[1];
		rays[5*i+2] = rays[2];
		rays[5*i+3] = rays[3];
	}

	

	// reset borders auxiliary structures; memcpy should be faster then new refilling every time
	memcpy( pLeftBorder, pLeftBorderBackup, sizeof( int ) * SegsCountZ );
	memcpy( pRightBorder, pRightBorderBackup, sizeof( int ) * SegsCountZ );



	// find the intersectors with bottom surface plane
	// TerrainVisibilityForSinglePlane( rays, &mat, &cameraPos, 0 );
	for ( int i = 0; i < TERRAIN_VISIBLE_LEVELS_COUNT; i++ ) 
		TerrainVisibilityForSinglePlane( &rays[5*i], &mat, &cameraPos, i * TERRAIN_PLATE_HEIGHT );

	

	// set those visible fields to BE visible
	for ( int i = 0; i < (int) SegsCountZ; i++ )
	{
		index = i * SegsCountX;
		if ( pLeftBorder[i] < 0 ) pLeftBorder[i] = 0;
		if ( pRightBorder[i] >= (int) SegsCountX ) pRightBorder[i] = SegsCountX - 1;
		if ( pLeftBorder[i] >= (int) SegsCountX || pRightBorder[i] < 0 ) continue;
		for ( int j = pLeftBorder[i]; j <= pRightBorder[i]; j++ )
		{
			int a = 2;
			pFields[index + j].TerrainPlate.SetVisible();
			a = 3;
		}
	}



	// store found intersectors for later use (some shadow mapping or whatever? who knows... Fil knows :))
	for ( int i = 0; i < 4; i++ ) IntersectionPoints[i] = rays[i].intersectPoint;

	
	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders the terrain - all fields - and SkyBox if the RenderSkyBox property is set
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectTerrain::Render()
{
	for ( DWORD i = 0; i < SegsCountZ; i++ )
	{
		for ( DWORD j = 0; j < SegsCountX; j++ )
		{
			pFields[i*SegsCountX + j].TerrainPlate.Render();
		}
	}
	
	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders terrain borders (walls)
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectTerrain::RenderBorders()
{
	HRESULT hr;

	// place an object to scene
	Shader->SetWorldMatrix( &IdentityMatrix );
	Shader->SetVSMaterial( &StandardMaterial );
	D3DDevice->SetTexture( 1, pBorderTexture );
	D3DDevice->SetStreamSource( 0, pBorderVBuffer, 0, sizeof( BORDERVERTEX ) );
	D3DDevice->SetFVF( BorderVertexFVF );
	D3DDevice->SetIndices( pBorderIBuffer );

	hr = D3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, uiBorderVertCount, 
											0, 2 * (uiBorderVertCount - 3) / 3 );
	hr = D3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, uiBorderVertCount,
									2 + 2 * (uiBorderVertCount - 3) / 3, 2 * (uiBorderVertCount - 3) / 3 );

	return hr;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders the specified terrain field
// x and z are the field's coordinates in terrain coord. system
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectTerrain::RenderField( int x, int z, UINT LOD, bool secondPass )
{
	if ( x < 0 || x >= (int) SegsCountX || z < 0 || z >= (int) SegsCountZ ) return ERRNOERROR;
	
	//if ( LOD >= TERRAIN_LOD_COUNT ) LOD = TERRAIN_LOD_COUNT - 1;
	
	if ( !pFields[z*SegsCountX + x].TerrainPlate.IsVisible() ) return ERRNOERROR;

	pFields[z*SegsCountX + x].TerrainPlate.SetLOD( LOD );
	
	pFields[z*SegsCountX + x].TerrainPlate.Render( secondPass );

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders the terrain fields specified by rectangular section (including borders)
// Left-Top = [x1,z1], Right-Bottom = [x2,z2]. 
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectTerrain::RenderSection( int x1, int z1, int x2, int z2 )
{
	int		_x1, _x2, _z1, _z2;

	_x1 = min( x1, x2 );
	_x2 = max( x1, x2 );
	_z1 = min( z1, z2 );
	_z2 = max( z1, z2 );

	if ( _x2 < 0 || _x1 >= (int) SegsCountX || _z2 < 0 || _z1 >=(int)  SegsCountZ ) return ERRNOERROR;

	__SETTLE_BETWEEN( x1, 0, (int) SegsCountX - 1 )
	__SETTLE_BETWEEN( x2, 0, (int) SegsCountX - 1 )
	__SETTLE_BETWEEN( z1, 0, (int) SegsCountZ - 1 )
	__SETTLE_BETWEEN( z2, 0, (int) SegsCountZ - 1 )


	for ( int i = _z1; i <= _z2; i++ )
	{
		for ( int j = _x1; j <= _x2; j++ )
		{
			pFields[i*SegsCountX + j].TerrainPlate.Render();
		}
	}
	
	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders the terrain fields inside a rectangular section that is specified by its
// center field and the Range, that is count of fields to render to each direction
// Range 0 means to render only single fields, whereas 1 means to render a 3x3 section
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectTerrain::RenderSection( int x, int z, int range )
{
	for ( int i = z - range; i <= z + range; i++ )
	{
		if ( i - range < 0 || i + range >= (int) SegsCountZ ) continue;
		for ( int j = x - range; j <= x + range; j++ )
		{
			if ( j - range < 0 || j + range >= (int) SegsCountX  ) continue;
			pFields[i*SegsCountX + j].TerrainPlate.Render();
		}
	}

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the specified field visibility status to TRUE
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrain::SetFieldVisible( int x, int z )
{
	if ( x < 0 || x >= (int) SegsCountX || z < 0 || z >= (int) SegsCountZ ) return;

	pFields[z*SegsCountX + x].TerrainPlate.SetVisible();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the specified field visibility status to TRUE
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrain::SetFieldVisible( MAPPOSITION *pos )
{
	if ( pos->PosX < 0 || pos->PosX >= (int) SegsCountX || pos->PosZ < 0 || pos->PosZ >= (int) SegsCountZ ) return;

	pFields[pos->PosZ*SegsCountX + pos->PosX].TerrainPlate.SetVisible();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the specified field visibility status to FALSE
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrain::SetFieldInvisible( int x, int z )
{
	if ( x < 0 || x >= (int) SegsCountX || z < 0 || z >= (int) SegsCountZ ) return;

	pFields[z*SegsCountX + x].TerrainPlate.SetInvisible();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the specified field visibility status to FALSE
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrain::SetFieldInvisible( MAPPOSITION *pos )
{
	if ( pos->PosX < 0 || pos->PosX >= (int) SegsCountX || pos->PosZ < 0 || pos->PosZ >= (int) SegsCountZ ) return;

	pFields[pos->PosZ*SegsCountX + pos->PosX].TerrainPlate.SetInvisible();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns true if the specified field visibility status is TRUE
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGrObjectTerrain::IsFieldVisible( int x, int z )
{
	if ( x < 0 || x >= (int) SegsCountX || z < 0 || z >= (int) SegsCountZ ) return FALSE;
	
	return pFields[z*SegsCountX + x].TerrainPlate.IsVisible();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns true if the specified field visibility status is TRUE
// this version of Visibility method is a bit faster since it doesn't recount the map index,
//   it expects valid index however
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGrObjectTerrain::IsFieldVisible( int index )
{
	return pFields[index].TerrainPlate.IsVisible();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns true if the specified field visibility status is TRUE
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGrObjectTerrain::IsFieldVisible( MAPPOSITION *pos )
{
	if ( pos->PosX < 0 || pos->PosX >= (int) SegsCountX || pos->PosZ < 0 || pos->PosZ >= (int) SegsCountZ ) return FALSE;

	return pFields[pos->PosZ*SegsCountX + pos->PosX].TerrainPlate.IsVisible();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets all of the fields of the map invisibile (visibility status to FALSE)
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrain::SetWholeMapInvisible()
{
	memset( pVisibilityBitField, 0, ( (SegsCountX * SegsCountZ) / 8) + 1 );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns true if any of the map positions has a visibility status TRUE
// this can be used to determine whether a object that has its own MAPPOSITIONLIST
// interfers any visible field
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGrObjectTerrain::ObjectIsVisible( MAPPOSITIONLIST *posList )
{
	MAPPOS_ITER			posIter;
	
	for ( posIter = posList->begin(); posIter != posList->end(); posIter++ )
		if ( IsFieldVisible( &(*posIter) ) ) return TRUE;

	return FALSE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns field's height measured in terrain levels - zero based
//
//////////////////////////////////////////////////////////////////////////////////////////////
UINT CGrObjectTerrain::GetFieldHeight( int x, int z )
{
	if ( x < 0 || x >= (int) SegsCountX || z < 0 || z >= (int) SegsCountZ ) return 0;

	return pFields[z*SegsCountX + x].Height;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns field height measured in terrain levels - zero based
//
//////////////////////////////////////////////////////////////////////////////////////////////
UINT CGrObjectTerrain::GetFieldHeight( MAPPOSITION *pos )
{
	if ( pos->PosX < 0 || pos->PosX >= (int) SegsCountX || pos->PosZ < 0 || pos->PosZ >= (int) SegsCountZ ) return 0;

	return pFields[pos->PosZ*SegsCountX + pos->PosX].Height;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
// returns field rotation - 4 positions
//
///////////////////////////////////////////////////////////////////////////////////////////
UINT CGrObjectTerrain::GetFieldRotation( int x, int z)
{
	if ( x < 0 || x >= (int) SegsCountX || z < 0 || z >= (int) SegsCountZ ) return 0;

	return pFields[z * SegsCountX + x].Rotation;
};


///////////////////////////////////////////////////////////////////////////////////////////
//
// returns field rotation - 4 positions
//
///////////////////////////////////////////////////////////////////////////////////////////
UINT CGrObjectTerrain::GetFieldRotation( MAPPOSITION * pos)
{
	if ( pos->PosX < 0 || pos->PosX >= (int) SegsCountX || pos->PosZ < 0 || pos->PosZ >= (int) SegsCountZ ) return 0;

	return pFields[pos->PosZ * SegsCountX + pos->PosX].Rotation;
};


///////////////////////////////////////////////////////////////////
//
// returns terrain plate object pointer
//
//////////////////////////////////////////////////////////////////
CGrObjectTerrainPlate * CGrObjectTerrain::GetPlate( int x, int z )
{
	return &pFields[z * SegsCountX + x].TerrainPlate;
};


///////////////////////////////////////////////////////////////////////////////////////
//
// returns a pointer to terrain resource as is stored in resource manager
//
////////////////////////////////////////////////////////////////////////////////////////
resManNS::__Terrain	* CGrObjectTerrain::GetTerrainResource()
{
	return ResourceManager->GetTerrain( this->ridTerrain );
};


///////////////////////////////////////////////////////////////////////////////////////
//
// sets object pointer according to object map position list on certain terrain plate
//
////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrain::BoundObject(CGrObjectMesh * Mesh)
{
	MAPPOSITIONLIST * MapPosList;
	UINT i;
	
	MapPosList = Mesh->GetMapPosList();
	for (i = 0; i < MapPosList->size(); i++)
		GetPlate(MapPosList->at(i).PosX, MapPosList->at(i).PosZ)->GetObjects()->push_back(Mesh);
};

///////////////////////////////////////////////////////////////////////////////////////
//
// deletes object pointer according to object map position list on certain terrain plates
//
////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrain::UnBoundObject(CGrObjectMesh * Mesh)
{
	vector<CGrObjectMesh *> * MeshesOnTerrain;
	UINT i, e;
	MAPPOSITIONLIST * MapPosList;

	MapPosList = Mesh->GetMapPosList();
	for (i = 0; i < MapPosList->size(); i++)
	{
		MeshesOnTerrain = GetPlate( MapPosList->at(i).PosX, MapPosList->at(i).PosZ)->GetObjects();
		for (e = 0; e < MeshesOnTerrain->size(); e++)
			if (MeshesOnTerrain->at(e) == Mesh)
			{
				MeshesOnTerrain->erase(MeshesOnTerrain->begin() + e);
				break;
			}
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////