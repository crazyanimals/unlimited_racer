#include "stdafx.h"
#include "main.h"
#include "dialogs.h"
#include "controls.h"
#include "..\Graphic\GUIDialog.h"
#include "..\Graphic\Graphic.h"
#include "initFunctions.h"
#include "..\Client\GameInfo.h"
#include "..\Server\CServer.h"


using namespace graphic;


extern CRITICAL_SECTION	g_csServerList;
extern int gameState;
extern CMyDialogs * pDialogs;
extern HWND hWnd;
extern CGameInput gInput;
extern CGameInfo  GameInfo;
extern CClient *Client;
extern CServer * Server;
extern CMODWorkshop	GameModsWrk;



///////////////////////////////////////////
// Screenshot capture function
////////////////////////////////////////////
HRESULT CaptureScreenShot()
{
	HRESULT hr;
	CString FileName;
	char Num[20];
	NAMEVALUEPAIR Pair;

	// Save screen shot
	FileName = "ScreenShot" + CString(_itoa(Configuration.ScreenShotNumber, Num, 10) ) + CString(".bmp");
	pDialogs->pGraphic->CaptureScreenShot(FileName);
	// save new screen shot number
	Configuration.ScreenShotNumber++;
	Pair.SetName("ScreenShotNumber");
	Pair.SetInt((int) Configuration.ScreenShotNumber);
	hr = Pair.SaveValue("Options.cfg");
	if (FAILED(hr) )
		ERRORMSG(hr, "CaptureScreenShot()", "Unable to save new screen shot number.");

	return ERRNOERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
// ReloadMapList function clears the 'pList' listbox and adds valid map names into it.
// All maps added this way are those that are supported by the active (actual) game MOD.
// Every item added is signed by an index of particular item in the original full map list.
// Notice that the game MOD must be already initialized so we can create the map list properly.
//
//////////////////////////////////////////////////////////////////////////////////////////////
void ReloadMapList( CGUIControlListBox * pList )
{
	std::vector<CGameInfo::MapShortInfo>::iterator It;
	int		index;

	pList->Clear();
	if ( !pGameMod ) return; // no game MOD chosen yet, we can not initialize maps
	index = 0;

	// check all available maps, but add just those that supports our MOD (the active one)
	for ( It = GameInfo.MapNames.begin(); It != GameInfo.MapNames.end(); It++, index++ )
	{
		// look for the particular module ID (the one active now)
		for ( int i = 0; i < (int) It->ModsList.size(); i++ )
			if ( It->ModsList[i] == pGameMod->GetModulePrimaryID() )
			{
				// module ID found -> this map supports our game type (MOD), lets add this map to the list
				//  and accociated an index with it, the index points to the full map list
				pList->PushBack( It->Name, index );
				break;
			}
	}
	
	pList->SetValue( 0 );
};


///////////////////////////////////
// callback functions
//////////////////////////////////

//////////////////////////////////
// Message box
HRESULT CALLBACK OnMBOkButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	// return to return to dialog
	pDialogs->pActDialog->OnLeave();
	pDialogs->pActDialog= pDialogs->pMessageBoxDialog->ReturnToDialog;
	pDialogs->pActDialog->OnEnter();
	if(pDialogs->visible) pDialogs->showDialog();

	return ERRNOERROR;
}

//////////////////////////////////////
// Multiplayer wait dialog
HRESULT CALLBACK OnMWCancelButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	if (pDialogs->pMultiplayerWaitDialog->Server)
		Server->ChangeServerState(0);
	if (! pDialogs->pMultiplayerWaitDialog->Dedicated)
		Client->ChangeState(CS_MG_ENUMERATE_HOSTS);

	CHANGE_ACTDIALOG(pMultiplayerDialog);
	return ERRNOERROR;
}

////////////////////////////////////
// Opponents dialog
HRESULT CALLBACK OnODOkButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	HRESULT hr;
	hr = pDialogs->pOpponentsDialog->SaveValues();
	if (FAILED(hr) ) ERRORMSG(hr, "OnODOkButtonClick", "Unable to save configuration of opponents");
	
	// return to return to dialog
	pDialogs->pActDialog = pDialogs->pOpponentsDialog->ReturnDialog;
	if (pDialogs->visible) pDialogs->showDialog();

	return ERRNOERROR;
}

 HRESULT CALLBACK OnODCancelButtonClick(CGUIControlBase * pObject, LPARAM lParam)
 {
	// return to return to dialog
	pDialogs->pActDialog = pDialogs->pOpponentsDialog->ReturnDialog;
	if (pDialogs->visible) pDialogs->showDialog();

	return ERRNOERROR;
 }	

 HRESULT CALLBACK OnODLeftArrowButtonClick(CGUIControlBase * pObject, LPARAM lParam)
 {
	 // Adds selected car as next opponent
	pDialogs->pOpponentsDialog->AddOpponent();

	return ERRNOERROR;
 }

 HRESULT CALLBACK OnODRightArrowButtonClick(CGUIControlBase * pObject, LPARAM lParam)
 {
	// Delete selected opponent in opponent list
	 pDialogs->pOpponentsDialog->DeleteOpponent();

	return ERRNOERROR;
 }

//////////////////////////////////////////
// Race result dialog
HRESULT CALLBACK  OnRRToMainMenuButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	FreeGamePlayResources();
	CHANGE_ACTDIALOG(pMainDialog);
	
	return ERRNOERROR;	
}
 
HRESULT CALLBACK  OnRRToLobbyButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	// Is this server???
	if (Server->GetServerState() == SS_GAME_PLAY)
	{
		// Send to all, that server returned to lobby and all others must also
		Server->SendServerReturnedToLobby();

		// Only clear resources directly involved with game
		// Do not clear all multiplayer resources
		FreeGamePlayResources(TRUE);

		CHANGE_ACTDIALOG(pMultiStartGameDialog);
	}
	else
	{
		// Client
		// Send request to return to lobby
		Client->SendReturnToLobbyRequest();
		CHANGE_ACTDIALOG(pToLobbyWaitDialog);
	}

	return ERRNOERROR;
}


//////////////////////////////////
// Single player dialog
HRESULT CALLBACK  OnSPStartButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	HRESULT hr;
	hr = pDialogs->pSinglePlayerDialog->CheckValues();
	if (!hr)
	{
		hr = pDialogs->pSinglePlayerDialog->SaveValues();
		if (FAILED(hr) ) 
		{
			SHOWMESSAGEBOX("ERROR", "ERROR WHILE SAVING SINGLE PLAYER SETTINGS", pDialogs->pSinglePlayerDialog);
			// Unload all game play resources
			FreeGamePlayResources();
			ERRORMSG(hr, "OnSPStartButtonClick", "Unable to save single player configuration.");
		}

		hr = startSinglePlayer();
		if (FAILED(hr) )
		{
			// Unload all game play resources
			FreeGamePlayResources();
			SHOWMESSAGEBOX("ERROR", "THERE WAS AN ERROR WHILE LOADING SINGLE PLAYER GAME", pDialogs->pSinglePlayerDialog);
		}
	}

	return ERRNOERROR;	
}

HRESULT CALLBACK  OnSPBackButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	CHANGE_ACTDIALOG(pMainDialog);
	return ERRNOERROR;	
}

HRESULT CALLBACK  OnOnlyNumbers(CGUIControlBase * pObject, LPARAM lParam)
{
	CString Value;
	graphic::CGUIControlEdit * EditBox;
	EditBox = (graphic::CGUIControlEdit*) pObject;
	Value = EditBox->GetValue();
	pDialogs->pSinglePlayerDialog->OnlyNumbers(Value);
	EditBox->SetValueSilent(Value);
	
	return ERRNOERROR;	
}

HRESULT CALLBACK  OnOpponentsButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	pDialogs->pOpponentsDialog->SetInitialValues();
	pDialogs->pOpponentsDialog->ReturnDialog = pDialogs->pSinglePlayerDialog;
	CHANGE_ACTDIALOG(pOpponentsDialog);
	return ERRNOERROR;
}

HRESULT CALLBACK  OnMapListBoxChange(CGUIControlBase * pObject, LPARAM lParam)
{
	graphic::CGUIControlListBox * ListBox;
	CString MapFileName;
	resManNS::RESOURCEID MapID;
	resManNS::Map * MapObj;
	char DayTime[20];
	

	ListBox = (graphic::CGUIControlListBox *) pObject;
	
	if (ListBox->GetLinesCount() == 0) return ERRNOERROR;
	
	// Load map
	MapFileName = GameInfo.MapNames[ListBox->GetActualItem().iValue].FileName;	
	MapID = pDialogs->pResourceManager->LoadResource(MapFileName, RES_Map, false, 0);
	if (FAILED(MapID) )
		return ERRNOERROR;
	
	MapObj = pDialogs->pResourceManager->GetMap(MapID);
		
	// Set default day time 
	_itoa(  (int) (MapObj->fDayTime * 24), DayTime, 10);
	pDialogs->pSinglePlayerDialog->SPDayHoursEditBox->SetValue( DayTime);
	_itoa( (int) (fmod(MapObj->fDayTime * 24, 1.0f) * 60), DayTime, 10);
	pDialogs->pSinglePlayerDialog->SPDayMinutesEditBox->SetValue( DayTime);
	// Free resource
	pDialogs->pResourceManager->ReleaseResource(MapID);
	
	return ERRNOERROR;
}


///////////////////////////////////
// Credits dialog
HRESULT CALLBACK  OnCDOkButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	CHANGE_ACTDIALOG(pMainDialog);
	return ERRNOERROR;
}

////////////////////////////////
// Key settings dialog
HRESULT CALLBACK  OnKSOkClick(CGUIControlBase * pObject, LPARAM lParam)
{	
	HRESULT hr;
	hr = pDialogs->pKeySettingsDialog->SaveValues();
	if (FAILED(hr) ) ERRORMSG(hr, "OnKSOkClick", "Unable to save key settings");

	CHANGE_ACTDIALOG(pSettingsDialog);
	return ERRNOERROR;
}

HRESULT CALLBACK  OnKSCancelClick(CGUIControlBase * pObject, LPARAM lParam)
{

	CHANGE_ACTDIALOG(pSettingsDialog);
	return ERRNOERROR;
}

HRESULT CALLBACK  OnKSKeyChange(CGUIControlBase * pObject, LPARAM lParam)
{
	graphic::CGUIControlEdit * EditBox;
	
	EditBox = (graphic::CGUIControlEdit *) pObject;
	EditBox->SetValueSilent(" ");


	return ERRNOERROR;
}



HRESULT CALLBACK  OnKSKeyUp(CGUIControlBase * pObject, LPARAM lParam)
{
	graphic::CGUIControlEdit * EditBox;
	MESSAGEPARAMSSTRUCT	params;

	EditBox = (graphic::CGUIControlEdit *) pObject;

	params = *((MESSAGEPARAMSSTRUCT *) lParam);

	// Check whether value is not set for another action
	if ( !pDialogs->pKeySettingsDialog->CheckValue(EditBox, params.wParam) )
		EditBox->SetValueSilent( Configuration.UnParseKeyBindConfigPair((int) params.wParam) );
	else 
		EditBox->SetValueSilent( pDialogs->pKeySettingsDialog->OldValue);

	return ERRNOERROR;
}

HRESULT CALLBACK OnKSKeyDown(CGUIControlBase * pObject, LPARAM lParam)
{
	graphic::CGUIControlEdit * EditBox;
	
	EditBox = (graphic::CGUIControlEdit *) pObject;
	
	// store old value in case its needed to be restored
	pDialogs->pKeySettingsDialog->OldValue = EditBox->GetValue();

	return ERRNOERROR;
}



//////////////////////////////////
// custom key settings dialog
HRESULT CALLBACK  OnCKSOkClick(CGUIControlBase * pObject, LPARAM lParam)
{	
	HRESULT hr;
	hr = pDialogs->pCustomKeySetsDialog->SaveValues();
	if ( FAILED(hr) ) ERRORMSG( hr, "OnCKSOkClick", "Unable to save custom key settings." );

	pDialogs->pActDialog->OnLeave();
	pDialogs->pActDialog = pDialogs->pCustomKeySetsDialog->pReturnDialog;
	pDialogs->pActDialog->OnEnter();
	if ( pDialogs->visible ) pDialogs->showDialog();

	return ERRNOERROR;
};

HRESULT CALLBACK  OnCKSCancelClick(CGUIControlBase * pObject, LPARAM lParam)
{
	pDialogs->pActDialog->OnLeave();
	pDialogs->pActDialog = pDialogs->pCustomKeySetsDialog->pReturnDialog;
	pDialogs->pActDialog->OnEnter();
	if ( pDialogs->visible ) pDialogs->showDialog();

	return ERRNOERROR;
};

HRESULT CALLBACK  OnCKSKeyUp(CGUIControlBase * pObject, LPARAM lParam)
{
	graphic::CGUIControlEdit * EditBox;
	MESSAGEPARAMSSTRUCT	params;

	EditBox = (graphic::CGUIControlEdit *) pObject;

	params = *((MESSAGEPARAMSSTRUCT *) lParam);

	// Check whether value is not set for another action
	if ( !pDialogs->pCustomKeySetsDialog->CheckValue(EditBox, params.wParam) )
		EditBox->SetValueSilent( Configuration.UnParseKeyBindConfigPair((int) params.wParam) );
	else 
		EditBox->SetValueSilent( pDialogs->pCustomKeySetsDialog->OldValue );

	return ERRNOERROR;
};

HRESULT CALLBACK OnCKSKeyDown(CGUIControlBase * pObject, LPARAM lParam)
{
	graphic::CGUIControlEdit * EditBox;
	
	EditBox = (graphic::CGUIControlEdit *) pObject;
	
	// store old value in case its needed to be restored
	pDialogs->pCustomKeySetsDialog->OldValue = EditBox->GetValue();

	return ERRNOERROR;
};




