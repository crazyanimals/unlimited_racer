#include "stdafx.h"
#include "NameValuePair.h"
#include "ErrorHandler.h"
#include "Configuration.h"

CConfig Configuration;

// constructor
CConfig::CConfig()
{
	AnisotropicFiltering = 0;
	Antialiasing = 0;
	BBoxesPath = "BBoxes\\";
	CarFileName = "";
	CarsPath = "Cars\\";
	ColorDepth = 32;
	CursorsPath = "Cursors\\";
	DataPath = "Data\\";
	DebugFontColor = 0xffffff00;
	DebugFontName = "Arial";
	DebugFontSize = 14;
	DebugLevel = 3;
	DemosPath = "Demos\\";
	DialogsPath = "Dialogs\\";
	DynamicLights = TRUE;
	EditorPath = "Editor\\";
	FontsPath = "Fonts\\";
	FullScreen = 1;
	FilesFormatPath = "Support\\FilesFormat\\";
	GraphicsPath = "";
	GrassComplexity = 3;
	GrassLODCount = 0;
	HoursOfDay = 12;
	HUD = "Default.HUD";
	HUDPath = "HUD\\";
	KeyAccelerate = VK_UP;
	KeyBackward = 's';
	KeyBrake = VK_SPACE;
	KeyChangeView = 'c';
	KeyDecelerate = VK_DOWN;
	KeyDriverView = 'v';
	KeyForward = 'w';
	KeyMenu = VK_F1;
	KeyMoveLeft = 'a';
	KeyMoveRight = 'd';
	KeyPause = 'p';
	KeyRestart = 'r';
	KeyShiftDown = VK_NUMPAD0;
	KeyShiftUp = VK_NUMPAD1;
	KeySteerLeft = VK_LEFT;
	KeySteerRight = VK_RIGHT;
	KeyTurnDown = VK_DOWN;
	KeyTurnLeft = VK_LEFT;
	KeyTurnRight = VK_RIGHT;
	KeyTurnUp = VK_UP;
	Laps = 3;
	LapsEnabled = TRUE;
	LightsPath = "Lights\\";
	LOD0TerrainTexHeight = 2048;
	LOD0TerrainTexWidth = 2048;
	LODDefinitionsPath = "LODDefinitions\\";
	MapDependentRendering = 1;
	MapFileName = "";
	MapsPath = "Maps\\";
	MinutesOfDay = 30;
	ModelComplexity = 3;
	ModelLODCount = 0;
	ModelsPath = "Models\\";
	MsgFontColor = 0xffff0000;
	MsgFontName = "Courier";
	MsgFontSize = 16;
	MultisampleQuality = 0;
	MusicPath = "Music\\";
	MusicVolume = 1.0;
	netPort = 58787;
	netBroadPort= 58788;
	NoRelativePaths = 0;
	ObjectsPath = "Objects\\";
	PhTexturesPath = "Physics\\";
	PlayerName = "Unknown";
	RenderAmbientLight = TRUE;
	RenderBoundingEnvelopes = FALSE;
	RenderDirectionalLight = TRUE;
	RenderDynamicObjects = FALSE;
	RenderFarObjects = TRUE;
	RenderGrass = FALSE;
	RenderHUD = TRUE;
	RenderLightGlows = FALSE;
	RenderSky = TRUE;
	RenderStaticObjects = TRUE;
	RenderTerrain = TRUE;
	ScreenShotNumber = 0;
	ShadersPath = "Shaders\\";
	ShadowComplexity = 2;
	ShowDebugMsgs = FALSE;
	ShowFPS = FALSE;
	SkySystemPath = "SkySystem\\";
	SoundsPath = "Sounds\\";
	SpritesPath = "Sprites\\";
	TerrainComplexity = 3;
	TerrainLODCount = 0;
	TextureSetPath = "TextureSet\\";
	TerrainsPath = "Terrains\\";
	TexturesPath = "Textures\\";
	MutatorsPath = "Mutators\\";
	Time = 0;
	TimeEnabled = 0;
	WindowHeight = 768;
	WindowWidth = 1024;
	WireFrameMode = FALSE;
	GammaFactor = 1.0f;
	CarPerPixelLight = TRUE;
	TextureQuality = 1;
	WaitVSync = FALSE;
	SolverModel = 1;
	FrictionModel = 1;
	RoadMaterialIndex = 0;
	TerrainMaterialIndex = 0;
	RoadSpeedCoef = 0.0f;
	TerrainSpeedCoef = 0.0f;
	CarSpeedLimit;
    UpdateTimeClip              = 1.5f;
    CarMaxSpeed                 = 35.0f;
    CarSpeedTreshold            = 0.2f;
    CarSpeedChangeProbability   = 0.1f;
    CarSpeedChange              = 0.1f;
    MassAvoidanceCoef           = 10.0f;
    LeadTime                    = 1.5f;
    CarTimeBeforeRestart        = 5;
    CarTrackProgress            = 10.0f;
    LeakThrough                 = 1.1f;
    DistanceTolerance           = 5.0f;
    MaxWaypointsMissed          = 1;
    MapToPathLocalDistance      = 10.0f;
    DefaultRadius               = 2.0f;  
    OptimizeWaypointPlacement   = TRUE;
    MaxWaypointDistance         = 5.0f;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function, calls the nonstatic version that could access object's properties
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK CConfig::LoadCallBackStatic( NAMEVALUEPAIR * nameValue){
	CConfig *res; 
	res = (CConfig *) nameValue->ReservedPointer;
	res->LoadCallBack( nameValue );
}



//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function, stores one configuration pair to any member
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CConfig::LoadCallBack( NAMEVALUEPAIR * pPair ){

	CString		name = pPair->GetName();
	
	name.MakeUpper();

	// temporary configuration
	if (name == _T("TEST_VALUE_0") )		{ SolverModel = pPair->GetInt(); return; }
	if (name == _T("TEST_VALUE_1") )		{ FrictionModel = pPair->GetInt(); return; }
	
	// paths configuration
	if ( name == _T("DATAPATH") )			{ DataPath = pPair->GetString(); return; }
	if ( name == _T("GRAPHICSPATH") )		{ GraphicsPath = pPair->GetString(); return; }
	if ( name == _T("TEXTURESPATH") )		{ TexturesPath = pPair->GetString(); return; }
	if ( name == _T("SPRITESPATH") )		{ SpritesPath = pPair->GetString(); return; }
	if ( name == _T("MODELSPATH") )			{ ModelsPath = pPair->GetString(); return; }
	if ( name == _T("TERRAINSPATH") )		{ TerrainsPath = pPair->GetString(); return; }
	if ( name == _T("DIALOGSPATH") )		{ DialogsPath = pPair->GetString(); return; }
	if ( name == _T("FONTSPATH") )			{ FontsPath = pPair->GetString(); return; }
	if ( name == _T("CURSORSPATH") )		{ CursorsPath = pPair->GetString(); return; }
	if ( name == _T("LODDEFINITIONSPATH") )	{ LODDefinitionsPath = pPair->GetString(); return; }
	if ( name == _T("SHADERSPATH") )		{ ShadersPath = pPair->GetString(); return; }
	if ( name == _T("MAPSPATH") )			{ MapsPath = pPair->GetString(); return; }
	if ( name == _T("OBJECTSPATH") )		{ ObjectsPath = pPair->GetString();CarsPath=pPair->GetString(); return; } 
	if ( name == _T("LIGHTSPATH") )			{ LightsPath = pPair->GetString(); return; }
	if ( name == _T("EDITORPATH") )			{ EditorPath = pPair->GetString(); return; }
	if ( name == _T("PHTEXTURESPATH") )		{ PhTexturesPath = pPair->GetString(); return; }
	if ( name == _T("DEMOSPATH") )			{ DemosPath = pPair->GetString(); return; }
	if ( name == _T("SKYSYSTEMPATH") )		{ SkySystemPath = pPair->GetString(); return; }
	if ( name == _T("HUDPATH") )			{ HUDPath = pPair->GetString(); return; }
	if ( name == _T("MUSICPATH") )			{ MusicPath = pPair->GetString(); return; }
	if ( name == _T("TEXTURESETPATH"))      { TextureSetPath = pPair->GetString(); return; }
	if ( name == _T("SOUNDPATH"))			{ SoundsPath = pPair->GetString(); return; }
	if ( name == _T("FILESFORMATPATH"))		{ FilesFormatPath =  pPair->GetString(); return; }
	if ( name == _T("NORELATIVEPATHS") )	{ NoRelativePaths = pPair->GetInt(); return; }
	

	// graphic settings config.
	if ( name == _T("SHOWFPS") )			{ ShowFPS = pPair->GetInt(); return; }
	if ( name == _T("WINDOWWIDTH") )		{ WindowWidth = pPair->GetInt(); return; }
	if ( name == _T("WINDOWHEIGHT") )		{ WindowHeight = pPair->GetInt(); return; }
	if ( name == _T("FULLSCREEN") )			{ FullScreen = pPair->GetInt(); return; }
	if ( name == _T("COLORDEPTH") )			{ ColorDepth = pPair->GetInt(); return; }
	if ( name == _T("TERRAINCOMPLEXITY") )	{ TerrainComplexity = pPair->GetInt(); return; }
	if ( name == _T("MODELCOMPLEXITY") )	{ ModelComplexity = pPair->GetInt(); return; }
	if ( name == _T("GRASSCOMPLEXITY") )	{ GrassComplexity = pPair->GetInt(); return; }
	if ( name == _T("SHOWDEBUGMESSAGES") )	{ ShowDebugMsgs = pPair->GetInt(); return; }
	if ( name == _T("FONTSIZE") )			{ MsgFontSize = pPair->GetInt(); return; }
	if ( name == _T("DEBUGFONTSIZE") )		{ DebugFontSize = pPair->GetInt(); return; }
	if ( name == _T("FONTNAME") )			{ MsgFontName = pPair->GetString(); return; }
	if ( name == _T("DEBUGFONTNAME") )		{ DebugFontName = pPair->GetString(); return; }
	if ( name == _T("FONTCOLOR") )			{ MsgFontColor = (D3DCOLOR) pPair->GetInt(); return; }
	if ( name == _T("DEBUGFONTCOLOR") )		{ DebugFontColor = (D3DCOLOR) pPair->GetInt(); return; }
	if ( name == _T("RENDERDIRECTIONALLIGHT") )	{ RenderDirectionalLight = pPair->GetInt(); return; }
	if ( name == _T("RENDERAMBIENTLIGHT") )	{ RenderAmbientLight = pPair->GetInt(); return; }
	if ( name == _T("RENDERLIGHTGLOWS") )	{ RenderLightGlows = pPair->GetInt(); return; }
	if ( name == _T("SHADOWCOMPLEXITY") )	{ ShadowComplexity = pPair->GetInt(); return; }
	if ( name == _T("RENDERSTATICOBJECTS") )	{ RenderStaticObjects = pPair->GetInt(); return; }
	if ( name == _T("RENDERDYNAMICOBJECTS") )	{ RenderDynamicObjects = pPair->GetInt(); return; }
	if ( name == _T("DYNAMICLIGHTS") )		{ DynamicLights = pPair->GetInt(); return; }
	if ( name == _T("RENDERTERRAIN") )		{ RenderTerrain = pPair->GetInt(); return; }
	if ( name == _T("RENDERSKY") )			{ RenderSky = pPair->GetInt(); return; }
	if ( name == _T("MAPDEPENDENTRENDERING") )	{ MapDependentRendering = pPair->GetInt(); return; }
	if ( name == _T("WIREFRAMEMODE") )		{ WireFrameMode = pPair->GetInt(); return; }
	if ( name == _T("RENDERBOUNDINGENVELOPES") )	{ RenderBoundingEnvelopes = pPair->GetInt(); return; }
	if ( name == _T("RENDERFAROBJECTS") )		{ RenderFarObjects = pPair->GetInt(); return; }
	if ( name == _T("RENDERHUD") )			{ RenderHUD = pPair->GetInt(); return; }
	if ( name == _T("RENDERGRASS") )		{ RenderGrass = pPair->GetInt(); return; }
	if ( name == _T("WAITVSYNC") )			{ WaitVSync = pPair->GetInt(); return; }
	

	if ( name == _T("LOD0TERRAINTEXWIDTH") )	{ LOD0TerrainTexWidth = pPair->GetInt(); return; }
	if ( name == _T("LOD0TERRAINTEXHEIGHT") )	{ LOD0TerrainTexHeight = pPair->GetInt(); return; }
	if ( name == _T("ANISOTROPICFILTERING") )	{ AnisotropicFiltering = pPair->GetInt(); return; }
	if ( name == _T("ANTIALIASING") )			{ Antialiasing = pPair->GetInt(); return; }
	if ( name == _T("MULTISAMPLEQUALITY") )		{ MultisampleQuality = pPair->GetInt(); return; }
	if ( name == _T("GAMMAFACTOR") )			{ GammaFactor = pPair->GetFloat(); return; }
	if ( name == _T("CARPERPIXELLIGHT") )		{ CarPerPixelLight = pPair->GetInt(); return; }
	if ( name == _T("TEXTUREQUALITY") )			{ TextureQuality = pPair->GetInt(); return; }

	// network setting
	if ( name == _T("NTPORT")			)	{ netPort = pPair->GetInt(); return; }
	if ( name == _T("NTBROADPORT")		)	{ netBroadPort = pPair->GetInt(); return; }
	// HUD setting
	if ( name == _T("HUD"))					{ HUD = pPair->GetString(); return; }

	// Sound settings
	if ( name == _T("MUSICVOLUME") )		{ MusicVolume = pPair->GetFloat(); return; }

	// physics configuration
	if (name == _T("SOLVERMODEL") )			{ SolverModel = pPair->GetInt(); return; }
	if (name == _T("FRICTIONMODEL") )		{ FrictionModel = pPair->GetInt(); return; }
	if (name == _T("ROADMATERIALINDEX") )	{ RoadMaterialIndex = pPair->GetInt(); return; }
	if (name == _T("TERRAINMATERIALINDEX") ){ TerrainMaterialIndex = pPair->GetInt(); return; }
	if (name == _T("ROADSPEEDCOEF") )		{ RoadSpeedCoef = pPair->GetFloat(); return; }
	if (name == _T("TERRAINSPEEDCOEF") )	{ TerrainSpeedCoef = pPair->GetFloat(); return; }
	if (name == _T("CARSPEEDLIMIT") )		{ CarSpeedLimit = pPair->GetFloat(); return; }
	
	// screen shot number
	if (name == _T("SCREENSHOTNUMBER") )	{ScreenShotNumber = (UINT) max(0,pPair->GetInt()); return; }

	// key binding config.
	if ( name.Left(3) == _T("KEY") )
	{
		if ( name == _T("KEYFORWARD") )		KeyForward = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYBACKWARD") )	KeyBackward = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYMOVELEFT") )	KeyMoveLeft = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYMOVERIGHT") )	KeyMoveRight = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYTURNLEFT") )	KeyTurnLeft = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYTURNRIGHT") )	KeyTurnRight = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYTURNUP") )		KeyTurnUp = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYTURNDOWN") )	KeyTurnDown = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYMENU") )		KeyMenu = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYACCELERATE") )  KeyAccelerate = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYDECELERATE") )  KeyDecelerate = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYSTEERLEFT") )   KeySteerLeft = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYSTEERRIGHT") )  KeySteerRight = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYBRAKE") )		KeyBrake = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYSHIFTUP") )		KeyShiftUp = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYSHIFTDOWN") )	KeyShiftDown = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYCHANGEVIEW") )  KeyChangeView = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYDRIVERVIEW") )  KeyDriverView = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYRESTART") )		KeyRestart = ParseKeyBindConfigPair( pPair->GetString() );
		if ( name == _T("KEYPAUSE") )		KeyPause = ParseKeyBindConfigPair( pPair->GetString() );
		return;
	}
