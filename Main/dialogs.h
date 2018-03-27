#pragma once


#include "..\Graphic\GUIDialog.h"
#include "..\Graphic\Graphic.h"
#include "..\ResourceManager\ResourceManager.h"
#include "..\Client\CClient.h"
#include "..\GameMods\GameMod.h"
//#include "..\Client\IntegrityChecker.h"


//////////////////////////////////////////////////////////
// Screen shot capture
HRESULT CaptureScreenShot();

//////////////////////////////////////////////////////////
// If you wanna work with dialogs, let the force be with you.

#define CHANGE_ACTDIALOG(dialog) CommonSounds.pPrimarySoundBank->Play( XACT_CUE_CAR_MOUSECLICK, 0, 0, NULL );\
	pDialogs->pActDialog->OnLeave();\
	pDialogs->pActDialog=pDialogs->dialog;\
	pDialogs->pActDialog->OnEnter();\
	if(pDialogs->visible) pDialogs->showDialog();

#define CREATE_AND_INIT_DIALOG(variable,class) \
	variable=new class();\
	if ( !(variable) ) ERRORMSG( ERROUTOFMEMORY, "dialogs::Init()", "Unsuccessful variable Creation." );\
	hr = variable->init(pGraphic,pResourceManager,pTimer,hWind);\
	if(hr) ERRORMSG(  hr, "GraphicTest::Init()","class init error");\
	Dialogs.push_back( (CMyDialog*) (variable) );

#define GETDIALOGCONTROL(variable,type,strName) \
	variable = (type *)(pGUIDialog->GetControl(#strName));\
	if(! variable) ERRORMSG(ERRGENERIC,"MyDialog::*::GetControl(" #strName ")",#variable " is NULL");

#define GETDIALOGCONTROL2(variable, type) GETDIALOGCONTROL( variable, type, variable )

#define GETDIALOGCONTROL3(variable,type,strName) \
	variable = (type *)(pGUIDialog->GetControl( strName ));\
	if(! variable) ERRORMSG(ERRGENERIC,"MyDialog::*::GetControl(...)",#variable " is NULL");

// For showing message box
#define SHOWMESSAGEBOX(TypeMessage, Message, ReturnToDialog) \
	pDialogs->pActDialog->OnLeave();\
	pDialogs->pMessageBoxDialog->PrepareMessageBox( (TypeMessage), (Message), (ReturnToDialog) ); \
	pDialogs->pActDialog=pDialogs->pMessageBoxDialog; \
	CommonSounds.pPrimarySoundBank->Play( XACT_CUE_CAR_ERROR, 0, 0, NULL );\
	if (pDialogs->visible) pDialogs->showDialog();

class CMyDialog{
	
	public:
		CMyDialog(){
			pGUIDialog = NULL;

		}
		~CMyDialog() {	
			SAFE_DELETE( pGUIDialog );
		}
		graphic::CGUIDialog			*	pGUIDialog;
		graphic::CGraphic			*   pGraphic;
		resManNS::CResourceManager			*	pResourceManager;
		CTimeCounter				*	pTimer;
		HWND							hWind;
		
		virtual HRESULT init(CString Dialog_file,graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);
		virtual void OnLeave();
		virtual void OnEnter();
		// Filters out text and lets only number in string
		void OnlyNumbers(CString & Text);
};
/////////////////////////////////////////////////
// Dialogs:
//////////////////////////////////////////////////

// Message box dialog - is general purpose message box
class CMessageBox: public CMyDialog
{
public:
	virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);
	
	// sets messages and return dialog. 
	void PrepareMessageBox(CString TypeMessage, CString Message, CMyDialog * ReturnDialog);

	graphic::CGUIControlLabel * MBMessageTypeLabel;
	graphic::CGUIControlLabel * MBMessageLabel;
	graphic::CGUIControlButton * MBOkButton;

	// Dialog to return to, when OK button is pressed
	CMyDialog * ReturnToDialog;
};

// Message box dialog - is general purpose message box
class CDedicatedServerDialog: public CMyDialog
{
public:
	virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);
	
	// Starts dedicated timer
	virtual void OnEnter();

	// Stopts dedicated timer
	virtual void OnLeave();

	// Timer procedure
	static void CALLBACK DedicatedServerTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time);
	static void CALLBACK DedicatedServerGameUpdateTimerProc(HWND Hwnd, UINT Msg,UINT_PTR IdEvent, DWORD Time);

	graphic::CGUIControlLabel * DSServerRunningLabel;
	graphic::CGUIControlButton * DSStopServerButton;
	graphic::CGUIControlButton * DSToLobbyButton;

	// Timer
	UINT_PTR Timer;

	UINT_PTR GameUpdateTimer;
};

// Dialog as race result, shows result of race to the player
class CRaceResultDialog: public CMyDialog
{
public:
	virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);

	// On enter of race results
	virtual void OnEnter();

	// Kills timer
	virtual void OnLeave();

	// Clears names in race results
	void ClearRaceResults();

	// Updates race results
	void UpdateRaceResults();

	// Updates records
	void UpdateRecords();

	// Converts time in miliseconds into strinf formated: minutes:seconds.hundreths of seconds
	CString ConvertTime(APPRUNTIME Time);

	// Timer procedure
	static void CALLBACK RaceResultTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time);

	// labels
	graphic::CGUIControlLabel * RRRaceResultLabel;
	graphic::CGUIControlLabel * RRPositionLabel;
	graphic::CGUIControlLabel * RRPosLabel[8];
	graphic::CGUIControlLabel * RRPlayerNameLabel;
	graphic::CGUIControlLabel * RRNameLabel[8];
	graphic::CGUIControlLabel * RRBestLapTimeLabel;
	graphic::CGUIControlLabel * RRBestLapLabel[8];
	graphic::CGUIControlLabel * RRRaceTimeLabel;
	graphic::CGUIControlLabel * RRRaceLabel[8];
	graphic::CGUIControlLabel * RRLapRecordLabel;
	graphic::CGUIControlLabel * RRLapRecordPlayerLabel;
	graphic::CGUIControlLabel * RRLapRecordTimeLabel;
	graphic::CGUIControlLabel * RRRaceRecordLabel;
	graphic::CGUIControlLabel * RRRaceRecordPlayerLabel;
	graphic::CGUIControlLabel * RRRaceRecordTimeLabel;
	// buttons
	graphic::CGUIControlButton * RRToLobbyButton;
	graphic::CGUIControlButton * RRToMainMenuButton;

	// Record times and names
	APPRUNTIME RecordLapTime;
	APPRUNTIME RecordRaceTime;
	CString RecordLapPlayerName;
	CString RecordRacePlayerName;
	int MapEntry, LapEntry;

	// Timer
	UINT_PTR Timer;
};