//////////////////////////////////
// Graphic settings dialog
HRESULT CALLBACK  OnGSOkClick(CGUIControlBase * pObject, LPARAM lParam)
{
	HRESULT			hr;
	int				Resolution;
	int				Width = 1024; 
	int				Height = 768;
	int				ShadowLOD = 2;
	NAMEVALUEPAIR	Pair;
	CString			str;
	UINT			TComp, MComp, GComp;

	// for remembering previous setting
	int PreviousWidth;
	int PreviousHeight;
	UINT PreviousAntialiasing;
	UINT PreviousMultisampleQuality;
	int	PreviousRenderGrass;

	// Get old settings
	PreviousWidth = (int) pDialogs->pGraphic->GetWidth();
	PreviousHeight = (int) pDialogs->pGraphic->GetHeight();
	PreviousAntialiasing = pDialogs->pGraphic->GetAntialiasing();
	PreviousMultisampleQuality = pDialogs->pGraphic->GetMultisampleQuality();
	PreviousRenderGrass = Configuration.RenderGrass;


	// save old config
	TComp = Configuration.TerrainComplexity;
	MComp = Configuration.ModelComplexity;
	GComp = Configuration.GrassComplexity;


	// Resolution
	Resolution = pDialogs->pGraphicSettingsDialog->GSResolutionBox->GetValue(); 
	if ( Resolution == 0 ) { Width = 1600; Height = 1200; }
	if ( Resolution == 1 ) { Width = 1280; Height = 1024; }
	if ( Resolution == 2 ) { Width = 1024; Height = 768; }
	if ( Resolution == 3 ) { Width = 800; Height = 600; }
	if ( Resolution == 4 ) { Width = 640; Height = 480; }
	
	Configuration.WindowWidth = Width;
	Configuration.WindowHeight = Height;

	Pair.SetName( "WindowWidth" );
	Pair.SetInt( Width );
	hr = Pair.SaveValue( "Options.cfg" );
	if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save resolution to options.cfg." );
	Pair.SetName( "WindowHeight" );
	Pair.SetInt( Height );
	hr = Pair.SaveValue( "Options.cfg" );
	if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save resolution to options.cfg." );


	// Shadow settings
	if ( pDialogs->pGraphicSettingsDialog->GSShadowsBox->IsChecked() ) ShadowLOD = SC_SIMPLE;
	else ShadowLOD = SC_NONE;
	Configuration.ShadowComplexity = ShadowLOD;
	Pair.SetName( "ShadowComplexity" );
	Pair.SetInt( ShadowLOD );
	hr = Pair.SaveValue( "Options.cfg" );
	if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save shadow complexity setting to options.cfg." );

	
	// Specular per-pixel lighting
	Configuration.CarPerPixelLight = pDialogs->pGraphicSettingsDialog->GSSpecularBox->IsChecked();
	Pair.SetName( "CarPerPixelLight" );
	Pair.SetInt( pDialogs->pGraphicSettingsDialog->GSSpecularBox->IsChecked() );
	hr = Pair.SaveValue( "Options.cfg" );
	if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save setting to options.cfg." );


	// LEVEL OF DETAIL

	// terrain complexity
	Configuration.TerrainComplexity = pDialogs->pGraphicSettingsDialog->GSTerrainBox->GetValue();
	Pair.SetName( "TerrainComplexity" );
	Pair.SetInt( Configuration.TerrainComplexity );
	hr = Pair.SaveValue( "Options.cfg" );
	if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save setting to options.cfg." );

	// models complexity
	Configuration.ModelComplexity = pDialogs->pGraphicSettingsDialog->GSModelsBox->GetValue();
	Pair.SetName( "ModelComplexity" );
	Pair.SetInt( Configuration.ModelComplexity );
	hr = Pair.SaveValue( "Options.cfg" );
	if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save setting to options.cfg." );

	// grass complexity
	if ( pDialogs->pGraphicSettingsDialog->GSGrassBox->GetValue() > 0 )
	{
		Configuration.GrassComplexity = pDialogs->pGraphicSettingsDialog->GSGrassBox->GetValue() - 1;
		Configuration.RenderGrass = 1;
	}
	else Configuration.RenderGrass = 0;
	Pair.SetName( "GrassComplexity" );
	Pair.SetInt( Configuration.GrassComplexity );
	hr = Pair.SaveValue( "Options.cfg" );
	if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save setting to options.cfg." );
	Pair.SetName( "RenderGrass" );
	Pair.SetInt( Configuration.RenderGrass );
	hr = Pair.SaveValue( "Options.cfg" );
	if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save setting to options.cfg." );

	// texture quality
	Configuration.TextureQuality = pDialogs->pGraphicSettingsDialog->GSTextureBox->GetValue();
	Pair.SetName( "TextureQuality" );
	Pair.SetInt( Configuration.TextureQuality );
	hr = Pair.SaveValue( "Options.cfg" );
	if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save setting to options.cfg." );


	// anisotropic filtering
	Configuration.AnisotropicFiltering = 1 << pDialogs->pGraphicSettingsDialog->GSAnisoBox->GetValue();
	if ( Configuration.AnisotropicFiltering == 1 ) Configuration.AnisotropicFiltering = 0;
	Pair.SetName( "AnisotropicFiltering" );
	Pair.SetInt( Configuration.AnisotropicFiltering );
	hr = Pair.SaveValue( "Options.cfg" );
	if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save setting to options.cfg." );

	// antialiasing
	Configuration.Antialiasing = pDialogs->pGraphicSettingsDialog->GSAntialiasBox->GetValue() != 0 ? 1 : 0;
	Configuration.MultisampleQuality = max( 0, pDialogs->pGraphicSettingsDialog->GSAntialiasBox->GetValue() - 1 );
	Pair.SetName( "Antialiasing" );
	Pair.SetInt( Configuration.Antialiasing );
	hr = Pair.SaveValue( "Options.cfg" );
	if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save setting to options.cfg." );
	Pair.SetName( "MultisampleQuality" );
	Pair.SetInt( Configuration.MultisampleQuality );
	hr = Pair.SaveValue( "Options.cfg" );
	if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save setting to options.cfg." );

	
	// gamma factor
	Configuration.GammaFactor = 2.0f * pDialogs->pGraphicSettingsDialog->GSGammaTrackBar->GetValue();
	Pair.SetName( "GammaFactor" );
	Pair.SetFloat( Configuration.GammaFactor );
	hr = Pair.SaveValue( "Options.cfg" );
	if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save setting to options.cfg." );


	// perform some changes immediately
	pDialogs->pGraphic->SetCarPerPixelLight( Configuration.CarPerPixelLight );
	pDialogs->pGraphic->SetShadowComplexity( Configuration.ShadowComplexity );
	pDialogs->pGraphic->SetGamma( Configuration.GammaFactor );
	pDialogs->pGraphic->SetAnisotropicFiltering( Configuration.AnisotropicFiltering );
	if ( !Configuration.RenderGrass ) pDialogs->pGraphic->SetRenderGrass( Configuration.RenderGrass ); // only allow change to FALSE !


	// perform graphic mode change when needed
	if ( pDialogs->pGraphic->GetWidth() != Width || pDialogs->pGraphic->GetHeight() != Height ||
		pDialogs->pGraphic->GetAntialiasing() != Configuration.Antialiasing ||
		pDialogs->pGraphic->GetMultisampleQuality() != Configuration.MultisampleQuality )
	{
		graphic::GRAPHICINIT		grInit;
	
		grInit.hWnd = hWnd;
		grInit.Width = Configuration.WindowWidth;
		grInit.Height = Configuration.WindowHeight;
		grInit.ColorDepth = Configuration.ColorDepth;
		grInit.FullScreen = Configuration.FullScreen;
		grInit.TerrainLOD = CreateLODMapFileName( LTTerrain, Configuration.TerrainComplexity );
		grInit.ModelLOD = CreateLODMapFileName( LTModel, Configuration.ModelComplexity );
		grInit.GrassLOD = CreateLODMapFileName( LTGrass, Configuration.GrassComplexity );
		grInit.pResourceManager = pDialogs->pResourceManager;
		grInit.LightGlows = Configuration.RenderLightGlows;
		grInit.LOD0TerrainTexWidth = Configuration.LOD0TerrainTexWidth;
		grInit.LOD0TerrainTexHeight = Configuration.LOD0TerrainTexHeight;
		grInit.AnisotropicFiltering = Configuration.AnisotropicFiltering;
		grInit.Antialiasing = Configuration.Antialiasing;
		grInit.MultisampleQuality = Configuration.MultisampleQuality;

		hr = ChangeResolution( &grInit );
		if ( FAILED(hr) )
		{
			// something goes wrong, try to restore previous settings
			grInit.Width = PreviousWidth;
			grInit.Height = PreviousHeight;
			grInit.Antialiasing = PreviousAntialiasing;
			grInit.MultisampleQuality = PreviousMultisampleQuality;

			// Save old values back
			Configuration.WindowWidth = PreviousWidth;
			Configuration.WindowHeight = PreviousHeight;
			Pair.SetName( "WindowWidth" );
			Pair.SetInt( PreviousWidth );
			hr = Pair.SaveValue( "Options.cfg" );
			if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save resolution to options.cfg." );
			Pair.SetName( "WindowHeight" );
			Pair.SetInt( PreviousHeight );
			hr = Pair.SaveValue( "Options.cfg" );
			if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save resolution to options.cfg." );
			Configuration.Antialiasing = PreviousAntialiasing;
			Configuration.MultisampleQuality = PreviousMultisampleQuality;
			Pair.SetName( "Antialiasing" );
			Pair.SetInt( Configuration.Antialiasing );
			hr = Pair.SaveValue( "Options.cfg" );
			if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save setting to options.cfg." );
			Pair.SetName( "MultisampleQuality" );
			Pair.SetInt( Configuration.MultisampleQuality );
			hr = Pair.SaveValue( "Options.cfg" );
			if ( FAILED(hr) ) ERRORMSG( hr, "dialogs::OnGSOkClick()", "Unable to save setting to options.cfg." );

			// Try to change back to something, that worked
			hr = ChangeResolution( &grInit);
			if (FAILED(hr) )
			{
				// O Ou something goes awfully wrong!!!
				// This is really, really bad
				PostQuitMessage(0);
				ERRORMSG(hr, "OnGSOKClick()", "Unable to change resolution and ressurect from changes.");
			}
			
			SHOWMESSAGEBOX("ERROR CHANGING GRAPHIC SETTINGS", "Previous settings was succesfully restored.", pDialogs->pSettingsDialog);
			return ERRNOERROR;
		}
	}

	
	if ( TComp != Configuration.TerrainComplexity || MComp != Configuration.ModelComplexity || 
		GComp != Configuration.GrassComplexity )
	{
		// some changes can't be made during game
		if ( gameState == GS_PLAY )
		{
			SHOWMESSAGEBOX( "WARNING", "Some changes will take effect after game restart.", pDialogs->pSettingsDialog );
			return ERRNOERROR;
		}
		else
		{
			str = CreateLODMapFileName( LTTerrain, Configuration.TerrainComplexity );
			hr = pDialogs->pGraphic->LoadMapLODDefinition( str );
			if ( FAILED(hr) ) ERRORMSG( ERRGENERIC, "dialogs::OnGSOkClick()", "Unable to create LOD map." );

			str = CreateLODMapFileName( LTModel, Configuration.ModelComplexity );
			hr = pDialogs->pGraphic->LoadModelLODDefinition( str );
			if ( FAILED(hr) ) ERRORMSG( ERRGENERIC, "dialogs::OnGSOkClick()", "Unable to create LOD map." );

			str = CreateLODMapFileName( LTGrass, Configuration.GrassComplexity );
			hr = pDialogs->pGraphic->LoadGrassLODDefinition( str );
			if ( FAILED(hr) ) ERRORMSG( ERRGENERIC, "dialogs::OnGSOkClick()", "Unable to create LOD map." );
		}
	}
	

	CHANGE_ACTDIALOG( pSettingsDialog );

	return ERRNOERROR;
}

HRESULT CALLBACK  OnGSCancelClick(CGUIControlBase * pObject, LPARAM lParam)
{

	CHANGE_ACTDIALOG(pSettingsDialog);
	return ERRNOERROR;
}

HRESULT CALLBACK  OnGSResolutionBoxChange(CGUIControlBase * pObject, LPARAM lParam)
{
	
	
	return ERRNOERROR;
}

HRESULT CALLBACK  OnGSShadowsBoxChange(CGUIControlBase * pObject, LPARAM lParam)
{
	
	
	return ERRNOERROR;
}

HRESULT CALLBACK  OnGSSpecularBoxChange( CGUIControlBase * pObject, LPARAM lParam )
{
	
	
	return ERRNOERROR;
}

HRESULT CALLBACK  OnGSGammaTrackBarChange( CGUIControlBase * pObject, LPARAM lParam )
{
	
	
	return ERRNOERROR;
}

HRESULT CALLBACK  OnGSAntialiasBoxChange( CGUIControlBase * pObject, LPARAM lParam )
{
	
	
	return ERRNOERROR;
}

HRESULT CALLBACK  OnGSAnisoBoxChange( CGUIControlBase * pObject, LPARAM lParam )
{
	
	
	return ERRNOERROR;
}

HRESULT CALLBACK  OnGSGrassBoxChange( CGUIControlBase * pObject, LPARAM lParam )
{
	
	
	return ERRNOERROR;
}

HRESULT CALLBACK  OnGSTextureBoxChange( CGUIControlBase * pObject, LPARAM lParam )
{
	
	
	return ERRNOERROR;
}

HRESULT CALLBACK  OnGSTerrainBoxChange(CGUIControlBase * pObject, LPARAM lParam)
{
	
	
	return ERRNOERROR;
}

HRESULT CALLBACK  OnGSModelsBoxChange(CGUIControlBase * pObject, LPARAM lParam)
{
	
	
	return ERRNOERROR;
}

/////////////////////////////////////
// Sound settings
HRESULT CALLBACK OnSSOkClick(CGUIControlBase * pObject, LPARAM lParam)
{
	HRESULT hr;
	NAMEVALUEPAIR Pair;
	
	// save value
	Configuration.MusicVolume = pDialogs->pSoundSettingsDialog->SSMusicVolumeTrackBar->GetValue();
	Client->Sound.SetMusicVolume(Configuration.MusicVolume);
	if (!Client->Sound.IsMusicPlaying())
		if (Client->GetClientState() && CS_GAME_PLAY)
		{
			Client->Sound.PlayMusic(TRUE);
		}
	
	Pair.SetName("MusicVolume");
	Pair.SetFloat(Configuration.MusicVolume);
	hr = Pair.SaveValue("Options.cfg");
	if (FAILED(hr) ) ERRORMSG(hr, "OnSSOkClick", "Unable to save music volume setting to options.cfg .");

	CHANGE_ACTDIALOG(pSettingsDialog);
	
	return ERRNOERROR;
}

HRESULT CALLBACK OnSSCancelClick(CGUIControlBase * pObject, LPARAM lParam)
{
	Client->Sound.SetMusicVolume(Configuration.MusicVolume);
	if (!Client->Sound.IsMusicPlaying())
		if (Client->GetClientState() && CS_GAME_PLAY)
		{
			Client->Sound.PlayMusic(TRUE);
		}

	CHANGE_ACTDIALOG(pSettingsDialog);
	return ERRNOERROR;
}

// Changes music volume setting
HRESULT CALLBACK OnSSMusicVolumeChange(CGUIControlBase * pObject, LPARAM lParam)
{
	graphic::CGUIControlTrackBar * TrackBar;
	TrackBar = (graphic::CGUIControlTrackBar *) pObject;

	Client->Sound.SetMusicVolume(TrackBar->GetValue() );
	if (!Client->Sound.IsMusicPlaying())
		if (Client->GetClientState() && CS_GAME_PLAY)
		{
			Client->Sound.PlayMusic(TRUE);
		}
	return ERRNOERROR;
}



///////////////////////////////
// Settings dialog
HRESULT CALLBACK  OnGraphicSettingsClick(CGUIControlBase * pObject, LPARAM lParam)
{
	pDialogs->pGraphicSettingsDialog->SetInitialValues();

	CHANGE_ACTDIALOG(pGraphicSettingsDialog);
	return ERRNOERROR;
}

HRESULT CALLBACK  OnSoundSettingsClick(CGUIControlBase * pObject, LPARAM lParam)
{
	pDialogs->pSoundSettingsDialog->SetInitialValues();

	CHANGE_ACTDIALOG(pSoundSettingsDialog);
	return ERRNOERROR;
}

HRESULT CALLBACK  OnKeySettingsClick(CGUIControlBase * pObject, LPARAM lParam)
{
	// Init right values
	pDialogs->pKeySettingsDialog->SetInitialValues();

	CHANGE_ACTDIALOG(pKeySettingsDialog);
	return ERRNOERROR;
}

HRESULT CALLBACK  OnMainMenuClick(CGUIControlBase * pObject, LPARAM lParam)
{
	CHANGE_ACTDIALOG(pSettingsDialog->ReturnToDialog);
	return ERRNOERROR;
}



////////////////////////////////
// Start Server dialog

HRESULT CALLBACK  OnMultiStartGameButtonClick(CGUIControlBase * pObject, LPARAM lParam){

//	CHANGE_ACTDIALOG(pMultiStartGameDialog);
	return ERRNOERROR;
};

HRESULT CALLBACK OnMultiStartGameCloseButtonClick(CGUIControlBase * pObject, LPARAM lParam){
	CHANGE_ACTDIALOG(pMultiCreateServerDialog);
	return ERRNOERROR;
};

HRESULT CALLBACK OnMultiSendButtonClick(CGUIControlBase * pObject, LPARAM lParam){
	
	
	return ERRNOERROR;
};



/////////////////////////////////
// CREATE SERVER DIALOG
HRESULT CALLBACK OnMCChangeModComboChange( CGUIControlBase * pObject, LPARAM lParam )
{
	int		LineID;

	LineID = pDialogs->pMultiCreateServerDialog->MCGameTypeCombo->GetValue();
	
	// this can happen when no modules are available:
	if ( LineID < 0 ) return ERRGENERIC;

	pGameMod = GameModsWrk[LineID];
	pGameMod->GetConfiguration( &ModConfig );
	pGameMod->ReloadConfig();


	// reload map list
	pDialogs->pMultiCreateServerDialog->ReloadMapList();


	// (DE)ACTIVATE CONTROLS ON THE BASE OF MOD CONFIGURATION

	// 'Advanced' button
	if ( ModConfig.bHasAdvancedDialog )
	{
		pDialogs->pMultiCreateServerDialog->MCAdvancedButton->Enable();
		pDialogs->pMultiCreateServerDialog->MCAdvancedButton->SetOrigTransparency();
	}
	else
	{
		pDialogs->pMultiCreateServerDialog->MCAdvancedButton->Disable();
		pDialogs->pMultiCreateServerDialog->MCAdvancedButton->SetTransparency( 
			1.0f - 0.4f * (1.0f - pDialogs->pMultiCreateServerDialog->MCAdvancedButton->GetOrigTransparency()) );
	}

	// 'controls' button
	if ( ModConfig.uiCustomKeysCount )
	{
		pDialogs->pMultiCreateServerDialog->MCControlsButton->Enable();
		pDialogs->pMultiCreateServerDialog->MCControlsButton->SetOrigTransparency();
	}
	else
	{
		pDialogs->pMultiCreateServerDialog->MCControlsButton->Disable();
		pDialogs->pMultiCreateServerDialog->MCControlsButton->SetTransparency(
			1.0f - 0.4f * (1.0f - pDialogs->pMultiCreateServerDialog->MCControlsButton->GetOrigTransparency() ) );
	}

	// 'opponents' button
	if ( ModConfig.bAI )
	{
		pDialogs->pMultiCreateServerDialog->MCOpponentsButton->Enable();
		pDialogs->pMultiCreateServerDialog->MCOpponentsButton->SetOrigTransparency();
	}
	else
	{
		pDialogs->pMultiCreateServerDialog->MCOpponentsButton->Disable();
		pDialogs->pMultiCreateServerDialog->MCOpponentsButton->SetTransparency( 
			1.0f - 0.4f * (1.0f - pDialogs->pMultiCreateServerDialog->MCOpponentsButton->GetOrigTransparency()) );
	}


	return ERRNOERROR;
};

HRESULT CALLBACK OnMCMapListChange( CGUIControlBase * pObject, LPARAM lParam )
{
	// change the map logo (image)
	
	// first, remove the old image
	pDialogs->pMultiCreateServerDialog->MCMapImage->SetImage( -1, NULL );
	pDialogs->pMultiCreateServerDialog->MCMapImage->SetTransparency( 1.0f );
	
	// now, set the new image only if the image filename is defined in the map
	int index = pDialogs->pMultiCreateServerDialog->MCMapListBox->GetActualItem().iValue;
	if ( GameInfo.MapNames[index].ImageFileName.GetLength() ) 
	{
		pDialogs->pMultiCreateServerDialog->MCMapImage->LoadImage( GameInfo.MapNames[index].ImageFileName, NULL );
		pDialogs->pMultiCreateServerDialog->MCMapImage->SetTransparency( 0.0f );
	}

	return ERRNOERROR;
};

HRESULT CALLBACK OnMCStartServerButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	BOOL Dedicated;
	HRESULT Result;
	int LineID;
	CString GameName;
	CString MapName;
	CString MapFileName;


	GameName = pDialogs->pMultiCreateServerDialog->MCGameNameEdit->GetValue();

	// Test whether game name was entered
	if (GameName == "")
	{
		SHOWMESSAGEBOX("ERROR CREATING SERVER", "Game name must be entered.", pDialogs->pMultiCreateServerDialog);
		return ERRNOERROR;
	}

	// Get map name
	LineID = pDialogs->pMultiCreateServerDialog->MCMapListBox->GetActualItem().iValue;
	MapName = pDialogs->pMultiCreateServerDialog->MCMapListBox->GetActualItemText();
	if (MapName == "")
	{
		SHOWMESSAGEBOX("ERROR CREATING SERVER", "You must select map.", pDialogs->pMultiCreateServerDialog);
		return ERRNOERROR;
	}
	
	// Get map filename
	MapFileName = GameInfo.MapNames[LineID].FileName;

	// Get Game type
	if ( !pGameMod ) 
	{
		SHOWMESSAGEBOX("ERROR CREATING SERVER", "You must select valid game type.", pDialogs->pMultiCreateServerDialog);
		return ERRNOERROR;
	}
	Server->SetGameMOD( pGameMod );

	
	// set the selected mutator
	Server->GetPhysics()->materials.SelectMutator( pDialogs->pMultiCreateServerDialog->MCMutatorsCombo->GetActualString() );

		
	// Set server settings
	Dedicated = pDialogs->pMultiCreateServerDialog->MCDedicatedCheckBox->IsChecked();
	if (Dedicated)
	{
		pDialogs->pMultiStartGameDialog->SetMode(true, true);
		pDialogs->pMultiplayerWaitDialog->SetMode(true, true);
	}
	else
	{
		// Set mode of lobby
		pDialogs->pMultiStartGameDialog->SetMode(true, false);
		pDialogs->pMultiplayerWaitDialog->SetMode(true, false);
		Client->SetGameMOD( pGameMod );
	}
	
	// Clear multiplayer info
	Server->MultiPlayerInfo.Clear();

	// Update opponents settings
	Result = pDialogs->pOpponentsDialog->SaveValues();
	if (FAILED(Result) )
	{
		SHOWMESSAGEBOX("ERROR CREATING SERVER", "Unable to get AI opponents.", pDialogs->pMultiCreateServerDialog);
		return ERRNOERROR;
	}
	
	CHANGE_ACTDIALOG(pMultiplayerWaitDialog);
	
	// Start server game
	Result = startMultiServer( pDialogs->pMultiCreateServerDialog->MCGameNameEdit->GetValue(), Dedicated, MapFileName, pGameMod );
	if (FAILED(Result) )
	{
		if (Result == -2)
		{
			SHOWMESSAGEBOX("ERROR CREATING SERVER", "Selected map is corrupted.", pDialogs->pMultiCreateServerDialog);
		}
		else
		{
			SHOWMESSAGEBOX("ERROR CREATING SERVER", "Failed to start server.", pDialogs->pMultiCreateServerDialog);
		}
		
		return ERRNOERROR;
	}
	// Start server pinging
	Server->StartPing();

	return ERRNOERROR;
}
HRESULT CALLBACK OnMCBackButtonClick(CGUIControlBase * pObject, LPARAM lParam){
	CHANGE_ACTDIALOG(pMultiplayerDialog);
	return ERRNOERROR;
};

