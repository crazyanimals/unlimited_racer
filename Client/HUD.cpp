/*
HUD.cpp 
Implementation of class CHUD
by Pavel Celba
Created: 8. 12. 2005
*/

#include "HUD.h"

// constructor
CHUD::CSpeedometer::CSpeedometer()
{
	MaxRPM = 6500.0f;
	Registered = FALSE;
}

// Init - sets defaults for speedometer
HRESULT CHUD::CSpeedometer::Init(CHUDCore * _HUDCore, SpeedometerInit & Init)
{
	// HUD Core
	HUDCore = _HUDCore;

	// Background texture
	// Set texture and its size
	BackgroundTexture.Texture = Init.BackgroundTexture;
	BackgroundTexture.DestRect.right = Init.Width;
	BackgroundTexture.DestRect.bottom = Init.Height;
	BackgroundTexture.PositionX = Init.BackgroundTexturePositionX;
	BackgroundTexture.PositionY = Init.BackgroundTexturePositionY;

	// Set default align to bottom right corner
	BackgroundTexture.Align = CHUDCore::HUDALIGN_INNERBOTTOM | CHUDCore::HUDALIGN_INNERRIGHT;

	// speed texture
	// Speed texture is anchored to Background texture (which cannot be done here in init)
	SpeedTexture.Texture = Init.SpeedTexture;
	SpeedTexture.DestRect = Init.SpeedTexturePosition; // Position depends on background texture
	SpeedTexture.PositionX = Init.SpeedTexturePositionX;
	SpeedTexture.PositionY = Init.SpeedTexturePositionY;

	// KmhText label
	// KmhText is anchored to Background texture
	KmhText.Text = Init.KmhText;
	KmhText.DestRect = Init.KmhPosition; // Position depends on background texture
	KmhText.Font = Init.KmhTextFont;
	KmhText.Format = DT_CENTER | DT_VCENTER;
	KmhText.Color = Init.KmhTextFontColor; 

	// Speed text
	SpeedText.DestRect = Init.SpeedTextPosition; // Position depend on background texture
	SpeedText.Font = Init.SpeedTextFont;
	SpeedText.Format = DT_CENTER | DT_VCENTER;
	SpeedText.Color = Init.SpeedTextFontColor;

	return 0;
}

// Registers speedometer with HUD Core
// Note: After registering changes to speedometer can be done only by calling its functions
//       Direct changes will not have effect, because HUDCore makes copy of all added elements
HRESULT CHUD::CSpeedometer::Register()
{
	HRESULT hr;
	// add
	hr = HUDCore->AddElement(&BackgroundTexture, BackgroundTextureElemID);
	if (FAILED(hr) )
		ERRORMSG(hr, "CHUD::CSpeedometer::Register()", "Unable to add background texture element.");
	hr = HUDCore->AddElement(&SpeedTexture, SpeedTextureElemID );
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::CSpeedometer::Register()", "Unable to add speed texture element.");
	hr = HUDCore->AddElement(&KmhText, KmhTextElemID );
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::CSpeedometer::Register()", "Unable to add kmh text element.");
	hr = HUDCore->AddElement(&SpeedText, SpeedTextElemID );
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::CSpeedometer::Register()", "Unable to add speed text element.");

	// link 
	HUDCore->SetAnchor(SpeedTextureElemID, BackgroundTextureElemID);
	HUDCore->SetAnchor(KmhTextElemID, BackgroundTextureElemID);
	HUDCore->SetAnchor(SpeedTextElemID, BackgroundTextureElemID);

	Registered = TRUE;

	return ERRNOERROR;
}

// Sets speed on speedometer
HRESULT CHUD::CSpeedometer::SetSpeed(int NewSpeed)
{
	char SpeedText[20];
	//float SpeedAmount = 0.0f;
	if (!Registered) return 0;

	Speed = NewSpeed;

	/*
	// Compute speed amount
	if (MaxRPM != 0)
		SpeedAmount = (float) Speed / (float) MaxRPM;

	// Set speed texture portion
	HUDCore->SetWidthAmount(SpeedTextureElemID, SpeedAmount);
	*/

	// Set speed text
	_itoa_s(Speed, SpeedText, 15, 10);
	HUDCore->SetText(SpeedTextElemID, CString( SpeedText ));

	return 0;
}

HRESULT CHUD::CSpeedometer::SetRPM(float rpm)
{
	char SpeedText[20];
	float SpeedAmount = 0.0f;
	if (!Registered) return 0;

	// set member
	m_rpm = rpm;

	// check bounds
	if (m_rpm > MaxRPM)
		m_rpm = MaxRPM;
	else if (m_rpm <= 0)
		m_rpm = 1.0f;

	// Compute speed amount
	SpeedAmount = rpm / MaxRPM;

	// Set speed texture portion
	HUDCore->SetWidthAmount(SpeedTextureElemID, SpeedAmount);

	return 0;
}

// constructor
CHUD::SpeedometerInit::SpeedometerInit()
{
	Width = Height = 0;
	BackgroundTextureFilename = "";
	SpeedTextureFilename = "";
	SpeedTextFontFilename = "";
	KmhTextFontFilename = "";
	KmhText = "";
	ZeroMemory(&KmhPosition, sizeof(RECT) );
	ZeroMemory(&SpeedTexturePosition, sizeof(RECT) );
	ZeroMemory(&SpeedTextPosition, sizeof(RECT) );
	BackgroundTexturePositionX = 0;
	BackgroundTexturePositionY = 0;
	SpeedTexturePositionX = 0;
	SpeedTexturePositionY = 0;

	KmhTextFont = 0;
	SpeedTextFont = 0;
	BackgroundTexture = 0;
	SpeedTexture = 0;
	KmhTextFontColor = 0xFF;
	SpeedTextFontColor = 0xFF;

	SpeedometerLoading = FALSE;
}

// constructor
CHUD::PlaceListInit::PlaceListInit()
{
	BottomBackgroundHeight = 0;
	BottomBackgroundTexture = 0;
	BottomBackgroundTextureFilename = "";
	BottomBackgroundTexturePositionX = 0;
	BottomBackgroundTexturePositionY = 0;
	CaptionFont = 0;
	CaptionFontFilename = "";
	CaptionText = "";
	ItemBackgroundHeight = 0;
	ItemBackgroundTexture = 0;
	ItemBackgroundTextureFilename = "";
	ItemBackgroundTexturePositionX = 0;
	ItemBackgroundTexturePositionY = 0;
	NameFont = 0;
	NameFontFilename = "";
	ZeroMemory(&NamePosition, sizeof(RECT) );
	NumberingFont = 0;
	NumberingFontFilename = "";
	ZeroMemory(&NumberingPosition, sizeof(RECT) );
	PlaceListLoading = FALSE;
	TopBackgroundHeight = 0;
	TopBackgroundTexture = 0;
	TopBackgroundTextureFilename = "";
	TopBackgroundTexturePositionX = 0;
	TopBackgroundTexturePositionY = 0;
	Width = 0;
	CaptionFontColor = 0xFF;
	NameFontColor = 0xFF;
	NumberingFontColor = 0xFF;
}

// constructor
CHUD::GearInit::GearInit()
{
	BackgroundTexture = 0;
	BackgroundTextureFilename = "";
	BackgroundTexturePositionX = 0;
	BackgroundTexturePositionY = 0;
	CaptionFont = 0;
	CaptionFontColor = 0xFF;
	CaptionFontFilename = "";
	ZeroMemory(&CaptionPosition, sizeof(RECT));
	CaptionText = "";
	GearFont = 0;
	GearFontColor = 0xFF;
	GearFontFilename = "";
	GearLoading = FALSE;
	ZeroMemory(&GearPosition, sizeof(RECT) );
	Height = 0;
	Width = 0;
}

// constructor
CHUD::CGear::CGear()
{
	Gear = 0;
	Registered = FALSE;
}

// Init
HRESULT CHUD::CGear::Init(CHUDCore * _HUDCore, GearInit & Init)
{
	// HUD Core
	HUDCore = _HUDCore;

	// Background texture
	BackgroundTexture.Texture = Init.BackgroundTexture;
	BackgroundTexture.DestRect.right = Init.Width;
	BackgroundTexture.DestRect.bottom = Init.Height;
	BackgroundTexture.PositionX = Init.BackgroundTexturePositionX;
	BackgroundTexture.PositionY = Init.BackgroundTexturePositionY;
	BackgroundTexture.Align = CHUDCore::HUDALIGN_INNERLEFT | CHUDCore::HUDALIGN_INNERBOTTOM;

	// Caption text
	CaptionText.Text = Init.CaptionText;
	CaptionText.DestRect = Init.CaptionPosition;
	CaptionText.Font = Init.CaptionFont;
	CaptionText.Color = Init.CaptionFontColor;
	
	// Gear text
	GearText.Text = "N";
	GearText.DestRect = Init.GearPosition;
	GearText.Font = Init.GearFont;
	GearText.Color = Init.GearFontColor;

	return 0;
}