// To lobby wait dialog - because server must return first to lobby, others must wait
class CToLobbyWaitDialog: public CMyDialog
{
public:
	virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);
	
	// On enter of to lobby waiting
	virtual void OnEnter();

	// Kills timer
	virtual void OnLeave();

	// Timer procedure
	static void CALLBACK ToLobbyWaitTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time);

	// labels
	graphic::CGUIControlLabel * LWTopLabel;
	graphic::CGUIControlLabel * LWWaitingLabel;
	// button
	graphic::CGUIControlButton * LWToMainMenuButton;

	// Timer
	UINT_PTR Timer;
};

// Multiplayer wait dialog - is dialog for multiplayer waiting 
class CMultiplayerWaitDialog: public CMyDialog
{
public:
	virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);

	// Set messages along mode
	void SetMode(BOOL _Server, BOOL _Dedicated);

	// Starts timer for checking connection process status
	virtual void OnEnter();

	// Kills times
	virtual void OnLeave();

	// Timer procedure
	static void CALLBACK MultiplayerWaitTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time);

	graphic::CGUIControlLabel * MWMessageTypeLabel;
	graphic::CGUIControlLabel * MWMessageLabel;
	graphic::CGUIControlButton * MWCancelButton;

	// Mode - server, client
	BOOL Server;
	// Dedicated
	BOOL Dedicated;

	// Timer
	UINT_PTR Timer;
};

