/*
HUD.h 
HUD (Head-up display) implementation
This class implements speedometer, list of opponents and their position
by Pavel Celba, Roman Margold
Created:		8.12.2005
Last update:	3.3.2007
*/

#pragma once 

#include "stdafx.h"
#include "..\Globals\Globals.h"
#include "..\Globals\ErrorHandler.h"
#include "..\Globals\Configuration.h"
#include "..\Globals\NameValuePair.h"
#include "..\ResourceManager\ResourceManager.h"
#include "..\Graphic\HUDCore.h"



// HUD (Head-up display) class
class CHUD
{
public:
	CHUD() { HUDCore = NULL; };
	
	// destructor
	~CHUD();

	// Init
	HRESULT Init(CHUDCore * _HUDCore, resManNS::CResourceManager * _ResourceManager);
	
	// Frees all loaded HUD resources
	void FreeHUD();

	// Loads HUD from file
	HRESULT LoadHUD(CString FileName);

	// Registers HUD with HUD Core rendering 
	HRESULT Register();

	// Must be called, when D3DDevice is lost
	void OnLostDevice();

	// Must be called after D3DDevice is reset
	void OnResetDevice();

	// PUBLIC CLASS DECLARATIONS
	struct SpeedometerInit
	{
		// constructor
		SpeedometerInit();
			
		// From file
		int Width, Height;
		CString BackgroundTextureFilename;
		CString SpeedTextureFilename;
		RECT SpeedTexturePosition;
		CString KmhText;
		RECT KmhPosition;
		RECT SpeedTextPosition;
		CString KmhTextFontFilename;
		CString SpeedTextFontFilename;
		UINT BackgroundTexturePositionX;
		UINT BackgroundTexturePositionY;
		UINT SpeedTexturePositionX;
		UINT SpeedTexturePositionY;

		// loaded
		LPDIRECT3DTEXTURE9 BackgroundTexture;
		LPDIRECT3DTEXTURE9 SpeedTexture;
		ID3DXFont *	KmhTextFont;
		DWORD KmhTextFontColor;
		ID3DXFont * SpeedTextFont;
		DWORD SpeedTextFontColor;

		// misc
		BOOL SpeedometerLoading;
	};

	class CSpeedometer
	{
	public:
		// constructor
		CSpeedometer();

		// Init - sets defaults for speedometer
		HRESULT Init(CHUDCore * HUDCore, SpeedometerInit & Init);


		// Registers speedometer with HUD Core
		// Note: After registering changes to speedometer can be done only by calling its functions
		//       Direct changes will not have effect, because HUDCore makes copy of all added elements
		HRESULT Register();

		// Must be called, when D3DDevice is lost
		void OnLostDevice();

		// Must be called after D3DDevice is reset
		void OnResetDevice();

		// THIS FUNCTIONS WILL NOT HAVE EFFECT, WHEN CALLED BEFORE Register() WAS CALLED!!!
		// Sets speed on speedometer
		HRESULT SetSpeed(int NewSpeed);
		HRESULT SetRPM(float NewRPM);

		// Sets speedometer visible/invisible
		void SetVibibility(BOOL NewVisibility);

		// END THIS FUNCTIONS

		// Gets speed 
		inline int GetSpeed()
		{
			return Speed;
		}

		// sets maximal speed
		inline void SetMaxSpeed(int NewMaxSpeed)
		{
			MaxRPM = NewMaxSpeed;
		}

		// Gets maximal speed
		inline int GetMaxSpeed()
		{
			return MaxRPM;
		}

		// Background speedometer texture
		CHUDCore::HUDTextureElement BackgroundTexture;
		// Speed texture
		CHUDCore::HUDTextureElement SpeedTexture;

		// current speed text
		CHUDCore::HUDTextElement SpeedText;

		// Kmh label
		CHUDCore::HUDTextElement KmhText;

	private:
		// Current speed
		int Speed;
		float m_rpm;

		// Maximal speed
		int MaxRPM;
		
		// HUD Rendering class
		CHUDCore * HUDCore;

		// IDs of HUD Core added elements 
		UINT SpeedTextElemID;
		UINT KmhTextElemID;
		UINT SpeedTextureElemID;
		UINT BackgroundTextureElemID;

		// Elements added
		BOOL Registered;
	};

