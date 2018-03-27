/*
	Graphic:		Basic D3D graphic class, covers all rendering operations
					For first, you must call Init to set resolution and provide a valid pointer to ResourceManager object
					Before first scene rendering it is also needed to call Activate() method - this sets some variables
					and also starts (or restarts) counting frames.
	Creation Date:	14.10.2003
	Last Update:	2.3.2007
	Author:			Roman Margold


TODO:
Trochu spatne je to navrzeno, protoze enumerace rozliseni a zjisteni capabilit by se melo provest nekde zvlast davno pred tim, nez se vola Init a InitD3D,
takto totiz nelze pred tim zvolit rozliseni dle nabidnutych ani nejde provest jina volba... to musi byt nezavisle volani
Patrnì se tedy vymaže volání metody na nalezení rozlišení z Initu, protože je tam zbyteèné, i kontrola nastavení je zbyteèná, volání nastavení rozlišení totiž zahlásí
chybu a z té se vše dozvíme. Zjištìní rozlišení se udìlá asi static a bude možno jej volat i døíve...

Možná èasem vymizí ukazatel na ResourceManager, protože by se mìl využívt spíše jen v grafických objektech a ne pøímo v Graphic objektu
*/


#pragma once

// Define when you don't want to use D3DTADDRESS_BORDER in texture addressing mode
// Mainly while using Debug drivers of DirectX on ATI
// #define NO_BORDER_ADDRESS

#include "stdafx.h"
#include "..\Globals\GlobalsIncludes.h"
#include "..\ResourceManager\ResourceManager.h"
#include "..\GraphicObjects\GraphicObjectsIncludes.h"
#include "..\DXUtils\d3dfont.h"
#include "GraphicModes.h"
#include "FPSCounter.h"
#include "Camera.h"
#include "CShadowMap.h"
#include "Shaders.h"
#include "GUIDialog.h"
#include "Mouse.h"
#include "TimedTextOutput.h"
#include "HUDCore.h"
#include "RectInclusion.h"
#include "GeometryReducer.h"



// macros
#define COLORFORMAT16BIT	D3DFMT_R5G6B5
#define COLORFORMAT32BIT	D3DFMT_X8R8G8B8

#define MAX_TRANSPARENT_OBJECTS	1024



namespace graphic
{
	
	// shadow complexity constants
	#define SC_MINSHADOWCOMPLEXITY	0
	#define SC_MAXSHADOWCOMPLEXITY	3
	#define SC_NONE					0
	#define SC_FAST					1
	#define SC_SIMPLE				2
	#define SC_COMPLEX				3


	#define SM_MAX				2048 // Shadow map maximal resolution
	#define MAX_ANISOTROPY		16 // maximal allowed anisotropy value
	#define MAX_ANTIALIASING	16 // maximal allowed antialiasing value

	// defaults
	#define DEFAULT_LOD0TERRAINTEXTURESIZE	1024 

	// Vendor IDs
	#define NVIDIA_VENDOR_ID 0x10DE
	#define ATI_VENDOR_ID 0x1002

	//////////////////////////////////////////////////////////////////////////////////////////////
	//
	// typedefs and structures
	//
	//////////////////////////////////////////////////////////////////////////////////////////////
	
	typedef DWORD				FVFORMAT; // flexible vertex format

	
	// vertex of the large shadow polygon covering the entire viewport
	// FVF defined in Graphic.cpp
	struct SHADOWVERTEX
	{
		D3DXVECTOR4				pos;
		D3DXVECTOR2				coord;

	    static const DWORD		FVF;
	};

	
	// structure containing information needed for culling
	struct CULLINFO
    {
        D3DXVECTOR3				vecFrustum[8]; // corners of the view frustum
        D3DXPLANE				planeFrustum[6]; // planes of the view frustum
    };


	// a structure representing a box used for recursive computing terrain visibility
	struct __BOX
	{
		int				x1, x2, z1, z2;
		D3DXVECTOR4		transPoints[8];
	} ;
	

	// structure used for passing arguments into CGraphic::Init() method 
	struct GRAPHICINIT
	{
		HWND				hWnd;
		UINT				Width, Height;
		UINT				ColorDepth;
		BOOL				FullScreen;
		BOOL				LightGlows;
		CString				TerrainLOD; // specifies the filename of LODMap for terrain
		CString				ModelLOD; // specifies the filename of LODMap for models
		CString				GrassLOD; // specifies the filename of LODMap for grass
		UINT				LOD0TerrainTexWidth; // specifies width of the texture used for terrain LOD0 rendering
		UINT				LOD0TerrainTexHeight; // specifies Height of the texture used for terrain LOD0 rendering
		UINT				AnisotropicFiltering; // specifies how strong the anisotropy is; if set to 0, it's disabled at all
		UINT				Antialiasing; // specifies multisampling level; if set to 0, it's disabled at all
		UINT				MultisampleQuality; // specifies multisample quality level, when Antialiasing is set to 1=nonmaskable
		BOOL				WaitVSync; // the scene presentation waits for vertical retrace period
		resManNS::CResourceManager *	pResourceManager;
	} ;


	// structure used by CGraphic object for sorting transparent objects during rendering
	struct TRANSPARENTOBJECT
	{
		LPGROBJECTBASE		pObject; // pointer to the transparent object
		FLOAT				z; // z coordinate in viewer's space
	};




	//////////////////////////////////////////////////////////////////////////////////////////////
	//
	// main graphic class
	//
	//////////////////////////////////////////////////////////////////////////////////////////////
	class CGraphic
	{
		