// Credits dialog
class CCreditsDialog: public CMyDialog
{
public:
	virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);

	// Labels
	graphic::CGUIControlLabel * CDCreditsLabel;
	graphic::CGUIControlLabel * CDCrazyAnimalsLabel;
	graphic::CGUIControlLabel * CDPavelCelbaLabel;
	graphic::CGUIControlLabel * CDPavelJandaLabel;
	graphic::CGUIControlLabel * CDRadimKrupickaLabel;
	graphic::CGUIControlLabel * CDRomanMargoldLabel;
	graphic::CGUIControlLabel * CDJiriSofkaLabel;
	graphic::CGUIControlLabel * CDOthersLabel;
	graphic::CGUIControlLabel * CDKarelPrasekLabel;
	graphic::CGUIControlLabel * CDMartinHolecekLabel;
	graphic::CGUIControlLabel * CDTiborPetakLabel;
	graphic::CGUIControlLabel * CDHardlinerRecordingsLabel;
	graphic::CGUIControlLabel * CDJakobFischerLabel;
	graphic::CGUIControlLabel * CDPavelCelbaNoteLabel;
	graphic::CGUIControlLabel * CDPavelJandaNoteLabel;
	graphic::CGUIControlLabel * CDRadimKrupickaNoteLabel;
	graphic::CGUIControlLabel * CDRomanMargoldNoteLabel;
	graphic::CGUIControlLabel * CDJiriSofkaNoteLabel;
	graphic::CGUIControlLabel * CDKarelPrasekNoteLabel;
	graphic::CGUIControlLabel * CDMartinHolecekNoteLabel;
	graphic::CGUIControlLabel * CDTiborPetakNoteLabel;
	graphic::CGUIControlLabel * CDHardlinerRecordingsNoteLabel;
	graphic::CGUIControlLabel * CDJakobFischerNoteLabel;
	// buttons
	graphic::CGUIControlButton * CDOkButton;
};

// Key settings dialog - is interface for key setting in game
class CKeySettingsDialog: public CMyDialog
{
public:
	virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);

	void SetInitialValues(); // Sets correct values when entering dialog
	HRESULT SaveValues(); // Save key settings
	BOOL CheckValue(graphic::CGUIControlEdit * ChangedEdit, WPARAM NewValue); // checks, whether such key mapping is not used elsewhere
	// Labels
	graphic::CGUIControlLabel * KSKeySettingsLabel;
	graphic::CGUIControlLabel * KSAccelerateLabel;
	graphic::CGUIControlLabel * KSDecelerateLabel;
	graphic::CGUIControlLabel * KSSteerLeftLabel;
	graphic::CGUIControlLabel * KSSteerRightLabel;
	graphic::CGUIControlLabel * KSBrakeLabel;
	graphic::CGUIControlLabel * KSShiftUpLabel;
	graphic::CGUIControlLabel * KSShiftDownLabel;
	graphic::CGUIControlLabel * KSChangeViewLabel;
	//graphic::CGUIControlLabel * KSDriverViewLabel;
	graphic::CGUIControlLabel * KSRestartLabel;
	//graphic::CGUIControlLabel * KSGamePauseLabel;

	// edit boxes
	graphic::CGUIControlEdit * KSAccelerateEdit;
	graphic::CGUIControlEdit * KSDecelerateEdit;
	graphic::CGUIControlEdit * KSSteerLeftEdit;
	graphic::CGUIControlEdit * KSSteerRightEdit;
	graphic::CGUIControlEdit * KSBrakeEdit;
	graphic::CGUIControlEdit * KSShiftUpEdit;
	graphic::CGUIControlEdit * KSShiftDownEdit;
	graphic::CGUIControlEdit * KSChangeViewEdit;
	//graphic::CGUIControlEdit * KSDriverViewEdit;
	graphic::CGUIControlEdit * KSRestartEdit;
	//graphic::CGUIControlEdit * KSGamePauseEdit;	


	// buttons
	graphic::CGUIControlButton * KSOk;
	graphic::CGUIControlButton * KSCancel;

	// stores old value
	CString OldValue;
};