HRESULT CALLBACK OnMCOpponentsButtonClick(CGUIControlBase * pObject, LPARAM lParam){
	pDialogs->pOpponentsDialog->SetInitialValues();
	pDialogs->pOpponentsDialog->ReturnDialog = pDialogs->pMultiCreateServerDialog;
	CHANGE_ACTDIALOG(pOpponentsDialog);

	return ERRNOERROR;
};

HRESULT CALLBACK OnMCControlsButtonClick( CGUIControlBase * pObject, LPARAM lParam )
{
	pDialogs->pCustomKeySetsDialog->SetInitialValues();
	pDialogs->pCustomKeySetsDialog->pReturnDialog = pDialogs->pMultiCreateServerDialog;
	CHANGE_ACTDIALOG(pCustomKeySetsDialog);

	return ERRNOERROR;
};

HRESULT CALLBACK OnMCAdvancedButtonClick( CGUIControlBase * pObject, LPARAM lParam )
{
	//pDialogs->pCustomKeySetsDialog->SetInitialValues();
	//pDialogs->pCustomKeySetsDialog->pReturnDialog = pDialogs->pMultiCreateServerDialog;
	//CHANGE_ACTDIALOG(pCustomKeySetsDialog);

	return ERRNOERROR;
};


///////////////////////////////////////////////
// MULTIPLAYER PLAYER SETTING DIALOG
HRESULT CALLBACK OnMSOkButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	CMultiPlayerSettingsDialog * Dialog;
	CString PlayerName;
	CString PlayerCar;
	CString CarFileName;
	int		TeamID;

	Dialog = pDialogs->pMultiPlayerSettingsDialog;

	PlayerName = Dialog->MSPlayerNameEdit->GetValue();
	if (PlayerName == "")
	{
		SHOWMESSAGEBOX("ERROR PLAYER SETTINGS", "You must type in player name.", pDialogs->pMultiPlayerSettingsDialog);
		return ERRNOERROR;
	}

	PlayerCar = Dialog->MSCarListBox->GetActualString();

	if (PlayerCar == "")
	{
		SHOWMESSAGEBOX("ERROR PLAYER SETTINGS", "You must select car.", pDialogs->pMultiPlayerSettingsDialog);
		return ERRNOERROR;
	}

	CarFileName = Client->MultiPlayerInfo.GetAllowedCarsFileNames().at(Dialog->MSCarListBox->GetValue());

	// obtain the team ID
	if ( Dialog->MSTeamComboBox->GetValue() <= 0 ) TeamID = Server->AutoAssignTeam();
	else TeamID = Dialog->MSTeamComboBox->GetValue() - 1;

	if ( TeamID < 0 && ModConfig.uiMinTeamCount > 1 )
	{
		SHOWMESSAGEBOX("ERROR PLAYER SETTINGS", "The team you selected is full.", pDialogs->pMultiPlayerSettingsDialog);
		return ERRNOERROR;
	}

	// Set player status and send message to server
	
	Client->UpdatePlayerStatus(PlayerName, CarFileName, TeamID);
	
	CHANGE_ACTDIALOG(pMultiStartGameDialog);
	return ERRNOERROR;
}

HRESULT CALLBACK OnMSCancelButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{	

	CHANGE_ACTDIALOG(pMultiStartGameDialog);
	return ERRNOERROR;
}


/////////////////////////////////
// MULTIPLAYER DIALOG

HRESULT CALLBACK OnMPCreateButtonClick(CGUIControlBase * pObject, LPARAM lParam){
	
	CHANGE_ACTDIALOG(pMultiCreateServerDialog);
	return ERRNOERROR;
}

HRESULT CALLBACK OnMPJoinButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	CString		IP, csName, csPom;
	int			Port;


	IP = pDialogs->pMultiplayerDialog->MPIPEdit->GetValue();
	Port = pDialogs->pMultiplayerDialog->MPPortEdit->GetValueInt();

	if (Port < 1000 || Port > 65535) 
	{
		SHOWMESSAGEBOX("JOIN GAME FAILED.", "You must enter port between 1000 and 65535.", pDialogs->pMultiplayerDialog);
		return ERRNOERROR;
	}
	if (IP == "")
	{
		SHOWMESSAGEBOX("JOIN GAME FAILED.", "You must enter valid IP address.", pDialogs->pMultiplayerDialog);
		return ERRNOERROR;
	}

	pDialogs->pMultiStartGameDialog->SetMode(false, false);
	pDialogs->pMultiplayerWaitDialog->SetMode(false, false);
	
	Client->ChangeState(CS_MG_START);
	Client->ConnectToServer(IP, Port);
	CHANGE_ACTDIALOG(pMultiplayerWaitDialog);

	return ERRNOERROR;
}
HRESULT CALLBACK OnMPBackButtonClick(CGUIControlBase * pObject, LPARAM lParam){
	
	
	CHANGE_ACTDIALOG(pMainDialog);
	return ERRNOERROR;
}

HRESULT CALLBACK OnMPGameListListChange(CGUIControlBase * pObject, LPARAM lParam)
{
	char IP[20];
	char Port[10];
	graphic::CGUIControlListBox * ListBox;
	ListBox = (graphic::CGUIControlListBox *) pObject;
	CClient::DialogServerInfo ServInfo;
	in_addr Addr;
	
	// Convert IP and Port into string
	if (pDialogs->pMultiplayerDialog->ServerList.size() > 0)
	{
		ServInfo = pDialogs->pMultiplayerDialog->ServerList[ListBox->GetValue()];
		
		Addr.S_un.S_addr = ServInfo.addr;
		strcpy_s( IP, sizeof(IP), inet_ntoa( Addr ) );
		sprintf_s( Port, sizeof(Port), "%i", ServInfo.port );

		pDialogs->pMultiplayerDialog->MPIPEdit->SetValueSilent(IP);
		pDialogs->pMultiplayerDialog->MPPortEdit->SetValueSilent(Port);
	}
	else
	{
		pDialogs->pMultiplayerDialog->MPIPEdit->SetValueSilent("");
		pDialogs->pMultiplayerDialog->MPPortEdit->SetValueSilent("");
	}

	return ERRNOERROR;
}



/////////////////////////////////////////////
// MULTIPLAYER START GAME DIALOG (LOBBY)
HRESULT CALLBACK OnMGSendButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	CMultiStartGameDialog * Dialog;
	CString ChatMessage;

	Dialog = pDialogs->pMultiStartGameDialog;
	
	ChatMessage = Dialog->MGChatEdit->GetValue();
	if (ChatMessage != "")
	{
		if (! Dialog->Dedicated)
			Client->AddChatMessage(ChatMessage);
		else
			::Server->AddChatMessage(ChatMessage);
		Dialog->MGChatEdit->SetValueSilent("");
	}
	
	return ERRNOERROR;
}

HRESULT CALLBACK OnMGLeaveButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	// if server, stop server
	Server->ChangeServerState(0);
	
	// if client
	if (! pDialogs->pMultiStartGameDialog->Dedicated)
	{
		Client->ChangeState(CS_MG_ENUMERATE_HOSTS);
	}

	CHANGE_ACTDIALOG(pMultiplayerDialog);
	return ERRNOERROR;
}

HRESULT CALLBACK OnMGStartGameButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	HRESULT Result;
	UINT ID;
	CString PlayerName;
	CString CarFileName;
	CString TempCarName;
	CString Temp;
	BOOL Ready;
	BOOL Active;
	BOOL AllPlayersReady;
	BOOL AllCarNamesValid;
	UINT i;
	int DayHours, DayMinutes, NumLaps;
	BOOL LapMode;
	UINT TeamID;

	// Is client -> READY pressed
	if (! pDialogs->pMultiStartGameDialog->Dedicated)
	{
		ID = Client->MultiPlayerInfo.GetID();
		Client->MultiPlayerInfo.GetPlayerReady(ID, Ready);
		Client->MultiPlayerInfo.GetPlayerName(ID, PlayerName);
		Client->MultiPlayerInfo.GetPlayerCar(ID, CarFileName);
		TeamID = Client->MultiPlayerInfo.GetPlayerTeamID(ID);
		if (!Ready)
		{
			if (PlayerName == "")
			{
				SHOWMESSAGEBOX("MULTIPLAYER LOBBY ERROR", "You must have a name in order to be ready.", pDialogs->pMultiStartGameDialog);
				return ERRNOERROR;
			}

			if (CarFileName == "")
			{
				SHOWMESSAGEBOX("MULTIPLAYER LOBBY ERROR", "You must select a valid car in order to be ready.", pDialogs->pMultiStartGameDialog);
				return ERRNOERROR;
			}

			Client->UpdatePlayerStatus(PlayerName, CarFileName, (int) TeamID, true);
			// Update also server, if this is also server
			if (pDialogs->pMultiStartGameDialog->Server)
				Server->MultiPlayerInfo.SetPlayerStatus(ID, PlayerName, CarFileName, (int) TeamID, true, true);
			// hide and disable player settings
			pDialogs->pMultiStartGameDialog->MGPlayerSettingsButton->SetInvisible();
			pDialogs->pMultiStartGameDialog->MGPlayerSettingsButton->Disable();

		}
		else
		{
			Client->UpdatePlayerStatus(PlayerName, CarFileName, false);
			// show player settings
			pDialogs->pMultiStartGameDialog->MGPlayerSettingsButton->SetVisible();
			pDialogs->pMultiStartGameDialog->MGPlayerSettingsButton->Enable();
		}
	}

	if (pDialogs->pMultiStartGameDialog->Server)
	{
		// Get all game settings
		Temp = pDialogs->pMultiStartGameDialog->MGDayHoursEdit->GetValue();
		pDialogs->pMultiStartGameDialog->OnlyNumbers(Temp);
		sscanf(Temp, "%i", &DayHours);
		Temp = pDialogs->pMultiStartGameDialog->MGDayMinutesEdit->GetValue();
		pDialogs->pMultiStartGameDialog->OnlyNumbers(Temp);
		sscanf(Temp, "%i", &DayMinutes);
		Temp = pDialogs->pMultiStartGameDialog->MGLapsEdit->GetValue();
		pDialogs->pMultiStartGameDialog->OnlyNumbers(Temp);
		sscanf(Temp, "%i", &NumLaps);
		LapMode = pDialogs->pMultiStartGameDialog->MGLapsCheckBox->IsChecked();

		// check game settings
		if (DayHours < 0 || DayHours > 23)
		{
			SHOWMESSAGEBOX("MULTIPLAYER LOBBY ERROR", "Hours must be between 0 and 23.", pDialogs->pMultiStartGameDialog);		
			return ERRNOERROR;
		}
		if (DayMinutes < 0 || DayMinutes > 59)
		{
			SHOWMESSAGEBOX("MULTIPLAYER LOBBY ERROR", "Minutes must be between 0 and 59.", pDialogs->pMultiStartGameDialog);
			return ERRNOERROR;
		}
		if (LapMode && NumLaps <= 0 || NumLaps > 999)
		{
			SHOWMESSAGEBOX("MULTIPLAYER LOBBY ERROR", "Laps must be between 1 and 999.", pDialogs->pMultiStartGameDialog);
			return ERRNOERROR;
		}

		// Check if all players are ready and their car names valid
		AllPlayersReady = TRUE;
		AllCarNamesValid = TRUE;
		for (i = 0; i < MAX_PLAYERS; i++)
		{
			Server->MultiPlayerInfo.GetPlayerActive(i, Active);
			if (Active)
			{
				Server->MultiPlayerInfo.GetPlayerReady(i, Ready);
				if (! Ready)
				{
					AllPlayersReady = FALSE;
					break;
				}
				Server->MultiPlayerInfo.GetPlayerCar(i, TempCarName);
				if (TempCarName == "")
				{
					AllCarNamesValid = FALSE;
					break;
				}
			}
		}

		if (!AllPlayersReady)
		{
			// Set server's client not to be ready
			if (! pDialogs->pMultiStartGameDialog->Dedicated)
				Server->MultiPlayerInfo.SetPlayerStatus(ID, PlayerName, CarFileName, -1, false, true);

			return ERRNOERROR;
		}

		if (!AllCarNamesValid)
		{
			// Set server client not to be ready
			if (! pDialogs->pMultiStartGameDialog->Dedicated)
				Server->MultiPlayerInfo.SetPlayerStatus(ID, PlayerName, CarFileName, -1, false, true);
			
			SHOWMESSAGEBOX("ERROR STARTING SERVER.", "All car names must be valid.", pDialogs->pMultiStartGameDialog);

			return ERRNOERROR;
		}

		CHANGE_ACTDIALOG(pWaitDialog);
		pDialogs->pGraphic->RenderDialogOnly();
		
		Result = startMultiPlayerGame();
		if (FAILED(Result) )
		{
			// Stop server
			stopServer();
			// Free all (potentially) loaded game play resources
			FreeGamePlayResources();
			
			SHOWMESSAGEBOX("ERROR STARTING SERVER.", "Unable to start server. Loading failed.", pDialogs->pMultiplayerDialog);
			return ERRNOERROR;
		}
		
		// Start client loading
		if (! pDialogs->pMultiStartGameDialog->Dedicated)
		{
			Result = Client->ClientStartMultiLoad();
			if (FAILED(Result) )
			{
				Client->ChangeState(CS_MG_ENUMERATE_HOSTS);
				FreeGamePlayResources();
				SHOWMESSAGEBOX("ERROR LOADING CLIENT", "Unable to start client. Client load failed.", pDialogs->pMultiplayerDialog);
			}

			return ERRNOERROR;
		}
	
	}

	return ERRNOERROR;
}

HRESULT CALLBACK OnMGPlayerSettingsButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	CHANGE_ACTDIALOG(pMultiPlayerSettingsDialog);
	return ERRNOERROR;
}


HRESULT CALLBACK OnMGChatEditKeyUp(CGUIControlBase * pObject, LPARAM lParam)
{
	MESSAGEPARAMSSTRUCT	* params;

	params = (MESSAGEPARAMSSTRUCT *) lParam;

	if (params->wParam == VK_RETURN)
	{
		OnMGSendButtonClick(pObject, lParam);
	}


	return ERRNOERROR;
}

////////////////////////////////////
// Dedicated server dialog
HRESULT CALLBACK OnDSStopServerButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	stopServer();

	CHANGE_ACTDIALOG(pMultiplayerDialog);

	return ERRNOERROR;
}

HRESULT CALLBACK OnDSToLobbyButtonClick(CGUIControlBase * pObject, LPARAM lParam)
{
	Server->SendServerReturnedToLobby();
	FreeGamePlayResources(TRUE);
	CHANGE_ACTDIALOG(pMultiStartGameDialog);

	return ERRNOERROR;
}


/////////////////////////////////
// In Game menu

HRESULT CALLBACK OnCloseButtonClick( CGUIControlBase * pObject, LPARAM lParam )
{
	pDialogs->hideDialog();
	return ERRNOERROR;
}


////////////////////////////////
// MAIN MENU
HRESULT CALLBACK OnCreditsButtonClick( CGUIControlBase * pObject, LPARAM lParam )
{
	CHANGE_ACTDIALOG(pCreditsDialog);
	return ERRNOERROR;
}

HRESULT CALLBACK OnSingleStartButtonClick( CGUIControlBase * pObject, LPARAM lParam )
{
	// Set initials
	pDialogs->pSinglePlayerDialog->SetInitialValues();
	CHANGE_ACTDIALOG(pSinglePlayerDialog);
	return ERRNOERROR;
}

HRESULT CALLBACK OnMultiStartButtonClick( CGUIControlBase * pObject, LPARAM lParam )
{	
	CHANGE_ACTDIALOG(pMultiplayerDialog);
	return ERRNOERROR;
}
HRESULT CALLBACK OnSettingsButtonClick( CGUIControlBase * pObject, LPARAM lParam )
{
	if (pObject == pDialogs->pMainDialog->SettingsButton)
		pDialogs->pSettingsDialog->ReturnToDialog = pDialogs->pMainDialog;
	else
		pDialogs->pSettingsDialog->ReturnToDialog = pDialogs->pInGameDialog;

	CHANGE_ACTDIALOG(pSettingsDialog);
	return ERRNOERROR;
}

HRESULT CALLBACK OnExitButtonClick( CGUIControlBase * pObject, LPARAM lParam )
{

	gameState = GS_QUIT;
	return ERRNOERROR;
}

HRESULT CALLBACK OnMainMenuButtonClick( CGUIControlBase * pObject, LPARAM lParam )
{
	FreeGamePlayResources();

	// Set game state to dialog loop
	gameState = GS_START;
	CHANGE_ACTDIALOG(pMainDialog);
	return ERRNOERROR;
}


void CMyDialogs::hideDialog(){
	pGraphic->SetDialog( NULL );
	pGraphic->SetMouseVisible( false );
	visible = false;
	
}

void CMyDialogs::showDialog(){
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(hWnd,&point);
	pGraphic->SetMousePositionXY(point.x,point.y);
	pActDialog->pGUIDialog->Reset();
	pGraphic->SetDialog( pActDialog->pGUIDialog );
	pGraphic->SetMouseVisible();

	visible= true;
}