		// TYPE DECLARATIONS
		typedef	std::list<CShadowVolume *>				SHADOWVOLUME;
		typedef	std::list<CShadowVolume *>::iterator	SHADOWVOLUME_ITER;

		struct LIGHTGLOWVERTEX
		{
			FLOAT				x, y, z;
			FLOAT				tu, tv; // Vertex texture coordinates

			static const DWORD	FVF;
		};



	public:
		//////////////////////////////////////////////////////////////////////////////////////////////
		//			P U B L I C       P A R T
		//////////////////////////////////////////////////////////////////////////////////////////////

		// PUBLIC VARIABLES
		CScene				*	Scene; // pointer to a scene, that'll be rendered with method RenderScene, this pointer must be filled with valid reference
		CHUDCore				HUDCore; // Core for HUD (Head-up display) 

		//TEST:
		CGrObjectGrass		*	pGrass;


		// CONSTRUCTORS & DESTRUCTOR
								CGraphic();
								~CGraphic();


		// PUBLIC METHODS

		// initializing methods
		inline CCamera	* GetCamera (void)	{ return &Camera;	}
		HRESULT					Init( GRAPHICINIT * gInitStruc );
		HRESULT					InitLightMaps();
		HRESULT					InitNormalMap();
		HRESULT					PerformTerrainOptimalization();
		HRESULT					FindSimilarRegions(vector<MAPPOSITION> * Rects);
		HRESULT					RegisterLights();
		void					SetProjectionParameters( FLOAT fieldOfViewY, FLOAT aspect, FLOAT zNear, FLOAT zFar );
		HRESULT					ChangeResolution( GRAPHICINIT * grInit );
		void					Free(); // Free all gameplay related resources


		void					SetResourceManager( resManNS::CResourceManager *pResMng ) { this->ResourceManager = pResMng; } ; // assigns the pointer to ResourceManager to this Graphic object
		void					SetScreenWidth( UINT width ) { uiScreenWidth = width; }; // sets the size of the visible screen (!NOT THE BACKBUFFER)
		void					SetScreenHeight( UINT height ) { uiScreenHeight = height; }; // sets the size of the visible screen (!NOT THE BACKBUFFER)
		void					SetScreenSize( UINT wid, UINT hei ) { uiScreenWidth = wid; uiScreenHeight = hei; }; // sets the size of the visible screen (!NOT THE BACKBUFFER)
		HRESULT					LoadMapLODDefinition( LPCTSTR fileName );
		HRESULT					LoadModelLODDefinition( LPCTSTR fileName );
		HRESULT					LoadGrassLODDefinition( LPCTSTR fileName );
		

		// state accessing methods
		void					SetShowingFPS( BOOL b ) { bShowingFPS = b; } ; // makes the FPS to be drawn
		void					Activate();
		void					Deactivate();
		void					SetShowingDebugMessages( BOOL b ) { bShowingDebugMsgs = b; };
		void					SetUseDynamicLights( BOOL b ) { bDynamicLights = b; };
		void					SetRenderTerrain( BOOL b ) { bRenderTerrain = b; };
		void					SetRenderFarObjects( BOOL b) { bRenderFarObjects = b; };
		void					SetRenderStaticObjects( BOOL b ) { bRenderStaticObjects = b; };
		void					SetRenderDynamicObjects( BOOL b ) { bRenderDynamicObjects = b; };
		void					SetRenderDirectionalLight( BOOL b ) { bRenderDirectionalLight = b; };
		void					SetRenderAmbientLight( BOOL b ) { bRenderAmbientLight = b; };
		void					SetRenderSkyBox( BOOL b ) { bRenderSkyBox = b; };
		void					SetMapDependentRendering( BOOL b ) { bMapDependentRendering = b; };
		void					SetRenderHUD(BOOL b) {bRenderHUD = b; };
		void					SetRenderEnabled( BOOL b) { bRenderEnabled = b; }; // Enable/disable whole rendering
		void					SetRenderLightGlows( BOOL b ) { bRenderLightGlows = b && bLightGlowsLoaded; };
		void					SetWireframe( BOOL b ) { bWireframeMode = b; D3DDevice->SetRenderState(D3DRS_FILLMODE, b ? D3DFILL_WIREFRAME : D3DFILL_SOLID); } // sets wireframe mode on/off
		void					SetRenderBoundingEnvelopes( BOOL b ) { bRenderBoundingEnvelopes = b; };
		void					SetRenderGrass( BOOL b ) { bRenderGrass = b; };
		void					SetRenderCheckpoints( BOOL b) { RenderCheckpoints = b; };

		BOOL					GetRenderEnabled( BOOL b ) const { return bRenderEnabled; };
		BOOL					GetWireframe() const { return bWireframeMode; }; // returns true when WireFrame mode is turned on


		// appearance
		void					SetShadowComplexity( UINT i ) { __SETTLE_BETWEEN( i, SC_MINSHADOWCOMPLEXITY, SC_MAXSHADOWCOMPLEXITY ); uiShadowComplexity = i; };
		void					SetGamma( float scale );
		void					SetGamma( int percent );
		void					SetGammaRGB( float red, float green, float blue );
		void					SetGammaRotate( float red, float green, float blue );
		void					SetCarPerPixelLight( BOOL b ) { bCarPerPixelLight = b; };
		void					SetAnisotropicFiltering( UINT i ) { uiAnisotropicFiltering = i; };
		void					SetFogColor( D3DXVECTOR3 & color ) { FogColor = color; };
		
