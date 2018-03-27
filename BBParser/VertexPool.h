#pragma once

#include "stdafx.h"
#include "XFile.h"

typedef struct __Pool {
    DWORD * pdwIndexes;
    DWORD dwIndexesCount;
    float fCentre[3];
    struct __Pool * pNext;
} Pool;

extern Pool * Pools, * ActivePool;
extern bool * bUsed;

void InitTrianglesPool();
void DeletePool(Pool * p);
void MergePools(Pool * p1, Pool * p2);
Pool * FindPoolOf(DWORD _i);
void AddToPools(DWORD i1, DWORD i2, DWORD i3);
void ReleasePools();