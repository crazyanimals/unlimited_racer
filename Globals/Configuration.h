/*
	Configuration:	Units all the configuration data for most project parts to a single class.
					Also contains some usable macros.
	Creation Date:	
	Last Update:	15.12.2006
	Author:			Roman Margold
*/





#pragma once

#include "ErrorHandler.h"
#include "NameValuePair.h"


#define	CONFIG_TEXTURESPATH		(Configuration.NoRelativePaths ? Configuration.TexturesPath : Configuration.DataPath + Configuration.TexturesPath)
#define	CONFIG_SPRITESPATH		(Configuration.NoRelativePaths ? Configuration.SpritesPath : Configuration.DataPath + Configuration.SpritesPath)
#define	CONFIG_MODELSPATH		(Configuration.NoRelativePaths ? Configuration.ModelsPath : Configuration.DataPath + Configuration.ModelsPath)
#define	CONFIG_TERRAINSPATH		(Configuration.NoRelativePaths ? Configuration.TerrainsPath : Configuration.DataPath + Configuration.TerrainsPath)
#define	CONFIG_CARSPATH			(Configuration.NoRelativePaths ? Configuration.CarsPath : Configuration.DataPath + Configuration.ObjectsPath)
#define	CONFIG_DIALOGSPATH		(Configuration.NoRelativePaths ? Configuration.DialogsPath : Configuration.DataPath + Configuration.DialogsPath)
#define	CONFIG_FONTSPATH		(Configuration.NoRelativePaths ? Configuration.FontsPath : Configuration.DataPath + Configuration.FontsPath)
#define	CONFIG_CURSORSPATH		(Configuration.NoRelativePaths ? Configuration.CursorsPath : Configuration.DataPath + Configuration.CursorsPath)
#define	CONFIG_LODDEFINITIONSPATH	(Configuration.NoRelativePaths ? Configuration.LODDefinitionsPath : Configuration.DataPath + Configuration.LODDefinitionsPath)
#define	CONFIG_SHADERSPATH		(Configuration.NoRelativePaths ? Configuration.ShadersPath : Configuration.DataPath + Configuration.ShadersPath)
#define	CONFIG_MAPSPATH			(Configuration.NoRelativePaths ? Configuration.MapsPath : Configuration.DataPath + Configuration.MapsPath)
#define	CONFIG_OBJECTSPATH		(Configuration.NoRelativePaths ? Configuration.ObjectsPath : Configuration.DataPath + Configuration.ObjectsPath)
#define	CONFIG_PHTEXTURESPATH	(Configuration.NoRelativePaths ? Configuration.PhTexturesPath : Configuration.DataPath + Configuration.PhTexturesPath)
#define	CONFIG_LIGHTSPATH		(Configuration.NoRelativePaths ? Configuration.LightsPath : Configuration.DataPath + Configuration.LightsPath)
#define	CONFIG_EDITORPATH		(Configuration.NoRelativePaths ? Configuration.EditorPath : Configuration.DataPath + Configuration.EditorPath)
#define	CONFIG_DEMOSPATH		(Configuration.NoRelativePaths ? Configuration.DemosPath : Configuration.DataPath + Configuration.DemosPath)
#define	CONFIG_BBOXESPATH		(Configuration.NoRelativePaths ? Configuration.BBoxesPath : Configuration.DataPath + Configuration.BBoxesPath)
#define CONFIG_SKYSYSTEMPATH    (Configuration.NoRelativePaths ? Configuration.SkySystemPath : Configuration.DataPath + Configuration.SkySystemPath)
#define CONFIG_HUDPATH			(Configuration.NoRelativePaths ? Configuration.HUDPath : Configuration.DataPath + Configuration.HUDPath)
#define CONFIG_MUSICPATH        (Configuration.NoRelativePaths ? Configuration.MusicPath: Configuration.DataPath + Configuration.MusicPath)
#define CONFIG_TEXTURESETPATH   (Configuration.NoRelativePaths ? Configuration.TextureSetPath: Configuration.DataPath + Configuration.TextureSetPath)
#define CONFIG_SOUNDSPATH		(Configuration.NoRelativePaths ? Configuration.SoundsPath: Configuration.DataPath + Configuration.SoundsPath)
#define CONFIG_FILESFORMATPATH  (Configuration.FilesFormatPath)
#define CONFIG_MUTATORSPATH		(Configuration.NoRelativePaths ? Configuration.MutatorsPath: Configuration.DataPath + Configuration.MutatorsPath)


