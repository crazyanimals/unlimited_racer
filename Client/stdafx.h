// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once



#include "..\Globals\GlobalsIncludes.h"
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <set>
#include <list>
#include <algorithm>


// FMOD headers
#include "..\FMOD\inc\fmod.hpp"
#include "..\FMOD\inc\fmod_errors.h"


typedef HRESULT ( * pFuncMH)(void *,DWORD,void *);