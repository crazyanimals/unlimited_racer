#pragma once

#include "stdafx.h"

extern DWORD dwVertices;
extern DWORD dwFaces;


int InitD3DDevice(HWND hWnd);
int LoadXFile(LPTSTR lpInFile);
inline DWORD GetIBEntry(DWORD dwFace, BYTE bEntry);
inline void SetIBEntry(DWORD dwFace, BYTE bEntry, DWORD dwValue);
inline float GetVBEntry(DWORD dwVertex, BYTE bEntry);
int dw_cmp(const void * e1, const void * e2);
int ParseXFile();
int ReleaseXFile();
void MakeVertexBufferUnique();