	struct PlaceListInit
	{
		// constructor
		PlaceListInit();

		// from file
		UINT Width;
		UINT TopBackgroundHeight;
		UINT ItemBackgroundHeight;
		UINT BottomBackgroundHeight;
		CString TopBackgroundTextureFilename;
		CString ItemBackgroundTextureFilename;
		CString BottomBackgroundTextureFilename;
		UINT TopBackgroundTexturePositionX;
		UINT TopBackgroundTexturePositionY;
		UINT ItemBackgroundTexturePositionX;
		UINT ItemBackgroundTexturePositionY;
		UINT BottomBackgroundTexturePositionX;
		UINT BottomBackgroundTexturePositionY;
		CString CaptionText;
		CString CaptionFontFilename;
		CString NumberingFontFilename;
		CString NameFontFilename;
		RECT NumberingPosition; // according to Item position
		RECT NamePosition; // according to Item position

		// loaded
		LPDIRECT3DTEXTURE9 TopBackgroundTexture;
		LPDIRECT3DTEXTURE9 ItemBackgroundTexture;
		LPDIRECT3DTEXTURE9 BottomBackgroundTexture;
		ID3DXFont * CaptionFont;
		ID3DXFont * NumberingFont;
		ID3DXFont * NameFont;
		UINT CaptionFontColor;
		UINT NumberingFontColor;
		UINT NameFontColor;

		// misc
		BOOL PlaceListLoading;
	};

	// Place list class implementation
	class CPlaceList
	{
	public:
		// Constructor
		CPlaceList();

		// Init
		HRESULT Init(CHUDCore * _HUDCore, PlaceListInit & Init);

		// Registers place list with HUD Core
		// Note: After registering changes to place list can be done only by calling its functions
		//       Direct changes will not have effect, because HUDCore makes copy of all added elements
		HRESULT Register();

		// Must be called, when D3DDevice is lost
		void OnLostDevice();

		// Must be called after D3DDevice is reset
		void OnResetDevice();

		// THIS FUNCTIONS WILL NOT HAVE EFFECT, WHEN CALLED BEFORE Register() WAS CALLED!!!
		// Sets number of players and changes list appearance respectively
		HRESULT SetNumberOfPlayers(int NumOfPlayers);

		// Updates place list
		// Updated names are shown 
		HRESULT UpdateNameList();

		// Sets place list visible/invisible
		void SetVibibility(BOOL NewVisibility);

		// END THIS FUNCTIONS

		// Player info used for recording player status in the race
		struct PlayerInfo
		{
		public:
			PlayerInfo() 
			{
				Clear();
			};

			void Clear()
			{
				PlayerName = "";
				LapsGone = 0;
				Position = 0;
			};

			CString PlayerName;
			UINT LapsGone;
			UINT Position;
		};

		
		// Clears race state
		void ClearRaceState();

		// Sets race state of certain player
		HRESULT SetRaceState(UINT ID, CString & Name, UINT Laps, UINT RacePos);

		// Race state information
		PlayerInfo RaceState[MAX_PLAYERS];

		// Top texture
		CHUDCore::HUDTextureElement TopTexture;
		// List item textures
		// Number of opponents + player
		CHUDCore::HUDTextureElement ItemTexture[MAX_OPPONENTS + 1];
		// Bottom texture
		CHUDCore::HUDTextureElement BottomTexture;
		// Caption text
		CHUDCore::HUDTextElement CaptionText;
		// Numbering texts
		CHUDCore::HUDTextElement NumberingText[MAX_OPPONENTS + 1];
		// name texts
		CHUDCore::HUDTextElement NameText[MAX_OPPONENTS + 1];
	private:
		// Number of players currently playing game
		int NumPlayers;

		// HUD Rendering class
		CHUDCore * HUDCore;

		// IDs of HUD Core added elements
		UINT TopTextureID;
		UINT ItemTextureID[MAX_OPPONENTS + 1];
		UINT BottomTextureID;
		UINT CaptionTextID;
		UINT NumberingTextID[MAX_OPPONENTS + 1];
		UINT NameTextID[MAX_OPPONENTS + 1];

		// Elements added
		BOOL Registered;
	};

	// Gear init
	struct GearInit
	{
		// constructor
		GearInit();

