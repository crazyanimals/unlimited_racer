// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//



// TODO: reference additional headers your program requires here


#define WIN32_LEAN_AND_MEAN		

#include <windows.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <d3d9.h>
#include <d3dx9core.h>
#include <d3dx9math.h>
#include <dxerr9.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define LOG(A,B)\
    {\
        /*fw = fopen("BBox.log", "a");*/\
        fprintf(fw, A, B);\
        /*fclose(fw);*/\
    }

extern FILE *fw;