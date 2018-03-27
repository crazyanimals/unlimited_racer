#include "..\ResourceManager\ResourceManager.h"
#include "..\Globals\GlobalsIncludes.h"

using namespace resManNS;

LPDIRECT3DDEVICE9		D3DDevice; 
D3DPRESENT_PARAMETERS	D3DPP; 
LPDIRECT3D9				Direct3D;

CResourceManager * RM = new CResourceManager();

void CALLBACK fErrorHandler(CAtlString msg, int level) {

}

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING] = "MyWindow"; // 	 The title bar text
TCHAR szWindowClass[MAX_LOADSTRING] = "MyWindowClass";// the main window class name

FILE *fw;

int InitD3DDevice(HWND hWnd) {
    HRESULT hr;
	D3DCAPS9 D3DCaps;
	Direct3D = Direct3DCreate9( D3D_SDK_VERSION );
	if ( !Direct3D ) return 0;
	Direct3D->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &D3DCaps );
	ZeroMemory( &D3DPP, sizeof(D3DPP) );
	D3DPP.Windowed = TRUE;     
	D3DPP.EnableAutoDepthStencil = true;
	D3DPP.AutoDepthStencilFormat = D3DFMT_D24S8;
	D3DPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
	D3DPP.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	D3DPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	D3DPP.BackBufferHeight = 1;
	D3DPP.BackBufferWidth = 1;
	D3DPP.BackBufferFormat = D3DFMT_UNKNOWN;
    hr = Direct3D->CreateDevice( 
				D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL,
				hWnd,
				(D3DCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) ? D3DCREATE_MIXED_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING,
				&D3DPP,
				&D3DDevice
			);
	
	return !hr;
}
//////////////////////////////////////////////////////////////////////////////////////
int Test() {
    RESOURCEID Id, Id2, Id3;
    ModelLoadParams CMLP;
    CMLP.uiLODCount = 5;
    CMLP.uiOptions = D3DXMESH_SYSTEMMEM;
    ErrorHandler.Init(&fErrorHandler);
    DebugLevel = 10;
    DebugOutput.Init(DebugOutputCallBack);
    OUTSN("------------------------------------------------------------------------------------------------------------------------", 0);
    OUTSN("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@", 0);
    OUTSN("------------------------------------------------------------------------------------------------------------------------", 0);
    OUTS("Application 'RMTest.exe' starting at ",0);
    OUTDATETIME(0);
    OUTSN(" :",0);
    RM->SetResourcePath("D:\\progs\\Projekty\\Unlimited Racer\\sources\\compilation\\", RES_Model);
    RM->SetResourcePath("D:\\progs\\Projekty\\Unlimited Racer\\sources\\compilation\\", RES_XFile);
    RM->SetResourcePath("D:\\progs\\Projekty\\Unlimited Racer\\sources\\compilation\\", RES_Texture);
    RM->SetResourcePath("D:\\progs\\Projekty\\Unlimited Racer\\sources\\compilation\\Maps", RES_Map);
    RM->SetResourcePath("D:\\progs\\Projekty\\Unlimited Racer\\sources\\compilation\\Objects", RES_MainObject);
    RM->SetResourcePath("D:\\progs\\Projekty\\Unlimited Racer\\sources\\compilation\\Objects", RES_GameObject);

    RM->InitD3DDevice(D3DDevice);
    Id  = RM->LoadResource(CAtlString("test.cm"), RES_Model, &CMLP); 
    Id2 = RM->LoadResource(CAtlString("test.cm"), RES_Model, &CMLP); 
	RM->DuplicateResource(Id);
    CResourceManager::CResourceContainer * rcX;
    if (RM->GetResource(Id, &rcX) == -1) return 0;
    RM->ReleaseResource(Id);
    RM->ReleaseResource(Id2);
	RM->ReleaseResource(Id);
    return 1;
}
//////////////////////////////////////////////////////////////////////////////////////
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow) {
    HWND hWnd;
	WNDCLASSEX wcex;
    int iRet;
    fw = fopen("RMTest.log", "w");
    fprintf(fw, "");
    wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)DefWindowProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);	
    
    hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    iRet = InitD3DDevice(hWnd) && Test();
    fclose(fw);
    return !iRet;
}