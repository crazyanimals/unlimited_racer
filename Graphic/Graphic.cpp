#include "stdafx.h"
#include "Graphic.h"
#include "..\GraphicObjects\GrObjectMesh.h"


using namespace graphic;


// Defined for easily switching to mode, where one can debug shaders
// #define SHADER_DEBUG
// Defined for analysing app with NVPerfHUD
// #define PERFORMANCE_ANALYSIS


//const DWORD SHADOWVERTEX::FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE; // posttransformed colored vertex
const DWORD SHADOWVERTEX::FVF = D3DFVF_XYZRHW | D3DFVF_TEX1; // posttransformed colored vertex
const DWORD CGraphic::LIGHTGLOWVERTEX::FVF = D3DFVF_XYZ | D3DFVF_TEX1;



//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGraphic::CGraphic()
{
	// set default values
	Direct3D = NULL;
	D3DDevice = NULL;
	MsgFont = NULL;
	DebugFont = NULL;
	pDebugTextOutput = NULL;
	Scene = NULL;
	pShadowPolygonVB = NULL;
	pLightGlowsVB = NULL;
	RIDLightGlowTexture = -1;
	ResourceManager = NULL;
	RIDLampGlowMesh = -1;
	ridBlackTexture = -1;
	pGUIDialog = NULL;
	pMouse = NULL;
	TerrainVBuffer = NULL;
	TerrainIBuffer = NULL;
	pTerrainLoDMap = NULL;
	pModelLODMap = NULL;
	pGrassLODMap = NULL;
	pShadowTexture = NULL;
	DepthStencil = NULL;
	DepthStencil2 = NULL;
	DepthTexture2 = NULL;
	TerrainTexture = NULL;
	pShadowTexture = NULL;
	InvisibleTexture = NULL;
	ShadowMap = NULL;
	pLODBorders = NULL;
	NormalMapTexture = NULL;
//	FieldOrder = new FieldStruct[500];
	uiTransparentObjCount = 0;

	// clamp vectors
	ClampVectors[0] = D3DXVECTOR4(0.0f, 0.0f, 0.5f, 0.5f);
	ClampVectors[1] = D3DXVECTOR4(0.5f, 0.0f, 1.0f, 0.5f);
	ClampVectors[2] = D3DXVECTOR4(0.0f, 0.5f, 0.5f, 1.0f);
	ClampVectors[3] = D3DXVECTOR4(0.5f, 0.5f, 1.0f, 1.0f);

	ZeroMemory( &TerrainMaterial, sizeof(D3DMATERIAL9) );
	TerrainMaterial.Diffuse = D3DXCOLOR(1.0, 1.0, 1.0, 1.0);
	TerrainMaterial.Ambient = D3DXCOLOR(1.0, 1.0, 1.0, 1.0);

	CheckpointObject = NULL;
	
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGraphic::~CGraphic()
{
	// release RM resources
	if ( RIDLightGlowTexture >= 0 ) ResourceManager->ReleaseResource( RIDLightGlowTexture );
	if ( RIDLampGlowMesh >= 0 ) ResourceManager->ReleaseResource( RIDLampGlowMesh );
	if ( ridMapLODDefinition >= 0 ) ResourceManager->ReleaseResource( ridMapLODDefinition );
	if ( ridModelLODDefinition >= 0 ) ResourceManager->ReleaseResource( ridModelLODDefinition );
	if ( ridGrassLODDefinition >= 0 ) ResourceManager->ReleaseResource( ridGrassLODDefinition );
	if ( ridBlackTexture >= 0 ) ResourceManager->ReleaseResource( ridBlackTexture );
	
	
	// release shadow volumes
	for ( SHADOWVOLUME_ITER iter = ShadowVolumes.begin(); iter != ShadowVolumes.end(); iter++ )
		SAFE_DELETE( *iter );
	ShadowVolumes.clear();
	
	
	// release D3D interfaces
	SAFE_DELETE( pMouse );
	SAFE_DELETE( pDebugTextOutput );
	SAFE_RELEASE( MsgFont );
	SAFE_RELEASE( DebugFont );
	SAFE_RELEASE( pShadowPolygonVB );
	SAFE_RELEASE( pLightGlowsVB );
	SAFE_RELEASE( StandardRenderTarget ); // Release standard render target
	SAFE_RELEASE( DepthTexture2 );
	SAFE_RELEASE( DepthStencil2); 
	SAFE_RELEASE( InvisibleTexture );
	SAFE_RELEASE( TerrainTexture );
	SAFE_RELEASE( TerrainVBuffer );
	SAFE_RELEASE( TerrainIBuffer );
	SAFE_RELEASE( pShadowTexture );
	//SAFE_RELEASE( ShaderTarget );
	SAFE_RELEASE( DepthStencil );
	SAFE_RELEASE( StandardRenderTarget );
	SAFE_RELEASE( NormalMapTexture );
	SAFE_DELETE_ARRAY( pLODBorders );
	SAFE_RELEASE(CheckpointObject );

	ShadowMap->ReleaseD3DResources();
	SAFE_DELETE(ShadowMap);

	ReleaseAllShaders();


	OUTS( "Graphic memory available: ", 2 );
	OUTI( GetAvailableTextureMem(), 2 );
	OUTSN( " kB", 2 );


	// release main D3D objects
	SAFE_RELEASE( D3DDevice );
    SAFE_RELEASE( Direct3D );


	OUTMSG( "Graphic objects released.", 1 );
	OUTS( "Total count of frames rendered: ", 2 );
	OUTIN( (int)FPSCounter.GetFramesFromStart(), 2 );
	OUTS( "Average framerate: ", 2 );
	OUTF( FPSCounter.GetFPSFromStart(), 2 );
	OUTSN( " FPS", 2 );
};


///////////////////////////////////////////////////////////
// Free all gameplay related resources
///////////////////////////////////////////////////////////
void CGraphic::Free()
{
	// Release terrain optimalization
	SAFE_RELEASE( TerrainVBuffer );
	SAFE_RELEASE( TerrainIBuffer );

	// Release terrain normal map texture
	SAFE_RELEASE(NormalMapTexture);

	// Release light maps
	ShadowMap->Free();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
//		!!!! ResourceManager must be initialized before calling this !!!!
//
// initialize window and start D3D
// obtains a pointer to valid ResourceManager and sets the graphic D3Ddevice on this manager
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::Init(	GRAPHICINIT * gInitStruc )
{
	HRESULT			hr;
	
	// custom init settings 
	hWnd = gInitStruc->hWnd;
	uiRenderableWidth	= gInitStruc->Width;
	uiRenderableHeight = gInitStruc->Height;
	uiScreenWidth = gInitStruc->Width;
	uiScreenHeight = gInitStruc->Height;
	uiColorDepth = gInitStruc->ColorDepth;
	bFullScreen = gInitStruc->FullScreen;
	ResourceManager = gInitStruc->pResourceManager;
	bLightGlowsLoaded = gInitStruc->LightGlows;
	if ( !gInitStruc->LOD0TerrainTexWidth ) uiLOD0TerrainTexWidth = DEFAULT_LOD0TERRAINTEXTURESIZE;
	else uiLOD0TerrainTexWidth = gInitStruc->LOD0TerrainTexWidth;
	if ( !gInitStruc->LOD0TerrainTexHeight ) uiLOD0TerrainTexHeight = DEFAULT_LOD0TERRAINTEXTURESIZE;
	else uiLOD0TerrainTexHeight = gInitStruc->LOD0TerrainTexHeight;
	uiAnisotropicFiltering = gInitStruc->AnisotropicFiltering;
	uiAntialiasing = gInitStruc->Antialiasing;
	uiMultisampleQuality = gInitStruc->MultisampleQuality;
	bWaitVSync = gInitStruc->WaitVSync;


	// check configuration validity
	if ( uiAnisotropicFiltering > MAX_ANISOTROPY ) ERRORMSG( ERRINVALIDCONFIGURATION, "CGraphic::Init()", "Too high anisotropy level set." );
	if ( uiAntialiasing > MAX_ANTIALIASING ) ERRORMSG( ERRINVALIDCONFIGURATION, "CGraphic::Init()", "Too high antialiasing level set." );
	
	if ( uiColorDepth == 16 ) ColorFormat = COLORFORMAT16BIT;
	else if ( uiColorDepth == 32 ) ColorFormat = COLORFORMAT32BIT;
	else ERRORMSG( ERRINVALIDCONFIGURATION, "CGraphic::Init()", "Unknown color depth set." );



	// default values and settings
	bShowingFPS = FALSE;
	bShowingDebugMsgs = FALSE;
	MsgFontSize = 16;
	DebugFontSize = 12;
	MsgFontName = "Arial";
	DebugFontName = "Arial";

	bRenderDirectionalLight = TRUE;
	bRenderAmbientLight = TRUE;
	bRenderStaticObjects = TRUE;
	bRenderDynamicObjects = TRUE;
	bDynamicLights = TRUE;
	bRenderLightGlows = FALSE;
	bRenderSkyBox = FALSE;
	bDrawMouseCursor = FALSE;

	bRenderTerrain = TRUE;
	bRenderFarObjects = TRUE;
	bMapDependentRendering = TRUE;
	
	uiShadowComplexity = SC_SIMPLE;

	DeviceType = D3DDEVTYPE_HAL;
	bHWVertexProcessing = TRUE;

	Camera.Reset ();
	D3DXMatrixIdentity( &ViewMatrix ); // set camera view matrix
	D3DXMatrixIdentity( &ProjectionMatrix ); // set projection matrix
	bViewMatrixNeedRecount = FALSE;

	RIDLightGlowTexture = -1;
	ridMapLODDefinition = -1;
	ridModelLODDefinition = -1;
	ridGrassLODDefinition = -1;
	ridBlackTexture = -1;

	pTerrainLoDMap = NULL;
	pModelLODMap = NULL;
	pGrassLODMap = NULL;

	iMapLODDefSize = 10;
	iModelLODDefSize = 10;
	iGrassLODDefSize = 3;

	fGammaFactor = 1.0f;

	D3DXMatrixIdentity( &IdentityMatrix );
	


	// Load LOD definitions and set maximal LOD number
	hr = LoadMapLODDefinition( gInitStruc->TerrainLOD );
	hr += LoadModelLODDefinition( gInitStruc->ModelLOD );
	hr += LoadGrassLODDefinition( gInitStruc->GrassLOD );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGraphic::Init()", "Some of needed LOD map initialization failed." );

	if ( uiTerrainLOD >= TERRAIN_LOD_COUNT || uiModelLOD >= MODEL_LOD_COUNT || uiGrassLOD >= MODEL_LOD_COUNT )
		ERRORMSG( ERRINVALIDCONFIGURATION, "CGraphic::Init()", "Requested LOD too high." );



	// init D3D object and do associated operations
	hr = InitD3D();
	if ( hr ) ERRORMSG( ERRGENERIC, "CGraphic::Init()", "D3D initialization failed. Can not continue in execution." );

	if ( !ResourceManager ) ERRORMSG( ERRINVALIDPARAMETER, "CGraphic::Init()", "Resource manager not initialized!" );
	ResourceManager->InitD3DDevice( D3DDevice );


	SetStandardStates();



	// get standart viewport and render target
	D3DDevice->GetViewport( &StandardViewport );
	D3DDevice->GetRenderTarget( 0, &StandardRenderTarget );



	// INIT ALL CLASSES AND STRUCTURES

	// start framerate counting
	FPSCounter.Start();

	// create font object for writing debug msgs
	MsgFont = InitDefaultFont( MsgFontSize, MsgFontName );
	DebugFont = InitDefaultFont( DebugFontSize, DebugFontName );
	if ( !MsgFont || !DebugFont ) ERRORMSG( ERRGENERIC, "CGraphic::Init()", "One or more graphic surfaces could not be created." );

	pDebugTextOutput = new CTimedTextOutput;
	if ( !pDebugTextOutput ) ERRORMSG( ERROUTOFMEMORY, "CGraphic::Init()", "" );
	hr = pDebugTextOutput->Init( DebugFont, FPSCounter.GetTimer(), 5000, DebugFontColor, 15 );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGraphic::Init()", "Text output object could not be initlialized." );

	// create shadow polygon that is used to make a shadow illusion
	hr = InitShadowPolygon();
	if ( hr ) ERRORMSG( hr, "CGraphic::Init()", "An error occurs while creating shadow polygon." );

	// create shadow texture that is used to define shadowed regions when shadow volume method is applied
	hr = D3DDevice->CreateTexture( uiRenderableWidth, uiRenderableHeight, 1, D3DUSAGE_RENDERTARGET, /*D3DFMT_R16F*/D3DFMT_A8R8G8B8,  D3DPOOL_DEFAULT , &pShadowTexture, NULL );
	if ( hr ) ERRORMSG( hr, "CGraphic::Init()", "An error occurs while creating shadow texture." );


	// init the light glow billboard
	if ( bLightGlowsLoaded )
	{
		hr = InitLightGlows();
		if ( hr ) ERRORMSG( hr, "CGraphic::Init()", "An error occurs while initializing light glows." );
	}


	// Mouse initialization
	pMouse = new CMouse( ResourceManager, D3DDevice );
	if ( !pMouse ) ERRORMSG( ERROUTOFMEMORY, "CGraphic::Init()", "Can't create mouse object." );


	// init shaders class
	hr = InitShaders();
	if (hr) ERRORMSG( hr, "CGraphic::Init()", "An error occurs while initializing shaders.");
	

	// init shadow mapping
	hr = InitShadowMapping();
	if (hr) ERRORMSG(hr, "CGraphic::Init()", "An error occurs during shadow mapping initialization." );


	// init terrain optimalization
	hr = InitTerrainOptimalization();
	if (hr) ERRORMSG(hr, "CGraphic::Init()", "An error occurs during terrain optimalization initialization.");

	
	// init HUD core
	hr = HUDCore.Init(D3DDevice, uiRenderableWidth, uiRenderableHeight);
	if (FAILED(hr) ) ERRORMSG(hr, "CGraphic::Init()", "An error occurs during HUDCore initialization.");

	// init checkpoint object
	hr = InitCheckpointObject();
	if (FAILED(hr) ) ERRORMSG(hr, "CGraphic::Init()", "Unable to init checkpoint object.");


	BoundingEnvelopes.Init( D3DDevice );


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// inits the shadow polygon vertex buffer
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::InitShadowPolygon()
{
	HRESULT			hr;

	hr = D3DDevice->CreateVertexBuffer( 4 * sizeof( SHADOWVERTEX ), 0, SHADOWVERTEX::FVF, D3DPOOL_MANAGED, &pShadowPolygonVB, NULL );
	if ( hr ) ERRORMSG( hr, "CGraphic::InitShadowPolygon()", "Shadow vertex buffer could not be created." );

	SHADOWVERTEX	*v;
	FLOAT sx = (FLOAT) this->uiRenderableWidth;
	FLOAT sy = (FLOAT) this->uiRenderableHeight;
   
	hr = pShadowPolygonVB->Lock( 0, 0, (LPVOID *) &v, 0 );
	if ( hr ) ERRORMSG( hr, "CGraphic::InitShadowPolygon()", "Shadow vertex buffer could not be locked." );
    
	v[0].pos = D3DXVECTOR4(  0, sy, 0.5f , 1 );
    v[1].pos = D3DXVECTOR4(  0,  0, 0.5f, 1 );
    v[2].pos = D3DXVECTOR4( sx, sy, 0.5f, 1 );
    v[3].pos = D3DXVECTOR4( sx,  0, 0.5f, 1 );
	//v[0].pos = D3DXVECTOR3(  0, sy, 0.5f );
 //   v[1].pos = D3DXVECTOR3(  0,  0, 0.5f );
 //   v[2].pos = D3DXVECTOR3( sx, sy, 0.5f );
 //   v[3].pos = D3DXVECTOR3( sx,  0, 0.5f );

    //v[0].color = 0x50000000;
    //v[1].color = 0x50000000;
    //v[2].color = 0x50000000;
    //v[3].color = 0x50000000;

	v[0].coord = D3DXVECTOR2( 0, 1 );
    v[1].coord = D3DXVECTOR2( 0, 0 );
    v[2].coord = D3DXVECTOR2( 1, 1 );
    v[3].coord = D3DXVECTOR2( 1, 0 );

    pShadowPolygonVB->Unlock();

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// inits the vertex buffer for lamp-light-glows billboards and light-glow texture
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::InitLightGlows()
{
	HRESULT			hr;
	LIGHTGLOWVERTEX	*v;
	
	hr = D3DDevice->CreateVertexBuffer( 4 * sizeof( LIGHTGLOWVERTEX ), 
										D3DUSAGE_WRITEONLY, 
										LIGHTGLOWVERTEX::FVF, 
										D3DPOOL_MANAGED, 
										&pLightGlowsVB, 
										NULL );
	if ( hr ) ERRORMSG( hr, "CGraphic::InitLightGlows()", "Unable to create vertex buffer for light billboard." );

	hr = pLightGlowsVB->Lock( 0, 0, (LPVOID *) &v, 0 );
	if ( hr ) ERRORMSG( hr, "CGraphic::InitLightGlows()", "Unable to fill the vertex buffer with light billboard data." );

	v[0].x = -50; v[0].y =  50; v[0].z = 0; v[0].tu = 0.0f; v[0].tv = 0.0f;
	v[1].x =  50; v[1].y =  50; v[1].z = 0; v[1].tu = 1.0f; v[1].tv = 0.0f;
	v[2].x = -50; v[2].y = -50; v[2].z = 0; v[2].tu = 0.0f; v[2].tv = 1.0f;
	v[3].x =  50; v[3].y = -50; v[3].z = 0; v[3].tu = 1.0f; v[3].tv = 1.0f;

	pLightGlowsVB->Unlock();

	RIDLightGlowTexture = ResourceManager->LoadResource( "glow.png", RES_Texture );
	if ( RIDLightGlowTexture < 0 ) ERRORMSG( ERRGENERIC, "CGraphic::InitLightGlows()", "An error occurs while loading light glow texture." ); 

//	RIDLampGlowMesh = ResourceManager->LoadResource( "glow.x", RES_XFILE );
//	if ( RIDLampGlowMesh < 0 ) ERRORMSG( ERRGENERIC, "CGraphic::InitLightGlows()", "An error occurs while loading light glow mesh." ); 

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// inits the font using default parameters, but the font size and face name
// face name should not be longer then 31 characters, it is truncated in that case
//
// returns nonzero pointer to a successfully created font or a zero value when an error occurs
//
//////////////////////////////////////////////////////////////////////////////////////////////
LPD3DXFONT CGraphic::InitDefaultFont( int size, LPCTSTR name )
{
	D3DXFONT_DESC	fontDesc = { size, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_TT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH, "" };
	LPD3DXFONT		font;

	for ( UINT i = 0; i < 31 && i < strlen(name); i++ ) fontDesc.FaceName[i] = name[i];
	fontDesc.FaceName[31] = 0;

	
	if ( !(font = InitFont( &fontDesc )) )
	{
		ErrorHandler.HandleError( ERRGENERIC, "CGraphic::InitDefaultFont()", "Font object could not be created." );
		return NULL;
	}

	return font;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// inits the font using specified font description pattern
// returns nonzero pointer to a successfully created font or a zero value when an error occurs
//
//////////////////////////////////////////////////////////////////////////////////////////////
LPD3DXFONT CGraphic::InitFont( D3DXFONT_DESC *desc )
{
	HRESULT		hr;
	LPD3DXFONT	font;

	//LOGFONT	logFont = {20,0,0,0,FW_NORMAL,false,false,false,DEFAULT_CHARSET,OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,DEFAULT_PITCH,_T("Arial")};
	hr = D3DXCreateFontIndirect( D3DDevice, desc, &font );
	if ( hr )
	{
		ErrorHandler.HandleError( hr, "CGraphic::Init()", "Font object could not be created." );
		return NULL;
	}

	return font;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the gamma factor for default swapchain
// this changes the gamma correction linearly and the same for all
// the original and default falue for scale is 1.0; the higher the scale value is, that
//   more bright the output image is
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetGamma( float scale )
{
	D3DGAMMARAMP ramp;

	for ( int i = 0; i < 256; i++ )
	{
		ramp.red[i] = (WORD) min( 65535.0f, ((float) (i << 8) * scale));
		ramp.green[i] = ramp.red[i];
		ramp.blue[i] = ramp.red[i];
	}

	D3DDevice->SetGammaRamp( 0, D3DSGR_NO_CALIBRATION, &ramp );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the gamma factor for default swapchain
// this changes the gamma correction linearly and the same for all
// 'percent' specifies percentage from the default value, that means 100 is the original value
//   and the higher the percent value is, that more bright the output image is
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetGamma( int percent )
{
	D3DGAMMARAMP ramp;

	for ( int i = 0; i < 256; i++ )
	{
		ramp.red[i] = (WORD) min( 65535, (i << 8) * percent );
		ramp.green[i] = ramp.red[i];
		ramp.blue[i] = ramp.red[i];
	}

	D3DDevice->SetGammaRamp( 0, D3DSGR_NO_CALIBRATION, &ramp );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the gamma factor for default swapchain
// this changes the gamma correction linearly, but independetly for each channel 
// the original and default falue for scale is 1.0; the higher the scale value is, that
//   more bright the output image is
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetGammaRGB( float red, float green, float blue )
{
	D3DGAMMARAMP ramp;

	for ( int i = 0; i < 256; i++ )
	{
		ramp.red[i] = (WORD) min( 65535.0f, ((float) (i << 8) * red) );
		ramp.green[i] = (WORD) min( 65535.0f, ((float) (i << 8) * green) );
		ramp.blue[i] = (WORD) min( 65535.0f, ((float) (i << 8) * blue) );
	}

	D3DDevice->SetGammaRamp( 0, D3DSGR_NO_CALIBRATION, &ramp );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the gamma factor for default swapchain
// this changes the gamma correction linearly, but independetly for each channel 
// the original and default falue for scale is 1.0
// the higher the scale value is, that more bright the output image is; however, if the computed
//   value is higher than 65535, it's wrapped around zero (which means 65537 == 2) 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetGammaRotate( float red, float green, float blue )
{
	D3DGAMMARAMP ramp;

	for ( int i = 0; i < 256; i++ )
	{
		ramp.red[i] = (WORD) ((float) (i << 8) * red);
		ramp.green[i] = (WORD) ((float) (i << 8) * green);
		ramp.blue[i] = (WORD) ((float) (i << 8) * blue);
	}

	D3DDevice->SetGammaRamp( 0, D3DSGR_NO_CALIBRATION, &ramp );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// releases the font object (surface) and than tries to create it once more with a new font size
//
// returns a nonzero value when an error occurs
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::SetMsgFontSize( int size )
{
	MsgFontSize = size;
	SAFE_RELEASE( MsgFont );
	MsgFont = InitDefaultFont( MsgFontSize, MsgFontName );
	if ( !MsgFont ) ERRORMSG( ERRGENERIC, "CGraphic::Init()", "An error occurs when attempting to recreate font object." )

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// releases the font object (surface) and than tries to create it once more with a new font size
//
// returns a nonzero value when an error occurs
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::SetDebugFontSize( int size )
{
	DebugFontSize = size;
	SAFE_RELEASE( DebugFont );
	DebugFont = InitDefaultFont( DebugFontSize, DebugFontName );
	if ( !DebugFont ) ERRORMSG( ERRGENERIC, "CGraphic::Init()", "An error occurs when attempting to recreate font object." );

	pDebugTextOutput->SetFont( DebugFont );

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// releases the font object (surface) and than tries to create it once more with a new face name
//
// returns a nonzero value when an error occurs
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::SetMsgFontName( LPCTSTR name )
{
	MsgFontName = name;
	SAFE_RELEASE( MsgFont );
	MsgFont = InitDefaultFont( MsgFontSize, MsgFontName );
	if ( !MsgFont ) ERRORMSG( ERRGENERIC, "CGraphic::Init()", "An error occurs when attempting to recreate font object." )

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// releases the font object (surface) and than tries to create it once more with a new font name
//
// returns a nonzero value when an error occurs
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::SetDebugFontName( LPCTSTR name )
{
	DebugFontName = name;
	SAFE_RELEASE( DebugFont );
	DebugFont = InitDefaultFont( DebugFontSize, DebugFontName );
	if ( !DebugFont ) ERRORMSG( ERRGENERIC, "CGraphic::Init()", "An error occurs when attempting to recreate font object." )

	pDebugTextOutput->SetFont( DebugFont );

	return ERRNOERROR;
};
	

//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a debug message to the msgs list
// if the timeout is set to zero (or unused), the default timeout is used instead
// (timeout is specified in miliseconds)
// if the msg color is set to zero (or unused), the default color is used instead
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::AddDebugMessage( LPCTSTR text, D3DCOLOR col, APPRUNTIME timeout )
{
	pDebugTextOutput->AddMessage( text, col, timeout );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets rendering parameters for timed debug messages
//
// x,y are coordinates of clipping rectangle
// format specifies text output format, could contain these flags:
//	  DT_LEFT, DT_CENTER, DT_RIGHT
//    DT_TOP, DT_VCENTER, DT_BOTTOM
//    DT_NOCLIP, DT_WORDBREAK, DT_SINGLELINE
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetDebugMsgParams( int x1, int y1, int x2, int y2, DWORD format )
{
	RECT	rect;
	
	rect.left = x1;
	rect.right = x2;
	rect.bottom = y2;
	rect.top = y1;

	pDebugTextOutput->SetRenderParams( &rect, format );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// initializes D3D structures; called from Init after obtaining optional settings
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::InitD3D()
{
	HRESULT hr1, hr2;
	
	// check for D3D version
	Direct3D = Direct3DCreate9( D3D_SDK_VERSION );
	if ( !Direct3D ) ERRORMSG( ERRD3DGENERIC, "CGraphic::InitD3D()", "Cannot create D3D object." );


	GetDeviceCaps(); // fills D3DCaps structure with valid capability data

	// Get adapter identifier information
	hr1 = Direct3D->GetAdapterIdentifier( D3DADAPTER_DEFAULT, 0, &AdapterIdentifier );
	if ( FAILED(hr1) )
		ERRORMSG(hr1, "CGraphic::InitD3D()", "Unable to get adapter identifier.");

	
	// enumerate all availabel modes
	hr1 = Modes16bpp.Init( Direct3D, Direct3D->GetAdapterModeCount( D3DADAPTER_DEFAULT, COLORFORMAT16BIT ), COLORFORMAT16BIT );
	hr2 = Modes32bpp.Init( Direct3D, Direct3D->GetAdapterModeCount( D3DADAPTER_DEFAULT, COLORFORMAT32BIT ), COLORFORMAT32BIT );
	if ( hr1 || hr2 )
	{
		ErrorHandler.HandleError( ERRGENERIC, "CGraphic::InitD3D()", "Display modes could not be enumerated" );
		return hr1 ? hr1 : hr2;
	}
	Modes16bpp.EnumerateModes(); 
	Modes32bpp.EnumerateModes();




	// SET UP ALL THE CONFIGURATION
	uiAdapter = D3DADAPTER_DEFAULT;
	DeviceType = D3DDEVTYPE_HAL;
	DepthStencilFormat = D3DFMT_D24S8;
	BackBufferFormat = bFullScreen ? ColorFormat : GetAdapterColorFormat();
	bHWVertexProcessing = CanUseHWVertexProcessing(); // check D3DCaps structure for needed HW features
	dwVertexProcessing = bHWVertexProcessing ? D3DCREATE_MIXED_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	dwDeviceBehaviorFlags = dwVertexProcessing | D3DCREATE_MULTITHREADED;

	// check out possible modifications
#ifdef PERFORMANCE_ANALYSIS
	uiAdapter = Direct3D->GetAdapterCount() - 1;
	DeviceType = D3DDEVTYPE_REF;
#endif
#ifdef SHADER_DEBUG
	DeviceType = D3DDEVTYPE_REF;
#endif



#ifdef MY_DEBUG_VERSION
	// debug output
	if ( DeviceType == D3DDEVTYPE_HAL ) { OUTMSG( "Device type used: HAL.", 1 ) }
	else if ( DeviceType == D3DDEVTYPE_SW ) { OUTMSG( "Device type used: SW.", 1 ) }
	else if ( DeviceType == D3DDEVTYPE_REF ) { OUTMSG( "Device type used: REF.", 1 ) }
	if ( bHWVertexProcessing ) { OUTMSG( "Vertex processing done in HW.", 1 ) }
	else { OUTMSG( "Vertex processing emulated by SW.", 1 ) }
#endif // defined MY_DEBUG_VERSION



	// CHECK SETTINGS VALIDITY !!!
	if ( !CheckSettings() )
		ERRORMSG( ERRGENERIC, "CGraphic::InitD3D()", "Setting doesn't correspond to hardware capabilities." );




	// set requested D3D device parameters
	ZeroMemory( &D3DPP, sizeof(D3DPP) );
	D3DPP.Windowed = !bFullScreen;
	D3DPP.EnableAutoDepthStencil = false;
	D3DPP.AutoDepthStencilFormat = DepthStencilFormat;
	D3DPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
	//D3DPP.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	D3DPP.Flags = 0;
	D3DPP.PresentationInterval = bWaitVSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	D3DPP.BackBufferHeight = uiRenderableHeight;
	D3DPP.BackBufferWidth = uiRenderableWidth;
	D3DPP.BackBufferFormat = BackBufferFormat;
	D3DPP.FullScreen_RefreshRateInHz = 0;
	GetMultisampleType( &D3DPP.MultiSampleType, &D3DPP.MultiSampleQuality );


	// obtain D3D device
	hr1 = Direct3D->CreateDevice( uiAdapter, DeviceType, hWnd, dwDeviceBehaviorFlags, &D3DPP, &D3DDevice );
	if ( hr1 ) ERRORMSG( hr1, "CGraphic::InitD3D()", "Main Direct3D device could not be created." );
	

	OUTS( "Graphic memory available: ", 2 );
	OUTI( GetAvailableTextureMem(), 2 );
	OUTSN( " kB", 2 );

	// Initialize depth/stencil buffers
	hr1 = InitDepthStencils();
	if ( FAILED(hr1) ) ERRORMSG( hr1, "CGraphic::InitD3D()", "Unable to initialize depth/stencil buffers." );



	// Create invisible texture - 1x1 texture with all channels 0
	hr1 = D3DDevice->CreateTexture( 1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &InvisibleTexture, 0 );
	if ( FAILED(hr1) ) ERRORMSG( hr1, "CGraphic.Init3D()", "Unable to create invisible texture." );

	// write 0 into the invisible texture
	D3DLOCKED_RECT Rect;
	InvisibleTexture->LockRect( 0, &Rect, 0, D3DLOCK_NOSYSLOCK );
	(* ((DWORD*) Rect.pBits)) = (DWORD) 0;
	InvisibleTexture->UnlockRect( 0 );


	return ERRNOERROR;
};


/////////////////////////////////////////////////////////////////////////////////////////////
//
// Inits depth-stencil buffers
//
/////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::InitDepthStencils()
{
	HRESULT				hr;
	D3DMULTISAMPLE_TYPE	ms_type;
	DWORD				ms_level;
	
	
	// Create depth stencil
	GetMultisampleType( &ms_type, &ms_level );

	SAFE_RELEASE( DepthStencil );
	
	/*hr = D3DDevice->CreateDepthStencilSurface(	SM_MAX,
												SM_MAX,
												D3DFMT_D24S8, ms_type, ms_level, false, &DepthStencil, 0 );*/
	
	hr = D3DDevice->CreateDepthStencilSurface(	uiRenderableWidth,
												uiRenderableHeight,
												D3DFMT_D24S8, ms_type, ms_level, false, &DepthStencil, 0 );
	if ( FAILED(hr) ) ERRORMSG( hr, "CGraphic::InitDepthStencil()", "Unable to create depth/stencil surface." );
		
	hr = D3DDevice->SetDepthStencilSurface( DepthStencil );
	if ( hr ) ERRORMSG( hr, "CGraphic::InitDepthStencil()", "Unable to set up depth surface." );
	
	
	// second depth stencil
#ifndef SHADER_DEBUG // ref device doesn't support NV HW shadow maps
	SAFE_RELEASE(DepthTexture2);
	SAFE_RELEASE(DepthStencil2);
	if ( !Direct3D->CheckDeviceFormat( uiAdapter, DeviceType, BackBufferFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, D3DFMT_D24S8 ) )
	{
		// create depth texture and get surface from it
		hr = D3DDevice->CreateTexture(SM_MAX, SM_MAX, 1, D3DUSAGE_DEPTHSTENCIL, D3DFMT_D24S8, D3DPOOL_DEFAULT, &DepthTexture2, 0);
		
		if ( FAILED(hr) )
			ERRORMSG(hr, "CGraphic.InitDepthStencil()", "Unable to create depth/stencil texture.");
		
		// get depth surface
		DepthTexture2->GetSurfaceLevel(0, &DepthStencil2);
	}
	else
	{
		hr = D3DDevice->CreateDepthStencilSurface(SM_MAX, SM_MAX,D3DFMT_D24S8, 
												   D3DMULTISAMPLE_NONE, 0, false, &DepthStencil2, 0);
		if ( FAILED(hr) )
			ERRORMSG(hr, "CGraphic.InitDepthStencils()", "Unable to create depth/stencil surface.");
		
	}
#else
		hr = D3DDevice->CreateDepthStencilSurface(SM_MAX, SM_MAX,D3DFMT_D24S8, 
												   D3DMULTISAMPLE_NONE, 0, false, &DepthStencil2, 0);
		if ( FAILED(hr) )
			ERRORMSG(hr, "CGraphic.InitDepthStencils()", "Unable to create depth/stencil surface.");
#endif

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns the multisample type and quality level actually configured
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::GetMultisampleType( D3DMULTISAMPLE_TYPE * type, DWORD * level )
{
	*level = 0;
	
	switch ( uiAntialiasing )
	{
	case 0: *type = D3DMULTISAMPLE_NONE; break;
	case 1: *type = D3DMULTISAMPLE_NONMASKABLE; *level = uiMultisampleQuality; break;
	case 2: *type = D3DMULTISAMPLE_2_SAMPLES; break;
	case 3: *type = D3DMULTISAMPLE_3_SAMPLES; break;
	case 4: *type = D3DMULTISAMPLE_4_SAMPLES; break;
	case 5: *type = D3DMULTISAMPLE_5_SAMPLES; break;
	case 6: *type = D3DMULTISAMPLE_6_SAMPLES; break;
	case 7: *type = D3DMULTISAMPLE_7_SAMPLES; break;
	case 8: *type = D3DMULTISAMPLE_8_SAMPLES; break;
	case 9: *type = D3DMULTISAMPLE_9_SAMPLES; break;
	case 10: *type = D3DMULTISAMPLE_10_SAMPLES; break;
	case 11: *type = D3DMULTISAMPLE_11_SAMPLES; break;
	case 12: *type = D3DMULTISAMPLE_12_SAMPLES; break;
	case 13: *type = D3DMULTISAMPLE_13_SAMPLES; break;
	case 14: *type = D3DMULTISAMPLE_14_SAMPLES; break;
	case 15: *type = D3DMULTISAMPLE_15_SAMPLES; break;
	case 16: *type = D3DMULTISAMPLE_16_SAMPLES; break;
	default: *type = D3DMULTISAMPLE_NONE;
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns true if chosen settings is supported by HW, otherwise returns an error code and
//   ends immediately
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGraphic::CheckSettings()
{
#define _CHECK_ERROR_CS(hr,er,msg) if ( hr ) { ErrorHandler.HandleError( er, "CGraphic::CheckSettings()", msg ); return FALSE; }

	BOOL				found;
	CGraphicModes	*	pMode = ( uiColorDepth == 16 ? &Modes16bpp : &Modes32bpp );
	D3DMULTISAMPLE_TYPE	ms_type;
	DWORD				ms_level, ms_levels_count;
	HRESULT				hr;

	
	// check for multisampling support
	GetMultisampleType( &ms_type, &ms_level );
	
	hr = Direct3D->CheckDeviceMultiSampleType( uiAdapter, DeviceType, BackBufferFormat, !bFullScreen, ms_type, &ms_levels_count );
	_CHECK_ERROR_CS( hr, hr, "Your graphic card doesn't support requested multisampling type." );
    _CHECK_ERROR_CS( ms_type == D3DMULTISAMPLE_NONMASKABLE && ms_level >= ms_levels_count, ERRINVALIDCONFIGURATION, "Your graphic card doesn't support requested multisampling quality level." );

	hr = Direct3D->CheckDeviceMultiSampleType( uiAdapter, DeviceType, DepthStencilFormat, !bFullScreen, ms_type, &ms_levels_count );
	_CHECK_ERROR_CS( hr, hr, "Your graphic card doesn't support requested multisampling type." );
    _CHECK_ERROR_CS( ms_type == D3DMULTISAMPLE_NONMASKABLE && ms_level >= ms_levels_count, ERRINVALIDCONFIGURATION, "Your graphic card doesn't support requested multisampling quality level." );



	// check for shader 2.0 support
	bShaderSupported = D3DCaps.VertexShaderVersion >= D3DVS_VERSION(2, 0) && D3DCaps.PixelShaderVersion >= D3DPS_VERSION(2, 0);
	_CHECK_ERROR_CS( !bShaderSupported, ERRNOTFOUND, "Your graphic card doesn't support 2.0 shaders. This program will never run on such crap." );



	// check for compatible graphic mode
	found = false;
	for ( DWORD i = 0; i < pMode->cModes; i++ )
	{
		if ( (pMode->Format == ColorFormat) && (pMode->pModes[i].Width == uiRenderableWidth) && 
			(pMode->pModes[i].Height == uiRenderableHeight) ) { found = true; break; }
	}
	if ( !bFullScreen ) found = true;

	_CHECK_ERROR_CS( !found, ERRINVALIDCONFIGURATION, "No compatible graphic mode found." );



	// check for device type existency
	hr = Direct3D->CheckDeviceType(	uiAdapter, DeviceType, ColorFormat, BackBufferFormat, !bFullScreen );
	_CHECK_ERROR_CS( hr, hr, "Your graphic card doesn't support requested color/depth/device format." );

										
	
	// Verify that the depth format exists
    hr = Direct3D->CheckDeviceFormat(	uiAdapter,
										DeviceType,
										ColorFormat,
										D3DUSAGE_DEPTHSTENCIL,
										D3DRTYPE_SURFACE,
										DepthStencilFormat );
	_CHECK_ERROR_CS( hr, hr, "No compatible depth/stencil buffer format found." );



    // Verify that the depth format is compatible with back and color buffers
    hr = Direct3D->CheckDepthStencilMatch( uiAdapter, DeviceType, ColorFormat, BackBufferFormat, DepthStencilFormat );
	_CHECK_ERROR_CS( hr, hr, "No compatible depth/stencil buffer format found." );



	// check for simultaneous texture stages
	CGrObjectBase::MaxTextureStages = D3DCaps.MaxSimultaneousTextures;
	_CHECK_ERROR_CS( D3DCaps.MaxSimultaneousTextures < 5, ERRNOTFOUND, "Your graphic card doesn't support 8 simultaneous textures. Please, contact the developers team." );


	return TRUE;

#undef _CHECK_ERROR_CS
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns the D3DFORMAT of display mode actually used on the default (or other, if specified)
//   monitor
//
//////////////////////////////////////////////////////////////////////////////////////////////
D3DFORMAT CGraphic::GetAdapterColorFormat( UINT adapter )
{
	D3DDISPLAYMODE mode;

	if ( SUCCEEDED( Direct3D->GetAdapterDisplayMode( adapter, &mode ) ) ) return mode.Format;
	else return D3DFMT_UNKNOWN;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Determines which LOD to render on the base of viewer position and the LOD settings
// just specify the coordinates of the object in the map and this returns the LOD index to use
//
//////////////////////////////////////////////////////////////////////////////////////////////
UINT CGraphic::GetGrassLODToUse( int x, int z )
{
	int					iX, iZ;
	
	if ( !pGrassLODMap ) return uiGrassLOD;

	iX =  pGrassLODMap->iWidth / 2 - (x - MapPosition.PosX);
	iZ =  pGrassLODMap->iHeight / 2 - (z - MapPosition.PosZ);
	if ( iX < 0 || iZ < 0 || iX >= pGrassLODMap->iWidth || iZ >= pGrassLODMap->iHeight ) return 0;

	return (UINT) pGrassLODMap->pData[iZ * pGrassLODMap->iWidth + iX];
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Determines which LOD to render on the base of viewer position and the LOD settings
// just specify the coordinates of the object in the map and this returns the LOD index to use
//
//////////////////////////////////////////////////////////////////////////////////////////////
UINT CGraphic::GetModelLODToUse( int x, int z )
{
	int					iX, iZ;
	
	if ( !pModelLODMap ) return uiModelLOD;

	iX =  pModelLODMap->iWidth / 2 - (x - MapPosition.PosX);
	iZ =  pModelLODMap->iHeight / 2 - (z - MapPosition.PosZ);
	if ( iX < 0 || iZ < 0 || iX >= pModelLODMap->iWidth || iZ >= pModelLODMap->iHeight ) return 0;

	return (UINT) pModelLODMap->pData[iZ * pModelLODMap->iWidth + iX];
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Determines which LOD to render on the base of viewer position and the LOD settings
// just specify the coordinates of a certain filed (terrain plate) in the map and this
// returns the LOD index to use
//
//////////////////////////////////////////////////////////////////////////////////////////////
UINT CGraphic::GetTerrainLODToUse( int x, int z )
{
	int					iX, iZ;
	
	//if ( !pTerrainLoDMap ) return uiTerrainLOD;

	iX =  pTerrainLoDMap->iWidth / 2 - (x - MapPosition.PosX);
	iZ =  pTerrainLoDMap->iHeight / 2 - (z - MapPosition.PosZ);
	if ( iX < 0 || iZ < 0 || iX >= pTerrainLoDMap->iWidth || iZ >= pTerrainLoDMap->iHeight ) return 0;

	return (UINT) pTerrainLoDMap->pData[iZ * pTerrainLoDMap->iWidth + iX];
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads the LODMap for map rendering
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::LoadMapLODDefinition( LPCTSTR fileName )
{
	ridMapLODDefinition = ResourceManager->LoadResource( fileName, RES_LoDMap );
	
	if ( ridMapLODDefinition < 1 ) 
		ERRORMSG( ERRGENERIC, "CGraphic::LoadMapLODDefinition()", "Failed to load map LOD definition." );

	pTerrainLoDMap = ResourceManager->GetLoDMap( ridMapLODDefinition );
	if ( !pTerrainLoDMap )
		ERRORMSG( ERRGENERIC, "CGraphic::LoadMapLODDefinition()", "Unable to acquire terrain LOD definition from resource manager." );

	iMapLODDefSize = max( pTerrainLoDMap->iWidth, pTerrainLoDMap->iHeight );


	// compute maximal used Terrain LOD
	uiTerrainLOD = 0;
	for ( int i = 0; i < pTerrainLoDMap->iHeight; i++ )
		for ( int j = 0; j < pTerrainLoDMap->iWidth; j++ )
			if ( pTerrainLoDMap->pData[i*pTerrainLoDMap->iWidth + j] > (int) uiTerrainLOD )
				uiTerrainLOD = pTerrainLoDMap->pData[i*pTerrainLoDMap->iWidth + j];


	
	// INIT LOD BORDERS MAP
	SAFE_DELETE_ARRAY( pLODBorders );

	pLODBorders = new bool[pTerrainLoDMap->iWidth * pTerrainLoDMap->iHeight];
	if ( !pLODBorders )
		ERRORMSG( ERRGENERIC, "CGraphic::LoadMapLODDefinition()", "Unable to create auxiliary terrain structure." );
	
	// reset all fields
	for ( int iz = 0; iz < pTerrainLoDMap->iHeight; iz++ )
		for ( int ix = 0; ix < pTerrainLoDMap->iWidth; ix++ )
			pLODBorders[iz * pTerrainLoDMap->iWidth + ix] = false;

	// check the farther field
	for ( int iz = 0; iz < pTerrainLoDMap->iHeight - 1; iz++ )
	{
		for ( int ix = 0; ix < pTerrainLoDMap->iWidth; ix++ )
		{
			int index = iz * pTerrainLoDMap->iWidth + ix;
			if ( pTerrainLoDMap->pData[index + pTerrainLoDMap->iWidth] > pTerrainLoDMap->pData[index] )
				 pLODBorders[index] = true;
		}
	}

	// check the nearer field
	for ( int iz = 1; iz < pTerrainLoDMap->iHeight; iz++ )
	{
		for ( int ix = 0; ix < pTerrainLoDMap->iWidth; ix++ )
		{
			int index = iz * pTerrainLoDMap->iWidth + ix;
			if ( pTerrainLoDMap->pData[index - pTerrainLoDMap->iWidth] > pTerrainLoDMap->pData[index] )
				 pLODBorders[index] = true;
		}
	}

	// check the field on the left
	for ( int iz = 0; iz < pTerrainLoDMap->iHeight; iz++ )
	{
		for ( int ix = 1; ix < pTerrainLoDMap->iWidth; ix++ )
		{
			int index = iz * pTerrainLoDMap->iWidth + ix;
			if ( pTerrainLoDMap->pData[index - 1] > pTerrainLoDMap->pData[index] )
				 pLODBorders[index] = true;
		}
	}

	// check the field on the right
	for ( int iz = 0; iz < pTerrainLoDMap->iHeight; iz++ )
	{
		for ( int ix = 0; ix < pTerrainLoDMap->iWidth - 1; ix++ )
		{
			int index = iz * pTerrainLoDMap->iWidth + ix;
			if ( pTerrainLoDMap->pData[index + 1] > pTerrainLoDMap->pData[index] )
				 pLODBorders[index] = true;
		}
	}

#ifdef MY_DEBUG_VERSION	
	int cnt = 0;
	for ( int iz = 0; iz < pTerrainLoDMap->iHeight; iz++ )
		for ( int ix = 0; ix < pTerrainLoDMap->iWidth; ix++ )
			if ( pLODBorders[iz * pTerrainLoDMap->iWidth + ix] ) cnt++;

	OUTS( "Terrain LOD map changed. New count of LOD borders: ", 2 );
	OUTIN( cnt, 2 );
#endif 


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads the LODMap for models rendering
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::LoadModelLODDefinition( LPCTSTR fileName )
{
	ridModelLODDefinition = ResourceManager->LoadResource( fileName, RES_LoDMap );
	
	if ( ridModelLODDefinition < 1 ) 
		ERRORMSG( ERRGENERIC, "CGraphic::LoadModelLODDefinition()", "Failed to load models LOD definition." );

	pModelLODMap = ResourceManager->GetLoDMap( ridModelLODDefinition );
	if ( !pModelLODMap )
		ERRORMSG( ERRGENERIC, "CGraphic::LoadModelLODDefinition()", "Unable to acquire models LOD definition from resource manager." );

	iModelLODDefSize = max( pModelLODMap->iHeight, pModelLODMap->iWidth );
	
	// compute maximal used model LOD
	uiModelLOD = 0;
	for ( int i = 0; i < pModelLODMap->iHeight; i++ )
		for ( int j = 0; j < pModelLODMap->iWidth; j++ )
			if ( pModelLODMap->pData[i*pModelLODMap->iWidth + j] > (int) uiModelLOD )
				uiModelLOD = pModelLODMap->pData[i*pModelLODMap->iWidth + j];


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads the LODMap for grass rendering
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::LoadGrassLODDefinition( LPCTSTR fileName )
{
	ridGrassLODDefinition = ResourceManager->LoadResource( fileName, RES_LoDMap );
	
	if ( ridGrassLODDefinition < 1 ) 
		ERRORMSG( ERRGENERIC, "CGraphic::LoadGrassLODDefinition()", "Failed to load grass LOD definition." );

	pGrassLODMap = ResourceManager->GetLoDMap( ridGrassLODDefinition );
	if ( !pGrassLODMap )
		ERRORMSG( ERRGENERIC, "CGraphic::LoadGrassLODDefinition()", "Unable to acquire grass LOD definition from resource manager." );

	iGrassLODDefSize = max( pGrassLODMap->iHeight, pGrassLODMap->iWidth );
	
	// compute maximal used grass LOD
	uiGrassLOD = 0;
	for ( int i = 0; i < pGrassLODMap->iHeight; i++ )
		for ( int j = 0; j < pGrassLODMap->iWidth; j++ )
			if ( pGrassLODMap->pData[i*pGrassLODMap->iWidth + j] > (int) uiGrassLOD )
				uiGrassLOD = pGrassLODMap->pData[i*pGrassLODMap->iWidth + j];


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders whole scene without the map dependency
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderWithoutMap()
{
	HRESULT					hr;
	DWORD					color;
	LIGHTS_ITER				lightIter;
	GROBJECTS_ITER			objIter;
	D3DCOLORVALUE			ambLightCol;


	hr = PrepareScene();

	// RENDER SCENE WITH ONLY AMBIENT AND SUN LIGHT

	D3DDevice->LightEnable( 0, FALSE );


	// compute ambient light
	ambLightCol.r = Scene->AmbientLight->Ambient.r;
	ambLightCol.g = Scene->AmbientLight->Ambient.g;
	ambLightCol.b = Scene->AmbientLight->Ambient.b;
	ambLightCol.a = Scene->AmbientLight->Ambient.a;

	__SETTLE_BETWEEN( ambLightCol.r, 0.0f, 1.0f );
	__SETTLE_BETWEEN( ambLightCol.g, 0.0f, 1.0f );
	__SETTLE_BETWEEN( ambLightCol.b, 0.0f, 1.0f );
	__SETTLE_BETWEEN( ambLightCol.a, 0.0f, 1.0f );
	
	color = ( (DWORD)(255*ambLightCol.a) << 24 ) + ( (DWORD)(255*ambLightCol.r) << 16 ) + ( (DWORD)(255*ambLightCol.g) << 8 ) + (DWORD)(255*ambLightCol.b);
	
	D3DDevice->SetRenderState( D3DRS_AMBIENT, color );

	
	// render all static and dynamic objects in a scene list
	for ( objIter = Scene->StaticObjects.begin(); objIter != Scene->StaticObjects.end(); objIter++ ) (*objIter)->Render();
	for ( objIter = Scene->DynamicObjects.begin(); objIter != Scene->DynamicObjects.end(); objIter++) (*objIter)->Render();

	hr = PresentScene();

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders static objects in target square area or all static objects in whole map if the 
//   pos parameter is NULL
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderStaticObjects( MAPPOSITION *pos, int range )
{
 	GROBJECTS_ITER			objIter;

	if ( bMapDependentRendering )
	{
        for ( objIter = Scene->StaticObjects.begin(); objIter != Scene->StaticObjects.end(); objIter++ )
			if ( Scene->Map.ObjectIsVisible( (*objIter)->GetMapPosList() ) ) 
				if ( !pos || (*objIter)->InterferesToSquareArea( pos, range ) )
				{
					D3DXVECTOR3 vec = (*objIter)->GetPosition();
					((CGrObjectMesh *) *objIter)->SetLOD( GetModelLODToUse( ((int) vec.x) / 1000, ((int) vec.z) / 1000 ) );
					(*objIter)->Render();
				}
	}
	else
	{
        for ( objIter = Scene->StaticObjects.begin(); objIter != Scene->StaticObjects.end(); objIter++ )
			(*objIter)->Render();
	}

	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders static objects in target rectangular area 
// 
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderStaticObjects( RECT & Rect, BOOL VisibleTest )
{
 	GROBJECTS_ITER			objIter;

	for ( objIter = Scene->StaticObjects.begin(); objIter != Scene->StaticObjects.end(); objIter++ )
		if ( !VisibleTest || Scene->Map.ObjectIsVisible( (*objIter)->GetMapPosList() ) ) 
			if ( (*objIter)->InterferesToRectangularArea(Rect.left, Rect.top, Rect.right, Rect.bottom) )
			{
				D3DXVECTOR3 vec = (*objIter)->GetPosition();
				((CGrObjectMesh *) *objIter)->SetLOD( GetModelLODToUse( ((int) vec.x) / 1000, ((int) vec.z) / 1000 ) );
				(*objIter)->Render();
			}

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders dynamic objects in target square area or all dynamic objects in whole map if the
//   pos parameter is NULL
// set cars to TRUE to enable rendering of cars
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderDynamicObjects( MAPPOSITION *pos, int range, BOOL cars )
{
 	GROBJECTS_ITER			objIter;

	if ( bMapDependentRendering )
	{
        for ( objIter = Scene->DynamicObjects.begin(); objIter != Scene->DynamicObjects.end(); objIter++ )
			if ( Scene->Map.ObjectIsVisible( (*objIter)->GetMapPosList() ) && 
				( cars || (*objIter)->GetObjectType() != GROBJECT_TYPE_CAR ) )
				if ( !pos || (*objIter)->InterferesToSquareArea( pos, range ) )
				{
					D3DXVECTOR3 vec = (*objIter)->GetPosition();
					((CGrObjectMesh *) *objIter)->SetLOD( GetModelLODToUse( ((int) vec.x) / 1000, ((int) vec.z) / 1000 ) );
					if ( (*objIter)->GetObjectType() == GROBJECT_TYPE_CAR && bCarPerPixelLight ) {
						// set the color modifier matrix for car object - this can be moved
						// elsewhere once the color modifier is implemented for all object types
						Shader->SetPSValue( ColorModifierHandle, (*objIter)->GetColorModifier(), sizeof( D3DXMATRIX ) );
						((CGrObjectCar *)(*objIter))->RenderSpecular();
					}
					else (*objIter)->Render();
				}
	}
	else
	{
        for ( objIter = Scene->DynamicObjects.begin(); objIter != Scene->DynamicObjects.end(); objIter++ )
			if ( cars || (*objIter)->GetObjectType() != GROBJECT_TYPE_CAR )
			{
				if ( (*objIter)->GetObjectType() == GROBJECT_TYPE_CAR && bCarPerPixelLight ) {
					// set the color modifier matrix for car object - this can be moved
					// elsewhere once the color modifier is implemented for all object types
					Shader->SetPSValue( ColorModifierHandle, (*objIter)->GetColorModifier(), sizeof( D3DXMATRIX ) );
					((CGrObjectCar *)(*objIter))->RenderSpecular();
				}
				else (*objIter)->Render();
			}
	}

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders cars in target square area or all cars in whole map if the pos parameter is NULL
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderCarObjects( MAPPOSITION *pos, int range )
{
 	GROBJECTS_ITER			objIter;

	if ( bMapDependentRendering )
	{
        for ( objIter = Scene->Cars.begin(); objIter != Scene->Cars.end(); objIter++ )
			if ( Scene->Map.ObjectIsVisible( (*objIter)->GetMapPosList() ) )
				if ( !pos || (*objIter)->InterferesToSquareArea( pos, range ) )
				{
					D3DXVECTOR3 vec = (*objIter)->GetPosition();
					((CGrObjectMesh *) *objIter)->SetLOD( GetModelLODToUse( ((int) vec.x) / 1000, ((int) vec.z) / 1000 ) );
					if ( bCarPerPixelLight ) {
						// set the color modifier matrix for car object - this can be moved
						// elsewhere once the color modifier is implemented for all object types
						Shader->SetPSValue( ColorModifierHandle, (*objIter)->GetColorModifier(), sizeof( D3DXMATRIX ) );
						((CGrObjectCar *)(*objIter))->RenderSpecular();
					}
					else (*objIter)->Render();
				}
	}
	else
	{
        for ( objIter = Scene->Cars.begin(); objIter != Scene->Cars.end(); objIter++ )
		{
			if ( bCarPerPixelLight ) {
				// set the color modifier matrix for car object - this can be moved
				// elsewhere once the color modifier is implemented for all object types
				Shader->SetPSValue( ColorModifierHandle, (*objIter)->GetColorModifier(), sizeof( D3DXMATRIX ) );
				((CGrObjectCar *)(*objIter))->RenderSpecular();
			}
			else (*objIter)->Render();
		}
	}

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders objects in the specified square area or all of the objects (static and dynamic) 
//   in the whole scene when the pos parameter is NULL
// set the stat parameter to TRUE to render static objects
// set the dyn parameter to TRUE to render dynamic objects
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderAllObjects( MAPPOSITION *pos, int range, BOOL stat, BOOL dyn )
{
	if ( !stat && !dyn ) return ERRNOERROR;

	HRESULT					hr;

	if ( stat )	hr = RenderStaticObjects( pos, range );
	else hr = 0;
	if ( hr ) ERRORMSG( hr, "CGraphic::RenderAllObjects()", "An error occurs while rendering static objects." );

	if ( dyn )	hr = RenderDynamicObjects( pos, range, TRUE );
	else hr = 0;
	if ( hr ) ERRORMSG( hr, "CGraphic::RenderAllObjects()", "An error occurs while rendering dynamic objects." );

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders all (static and dynamic) shadow volumes
// iWhichSide parameter specifies which shadow polygons should be rendered
//   if the iWhichSide is positive, only front sided polygons are rendered
//   if it is negative, only back sided polygons are rendered
//   if iWhichSide is zero, all (back and front faced) polygons are rendered (default value)
// iDynShadowsUsed parameter says how many dynamic shadows are actually generated and should be
//   rendered. Default value is zero. Note that these shadow volumes has to be pregenerated
//   somewhere else.
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderShadowVolumes( int iWhichSide, int iDynShadowsUsed )
{
 	GROBJECTS_ITER			objIter;
	SHADOWVOLUME_ITER		iterShadow;	


	//Shader->SetVertexShader( ObjectVS );
    D3DDevice->SetFVF( D3DFVF_XYZ );


	// RENDER FRONTFACED POLYGONS
	if ( iWhichSide > 0 )
	{
		// render static shadow volumes
		if ( bMapDependentRendering )
		{
		    for ( objIter = Scene->StaticObjects.begin(); objIter != Scene->StaticObjects.end(); objIter++ )
				if ( Scene->Map.ObjectIsVisible( (*objIter)->GetMapPosList() ) )
					//TODO: Cert vi, jestli to tu ma byt nebo ne - nejak se tem stinum nechce
					//if ( (*objIter)->InterferesToSquareArea( MapPosition.PosX, MapPosition.PosZ, iModelLODDefSize / 2 ) )
						if ( (*objIter)->GetObjectType() == GROBJECT_TYPE_MESH )
						{
							D3DXVECTOR3 vec = (*objIter)->GetPosition();
							((CGrObjectMesh *) *objIter)->SetLOD( GetModelLODToUse( ((int) vec.x) / 1000, ((int) vec.z) / 1000 ) );
							((CGrObjectMesh *) *objIter)->RenderShadowVolume( 1 );
						}
		}
		else
		{
			for ( objIter = Scene->StaticObjects.begin(); objIter != Scene->StaticObjects.end(); objIter++ )
				if ( (*objIter)->GetObjectType() == GROBJECT_TYPE_MESH )
					{
						D3DXVECTOR3 vec = (*objIter)->GetPosition();
						((CGrObjectMesh *) *objIter)->SetLOD( GetModelLODToUse( ((int) vec.x) / 1000, ((int) vec.z) / 1000 ) );
						((CGrObjectMesh *) *objIter)->RenderShadowVolume( 1 );
					}
		}

		// render dynamic shadows
		iterShadow = ShadowVolumes.begin();
		for ( int j = 0; j < iDynShadowsUsed; iterShadow++, j++ ) (*iterShadow)->Render( 1 );
	}
	
	
	// RENDER BACKFACED POLYGONS
	if ( iWhichSide < 0 )
	{
		// render static shadow volumes
		if ( bMapDependentRendering )
		{
		    for ( objIter = Scene->StaticObjects.begin(); objIter != Scene->StaticObjects.end(); objIter++ )
				if ( Scene->Map.ObjectIsVisible( (*objIter)->GetMapPosList() ) )
					//if ( (*objIter)->InterferesToSquareArea( MapPosition.PosX, MapPosition.PosZ, iModelLODDefSize / 2 ) )
						if ( (*objIter)->GetObjectType() == GROBJECT_TYPE_MESH )
						{
							D3DXVECTOR3 vec = (*objIter)->GetPosition();
							((CGrObjectMesh *) *objIter)->SetLOD( GetModelLODToUse( ((int) vec.x) / 1000, ((int) vec.z) / 1000 ) );
							((CGrObjectMesh *) *objIter)->RenderShadowVolume( -1 );
						}
		}
		else
		{
			for ( objIter = Scene->StaticObjects.begin(); objIter != Scene->StaticObjects.end(); objIter++ )
				if ( (*objIter)->GetObjectType() == GROBJECT_TYPE_MESH )
					{
						D3DXVECTOR3 vec = (*objIter)->GetPosition();
						((CGrObjectMesh *) *objIter)->SetLOD( GetModelLODToUse( ((int) vec.x) / 1000, ((int) vec.z) / 1000 ) );
						((CGrObjectMesh *) *objIter)->RenderShadowVolume( -1 );
					}
		}

		// render dynamic shadows
		iterShadow = ShadowVolumes.begin();
		for ( int j = 0; j < iDynShadowsUsed; iterShadow++, j++ ) (*iterShadow)->Render( -1 );
	}


	// RENDER ALL POLYGONS
	if ( iWhichSide == 0 )
	{
		// render static shadow volumes
		if ( bMapDependentRendering )
		{
		    for ( objIter = Scene->StaticObjects.begin(); objIter != Scene->StaticObjects.end(); objIter++ )
				if ( Scene->Map.ObjectIsVisible( (*objIter)->GetMapPosList() ) )
					//if ( (*objIter)->InterferesToSquareArea( MapPosition.PosX, MapPosition.PosZ, iModelLODDefSize / 2 ) )
						if ( (*objIter)->GetObjectType() == GROBJECT_TYPE_MESH )
						{
							D3DXVECTOR3 vec = (*objIter)->GetPosition();
							((CGrObjectMesh *) *objIter)->SetLOD( GetModelLODToUse( ((int) vec.x) / 1000, ((int) vec.z) / 1000 ) );
							((CGrObjectMesh *) *objIter)->RenderShadowVolume( 0 );
						}
		}
		else
		{
			for ( objIter = Scene->StaticObjects.begin(); objIter != Scene->StaticObjects.end(); objIter++ )
				if ( (*objIter)->GetObjectType() == GROBJECT_TYPE_MESH )
					{
						D3DXVECTOR3 vec = (*objIter)->GetPosition();
						((CGrObjectMesh *) *objIter)->SetLOD( GetModelLODToUse( ((int) vec.x) / 1000, ((int) vec.z) / 1000 ) );
						((CGrObjectMesh *) *objIter)->RenderShadowVolume( 0 );
					}
		}

		// render dynamic shadows
		iterShadow = ShadowVolumes.begin();
		for ( int j = 0; j < iDynShadowsUsed; iterShadow++, j++ ) (*iterShadow)->Render( 0 );
	}


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders a scene with only ambient and one directional light
// the last directional light in the light list is used
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderDayScene()
{
	HRESULT					hr;
	D3DLIGHT9				Light;
	D3DXCOLOR				ambLightCol;
	LIGHTS_ITER				lightIter;
 	GROBJECTS_ITER			objIter;
	SHADOWVOLUME_ITER		iterShadow;	
	SHADOWVOLUME::size_type	numShadows;
	GROBJECTS::size_type	numObjects;
	int						usedShadowObjects;
	D3DXMATRIXA16			mat;
	CShadowVolume		*	pShadowVolume;

	D3DXMATRIX				LightViewMat, LightProjMat;
	D3DXMATRIX				TSMMatrix, TSMCameraProjMatrix;
	D3DXMATRIX				ShadowMat, PostProjMat;
	D3DXMATRIX				BiasMat;
	float					ZRange, fValue;

	D3DXVECTOR3				LightDirection;
	D3DXVECTOR3				ViewDirection;
	vector<UINT>		*	ExtendedTable;
	UINT					uiPSSimple, uiPSSpecular;
	IDirect3DTexture9	*	pTexture;
	RECT					CorrectMatRect;

	// TODO: for debugging purposes 
	//D3DXMatrixLookAtLH(&ViewMatrix, &D3DXVECTOR3(4346, 538, 6913), &D3DXVECTOR3(4346 + 13.7, 538 - 11.9, 6913 + 98.3), &D3DXVECTOR3(0, 1, 0) );




	// Obtain Lights
	ambLightCol.r = Scene->AmbientLight->Ambient.r;
	ambLightCol.g = Scene->AmbientLight->Ambient.g;
	ambLightCol.b = Scene->AmbientLight->Ambient.b;
	ambLightCol.a = 1.0f;

	Scene->SunLight->GetD3DLight( &Light );
	
	// No shadows - simply render scene
	if ( uiShadowComplexity == SC_NONE )
	{
		// Render objects
		SetVertexShader( ObjectVS );
		SetPixelShader( ObjectPS );

		// Set ambient & fog color
		Shader->SetPSValue( Shader->Ambient, &ambLightCol, sizeof(ambLightCol) );
		Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );

		// Set directional lights
		Shader->SetVSSingleDirLight( &Light );

		// for per pixel lighting cars must be rendered separately
		if ( bCarPerPixelLight && bRenderDynamicObjects )
		{
			// Render static and dynamic objects, but not cars !
			if ( bRenderStaticObjects ) RenderStaticObjects( &MapPosition, iMapLODDefSize / 2 );
			RenderDynamicObjects( &MapPosition, iMapLODDefSize / 2, FALSE );

			SetVertexShader( ObjectSpecularVS );
			SetPixelShader( ObjectSpecularPS );
			
			// Set ambient & fog color
			Shader->SetPSValue( Shader->Ambient, &ambLightCol, sizeof(ambLightCol) );
			Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );

			// Set directional lights
			ExtendedTable = Shader->GetExtendedTable( Shader->DirLight );
			Light.Specular.r = Light.Specular.g = Light.Specular.b = 1;
			Shader->SetVSSingleDirLight( &Light );
			Shader->SetPSValue( ExtendedTable->at(0), &Light.Ambient, 16 );
			Shader->SetPSValue( ExtendedTable->at(1), &Light.Diffuse, 16 );
			Shader->SetPSValue( ExtendedTable->at(2), &Light.Specular, 16 );

			RenderCarObjects( &MapPosition, iMapLODDefSize / 2 );
		}
		else
		{
			// Render static & dynamic objects and cars
			hr = RenderAllObjects( &MapPosition, iMapLODDefSize / 2, bRenderStaticObjects, bRenderDynamicObjects );
			if ( hr ) ERRORMSG( hr, "CGraphic::RenderSceneWithAmbientAndSunLight()", "Unable to render one or more scene objects." )
		}
		


		// Render terrain
		SetVertexShader( TerrainVS );
		SetPixelShader( TerrainPS );

		D3DDevice->SetTexture( 3, NormalMapTexture );

		// set map width and depth
		fValue = (float) Scene->Map.GetWidth() * TERRAIN_PLATE_WIDTH;
		Shader->SetVSValue( MapWidthHandle, &fValue, sizeof( fValue ) );
		fValue = (float) Scene->Map.GetDepth() * TERRAIN_PLATE_WIDTH;
		Shader->SetVSValue( MapDepthHandle, &fValue, sizeof( fValue ) );
		
		// Set ambient & fog color
		Shader->SetPSValue( Shader->Ambient, &ambLightCol, sizeof(ambLightCol) );
		Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );

		// render the map with ambient light
		if ( bRenderTerrain ) RenderTerrainSection( MapPosition.PosX, MapPosition.PosZ, iMapLODDefSize / 2, true );		
		if ( bRenderTerrain ) RenderTerrainSection( MapPosition.PosX, MapPosition.PosZ, iMapLODDefSize / 2, false );		
	}
	else if ( uiShadowComplexity == SC_SIMPLE )
	{
		LightDirection = Light.Direction;
		ViewDirection = Camera.GetLookAtVector() - Camera.GetEyePtVector();

		D3DXVec3Normalize(&LightDirection, &LightDirection);
		D3DXVec3Normalize(&ViewDirection, &ViewDirection);
		
		// Get projection matrix with limited far plane
		D3DXMatrixPerspectiveFovLH(&TSMCameraProjMatrix, FIELD_OF_VIEW_Y, 1.33f, NEAR_CLIPPING_PLANE, 100 * ONE_METER);

		// Compute view and projection matrix for light
		ShadowMap->Tsm.ComputeDirectionalLightMatrixesFromFrustrum( &TSMCameraProjMatrix,
																	&ViewMatrix, 
																	&LightDirection,
																	&LightProjMat,
																	&LightViewMat,
																	&ZRange );

		
		// Compute TSM post-proj matrix
		D3DXMatrixIdentity(&TSMMatrix);

		hr = ShadowMap->Tsm.ComputeTSMMatrix( 	&TSMCameraProjMatrix,
												&ViewMatrix,
												&LightProjMat,
												&LightViewMat, 
											 	&ViewDirection,
											 	&LightDirection, 
											 	&TSMMatrix);

		/*
		D3DXVECTOR3 DebugVec;

		DebugVec = D3DXVECTOR3(1.03, 0, 0);
		D3DXVec3TransformCoord(&DebugVec, &DebugVec, &TSMMatrix);
		DebugVec = D3DXVECTOR3(1.02, 0, 0);
		D3DXVec3TransformCoord(&DebugVec, &DebugVec, &TSMMatrix);
		DebugVec = D3DXVECTOR3(1.01, 0, 0);
		D3DXVec3TransformCoord(&DebugVec, &DebugVec, &TSMMatrix);
		DebugVec = D3DXVECTOR3(1.001, 0, 0);
		D3DXVec3TransformCoord(&DebugVec, &DebugVec, &TSMMatrix);
		DebugVec = D3DXVECTOR3(1.0001, 0, 0);
		D3DXVec3TransformCoord(&DebugVec, &DebugVec, &TSMMatrix);
		DebugVec = D3DXVECTOR3(1.09, 0, 0);
		D3DXVec3TransformCoord(&DebugVec, &DebugVec, &TSMMatrix);
		DebugVec = D3DXVECTOR3(1.08, 0, 0);
		D3DXVec3TransformCoord(&DebugVec, &DebugVec, &TSMMatrix);
		DebugVec = D3DXVECTOR3(1.07, 0, 0);
		D3DXVec3TransformCoord(&DebugVec, &DebugVec, &TSMMatrix);
		*/

		

		// Obtain bias
		D3DXMatrixTranslation(&BiasMat, 0.0f,0.0f, 15.0f);

		// DEBUGGING PURPOSES
		//ViewMatrix = LightViewMat;
		//ProjectionMatrix = LightProjMat;

		// Render scene depth from sun
		hr = RenderDepthSceneFromSun(&Light, &(LightViewMat * BiasMat), &LightProjMat , &TSMMatrix);
		if (hr) ERRORMSG(hr, "CGraphic::RenderSceneWithAmbientAndSunLight", "Unable to render depth scene from sun");



		// RENDER OBJECTS
		if ( ShadowMap->GetDynMapFormat() == D3DFMT_D24S8 ) {
			uiPSSimple = ObjectWithHWTSMPS;
			uiPSSpecular = ObjectWithHWTSMSpecularPS;
			pTexture = DepthTexture2;
		} else {
			uiPSSimple = ObjectWithTSMLinearPCFPS;
			uiPSSpecular = ObjectWithTSMLinearPCFSpecularPS;
			pTexture = ShadowMap->GetDynShadowTex();
		}

		SetVertexShader( ObjectWithTSMVS );
		SetPixelShader( uiPSSimple );
		D3DDevice->SetTexture( 3, pTexture );

		// set fog color
		Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );


		// Set matrices
		ShadowMat = LightViewMat * LightProjMat;
		CorrectMatRect.left = 1;
		CorrectMatRect.top = 1;
		CorrectMatRect.right =  ShadowMap->GetSunMapSize() - 2;
		CorrectMatRect.bottom = ShadowMap->GetSunMapSize() - 2;
		PostProjMat = TSMMatrix * ShadowMap->GetCorrectMat(CorrectMatRect);

		Shader->SetVSMatrix( ShadowMatHandle, &ShadowMat );
		Shader->SetVSMatrix( PostProjMatHandle, &PostProjMat );
		
		Shader->SetPSValue( Shader->Ambient, &ambLightCol, sizeof(ambLightCol) );
		Shader->SetVSSingleDirLight( &Light );
	
		// for per pixel lighting cars must be rendered separately
		if ( bCarPerPixelLight && bRenderDynamicObjects )
		{
			// Render static and dynamic objects, but not cars !
			if ( bRenderStaticObjects ) RenderStaticObjects( &MapPosition, iMapLODDefSize / 2 );
			RenderDynamicObjects( &MapPosition, iMapLODDefSize / 2, FALSE );

			/*
			// DEBUGGING PURPOSES
			// Render objects
			SetVertexShader( ObjectVS );
			SetPixelShader( ObjectPS );

			// Set ambient & fog color
			Shader->SetPSValue( Shader->Ambient, &ambLightCol, sizeof(ambLightCol) );
			Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );

			// Set directional lights
			Shader->SetVSSingleDirLight( &Light );

			RenderStaticObjects( &MapPosition, iMapLODDefSize / 2 );
			// END DEBUGGING PURPOSES
			*/

			SetVertexShader( ObjectWithTSMSpecularVS );
			SetPixelShader( uiPSSpecular );
			
			// set fog color
			Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );
	
			Shader->SetVSMatrix( ShadowMatHandle, &ShadowMat );
			Shader->SetVSMatrix( PostProjMatHandle, &PostProjMat );
			
			// Set ambient color
			Shader->SetPSValue( Shader->Ambient, &ambLightCol, sizeof(ambLightCol) );

			// Set directional lights
			ExtendedTable = Shader->GetExtendedTable( Shader->DirLight );
			Light.Specular.r = Light.Specular.g = Light.Specular.b = 1;
			Shader->SetVSSingleDirLight( &Light );
			Shader->SetPSValue( ExtendedTable->at(0), &Light.Ambient, 16 );
			Shader->SetPSValue( ExtendedTable->at(1), &Light.Diffuse, 16 );
			Shader->SetPSValue( ExtendedTable->at(2), &Light.Specular, 16 );

			RenderCarObjects( &MapPosition, iMapLODDefSize / 2 );
		}
		else
		{
			// Render static & dynamic objects and cars
			hr = RenderAllObjects( &MapPosition, iMapLODDefSize / 2, bRenderStaticObjects, bRenderDynamicObjects );
			if ( hr ) ERRORMSG( hr, "CGraphic::RenderSceneWithAmbientAndSunLight()", "Unable to render one or more scene objects." )
		}



		// RENDER TERRAIN
		SetVertexShader(TerrainWithTSMVS);

		if (ShadowMap->GetDynMapFormat() == D3DFMT_D24S8) 
		{
			SetPixelShader(TerrainWithHWTSMPS);
		}
		else
		{
			//SetPixelShader(TerrainWithTSMPS);
			SetPixelShader(TerrainWithTSMLinearPCFPS);
		}
			
		// set matrixes
		Shader->SetVSMatrix(ShadowMatHandle, &ShadowMat);
		Shader->SetVSMatrix(PostProjMatHandle, &PostProjMat);
		
		// Set ambient & fog color
		Shader->SetPSValue(Shader->Ambient, &ambLightCol, sizeof(ambLightCol) );
		Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );
		Shader->SetVSSingleDirLight( &Light );

		D3DDevice->SetTexture( 4, NormalMapTexture );

		// set map width and depth
		fValue = (float) Scene->Map.GetWidth() * TERRAIN_PLATE_WIDTH;
		Shader->SetVSValue( MapWidthHandle, &fValue, sizeof( fValue ) );
		fValue = (float) Scene->Map.GetDepth() * TERRAIN_PLATE_WIDTH;
		Shader->SetVSValue( MapDepthHandle, &fValue, sizeof( fValue ) );

		// render the map with ambient light
		if ( bRenderTerrain ) RenderTerrainSection( MapPosition.PosX, MapPosition.PosZ, iMapLODDefSize / 2, true );		
		if ( bRenderTerrain ) RenderTerrainSection( MapPosition.PosX, MapPosition.PosZ, iMapLODDefSize / 2, false );	

		D3DDevice->SetTexture( 3, 0 );
	}

	// THIS SECTION IS LEFT ONLY FOR CONSIDERATION REASONS  //
	// PROBABLY WILL BE DELETED SOON						//
	else if ( uiShadowComplexity == SC_COMPLEX ) // use shadow volume method for creating shadows
	{

 
		// GENERATE DYNAMIC SHADOW VOLUMES

		if ( bRenderDynamicObjects )
		{
			// allocate memory for ShadowVolume objects 
			numShadows = ShadowVolumes.size();
			numObjects = Scene->DynamicObjects.size();
	
			for (  GROBJECTS::size_type i = numShadows; i < numObjects; i++ )
			{
				pShadowVolume = new CShadowVolume;
				ShadowVolumes.push_back( pShadowVolume );
			}

			iterShadow = ShadowVolumes.begin();
			usedShadowObjects = 0;

			if ( bMapDependentRendering )
			{
				for ( objIter = Scene->DynamicObjects.begin(); objIter != Scene->DynamicObjects.end(); objIter++ )
					if ( Scene->Map.ObjectIsVisible( (*objIter)->GetMapPosList() ) )
						if ( (*objIter)->InterferesToSquareArea( MapPosition.PosX, MapPosition.PosZ, iModelLODDefSize / 2 ) )
							if ( (*objIter)->GetObjectType() == GROBJECT_TYPE_MESH )
							{
								LPGROBJECTMESH	objMesh = (LPGROBJECTMESH) *objIter;
								D3DXVECTOR3		vec = objMesh->GetPosition();
								int				iLODToUse = GetModelLODToUse( ((int) vec.x) / 1000, ((int) vec.z) / 1000 );
					
								mat = objMesh->GetWorldMat();
								(*iterShadow)->Init( D3DDevice, objMesh->GetMesh( iLODToUse ), (D3DXVECTOR3*) &(Light.Direction), &mat, false, true );
								iterShadow++;
								usedShadowObjects++;
							}
			}
			else 
			{
				for ( objIter = Scene->DynamicObjects.begin(); objIter != Scene->DynamicObjects.end(); objIter++ )
					if ( (*objIter)->GetObjectType() == GROBJECT_TYPE_MESH )
					{
						LPGROBJECTMESH	objMesh = (LPGROBJECTMESH) *objIter;
						D3DXVECTOR3		vec = objMesh->GetPosition();
						int				iLODToUse = GetModelLODToUse( ((int) vec.x) / 1000, ((int) vec.z) / 1000 );
				
						mat = objMesh->GetWorldMat();
						(*iterShadow)->Init( D3DDevice, objMesh->GetMesh( iLODToUse ), (D3DXVECTOR3*) &(Light.Direction), &mat, false, true );
						iterShadow++;
						usedShadowObjects++;
					}
			}
		}
		else usedShadowObjects = 0;
	
 
		// RENDER TERRAIN AND ALL OBJECTS WITH AMBIENT AND SUN LIGHT
		D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

		Shader->SetVertexShader( ObjectNoDirLightVS );
		Shader->SetPixelShader( ObjectPS );

		// Set ambient & fog color
		Shader->SetPSValue( Shader->Ambient, &ambLightCol, sizeof(ambLightCol) );
		Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );

		// render all static and dynamic objects in a scene list
		hr = RenderAllObjects( NULL, 0, bRenderStaticObjects, bRenderDynamicObjects );
		if ( hr ) ERRORMSG( hr, "CGraphic::RenderSceneWithAmbientAndSunLight()", "Unable to render one or more scene objects." );

		Shader->SetVertexShader( TerrainNoDirLightVS );
		Shader->SetPixelShader( TerrainPS );

		// Set ambient & fog color
		Shader->SetPSValue( Shader->Ambient, &ambLightCol, sizeof(ambLightCol) );
		Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );

		// render the map with ambient light
		if ( bRenderTerrain ) RenderTerrainSection( MapPosition.PosX, MapPosition.PosZ, iMapLODDefSize / 2 );





		// RENDER ALL SHADOW VOLUMES TO A TEXTURE
		D3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
		D3DDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESS );
		D3DDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
		D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
		D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		D3DDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
		D3DDevice->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );
		D3DDevice->SetRenderState( D3DRS_STENCILENABLE, TRUE );
		D3DDevice->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, TRUE );
		D3DDevice->SetRenderState( D3DRS_STENCILREF, 1 );
		D3DDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_DECR );
		D3DDevice->SetRenderState( D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_INCR );
		//D3DDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_INCR );
		//D3DDevice->SetRenderState( D3DRS_CCW_STENCILPASS, D3DSTENCILOP_DECR );
        //StencilMask = 0xFFFFFFFF;
        //StencilWriteMask = 0xFFFFFFFF;
        //Ccw_StencilFunc = Always;
        //Ccw_StencilZFail = Incr;
        //Ccw_StencilPass = Keep;
        //StencilFunc = Always;
        //StencilZFail = Decr;
        //StencilPass = Keep;

		Shader->SetVertexShader( TransformVS );
		Shader->SetPixelShader( ZeroPS );

		RenderShadowVolumes( 0, usedShadowObjects );

		// render front faced shadow polygons
		//RenderShadowVolumes( 1, usedShadowObjects );

		//D3DDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT );

		//// render back faced shadow polygons
		//RenderShadowVolumes( -1, usedShadowObjects );

		//// return rendering states to their standard values

		//D3DDevice->SetRenderTarget( 0, StandardRenderTarget );

		//D3DXSaveSurfaceToFile( "e:\\image.bmp", D3DXIFF_BMP, pTexRenderSurface, NULL, NULL ); 


		ambLightCol.r = ambLightCol.g = ambLightCol.b = 0.0f;

		D3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
		D3DDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_EQUAL );
		D3DDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
		D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
		D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		D3DDevice->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, FALSE );
		D3DDevice->SetRenderState( D3DRS_STENCILREF, 16 );
		D3DDevice->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL );
		D3DDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );
		D3DDevice->SetRenderState( D3DRS_CCW_STENCILPASS, D3DSTENCILOP_KEEP );
		D3DDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
		D3DDevice->SetRenderState( D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP );

		Shader->SetVertexShader( ObjectVS );
		Shader->SetPixelShader( ObjectPS );

		// Set ambient & fog color
		Shader->SetPSValue( Shader->Ambient, &ambLightCol, sizeof(ambLightCol) );
		Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );
		Shader->SetVSSingleDirLight( &Light );

		// render all static and dynamic objects in a scene list
		hr = RenderAllObjects( NULL, 0, bRenderStaticObjects, bRenderDynamicObjects );
		if ( hr ) ERRORMSG( hr, "CGraphic::RenderSceneWithAmbientAndSunLight()", "Unable to render one or more scene objects." );

		Shader->SetVertexShader( TerrainVS );
		Shader->SetPixelShader( TerrainPS );

		//TODO: shader properties for normal mapping must be declared here 

		// Set ambient & fog color
		Shader->SetPSValue( Shader->Ambient, &ambLightCol, sizeof(ambLightCol) );
		Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );
		Shader->SetVSSingleDirLight( &Light );

		// render the map with ambient light
		if ( bRenderTerrain ) RenderTerrainSection( MapPosition.PosX, MapPosition.PosZ, iMapLODDefSize / 2 );

		D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		D3DDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
		D3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
		D3DDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESS );
		D3DDevice->SetRenderState( D3DRS_STENCILENABLE, FALSE );


		// USE THE SHADOW TEXTURE TO COVER ENTIRE SCREEN
		//D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
		//D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		//D3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
		//D3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
		//D3DDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );

		//Shader->SetPixelShader( SVCoveringPolygonPS );

		//// Set the sun color and add ambient
		//Light.Diffuse.r = Scene->SkySystem.GetSunColor().x;
		//Light.Diffuse.g = Scene->SkySystem.GetSunColor().y;
		//Light.Diffuse.b = Scene->SkySystem.GetSunColor().z;
		//Shader->SetPSValue( Shader->Ambient, &Light.Diffuse, sizeof(Light.Diffuse) );

		//D3DDevice->SetTexture( 3, pShadowTexture );
		//D3DDevice->SetFVF( SHADOWVERTEX::FVF );
		//D3DDevice->SetStreamSource( 0, pShadowPolygonVB, 0, sizeof( SHADOWVERTEX ) );

		//D3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

		//D3DDevice->SetTexture( 3, NULL );
		//D3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
		//D3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
		//D3DDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );

		//D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		//D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		//D3DDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
 
	}

	
	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders all parts of scene that are enlighted by dynamic lights
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderNightScene()
{
	HRESULT					hr;
	LIGHTS_ITER				lightIter;
	GROBJECTS_ITER			objIter;
	SHADOWVOLUME_ITER		iterShadow;	
	D3DXMATRIXA16			mat;
	CGrObjectTerrainPlate * Plate;
	MAPPOSITIONLIST::iterator MapPosIt;
	UINT					i;

	RECT LightBoundRect[9];
	GROBJECTS_ITER It;
	CGrObjectMesh * DynMesh;
	CGrObjectMesh * DynMeshes[9];
	SpotLightShaderStructVector SpotLightsVector[9];
	pair<UINT, float> Distance[9];
	UINT SLNumber = 0; // Spot light number
	CLight * CarLight;
	CLight Light;
	vector<UINT> * ExtendedTable;
	D3DVIEWPORT9 CarLightViewport;
	int TempI;
	RECT ComputationArea;
	CRectangleInclusion::RECTINCLUSIONS RectInclusions;
	CRectangleInclusion::RECTINCLUSIONS::iterator RIIt;
	vector<UINT> KeyNumbers;
	D3DXMATRIX ShadowMats[9];
	D3DXMATRIX LightViewMats[9];
	D3DXMATRIX LightProjMats[9];
	RECT ShadowRect;
	vector< pair<UINT, UINT> >::iterator MPIt;
	CGrObjectMesh * LastRenderedMesh;
	vector<CGrObjectMesh *>::iterator MeshIt;
	static vector<CGrObjectMesh *> MeshesToRender;
	D3DXVECTOR3 ViewDirection;
	D3DXVECTOR3 ObjectDirection;



	// Traverse all dynamic objects (cars)
	for (It = Scene->DynamicObjects.begin(); It != Scene->DynamicObjects.end(); It++)
	{
		DynMesh = (CGrObjectMesh *) (*It);
		if (DynMesh->GetLight() && DynMesh->GetLight()->Type == LT_CARREFLECTOR)
		{
			// store dyn. mesh
			DynMeshes[SLNumber] = DynMesh;
			// Compute view dependent metrics
			// Alpha - angle between view direction and direction to viewed object
			// Metric: Alpha * DistanceToViewedObject
			ObjectDirection = DynMesh->GetPosition() - Camera.GetEyePtVector();
			Distance[SLNumber] = pair<UINT, float>(SLNumber, D3DXVec3Length( & ObjectDirection ) );
			D3DXVec3Normalize(&ObjectDirection, &ObjectDirection);
			D3DXVec3Normalize(&ViewDirection, & (Camera.GetLookAtVector() - Camera.GetEyePtVector()) );
			Distance[SLNumber].second *= acos( D3DXVec3Dot(&ObjectDirection, &ViewDirection) );
			// Perform conversion into spotlight shader structure
			CarLight = DynMesh->GetLight();
			ConvertToShaderSpotLightVector(CarLight, DynMesh->GetWorldMat(), &SpotLightsVector[SLNumber] );
			SLNumber++;
			if (SLNumber == 9) break; // we are not capable to handle more lights
		}
	}
	
	if (uiShadowComplexity == SC_NONE)
	{
		Shader->SetVertexShader(TerrainNightVS);
		Shader->SetPixelShader(TerrainNightPS);
		
		// Set common shader constants
		// Set spotlights into vertex shader
		ExtendedTable = Shader->GetExtendedTable(SpotLightsHandle);
		for (i = 0; i < 9; i++)
		{
			Shader->SetVSValue(ExtendedTable->at(i * 7), &SpotLightsVector[i].Ambient, 16); 
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 1), &SpotLightsVector[i].Diffuse, 16);
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 2), &SpotLightsVector[i].Specular, 16);
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 3), &SpotLightsVector[i].Position, 16);
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 4), &SpotLightsVector[i].Direction, 16);
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 5), &SpotLightsVector[i].Atten, 16);
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 6), &SpotLightsVector[i].Angles, 16);
		}

		// Set ambient into pixel shader
		Shader->SetPSValue(Shader->Ambient, & (Scene->AmbientLight->Ambient), sizeof(D3DXVECTOR4) );

		if (bRenderTerrain) RenderTerrainSection( MapPosition.PosX, MapPosition.PosZ, iMapLODDefSize / 2, true );
		if (bRenderTerrain) RenderTerrainSection( MapPosition.PosX, MapPosition.PosZ, iMapLODDefSize / 2, false );

		Shader->SetVertexShader( ObjectNightVS );
		Shader->SetPixelShader( ObjectNightPS );

		// Set common shader constants
		// Set spotlights into vertex shader
		ExtendedTable = Shader->GetExtendedTable(SpotLightsHandle);
		for (i = 0; i < 9; i++)
		{
			Shader->SetVSValue(ExtendedTable->at(i * 7), &SpotLightsVector[i].Ambient, 16); 
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 1), &SpotLightsVector[i].Diffuse, 16);
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 2), &SpotLightsVector[i].Specular, 16);
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 3), &SpotLightsVector[i].Position, 16);
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 4), &SpotLightsVector[i].Direction, 16);
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 5), &SpotLightsVector[i].Atten, 16);
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 6), &SpotLightsVector[i].Angles, 16);
		}

		// Set ambient into pixel shader
		Shader->SetPSValue(Shader->Ambient, & (Scene->AmbientLight->Ambient), sizeof(D3DXVECTOR4) );
		
		RenderAllObjects(&MapPosition, iMapLODDefSize / 2, bRenderStaticObjects, bRenderDynamicObjects);
	}
	else 
	{
		// sort distance
		sort< pair<UINT, float> *, bool (_cdecl *) (pair<UINT, float> &, pair<UINT, float> &) >(Distance, Distance + SLNumber, PairSecondLesser<UINT, float> );

		// Render depth from light view
		// render four nearest lights to the viewer

		// Set shaders
		Shader->SetVertexShader(DepthVS);
		Shader->SetPixelShader(DepthPS);

		// Set render target and depth stencil
		D3DDevice->SetRenderTarget(0, ShadowMap->GetDynShadowSurf() );
		D3DDevice->SetDepthStencilSurface(DepthStencil2);
		
		// Clear entire surface
		CarLightViewport.X = 0;
		CarLightViewport.Y = 0;
		CarLightViewport.MinZ = 0;
		CarLightViewport.MaxZ = 1;
		CarLightViewport.Width = ShadowMap->GetDynMapSize();
		CarLightViewport.Height = ShadowMap->GetDynMapSize();
		D3DDevice->SetViewport(&CarLightViewport);

		// clear render target and depth stecil
		D3DDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(255, 255, 255, 255), 1, 0 );
		// init viewport
		CarLightViewport.MaxZ = 1;
		CarLightViewport.MinZ = 0;
		CarLightViewport.Width = ShadowMap->GetCarLOD();
		CarLightViewport.Height = ShadowMap->GetCarLOD();
		for (i = 0; i < min(SLNumber, 4); i++)
		{
			// init viewport
			CarLightViewport.X = 1 + 1024 * (i % 2);
			CarLightViewport.Y = 1 + 1024 * (i / 2);

			TempI = Distance[i].first;

			// Get car light bounding rect
			FindCarLightBoundingRect(DynMeshes[TempI]->GetLight(), DynMeshes[TempI]->GetWorldMat(), &LightBoundRect[i]);
			RectToMapPositionRect(LightBoundRect[i], LightBoundRect[i] );

			// compute light view and projection matrixes
			memcpy(&Light, DynMeshes[TempI]->GetLight(), sizeof(CLight) );
			D3DXVec3TransformCoord(& (Light.Position), & (Light.Position), DynMeshes[TempI]->GetWorldMatPtr() );
			D3DXVec3TransformNormal( & (Light.Direction), &(Light.Direction), DynMeshes[TempI]->GetWorldMatPtr() );

			ShadowMap->ComputeLightViewProjMat(& LightViewMats[i], & LightProjMats[i], &Light);
				
			RenderCarLightDepth( LightBoundRect[i], DynMeshes[TempI], CarLightViewport, LightViewMats[i], LightProjMats[i]);
		}

		// set things back 
		// viewport
		D3DDevice->SetViewport(&StandardViewport);
		// Render target and depth stencil
		D3DDevice->SetRenderTarget(0, StandardRenderTarget);
		D3DDevice->SetDepthStencilSurface(DepthStencil);
		// matrixes
		//ViewMatrix = LightViewMats[0];
		//ProjectionMatrix = LightProjMats[0];
		Shader->SetViewMatrix(&ViewMatrix);
		Shader->SetProjMatrix(&ProjectionMatrix);
		//Shader->SetViewMatrix(&LightViewMats[0]);
		//Shader->SetProjMatrix(&LightProjMats[0]);

		// Set computation area
		ComputationArea.left = max(0, MapPosition.PosX - iMapLODDefSize / 2);
		ComputationArea.right = min( (long) Scene->Map.GetWidth() - 1, MapPosition.PosX + iMapLODDefSize / 2);
		ComputationArea.top = max(0, MapPosition.PosZ - iMapLODDefSize / 2);
		ComputationArea.bottom = min( (long) Scene->Map.GetDepth() - 1, MapPosition.PosZ + iMapLODDefSize / 2);
		
		hr = RectangleInclusion.SetComputationArea(ComputationArea);
		if (FAILED(hr ) )
			ERRORMSG(hr, "CGraphic::RenderNightScene()", "Unable to set computation area for rectangle inclusion.");
		// Compute rectangle inclusions
		RectangleInclusion.ComputeInclusions(LightBoundRect, SLNumber, RectInclusions);

		for (RIIt = RectInclusions.begin(); RIIt != RectInclusions.end(); RIIt++)
		{
			// List lights used on current square list
			KeyNumbers.clear();
			RectangleInclusion.ExpandKey(RIIt->first, KeyNumbers, 4);

			// Along size set appropriately complex vertex and pixel shaders 
			if (KeyNumbers.size() == 0)
			{
				Shader->SetVertexShader(TerrainNightVS);
				Shader->SetPixelShader(TerrainNightPS);
			}
			else 
			{
				Shader->SetVertexShader(TerrainNightSMVS[KeyNumbers.size() - 1] );
				if (ShadowMap->GetDynMapFormat() == D3DFMT_D24S8)
				{
					Shader->SetPixelShader(TerrainNightHWSMPS[KeyNumbers.size() - 1] );
					// set texture
					D3DDevice->SetTexture(3, DepthTexture2);
				}
				else
				{
					Shader->SetPixelShader(TerrainNightSMPS[KeyNumbers.size() - 1] );
					// set texture
					D3DDevice->SetTexture(3, ShadowMap->GetDynShadowTex() );
				}

				// Set specific shadow map shader constants
				for (i = 0; i < KeyNumbers.size(); i++)
				{
					// Set clamp vectors
					Shader->SetVSValue(ClampVectorsHandles[i], &ClampVectors[KeyNumbers[i]], sizeof(D3DXVECTOR4) );

					// Compute and set shadow matrixes
					ShadowRect.left = 1 + 1024 * ( (KeyNumbers[i]) % 2);
					ShadowRect.top = 1 + 1024 * ( (KeyNumbers[i]) / 2);
					ShadowRect.right = ShadowRect.left + ShadowMap->GetCarLOD();
					ShadowRect.bottom = ShadowRect.top + ShadowMap->GetCarLOD();
					ShadowMats[i] = LightViewMats[KeyNumbers[i]] * LightProjMats[KeyNumbers[i]] * ShadowMap->GetCorrectMat(ShadowRect);
					Shader->SetVSMatrix(ShadowMatsHandles[i], &ShadowMats[i] );
				}

					
			}

			// Set common shader constants
			// Set spotlights into vertex shader
			ExtendedTable = Shader->GetExtendedTable(SpotLightsHandle);
			for (i = 0; i < 9; i++)
			{
				Shader->SetVSValue(ExtendedTable->at(i * 7), &SpotLightsVector[i].Ambient, 16); 
				Shader->SetVSValue(ExtendedTable->at(i * 7 + 1), &SpotLightsVector[i].Diffuse, 16);
				Shader->SetVSValue(ExtendedTable->at(i * 7 + 2), &SpotLightsVector[i].Specular, 16);
				Shader->SetVSValue(ExtendedTable->at(i * 7 + 3), &SpotLightsVector[i].Position, 16);
				Shader->SetVSValue(ExtendedTable->at(i * 7 + 4), &SpotLightsVector[i].Direction, 16);
				Shader->SetVSValue(ExtendedTable->at(i * 7 + 5), &SpotLightsVector[i].Atten, 16);
				Shader->SetVSValue(ExtendedTable->at(i * 7 + 6), &SpotLightsVector[i].Angles, 16);
			}

			// Set ambient into pixel shader
			Shader->SetPSValue(Shader->Ambient, & (Scene->AmbientLight->Ambient), sizeof(D3DXVECTOR4) );

			// Render listed terrain plates
			for (MPIt = RIIt->second.begin(); MPIt != RIIt->second.end(); MPIt++)
			{
				RenderTerrainField(MPIt->first, MPIt->second, true);
				RenderTerrainField(MPIt->first, MPIt->second, false);
			}
					
		}

		// Render objects
		for (RIIt = RectInclusions.begin(); RIIt != RectInclusions.end(); RIIt++)
		{
			// List lights used on current square list
			KeyNumbers.clear();
			RectangleInclusion.ExpandKey(RIIt->first, KeyNumbers, 4);
			
			// Along size set appropriately complex vertex and pixel shaders 
			if (KeyNumbers.size() == 0)
			{
				Shader->SetVertexShader(ObjectNightVS);
				Shader->SetPixelShader(ObjectNightPS);
			}
			else 
			{
				Shader->SetVertexShader(ObjectNightSMVS[KeyNumbers.size() - 1] );
				if (ShadowMap->GetDynMapFormat() == D3DFMT_D24S8)
				{
					Shader->SetPixelShader(ObjectNightHWSMPS[KeyNumbers.size() - 1] );
					// set texture
					D3DDevice->SetTexture(3, DepthTexture2);
				}
				else
				{
					Shader->SetPixelShader(ObjectNightSMPS[KeyNumbers.size() - 1] );
					// set texture
					D3DDevice->SetTexture(3, ShadowMap->GetDynShadowTex() );
				}
				
				// Set specific shadow map shader constants
				for (i = 0; i < KeyNumbers.size(); i++)
				{
					// Set clamp vectors
					Shader->SetVSValue(ClampVectorsHandles[i], &ClampVectors[KeyNumbers[i]], sizeof(D3DXVECTOR4) );

					// Compute and set shadow matrixes
					ShadowRect.left = 1 + 1024 * ( (KeyNumbers[i]) % 2);
					ShadowRect.top = 1 + 1024 * ( (KeyNumbers[i]) / 2);
					ShadowRect.right = ShadowRect.left + ShadowMap->GetCarLOD();
					ShadowRect.bottom = ShadowRect.top + ShadowMap->GetCarLOD();
					ShadowMats[i] = LightViewMats[KeyNumbers[i]] * LightProjMats[KeyNumbers[i]] * ShadowMap->GetCorrectMat(ShadowRect);
					Shader->SetVSMatrix(ShadowMatsHandles[i], &ShadowMats[i] );
				}
			}

			// Set common shader constants
			// Set spotlights into vertex shader
			ExtendedTable = Shader->GetExtendedTable(SpotLightsHandle);
			for (i = 0; i < 9; i++)
			{
				Shader->SetVSValue(ExtendedTable->at(i * 7), &SpotLightsVector[i].Ambient, 16); 
				Shader->SetVSValue(ExtendedTable->at(i * 7 + 1), &SpotLightsVector[i].Diffuse, 16);
				Shader->SetVSValue(ExtendedTable->at(i * 7 + 2), &SpotLightsVector[i].Specular, 16);
				Shader->SetVSValue(ExtendedTable->at(i * 7 + 3), &SpotLightsVector[i].Position, 16);
				Shader->SetVSValue(ExtendedTable->at(i * 7 + 4), &SpotLightsVector[i].Direction, 16);
				Shader->SetVSValue(ExtendedTable->at(i * 7 + 5), &SpotLightsVector[i].Atten, 16);
				Shader->SetVSValue(ExtendedTable->at(i * 7 + 6), &SpotLightsVector[i].Angles, 16);
			}

			// Set ambient into pixel shader
			Shader->SetPSValue(Shader->Ambient, & (Scene->AmbientLight->Ambient), sizeof(D3DXVECTOR4) );

			// Render static and dynamic objects on listed terrain plates
			MeshesToRender.clear();
			for (MPIt = RIIt->second.begin(); MPIt != RIIt->second.end(); MPIt++)
			{
				Plate = Scene->Map.GetPlate(MPIt->first, MPIt->second);
				if (Plate->IsVisible() )
					for (MeshIt = Plate->GetObjects()->begin(); MeshIt != Plate->GetObjects()->end(); MeshIt++)
						MeshesToRender.push_back( (*MeshIt) );

			}

			// sort meshes, so that every mesh will render only once
			sort(MeshesToRender.begin(), MeshesToRender.end() );

			// finally render
			LastRenderedMesh = 0;
			for (MeshIt = MeshesToRender.begin(); MeshIt != MeshesToRender.end(); MeshIt++)
				if ((*MeshIt) != LastRenderedMesh)
				{
					if ( ( (*MeshIt)->IsStatic() && bRenderStaticObjects) || bRenderDynamicObjects )
					{
						D3DXVECTOR3 vec = (*MeshIt)->GetPosition();
						((CGrObjectMesh *) *MeshIt)->SetLOD( GetModelLODToUse( ((int) vec.x) / 1000, ((int) vec.z) / 1000 ) );
						(*MeshIt)->Render();
						LastRenderedMesh = (*MeshIt);
					}
				}
		}
	}
		
	
	// Render lamps lights and shadows
	for (i = 0; i < MapExclude.size(); i++)
	{
		Plate = Scene->Map.GetPlate(MapExclude[i].Light->MapPosition.PosX, MapExclude[i].Light->MapPosition.PosZ);
		if ( Plate->IsVisible() )
		{
			switch ( uiShadowComplexity )
			{
			case SC_NONE:
				hr = RenderLamp( MapExclude[i].Object, MapExclude[i].Light);
				if ( hr) ERRORMSG(hr, "CGraphic::RenderSceneWithDynamicLights()", "Unable to render lamp light.");
				
				break;
			case SC_COMPLEX:
			case SC_SIMPLE:
				// SHADOW MAPPING
					hr = RenderDynamicShadowMapForLamp( MapExclude[i].Object, MapExclude[i].Light);
					if ( hr) ERRORMSG(hr, "CGraphic::RenderSceneWithDynamicLights()", "Unable to render lamp light with shadow mapping.");
			break;
			}
		}
	}

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders glows for lamp lights, should be called at the end of rendering because this uses 
//   alpha blended billboards
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderLightGlows()
{
	LIGHTS_ITER				lightIter;
	D3DXMATRIX				mat, mat2, mat3, mat4;
	LPDIRECT3DTEXTURE9		tex;
	D3DXVECTOR3				vec, look;
	FLOAT					r, a, b;
	resManNS::__Texture			*	texStruct;

	
	texStruct = ResourceManager->GetTexture( RIDLightGlowTexture );
	tex = texStruct->texture;

	D3DDevice->SetTexture( 1, tex );
	D3DDevice->SetFVF( LIGHTGLOWVERTEX::FVF );
	D3DDevice->SetStreamSource( 0, pLightGlowsVB, 0, sizeof( LIGHTGLOWVERTEX ) );

	Shader->SetVertexShader( BillboardVS );
	Shader->SetPixelShader( BillboardPS );
	
	// set fog color
	Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );

	D3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	for ( lightIter = Scene->Lights.begin(); lightIter != Scene->Lights.end(); lightIter++ )
	{
		if ( !(*lightIter)->Enabled || (*lightIter)->Type != LT_LAMP ) continue;
								 
		look = - (*lightIter)->Position + GetEyePtVector();

		r = sqrtf( look.x*look.x + look.z*look.z );
		a = acosf( ABS( look.z / r ) );
		r = sqrtf( look.z*look.z + look.y*look.y );
		b = acosf( ABS( look.z / r ) );
		

		if ( look.x * look.z > 0 ) D3DXMatrixRotationY( &mat2, a );
		else D3DXMatrixRotationY( &mat2, -a );
		
		if ( look.y * look.z < 0 ) D3DXMatrixRotationX( &mat, b );
		else D3DXMatrixRotationX( &mat, -b );
		D3DXMatrixMultiply( &mat3, &mat2, &mat );

		D3DXMatrixTranslation( &mat4, (*lightIter)->Position.x, (*lightIter)->Position.y, (*lightIter)->Position.z );
		D3DXMatrixMultiply( &mat, &mat3, &mat4 );

		Shader->SetWorldMatrix( &mat );
		D3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	}

	D3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );

/*
	for ( lightIter = Scene->Lights.begin(); lightIter != Scene->Lights.end(); lightIter++ )
	{
		if ( !(*lightIter)->Enabled || (*lightIter)->Type != LT_LAMP ) continue;

		LPD3DXMESH	Mesh;
		XFile		*pXFile;
		D3DXMATRIX	mat;

	
		// place mesh to scene
		D3DXMatrixTranslation( &mat, (*lightIter)->Position.x, (*lightIter)->Position.y, (*lightIter)->Position.z ); 
		D3DDevice->SetTransform( D3DTS_WORLD, &mat );

		pXFile = ResourceManager->GetXFile( RIDLampGlowMesh ); // find xFile resource
        if ( !pXFile ) ERRORMSG( ERRGENERIC, "CGraphic::RenderLightsGlows()", "Invalid XFile pointer." );

		Mesh = pXFile->Mesh; // obtain a Mesh pointer
	

		// render an object
		for ( DWORD i = 0; i < pXFile->cMtrl; i++ ) // draw each subset
		{
			D3DDevice->SetMaterial( &(pXFile->Material[i]) );
		
			// use a texture if it is defined
			if ( pXFile->pTextureIDs[i] >= 0 ) 
			{
				LPDIRECT3DTEXTURE9	tex;
				
				texStruct = ResourceManager->GetTexture( pXFile->pTextureIDs[i] ); // obtain a texture pointer

				if ( !texStruct ) ERRORMSG( ERRGENERIC, "CGraphic::RenderLightsGlows()", "Invalid texture pointer." );
				tex = texStruct->texture;

				D3DDevice->SetTexture( 0, tex );

				// render a subset of vertices
				Mesh->DrawSubset( i );
			}
			else
			{
				DWORD	state;
				D3DDevice->GetTextureStageState( 0, D3DTSS_COLOROP, &state );
				D3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DISABLE );
				Mesh->DrawSubset( i );
				D3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, state );
			}
		}
	}
*/
	
	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders whole scene
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderScene()
{
	HRESULT					hr;
	BOOL Day = Scene->SkySystem.IsDay();

#ifdef MY_DEBUG_VERSION
	if ( !Scene ) ERRORMSG( ERRINVALIDPARAMETER, "CGraphic::RenderScene()", "Fuck off with this unitialized piece of shit!" );
#endif

//APPRUNTIME T1, T2;
//T1 = FPSCounter.GetRunTime();

	// FIRST PART 

	// test whether rendering is enabled
	if ( !bRenderEnabled ) return 0;

	hr = PrepareScene();

	SetVertexShader( 0 );
	SetPixelShader( 0 );

	// obtain the actual fog color from scene
	FogColor = Scene->FogColor;

	// compute map visibility
	if ( bMapDependentRendering )
	{
		ComputeTerrainVisibility();
	}
	
	// Render LOD 0 terrain optimalization - all terrain with the lowest quality level 
	if ( bRenderTerrain ) RenderTerrainOptimalization( Day );

	// Render far objects - objects out of player area focus
	if ( bRenderFarObjects ) 
		RenderFarObjects( &MapPosition, iMapLODDefSize / 2, bRenderStaticObjects, bRenderDynamicObjects, Day );

	
	RenderMapBorders( Day );

	// bRenderDynamicObjects sky system
	if ( bRenderSkyBox ) RenderSkySystem( Day );

	if ( RenderCheckpoints ) RenderCheckpointObject(true);

	if ( bRenderLightGlows && bRenderFarObjects) RenderTransparentObjects( Day, true );

	

	// SECOND PART 

	// clear z-buffer
	D3DDevice->Clear( 0, 0, D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(255, 255, 255, 255), 1, 0 );


	// exclude from light map rendering
	if ( bMapDependentRendering ) ExcludeFromLightMapRendering( true );

	// Render day scene - ambient lights + sun
	if ( Day )
	{
		if ( bRenderAmbientLight || bRenderDirectionalLight )
		{
			hr = RenderDayScene();
			if ( hr ) ERRORMSG( hr, "CGraphic::RenderScene()", "Unable to render scene without dynamic lights." );
		}
	}
	else
	// Render night scene
	{
		if ( bDynamicLights ) hr = RenderNightScene();
		else hr = 0;
		if ( hr ) ERRORMSG( hr, "CGraphic::RenderScene()", "Unable to render scene with dynamic lights." );
	}
	
	if( bRenderGrass ) RenderGrass( Day );

	/*
	// light glows
	if ( bRenderLightGlows )
	{
		hr = RenderLightGlows();
		if ( hr ) ERRORMSG( hr, "CGraphic::RenderScene()", "Unable to render light glows." );
	}*/

	// Render checkpoints
	if ( RenderCheckpoints ) RenderCheckpointObject(false);
	
	if ( bRenderBoundingEnvelopes ) RenderBoundingEnvelopes();
	
	if ( bRenderLightGlows ) RenderTransparentObjects( Day, false );

	if ( bRenderHUD ) 
	{
		hr = HUDCore.Render();
		if (FAILED(hr) ) ERRORMSG(hr, "CGraphic::RenderScene()", "Unable to render HUD (Head-up display).");
	}

	// reinclude to light map rendering
	if ( bMapDependentRendering ) ExcludeFromLightMapRendering(false);

//T2 = FPSCounter.GetRunTime();
//OUTS("Render scene:\t",1);OUTIN(T2-T1,1);

	hr = PresentScene();

//T1 = FPSCounter.GetRunTime();
//OUTS("Present scene:\t",1);OUTIN(T1-T2,1);

	/*
	// draw visible map area on screen
	for ( UINT iz = Scene->Map.GetDepth(); iz > 0; iz-- )
	{
		CString str = "";
		for ( UINT jx = 0; jx < Scene->Map.GetWidth(); jx++ )
		{
			if ( Scene->Map.IsFieldVisible( (int) jx, (int) iz - 1 ) ) str += "@";
			else str += ".";
		}
		AddDebugMessage( str, 0, 0 );
	}*/

	return 0;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders terrain borders (wall if you want)
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderMapBorders( BOOL Day )
{
	D3DLIGHT9				Light;
	D3DXCOLOR				ambLightCol;
		

	if ( Day )
	{
		SetVertexShader( ObjectVS );
		SetPixelShader( ObjectPS );
		Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );
	}
	else // disable fog during night
	{
		SetVertexShader( ObjectNoFogVS );
		SetPixelShader( ObjectNightPS );
	}
	
	// Set ambient color
	ambLightCol.r = Scene->AmbientLight->Ambient.r;
	ambLightCol.g = Scene->AmbientLight->Ambient.g;
	ambLightCol.b = Scene->AmbientLight->Ambient.b;
	ambLightCol.a = 1.0f;
	Shader->SetPSValue( Shader->Ambient, &ambLightCol, sizeof( ambLightCol ) );
	

	// obtain or set "Sun" light
	if ( Day )
	{
		Scene->SunLight->GetD3DLight( &Light );
		Light.Direction = D3DXVECTOR3( 0, -1, 0 );
	}
	else
	{
		ZeroMemory( &Light, sizeof( Light ) );
		Light.Direction = D3DXVECTOR3( 0, 1, 0 );
	}

	// Set directional lights
	Shader->SetVSSingleDirLight( &Light );

	return Scene->Map.RenderBorders();

};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// compares the Z coordinate of two transparent objects - it is supposed to be already
//   pretransformed by World*View matrix; the output should be sorted downwards
//
//////////////////////////////////////////////////////////////////////////////////////////////
int __cdecl graphic::CompareObjectsDepth( void * pObj, const void * i0, const void * i1 )
{
	if ( ((TRANSPARENTOBJECT *)i0)->z < ((TRANSPARENTOBJECT *)i1)->z )
		return 1;
	if ( ((TRANSPARENTOBJECT *)i0)->z > ((TRANSPARENTOBJECT *)i1)->z )
		return -1;

	return 0;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders "all" transparent or semitransparent objects (actually only first 1024 are being
//   rendered)
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderTransparentObjects( BOOL day, BOOL RenderFar )
{
	D3DXVECTOR3 vec, pos;
	UINT e;
	MAPPOSITION ObjectPosition;
	int Range;

	// get count of transparent objects, if it is higher than 1024, omit the rest of them
	uiTransparentObjCount = min( (UINT) Scene->TransparentObjects.size(), MAX_TRANSPARENT_OBJECTS );
	
	// copy pointers to these objects to single array so they could be sorted easily
	e = 0;
	Range = iMapLODDefSize / 2;
	for ( UINT i = 0; i < uiTransparentObjCount; i++ )
	{
		
		// Check whether transparent object is in near square
		vec = Scene->TransparentObjects[i]->GetPosition();
		ObjectPosition.PosX = (int) (vec.x / 1000);
		ObjectPosition.PosZ = (int) (vec.z / 1000);
		if ( ObjectPosition.PosX >= MapPosition.PosX - Range && ObjectPosition.PosX <= MapPosition.PosX + Range &&
			ObjectPosition.PosZ >= MapPosition.PosZ - Range && ObjectPosition.PosZ <= MapPosition.PosZ + Range)
		{
			if (!RenderFar)
			{
				pTransparentObjects[e].pObject = Scene->TransparentObjects[i];
				D3DXVec3TransformCoord( &pos, &vec, &ViewMatrix );
				pTransparentObjects[e].z = pos.z;
				e++;
			}
		}
		else if (RenderFar)
		{
			pTransparentObjects[e].pObject = Scene->TransparentObjects[i];
			D3DXVec3TransformCoord( &pos, &vec, &ViewMatrix );
			pTransparentObjects[e].z = pos.z;
			e++;
		}
	}
	
	// sort the objects by their Z coordinate in viewer's space (downwards)
	qsort_s( pTransparentObjects, e, sizeof( *pTransparentObjects ),
			CompareObjectsDepth, this );

	// set shaders and render all the objects
	if ( day )
	{
		Shader->SetVertexShader( BillboardVS );
		Shader->SetPixelShader( BillboardPS );
		Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );
	}
	else
	{
		Shader->SetVertexShader( BillboardNightVS );
		Shader->SetPixelShader( BillboardNightPS );
	}

	CGrObjectBillboard::SetViewerPosition( Camera.GetEyePtVector() );

	D3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	D3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	for ( UINT i = 0; i < e; i++ )
		pTransparentObjects[i].pObject->Render();
	
	D3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	D3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders grass
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderGrass( BOOL Day )
{
	float				GrassTime, GrassMaxDepth;
	int					GrassRadius;
	D3DLIGHT9			Light;

	
	GrassRadius = iGrassLODDefSize / 2;
	
	// set shaders and shader constants
	Shader->SetVertexShader( GrassVS );
	if ( Day ) Shader->SetPixelShader( GrassPS );
	else Shader->SetPixelShader( GrassNightPS );

	GrassMaxDepth = ( GrassRadius + 1 ) * TERRAIN_PLATE_WIDTH;
	Shader->SetVSValue( GrassMaxDepthHandle, &GrassMaxDepth, sizeof( GrassMaxDepth ) );

	GrassTime = (float) FPSCounter.GetRunTime();
	Shader->SetVSValue( GrassTimeHandle, &GrassTime, sizeof( GrassTime ) );

	Shader->SetVSValue( GrassViewerPosHandle, Camera.GetEyePtVector(), sizeof( D3DXVECTOR3 ) );

	Scene->SunLight->GetD3DLight( &Light );
	Shader->SetVSSingleDirLight( &Light );


	D3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	//D3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

	for ( int i = GrassRadius; i > 0; i-- ) RenderGrassCycle( i );
	if ( Scene->Map.IsFieldVisible( MapPosition.PosX, MapPosition.PosZ ) &&
		Scene->pGrassFields[MapPosition.PosX + MapPosition.PosZ * Scene->Map.GetWidth()] )	
	{
		Shader->SetWorldMatrix( Scene->Map.GetPlate( MapPosition.PosX, MapPosition.PosZ )->GetWorldMatPtr() );
		Scene->pGrassFields[MapPosition.PosX + MapPosition.PosZ * Scene->Map.GetWidth()]->
			Render( GetGrassLODToUse( MapPosition.PosX, MapPosition.PosZ ) );
	}

	//D3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	D3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders grass on a section of fields
// this places grass on a boundary of square area with specified radius
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderGrassCycle( int radius )
{
	int top, bottom, left, right;
	int minx, maxx, minz, maxz, minz2, maxz2;
	int width, height;
	int index;

	left = MapPosition.PosX - radius;
	right = MapPosition.PosX + radius;
	top = MapPosition.PosZ + radius;
	bottom = MapPosition.PosZ - radius;

	width = (int) Scene->Map.GetWidth() - 1;
	height = (int) Scene->Map.GetDepth() - 1;

	minx = max( 0, left );
	maxx = min( width, right );
	minz = max( 0, bottom );
	maxz = min( height, top );
	minz2 = max( 0, bottom + 1 );
	maxz2 = min( height, top - 1 );


	// render top horizontal line
	if ( top <= maxz )
	{
		index = Scene->Map.GetWidth() * top;
		for ( int i = minx; i <= maxx; i++ ) 
			if ( Scene->Map.IsFieldVisible( index + i ) && Scene->pGrassFields[index + i] )
			{
				Shader->SetWorldMatrix( Scene->Map.GetPlate( i, top )->GetWorldMatPtr() );
				Scene->pGrassFields[index + i]->Render( GetGrassLODToUse( i, top ) );
			}
	}
	
	// render bottom horizontal line
	if ( bottom >= minz )
	{
		index = Scene->Map.GetWidth() * bottom;
		for ( int i = minx; i <= maxx; i++ ) 
			if ( Scene->Map.IsFieldVisible( index + i ) && Scene->pGrassFields[index + i] ) 
			{
				Shader->SetWorldMatrix( Scene->Map.GetPlate( i, bottom )->GetWorldMatPtr() );
				Scene->pGrassFields[index + i]->Render( GetGrassLODToUse( i, bottom ) );
			}
	}

	// render left vertical line
	if ( left >= minx )
	{
		index = Scene->Map.GetWidth() * minz2 + left;
		for ( int i = minz2; i <= maxz2; i++, index += Scene->Map.GetWidth() )
			if ( Scene->Map.IsFieldVisible( index ) && Scene->pGrassFields[index] )
			{
				Shader->SetWorldMatrix( Scene->Map.GetPlate( left, i )->GetWorldMatPtr() );
				Scene->pGrassFields[index]->Render( GetGrassLODToUse( left, i ) );
			}
	}

	// render right vertical line
	if ( right <= maxx )
	{
		index = Scene->Map.GetWidth() * minz2 + right;
		for ( int i = minz2; i <= maxz2; i++, index += Scene->Map.GetWidth() )
			if ( Scene->Map.IsFieldVisible( index ) && Scene->pGrassFields[index] )
			{
				Shader->SetWorldMatrix( Scene->Map.GetPlate( right, i )->GetWorldMatPtr() );
				Scene->pGrassFields[index]->Render( GetGrassLODToUse( right, i ) );
			}
	}


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// render all bounding envelopes - structures obtained from physical representation of scene objects
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderBoundingEnvelopes()
{
	D3DXCOLOR		color;
	D3DXMATRIX		Identity;

	color.r = color.g = color.a = 1.0f;
	color.b = 0;

	D3DXMatrixIdentity( &Identity );

	SetVertexShader( TransformVS );
	SetPixelShader( SingleColorPS );

	Shader->SetWorldMatrix( &Identity );
	Shader->SetPSValue( Shader->Ambient, &color, sizeof(color) );

	BoundingEnvelopes.Render();
	
	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders only the dialog and debug messages; this could be used in the intial game dialog
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderDialogOnly()
{
	HRESULT	hr;

	// test whether rendering is enabled
	if ( !bRenderEnabled ) return 0;

	hr = PrepareScene();

	SetVertexShader( 0 );
	SetPixelShader( 0 );

 	hr = PresentScene();
 	
 	return ERRNOERROR; 
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// prepare scene fro rendering (restores lost devices and calls beginScene on device)
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::PrepareScene()
{
	HRESULT	hr;

	D3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_XRGB( 0, 0, 0 ), 1.0f, 16 );
	hr = D3DDevice->BeginScene();

	// transformations
	if ( bViewMatrixNeedRecount ) RecountViewMatrix();

	

	D3DDevice->SetTransform( D3DTS_VIEW, &ViewMatrix );
	D3DDevice->SetTransform( D3DTS_PROJECTION, &ProjectionMatrix );
	Shader->SetViewMatrix( &ViewMatrix );
	Shader->SetProjMatrix( &ProjectionMatrix );

	SetStandardStates();

	return hr;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// presents rendered scene to front buffer
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::PresentScene()
{
	HRESULT	hr = 0;


	FPSCounter++;
	if ( pGUIDialog )
	{
		pGUIDialog->Render();

		RECT	rect = { uiRenderableWidth / 2, uiRenderableHeight - 25, uiRenderableWidth - 5, uiRenderableHeight - 5 };
		MsgFont->DrawText( NULL, UR_VERSION_STRING, -1, &rect, DT_RIGHT | DT_BOTTOM | DT_NOCLIP, MsgFontColor );
	}
	if ( bShowingDebugMsgs ) DrawDebugMsgs();
	if ( bShowingFPS ) DrawFPS();
	if ( bDrawMouseCursor ) pMouse->DrawCursor();

	D3DDevice->EndScene();

	hr = D3DDevice->Present( NULL, NULL, NULL, NULL );

	return hr;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// checks if the device is not lost
//
////////////////////////////////////////////////////////////////////////////////////////////// 
HRESULT CGraphic::ValidateLostDevice()
{
	HRESULT	hr = D3D_OK;
	
	hr = D3DDevice->TestCooperativeLevel();
	
	return hr;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Tryes to reset device
//
////////////////////////////////////////////////////////////////////////////////////////////// 
HRESULT CGraphic::ResetDevice()
{
	HRESULT hr;
	
	hr = D3DDevice->Reset( &D3DPP );

	return hr;
};

//////////////////////////////////////////////////////////////////////////////////////
//
// Must be called, when D3D device is lost
//
//////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::OnLostDevice()
{
	MsgFont->OnLostDevice();
	DebugFont->OnLostDevice();

	// Release shadow mapping render targets, ...
	ShadowMap->ReleaseD3DResources();

	// Release depth stencils
	SAFE_RELEASE(DepthTexture2);
	SAFE_RELEASE(DepthStencil);
	SAFE_RELEASE(DepthStencil2);

	// Release terrain texture
	SAFE_RELEASE(TerrainTexture);

	// Release shadow texture
	SAFE_RELEASE(pShadowTexture);

	// Release standard render target handle
	SAFE_RELEASE(StandardRenderTarget);

	// Release normal map
	SAFE_RELEASE(NormalMapTexture);

	//pGUIDialog->OnLostDevice();
	pMouse->OnLostDevice();

	HUDCore.OnLostDevice();

	return 0;
};

//////////////////////////////////////////////////////////////////////////////////////
//
// Must be called after D3D device is reset
//
//////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::OnResetDevice()
{
	HRESULT hr;
		
	// Get back standard render target
	D3DDevice->GetRenderTarget( 0, &StandardRenderTarget );

	// Set all states
	SetStandardStates();

	// Hafo of things must be reinitialized
	MsgFont->OnResetDevice();
	DebugFont->OnResetDevice();
		
	// Create depth stencils
	hr = InitDepthStencils();
	if (FAILED( hr) )
		ERRORMSG(hr, "CGraphic::OnResetDevice()", "Unable to reinit depth/stencil buffers.");
		
	// Create new shadow mapping render targets, ...
	hr = ShadowMap->CreateD3DResources();
	if (FAILED( hr ) )
		ERRORMSG(hr, "CGraphic::OnResetDevice()", "Unable to reinit shadow mapping.");

	hr = InitTerrainOptimalization();
		if (FAILED (hr) )
			ERRORMSG(hr, "CGraphic::OnResetDevice()", "Unable to reinit terrain optimalization.");

	if (Scene->Exists)
	{
		hr = InitLightMaps();
		if (FAILED( hr) )
			ERRORMSG(hr, "CGraphic::OnResetDevice()", "Unable to reinit light maps.");

		hr = InitNormalMap();
		if (FAILED(hr) )
			ERRORMSG(hr, "CGraphic::OnResetDevice()", "Unable to reinit normal map generation.");

		hr = PerformTerrainOptimalization();
		if (FAILED (hr) )
			ERRORMSG(hr, "CGraphic::OnResetDevice()", "Unable to reperform terrain optimalization.");
	}

	// create shadow texture that is used to define shadowed regions when shadow volume method is applied
	hr = D3DDevice->CreateTexture(	uiRenderableWidth,
									uiRenderableHeight,
									1,
									D3DUSAGE_RENDERTARGET,
									/*D3DFMT_R16F*/D3DFMT_A8R8G8B8,
									D3DPOOL_DEFAULT,
									&pShadowTexture,
									NULL );
	if ( hr ) ERRORMSG( hr, "CGraphic::OnResetDevice()", "An error occurs while creating shadow texture." );

	pMouse->OnResetDevice();

	HUDCore.OnResetDevice();


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////
//
// Changes resolution according to specified width and height; only few members
//   of init structure are used by this call
//
//////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::ChangeResolution( GRAPHICINIT * grInit )
{
	HRESULT hr;
	
	uiRenderableWidth = grInit->Width;
	uiRenderableHeight = grInit->Height;
	uiScreenWidth = grInit->Width;
	uiScreenHeight = grInit->Height;
	uiAnisotropicFiltering = grInit->AnisotropicFiltering;
	uiAntialiasing = grInit->Antialiasing;
	uiMultisampleQuality = grInit->MultisampleQuality;

	D3DPP.BackBufferWidth = uiRenderableWidth;
	D3DPP.BackBufferHeight = uiRenderableHeight;
	GetMultisampleType( &D3DPP.MultiSampleType, &D3DPP.MultiSampleQuality );

	hr = D3DDevice->Reset( &D3DPP );
	if ( FAILED(hr) ) ERRORMSG( hr, "CGraphic::ChangeResolution()", "Unable to change resolution." );

	StandardViewport.Height = (DWORD) uiRenderableHeight;
	StandardViewport.Width = (DWORD) uiRenderableWidth;
	D3DDevice->SetViewport(&StandardViewport);

	// Call HUDCore, so that it can recompute HUD elements position
	hr = HUDCore.OnScreenResolutionChange( uiRenderableWidth, uiRenderableHeight );
	if ( FAILED(hr) ) ERRORMSG(hr, "CGraphic::ChangeResolution()", "Unable to recompute HUD elements when screen resolution changes");

	
	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// draws the FPS count to a backbuffer
//
////////////////////////////////////////////////////////////////////////////////////////////// 
void CGraphic::DrawFPS()
{
	RECT rect;
	char ret[100];

	rect.left = 0;
	rect.right = 40;

	rect.top = 0;
	rect.bottom = MsgFontSize;
	sprintf( ret, "FPS: %.2f", FPSCounter.GetFPS() );
	MsgFont->DrawText( NULL, ret, -1, &rect, DT_LEFT | DT_TOP | DT_NOCLIP, MsgFontColor );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// draws all debug messages to backbuffer
//
////////////////////////////////////////////////////////////////////////////////////////////// 
void CGraphic::DrawDebugMsgs()
{
	RECT				rect;
	char				ret[100];
	D3DXVECTOR3			vec;
	D3DXVECTOR3 		CamDirection;

	rect.left = uiRenderableWidth - 40;
	rect.right = uiRenderableWidth - 1;

	rect.top = 0;
	rect.bottom = DebugFontSize - 1;
	sprintf( ret, "%i x %i x %i", uiRenderableWidth, uiRenderableHeight, uiColorDepth );
	DebugFont->DrawText( NULL, ret, -1, &rect, DT_RIGHT | DT_TOP | DT_NOCLIP, DebugFontColor );
	
	rect.top = DebugFontSize;
	rect.bottom = 2 * DebugFontSize - 1;
	sprintf( ret, "%i x %i", uiScreenWidth, uiScreenHeight );
	DebugFont->DrawText( NULL, ret, -1, &rect, DT_RIGHT | DT_TOP | DT_NOCLIP, DebugFontColor );

	rect.top = 2 * DebugFontSize;
	rect.bottom = 3 * DebugFontSize;
	Camera.GetEyePtVector( &vec );
	sprintf( ret, "[%.0f,%.0f,%.0f]", vec.x, vec.y, vec.z );
	DebugFont->DrawText( NULL, ret, -1, &rect, DT_RIGHT | DT_TOP | DT_NOCLIP, DebugFontColor );

    // camera direction
 	rect.top = 3 * DebugFontSize;
	rect.bottom = 4 * DebugFontSize;
	CamDirection = Camera.GetLookAtVector() - Camera.GetEyePtVector();
	D3DXVec3Normalize(&CamDirection, &CamDirection);
	sprintf(ret, "[%.3f, %.3f, %.3f]", CamDirection.x, CamDirection.y, CamDirection.z);
	DebugFont->DrawText( NULL, ret, -1, &rect, DT_RIGHT | DT_TOP | DT_NOCLIP, DebugFontColor );

	// Sky time
	rect.top = 4 * DebugFontSize;
	rect.bottom = 5 * DebugFontSize;
	sprintf(ret, "Sky time: %.3f", Scene->SkySystem.GetSkyTime() );
	DebugFont->DrawText( NULL, ret, -1, &rect, DT_RIGHT | DT_TOP | DT_NOCLIP, DebugFontColor );


	pDebugTextOutput->Render();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// activates a rendering mechanism
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::Activate()
{
	static bool WasCalled = false;
	
	if ( !WasCalled ) FPSCounter.Start();
	else FPSCounter.Continue();

	WasCalled = true;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// deactivates a rendering mechanism
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::Deactivate()
{
	FPSCounter.Continue();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// detects (on the base of known D3DCaps), whether the HW vertex processing can be used
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGraphic::CanUseHWVertexProcessing()
{
	return ( D3DCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// if caps parameter is not specified, this fills D3DCaps member with valid device
// capabilities info; if caps is valid is specified it'll be filled instead of D3DCaps
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::GetDeviceCaps( D3DCAPS9 *pCaps )
{
	if ( pCaps ) Direct3D->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, pCaps );
	else Direct3D->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &D3DCaps );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets LeftHanded perspective transformation parameters
//
// fieldOfViewY is an viewable angle in y-axis direction,
// aspect represents aspect ration defined as view space height divided by width,
// zNear, zFar are the near and far clipping planes which define at what distances geometry
// should no longer be rendered
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetProjectionParameters( FLOAT fieldOfViewY, FLOAT aspect, FLOAT zNear, FLOAT zFar )
{
	fFieldOfViewY = fieldOfViewY;
	fViewAspectRatioXY = aspect;
	fZNear = zNear;
	fZFar = zFar;

	D3DXMatrixPerspectiveFovLH( &ProjectionMatrix, fieldOfViewY, aspect, zNear, zFar );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets camera LookAt vector
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetLookAtVector( D3DXVECTOR3 vec )
{
	Camera.SetLookAtVector( vec );
	bViewMatrixNeedRecount = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets camera LookAt vector
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetLookAtVector( FLOAT x, FLOAT y, FLOAT z )
{
	Camera.SetLookAtVector( x, y ,z );
	bViewMatrixNeedRecount = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets camera EyePoint vector
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetEyePtVector( D3DXVECTOR3 vec )
{
	Camera.SetEyePtVector( vec );
	bViewMatrixNeedRecount = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets camera EyePoint vector
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetEyePtVector( FLOAT x, FLOAT y, FLOAT z )
{
	Camera.SetEyePtVector( x, y, z );
	bViewMatrixNeedRecount = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets camera Up vector
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetUpVector( D3DXVECTOR3 vec )
{
	Camera.SetUpVector( vec );
	bViewMatrixNeedRecount = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets camera Up vector
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetUpVector( FLOAT x, FLOAT y, FLOAT z )
{
	Camera.SetUpVector( x, y, z );
	bViewMatrixNeedRecount = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// recounts view matrix from camera parameters
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::RecountViewMatrix()
{
	D3DXVECTOR3	eye, look, up;

	D3DXMatrixLookAtLH(
		&ViewMatrix,
		Camera.GetEyePtVector( &eye ),
		Camera.GetLookAtVector( &look ),
		Camera.GetUpVector( &up )
	);
	bViewMatrixNeedRecount = FALSE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Moves a camera
// direction is one of the macros defined in Camera.h
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::MoveCamera( int Direction )
{
	Camera.MoveCamera( Direction );
	bViewMatrixNeedRecount = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// rotates a camera in specified direction in a camera coordinates
// direction is one of the macros defined in Camera.h
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::RotateCamera( int Direction )
{
	Camera.RotateCamera( Direction );
	bViewMatrixNeedRecount = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// rotates a camera - mouse look; this rotates a camera by an angle in specified world direction
// direction is one of the macros defined in Camera.h
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::RotateCameraM( int Direction, float Angle )
{
	Camera.RotateCameraM( Direction, Angle );
	bViewMatrixNeedRecount = TRUE;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets predefined standard render and texture states
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetStandardStates()
{
	

	D3DDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	D3DDevice->SetRenderState( D3DRS_AMBIENT, 0);
	D3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ); 
	D3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );

	if ( uiAntialiasing || uiMultisampleQuality ) D3DDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
	else D3DDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, FALSE );
	
	//alpha blending
	D3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	// texture resizing    
	if ( uiAnisotropicFiltering )
	{
		D3DDevice->SetSamplerState( 0, D3DSAMP_MAXANISOTROPY, uiAnisotropicFiltering );
		D3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		D3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC );
		D3DDevice->SetSamplerState( 1, D3DSAMP_MAXANISOTROPY, uiAnisotropicFiltering );
		D3DDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		D3DDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC );
		D3DDevice->SetSamplerState( 2, D3DSAMP_MAXANISOTROPY, uiAnisotropicFiltering );
		D3DDevice->SetSamplerState( 2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		D3DDevice->SetSamplerState( 2, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC );
	}
	else
	{
		D3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		D3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		D3DDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		D3DDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		D3DDevice->SetSamplerState( 2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		D3DDevice->SetSamplerState( 2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	}

	D3DDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	D3DDevice->SetSamplerState( 1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	D3DDevice->SetSamplerState( 2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	D3DDevice->SetSamplerState( 3, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	D3DDevice->SetSamplerState( 3, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	D3DDevice->SetSamplerState( 3, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	D3DDevice->SetSamplerState( 4, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	D3DDevice->SetSamplerState( 4, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	D3DDevice->SetSamplerState( 4, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
 
 	//multitexturing
	D3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
	D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
	//D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	//D3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );

	D3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
	//D3DDevice->SetSamplerState( 1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	//D3DDevice->SetSamplerState( 1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
	D3DDevice->SetSamplerState( 1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	D3DDevice->SetSamplerState( 1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
	

	D3DDevice->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 2 );
	D3DDevice->SetSamplerState( 2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	D3DDevice->SetSamplerState( 2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

    D3DDevice->SetTextureStageState( 3, D3DTSS_TEXCOORDINDEX, 3 );
	D3DDevice->SetSamplerState( 3, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	D3DDevice->SetSamplerState( 3, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	// color source
	D3DDevice->SetRenderState( D3DRS_COLORVERTEX, FALSE );
	D3DDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
	D3DDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL );
	D3DDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the Camera object's properties
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetCamera( CCamera *cam )
{
	D3DXVECTOR3	vec;

	this->Camera.SetEyePtVector( *(cam->GetEyePtVector( &vec )) );
	this->Camera.SetLookAtVector( *(cam->GetLookAtVector( &vec )) );
	this->Camera.SetUpVector( *(cam->GetUpVector( &vec )) );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// fills the parameter with actual Camera object's properties
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::GetCamera( CCamera *cam )
{
	D3DXVECTOR3	vec;

	cam->SetEyePtVector( *(this->Camera.GetEyePtVector( &vec )) );
	cam->SetLookAtVector( *(this->Camera.GetLookAtVector( &vec )) );
	cam->SetUpVector( *(this->Camera.GetUpVector( &vec )) );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the x-coordinate of the viewer in the map
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetMapPosX( int x )
{
	if ( x >= 0 && x < (int) Scene->Map.GetWidth() ) MapPosition.PosX = x;
}

		
//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the z-coordinate of the viewer in the map
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetMapPosZ( int z )
{
	if ( z >= 0 && z < (int) Scene->Map.GetDepth() ) MapPosition.PosZ = z;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// computes the LOD which to use to render specified terrain field on the base of
// the field's distance from a viewer and renders it
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderTerrainField( int x, int z )
{
	Scene->Map.RenderField( x, z, GetTerrainLODToUse( x, z ) );
	
	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// computes the LOD which to use to render specified terrain field on the base of
// the field's distance from a viewer and renders it
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderTerrainField( MAPPOSITION *pos )
{
	Scene->Map.RenderField( pos->PosX, pos->PosZ, GetTerrainLODToUse( pos->PosX, pos->PosZ ) );

	return ERRNOERROR;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//
// computes the LOD which to use to render specified terrain field on the base of
// the field's distance from a viewer and renders it, second rendering mode for
// hiding holes in the terrain
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderTerrainField(int x, int z, BOOL firstPass)
{
	int iX, iZ;

	if ( firstPass )
	{
				Scene->Map.RenderField( x, z, GetTerrainLODToUse( x, z ) );
	}
	else
	{
		if ( Scene->Map.GetPlate( x, z )->IsPlanar() ) return ERRNOERROR;
		iX =  pTerrainLoDMap->iWidth / 2 - (x - MapPosition.PosX);
		iZ =  pTerrainLoDMap->iHeight / 2 - (z - MapPosition.PosZ);
		if ( iX < 0 || iZ < 0 || iX >= pTerrainLoDMap->iWidth || iZ >= pTerrainLoDMap->iHeight ) return ERRNOERROR;
		if ( pLODBorders[iZ * pTerrainLoDMap->iWidth + iX] )
				Scene->Map.RenderField( x, z, pTerrainLoDMap->pData[iZ * pTerrainLoDMap->iWidth + iX], true );
	}
	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// for each terrain field specified by a rectangular area this method computes the LOD 
// which to use on the base of the field's distance from a viewer; then renders those fields
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderTerrainSection( int x1, int z1, int x2, int z2 )
{
	int		_x1, _x2, _z1, _z2;

	_x1 = min( x1, x2 );
	_x2 = max( x1, x2 );
	_z1 = min( z1, z2 );
	_z2 = max( z1, z2 );

	for ( int i = _z1; i <= _z2; i++ )
	{
		for ( int j = _x1; j <= _x2; j++ )
		{
			Scene->Map.RenderField( j, i, GetTerrainLODToUse( j, i ) );
		}
	}

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// for each terrain field specified by a square area this method computes the LOD 
// which to use on the base of the field's distance from a viewer; then renders those fields
//
// range of 0 means only one field, whereas the range = 1 means a 3x3 square area
//
// set firstPass to false to announce second pass per frame; on such pass only non-planer
//   terrain plates neighbouring to higher LOD level are being rendered 
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderTerrainSection( int x, int z, int range, bool firstPass )
{
	int		iX, iZ;
/*	
	MAPPOSITION MapPos;
	FieldStruct F;

	// push it in
	int Num = 0;
	
	for (int i = max(z - range, 0); i <= min(z + range, Scene->Map.GetDepth() - 1); i++)
		for (int j = max(x - range, 0); j <= min(x + range, Scene->Map.GetWidth() - 1); j++)
		{
			F.MapPos.PosX = j;
			F.MapPos.PosZ = i;
			F.TexID = Scene->Map.GetFieldTextureID(j, i);
			
			FieldOrder[Num] = F;
			Num++;
		}

	// sort it based on texture id
	qsort(FieldOrder, Num, sizeof(FieldStruct), CompareFields);

	for (int i = 0; i < Num; i++)
	{
		Scene->Map.RenderField( FieldOrder[i].MapPos.PosX, FieldOrder[i].MapPos.PosZ,
			GetTerrainLODToUse(FieldOrder[i].MapPos.PosX, FieldOrder[i].MapPos.PosZ) );
	}
*/
	if ( firstPass )
	{
		for ( int i = z - range; i <= z + range; i++ )
		{
			for ( int j = x - range; j <= x + range; j++ )
			{
				Scene->Map.RenderField( j, i, GetTerrainLODToUse( j, i ) );
			}
		}
	}
	else
	{
		for ( int i = max(0, z-range); i <= min((int)Scene->Map.GetDepth()-1, z+range); i++ )
		{
			for ( int j = max(0, x-range); j <= min((int)Scene->Map.GetWidth()-1, x+range); j++ )
			{
				if ( Scene->Map.GetPlate( j, i )->IsPlanar() ) continue;
				
				iX =  pTerrainLoDMap->iWidth / 2 - (j - MapPosition.PosX);
				iZ =  pTerrainLoDMap->iHeight / 2 - (i - MapPosition.PosZ);
				if ( iX < 0 || iZ < 0 || iX >= pTerrainLoDMap->iWidth || iZ >= pTerrainLoDMap->iHeight ) continue;
				if ( pLODBorders[iZ * pTerrainLoDMap->iWidth + iX] )
					Scene->Map.RenderField( j, i, pTerrainLoDMap->pData[iZ * pTerrainLoDMap->iWidth + iX], true );
			}
		}
	}


	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// for each terrain field specified by a square area this method computes the LOD 
// which to use on the base of the field's distance from a viewer; then renders those fields
//
// range of 0 means only one field, whereas the range = 1 means a 3x3 square area
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::RenderTerrainSection( MAPPOSITION *pos, int range )
{
	for ( int i = pos->PosZ - range; i <= pos->PosZ + range; i++ )
	{
		for ( int j = pos->PosX - range; j <= pos->PosX + range; j++ )
		{
			Scene->Map.RenderField( j, i, GetTerrainLODToUse( j, i ) );
		}
	}

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// says whether the specified view-transformed point is in viewing frustum so it could be visible
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGraphic::PointIsInViewFrustum( FLOAT x, FLOAT y, FLOAT z, FLOAT w )
{
	D3DXVECTOR4		vecOut, vecIn( x, y, z, w );

	D3DXVec4Transform( &vecOut, &vecIn, &ProjectionMatrix );

	return ( (vecOut.x > -vecOut.w) && (vecOut.x <= vecOut.w) && (vecOut.y > -vecOut.w)
			&& (vecOut.y <= vecOut.w) && (vecOut.z > 0) && (vecOut.z <= vecOut.w) );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// says whether the specified view-transformed point is in viewing frustum so it could be visible
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGraphic::PointIsInViewFrustum( D3DXVECTOR4 *vec )
{
	D3DXVECTOR4		vecOut;

	D3DXVec4Transform( &vecOut, vec, &ProjectionMatrix );

	return ( (vecOut.x > -vecOut.w) && (vecOut.x <= vecOut.w) && (vecOut.y > -vecOut.w)
			&& (vecOut.y <= vecOut.w) && (vecOut.z > 0) && (vecOut.z <= vecOut.w) );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// says whether the specified point in screen space is visible
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGraphic::PointIsInVisibleScreen( FLOAT x, FLOAT y, FLOAT z, FLOAT w )
{
	return ( x >= 0 && x <= 800 && y >= 0 && y <= 600 );// && z <= 1 );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// transforms the vertex and computes its Cohen-Sutherland code
// returns true if the vertex is in view frustum
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGraphic::CohenSutherlandCode( D3DXVECTOR4 *vert, BYTE &CScode )
{
	D3DXVECTOR4		vec;

	D3DXVec4Transform( &vec, vert, &ProjectionMatrix );

	CScode = 0;
	if ( vec.x <= -vec.w )	CScode += 1;
	if ( vec.x > vec.w )	CScode += 2;
	if ( vec.y <= -vec.w )	CScode += 4;
	if ( vec.y > vec.w )	CScode += 8;
	if ( vec.z <= 0 )		CScode += 16;
	if ( vec.z > vec.w )	CScode += 32;


	return ( (vec.x > -vec.w) && (vec.x <= vec.w) && (vec.y > -vec.w)
			&& (vec.y <= vec.w) && (vec.z > 0) && (vec.z <= vec.w) );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// computes the visibility of all terrain fields
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::ComputeTerrainVisibility()
{
	APPRUNTIME						time, time2;

	time = FPSCounter.GetRunTime();
 /*
	std::vector<__BOX>				boxes;
	std::vector<__BOX>::iterator	boxIter;
	D3DXVECTOR4						newPoints[18];
	__BOX							oBox;
	FLOAT							width, height, depth; 
	BOOL							isVisible[18], isIn, visible;
	FLOAT							dX, dZ;
	int								pX, pZ, x1, x2, z1, z2;
	BYTE							CScodes[18];
	BOOL							Edges[33]; // specifies the visibility of each segment (=edge)


	Scene->Map.SetWholeMapInvisible();


	height = TERRAIN_PLATE_HEIGHT * 7;
	width = TERRAIN_PLATE_WIDTH * Scene->Map.GetWidth();
	depth = TERRAIN_PLATE_WIDTH * Scene->Map.GetDepth();

	// set the vertices of box that is surrounding whole scene
	oBox.x1 = 0;
	oBox.z1 = 0;
	oBox.x2 = Scene->Map.GetWidth() - 1;
	oBox.z2 = Scene->Map.GetDepth() - 1;
	
	newPoints[0] = D3DXVECTOR4( 0.0f,	height,	depth,	1.0f );
	newPoints[1] = D3DXVECTOR4( width,	height,	depth,	1.0f );
	newPoints[2] = D3DXVECTOR4( 0.0f,	height,	0.0f,	1.0f );
	newPoints[3] = D3DXVECTOR4( width,	height,	0.0f,	1.0f );
	newPoints[4] = D3DXVECTOR4( 0.0f,	0.0f,	depth,	1.0f );
	newPoints[5] = D3DXVECTOR4( width,	0.0f,	depth,	1.0f );
	newPoints[6] = D3DXVECTOR4( 0.0f,	0.0f,	0.0f,	1.0f );
	newPoints[7] = D3DXVECTOR4( width,	0.0f,	0.0f,	1.0f );

	D3DXVec4Transform( &(oBox.transPoints[0]), &newPoints[0], &ViewMatrix );
	D3DXVec4Transform( &(oBox.transPoints[1]), &newPoints[1], &ViewMatrix );
	D3DXVec4Transform( &(oBox.transPoints[2]), &newPoints[2], &ViewMatrix );
	D3DXVec4Transform( &(oBox.transPoints[3]), &newPoints[3], &ViewMatrix );
	D3DXVec4Transform( &(oBox.transPoints[4]), &newPoints[4], &ViewMatrix );
	D3DXVec4Transform( &(oBox.transPoints[5]), &newPoints[5], &ViewMatrix );
	D3DXVec4Transform( &(oBox.transPoints[6]), &newPoints[6], &ViewMatrix );
	D3DXVec4Transform( &(oBox.transPoints[7]), &newPoints[7], &ViewMatrix );

	boxes.push_back( oBox );

	// while there are any boxes on the stack
	while ( !boxes.empty() )
	{
		__BOX		&iBox = boxes.back();

		x1 = iBox.x1;
		x2 = iBox.x2;
		z1 = iBox.z1;
		z2 = iBox.z2;

		// compute the middle fields and an interpolants
		pX = (x2 - x1 + 1) / 2;
		pZ = (x2 - x1 + 1) / 2;
		dX = (float) pX / (float) (x2 - x1 + 1);
		dZ = (float) pZ / (float) (z2 - z1 + 1);

		// check whether the viewer is not in the box
		isIn = ( ( MapPosition.PosX >= x1 ) && ( MapPosition.PosX <= x2 ) 
			&& ( MapPosition.PosZ >= z1 ) && ( MapPosition.PosZ <= z2 ) );

		// get already counted vertices
		newPoints[0] = iBox.transPoints[0];
		newPoints[2] = iBox.transPoints[1];
		newPoints[6] = iBox.transPoints[2];
		newPoints[8] = iBox.transPoints[3];
		newPoints[9] = iBox.transPoints[4];
		newPoints[11] = iBox.transPoints[5];
		newPoints[15] = iBox.transPoints[6];
		newPoints[17] = iBox.transPoints[7];
		
		// remove the box from the stack
		boxes.pop_back();

		// compute another vertices by interpolating those known
		D3DXVec4Lerp( &newPoints[1], &newPoints[0], &newPoints[2], dX );
		D3DXVec4Lerp( &newPoints[3], &newPoints[6], &newPoints[0], dZ );
		D3DXVec4Lerp( &newPoints[5], &newPoints[8], &newPoints[2], dZ );
		D3DXVec4Lerp( &newPoints[4], &newPoints[3], &newPoints[5], dX );
		D3DXVec4Lerp( &newPoints[7], &newPoints[6], &newPoints[8], dX );
		D3DXVec4Lerp( &newPoints[10], &newPoints[9], &newPoints[11], dX );
		D3DXVec4Lerp( &newPoints[12], &newPoints[15], &newPoints[9], dZ );
		D3DXVec4Lerp( &newPoints[14], &newPoints[17], &newPoints[11], dZ );
		D3DXVec4Lerp( &newPoints[13], &newPoints[12], &newPoints[14], dX );
		D3DXVec4Lerp( &newPoints[16], &newPoints[15], &newPoints[17], dX );

		// compute Cohen Sutherland codes and select visible vertices and edges
		for ( int i = 0; i < 18; i++ ) isVisible[i] = CohenSutherlandCode( &newPoints[i], CScodes[i] );
		Edges[0] = CScodes[0] & CScodes[1] ? FALSE : TRUE;
		Edges[1] = CScodes[2] & CScodes[1] ? FALSE : TRUE;
		Edges[2] = CScodes[3] & CScodes[4] ? FALSE : TRUE;
		Edges[3] = CScodes[4] & CScodes[5] ? FALSE : TRUE;
		Edges[4] = CScodes[7] & CScodes[6] ? FALSE : TRUE;
		Edges[5] = CScodes[7] & CScodes[8] ? FALSE : TRUE;
		Edges[6] = CScodes[0] & CScodes[3] ? FALSE : TRUE;
		Edges[7] = CScodes[4] & CScodes[1] ? FALSE : TRUE;
		Edges[8] = CScodes[2] & CScodes[5] ? FALSE : TRUE;
		Edges[9] = CScodes[6] & CScodes[3] ? FALSE : TRUE;
		Edges[10] = CScodes[4] & CScodes[7] ? FALSE : TRUE;
		Edges[11] = CScodes[8] & CScodes[5] ? FALSE : TRUE;
		Edges[12] = CScodes[10] & CScodes[9] ? FALSE : TRUE;
		Edges[13] = CScodes[10] & CScodes[11] ? FALSE : TRUE;
		Edges[14] = CScodes[12] & CScodes[13] ? FALSE : TRUE;
		Edges[15] = CScodes[13] & CScodes[14] ? FALSE : TRUE;
		Edges[16] = CScodes[15] & CScodes[16] ? FALSE : TRUE;
		Edges[17] = CScodes[16] & CScodes[17] ? FALSE : TRUE;
		Edges[18] = CScodes[9] & CScodes[12] ? FALSE : TRUE;
		Edges[19] = CScodes[10] & CScodes[13] ? FALSE : TRUE;
		Edges[20] = CScodes[11] & CScodes[14] ? FALSE : TRUE;
		Edges[21] = CScodes[15] & CScodes[12] ? FALSE : TRUE;
		Edges[22] = CScodes[13] & CScodes[16] ? FALSE : TRUE;
		Edges[23] = CScodes[17] & CScodes[14] ? FALSE : TRUE;
		Edges[24] = CScodes[0] & CScodes[9] ? FALSE : TRUE;
		Edges[25] = CScodes[10] & CScodes[1] ? FALSE : TRUE;
		Edges[26] = CScodes[2] & CScodes[11] ? FALSE : TRUE;
		Edges[27] = CScodes[3] & CScodes[12] ? FALSE : TRUE;
		Edges[28] = CScodes[4] & CScodes[13] ? FALSE : TRUE;
		Edges[29] = CScodes[5] & CScodes[14] ? FALSE : TRUE;
		Edges[30] = CScodes[6] & CScodes[15] ? FALSE : TRUE;
		Edges[31] = CScodes[7] & CScodes[16] ? FALSE : TRUE;
		Edges[32] = CScodes[8] & CScodes[17] ? FALSE : TRUE;

		visible = isIn;
		if ( !isIn )
		{
			for ( int i = 0; i < 18; i++ ) if ( isVisible[i] ) visible = TRUE;
			for ( int i = 0; i < 33; i++ ) if ( Edges[i] ) visible = TRUE;
		}

 		// the bottom of the recursion - any of the dimensions of the box counts only one field
		if ( x1 == x2 || z1 == z2 )
		{
			if ( visible )
			{			
				for ( int i = x1; i <= x2; i++ ) 
					for ( int j = z1; j <= z2; j++ ) Scene->Map.SetFieldVisible( i, j );
			}
			continue;
		}

		// add 4 new boxes on the stack
		
		if ( isIn || isVisible[4] || isVisible[5] || isVisible[7] || isVisible[8] || isVisible[13] 
			|| isVisible[14] || isVisible[16] || isVisible[17] || Edges[3] || Edges[5] || Edges[11] || Edges[10] 
			|| Edges[15] || Edges[17] || Edges[22] || Edges[23] || Edges[28] || Edges[29] || Edges[31] || Edges[32] )
		{
			oBox.x1 = x1 + pX;
			oBox.x2 = x2;
			oBox.z1 = z1;
			oBox.z2 = z1 + pZ - 1;
			oBox.transPoints[0] = newPoints[4];
			oBox.transPoints[1] = newPoints[5];
			oBox.transPoints[2] = newPoints[7];
			oBox.transPoints[3] = newPoints[8];
			oBox.transPoints[4] = newPoints[13];
			oBox.transPoints[5] = newPoints[14];
			oBox.transPoints[6] = newPoints[16];
			oBox.transPoints[7] = newPoints[17];
			boxes.push_back( oBox );
		}
	
		if ( isIn || isVisible[0] || isVisible[1] || isVisible[3] || isVisible[4] || isVisible[9] 
			|| isVisible[10] || isVisible[12] || isVisible[13] || Edges[0] || Edges[7] || Edges[2] || Edges[28] 
			|| Edges[6] || Edges[12] || Edges[19] || Edges[14] || Edges[18] || Edges[24] || Edges[25] || Edges[27]  ) 
		{
			oBox.x1 = x1;
			oBox.x2 = x1 + pX - 1;
			oBox.z1 = z1 + pZ;
			oBox.z2 = z2;
			oBox.transPoints[0] = newPoints[0];
			oBox.transPoints[1] = newPoints[1];
			oBox.transPoints[2] = newPoints[3];
			oBox.transPoints[3] = newPoints[4];
			oBox.transPoints[4] = newPoints[9];
			oBox.transPoints[5] = newPoints[10];
			oBox.transPoints[6] = newPoints[12];
			oBox.transPoints[7] = newPoints[13];
			boxes.push_back( oBox );
		}

		if ( isIn || isVisible[1] || isVisible[2] || isVisible[4] || isVisible[5] || isVisible[10] 
			|| isVisible[11] || isVisible[13] || isVisible[14] || Edges[1] || Edges[7] || Edges[8] || Edges[3] 
			|| Edges[13] || Edges[15] || Edges[19] || Edges[20] || Edges[25] || Edges[26] || Edges[28] || Edges[29]  ) 
		{
			oBox.x1 = x1 + pX;
			oBox.x2 = x2;
			oBox.z1 = z1 + pZ;
			oBox.z2 = z2;
			oBox.transPoints[0] = newPoints[1];
			oBox.transPoints[1] = newPoints[2];
			oBox.transPoints[2] = newPoints[4];
			oBox.transPoints[3] = newPoints[5];
			oBox.transPoints[4] = newPoints[10];
			oBox.transPoints[5] = newPoints[11];
			oBox.transPoints[6] = newPoints[13];
			oBox.transPoints[7] = newPoints[14];
			boxes.push_back( oBox );
		}
		if ( isIn || isVisible[3] || isVisible[4] || isVisible[6] || isVisible[7] || isVisible[12] 
			|| isVisible[13] || isVisible[15] || isVisible[16] || Edges[2] || Edges[4] || Edges[9] || Edges[10] 
			|| Edges[14] || Edges[16] || Edges[21] || Edges[22] || Edges[27] || Edges[28] || Edges[30] || Edges[31]  )
		{
            oBox.x1 = x1;
			oBox.x2 = x1 + pX - 1;
			oBox.z1 = z1;
			oBox.z2 = z1 + pZ - 1;
			oBox.transPoints[0] = newPoints[3];
			oBox.transPoints[1] = newPoints[4];
			oBox.transPoints[2] = newPoints[6];
			oBox.transPoints[3] = newPoints[7];
			oBox.transPoints[4] = newPoints[12];
			oBox.transPoints[5] = newPoints[13];
			oBox.transPoints[6] = newPoints[15];
			oBox.transPoints[7] = newPoints[16];
			boxes.push_back( oBox );
		}
	}
/**/


	D3DVIEWPORT9		viewPort;
	
	viewPort.X = viewPort.Y = 0;
	viewPort.Width = uiRenderableWidth;
	viewPort.Height = uiRenderableHeight;
	viewPort.MinZ = 0.0f;
	viewPort.MaxZ = 1.0f;
	Scene->Map.ComputeTerrainVisibility( &viewPort, &ViewMatrix, GetFieldOfViewX(), fFieldOfViewY, fZNear, fZFar );
	Scene->Map.SetFieldVisible( &MapPosition );
	
	time2 = FPSCounter.GetRunTime(); 
	time = time2 - time;
	//OUTIN( time, 2 );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads a mouse cursor from a file and applies it
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::LoadMouseCursor( LPCTSTR fileName, int * index )
{
	HRESULT		hr;

	hr = pMouse->AddCursor( fileName, index );
	if ( hr ) ERRORMSG( hr, "CGraphic::LoadMouseCursor()", "Adding mouse cursor unsuccessul." );

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the actual mouse cursor from those already loaded
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetMouseCursor( int index )
{
	pMouse->SelectCursor( index );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the actual mouse cursor
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::SetMouseCursor( CMouseCursor * pCursor )
{
	pMouse->SetCursor( pCursor );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the actual pixel shader
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::SetPixelShader( const UINT PixelShader )
{
	CGrObjectBase::bPixelShader = ( PixelShader != 0 );
	return Shader->SetPixelShader( PixelShader );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Sets the actual vertex shader
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::SetVertexShader( const UINT VertexShader )
{
	CGrObjectBase::bVertexShader = ( VertexShader != 0 );
	return Shader->SetVertexShader( VertexShader );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Renders scene with shadow mapping
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::RenderWithShadowMapping()
{
	
	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Pre-renders static light maps with shadow
// 1) Iterates through all LAMP lights in scene
//    Checks for terrain plate type for each lamp    
// 2) For each DISTINCT terrain plate type generate light map with shadow
//    a) Create light map texture of appropriate size
//    b) FIRST PASS - Render depth shadow texture using shaders
//    c) SECOND PASS - Render spot light and add shadows to light map texture
// 3) For each terrain plate, on which is lamp, set light map texture of given terrain plate type
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::InitLightMaps()
{
	HRESULT hr;
	D3DXMATRIX LightViewMat; // Light view matrix
	D3DXMATRIX LightProjMat; // Light projection matrix
	D3DXMATRIX AllMat; // All matrix
	D3DXMATRIX ShadowMat; // Shadow matrix - from view space to texture space on shadow map

	D3DXMATRIX MapProjMat; // light map projection matrix
	D3DXMATRIX MapViewMat; // Light map view matrix
	D3DXMATRIX InvMapViewMat; // Inversed light map view matrix
	D3DXMATRIX PlateWorldMat; // Terrain plate world matrix
	D3DXMATRIX InvPlateWorldMat; // Inversed terrain plate world matrix

	resManNS::RESOURCEID TerrainType; // Terrain type
	TypeLightPos DistinctType = TypeLightPos(-1, 0, 0);
	CLight * Light; // Light
	multimap<TypeLightPos, CLight*> TerrainTypes; // contains all search types used
	LIGHTS_ITER It;
	multimap<TypeLightPos, CLight *>::iterator TypeIt; 
	D3DLIGHT9 SpotLight; // Spot light

	CGrObjectTerrainPlate * Plate; // current plate
	IDirect3DTexture9 * LightMap; // Light map texture
	UINT LightMapNum; // Handle to light map container
	IDirect3DSurface9 * MapSurface; // Shadow map surface
	IDirect3DTexture9 * ProjectedTex; // Projected shadow texture
	
	D3DVIEWPORT9 LightMapView; // Light Map viewport
	
	// Compute light map projection matrix
	D3DXMatrixOrthoLH(&MapProjMat, 1000.0f, 1000.0f, 1.0f, 1200.0f);

	// clear all light maps from container
	ShadowMap->ClearLightMaps();

	// create texture to render projected shadow of light
	hr = D3DDevice->CreateTexture(ShadowMap->GetLightMapSize(), ShadowMap->GetLightMapSize(), 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
				D3DPOOL_DEFAULT, &ProjectedTex  , 0);
			if (FAILED(hr) )
			{
				ERRORMSG(hr, "CGraphic::InitLightMaps()", "Create projective texture failed");
			}
	

	// list all terrain plates and set their light map to black
	for (unsigned int x = 0; x < Scene->Map.GetWidth(); x++)
		for (unsigned int z = 0; z < Scene->Map.GetDepth(); z++)
			//Scene->Map.GetPlate(x, z)->SetLightMap(ShadowMap->GetLightMap(0) );
			  Scene->Map.GetPlate(x, z)->SetLightMap(InvisibleTexture );

		
	// Search through all lights of type LAMP and save their terrain plate type
	for (It = Scene->Lights.begin(); It != Scene->Lights.end(); It++)
	{
		if ((*It)->Type != LT_LAMP) continue;
		TerrainType = Scene->Map.GetPlate((*It)->MapPosition.PosX, (*It)->MapPosition.PosZ)->GetTerrainType();
		TerrainTypes.insert(
			pair<TypeLightPos, CLight *>( TypeLightPos(TerrainType, 
													   ((int) (*It)->Position.x) % (int) TERRAIN_PLATE_WIDTH,
													   ((int) (*It)->Position.z) % (int) TERRAIN_PLATE_WIDTH),
													   (*It) ) );
	}

	// for each terrain plate type, render light map with shadow from the light stand
	for (TypeIt = TerrainTypes.begin(); TypeIt != TerrainTypes.end(); TypeIt++)
	{
		Light = TypeIt->second;
		// Get terrain plate object
		Plate = Scene->Map.GetPlate(Light->MapPosition.PosX, Light->MapPosition.PosZ);
		
		// Render light map for new terrain type
		if (DistinctType != TypeIt->first)
		{
			DistinctType = TypeIt->first;
			
			ShadowMap->ComputeLightViewProjMat(&LightViewMat, &LightProjMat, Light);

			// Set maximum quality LOD
			Plate->SetLOD( uiTerrainLOD );
		
			// Create light map texture containing static shadows
			hr = D3DDevice->CreateTexture(ShadowMap->GetLightMapSize(), ShadowMap->GetLightMapSize(), 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
				D3DPOOL_DEFAULT, & LightMap  , 0);
			if (FAILED(hr) )
			{
				ERRORMSG(hr, "CGraphic::InitLightMaps()", "Create light map texture failed");
			}

			LightMapView.X = 0;
			LightMapView.Y = 0;
			LightMapView.Width = ShadowMap->GetLightMapSize();
			LightMapView.Height = ShadowMap->GetLightMapSize();
			LightMapView.MinZ = 0;
			LightMapView.MaxZ = 1;

			// Render projected shadow
			ProjectedTex->GetSurfaceLevel(0, &MapSurface);
			D3DDevice->SetRenderTarget(0, MapSurface);
			D3DDevice->SetViewport(&LightMapView);
			
			D3DDevice->Clear(0,0,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(255,255,255,255), 1, 0);

			SetVertexShader(TransformVS);
			SetPixelShader(ZeroPS);

			Shader->SetViewMatrix(&LightViewMat);
			Shader->SetProjMatrix(&LightProjMat);

			D3DDevice->BeginScene();
				Light->pEmitor->Render();
			D3DDevice->EndScene();

			SAFE_RELEASE(MapSurface);

			//D3DXSaveTextureToFile("ProjectedTex.jpg", D3DXIFF_JPG, ProjectedTex, 0);

			// Get light map surface
			LightMap->GetSurfaceLevel(0, &MapSurface);

			// Set render target and clear it
			D3DDevice->SetRenderTarget(0, MapSurface);
			
			D3DDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0,0, 0, 255), 1, 0);

			// compute and set matrixes
			PlateWorldMat = Plate->GetWorldMat();
			D3DXMatrixInverse(&InvPlateWorldMat, 0, &PlateWorldMat);
			D3DXMatrixLookAtLH(&MapViewMat, &D3DXVECTOR3(0, 500, 0), &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(0,0,1) );
			MapViewMat = InvPlateWorldMat * MapViewMat;
			
			SetVertexShader(LightMapVS);
			SetPixelShader(LightMapPS);
			
			Shader->SetViewMatrix(&MapViewMat);
			Shader->SetProjMatrix(&MapProjMat);
	
			// set spot light
			Light->GetD3DLight(&SpotLight);
			Shader->SetVSSingleSpotLight(&SpotLight);
			
			D3DXMatrixInverse(&InvMapViewMat, 0, &MapViewMat);
			ShadowMat = InvMapViewMat * LightViewMat * LightProjMat * ShadowMap->GetCorrectMat(ShadowMap->GetDynMapSize());
			Shader->SetVSMatrix(ShadowMatHandle, &ShadowMat);

			// set texture
			D3DDevice->SetTexture(3, ProjectedTex);

			// FIRST PASS - render z-depth map
			D3DDevice->BeginScene();
				// Render plate
				Plate->RenderLightMap();
			D3DDevice->EndScene();

			D3DDevice->SetTexture(3, 0);

			SAFE_RELEASE(MapSurface);

			LightMapNum = ShadowMap->AddLightMap(LightMap);

			//hr = D3DXSaveTextureToFile("LightMap.bmp", D3DXIFF_BMP, LightMap, 0); //- DO NOT DELETE - warning for Tempsoft
		}
		
		// add light map to terrain plate
		Plate->SetLightMap(ShadowMap->GetLightMap(LightMapNum) );
		
	}
	
	// Set render target back
	D3DDevice->SetRenderTarget(0, StandardRenderTarget);
	D3DDevice->SetViewport(&StandardViewport);

	// Release temporary projected shadow texture
	SAFE_RELEASE(ProjectedTex);
	
	return ERRNOERROR;

};

 
//////////////////////////////////////////////////////////////////////////////////////////////
//
// Initializes shaders
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::InitShaders()
{
	char CharShaderName[255];
	CString ShaderName;
	UINT i;
	HRESULT hr;


	Shader = new CShaders(D3DDevice);
	if (!Shader)
		ERRORMSG(ERROUTOFMEMORY, "CGraphic::InitShaders()", "Unable to allocate place for CShaders class.");

	// Init CShaders class
	hr = Shader->Init();
	if (hr) ERRORMSG(hr, "CGraphic::InitShaders()", "Unable to init CShaders class.");


	resManNS::ShaderLoadParams ShaderParams; // Shader loading params	


	/////////////////////////////////////////////////////////////
	// VERTEX SHADER LOADING

#ifdef SHADER_DEBUG
	ShaderParams.Flags = D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION;
	ShaderParams.ShaderVersion = "vs_2_sw";
#else
	ShaderParams.Flags = 0;
	ShaderParams.ShaderVersion = "vs_2_0";
#endif
	
	ShaderParams.MainName = "TerrainVS";
	TerrainVS = LoadShader(RES_VertexShader, "Shaders.sh$TerrainVS", ShaderParams);
	if (FAILED(TerrainVS) )
		ERRORMSG(TerrainVS, "CGraphic::InitShaders()", "Unable to load terrain vertex shader");

	ShaderParams.MainName = "TerrainSpotLightVS";
	TerrainSpotLightVS = LoadShader(RES_VertexShader, "Shaders.sh$TerrainSpotLightVS", ShaderParams);
	if (FAILED(TerrainSpotLightVS) )
		ERRORMSG(TerrainSpotLightVS, "CGraphic::InitShaders()", "Unable to load terrain spotlight vertex shader");

	ShaderParams.MainName = "TerrainWithTSMVS";
	TerrainWithTSMVS = LoadShader(RES_VertexShader, "Shaders.sh$TerrainWithTSMVS", ShaderParams);
	if (FAILED(TerrainWithTSMVS) )
		ERRORMSG(TerrainWithTSMVS, "CGraphic::InitShaders()", "Unable to load terrain with TSM vertex shader");

	ShaderParams.MainName = "TerrainSpotLightWithSMVS";
	TerrainSpotLightWithSMVS = LoadShader(RES_VertexShader, "Shaders.sh$TerrainSpotLightWithSMVS", ShaderParams);
	if (FAILED(TerrainSpotLightWithSMVS) )
		ERRORMSG(TerrainSpotLightWithSMVS, "CGraphic::InitShaders()", "Unable to load terrain spot light with SM vertex shader");

	ShaderParams.MainName = "ObjectVS";
	ObjectVS = LoadShader(RES_VertexShader, "Shaders.sh$ObjectVS", ShaderParams);
	if (FAILED(ObjectVS) )
		ERRORMSG(ObjectVS, "CGraphic::InitShaders()", "Unable to load object vertex shader");

	ShaderParams.MainName = "ObjectSpecularVS";
	ObjectSpecularVS = LoadShader(RES_VertexShader, "Shaders.sh$ObjectSpecularVS", ShaderParams);
	if (FAILED(ObjectSpecularVS) )
		ERRORMSG(ObjectSpecularVS, "CGraphic::InitShaders()", "Unable to load object vertex shader");

	ShaderParams.MainName = "ObjectWithTSMVS";
	ObjectWithTSMVS = LoadShader(RES_VertexShader, "Shaders.sh$ObjectWithTSMVS", ShaderParams);
	if (FAILED(ObjectWithTSMVS) )
		ERRORMSG(ObjectWithTSMVS, "CGraphic::InitShaders()", "Unable to load object with TSM vertex shader");

	ShaderParams.MainName = "ObjectWithTSMSpecularVS";
	ObjectWithTSMSpecularVS = LoadShader(RES_VertexShader, "Shaders.sh$ObjectWithTSMSpecularVS", ShaderParams);
	if (FAILED(ObjectWithTSMSpecularVS) )
		ERRORMSG(ObjectWithTSMSpecularVS, "CGraphic::InitShaders()", "Unable to load object with TSM vertex shader");

	ShaderParams.MainName = "ObjectSpotLightVS";
	ObjectSpotLightVS = LoadShader(RES_VertexShader, "Shaders.sh$ObjectSpotLightVS", ShaderParams);
	if (FAILED(ObjectSpotLightVS) )
		ERRORMSG(ObjectSpotLightVS, "CGraphic::InitShaders()", "Unable to load object spot lighting vertex shader");

	ShaderParams.MainName = "ObjectSpotLightWithSMVS";
	ObjectSpotLightWithSMVS = LoadShader(RES_VertexShader, "Shaders.sh$ObjectSpotLightWithSMVS", ShaderParams);
	if (FAILED(ObjectSpotLightWithSMVS) )
		ERRORMSG(ObjectSpotLightWithSMVS, "CGraphic::InitShaders()", "Unable to load object spot light with SM vertex shader");

	ShaderParams.MainName = "DepthVS";
	DepthVS = LoadShader(RES_VertexShader, "Shaders.sh$DepthVS", ShaderParams);

	if (FAILED(DepthVS) )
		ERRORMSG(DepthVS, "CGraphic::InitShaders()", "Error loading depth vexter shader.");
	
	ShaderParams.MainName = "TerrainSpotLightMapVS";
	LightMapVS = LoadShader(RES_VertexShader, "Shaders.sh$TerrainSpotLightMapVS", ShaderParams);
	if (FAILED( LightMapVS) )
		ERRORMSG(LightMapVS, "CGraphic::InitShaders()", "Error loading terrain spotlight light map vexter shader.");

	ShaderParams.MainName = "TransformVS";
	TransformVS = LoadShader(RES_VertexShader, "Shaders.sh$TransformVS", ShaderParams);
	if (FAILED( TransformVS) )
		ERRORMSG(TransformVS, "CGraphic::InitShader()", "Error loading transform vertex shader.");

	ShaderParams.MainName = "TransformTexVS";
	TransformTexVS = LoadShader(RES_VertexShader, "Shaders.sh$TransformTexVS", ShaderParams);
	if (FAILED( TransformTexVS) )
		ERRORMSG(TransformTexVS, "CGraphic::InitShader()", "Error loading transform-texture vertex shader.");

	ShaderParams.MainName = "TSMDepthVS";
	TSMDepthVS = LoadShader(RES_VertexShader, "Shaders.sh$TSMDepthVS", ShaderParams);
	if (FAILED( TSMDepthVS) )
		ERRORMSG(TSMDepthVS, "CGraphic::InitShader()", "Error loading TSM depth vertex shader.");

	ShaderParams.MainName = "TerrainNoDirLightVS";
	TerrainNoDirLightVS = LoadShader(RES_VertexShader, "Shaders.sh$TerrainNoDirLightVS", ShaderParams);
	if (FAILED( TerrainNoDirLightVS) )
		ERRORMSG(TerrainNoDirLightVS, "CGraphic::InitShader()", "Error loading vertex shader for (not enlightened) terrain.");

	ShaderParams.MainName = "ObjectNoDirLightVS";
	ObjectNoDirLightVS = LoadShader(RES_VertexShader, "Shaders.sh$ObjectNoDirLightVS", ShaderParams);
	if (FAILED( ObjectNoDirLightVS) )
		ERRORMSG(ObjectNoDirLightVS, "CGraphic::InitShader()", "Error loading vertex shader for (not enlightened) objects.");

	ShaderParams.MainName = "TerrainLOD0VS";
	TerrainLOD0VS = LoadShader(RES_VertexShader, "Shaders.sh$TerrainLOD0VS", ShaderParams);
	if (FAILED( TerrainLOD0VS) )
		ERRORMSG(TerrainLOD0VS, "CGraphic::InitShader()", "Error loading terrain LOD0 vertex shader.");

	ShaderParams.MainName = "GrassVS";
	GrassVS = LoadShader(RES_VertexShader, "Shaders.sh$GrassVS", ShaderParams);
	if (FAILED( GrassVS) )
		ERRORMSG(GrassVS, "CGraphic::InitShader()", "Error loading grass vertex shader.");

	ShaderParams.MainName = "BillboardVS";
	BillboardVS = LoadShader(RES_VertexShader, "Shaders.sh$BillboardVS", ShaderParams);
	if (FAILED( BillboardVS) )
		ERRORMSG(BillboardVS, "CGraphic::InitShader()", "Error loading vertex shader for transparent objects.");


	// NIGHT VERTEX SHADERS
	ShaderParams.MainName = "TerrainNightVS";
	TerrainNightVS = LoadShader(RES_VertexShader, "Shaders.sh$TerrainNightVS", ShaderParams);
	if (FAILED( TerrainNightVS) )
		ERRORMSG(TerrainNightVS, "CGraphic::InitShader()", "Error loading vertex shader for terrain night mode.");

	ShaderParams.MainName = "ObjectNightVS";
	ObjectNightVS = LoadShader(RES_VertexShader, "Shaders.sh$ObjectNightVS", ShaderParams);
	if (FAILED( ObjectNightVS) )
		ERRORMSG(ObjectNightVS, "CGraphic::InitShader()", "Error loading vertex shader for object night mode.");

	ShaderParams.MainName = "ObjectNoFogVS";
	ObjectNoFogVS = LoadShader(RES_VertexShader, "Shaders.sh$ObjectNoFogVS", ShaderParams);
	if (FAILED(ObjectNoFogVS) )
		ERRORMSG(ObjectNoFogVS, "CGraphic::InitShaders()", "Unable to load object vertex shader");

	ShaderParams.MainName = "TerrainNightLOD0VS";
	TerrainNightLOD0VS = LoadShader(RES_VertexShader, "Shaders.sh$TerrainNightLOD0VS", ShaderParams);
	if (FAILED( TerrainNightLOD0VS) )
		ERRORMSG(TerrainNightLOD0VS, "CGraphic::InitShader()", "Error loading vertex shader for terrain LOD0 in night mode.");

	ShaderParams.MainName = "TerrainLOD0DLVS";
	 TerrainLOD0DLVS = LoadShader(RES_VertexShader, "Shaders.sh$TerrainLOD0DLVS", ShaderParams);
	if (FAILED( TerrainLOD0DLVS) )
		ERRORMSG(TerrainLOD0DLVS, "CGraphic::InitShader()", "Error loading vertex shader for terrain LOD0 directional light computation.");

	ShaderParams.MainName = "NormalMapStage1VS";
	NormalMapStage1VS = LoadShader(RES_VertexShader, "Shaders.sh$NormalMapStage1VS", ShaderParams);
	if (FAILED( NormalMapStage1VS) )
		ERRORMSG(NormalMapStage1VS, "CGraphic::InitShader()", "Error loading vertex shader for terrain normal map stage 1.");

	ShaderParams.MainName = "CheckpointVS";
	CheckpointVS = LoadShader(RES_VertexShader, "Shaders.sh$CheckpointVS", ShaderParams);
	if (FAILED( CheckpointVS) )
		ERRORMSG(CheckpointVS, "CGraphic::InitShader()", "Error loading vertex shader for checkpoint rendering.");

	
	// night shadow mapping vertex shaders for 1 - 4 shadow maps
	for (i = 0; i < 4; i++)
	{
		sprintf(CharShaderName, "TerrainNightSM%iVS", i + 1);
		ShaderName = CString(CharShaderName);
		ShaderParams.MainName = ShaderName;
		TerrainNightSMVS[i] = LoadShader(RES_VertexShader, CString("Shaders.sh$") + ShaderName, ShaderParams);
		if (FAILED( TerrainNightSMVS[i]) )
			ERRORMSG(TerrainNightSMVS[i], "CGraphic::InitShader()", "Error loading vertex shader for terrain shadow mapping in night mode.");

		sprintf(CharShaderName, "ObjectNightSM%iVS", i + 1);
		ShaderName = CString(CharShaderName);
		ShaderParams.MainName = ShaderName;
		ObjectNightSMVS[i] = LoadShader(RES_VertexShader, CString("Shaders.sh$") + ShaderName, ShaderParams);
		if (FAILED( ObjectNightSMVS[i]) )
			ERRORMSG(ObjectNightSMVS[i], "CGraphic::InitShader()", "Error loading vertex shader for object shadow mapping in night mode.");

	}
	
	ShaderParams.MainName = "BillboardNightVS";
	BillboardNightVS = LoadShader(RES_VertexShader, "Shaders.sh$BillboardNightVS", ShaderParams);
	if (FAILED( BillboardNightVS) )
		ERRORMSG(BillboardNightVS, "CGraphic::InitShader()", "Error loading vertex shader for transparent objects.");


	// SKYSYSTEM
	ShaderParams.MainName = "SkySystemVS";
	SkySystemVS = LoadShader(RES_VertexShader, "SkySystem.sh$SkySystemVS", ShaderParams);
	if (FAILED( SkySystemVS) )
		ERRORMSG(SkySystemVS, "CGraphic::InitShader()", "Error loading sky system vertex shader.");

	ShaderParams.MainName = "StarsVS";
	StarsVS = LoadShader(RES_VertexShader, "SkySystem.sh$StarsVS", ShaderParams);
	if (FAILED( StarsVS) )
		ERRORMSG(StarsVS, "CGraphic::InitShader()", "Error loading stars vertex shader.");

	ShaderParams.MainName = "CloudVS";
	CloudVS = LoadShader(RES_VertexShader, "SkySystem.sh$CloudVS", ShaderParams);
	if (FAILED( CloudVS) )
		ERRORMSG(CloudVS, "CGraphic::InitShader()", "Error loading cloud vertex shader.");

	ShaderParams.MainName = "MountainVS";
	MountainVS = LoadShader(RES_VertexShader, "SkySystem.sh$MountainVS", ShaderParams);
	if (FAILED( MountainVS) )
		ERRORMSG(MountainVS, "CGraphic::InitShader()", "Error loading mountain vertex shader.");

	ShaderParams.MainName = "MountainNightVS";
	MountainNightVS = LoadShader(RES_VertexShader, "SkySystem.sh$MountainNightVS", ShaderParams);
	if (FAILED( MountainNightVS) )
		ERRORMSG(MountainNightVS, "CGraphic::InitShader()", "Error loading mountain vertex shader.");




	/////////////////////////////////////////////////////////////
	// PIXEL SHADER LOADING

#ifdef SHADER_DEBUG
	ShaderParams.Flags = D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION;
	ShaderParams.ShaderVersion = "ps_2_sw";
#else
	ShaderParams.Flags = 0;
	ShaderParams.ShaderVersion = "ps_2_0";
#endif


	ShaderParams.MainName = "TerrainPS";
	TerrainPS = LoadShader(RES_PixelShader, "Shaders.sh$TerrainPS", ShaderParams);
	if (FAILED(TerrainPS) )
		ERRORMSG(TerrainPS, "CGraphic::InitShaders()", "Unable to load terrain pixel shader");

	ShaderParams.MainName = "TerrainWithSMPS";
	TerrainWithSMPS = LoadShader(RES_PixelShader, "Shaders.sh$TerrainWithSMPS", ShaderParams);
	if (FAILED(TerrainWithSMPS) )
		ERRORMSG(TerrainWithSMPS, "CGraphic::InitShaders()", "Unable to load terrain with shadow mapping pixel shader");

	ShaderParams.MainName = "TerrainWithHWSMPS";
	TerrainWithHWSMPS = LoadShader(RES_PixelShader, "Shaders.sh$TerrainWithHWSMPS", ShaderParams);
	if (FAILED(TerrainWithHWSMPS) )
		ERRORMSG(TerrainWithHWSMPS, "CGraphic::InitShaders()", "Unable to load terrain with HW shadow mapping pixel shader");

	ShaderParams.MainName = "TerrainWithTSMPS";
	TerrainWithTSMPS = LoadShader(RES_PixelShader, "Shaders.sh$TerrainWithTSMPS", ShaderParams);
	if (FAILED(TerrainWithTSMPS) )
		ERRORMSG(TerrainWithTSMPS, "CGraphic::InitShaders()", "Unable to load terrain with TSM pixel shader");

	ShaderParams.MainName = "TerrainWithHWTSMPS";
	TerrainWithHWTSMPS = LoadShader(RES_PixelShader, "Shaders.sh$TerrainWithHWTSMPS", ShaderParams);
	if (FAILED(TerrainWithHWTSMPS) )
		ERRORMSG(TerrainWithHWTSMPS, "CGraphic::InitShaders()", "Unable to load terrain with HW TSM pixel shader");

	ShaderParams.MainName = "TerrainWithTSMLinearPCFPS";
	TerrainWithTSMLinearPCFPS = LoadShader(RES_PixelShader, "Shaders.sh$TerrainWithTSMLinearPCFPS", ShaderParams);
	if (FAILED(TerrainWithTSMLinearPCFPS) )
		ERRORMSG(TerrainWithTSMLinearPCFPS, "CGraphic::InitShaders()", "Unable to load terrain with TSM and linear PCF pixel shader");


	ShaderParams.MainName = "ObjectPS";
	ObjectPS = LoadShader(RES_PixelShader, "Shaders.sh$ObjectPS", ShaderParams);
	if (FAILED(ObjectPS) )
		ERRORMSG(ObjectPS, "CGraphic::InitShaders()", "Unable to load object pixel shader");

	ShaderParams.MainName = "ObjectSpecularPS";
	ObjectSpecularPS = LoadShader(RES_PixelShader, "Shaders.sh$ObjectSpecularPS", ShaderParams);
	if (FAILED(ObjectSpecularPS) )
		ERRORMSG(ObjectSpecularPS, "CGraphic::InitShaders()", "Unable to load object pixel shader");

	ShaderParams.MainName = "ObjectNightPS";
	ObjectNightPS = LoadShader(RES_PixelShader, "Shaders.sh$ObjectNightPS", ShaderParams);
	if (FAILED(ObjectNightPS) )
		ERRORMSG(ObjectNightPS, "CGraphic::InitShaders()", "Unable to load night object pixel shader");

	ShaderParams.MainName = "ObjectWithSMPS";
	ObjectWithSMPS = LoadShader(RES_PixelShader, "Shaders.sh$ObjectWithSMPS", ShaderParams);
	if (FAILED(ObjectWithSMPS) )
		ERRORMSG(ObjectWithSMPS, "CGraphic::InitShaders()", "Unable to load object with shadow mapping pixel shader");

	ShaderParams.MainName = "ObjectWithHWSMPS";
	ObjectWithHWSMPS = LoadShader(RES_PixelShader, "Shaders.sh$ObjectWithHWSMPS", ShaderParams);
	if (FAILED(ObjectWithHWSMPS) )
		ERRORMSG(ObjectWithHWSMPS, "CGraphic::InitShaders()", "Unable to load object with HW shadow mapping pixel shader");
	
	ShaderParams.MainName = "ObjectWithTSMPS";
	ObjectWithTSMPS = LoadShader(RES_PixelShader, "Shaders.sh$ObjectWithTSMPS", ShaderParams);
	if (FAILED(ObjectWithTSMPS) )
		ERRORMSG(ObjectWithTSMPS, "CGraphic::InitShaders()", "Unable to load object with TSM pixel shader");

	ShaderParams.MainName = "ObjectWithHWTSMPS";
	ObjectWithHWTSMPS = LoadShader(RES_PixelShader, "Shaders.sh$ObjectWithHWTSMPS", ShaderParams);
	if (FAILED(ObjectWithHWTSMPS) )
		ERRORMSG(ObjectWithHWTSMPS, "CGraphic::InitShaders()", "Unable to load object with HW TSM pixel shader");

	ShaderParams.MainName = "ObjectWithHWTSMSpecularPS";
	ObjectWithHWTSMSpecularPS = LoadShader(RES_PixelShader, "Shaders.sh$ObjectWithHWTSMSpecularPS", ShaderParams);
	if (FAILED(ObjectWithHWTSMSpecularPS) )
		ERRORMSG(ObjectWithHWTSMSpecularPS, "CGraphic::InitShaders()", "Unable to load object with TSM pixel shader");

	ShaderParams.MainName = "ObjectWithTSMLinearPCFPS";
	ObjectWithTSMLinearPCFPS = LoadShader(RES_PixelShader, "Shaders.sh$ObjectWithTSMLinearPCFPS", ShaderParams);
	if (FAILED(ObjectWithTSMLinearPCFPS) )
		ERRORMSG(ObjectWithTSMLinearPCFPS, "CGraphic::InitShaders()", "Unable to load object with TSM and linear PCF pixel shader");
	
	ShaderParams.MainName = "ObjectWithTSMLinearPCFSpecularPS";
	ObjectWithTSMLinearPCFSpecularPS = LoadShader(RES_PixelShader, "Shaders.sh$ObjectWithTSMLinearPCFSpecularPS", ShaderParams);
	if (FAILED(ObjectWithTSMLinearPCFSpecularPS) )
		ERRORMSG(ObjectWithTSMLinearPCFSpecularPS, "CGraphic::InitShaders()", "Unable to load object with TSM and linear PCF pixel shader");
	
	ShaderParams.MainName = "DepthPS";
	DepthPS = LoadShader(RES_PixelShader, "Shaders.sh$DepthPS", ShaderParams);
	if (FAILED(DepthPS) )
		ERRORMSG(DepthPS, "CGraphic::InitShaders()", "Error loading depth pixel shader.");
	
	ShaderParams.MainName = "LightMapPS";
	LightMapPS = LoadShader(RES_PixelShader, "Shaders.sh$LightMapPS", ShaderParams);
	if (FAILED(LightMapPS) )
		ERRORMSG(LightMapPS, "CGraphic::InitShaders()", "Error loading light map pixel shader.");

	ShaderParams.MainName = "ZeroPS";
	ZeroPS = LoadShader(RES_PixelShader, "Shaders.sh$ZeroPS", ShaderParams);
	if (FAILED(ZeroPS) )
		ERRORMSG(ZeroPS, "CGraphic::InitShaders()", "Error loading zero pixel shader.");

	ShaderParams.MainName = "TexPS";
	TexPS = LoadShader(RES_PixelShader, "Shaders.sh$TexPS", ShaderParams);
	if (FAILED(TexPS) )
		ERRORMSG(TexPS, "CGraphic::InitShaders()", "Error loading tex pixel shader.");
		
	ShaderParams.MainName = "TSMDepthPS";
	TSMDepthPS = LoadShader(RES_PixelShader, "Shaders.sh$TSMDepthPS", ShaderParams);
	if (FAILED(TSMDepthPS) )
		ERRORMSG(TSMDepthPS, "CGraphics::InitShaders()", "Error loading TSM depth pixel shader.");

	ShaderParams.MainName = "SVIncreaseCounterPS";
	ShadowVolumeIncPS = LoadShader(RES_PixelShader, "Shaders.sh$SVIncreaseCounterPS", ShaderParams);
	if (FAILED(ShadowVolumeIncPS) )
		ERRORMSG(ShadowVolumeIncPS, "CGraphics::InitShaders()", "An error occurs while loading shadow volume pixel shader.");

	ShaderParams.MainName = "SVCoveringPolygonPS";
	SVCoveringPolygonPS = LoadShader(RES_PixelShader, "Shaders.sh$SVCoveringPolygonPS", ShaderParams);
	if (FAILED(SVCoveringPolygonPS) )
		ERRORMSG(SVCoveringPolygonPS, "CGraphics::InitShaders()", "An error occurs while loading shadow volume pixel shader.");

 	ShaderParams.MainName = "SingleColorPS";
	SingleColorPS = LoadShader( RES_PixelShader, "Shaders.sh$SingleColorPS", ShaderParams );
	if ( FAILED(SingleColorPS) )
		ERRORMSG( SingleColorPS, "CGraphics::InitShaders()", "An error occurs while loading bounding envelopes pixel shader.");

 	ShaderParams.MainName = "TerrainLOD0PS";
	TerrainLOD0PS = LoadShader( RES_PixelShader, "Shaders.sh$TerrainLOD0PS", ShaderParams );
	if ( FAILED(TerrainLOD0PS) )
		ERRORMSG( TerrainLOD0PS, "CGraphics::InitShaders()", "An error occurs while loading terrain LOD0 pixel shader.");

 	ShaderParams.MainName = "GrassPS";
	GrassPS = LoadShader( RES_PixelShader, "Shaders.sh$GrassPS", ShaderParams );
	if ( FAILED(GrassPS) )
		ERRORMSG( GrassPS, "CGraphics::InitShaders()", "An error occurs while loading grass pixel shader.");

 	ShaderParams.MainName = "GrassNightPS";
	GrassNightPS = LoadShader( RES_PixelShader, "Shaders.sh$GrassNightPS", ShaderParams );
	if ( FAILED(GrassNightPS) )
		ERRORMSG( GrassNightPS, "CGraphics::InitShaders()", "An error occurs while loading night grass pixel shader.");

 	ShaderParams.MainName = "BillboardPS";
	BillboardPS = LoadShader( RES_PixelShader, "Shaders.sh$BillboardPS", ShaderParams );
	if ( FAILED(BillboardPS) )
		ERRORMSG( BillboardPS, "CGraphics::InitShaders()", "An error occurs while loading transparent pixel shader.");

 	ShaderParams.MainName = "BillboardNightPS";
	BillboardNightPS = LoadShader( RES_PixelShader, "Shaders.sh$BillboardNightPS", ShaderParams );
	if ( FAILED(BillboardNightPS) )
		ERRORMSG( BillboardNightPS, "CGraphics::InitShaders()", "An error occurs while loading transparent pixel shader.");

	ShaderParams.MainName = "TerrainNightPS";
	TerrainNightPS = LoadShader( RES_PixelShader, "Shaders.sh$TerrainNightPS", ShaderParams );
	if ( FAILED(TerrainNightPS) )
		ERRORMSG( TerrainNightPS, "CGraphics::InitShaders()", "An error occurs while loading terrain night shader.");

	ShaderParams.MainName = "TerrainNightLOD0PS";
	TerrainNightLOD0PS = LoadShader( RES_PixelShader, "Shaders.sh$TerrainNightLOD0PS", ShaderParams );
	if ( FAILED(TerrainNightLOD0PS) )
		ERRORMSG( TerrainNightLOD0PS, "CGraphics::InitShaders()", "An error occurs while loading terrain night LOD0 shader.");

	ShaderParams.MainName = "TerrainLOD0OptimalizationPS";
	TerrainLOD0OptimalizationPS = LoadShader( RES_PixelShader, "Shaders.sh$TerrainLOD0OptimalizationPS", ShaderParams );
	if ( FAILED(TerrainLOD0OptimalizationPS) )
		ERRORMSG( TerrainLOD0OptimalizationPS, "CGraphics::InitShaders()", "An error occurs while loading terrain LOD0 optimalization pixel shader.");
	
	ShaderParams.MainName = "TerrainLOD0DLPS";
	TerrainLOD0DLPS = LoadShader( RES_PixelShader, "Shaders.sh$TerrainLOD0DLPS", ShaderParams );
	if ( FAILED(TerrainLOD0DLPS) )
		ERRORMSG( TerrainLOD0DLPS, "CGraphics::InitShaders()", "An error occurs while loading terrain LOD0 directional light pixel shader.");

	ShaderParams.MainName = "TerrainLOD0TexturePS";
	TerrainLOD0TexturePS = LoadShader( RES_PixelShader, "Shaders.sh$TerrainLOD0TexturePS", ShaderParams );
	if ( FAILED(TerrainLOD0TexturePS) )
		ERRORMSG( TerrainLOD0TexturePS, "CGraphics::InitShaders()", "An error occurs while loading terrain LOD0 texture pixel shader.");

	ShaderParams.MainName = "TerrainLOD0TextureNightPS";
	TerrainLOD0TextureNightPS= LoadShader( RES_PixelShader, "Shaders.sh$TerrainLOD0TextureNightPS", ShaderParams );
	if ( FAILED(TerrainLOD0TextureNightPS) )
		ERRORMSG( TerrainLOD0TextureNightPS, "CGraphics::InitShaders()", "An error occurs while loading terrain LOD0 texture night pixel shader.");

	ShaderParams.MainName = "TerrainLightMapPS";
	TerrainLightMapPS= LoadShader( RES_PixelShader, "Shaders.sh$TerrainLightMapPS", ShaderParams );
	if ( FAILED(TerrainLightMapPS) )
		ERRORMSG( TerrainLightMapPS, "CGraphics::InitShaders()", "An error occurs while loading terrain light map pixel shader.");

	ShaderParams.MainName = "NormalMapStage1PS";
	NormalMapStage1PS = LoadShader( RES_PixelShader, "Shaders.sh$NormalMapStage1PS", ShaderParams );
	if ( FAILED(NormalMapStage1PS) )
		ERRORMSG( NormalMapStage1PS, "CGraphics::InitShaders()", "An error occurs while loading terrain normal map stage 1 pixel shader.");

	ShaderParams.MainName = "NormalMapStage2PS";
	NormalMapStage2PS = LoadShader( RES_PixelShader, "Shaders.sh$NormalMapStage2PS", ShaderParams );
	if ( FAILED(NormalMapStage2PS) )
		ERRORMSG( NormalMapStage2PS, "CGraphics::InitShaders()", "An error occurs while loading terrain normal map stage 2 pixel shader.");

	ShaderParams.MainName = "CheckpointPS";
	CheckpointPS = LoadShader( RES_PixelShader, "Shaders.sh$CheckpointPS", ShaderParams );
	if ( FAILED(CheckpointPS) )
		ERRORMSG( CheckpointPS, "CGraphics::InitShaders()", "An error occurs while loading checkpoint pixel shader.");



	// Pixel shaders for night shadow mapping
	for (i = 0; i < 4; i++)
	{
		sprintf(CharShaderName, "TerrainNightSM%iPS", i + 1);
		ShaderName = CString(CharShaderName);
		ShaderParams.MainName = ShaderName;
		TerrainNightSMPS[i] = LoadShader( RES_PixelShader, CString("Shaders.sh$") + ShaderName, ShaderParams );
		if ( FAILED(TerrainNightSMPS[i]) )
			ERRORMSG( TerrainNightSMPS[i], "CGraphics::InitShaders()", "An error occurs while loading terrain night shadow mapping shader.");

		sprintf(CharShaderName, "TerrainNightSM%iHWPS", i + 1);
		ShaderName = CString(CharShaderName);
		ShaderParams.MainName = ShaderName;
		TerrainNightHWSMPS[i] = LoadShader( RES_PixelShader, CString("Shaders.sh$") + ShaderName, ShaderParams );
		if ( FAILED(TerrainNightHWSMPS[i]) )
			ERRORMSG( TerrainNightHWSMPS[i], "CGraphics::InitShaders()", "An error occurs while loading terrain night hardware shadow mapping shader.");

		sprintf(CharShaderName, "ObjectNightSM%iPS", i + 1);
		ShaderName = CString(CharShaderName);
		ShaderParams.MainName = ShaderName;
		ObjectNightSMPS[i] = LoadShader( RES_PixelShader, CString("Shaders.sh$") + ShaderName, ShaderParams );
		if ( FAILED(ObjectNightSMPS[i]) )
			ERRORMSG( ObjectNightSMPS[i], "CGraphics::InitShaders()", "An error occurs while loading object night shadow mapping shader.");

		sprintf(CharShaderName, "ObjectNightHWSM%iPS", i + 1);
		ShaderName = CString(CharShaderName);
		ShaderParams.MainName = ShaderName;
		ObjectNightHWSMPS[i] = LoadShader( RES_PixelShader, CString("Shaders.sh$") + ShaderName, ShaderParams );
		if ( FAILED(ObjectNightHWSMPS[i]) )
			ERRORMSG( ObjectNightHWSMPS[i], "CGraphics::InitShaders()", "An error occurs while loading object night hardware shadow mapping shader.");


	}

	// SKYSYSTEM
	ShaderParams.MainName = "SkySystemPS";
	SkySystemPS = LoadShader(RES_PixelShader, "SkySystem.sh$SkySystemPS", ShaderParams);

	if (FAILED(SkySystemPS) )
		ERRORMSG(SkySystemPS, "CGraphics::InitShaders()", "Error loading sky system pixel shader.");

	ShaderParams.MainName = "StarsPS";
	StarsPS = LoadShader(RES_PixelShader, "SkySystem.sh$StarsPS", ShaderParams);
	if (FAILED(StarsPS) )
		ERRORMSG(StarsPS, "CGraphics::InitShaders()", "Error loading stars pixel shader.");

	ShaderParams.MainName = "CloudPS";
	CloudPS = LoadShader(RES_PixelShader, "SkySystem.sh$CloudPS", ShaderParams);
	if (FAILED(CloudPS) )
		ERRORMSG(CloudPS, "CGraphics::InitShaders()", "Error loading cloud pixel shader.");

	ShaderParams.MainName = "MountainPS";
	MountainPS = LoadShader(RES_PixelShader, "SkySystem.sh$MountainPS", ShaderParams);
	if (FAILED(MountainPS) )
		ERRORMSG(MountainPS, "CGraphics::InitShaders()", "Error loading mountain pixel shader.");

	ShaderParams.MainName = "MountainNightPS";
	MountainNightPS = LoadShader(RES_PixelShader, "SkySystem.sh$MountainNightPS", ShaderParams);
	if (FAILED(MountainNightPS) )
		ERRORMSG(MountainNightPS, "CGraphics::InitShaders()", "Error loading mountain pixel shader.");




	/////////////////////////////////////////////////////////////
	// SHADER CONSTANT HANDLE FETCH
	Shader->BeginShaderConstantFetch();

		if (FAILED( Shader->GetHandleByName("ShadowMat", &ShadowMatHandle) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get shadow matrix handle.");
		if (FAILED( Shader->GetHandleByName("PostProjMat", &PostProjMatHandle) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get post projection matrix handle.");
		if (FAILED( Shader->GetHandleByName("SpotLights", &SpotLightsHandle) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get spot lights handle.");
		if (FAILED( Shader->GetHandleByName("GrassMaxDepth", &GrassMaxDepthHandle) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get grass max depth handle.");
		if (FAILED( Shader->GetHandleByName("GrassTime", &GrassTimeHandle) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get grass time handle.");
		if (FAILED( Shader->GetHandleByName("GrassViewerPos", &GrassViewerPosHandle) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get grass viewer position handle.");
		if (FAILED( Shader->GetHandleByName("MapWidth", &MapWidthHandle) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get map width shader constant handle.");
		if (FAILED( Shader->GetHandleByName("MapDepth", &MapDepthHandle) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get map depth shader constant handle.");
		if (FAILED( Shader->GetHandleByName("ColorModifier", &ColorModifierHandle) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get color modifier shader constant handle.");
		if (FAILED( Shader->GetHandleByName("FogColor", &FogColorHandle) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get fog color shader constant handle.");

		// shadow mats
		if (FAILED( Shader->GetHandleByName("ShadowMat", &ShadowMatsHandles[0] ) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get shadow matrix handle.");
		if (FAILED( Shader->GetHandleByName("ShadowMat2", &ShadowMatsHandles[1] ) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get shadow matrix 2 handle.");
		if (FAILED( Shader->GetHandleByName("ShadowMat3", &ShadowMatsHandles[2] ) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get shadow matrix 3 handle.");
		if (FAILED( Shader->GetHandleByName("ShadowMat4", &ShadowMatsHandles[3] ) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get shadow matrix 4 handle.");
		if (FAILED( Shader->GetHandleByName("ClampVector1", &ClampVectorsHandles[0] ) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get clamp vector 1 handle.");
		if (FAILED( Shader->GetHandleByName("ClampVector2", &ClampVectorsHandles[1] ) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get clamp vector 2 handle.");
		if (FAILED( Shader->GetHandleByName("ClampVector3", &ClampVectorsHandles[2] ) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get clamp vector 3 handle.");
		if (FAILED( Shader->GetHandleByName("ClampVector4", &ClampVectorsHandles[3] ) ) )
			ERRORMSG(-1, "CGraphic::InitShaders()", "Unable to get clamp vector 4 handle.");

	Shader->EndShaderConstantFetch();


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// releases all shaders
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::ReleaseAllShaders()
{
	vector<resManNS::RESOURCEID>::iterator	iter;

	SAFE_DELETE( Shader );

	for ( iter = ShaderRIDs.begin(); iter != ShaderRIDs.end(); iter++ )
		ResourceManager->ReleaseResource( *iter );
	
};


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Load vertex or pixel shader from resource manager to shader class
// ShaderType is RES_VERTEXSHADER or RES_PIXELSHADER
//
///////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::LoadShader(const UINT ShaderType, const CString FileName, resManNS::ShaderLoadParams ShaderParams)
{
	UINT		ShaderHandle;
	resManNS::RESOURCEID	ShaderID;

	ShaderID = ResourceManager->LoadResource(FileName,ShaderType, &ShaderParams);
	if (ShaderID < 0)
			ERRORMSG(ShaderID, "CGraphic::LoadShader()", 
			CString("Unable to load shader in file: ") + FileName + CString(" entry point name: ") +
			 CString(ShaderParams.MainName) );
	if (ShaderType == RES_VertexShader)
	{
		ShaderHandle = Shader->LoadVertexShader( * (ResourceManager->GetVertexShader(ShaderID)) );
	}
	else
	{
		ShaderHandle = Shader->LoadPixelShader( * (ResourceManager->GetPixelShader(ShaderID)) );
	}
	if (FAILED(ShaderHandle) )
			ERRORMSG(ShaderHandle, "CGraphic::LoadShader()",
			CString("Unable to load shader in file: ") + FileName + CString(" entry point name: ") +
			 CString(ShaderParams.MainName) );

	ShaderRIDs.push_back( ShaderID );
	
	return ShaderHandle;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Initializes shadow mapping
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::InitShadowMapping()
{
	HRESULT hr;
	ShadowMap = new CShadowMap();
	if (!ShadowMap)
		ERRORMSG(ERROUTOFMEMORY, "CGraphic::InitShadowMapping()", "Unable to allocate place for shadow mapping class.");

	ShadowMapInit Init;

	Init.Device = D3DDevice;
	Init.DynMapSize = SM_MAX;
	Init.DynMap2Size = SM_MAX;
	Init.SunMapSize = SM_MAX;
	Init.Bias = 15.0f;
	Init.LightMapSize = 256;
	Init.MaxLampLOD = 1024;
	Init.MinLampLOD = 256;
	Init.CarLOD = 500;
	
	// Init black light map
	ridBlackTexture = ResourceManager->LoadResource( "Tex.jpg", RES_Texture, 0 );

	if (ridBlackTexture < 0)
		ERRORMSG(ridBlackTexture, "CGraphic::InitShadowMapping()", "Unable to load black light map texture");
	resManNS::__Texture * tex = ResourceManager->GetTexture(ridBlackTexture);
	Init.BlackLightMap = tex->texture;

	// determine dynamic shadow texture format
	if (bShaderSupported)
	{
		Init.Method = SHADOW_MAPPING;
		// Check for appropriate format

#ifndef SHADER_DEBUG
		
		if (AdapterIdentifier.VendorId == NVIDIA_VENDOR_ID)
		//if (!Direct3D->CheckDeviceFormat(uiAdapter, DeviceType, BackBufferFormat, D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_TEXTURE, D3DFMT_D24S8) )
		{
			Init.DynTextureFormat = D3DFMT_D24S8;
		}
		else
		
		
#endif
		if (!Direct3D->CheckDeviceFormat(uiAdapter, DeviceType, BackBufferFormat, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_R32F))
		{
			Init.DynTextureFormat= D3DFMT_R32F;
		}
		else
		{
			Init.DynTextureFormat = BackBufferFormat;
			Init.Method = PROJECTIVE;
		}
	}
	else
	{
		Init.DynTextureFormat = BackBufferFormat;
		Init.Method = PROJECTIVE;
	}

	// FOR TESTING PURPOSES - BEGIN
	//Init.Method = PROJECTIVE;
	//Init.DynTextureFormat = D3DPP.BackBufferFormat;
	//bShaderSupported = false;
	// FOR TESTING PURPOSES - END

	hr = ShadowMap->Init(Init);
	if ( hr) ERRORMSG(hr, "CGraphic.InitShadowMapping()", "Unable to initialize CShadowMap class");

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Registers lights with shadow mapping
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::RegisterLights()
{

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Renders lamp and objects eluminated by it
//
//////////////////////////////////////////////////////////////////////////////////////////////

HRESULT	CGraphic::RenderLamp(const std::vector<CGrObjectBase*> Object, CLight * Light)
{
	D3DLIGHT9 SpotLight;
	int i;

	D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);

	// Get spot light
	Light->GetD3DLight(&SpotLight);

	SetVertexShader(ObjectSpotLightVS);
	SetPixelShader(ObjectPS);

	// set fog color
	Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );

	// Set spot light
	Shader->SetVSSingleSpotLight(&SpotLight);
	
	// render objects
	// Light->pEmitor->Render();	
	for (i = 0; i < (int) Object.size(); i++)
		Object[i]->Render();

	SetVertexShader(TerrainVS);
	SetPixelShader(TerrainLightMapPS);

	CGrObjectTerrainPlate * Plate; // current plate
	
	// Get Plate
	Plate = Scene->Map.GetPlate(Light->MapPosition.PosX, Light->MapPosition.PosZ);

	// render terrain plate
	Plate->SetLightMap(Light->LightMap);
	Plate->Render();
	

	D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders dynamic shadow map for certain MESH object and LAMP LIGHT
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::RenderDynamicShadowMapForLamp(const std::vector<CGrObjectBase*> Object, CLight * Light)
{
	D3DXMATRIX LightViewMat; // Light view matrix
	D3DXMATRIX LightProjMat; // Light projection matrix
	D3DXMATRIX ShadowMat; // Shadow matrix - from view space to texture space on shadow map
	UINT i;
	D3DVIEWPORT9 ShadowMapView;
	CGrObjectTerrainPlate * Plate; // current plate
	IDirect3DSurface9 * DepthStencil; // depth stencil storage
	D3DLIGHT9 D3DLight;
	D3DVIEWPORT9 WholeViewport;
	//D3DXVECTOR4 ClampVector;

	// Get Plate
	Plate = Scene->Map.GetPlate(Light->MapPosition.PosX, Light->MapPosition.PosZ);

	// Compute light view and proj matrix
	ShadowMap->ComputeLightViewProjMat(&LightViewMat, &LightProjMat, Light);
	
	// Set render target
	D3DDevice->SetRenderTarget(0,  ShadowMap->GetDynShadowSurf());
	// Set viewport 
	ShadowMapView = ShadowMap->GetLampViewport(Plate, &Camera);
	D3DDevice->SetViewport(&ShadowMapView);

	// Compute clamp vector
	/*
	ClampVector.x = (float) ShadowMapView.X / (float) ShadowMap->GetDynMapSize();
	ClampVector.y = (float) ShadowMapView.Y / (float) ShadowMap->GetDynMapSize();
	ClampVector.z = (float) (ShadowMapView.X + ShadowMapView.Width) / (float) ShadowMap->GetDynMapSize();
	ClampVector.w = (float) (ShadowMapView.Y + ShadowMapView.Height) / (float) ShadowMap->GetDynMapSize();
	*/

	D3DDevice->GetDepthStencilSurface(&DepthStencil);
	D3DDevice->SetDepthStencilSurface(DepthStencil2);

	WholeViewport.MaxZ = 1;
	WholeViewport.MinZ = 0;
	WholeViewport.X = WholeViewport.Y = 0;
	WholeViewport.Width = WholeViewport.Height = ShadowMap->GetDynMapSize();
	D3DDevice->SetViewport(&WholeViewport);

	if (ShadowMap->GetDynMapFormat() == D3DFMT_D24S8)
	{
		SetVertexShader(TransformVS);
		SetPixelShader(ZeroPS);
		D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO);
		D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);
		D3DDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0);
	
		// Clear depth stencil
		D3DDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(255,255, 255, 255), 1, 0);
	}
	else
	{
		SetVertexShader(DepthVS);
		SetPixelShader(DepthPS);
		D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
		D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO);
		// Clear render target
		D3DDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(255,255, 255, 255), 1, 0);
	}

	// set some border
	ShadowMapView.X = 1;
	ShadowMapView.Y = 1;
	ShadowMapView.Height -= 2;
	ShadowMapView.Width -= 2;
	D3DDevice->SetViewport(&ShadowMapView);
	
	// apply bias to view space 
	Shader->SetViewMatrix(&(LightViewMat * ShadowMap->GetBiasMat()) );
	Shader->SetProjMatrix(&LightProjMat);

	// FIRST PASS - render z-depth map
	// Render plate and light stand and dynamic mesh
	//Plate->SetLOD(CGraphic::uiTerrainLOD);
	//Plate->Render();
	Light->pEmitor->Render();
	for (i = 0; i < Object.size(); i++)
		Object[i]->Render();

	//D3DXSaveTextureToFile("DepthMap.jpg", D3DXIFF_JPG, ShadowMap->GetDynShadowTex(), 0);

	// PASS 3 - render lamp with dynamic shadow from camera view
	D3DDevice->SetRenderTarget(0, StandardRenderTarget);
	D3DDevice->SetViewport(&StandardViewport);

	D3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	D3DDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0x0000000F);
	D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);
	//D3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, false);
	
	D3DDevice->SetDepthStencilSurface(DepthStencil);
	
	// set shaders for rendering correct scene
	SetVertexShader(TerrainSpotLightWithSMVS);
	
	if (ShadowMap->GetDynMapFormat() == D3DFMT_D24S8)
	{
		// use NVidia Hardware shadow mapping
		SetPixelShader(TerrainWithHWSMPS);
		// set depth texture
		D3DDevice->SetTexture(3, DepthTexture2);
	}
	else
	{
		SetPixelShader(TerrainWithSMPS);
		// set depth texture
		D3DDevice->SetTexture(3, ShadowMap->GetDynShadowTex() );

	}

	// Set clamp vector
	//Shader->SetVSValue(ClampVectorsHandles[0], &ClampVector, sizeof(D3DXVECTOR4) );

	// Set matrixes
	Shader->SetViewMatrix(&ViewMatrix);
	Shader->SetProjMatrix(&ProjectionMatrix);
	//ShadowMat = LightViewMat * LightProjMat * ShadowMap->GetCorrectMat(ShadowMapView.Width);
	RECT ShadowViewRect;
	ShadowViewRect.left = ShadowMapView.X;
	ShadowViewRect.right = ShadowMapView.X + ShadowMapView.Width;
	ShadowViewRect.top = ShadowMapView.Y;
	ShadowViewRect.bottom = ShadowMapView.Y + ShadowMapView.Height;
	ShadowMat = LightViewMat * LightProjMat * ShadowMap->GetCorrectMat(ShadowViewRect);
	
	Shader->SetVSMatrix(ShadowMatHandle, &ShadowMat);
	
	D3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );

	// set light
	Light->GetD3DLight(&D3DLight);
	Shader->SetVSSingleSpotLight(&D3DLight);
	
	// Render terrain plate
	Plate->SetLightMap(Light->LightMap);
	Plate->Render();

	// Set shaders for objects
	SetVertexShader(ObjectSpotLightWithSMVS);

	if (ShadowMap->GetDynMapFormat() == D3DFMT_D24S8)
		SetPixelShader(ObjectWithHWSMPS);
	else
		SetPixelShader(ObjectWithSMPS);
	
	// set shader constants
	Shader->SetVSMatrix(ShadowMatHandle, &ShadowMat);
	Shader->SetVSSingleSpotLight(&D3DLight);
	// Set clamp vector
	//Shader->SetVSValue(ClampVectorsHandles[0], &ClampVector, sizeof(D3DXVECTOR4) );

	// render objects
	// Light->pEmitor->Render();	
	for (i = 0; i < Object.size(); i++)
		Object[i]->Render();
		
	// return back standard setting
	D3DDevice->SetTexture(3, 0);

	D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	//D3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, true);

	SAFE_RELEASE(DepthStencil);
	
	
	return ERRNOERROR;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// the call of function with parameter true will determine terrain plates, which will be excluded from
// light map rendering (sets black light map)
// the call of function with parameter false will reinclude such terrain plates back to light map rendering
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::ExcludeFromLightMapRendering(bool ExcludeInclude)
{
	GROBJECTS_ITER			objIter;
	MAPPOSITIONLIST *		MapPosList;
	MAPPOSITIONLIST::iterator MapPosIt;
	CGrObjectTerrainPlate * Plate;
	LightMapExclude Exclude;
	bool LampAdded;


	// exclude
	if (ExcludeInclude)
	{
		// iterate through all dynamic objects and determine, if it is on plate with LAMP
		for (objIter = Scene->DynamicObjects.begin(); objIter != Scene->DynamicObjects.end(); objIter++)
		{
			if ( (*objIter)->GetObjectType() != GROBJECT_TYPE_MESH && (*objIter)->GetObjectType() != GROBJECT_TYPE_CAR) continue;
			MapPosList = (*objIter)->GetMapPosList();
			for (MapPosIt = MapPosList->begin(); MapPosIt != MapPosList->end(); MapPosIt++)
			{
				int x = MapPosIt->PosX;
				int z = MapPosIt->PosZ;
				Plate = Scene->Map.GetPlate(MapPosIt->PosX, MapPosIt->PosZ);
				if (MapPosIt->PosX < MapPosition.PosX - iMapLODDefSize / 2 || MapPosIt->PosX > MapPosition.PosX + iMapLODDefSize / 2 ||
					MapPosIt->PosZ < MapPosition.PosZ - iMapLODDefSize / 2 || MapPosIt->PosZ > MapPosition.PosZ + iMapLODDefSize / 2)
					continue;
				Exclude.Light = Plate->GetLamp();
				// if true, generate dynamic shadow map for terrain plate with light and dynamic objects
				if (Exclude.Light)
				{
					// Check, if the lamp is not already added
					LampAdded = false;
					for (UINT i = 0; i < MapExclude.size(); i++)
						if (Exclude.Light == MapExclude[i].Light)
						{
							MapExclude[i].Object.push_back( *objIter ); // just push object to render with that lamp
							LampAdded = true;
							break;
						}
					
					if (!LampAdded)
					{
						Exclude.Map = Plate->GetLightMap();
						Plate->SetLightMap(ShadowMap->GetLightMap(0) );
						MapExclude.push_back(Exclude);
						MapExclude[MapExclude.size() - 1].Object.push_back( *objIter);
					}
				}
			}
		}
	}
	// include
	else
	{
		for (UINT i = 0; i < MapExclude.size(); i++)
		{
			Plate = Scene->Map.GetPlate(MapExclude[i].Light->MapPosition.PosX, MapExclude[i].Light->MapPosition.PosZ);
			Plate->SetLightMap( MapExclude[i].Map );
		}

		MapExclude.clear();
	}

	return ERRNOERROR;
	
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders dynamic shadow map for certain MESH object and LAMP LIGHT using projected shadows
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::RenderDynamicProjectedMapForLamp(const std::vector<CGrObjectBase*> Object, CLight * Light)
{
	D3DXMATRIX LightViewMat; // Light view matrix
	D3DXMATRIX LightProjMat; // Light projection matrix
	D3DXMATRIX InvViewMat; // Inverse camera view matrix
	D3DXMATRIX ShadowMat; // Shadow matrix - from view space to texture space on shadow map
	UINT i;

	D3DVIEWPORT9 ShadowMapView;
	CGrObjectTerrainPlate * Plate; // current plate
	D3DLIGHT9 D3DLight;

  	// Get Plate
	Plate = Scene->Map.GetPlate(Light->MapPosition.PosX, Light->MapPosition.PosZ);

	// Compute light view and proj matrix
	ShadowMap->ComputeLightViewProjMat(&LightViewMat, &LightProjMat, Light);

	// Set viewport and render target
	ShadowMapView = ShadowMap->GetLampViewport(Plate, &Camera);
	
	D3DDevice->SetRenderTarget(0,  ShadowMap->GetDynShadowSurf());
	D3DDevice->SetViewport(&ShadowMapView);

	// Disable z-buffer rendering
	D3DDevice->SetRenderState( D3DRS_ZENABLE , D3DZB_FALSE);

	D3DDevice->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_RGBA(255,255, 255, 255), 1, 0);

	
	if (bShaderSupported)
	{
		// Set vertex and pixel shader
		SetVertexShader(TransformVS);
		SetPixelShader(ZeroPS);

		Shader->SetViewMatrix(&LightViewMat);
		Shader->SetProjMatrix(&LightProjMat);
		D3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		D3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

	}
	else
	{
		D3DDevice->SetTransform(D3DTS_VIEW, &LightViewMat);
		D3DDevice->SetTransform(D3DTS_PROJECTION, &LightProjMat);
		D3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		D3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	}

	// FIRST PASS - render black map
	// Render light stand and dynamic mesh
		Light->pEmitor->Render();
		for (i = 0; i < Object.size(); i++)
			Object[i]->Render();
	
	D3DDevice->SetRenderTarget(0, StandardRenderTarget);
	D3DDevice->SetViewport(&StandardViewport);

    // enable z-buffer
	D3DDevice->SetRenderState( D3DRS_ZENABLE , D3DZB_TRUE);

	// compute transform matrix to projective texture
	ShadowMat = LightViewMat * LightProjMat * ShadowMap->GetCorrectMat(ShadowMapView.Width );

	// light
	Light->GetD3DLight(&D3DLight);

	if (bShaderSupported)
	{
		SetVertexShader(LampShadowMapVS);
		SetPixelShader(LampProjectiveMapPS);
		// set matrixes
		Shader->SetViewMatrix(&ViewMatrix);
		Shader->SetProjMatrix(&ProjectionMatrix);
		Shader->SetVSMatrix(ShadowMatHandle, &ShadowMat);
		// set light
		Shader->SetVSSingleSpotLight(&D3DLight);
		// other
		D3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
	}
	else
	{
		// Set transform for texture 2
		D3DDevice->SetTransform(D3DTS_VIEW, &ViewMatrix);
		D3DDevice->SetTransform(D3DTS_PROJECTION, &ProjectionMatrix);
		D3DDevice->SetTextureStageState(2,  D3DTSS_TEXCOORDINDEX, 2 | D3DTSS_TCI_CAMERASPACEPOSITION);
		D3DDevice->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT4 | D3DTTFF_PROJECTED);
		D3DXMatrixInverse(&InvViewMat, 0, &ViewMatrix);
		ShadowMat = InvViewMat * ShadowMat;
		D3DDevice->SetTransform(D3DTS_TEXTURE2, &ShadowMat);
		D3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_MODULATE );
		// set light
		D3DDevice->LightEnable(0, true);
		D3DDevice->SetLight(0, &D3DLight);
	}

	// set projective texture
	D3DDevice->SetTexture(2, ShadowMap->GetDynShadowTex() );
	
	D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);
		
	// SECOND PASS - render lamp with dynamic shadow
	Plate->Render();

	D3DDevice->SetTexture(2, 0);
	if (bShaderSupported)
	{
		SetVertexShader(SpotLightVS);
		SetPixelShader(SimpleTexPS);
		Shader->SetVSSingleSpotLight(&D3DLight);
	}
	else
	{
		D3DDevice->SetTextureStageState(2,  D3DTSS_TEXCOORDINDEX, 2);
		D3DDevice->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
		D3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
	}

	Light->pEmitor->Render();
	for (i = 0; i < Object.size(); i++)
		Object[i]->Render();


	// return back standard setting
	if (bShaderSupported)
	{
		D3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
	}
	else
	{
		D3DDevice->LightEnable(0, false);
	}
	
	D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	return ERRNOERROR;
};


/////////////////////////////////////////////////////////////////////////////////////////////
//
// this only creates a texture for later use with rendering of LOD0 terrain
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::InitTerrainOptimalization()
{
	HRESULT hr;

	// create texture for terrain optimization
	SAFE_RELEASE(TerrainTexture);
	hr = D3DDevice->CreateTexture(	uiLOD0TerrainTexWidth,
									uiLOD0TerrainTexHeight, 
									1,
									D3DUSAGE_RENDERTARGET,
									D3DFMT_A8R8G8B8,
									D3DPOOL_DEFAULT,
									&TerrainTexture,
									0 );
	if (FAILED(hr) )
		ERRORMSG(hr, "CGraphic::InitTerrainOptimalization()", "Unable to create complete terrain low-res texture");

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this function returns the index of a vertex inside of a terrain plate if you specify
//   the plate rotation, its width and depth (segments count), plate coordinates in the map,
//   mapwidth (count of fields in X direction) and the vertex index number inside vertex buffer
//  1 ----- 2,3 
//  |         |
// 0,5 ---- 4  
//
//////////////////////////////////////////////////////////////////////////////////////////////
int	CGraphic::GetVertexIndexAfterPlateRotation(	int platex, int platez,
												int width, int depth,
												int mapwidth,
												int rot,
												int index )
{
	int	mXx, mXz, mXd, mZx, mZz, mZd; // X and Z multipliers
	int x, z, newX, newZ;
	
	x = (index / 6) % width;
	z = (index / 6) / width;

	// pick proper indices
	switch ( index % 6 )
	{
	case 1: z++; break;
	case 2:
	case 3: z++; x++; break;
	case 4: x++; break;
	}
	
	mXx = mXz = mXd = mZx = mZz = mZd = 0;

	// perform axes rotation
	switch ( rot % 4 )
	{
	case 0: 
		mXx = mZz = 1;
		break;
	case 1: 
		mXz = mZd = 1;
		mZx = -1;
		break;
	case 2:
		mXx = mZz = -1;
		mXd = mZd = 1;
		break;
	case 3:
		mXd = mZx = 1;
		mXz = -1;
		break;
	}

	newX = mXx * x + mXz * z + mXd * width;
	newZ = mZx * x + mZz * z + mZd * depth;

	newX += platex * width;
	newZ += platez * depth;
	
	return ( mapwidth * width + 1 ) * newZ + newX;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// rotates the normal vector around y axis
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGraphic::RotateNormal( D3DXVECTOR3 * norm, int rot )
{
	float a;

	switch ( rot % 4 )
	{
	case 1:
		a = norm->x;
		norm->x = norm->z;
		norm->z = -a;
		break;
	case 2:
		norm->x = -norm->x;
		norm->z = -norm->z;
		break;
	case 3:
		a = norm->x;
		norm->x = -norm->z;
		norm->z = a;
		break;
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// computes the normal map for whole terrain
// this must be called before PerformTerrainOptimalization() is called
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGraphic::InitNormalMap()
{

	int					iMapWidth, iMapDepth; // map dimension

	D3DXMATRIX AllTerView;
	D3DXMATRIX AllTerProj;
	D3DVIEWPORT9 TerrainViewport;
	LPDIRECT3DSURFACE9 RenderTargetSurface;
	CGrObjectTerrainPlate * Plate = 0;
	HRESULT hr;
	APPRUNTIME tim = FPSCounter.GetRunTime();
	D3DXMATRIX PlaneMat;
	D3DLIGHT9 Light;
	vector<UINT> * ExtendedTable;
	int Width, Height;

	// some debug info
	OUTS( "Generating normal maps... ", 1 );

	Width = Scene->Map.GetWidth();
	Height = Scene->Map.GetDepth();
	if ( Width < 26)
		iMapWidth = 256;
	else if (Width < 51)
		iMapWidth = 512;
	else 
		iMapWidth = 1024;

	if ( Height < 26)
		iMapDepth = 256;
	else if ( Height < 51)
		iMapDepth = 512;
	else 
		iMapDepth = 1024;
	
	// create normal map texture
	hr = D3DDevice->CreateTexture( iMapWidth, iMapDepth, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &NormalMapTexture, NULL );
	if ( hr ) ERRORMSG( hr, "CGraphic::InitNormalMap()", "Unable to create normal map texture." );

	// set viewport, render target and all states
	TerrainViewport.X = 0;
	TerrainViewport.Y = 0;
	TerrainViewport.Width = iMapWidth;
	TerrainViewport.Height = iMapDepth;
	TerrainViewport.MinZ = 0;
	TerrainViewport.MaxZ = 1;
	
	D3DDevice->SetViewport(&TerrainViewport);
	
	//FloatTexture->GetSurfaceLevel(0, &RenderTargetSurface);
	NormalMapTexture->GetSurfaceLevel(0, &RenderTargetSurface);
	D3DDevice->SetRenderTarget(0, RenderTargetSurface);
	//D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	//D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);

	// set shaders for stage 1
	Shader->SetVertexShader(NormalMapStage1VS);
	Shader->SetPixelShader(NormalMapStage1PS);

	// compute and set matrices
	D3DXMatrixLookAtLH( &AllTerView, 
						& D3DXVECTOR3( (float) Scene->Map.GetWidth() * TERRAIN_PLATE_WIDTH / 2.0f,
							200 * ONE_METER,
							(float) Scene->Map.GetDepth() * TERRAIN_PLATE_WIDTH / 2.0f),
						& D3DXVECTOR3( (float) Scene->Map.GetWidth() * TERRAIN_PLATE_WIDTH / 2.0f,
							0,
							(float) Scene->Map.GetDepth() * TERRAIN_PLATE_WIDTH / 2.0f),
						& D3DXVECTOR3(0,0,-1) );

	D3DXMatrixOrthoLH(	&AllTerProj, (float) Scene->Map.GetWidth() * TERRAIN_PLATE_WIDTH,
						(float) Scene->Map.GetDepth() * TERRAIN_PLATE_WIDTH,
						10.0f * ONE_CENTIMETER,
						210 * ONE_METER );

	D3DXMatrixScaling(&PlaneMat, -1, 1, 1);
	
	Shader->SetViewMatrix( & (AllTerView * PlaneMat) );
	Shader->SetProjMatrix( &AllTerProj );

	Scene->SunLight->GetD3DLight( &Light );

	// set directional - sun - light for pixel shader
	ExtendedTable = Shader->GetExtendedTable( Shader->DirLight );
	Shader->SetPSValue( ExtendedTable->at(1), &Light.Diffuse, 16 );
	Shader->SetPSValue( ExtendedTable->at(3), &Light.Direction, 12 );

	// clear render target
	D3DDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(255, 255, 255, 255), 1, 0);

	D3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	// render scene
	hr = D3DDevice->BeginScene();

	for ( UINT x = 0; x < Scene->Map.GetWidth(); x++ )
		for ( UINT z = 0; z < Scene->Map.GetDepth(); z++ )
		{
			Plate = Scene->Map.GetPlate( x ,z );
			Plate->SetLOD( uiTerrainLOD ); // Set maximum quality LOD
			Plate->Render();
		}

	hr = D3DDevice->EndScene();

	D3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);


	// Return all things back
	D3DDevice->SetTexture(0, 0);
	D3DDevice->SetViewport(&StandardViewport);
	D3DDevice->SetRenderTarget(0, StandardRenderTarget);
	SAFE_RELEASE(RenderTargetSurface);
	
	//D3DXSaveTextureToFile( "lightmap.png", D3DXIFF_PNG, NormalMapTexture, NULL ); 

	OUTSN( " DONE.", 1 );
	OUTS( "Normal maps generated in ", 2 );
	OUTI( FPSCounter.GetRunTime() - tim, 2 );
	OUTSN( " ms", 2 );


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this function returns the index of a vertex inside of a map vertex 2D array if you specify
//   the plate rotation, map width, base index of lower left corner and the vertex
//   index number 0-5:
//  1 ----- 2,3 
//  |         |
// 0,5 ---- 4  
//
//////////////////////////////////////////////////////////////////////////////////////////////
int	CGraphic::GetMapIndexAfterRotation( int basex, int basez, int width, int rot, int index )
{
	int baseind = basez * (width + 1) + basex;
	int vertind = ( ((index % 6) < 3) ? (index + rot) : (index + rot - 1) ) % 4;
	
	switch ( vertind )
	{
	case 0: return baseind;
	case 1: return baseind + width + 1;
	case 2: return baseind + width + 2;
	case 3: return baseind + 1;
	}
	
	return baseind;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// prepares the date for fast LOD0 terrain rendering
// InitNormalMap must be called first
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::PerformTerrainOptimalization()
{
	struct __MAPVERT
	{
		bool			bUsed; // is used
		D3DXVECTOR3		pos, normal; // geometry
		int				iCount; // count of vertices contributing to the resultant normal vector
		int				iNewInd; // new vertex index after unused vertices removal
	};

	HRESULT				hr;
	resManNS::__GrSurface * PlateData; // terrain plate - rendering data
	D3DXMATRIX			PlateWorldMat; // plate world matrix
	DWORD			*	pIndices;
	__MAPVERT		*	pVertices;
	D3DXVECTOR3			vec1, vec2, normal;
	int					iVCount; // old vertex count
	float				fPixWidth, fPixDepth; // width and depth of a texture pixel
	float				fMapWidth, fMapDepth; // map width and depth
	TERRAINVERTEX	*	pVertexBuffer; // pointer to all plate vertices
	WORD			*	pIndexBuffer; // pointer to all plate indices
	APPRUNTIME			tim;
	int					iMapWidth, iMapDepth; // map dimension
	int					iMapWidthPlus1, iMapDepthPlus1; // map dimension plus 1

	/*
	// variables for GeometryReducer mode
	CGeometryReducer	GeometryReducer;
	int					iOrigVertCount, iOrigIndCount;
	TERRAINVERTEX		VertexData;
	CGeometryReducer::GRC_VERTEXINITDATA	NewVertices[3]; // vertices of newly added face
	CGeometryReducer::GRC_OUTPUTDATA		IOStruct; // IO struct for GetD3DBuffers method*/
	
	// Variables for rendering texture
	IDirect3DSurface9	*	TerrainSurface;
	D3DVIEWPORT9			TerrainViewport;
	D3DXMATRIX				AllTerView; // complete terrain view matrix
	D3DXMATRIX				AllTerProj; // complete terrain projection matrix
	LIGHTS_ITER				It; // Light iterator
	D3DLIGHT9				Sun;
	CGrObjectTerrainPlate *	Plate;
	float					fValue;



	// some debug info
	OUTMSG( "Perform terrain geometry optimization...", 1 );


	// INITIALIZING PART

	tim = FPSCounter.GetRunTime();
	iMapWidth = (int) Scene->Map.GetWidth();
	iMapDepth = (int) Scene->Map.GetDepth();
	iMapWidthPlus1 = iMapWidth + 1;
	iMapDepthPlus1 = iMapDepth + 1;
	fMapWidth = iMapWidth * TERRAIN_PLATE_WIDTH;
	fMapDepth = iMapDepth * TERRAIN_PLATE_WIDTH;
	fPixWidth = 1.0f / (float) uiLOD0TerrainTexWidth;
	fPixDepth = 1.0f / (float) uiLOD0TerrainTexHeight;

	iVCount = iMapWidthPlus1 * iMapDepthPlus1;
	uiTerrainIndexCount = 6 * iMapWidth * iMapDepth;

	pIndices = new DWORD[uiTerrainIndexCount];
	pVertices = new __MAPVERT[iVCount];
	if ( !pIndices || !pVertices ) 
		ERRORMSG( ERROUTOFMEMORY, "CGraphic::PerformTerrainOptimalization()", "Cannot create auxiliary structures." );

	ZeroMemory( pVertices, iVCount * sizeof( __MAPVERT ) );
	for ( int iz = 0; iz < iMapDepthPlus1; iz++ )
		for ( int ix = 0; ix < iMapWidthPlus1; ix++ )
			pVertices[iz * iMapWidthPlus1 + ix].normal = D3DXVECTOR3( 0, 0, 0 );




	// BUILD THE GEOMETRY DATA

	uiTerrainVertexCount = uiTerrainIndexCount = 0;

	// process each map plate
	for ( int iz = 0; iz < iMapDepth; iz++ )
	{
		for ( int ix = 0; ix < iMapWidth; ix++ )
		{
			UINT	facesCount;

			PlateData = ResourceManager->GetGrSurface( Scene->Map.GetPlate( ix, iz )->GetLODResourceID( 0 ) );
			PlateWorldMat = Scene->Map.GetPlate( ix, iz )->GetWorldMat();
		
			PlateData->pVertices->Lock( 0, 0, (void **) &pVertexBuffer, 0 );
			PlateData->pIndices->Lock( 0, 0, (void **) &pIndexBuffer, 0 );

			// process all faces of this plate
			facesCount = PlateData->SegsCountX * PlateData->SegsCountZ * 2;
			for ( UINT k = 0; k < facesCount; k++ )			
			{
				int rot = Scene->Map.GetFieldRotation( ix, iz ); // 0 = North
				int index0 = GetMapIndexAfterRotation( ix, iz, iMapWidth, rot, 3*k     );
				int index1 = GetMapIndexAfterRotation( ix, iz, iMapWidth, rot, 3*k + 1 );
				int index2 = GetMapIndexAfterRotation( ix, iz, iMapWidth, rot, 3*k + 2 );

				D3DXVec3TransformCoord( &(pVertices[index0].pos), &pVertexBuffer[pIndexBuffer[3*k  ]].Pos, &PlateWorldMat );
				D3DXVec3TransformCoord( &(pVertices[index1].pos), &pVertexBuffer[pIndexBuffer[3*k+1]].Pos, &PlateWorldMat );
				D3DXVec3TransformCoord( &(pVertices[index2].pos), &pVertexBuffer[pIndexBuffer[3*k+2]].Pos, &PlateWorldMat );

				vec1 =	pVertices[index0].pos - 
						pVertices[index1].pos;
				vec2 =	pVertices[index2].pos - 
						pVertices[index1].pos;
				D3DXVec3Cross( &normal, &vec2, &vec1 );
				
				// normals must be added per quad, not per face, thus only first two vertices will obtain normal
				pVertices[index0].normal += normal;
				pVertices[index1].normal += normal;
			//	pVertices[index2].normal += normal;

				pVertices[index0].iCount += 1;
				pVertices[index1].iCount += 1;
			//	pVertices[index2].iCount += 1;
			}

			PlateData->pVertices->Unlock();
			PlateData->pIndices->Unlock();
		}
	}




	// CREATE INDICES
	for ( int iz = 0; iz < iMapDepth; iz++ )
	{
		for ( int ix = 0; ix < iMapWidth; ix++ )
		{
			bool	bFound = false;
			/* // merging faces optimalization - brings other problem with bad normals
			if ( Scene->Map.GetPlate( ix, iz )->IsPlanar() ) // planar field, lets try to find larger area
			{ 
				int		len = 0;
				for ( int dx = ix + 1; dx < iMapWidth; dx++ )
				{
					if ( !Scene->Map.GetPlate( dx, iz )->IsPlanar() ) break;
					len++;
				}
				
				if ( len ) // larger horizontal area found -> replace it with two faces only
				{
					bFound = true;
					
					pIndices[uiTerrainIndexCount++] = iz       * iMapWidthPlus1 + ix;
					pIndices[uiTerrainIndexCount++] = (iz + 1) * iMapWidthPlus1 + ix;
					pIndices[uiTerrainIndexCount++] = (iz + 1) * iMapWidthPlus1 + ix + len + 1;

					pIndices[uiTerrainIndexCount++] = (iz + 1) * iMapWidthPlus1 + ix + len + 1;
					pIndices[uiTerrainIndexCount++] = iz       * iMapWidthPlus1 + ix + len + 1;
					pIndices[uiTerrainIndexCount++] = iz       * iMapWidthPlus1 + ix;

					pVertices[iz       * iMapWidthPlus1 + ix          ].bUsed = true;
					pVertices[(iz + 1) * iMapWidthPlus1 + ix          ].bUsed = true;
					pVertices[(iz + 1) * iMapWidthPlus1 + ix + len + 1].bUsed = true;
					pVertices[iz       * iMapWidthPlus1 + ix + len + 1].bUsed = true;
				}

				ix += len;
			}*/

			if ( !bFound ) // no larger area of planar fields found -> add just one plate
			{ 
				int rot = Scene->Map.GetFieldRotation( ix, iz ) % 2; // 0 = N or S direction, 1 = W or E
				
				// West and East direction needs different vertex order so that the square diagonal
				//   does not cross the hillside
				pIndices[uiTerrainIndexCount++] = iz         * iMapWidthPlus1 + ix;
				pIndices[uiTerrainIndexCount++] = (iz + 1)   * iMapWidthPlus1 + ix;
				pIndices[uiTerrainIndexCount++] = (iz + 1 - rot) * iMapWidthPlus1 + ix + 1;

				pIndices[uiTerrainIndexCount++] = (iz + 1)   * iMapWidthPlus1 + ix + 1;
				pIndices[uiTerrainIndexCount++] = iz         * iMapWidthPlus1 + ix + 1;
				pIndices[uiTerrainIndexCount++] = (iz + rot) * iMapWidthPlus1 + ix;

				pVertices[iz       * iMapWidthPlus1 + ix          ].bUsed = true;
				pVertices[(iz + 1) * iMapWidthPlus1 + ix          ].bUsed = true;
				pVertices[(iz + 1) * iMapWidthPlus1 + ix + 1].bUsed = true;
				pVertices[iz       * iMapWidthPlus1 + ix + 1].bUsed = true;
			}
		}
	}




	// CREATE AND FILL DX STRUCTURES

	// count used vertices
	uiTerrainVertexCount = 0;
	for ( int i = 0; i < iVCount; i++ ) if ( pVertices[i].bUsed ) uiTerrainVertexCount++;

	// create vertex buffer
	hr = D3DDevice->CreateVertexBuffer( uiTerrainVertexCount * sizeof( TERRAINVERTEX ), 
									0, TerrainVertexFVF, D3DPOOL_MANAGED, &TerrainVBuffer, NULL );
	if ( hr ) ERRORMSG( hr, "CGraphic::PerformTerrainOptimalization()", "Unable to create terrain vertex buffer." );

	// create index buffer
	hr = D3DDevice->CreateIndexBuffer( uiTerrainIndexCount * sizeof( *pIndexBuffer ),
									0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &TerrainIBuffer, NULL );
	if ( hr ) ERRORMSG( hr, "CGraphic::PerformTerrainOptimalization()", "Unable to create terrain index buffer." );
	
	hr = TerrainVBuffer->Lock( 0, 0, (void **) &pVertexBuffer, 0 );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGraphic::PerformTerrainOptimalization()", "Unable to lock terrain vertex buffer." );

	hr = TerrainIBuffer->Lock( 0, 0, (void **) &pIndexBuffer, 0 );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGraphic::PerformTerrainOptimalization()", "Unable to lock terrain index buffer." );


	// copy vertices to the vertex buffer
	uiTerrainVertexCount = 0;
	for ( int i = 0; i < iVCount; i++ )
	{
		if ( pVertices[i].bUsed )
		{
			pVertices[i].normal /= (float) pVertices[i].iCount; // make the normal an average of all surrounding faces
			pVertexBuffer[uiTerrainVertexCount].Pos = pVertices[i].pos;
			D3DXVec3Normalize( &(pVertices[i].normal), & (pVertices[i].normal) );
			pVertexBuffer[uiTerrainVertexCount].Normal = pVertices[i].normal;
			pVertexBuffer[uiTerrainVertexCount].TexCoord.x = 1 - pVertices[i].pos.x / fMapWidth + 0.5f * fPixWidth;
			pVertexBuffer[uiTerrainVertexCount].TexCoord.y = pVertices[i].pos.z / fMapDepth + 0.5f * fPixDepth;
		
			pVertices[i].iNewInd = uiTerrainVertexCount; // store the new vertex index -> perform reindexing

			uiTerrainVertexCount++;
		}
	}


	// copy indices to index buffer
	for ( int i = 0; i < (int) uiTerrainIndexCount; i++ )
		pIndexBuffer[i] = pVertices[pIndices[i]].iNewInd;


	TerrainVBuffer->Unlock();
	TerrainIBuffer->Unlock();

	SAFE_DELETE_ARRAY( pIndices );
	SAFE_DELETE_ARRAY( pVertices );


/*
	// OLD SLOW VERSION WITH GeometryReducer class

	// some debug info
	OUTMSG( "Perform terrain geometry optimization...", 1 );
	OUTS( "\toriginal count of vertices: ", 2 );
	iOrigVertCount = Scene->Map.GetWidth() * Scene->Map.GetDepth() *
					( ( 1 << uiTerrainLOD ) + 1 ) * ( (1 << uiTerrainLOD) + 1);
	OUTIN( iOrigVertCount, 2 );
	OUTS( "\toriginal count of indices: ", 2 );
	OUTIN( 6 * Scene->Map.GetWidth() * Scene->Map.GetDepth() * (1 << uiTerrainLOD) * (1 << uiTerrainLOD), 2 );
	OUTS( "\tinitializing geometry reducer with terrain data...", 2 );




	// INIT TERRAIN GEOMETRY
	iOrigVertCount = 0;
	iOrigIndCount = 0;



	hr = GeometryReducer.Init();
	if ( hr ) ERRORMSG( ERRGENERIC, "CGraphic::PerformTerrainOptimalization()", "Can't intitialize terrain geometry reduction." );

	
	// process each map plate
	for ( UINT j = 0; j < Scene->Map.GetDepth(); j++ )
		for ( UINT i = 0; i < Scene->Map.GetWidth(); i++ )
		{
			UINT	lod, facesCount;

			if ( Scene->Map.GetPlate( i, j )->IsConcave() ) lod = 0;
			else lod = max( 0, (int) uiTerrainLOD - 3 );
			
			PlateData = ResourceManager->GetGrSurface( Scene->Map.GetPlate( i, j )->GetLODResourceID( lod ) );
			PlateWorldMat = Scene->Map.GetPlate( i, j )->GetWorldMat();
		
			PlateData->pVertices->Lock( 0, 0, (void **) &pVertexBuffer, 0 );
			PlateData->pIndices->Lock( 0, 0, (void **) &pIndexBuffer, 0 );

			for ( int k = 0; k < 3; k++ ) 
			{
				NewVertices[k].bDontTouch = false;
				NewVertices[k].pUserData = NULL;
			}

			// process all faces of this plate
			facesCount = PlateData->SegsCountX * PlateData->SegsCountZ * 2;
			for ( UINT k = 0; k < facesCount; k++ )			
			{
				UINT	index;
				
				index = 3*k;
				D3DXVec3TransformCoord( &(NewVertices[0].Position), &(pVertexBuffer[pIndexBuffer[index]].Pos), &PlateWorldMat );
				D3DXVec3TransformCoord( &(NewVertices[1].Position), &(pVertexBuffer[pIndexBuffer[index + 1]].Pos), &PlateWorldMat );
				D3DXVec3TransformCoord( &(NewVertices[2].Position), &(pVertexBuffer[pIndexBuffer[index + 2]].Pos), &PlateWorldMat );

				hr = GeometryReducer.AddFace( NULL, &NewVertices[0], &NewVertices[1], &NewVertices[2] );
				if ( hr ) ERRORMSG( ERRGENERIC, "CGraphic::PerformTerrainOptimalization()", "Can't intitialize terrain geometry." );
			}

			PlateData->pVertices->Unlock();
			PlateData->pIndices->Unlock();

			iOrigVertCount += PlateData->VertsCount;
			iOrigIndCount += facesCount * 3;
		}


	hr = GeometryReducer.CloseInput();
	if ( hr ) ERRORMSG( ERRGENERIC, "CGraphic::PerformTerrainOptimalization()", "Can't intitialize terrain geometry." );



	// some debug info
	OUTSN( " DONE", 2 );
	OUTS( "\treducing terrain geometry... ", 2 );




	// GET REDUCED GEOMETRY

	// reduce geometry
	hr = GeometryReducer.ReduceGeometry( 5.0f, true, Scene->Map.GetWidth(), Scene->Map.GetDepth(), 3 );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGraphic::PerformTerrainOptimalization()", "Geometry reduction failed." );


	IOStruct.pD3DDevice = D3DDevice;
	IOStruct.uiVertexStride = sizeof( TERRAINVERTEX );
	IOStruct.dwNormalOffset = sizeof( D3DXVECTOR3 );
	IOStruct.dwVBufferUsage = 0;
	IOStruct.dwIBufferUsage = 0;
	IOStruct.VBufferPool = D3DPOOL_MANAGED;
	IOStruct.IBufferPool = D3DPOOL_MANAGED;
	IOStruct.dwFVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
	IOStruct.IBufferFormat = D3DFMT_INDEX32;

	SAFE_RELEASE( TerrainVBuffer );
	SAFE_RELEASE( TerrainIBuffer );

	// obtain reduced geometry
	hr = GeometryReducer.GetD3DBuffers( &IOStruct, &TerrainVBuffer, &TerrainIBuffer );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGraphic::PerformTerrainOptimalization()", "Can't utilize reduced geometry." );



	
	// compute texture coordinates (missing information)
	hr = TerrainVBuffer->Lock( 0, 0, (void **) &pVertexBuffer, 0 );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGraphic::PerformTerrainOptimalization()", "Unable to lock terrain vertex buffer." );

	for ( int i = 0; i < IOStruct.iVertexCount; i++ )
	{
		pVertexBuffer[i].TexCoord.x = 1 - pVertexBuffer[i].Pos.x / (Scene->Map.GetWidth() * TERRAIN_PLATE_WIDTH) + 0.5f / (float) uiLOD0TerrainTexWidth;
		pVertexBuffer[i].TexCoord.y = pVertexBuffer[i].Pos.z / (Scene->Map.GetDepth() * TERRAIN_PLATE_WIDTH) + 0.5f / (float) uiLOD0TerrainTexHeight;
	}

	TerrainVBuffer->Unlock();

	uiTerrainVertexCount = (UINT) IOStruct.iVertexCount;
	uiTerrainIndexCount = (UINT) IOStruct.iIndexCount;



	// some debug info
	OUTSN( " DONE", 2 );
	OUTS( "\tcount of vertices after geometry reduction: ", 2 );
	OUTIN( 2 * 3 * Scene->Map.GetWidth() * Scene->Map.GetDepth(), 2 );
	OUTS( "\tcount of vertices after triangulation: ", 2 );
	OUTIN( uiTerrainVertexCount, 2 );
	OUTS( "\tcount of indices after triangulation: ", 2 );
	OUTIN( uiTerrainIndexCount, 2 );
	OUTS( "\tpreparing world texture... ", 2 );
*/




	// RENDER TERRAIN TO A TEXTURE

	// set viewport, render target and all states
	TerrainViewport.X = 0;
	TerrainViewport.Y = 0;
	TerrainViewport.Width = uiLOD0TerrainTexWidth;
	TerrainViewport.Height = uiLOD0TerrainTexHeight;
	TerrainViewport.MinZ = 0;
	TerrainViewport.MaxZ = 1;

  	TerrainTexture->GetSurfaceLevel( 0, &TerrainSurface );

	D3DDevice->SetRenderTarget( 0, TerrainSurface );
	D3DDevice->SetDepthStencilSurface( NULL );
	D3DDevice->SetViewport( &TerrainViewport );
	D3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
	D3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE);
	D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO);

	D3DDevice->SetTexture( 3, NormalMapTexture );

	BOOL Day;
	float frac, n;
	// Is day or night???
	frac = modff(Scene->SkySystem.GetSkyTime(), &n);
	Day = frac > Scene->SkySystem.GetDayStart() && frac < Scene->SkySystem.GetDayEnd();

	
	SetVertexShader( TerrainVS );
	if (Day)
	{
		SetPixelShader( TerrainLOD0TexturePS );
		// set light
		Scene->SunLight->GetD3DLight( &Sun );
		//Shader->SetVSSingleDirLight( &Sun );
		Shader->SetPSValue( Shader->Ambient, &(Scene->AmbientLight->Ambient), sizeof(D3DXVECTOR4) );
	}
	else
	{
		SetPixelShader( TerrainLOD0TextureNightPS );
	}


	// compute and set matrices
	D3DXMatrixLookAtLH( &AllTerView, 
						& D3DXVECTOR3( (float) Scene->Map.GetWidth() * TERRAIN_PLATE_WIDTH / 2.0f,
							200 * ONE_METER,
							(float) Scene->Map.GetDepth() * TERRAIN_PLATE_WIDTH / 2.0f),
						& D3DXVECTOR3( (float) Scene->Map.GetWidth() * TERRAIN_PLATE_WIDTH / 2.0f,
							0,
							(float) Scene->Map.GetDepth() * TERRAIN_PLATE_WIDTH / 2.0f),
						& D3DXVECTOR3(0,0,-1) );

	D3DXMatrixOrthoLH(	&AllTerProj, (float) Scene->Map.GetWidth() * TERRAIN_PLATE_WIDTH,
						(float) Scene->Map.GetDepth() * TERRAIN_PLATE_WIDTH,
						10.0f * ONE_CENTIMETER,
						210 * ONE_METER );
	
	Shader->SetViewMatrix( &AllTerView );
	Shader->SetProjMatrix( &AllTerProj );

	// set map width and depth
	fValue = (float) Scene->Map.GetWidth() * TERRAIN_PLATE_WIDTH;
	Shader->SetVSValue( MapWidthHandle, &fValue, sizeof( fValue ) );
	fValue = (float) Scene->Map.GetDepth() * TERRAIN_PLATE_WIDTH;
	Shader->SetVSValue( MapDepthHandle, &fValue, sizeof( fValue ) );

	

	// render scene
	hr = D3DDevice->BeginScene();

	D3DDevice->Clear( 0, 0, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0,0,0,255), 1, 0 );

	for ( UINT x = 0; x < Scene->Map.GetWidth(); x++ )
		for ( UINT z = 0; z < Scene->Map.GetDepth(); z++ )
		{
			Plate = Scene->Map.GetPlate( x ,z );
			Plate->SetLOD( uiTerrainLOD ); // Set maximum quality LOD
			Plate->Render();
		}

	hr = D3DDevice->EndScene();


	D3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE);
	D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	D3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
	D3DDevice->SetRenderTarget( 0, StandardRenderTarget );
	D3DDevice->SetDepthStencilSurface( DepthStencil );
	D3DDevice->SetViewport( &StandardViewport );
	
	SAFE_RELEASE( TerrainSurface );

	//D3DXSaveTextureToFile( "TerrainLOD0Texture.bmp", D3DXIFF_BMP, TerrainTexture, 0 );


	/*	
	OUTSN( " DONE", 2 );
	OUTS( "Terrain geometry optimization successfully done. Geometry reduced to ", 1 );
	OUTF( (float)IOStruct.iVertexCount / (float)iOrigVertCount, 1 );
	OUTSN( " %.", 1 ); */
	
	OUTS( "Time spent on geometry reduction: ", 2 );
	OUTI( FPSCounter.GetRunTime() - tim, 2 );
	OUTSN( " ms", 2 );

		
	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// render terrain optimalization - LOD 0 transformed to world coords with terrain texture
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::RenderTerrainOptimalization(BOOL Day)
{
	D3DXMATRIX Identity;
	GROBJECTS_ITER It;
	CGrObjectMesh * DynMesh;
	SpotLightShaderStructVector SpotLightsVector[9];
	int SLNumber = 0; // Spot light number
	CLight * CarLight;
	vector<UINT> * ExtendedTable;
	UINT i;
	D3DLIGHT9 SunLight;
	
	//D3DXMatrixIdentity(&Identity);

	Shader->SetWorldMatrix( &IdentityMatrix );
	
	if ( Day )
	{
		SetVertexShader( TerrainLOD0DLVS );
		SetPixelShader( TerrainLOD0DLPS );
		//SetVertexShader(TerrainLOD0PS);
		//SetPixelShader(TerrainLOD0VS);

		// set fog color	
		Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );

		// Set sun light
		Scene->SunLight->GetD3DLight(&SunLight);

		//SunLight.Direction = Scene->SkySystem.GetSunNormal();
		
		//SunLight.Diffuse.r = Scene->SkySystem.GetSunColor().x;
		//SunLight.Diffuse.g = Scene->SkySystem.GetSunColor().y;
		//SunLight.Diffuse.b = Scene->SkySystem.GetSunColor().z;

		Shader->SetVSSingleDirLight(&SunLight);
		
	}
	else
	{
		// Traverse all dynamic objects (cars)
		for (It = Scene->DynamicObjects.begin(); It != Scene->DynamicObjects.end(); It++)
		{
			DynMesh = (CGrObjectMesh *) (*It);
			if (DynMesh->GetLight() && DynMesh->GetLight()->Type == LT_CARREFLECTOR)
			{
				// Get car light bounding rect
				//FindCarLightBoundingRect(DynMesh->GetLight(), DynMesh->GetWorldMat(), &LightBoundRect);
				//Shader->SetVSSingleSpotLight(	
				// Perform conversion into spotlight shader structure
				CarLight = DynMesh->GetLight();
				ConvertToShaderSpotLightVector(CarLight, DynMesh->GetWorldMat(), &SpotLightsVector[SLNumber] );
				SLNumber++;
				if (SLNumber == 9) break; // we are not capable to handle more lights
			}
		}

		// Set vertex shader
		Shader->SetVertexShader(TerrainNightLOD0VS);
		

		ExtendedTable = Shader->GetExtendedTable(SpotLightsHandle);
		for (i = 0; i < 9; i++)
		{
			Shader->SetVSValue(ExtendedTable->at(i * 7), &SpotLightsVector[i].Ambient, 16); 
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 1), &SpotLightsVector[i].Diffuse, 16);
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 2), &SpotLightsVector[i].Specular, 16);
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 3), &SpotLightsVector[i].Position, 16);
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 4), &SpotLightsVector[i].Direction, 16);
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 5), &SpotLightsVector[i].Atten, 16);
			Shader->SetVSValue(ExtendedTable->at(i * 7 + 6), &SpotLightsVector[i].Angles, 16);
		}
		

		// Set pixel shader
		Shader->SetPixelShader( TerrainNightLOD0PS );
	}

	// Set ambient into pixel shader
	Shader->SetPSValue( Shader->Ambient, & (Scene->AmbientLight->Ambient), sizeof(D3DXVECTOR4) );
	
	// Set material
	Shader->SetVSMaterial( &TerrainMaterial );

	// render terrain LOD 0 
	D3DDevice->SetTexture( 0, TerrainTexture );

	D3DDevice->SetIndices( TerrainIBuffer );
	D3DDevice->SetFVF( TerrainVertexFVF );
	D3DDevice->SetStreamSource( 0, TerrainVBuffer, 0, sizeof(TERRAINVERTEX) );

	D3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, uiTerrainVertexCount, 0, uiTerrainIndexCount / 3 );
		
	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders objects out of focus area around player
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::RenderFarObjects( MAPPOSITION * pos, int range, 
									BOOL bRenderStaticObjects,
									BOOL bRenderDynamicObjects,
									BOOL Day )
{
	GROBJECTS_ITER			objIter;
	D3DXCOLOR				ambLightCol;
	D3DLIGHT9				Light;
	

	// Set vertex and pixel shader
	if ( Day ) 
	{
		SetVertexShader( ObjectVS );
		SetPixelShader( ObjectPS );
		Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );
	}
	else
	{
		SetVertexShader( ObjectNoFogVS );
		SetPixelShader( ObjectNightPS );
	}

	// get ambient light
	ambLightCol.r = Scene->AmbientLight->Ambient.r;
	ambLightCol.g = Scene->AmbientLight->Ambient.g;
	ambLightCol.b = Scene->AmbientLight->Ambient.b;
	ambLightCol.a = Scene->AmbientLight->Ambient.a;

	// get sun light
	Scene->SunLight->GetD3DLight(&Light);

	// Set ambient color and directional light
	Shader->SetPSValue(Shader->Ambient, &ambLightCol, sizeof(ambLightCol) );
	Shader->SetVSSingleDirLight( &Light );

	if ( bMapDependentRendering )
	{
		// Render static objects
		if (bRenderStaticObjects)
			for ( objIter = Scene->StaticObjects.begin(); objIter != Scene->StaticObjects.end(); objIter++ )
				if ( Scene->Map.ObjectIsVisible( (*objIter)->GetMapPosList() ) ) 
					if ( !pos || !(*objIter)->InterferesToSquareArea( pos, range ) )
						(*objIter)->Render();
		// Render dynamic objects
		if (bRenderDynamicObjects)
			for ( objIter = Scene->DynamicObjects.begin(); objIter != Scene->DynamicObjects.end(); objIter++ )
				if ( Scene->Map.ObjectIsVisible( (*objIter)->GetMapPosList() ) ) 
					if ( !pos || !(*objIter)->InterferesToSquareArea( pos, range ) )
						(*objIter)->Render();
	}
	else
	{
		// Render static objects
		if (bRenderStaticObjects)
			for ( objIter = Scene->StaticObjects.begin(); objIter != Scene->StaticObjects.end(); objIter++ )
				(*objIter)->Render();
		// Render dynamic objects
		if (bRenderDynamicObjects)
			for ( objIter = Scene->DynamicObjects.begin(); objIter != Scene->DynamicObjects.end(); objIter++ )
				(*objIter)->Render();
	}

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders scene depth from sun view
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::RenderDepthSceneFromSun(D3DLIGHT9 * Light, D3DXMATRIX * LightViewMat, D3DXMATRIX * LightProjMat, D3DXMATRIX * TSMMatrix)
{
	D3DVIEWPORT9 DepthView;
	IDirect3DSurface9 * DepthStencil;
	HRESULT hr;
	


	SetVertexShader(TSMDepthVS);
	SetPixelShader(TSMDepthPS);

	Shader->SetViewMatrix(LightViewMat );
	Shader->SetProjMatrix(LightProjMat);

	Shader->SetVSMatrix(PostProjMatHandle, TSMMatrix);
	
	// set depth render target
	D3DDevice->SetRenderTarget(0, ShadowMap->GetDynShadowSurf() );
	
	// init viewport
	DepthView.X = 0;
	DepthView.Y = 0;
	DepthView.Width = ShadowMap->GetSunMapSize();
	DepthView.Height = ShadowMap->GetSunMapSize();
	DepthView.MinZ = 0;
	DepthView.MaxZ = 1;

	D3DDevice->SetViewport(&DepthView);

	hr = D3DDevice->GetDepthStencilSurface(&DepthStencil);
	if ( hr ) ERRORMSG( ERRNOTFOUND, "CGraphic::RenderDepthSceneFromSun()", "Depth buffer not initialized." );

	D3DDevice->SetDepthStencilSurface(DepthStencil2);
	
	// Clear target
	D3DDevice->Clear(0, 0, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, D3DCOLOR_RGBA(255, 255, 255, 255), 1, 0);

	// set some border
	DepthView.X = 1;
	DepthView.Y = 1;
	DepthView.Width = ShadowMap->GetSunMapSize() - 2;
	DepthView.Height = ShadowMap->GetSunMapSize() - 2;
	
	D3DDevice->SetViewport(&DepthView);

	if (ShadowMap->GetDynMapFormat() == D3DFMT_D24S8)
	{
		D3DDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0);
	}
	

	// Render
	hr = RenderAllObjects( NULL, 0, bRenderStaticObjects, bRenderDynamicObjects );
	if ( hr ) ERRORMSG( hr, "CGraphic::RenderDepthSceneFromSun()", "Unable to render one or more scene objects." );

	if (ShadowMap->GetDynMapFormat() == D3DFMT_D24S8)
	{
		D3DDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0x0000000F);
	}

	//D3DXSaveTextureToFile("TSMDepth2.jpg", D3DXIFF_JPG, ShadowMap->GetDynShadowTex(), 0); 

	D3DDevice->SetRenderTarget(0, StandardRenderTarget);
	D3DDevice->SetDepthStencilSurface(DepthStencil);

	D3DDevice->SetViewport(&StandardViewport);

	//ViewMatrix = *LightViewMat;
	//ProjectionMatrix = *LightProjMat;

	Shader->SetViewMatrix(&ViewMatrix);
	Shader->SetProjMatrix(&ProjectionMatrix);
	
	SAFE_RELEASE(DepthStencil);

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Finds similar regions for terrain optimization
// Rects return map positions of rectangles with similar properties (plate type, rotation)
// All four rectangle corners are returned (Lower left, lower right, upper right, upper left)
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::FindSimilarRegions(vector<MAPPOSITION> * Rects)
{
	CGrObjectTerrainPlate * Plate;
	resManNS::RESOURCEID CurrentType;
	UINT CurrentRotation;
	UINT x, z; // also counts as lower left x and z
	UINT i;
	int dx, dz; // direction x, z
	int px, pz; // perpendicular direction x, z
	int cx, cz; // current x, z
	int ex, ez; // expand x, z
	int urx, urz; // Upper right x and z
	bool AllSimilar;
	MAPPOSITION MapPos;
	
	// Area just covered by similar region search algorithm
	bool * MapCover = new bool[Scene->Map.GetWidth() * Scene->Map.GetDepth() ];
	if (!MapCover)
		ERRORMSG(ERROUTOFMEMORY, "CGraphic::FindSimilarRegions()", "Unable to allocate place for map cover.");

	ZeroMemory(MapCover, Scene->Map.GetWidth() * Scene->Map.GetDepth() * sizeof(bool) );
	
	for (x = 0; x < Scene->Map.GetWidth(); x++)
		for (z = 0; z < Scene->Map.GetDepth(); z++)
			if (! MapCover[x * Scene->Map.GetWidth() + z])
			{
				MapCover[x * Scene->Map.GetWidth() + z] = true;
				Plate = Scene->Map.GetPlate(x, z);
				CurrentType = Plate->GetTerrainType();
				CurrentRotation = Scene->Map.GetFieldRotation(x, z);
				
				// determine direction to search for similar
				dx = dz = 0;
				if (x + 1 < Scene->Map.GetWidth() )
				{
					Plate = Scene->Map.GetPlate(x + 1, z);
					if (Plate->GetTerrainType() == CurrentType && 
						Scene->Map.GetFieldRotation(x + 1, z) == CurrentRotation && 
						! MapCover[ (x + 1) * Scene->Map.GetWidth() + z] )
					{
						dx = 1; dz = 0;
					}
				}
				if (z + 1 < Scene->Map.GetDepth() && dx == 0)
				{
					Plate = Scene->Map.GetPlate(x, z + 1);
					if (Plate->GetTerrainType() == CurrentType && 
						Scene->Map.GetFieldRotation(x, z + 1) == CurrentRotation && 
						! MapCover[x * Scene->Map.GetWidth() + z + 1])
					{
						dx = 0; dz = 1;
					}
				}

				if (dx == 0 && dz == 0)
				{
					urx = x; urz = z;
				}
				else
				{
					cx = x + dx;
					cz = z + dz;
					// find furthest similar plate in given direction
					while (true)
					{
						if (cx == Scene->Map.GetWidth() || cz == Scene->Map.GetDepth())
						{
							cx -= dx;
							cz -= dz;
							break;
						}

						Plate = Scene->Map.GetPlate(cx, cz);
						if (Plate->GetTerrainType() == CurrentType && 
							Scene->Map.GetFieldRotation(cx, cz) == CurrentRotation && 
							! MapCover[cx * Scene->Map.GetWidth() + cz] )
						{
							MapCover[cx * Scene->Map.GetWidth() + cz] = true;
						}
						else
						{
							cx -= dx;
							cz -= dz;
							break;
						}

						cx += dx;
						cz += dz;
					}

					// try to expand in perpendicular direction
					// get perpendicular direction
					if (dx == 0)
					{
						px = 1;
						pz = 0;
					}
					else
					{
						px = 0;
						pz = 1;
					}

					ex = x + px;
					ez = z + pz;
					while (true)
					{
						// check for going out of map
						if (ex == Scene->Map.GetWidth() || ez == Scene->Map.GetDepth() )
						{
							ex -= px;
							ez -= pz;
							break;
						}

						// check plates in direction
						AllSimilar = true;
						for (i = 0; i <= max(cx - x, cz - z); i++)
						{
							Plate = Scene->Map.GetPlate(ex + i * dx, ez + i * dz);
							if (Plate->GetTerrainType() != CurrentType || 
								Scene->Map.GetFieldRotation(ex + i * dx, ez + i * dz) != CurrentRotation ||
								MapCover[ (ex + i * dx) * Scene->Map.GetWidth() + ez + i * dz])
							{
								AllSimilar = false;
								break;
							}
						}

						if (! AllSimilar)
						{
							ex -= px;
							ez -= pz;
							break;
						}

						// mark plates in direction
						for (i = 0; i <= max(cx - x, cz - z); i++)
						{
							MapCover[ (ex + i * dx) * Scene->Map.GetWidth() + ez + i * dz] = true;
						}

						ex += px;
						ez += pz;
					}

					// determine upper right corner
					urx = max(cx, ex);
					urz = max(cz, ez);
				}
			
				// Add similar rect information
				MapPos.PosX = x;
				MapPos.PosZ = z;
				Rects->push_back(MapPos);
				MapPos.PosX = urx;
				Rects->push_back(MapPos);
				MapPos.PosZ = urz;
				Rects->push_back(MapPos);
				MapPos.PosX = x;
				Rects->push_back(MapPos);
			}
		
	delete [] MapCover;

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders sky system
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGraphic::RenderSkySystem( BOOL day )
{
	D3DXMATRIX Identity;
	D3DXMATRIX AlteredViewMat;

	D3DXMatrixIdentity(&Identity);

	Shader->SetWorldMatrix(&Identity);

	// Remove translation vector from view matrix
	D3DXMatrixLookAtLH(&AlteredViewMat, &D3DXVECTOR3(0,0,0),
		&(Camera.GetLookAtVector() - Camera.GetEyePtVector()),
		&Camera.GetUpVector() );

	Shader->SetViewMatrix(&AlteredViewMat);
	
	D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO);

	D3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	// Set shaders
	SetVertexShader(SkySystemVS);
	SetPixelShader(SkySystemPS);

	// Render sky system
	Scene->SkySystem.Render();

	// Render star field
	SetVertexShader(StarsVS);
	SetPixelShader(StarsPS);

	Scene->SkySystem.RenderStarField();

	// Render clouds
	SetVertexShader(CloudVS);
	SetPixelShader(CloudPS);
	
	
	D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	Scene->SkySystem.RenderClouds();

	D3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	if ( day )
	{
		SetVertexShader( MountainVS );
		SetPixelShader( MountainPS );
		Shader->SetPSValue( FogColorHandle, &FogColor, sizeof( FogColor ) );
	}
	else
	{
		SetVertexShader( MountainNightVS );
		SetPixelShader( MountainNightPS );
	}
	
	D3DXMatrixTranslation(&Identity, Camera.GetEyePtVector().x, Camera.GetEyePtVector().y, Camera.GetEyePtVector().z);
	Shader->SetWorldMatrix(&Identity);
	Shader->SetViewMatrix(&ViewMatrix);
	
	Scene->SkySystem.RenderMountains();

	return ERRNOERROR;
};


// Compares to terrain fields according to their textureID
/*
int CGraphic::CompareFields(const void * Elem1, const void * Elem2)
{
	FieldStruct * F1;
	FieldStruct * F2;
	
	F1 = (FieldStruct*) Elem1;
	F2 = (FieldStruct*) Elem2;
	
	if (F1->TexID < F2->TexID)
		return -1;
	else if (F1->TexID > F2->TexID)
		return 1;
	else
		return 0;
}
*/

// Finds bounding rect of car light
HRESULT	CGraphic::FindCarLightBoundingRect(CLight * Light, D3DXMATRIX WorldMat, RECT * Rect)
{
	D3DXVECTOR3 LightWorldPos;
	D3DXVECTOR3 LightWorldDirection;
	D3DXMATRIX LightViewMat;
	D3DXMATRIX InvLightMat;
	D3DXMATRIX LightProjMat;
	D3DXMATRIX InvLightProjMat;

	D3DXVECTOR3 LightBounds[8] = {D3DXVECTOR3(-1, -1, 0), D3DXVECTOR3(-1, 1, 0), D3DXVECTOR3(1, 1, 0), D3DXVECTOR3(1, -1, 0),
		D3DXVECTOR3(-1, -1, 1), D3DXVECTOR3(-1, 1, 1), D3DXVECTOR3(1, 1, 1), D3DXVECTOR3(1, -1, 1) };
	D3DXVECTOR2 Min = D3DXVECTOR2(FLT_MAX, FLT_MAX);
	D3DXVECTOR2 Max = D3DXVECTOR2(-FLT_MAX, -FLT_MAX);
	int i;

	if (Light->Type != LT_CARREFLECTOR)
		ERRORMSG(-1, "CGraphic::FindCarLightBoundingRect", "Light is not car reflector.");

	// Transform light pos and direction to world coordinates
	D3DXVec3TransformCoord(&LightWorldPos, &(Light->Position), &WorldMat);
	D3DXVec3TransformNormal(&LightWorldDirection, &(Light->Direction), &WorldMat);

	// Compute light view mat
	D3DXMatrixLookAtLH(&LightViewMat, &LightWorldPos, &(LightWorldPos + LightWorldDirection), &D3DXVECTOR3(0, 1, 0) );
	
	// Projection matrix computation
	D3DXMatrixPerspectiveFovLH(& LightProjMat, Light->Phi, 1, 1.0f, Light->Range);

	// Compute inverse matrix
	D3DXMatrixInverse(&InvLightMat, 0, & (LightViewMat * LightProjMat));
	
	// Transform bounding points into world space
	D3DXVec3TransformCoordArray(LightBounds, sizeof(D3DXVECTOR3), LightBounds, sizeof(D3DXVECTOR3), &InvLightMat, 8);

	// find min and max
	for (i = 0; i < 8; i++)
	{
		if (LightBounds[i].x < Min.x) Min.x = LightBounds[i].x;
		if (LightBounds[i].z < Min.y) Min.y = LightBounds[i].z;
		if (LightBounds[i].x > Max.x) Max.x = LightBounds[i].x;
		if (LightBounds[i].z > Max.y) Max.y = LightBounds[i].z;
	}

	Rect->left = (long) Min.x;
	Rect->right = (long) Max.x;
	Rect->top = (long) Min.y;
	Rect->bottom = (long) Max.y;

	return ERRNOERROR;
}

// Converts light into spot light structure
HRESULT	CGraphic::ConvertToShaderSpotLight(CLight * Light, D3DXMATRIX & WorldMat, SpotLightShaderStruct * SpotLight)
{
	D3DXVECTOR3 LightWorldPosition;
	D3DXVECTOR3 LightWorldDirection;
	
	// Colors
	SpotLight->Ambient = D3DXVECTOR4(Light->Ambient.r, Light->Ambient.g, Light->Ambient.b, Light->Ambient.a);
	SpotLight->Diffuse = D3DXVECTOR4(Light->Diffuse.r, Light->Diffuse.g, Light->Diffuse.b, Light->Diffuse.a);
	SpotLight->Specular = D3DXVECTOR4(Light->Specular.r, Light->Specular.g, Light->Specular.b, Light->Specular.a);
	
	// coords
	D3DXVec3TransformCoord(&LightWorldPosition, &(Light->Position), &WorldMat);
	D3DXVec3TransformNormal(&LightWorldDirection, &(Light->Direction), &WorldMat);

	SpotLight->Position = LightWorldPosition;
	SpotLight->Direction = LightWorldDirection;

	// atten
	SpotLight->Atten = D3DXVECTOR3(Light->Attenuation0, Light->Attenuation1, Light->Attenuation2);
	SpotLight->Angles = D3DXVECTOR3(cos(Light->Theta / 2), cos(Light->Phi / 2), Light->Falloff);
	SpotLight->Range = Light->Range;

	return ERRNOERROR;
}

HRESULT CGraphic::ConvertToShaderSpotLightVector(CLight * Light, D3DXMATRIX & WorldMat, SpotLightShaderStructVector * SpotLight)
{
	D3DXVECTOR3 LightWorldPosition;
	D3DXVECTOR3 LightWorldDirection;
	
	// Colors
	SpotLight->Ambient = D3DXVECTOR4(Light->Ambient.r, Light->Ambient.g, Light->Ambient.b, Light->Ambient.a);
	SpotLight->Diffuse = D3DXVECTOR4(Light->Diffuse.r, Light->Diffuse.g, Light->Diffuse.b, Light->Diffuse.a);
	SpotLight->Specular = D3DXVECTOR4(Light->Specular.r, Light->Specular.g, Light->Specular.b, Light->Specular.a);
	
	// coords
	D3DXVec3TransformCoord(&LightWorldPosition, &(Light->Position), &WorldMat);
	D3DXVec3TransformNormal(&LightWorldDirection, &(Light->Direction), &WorldMat);

	SpotLight->Position = D3DXVECTOR4(LightWorldPosition.x,LightWorldPosition.y, LightWorldPosition.z, 1) ;
	SpotLight->Direction = D3DXVECTOR4(LightWorldDirection.x,LightWorldDirection.y, LightWorldDirection.z, 0); 

	// atten
	SpotLight->Atten = D3DXVECTOR4(Light->Attenuation0, Light->Attenuation1, Light->Attenuation2, Light->Range);
	SpotLight->Angles = D3DXVECTOR4(cos(Light->Theta / 2), cos(Light->Phi / 2), Light->Falloff, 0);
	

	return ERRNOERROR;
}


// Converts rect into correct map position rect
void CGraphic::RectToMapPositionRect(RECT & Rect, RECT & MapPosRect)
{
	MapPosRect.left = max(0, Rect.left / (long) TERRAIN_PLATE_WIDTH);
	MapPosRect.right = min((long) Scene->Map.GetWidth() - 1, Rect.right / (long) TERRAIN_PLATE_WIDTH);
	MapPosRect.top = max(0, Rect.top / (long) TERRAIN_PLATE_WIDTH);
	MapPosRect.bottom = min((long) Scene->Map.GetDepth() - 1, Rect.bottom / (long) TERRAIN_PLATE_WIDTH);
}

// pair second lesser compare predicate
template <typename T, typename U>
bool CGraphic::PairSecondLesser(std::pair<T, U> & Pair1, std::pair<T, U> & Pair2)
{
	return Pair1.second < Pair2.second;
}

// Renders car light depth into shadow map
// Only set needed things and render - everything else must already be set
HRESULT CGraphic::RenderCarLightDepth(RECT Rect, CGrObjectMesh * Mesh, D3DVIEWPORT9 ShadowMapViewport, D3DXMATRIX & LightViewMat, D3DXMATRIX & LightProjMat )
{
	GROBJECTS_ITER			objIter;
	long x, z;
	CGrObjectTerrainPlate * Plate;
	vector<CGrObjectMesh *>::iterator MeshIt;
	static vector<CGrObjectMesh *> MeshesToRender;
	CGrObjectMesh * LastRenderedMesh;

	// set viewport
	D3DDevice->SetViewport(&ShadowMapViewport);

	// set matrixes
	Shader->SetViewMatrix( &(LightViewMat * ShadowMap->GetBiasMat()) );
	Shader->SetProjMatrix(& LightProjMat);

	// Render
	MeshesToRender.clear();
	for (x = Rect.left; x <= Rect.right; x++)
		for (z = Rect.top; z <= Rect.bottom; z++)
		{
			Plate = Scene->Map.GetPlate(x, z);
			for (MeshIt = Plate->GetObjects()->begin(); MeshIt != Plate->GetObjects()->end(); MeshIt++)
				MeshesToRender.push_back((*MeshIt) );
		}

	// sort objects to render
	sort(MeshesToRender.begin(), MeshesToRender.end() );

	LastRenderedMesh = 0;
	for (MeshIt = MeshesToRender.begin(); MeshIt != MeshesToRender.end(); MeshIt++)
		if ( (*MeshIt) != LastRenderedMesh && (*MeshIt) != Mesh && 
			(*MeshIt) != Mesh->GetWheel(0) &&
			(*MeshIt) != Mesh->GetWheel(1) &&
			(*MeshIt) != Mesh->GetWheel(2) &&
			(*MeshIt) != Mesh->GetWheel(3) )
			if ( ((*MeshIt)->IsStatic() && bRenderStaticObjects) || bRenderDynamicObjects)
			{
				D3DXVECTOR3 vec = (*MeshIt)->GetPosition();
				((CGrObjectMesh *) *MeshIt)->SetLOD( GetModelLODToUse( ((int) vec.x) / 1000, ((int) vec.z) / 1000 ) );
				(*MeshIt)->Render();
			}

	return ERRNOERROR;
}

// Loads checkpoint object into scene class
HRESULT	CGraphic::InitCheckpointObject()
{
	HRESULT Result;
	LPD3DXMESH TempMesh;
	LPD3DXBUFFER Adjacency;

	// Create sphere as checkpoint object
	Result = D3DXCreateSphere(D3DDevice, 50.0f, 15, 15, & TempMesh, &Adjacency);
	if (FAILED(Result) )
		ERRORMSG(Result, "CGraphic::InitCheckpointObject()", "Unable to create checkpoint sphere.");

	// Add normals
	Result = TempMesh->CloneMeshFVF(D3DXMESH_MANAGED, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, D3DDevice, & CheckpointObject);
	if (FAILED(Result) )
	{
		SAFE_RELEASE(TempMesh);
		SAFE_RELEASE(Adjacency);
		ERRORMSG(Result, "CGraphic::InitCheckpointObject()", "Unable to add normal into checkpoint sphere.");
	}

	SAFE_RELEASE(TempMesh);

	// compute normals
	Result = D3DXComputeNormals(CheckpointObject, (DWORD *) Adjacency->GetBufferPointer());
	if (FAILED(Result) )
	{
		SAFE_RELEASE(CheckpointObject);
		SAFE_RELEASE(Adjacency);
		ERRORMSG(Result, "CGraphic::InitCheckpointObject()", "Unable to compute normals for checkpoint sphere.");
	}

	SAFE_RELEASE(Adjacency);

	// Matrix
	D3DXMatrixIdentity(&CheckpointMatrix);
	// Rendering
	RenderCheckpoints = FALSE;
	// Material
	CheckpointMaterial.Diffuse.r = CheckpointMaterial.Diffuse.a = 1.0f;
	CheckpointMaterial.Diffuse.g = CheckpointMaterial.Diffuse.b = 0.0f;
	CheckpointMaterial.Ambient.r = 0.1f;
	CheckpointMaterial.Ambient.a = 1.0f;
	CheckpointMaterial.Ambient.g = CheckpointMaterial.Ambient.b = 0.0f;
	CheckpointMaterial.Specular.r = CheckpointMaterial.Specular.g = 0.0f;
	CheckpointMaterial.Specular.b = 0.0f;
	CheckpointMaterial.Specular.a = 1.0f;
	CheckpointMaterial.Power = 16.0f;
	// Light
	CheckpointLight.Direction = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
	CheckpointLight.Ambient = D3DXCOLOR(0.3f,0.3f, 0.3f, 1.0f);
	CheckpointLight.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	CheckpointLight.Specular = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);	

	return ERRNOERROR;
}

// Renders checkpoint object
HRESULT	 CGraphic::RenderCheckpointObject(BOOL RenderFar)
{
	int Range = iMapLODDefSize / 2;
	MAPPOSITION CheckMapPos;
	
	CheckMapPos.PosX = (int) (CheckpointMatrix._41 / 1000);
	CheckMapPos.PosZ = (int) (CheckpointMatrix._43 / 1000);

	if (CheckMapPos.PosX >= MapPosition.PosX - Range && CheckMapPos.PosX <= MapPosition.PosX + Range &&
		CheckMapPos.PosZ >= MapPosition.PosZ - Range && CheckMapPos.PosZ <= MapPosition.PosZ + Range)
	{
		if (RenderFar) return ERRNOERROR;
	}
	else if (!RenderFar)
	{
		return ERRNOERROR;
	}
	
	Shader->SetVertexShader(CheckpointVS);
	Shader->SetPixelShader(CheckpointPS);

	D3DDevice->SetTexture(1, 0);
	Shader->SetWorldMatrix(&CheckpointMatrix);
	Shader->SetVSMaterial(&CheckpointMaterial);
	CheckpointLight.Direction = Camera.GetLookAtVector() - Camera.GetEyePtVector();
	Shader->SetVSSingleDirLight(&CheckpointLight);

	// Draw checkpoint
	CheckpointObject->DrawSubset(0);

	return ERRNOERROR;
}

// Sets new checkpoint position
HRESULT	CGraphic::SetCheckpointPosition(D3DXVECTOR3 & Position)
{
	D3DXMatrixTranslation(&CheckpointMatrix, Position.x, Position.y, Position.z);
	return ERRNOERROR;
}

// Captures screen shot to the file
HRESULT	CGraphic::CaptureScreenShot(CString FileName)
{
	D3DXSaveSurfaceToFile(FileName, D3DXIFF_BMP, StandardRenderTarget, 0, 0);
	return ERRNOERROR;
}



//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