		void					GetMultisampleType( D3DMULTISAMPLE_TYPE * type, DWORD * level );
		UINT					GetAnisotropicFiltering() { return uiAnisotropicFiltering; };
		UINT					GetAntialiasing() { return uiAntialiasing; };
		UINT					GetMultisampleQuality() { return uiMultisampleQuality; };
		D3DXVECTOR3				GetFogColor() const { return FogColor; };

		
		
		// font-accessing methods
		HRESULT					SetMsgFontSize( int size );
		HRESULT					SetDebugFontSize( int size );
		void					SetMsgFontColor( D3DCOLOR col ) { MsgFontColor = col; }; // sets the Msg font color
		void					SetDebugFontColor( D3DCOLOR col ) { DebugFontColor = col; pDebugTextOutput->SetColor( col ); }; // sets the Debug font color
		HRESULT					SetMsgFontName( LPCTSTR name );
		HRESULT					SetDebugFontName( LPCTSTR name );
		LPD3DXFONT				InitFont( D3DXFONT_DESC *desc );
		LPD3DXFONT				InitDefaultFont( int size, LPCTSTR name );
		
		void					SetMaxDebugMsgs( UINT count ) { pDebugTextOutput->SetMaxMessagesCount( count ); }; // sets the maximal count of debug messages displayed at the same time
		void					SetDebugMsgParams( int x1, int y1, int x2, int y2, DWORD format );
		void					AddDebugMessage( LPCTSTR text, D3DCOLOR col = 0, APPRUNTIME timeout = 0 );


		// camera-accessing methods
		void					SetLookAtVector( D3DXVECTOR3 vec );
		void					SetLookAtVector( FLOAT x, FLOAT y, FLOAT z );
		void					SetEyePtVector( D3DXVECTOR3 vec );
		void					SetEyePtVector( FLOAT x, FLOAT y, FLOAT z );
		void					SetUpVector( D3DXVECTOR3 vec );
		void					SetUpVector( FLOAT x, FLOAT y, FLOAT z );
		
		D3DXVECTOR3				GetEyePtVector() { return Camera.GetEyePtVector(); }; // retreives camera EyePoint vector
		D3DXVECTOR3				GetLookAtVector() { return Camera.GetLookAtVector(); }; // retreives camera LookAt vector
		D3DXVECTOR3				GetUpVector() { return Camera.GetUpVector(); }; // retreives camera Up vector
		D3DXMATRIX				GetViewMatrix() const { return ViewMatrix; }
		FLOAT					GetZNear() const { return fZNear; }
		FLOAT					GetZFar() const { return fZFar; }
		FLOAT					GetFieldOfViewY() const { return fFieldOfViewY; }
		FLOAT					GetFieldOfViewX() const { return fFieldOfViewY * fViewAspectRatioXY; }
		FLOAT					GetViewAspectRatioXY() const { return fViewAspectRatioXY; }

		void					MoveCamera( int Direction );
		void					RotateCamera( int Direction );
		void					RotateCameraM( int Direction, float Angle );
		void					SetCamera( CCamera *cam );
		void					GetCamera( CCamera *cam );


		// rendering methods
		HRESULT					RenderScene();
		HRESULT					RenderWithoutMap();
		HRESULT					RenderDialogOnly();


		// mouse operating methods
		HRESULT					LoadMouseCursor( LPCTSTR fileName, int * index );
		void					SetMouseCursor( int index );
		void					SetMouseCursor( CMouseCursor * pCursor );
		void					SetMouseVisible( BOOL visi = true ) { bDrawMouseCursor = visi; }; // sets the mouse (in)visible
		void					SetMousePositionX( int X ) { pMouse->SetPosX( X ); };
		void					SetMousePositionY( int Y ) { pMouse->SetPosY( Y ); };
		void					SetMousePositionXY( int X, int Y ) { pMouse->SetPosXY( X, Y ); };


		// map-accessing methods
		void					SetMapPosX( int x ); // sets the x-coordinate of the viewer in the map
		void					SetMapPosZ( int z );

		// Device validation and maintenance
		HRESULT					ValidateLostDevice();
		HRESULT					ResetDevice();
		HRESULT					OnLostDevice();
		HRESULT					OnResetDevice();



		// other staff
		void					GetDeviceCaps( D3DCAPS9 *pCaps = NULL );
		LPDIRECT3DDEVICE9		GetD3DDevice() const { return D3DDevice; }; // returns a pointer to Direct3DDevice9
		D3DFORMAT				GetAdapterColorFormat( UINT adapter = D3DADAPTER_DEFAULT );
		CShaders			*	GetShader() const { return Shader; } // return pointer to shader class
		IDirect3DTexture9	*	GetInvisibleTexture() const { return InvisibleTexture; };
		UINT					GetWidth() const { return uiRenderableWidth; };
		UINT					GetHeight() const { return uiRenderableHeight; };
		CBoundingEnvelopes	*	GetBoundingEnvelopesObj() { return &BoundingEnvelopes; }; // returns a pointer to BoundingEnvelopes object
		UINT					GetAvailableTextureMem() { return D3DDevice->GetAvailableTextureMem() / 1024; }; // returns the available texture memory in kB's
		HRESULT					CaptureScreenShot(CString FileName); // Captures screen shot to the file
		

		UINT					GetModelLODCount() { return uiModelLOD + 1; }; // returns count of LODs for models
		UINT					GetTerrainLODCount() { return uiTerrainLOD + 1; }; // returns count of LODs for terrain
		UINT					GetGrassLODCount() { return uiGrassLOD + 1; }; // returns count of LODs for grass