// Key settings dialog - is interface for key setting in game
class CCustomKeySetsDialog: public CMyDialog
{
public:
	virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);

	virtual void OnEnter() { ResetDialog(); };

	void SetInitialValues(); // Sets correct values when entering dialog
	HRESULT SaveValues(); // Save key settings
	BOOL CheckValue(graphic::CGUIControlEdit * ChangedEdit, WPARAM NewValue); // checks, whether such key mapping is not used elsewhere

	// Labels
	graphic::CGUIControlLabel * CKSMainLabel;
	graphic::CGUIControlLabel * CKSKeyLabel[MAX_CUSTOM_KEYS];

	// edit boxes
	graphic::CGUIControlEdit * CKSKeyEdit[MAX_CUSTOM_KEYS];

	// buttons
	graphic::CGUIControlButton * CKSOk;
	graphic::CGUIControlButton * CKSCancel;

	// stores old value
	CString			OldValue;

	// store the dialog which was used to came to this one
	CMyDialog	*	pReturnDialog;

private:
	void	ResetDialog();

};

// Opponents dialog
// For setting of opponents
class COpponentsDialog: public CMyDialog
{
public:
	virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);
	void SetInitialValues(); // Sets correct values when entering dialog
	void AddOpponent(); // Adds selected car as next opponent
	void DeleteOpponent(); // Delete selected opponent
	HRESULT SaveValues(); // Saves values

	// Labels
	graphic::CGUIControlLabel * ODOpponentsSettingLabel;
	graphic::CGUIControlLabel * ODOpponentsLabel;
	graphic::CGUIControlLabel * ODCarsLabel;
	
	// List box
	graphic::CGUIControlListBox * ODOpponentsListBox;
	graphic::CGUIControlListBox * ODCarsListBox;

	// buttons
	graphic::CGUIControlButton * ODLeftArrowButton;
	graphic::CGUIControlButton * ODRightArrowButton;
	graphic::CGUIControlButton * ODOkButton;
	graphic::CGUIControlButton * ODCancelButton;

	// Dialog to return to, when cancel button is pressed
	CMyDialog * ReturnDialog;

	
};


// Single player dialog
// For start of single player game 
// Player can set:
// Racing map
// Car
// Time of day
// Player name
// Number of laps or time until end of race
// Opponents settings button
class CSinglePlayerDialog: public CMyDialog
{
public:
	virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);

	void SetInitialValues(); // sets correct values when entering dialog
	HRESULT SaveValues(); // saves values
	HRESULT CheckValues(); // checks, whether setting is correct

	// Labels
	graphic::CGUIControlLabel * SPSinglePlayerLabel;
	graphic::CGUIControlLabel * SPPlayerNameLabel;
	graphic::CGUIControlLabel * SPCarLabel;
	graphic::CGUIControlLabel * SPMapLabel;
	graphic::CGUIControlLabel * SPTimeLabel;
	graphic::CGUIControlLabel * SPCircuitModeLabel;
	graphic::CGUIControlLabel * SPLapsLabel;
	graphic::CGUIControlLabel * SPOpponentsLabel;

	// List boxes
	graphic::CGUIControlListBox * SPMapListBox;
	graphic::CGUIControlListBox * SPCarListBox;

	// Edit boxes
	graphic::CGUIControlEdit * SPPlayerNameEditBox;
	graphic::CGUIControlEdit * SPDayHoursEditBox;
	graphic::CGUIControlEdit * SPDayMinutesEditBox;
	graphic::CGUIControlEdit * SPLapsEditBox;

	// Check boxes buttons
	graphic::CGUIControlCheckBox * SPLapsModeCheckBox;

	// buttons
	graphic::CGUIControlButton * SPStartButton;
	graphic::CGUIControlButton * SPBackButton;
	graphic::CGUIControlButton * SPOpponentsButton;

};