///////////////////////////////////////////////////////////////////////////////
//
// Must be called when D3D device is lost
//
///////////////////////////////////////////////////////////////////////////////
void CMyDialogs::OnLostDevice()
{
	list<CMyDialog *>::iterator It;

	for (It = Dialogs.begin(); It != Dialogs.end(); It++)
	{
		(*It)->pGUIDialog->OnLostDevice();
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// Must be called after D3D device is reset
//
///////////////////////////////////////////////////////////////////////////////
void CMyDialogs::OnResetDevice()
{
	list<CMyDialog *>::iterator It;

	for (It = Dialogs.begin(); It != Dialogs.end(); It++)
	{
		(*It)->pGUIDialog->OnResetDevice();
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// Places dialogs into the center according to actual resolution
//
///////////////////////////////////////////////////////////////////////////////
void CMyDialogs::RecenterDialogs()
{
	list<CMyDialog *>::iterator It;

	int Width, Height;
	CGUIDialog * TempDialog;

	Width = pGraphic->GetWidth();
	Height = pGraphic->GetHeight();

	for (It = Dialogs.begin(); It != Dialogs.end(); It++)
	{
		TempDialog = (*It)->pGUIDialog;
		TempDialog->SetPosition( Width / 2 - TempDialog->GetWidth() / 2,
								 Height / 2 - TempDialog->GetHeight() / 2);

	}
}

///////////////////////////////////////
// MyDialog init functions
///////////////////////////////////////
HRESULT CMyDialog::init(CString Dialog_file,CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind)
{
	HRESULT hr;

	pGraphic = _pGraphic;
	pResourceManager = _pRM;
	pTimer= _pTimer;
	hWind= _hWind;
	pGUIDialog = new CGUIDialog();
	
	if ( !(pGUIDialog) ) ERRORMSG( ERROUTOFMEMORY, "GraphicTest::InitInstance()", "Unsuccessful GUI dialog creation." );
	
	
	
	hr = pGUIDialog->InitFromFile( CONFIG_DIALOGSPATH + Dialog_file, hWind, pGraphic->GetD3DDevice(), pResourceManager, pTimer );
	if ( hr ) ERRORMSG( hr, "GraphicTest::InitInstance()", "Main dialog object could not be initialized." );
		
	return ERRNOERROR;
}
void CMyDialog::OnEnter(){

}
void CMyDialog::OnLeave(){
}

///////////////////////////////////////
// Dialogs init functions
///////////////////////////////////////

// sets messages and return dialog. 
void CMessageBox::PrepareMessageBox(CString TypeMessage, CString Message, CMyDialog * ReturnDialog)
{
	// Set messages
	MBMessageTypeLabel->SetValue(TypeMessage);
	MBMessageLabel->SetValue(Message);

	// Set dialog to return to
	ReturnToDialog = ReturnDialog;
}

// Init message box dialog
HRESULT CMessageBox::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind)
{
	HRESULT hr;
	hr = CMyDialog::init("MessageBox.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "CMessageBox::Init()", " Message box dialog object could not be initialized." );

	// labels
	GETDIALOGCONTROL2(MBMessageTypeLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MBMessageLabel, graphic::CGUIControlLabel);
	
	// buttons
	GETDIALOGCONTROL2(MBOkButton, graphic::CGUIControlButton);

	MBOkButton->lpOnMouseClick = OnMBOkButtonClick;

	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	return ERRNOERROR;
}

// Init Credits dialog
HRESULT CCreditsDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind)
{
	HRESULT hr;
	hr = CMyDialog::init("Credits.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "CCreditsDialog::Init()", "Credits dialog object could not be initialized." );

	// Labels
	GETDIALOGCONTROL2(CDCreditsLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDCrazyAnimalsLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDPavelCelbaLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDPavelJandaLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDRadimKrupickaLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDRomanMargoldLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDJiriSofkaLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDOthersLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDKarelPrasekLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDMartinHolecekLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDTiborPetakLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDHardlinerRecordingsLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDJakobFischerLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDPavelCelbaNoteLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDPavelJandaNoteLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDRadimKrupickaNoteLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDRomanMargoldNoteLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDJiriSofkaNoteLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDKarelPrasekNoteLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDMartinHolecekNoteLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDTiborPetakNoteLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDHardlinerRecordingsNoteLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(CDJakobFischerNoteLabel, graphic::CGUIControlLabel);
	// buttons
	GETDIALOGCONTROL2(CDOkButton, graphic::CGUIControlButton);

	// events
	CDOkButton->lpOnMouseClick = OnCDOkButtonClick;

	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	return ERRNOERROR;
}

// Init key settings dialog
HRESULT CKeySettingsDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind)
{
	HRESULT hr;
	hr = CMyDialog::init("KeySettings.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "GraphicTest::Init()", "Key settings dialog object could not be initialized." );

	GETDIALOGCONTROL2(KSKeySettingsLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(KSAccelerateLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(KSDecelerateLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(KSSteerLeftLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(KSSteerRightLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(KSBrakeLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(KSShiftUpLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(KSShiftDownLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(KSChangeViewLabel, graphic::CGUIControlLabel);
	//GETDIALOGCONTROL2(KSDriverViewLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(KSRestartLabel, graphic::CGUIControlLabel);
	//GETDIALOGCONTROL2(KSGamePauseLabel, graphic::CGUIControlLabel);

	GETDIALOGCONTROL2(KSAccelerateEdit, graphic::CGUIControlEdit);
	GETDIALOGCONTROL2(KSDecelerateEdit, graphic::CGUIControlEdit);
	GETDIALOGCONTROL2(KSSteerLeftEdit, graphic::CGUIControlEdit);
	GETDIALOGCONTROL2(KSSteerRightEdit, graphic::CGUIControlEdit);
	GETDIALOGCONTROL2(KSBrakeEdit, graphic::CGUIControlEdit);
	GETDIALOGCONTROL2(KSShiftUpEdit, graphic::CGUIControlEdit);
	GETDIALOGCONTROL2(KSShiftDownEdit, graphic::CGUIControlEdit);
	GETDIALOGCONTROL2(KSChangeViewEdit, graphic::CGUIControlEdit);
	//GETDIALOGCONTROL2(KSDriverViewEdit, graphic::CGUIControlEdit);
	GETDIALOGCONTROL2(KSRestartEdit, graphic::CGUIControlEdit);
	//GETDIALOGCONTROL2(KSGamePauseEdit, graphic::CGUIControlEdit);

	GETDIALOGCONTROL2(KSOk, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(KSCancel, graphic::CGUIControlButton);

	KSAccelerateEdit->lpOnKeyUp = OnKSKeyUp;
	KSDecelerateEdit->lpOnKeyUp = OnKSKeyUp;
	KSSteerLeftEdit->lpOnKeyUp = OnKSKeyUp;
	KSSteerRightEdit->lpOnKeyUp = OnKSKeyUp;
	KSBrakeEdit->lpOnKeyUp = OnKSKeyUp;
	KSShiftUpEdit->lpOnKeyUp = OnKSKeyUp;
	KSShiftDownEdit->lpOnKeyUp = OnKSKeyUp;
	KSChangeViewEdit->lpOnKeyUp = OnKSKeyUp;
	//KSDriverViewEdit->lpOnKeyUp = OnKSKeyUp;
	KSRestartEdit->lpOnKeyUp = OnKSKeyUp;
	//KSGamePauseEdit->lpOnKeyUp = OnKSKeyUp;

	KSAccelerateEdit->lpOnKeyDown = OnKSKeyDown;
	KSDecelerateEdit->lpOnKeyDown = OnKSKeyDown ;
	KSSteerLeftEdit->lpOnKeyDown  = OnKSKeyDown ;
	KSSteerRightEdit->lpOnKeyDown  = OnKSKeyDown ;
	KSBrakeEdit->lpOnKeyDown  = OnKSKeyDown;
	KSShiftUpEdit->lpOnKeyDown  = OnKSKeyDown ;
	KSShiftDownEdit->lpOnKeyDown  = OnKSKeyDown ;
	KSChangeViewEdit->lpOnKeyDown  = OnKSKeyDown ;
	//KSDriverViewEdit->lpOnKeyDown  = OnKSKeyDown ;
	KSRestartEdit->lpOnKeyDown  = OnKSKeyDown ;
	//KSGamePauseEdit->lpOnKeyDown  = OnKSKeyDown ;


	
	KSOk->lpOnMouseClick = OnKSOkClick;
	KSCancel->lpOnMouseClick = OnKSCancelClick;

	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	return ERRNOERROR;
}

void CKeySettingsDialog::SetInitialValues()
{
	// Key value initialization
	KSAccelerateEdit->SetValueSilent(Configuration.UnParseKeyBindConfigPair( Configuration.KeyAccelerate) );
	KSDecelerateEdit->SetValueSilent(Configuration.UnParseKeyBindConfigPair( Configuration.KeyDecelerate) );
	KSSteerLeftEdit->SetValueSilent(Configuration.UnParseKeyBindConfigPair( Configuration.KeySteerLeft) );
	KSSteerRightEdit->SetValueSilent(Configuration.UnParseKeyBindConfigPair( Configuration.KeySteerRight) );
	KSBrakeEdit->SetValueSilent(Configuration.UnParseKeyBindConfigPair( Configuration.KeyBrake) );
	KSShiftUpEdit->SetValueSilent(Configuration.UnParseKeyBindConfigPair( Configuration.KeyShiftUp) );
	KSShiftDownEdit->SetValueSilent(Configuration.UnParseKeyBindConfigPair( Configuration.KeyShiftDown) );
	KSChangeViewEdit->SetValueSilent(Configuration.UnParseKeyBindConfigPair( Configuration.KeyChangeView) );
//	KSDriverViewEdit->SetValueSilent(Configuration.UnParseKeyBindConfigPair( Configuration.KeyDriverView) );
	KSRestartEdit->SetValueSilent(Configuration.UnParseKeyBindConfigPair( Configuration.KeyRestart) );
//	KSGamePauseEdit->SetValueSilent( Configuration.UnParseKeyBindConfigPair( Configuration.KeyPause) );

	return;
}

// checks, whether such key mapping is not used elsewhere
BOOL CKeySettingsDialog::CheckValue(graphic::CGUIControlEdit * ChangedEdit, WPARAM NewValue)
{
	CString InName;
	InName = Configuration.UnParseKeyBindConfigPair((int) NewValue);

	if (KSAccelerateEdit->GetValue() == InName && ChangedEdit != KSAccelerateEdit)
		return TRUE;
	if (KSDecelerateEdit->GetValue() == InName && ChangedEdit != KSDecelerateEdit)
		return TRUE;
	if (KSSteerLeftEdit->GetValue() == InName && ChangedEdit != KSSteerLeftEdit)
		return TRUE;
	if (KSSteerRightEdit->GetValue() == InName && ChangedEdit != KSSteerRightEdit)
		return TRUE;
	if (KSBrakeEdit->GetValue() == InName && ChangedEdit != KSBrakeEdit)
		return TRUE;
	if (KSShiftUpEdit->GetValue() == InName && ChangedEdit != KSShiftUpEdit)
		return TRUE;
	if (KSShiftDownEdit->GetValue() == InName && ChangedEdit != KSShiftDownEdit)
		return TRUE;
	if (KSChangeViewEdit->GetValue() == InName && ChangedEdit != KSChangeViewEdit)
		return TRUE;
//	if (KSDriverViewEdit->GetValue() == InName && ChangedEdit != KSDriverViewEdit)
//		return TRUE;
	if (KSRestartEdit->GetValue() == InName && ChangedEdit != KSRestartEdit)
		return TRUE;
//	if (KSGamePauseEdit->GetValue() == InName && ChangedEdit != KSGamePauseEdit)
//		return TRUE;

	return FALSE;
}

// Save key settings values 
HRESULT CKeySettingsDialog::SaveValues()
{
	HRESULT				hr;
	NAMEVALUEPAIR		Pair;
	CGameMod::INT_LIST	keysList;
	

	// Update configuration
	Configuration.KeyAccelerate = Configuration.ParseKeyBindConfigPair(KSAccelerateEdit->GetValue() );
	Configuration.KeyDecelerate = Configuration.ParseKeyBindConfigPair(KSDecelerateEdit->GetValue() );
	Configuration.KeySteerLeft = Configuration.ParseKeyBindConfigPair(KSSteerLeftEdit->GetValue() );
	Configuration.KeySteerRight = Configuration.ParseKeyBindConfigPair(KSSteerRightEdit->GetValue() );
	Configuration.KeyBrake = Configuration.ParseKeyBindConfigPair(KSBrakeEdit->GetValue() );
	Configuration.KeyShiftUp = Configuration.ParseKeyBindConfigPair(KSShiftUpEdit->GetValue() );
	Configuration.KeyShiftDown = Configuration.ParseKeyBindConfigPair(KSShiftDownEdit->GetValue() );
	Configuration.KeyChangeView = Configuration.ParseKeyBindConfigPair(KSChangeViewEdit->GetValue() );
//	Configuration.KeyDriverView = Configuration.ParseKeyBindConfigPair(KSDriverViewEdit->GetValue() );
	Configuration.KeyRestart = Configuration.ParseKeyBindConfigPair(KSRestartEdit->GetValue() );
//	Configuration.KeyPause = Configuration.ParseKeyBindConfigPair(KSGamePauseEdit->GetValue() );

	// save to KeyBinding.cfg
	Pair.SetName("KeyAccelerate");
	Pair.SetString(KSAccelerateEdit->GetValue());
	hr = Pair.SaveValue("KeyBinding.cfg");
	if (FAILED(hr) ) ERRORMSG(hr, "CKeySettingsDialog::SaveValues()", "Unable to save key bindings to KeyBinding.cfg .");

	Pair.SetName("KeyDecelerate");
	Pair.SetString(KSDecelerateEdit->GetValue());
	hr = Pair.SaveValue("KeyBinding.cfg");
	if (FAILED(hr) ) ERRORMSG(hr, "CKeySettingsDialog::SaveValues()", "Unable to save key bindings to KeyBinding.cfg .");

	Pair.SetName("KeySteerLeft");
	Pair.SetString(KSSteerLeftEdit->GetValue());
	hr = Pair.SaveValue("KeyBinding.cfg");
	if (FAILED(hr) ) ERRORMSG(hr, "CKeySettingsDialog::SaveValues()", "Unable to save key bindings to KeyBinding.cfg .");

	Pair.SetName("KeySteerRight");
	Pair.SetString(KSSteerRightEdit->GetValue());
	hr = Pair.SaveValue("KeyBinding.cfg");
	if (FAILED(hr) ) ERRORMSG(hr, "CKeySettingsDialog::SaveValues()", "Unable to save key bindings to KeyBinding.cfg .");

	Pair.SetName("KeyBrake");
	Pair.SetString(KSBrakeEdit->GetValue());
	hr = Pair.SaveValue("KeyBinding.cfg");
	if (FAILED(hr) ) ERRORMSG(hr, "CKeySettingsDialog::SaveValues()", "Unable to save key bindings to KeyBinding.cfg .");

	Pair.SetName("KeyShiftUp");
	Pair.SetString(KSShiftUpEdit->GetValue());
	hr = Pair.SaveValue("KeyBinding.cfg");
	if (FAILED(hr) ) ERRORMSG(hr, "CKeySettingsDialog::SaveValues()", "Unable to save key bindings to KeyBinding.cfg .");

	Pair.SetName("KeyShiftDown");
	Pair.SetString(KSShiftDownEdit->GetValue());
	hr = Pair.SaveValue("KeyBinding.cfg");
	if (FAILED(hr) ) ERRORMSG(hr, "CKeySettingsDialog::SaveValues()", "Unable to save key bindings to KeyBinding.cfg .");

	Pair.SetName("KeyChangeView");
	Pair.SetString(KSChangeViewEdit->GetValue());
	hr = Pair.SaveValue("KeyBinding.cfg");
	if (FAILED(hr) ) ERRORMSG(hr, "CKeySettingsDialog::SaveValues()", "Unable to save key bindings to KeyBinding.cfg .");

//	Pair.SetName("KeyDriverView");
//	Pair.SetString(KSDriverViewEdit->GetValue());
//	hr = Pair.SaveValue("KeyBinding.cfg");
//	if (FAILED(hr) ) ERRORMSG(hr, "CKeySettingsDialog::SaveValues()", "Unable to save key bindings to KeyBinding.cfg .");

	Pair.SetName("KeyRestart");
	Pair.SetString(KSRestartEdit->GetValue());
	hr = Pair.SaveValue("KeyBinding.cfg");
	if (FAILED(hr) ) ERRORMSG(hr, "CKeySettingsDialog::SaveValues()", "Unable to save key bindings to KeyBinding.cfg .");

//	Pair.SetName("KeyPause");
//	Pair.SetString(KSGamePauseEdit->GetValue());
//	hr = Pair.SaveValue("KeyBinding.cfg");
//	if (FAILED(hr) ) ERRORMSG(hr, "CKeySettingsDialog::SaveValues()", "Unable to save key bindings to KeyBinding.cfg .");

	
	// obtain game MOD keys
	/* 	Problem is, that there is some bloody crazy behaviour in the vector being passed as parameter into DLL.
		No new items can be added into the vector. When any new item is added, the function crashes after the
		end of this scope. Hovewer, existing items can be changed, so we are preparing the vector object before
		passing it to the function. (It's damn' crazy thing since it DOES work when the called function is not in
		DLL, but in this module.
	*/
	keysList.clear();
	for ( UINT i = 0; i < ModConfig.uiCustomKeysCount; i++ ) keysList.push_back( i );
	pGameMod->GetCustomKeys( &keysList );
	
	// Refresh action mapping in controls
	gInput.RefreshActionMapping( &Configuration, &keysList );

	return 0;
};

// resets the data of this dialog - enables and disables controls
void CCustomKeySetsDialog::ResetDialog()
{
	// enable only as many fields as are defined by the MOD config
	//	disable the rest of them
	for ( int i = 0; i < MAX_CUSTOM_KEYS; i++ )
	{
		if ( i < (int) ModConfig.uiCustomKeysCount )
		{
			CKSKeyEdit[i]->Enable();
			CKSKeyEdit[i]->SetVisible();
			CKSKeyLabel[i]->SetVisible();
		}
		else 
		{
			CKSKeyEdit[i]->Disable();
			CKSKeyEdit[i]->SetInvisible();
			CKSKeyLabel[i]->SetInvisible();
		}
	}
};

// Init custom key settings dialog
HRESULT CCustomKeySetsDialog::init( CGraphic * _pGraphic,
									resManNS::CResourceManager * _pRM,
									CTimeCounter * _pTimer,
									HWND _hWind )
{
	HRESULT		hr;
	char		str[128];

	hr = CMyDialog::init( "CustomKeySettings.dlgDef", _pGraphic, _pRM, _pTimer, _hWind );
	if ( hr ) ERRORMSG( hr, "CCustomKeySetsDialog::init()", "Key settings dialog object could not be initialized." );

	// init common dialog controls
	GETDIALOGCONTROL2( CKSMainLabel, graphic::CGUIControlLabel );
	
	GETDIALOGCONTROL2( CKSOk, graphic::CGUIControlButton );
	GETDIALOGCONTROL2( CKSCancel, graphic::CGUIControlButton );

	// init configurable controls - edit boxes and labels
	for ( int i = 0; i < MAX_CUSTOM_KEYS; i++ )
	{
		sprintf_s( str, 128, "CKSKey%iLabel", i+1 );
		GETDIALOGCONTROL3( CKSKeyLabel[i], graphic::CGUIControlLabel, str );
		sprintf_s( str, 128, "CKSKey%iEdit", i+1 );
		GETDIALOGCONTROL3( CKSKeyEdit[i], graphic::CGUIControlEdit, str );
		
		CKSKeyEdit[i]->lpOnKeyDown = OnCKSKeyDown;
		CKSKeyEdit[i]->lpOnKeyUp = OnCKSKeyUp;
	}		

	CKSOk->lpOnMouseClick = OnCKSOkClick;
	CKSCancel->lpOnMouseClick = OnCKSCancelClick;

	// reset custom control visibility and usability
	ResetDialog();

	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed

	return ERRNOERROR;
};

void CCustomKeySetsDialog::SetInitialValues()
{
	CGameMod::INT_LIST		Keys;
	CGameMod::STRING_LIST	KeyNames;

	/* First, obtain custom keys from actual game MOD.
		Problem is, that there is some bloody crazy behaviour in the vector being passed as parameter into DLL.
		No new items can be added into the vector. When any new item is added, the function crashes after the
		end of this scope. Hovewer, existing items can be changed, so we are preparing the vector object before
		passing it to the function. (It's damn' crazy thing since it DOES work when the called function is not in
		DLL, but in this module.
	*/
	for ( UINT i = 0; i < ModConfig.uiCustomKeysCount; i++ ) 
	{ 
		Keys.push_back( i );
		KeyNames.push_back("");
	}
	pGameMod->GetCustomKeyNames( &KeyNames );
	pGameMod->GetCustomKeys( &Keys );

	if ( Keys.size() != ModConfig.uiCustomKeysCount || KeyNames.size() != ModConfig.uiCustomKeysCount )
	{
		ErrorHandler.HandleError( ERRGENERIC, "CCustomKeySetsDialog::SetInitialValues()", "Game module responded improperly." );
		return;
	}

	// Key value initialization
	for ( UINT i = 0; i < ModConfig.uiCustomKeysCount; i++ )
	{
		CKSKeyEdit[i]->SetValue( Configuration.UnParseKeyBindConfigPair( Keys[i] ) );
		CKSKeyLabel[i]->SetValue( KeyNames[i] );
	}
	
	// disable and hide those unused config boxes
	for ( UINT i = ModConfig.uiCustomKeysCount; i < MAX_CUSTOM_KEYS; i++ )
	{
		CKSKeyEdit[i]->SetInvisible();
		CKSKeyEdit[i]->Disable();
		CKSKeyLabel[i]->SetInvisible();
		CKSKeyLabel[i]->Disable();
	}
};

// checks, whether such key mapping is not used elsewhere
BOOL CCustomKeySetsDialog::CheckValue( graphic::CGUIControlEdit * ChangedEdit, WPARAM NewValue )
{
	CString InName;

	// check against classic game controls configuration
	if ( pDialogs->pKeySettingsDialog->CheckValue( ChangedEdit, NewValue ) ) return TRUE;
	
	InName = Configuration.UnParseKeyBindConfigPair( (int) NewValue );

	// P key is reserved for 'pause game'
	if ( InName == CString("P") ) return TRUE;

	// check against MOD controls configuration
	for ( UINT i = 0; i < ModConfig.uiCustomKeysCount; i++ )
	{
		if ( CKSKeyEdit[i]->GetValue() == InName && ChangedEdit != CKSKeyEdit[i] )
			return TRUE;
	}

	// in other cases, return false - this control is not being used
	return FALSE;
};

// Save custom key settings values 
HRESULT CCustomKeySetsDialog::SaveValues()
{
	CGameMod::INT_LIST	Keys;
	NAMEVALUEPAIR		Pair;
	
	// Update MOD configuration
	for ( UINT i = 0; i < ModConfig.uiCustomKeysCount; i++ )
		Keys.push_back( Configuration.ParseKeyBindConfigPair( CKSKeyEdit[i]->GetValue() ) );

	pGameMod->SetCustomKeys( &Keys );
	pGameMod->SaveConfig();

	// Refresh action mapping in controls
	gInput.RefreshActionMapping( &Configuration, &Keys );

	return 0;
};


HRESULT CDedicatedServerDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind)
{
	HRESULT hr;
	hr = CMyDialog::init("DedicatedServer.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "CDedicatedServerDialog::Init()", "Dedicated server dialog object could not be initialized." );

	GETDIALOGCONTROL2(DSServerRunningLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(DSStopServerButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(DSToLobbyButton, graphic::CGUIControlButton);

	DSStopServerButton->lpOnMouseClick = OnDSStopServerButtonClick;
	DSToLobbyButton->lpOnMouseClick = OnDSToLobbyButtonClick;

	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	return ERRNOERROR;
}

// Starts dedicated timer
void CDedicatedServerDialog::OnEnter()
{
	Timer = SetTimer(0, 0, 333, DedicatedServerTimerProc);
	GameUpdateTimer = SetTimer(0,0, 10, DedicatedServerGameUpdateTimerProc);
}

// Stops dedicated timer
void CDedicatedServerDialog::OnLeave()
{
	KillTimer(0, Timer);
	KillTimer(0, GameUpdateTimer);
}

// Game update timer procedure
void CALLBACK CDedicatedServerDialog::DedicatedServerGameUpdateTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time)
{
	HRESULT hr;

	hr = Server->updateGame();
	if (FAILED(hr) )
	{
		FreeGamePlayResources();
		ErrorHandler.HandleError(hr, "CDedicatedServerDialog::DedicatedServerGameUpdateTimerProc()", "Error while updating game");
		SHOWMESSAGEBOX("SERVER ERROR", "There was an error on server. Server stopped.", pDialogs->pMultiplayerDialog);
		return;
	}
}

// Timer procedure
void CALLBACK CDedicatedServerDialog::DedicatedServerTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time)
{
	BOOL SomeClientConnected;
	BOOL PlayerActive;
	BOOL ToLobby;
	BOOL AllToLobby;
	UINT i;

	// Check if some client is connected to server
	SomeClientConnected = FALSE;
	for (i = 0; i < MAX_PLAYERS; i++)
		if (Server->isConnected(i) )
		{
			SomeClientConnected = TRUE;
			break;
		}

	if (! SomeClientConnected)
	{
		// Stop server
		FreeGamePlayResources();
		SHOWMESSAGEBOX("NO CLIENT IS CONNECTED TO SERVER", "Server will now stop.", pDialogs->pMultiplayerDialog);
		return;
	}

	// Check if all player wants to return to lobby
	AllToLobby = TRUE;
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		Server->MultiPlayerInfo.GetPlayerActive(i, PlayerActive);
		if (PlayerActive)
		{
			Server->MultiPlayerInfo.GetReturnToLobby(i, ToLobby);
			if (ToLobby == FALSE)
			{
				AllToLobby = FALSE;
				break;
			}
		}
	}

	if (AllToLobby)
	{
		FreeGamePlayResources(TRUE);
		Server->SendServerReturnedToLobby();
		CHANGE_ACTDIALOG(pMultiStartGameDialog);
		return;
	}

}


// Init graphic settings dialog
HRESULT CGraphicSettingsDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager * _pRM,CTimeCounter * _pTimer,HWND _hWind)
{
	HRESULT hr;
	hr = CMyDialog::init("GraphicSettings.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "CGraphicSettingsDialog::Init()", "Graphic settings dialog object could not be initialized." );

	GETDIALOGCONTROL2( GSGraphicSettingsLabel,	graphic::CGUIControlLabel );
	GETDIALOGCONTROL2( GSResolutionLabel,		graphic::CGUIControlLabel );
	GETDIALOGCONTROL2( GSGammaLabel,			graphic::CGUIControlLabel );
	GETDIALOGCONTROL2( GSTerrainLabel,			graphic::CGUIControlLabel );
	GETDIALOGCONTROL2( GSModelsLabel,			graphic::CGUIControlLabel );
	GETDIALOGCONTROL2( GSGrassLabel,			graphic::CGUIControlLabel );
	GETDIALOGCONTROL2( GSTextureLabel,			graphic::CGUIControlLabel );
	GETDIALOGCONTROL2( GSAnisoLabel,			graphic::CGUIControlLabel );
	GETDIALOGCONTROL2( GSAntialiasLabel,		graphic::CGUIControlLabel );
	GETDIALOGCONTROL2( GSShadowsLabel,			graphic::CGUIControlLabel );
	GETDIALOGCONTROL2( GSSpecularLabel,			graphic::CGUIControlLabel );

	GETDIALOGCONTROL2( GSResolutionBox,	graphic::CGUIControlComboBox );
	GETDIALOGCONTROL2( GSTerrainBox,	graphic::CGUIControlComboBox );
	GETDIALOGCONTROL2( GSModelsBox,		graphic::CGUIControlComboBox );
	GETDIALOGCONTROL2( GSGrassBox,		graphic::CGUIControlComboBox );
	GETDIALOGCONTROL2( GSTextureBox,	graphic::CGUIControlComboBox );
	GETDIALOGCONTROL2( GSAnisoBox,		graphic::CGUIControlComboBox );
	GETDIALOGCONTROL2( GSAntialiasBox,	graphic::CGUIControlComboBox );

	GETDIALOGCONTROL2( GSShadowsBox,	graphic::CGUIControlCheckBox );
	GETDIALOGCONTROL2( GSSpecularBox,	graphic::CGUIControlCheckBox );

	GETDIALOGCONTROL2( GSGammaTrackBar,	graphic::CGUIControlTrackBar );

	GETDIALOGCONTROL2( GSOk,			graphic::CGUIControlButton );
	GETDIALOGCONTROL2( GSCancel,		graphic::CGUIControlButton );

	GSShadowsBox->lpOnChange = OnGSShadowsBoxChange;
	GSSpecularBox->lpOnChange = OnGSSpecularBoxChange;
	GSGammaTrackBar->lpOnChange = OnGSGammaTrackBarChange;

	GSOk->lpOnMouseClick = OnGSOkClick;
	GSCancel->lpOnMouseClick = OnGSCancelClick;

	// add resolution to combo box
	GSResolutionBox->AddLine( "  1600 x 1200" );
	GSResolutionBox->AddLine( "  1280 x 1024" );
	GSResolutionBox->AddLine( "  1024 x 768" );
	GSResolutionBox->AddLine( "  800 x 600" );
	GSResolutionBox->AddLine( "  640 x 480" );

	GSResolutionBox->lpOnChange = OnGSResolutionBoxChange;

	// add terrain detail selection
	GSTerrainBox->AddLine( "  Very low" );
	GSTerrainBox->AddLine( "  Low" );
	GSTerrainBox->AddLine( "  Medium" );
	GSTerrainBox->AddLine( "  High" );
	GSTerrainBox->AddLine( "  Very high" );

	GSTerrainBox->lpOnChange = OnGSTerrainBoxChange;

	// add model detail selection
	GSModelsBox->AddLine( "  Very low" );
	GSModelsBox->AddLine( "  Low" );
	GSModelsBox->AddLine( "  Medium" );
	GSModelsBox->AddLine( "  High" );
	GSModelsBox->AddLine( "  Very high" );

	GSModelsBox->lpOnChange = OnGSModelsBoxChange;

	// add grass complexity selection
	GSGrassBox->AddLine( "  Disabled" );
	GSGrassBox->AddLine( "  Very low" );
	GSGrassBox->AddLine( "  Low" );
	GSGrassBox->AddLine( "  Medium" );
	GSGrassBox->AddLine( "  High" );
	GSGrassBox->AddLine( "  Very high" );

	GSGrassBox->lpOnChange = OnGSGrassBoxChange;

	// add texture quality selection
	GSTextureBox->AddLine( "  Low" );
	GSTextureBox->AddLine( "  Medium" );
	GSTextureBox->AddLine( "  High" );

	GSTextureBox->lpOnChange = OnGSTextureBoxChange;

	// add anisotropic filtering selection
	GSAnisoBox->AddLine( "  Disabled" );
	GSAnisoBox->AddLine( "  Low" );
	GSAnisoBox->AddLine( "  Medium" );
	GSAnisoBox->AddLine( "  High" );
	GSAnisoBox->AddLine( "  Very high" );

	GSAnisoBox->lpOnChange = OnGSAnisoBoxChange;

	// add antialiasing selection
	GSAntialiasBox->AddLine( "  Disabled" );
	GSAntialiasBox->AddLine( "  Low" );
	GSAntialiasBox->AddLine( "  Medium" );
	GSAntialiasBox->AddLine( "  High" );
	GSAntialiasBox->AddLine( "  Very high" );

	GSAntialiasBox->lpOnChange = OnGSAntialiasBoxChange;

	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed

	return ERRNOERROR;
}

// Sets correct values when entering dialog
void CGraphicSettingsDialog::SetInitialValues()
{
	// Resolution
	if ( Configuration.WindowWidth == 1600 )
		GSResolutionBox->SetValue( 0 );
	else if ( Configuration.WindowWidth == 1280 )
		GSResolutionBox->SetValue( 1 );
	else if ( Configuration.WindowWidth == 1024 )
		GSResolutionBox->SetValue( 2 );
	else if ( Configuration.WindowWidth == 800 )
		GSResolutionBox->SetValue( 3 );
	else
		GSResolutionBox->SetValue( 4 );

	// Shadows
	GSShadowsBox->SetChecked( Configuration.ShadowComplexity != 0 );

	// per-pixel lighting for cars
	GSSpecularBox->SetChecked( Configuration.CarPerPixelLight != 0 );

	// terrain complexity
	if ( Configuration.TerrainComplexity >= 0 && Configuration.TerrainComplexity <= 4 )
		GSTerrainBox->SetValue( Configuration.TerrainComplexity );
	else GSTerrainBox->SetValue( 2 );
	
	// models complexity
	if ( Configuration.ModelComplexity >= 0 && Configuration.ModelComplexity <= 4 )
		GSModelsBox->SetValue( Configuration.ModelComplexity );
	else GSModelsBox->SetValue( 2 );

	// grass complexity
	if ( !Configuration.RenderGrass ) GSGrassBox->SetValue( 0 );
	else if ( Configuration.GrassComplexity >= 0 && Configuration.GrassComplexity <= 4 )
		GSGrassBox->SetValue( Configuration.GrassComplexity + 1 );
	else GSGrassBox->SetValue( 2 );

	// texture quality
	if ( Configuration.TextureQuality >= 0 && Configuration.TextureQuality <= 2 )
		GSTextureBox->SetValue( Configuration.TextureQuality );
	else GSTextureBox->SetValue( 1 );

	// anisotropic filtering
	if ( Configuration.AnisotropicFiltering / 2 == 0 )
		GSAnisoBox->SetValue( 0 );
	else if ( Configuration.AnisotropicFiltering / 2 == 1 )
		GSAnisoBox->SetValue( 1 );
	else if ( Configuration.AnisotropicFiltering / 4 == 1 )
		GSAnisoBox->SetValue( 2 );
	else if ( Configuration.AnisotropicFiltering / 8 == 1 )
		GSAnisoBox->SetValue( 3 );
	else if ( Configuration.AnisotropicFiltering / 16 >= 1 )
		GSAnisoBox->SetValue( 4 );
	else GSAnisoBox->SetValue( 2 );

	// antialiasing
	if ( !Configuration.Antialiasing ) GSAntialiasBox->SetValue( 0 );
	else
	{
		if ( Configuration.MultisampleQuality >= 0 && Configuration.MultisampleQuality <= 3 )
			GSAntialiasBox->SetValue( Configuration.MultisampleQuality + 1 );
		else if ( Configuration.MultisampleQuality > 3 )
			GSAntialiasBox->SetValue( 4 );
		else 
			GSAntialiasBox->SetValue( 1 );
	}

	// gamma factor
	if ( Configuration.GammaFactor <= 0.0f )
		GSGammaTrackBar->SetValue( 0.0f );
	else if ( Configuration.GammaFactor >= 2.0f )
		GSGammaTrackBar->SetValue( 1.0f );
	else GSGammaTrackBar->SetValue( Configuration.GammaFactor / 2.0f );


	return;
}

HRESULT CSoundSettingsDialog::init(graphic::CGraphic *_pGraphic, resManNS::CResourceManager * _pRM, CTimeCounter * _pTimer, HWND _hWind)
{
	HRESULT hr;
	hr = CMyDialog::init("SoundSettings.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "CSoundSettingsDialog::Init()", "Sound settings dialog object could not be initialized." );

	// Labels
	GETDIALOGCONTROL2(SSSoundSettingsLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(SSMusicVolumeLabel, graphic::CGUIControlLabel);
	
	// Trackbars
	GETDIALOGCONTROL2(SSMusicVolumeTrackBar, graphic::CGUIControlTrackBar);

	// Buttons
	GETDIALOGCONTROL2(SSOk, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(SSCancel, graphic::CGUIControlButton);

	// Set callback functions
	SSOk->lpOnMouseClick = OnSSOkClick;
	SSCancel->lpOnMouseClick = OnSSCancelClick;
	SSMusicVolumeTrackBar->lpOnChange = OnSSMusicVolumeChange;

	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed

	return ERRNOERROR;
}

// Sets correct values when entering dialog
void CSoundSettingsDialog::SetInitialValues()
{
	SSMusicVolumeTrackBar->SetValueSilent(Configuration.MusicVolume);
}

// Saves values
HRESULT COpponentsDialog::SaveValues()
{
	HRESULT hr;
	unsigned int i, j;
	CString Temp;
	NAMEVALUEPAIR Pair;
	char Number[10];
	CString OpponentName; 

	// Update configuration
	for (i = 0; i < ODOpponentsListBox->GetLinesCount(); i++)
	{
		// Find filename 
		for (j = 0; j < GameInfo.CarNames.size(); j++)
			if (GameInfo.CarNames[j].Name == ODOpponentsListBox->GetItemText(i))
				Temp = GameInfo.CarNames[j].FileName;

		Configuration.OpponentsFileNames[i] = Temp;
	}
	// Clear all other names before they are saved to options.cfg
	for (i = ODOpponentsListBox->GetLinesCount(); i < MAX_OPPONENTS; i++)
		Configuration.OpponentsFileNames[i] = "";

	// Save opponents configuration
	for (i = 0; i < MAX_OPPONENTS; i++)
	{
		Temp = CString("Opponent") + CString(_itoa(i, Number, 10) ) + CString("Car");
		Pair.SetName( Temp );
		Pair.SetString( Configuration.OpponentsFileNames[i]);
		hr = Pair.SaveValue("Options.cfg");
		if (FAILED(hr) ) ERRORMSG(hr, "COpponentsDialog::SaveValues()", "Unable to save configuration into options.cfg");
		
	}

	// Add to multiplayer info from back
	for (i = 0; i < MAX_OPPONENTS; i++)
	{
		if (Configuration.OpponentsFileNames[i] != "")
		{
			OpponentName = CString("Opponent") + CString(_itoa(i, Number, 10));
			Server->MultiPlayerInfo.SetPlayerStatus(MAX_PLAYERS - 1 - i, OpponentName,
				Configuration.OpponentsFileNames[i], -1, TRUE, TRUE);
			Server->MultiPlayerInfo.SetPlayerAI(MAX_PLAYERS - 1 - i, TRUE);
			Client->MultiPlayerInfo.SetPlayerStatus(MAX_PLAYERS - 1 - i, OpponentName,
				Configuration.OpponentsFileNames[i], -1, TRUE, TRUE);
			Client->MultiPlayerInfo.SetPlayerAI(MAX_PLAYERS - 1 - i, TRUE);
		}
		else
		{
			Server->MultiPlayerInfo.SetPlayerStatus(MAX_PLAYERS - 1 - i, CString(""),
				CString(""), -1, FALSE, FALSE);
			Server->MultiPlayerInfo.SetPlayerAI(MAX_PLAYERS - 1 - i, FALSE);
			Client->MultiPlayerInfo.SetPlayerStatus(MAX_PLAYERS - 1 - i, CString(""),
				CString(""), -1, FALSE, FALSE);
			Client->MultiPlayerInfo.SetPlayerAI(MAX_PLAYERS - 1 - i, FALSE);

		}
	}
	
	return 0;
}

// Delete selected opponent
void COpponentsDialog::DeleteOpponent() 
{
	int i;

	if ( ODOpponentsListBox->GetLinesCount() <= 0 ) return;

	for (i = ODOpponentsListBox->GetValue(); i < (int) ODOpponentsListBox->GetLinesCount() - 1; i++)
	{
		ODOpponentsListBox->GetItemText(i) = ODOpponentsListBox->GetItemText(i + 1);
	}

	// change the selection
	if ( ODOpponentsListBox->GetValue() > 0 && ODOpponentsListBox->GetValue() == ODOpponentsListBox->GetLinesCount() - 1 ) 
		ODOpponentsListBox->SetValueSilent( ODOpponentsListBox->GetValue() - 1 );

	ODOpponentsListBox->PopBack();
}

// Adds selected car as next opponent
void COpponentsDialog::AddOpponent()
{
	// Add only to max opponents, no more
	if (ODOpponentsListBox->GetLinesCount() < MAX_OPPONENTS)
	{
		ODOpponentsListBox->PushBack( ODCarsListBox->GetActualString() );
	}
}

// Sets correct values when entering dialog
void COpponentsDialog::SetInitialValues()
{
	int i, j;

	// Search all opponents saved in configuration and add, when possible
	ODOpponentsListBox->Clear();
	for (i = 0; i < MAX_OPPONENTS; i++)
		if (Configuration.OpponentsFileNames[i] != "")
			for (j = 0; j < (int) GameInfo.CarNames.size(); j++ )
				if (Configuration.OpponentsFileNames[i].MakeUpper() == GameInfo.CarNames[j].FileName.MakeUpper())
					ODOpponentsListBox->PushBack(GameInfo.CarNames[j].Name);
	
}


// init opponents dialog
HRESULT COpponentsDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind)
{
	HRESULT hr;
	hr = CMyDialog::init("Opponents.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "COpponentsDialog::Init()", "Opponents dialog object could not be initialized." );

	// labels
	GETDIALOGCONTROL2(ODOpponentsSettingLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(ODOpponentsLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(ODCarsLabel, graphic::CGUIControlLabel);

	// list boxes
	GETDIALOGCONTROL2(ODOpponentsListBox, graphic::CGUIControlListBox);
	GETDIALOGCONTROL2(ODCarsListBox, graphic::CGUIControlListBox);

	// buttons
	GETDIALOGCONTROL2(ODLeftArrowButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(ODRightArrowButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(ODOkButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(ODCancelButton, graphic::CGUIControlButton);

	// add actions
	ODOkButton->lpOnMouseClick = OnODOkButtonClick;
	ODCancelButton->lpOnMouseClick = OnODCancelButtonClick;
	ODLeftArrowButton->lpOnMouseClick = OnODLeftArrowButtonClick;
	ODRightArrowButton->lpOnMouseClick = OnODRightArrowButtonClick;

	// init car names list
	vector<CGameInfo::NameToFileNameMapping>::iterator It;
	ODCarsListBox->Clear();
	for (It = GameInfo.CarNames.begin(); It != GameInfo.CarNames.end(); It++)
	{
		ODCarsListBox->PushBack( It->Name );
	}
	
	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	return ERRNOERROR;
}

// Checks, whether setting is correct
HRESULT CSinglePlayerDialog::CheckValues()
{
	if (SPDayHoursEditBox->GetValueInt() > 23 || SPDayHoursEditBox->GetValue() == "")
	{
		SHOWMESSAGEBOX("ERROR", "Hours of day must be between 0 and 23", this);
		return -1;
	}

	if (SPDayMinutesEditBox->GetValueInt() > 59 || SPDayMinutesEditBox->GetValue() == "")
	{
		SHOWMESSAGEBOX("ERROR", "Minutes of day must be between 0 and 59", this);
		return -1;
	}

	if (SPLapsModeCheckBox->IsChecked() && SPLapsEditBox->GetValueInt() <= 0)
	{
		SHOWMESSAGEBOX("ERROR", "Laps must be greater than zero.", this);
		return -1;
	}


	return 0;
}

// Init single player dialog
HRESULT CSinglePlayerDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind)
{
	HRESULT hr;
	hr = CMyDialog::init("SinglePlayer.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "CSinglePlayerDialog::Init()", "Single player dialog object could not be initialized." );

	// labels
	GETDIALOGCONTROL2(SPSinglePlayerLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(SPPlayerNameLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(SPCarLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(SPMapLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(SPTimeLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(SPLapsLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(SPCircuitModeLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(SPOpponentsLabel, graphic::CGUIControlLabel);
	
	// list boxes
	GETDIALOGCONTROL2(SPMapListBox, graphic::CGUIControlListBox);
	GETDIALOGCONTROL2(SPCarListBox, graphic::CGUIControlListBox);

	// Edit boxes
	GETDIALOGCONTROL2(SPPlayerNameEditBox, graphic::CGUIControlEdit);
	GETDIALOGCONTROL2(SPDayHoursEditBox, graphic::CGUIControlEdit);
	GETDIALOGCONTROL2(SPDayMinutesEditBox, graphic::CGUIControlEdit);
	GETDIALOGCONTROL2(SPLapsEditBox, graphic::CGUIControlEdit);

	// check boxes
	GETDIALOGCONTROL2(SPLapsModeCheckBox, graphic::CGUIControlCheckBox);

	// buttons
	GETDIALOGCONTROL2(SPStartButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(SPBackButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(SPOpponentsButton, graphic::CGUIControlButton);
	
	// actions
	SPStartButton->lpOnMouseClick = OnSPStartButtonClick;
	SPBackButton->lpOnMouseClick = OnSPBackButtonClick;
	SPDayHoursEditBox->lpOnChange = OnOnlyNumbers;
	SPDayMinutesEditBox->lpOnChange = OnOnlyNumbers;
	SPLapsEditBox->lpOnChange = OnOnlyNumbers;
	SPOpponentsButton->lpOnMouseClick = OnOpponentsButtonClick;
	SPMapListBox->lpOnChange = OnMapListBoxChange;
	
	// Init map names
	ReloadMapList( SPMapListBox );

	// Init car names
	std::vector<CGameInfo::NameToFileNameMapping>::iterator		It;

	SPCarListBox->Clear();
	for (It = GameInfo.CarNames.begin(); It != GameInfo.CarNames.end(); It++)
	{
		SPCarListBox->PushBack( It->Name );
	}

	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed

	return ERRNOERROR;
};

// Filters out text and lets only number in string
void CMyDialog::OnlyNumbers(CString & Text)
{
	CString NewText;

	for (int i = 0; i < Text.GetLength(); i++)
	{
		if ((Text[i] >= '0' && Text[i] <= '9') )
			NewText.AppendChar(Text[i]);
	}

	Text = NewText;
}

// sets correct values when entering dialog
void CSinglePlayerDialog::SetInitialValues()
{
	if ( SPMapListBox->GetLinesCount() > 0 )
	{
		SPMapListBox->SetValue( SPMapListBox->GetValue() );
	}

	if (Configuration.PlayerName == "")
		SPPlayerNameEditBox->SetValue("Player");
	else
		SPPlayerNameEditBox->SetValue(Configuration.PlayerName);
	
	/*
	// Try to set previous setting
	// for player name
	SPPlayerNameEditBox->SetValue(Configuration.PlayerName);

	// for map
	for (i = 0; i < GameInfo.MapNames.size(); i++)
	{
		if (!strcmp(GameInfo.MapNames[i].FileName.MakeUpper(), Configuration.MapFileName.MakeUpper()) )
		{
			SPMapListBox->SetValue(i);
			break;
		}
	}
	
	// for player car
	for (i = 0; i < GameInfo.CarNames.size(); i++)
	{
		if (!strcmp(GameInfo.CarNames[i].FileName.MakeUpper(), Configuration.CarFileName.MakeUpper()  ) )
		{
			SPCarListBox->SetValue(i);
			break;
		}
	}
	
	// for time of day
	char Temp[255];
	//SPDayHoursEditBox->SetValueSilent( _itoa(Configuration.HoursOfDay, Temp,10 ) );
	//SPDayMinutesEditBox->SetValueSilent( _itoa(Configuration.MinutesOfDay, Temp, 10) );

	// for race mode
	SPLapsModeCheckBox->SetChecked( Configuration.LapsEnabled != 0);
	SPTimeModeCheckBox->SetChecked( Configuration.TimeEnabled != 0);
	SPLapsEditBox->SetValue( _itoa(Configuration.Laps, Temp, 10) );
	SPMinutesEditBox->SetValue( _itoa(Configuration.Time, Temp, 10) );
	*/
}

// saves values
HRESULT CSinglePlayerDialog::SaveValues()
{
	HRESULT hr;

	NAMEVALUEPAIR Pair;

	// Update configuration
	Configuration.PlayerName = SPPlayerNameEditBox->GetValueString();

	// save
	Pair.SetName("PlayerName");
	Pair.SetString(Configuration.PlayerName);
	hr = Pair.SaveValue("options.cfg");
	if (FAILED(hr) ) ERRORMSG(hr, "CSinglePlayerDialog::SaveValues()", "Unable to save configuration into options.cfg");

	Client->ChangeState(CS_SG_START);
	Server->ChangeServerState(SS_SG_START);

	// Set map name
	hr = Client->MultiPlayerInfo.SetMapFileName(GameInfo.MapNames[SPMapListBox->GetActualItem().iValue].FileName);
	if (FAILED(hr) )
		ERRORMSG(hr, "CSinglePlayerDialog::SaveValues()", "Unable to save map filename into client.");
	hr = Server->MultiPlayerInfo.SetMapFileName(GameInfo.MapNames[SPMapListBox->GetActualItem().iValue].FileName);
	if (FAILED(hr) )
		ERRORMSG(hr, "CSinglePlayerDialog::SaveValues()", "Unable to save map filename into server.");

	// fetch all cars
	Client->MultiPlayerInfo.FetchAllCars();
	Server->MultiPlayerInfo.FetchAllCars();

	// Set car name - alla active single player 
	Client->MultiPlayerInfo.SetPlayerStatus(0, Configuration.PlayerName, 
		GameInfo.CarNames[SPCarListBox->GetValue()].FileName, -1, TRUE, TRUE);
	Server->MultiPlayerInfo.SetPlayerStatus(0, Configuration.PlayerName, 
		GameInfo.CarNames[SPCarListBox->GetValue()].FileName, -1, TRUE, TRUE);
	
	// Set time of day, lap mode and number of laps
//RACER MODE ONLY:
	Client->MultiPlayerInfo.SetGameSettings( SPDayHoursEditBox->GetValueInt(), SPDayMinutesEditBox->GetValueInt() );
	Server->MultiPlayerInfo.SetGameSettings( SPDayHoursEditBox->GetValueInt(), SPDayMinutesEditBox->GetValueInt() );

	// add opponents
	hr = pDialogs->pOpponentsDialog->SaveValues();
	if (FAILED(hr) )
		ERRORMSG(hr, "CSinglePlayerDialog::SaveValues()", "Unable to add AI opponents to single player.");

	return 0;
}

// Init settings dialog
HRESULT CSettingsDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind)
{
	HRESULT hr;
	hr = CMyDialog::init("Settings.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "GraphicTest::Init()", "Settings dialog object could not be initialized." );

	GETDIALOGCONTROL(SettingsLabel, graphic::CGUIControlLabel, SettingsLabel);
	GETDIALOGCONTROL(GraphicSettings, graphic::CGUIControlButton, GraphicSettings);
	GETDIALOGCONTROL(KeySettings, graphic::CGUIControlButton, KeySettings);
	GETDIALOGCONTROL(MainMenu, graphic::CGUIControlButton, MainMenu);
	GETDIALOGCONTROL2(SoundSettings, graphic::CGUIControlButton);

	GraphicSettings->lpOnMouseClick = OnGraphicSettingsClick;
	SoundSettings->lpOnMouseClick = OnSoundSettingsClick;
	KeySettings->lpOnMouseClick = OnKeySettingsClick;
	MainMenu->lpOnMouseClick = OnMainMenuClick;

	ReturnToDialog = pDialogs->pMainDialog;

	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	return ERRNOERROR;
}


HRESULT CMainDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind){
	
	HRESULT hr;
	hr = CMyDialog::init("main.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "GraphicTest::Init()", "My dialog object could not be initialized." );
	
	GETDIALOGCONTROL(MainLabel,CGUIControlLabel ,TopLabel)
	GETDIALOGCONTROL(SingleStartButton,CGUIControlButton,SingleStartButton)
	GETDIALOGCONTROL(MultiStartButton,CGUIControlButton,MultiStartButton)
	GETDIALOGCONTROL(SettingsButton,CGUIControlButton,SettingsButton)
	GETDIALOGCONTROL(ExitButton,CGUIControlButton,ExitButton)
	GETDIALOGCONTROL2(CreditsButton, graphic::CGUIControlButton);
	
	SingleStartButton->lpOnMouseClick = OnSingleStartButtonClick;
	MultiStartButton->lpOnMouseClick = OnMultiStartButtonClick;
	SettingsButton->lpOnMouseClick = OnSettingsButtonClick;
	ExitButton->lpOnMouseClick = OnExitButtonClick;
	CreditsButton->lpOnMouseClick = OnCreditsButtonClick;
	
	
	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	return ERRNOERROR;

}

HRESULT CWaitDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind){
	
	HRESULT hr;
	hr = CMyDialog::init("wait.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "GraphicTest::Init()", "My dialog object could not be initialized." );
	
	GETDIALOGCONTROL(MainLabel,CGUIControlLabel ,TopLabel)
	GETDIALOGCONTROL2(ProgressLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(DotLabel, graphic::CGUIControlLabel);
	DotLabel->SetValue("");
	
	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	return ERRNOERROR;
}

// Starts timer
void CWaitDialog::OnEnter()
{
	Timer = SetTimer(0, 0, 333, WaitDialogTimerProc);
	pDialogs->pGraphic->SetMouseVisible(false);
	// Clear dot label
	DotLabel->SetValue("");
}

// Ends timer
void CWaitDialog::OnLeave()
{
	KillTimer(0, Timer);
	pDialogs->pGraphic->SetMouseVisible(true);
}

// callback function for reporting loading status
void CALLBACK CWaitDialog::LoadingProgress(CString Progress)
{
	pDialogs->pWaitDialog->ProgressLabel->SetValue(Progress);
	// Add dot into dot label
	pDialogs->pWaitDialog->DotLabel->SetValue(pDialogs->pWaitDialog->DotLabel->GetValue() + CString(".") );

	pDialogs->pGraphic->RenderDialogOnly();
}

void CALLBACK CWaitDialog::WaitDialogTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time)
{
	UINT i;
	BOOL SomePlayerConnected;

	// Test whether client is connected to server
	if (! pDialogs->pMultiStartGameDialog->Dedicated)
	{
		if (! Client->isConnected() )
		{
			// Free all game play resources
			FreeGamePlayResources();
			
			Client->ChangeState(CS_MG_ENUMERATE_HOSTS);
			SHOWMESSAGEBOX("ERROR CONNECTION TO SERVER", "You are no longer connected to server.", pDialogs->pMultiplayerDialog);
			return;
		}
	}
	
	if (pDialogs->pMultiStartGameDialog->Server)
	{
		// Test whether some player is connected to dedicated server
		if (pDialogs->pMultiStartGameDialog->Dedicated)
		{
			SomePlayerConnected = FALSE;
			for (i = 0; i < MAX_PLAYERS; i++)
				if (Server->isConnected(i) )
				{
					SomePlayerConnected = TRUE;
					break;
				}

			if (! SomePlayerConnected)
			{
				stopServer();
				// Free all game play resources
				FreeGamePlayResources();
				SHOWMESSAGEBOX("ERROR CONNECTION OF PLAYERS", "No player is connected to dedicated server.", pDialogs->pMultiplayerDialog);
				return;
			}
		}

		// Test whether all players are loaded
		Server->CheckAllPlayersLoaded();
	}

	// Test whether all players are loaded
	// if so, start game
	if (Client->MultiPlayerInfo.GetState() == CMultiPlayerGameInfo::PHASE_FIVE)
	{
		gameState = GS_PLAY;
		if (! pDialogs->pMultiStartGameDialog->Dedicated)
		{
			CHANGE_ACTDIALOG(pInGameDialog);
			pDialogs->hideDialog();
		}
	}
	else if (pDialogs->pMultiStartGameDialog->Dedicated)
	{
		if (::Server->MultiPlayerInfo.GetState() == CMultiPlayerGameInfo::PHASE_FIVE)
		{
			// Start dedicated server dialog, when dedicated server
			CHANGE_ACTDIALOG(pDedicatedServerDialog);
		}
	}
}



HRESULT CInGameDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind){
	
	HRESULT hr;
	hr = CMyDialog::init("ingamemenu.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "GraphicTest::Init()", "My dialog object could not be initialized." );
	
	GETDIALOGCONTROL(SettingsButton,CGUIControlButton,SettingsButton)
	GETDIALOGCONTROL(ExitButton,CGUIControlButton,ExitButton)
	GETDIALOGCONTROL(CloseButton,CGUIControlButton,CloseButton)

	SettingsButton->lpOnMouseClick = OnSettingsButtonClick;
	ExitButton->lpOnMouseClick = OnMainMenuButtonClick;
	CloseButton->lpOnMouseClick = OnCloseButtonClick;
	
	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	return ERRNOERROR;

};

void CMultiplayerDialog::OnEnter(){
	Timer = SetTimer( 0,0, 2000,BroadcastTimerProc);
	// Start broadcast
	Client->ChangeState(CS_MG_ENUMERATE_HOSTS);
}
void CMultiplayerDialog::OnLeave(){
	KillTimer(0,Timer ); 
	Client->StopBroadcast();
}
void CALLBACK CMultiplayerDialog::BroadcastTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time){
	Client->UpdateServers();
	Client->SendServersInfoDemand();
	pDialogs->pMultiplayerDialog->RefreshList(Client);
};
HRESULT CMultiplayerDialog::RefreshList(CClient * pCl)
{
	set<CClient::CServerInfoWithTime>::iterator It;
	u_long				SelectedIP;
	CString				csName;

	// clear game list
	MPGameListList->Clear();

	if (ServerList.size() != 0)
	{
		SelectedIP = ServerList[MPGameListList->GetValue()].addr;
		ServerList.clear();
	}
	else 
	{
		SelectedIP = 0;
	}

	// Add all server names into game list
	for (It = pCl->ServerList.begin(); It != pCl->ServerList.end(); It++)
	{
		csName  = It->ServerInfo.name;
		csName += " <";
		csName += It->ServerInfo.type;
		csName += ">";
		
		MPGameListList->PushBack(csName);
		
		// Save actual server info
		ServerList.push_back(It->ServerInfo);
		if (SelectedIP == It->ServerInfo.addr)
			MPGameListList->SetValueSilent((int) MPGameListList->GetLinesCount() - 1);
	}

	MPGameListList->SetValueSilent(MPGameListList->GetValue() );

	return ERRNOERROR;
};

// Multiplayer wait dialog
HRESULT CMultiplayerWaitDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind)
{
	HRESULT hr;
	hr = CMyDialog::init("MultiWait.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "CMultiplayerWaitDialog::Init()", "Multiplayer wait dialog could not be initialized." );

	GETDIALOGCONTROL2(MWMessageTypeLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MWMessageLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MWCancelButton, graphic::CGUIControlButton);

	MWCancelButton->lpOnMouseClick = OnMWCancelButtonClick;

	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	return ERRNOERROR;
}

// Starts timer for checking connection process status
void CMultiplayerWaitDialog::OnEnter()
{
	// Set timer
	Timer = SetTimer(0, 0, 500, MultiplayerWaitTimerProc);
}

// Kills times
void CMultiplayerWaitDialog::OnLeave()
{
	KillTimer(0, Timer);
}

// Timer procedure
void CALLBACK CMultiplayerWaitDialog::MultiplayerWaitTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time)
{
	CMultiplayerWaitDialog * WaitDialog;
	WaitDialog = pDialogs->pMultiplayerWaitDialog;

	// Server is starting
	if (WaitDialog->Server)
	{
		// NOT Dedicated
		if (! WaitDialog->Dedicated)
		{
			if(::Server->MultiPlayerInfo.GetState() == CMultiPlayerGameInfo::PHASE_THREE &&
				Client->MultiPlayerInfo.GetState() == CMultiPlayerGameInfo::PHASE_THREE)
			{
				pDialogs->pMultiPlayerSettingsDialog->SetInitialValues();
				Client->StartPing();
				CHANGE_ACTDIALOG(pMultiPlayerSettingsDialog);
			}
		}
		// Dedicated
		else
		{
			if (::Server->MultiPlayerInfo.GetState() == CMultiPlayerGameInfo::PHASE_THREE)
			{
				
				// Activate lobby updating
				CHANGE_ACTDIALOG(pMultiStartGameDialog);
			}
		}
	}
	// Client
	else
	{
		if (Client->MultiPlayerInfo.GetState() == CMultiPlayerGameInfo::PHASE_THREE)
		{
			pDialogs->pMultiPlayerSettingsDialog->SetInitialValues();
			Client->StartPing();
			CHANGE_ACTDIALOG(pMultiPlayerSettingsDialog);
			return;
		}
		if (! Client->isConnected())
		{
			SHOWMESSAGEBOX("ERROR WHILE JOINING", "There was an error while joining session.", pDialogs->pMultiplayerDialog);
			return;
		}
	}
}

// Set messages along mode
void CMultiplayerWaitDialog::SetMode(BOOL _Server, BOOL _Dedicated)
{
	Server = _Server;
	Dedicated = _Dedicated;

	if (Server)
	{
		MWMessageTypeLabel->SetValue("PLEASE WAIT - STARTING SERVER");
		MWMessageLabel->SetValue("Press cancel to cancel server creation.");
	}
	else
	{
		MWMessageTypeLabel->SetValue("PLESE WAIT - JOINING SERVER");
		MWMessageLabel->SetValue("Press cancel to cancel joining.");
	}
}

HRESULT CMultiplayerDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind){
	
	HRESULT hr;
	hr = CMyDialog::init("multidialog.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "CMultiplayerDialog::Init()", "Multiplayer dialog could not be initialized." );
	
	// Labels
	GETDIALOGCONTROL2(MPMultiplayerLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MPGameListLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MPIPLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MPPortLabel, graphic::CGUIControlLabel);
	// edits
	GETDIALOGCONTROL2(MPIPEdit, graphic::CGUIControlEdit);
	GETDIALOGCONTROL2(MPPortEdit, graphic::CGUIControlEdit);
	// list box
	GETDIALOGCONTROL2(MPGameListList, graphic::CGUIControlListBox);
	// Buttons
	GETDIALOGCONTROL2(MPCreateButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(MPJoinButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(MPBackButton, graphic::CGUIControlButton);

	// Events
	MPCreateButton->lpOnMouseClick = OnMPCreateButtonClick;
	MPJoinButton->lpOnMouseClick = OnMPJoinButtonClick;
	MPBackButton->lpOnMouseClick = OnMPBackButtonClick;
	MPGameListList->lpOnChange = OnMPGameListListChange;
	
	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	return ERRNOERROR;

}

HRESULT CToLobbyWaitDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind)
{
	HRESULT hr;
	
	hr = CMyDialog::init("ToLobbyWait.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "CToLobbyWaitDialog::init()", "To lobby wait dialog could not be initialized." );

	// labels
	GETDIALOGCONTROL2(LWTopLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(LWWaitingLabel, graphic::CGUIControlLabel);
	// button
	GETDIALOGCONTROL2(LWToMainMenuButton, graphic::CGUIControlButton);

	// Events
	LWToMainMenuButton->lpOnMouseClick = OnRRToMainMenuButtonClick;

	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	return ERRNOERROR;
}

// On enter of to lobby waiting
void CToLobbyWaitDialog::OnEnter()
{
	// Set timer
	Timer = SetTimer(0, 0, 333, ToLobbyWaitTimerProc);
}

// Kills timer
void CToLobbyWaitDialog::OnLeave()
{
	KillTimer(0, Timer);
}

// To lobby wait timer procedure
void CALLBACK CToLobbyWaitDialog::ToLobbyWaitTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time)
{
	if (Client->MultiPlayerInfo.GetServerReturnedToLobby() )
	{
		FreeGamePlayResources(TRUE);
		CHANGE_ACTDIALOG(pMultiStartGameDialog);
	}

	if (!Client->isConnected() )
	{
		FreeGamePlayResources();
		SHOWMESSAGEBOX("SERVER NO LONGER EXISTS", "Server is no longer available.", pDialogs->pMainDialog);
	}
}



HRESULT CRaceResultDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind)
{
	HRESULT hr;
	
	hr = CMyDialog::init("RaceResult.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "CRaceResultDialog::init()", "Race result dialog could not be initialized." );

	// labels
	GETDIALOGCONTROL2(RRRaceResultLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(RRPositionLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(RRPlayerNameLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(RRBestLapTimeLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(RRRaceTimeLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL(RRPosLabel[0], graphic::CGUIControlLabel, RRPos1Label);
	GETDIALOGCONTROL(RRPosLabel[1], graphic::CGUIControlLabel, RRPos2Label);
	GETDIALOGCONTROL(RRPosLabel[2], graphic::CGUIControlLabel, RRPos3Label);
	GETDIALOGCONTROL(RRPosLabel[3], graphic::CGUIControlLabel, RRPos4Label);
	GETDIALOGCONTROL(RRPosLabel[4], graphic::CGUIControlLabel, RRPos5Label);
	GETDIALOGCONTROL(RRPosLabel[5], graphic::CGUIControlLabel, RRPos6Label);
	GETDIALOGCONTROL(RRPosLabel[6], graphic::CGUIControlLabel, RRPos7Label);
	GETDIALOGCONTROL(RRPosLabel[7], graphic::CGUIControlLabel, RRPos8Label);
	GETDIALOGCONTROL(RRNameLabel[0], graphic::CGUIControlLabel, RRName1Label);
	GETDIALOGCONTROL(RRNameLabel[1], graphic::CGUIControlLabel, RRName2Label);
	GETDIALOGCONTROL(RRNameLabel[2], graphic::CGUIControlLabel, RRName3Label);
	GETDIALOGCONTROL(RRNameLabel[3], graphic::CGUIControlLabel, RRName4Label);
	GETDIALOGCONTROL(RRNameLabel[4], graphic::CGUIControlLabel, RRName5Label);
	GETDIALOGCONTROL(RRNameLabel[5], graphic::CGUIControlLabel, RRName6Label);
	GETDIALOGCONTROL(RRNameLabel[6], graphic::CGUIControlLabel, RRName7Label);
	GETDIALOGCONTROL(RRNameLabel[7], graphic::CGUIControlLabel, RRName8Label);
	GETDIALOGCONTROL(RRBestLapLabel[0], graphic::CGUIControlLabel, RRBestLap1Label);
	GETDIALOGCONTROL(RRBestLapLabel[1], graphic::CGUIControlLabel, RRBestLap2Label);
	GETDIALOGCONTROL(RRBestLapLabel[2], graphic::CGUIControlLabel, RRBestLap3Label);
	GETDIALOGCONTROL(RRBestLapLabel[3], graphic::CGUIControlLabel, RRBestLap4Label);
	GETDIALOGCONTROL(RRBestLapLabel[4], graphic::CGUIControlLabel, RRBestLap5Label);
	GETDIALOGCONTROL(RRBestLapLabel[5], graphic::CGUIControlLabel, RRBestLap6Label);
	GETDIALOGCONTROL(RRBestLapLabel[6], graphic::CGUIControlLabel, RRBestLap7Label);
	GETDIALOGCONTROL(RRBestLapLabel[7], graphic::CGUIControlLabel, RRBestLap8Label);
	GETDIALOGCONTROL(RRRaceLabel[0], graphic::CGUIControlLabel, RRRace1Label);
	GETDIALOGCONTROL(RRRaceLabel[1], graphic::CGUIControlLabel, RRRace2Label);
	GETDIALOGCONTROL(RRRaceLabel[2], graphic::CGUIControlLabel, RRRace3Label);
	GETDIALOGCONTROL(RRRaceLabel[3], graphic::CGUIControlLabel, RRRace4Label);
	GETDIALOGCONTROL(RRRaceLabel[4], graphic::CGUIControlLabel, RRRace5Label);
	GETDIALOGCONTROL(RRRaceLabel[5], graphic::CGUIControlLabel, RRRace6Label);
	GETDIALOGCONTROL(RRRaceLabel[6], graphic::CGUIControlLabel, RRRace7Label);
	GETDIALOGCONTROL(RRRaceLabel[7], graphic::CGUIControlLabel, RRRace8Label);
	
	GETDIALOGCONTROL2(RRLapRecordLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(RRLapRecordPlayerLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(RRLapRecordTimeLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(RRRaceRecordLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(RRRaceRecordPlayerLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(RRRaceRecordTimeLabel, graphic::CGUIControlLabel);

	// buttons
	GETDIALOGCONTROL2(RRToLobbyButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(RRToMainMenuButton, graphic::CGUIControlButton);

	// add events
	RRToLobbyButton->lpOnMouseClick = OnRRToLobbyButtonClick;
	RRToMainMenuButton->lpOnMouseClick = OnRRToMainMenuButtonClick;

	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	return ERRNOERROR;
}

// Clears names in race results
void CRaceResultDialog::ClearRaceResults()
{
	UINT i;

	for (i = 0; i < 8; i++)
		RRNameLabel[i]->SetValue(CString("") );
}

// Converts time in miliseconds into strinf formated: minutes:seconds.hundreths of seconds
CString CRaceResultDialog::ConvertTime(APPRUNTIME Time)
{
	CString ConvertedTime;
	char Num[20];
	UINT HundrethsPart;
	UINT SecondsPart;
	UINT MinutesPart;

	HundrethsPart = (Time % 1000) / 10;
	SecondsPart = (Time % 60000) / 1000;
	MinutesPart = Time / 60000;

	ConvertedTime = CString(_itoa((int) MinutesPart, Num, 10)) + CString(":") + CString(_itoa((int) SecondsPart, Num, 10) ) + CString(".") + 
		CString(_itoa((int) HundrethsPart, Num, 10) );

	return ConvertedTime;
};

// Updates race results
void CRaceResultDialog::UpdateRaceResults()
{
/*
//RACER MOD ONLY:
	UINT i;

	for (i = 0; i < (UINT) Client->MultiPlayerInfo.FinalPosition.size(); i++)
		RRNameLabel[i]->SetValue(Client->MultiPlayerInfo.FinalPosition[i]);
	for (i = 0; i < (UINT) Client->MultiPlayerInfo.FinalBestLapTime.size(); i++)
		RRBestLapLabel[i]->SetValue(ConvertTime(Client->MultiPlayerInfo.FinalBestLapTime[i]));
	for (i = 0; i < (UINT) Client->MultiPlayerInfo.FinalRaceTime.size(); i++)
		RRRaceLabel[i]->SetValue(ConvertTime(Client->MultiPlayerInfo.FinalRaceTime[i]));

	UpdateRecords();
*/
};

// Updates records
void CRaceResultDialog::UpdateRecords()
{
/*
//RACER MOD ONLY:
	BOOL NewRaceRecord = FALSE;
	BOOL NewLapRecord = FALSE;
	CGameInfo::CMapEntry objMapEntry;
	CGameInfo::CLapEntry objLapEntry;
	UINT i;
	
	// Do we have some race time
	if (Client->MultiPlayerInfo.FinalRaceTime.size() != 0)
	{
		// new race record???
		if (Client->MultiPlayerInfo.FinalRaceTime[0] < this->RecordRaceTime)
		{
			RecordRaceTime = Client->MultiPlayerInfo.FinalRaceTime[0];
			RecordRacePlayerName = Client->MultiPlayerInfo.FinalPosition[0];
			NewRaceRecord = TRUE;
		}
	}

	// new best lap record???
	for (i = 0; i < Client->MultiPlayerInfo.FinalBestLapTime.size(); i++)
		if (Client->MultiPlayerInfo.FinalBestLapTime[i] < this->RecordLapTime)
		{
			RecordLapTime = Client->MultiPlayerInfo.FinalBestLapTime[i];
			RecordLapPlayerName = Client->MultiPlayerInfo.FinalPosition[i];
			NewLapRecord = TRUE;
		}

	// Save back to game info
	if (NewLapRecord || NewRaceRecord)
	{
		objMapEntry.BestLapTime = RecordLapTime;
		objMapEntry.PlayerName = RecordLapPlayerName;
		objLapEntry.BestRaceTime = RecordRaceTime;
		objLapEntry.PlayerName = RecordRacePlayerName;
		objLapEntry.NumOfLaps = Client->MultiPlayerInfo.GetNumberOfLaps();
		
		if (MapEntry == -1)
		{
			objMapEntry.MapFileName = Client->MultiPlayerInfo.GetMapFileName();
			objMapEntry.LapEntries.push_back(objLapEntry);
			GameInfo.BestResults.push_back(objMapEntry);
			MapEntry = 0;
			LapEntry = 0;
		}
		else
		{
			if (NewLapRecord)
			{
				GameInfo.BestResults[MapEntry].BestLapTime = RecordLapTime;
				GameInfo.BestResults[MapEntry].PlayerName = RecordLapPlayerName;
			}
			if (NewRaceRecord)
			{
				if (LapEntry == -1)
				{
					GameInfo.BestResults[MapEntry].LapEntries.push_back(objLapEntry);
					LapEntry = (int) GameInfo.BestResults[MapEntry].LapEntries.size() - 1;
				}
				else
				{
					GameInfo.BestResults[MapEntry].LapEntries[LapEntry].BestRaceTime = RecordRaceTime;
					GameInfo.BestResults[MapEntry].LapEntries[LapEntry].PlayerName = RecordRacePlayerName;
				}
			}
		}

		// Save new best results
		GameInfo.SaveBestResults();
	}

	// Update dialog labels
	this->RRLapRecordTimeLabel->SetValue(ConvertTime(RecordLapTime) );
	this->RRLapRecordPlayerLabel->SetValue(RecordLapPlayerName);
	this->RRRaceRecordTimeLabel->SetValue(ConvertTime(RecordRaceTime) );
	this->RRRaceRecordPlayerLabel->SetValue(RecordRacePlayerName);
*/
}

// On enter of race results
void CRaceResultDialog::OnEnter()
{
/*
//RACER MOD ONLY:
	UINT i, e;
	CString MapFileName;
	UINT NumOfLaps;
	ClearRaceResults();

	// arrange buttons
	if (Client->IsSinglePlayer() )
	{
		RRToLobbyButton->SetInvisible();
		RRToLobbyButton->Disable();
		RRToMainMenuButton->SetPosition(240, 440);
	}
	else
	{
		RRToLobbyButton->SetVisible();
		RRToLobbyButton->Enable();
		RRToMainMenuButton->SetPosition(400, 440);
	}

	// Clear race results
	for (i = 0; i < 8; i++)
	{
		RRNameLabel[i]->SetValue("");
		RRBestLapLabel[i]->SetValue("");
		RRRaceLabel[i]->SetValue("");
	}
	
	// Clear record info
	RecordLapTime = 4294967284;
	RecordRaceTime = 4294967284;
	RecordLapPlayerName = "";
	RecordRacePlayerName = "";
	RRLapRecordTimeLabel->SetValue("");
	RRRaceRecordTimeLabel->SetValue("");
	RRLapRecordPlayerLabel->SetValue("");
	RRLapRecordPlayerLabel->SetValue("");
	MapEntry = LapEntry = -1;

	// Get records from game info
	MapFileName = Client->MultiPlayerInfo.GetMapFileName();
	NumOfLaps = (UINT) Client->MultiPlayerInfo.GetNumberOfLaps();
	for (i = 0; i < (UINT) GameInfo.BestResults.size(); i++)
		if (GameInfo.BestResults[i].MapFileName == MapFileName)
		{
			MapEntry = i;
			RecordLapTime = GameInfo.BestResults[i].BestLapTime;
			RecordLapPlayerName = GameInfo.BestResults[i].PlayerName;
			

			for (e = 0; e < (UINT) GameInfo.BestResults[i].LapEntries.size(); e++)
				if (GameInfo.BestResults[i].LapEntries[e].NumOfLaps == NumOfLaps)
				{
					LapEntry = e;
					RecordRaceTime = GameInfo.BestResults[i].LapEntries[e].BestRaceTime;
					RecordRacePlayerName = GameInfo.BestResults[i].LapEntries[e].PlayerName;
					break;
				}
				
			break;
		}

	// Set timer
	Timer = SetTimer(0, 0, 333, RaceResultTimerProc);
*/
}

// Kills timer
void CRaceResultDialog::OnLeave()
{
	KillTimer(0, Timer);
};	

// Timer procedure
void CALLBACK CRaceResultDialog::RaceResultTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time)
{
	pDialogs->pRaceResultDialog->UpdateRaceResults();
};


///////////////////////////////////////////////////////////////////////////////////
//
//			MultiCreateServerDialog
//
///////////////////////////////////////////////////////////////////////////////////
HRESULT CMultiCreateServerDialog::init(	CGraphic * _pGraphic,
										resManNS::CResourceManager * _pRM,
										CTimeCounter * _pTimer,
										HWND _hWind )
{
	
	HRESULT		hr;

	hr = CMyDialog::init("multicreateserver.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "CMultiCreateServerDialog::init()", "Multiplayer create server dialog could not be initialized." );
	
	// labels
	GETDIALOGCONTROL2(MCCreateGameLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MCGameNameLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MCDedicatedLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MCMapLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MCMutatorsLabel, graphic::CGUIControlLabel);
	// edit
	GETDIALOGCONTROL2(MCGameNameEdit, graphic::CGUIControlEdit);
	// List box
	GETDIALOGCONTROL2(MCMapListBox, graphic::CGUIControlListBox);
	// check box
	GETDIALOGCONTROL2(MCDedicatedCheckBox, graphic::CGUIControlCheckBox);
	// combo box
	GETDIALOGCONTROL2(MCGameTypeCombo, graphic::CGUIControlComboBox);
	GETDIALOGCONTROL2(MCMutatorsCombo, graphic::CGUIControlComboBox);
	// image
	GETDIALOGCONTROL2(MCMapImage, graphic::CGUIControlImage);
	// buttons
	GETDIALOGCONTROL2(MCStartServerButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(MCBackButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(MCOpponentsButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(MCControlsButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(MCAdvancedButton, graphic::CGUIControlButton);

	// events
	MCStartServerButton->lpOnMouseClick = OnMCStartServerButtonClick;
	MCBackButton->lpOnMouseClick = OnMCBackButtonClick;
	MCOpponentsButton->lpOnMouseClick = OnMCOpponentsButtonClick;
	MCControlsButton->lpOnMouseClick = OnMCControlsButtonClick;
	MCAdvancedButton->lpOnMouseClick = OnMCAdvancedButtonClick;
	MCGameTypeCombo->lpOnChange = OnMCChangeModComboChange;
	MCMapListBox->lpOnChange = OnMCMapListChange;


	// init game types
	MCGameTypeCombo->ClearAllLines();
	for (int i = 0; i < GameModsWrk.GetModCount(); i++)
	{
		MCGameTypeCombo->AddLine( GameModsWrk[i]->GetModuleName() );
	}
	MCGameTypeCombo->SetValue( 0 );
	

	// Init map names
	ReloadMapList();
	MCMapListBox->SetValue( 0 );

	// fill the mutators list
	std::vector<CString> list;
	Server->GetPhysics()->materials.GetMutatorsNamesList( list );
	MCMutatorsCombo->ClearAllLines();
	for ( int i = 0; i < (int)list.size(); i++ )
		MCMutatorsCombo->AddLine( list[i] );
	MCMutatorsCombo->SetValue( 0 );


	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	
	return ERRNOERROR;
};

void CMultiCreateServerDialog::ReloadMapList()
{
	::ReloadMapList( MCMapListBox );
};


HRESULT CMultiPlayerSettingsDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind)
{
	HRESULT hr;
	hr = CMyDialog::init("MultiPlayerSetting.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "CMultiPlayerSettingsDialog::Init()", "Multiplayer player settings dialog could not be initialized." );

	// labels
	GETDIALOGCONTROL2(MSPlayerSettingsLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MSGameLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MSGameNameLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MSPlayerNameLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MSCarLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MSTeamLabel, graphic::CGUIControlLabel);
	// edit box
	GETDIALOGCONTROL2(MSPlayerNameEdit, graphic::CGUIControlEdit);
	// list box
	GETDIALOGCONTROL2(MSCarListBox, graphic::CGUIControlListBox);
	// combo
	GETDIALOGCONTROL2(MSTeamComboBox, graphic::CGUIControlComboBox);
	// Buttons
	GETDIALOGCONTROL2(MSOkButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(MSCancelButton, graphic::CGUIControlButton);

	// events
	MSOkButton->lpOnMouseClick = OnMSOkButtonClick;
	MSCancelButton->lpOnMouseClick = OnMSCancelButtonClick;

	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	return ERRNOERROR;
}
	

// Sets initial values from client multiplayer game info
void CMultiPlayerSettingsDialog::SetInitialValues()
{
	CString Temp;
	UINT ID;
	UINT i, e;
	std::vector<CString>	AllowedCarNames;
	CString					CarName;
	CGameMod::STRING_LIST	TeamNames;


	// Set game name
	MSGameNameLabel->SetValue(Client->MultiPlayerInfo.GetGameName() );

	ID = Client->MultiPlayerInfo.GetID();
	// Set player name
	Client->MultiPlayerInfo.GetPlayerName(ID, Temp);
	MSPlayerNameEdit->SetValue(Temp);
	// Get car name
	Client->MultiPlayerInfo.GetPlayerCar(ID, CarName);
	
	// Set allowed car names
	AllowedCarNames = Client->MultiPlayerInfo.GetAllowedCarsFileNames();

	// Convert from filenames to names and add into list
	// Set car name
	MSCarListBox->Clear();
	for (i = 0; i < AllowedCarNames.size(); i++)
		for (e = 0; e < GameInfo.CarNames.size(); e++)
			if (AllowedCarNames[i] == GameInfo.CarNames[e].FileName)
			{
				Temp = GameInfo.CarNames[e].Name.Left(MAX_CAR_NAME_LENGTH);
				MSCarListBox->PushBack(Temp);
				if (AllowedCarNames[i] == CarName)
					MSCarListBox->SetValue( (int) MSCarListBox->GetLinesCount() - 1);
				break;
			}
	
	// compose team names combobox
	if ( ModConfig.uiMinTeamCount ) pGameMod->GetTeamNames( &TeamNames );
	MSTeamComboBox->ClearAllLines();
	MSTeamComboBox->AddLine( "Auto-assign team" );
	for ( UINT i = 0; i < ModConfig.uiMinTeamCount; i++ )
		MSTeamComboBox->AddLine( TeamNames[i] );
	MSTeamComboBox->SetValue( 0 );

	if ( ModConfig.uiMinTeamCount >= 2 ) {
		MSTeamComboBox->Enable();
		MSTeamComboBox->SetVisible();
		MSTeamLabel->Enable();
		MSTeamLabel->SetVisible();
	} else {
		MSTeamComboBox->Disable();
		MSTeamComboBox->SetInvisible();
		MSTeamLabel->Disable();
		MSTeamLabel->SetInvisible();
	}
};
	

HRESULT CMultiStartGameDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind){
	
	HRESULT hr;

	hr = CMyDialog::init("multistartgame.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "CMultiStartGameDialog::Init()", "Multiplayer start game dialog (Lobby) could not be initialized." );
	
	// labels
	GETDIALOGCONTROL2(MGLobbyLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MGGameLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MGGameNameLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MGMapLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MGMapNameLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MGLapsLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MGTimeOfDayLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MGDoubleDotLabel, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MGPlayer1Label, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MGCar1Label, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MGReady1Label, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MGPlayer2Label, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MGCar2Label, graphic::CGUIControlLabel);
	GETDIALOGCONTROL2(MGReady2Label, graphic::CGUIControlLabel);
	// check boxes
	GETDIALOGCONTROL2(MGLapsCheckBox, graphic::CGUIControlCheckBox);
	// edit boxes
	GETDIALOGCONTROL2(MGLapsEdit, graphic::CGUIControlEdit);
	GETDIALOGCONTROL2(MGDayHoursEdit, graphic::CGUIControlEdit);
	GETDIALOGCONTROL2(MGDayMinutesEdit, graphic::CGUIControlEdit);
	GETDIALOGCONTROL2(MGChatEdit, graphic::CGUIControlEdit);
	// list box
	GETDIALOGCONTROL2(MGChatList, graphic::CGUIControlListBox);
	// Buttons
	GETDIALOGCONTROL2(MGSendButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(MGStartGameButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(MGPlayerSettingsButton, graphic::CGUIControlButton);
	GETDIALOGCONTROL2(MGLeaveButton, graphic::CGUIControlButton);

	// init multiple items
	GETDIALOGCONTROL(MGPlayerNameLabel[0], graphic::CGUIControlLabel, MGPlayerName1Label);
	GETDIALOGCONTROL(MGPlayerNameLabel[1], graphic::CGUIControlLabel, MGPlayerName2Label);
	GETDIALOGCONTROL(MGPlayerNameLabel[2], graphic::CGUIControlLabel, MGPlayerName3Label);
	GETDIALOGCONTROL(MGPlayerNameLabel[3], graphic::CGUIControlLabel, MGPlayerName4Label);
	GETDIALOGCONTROL(MGPlayerNameLabel[4], graphic::CGUIControlLabel, MGPlayerName5Label);
	GETDIALOGCONTROL(MGPlayerNameLabel[5], graphic::CGUIControlLabel, MGPlayerName6Label);
	GETDIALOGCONTROL(MGPlayerNameLabel[6], graphic::CGUIControlLabel, MGPlayerName7Label);
	GETDIALOGCONTROL(MGPlayerNameLabel[7], graphic::CGUIControlLabel, MGPlayerName8Label);

	GETDIALOGCONTROL(MGCarNameLabel[0], graphic::CGUIControlLabel, MGPlayerCar1Label);
	GETDIALOGCONTROL(MGCarNameLabel[1], graphic::CGUIControlLabel, MGPlayerCar2Label);
	GETDIALOGCONTROL(MGCarNameLabel[2], graphic::CGUIControlLabel, MGPlayerCar3Label);
	GETDIALOGCONTROL(MGCarNameLabel[3], graphic::CGUIControlLabel, MGPlayerCar4Label);
	GETDIALOGCONTROL(MGCarNameLabel[4], graphic::CGUIControlLabel, MGPlayerCar5Label);
	GETDIALOGCONTROL(MGCarNameLabel[5], graphic::CGUIControlLabel, MGPlayerCar6Label);
	GETDIALOGCONTROL(MGCarNameLabel[6], graphic::CGUIControlLabel, MGPlayerCar7Label);
	GETDIALOGCONTROL(MGCarNameLabel[7], graphic::CGUIControlLabel, MGPlayerCar8Label);

	GETDIALOGCONTROL(MGPlayerReadyCheckBox[0], graphic::CGUIControlCheckBox, MGPlayerReady1CheckBox);
	GETDIALOGCONTROL(MGPlayerReadyCheckBox[1], graphic::CGUIControlCheckBox, MGPlayerReady2CheckBox);
	GETDIALOGCONTROL(MGPlayerReadyCheckBox[2], graphic::CGUIControlCheckBox, MGPlayerReady3CheckBox);
	GETDIALOGCONTROL(MGPlayerReadyCheckBox[3], graphic::CGUIControlCheckBox, MGPlayerReady4CheckBox);
	GETDIALOGCONTROL(MGPlayerReadyCheckBox[4], graphic::CGUIControlCheckBox, MGPlayerReady5CheckBox);
	GETDIALOGCONTROL(MGPlayerReadyCheckBox[5], graphic::CGUIControlCheckBox, MGPlayerReady6CheckBox);
	GETDIALOGCONTROL(MGPlayerReadyCheckBox[6], graphic::CGUIControlCheckBox, MGPlayerReady7CheckBox);
	GETDIALOGCONTROL(MGPlayerReadyCheckBox[7], graphic::CGUIControlCheckBox, MGPlayerReady8CheckBox);

	// events
	MGSendButton->lpOnMouseClick = OnMGSendButtonClick;
	MGStartGameButton->lpOnMouseClick = OnMGStartGameButtonClick;
	MGPlayerSettingsButton->lpOnMouseClick = OnMGPlayerSettingsButtonClick;
	MGLeaveButton->lpOnMouseClick = OnMGLeaveButtonClick;
	MGChatEdit->lpOnKeyUp = OnMGChatEditKeyUp;

	// Set mode to client
	SetMode(false, false);
	
	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	return ERRNOERROR;
}

// Set mode, whether it's in server mode or not
// Whether server is dedicated or not
void CMultiStartGameDialog::SetMode(BOOL _Server, BOOL _Dedicated)
{
	Server = _Server;
	Dedicated = _Dedicated;

	if (!Server)
	{
		MGLapsEdit->Disable();
		MGDayHoursEdit->Disable();
		MGDayMinutesEdit->Disable();
		MGLapsCheckBox->Disable();
		MGStartGameButton->SetCaption("READY");
	}
	else 
	{
		MGLapsEdit->Enable();
		MGDayHoursEdit->Enable();
		MGDayMinutesEdit->Enable();
		MGLapsCheckBox->Enable();
		MGStartGameButton->SetCaption("START GAME");

		if (Dedicated)
		{
			MGPlayerSettingsButton->SetInvisible();
			MGPlayerSettingsButton->Disable();
		}
		else
		{
			MGPlayerSettingsButton->SetVisible();
			MGPlayerSettingsButton->Enable();
		}
	}
}

// Starts timer and update
void CMultiStartGameDialog::OnEnter()
{
	CString Temp;
	resManNS::RESOURCEID MapID;
	resManNS::Map * MapObj;
	UINT i;
	char DayTime[20];

	CMultiPlayerGameInfo * MultiPlayerInfo;

	if (!Dedicated)
		MultiPlayerInfo = &(Client->MultiPlayerInfo);
	else
		MultiPlayerInfo = &(::Server->MultiPlayerInfo);

	// Game name


	Temp = MultiPlayerInfo->GetGameName();
	MGGameNameLabel->SetValue(Temp);

	// Map name
	Temp = MultiPlayerInfo->GetMapFileName();
	// Load map
	MapID = MultiPlayerInfo->GetMapID();
	if (MapID >= 0)
	{
		MapObj = pDialogs->pResourceManager->GetMap(MapID);

		// Set default day time 
		_itoa(  (int) (MapObj->fDayTime * 24), DayTime, 10);
		this->MGDayHoursEdit->SetValue( DayTime);
		_itoa( (int) (fmod(MapObj->fDayTime * 24, 1.0f) * 60), DayTime, 10);
		this->MGDayMinutesEdit->SetValue( DayTime);
	}
	else
	{
		this->MGDayHoursEdit->SetValue("12");
		this->MGDayMinutesEdit->SetValue("0");
	}

	for (i = 0; i < GameInfo.MapNames.size(); i++)
		if (Temp == GameInfo.MapNames[i].FileName)
			MGMapNameLabel->SetValue(GameInfo.MapNames[i].Name);

	Update();
	Timer = SetTimer(0, 0, 333, LobbyTimerProc);
}

// Ends timer
void CMultiStartGameDialog::OnLeave()
{
	KillTimer(0, Timer);
	
	// Clear chat
	MGChatList->Clear();
}

// Updates lobby
void CMultiStartGameDialog::Update()
{
	CString Temp;
	char Buf[10];
	int TempI;
	BOOL TempB;
	UINT i, e;
	HRESULT Result;

	int DayHours, DayMinutes, NumLaps;
	BOOL LapMode;

	CMultiPlayerGameInfo * MultiPlayerInfo;
	
	// Test if server is alive
	if (!Server)
	{
		if (! Client->isConnected() )
		{
			// Clear chat
			MGChatList->Clear();
			// Stop client ping
			Client->StopPing();
			OnLeave();
			pDialogs->pMultiplayerDialog->OnEnter();
			SHOWMESSAGEBOX("CONNECTION TO SERVER LOST", "Server is no longer available.", pDialogs->pMultiplayerDialog);
			return;
		}
	}

	// Test whether loading started
	if (Client->MultiPlayerInfo.GetState() == CMultiPlayerGameInfo::PHASE_FOUR)
	{
		CHANGE_ACTDIALOG(pWaitDialog);
		pDialogs->pGraphic->SetMouseVisible( false );
		pDialogs->pGraphic->RenderDialogOnly();

		// Start client loading
		Result = Client->ClientStartMultiLoad();
		if (FAILED(Result) )
		{
			Client->ChangeState(CS_MG_ENUMERATE_HOSTS);
			FreeGamePlayResources();
			SHOWMESSAGEBOX("ERROR LOADING CLIENT", "Unable to start client. Client load failed.", pDialogs->pMultiplayerDialog);
		}

		return;
	}
	

	if (!Dedicated)
		MultiPlayerInfo = & (Client->MultiPlayerInfo);
	else
		MultiPlayerInfo = & (::Server->MultiPlayerInfo);

	if (!Server)
	{
		// Update game settings
		TempI = MultiPlayerInfo->GetHours();
		sprintf(Buf, "%i", TempI);
		MGDayHoursEdit->SetValueSilent(Buf);
		TempI = MultiPlayerInfo->GetMinutes();
		sprintf(Buf, "%i", TempI);
		MGDayMinutesEdit->SetValueSilent(Buf);
/*
//RACER MOD ONLY:
		TempI = MultiPlayerInfo->GetNumberOfLaps();
		sprintf(Buf, "%i", TempI);
		MGLapsEdit->SetValueSilent(Buf);
		TempB = MultiPlayerInfo->GetLapMode();
		MGLapsCheckBox->SetCheckedSilent( TempB != 0);
*/
	}

	// Update players and their status
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		// Player name
		MultiPlayerInfo->GetPlayerName(i, Temp);
		if (Temp == "")
			Temp = "N/A";
		Temp = Temp.Left(MAX_PLAYER_NAME_LENGTH);
		MGPlayerNameLabel[i]->SetValue(Temp);
		// Player car
		MultiPlayerInfo->GetPlayerCar(i, Temp);
		if (Temp == "")
			Temp = "N/A";
		else
		{
			for (e = 0; e < GameInfo.CarNames.size(); e++)
				if (Temp == GameInfo.CarNames[e].FileName)
				{
					Temp = GameInfo.CarNames[e].Name;
					break;
				}
		}
		Temp = Temp.Left(MAX_CAR_NAME_LENGTH);
		MGCarNameLabel[i]->SetValue(Temp);
		// Player ready state
		MultiPlayerInfo->GetPlayerReady(i, TempB);
		MGPlayerReadyCheckBox[i]->SetChecked( TempB != 0);
	
	}

	// Update chat
	if (MultiPlayerInfo->GetChatMessagesNumber() > MGChatList->GetLinesCount() )
	{
		for (i = (UINT) MGChatList->GetLinesCount(); i < MultiPlayerInfo->GetChatMessagesNumber(); i++)
		{
			MultiPlayerInfo->GetChatMessage(i, Temp);
			MGChatList->PushBack(Temp);
		}
		// Select last message
		MGChatList->SetValueSilent((int) MGChatList->GetLinesCount() - 1);
	}
	
	// Game settings update to server
	if (Server)
	{
		// Get all game settings
		Temp = MGDayHoursEdit->GetValue();
		OnlyNumbers(Temp);
		sscanf(Temp, "%i", &DayHours);
		Temp = MGDayMinutesEdit->GetValue();
		OnlyNumbers(Temp);
		sscanf(Temp, "%i", &DayMinutes);
		Temp = MGLapsEdit->GetValue();
		OnlyNumbers(Temp);
		sscanf(Temp, "%i", &NumLaps);
		LapMode = MGLapsCheckBox->IsChecked();

		// check game settings
		TempB = true;
		if (DayHours < 0 || DayHours > 23) TempB = false;
		if (DayMinutes < 0 || DayMinutes > 59) TempB = false;
		if (NumLaps <= 0 || NumLaps > 999) TempB = false;

		if (TempB)
		{
			// Compare settings with what is set
			// if different, update game settings
			TempB = false;
			if (::Server->MultiPlayerInfo.GetHours() != DayHours) TempB = true;
			if (::Server->MultiPlayerInfo.GetMinutes() != DayMinutes) TempB = true;
/*
//RACER MOD ONLY:
			if (::Server->MultiPlayerInfo.GetNumberOfLaps() != NumLaps) TempB = true;
			if (::Server->MultiPlayerInfo.GetLapMode() != LapMode) TempB = true;
*/
			if (TempB)
				::Server->UpdateGameSettings( DayHours, DayMinutes );
		}
	}
}

// Timer procedure
void CALLBACK CMultiStartGameDialog::LobbyTimerProc(HWND Hwnd, UINT Msg, UINT_PTR IdEvent, DWORD Time)
{
	// Update lobby
	pDialogs->pMultiStartGameDialog->Update();
}

HRESULT CStartDialog::init(CGraphic * _pGraphic,resManNS::CResourceManager	* _pRM,CTimeCounter * _pTimer,HWND _hWind)
{
	HRESULT hr;
	hr = CMyDialog::init("321go.dlgDef", _pGraphic,_pRM,_pTimer,_hWind);
	if(hr) ERRORMSG( hr, "CStartDialog::Init()", "Start dialog object could not be initialized." );

	// Labels
	GETDIALOGCONTROL2(CDLabel, graphic::CGUIControlLabel);
	
	
	pGUIDialog->RebuildStructures(); // must be called because one ZOrder property was changed
	return ERRNOERROR;
}