		void					SetDialog( CGUIDialog * pDialog ) { pGUIDialog = pDialog; }; // sets the pointer to active GUI dialog
		HRESULT					SetCheckpointPosition(D3DXVECTOR3 & Position); // Sets new checkpoint position



	private: 
		//////////////////////////////////////////////////////////////////////////////////////////////
		//			P R I V A T E       P A R T
		//////////////////////////////////////////////////////////////////////////////////////////////
		
		// PRIVATE VARIABLES
		
		// D3D vars
		HWND					hWnd; // handle GraphicWindow
		LPDIRECT3D9				Direct3D; // object Direct3D
		LPDIRECT3DDEVICE9		D3DDevice; // object Direct3DDevice
		D3DDEVTYPE				DeviceType;	// D3D device type, default is D3DDEVTYPE_HAL
		D3DCAPS9				D3DCaps; // stored device caps
		D3DPRESENT_PARAMETERS	D3DPP; // presentation parameters
		IDirect3DSurface9 *		DepthStencil; // Depth and stencil surface - minimal size 1024x1024
		D3DVIEWPORT9			StandardViewport;
		IDirect3DSurface9 *		StandardRenderTarget;
		D3DADAPTER_IDENTIFIER9  AdapterIdentifier;

		// screen & view settings
		D3DFORMAT				ColorFormat; // chosen color format
		D3DFORMAT				BackBufferFormat; // chosen format of the back buffer 
		D3DFORMAT				DepthStencilFormat; // chosen depth/stencil buffer format
		UINT					uiRenderableWidth, uiRenderableHeight; // size of the rendering back buffer
		UINT					uiScreenWidth, uiScreenHeight; // size of the visible screen - is filled by possibly resized backbuffer
		BOOL					bFullScreen; // true if full-screen app is used
		UINT					uiColorDepth; // 16 or 32 bits
		DWORD					dwVertexProcessing; // flags specifying vertex processing method (HW/SW/mixed)
		UINT					uiAdapter; // adapter handle (mostly D3DADAPTER_DEFAULT == 0)
		DWORD					dwDeviceBehaviorFlags; // flags used for device creation (D3D9->CreateDevice())
		float					fGammaFactor; // scaling factor for gamma ramp
		D3DXVECTOR3				FogColor;

		// quality settings
		BOOL					bHWVertexProcessing;	// HW support for Vertex Processing
		UINT					uiTerrainLOD; // specifies the maximum quality LOD used to render terrain plate - <0,TERRAIN_LOD_COUNT-1>
		UINT					uiModelLOD; // specifies the maximum quality LOD used to render models - <0,MODEL_LOD_COUNT-1>
		UINT					uiGrassLOD; // specifies the maximum quality LOD used to render grass - <0,MODEL_LOD_COUNT-1>
		UINT					uiShadowComplexity; // 0 = none; 1 = fast (sprite); 2 = simple (shadowmap); 3 = complex (shadow volume)
		UINT					uiLOD0TerrainTexWidth; // specifies width of the texture used for terrain LOD0 rendering
		UINT					uiLOD0TerrainTexHeight; // specifies Height of the texture used for terrain LOD0 rendering
		UINT					uiAnisotropicFiltering; // specifies how strong the anisotropy is; if set to 0, it's disabled at all
		UINT					uiAntialiasing; // specifies multisampling level; if set to 0, it's disabled at all
		UINT					uiMultisampleQuality; // specifies multisample quality level, when Antialiasing is set to 1=nonmaskable
		BOOL					bCarPerPixelLight; // enables per-pixel specular lighting computations for cars

		// font settings
		int						MsgFontSize; // font size that is used for displaying messages like FPS and so on
		int						DebugFontSize; // font size that is used for displaying debug messages like position
		D3DCOLOR				MsgFontColor; // font color that is used for displaying messages like FPS and so on
		D3DCOLOR				DebugFontColor; // font color that is used for displaying debug messages like position
		CString					MsgFontName; // msg font face name
		CString					DebugFontName; // msg font face name

		// rendering states 
		BOOL					bShowingFPS; // true to enable showing frame rate
		BOOL					bShowingDebugMsgs; // true to enable showing debug msgs
		BOOL					bRenderDirectionalLight; // renders the scene with directional light (Sun) and its shadows
		BOOL					bRenderAmbientLight; // renders the scene with ambient lights - without it, the scene is not visible where is no dynamic light
		BOOL					bRenderStaticObjects; // turns on rendering of static objects
		BOOL					bRenderDynamicObjects; // turns on rendering of dynamic objects
		BOOL					bDynamicLights; // turns on dynamic lights, without dynamic lights there are no complex shadows computed
		BOOL					bRenderTerrain; // turns on rendering of the terrain
		BOOL					bMapDependentRendering; // the rendering of objects and lights depends on their map position because their visibility is computed on the base of it
		BOOL					bRenderLightGlows; // turns on rendering glows of the lights
		BOOL					bRenderSkyBox; // turns on rendering of the SkyBox object (map's textured bounding box)
		BOOL					bRenderFarObjects; // turns on rendering of the far objects
		BOOL					bRenderEnabled; // turns on/off whole rendering
		BOOL					bDrawMouseCursor; // turns on displaying mouse cursor
		BOOL					bShaderSupported; // true, if vertex and pixel shader version >= 2.0
		BOOL					bWireframeMode; // when true, wireframe mode is initialized
 		BOOL					bRenderBoundingEnvelopes; // turns rendering of the bounding envelopes on and off
		BOOL					bRenderHUD; // turns on HUD rendering
		BOOL					bRenderGrass; // turns on rendering of grass
		BOOL					bWaitVSync; // the scene presentation waits for vertical retrace period