// Graphic settings dialog - is interface for graphic setting in game
// Can be set:
// Resolution, Gamma
// Terrain, Models, Grass & Texture quality
// Shadows & Per-Pixel lighting flags
class CGraphicSettingsDialog: public CMyDialog
{
	public:
	virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);

	void SetInitialValues(); // Sets correct values when entering dialog

	// Labels
	graphic::CGUIControlLabel * GSGraphicSettingsLabel;
	graphic::CGUIControlLabel * GSResolutionLabel;
	graphic::CGUIControlLabel * GSGammaLabel;
	graphic::CGUIControlLabel * GSTerrainLabel;
	graphic::CGUIControlLabel * GSModelsLabel;
	graphic::CGUIControlLabel * GSGrassLabel;
	graphic::CGUIControlLabel * GSTextureLabel;
	graphic::CGUIControlLabel * GSAnisoLabel;
	graphic::CGUIControlLabel * GSAntialiasLabel;
	graphic::CGUIControlLabel * GSShadowsLabel;
	graphic::CGUIControlLabel * GSSpecularLabel;

	// combo-boxes
	graphic::CGUIControlComboBox * GSResolutionBox;
	graphic::CGUIControlComboBox * GSTerrainBox;
	graphic::CGUIControlComboBox * GSModelsBox;
	graphic::CGUIControlComboBox * GSGrassBox;
	graphic::CGUIControlComboBox * GSTextureBox;
	graphic::CGUIControlComboBox * GSAnisoBox;
	graphic::CGUIControlComboBox * GSAntialiasBox;

	// trackbar
	graphic::CGUIControlTrackBar * GSGammaTrackBar;

	// check box
	graphic::CGUIControlCheckBox * GSShadowsBox;
	graphic::CGUIControlCheckBox * GSSpecularBox;

	// buttons
	graphic::CGUIControlButton * GSOk;
	graphic::CGUIControlButton * GSCancel;
};

// Sound settings dialog - is interface for sound & music setting in game
// Can be set:
// Music volume - where zero volume is music off
class CSoundSettingsDialog: public CMyDialog
{
public:
	virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);

	void SetInitialValues(); // Sets correct values when entering dialog
	// Labels
	graphic::CGUIControlLabel * SSSoundSettingsLabel;
	graphic::CGUIControlLabel * SSMusicVolumeLabel;

	// Trackbars
	graphic::CGUIControlTrackBar * SSMusicVolumeTrackBar;

	// buttons
	graphic::CGUIControlButton * SSOk;
	graphic::CGUIControlButton * SSCancel;
};

// Settings dialog - contains buttons for advancing to concrete settings dialogs
class CSettingsDialog: public CMyDialog
{
	public:
		virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);
		graphic::CGUIControlLabel * SettingsLabel;
		graphic::CGUIControlButton * GraphicSettings;
		graphic::CGUIControlButton * SoundSettings;
		graphic::CGUIControlButton * KeySettings;
		graphic::CGUIControlButton * MainMenu;

		// Return to dialog
		CMyDialog * ReturnToDialog;
};



class CMainDialog :public CMyDialog
{
	public:
		virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);	
		graphic::CGUIControlLabel	*	MainLabel;
		graphic::CGUIControlButton  *	SingleStartButton;
		graphic::CGUIControlButton	*	MultiStartButton;
		graphic::CGUIControlButton	*	SettingsButton;
		graphic::CGUIControlButton	*	ExitButton;
		graphic::CGUIControlButton  *   CreditsButton;
		
		
};

// Multiplayer basic dialog
// Has joining and creating server option
class CMultiplayerDialog :public CMyDialog
{
	public:
		virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);
		
		// Refreshes server list
		HRESULT RefreshList(CClient * pCl);
		// labels
		graphic::CGUIControlLabel * MPMultiplayerLabel;
		graphic::CGUIControlLabel * MPGameListLabel;
		graphic::CGUIControlLabel * MPIPLabel;
		graphic::CGUIControlLabel * MPPortLabel;
		// edit
		graphic::CGUIControlEdit * MPIPEdit;
		graphic::CGUIControlEdit * MPPortEdit;
		// list
		graphic::CGUIControlListBox * MPGameListList;
		// buttons
		graphic::CGUIControlButton * MPCreateButton;
		graphic::CGUIControlButton * MPJoinButton;
		graphic::CGUIControlButton * MPBackButton;

		// Actual server list
		vector<CClient::DialogServerInfo> ServerList;
		
		virtual void OnEnter();
		virtual void OnLeave();
		static void CALLBACK BroadcastTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time);

		UINT_PTR Timer;
};

