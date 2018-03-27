#include "stdafx.h"
#include "GrObjectMesh.h"

using namespace graphic;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrObjectMesh::CGrObjectMesh()
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
CGrObjectMesh::~CGrObjectMesh()
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
void CGrObjectMesh::PreInitThis()
{
	for ( UINT i = 0; i < MODEL_LOD_COUNT; i++ ) 
	{
		pLODs[i].ridXFile = -1;
		pLODs[i].uiSubsetsCount = 0;
		pLODs[i].pSubsets = NULL;
	}

	for ( UINT i = 0; i < MODEL_LOD_COUNT; i++ ) ShadowVolumes[i] = NULL;
	uiLODCount = 0;
	Light = 0;
	for (UINT i = 0; i < 4; i++)
		Wheels[i] = 0;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this initializes all internal structures to default values
// !!! also calls PreInit() method of all ancestors
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectMesh::PreInit()
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
void CGrObjectMesh::Free()
{
	for ( UINT i = 0; i < MODEL_LOD_COUNT; i++ ) 
	{
		if ( pLODs[i].ridXFile >= 0 ) ResourceManager->ReleaseResource( pLODs[i].ridXFile );
		pLODs[i].ridXFile = -1;
		for ( UINT j = 0; j < pLODs[i].uiSubsetsCount; j++ )
		{
			if ( pLODs[i].pSubsets[j].ridNewTexture >= 0 ) ResourceManager->ReleaseResource( pLODs[i].pSubsets[j].ridNewTexture );
			pLODs[i].pSubsets[j].ridOrigTexture = pLODs[i].pSubsets[j].ridNewTexture = -1;
		}

		SAFE_DELETE_ARRAY( pLODs[i].pSubsets );
	}

	for ( UINT i = 0; i < MODEL_LOD_COUNT; i++ ) { SAFE_DELETE( ShadowVolumes[i] ) }
		
	// call inherited dealocator
	this->CGrObjectBase::Free();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// load callback used for loading model definition from .modelDef file type
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK CGrObjectMesh::LoadModelDefinitionCallBack( LPNAMEVALUEPAIR pPair )
{
	((CGrObjectMesh *) (pPair->ReservedPointer))->LoadModelDefinitionCallBackNonStatic( pPair );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// non-static variant of the load callback method used for loading model definition from
//   .modelDef file type; this is called from the static variant
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK CGrObjectMesh::LoadModelDefinitionCallBackNonStatic( LPNAMEVALUEPAIR pPair )
{
	CString		name = pPair->GetName();
	int			iPom;
	resManNS::RESOURCEID	ridTex;
	resManNS::XFile	*	pXFile;
	DWORD		dwParam;
	
#define _THROW_ERR(code, str)	{	ErrorHandler.HandleError( code, "CGrObjectMesh::LoadModelDefinitionCallBackNonStatic()", str ); \
									pPair->ThrowError(); return; }


	name.MakeLower();
	
	if ( name == "" ) return;
	
	
	// opening tag
	if ( pPair->GetFlags() == NVPTYPE_TAGBEGIN )
	{
		if ( name.GetLength() < 1 ) return;

		if ( name[0] == 'l' && LoadingState == LS_Undefined ) // LOD# tag
		{
			sscanf( (LPCTSTR) name, "lod%i", &iLoadingActiveLOD );
			if ( iLoadingActiveLOD >= MODEL_LOD_COUNT || iLoadingActiveLOD < 0 )
			{
				LoadingState = LS_Skip;
				return;
			}

			if ( pLoadingStructs[iLoadingActiveLOD].bDefined ) 
			{
				LoadingState = LS_Skip;
				return;
			}

			LoadingState = LS_LODTag;
		}
		else if ( name == "textures" && LoadingState == LS_LODTag ) LoadingState = LS_TexturesTag;

		return;
	}

	

	// closing tag
	if ( pPair->GetFlags() == NVPTYPE_TAGEND )
	{
		if ( name.GetLength() < 1 ) return;

		if ( name[0] == 'l' && LoadingState == LS_LODTag ) // LOD# tag
		{
			sscanf( (LPCTSTR) name, "lod%i", &iPom );
			if ( iLoadingActiveLOD != iPom ) return;

			LoadingState = LS_Undefined;

			// ignore this whole section when 1) LOD number is too high, 2) LOD # is to low, 3) loading state is Skip 
			if ( iLoadingActiveLOD >= MODEL_LOD_COUNT || iLoadingActiveLOD < 0 || LoadingState == LS_Skip ) return;

			pLoadingStructs[iLoadingActiveLOD].bDefined = true;

			// load the .x file
			dwParam = D3DXMESH_MANAGED;
			pLODs[iLoadingActiveLOD].ridXFile = ResourceManager->LoadResource( pLoadingStructs[iLoadingActiveLOD].ModelFileName, RES_XFile, &dwParam );
			if ( pLODs[iLoadingActiveLOD].ridXFile < 0 ) _THROW_ERR( ERRGENERIC, "resManNS::XFile could not be loaded." )
			
			// resManNS::Texture substitution: we have to obtain the resManNS::XFile resource from RM and go thru all polygon subsets 
			// for each subset we have to go thru the TexRemapTable list and discover if the texture of the subset 
			// should be substituted by any other texture (this is done by comparing of RIDs)
			// if positive, this substitution is made (new texture is loaded)
			pXFile = ResourceManager->GetXFile( pLODs[iLoadingActiveLOD].ridXFile );
			if ( !pXFile ) _THROW_ERR( ERRNOTFOUND, "Could not retreive mesh data from resource manager." )

			pLODs[iLoadingActiveLOD].uiSubsetsCount = pXFile->cMtrl;

			pLODs[iLoadingActiveLOD].pSubsets = new MeshSubset[pXFile->cMtrl];
			if ( !(pLODs[iLoadingActiveLOD].pSubsets) ) _THROW_ERR( ERROUTOFMEMORY, "" )

			for ( UINT j = 0; j < pXFile->cMtrl; j++ )
			{
				pLODs[iLoadingActiveLOD].pSubsets[j].ridOrigTexture = pXFile->pTextureIDs[j];
				pLODs[iLoadingActiveLOD].pSubsets[j].ridNewTexture = -1;
	
				for ( STRREMAP_ITER iter = pLoadingStructs[iLoadingActiveLOD].TexRemapTable.begin(); iter != pLoadingStructs[iLoadingActiveLOD].TexRemapTable.end(); iter++ )
				{
					ridTex = ResourceManager->GetResourceID( iter->first, RES_Texture );
					if ( ridTex == pXFile->pTextureIDs[j] ) 
					{
						ridTex = ResourceManager->LoadResource( iter->second, RES_Texture );
						if ( ridTex < 0 ) _THROW_ERR( ERRGENERIC, "Could not load model texture." )

						pLODs[iLoadingActiveLOD].pSubsets[j].ridNewTexture = ridTex;
						pLODs[iLoadingActiveLOD].pSubsets[j].ridOrigTexture = -1;
						break;
					}
				}
			}
		}
		else if ( name == "textures" && LoadingState == LS_TexturesTag ) LoadingState = LS_LODTag;
		
		return;
	}


	
	// other (non-tag) elements
	if ( LoadingState == LS_Skip || LoadingState == LS_Undefined ) return;
	
	if ( LoadingState == LS_LODTag )
	{
		if ( name == "model" ) { pLoadingStructs[iLoadingActiveLOD].ModelFileName = pPair->GetString(); return; }
	}

	if ( LoadingState == LS_TexturesTag )
	{
		pLoadingStructs[iLoadingActiveLOD].TexRemapTable.insert( STRREMAP_PAIR( name, pPair->GetString() ) );
		return;
	}
	

#undef _THROW_ERR
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads a mesh from X file
// Static parameter specifies whether this object is static or dynamic
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectMesh::Init(	BOOL Static,
								CString fileName,
								UINT LODcount )
{
	HRESULT			hr;
	CString			csPath;
	

	// use inherited initializator
	hr = this->CGrObjectBase::Init( Static );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGrObjectMesh::Init()", "An error occurs while initializing global parts of the object." );

	// sets all properties to default values
	PreInitThis();

	// set other default values that are not included in Base object
	uiLODCount = MODEL_LOD_COUNT;
	uiLODToRender = 0;

	// set the default material as WHITE
	bUsePredefinedMaterial = TRUE;
	ZeroMemory( &OptionalMaterial, sizeof( OptionalMaterial ) );
	OptionalMaterial.Ambient.a = 1;
	OptionalMaterial.Ambient.r = 1;
	OptionalMaterial.Ambient.g = 1;
	OptionalMaterial.Ambient.b  = 1;
	OptionalMaterial.Diffuse = OptionalMaterial.Ambient;



	// load definition file and all models and textures
	for ( UINT i = 0; i < MODEL_LOD_COUNT; i++ )
	{
		pLoadingStructs[i].bDefined = false;
		pLoadingStructs[i].TexRemapTable.clear();
		pLoadingStructs[i].ModelFileName = "";
	}
	
	iLoadingActiveLOD = -1;
	LoadingState = LS_Undefined;
	csPath = ResourceManager->GetResourcePath( RES_Model );

	hr = NAMEVALUEPAIR::Load( csPath + fileName, LoadModelDefinitionCallBack, this );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGrObjectMesh::Init", "Some model parts could not be loaded." );
	
	for ( UINT i = 0; i < MODEL_LOD_COUNT; i++ ) 
		if ( !pLoadingStructs[i].bDefined ) ERRORMSG( ERRFILECORRUPTED, "CGrObjectMesh::Init", fileName );

	
/*
	// create shadow volume objects
	for ( UINT i =0; i < MODEL_LOD_COUNT; i++ )
	{
		ShadowVolumes[i] =  new CShadowVolume;
		if ( !ShadowVolumes[i] ) ERRORMSG( ERROUTOFMEMORY, "CGrObjectMesh::Init()", "Can't initialize shadow volume objects." );
	}
*/

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// inits shadow volume objects for all of the model LODs 
// has to be called only if the object is already placed into scene
// this is inefficient when used on non-static object
// use _static parameter to specify if the shadow is static or dynamic one
//
// when successfull this returns zero, otherwise a nonzero error code is returned
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectMesh::InitShadowVolumes( D3DXVECTOR3 * pLightPos, bool _static )
{
	HRESULT		hr;
	resManNS::XFile	*	pXFile;
	D3DXMATRIXA16	mat;

	mat = GetWorldMat();
	
	for ( UINT i = 0; i < uiLODCount; i++ )
	{
		pXFile = ResourceManager->GetXFile( pLODs[i].ridXFile );
		if ( !pXFile ) ERRORMSG( ERRNOTFOUND, "CGrObjectMesh::InitShadowVolumes()", "Could not retreive mesh data from resource manager." );

		hr = ShadowVolumes[i]->Init( D3DDevice, pXFile->Mesh, pLightPos, &mat, true, true );
		if ( hr ) ERRORMSG( ERRGENERIC, "CGrObjectMesh::InitShadowVolumes()", "Unable to create object's shadow volume structure." );
	}
	
	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets all needed states and renders the object, then restores all saved state
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectMesh::Render()
{
	LPD3DXMESH	Mesh;
	resManNS::XFile	*	pXFile;
	resManNS::Texture *	pTextureStruct;
	resManNS::Texture *	pTextureStructOld;

	
#ifdef GROBJECT_USE_OLD_WORLDMAT_STYLE
	if ( WorldMatNeedRecount ) CountWorldMat();
#endif
	
	// place an object to scene
	Shader->SetWorldMatrix( &WorldMat );


	pXFile = ResourceManager->GetXFile( pLODs[uiLODToRender].ridXFile ); // find xFile resource

#ifdef MY_DEBUG_VERSION
	if ( !pXFile ) ERRORMSG( ERRNOTFOUND, "CGrObjectMesh::Render()", "Invalid model pointer." );
#endif // defined MY_DEBUG_VERSION

	Mesh = pXFile->Mesh; // obtain a Mesh pointer
	

	// render an object
	pTextureStructOld = NULL;

	for ( DWORD i = 0; i < pXFile->cMtrl; i++ ) // draw each subset
	{
		if ( bUsePredefinedMaterial ) Shader->SetVSMaterial( &(pXFile->Material[i]) );
		else Shader->SetVSMaterial( &OptionalMaterial );
		
		// use a texture if it is defined
		if ( pLODs[uiLODToRender].pSubsets[i].GetTextureID() > 0 ) 
		{
			pTextureStruct = ResourceManager->GetTexture( pLODs[uiLODToRender].pSubsets[i].GetTextureID() );
			// change texture only if it is differs from the last one
			if ( pTextureStruct != pTextureStructOld )
				D3DDevice->SetTexture( 1, pTextureStruct->texture );

			// render a subset of vertices
			Mesh->DrawSubset( i );

			pTextureStructOld = pTextureStruct;
		}
	}


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets all needed states and renders the shadow volume object
// this uses the uiLODToRender to choose shadow volume detail
// use the iWhichSide parameter to define which polygons should be rendered:
//   if the iWhichSide is positive, only front sided polygons are rendered
//   if it is negative, only back sided polygons are rendered
//   if iWhichSide is zero, all (back and front faced) polygons are rendered (default value)
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectMesh::RenderShadowVolume( int iWhichSide )
{
	return ShadowVolumes[uiLODToRender]->Render( iWhichSide );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets an optional material that is used if UsePredefinedMaterial is false
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectMesh::SetMaterial( D3DMATERIAL9 *mtrl )
{
	OptionalMaterial.Ambient = mtrl->Ambient;
	OptionalMaterial.Diffuse = mtrl->Diffuse;
	OptionalMaterial.Emissive = mtrl->Emissive;
	OptionalMaterial.Power = mtrl->Power;
	OptionalMaterial.Specular = mtrl->Specular;
};
 

//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns a valid pointer to mesh structure or NULL when an error occurs
// iLOD parameter specifies which LOD you would like to pick up, default value is -1 that 
// means the highest available LOD
//
//////////////////////////////////////////////////////////////////////////////////////////////
LPD3DXMESH CGrObjectMesh::GetMesh( int iLOD )
{
	resManNS::XFile	*	pXFile;

	if ( iLOD < 0 || iLOD >= MODEL_LOD_COUNT ) iLOD = (int) uiLODCount - 1;
	pXFile = ResourceManager->GetXFile( pLODs[iLOD].ridXFile ); // find xFile resource

#ifdef MY_DEBUG_VERSION
	if ( !pXFile )
	{
		ErrorHandler.HandleError( ERRGENERIC, "CGrObjectMesh::GetMesh()", "Invalid model pointer." );
		return NULL;
	}
#endif // defined MY_DEBUG_VERSION

	return pXFile->Mesh; // obtain a Mesh pointer
};


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
