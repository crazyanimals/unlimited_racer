/*
	Globals:		This source file contains definitions of globally used identifiers throughout whole game (=Main module).
					The real definitions can be found in main.cpp
	Creation Date:	14.12.2006
	Last Update:	7.1.2006
	Author:			Roman Margold
*/


#pragma once

#include "stdafx.h"

#include "Control.h"
#include "dialogs.h"
#include "SoundBankCar.h"
#include "..\Server\CServer.h"
#include "..\Client\CClient.h"
#include "..\Client\GameInfo.h"
#include "..\ResourceManager\ResourceManager.h"
#include "..\GraphicObjects\Scene.h"
#include "..\GraphicObjects\GrObjectMesh.h"
#include "..\Graphic\Graphic.h"
#include "..\Graphic\Camera.h"
#include "..\Physics\Physics.h"
#include "..\GameMods\GameMod.h"
#include "..\GameMods\ModWorkshop.h"
#include "CommonDefs.h"



// global variables
extern CServer				*	Server;

extern CGameInfo				GameInfo;
extern CGameInput				gInput;
extern CTimeCounter				Timer; // time counter
extern resManNS::CResourceManager	* ResourceManager;
extern graphic::CGraphic	*	Graphic;
extern graphic::CScene		*	Scene;
extern physics::CPhysics	*	Physics;
extern CMyDialogs			*	pDialogs;
extern CRITICAL_SECTION			g_csServerList;
extern int						gameState;
extern HWND						hWnd;
extern CMODWorkshop				GameModsWrk; // object used to access game modifications
extern CGameMod				*	pGameMod; // pointer to the choosen game MOD object
extern MODCONFIG				ModConfig; // configuration of the choosen game MOD
extern IXACTEngine			*	pSoundEngine;
extern COMMON_SOUNDS			CommonSounds;