// Multiplayer dialog for creating server
// Player types in game name
// Allows player to choose map, AI opponents and dedicated server option
class CMultiCreateServerDialog :public CMyDialog
{
	public:
		virtual HRESULT		init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);	

		void				ReloadMapList();
		
		// Labels
		graphic::CGUIControlLabel	*	MCCreateGameLabel;
		graphic::CGUIControlLabel	*	MCGameNameLabel;
		graphic::CGUIControlLabel	*	MCDedicatedLabel;
		graphic::CGUIControlLabel	*	MCMapLabel;
		graphic::CGUIControlLabel	*	MCMutatorsLabel;
		// Edit
		graphic::CGUIControlEdit	*	MCGameNameEdit;
		// List box
		graphic::CGUIControlListBox *	MCMapListBox;
		// combo box
		graphic::CGUIControlComboBox *	MCGameTypeCombo;
		graphic::CGUIControlComboBox *	MCMutatorsCombo;
		// check box
		graphic::CGUIControlCheckBox *	MCDedicatedCheckBox;
		// image
		graphic::CGUIControlImage	*	MCMapImage;
		// buttons
		graphic::CGUIControlButton	*	MCOpponentsButton;
		graphic::CGUIControlButton	*	MCControlsButton;
		graphic::CGUIControlButton	*	MCAdvancedButton;
		graphic::CGUIControlButton	*	MCStartServerButton;
		graphic::CGUIControlButton	*	MCBackButton;

};

// Multiplayer dialog for player setting
// Allows player to type in his name and choose racing car
class CMultiPlayerSettingsDialog: public CMyDialog
{
public:
	virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);	

	// Sets game name
	inline void SetGameName(CString & _GameName)
	{
		GameName = _GameName;
		MSGameNameLabel->SetValueSilent(GameName);
	}

	// Sets initial values from client multiplayer game info
	void SetInitialValues();

	// Labels
	graphic::CGUIControlLabel * MSPlayerSettingsLabel;
	graphic::CGUIControlLabel * MSGameLabel;
	graphic::CGUIControlLabel * MSGameNameLabel;
	graphic::CGUIControlLabel * MSPlayerNameLabel;
	graphic::CGUIControlLabel * MSCarLabel;
	graphic::CGUIControlLabel * MSTeamLabel;
	// edit box
	graphic::CGUIControlEdit * MSPlayerNameEdit;
	// list box
	graphic::CGUIControlListBox * MSCarListBox;
	// combo
	graphic::CGUIControlComboBox * MSTeamComboBox;
	// Buttons
	graphic::CGUIControlButton * MSOkButton;
	graphic::CGUIControlButton * MSCancelButton;

	// Game name
	CString GameName;
};

class CMultiStartGameDialog :public CMyDialog
{
	public:
		virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);	
		
		// Set mode, whether it's in server mode or not
		// Whether server is dedicated or not
		void SetMode(BOOL _Server, BOOL _Dedicated);

		// Starts timer and update
		virtual void OnEnter();

		// Ends timer
		virtual void OnLeave();

		// Updates lobby
		void Update();

		// Timer procedure
		static void CALLBACK LobbyTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time);

		// labels
		graphic::CGUIControlLabel * MGLobbyLabel;
		graphic::CGUIControlLabel * MGGameLabel;
		graphic::CGUIControlLabel * MGGameNameLabel;
		graphic::CGUIControlLabel * MGMapLabel;
		graphic::CGUIControlLabel * MGMapNameLabel;
		graphic::CGUIControlLabel * MGLapsLabel;
		graphic::CGUIControlLabel * MGTimeOfDayLabel;
		graphic::CGUIControlLabel * MGDoubleDotLabel;
		graphic::CGUIControlLabel * MGPlayer1Label;
		graphic::CGUIControlLabel * MGCar1Label;
		graphic::CGUIControlLabel * MGReady1Label;
		graphic::CGUIControlLabel * MGPlayer2Label;
		graphic::CGUIControlLabel * MGCar2Label;
		graphic::CGUIControlLabel * MGReady2Label;
		// Player names labels
		graphic::CGUIControlLabel * MGPlayerNameLabel[8];
		// Player car names labels
		graphic::CGUIControlLabel * MGCarNameLabel[8];
		
		// Check boxes
		graphic::CGUIControlCheckBox * MGLapsCheckBox;
		// player ready status
		graphic::CGUIControlCheckBox * MGPlayerReadyCheckBox[8];
		
		// Edit boxes
		graphic::CGUIControlEdit * MGLapsEdit;
		graphic::CGUIControlEdit * MGDayHoursEdit;
		graphic::CGUIControlEdit * MGDayMinutesEdit;
		graphic::CGUIControlEdit * MGChatEdit;

		// list box
		graphic::CGUIControlListBox * MGChatList;

		// Buttons
		graphic::CGUIControlButton * MGSendButton;
		graphic::CGUIControlButton * MGStartGameButton;
		graphic::CGUIControlButton * MGPlayerSettingsButton;
		graphic::CGUIControlButton * MGLeaveButton;

		// Is it server
		BOOL Server;

		// Is server dedicated
		BOOL Dedicated;

		// Timer
		UINT_PTR Timer;
};