		// from file
		UINT Width;
		UINT Height;
		CString BackgroundTextureFilename;
		CString CaptionText;
		CString CaptionFontFilename;
		RECT CaptionPosition;
		CString GearFontFilename;
		RECT GearPosition;
		UINT BackgroundTexturePositionX;
		UINT BackgroundTexturePositionY;

		// load
		LPDIRECT3DTEXTURE9 BackgroundTexture;
		ID3DXFont * CaptionFont;
		ID3DXFont * GearFont;
		DWORD CaptionFontColor;
		DWORD GearFontColor;

		// mics
		BOOL GearLoading;
	};

	// Gear info class - show gear already geared
	class CGear 
	{
	public:
		// constructor
		CGear();

		// Init
		HRESULT Init(CHUDCore * _HUDCore, GearInit & Init);

		// Registers place list with HUD Core
		// Note: After registering changes to place list can be done only by calling its functions
		//       Direct changes will not have effect, because HUDCore makes copy of all added elements
		HRESULT Register();

		// Must be called, when D3DDevice is lost
		void OnLostDevice();

		// Must be called after D3DDevice is reset
		void OnResetDevice();

		// THIS FUNCTIONS WILL NOT HAVE EFFECT, WHEN CALLED BEFORE Register() WAS CALLED!!!
		// Set gear, -1 = R, 0 = N, 1, 2, 3...
		HRESULT SetGear(int NewGear);

		// Sets gear visible/invisible
		void SetVisibility(BOOL NewVisibility);

		// END THIS FUNCTIONS

		// get gear
		inline int GetGear()
		{
			return Gear;
		}

		// Background texture
		CHUDCore::HUDTextureElement BackgroundTexture;
		// Caption
		CHUDCore::HUDTextElement CaptionText;
		// Gear 
		CHUDCore::HUDTextElement GearText;
	private:
		// Gear
		int Gear;

		// HUD Rendering class
		CHUDCore * HUDCore;

		// IDs of HUD Core added elements
		UINT BackgroundTextureID;
		UINT CaptionTextID;
		UINT GearTextID;

		// Elements added
		BOOL Registered;
	};



	struct NowPlayingInit
	{
		// constructor
		NowPlayingInit();
		
		// from file
		CString BackgroundTextureFilename;
		UINT BackgroundTexturePositionX;
		UINT BackgroundTexturePositionY;
		UINT Width;
		UINT Height;
		CString CaptionText;
		RECT CaptionPosition;
		CString CaptionFontFilename;
		CString NowPlayingFontFilename;
		RECT NowPlayingPosition;
		UINT MaxLength;
		CString Separator;

		// load
		LPDIRECT3DTEXTURE9 BackgroundTexture;
		ID3DXFont * CaptionFont;
		ID3DXFont * NowPlayingFont;
		DWORD CaptionFontColor;
		DWORD NowPlayingFontColor;

		// misc
		BOOL NowPlayingLoading;
	};

	// Now playing info class
	class CNowPlaying
	{
	public:
		// Constructor
		CNowPlaying();

		// Init
		HRESULT Init(CHUDCore *  _HUDCore, NowPlayingInit & Init);

		// Registers "Now playing" with HUD Core
		// Note: After registering changes to place list can be done only by calling its functions
		//       Direct changes will not have effect, because HUDCore makes copy of all added elements
		HRESULT Register();

		// THIS FUNCTIONS WILL NOT HAVE EFFECT, WHEN CALLED BEFORE Register() WAS CALLED!!!
		// Sets now playing text
		HRESULT SetNowPlayingText(CString & Artist, CString & Title);

		// Sets "Now playing" visible/invisible
		void SetVisibility(BOOL NewVisibility);
			
		// END THIS FUNCTIONS

		// returns now playing text
		inline CString & GetNowPlayingText()
		{
			return NowPlaying;
		}

		// Must be called, when D3DDevice is lost
		void OnLostDevice();

		// Must be called after D3DDevice is reset
		void OnResetDevice();

		// Background texture
		CHUDCore::HUDTextureElement BackgroundTexture;
		// Caption text
		CHUDCore::HUDTextElement CaptionText;
		// NowPlayingText
		CHUDCore::HUDTextElement NowPlayingText;
	private:
		// maximal text length
		UINT MaxTextLength;

