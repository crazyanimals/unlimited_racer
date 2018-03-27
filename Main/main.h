// main.h : main header file for the PROJECT_NAME application
//

#pragma once

//#define MEMLEAK_DEBUG


#include "stdafx.h"
#include "resource.h"
#include "Globals.h"
#include "initFunctions.h"
#include "wndProcs.h"
#include "controls.h"
#include "dialogs.h"
#include "GlobalSounds.h"
#ifdef MEMLEAK_DEBUG
	#include "Stackwalker.h"
#endif
#include "..\Globals\Configuration.h"
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


#define GS_GROUP_MASK 0xff0
#define GS_QUIT 0x0
#define GS_START 0x10
#define GS_SERVER_ENUM 0x15
#define	GS_PLAY  0x50


HRESULT ChangeResolution( graphic::GRAPHICINIT * grInit );
void WINAPI XACTNotificationCallback( const XACT_NOTIFICATION* pNotification );
