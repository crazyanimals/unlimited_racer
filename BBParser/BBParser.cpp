// BBParser.cpp : Defines the entry point for the application.

#include "stdafx.h"
#include "BBParser.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING] = "MyWindow"; // 	 The title bar text
TCHAR szWindowClass[MAX_LOADSTRING] = "MyWindowClass";// the main window class name

FILE *fw;
           
int Parse(HWND hWnd, LPTSTR lpCmdLine) {
    LPSTR lpInFile, lpOutFile = NULL;
    int i, c = 0, length;
    for (i = 0; lpCmdLine[i]; i++);
    length = i;
    lpInFile = lpCmdLine;
    for (i = 0; i < length; i++) {
        if (lpCmdLine[i] == '"') c = !c;
        if (lpCmdLine[i] == ' ' && !c) {
            lpCmdLine[i] = 0;
            lpOutFile = lpCmdLine + i + 1;
        }
    }
    return (lpOutFile && InitD3DDevice(hWnd) && LoadXFile(lpInFile) && ParseXFile() && ExtractBB() && ReleaseXFile() && SaveBBFile(lpOutFile));
}


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow) {
    HWND hWnd;
	WNDCLASSEX wcex;
    int iRet;
    fw = fopen("BBox.log", "w");
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
    iRet = Parse(hWnd, lpCmdLine);
    fclose(fw);
    return !iRet;
}