		// viewer
		CCamera					Camera; // object representing viewer poisition
		D3DXMATRIX				ViewMatrix; // camera view matrix
		D3DXMATRIX				ProjectionMatrix; // projective matrix
		FLOAT					fFieldOfViewY; // projection parameters
		FLOAT					fViewAspectRatioXY; // ---- || ----
		FLOAT					fZNear, fZFar; // ---- || ----
		BOOL					bViewMatrixNeedRecount; // true if any camera parameter was changed
		MAPPOSITION				MapPosition; // viewers position in the map

		// objects
		LPD3DXFONT				MsgFont; // font to write messages like FPS
		LPD3DXFONT				DebugFont; // font that is used to write debug messages
		CTimedTextOutput	*	pDebugTextOutput; // provides debug text output to the screen
		CGUIDialog			*	pGUIDialog; // pointer to a dialog class that is rendered in RenderScene method over the actual rendered scene
		CFPSCounter				FPSCounter; // framerate counter
		CGraphicModes			Modes16bpp, Modes32bpp; // stores 16 a 32bit display modes available on default adapter
		resManNS::CResourceManager	*	ResourceManager; // stored pointer to resource manager
		LPDIRECT3DVERTEXBUFFER9	pShadowPolygonVB; // polygon that covers entire
		SHADOWVOLUME			ShadowVolumes; // templated list of pointers to shadow volume objects
		resManNS::RESOURCEID	RIDLightGlowTexture; // resource ID of the texture used as lights glow
		LPDIRECT3DVERTEXBUFFER9 pLightGlowsVB; // vertex buffer containing the vertices of lamp-glows billboards
		resManNS::RESOURCEID	RIDLampGlowMesh; // mesh of the lamp's light glow
		CShadowMap			*	ShadowMap; // shadow map object
		CMouse				*	pMouse; // Mouse representing object
		resManNS::RESOURCEID	ridMapLODDefinition; // LOD definition for map rendering
		resManNS::RESOURCEID	ridModelLODDefinition; // LOD definition for models rendering
		resManNS::RESOURCEID	ridGrassLODDefinition; // LOD definition for grass rendering
		resManNS::LoDMap	*	pTerrainLoDMap; // this pointer to terrain LOD map is redundant, but useful (we don't need to ask for pointer for every single terrain plate)
		resManNS::LoDMap	*	pModelLODMap; // pointer to model LOD definition structure
		resManNS::LoDMap	*	pGrassLODMap; // pointer to grass LOD definition structure
		IDirect3DTexture9	*	pShadowTexture; // this texture defines shadowed regions when shadow volume is used
		CBoundingEnvelopes		BoundingEnvelopes;
		resManNS::RESOURCEID	ridBlackTexture; // texture used for ShadowMapping 
		
		// Checkpoint rendering
		LPD3DXMESH				CheckpointObject; // Checkpoint object
		D3DXMATRIX				CheckpointMatrix; // World matrix for check point
		D3DMATERIAL9			CheckpointMaterial; // Checkpoint material
		D3DLIGHT9				CheckpointLight; // Checkpoint light
		BOOL					RenderCheckpoints; // Whether render checkpoint object or not


		// SHADER STUFF
        CShaders			*	Shader; // Main shader class - provides shader setting interface
		vector<resManNS::RESOURCEID>	ShaderRIDs; // stored RIDs of shaders - without exact order, should be handled together
		UINT					DirLightVS; // Directional light vertex shader
		UINT					SpotLightVS;
		UINT					SimpleTexPS;
		UINT					DepthVS;
		UINT					DepthPS;
		UINT					LightMapVS;
		UINT					LightMapPS;
		UINT					LampShadowMapVS;
		UINT					LampShadowMapPS;
		UINT					LampShadowMapFilter22PS;
		UINT					TransformVS;
		UINT					TransformTexVS;
		UINT					ZeroPS;
		UINT					SingleColorPS; // handle of shader returning just the ambient color
		UINT					LampProjectiveMapPS;
		UINT					DepthTexPS;
		UINT					DepthTexShadowMapPS;
		UINT					TexPS;
		
		UINT					TerrainVS; // Basic terrain vertex shader
		UINT					TerrainPS; // Basic terrain pixel shader
		UINT					TerrainNoDirLightVS; // vertex shader for terrain enlighted only by ambient light
		UINT					TerrainWithTSMVS; // Terrain vertex shader with TSM
		UINT					TerrainWithTSMPS; // Terrain pixel shader with TSM
		UINT					TerrainWithHWTSMPS; // Terrain pixel shader with HW TSM
		UINT					TerrainSpotLightVS; // Terrain spot lightened
		UINT					TerrainSpotLightWithSMVS; // Terrain spot light vertex shader with shadow mapping
		UINT					TerrainWithSMPS; // Terrain pixel shader with shadow mapping
		UINT					TerrainWithHWSMPS; // Terrain pixel shader with hardware shadow mapping
		UINT					TerrainWithTSMLinearPCFPS; // Terrain with TSM and linear PCF pixel shader
		UINT					TerrainLOD0VS; // Terrain LOD0 vertex shader
		UINT					TerrainLOD0PS; // Terrain LOD0 pixel shader
		UINT					TerrainNightLOD0VS; // Terrain night LOD0 vertex shader
		UINT					TerrainNightLOD0PS; // Terrain night LOD0 pixel shader
		UINT					TerrainNightSMVS[4]; // Terrain night vertex shader shadow mapping
		UINT					TerrainNightSMPS[4]; // Terrain night pixel shader shadow mapping
		UINT					TerrainNightHWSMPS[4]; // Terrain night pixel shader hardware shadow mapping

