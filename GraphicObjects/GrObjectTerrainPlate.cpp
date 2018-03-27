#include "stdafx.h"
#include "GrObjectTerrainPlate.h"

using namespace graphic;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrObjectTerrainPlate::CGrObjectTerrainPlate()
{
	// call inherited constructor

	// sets all properties to default values
	PreInitThis();
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrObjectTerrainPlate::~CGrObjectTerrainPlate()
{
	Free();
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// private method called by constructor
// this initializes all internal structures to default values
// !!! this doesn't call inherited PreInit() method
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrainPlate::PreInitThis()
{
	for ( UINT i = 0; i < TERRAIN_LOD_COUNT; i++ ) TerrainID[i] = -1;
	LightMapTexture = NULL;
	Lamp = NULL;
	ridRoadTexture = -1;
	TextureID = -1;
	ridGrPlate = -1;
	uiLODCount = 0;
	pVisibilityBitField = NULL;
	Objects.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this initializes all internal structures to default values
// !!! also calls PreInit() method of all ancestors
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrainPlate::PreInit()
{
	// calls inherited PreInit() method
	this->CGrObjectBase::PreInit();

	// init own data
	PreInitThis();
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// releases all structures created by Init() or other calls
// this is called by destructor, but could be used separately for this object's reuse
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrainPlate::Free()
{
	for ( UINT i = 0; i < TERRAIN_LOD_COUNT; i++ ) 
		if ( (TerrainID[i] >= 0) && (ResourceManager) )
		{
			ResourceManager->ReleaseResource( TerrainID[i] );
			TerrainID[i] = -1;
		}

	if ( ridRoadTexture >= 0 ) ResourceManager->ReleaseResource( ridRoadTexture );
	ridRoadTexture = -1;
	if ( TextureID >= 0 ) ResourceManager->ReleaseResource( TextureID );
	TextureID = -1;
	if ( ridGrPlate >= 0 ) ResourceManager->ReleaseResource( ridGrPlate );
	ridGrPlate = -1;
		
	// call inherited dealocator
	this->CGrObjectBase::Free();
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads a terrain plate information from specified file
// then opens all needed files and loads a vertex data of all LOD levels from them
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectTerrainPlate::Init(	BOOL Static, 
										CString fileName,
										UINT LODcount )
{
	HRESULT			hr;
	resManNS::__GrPlate	*	plate;

	
	// use inherited initializator
	hr = this->CGrObjectBase::Init( Static );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGrObjectTerrainPlate::Init()", "An error occured while initializing global parts of the object." );

	// sets all properties to default values
	PreInitThis();


	// set other default values that are not included in Base object
	uiLODCount = LODcount;
	uiLODToRender = 0;
	bUseTextureRotation = FALSE;
	bUseRoadTextureRotation = FALSE;
	fTextureRotAngle = fRoadTextureRotAngle = fRotationAngle = 0;
	bPlanar = false;
	
	ZeroMemory( &Material, sizeof( D3DMATERIAL9 ) );
	Material.Diffuse.r = 1.0f;
	Material.Diffuse.g = 1.0f;
	Material.Diffuse.b = 1.0f;
	Material.Diffuse.a = 1.0f;

	Material.Ambient.r = 1.0f;
	Material.Ambient.g = 1.0f;
	Material.Ambient.b = 1.0f;
	Material.Ambient.a = 1.0f;
	

	if ( !fileName.GetLength() ) return ERRNOERROR; // don't load plate if no file is specified


	// load the grPlate
	ridGrPlate = ResourceManager->LoadResource( fileName, RES_GrPlate, &uiLODCount );
	if ( ridGrPlate <= 0 ) ERRORMSG( ERRGENERIC, "CGrObjectTerrainPlate::Init()", "Could not load terrain plate object." );

	plate = ResourceManager->GetGrPlate( ridGrPlate );
	if ( !plate ) ERRORMSG( ERRNOTFOUND, "CGrObjectTerrainPlate::Init()", "Could not retrieve terrain plate object." );

	// check the set count of LoDs and count of LoD actually loaded
	if ( uiLODCount > plate->uiLoDCount ) ERRORMSG( ERRINVALIDPARAMETER, "CGrObjectTerrainPlate::Init()", "LOD count set too high or not enough LODs loaded." );

	// set new surfaces
	for ( UINT i = 0; i < uiLODCount; i++ )	TerrainID[i] = plate->LoD[i];


	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// InitialWorldPlacement method can be used for intuitive specification of position and
//   rotation of the object without need of computing the world matrix manually.
// This one redeclares the inherited method because it also rotates all textures placed on 
//   the terrain field.
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrainPlate::InitialWorldPlacement(	D3DXVECTOR3 & position,
													D3DXVECTOR3 & rotation,
													D3DXVECTOR3 & scale )
{
	D3DXMATRIX		mat;
	D3DXMATRIX		mat2;					

	if ( Static && WasMoved ) return;


	D3DXMatrixScaling( &WorldMat, scale.x, scale.y, scale.z );
	D3DXMatrixRotationYawPitchRoll( &mat, rotation.y, rotation.x, rotation.z );
	D3DXMatrixMultiply( &WorldMat, &WorldMat, &mat );
	D3DXMatrixTranslation( &mat, position.x, position.y - TERRAIN_TRANSLATION_HACK, position.z );
	D3DXMatrixMultiply( &TranslatedMatrix, &WorldMat, &mat ); // compute
	D3DXMatrixTranslation( &mat, position.x, position.y, position.z );
	D3DXMatrixMultiply( &WorldMat, &WorldMat, &mat );

#ifdef GROBJECT_USE_OLD_WORLDMAT_STYLE
	Position = position;
	TranslatedPosition = position;
	TranslatedPosition.y -= TERRAIN_TRANSLATION_HACK;
	Rotation = rotation;
	Scaling = scale;
#endif

	fRotationAngle = rotation.y;
	ComputeTextureRotation();
	ComputeRoadTextureRotation();

	WasMoved = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the actual primary texture to the one specified by its handler to ResourceManager
//
// also performs releasing of the old texture and increasing reference count of the new one 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrainPlate::SetTexture( resManNS::RESOURCEID textureID )
{
	// increase the reference count to the new texture
	ResourceManager->DuplicateResource( textureID );

	// release the old primary texture if one is set
	if ( TextureID > 0 ) ResourceManager->ReleaseResource( TextureID );

	// set the new actual primary texture
	TextureID = textureID;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// releases actually attached primary texture and loads a new one
//
// returns a nonzero error value when unsuccessful
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectTerrainPlate::LoadTexture( CString fileName )
{
	resManNS::RESOURCEID		resID;
	
	resID = TextureID; // save the ID

	// load new resource (if it is same, it is duplicated)
	TextureID = ResourceManager->LoadResource( fileName, RES_Texture );
	if ( TextureID <= 0 ) ERRORMSG( ERRGENERIC, "CGrObjectTerrainPlate::LoadTexture()", "Can't obtain primary texture for terrain plate." )

	// release the old primary texture if one is set
	if ( resID > 0 ) ResourceManager->ReleaseResource( resID );

	
	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the actual road (secondary) texture to the one specified by its handler to ResourceManager
//
// also performs releasing of the old texture and increasing reference count of the new one 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrainPlate::SetRoadTexture( resManNS::RESOURCEID ridTex )
{
	// increase the reference count to the new texture
	ResourceManager->DuplicateResource( ridTex );

	// release the old primary texture if one is set
	if ( ridRoadTexture > 0 ) ResourceManager->ReleaseResource( ridRoadTexture );

	// set the new actual primary texture
	ridRoadTexture = ridTex;
}

	
//////////////////////////////////////////////////////////////////////////////////////////////
//
// releases actually attached secondary (road) texture and loads a new one
//
// returns a nonzero error value when unsuccessful
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectTerrainPlate::LoadRoadTexture( CString fileName )
{
	resManNS::RESOURCEID		resID;
	
	resID = ridRoadTexture; // save the ID

	// load new resource (if it is same, it is duplicated)
	ridRoadTexture = ResourceManager->LoadResource( fileName, RES_Texture );
	if ( ridRoadTexture <= 0 ) ERRORMSG( ERRGENERIC, "CGrObjectTerrainPlate::LoadRoadTexture()", "Can't obtain secondary texture for terrain plate." )

	// release the old background texture if one is set
	if ( resID > 0 ) ResourceManager->ReleaseResource( resID );

	
	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the actual terrain plate to the one specified by its handler to ResourceManager
//
// also performs releasing of the old plate and increasing reference count of the new one 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrainPlate::SetPlate( resManNS::RESOURCEID resID )
{
	resManNS::__GrPlate	*	plate;
	
	
	// release old resources
	if ( ridGrPlate > 0 ) ResourceManager->ReleaseResource( ridGrPlate );
	ridGrPlate = -1;

	for ( UINT i = 0; i < uiLODCount; i++ )
	{
		if ( TerrainID[i] > 0 ) ResourceManager->ReleaseResource( TerrainID[i] );
		TerrainID[i] = -1;
	}


	// obtain a pointer to the new plate
	plate = ResourceManager->GetGrPlate( resID );
	if ( !plate ) return;

	// check the set count of LoDs and count of LoD actually loaded
	if ( uiLODCount > plate->uiLoDCount )
	{
		ErrorHandler.HandleError( ERRINVALIDPARAMETER, "CGrObjectTerrainPlate::SetPlate()", "LoD count set too high or not enough LoD's loaded." );
		return;
	}

	
	// set new surfaces
	ResourceManager->DuplicateResource( resID );
	for ( UINT i = 0; i < uiLODCount; i++ )
	{
		ResourceManager->DuplicateResource( plate->LoD[i] );
		TerrainID[i] = plate->LoD[i];
	}


	ridGrPlate = resID;
	bPlanar = plate->bPlanar != 0;
	bConcave = plate->bConcave != 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// creates a lightmap texture and renders valid data into it
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectTerrainPlate::InitLightMap( CLight * Light )
{
	
	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders the plate for light map texture - it's only rendered by fixed pipeline
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGrObjectTerrainPlate::RenderLightMap()
{
	// place a plate into scene
#ifdef GROBJECT_USE_OLD_WORLDMAT_STYLE
	if ( WorldMatNeedRecount ) CountWorldMat();
#endif

	Shader->SetWorldMatrix( &WorldMat );
	Shader->SetVSMaterial( &Material );
	
	resManNS::__GrSurface *	tp = CGrObjectBase::ResourceManager->GetGrSurface( TerrainID[uiLODToRender] );

	D3DDevice->SetFVF( VertexFVF );
	D3DDevice->SetStreamSource( 0, tp->pVertices, 0, sizeof(VERTEX) );
	D3DDevice->SetIndices( tp->pIndices );

	HRESULT hr = D3DDevice->DrawIndexedPrimitive(	D3DPT_TRIANGLESTRIP, 
													0, 
													0, 
													(tp->SegsCountX + 1) * (tp->SegsCountZ + 1), 
													0, 
													(2 * tp->SegsCountX + 1) * tp->SegsCountZ - 1 );
 

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets all needed states and renders the object, then restores all saved states
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectTerrainPlate::Render( bool secondPass )
{
	LPDIRECT3DTEXTURE9			tex2, tex;
	resManNS::__Texture		*	texStruct2;
	resManNS::__Texture		*	texStruct;
	resManNS::__GrSurface	*	tp;


	// place a plate into scene
#ifdef GROBJECT_USE_OLD_WORLDMAT_STYLE
	if ( WorldMatNeedRecount ) CountWorldMat();
#endif
	
	if ( secondPass ) Shader->SetWorldMatrix( &TranslatedMatrix );
	else Shader->SetWorldMatrix( &WorldMat );

	Shader->SetVSMaterial( &Material );
	
	// get resources
	texStruct = CGrObjectBase::ResourceManager->GetTexture( TextureID );
	tp = CGrObjectBase::ResourceManager->GetGrSurface( TerrainID[uiLODToRender] );
	texStruct2 = ridRoadTexture > 0 ? CGrObjectBase::ResourceManager->GetTexture( ridRoadTexture ) : (resManNS::Texture *) 1;
		
#ifdef MY_DEBUG_VERSION
	if ( !texStruct ) ERRORMSG( ERRNOTFOUND, "CGrObjectTerrainPlate::Render()", "Invalid primary texture pointer." );
	if ( !tp ) ERRORMSG( ERRNOTFOUND, "CGrObjectTerrainPlate::Render()", "Invalid terrain plate pointer." );
	if ( !texStruct2 ) ERRORMSG( ERRNOTFOUND, "CGrObjectTerrainPlate::Render()", "Invalid secondary texture pointer." );
#endif // defined MY_DEBUG_VERSION

	tex = texStruct->texture;
	tex2 = ridRoadTexture > 0 ? texStruct2->texture : NULL;
	
	Shader->SetVSBoolean(TextureRotationHandle, bUseTextureRotation);
	Shader->SetVSBoolean(RoadRotationHandle, bUseRoadTextureRotation);

	if ( bUseTextureRotation )
		Shader->SetVSMatrix(TextureRotationMatHandle, &TextureMatrix);
	
	// set textures
	D3DDevice->SetTexture( 0, LightMapTexture );
	D3DDevice->SetTexture( 1, tex );

	// if secondary texture is used...
	if ( ridRoadTexture >= 0 )
	{
		D3DDevice->SetTexture( 2, tex2 );
		if ( bUseRoadTextureRotation )
			Shader->SetVSMatrix(RoadRotationMatHandle, &RoadTextureMatrix);
			
	}
	else
		D3DDevice->SetTexture(2, InvisibleTexture);
	
	// set mutual states
	D3DDevice->SetFVF( VertexFVF );
	D3DDevice->SetStreamSource( 0, tp->pVertices, 0, sizeof( VERTEX ) );
	D3DDevice->SetIndices( tp->pIndices );

	// render the upper terrain texture (it could be both, primary and secondary)
	//HRESULT hr = D3DDevice->DrawIndexedPrimitive(	D3DPT_TRIANGLESTRIP, 
	//												0, 
	//												0, 
	//												(tp->SegsCountX + 1) * (tp->SegsCountZ + 1), 
	//												0, 
	//												(2 * tp->SegsCountX + 1) * tp->SegsCountZ - 1 );
 
	// another way - rendering plate as a triangle list
	D3DDevice->DrawIndexedPrimitive(	D3DPT_TRIANGLELIST,
										0,
										0,
										(tp->SegsCountX + 1) * (tp->SegsCountZ + 1),
										0,
										2 * tp->SegsCountX * tp->SegsCountZ );
	


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets a material, the default material is gray
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrainPlate::SetMaterial( D3DMATERIAL9 *mtrl )
{
	Material.Ambient = mtrl->Ambient;
	Material.Diffuse = mtrl->Diffuse;
	Material.Emissive = mtrl->Emissive;
	Material.Power = mtrl->Power;
	Material.Specular = mtrl->Specular;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the rotation angle of texture
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrainPlate::SetTextureRotation( float angle )
{
	fTextureRotAngle = angle;
	ComputeTextureRotation();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the rotation angle of the road (=secondary) texture
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrainPlate::SetRoadTextureRotation( float angle )
{
	fRoadTextureRotAngle = angle;
	ComputeRoadTextureRotation();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// computes the real texture rotation angle and recomputes texture rotation matrix
//
// Implementation note:
//  -fRotationAngle has to be subtracted from the text. rotation angle so that the texture
//     keep the original direction when the field plate is rotated
//  -fTextureRotAngle has to be minus so that the texture rotation would be applied in 
//     clockwise direction
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrainPlate::ComputeTextureRotation()
{
	float	angle;

	angle = -fTextureRotAngle + fRotationAngle;
	bUseTextureRotation = ( angle != 0.0 );

	D3DXMatrixIdentity( &TextureMatrix );
	TextureMatrix(0,0) = cosf( angle );
	TextureMatrix(0,1) = sinf( angle );
	TextureMatrix(1,0) = -sinf( angle );
	TextureMatrix(1,1) = cosf( angle );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// recomputes secondary texture rotation matrix
//
// Implementation note:
//  -fRotationAngle has to be subtracted from the text. rotation angle so that the texture
//     keep the original direction when the field plate is rotated
//  -fRoadTextureRotAngle has to be minus so that the texture rotation would be applied in 
//     clockwise direction
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrainPlate::ComputeRoadTextureRotation()
{
	float	angle;

	angle = -fRoadTextureRotAngle + fRotationAngle;
	
	bUseRoadTextureRotation = (angle != 0.0);

	D3DXMatrixIdentity( &RoadTextureMatrix );
	RoadTextureMatrix(0,0) = cosf( angle );
	RoadTextureMatrix(0,1) = sinf( angle );
	RoadTextureMatrix(1,0) = -sinf( angle );
	RoadTextureMatrix(1,1) = cosf( angle );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets parameters needed for accessing visibility bit
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGrObjectTerrainPlate::SetVisibilityParams( char * bitField, char bitMask )
{
	pVisibilityBitField = bitField;
	cVisibilityBitMask = bitMask;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
