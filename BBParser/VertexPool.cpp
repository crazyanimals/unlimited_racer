#include "stdafx.h"
#include "VertexPool.h"

Pool * Pools, * ActivePool;
bool * bUsed;


void InitTrianglesPool() {
    DWORD i;
    Pools = NULL;
    bUsed = (bool *) malloc(sizeof(bool)*dwVertices);
    for (i = 0; i < dwVertices; i++)
        bUsed[i] = 0;
}
     
void DeletePool(Pool * p) {
    Pool ** _last = & Pools;
    while (*_last != p && *_last)
        _last = & ((*_last)->pNext);
    if (*_last) *_last = (*_last)->pNext;
    free(p->pdwIndexes);
    free(p);
}

void MergePools(Pool * p1, Pool * p2) {
    DWORD * _tmp;
    if (p1 == p2 || !p1 || !p2) return;
    _tmp = (DWORD *) realloc(p1->pdwIndexes, sizeof(DWORD)*(p1->dwIndexesCount+p2->dwIndexesCount));
    if (!_tmp) return;
    p1->pdwIndexes = _tmp;
    memcpy(_tmp + p1->dwIndexesCount, p2->pdwIndexes, sizeof(DWORD)*p2->dwIndexesCount);
    p1->dwIndexesCount += p2->dwIndexesCount;
    p1->fCentre[0] += p2->fCentre[0];
    p1->fCentre[1] += p2->fCentre[1];
    p1->fCentre[2] += p2->fCentre[2];
    DeletePool(p2);
}

Pool * FindPoolOf(DWORD _i) {
    Pool * _tmp = Pools;
    DWORD i;
    if (!bUsed[_i]) return NULL;
    while (_tmp) {
        for (i = 0; i < _tmp->dwIndexesCount && _tmp->pdwIndexes[i] != _i; i++);
        if (i < _tmp->dwIndexesCount) break;
        _tmp = _tmp->pNext;
    }
    return _tmp;
}

void AddToPools(DWORD i1, DWORD i2, DWORD i3) {
    Pool * _p = (Pool *) malloc(sizeof(Pool));
    DWORD dw;
    _p->dwIndexesCount = 0;
    _p->pdwIndexes = (DWORD *) malloc(sizeof(DWORD)*3);
    if (!bUsed[i1]) _p->pdwIndexes[_p->dwIndexesCount++] = i1;
    if (!bUsed[i2]) _p->pdwIndexes[_p->dwIndexesCount++] = i2;
    if (!bUsed[i3]) _p->pdwIndexes[_p->dwIndexesCount++] = i3;

    _p->fCentre[0] = _p->fCentre[1] = _p->fCentre[2] = 0;

    for (dw = 0; dw < _p->dwIndexesCount; dw++) {
        _p->fCentre[0] += GetVBEntry(_p->pdwIndexes[dw], 0);
        _p->fCentre[1] += GetVBEntry(_p->pdwIndexes[dw], 1);
        _p->fCentre[2] += GetVBEntry(_p->pdwIndexes[dw], 2);
    }

    if (!_p->dwIndexesCount) {
        free(_p->pdwIndexes);
        free(_p);
    } else {
        _p->pNext = Pools;
        Pools = _p;
    }
    bUsed[i1] = bUsed[i2] = bUsed[i3] = true;
    MergePools(FindPoolOf(i1), FindPoolOf(i2));
    MergePools(FindPoolOf(i1), FindPoolOf(i3));
}

void ReleasePools() {
    free(bUsed);
    while (Pools) DeletePool(Pools);
}