		UINT					TerrainNightVS; // Terrain night vertex shader
		UINT					TerrainNightPS; // Terrain night pixel shader

		UINT					ObjectVS; // Basic object vertex shader
		UINT					ObjectPS; // Basic object pixel shader
		UINT					ObjectSpecularVS; // object vertex shader with per pixel lighting
		UINT					ObjectSpecularPS; // object pixel shader with per pixel lighting
		UINT					ObjectNoFogVS; // Basic object vertex shader
		UINT					ObjectNoDirLightVS; // vertex shader for objects enlightened only by ambient light
		UINT					ObjectWithTSMVS; // Object vertex shader with TSM
		UINT					ObjectWithTSMPS; // Object pixel shader with TSM
		UINT					ObjectWithHWTSMPS; // Object pixel shader with HW TSM
		UINT					ObjectWithTSMSpecularVS; // Object vertex shader with TSM and perpixel specular lighting
		UINT					ObjectWithHWTSMSpecularPS; // Object pixel shader with HW TSM and specular lighting
		UINT					ObjectSpotLightVS; // Object spot lighting
		UINT					ObjectSpotLightWithSMVS; // Object spot light with shadow mapping vertex shader
		UINT					ObjectWithSMPS; // Object with shadow mapping pixel shader
		UINT					ObjectWithHWSMPS; // Object with hardware shadow mapping pixel shader
		UINT					ObjectWithTSMLinearPCFPS; // Object with TSM and linear PCF pixel shader
		UINT					ObjectWithTSMLinearPCFSpecularPS; // Object pixel shader with TSM, linear PCF and specular lighting
		UINT					ObjectNightVS; // Object night vertex shader
		UINT					ObjectNightPS; // Object night pixel shader
		UINT					ObjectNightSMVS[4]; // Object night shadow mapping vertex shaders
		UINT					ObjectNightSMPS[4]; // Object night shadow mapping pixel shaders
		UINT					ObjectNightHWSMPS[4]; // Object night shadow mapping HW pixel shaders

 		UINT					TSMDepthVS; // TSM scene depth vertex shader
		UINT					TSMDepthPS; // TSM scene depth pixel shader

		UINT					DirLightWithTSMVS; // Directional light shader with TSM vertex shader
		UINT					DirLightWithTSMPS; // Directional light shader with TSM pixel shader

		UINT					SecondDepthVS; // Second depth vertex shader
		UINT					SecondDepthPS; // Second depth pixel shader

		UINT					NVidiaHWShadowMapPS; // NVidia hardware shadow mapping pixel shader
		UINT					NVidiaHWTSMPS; // NVidia hardware shadow mapping pixel shader for TSM

		UINT					SkySystemVS; // Sky system vertex shader
		UINT					SkySystemPS; // Sky system pixel shader
		UINT					StarsVS; // Star field vertex shader
		UINT					StarsPS; // Star field pixel shader
		UINT					CloudVS; // Cloud vertex shader
		UINT					CloudPS; // Cloud pixel shader
		UINT					MountainVS; // vertex shader for rendering background mountains
		UINT					MountainNightVS; // vertex shader for rendering background mountains during night - no fog
		UINT					MountainPS; // pixel shader for rendering background mountains
		UINT					MountainNightPS; // pixel shader for rendering background mountains during night - no fog

		UINT					SVCoveringPolygonPS; // PS for the shadowing polygon, polygon's color should be set in ambient light variable
		UINT					ShadowVolumeIncPS; // pixel shader that increases front shadow polygons count

		UINT					GrassVS; // vertex shader for rendering (animated) grass items
		UINT					GrassPS; // pixel shader for rendering grass items
		UINT					GrassNightPS; // pixel shader for rendering grass items during night

		UINT					BillboardVS; // vertex shader used for all transparent items (billboards for ex.)
		UINT					BillboardPS; // pixel shader used for all transparent items
		UINT					BillboardNightVS; // vertex shader used for all transparent items (billboards for ex.) during night
		UINT					BillboardNightPS; // pixel shader used for all transparent items during night
		
		UINT					TerrainLOD0TexturePS; // Terrain LOD 0 texture pixel shader
		UINT					TerrainLOD0TextureNightPS; // Terrain LOD 0 night texture pixel shader
		UINT					TerrainLightMapPS; // Light map terrain pixel shader

		// Normal map shaders
		UINT					NormalMapStage1VS; // Normal map floating point transformation vertex shader stage
		UINT					NormalMapStage1PS; // Normal map floating point pixel shader stage
		UINT					NormalMapStage2PS; // Normal map write to RGB texture stage pixel shader

		// Checkpoint shaders
		UINT					CheckpointVS; // Checkpoint vertex shader
		UINT					CheckpointPS; // Checkpoint pixel shader

		// Test shaders
		UINT					TerrainLOD0OptimalizationPS;
		UINT					TerrainLOD0DLPS;
		UINT					TerrainLOD0DLVS;


		// SHADER CONSTANTS HANDLES
		UINT					ShadowMatHandle;
		UINT					PostProjMatHandle;
		UINT					SpotLightsHandle;
		UINT					GrassMaxDepthHandle;
		UINT					GrassTimeHandle;
		UINT					GrassViewerPosHandle;
		UINT					ShadowMatsHandles[4];
		UINT					ClampVectorsHandles[4];
		UINT					MapWidthHandle;
		UINT					MapDepthHandle;
		UINT					ColorModifierHandle;
		UINT					FogColorHandle;