// AI: configuration
    if ( name == _T("UPDATETIMECLIP"           )) UpdateTimeClip = pPair->GetFloat();
    if ( name == _T("CARMAXSPEED"              )) CarMaxSpeed    = pPair->GetFloat();
    if ( name == _T("CARSPEEDTRESHOLD"         )) CarSpeedTreshold = pPair->GetFloat();
    if ( name == _T("CARSPEEDCHANGEPROBABILTY" )) CarSpeedChangeProbability = pPair->GetFloat();
    if ( name == _T("CARSPEEDCHANGE"           )) CarSpeedChange = pPair->GetFloat();
    if ( name == _T("MASSAVOIDANCECOEF"        )) MassAvoidanceCoef = pPair->GetFloat();
    if ( name == _T("LEADTIME"                 )) LeadTime = pPair->GetFloat();
    if ( name == _T("CARTIMEBEFORERESTART"     )) CarTimeBeforeRestart = pPair->GetInt();
    if ( name == _T("CARTRACKPROGRESS"         )) CarTrackProgress = pPair->GetFloat();
    if ( name == _T("LEAKTHROUGH"              )) LeakThrough = pPair->GetFloat();
    if ( name == _T("DISTANCETOLERANCE"        )) DistanceTolerance = pPair->GetFloat();
    if ( name == _T("MAXWAYPOINTSMISSED"       )) MaxWaypointsMissed = pPair->GetInt();
    if ( name == _T("MAPTOPATHLOCALDISTANCE"   )) MapToPathLocalDistance = pPair->GetFloat();
    if ( name == _T("DEFAULTRADIUS"            )) DefaultRadius = pPair->GetFloat(); 
    if ( name == _T("OPTIMIZEWAYPOINTPLACEMENT")) OptimizeWaypointPlacement = (pPair->GetInt() != 0);
    if ( name == _T("MAXWAYPOINTDISTANCE"      )) MaxWaypointDistance = pPair->GetFloat(); 
