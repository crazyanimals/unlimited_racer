#pragma once

#include "VertexPool.h"
#include "stdafx.h"
#include "XFile.h"



typedef struct __BBox { 
    float fVertex[4][3];
    float fCentre[3];
} BBox;

void AnalyzePossibleBox(Pool *p);
int SaveBBFile(LPTSTR lpOutFile);
int ExtractBB();