		// Clamp vectors used in vertex shader for clamping
		D3DXVECTOR4 ClampVectors[4];


		
		// TERRAIN TRICKS

		// Terrain optimalization
		IDirect3DTexture9 *		TerrainTexture; // Complete terrain low resolution texture
		IDirect3DVertexBuffer9 *TerrainVBuffer; // Complete terrain LOD 0 vertex buffer
		IDirect3DIndexBuffer9 * TerrainIBuffer; // Complete terrain LOD 0 index buffer
 		UINT					TerrainQuadNum; // Number of quads in terrain LOD 0
		D3DMATERIAL9			TerrainMaterial; // Terrain optimalization material
		UINT					uiTerrainVertexCount; // count of vertices of terrain LOD0 mesh
 		UINT					uiTerrainIndexCount; // count of indices of terrain LOD0 mesh

		bool				*	pLODBorders; /* 2D array of the size of pTerrainLoDMap which contains true on borders 
											  between 2 different LODs (true is on the field with the lower LOD) */

								// auxiliary rotating methods
		int						GetMapIndexAfterRotation( int basex, int basez, int width, int rot, int index );
		int						GetVertexIndexAfterPlateRotation( int platex, int platez, int width, int depth, int mapwidth, int rot, int index );
		void					RotateNormal( D3DXVECTOR3 * norm, int rot );

		// Terrain vertex
		static const DWORD		TerrainVertexFVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;

		struct TERRAINVERTEX
		{
			D3DXVECTOR3 Pos;
			D3DXVECTOR3 Normal;
			D3DXVECTOR2 TexCoord;
		};


		// terrain normalmap
		IDirect3DTexture9	*	NormalMapTexture;



		// excluded from light maps structure
		struct LightMapExclude
		{
			CLight * Light; // Lights, for which render of dynamic objects is needed
			IDirect3DTexture9 * Map; // Excluded light map
			std::vector< CGrObjectBase *> Object; // Objects to render with given light
		};

		std::vector<LightMapExclude> MapExclude; // Excluded light maps, because will be rendered with dynamic objects
		
		// Second depth stencil surface
		IDirect3DSurface9 * DepthStencil2;
		// Second depth stencil surface texture (if depth stencil texture available) 
		IDirect3DTexture9 * DepthTexture2;
		
		// Second render target used for rendering with shaders
		IDirect3DSurface9 * ShaderTarget;

		// Invisible texture
		IDirect3DTexture9 * InvisibleTexture;

		/*
		// for sorting fields based on texture
		struct FieldStruct
		{
			MAPPOSITION MapPos;
			RESOURCEID TexID;

		};
		FieldStruct * FieldOrder;
		static int CompareFields(const void * Elem1, const void * Elem2);
		*/


		// other variables
		BOOL					bLightGlowsLoaded; // says whether the light glows was loaded or wasn't (therefor dissalows to render it)
		int						iMapLODDefSize, iModelLODDefSize, iGrassLODDefSize; // width of the square defining LOD map - this is useful to find out areas outside of the higher LOD
		D3DXMATRIX				IdentityMatrix; // contains indentity matrix so it doesn't has to be filled every frame (or op) again
		CRectangleInclusion		RectangleInclusion;
		TRANSPARENTOBJECT		pTransparentObjects[MAX_TRANSPARENT_OBJECTS]; // arrays of pointers to transparent objects
		UINT					uiTransparentObjCount; // actual count of transparent objects stored in pTransparentObjects array



		/////////////////////////////////////////////////////////////////
		// PRIVATE METHODS
		
		// (de)initializing methods
		HRESULT					InitD3D(); // initializes D3D structures; called from Init after obtaining optional settings
		BOOL					CanUseHWVertexProcessing(); // detects (on the base of known D3DCaps), whether the HW vertex processing can be used
		BOOL					CheckSettings(); // returns true if chosen settings is supported by HW
		HRESULT					InitLightGlows(); // inits the vertex buffer for lamp-light-glows billboards and light-glow texture
		HRESULT					InitShadowPolygon(); // inits the shadow polygon vertex buffer
		HRESULT					InitShaders(); // Initializes shaders
		HRESULT					InitShadowMapping(); // Initializes shadow mapping
		HRESULT					InitTerrainOptimalization(); // Initializes terrain optimalization
		HRESULT					InitDepthStencils(); // Inits depth-stencil buffers
		HRESULT					LoadShader(const UINT ShaderType, const CString FileName, resManNS::ShaderLoadParams ShaderParams); // Load vertex or pixel shader from resource manager to shader class
		HRESULT					InitCheckpointObject(); // Loads checkpoint object into scene class
		
		void					ReleaseAllShaders();


		// "about rendering stuff"
		HRESULT					PrepareScene(); // prepare scene for rendering (restores lost devices and calls beginScene on device)
		HRESULT					PresentScene(); // presents rendered scene to front buffer
		void					SetStandardStates(); // sets predefined standard render and texture states

		void					RecountViewMatrix(); // recounts view matrix from camera parameters

		HRESULT					SetPixelShader( const UINT PixelShader );
		HRESULT					SetVertexShader( const UINT VertexShader );

		friend int __cdecl		CompareObjectsDepth( void * pObj, const void * i0, const void * i1 );