// Registers gear with HUD Core
HRESULT CHUD::CGear::Register()
{
	HRESULT hr;
	// add
	hr = HUDCore->AddElement(&BackgroundTexture, BackgroundTextureID);
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::CGear::Register()", "Unable to add background texture element.");
	hr = HUDCore->AddElement(&CaptionText, CaptionTextID);
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::CGear::Register()", "Unable to add caption text element.");
	hr = HUDCore->AddElement(&GearText, GearTextID);
	if (FAILED(hr) )
		ERRORMSG(hr, "CHUD::CGear::Register()", "Unable to add gear text element.");

	// anchor
	HUDCore->SetAnchor(CaptionTextID, BackgroundTextureID);
	HUDCore->SetAnchor(GearTextID, BackgroundTextureID);

	Registered = TRUE;

	return ERRNOERROR;
}

// Set gear, -1 = R, 0 = N, 1, 2, 3...
HRESULT CHUD::CGear::SetGear(int NewGear)
{
	char Num[10];
	if (!Registered) return 0;
	
	Gear = NewGear;
	if (Gear == -1)
	{
		HUDCore->SetText(GearTextID, CString("R"));
	}
	else if (Gear == 0)
	{
		HUDCore->SetText(GearTextID, CString("N") );
	}
	else
	{
		_itoa_s(Gear, Num, 5, 10);
		HUDCore->SetText(GearTextID, CString( Num ) );
	}

	return 0;
}


// Constructor
CHUD::CPlaceList::CPlaceList()
{
	NumPlayers = MAX_OPPONENTS + 1;
	Registered = FALSE;
}

// Init
HRESULT CHUD::CPlaceList::Init(CHUDCore * _HUDCore, PlaceListInit & Init)
{
	int i;
	char Num[10];

	// HUD Core
	HUDCore = _HUDCore;

	// Top texture
	TopTexture.Texture = Init.TopBackgroundTexture;
	TopTexture.DestRect.right = Init.Width;
	TopTexture.DestRect.bottom = Init.TopBackgroundHeight;
	TopTexture.PositionX = Init.TopBackgroundTexturePositionX;
	TopTexture.PositionY = Init.TopBackgroundTexturePositionY;
	// align top left - all other objects will be anchored (indirectly in most cases) to top texture
	TopTexture.Align = CHUDCore::HUDALIGN_INNERTOP | CHUDCore::HUDALIGN_INNERLEFT;
	
	// Item textures
	for (i = 0; i < MAX_OPPONENTS + 1; i++)
	{
		ItemTexture[i].Texture = Init.ItemBackgroundTexture;
		ItemTexture[i].DestRect.right = Init.Width;
		ItemTexture[i].DestRect.bottom = Init.ItemBackgroundHeight;
		ItemTexture[i].PositionX = Init.ItemBackgroundTexturePositionX;
		ItemTexture[i].PositionY = Init.ItemBackgroundTexturePositionY;
		// align downwards
		ItemTexture[i].Align = CHUDCore::HUDALIGN_INNERLEFT | CHUDCore::HUDALIGN_OUTERBOTTOM;
	}

	// Bottom texture
	BottomTexture.Texture = Init.BottomBackgroundTexture;
	BottomTexture.DestRect.right = Init.Width;
	BottomTexture.DestRect.bottom = Init.BottomBackgroundHeight;
	BottomTexture.PositionX = Init.BottomBackgroundTexturePositionX;
	BottomTexture.PositionY = Init.BottomBackgroundTexturePositionY;
	BottomTexture.Align = CHUDCore::HUDALIGN_INNERLEFT | CHUDCore::HUDALIGN_OUTERBOTTOM;

	// Caption text
	CaptionText.Text = Init.CaptionText;
	CaptionText.Font = Init.CaptionFont;
	CaptionText.DestRect.right = Init.Width;
	CaptionText.DestRect.bottom = Init.TopBackgroundHeight;
	// Caption text is centered in top part
	// For some bizzare place lists - place caption directly on top background texture
	CaptionText.Format = DT_CENTER | DT_VCENTER;
	CaptionText.Color = Init.CaptionFontColor;
	
	// Numbering
	for (i = 0; i < MAX_OPPONENTS + 1; i++)
	{
		_itoa_s(i + 1, Num, 5, 10);
		NumberingText[i].Text = CString(Num) + CString(".");
		NumberingText[i].Font = Init.NumberingFont;
		NumberingText[i].Color = Init.NumberingFontColor;
		NumberingText[i].DestRect = Init.NumberingPosition;
	}

	// Names
	for (i = 0; i < MAX_OPPONENTS + 1; i++)
	{
		NameText[i].Font = Init.NameFont;
		NameText[i].Color = Init.NameFontColor;
		NameText[i].DestRect = Init.NamePosition;
	}

	return 0;
}

// Registers place list with HUD Core
// Note: After registering changes to place list can be done only by calling its functions
//       Direct changes will not have effect, because HUDCore makes copy of all added elements
HRESULT CHUD::CPlaceList::Register()
{
	HRESULT hr;
	int i;
	
	// add
	hr = HUDCore->AddElement(&TopTexture, TopTextureID);
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::CPlaceList::Register()", "Unable to add top texture element.");
	
	for (i = 0; i < MAX_OPPONENTS + 1; i++)
	{
		hr = HUDCore->AddElement(& (ItemTexture[i]), ItemTextureID[i]);
		if (FAILED(hr ) )
			ERRORMSG(hr, "CHUD::CPlaceList::Register()", "Unable to add item texture element.");
		hr = HUDCore->AddElement(& (NumberingText[i]), NumberingTextID[i]);
		if (FAILED(hr ) )
			ERRORMSG(hr, "CHUD::CPlaceList::Register()", "Unable to add numbering text element.");
		hr = HUDCore->AddElement(& (NameText[i] ), NameTextID[i]  );
		if (FAILED(hr) )
			ERRORMSG(hr, "CHUD::CPlaceList::Register()", "Unable to add name text element.");
	}

	hr = HUDCore->AddElement(&BottomTexture, BottomTextureID);
	if (FAILED(hr) )
		ERRORMSG(hr, "CHUD::CPlaceList::Register()", "Unable to add bottom texture element.");
	hr = HUDCore->AddElement(&CaptionText, CaptionTextID );
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::CPlaceList::Register()", "Unable to add caption text element.");

	// anchor
	HUDCore->SetAnchor(ItemTextureID[0], TopTextureID);

	for (i = 0; i < MAX_OPPONENTS + 1; i++)
	{
		HUDCore->SetAnchor(NumberingTextID[i], ItemTextureID[i]);
		HUDCore->SetAnchor(NameTextID[i], ItemTextureID[i]);
		if (i != 0)
			HUDCore->SetAnchor(ItemTextureID[i], ItemTextureID[i - 1]);
	}
	
	HUDCore->SetAnchor(CaptionTextID, TopTextureID);
	HUDCore->SetAnchor(BottomTextureID, ItemTextureID[MAX_OPPONENTS]);

	Registered = TRUE;

	return ERRNOERROR;
}

// Sets number of players and changes list appearance respectively
HRESULT CHUD::CPlaceList::SetNumberOfPlayers(int NumOfPlayers)
{
	int i;

	if (!Registered) return 0;
	if (NumOfPlayers < 1 || NumOfPlayers > MAX_OPPONENTS + 1) return 0;

	NumPlayers = NumOfPlayers;

	// Show only enough list items
	for (i = 0; i < MAX_OPPONENTS + 1; i++)
		if (i < NumPlayers)
		{
			ItemTexture[i].Visible = TRUE;
			NumberingText[i].Visible = TRUE;
			NameText[i].Visible = TRUE;
		}
		else
		{
			ItemTexture[i].Visible = FALSE;
			NumberingText[i].Visible = FALSE;
			NameText[i].Visible = FALSE;
		}

	// Reanchor bottom texture
	HUDCore->SetAnchor(BottomTextureID, ItemTextureID[NumPlayers - 1]);

	// Recompute positions, because of reanchoring
	HUDCore->PrepareForRendering();

	return 0;
}

