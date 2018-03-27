#include "stdafx.h"
#include "Scene.h"

using namespace graphic;


//////////////////////////////////////////////////////////////////////////////////////////////
// constructor initializes predefined material structures
//////////////////////////////////////////////////////////////////////////////////////////////
CScene::CScene()
{
	ResourceManager = NULL;
	AmbientLight = NULL;
	SunLight = NULL;
	Exists = FALSE;
	pGrassFields = NULL;
	
	ZeroMemory( &BlackMaterial, sizeof(D3DMATERIAL9) );
	BlackMaterial.Diffuse.r = 0.0f;
	BlackMaterial.Diffuse.g = 0.0f;
	BlackMaterial.Diffuse.b = 0.0f;
	BlackMaterial.Diffuse.a = 1.0f;
	BlackMaterial.Ambient.r = 0.0f;
	BlackMaterial.Ambient.g = 0.0f;
	BlackMaterial.Ambient.b = 0.0f;
	BlackMaterial.Ambient.a = 1.0f;

	ZeroMemory( &GreenMaterial, sizeof(D3DMATERIAL9) );
	GreenMaterial.Diffuse.r = 0.0f;
	GreenMaterial.Diffuse.g = 1.0f;
	GreenMaterial.Diffuse.b = 0.0f;
	GreenMaterial.Diffuse.a = 1.0f;
	GreenMaterial.Ambient.r = 0.0f;
	GreenMaterial.Ambient.g = 1.0f;
	GreenMaterial.Ambient.b = 0.0f;
	GreenMaterial.Ambient.a = 1.0f;

	ZeroMemory( &RedMaterial, sizeof(D3DMATERIAL9) );
	RedMaterial.Diffuse.r = 1.0f;
	RedMaterial.Diffuse.g = 0.0f;
	RedMaterial.Diffuse.b = 0.0f;
	RedMaterial.Diffuse.a = 1.0f;
	RedMaterial.Ambient.r = 1.0f;
	RedMaterial.Ambient.g = 0.0f;
	RedMaterial.Ambient.b = 0.0f;
	RedMaterial.Ambient.a = 1.0f;

	ZeroMemory( &GrayMaterial, sizeof(D3DMATERIAL9) );
	WhiteMaterial.Diffuse.r = 0.5f;
	WhiteMaterial.Diffuse.g = 0.5f;
	WhiteMaterial.Diffuse.b = 0.5f;
	WhiteMaterial.Diffuse.a = 1.0f;
	WhiteMaterial.Ambient.r = 0.5f;
	WhiteMaterial.Ambient.g = 0.5f;
	WhiteMaterial.Ambient.b = 0.5f;
	WhiteMaterial.Ambient.a = 1.0f;

	ZeroMemory( &BlueMaterial, sizeof(D3DMATERIAL9) );
	BlueMaterial.Diffuse.r = 0.0f;
	BlueMaterial.Diffuse.g = 0.0f;
	BlueMaterial.Diffuse.b = 1.0f;
	BlueMaterial.Diffuse.a = 1.0f;
	BlueMaterial.Ambient.r = 0.0f;
	BlueMaterial.Ambient.g = 0.0f;
	BlueMaterial.Ambient.b = 1.0f;
	BlueMaterial.Ambient.a = 1.0f;

	ZeroMemory( &YellowMaterial, sizeof(D3DMATERIAL9) );
	YellowMaterial.Diffuse.r = 1.0f;
	YellowMaterial.Diffuse.g = 1.0f;
	YellowMaterial.Diffuse.b = 0.0f;
	YellowMaterial.Diffuse.a = 1.0f;
	YellowMaterial.Ambient.r = 1.0f;
	YellowMaterial.Ambient.g = 1.0f;
	YellowMaterial.Ambient.b = 0.0f;
	YellowMaterial.Ambient.a = 1.0f;

	ZeroMemory( &CyanMaterial, sizeof(D3DMATERIAL9) );
	CyanMaterial.Diffuse.r = 0.0f;
	CyanMaterial.Diffuse.g = 1.0f;
	CyanMaterial.Diffuse.b = 1.0f;
	CyanMaterial.Diffuse.a = 1.0f;
	CyanMaterial.Ambient.r = 0.0f;
	CyanMaterial.Ambient.g = 1.0f;
	CyanMaterial.Ambient.b = 1.0f;
	CyanMaterial.Ambient.a = 1.0f;

	ZeroMemory( &MagentaMaterial, sizeof(D3DMATERIAL9) );
	MagentaMaterial.Diffuse.r = 1.0f;
	MagentaMaterial.Diffuse.g = 0.0f;
	MagentaMaterial.Diffuse.b = 1.0f;
	MagentaMaterial.Diffuse.a = 1.0f;
	MagentaMaterial.Ambient.r = 1.0f;
	MagentaMaterial.Ambient.g = 0.0f;
	MagentaMaterial.Ambient.b = 1.0f;
	MagentaMaterial.Ambient.a = 1.0f;

	ZeroMemory( &BlendedMaterial, sizeof(D3DMATERIAL9) );
	BlendedMaterial.Diffuse.r = 1.0f;
	BlendedMaterial.Diffuse.g = 1.0f;
	BlendedMaterial.Diffuse.b = 1.0f;
	BlendedMaterial.Diffuse.a = 0.5f;
	BlendedMaterial.Ambient.r = 1.0f;
	BlendedMaterial.Ambient.g = 1.0f;
	BlendedMaterial.Ambient.b = 1.0f;
	BlendedMaterial.Ambient.a = 0.5f;

	ZeroMemory( &WhiteMaterial, sizeof(D3DMATERIAL9) );
	WhiteMaterial.Diffuse.r = 1.0f;
	WhiteMaterial.Diffuse.g = 1.0f;
	WhiteMaterial.Diffuse.b = 1.0f;
	WhiteMaterial.Diffuse.a = 1.0f;
	WhiteMaterial.Ambient.r = 1.0f;
	WhiteMaterial.Ambient.g = 1.0f;
	WhiteMaterial.Ambient.b = 1.0f;
	WhiteMaterial.Ambient.a = 1.0f;
	
};