//////////////////////////////////////////////////////////////////////////////////////////////
//
// structure containing configuration data
//
// call Load to load configuration data; Load method calls global LoadConfig function and gives
// it a pointer to call back method; this CallBack is called for every config pair found
// and it sets an apropriate pair
//
//////////////////////////////////////////////////////////////////////////////////////////////
class CConfig
{
	// members
	public:
		// constructor
		CConfig();

		// path members
		CAtlString		DataPath;	// absolute path to data
		CAtlString		GraphicsPath;	// relative path to graphics from Data directory
		CAtlString		TexturesPath; // relative path to textures from Data directory
		CAtlString		SpritesPath; // relative path to sprites from Data directory
		CAtlString		ModelsPath; // relative path to models from Data directory
		CAtlString		TerrainsPath; // relative path to terrains from Data directory
		CAtlString		CarsPath;	//relative path to cars from Data directory
		CAtlString		ObjectsPath; //relative path to objects Data directory
		CAtlString		LightsPath;  //relative path to lights Data directory		
		CAtlString		EditorPath; //relative path to editor's Data directory
		CAtlString		DialogsPath; // relative path to dialogs from Data directory
		CAtlString		FontsPath; // relative path to fonts from Data directory
		CAtlString		CursorsPath; // relative path to cursors from Data directory
		CAtlString		LODDefinitionsPath; // relative path to LOD definitions from Data directory
		CAtlString		ShadersPath; // relative path to shaders from Data directory
		CAtlString		MapsPath; // relative path to maps from Data directory
		CAtlString		PhTexturesPath; // relative path to physical textures from Data directory
		CAtlString		BBoxesPath; // relative path to bounding boxes from Data directory
		CAtlString		DemosPath; // relative path to demo files from Data directory
		CAtlString		SkySystemPath; // relative path to sky system files from Data directory
		CAtlString		HUDPath; // relative path to HUD (Head-up display) files from Data directory
		CAtlString		MusicPath; // relative path to music filed from Data directory
		CAtlString		TextureSetPath; //relative path to textures set files from data dictionary
		CAtlString		SoundsPath; // relative path to directory containing sound samples and related data
		CAtlString		FilesFormatPath; // relative path to documentation - filesformat
		CAtlString		MutatorsPath;	// relative path to mutators
		BOOL			NoRelativePaths; // if set to true, all paths except DataPath are taken absolutely independet on DataPath 
		
		// basic graphic settings
		WORD			WindowWidth, WindowHeight; // screen resolution
		BOOL			FullScreen;
		int				ColorDepth;	// choosed color depth - 16 or 32

		// rendering flags
		BOOL			RenderDirectionalLight; // renders the scene with directional light (Sun) and its shadows
		BOOL			RenderAmbientLight; // renders the scene with ambient lights - without it, the scene is not visible where is no dynamic light
		BOOL			RenderStaticObjects; // turns on rendering of  objects
		BOOL			RenderDynamicObjects; // turns on rendering of dynamic objects
		BOOL			DynamicLights; // turns on dynamic lights, without dynamic lights there are no complex shadows computed
		BOOL			RenderTerrain; // turns on rendering of the terrain
		BOOL			MapDependentRendering; // the rendering of objects and lights depends on their map position because their visibility is computed on the base of it
		BOOL			RenderLightGlows; // turns on rendering of the light glows (blended sprites)
		BOOL			RenderSky; // turns on and off rendering of the sky system
		BOOL			RenderBoundingEnvelopes; // turns rendering of the bounding envelopes on and off
		BOOL			RenderFarObjects; // turns on rendering of the far objects
		BOOL			RenderHUD; // turns on HUD rendering
		BOOL			RenderGrass; // turns on rendering of grass
		BOOL			ShowFPS; // show frame rate
		BOOL			ShowDebugMsgs; // show debug messages
		BOOL			WaitVSync; // the scene presentation waits for vertical retrace period