// AI: sanity check
    if (UpdateTimeClip          <= 0  ) UpdateTimeClip          = 1.5f;
    if (CarMaxSpeed             <= 0.0) CarMaxSpeed             = 0;
    if (CarSpeedTreshold        >  1.0) CarSpeedTreshold        = 1.0f;
    if (CarSpeedChange          >  1.0) CarSpeedChange          = 1;
    if (LeadTime                <= 0.2) LeadTime                = 0.2f;
    if (CarTimeBeforeRestart    <  1  ) CarTimeBeforeRestart    = 1;
    if (DistanceTolerance       <  0.5) DistanceTolerance       = 0.5f;
    if (MaxWaypointsMissed      <  0  ) MaxWaypointsMissed      = 0;
    if (MapToPathLocalDistance  <  1.0) MapToPathLocalDistance  = 1.0f;
    if (DefaultRadius           <  0.5) DefaultRadius           = 0.5f;
    if (MaxWaypointDistance     <  1.0) MaxWaypointDistance     = 1.0f;


	// other configuration
	if ( name == _T("DEFAULTMAP") )		{ MapFileName = pPair->GetString(); return; }
	if ( name == _T("DEFAULTCAR") )		{ CarFileName = pPair->GetString(); return; } 
	
	int i;
	CString Temp;
	CString Opponent = CString("OPPONENT");
	CString Car = CString("CAR");
	char Number[10];
	for (i = 0; i < MAX_OPPONENTS; i++)
	{
		Temp = Opponent + CString(_itoa(i, Number, 10) ) + Car;
		if ( name == Temp) { OpponentsFileNames[i] = pPair->GetString(); return; }
	}		

	if ( name == _T("PLAYERNAME") )		{ PlayerName = pPair->GetString(); return; }
	if ( name == _T("HOURSOFDAY") )		{ HoursOfDay = pPair->GetInt(); 
										  if (HoursOfDay < 0) HoursOfDay = 0; 
										  if (HoursOfDay > 23) HoursOfDay = 23; return; }
	if ( name == _T("MINUTESOFDAY") )   { MinutesOfDay = pPair->GetInt();
										  if (MinutesOfDay < 0) MinutesOfDay = 0;
										  if (MinutesOfDay > 59) MinutesOfDay = 59;
										  return; }
	if ( name == _T("LAPS") )			{ Laps = pPair->GetInt(); 
										  if (Laps < 0) Laps = 0;
										  if (Laps > 99) Laps = 99; return; }
	if ( name == _T("TIME") )			{ Time = pPair->GetInt();
										  if (Time < 0) Time = 0;
										  if (Time > 999) Time = 999; return; }
	if ( name == _T("LAPSENABLED") )		{ LapsEnabled = pPair->GetInt(); return; }
	if ( name == _T("TIMEENABLED") )	{ TimeEnabled = pPair->GetInt(); return; }

	if ( name == _T("DEBUGLEVEL") )		{ DebugLevel = pPair->GetInt(); return; }
}




