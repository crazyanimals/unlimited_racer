#include "stdafx.h"
#include "GrObjectGrass.h"


using namespace graphic;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor of auxiliary class
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrassItems::CGrassItems()
{
	uiItemsCount = 0;
	Items.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// destructor of auxiliary class
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrassItems::~CGrassItems()
{
	Free();
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// inits this object passing a valid pointer to a resource manager object
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrassItems::Init( resManNS::CResourceManager * pResMan )
{
	static bool		bWasInitialized = false;
	
	pResourceManager = pResMan;

	// do the part that could be processed only once
	if ( !bWasInitialized )
	{
		uiItemsCount = 0;
		bWasInitialized = true;
	}
	

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads a grass item (.shrub) file, use maxLOD paramter to tell how many LODs are requested
// if the resource is already in the list, this only lets ResourceManager know, it is not
//    added into the list
//
// returns a negative value when any error occurs or valid resource index
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrassItems::Add( LPCTSTR fileName, UINT maxLOD )
{
	resManNS::RESOURCEID		rid;
	
	// TODO: tady se to musi uplne zmenit; nesmi se pridavat vsechny do toho seznamu,
	// ale pouze ty, ktere v nem jeste nejsou; stejne tak je zbytecne nacitat resource ktery uz existuje
	// protoze nakonci nebudeme vedet, ktere dealokovat,
	// takze se tu musi zavest vlastni seznam existujicich resources a vzdy se musi dany resource
	// najit, aby se vratil spravny ukazatel...no musi se to promyslet
	rid = pResourceManager->LoadResource( fileName, RES_GrassItem, &maxLOD );
	if ( rid < 0 ) ERRORMSG( ERRGENERIC, "CGrassItems::Add()", "Can't add a grass item." );

	Items.push_back( rid );
	uiItemsCount++;

	return (HRESULT) (uiItemsCount - 1);
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// frees all loaded data and resets all pointers and properties
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrassItems::Free()
{
	for ( RID_ITER iter = Items.begin(); iter != Items.end(); iter++ )
		if ( *iter >= 0 ) pResourceManager->ReleaseResource( *iter );

	Items.clear();
	uiItemsCount = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
/////////////////        C G r O b j e c t G r a s s         /////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrObjectGrass::CGrObjectGrass()
{
	pVertexBuffer = NULL;
	pIndexBuffer = NULL;
	uiLODCount = 0;
	uiObjectsCount = 0;
	uiLODToRender = 0;
	ridTexture = -1;
	ItemsList.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrObjectGrass::~CGrObjectGrass()
{
	SAFE_RELEASE( pVertexBuffer );
	SAFE_RELEASE( pIndexBuffer );

	ItemsList.clear();

	if ( ridTexture >= 0 ) ResourceManager->ReleaseResource( ridTexture );
	ridTexture = -1;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Initialization callback function
// Used for loading grass data
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK CGrObjectGrass::LoadCallBack( LPNAMEVALUEPAIR pPair )
{
	CString				name;
	GRASSLOADINGSTRUCT *point;
	HRESULT				hr;
	GRASSITEMTOPROCESS	itemToProcess;
	resManNS::GrassItem		*	pGrassItem;	
	
#define __GRASSERROR(ch)	{ ErrorHandler.HandleError( ch, "CGrObjectGrass::LoadCallBack()", \
									"Grass definition file has wrong syntax." ); \
							pPair->ThrowError(); return; }


	name = pPair->GetName();
	point = (GRASSLOADINGSTRUCT *) (pPair->ReservedPointer);
	
	name.MakeLower();

	
	if ( point->bInItemTag ) // in ITEM tag
	{
		if ( pPair->GetFlags() == NVPTYPE_TAGEND )
		{
			if ( name == "item" )
			{
				// add the grass item into the prototypes list (load new resource)
				hr = point->pGrassObject->ItemPrototypes.Add( point->csItemFile, point->uiLODsRequested );
				if ( hr < 0 ) __GRASSERROR( ERRGENERIC );
				
				// obtain a pointer to the loaded GrassItem resource
				pGrassItem = ResourceManager->GetGrassItem( point->pGrassObject->ItemPrototypes.GetItemRID( hr ) );
				if ( !pGrassItem ) __GRASSERROR( ERRNOTFOUND );

				// add this item to the list for later processing (later processing will be done by calling object)
				itemToProcess.x = point->fItemX;
				itemToProcess.y = point->fItemY;
				itemToProcess.z = point->fItemZ;
				itemToProcess.Rotation = point->fItemRotation;
				itemToProcess.Scale = point->fItemScale;
				itemToProcess.uiGrassItemIndex = (UINT) hr;
				itemToProcess.ridTexture = pGrassItem->ridTexture;
				point->pItemsToProcess->push_back( itemToProcess );

				point->bInItemTag = false;
			}
		}
		else if ( pPair->GetFlags() & NVPTYPE_VARIABLE )
		{
			if ( name == "file" )		point->csItemFile = pPair->GetString();
			if ( name == "x" )			point->fItemX = pPair->GetFloat();
			if ( name == "y" )			point->fItemY = pPair->GetFloat();
			if ( name == "z" )			point->fItemZ = pPair->GetFloat();
			if ( name == "rotation" )	point->fItemRotation = pPair->GetFloat();
			if ( name == "scale" )		point->fItemScale = pPair->GetFloat();
		}
	}
	else // global position (outside any tags)
	{
		// change default texture name
		if ( name == "texture" && pPair->GetFlags() & NVPTYPE_VARIABLE )
		{ 
			if ( !point->bTextureFileDefined )
			{
				point->csTextureFileName = pPair->GetString();
				point->pGrassObject->ridTexture = ResourceManager->LoadResource( point->csTextureFileName, RES_Texture );
				if ( point->pGrassObject->ridTexture < 0 ) 
				{
					ErrorHandler.HandleError( ERRGENERIC, "CGrObjectGrass::LoadCallBack()", "Grass texture could not be loaded." );
					pPair->ThrowError();
					return;
				}

				point->bTextureFileDefined = true;
			}
			else OUTMSG( "Warning: Only one texture is allowed for all grass items in one group.", 1 );
		}

		// change default .shrub file name
		if ( name == "itemfile" && pPair->GetFlags() & NVPTYPE_VARIABLE ) { point->csDefaultItemFileName = pPair->GetString(); }

		// item definition is beeing opened
		if ( name == "item" && pPair->GetFlags() == NVPTYPE_TAGBEGIN )
		{
			if ( point->bInItemTag ) __GRASSERROR( ERRGENERIC );
			point->UseItemDefaults();
			point->bInItemTag = true;
			point->csItemFile = point->csDefaultItemFileName;
		}
	}

#undef __GRASSERROR
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Initializing function - has to be called before any other operation is performed on this object.
// Loads the grass-group data from file, then load every grass item that is used in this group
// Finally this processes every single grass item and create a vertex buffer containing 
//    a geometry for all processed items.
//
// Use LODcount parameter to define count of LODs that should be loaded and set ridSurface
//    to RID of terrain surface (resManNS::GrSurface) on which this grass group will be planted
//
// Returns a nonzero error code or zero if no error occurs.
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectGrass::Init(	BOOL Static, 
								CString fileName,
								UINT LODcount,
								CGrObjectTerrainPlate * pTerrainPlate )
{
	HRESULT				hr;
	GRASSLOADINGSTRUCT	loadingStruct;
	TOPROCESS_LIST		ItemsToProcess;
	resManNS::GrassItem		*	pGrassItem;
	resManNS::GrSurface		*	pGrSurface;
	UINT				puiVertices[MODEL_LOD_COUNT], puiIndices[MODEL_LOD_COUNT];
	UINT				uiVertices, uiIndices;
	VERTEX			*	pVert;
	WORD			*	pInd;	
	UINT				puiVertInd[MODEL_LOD_COUNT], puiIndInd[MODEL_LOD_COUNT];
	float				fDeltaY;



	// INIT
	
	// use inherited initializator
	hr = this->CGrObjectBase::Init( true );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGrObjectGrass::Init()", "An error occured while initializing global parts of the object." );


	ItemsList.clear();
	ridTexture = -1;
	uiLODToRender = 0;
	iMapPosX = iMapPosZ = 0;

	// init Item prototypes static object - is written so that it could be called several times
	hr = ItemPrototypes.Init( ResourceManager );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGrObjectGrass::Init()", "Can't initialize grass prototypes structure." );
	


	
	// LOAD

	// init loading structure to default values
	ItemsToProcess.clear();
	loadingStruct.Init( this, &ItemsToProcess, LODcount );

	// load the grass data from configuration file
	hr = NAMEVALUEPAIR::Load( fileName, LoadCallBack, &loadingStruct );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGrObjectGrass::Init()", "Can't load grass object definition." );




	// CALCULATE NEEDED SPACE
	
	// obtain the terrain surface
	pGrSurface = ResourceManager->GetGrSurface( pTerrainPlate->GetTerrainType() );
	if ( !pGrSurface ) ERRORMSG( ERRNOTFOUND, "CGrObjectGrass::Init()", "Couldn't obtain terrain surface data." );


	// calculate the count of vertices and indices that will be used for each LOD
	uiVertices = uiIndices = 0;
	for ( UINT i = 0; i < LODcount; i++ ) puiVertices[i] = puiIndices[i] = 0;

	for ( TOPROCESS_ITER iter = ItemsToProcess.begin(); iter != ItemsToProcess.end(); iter++ )
	{
		if ( ridTexture != iter->ridTexture ) OUTMSG( "Warning: all grass items in a group should use the same texture.", 2 );

		pGrassItem = ResourceManager->GetGrassItem( ItemPrototypes.GetItemRID( iter->uiGrassItemIndex ) );
		if ( !pGrassItem ) ERRORMSG( ERRNOTFOUND, "CGrObjectGrass::Init()", "Couldn't retrieve grass item data." );

		// count each LOD separately
		for ( UINT i = 0; i < LODcount; i++ )
		{
			puiVertices[i] += (pGrassItem->pLODs[i].uiSegmentsCount + 1) * (pGrassItem->pLODs[i].uiStripesCount + 1);
			puiIndices[i] += 6 * pGrassItem->pLODs[i].uiSegmentsCount * pGrassItem->pLODs[i].uiStripesCount;
		}

		ItemsList.push_back( iter->uiGrassItemIndex ); 
	}

	for ( UINT i = 0; i < LODcount; i++ ) uiVertices += puiVertices[i];
	for ( UINT i = 0; i < LODcount; i++ ) uiIndices += puiIndices[i];
 
	InitialIndices[0] = 0;
	for ( UINT i = 0; i < LODcount; i++ ) InitialIndices[i + 1] = InitialIndices[i] + puiIndices[i];
	InitialVertices[0] = 0;
	for ( UINT i = 0; i < LODcount; i++ ) InitialVertices[i + 1] = InitialVertices[i] + puiVertices[i];




	// CREATE APPROPRIATE BUFFERS

	hr = D3DDevice->CreateVertexBuffer( uiVertices * sizeof( VERTEX ), 0, VertexFVF, D3DPOOL_MANAGED, &pVertexBuffer, NULL );
	if ( hr ) ERRORMSG( hr, "CGrObjectGrass::Init()", "Unable to create vertex buffer for grass group." );
	
	hr = D3DDevice->CreateIndexBuffer( uiIndices * 2, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pIndexBuffer, NULL );
	if ( hr ) ERRORMSG( hr, "CGrObjectGrass::Init()", "Unable to create index buffer for grass group." );

	hr = pVertexBuffer->Lock( 0, 0, (LPVOID *) &pVert, 0 );
	if ( hr ) ERRORMSG( hr, "CGrObjectGrass::Init()", "Unable to lock vertex buffer for grass group." );
	
	hr = pIndexBuffer->Lock( 0, 0, (LPVOID *) &pInd, 0 );
	if ( hr ) ERRORMSG( hr, "CGrObjectGrass::Init()", "Unable to lock index buffer for grass group." );


	

	// PROCESS LOADED GRASS ITEMS

	for ( UINT i = 0; i < LODcount; i++ ) puiVertInd[i] = InitialVertices[i];
	for ( UINT i = 0; i < LODcount; i++ ) puiIndInd[i] = InitialIndices[i];
	
	for ( TOPROCESS_ITER iter = ItemsToProcess.begin(); iter != ItemsToProcess.end(); iter++ )
	{
		pGrassItem = ResourceManager->GetGrassItem( ItemPrototypes.GetItemRID( iter->uiGrassItemIndex ) );
		if ( !pGrassItem ) ERRORMSG( ERRNOTFOUND, "CGrObjectGrass::Init()", "Couldn't retrieve grass item data." );

		// correct the items that are placed outside of the field
		if ( iter->x < 0 || iter->x > TERRAIN_PLATE_WIDTH )
		{
			iter->x = 500;
			OUTMSG( "Warning: Grass item out of field!", 2 );
		}

		if ( iter->z < 0 || iter->z > TERRAIN_PLATE_WIDTH )
		{
			iter->z = 500;
			OUTMSG( "Warning: Grass item out of field!", 2 );
		}

		fDeltaY = GetSurfaceHeight( pGrSurface, iter->x, iter->z );
		
		// process each LOD separately
		for ( UINT l = 0; l < LODcount; l++ )
		{
			// fill the index buffer
			for ( UINT i = 0; i < pGrassItem->pLODs[l].uiSegmentsCount; i++ )
			{
				for ( UINT j = 0; j < pGrassItem->pLODs[l].uiStripesCount; j++ )
				{
					pInd[puiIndInd[l]++] = puiVertInd[l] + i * (pGrassItem->pLODs[l].uiStripesCount + 1) + j;
					pInd[puiIndInd[l]++] = puiVertInd[l] + (i + 1) * (pGrassItem->pLODs[l].uiStripesCount + 1) + j;
					pInd[puiIndInd[l]++] = puiVertInd[l] + (i + 1) * (pGrassItem->pLODs[l].uiStripesCount + 1) + j + 1;
					
					pInd[puiIndInd[l]++] = puiVertInd[l] + i * (pGrassItem->pLODs[l].uiStripesCount + 1) + j;
					pInd[puiIndInd[l]++] = puiVertInd[l] + (i + 1) * (pGrassItem->pLODs[l].uiStripesCount + 1) + j + 1;
					pInd[puiIndInd[l]++] = puiVertInd[l] + i * (pGrassItem->pLODs[l].uiStripesCount + 1) + j + 1;
				}
			}

			// fill the vertex buffer
			for ( UINT i = 0; i <= pGrassItem->pLODs[l].uiSegmentsCount; i++ )
			{
				for ( UINT j = 0; j <= pGrassItem->pLODs[l].uiStripesCount; j++ )
				{
					UINT	_width = pGrassItem->pLODs[l].uiStripesCount + 1;
					UINT	_ind = i * _width + j;

					pVert[puiVertInd[l]].Coord.x = iter->Scale * pGrassItem->pLODs[l].pVertices[_ind].x;
					pVert[puiVertInd[l]].Coord.y = iter->Scale * pGrassItem->pLODs[l].pVertices[_ind].y;
					pVert[puiVertInd[l]].Coord.z = 0;
					
					pVert[puiVertInd[l]].TexCoord.x = pGrassItem->pLODs[l].pVertices[_ind].u;
					pVert[puiVertInd[l]].TexCoord.y = pGrassItem->pLODs[l].pVertices[_ind].v;
					
					pVert[puiVertInd[l]].RPCoord.x = iter->x;// - TERRAIN_PLATE_WIDTH;
					pVert[puiVertInd[l]].RPCoord.y = iter->y + fDeltaY;
					pVert[puiVertInd[l]].RPCoord.z = iter->z;// - TERRAIN_PLATE_WIDTH;
					
					pVert[puiVertInd[l]].Segment = pGrassItem->pLODs[l].pVertices[_ind].y / 
												   pGrassItem->pLODs[l].pVertices[pGrassItem->pLODs[l].uiSegmentsCount * _width + j].y;
					pVert[puiVertInd[l]].SinusY = sinf( iter->Rotation );
					pVert[puiVertInd[l]].CosinusY = cosf( iter->Rotation );

					puiVertInd[l] += 1;
				}
			}
		}
	}

	pVertexBuffer->Unlock();
	pIndexBuffer->Unlock();


	uiObjectsCount = (UINT) ItemsToProcess.size();
	ItemsToProcess.clear();


	uiLODCount = LODcount;

	Material.Ambient.r = Material.Ambient.g = Material.Ambient.b = Material.Ambient.a = 1.0f;
	Material.Diffuse.r = Material.Diffuse.g = Material.Diffuse.b = Material.Diffuse.a = 1.0f;
	Material.Specular.r = Material.Specular.g = Material.Specular.b = Material.Specular.a = 0.0f;
	Material.Emissive.r = Material.Emissive.g = Material.Emissive.b = Material.Emissive.a = 0.0f;
	Material.Power = 1.0;

	
	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns height of the surface in specified coordinates (performs interpolation)
//
//////////////////////////////////////////////////////////////////////////////////////////////
float CGrObjectGrass::GetSurfaceHeight( resManNS::GrSurface * pSurface, float x, float z )
{
    struct _VERTEX  {
        float x, y, z;
        float nx, ny, nz;
        float tu, tv;
    } ;

	int		ix, iz;
	float	dx, dz;
	int		iX1, iX2, iZ1, iZ2; // 4 coordinates of surrounding vertices
	int		i11, i12, i21, i22; // 4 indices of surrounding vertices
    HRESULT hr;
    _VERTEX *v;
	float	fVal1, fVal2;
	

	if ( x < 0 ) x = 0;
	if ( x > TERRAIN_PLATE_WIDTH ) x = TERRAIN_PLATE_WIDTH;
	if ( z < 0 ) z = 0;
	if ( z > TERRAIN_PLATE_WIDTH ) z = TERRAIN_PLATE_WIDTH;
	if ( !pSurface ) return 0;

	// compute nearest lower coordinates
	ix = (int) ( (float) pSurface->SegsCountX * ( x / TERRAIN_PLATE_WIDTH ) );
	dx = x - (float) ix * TERRAIN_PLATE_WIDTH / (float) pSurface->SegsCountX;
	dx = (float) pSurface->SegsCountX * ( dx / TERRAIN_PLATE_WIDTH );

	iz = (int) ( (float) pSurface->SegsCountZ * ( z / TERRAIN_PLATE_WIDTH ) );
	dz = z - (float) iz * TERRAIN_PLATE_WIDTH / (float) pSurface->SegsCountZ;
	dz = (float) pSurface->SegsCountZ * ( dz / TERRAIN_PLATE_WIDTH );

	// set coordinates of surrounding vertices
	iX1 = ix;
	iX2 = min( (int) pSurface->SegsCountX, ix + 1 );
	iZ1 = iz;
	iZ2 = min( (int) pSurface->SegsCountZ, iz + 1 );

	// set indices of surrounding vertices
	i11 = iZ1 * ( pSurface->SegsCountX + 1 ) + iX1;
	i12 = iZ2 * ( pSurface->SegsCountX + 1 ) + iX1;
	i21 = iZ1 * ( pSurface->SegsCountX + 1 ) + iX2;
	i22 = iZ2 * ( pSurface->SegsCountX + 1 ) + iX2;

	// lock the vertex buffer
	hr = pSurface->pVertices->Lock( 0, 0, (LPVOID *) &v, 0 );
	if ( hr ) return 0;

	// compute interpolants
	fVal1 = (v[i21].y - v[i11].y) * dx + v[i11].y;
	fVal2 = (v[i22].y - v[i12].y) * dx + v[i12].y;

	pSurface->pVertices->Unlock();

	return (fVal2 - fVal1) * dz + fVal1;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Renders this group of objects with specified LOD
// GrassPS and GrassVS are supposed to be used for render
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectGrass::Render( UINT uiLOD )
{
	resManNS::Texture *	pTextureStruct;

	
	pTextureStruct = ResourceManager->GetTexture( ridTexture );

	// TODO: this could be moved to some other place so that it would be called only once for all grass
	D3DDevice->SetTexture( 1, pTextureStruct->texture );
	D3DDevice->SetFVF( VertexFVF );
	D3DDevice->SetStreamSource( 0, pVertexBuffer, 0, sizeof(VERTEX) );
	D3DDevice->SetIndices( pIndexBuffer );
	
	D3DDevice->DrawIndexedPrimitive(	D3DPT_TRIANGLELIST,
										0,
										InitialVertices[uiLOD],
										InitialVertices[uiLOD + 1] - InitialVertices[uiLOD],
										InitialIndices[uiLOD],
										(InitialIndices[uiLOD + 1] - InitialIndices[uiLOD]) / 3 );


	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Renders this group of objects with LOD specified before
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectGrass::Render()
{
	return Render( min( uiLODToRender, uiLODCount - 1 ) );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
