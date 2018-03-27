#include "stdafx.h"
#include "wndProcs.h"
#include "Resource.h"
#include "..\Server\CServer.h"
#include "..\Client\CClient.h"
#include "..\globals\Configuration.h"
#include "..\ResourceManager\ResourceManager.h"
#include "..\GraphicObjects\Scene.h"
#include "..\GraphicObjects\GrObjectMesh.h"
#include "..\Graphic\Graphic.h"
#include "..\Graphic\Camera.h"
#include "InitFunctions.h"
#include "Dialogs.h"
#include "Controls.h"
#include <windows.h>


using namespace graphic;

extern CConfig Configuration;
extern CServer * Server;
extern CClient * Client;
extern resManNS::CResourceManager	*ResourceManager;
extern CGraphic * Graphic;
extern CScene	*Scene;
extern CMyDialogs * pDialogs;

extern CTimeCounter Timer;
extern TCHAR szWindowClass[256];
extern HWND hWnd;
extern HINSTANCE hInst;
extern TCHAR szApplicationName[256];
extern int gameState;
extern CGameInput gInput;

LRESULT CALLBACK gamePlayWndProc(HWND hWind, UINT msg, WPARAM wParam, LPARAM lParam){
	
//	RECT rect;
	switch(msg){
		
		case WM_TIMER:
			RECT rect;		
			HRESULT hr;
			GetWindowRect( hWnd, &rect );
			
			SetCursorPos(  rect.left/2 + rect.right/2, rect.top/2+rect.bottom/2 );
			gInput.CheckInput();
			gInput.React(Graphic);
			Client->SendControls(gInput.KeysToSend,sizeof(gInput.KeysToSend));
			//Client->sendControls(gInput.allKeys,NUM_FOR_SEND*sizeof(DWORD));
			hr = Server->updateGame();
			if (FAILED(hr) )
			{
						FreeGamePlayResources();
						ErrorHandler.HandleError(hr, "gamePlayWndProc()", "Error while updating game");
						SHOWMESSAGEBOX("SERVER ERROR", "There was an error on server. Server stopped.", pDialogs->pMainDialog);
						break;
			}
	
			break;
	
			
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);

	};

	return 0;
	
}