		// rendering methods
		void					DrawDebugMsgs();
		void					DrawFPS();
		//HRESULT					DrawShadowPolygon(); // renders the shadow polygon that covers the entire screen
		HRESULT					RenderTerrainField( int x, int z );
		HRESULT					RenderTerrainField( MAPPOSITION *pos );
		HRESULT					RenderTerrainField(int x, int z, BOOL firstPass); // with second pass option for covering terrain holes
		HRESULT					RenderTerrainSection( int x1, int z1, int x2, int z2 );
		HRESULT					RenderTerrainSection( int x, int z, int range, bool firstPass = true );
		HRESULT					RenderTerrainSection( MAPPOSITION *pos, int range );
		HRESULT					RenderDayScene();
		HRESULT					RenderAllObjects( MAPPOSITION *pos, int range, BOOL stat, BOOL dyn );
		HRESULT					RenderStaticObjects( MAPPOSITION *pos, int range );
		HRESULT					RenderStaticObjects( RECT & Rect, BOOL VisibleTest = TRUE );
		HRESULT					RenderDynamicObjects( MAPPOSITION *pos, int range, BOOL cars );
		HRESULT					RenderCarObjects( MAPPOSITION *pos, int range );
		HRESULT					RenderWithShadowMapping();
		HRESULT					RenderNightScene();
		HRESULT					RenderLightGlows();
		HRESULT					RenderTransparentObjects( BOOL day, BOOL RenderFar);
		HRESULT					RenderLamp(const std::vector<CGrObjectBase*> Object, CLight * Light);
		HRESULT					RenderDynamicShadowMapForLamp(const std::vector<CGrObjectBase*> Object, CLight * Light);
		HRESULT					RenderDynamicProjectedMapForLamp(const std::vector<CGrObjectBase*> Object, CLight * Light);
		HRESULT					RenderTerrainOptimalization(BOOL Day);
		HRESULT					RenderFarObjects(MAPPOSITION * pos, int range, BOOL bRenderStaticObjects, BOOL bRenderDynamicObjects, BOOL Day);
		HRESULT					RenderDepthSceneFromSun(D3DLIGHT9 * Light, D3DXMATRIX * LightViewMat, D3DXMATRIX * LightProjMat, D3DXMATRIX * TSMMatrix);
		HRESULT					RenderMapBorders( BOOL Day );
		HRESULT					ExcludeFromLightMapRendering(bool ExcludeInclude);
		HRESULT					RenderSkySystem( BOOL day );
		HRESULT					RenderShadowVolumes( int iWhichSide = 0, int iDynShadowsUsed = 0 );
		HRESULT					RenderBoundingEnvelopes();
		HRESULT					RenderGrass( BOOL Day );
		HRESULT					RenderGrassCycle( int radius );
		// Renders car light depth into shadow map
		HRESULT					RenderCarLightDepth(RECT Rect, CGrObjectMesh * Mesh, D3DVIEWPORT9 ShadowMapViewport, 
													D3DXMATRIX & LightViewMat, D3DXMATRIX & LightProjMat);
		HRESULT					RenderCheckpointObject(BOOL RenderFar); // Renders checkpoint object


		// other stuff
		UINT					GetTerrainLODToUse( int x, int z );
		UINT					GetModelLODToUse( int x, int z );
		UINT					GetGrassLODToUse( int x, int z );
		//void					ComputeTerrainVisibility( int x1, int z1, int x2, int z2 );
		BOOL					PointIsInViewFrustum( FLOAT x, FLOAT y, FLOAT z, FLOAT w );
		BOOL					PointIsInViewFrustum( D3DXVECTOR4 *vec );
		BOOL					PointIsInVisibleScreen( FLOAT x, FLOAT y, FLOAT z, FLOAT w );
		BOOL					CohenSutherlandCode( D3DXVECTOR4 *vert, BYTE &CScode );
		void					ComputeTerrainVisibility();
		// Finds bounding rect of light
		HRESULT					FindCarLightBoundingRect(CLight * Light, D3DXMATRIX WorldMat, RECT * Rect );
		// Converts light into spot light structure
		HRESULT					ConvertToShaderSpotLight(CLight * Light, D3DXMATRIX & WorldMat, SpotLightShaderStruct * SpotLight);
		HRESULT					ConvertToShaderSpotLightVector(CLight * Light, D3DXMATRIX & WorldMat, SpotLightShaderStructVector * SpotLight);

		// Converts rectangle to rectangle with map coordinates
		void RectToMapPositionRect(RECT & Rect, RECT & MapPosRect);

		// pair second lesser compare predicate
	template <typename T, typename U> static bool PairSecondLesser(std::pair<T, U> & Pair1, std::pair<T, U> & Pair2);


		// Light map generation type distinction structure
		struct TypeLightPos
		{
			// constructor
			TypeLightPos(resManNS::RESOURCEID _TerrainType, int _PosX, int _PosZ)
			{
				TerrainType = _TerrainType;
				PosX = _PosX;
				PosZ = _PosZ;
			};

			// operator !=
			bool operator!=(const TypeLightPos x)
			{
				if (this->TerrainType != x.TerrainType)
					return true;
				if (this->PosX != x.PosX)
					return true;
				if (this->PosZ != x.PosZ)
					return true;

				return false;
			};

			resManNS::RESOURCEID TerrainType;
			int PosX, PosZ;

			// operator<
			bool operator< (const TypeLightPos & y) const
			{
				
				if (this->TerrainType < y.TerrainType)
					return true;
				else if (this->TerrainType > y.TerrainType)
					return false;
				else if (this->PosX < y.PosX)
					return true;
				else if (this->PosX > y.PosX)
					return false;
				else if (this->PosZ < y.PosZ)
					return true;
				else 
					return false;
			};
		};

	} ;

	
	typedef CGraphic			*LPGRAPHIC;

} // end of namespace graphic