//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads configuration used in this structure from specified file
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CConfig::Load( CStringA fileName ){
	
	return  NAMEVALUEPAIR::Load( fileName, LoadCallBackStatic, this );	
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns a key's code represented by the given string (that is configuration alias)
//
//////////////////////////////////////////////////////////////////////////////////////////////
int CConfig::ParseKeyBindConfigPair( CString key )
{
	if ( key.GetLength() == 0 ) return 27654; // no key selected, so undefine this binding
	if ( key == _T("UP ARROW") ) return VK_UP;
	if ( key == _T("DOWN ARROW") ) return VK_DOWN;
	if ( key == _T("LEFT ARROW") ) return VK_LEFT;
	if ( key == _T("RIGHT ARROW") ) return VK_RIGHT;
	if ( key == _T("PAGE UP") ) return VK_PRIOR;
	if ( key == _T("PAGE DOWN") ) return VK_NEXT;
	if ( key == _T("INSERT") ) return VK_INSERT;
	if ( key == _T("DELETE") ) return VK_DELETE;
	if ( key == _T("SPACE") ) return VK_SPACE;
	if ( key == _T("END") ) return VK_END;
	if ( key == _T("HOME") ) return VK_HOME;
	if ( key == _T("NUMPAD0") ) return VK_NUMPAD0;
	if ( key == _T("NUMPAD1") ) return VK_NUMPAD1;
	if ( key == _T("NUMPAD2") ) return VK_NUMPAD2;
	if ( key == _T("NUMPAD3") ) return VK_NUMPAD3;
	if ( key == _T("NUMPAD4") ) return VK_NUMPAD4;
	if ( key == _T("NUMPAD5") ) return VK_NUMPAD5;
	if ( key == _T("NUMPAD6") ) return VK_NUMPAD6;
	if ( key == _T("NUMPAD7") ) return VK_NUMPAD7;
	if ( key == _T("NUMPAD8") ) return VK_NUMPAD8;
	if ( key == _T("NUMPAD9") ) return VK_NUMPAD9;
	if ( key == _T("NUMPAD*") ) return VK_MULTIPLY;
	if ( key == _T("NUMPAD+") ) return VK_ADD;
	if ( key == _T("NUMPAD-") ) return VK_SUBTRACT;
	if ( key == _T("NUMPAD.") ) return VK_DECIMAL;
	if ( key == _T("NUMPAD/") ) return VK_DIVIDE;
	if ( key == _T("F1") ) return VK_F1;
	if ( key == _T("F2") ) return VK_F2;
	if ( key == _T("F3") ) return VK_F3;
	if ( key == _T("F4") ) return VK_F4;
	if ( key == _T("F5") ) return VK_F5;
	if ( key == _T("F6") ) return VK_F6;
	if ( key == _T("F7") ) return VK_F7;
	if ( key == _T("F8") ) return VK_F8;
	if ( key == _T("F9") ) return VK_F9;
	if ( key == _T("F10") ) return VK_F10;
	if ( key == _T("F11") ) return VK_F11;
	if ( key == _T("F12") ) return VK_F12;
	if ( key == _T("F13") ) return VK_F13;
	if ( key == _T("F14") ) return VK_F14;
	if ( key == _T("F15") ) return VK_F15;
	if ( key == _T("F16") ) return VK_F16;
	if ( key == _T("F17") ) return VK_F17;
	if ( key == _T("F18") ) return VK_F18;
	if ( key == _T("F19") ) return VK_F19;
	if ( key == _T("F20") ) return VK_F20;
	if ( key == _T("F21") ) return VK_F21;
	if ( key == _T("F22") ) return VK_F22;
	if ( key == _T("F23") ) return VK_F23;
	if ( key == _T("F24") ) return VK_F24;
	if ( key == _T("BACKSPACE") ) return VK_BACK;
	if ( key == _T("CAPS LOCK") ) return VK_CAPITAL;
	if ( key == _T("SEPARATOR") ) return VK_SEPARATOR;
	if ( key == _T("NUM LOCK") ) return VK_NUMLOCK;
	if ( key == _T(";") ) return VK_OEM_1;
	if ( key == _T("+") ) return VK_OEM_PLUS;
	if ( key == _T(",") ) return VK_OEM_COMMA;
	if ( key == _T("-") ) return VK_OEM_MINUS;
	if ( key == _T(".") ) return VK_OEM_PERIOD;
	if ( key == _T("/") ) return VK_OEM_2;
	if ( key == _T("'") ) return VK_OEM_3;
	if ( key == _T("[") ) return VK_OEM_4;
	if ( key == _T("\\")) return VK_OEM_5;
	if ( key == _T("]" ) ) return VK_OEM_6;
	if ( key == _T("'") ) return VK_OEM_7;
	// alphabet + numbers
	if ( key.GetLength() == 1 ) return key.GetAt(0);

	return 27654; // unsupported key selected, so undefine this binding
}


//////////////////////////////////////////////////////////////////////////////////
//
// Translates from key codes to strings
//
//////////////////////////////////////////////////////////////////////////////////
CAtlString CConfig::UnParseKeyBindConfigPair( int key )
{
	CAtlString Temp = "";

	switch (key)
	{
	case VK_BACK:
		Temp = "BACKSPACE";
		break;
	//case VK_TAB:
	// !!! CAUTION !!! DO NOT DELETE
	//	Temp = "TAB";
	//	break;
	case VK_RETURN:
		Temp = "ENTER";
		break;
	case VK_CAPITAL:
		Temp = "CAPS LOCK";
		break;
	case VK_SPACE:
		Temp = "SPACE";
		break;
	case VK_PRIOR:
		Temp = "PAGE UP";
		break;
	case VK_NEXT:
		Temp = "PAGE DOWN";
		break;
	case VK_END:
		Temp = "END";
		break;
	case VK_HOME:
		Temp = "HOME";
		break;
	case VK_LEFT:
		Temp = "LEFT ARROW";
		break;
	case VK_UP:
		Temp = "UP ARROW";
		break;
	case VK_RIGHT:
		Temp = "RIGHT ARROW";
		break;
	case VK_DOWN:
		Temp = "DOWN ARROW";
		break;
	case VK_INSERT:
		Temp = "INSERT";
		break;
	case VK_DELETE:
		Temp = "DELETE";
		break;
	case VK_MULTIPLY:
		Temp = "NUMPAD*";
		break;
	case VK_ADD:
		Temp = "NUMPAD+";
		break;
	case VK_SEPARATOR:
		Temp = "SEPARATOR";
		break;
	case VK_SUBTRACT:
		Temp = "NUMPAD-";
		break;
	case VK_DECIMAL:
		Temp = "NUMPAD.";
		break;
	case VK_DIVIDE:
		Temp = "NUMPAD/";
		break;
	case VK_NUMLOCK:
		Temp = "NUM LOCK";
		break;
	//case VK_SCROLL:
	// !!! CAUTION !!! - DO NOT DELETE
	//	Temp = "SCROLL LOCK";
	//	break;
	case VK_OEM_1:
		Temp = ";";
		break;
	case VK_OEM_PLUS:
		Temp = "+";
		break;
	case VK_OEM_COMMA:
		Temp = ",";
		break;
	case VK_OEM_MINUS:
		Temp = "-";
		break;
	case VK_OEM_PERIOD:
		Temp = ".";
		break;
	case VK_OEM_2:
		Temp = "/";
		break;
	case VK_OEM_3:
		Temp = "`";
		break;
	case VK_OEM_4:
		Temp = "[";
		break;
	case VK_OEM_5:
		Temp = "\\";
		break;
	case VK_OEM_6:
		Temp = "]";
		break;
	case VK_OEM_7:
		Temp = "'";
		break;
	default:
		// Alphabet + numbers
		if (key > 0x2F && key < 0x5B)
		{	
			CHAR A[2];
			A[0] = (CHAR) key;
			A[1] = 0;
			
			Temp = A;
			break;
		}
		// Numpad keys
		if (key > 0x5F && key < 0x6A)
		{
			CHAR A[8] = "NUMPAD";
			_itoa((int) key - 0x60, &A[6], 10);
			A[7] = 0;

			Temp = A;
			break;
		}
		// F1 - F12 keys 
		if (key > 0x6F && key < 0x88)
		{
			CHAR A[4] = "F";
			A[2] = 0;
			A[3] = 0;
			_itoa((int) key - 0x6F, &A[1], 10);
			
			Temp = A;
			break;
		}
	}


	return Temp;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
