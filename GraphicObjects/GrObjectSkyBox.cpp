#include "stdafx.h"
#include "GrObjectSkyBox.h"

using namespace graphic;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrObjectSkyBox::CGrObjectSkyBox()
{
	// call inherited constructor

	// init other values
	vertexBuffer = NULL;
	for ( int i = 0; i < 6; i++ ) pTextureID[i] = -1;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGrObjectSkyBox::~CGrObjectSkyBox()
{
	if ( ResourceManager )
		for ( UINT i = 0; i < 6; i++ ) 
			if ( pTextureID[i] > 0 ) ResourceManager->ReleaseResource( pTextureID[i] );

	SAFE_RELEASE( vertexBuffer );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// inits this object
// takes fileName and loads 6 textures with names:
// fileName + '_n' - Northern texture
// fileName + '_e' - Eastern texture
// fileName + '_s' - Sousthsern texture
// fileName + '_w' - Western texture
// fileName + '_u' - Upper texture (heaven)
// fileName + '_b' - Bottom texture (under the surface)
//
// !!! note: fileName + '??' means "part of fileName before a dot + '??' + dot and the rest of it
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectSkyBox::Init(	BOOL Static,
								CString fileName,
								UINT LODcount )
{
	bool		Error = false;
	HRESULT		hr;
	int			strInd;
	CString		leftName, rightName;
	

	// use inherited initializator
	hr = this->CGrObjectBase::Init( Static );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGrObjectSkyBox::Init()", "An error occurs while initializing global parts of the object." );


	// load all six textures
	strInd = fileName.ReverseFind( '.' );

	if ( strInd >= 0 )
	{
		leftName = fileName.Left( strInd );
		rightName = fileName.Right( fileName.GetLength() - strInd );
	}
	else
	{
		leftName = fileName;
		rightName = "";
	}
	pTextureID[_SKYBOX_NORTH] =		ResourceManager->LoadResource( leftName + "_n" + rightName, RES_Texture );
	pTextureID[_SKYBOX_EAST] =		ResourceManager->LoadResource( leftName + "_e" + rightName, RES_Texture );
	pTextureID[_SKYBOX_SOUTH] =		ResourceManager->LoadResource( leftName + "_s" + rightName, RES_Texture );
	pTextureID[_SKYBOX_WEST] =		ResourceManager->LoadResource( leftName + "_w" + rightName, RES_Texture );
	pTextureID[_SKYBOX_UP] =		ResourceManager->LoadResource( leftName + "_u" + rightName, RES_Texture );
	pTextureID[_SKYBOX_BOTTOM] =	ResourceManager->LoadResource( leftName + "_b" + rightName, RES_Texture );

    // check the result
	for (UINT i = 0; i < 6; i++) if ( pTextureID[i] < 0 ) Error = true;


	// create vertex and index buffers
	hr = D3DDevice->CreateVertexBuffer( 24 * sizeof(VERTEX), D3DUSAGE_WRITEONLY, VertexFVF, D3DPOOL_MANAGED, &vertexBuffer, NULL );
	if ( hr ) Error = true;

	if ( Error )
	{
		for (UINT i = 0; i < 6; i++) if ( pTextureID[i] ) ResourceManager->ReleaseResource( pTextureID[i] );
		SAFE_RELEASE( vertexBuffer );
		ERRORMSG( ERRGENERIC, "CGrObjectSkyBox::Init()", "SkyBox initialization failed due to an error during loading a texture." )
	}


	// fill the buffers with proper data
	VERTEX	*v;
	hr = vertexBuffer->Lock( 0, 0, (LPVOID *) &v, 0 );
	if ( hr )
	{
		SAFE_RELEASE( vertexBuffer );
		ERRORMSG( hr, "CGrObjectSkyBox::Init()", "Can not lock vertex buffer to proper write operation." );
	}

	v[0].x = -0.5f; v[0].y =  0.5f; v[0].z =  0.5f; v[0].tu = 0.0f; v[0].tv = 0.0f;
	v[1].x =  0.5f; v[1].y =  0.5f; v[1].z =  0.5f; v[1].tu = 1.0f; v[1].tv = 0.0f;
	v[2].x = -0.5f; v[2].y = -0.5f; v[2].z =  0.5f; v[2].tu = 0.0f; v[2].tv = 1.0f;
	v[3].x =  0.5f; v[3].y = -0.5f; v[3].z =  0.5f; v[3].tu = 1.0f; v[3].tv = 1.0f;

	v[4].x =  0.5f; v[4].y =  0.5f; v[4].z =  0.5f; v[4].tu = 0.0f; v[4].tv = 0.0f;
	v[5].x =  0.5f; v[5].y =  0.5f; v[5].z = -0.5f; v[5].tu = 1.0f; v[5].tv = 0.0f;
	v[6].x =  0.5f; v[6].y = -0.5f; v[6].z =  0.5f; v[6].tu = 0.0f; v[6].tv = 1.0f;
	v[7].x =  0.5f; v[7].y = -0.5f; v[7].z = -0.5f; v[7].tu = 1.0f; v[7].tv = 1.0f;

	v[8].x = -0.5f; v[8].y =  0.5f; v[8].z = -0.5f; v[8].tu = 0.0f; v[8].tv = 0.0f;
	v[9].x = -0.5f; v[9].y =  0.5f; v[9].z =  0.5f; v[9].tu = 1.0f; v[9].tv = 0.0f;
	v[10].x = -0.5f; v[10].y = -0.5f; v[10].z = -0.5f; v[10].tu = 0.0f; v[10].tv = 1.0f;
	v[11].x = -0.5f; v[11].y = -0.5f; v[11].z =  0.5f; v[11].tu = 1.0f; v[11].tv = 1.0f;

	v[12].x =  0.5f; v[12].y =  0.5f; v[12].z = -0.5f; v[12].tu = 0.0f; v[12].tv = 0.0f;
	v[13].x = -0.5f; v[13].y =  0.5f; v[13].z = -0.5f; v[13].tu = 1.0f; v[13].tv = 0.0f;
	v[14].x =  0.5f; v[14].y = -0.5f; v[14].z = -0.5f; v[14].tu = 0.0f; v[14].tv = 1.0f;
	v[15].x = -0.5f; v[15].y = -0.5f; v[15].z = -0.5f; v[15].tu = 1.0f; v[15].tv = 1.0f;

	v[16].x = -0.5f; v[16].y =  0.5f; v[16].z = -0.5f; v[16].tu = 0.0f; v[16].tv = 0.0f;
	v[17].x =  0.5f; v[17].y =  0.5f; v[17].z = -0.5f; v[17].tu = 1.0f; v[17].tv = 0.0f;
	v[18].x = -0.5f; v[18].y =  0.5f; v[18].z =  0.5f; v[18].tu = 0.0f; v[18].tv = 1.0f;
	v[19].x =  0.5f; v[19].y =  0.5f; v[19].z =  0.5f; v[19].tu = 1.0f; v[19].tv = 1.0f;

	v[20].x = -0.5f; v[20].y = -0.5f; v[20].z =  0.5f; v[20].tu = 0.0f; v[20].tv = 0.0f;
	v[21].x =  0.5f; v[21].y = -0.5f; v[21].z =  0.5f; v[21].tu = 1.0f; v[21].tv = 0.0f;
	v[22].x = -0.5f; v[22].y = -0.5f; v[22].z = -0.5f; v[22].tu = 0.0f; v[22].tv = 1.0f;
	v[23].x =  0.5f; v[23].y = -0.5f; v[23].z = -0.5f; v[23].tu = 1.0f; v[23].tv = 1.0f;

	vertexBuffer->Unlock();
	
	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets all needed states and renders the object, then restores all saved state
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGrObjectSkyBox::Render()
{
	DWORD				states[3];
	LPDIRECT3DTEXTURE9	tex;
	HRESULT				hr;
	struct resManNS::__Texture *	texStruct;

#ifdef GROBJECT_USE_OLD_WORLDMAT_STYLE
	if ( WorldMatNeedRecount ) CountWorldMat();
#endif

	if ( bVertexShader )
		Shader->SetWorldMatrix( &WorldMat );
	else
		D3DDevice->SetTransform( D3DTS_WORLD, &WorldMat );

	// store and set texture stage state
	D3DDevice->GetTextureStageState( 0, D3DTSS_COLOROP, &states[0] );	
	D3DDevice->GetSamplerState( 0, D3DSAMP_ADDRESSU, &states[1] );
	D3DDevice->GetSamplerState( 0, D3DSAMP_ADDRESSV, &states[2] );

	D3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
	D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	D3DDevice->SetFVF( VertexFVF );
	D3DDevice->SetStreamSource( 0, vertexBuffer, 0, sizeof(VERTEX) );
	
	for (UINT i = 0; i < 6; i++)
	{
		texStruct = ResourceManager->GetTexture( pTextureID[i] );
#ifdef MY_DEBUG_VERSION
	if ( !texStruct ) ERRORMSG( ERRNOTFOUND, "CGrObjectMesh::Render()", "Invalid texture pointer." );
#endif // defined MY_DEBUG_VERSION
		tex = texStruct->texture;
		D3DDevice->SetTexture( 1, tex );
		hr = D3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 4*i, 2 );
	}

	D3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, states[0] );
	D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, states[1] );
	D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, states[2] );

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
