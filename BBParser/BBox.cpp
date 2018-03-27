#include "stdafx.h"
#include "BBox.h"

void AddAnotherBox(BBox * pBB) {
    /*
    
    Fill in this...

    */
    free(pBB);
}

void AnalyzePossibleBox(Pool *p) {
    DWORD dw;
    BBox *pBB;
    if (!p) return;
    ActivePool = p;
    qsort(p->pdwIndexes, p->dwIndexesCount, sizeof(DWORD), dw_cmp);
    LOG("Analyzing possible box (0x%u)", p);
    LOG(" - Centre at [%f,", p->fCentre[0]);
    LOG("%f", p->fCentre[1]);
    LOG(",%f] :\n", p->fCentre[2]);
    for (dw = 0; dw < 12; dw++)   {
        LOG("Comparing : 2*%f", p->fCentre[dw%3]);
        LOG("-(%f) =", GetVBEntry(p->pdwIndexes[dw/3], dw%3));
        LOG(" %f ", 2*p->fCentre[dw%3]-GetVBEntry(p->pdwIndexes[dw/3], dw%3));
        LOG(" and %f ...\n", GetVBEntry(p->pdwIndexes[7-dw/3], dw%3));
        if (2*p->fCentre[dw%3]-GetVBEntry(p->pdwIndexes[dw/3], dw%3) != GetVBEntry(p->pdwIndexes[7-dw/3], dw%3)) break;
    }
    if (dw < 12) {
        LOG("Not a box(%u)!!!\n", dw);
        return;
    } else {
        pBB = (BBox *) malloc(sizeof(BBox));
        memcpy(pBB->fCentre, p->fCentre, sizeof(p->fCentre));
        for (dw = 0; dw < 12; dw++) 
            pBB->fVertex[dw/3][dw%3] = p->fCentre[dw%3] - GetVBEntry(p->pdwIndexes[dw/3], dw%3);
        AddAnotherBox(pBB);
    }
}

int SaveBBFile(LPTSTR lpOutFile) {
    return 0;
}

int ExtractBB() {
    DWORD i, cnt = 0;

    LOG("Discarding duplicite vertices...\n", 0);
    MakeVertexBufferUnique();
    LOG("Vertices are now unique!\n", 0)

    InitTrianglesPool();
    for (i = 0; i < dwFaces; i++) 
        AddToPools(GetIBEntry(i, 0), GetIBEntry(i, 1), GetIBEntry(i, 2));
    cnt = 0;
    while(Pools) {
        LOG("Pool[%u]", cnt++);
        LOG(" (Vertices count : %u)", Pools->dwIndexesCount);
        LOG(" Indexes of Vertices : ", 0);
        for (i = 0; i < Pools->dwIndexesCount; i++)
            LOG("%u, ", Pools->pdwIndexes[i]);
        LOG("\n", 0);
        if (Pools->dwIndexesCount == 8) {
            LOG("Possible box ... analyzing!\n", 0);
            Pools->fCentre[0] /= Pools->dwIndexesCount;
            Pools->fCentre[1] /= Pools->dwIndexesCount;
            Pools->fCentre[2] /= Pools->dwIndexesCount;
            AnalyzePossibleBox(Pools);
        } else LOG("Not a box - skipping analysis!\n", 0);
        DeletePool(Pools);
    }
    ReleasePools();
    return 1;
}