////////////////////////////////////////////////////////////////////////////////////////
// Frees all scene resources - as if game was not loaded
/////////////////////////////////////////////////////////////////////////////////////////
void CScene::Free()
{
	// Free objects
	FreeObjects();
	// Free lights
	FreeLights();
	// Free map and terrain
	CScene::Map.Free();
	// Free sky system
	SkySystem.Free();
	// Scene doesn't exist at all
	Exists = FALSE;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// frees all light objects stored in Scene
//////////////////////////////////////////////////////////////////////////////////////////////
void CScene::FreeLights()
{
	LIGHTS_ITER		lightIter;

	for ( lightIter = Lights.begin(); lightIter != Lights.end(); lightIter++ )
	{
		SAFE_DELETE( (*lightIter) );
	}
	
	Lights.clear();
};


//////////////////////////////////////////////////////////////////////////////////////////////
// frees all objects stored in Scene
//////////////////////////////////////////////////////////////////////////////////////////////
void CScene::FreeObjects()
{
	GROBJECTS_ITER	objIter;

	// free all renderable objects
	for ( objIter = StaticObjects.begin(); objIter != StaticObjects.end(); objIter++ )
	{
		SAFE_DELETE( (*objIter) );
	}
	
	StaticObjects.clear();


	for ( objIter = DynamicObjects.begin(); objIter != DynamicObjects.end(); objIter++ )
	{
		SAFE_DELETE( (*objIter) );
	}

	DynamicObjects.clear();


	for ( objIter = TransparentObjects.begin(); objIter != TransparentObjects.end(); objIter++ )
	{
		SAFE_DELETE( (*objIter) );
	}

	TransparentObjects.clear();

	Cars.clear();


	SAFE_DELETE_ARRAY( pGrassFields );
	
	for ( GRASS_ITER i = GrassList.begin(); i != GrassList.end(); i++ )
		SAFE_DELETE( i->pGrass )
};


//////////////////////////////////////////////////////////////////////////////////////////////
// frees every graphic objects stored in Scene
//////////////////////////////////////////////////////////////////////////////////////////////
CScene::~CScene()
{
	RESOURCES_ITER	resIter;

	Map.Free();
	SkySystem.Free();
	
	FreeObjects();
	FreeLights();

	// delete ambient and sun light
	SAFE_DELETE ( AmbientLight);
	SAFE_DELETE ( SunLight);

	// free all resources
	for ( resIter = Resources.begin(); resIter != Resources.end(); resIter++ )
	{
		ResourceManager->ReleaseResource( *resIter );
	}

	Resources.clear();
};


//////////////////////////////////////////////////////////////////////////////////////////////
// inits the Scene object
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CScene::Init( resManNS::CResourceManager *ResManager )
{
	ResourceManager = ResManager;
	SAFE_DELETE_ARRAY( pGrassFields );

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
// 
// use this to create data structure for storing grass objects
// !!! call Init and set Map object before using this !!!
// you have to call this method before calling AddGrass()
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CScene::InitGrassMap( UINT lodcount )
{
	if ( pGrassFields ) OUTMSG( "!Warning! Grass object alreadt initialized.", 1 )
	
	pGrassFields = new LPGROBJECTGRASS[Map.GetDepth() * Map.GetWidth()];
	if ( !pGrassFields ) ERRORMSG( ERROUTOFMEMORY, "CScene::InitGrassMap()", "" );

	for ( UINT i = 0; i < Map.GetDepth(); i++ )
		for ( UINT j = 0; j < Map.GetWidth(); j++ )
			pGrassFields[i*Map.GetWidth() + j] = NULL;

	GrassList.clear();

	uiGrassLODCount = lodcount;

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
// 
// use this to create and initialize new grass object and attach it to specified map field
// new object is created only if the same type doesn't exist yet (that means no grass object
//   is already placed on the same field type with the same rotation)
// !!! call InitGrassMap and set Map object before using this !!!
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CScene::AddGrassToField( CString fileName, UINT x, UINT z )
{
	HRESULT			hr;
	CGrObjectGrass	*pGrass;
	BOOL			found;
	GRASSID			GrassID;


	// find a grass field that is the same as the one being actually added
	found = false;
	for ( GRASS_ITER i = GrassList.begin(); i != GrassList.end(); i++ )
		if ( ( i->csFileName == fileName ) && ( i->ridSurface == Map.GetPlate( x, z )->GetTerrainType() ) )
		{
			found = true;
			pGrass = i->pGrass;
			break;
		}

	// similar grass field doesn't exist yet, lets create a new one
	if ( !found ) 
	{
		pGrass = new CGrObjectGrass;
		if ( !pGrass ) ERRORMSG( ERROUTOFMEMORY, "CScene::AddGrassToField", "" );
	
		hr = pGrass->Init( false, fileName, uiGrassLODCount, Map.GetPlate( x, z ) );
		if ( hr ) ERRORMSG( ERRGENERIC, "CScene::AddGrassToField()", "Unable to assign grass to map field." );

		GrassID.csFileName = fileName;
		GrassID.pGrass = pGrass;
		GrassID.ridSurface = Map.GetPlate( x, z )->GetTerrainType();

		GrassList.push_back( GrassID );
	}
	
	// store the grass object pointer to specified map position
	pGrassFields[Map.GetWidth() * z + x] = pGrass;


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
// fills the FreeUseMaterial structure with this color
// the default alpha value is 1.0
//////////////////////////////////////////////////////////////////////////////////////////////
void CScene::SetMaterial( FLOAT r, FLOAT g, FLOAT b, FLOAT a )
{
	FreeUseMaterial.Diffuse.r = r;
	FreeUseMaterial.Diffuse.g = g;
	FreeUseMaterial.Diffuse.b = b;
	FreeUseMaterial.Diffuse.a = a;
	FreeUseMaterial.Ambient.r = r;
	FreeUseMaterial.Ambient.g = g;
	FreeUseMaterial.Ambient.b = b;
	FreeUseMaterial.Ambient.a = a;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