// Updates place list
// Updated names are shown 
HRESULT CHUD::CPlaceList::UpdateNameList()
{
	UINT i,e;
	CString Temp;
	char Num[10];
	
	if (!Registered) return 0;
	
	for (i = 0; i < MAX_PLAYERS; i++)
		if (RaceState[i].PlayerName != "")
		{

			Temp = RaceState[i].PlayerName;
			for (e = Temp.GetLength(); e < 10; e++)
				Temp += CString(" ");
			
			Temp += CString(" Laps: ") + CString(_itoa(RaceState[i].LapsGone, Num, 10) );
			HUDCore->SetText(NameTextID[RaceState[i].Position], Temp);
			
		}
	
	return 0;
}

// Clears race state
void CHUD::CPlaceList::ClearRaceState()
{
	for (UINT i = 0; i < MAX_PLAYERS; i++)
		RaceState[i].Clear();
}

// Sets race state of certain player
HRESULT CHUD::CPlaceList::SetRaceState(UINT ID, CString & Name, UINT Laps, UINT RacePos)
{
	if (ID >= MAX_PLAYERS)
		ERRORMSG(-1, "CHUD::CPlaceList::SetRaceState()", "ID is out of range.");
	if ( (int) RacePos >= NumPlayers )
	{
		OUTS("CPlaceList::SetRaceState(): WARNING: Position is bigger than number of players.", 2);
		return ERRNOERROR;
	}

	if (RaceState[ID].PlayerName == "")
		RaceState[ID].PlayerName = Name;

	RaceState[ID].LapsGone = Laps;
	RaceState[ID].Position = RacePos;

	UpdateNameList();

	return ERRNOERROR;
}


// Init
HRESULT CHUD::Init(CHUDCore * _HUDCore, resManNS::CResourceManager * _ResourceManager)
{
	HUDCore = _HUDCore;
	ResourceManager = _ResourceManager;

	return 0;
}

