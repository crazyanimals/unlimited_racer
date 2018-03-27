#include "stdafx.h"
#include "XFile.h"

LPDIRECT3DDEVICE9		D3DDevice; 
D3DPRESENT_PARAMETERS	D3DPP; 
LPDIRECT3D9				Direct3D;
LPD3DXMESH              XFileMesh;

BYTE * pVertexBuffer;
LPVOID * pIndexBuffer;
DWORD dwBytesPerVertex;
DWORD dwVertices;
DWORD dwFaces;
DWORD dwFVF;

DWORD * pdwIndexBuffer; // dwVertices > 65535
WORD  *  pwIndexBuffer; // dwVertices < 65536

DWORD * pdwUniqueIndex = 0; //Second level index buffer


//-----------------------------------------------------------------------------
// Initialiazes D3Device 
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
//  Loads XFile from lpInFile (.x file stored on disk)
//-----------------------------------------------------------------------------
int LoadXFile(LPTSTR lpInFile) {
    LPD3DXBUFFER    pMtrlBuf; // zahod
	DWORD tmp;
    HRESULT res;
    res = D3DXLoadMeshFromX(lpInFile, D3DXMESH_SYSTEMMEM, D3DDevice, NULL, &pMtrlBuf, NULL, &(tmp), &(XFileMesh));
    if (res == D3DERR_INVALIDCALL) return 0;
    if (res != D3D_OK) return 0;
   	free(pMtrlBuf);  
    return 1;
}

//-----------------------------------------------------------------------------
//  getting Index Buffer element
//-----------------------------------------------------------------------------
inline DWORD GetIBEntry(DWORD dwFace, BYTE bEntry) {
    DWORD dw = (dwVertices > 65535 ? (pdwIndexBuffer[dwFace*3+bEntry]) : (pwIndexBuffer[dwFace*3+bEntry]));
    return pdwUniqueIndex ? pdwUniqueIndex[dw]: dw;
}

//-----------------------------------------------------------------------------
// setting Index Buffer element
//-----------------------------------------------------------------------------
inline void SetIBEntry(DWORD dwFace, BYTE bEntry, DWORD dwValue) {
    if (dwVertices > 65536) pdwIndexBuffer[dwFace*3+bEntry] = dwValue;
    else pwIndexBuffer[dwFace*3+bEntry] = (DWORD) dwValue;
}

//-----------------------------------------------------------------------------
// getting Vertex Buffer lement
//-----------------------------------------------------------------------------
inline float GetVBEntry(DWORD dwVertex, BYTE bEntry) {
    return *((float *) (pVertexBuffer+dwVertex*dwBytesPerVertex+sizeof(float)*bEntry));
}

//-----------------------------------------------------------------------------
// dw comparison (for purposes of qsort function)
//-----------------------------------------------------------------------------
int dw_cmp(const void * e1, const void * e2) {
    DWORD dw1 = *((DWORD *) e1);
    DWORD dw2 = *((DWORD *) e2);
    if (GetVBEntry(dw1, 0) < GetVBEntry(dw2, 0)) return -1;
    if (GetVBEntry(dw1, 0) > GetVBEntry(dw2, 0)) return +1;
    if (GetVBEntry(dw1, 1) < GetVBEntry(dw2, 1)) return -1;
    if (GetVBEntry(dw1, 1) > GetVBEntry(dw2, 1)) return +1;
    if (GetVBEntry(dw1, 2) < GetVBEntry(dw2, 2)) return -1;
    return 1;
}

//-----------------------------------------------------------------------------
// rewrites indexes dwFrom to dwTo in whole IndexBuffer
//-----------------------------------------------------------------------------
void RewriteIndex(DWORD dwFrom, DWORD dwTo) {
    DWORD dw;
    for (dw = 0; dw < dwFaces; dw++) {
        if (GetIBEntry(dw, 0) == dwFrom) SetIBEntry(dw, 0, dwTo);
        if (GetIBEntry(dw, 1) == dwFrom) SetIBEntry(dw, 1, dwTo);
        if (GetIBEntry(dw, 2) == dwFrom) SetIBEntry(dw, 2, dwTo);
    }
}

