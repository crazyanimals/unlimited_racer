#pragma once


#include "stdafx.h"
#include "Globals.h"
#include "main.h"
#include "Control.h"
#include "dialogs.h"
#include "Resource.h"
#include "..\Globals\Configuration.h"
#include "..\Globals\ErrorHandler.h"
#include "..\Server\CServer.h"
#include "..\Client\CClient.h"
#include "..\GameMods\ModWorkshop.h"
#include "..\ResourceManager\ResourceManager.h"
#include "..\GraphicObjects\Scene.h"
#include "..\GraphicObjects\GrObjectMesh.h"
#include "..\Graphic\Graphic.h"
#include "..\Graphic\Camera.h"
#include "..\Physics\Physics.h"
#include "..\Client\GameInfo.h"


enum LODMAPTYPE
{
	LTTerrain,
	LTModel,
	LTGrass
};


HRESULT initialization();
HRESULT initGraphic(HINSTANCE,int nCmdShow);
CString CreateLODMapFileName( LODMAPTYPE type, int level );
HRESULT startSinglePlayer();
// Returns -2, when selected map is corrupted
HRESULT startMultiServer(CString name,BOOL dedicated, CString MapFileName, CGameMod * pGameMod );
HRESULT startMultiPlayerGame();
HRESULT startGame();
HRESULT stopMultiServer();
HRESULT stopServer();
HRESULT InitSoundEngine( bool bDebugMode = false, bool bAuditionMode = false );
void ReleaseSoundEngine();



// Frees game play resources - map and cars (mainly)
HRESULT FreeGamePlayResources(BOOL OnlyToLobby = FALSE);
HRESULT freeAll();
HRESULT initDialogs();