// Loads HUD from file
HRESULT CHUD::LoadHUD(CString FileName)
{
	HRESULT hr;
	NAMEVALUEPAIR Pair;
	resManNS::RESOURCEID TempID;
	resManNS::__Texture * tex;
	resManNS::__FontDef * font;

	hr = Pair.Load(FileName, HUDStaticCallback, this);
	if (FAILED(hr) ) ERRORMSG(hr, "CHUD::LoadHUD()", "Error while loading HUD");

	// Delete all resources
	FreeHUD();

	// Load textures and fonts for speedometer
	if (SMInit.BackgroundTextureFilename != "")
	{
		TempID = ResourceManager->LoadResource(SMInit.BackgroundTextureFilename, RES_Texture, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load background texture for speedometer.");
		ResourceIDs.push_back(TempID);
		tex = ResourceManager->GetTexture(TempID);
		SMInit.BackgroundTexture = tex->texture;
	}
	if (SMInit.SpeedTextureFilename != "")
	{
		TempID = ResourceManager->LoadResource(SMInit.SpeedTextureFilename, RES_Texture, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load speed texture for speedometer.");
		ResourceIDs.push_back(TempID);
		tex = ResourceManager->GetTexture(TempID);
		SMInit.SpeedTexture = tex->texture;
	}
	if (SMInit.KmhTextFontFilename != "")
	{
		TempID = ResourceManager->LoadResource(SMInit.KmhTextFontFilename, RES_FontDef, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load font for kmh text on speedometer.");
		ResourceIDs.push_back(TempID);
		font = ResourceManager->GetFontDef( TempID );
		SMInit.KmhTextFont = font->pFont;
		SMInit.KmhTextFontColor = font->iColor;
	}
	if (SMInit.SpeedTextFontFilename != "")
	{
		TempID = ResourceManager->LoadResource(SMInit.SpeedTextFontFilename, RES_FontDef, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load font for speed text on speedometer.");
		ResourceIDs.push_back(TempID);
		font = ResourceManager->GetFontDef( TempID);
		SMInit.SpeedTextFont = font->pFont;
		SMInit.SpeedTextFontColor = font->iColor;
	}

	// Initialize speedometer
	hr = Speedometer.Init(HUDCore, SMInit);
	if (FAILED(hr) ) ERRORMSG(hr, "CHUD::LoadHUD()", "Error while initializing speedometer.");

	// Load textures and fonts for place list
	if (PLInit.TopBackgroundTextureFilename != "")
	{
		TempID = ResourceManager->LoadResource(PLInit.TopBackgroundTextureFilename, RES_Texture, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load top background texture for place list.");
		ResourceIDs.push_back(TempID);
		tex = ResourceManager->GetTexture(TempID);
		PLInit.TopBackgroundTexture = tex->texture;
	}
	if (PLInit.ItemBackgroundTextureFilename != "")
	{
		TempID = ResourceManager->LoadResource(PLInit.ItemBackgroundTextureFilename, RES_Texture, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load item background texture for place list.");
		ResourceIDs.push_back(TempID);
		tex = ResourceManager->GetTexture(TempID);
		PLInit.ItemBackgroundTexture = tex->texture;
	}
	if (PLInit.BottomBackgroundTextureFilename != "")
	{
		TempID = ResourceManager->LoadResource(PLInit.BottomBackgroundTextureFilename, RES_Texture, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load bottom background texture for place list.");
		ResourceIDs.push_back(TempID);
		tex = ResourceManager->GetTexture(TempID);
		PLInit.BottomBackgroundTexture = tex->texture;
	}
	if (PLInit.CaptionFontFilename != "")
	{
		TempID = ResourceManager->LoadResource(PLInit.CaptionFontFilename, RES_FontDef, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load font for caption text on place list.");
		ResourceIDs.push_back(TempID);
		font = ResourceManager->GetFontDef( TempID);
		PLInit.CaptionFont = font->pFont;
		PLInit.CaptionFontColor = font->iColor;
	}
	if (PLInit.NumberingFontFilename != "")
	{
		TempID = ResourceManager->LoadResource(PLInit.NumberingFontFilename, RES_FontDef, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load font for numbering on place list.");
		ResourceIDs.push_back(TempID);
		font = ResourceManager->GetFontDef( TempID);
		PLInit.NumberingFont = font->pFont;
		PLInit.NumberingFontColor = font->iColor;
	}
	if (PLInit.NameFontFilename != "")
	{
		TempID = ResourceManager->LoadResource(PLInit.NameFontFilename, RES_FontDef, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load font for names on place list.");
		ResourceIDs.push_back(TempID);
		font = ResourceManager->GetFontDef( TempID);
		PLInit.NameFont = font->pFont;
		PLInit.NameFontColor = font->iColor;
	}

	// initialize place list
	hr = PlaceList.Init(HUDCore, PLInit);
	if (FAILED(hr) ) ERRORMSG(hr, "CHUD::LoadHUD()", "Error while initializing place list on HUD.");

	// Load textures and fonts for gear
	if (GInit.BackgroundTextureFilename != "")
	{
		TempID = ResourceManager->LoadResource(GInit.BackgroundTextureFilename, RES_Texture, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load background texture for gear.");
		ResourceIDs.push_back(TempID);
		tex = ResourceManager->GetTexture(TempID);
		GInit.BackgroundTexture= tex->texture;
	}
	if (GInit.CaptionFontFilename != "")
	{
		TempID = ResourceManager->LoadResource(GInit.CaptionFontFilename, RES_FontDef, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load font for caption text on gear.");
		ResourceIDs.push_back(TempID);
		font = ResourceManager->GetFontDef( TempID);
		GInit.CaptionFont= font->pFont;
		GInit.CaptionFontColor = font->iColor;
	}
	if (GInit.GearFontFilename != "")
	{
		TempID = ResourceManager->LoadResource(GInit.GearFontFilename, RES_FontDef, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load font for gear text on gear.");
		ResourceIDs.push_back(TempID);
		font = ResourceManager->GetFontDef( TempID);
		GInit.GearFont = font->pFont;
		GInit.GearFontColor = font->iColor;
	}

	// initialize gear
	hr = Gear.Init(HUDCore, GInit);
	if (FAILED(hr) ) ERRORMSG(hr, "CHUD::LoadHUD()", "Error while initializing gear on HUD.");

	// Load textures and fonts for "Now playing"
	if (NPInit.BackgroundTextureFilename != "")
	{
		TempID = ResourceManager->LoadResource(NPInit.BackgroundTextureFilename, RES_Texture, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load background texture for Now playing.");
		ResourceIDs.push_back(TempID);
		tex = ResourceManager->GetTexture(TempID);
		NPInit.BackgroundTexture= tex->texture;
	}
	if (NPInit.CaptionFontFilename != "")
	{
		TempID = ResourceManager->LoadResource(NPInit.CaptionFontFilename, RES_FontDef, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load font for caption text on Now playing.");
		ResourceIDs.push_back(TempID);
		font = ResourceManager->GetFontDef( TempID);
		NPInit.CaptionFont= font->pFont;
		NPInit.CaptionFontColor = font->iColor;
	}
	if (NPInit.NowPlayingFontFilename != "")
	{
		TempID = ResourceManager->LoadResource(NPInit.NowPlayingFontFilename, RES_FontDef, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load font for caption text on Now playing.");
		ResourceIDs.push_back(TempID);
		font = ResourceManager->GetFontDef( TempID);
		NPInit.NowPlayingFont= font->pFont;
		NPInit.NowPlayingFontColor = font->iColor;
	}

	// init "Now playing"
	hr = NowPlaying.Init(HUDCore, NPInit);
	if (FAILED(hr) ) ERRORMSG(hr, "CHUD::LoadHUD()", "Error while initializing Now playing on HUD.");
	


	// Load textures and fonts for "Score"
	if (ScInit.BackgroundTextureFilename != "")
	{
		TempID = ResourceManager->LoadResource(ScInit.BackgroundTextureFilename, RES_Texture, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load background texture for Score object.");
		ResourceIDs.push_back(TempID);
		tex = ResourceManager->GetTexture(TempID);
		ScInit.BackgroundTexture= tex->texture;
	}
	if (ScInit.CaptionFontFilename != "")
	{
		TempID = ResourceManager->LoadResource(ScInit.CaptionFontFilename, RES_FontDef, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load font for caption text on Score object.");
		ResourceIDs.push_back(TempID);
		font = ResourceManager->GetFontDef( TempID);
		ScInit.CaptionFont= font->pFont;
		ScInit.CaptionFontColor = font->iColor;
	}
	if (ScInit.TextFontFilename != "")
	{
		TempID = ResourceManager->LoadResource(ScInit.TextFontFilename, RES_FontDef, 0);
		if (FAILED(TempID) ) ERRORMSG(TempID, "CHUD::LoadHUD()", "Unable to load font for caption text on Score object.");
		ResourceIDs.push_back(TempID);
		font = ResourceManager->GetFontDef( TempID);
		ScInit.TextFont= font->pFont;
		ScInit.TextFontColor = font->iColor;
	}

	// init Score object 
	hr = Score.Init(HUDCore, ScInit);
	if (FAILED(hr) ) ERRORMSG(hr, "CHUD::LoadHUD()", "Error while initializing Score object on HUD.");


    
	// load textures and fonts for "InfoMessage" object
	if (InfoInit.BackgroundTextureFilename != "")
    {
        resManNS::RESOURCEID res_id = ResourceManager->LoadResource(InfoInit.BackgroundTextureFilename, RES_Texture, 0);
        
        if (FAILED(res_id))
        {
            ERRORMSG(res_id, "CHUD::LoadHUD()", "Unable to load background texture for the Info Message object.");
        }

        ResourceIDs.push_back(res_id);

        resManNS::Texture *texture = ResourceManager->GetTexture(res_id);
        InfoInit.BackgroundTexture = texture->texture;
    }

    if (InfoInit.CaptionFontFilename != "")
    {
        resManNS::RESOURCEID res_id = ResourceManager->LoadResource(InfoInit.CaptionFontFilename, RES_FontDef, 0);
        
        if (FAILED(res_id))
        {
            ERRORMSG(res_id, "CHUD::LoadHUD()", "Unable to load caption font for the Info Message object.");
        }

        ResourceIDs.push_back(res_id);

        resManNS::__FontDef *font_def = ResourceManager->GetFontDef(res_id);
        InfoInit.CaptionFont = font_def->pFont;
        InfoInit.CaptionFontColor = font_def->iColor;
    }

    if (InfoInit.TextFontFilename != "")
    {
        resManNS::RESOURCEID res_id = ResourceManager->LoadResource(InfoInit.TextFontFilename, RES_FontDef, 0);
        
        if (FAILED(res_id))
        {
            ERRORMSG(res_id, "CHUD::LoadHUD()", "Unable to load text font for the Info Message object.");
        }

        ResourceIDs.push_back(res_id);

        resManNS::__FontDef *font_def = ResourceManager->GetFontDef(res_id);
        InfoInit.TextFont = font_def->pFont;
        InfoInit.TextFontColor = font_def->iColor;
    }

    hr = InfoMessage.Init(HUDCore, InfoInit);
    if (FAILED(hr))
    {
        ERRORMSG(hr, "CHUD::LoadHUD()", "Unable to initialize Info Message object.");
    }


	
	// load textures and fonts for "Time" object
	if (TimeInit.BackgroundTextureFilename != "")
    {
        resManNS::RESOURCEID res_id = ResourceManager->LoadResource(TimeInit.BackgroundTextureFilename, RES_Texture, 0);
        
        if (FAILED(res_id))
        {
            ERRORMSG(res_id, "CHUD::LoadHUD()", "Unable to load background texture for the Time object.");
        }

        ResourceIDs.push_back(res_id);

        resManNS::Texture *texture = ResourceManager->GetTexture(res_id);
        TimeInit.BackgroundTexture = texture->texture;
    }

    if (TimeInit.CaptionFontFilename != "")
    {
        resManNS::RESOURCEID res_id = ResourceManager->LoadResource(TimeInit.CaptionFontFilename, RES_FontDef, 0);
        
        if (FAILED(res_id))
        {
            ERRORMSG(res_id, "CHUD::LoadHUD()", "Unable to load caption font for the Time object.");
        }

        ResourceIDs.push_back(res_id);

        resManNS::__FontDef *font_def = ResourceManager->GetFontDef(res_id);
        TimeInit.CaptionFont = font_def->pFont;
        TimeInit.CaptionFontColor = font_def->iColor;
    }

    if (TimeInit.TextFontFilename != "")
    {
        resManNS::RESOURCEID res_id = ResourceManager->LoadResource(TimeInit.TextFontFilename, RES_FontDef, 0);
        
        if (FAILED(res_id))
        {
            ERRORMSG(res_id, "CHUD::LoadHUD()", "Unable to load text font for the Time object.");
        }

        ResourceIDs.push_back(res_id);

        resManNS::__FontDef *font_def = ResourceManager->GetFontDef(res_id);
        TimeInit.TextFont = font_def->pFont;
        TimeInit.TextFontColor = font_def->iColor;
    }

	// init Time object
	hr = Time.Init(HUDCore, TimeInit);
    if (FAILED(hr))
    {
        ERRORMSG(hr, "CHUD::LoadHUD()", "Unable to initialize Time object.");
    }

	return 0;
}

// Frees all loaded HUD resources
void CHUD::FreeHUD()
{
	UINT i;
	for (i = 0; i < ResourceIDs.size(); i++)
		ResourceManager->ReleaseResource(ResourceIDs[i] );

	ResourceIDs.clear();

	if ( HUDCore ) HUDCore->DeleteElements();
}

// destructor
CHUD::~CHUD()
{
	FreeHUD();
}


// Registers HUD with HUD Core rendering 
HRESULT CHUD::Register()
{
	HRESULT hr;
	// Register speedometer
	hr = Speedometer.Register();
	if (FAILED(hr) )
		ERRORMSG(hr, "CHUD::Register()", "Unable to register speedometer with HUD core.");

	// Register place list
	hr = PlaceList.Register();
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::Register()", "Unable to register place list with HUD core.");

	// Register gear
	hr = Gear.Register();
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::Register()", "Unable to register gear with HUD core.");

	// Register "Now playing"
	hr = NowPlaying.Register();
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::Register()", "Unable to register now playing with HUD core.");

	// Register "Score" object
	hr = Score.Register();
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::Register()", "Unable to register Score object with HUD core.");

    hr = InfoMessage.Register();
    if (FAILED(hr))
    {
        ERRORMSG(hr, "CHUD::Register()", "Unable to register InfoMessage object with the HUD core.");
    }

    hr = Time.Register();
    if (FAILED(hr))
    {
        ERRORMSG(hr, "CHUD::Register()", "Unable to register Time object with the HUD core.");
    }

	// Call prepare for rendering in order to prepare all elements for rendering
	HUDCore->PrepareForRendering();

	return ERRNOERROR;
}

// HUD Load static callback
void CHUD::HUDStaticCallback(NAMEVALUEPAIR * Pair)
{
	CHUD * HUD;
	HUD =  (CHUD*) Pair->ReservedPointer;
	HUD->HUDCallback(Pair);
}

// HUD load callback
void CHUD::HUDCallback(NAMEVALUEPAIR * Pair)
{
	CString Name;

	Name = Pair->GetName();
	Name.MakeUpper();

	if (Name == _T("SPEEDOMETER") )
	{
		// Indicate, that we are in Speedometer tag
		if (Pair->GetFlags() == NVPTYPE_TAGBEGIN)
			SMInit.SpeedometerLoading = TRUE;
		// Indicate, that we are out of Speedometer tag
		else if (Pair->GetFlags() == NVPTYPE_TAGEND) 
			SMInit.SpeedometerLoading = FALSE;

		return;	
	}
	else if (Name == _T("PLACELIST") )
	{
		// Indicate, that we are in place list tag
		if (Pair->GetFlags() == NVPTYPE_TAGBEGIN)
			PLInit.PlaceListLoading = TRUE;
		// Indicate, that we are out of place list tag
		else if (Pair->GetFlags() == NVPTYPE_TAGEND)
			PLInit.PlaceListLoading = FALSE;
	}
	else if (Name == _T("GEAR") )
	{
		// Indicate, that we are in gear tag
		if (Pair->GetFlags() == NVPTYPE_TAGBEGIN)
			GInit.GearLoading = TRUE;
		// Indicate, that we are out of gear tag
		else if (Pair->GetFlags() == NVPTYPE_TAGEND)
			GInit.GearLoading = FALSE;
	}
	else if (Name == _T("NOWPLAYING") )
	{
		// Indicate, that we are in now playing tag
		if (Pair->GetFlags() == NVPTYPE_TAGBEGIN)
			NPInit.NowPlayingLoading = TRUE;
		// Indicate, that we are out of now playing tag
		else if (Pair->GetFlags() == NVPTYPE_TAGEND)
			NPInit.NowPlayingLoading = FALSE;
	}
	else if (Name == _T("SCORE") )
	{
        if (Pair->GetFlags() == NVPTYPE_TAGBEGIN)
        {
            // Indicate, that we are in score tag
            ScInit.bLoading = TRUE;
        }
        else if (Pair->GetFlags() == NVPTYPE_TAGEND)
        {
            // Indicate, that we are out of now playing tag
            ScInit.bLoading = FALSE;
        }
	}
    else if (Name == _T("INFO"))
    {
        if (Pair->GetFlags() == NVPTYPE_TAGBEGIN)
        {
            InfoInit.bLoading = TRUE;
        }
        else if (Pair->GetFlags() == NVPTYPE_TAGEND)
        {
            InfoInit.bLoading = FALSE;
        }
    }
    else if (Name == _T("TIME"))
    {
        if (Pair->GetFlags() == NVPTYPE_TAGBEGIN)
        {
            TimeInit.bLoading = TRUE;
        }
        else if (Pair->GetFlags() == NVPTYPE_TAGEND)
        {
            TimeInit.bLoading = FALSE;
        }
    }


	if (SMInit.SpeedometerLoading)
	{
		if (Name == _T("WIDTH") ) { SMInit.Width = Pair->GetInt(); return; }
		if (Name == _T("HEIGHT") ) {SMInit.Height = Pair->GetInt(); return; }
		if (Name == _T("BACKGROUNDTEXTURE") ) {SMInit.BackgroundTextureFilename = Pair->GetString(); return; }
		if (Name == _T("BACKGROUNDTEXTUREPOSITIONONTEXTURE.X") ) {SMInit.BackgroundTexturePositionX = Pair->GetInt(); return;  }
		if (Name == _T("BACKGROUNDTEXTUREPOSITIONONTEXTURE.Y") ) {SMInit.BackgroundTexturePositionY = Pair->GetInt(); return;  }
		if (Name == _T("SPEEDTEXTURE") ) {SMInit.SpeedTextureFilename = Pair->GetString(); return; }
		if (Name == _T("SPEEDTEXTUREPOSITION.LEFT") ) {SMInit.SpeedTexturePosition.left = Pair->GetInt(); return; }
		if (Name == _T("SPEEDTEXTUREPOSITION.RIGHT"))  {SMInit.SpeedTexturePosition.right = Pair->GetInt(); return; }
		if (Name == _T("SPEEDTEXTUREPOSITION.TOP") ) {SMInit.SpeedTexturePosition.top = Pair->GetInt(); return; }
		if (Name == _T("SPEEDTEXTUREPOSITION.BOTTOM") ) {SMInit.SpeedTexturePosition.bottom = Pair->GetInt(); return; }
		if (Name == _T("SPEEDTEXTUREPOSITIONONTEXTURE.X") ) {SMInit.SpeedTexturePositionX = Pair->GetInt(); return; }
		if (Name == _T("SPEEDTEXTUREPOSITIONONTEXTURE.Y") ) {SMInit.SpeedTexturePositionY = Pair->GetInt(); return; }
		if (Name == _T("KMHTEXT")) {SMInit.KmhText = Pair->GetString(); return; }
		if (Name == _T("KMHTEXTPOSITION.LEFT")) {SMInit.KmhPosition.left = Pair->GetInt(); return; }
		if (Name == _T("KMHTEXTPOSITION.RIGHT")) {SMInit.KmhPosition.right = Pair->GetInt(); return; }
		if (Name == _T("KMHTEXTPOSITION.TOP")) {SMInit.KmhPosition.top = Pair->GetInt(); return; }
		if (Name == _T("KMHTEXTPOSITION.BOTTOM")) {SMInit.KmhPosition.bottom = Pair->GetInt(); return; }
		if (Name == _T("SPEEDTEXTPOSITION.LEFT")) {SMInit.SpeedTextPosition.left = Pair->GetInt(); return; }
		if (Name == _T("SPEEDTEXTPOSITION.RIGHT")) {SMInit.SpeedTextPosition.right = Pair->GetInt(); return; }
		if (Name == _T("SPEEDTEXTPOSITION.TOP")) {SMInit.SpeedTextPosition.top = Pair->GetInt(); return; }
		if (Name == _T("SPEEDTEXTPOSITION.BOTTOM")) {SMInit.SpeedTextPosition.bottom = Pair->GetInt(); return; }
		if (Name == _T("KMHTEXTFONT")) {SMInit.KmhTextFontFilename = Pair->GetString(); return; }
		if (Name == _T("SPEEDTEXTFONT")) {SMInit.SpeedTextFontFilename = Pair->GetString(); return; }
		
		return;
	}

	if (PLInit.PlaceListLoading)
	{
		if (Name == _T("WIDTH") ) {PLInit.Width = Pair->GetInt(); return; }
		if (Name == _T("TOPBACKGROUNDTEXTURE") ) {PLInit.TopBackgroundTextureFilename = Pair->GetString(); return; }
		if (Name == _T("TOPBACKGROUNDTEXTUREHEIGHT") ) {PLInit.TopBackgroundHeight = Pair->GetInt(); return; }
		if (Name == _T("ITEMBACKGROUNDTEXTURE") ) {PLInit.ItemBackgroundTextureFilename = Pair->GetString(); return; }
		if (Name == _T("ITEMBACKGROUNDTEXTUREHEIGHT") ) {PLInit.ItemBackgroundHeight = Pair->GetInt(); return; }
		if (Name == _T("BOTTOMBACKGROUNDTEXTURE") ) {PLInit.BottomBackgroundTextureFilename = Pair->GetString(); return; }
		if (Name == _T("BOTTOMBACKGROUNDTEXTUREHEIGHT") ) {PLInit.BottomBackgroundHeight = Pair->GetInt(); return; }
		if (Name == _T("TOPBACKGROUNDTEXTUREPOSITIONONTEXTURE.X") ) {PLInit.TopBackgroundTexturePositionX = Pair->GetInt(); return; }
		if (Name == _T("TOPBACKGROUNDTEXTUREPOSITIONONTEXTURE.Y") ) {PLInit.TopBackgroundTexturePositionY = Pair->GetInt(); return; }
		if (Name == _T("ITEMBACKGROUNDTEXTUREPOSITIONONTEXTURE.X") ) {PLInit.ItemBackgroundTexturePositionX = Pair->GetInt(); return; }
		if (Name == _T("ITEMBACKGROUNDTEXTUREPOSITIONONTEXTURE.Y") ) {PLInit.ItemBackgroundTexturePositionY = Pair->GetInt(); return; }
		if (Name == _T("BOTTOMBACKGROUNDTEXTUREPOSITIONONTEXTURE.X") ) {PLInit.BottomBackgroundTexturePositionX = Pair->GetInt(); return; }
		if (Name == _T("BOTTOMBACKGROUNDTEXTUREPOSITIONONTEXTURE.Y") ) {PLInit.BottomBackgroundTexturePositionY = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONTEXT") ) {PLInit.CaptionText = Pair->GetString(); return; }
		if (Name == _T("CAPTIONFONT") ) {PLInit.CaptionFontFilename = Pair->GetString(); return; }
		if (Name == _T("NUMBERINGFONT" ) ) {PLInit.NumberingFontFilename = Pair->GetString(); return; }
		if (Name == _T("NAMEFONT") ) {PLInit.NameFontFilename = Pair->GetString(); return; }
		if (Name == _T("NUMBERINGPOSITION.LEFT") ) {PLInit.NumberingPosition.left = Pair->GetInt(); return; }
		if (Name == _T("NUMBERINGPOSITION.RIGHT") ) {PLInit.NumberingPosition.right = Pair->GetInt(); return; }
		if (Name == _T("NUMBERINGPOSITION.TOP") ) {PLInit.NumberingPosition.top = Pair->GetInt(); return; }
		if (Name == _T("NUMBERINGPOSITION.BOTTOM") ) {PLInit.NumberingPosition.bottom = Pair->GetInt(); return; }
		if (Name == _T("NAMEPOSITION.LEFT") ) {PLInit.NamePosition.left = Pair->GetInt(); return; }
		if (Name == _T("NAMEPOSITION.RIGHT" ) ) {PLInit.NamePosition.right = Pair->GetInt(); return; }
		if (Name == _T("NAMEPOSITION.TOP") ) {PLInit.NamePosition.top = Pair->GetInt(); return; }
		if (Name == _T("NAMEPOSITION.BOTTOM") ) {PLInit.NamePosition.bottom = Pair->GetInt(); return; }

		return;
	}

	if (GInit.GearLoading)
	{
		if (Name == _T("WIDTH") ) {GInit.Width = Pair->GetInt(); return; }
		if (Name == _T("HEIGHT") ) {GInit.Height = Pair->GetInt(); return; }
		if (Name == _T("BACKGROUNDTEXTURE") ) {GInit.BackgroundTextureFilename = Pair->GetString(); return; }
		if (Name == _T("CAPTIONTEXT") ) {GInit.CaptionText = Pair->GetString(); return; }
		if (Name == _T("CAPTIONFONT") ) {GInit.CaptionFontFilename = Pair->GetString(); return; }
		if (Name == _T("CAPTIONPOSITION.LEFT") ) {GInit.CaptionPosition.left = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONPOSITION.RIGHT" ) ) {GInit.CaptionPosition.right = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONPOSITION.TOP" ) ) {GInit.CaptionPosition.top = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONPOSITION.BOTTOM") ) {GInit.CaptionPosition.bottom = Pair->GetInt(); return; }
		if (Name == _T("GEARFONT") ) {GInit.GearFontFilename = Pair->GetString(); return; }
		if (Name == _T("GEARPOSITION.LEFT") ) {GInit.GearPosition.left = Pair->GetInt(); return; }
		if (Name == _T("GEARPOSITION.RIGHT") ) {GInit.GearPosition.right = Pair->GetInt(); return; }
		if (Name == _T("GEARPOSITION.TOP") ) {GInit.GearPosition.top = Pair->GetInt(); return; }
		if (Name == _T("GEARPOSITION.BOTTOM") ) {GInit.GearPosition.bottom = Pair->GetInt(); return; }
		if (Name == _T("BACKGROUNDTEXTUREPOSITIONONTEXTURE.X") ) {GInit.BackgroundTexturePositionX = Pair->GetInt(); return; }
		if (Name == _T("BACKGROUNDTEXTUREPOSITIONONTEXTURE.Y") ) {GInit.BackgroundTexturePositionY = Pair->GetInt(); return; }

		return;
	}

	if (NPInit.NowPlayingLoading)
	{
		if (Name == _T("WIDTH") ) {NPInit.Width = Pair->GetInt(); return; }
		if (Name == _T("HEIGHT") ) {NPInit.Height = Pair->GetInt(); return; }
		if (Name == _T("BACKGROUNDTEXTURE") ) {NPInit.BackgroundTextureFilename = Pair->GetString(); return; }
		if (Name == _T("BACKGROUNDTEXTUREPOSITIONONTEXTURE.X") ) {NPInit.BackgroundTexturePositionX = Pair->GetInt(); return; }
		if (Name == _T("BACKGROUNDTEXTUREPOSITIONONTEXTURE.Y") ) {NPInit.BackgroundTexturePositionY = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONTEXT") ) {NPInit.CaptionText = Pair->GetString(); return; }
		if (Name == _T("CAPTIONPOSITION.LEFT") ) {NPInit.CaptionPosition.left = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONPOSITION.RIGHT") ) {NPInit.CaptionPosition.right = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONPOSITION.TOP") ) {NPInit.CaptionPosition.top = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONPOSITION.BOTTOM") ) {NPInit.CaptionPosition.bottom = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONFONT") ) {NPInit.CaptionFontFilename = Pair->GetString(); return; }
		if (Name == _T("NOWPLAYINGFONT") ) {NPInit.NowPlayingFontFilename = Pair->GetString(); return; }
		if (Name == _T("NOWPLAYINGPOSITION.LEFT") ) {NPInit.NowPlayingPosition.left = Pair->GetInt(); return; }
		if (Name == _T("NOWPLAYINGPOSITION.RIGHT") ) {NPInit.NowPlayingPosition.right = Pair->GetInt(); return; }
		if (Name == _T("NOWPLAYINGPOSITION.TOP") ) { NPInit.NowPlayingPosition.top = Pair->GetInt(); return; }
		if (Name == _T("NOWPLAYINGPOSITION.BOTTOM") ) {NPInit.NowPlayingPosition.bottom = Pair->GetInt(); return; }
		if (Name == _T("MAXTEXTLENGTH") ) {NPInit.MaxLength = Pair->GetInt(); return; }
		if (Name == _T("SEPARATOR") ) {NPInit.Separator = Pair->GetString(); return; }

		return;
	}

	if (ScInit.bLoading)
	{
		if (Name == _T("WIDTH") ) {ScInit.Width = Pair->GetInt(); return; }
		if (Name == _T("HEIGHT") ) {ScInit.Height = Pair->GetInt(); return; }
		if (Name == _T("BACKGROUNDTEXTURE") ) {ScInit.BackgroundTextureFilename = Pair->GetString(); return; }
		if (Name == _T("BACKGROUNDTEXTUREPOSITIONONTEXTURE.X") ) {ScInit.BackgroundTexturePositionX = Pair->GetInt(); return; }
		if (Name == _T("BACKGROUNDTEXTUREPOSITIONONTEXTURE.Y") ) {ScInit.BackgroundTexturePositionY = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONTEXT") ) {ScInit.CaptionText = Pair->GetString(); return; }
		if (Name == _T("CAPTIONPOSITION.LEFT") ) {ScInit.CaptionPosition.left = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONPOSITION.RIGHT") ) {ScInit.CaptionPosition.right = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONPOSITION.TOP") ) {ScInit.CaptionPosition.top = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONPOSITION.BOTTOM") ) {ScInit.CaptionPosition.bottom = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONFONT") ) {ScInit.CaptionFontFilename = Pair->GetString(); return; }
		if (Name == _T("TEXTFONT") ) {ScInit.TextFontFilename = Pair->GetString(); return; }
		if (Name == _T("TEXTPOSITION.LEFT") ) {ScInit.TextPosition.left = Pair->GetInt(); return; }
		if (Name == _T("TEXTPOSITION.RIGHT") ) {ScInit.TextPosition.right = Pair->GetInt(); return; }
		if (Name == _T("TEXTPOSITION.TOP") ) { ScInit.TextPosition.top = Pair->GetInt(); return; }
		if (Name == _T("TEXTPOSITION.BOTTOM") ) {ScInit.TextPosition.bottom = Pair->GetInt(); return; }
		if (Name == _T("MAXTEXTLENGTH") ) {ScInit.MaxLength = Pair->GetInt(); return; }

		return;
	}

    if (InfoInit.bLoading)
    {
        if (Name == _T("WIDTH"))
        {
            InfoInit.Width = Pair->GetInt();
        }
        
        if (Name == _T("HEIGHT"))
        {
            InfoInit.Height = Pair->GetInt();
        }
        
        if (Name == _T("BACKGROUNDTEXTURE"))
        {
            InfoInit.BackgroundTextureFilename = Pair->GetString();
        }
        
        if (Name == _T("BACKGROUNDTEXTUREPOSITIONONTEXTURE.X"))
        {
            InfoInit.BackgroundTexturePositionX = Pair->GetInt();
        }
        
        if (Name == _T("BACKGROUNDTEXTUREPOSITIONONTEXTURE.Y"))
        {
            InfoInit.BackgroundTexturePositionY = Pair->GetInt();
        }
        
        if (Name == _T("CAPTIONTEXT"))
        {
            InfoInit.CaptionText = Pair->GetString();
        }
        
        if (Name == _T("CAPTIONPOSITION.LEFT"))
        {
            InfoInit.CaptionPosition.left = Pair->GetInt();
        }
        
        if (Name == _T("CAPTIONPOSITION.RIGHT"))
        {
            InfoInit.CaptionPosition.right = Pair->GetInt();
        }
        
        if (Name == _T("CAPTIONPOSITION.TOP"))
        {
            InfoInit.CaptionPosition.top = Pair->GetInt();
        }
        
        if (Name == _T("CAPTIONPOSITION.BOTTOM"))
        {
            InfoInit.CaptionPosition.bottom = Pair->GetInt();
        }
        
        if (Name == _T("CAPTIONFONT")) 
        {
            InfoInit.CaptionFontFilename = Pair->GetString();
        }
        
        if (Name == _T("TEXTFONT")) 
        {
            InfoInit.TextFontFilename = Pair->GetString();
        }
        
        if (Name == _T("TEXTPOSITION.LEFT"))
        {
            InfoInit.TextPosition.left = Pair->GetInt();
        }
        
        if (Name == _T("TEXTPOSITION.RIGHT"))
        {
            InfoInit.TextPosition.right = Pair->GetInt();
        }
        
        if (Name == _T("TEXTPOSITION.TOP"))
        {
            InfoInit.TextPosition.top = Pair->GetInt();
        }
        
        if (Name == _T("TEXTPOSITION.BOTTOM"))
        {
            InfoInit.TextPosition.bottom = Pair->GetInt();
        }
        
        if (Name == _T("MAXTEXTLENGTH"))
        {
            InfoInit.MaxLength = Pair->GetInt();
        }

        return;
    }


	if (TimeInit.bLoading)
	{
		if (Name == _T("WIDTH") ) {TimeInit.Width = Pair->GetInt(); return; }
		if (Name == _T("HEIGHT") ) {TimeInit.Height = Pair->GetInt(); return; }
		if (Name == _T("BACKGROUNDTEXTURE") ) {TimeInit.BackgroundTextureFilename = Pair->GetString(); return; }
		if (Name == _T("BACKGROUNDTEXTUREPOSITIONONTEXTURE.X") ) {TimeInit.BackgroundTexturePositionX = Pair->GetInt(); return; }
		if (Name == _T("BACKGROUNDTEXTUREPOSITIONONTEXTURE.Y") ) {TimeInit.BackgroundTexturePositionY = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONTEXT") ) {TimeInit.CaptionText = Pair->GetString(); return; }
		if (Name == _T("CAPTIONPOSITION.LEFT") ) {TimeInit.CaptionPosition.left = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONPOSITION.RIGHT") ) {TimeInit.CaptionPosition.right = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONPOSITION.TOP") ) {TimeInit.CaptionPosition.top = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONPOSITION.BOTTOM") ) {TimeInit.CaptionPosition.bottom = Pair->GetInt(); return; }
		if (Name == _T("CAPTIONFONT") ) {TimeInit.CaptionFontFilename = Pair->GetString(); return; }
		if (Name == _T("TEXTFONT") ) {TimeInit.TextFontFilename = Pair->GetString(); return; }
		if (Name == _T("TEXTPOSITION.LEFT") ) {TimeInit.TextPosition.left = Pair->GetInt(); return; }
		if (Name == _T("TEXTPOSITION.RIGHT") ) {TimeInit.TextPosition.right = Pair->GetInt(); return; }
		if (Name == _T("TEXTPOSITION.TOP") ) { TimeInit.TextPosition.top = Pair->GetInt(); return; }
		if (Name == _T("TEXTPOSITION.BOTTOM") ) {TimeInit.TextPosition.bottom = Pair->GetInt(); return; }
		if (Name == _T("MAXTEXTLENGTH") ) {TimeInit.MaxLength = Pair->GetInt(); return; }

		return;
	}

}



// Must be called when d3ddevice is lost
void CHUD::OnLostDevice()
{
	Speedometer.OnLostDevice();
	PlaceList.OnLostDevice();
	Gear.OnLostDevice();
	NowPlaying.OnLostDevice();
	Score.OnLostDevice();
    InfoMessage.OnLostDevice();
	Time.OnLostDevice();
}

// Must be called when d3ddevice is lost
void CHUD::CSpeedometer::OnLostDevice()
{
	if (KmhText.Font)
		KmhText.Font->OnLostDevice();

	if (SpeedText.Font)
		SpeedText.Font->OnLostDevice();
}

// Must be called when d3ddevice is lost
void CHUD::CPlaceList::OnLostDevice()
{
	if (CaptionText.Font)
		CaptionText.Font->OnLostDevice();

	if (NameText[0].Font)
		NameText[0].Font->OnLostDevice();

	if (NumberingText[0].Font)
		NumberingText[0].Font->OnLostDevice();
}

// Must be called when d3ddevice is lost
void CHUD::CGear::OnLostDevice()
{
	if (CaptionText.Font)
		CaptionText.Font->OnLostDevice();

	if (GearText.Font)
		GearText.Font->OnLostDevice();
}




// Must be called after d3ddevice is reset
void CHUD::OnResetDevice()
{
	Speedometer.OnResetDevice();
	PlaceList.OnResetDevice();
	Gear.OnResetDevice();
	NowPlaying.OnResetDevice();
	Score.OnResetDevice();
    InfoMessage.OnResetDevice();
	Time.OnResetDevice();
}

// Must be called after d3ddevice is reset
void CHUD::CSpeedometer::OnResetDevice()
{
	if (KmhText.Font)
		KmhText.Font->OnResetDevice();

	if (SpeedText.Font)
		SpeedText.Font->OnResetDevice();
}

// Must be called after d3ddevice is reset
void CHUD::CPlaceList::OnResetDevice()
{
	if (CaptionText.Font)
		CaptionText.Font->OnResetDevice();

	if (NameText[0].Font)
		NameText[0].Font->OnResetDevice();

	if (NumberingText[0].Font)
		NumberingText[0].Font->OnResetDevice();
}

// Must be called after d3ddevice is reset
void CHUD::CGear::OnResetDevice()
{
	if (CaptionText.Font)
		CaptionText.Font->OnResetDevice();

	if (GearText.Font)
		GearText.Font->OnResetDevice();
}

// Sets speedometer visible/invisible
void CHUD::CSpeedometer::SetVibibility(BOOL NewVisibility)
{
	if (!Registered) return;

	HUDCore->SetVisible(BackgroundTextureElemID, NewVisibility);
	HUDCore->SetVisible(KmhTextElemID, NewVisibility);
	HUDCore->SetVisible(SpeedTextElemID, NewVisibility);
	HUDCore->SetVisible(SpeedTextureElemID, NewVisibility);
}

// Sets place list visible/invisible
void CHUD::CPlaceList::SetVibibility(BOOL NewVisibility)
{
	if (!Registered) return;

	HUDCore->SetVisible(BottomTextureID, NewVisibility);
	HUDCore->SetVisible(CaptionTextID, NewVisibility);
	HUDCore->SetVisible(TopTextureID, NewVisibility);

	for (int i = 0; i < MAX_OPPONENTS + 1; i++)
	{
		HUDCore->SetVisible(ItemTextureID[i], NewVisibility);
		HUDCore->SetVisible(NameTextID[i], NewVisibility);
		HUDCore->SetVisible(NumberingTextID[i], NewVisibility);
	}

	if (NewVisibility)
		SetNumberOfPlayers(NumPlayers);
}


// Sets gear visible/invisible
void CHUD::CGear::SetVisibility(BOOL NewVisibility)
{
	if (!Registered) return;

	HUDCore->SetVisible(BackgroundTextureID, NewVisibility);
	HUDCore->SetVisible(CaptionTextID, NewVisibility);
	HUDCore->SetVisible(GearTextID, NewVisibility);
}

// constructor
CHUD::NowPlayingInit::NowPlayingInit()
{
	BackgroundTexture = 0;
	BackgroundTextureFilename = "";
	BackgroundTexturePositionX = 0;
	BackgroundTexturePositionY = 0;
	CaptionFont = 0;
	CaptionFontColor = 0xFF;
	CaptionFontFilename = "";
	ZeroMemory(&CaptionPosition, sizeof(RECT));
	CaptionText = "";
	Height = 0;
	MaxLength = 0;
	NowPlayingFont = 0;
	NowPlayingFontColor = 0xFF;
	NowPlayingFontFilename = "";
	NowPlayingLoading = FALSE;
	ZeroMemory(&NowPlayingPosition, sizeof(RECT) );
	Separator = "";
	Width = 0;
}

// Constructor
CHUD::CNowPlaying::CNowPlaying()
{
	Registered = FALSE;
	MaxTextLength = 100;
	Separator = "";
	NowPlaying = "";
}

// Init
HRESULT CHUD::CNowPlaying::Init(CHUDCore *  _HUDCore, NowPlayingInit & Init)
{
	// HUD Core
	HUDCore = _HUDCore;

	// Background texture
	BackgroundTexture.Texture = Init.BackgroundTexture;
	BackgroundTexture.DestRect.right = Init.Width;
	BackgroundTexture.DestRect.bottom = Init.Height;
	BackgroundTexture.PositionX = Init.BackgroundTexturePositionX;
	BackgroundTexture.PositionY = Init.BackgroundTexturePositionY;
	// Align Now playing to right top corner
	BackgroundTexture.Align = CHUDCore::HUDALIGN_INNERRIGHT | CHUDCore::HUDALIGN_INNERTOP;

	// Caption text
	CaptionText.Text = Init.CaptionText;
	CaptionText.DestRect =  Init.CaptionPosition;
	CaptionText.Font = Init.CaptionFont;
	CaptionText.Color = Init.CaptionFontColor;
	
	// Now playing text
	NowPlayingText.Text = "";
	NowPlayingText.DestRect = Init.NowPlayingPosition;
	NowPlayingText.Font = Init.NowPlayingFont;
	NowPlayingText.Color = Init.NowPlayingFontColor;

	return 0;
}


// Registers "Now playing" with HUD Core
HRESULT CHUD::CNowPlaying::Register()
{
	HRESULT hr;
	// Add
	hr = HUDCore->AddElement(&BackgroundTexture, BackgroundTextureID);
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::CNowPlaying::Register()", "Unable to add background texture element.");
	hr = HUDCore->AddElement(&CaptionText, CaptionTextID);
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::CNowPlaying::Register()", "Unable to add caption text element.");
	hr = HUDCore->AddElement(&NowPlayingText, NowPlayingTextID);
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::CNowPlaying::Register()", "Unable to add now playing text element.");

	// Anchor
	HUDCore->SetAnchor(CaptionTextID, BackgroundTextureID);
	HUDCore->SetAnchor(NowPlayingTextID, BackgroundTextureID);

	Registered = TRUE;

	return ERRNOERROR;
}

// Must be called, when D3DDevice is lost
void CHUD::CNowPlaying::OnLostDevice()
{
	if (CaptionText.Font)
		CaptionText.Font->OnLostDevice();

	if (NowPlayingText.Font)
		NowPlayingText.Font->OnLostDevice();
}	

// Must be called after D3DDevice is reset
void CHUD::CNowPlaying::OnResetDevice()
{
	if (CaptionText.Font)
		CaptionText.Font->OnResetDevice();

	if (NowPlayingText.Font)
		NowPlayingText.Font->OnResetDevice();
}

// Sets now playing text - artist and title
HRESULT CHUD::CNowPlaying::SetNowPlayingText(CString & Artist, CString & Title)
{
	if (!Registered) return 0;

	NowPlaying = Artist + CString(" ") + Separator + CString(" ") + Title;
	// Bound to right size
	NowPlaying = NowPlaying.Left(MaxTextLength);

	// Set text to HUD
	HUDCore->SetText(NowPlayingTextID, NowPlaying);

	return 0;
}

// Sets "Now playing" visible/invisible
void CHUD::CNowPlaying::SetVisibility(BOOL NewVisibility)
{
	if (!Registered) return;

	HUDCore->SetVisible(BackgroundTextureID, NewVisibility);
	HUDCore->SetVisible(CaptionTextID, NewVisibility);
	HUDCore->SetVisible(NowPlayingTextID, NewVisibility);
}



//////////////////////////////////////////////////////////////////////////////////////////////
//
//   S C O R E    O B J E C T
//
//////////////////////////////////////////////////////////////////////////////////////////////

// constructor
CHUD::ScoreInit::ScoreInit()
{
	BackgroundTexture = 0;
	BackgroundTextureFilename = "";
	BackgroundTexturePositionX = 0;
	BackgroundTexturePositionY = 0;
	CaptionFont = 0;
	CaptionFontColor = 0xFF;
	CaptionFontFilename = "";
	ZeroMemory(&CaptionPosition, sizeof(RECT));
	CaptionText = "";
	Height = 0;
	MaxLength = 0;
	TextFont = 0;
	TextFontColor = 0xFFFFFF00;
	TextFontFilename = "";
	bLoading = FALSE;
	ZeroMemory(&TextPosition, sizeof(RECT) );
	Width = 0;
}

// Constructor
CHUD::CScore::CScore()
{
	bRegistered = FALSE;
	uiMaxTextLength = 5;
	csText = "";
	csCaption = "";
}

// Init
HRESULT CHUD::CScore::Init(CHUDCore *  _HUDCore, ScoreInit & Init)
{
	// HUD Core
	pHUDCore = _HUDCore;

	// Background texture
	elBackgroundTexture.Texture = Init.BackgroundTexture;
	elBackgroundTexture.DestRect.right = Init.Width;
	elBackgroundTexture.DestRect.bottom = Init.Height;
	elBackgroundTexture.PositionX = Init.BackgroundTexturePositionX;
	elBackgroundTexture.PositionY = Init.BackgroundTexturePositionY;
	
	// Align this object to the middle of top edge of the screen
	elBackgroundTexture.Align = CHUDCore::HUDALIGN_HORIZONTALCENTER | CHUDCore::HUDALIGN_INNERTOP;

	// Caption text
	elCaption.Text = Init.CaptionText;
	elCaption.DestRect =  Init.CaptionPosition;
	elCaption.Font = Init.CaptionFont;
	elCaption.Color = Init.CaptionFontColor;
	elCaption.Format = DT_CENTER | DT_VCENTER; // center text both horiz. and vert.
	
	// Main text
	elText.Text = "";
	elText.DestRect = Init.TextPosition;
	elText.Font = Init.TextFont;
	elText.Color = Init.TextFontColor;
	elText.Format = DT_CENTER | DT_VCENTER; // center text both horiz. and vert.
	elText.Color = Init.TextFontColor;

    uiMaxTextLength = Init.MaxLength;

	return 0;
}


// Registers "Score" object with HUD Core
HRESULT CHUD::CScore::Register()
{
	HRESULT hr;
	// Add
	hr = pHUDCore->AddElement(&elBackgroundTexture, uiBackgroundTextureID);
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::CScore::Register()", "Unable to add background texture element.");
	hr = pHUDCore->AddElement(&elCaption, uiCaptionID);
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::CScore::Register()", "Unable to add caption text element.");
	hr = pHUDCore->AddElement(&elText, uiTextID);
	if (FAILED(hr ) )
		ERRORMSG(hr, "CHUD::CScore::Register()", "Unable to add now playing text element.");

	// Anchor
	pHUDCore->SetAnchor(uiCaptionID, uiBackgroundTextureID);
	pHUDCore->SetAnchor(uiTextID, uiBackgroundTextureID);

	bRegistered = TRUE;

	return ERRNOERROR;
}

// Must be called, when D3DDevice is lost
void CHUD::CScore::OnLostDevice()
{
	if (elCaption.Font)
		elCaption.Font->OnLostDevice();

	if (elText.Font)
		elText.Font->OnLostDevice();
}	

// Must be called after D3DDevice is reset
void CHUD::CScore::OnResetDevice()
{
	if (elCaption.Font)
		elCaption.Font->OnResetDevice();

	if (elText.Font)
		elText.Font->OnResetDevice();
}

// Sets the main text
HRESULT CHUD::CScore::SetText(CString & text)
{
	if (!bRegistered) return 0;

	csText = text;

	// cut longer (than allowed length) strings
	csText = csText.Left(uiMaxTextLength);

	// Set text to HUD
	pHUDCore->SetText(uiTextID, csText);

	return 0;
}

// Sets "Score" object's visibility
void CHUD::CScore::SetVisibility(BOOL NewVisibility)
{
	if (!bRegistered) return;

	pHUDCore->SetVisible(uiBackgroundTextureID, NewVisibility);
	pHUDCore->SetVisible(uiCaptionID, NewVisibility);
	pHUDCore->SetVisible(uiTextID, NewVisibility);
}