//-----------------------------------------------------------------------------
// discards duplicated vertices - repairs index buffer.
//-----------------------------------------------------------------------------
void MakeVertexBufferUnique() {
    DWORD * pdwIndexes = (DWORD *) malloc(sizeof(DWORD)*dwVertices);
    DWORD dw, dwRewritten = 0;
    for (dw = 0; dw < dwVertices; dw++) 
        pdwIndexes[dw] = dw;
    LOG("Sorting index buffer ...\n", 0);
    qsort(pdwIndexes, dwVertices, sizeof(DWORD), dw_cmp);
    LOG("Index buffer is now sorted!\n", 0)
    LOG("Sorted vertices :\n", 0);
    for (dw = 0; dw < dwVertices; dw++) {
        LOG("Sorted_Vertex[%u] = ", pdwIndexes[dw]);
        LOG("[%f, ", GetVBEntry(pdwIndexes[dw], 0));
        LOG("%f, ",  GetVBEntry(pdwIndexes[dw], 1));
        LOG("%f]\n", GetVBEntry(pdwIndexes[dw], 2));
    }
    LOG("Making vertices unique...\n", 0);
    pdwUniqueIndex = (DWORD *) malloc(sizeof(DWORD)*dwVertices);
    for (dw = 0; dw < dwVertices; dw++) 
        if (
            dw && GetVBEntry(pdwIndexes[dw-1],0) == GetVBEntry(pdwIndexes[dw], 0) 
               && GetVBEntry(pdwIndexes[dw-1],1) == GetVBEntry(pdwIndexes[dw], 1) 
               && GetVBEntry(pdwIndexes[dw-1],2) == GetVBEntry(pdwIndexes[dw], 2)
           ) 
        {
            LOG("Rewriting index %u\tto\t", pdwIndexes[dw]);
            LOG("%u\n", pdwIndexes[dw-1]);
            pdwUniqueIndex[pdwIndexes[dw]] = pdwIndexes[dw-1];
            pdwIndexes[dw] = pdwIndexes[dw-1];
            dwRewritten++;
        } else pdwUniqueIndex[pdwIndexes[dw]] = pdwIndexes[dw];
    LOG("All indexes are unique now! %u duplicates removed!\n", dwRewritten);
    free(pdwIndexes);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
int ParseXFile() {
    dwBytesPerVertex = XFileMesh->GetNumBytesPerVertex();
    dwVertices = XFileMesh->GetNumVertices();
    dwFaces = XFileMesh->GetNumFaces();
    dwFVF = XFileMesh->GetFVF();
    
    LOG("dwBytesPerVertex = %u\n", dwBytesPerVertex);    
    LOG("dwVertices = %u\n\n", dwVertices);
    LOG("dwFaces = %u\n\n", dwFaces);
    LOG("dwFVF = 0x%x :\n", dwFVF);
    
    if (dwFVF & D3DFVF_DIFFUSE)  LOG("D3DFVF_DIFFUSE\n",0);
    if (dwFVF & D3DFVF_NORMAL)   LOG("D3DFVF_NORMAL\n",0);
    if (dwFVF & D3DFVF_PSIZE)    LOG("D3DFVF_PSIZE\n",0);
    if (dwFVF & D3DFVF_SPECULAR) LOG("D3DFVF_SPECULAR\n",0);
    
    if (!(dwFVF & D3DFVF_XYZ)) return 0;
    

    if (D3D_OK == XFileMesh->LockIndexBuffer(D3DLOCK_READONLY, ((LPVOID *) &pIndexBuffer))) LOG("Locking index buffer (0x%x)\n", pIndexBuffer)
    else LOG("Locking of index buffer failed!!!\n",0);
    if (D3D_OK == XFileMesh->LockVertexBuffer(D3DLOCK_READONLY,((LPVOID *) &pVertexBuffer))) LOG("Locking vertex buffer (0x%lx)\n", pVertexBuffer)
    else LOG("Locking of vertex buffer failed!!!\n", 0);

    LOG("Creating private copy oIndexBuffer ...", 0);
    if (dwVertices > 65535) {
        pdwIndexBuffer = (DWORD *) malloc(sizeof(DWORD)*dwFaces*3);
        memcpy(pdwIndexBuffer, pIndexBuffer, sizeof(DWORD)*dwFaces*3);
        LOG("Done! (0x%x)\n", pdwIndexBuffer);
    } else {
        pwIndexBuffer = (WORD *) malloc(sizeof(WORD)*dwFaces*3);
        memcpy(pwIndexBuffer, pIndexBuffer, sizeof(WORD)*dwFaces*3);
        LOG("Done! (0x%x)\n", pwIndexBuffer);
    }
    
    LOG("Unlocking index buffer...\n", 0);
    XFileMesh->UnlockIndexBuffer();
    LOG("Index buffer unlocked\n", 0);

    for (DWORD i = 0; i < dwFaces; i++) {
        LOG("Faces[%i] = ", i);
        LOG("{%u, ", GetIBEntry(i, 0));
        LOG("%u, ",  GetIBEntry(i, 1));
        LOG("%u}\n", GetIBEntry(i, 2));
    }


    for (DWORD i = 0; i <  dwVertices; i++) {
        LOG("Vertex[%i] = ", i);
        LOG("[%f, ", GetVBEntry(i, 0));
        LOG("%f, ",  GetVBEntry(i, 1));
        LOG("%f]\n", GetVBEntry(i, 2));
    }

    return 1;
}

int ReleaseXFile() {
    LOG("Unlocking vertex buffer...\n", 0);
    XFileMesh->UnlockVertexBuffer();
    LOG("Vertex buffer unlocked\n", 0);
    free(pdwUniqueIndex);
    if (dwVertices > 65535) free(pdwIndexBuffer);
    else free(pwIndexBuffer);
    return 1;
}