class CWaitDialog :public CMyDialog
{
	public:
		// Starts timer
		virtual void OnEnter();

		// Ends timer
		virtual void OnLeave();

		static void CALLBACK WaitDialogTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time);
		// callback function for reporting loading status
		static void CALLBACK LoadingProgress(CString Progress);

		virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);	
		graphic::CGUIControlLabel	*	MainLabel;
		graphic::CGUIControlLabel	*	ProgressLabel;
		graphic::CGUIControlLabel   *   DotLabel;

		UINT_PTR Timer;
};

class CInGameDialog :public CMyDialog
{
	public:
		virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);	
		graphic::CGUIControlButton	*	ExitButton;
		graphic::CGUIControlButton	*	SettingsButton;
		graphic::CGUIControlButton	*	CloseButton;
		
		
};
class CStartDialog: public CMyDialog
{
public:
	virtual HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*,CTimeCounter *,HWND);

	// Labels
	graphic::CGUIControlLabel * CDLabel;
	
};

class CMyDialogs{

	public:	
		
		graphic::CGUIDialog			*	pGUIDialog;
		graphic::CGraphic			*   pGraphic;
		resManNS::CResourceManager			*	pResourceManager;
		CTimeCounter				*	pTimer;
		HWND							hWind;
		BOOL							allowkeystogame;
		int								PrimaryMCursor;
		
		void showDialog();
		void hideDialog();

		void OnLostDevice(); // Must be called when D3D device is lost
		void OnResetDevice();  // Must be called after D3D device is reset
		
		void RecenterDialogs(); // Places dialogs into the center according to actual resolution

		//graphic::CGUIDialog  * pActDialog;
		CMyDialog	* pActDialog;
		bool visible;

		///////////////////////////////////////////////////
		// HERE add Dialog to Dialogs - use your intuition
		//////////////////////////////////////////////////
		
		CMyDialogs(){
			allowkeystogame = FALSE;
			visible = FALSE;
			pMainDialog = NULL;
			pWaitDialog = NULL;
			pInGameDialog = NULL;
			pMultiplayerDialog =NULL;
			pMultiCreateServerDialog=NULL;
			pMultiStartGameDialog=NULL;
			pMultiPlayerSettingsDialog=NULL;
			pSettingsDialog=NULL;
			pGraphicSettingsDialog=NULL;
			pSoundSettingsDialog=NULL;
			pKeySettingsDialog=NULL;
			pSinglePlayerDialog=NULL;
			pMessageBoxDialog=NULL;
			pOpponentsDialog=NULL;
			pRaceResultDialog=NULL;
			pToLobbyWaitDialog=NULL;
			pCreditsDialog = NULL;
			pStartDialog = NULL;
		}
		~CMyDialogs(){
			SAFE_DELETE(pMainDialog);
			SAFE_DELETE(pWaitDialog);
			SAFE_DELETE(pInGameDialog);
			SAFE_DELETE(pMultiplayerDialog);
			SAFE_DELETE(pMultiCreateServerDialog);
			SAFE_DELETE(pMultiStartGameDialog);
			SAFE_DELETE(pMultiPlayerSettingsDialog);
			SAFE_DELETE(pSettingsDialog);
			SAFE_DELETE(pGraphicSettingsDialog);
			SAFE_DELETE(pSoundSettingsDialog);
			SAFE_DELETE(pKeySettingsDialog);
			SAFE_DELETE(pSinglePlayerDialog);
			SAFE_DELETE(pMessageBoxDialog);
			SAFE_DELETE(pOpponentsDialog);
			SAFE_DELETE(pMultiplayerWaitDialog);
			SAFE_DELETE(pDedicatedServerDialog);
			SAFE_DELETE(pRaceResultDialog);
			SAFE_DELETE(pToLobbyWaitDialog);
			SAFE_DELETE(pCreditsDialog);
			SAFE_DELETE(pStartDialog);
		}