		// graphic quality
		int				TerrainComplexity; // level of complexity for terrain, should be in <0,4>
		int				ModelComplexity; // level of complexity for models, should be in <0,4>
		int				GrassComplexity; // level of complexity for grass, should be in <0,4>
		UINT			LOD0TerrainTexWidth; // specifies width of the texture used for terrain LOD0 rendering
		UINT			LOD0TerrainTexHeight; // specifies Height of the texture used for terrain LOD0 rendering
		UINT			AnisotropicFiltering; // specifies how strong the anisotropy is; if set to 0, it's disabled at all
		UINT			Antialiasing; // specifies multisampling level; if set to 0, it's disabled at all; 1=nonmaskable multisampling - MultisampleQuality used
		UINT			MultisampleQuality; // specifies multisample quality level, when Antialiasing is set to 1=nonmaskable
		BOOL			CarPerPixelLight; // enables per-pixel specular lighting computations for cars
		int				ShadowComplexity; // 0 = none; 1 = fast (sprite); 2 = simple (shadowmap); 3 = complex (shadow volume)
		int				TextureQuality; // 0-2

		// appearance
		BOOL			WireFrameMode; // turns on and off the wire frame rendering mode
		float			GammaFactor; // uniform scaling factor for linear gamma correction
		int				MsgFontSize; // font size
		int				DebugFontSize; // debug font size
		CAtlString		MsgFontName; // font face name
		CAtlString		DebugFontName; // debug font face name
		D3DCOLOR		MsgFontColor; // font color
		D3DCOLOR		DebugFontColor; // debug font color

		int				ModelLODCount, TerrainLODCount, GrassLODCount; // these members are not loaded from file, but are computed from loaded LODMap file

		// screen shot number
		UINT			ScreenShotNumber; // Screen shot number, is automatically increased and saved when screen shot is captured


		// key binding members
		int				KeyForward;
		int				KeyBackward;
		int				KeyMoveLeft;
		int				KeyMoveRight;
		int				KeyTurnLeft;
		int				KeyTurnRight;
		int				KeyTurnUp;
		int				KeyTurnDown;
		int				KeyMenu;
	
		int				KeyAccelerate;
		int				KeyDecelerate;
		int				KeySteerLeft;
		int				KeySteerRight;
		int				KeyBrake;
		int				KeyShiftUp;
		int				KeyShiftDown;
		int				KeyChangeView;
		int				KeyDriverView;
		int				KeyRestart;
		int				KeyPause;

		// network settings
		int				netPort;
		int				netBroadPort;
		
		// HUD
		CAtlString		HUD;

		// Music volume (between 0.0 and 1.0)
		float			MusicVolume;

		// Current game related settings
		CAtlString		MapFileName; // Current map file to be loaded
		CAtlString		CarFileName; // Player car
		CAtlString		OpponentsFileNames[MAX_OPPONENTS]; // Car file names for opponents
		
		CAtlString		PlayerName; // Name of player
		int				HoursOfDay; // Hour of day
		int				MinutesOfDay; // Minutes in hour of day
		int				Laps; // Laps to go
		int				Time; // Time to go
		BOOL			LapsEnabled; // Whether laps racing mode is enabled
		BOOL			TimeEnabled; // Whether time racing mode is enabled

		int				DebugLevel; // level of debug messages (0-4, 0 means no messages)

		// physics configuration
		UINT			SolverModel;	//
		UINT			FrictionModel;
		UINT			RoadMaterialIndex;
		UINT			TerrainMaterialIndex;
		float			RoadSpeedCoef;
		float			TerrainSpeedCoef;
		float			CarSpeedLimit;

        //AI parameters
        float UpdateTimeClip;

        float CarMaxSpeed;
        float CarSpeedTreshold;

        float CarSpeedChangeProbability;
        float CarSpeedChange;
        float MassAvoidanceCoef;
        float LeadTime;
        int   CarTimeBeforeRestart;
        float CarTrackProgress;
        float LeakThrough;
        float DistanceTolerance;
        int   MaxWaypointsMissed;
        float MapToPathLocalDistance;
        float DefaultRadius;   
        bool  OptimizeWaypointPlacement;
        float MaxWaypointDistance;

	// methods
		HRESULT					Load( CAtlString fileName );
		void					LoadCallBack( NAMEVALUEPAIR * );
		static void CALLBACK	LoadCallBackStatic( NAMEVALUEPAIR * );


		static int				ParseKeyBindConfigPair( CAtlString key );
		static CAtlString		UnParseKeyBindConfigPair( int key );

	private:

};



extern CConfig	Configuration;