		// Now playing text
		CString NowPlaying;

		// Separator
		CString Separator;

		// HUD Core
		CHUDCore * HUDCore;

		// Element IDs
		UINT BackgroundTextureID;
		UINT CaptionTextID;
		UINT NowPlayingTextID;
		
		// Registered with HUD Core
		BOOL Registered;
	};



	struct ScoreInit
	{
		// constructor
		ScoreInit();
		
		// from file
		CString		BackgroundTextureFilename;
		UINT		BackgroundTexturePositionX;
		UINT		BackgroundTexturePositionY;
		UINT		Width;
		UINT		Height;
		CString		CaptionText;
		RECT		CaptionPosition;
		CString		CaptionFontFilename;
		CString		TextFontFilename;
		RECT		TextPosition;
		UINT		MaxLength;

		// load
		LPDIRECT3DTEXTURE9	BackgroundTexture;
		ID3DXFont		*	CaptionFont;
		ID3DXFont		*	TextFont;
		DWORD				CaptionFontColor;
		DWORD				TextFontColor;

		// misc
		BOOL		bLoading; // used as a flag during HUD configuration loading
	};


	// Score info class (although defined for free use to display any text information)
	class CScore
	{
	public:
		// Constructor
		CScore();

		// Init
		HRESULT Init(CHUDCore *  _HUDCore, ScoreInit & Init);

		// Registers "Score" with HUD Core
		// Note: After registering changes to place list can be done only by calling its functions
		//       Direct changes will not have effect, because HUDCore makes copy of all added elements
		HRESULT Register();

		// THESE FUNCTIONS WILL NOT HAVE EFFECT, WHEN CALLED BEFORE Register() WAS CALLED!!!
		// Sets now playing text
		HRESULT SetText(CString & text);

		// Sets now playing text
		HRESULT SetCaption(CString & text);

		// Sets this HUD item visible/invisible
		void SetVisibility(BOOL NewVisibility);
			
		// END THESE FUNCTIONS :)

		// returns the 'score' object's text
		CString & GetText() { return csText; };

		// returns the 'score' object's caption
		CString & GetCaption() { return csCaption; };

		// Must be called, when D3DDevice is lost
		void OnLostDevice();

		// Must be called after D3DDevice is reset
		void OnResetDevice();

		// Background texture
		CHUDCore::HUDTextureElement		elBackgroundTexture;
		// Caption text
		CHUDCore::HUDTextElement		elCaption;
		// NowPlayingText
		CHUDCore::HUDTextElement		elText;
	private:
		// maximal text length
		UINT		uiMaxTextLength;

		// the text to display
		CString		csText;

		// the caption text to display
		CString		csCaption;

		// HUD Core
		CHUDCore *	pHUDCore;

		// Element IDs
		UINT		uiBackgroundTextureID;
		UINT		uiCaptionID;
		UINT		uiTextID;
		
		// Registered with HUD Core
		BOOL		bRegistered;
	};

	// PUBLIC VARS
	// Speedometer
	CSpeedometer	Speedometer;

	// Place list
	CPlaceList		PlaceList;

	// Gear
	CGear			Gear;

	// Now playing
	CNowPlaying		NowPlaying;

	// Score (or other game info)
	CScore			Score;

    // Info text displayed under the score
    CScore          InfoMessage;

	// displayed game time (or time left)
	CScore			Time;

private:
// CHUD PRIVATE DECLARATIONS
// HUD Core - framework for HUD rendering
CHUDCore * HUDCore;

// Resource manager
resManNS::CResourceManager * ResourceManager;

// Speedometer init
SpeedometerInit	SMInit;

// Place list init
PlaceListInit	PLInit;

// Gear init
GearInit		GInit;

// Now playing init
NowPlayingInit	NPInit;

// Score object init structure
ScoreInit		ScInit;

ScoreInit       InfoInit;

ScoreInit       TimeInit;

// All resource IDs - for free function
std::vector<resManNS::RESOURCEID> ResourceIDs;


// CHUD PRIVATE FUNCTIONS
// HUD Load static callback
static void CALLBACK HUDStaticCallback(NAMEVALUEPAIR * Pair);

// HUD load callback
void HUDCallback(NAMEVALUEPAIR * Pair);


};