		HRESULT init(graphic::CGraphic * _pGraphic,resManNS::CResourceManager	*_pRM,CTimeCounter *_pTimer,HWND _hWind){
			HRESULT hr;pGraphic = _pGraphic;pResourceManager = _pRM;pTimer= _pTimer;hWind= _hWind;
			CREATE_AND_INIT_DIALOG(pMainDialog,CMainDialog);
			CREATE_AND_INIT_DIALOG(pWaitDialog,CWaitDialog);
			CREATE_AND_INIT_DIALOG(pInGameDialog,CInGameDialog);
			CREATE_AND_INIT_DIALOG(pMultiplayerDialog,CMultiplayerDialog);
			CREATE_AND_INIT_DIALOG(pMultiCreateServerDialog,CMultiCreateServerDialog);
			CREATE_AND_INIT_DIALOG(pMultiStartGameDialog,CMultiStartGameDialog);
			CREATE_AND_INIT_DIALOG(pMultiPlayerSettingsDialog, CMultiPlayerSettingsDialog);
			CREATE_AND_INIT_DIALOG(pSettingsDialog, CSettingsDialog);
			CREATE_AND_INIT_DIALOG(pGraphicSettingsDialog, CGraphicSettingsDialog);
			CREATE_AND_INIT_DIALOG(pSoundSettingsDialog, CSoundSettingsDialog);
			CREATE_AND_INIT_DIALOG(pKeySettingsDialog, CKeySettingsDialog);
			CREATE_AND_INIT_DIALOG(pCustomKeySetsDialog, CCustomKeySetsDialog);
			CREATE_AND_INIT_DIALOG(pSinglePlayerDialog, CSinglePlayerDialog);
			CREATE_AND_INIT_DIALOG(pMessageBoxDialog, CMessageBox);
			CREATE_AND_INIT_DIALOG(pOpponentsDialog, COpponentsDialog);
			CREATE_AND_INIT_DIALOG(pMultiplayerWaitDialog, CMultiplayerWaitDialog);
			CREATE_AND_INIT_DIALOG(pDedicatedServerDialog, CDedicatedServerDialog);
			CREATE_AND_INIT_DIALOG(pRaceResultDialog, CRaceResultDialog);
			CREATE_AND_INIT_DIALOG(pToLobbyWaitDialog, CToLobbyWaitDialog);
			CREATE_AND_INIT_DIALOG(pCreditsDialog, CCreditsDialog);
			CREATE_AND_INIT_DIALOG(pStartDialog, CStartDialog);
			return ERRNOERROR;

		}
		
		CMessageBox * pMessageBoxDialog; // General purpose message box
		CMultiplayerWaitDialog * pMultiplayerWaitDialog;
		CMultiplayerDialog * pMultiplayerDialog;
		CMainDialog * pMainDialog;
		CWaitDialog * pWaitDialog;
		CInGameDialog * pInGameDialog;
		CMultiCreateServerDialog * pMultiCreateServerDialog;
		CMultiStartGameDialog * pMultiStartGameDialog;
		CMultiPlayerSettingsDialog * pMultiPlayerSettingsDialog;
		CSettingsDialog * pSettingsDialog;
		CGraphicSettingsDialog * pGraphicSettingsDialog;
		CSoundSettingsDialog * pSoundSettingsDialog;
		CKeySettingsDialog * pKeySettingsDialog;
		CCustomKeySetsDialog * pCustomKeySetsDialog;
		CSinglePlayerDialog * pSinglePlayerDialog;
		COpponentsDialog * pOpponentsDialog;
		CDedicatedServerDialog * pDedicatedServerDialog;
		CRaceResultDialog * pRaceResultDialog;
		CToLobbyWaitDialog * pToLobbyWaitDialog;
		CCreditsDialog * pCreditsDialog;
		CStartDialog * pStartDialog;
		


	private:
		list<CMyDialog * > Dialogs; // all dialogs

};




//////////////////////////////////////////////////////////////////////////////////////////////
//
// Global functions.
//
//////////////////////////////////////////////////////////////////////////////////////////////
void ReloadMapList( graphic::CGUIControlListBox * pList );
