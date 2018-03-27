#include "ResourceManager.h"
#include "..\Globals\NameValuePair.h"
#include "..\TerrainGenerator\FileHeader.h"
#include <string.h>
#include "..\Globals\Base64Coder.h"


using namespace resManNS;

#define LOAD_RESOURCE_HEADER(TYPE) \
    HRESULT CResourceManager::Load##TYPE(CResourceContainer & rcX, CAtlString sResourceName, CAtlString sFileName, bool bLoadInheritedResources, void * pParam, RVec * pvInheritedResources)
    
///////////////////////////////////////////////////////////////////////////////
// Loads Texture from sFileName - PRIVATE.
///////////////////////////////////////////////////////////////////////////////
LOAD_RESOURCE_HEADER(Texture) {
    Texture _Texture;
    //HRESULT res = D3DXCreateTextureFromFile(D3DDevice, sFileName, &(_Texture.texture));
	HRESULT res = D3DXCreateTextureFromFileEx(	D3DDevice, sFileName, D3DX_DEFAULT, D3DX_DEFAULT, 
												D3DX_DEFAULT, 0, D3DFMT_DXT5, D3DPOOL_MANAGED, 
												D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &(_Texture.texture));
    if (res == D3DERR_INVALIDCALL) HE(ERRFILENOTFOUND, "CResourceManager::LoadTexture", sResourceName);
    if (res < D3D_OK) HE(res, "CResourceManager::LoadTexture()", CAtlString("Error while loading texture from file '") + sResourceName + "'.");
    if (res > D3D_OK) HE(ERRGENERIC, "CResourceManager::LoadTexture()", CAtlString("Error while loading texture from file '") + sResourceName + "'. (Positive error value)");
    rcX = CResourceContainer(this, sResourceName, _Texture);  
    return res;
};

///////////////////////////////////////////////////////////////////////////////
// Loads WaveBank from sFileName - PRIVATE.
// pParam is used as a pointer to XACTEngine object.
///////////////////////////////////////////////////////////////////////////////
LOAD_RESOURCE_HEADER(WaveBank) {
	WaveBank	_WaveBank;
	HRESULT		hr;
	HANDLE		hFile;
	DWORD		dwFileSize;
	HANDLE		hMapFile;

#define _THROWERR(cond,code,msg)		\
				if (cond) {			\
					SAFE_CLOSE_HANDLE( hFile );	\
					SAFE_CLOSE_HANDLE( hMapFile );	\
					if ( _WaveBank.pRawBuffer ) UnmapViewOfFile( _WaveBank.pRawBuffer );	\
					HE( code, "CResourceManager::LoadWaveBank()", msg );	\
				}


	hFile = hMapFile = INVALID_HANDLE_VALUE;
	_WaveBank.pRawBuffer = NULL;
	_WaveBank.pWaveBank = NULL;

	// Create an "in memory" XACT wave bank file using memory mapped file IO
	// Memory mapped files tend to be the fastest for most situations assuming you 
	// have enough virtual address space for a full map of the file
	hFile = CreateFile( sFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
	_THROWERR( hFile == INVALID_HANDLE_VALUE, ERRFILENOTFOUND, sResourceName );

	dwFileSize = GetFileSize( hFile, NULL );
    _THROWERR( dwFileSize == INVALID_FILE_SIZE, ERRFILECORRUPTED, sResourceName );
   

	// map the file into memory
	hMapFile = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, dwFileSize, NULL );
	_THROWERR( !hMapFile, ERRGENERIC, "Unable to create memory mapping of the resource file." );

	_WaveBank.pRawBuffer = MapViewOfFile( hMapFile, FILE_MAP_READ, 0, 0, 0 );    
	_THROWERR( !_WaveBank.pRawBuffer, ERRGENERIC, "Unable to map file into memory." );


	// test the input structure validity
	_THROWERR( !pParam, ERRINVALIDPARAMETER, "Invalid XACT pointer." );
	
	
	// create the WaveBank resource
	hr = ((IXACTEngine*)pParam)->CreateInMemoryWaveBank( _WaveBank.pRawBuffer, dwFileSize, 0, 0, &_WaveBank.pWaveBank );   
	_THROWERR( hr, hr, "Unable to create wave bank." );


	// register notification request for resource deletion
	XACT_NOTIFICATION_DESCRIPTION desc = {0};
	desc.type = XACTNOTIFICATIONTYPE_WAVEBANKDESTROYED;
	desc.pWaveBank = _WaveBank.pWaveBank;
	desc.cueIndex = XACTINDEX_INVALID;
	desc.pvContext = (PVOID) _WaveBank.pRawBuffer;
	((IXACTEngine*)pParam)->RegisterNotification( &desc );


	CloseHandle( hMapFile ); // pRawBuffer maintains a handle on the file so close this unneeded handle
	CloseHandle( hFile ); // pRawBuffer maintains a handle on the file so close this unneeded handle
	
    rcX = CResourceContainer( this, sResourceName, _WaveBank );  

    return ERRNOERROR;
#undef _THROWERR
};

///////////////////////////////////////////////////////////////////////////////
// Loads SoundBank from sFileName - PRIVATE.
// pParam is used as a pointer to XACTEngine object.
///////////////////////////////////////////////////////////////////////////////
LOAD_RESOURCE_HEADER(SoundBank) {
	SoundBank	_SoundBank;
	HRESULT		hr;
	HANDLE		hFile;
	DWORD		dwFileSize, dwBytesRead;

#define _THROWERR(cond,code,msg)		\
				if (cond) {			\
					SAFE_CLOSE_HANDLE( hFile );	\
					SAFE_DELETE_ARRAY( _SoundBank.pRawBuffer );	\
					HE( code, "CResourceManager::LoadSoundBank()", msg );	\
				}


	hFile = INVALID_HANDLE_VALUE;
	_SoundBank.pRawBuffer = NULL;
	_SoundBank.pSoundBank = NULL;

            
	// Read and register the sound bank file with XACT.  Do not use memory mapped file IO because the 
	// memory needs to be read/write and the working set of sound banks are small.
	hFile = CreateFile( sFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
	_THROWERR( hFile == INVALID_HANDLE_VALUE, ERRFILENOTFOUND, sResourceName );

	dwFileSize = GetFileSize( hFile, NULL );
    _THROWERR( dwFileSize == INVALID_FILE_SIZE, ERRFILECORRUPTED, sResourceName );
   

	// Allocate the data here and free the data when recieving the sound bank destroyed notification
	_SoundBank.pRawBuffer = new BYTE[dwFileSize];
	_THROWERR( !_SoundBank.pRawBuffer, ERROUTOFMEMORY, "Can't create sound buffer." );

	hr = ReadFile( hFile, _SoundBank.pRawBuffer, dwFileSize, &dwBytesRead, NULL );
	_THROWERR( !hr, ERRFILECORRUPTED, sResourceName );

	
	// test the input structure validity
	_THROWERR( !pParam, ERRINVALIDPARAMETER, "Invalid XACT pointer." );


	// create the sound bank
	hr = ((IXACTEngine*)pParam)->CreateSoundBank( _SoundBank.pRawBuffer, dwFileSize, 0, 0, &_SoundBank.pSoundBank );
	_THROWERR( hr, hr, "Unable to create sound bank." );

	// register notification request for resource deletion
	XACT_NOTIFICATION_DESCRIPTION desc = {0};
	desc.type = XACTNOTIFICATIONTYPE_SOUNDBANKDESTROYED;
	desc.pSoundBank = _SoundBank.pSoundBank;
	desc.cueIndex = XACTINDEX_INVALID;
	desc.pvContext = (PVOID) _SoundBank.pRawBuffer;
	((IXACTEngine*)pParam)->RegisterNotification( &desc );


	CloseHandle( hFile ); // pRawBuffer maintains a handle on the file so close this unneeded handle
	
    rcX = CResourceContainer( this, sResourceName, _SoundBank );  

    return ERRNOERROR;
#undef _THROWERR
};

///////////////////////////////////////////////////////////////////////////////
// Loads _XFile from sFileName - PRIVATE.
///////////////////////////////////////////////////////////////////////////////
LOAD_RESOURCE_HEADER(XFile) {
    XFile _XFile;
    LPD3DXBUFFER    pMtrlBuf; // zahod
    D3DXMATERIAL*   pMaterial; // tohle zahod
    long int i;
    HRESULT res;
    rcX.uiResType = RES_NoType;
    res = D3DXLoadMeshFromX(sFileName, *((DWORD *) pParam), D3DDevice, NULL, &pMtrlBuf, NULL, &(_XFile.cMtrl), &(_XFile.Mesh));
    if (res == D3DERR_INVALIDCALL) HE(res, "CResourceManager::LoadXFile()", "Could not load mesh from _XFile '"+sResourceName+"'. Invalid parameters.");
    if (res != D3D_OK) HE(res, "CResourceManager::LoadXFile()", "Could not load mesh from _XFile '"+sResourceName+"'.");
    pMaterial = (D3DXMATERIAL *) pMtrlBuf->GetBufferPointer();
    _XFile.Material =  new D3DMATERIAL9[_XFile.cMtrl];
    _XFile.pTextureIDs = new RESOURCEID[_XFile.cMtrl];
    if (!_XFile.pTextureIDs || !_XFile.Material) {
        delete[]_XFile.pTextureIDs;
        delete[]_XFile.Material;
        HE(ERROUTOFMEMORY, "CResourceManager::LoadXFile()", "Not Enough memory for pTextureIDs / Material!");
    }
    for (i = 0; (DWORD) i < _XFile.cMtrl; i++) {
        _XFile.Material[i] = pMaterial[i].MatD3D;
        _XFile.Material[i].Ambient = _XFile.Material[i].Diffuse;
    }
    for (i = 0; (DWORD) i < _XFile.cMtrl && res >= 0; i++) {
        if (pMaterial[i].pTextureFilename) {
            res = bLoadInheritedResources ? LoadResource(pMaterial[i].pTextureFilename, RES_Texture) : -1;
            if (res >= 0) {
                _XFile.pTextureIDs[i] = res;
                pvInheritedResources->push_back(res);
            } else _XFile.pTextureIDs[i] = -1;
        } else _XFile.pTextureIDs[i] = -1;
    }
    if (res < 0) {
        ErrorHandler.HandleError(ERRGENERIC, "CResourceManager::LoadXFile()", CAtlString("Could not load texture '") + (i < (int) _XFile.cMtrl ? CAtlString(pMaterial[i].pTextureFilename) : CAtlString("unknown")) + "'.");
        for (i--; i >= 0; i--) if (_XFile.pTextureIDs[i] >= 0) ReleaseResource(_XFile.pTextureIDs[i]);
        delete[]_XFile.pTextureIDs;
        delete[]_XFile.Material;
        return ERRGENERIC;
    }

	// Perform mesh optimization
	DWORD * AdjacencyInfo;
	AdjacencyInfo = new DWORD[3 * sizeof(DWORD) * _XFile.Mesh->GetNumFaces()];
	_XFile.Mesh->GenerateAdjacency(ONE_CENTIMETER, AdjacencyInfo);
	_XFile.Mesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE | D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_DONOTSPLIT,
								 AdjacencyInfo,
								 0,
								 0,
								 0);
	
	delete [] AdjacencyInfo;

    rcX = CResourceContainer(this, sResourceName, _XFile);

    SAFE_RELEASE(pMtrlBuf);  
    return RM_OK;
};

///////////////////////////////////////////////////////////////////////////////
// Loads Terrain from sFileName - PRIVATE.
///////////////////////////////////////////////////////////////////////////////
#define TERRAIN_LIST_LOAD(TYPE)\
    fscanf_s(fr, "%u\n", &(_Terrain.ui##TYPE##Count));\
    _Terrain.p##TYPE##IDs = new RESOURCEID[_Terrain.ui##TYPE##Count];\
    _Terrain.p##TYPE##FileNames = new CAtlString[_Terrain.ui##TYPE##Count];\
    if (!_Terrain.p##TYPE##IDs) HE(ERROUTOFMEMORY, "CResourceManager::LoadTerrain()", "Not enough memory for p"RES_NAME_##TYPE"IDs!");\
    for (i = 0; i < _Terrain.ui##TYPE##Count; i++) {\
        if (!fgets(buffer, MAX_FILENAME_SIZE, fr)) break;\
        buffer[strlen(buffer)-1] = 0;\
        _Terrain.p##TYPE##FileNames[i] = CAtlString(buffer);\
        res = bLoadInheritedResources ? LoadResource(buffer, RES_##TYPE, pParam) : -1;\
        if (res >= 0) {\
            _Terrain.p##TYPE##IDs[i] = res;\
            pvInheritedResources->push_back(res);\
        } else if (bLoadInheritedResources) break;\
    }\
    if (i < _Terrain.ui##TYPE##Count) {\
        ErrorHandler.HandleError(ERRGENERIC, "CResourceManager::LoadTerrain()", CAtlString("Could not load "RES_NAME_##TYPE" '") + CAtlString(buffer) + "'.");\
        _Terrain.ui##TYPE##Count = i;\
        if (bLoadInheritedResources) {\
            for (j = 0; j < _Terrain.uiTextureCount; j++)\
                ReleaseResource(_Terrain.pTextureIDs[j]);\
            for (j = 0; j < _Terrain.uiPhysicalTextureCount; j++)\
                ReleaseResource(_Terrain.pPhysicalTextureIDs[j]);\
        }\
        delete[] _Terrain.pTextureIDs;\
        delete[] _Terrain.pPhysicalTextureIDs;\
        delete[] _Terrain.pTerrainPlateInfo;\
        delete[] _Terrain.pTextureFileNames;\
        delete[] _Terrain.pPhysicalTextureFileNames;\
        delete[] _Terrain.pGrassFileNames;\
        fclose(fr);\
        return ERRGENERIC;\
    }



LOAD_RESOURCE_HEADER(Terrain) {
    Terrain _Terrain;
    TerrainPlateInfo _tpi;
    FILE *fr;
    UINT i, j, n;
    char buffer[MAX_FILENAME_SIZE];
    HRESULT res;
    std::set<UINT> uisetGrPlates;
    std::set<UINT>::iterator uisetIterator;
    CAtlString sFmt;
           
    if (fopen_s(&fr, sFileName, "r")) HE(ERRFILENOTFOUND, "CResourceManager::LoadTerrain()", sResourceName);
    
    fscanf_s(fr, "%u %u\n", &(_Terrain.uiSizeX), &(_Terrain.uiSizeY));
    if (!fgets(buffer, MAX_FILENAME_SIZE, fr)) HE(ERRFILECORRUPTED, "ResourceManager::LoadTerrain()", sResourceName);
    buffer[strlen(buffer) - 1] = 0;    
    _Terrain.sTextureSet =  new CAtlString(buffer);
    if (!fgets(buffer, MAX_FILENAME_SIZE, fr)) HE(ERRFILECORRUPTED, "ResourceManager::LoadTerrain()", sResourceName);
    buffer[strlen(buffer) - 1] = 0;    
    _Terrain.sWallTextureSet = new CAtlString(buffer);
    res = bLoadInheritedResources ? LoadResource(* _Terrain.sWallTextureSet, RES_TextureSet, true, NULL) : -1;
    if (res > 0) {
        _Terrain.ridWallTextureSet = res;
        pvInheritedResources->push_back(res);
    } else _Terrain.ridWallTextureSet = -1;


    _Terrain.pTerrainPlateInfo = new TerrainPlateInfo[_Terrain.uiSizeX*_Terrain.uiSizeY];
    if (!_Terrain.pTerrainPlateInfo) {
        fclose(fr);
        HE(ERROUTOFMEMORY, "CResourceManager::LoadTerrain()", "Not enough memory for pTerrainPlateInfo!");
    }

    _Terrain.uiGrassCount = 0;
    _Terrain.uiPhysicalTextureCount = 0;
    _Terrain.uiTextureCount = 0;
    _Terrain.uiGrPlateCount = 115;

        
    fscanf_s(fr, "%u\n", &(_Terrain.uiGrassCount));
    _Terrain.pGrassFileNames = new CAtlString[_Terrain.uiGrassCount];
    if (!_Terrain.pGrassFileNames) HE(ERROUTOFMEMORY, "CResourceManager::LoadTerrain()", "Not enough memory for pGrassFileNames!");
    for (i = 0; i < _Terrain.uiGrassCount; i++) {
        if (!fgets(buffer, MAX_FILENAME_SIZE, fr)) break;
        buffer[strlen(buffer)-1] = 0;
        _Terrain.pGrassFileNames[i] = CAtlString(buffer);
    }
    if (i < _Terrain.uiGrassCount) {
        ErrorHandler.HandleError(ERRGENERIC, "CResourceManager::LoadTerrain()", CAtlString("Could not load GrassFileNames, last was '") + CAtlString(buffer) + "'.");
        _Terrain.uiGrassCount = i;
        delete[] _Terrain.pTerrainPlateInfo;
        delete[] _Terrain.pGrassFileNames;
        fclose(fr);
        return ERRGENERIC;
    }


    TERRAIN_LIST_LOAD(Texture)
    TERRAIN_LIST_LOAD(PhysicalTexture)

    //Terrain plate info
    n = 7;
    for (i = _Terrain.uiSizeY; i > 0 && n == 7; i--)
        for (j = 0; j < _Terrain.uiSizeX; j++) {
            n = fscanf_s(fr, "%u %u %u %u %u %u %u", &(_tpi.uiHeight), &(_tpi.uiGrPlateIndex), &(_tpi.uiPlateRotation), &(_tpi.uiTextureIndex), &(_tpi.uiPhysicalTextureIndex), &(_tpi.uiTextureRotation), &(_tpi.uiGrassIndex));
            if (n < 7) break;
            _Terrain.pTerrainPlateInfo[(i-1)*_Terrain.uiSizeX+j] = _tpi;
            uisetGrPlates.insert(_tpi.uiGrPlateIndex);
        };
    if (n < 7) {
        ErrorHandler.HandleError(ERRFILECORRUPTED, "CResourceManager::LoadTerrain()", sResourceName);
        if (bLoadInheritedResources) {
            for (j = 0; j < _Terrain.uiTextureCount; j++)
                ReleaseResource(_Terrain.pTextureIDs[j]);
            for (j = 0; j < _Terrain.uiPhysicalTextureCount; j++)
                ReleaseResource(_Terrain.pPhysicalTextureIDs[j]);
        }
        delete[] _Terrain.pTextureIDs;
        delete[] _Terrain.pPhysicalTextureIDs;
        delete[] _Terrain.pTerrainPlateInfo;
        delete[] _Terrain.pPhysicalTextureFileNames;
        delete[] _Terrain.pTextureFileNames;
        fclose(fr);
        return ERRGENERIC;    
    }
    
    uisetIterator = uisetGrPlates.begin();
    _Terrain.pGrPlateIDs = new RESOURCEID[115];
    for (i = 0; i < 115; i++)
        _Terrain.pGrPlateIDs[i] = -1;
	if (bLoadInheritedResources) {
		do {
            sFmt.Format("type%i\\normal.grPlate", *uisetIterator);
            _Terrain.pGrPlateIDs[*uisetIterator] = LoadResource(sFmt, RES_GrPlate, bLoadInheritedResources, NULL);
			uisetIterator++;
		} while (uisetIterator != uisetGrPlates.end()); 
	}
    rcX = CResourceContainer(this, sResourceName, _Terrain);
    fclose(fr);
    return RM_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Loads GrSurface from sFileName - PRIVATE.
///////////////////////////////////////////////////////////////////////////////
LOAD_RESOURCE_HEADER(GrSurface) {
    GrSurface _GrSurface;
    struct TG_FILEHEADER tgFH;
    struct _VERTEX  {
        float x, y, z;
        float nx, ny, nz;
        float tu, tv;
    } ;
    DWORD VertexFVF = ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 );
    HRESULT hr;
    size_t sz;
    FILE *fr;
    struct _VERTEX  *v;
    D3DXVECTOR3 v3, *v3buffer;
    D3DXVECTOR2 v2, *v2buffer;
    int i, j;

//struct TG_FILEHEADER
//{
//	float	fSizeX, fSizeZ;
//	int		iSegsX, iSegsZ;
//	int		iOptions;
//};


    rcX.uiResType = RES_NoType;

    if (fopen_s(&fr, sFileName, "rb")) HE(ERRFILENOTFOUND, "CResourceManager::LoadGrSurface()", sResourceName);
    if (!fread(&tgFH, sizeof(struct TG_FILEHEADER), 1, fr)) {
        fclose(fr);
        HE(ERRFILECORRUPTED, "CResourceManager::LoadGrSurface()", sResourceName);
    }

    // allocate memory for vertex buffer
    if (!(v =  new _VERTEX[(tgFH.iSegsX+1) * (tgFH.iSegsZ+1)])) {
        fclose(fr);
        HE(ERROUTOFMEMORY, "CResourceManager::LoadGrSurface()", "");
    }
    //allocate memory for temporary structures
    if (!(v3buffer = new D3DXVECTOR3[(tgFH.iSegsX+1) * (tgFH.iSegsZ+1)]) ||
        !(v2buffer = new D3DXVECTOR2[(tgFH.iSegsX+1) * (tgFH.iSegsZ+1)])) 
    {
        fclose(fr);
        if (v3buffer) delete[] v3buffer;
        if (v2buffer) delete[] v2buffer;
        delete[] v;
        HE(ERROUTOFMEMORY, "CResourceManager::LoadGrSurface()", "");
    } 

    // load vertices
    sz = fread(v3buffer, sizeof(D3DXVECTOR3), (tgFH.iSegsX+1) * (tgFH.iSegsZ+1), fr);
    if (sz != (tgFH.iSegsX+1) * (tgFH.iSegsZ+1)) {
        fclose(fr);
        delete[] v3buffer;
        delete[] v2buffer;
        delete[] v;
        HE(ERRFILECORRUPTED, "CResourceManager::LoadGrSurface()", sResourceName);
    }
    for (i = 0; i <= tgFH.iSegsZ; i++) 
        for (j = 0; j <= tgFH.iSegsX; j++) {
            v[i*(tgFH.iSegsX + 1) + j].x = v3buffer[i*(tgFH.iSegsX + 1) + j].x;
            v[i*(tgFH.iSegsX + 1) + j].y = v3buffer[i*(tgFH.iSegsX + 1) + j].y;
            v[i*(tgFH.iSegsX + 1) + j].z = v3buffer[i*(tgFH.iSegsX + 1) + j].z;
        }

    // load texture coordinates    
    if (tgFH.iOptions & 2) {
        sz = fread(v2buffer, sizeof(D3DXVECTOR2), (tgFH.iSegsX+1) * (tgFH.iSegsZ+1), fr);
        if (sz != (tgFH.iSegsX+1) * (tgFH.iSegsZ+1)) {
            fclose(fr);
            delete[] v3buffer;
            delete[] v2buffer;
            delete[] v;
            HE(ERRFILECORRUPTED, "CResourceManager::LoadGrSurface()", sResourceName);
        }
        for (i = 0; i <= tgFH.iSegsZ; i++) 
            for (j = 0; j <= tgFH.iSegsX; j++) {
                v[i*(tgFH.iSegsX + 1) + j].tu = v2buffer[i*(tgFH.iSegsX + 1) + j].x;
                v[i*(tgFH.iSegsX + 1) + j].tv = v2buffer[i*(tgFH.iSegsX + 1) + j].y;
            }
    }
    
    delete[] v2buffer; //no longer needed

    // load normals    
    if (tgFH.iOptions & 4) {
        sz = fread(v3buffer, sizeof(D3DXVECTOR3), (tgFH.iSegsX+1) * (tgFH.iSegsZ+1), fr);
        if (sz != (tgFH.iSegsX+1) * (tgFH.iSegsZ+1)) {
            fclose(fr);
            delete[] v3buffer;
            delete[] v;
            HE(ERRFILECORRUPTED, "CResourceManager::LoadGrSurface()", sResourceName);
        }
        for (i = 0; i <= tgFH.iSegsZ; i++) 
            for (j = 0; j <= tgFH.iSegsX; j++) {
                v[i*(tgFH.iSegsX + 1) + j].nx = v3buffer[i*(tgFH.iSegsX + 1) + j].x;
                v[i*(tgFH.iSegsX + 1) + j].ny = v3buffer[i*(tgFH.iSegsX + 1) + j].y;
                v[i*(tgFH.iSegsX + 1) + j].nz = v3buffer[i*(tgFH.iSegsX + 1) + j].z;
            }
    }

    delete[] v3buffer; //no longer needed
    fclose(fr);

    // create and fill vertex buffer
    hr = D3DDevice->CreateVertexBuffer( (tgFH.iSegsX+1) * (tgFH.iSegsZ+1) * sizeof(_VERTEX), D3DUSAGE_WRITEONLY, VertexFVF, D3DPOOL_MANAGED, &_GrSurface.pVertices, NULL);
    if (hr) {
        delete[] v;
        HE(hr, "CResourceManager::LoadGrSurface()", "Vertex buffer can not be created.");
    }
    _VERTEX    *pBuf;
    _GrSurface.pVertices->Lock(0, 0, (LPVOID *) &pBuf, 0);
    pBuf = (_VERTEX *) memcpy( pBuf, v, sizeof(_VERTEX) * (tgFH.iSegsX + 1) * (tgFH.iSegsZ + 1) );
    _GrSurface.pVertices->Unlock();

    // create and fill index buffer
 //   hr = D3DDevice->CreateIndexBuffer( ( 2 * tgFH.iSegsX * (tgFH.iSegsZ + 1) + 1 ) * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &_GrSurface.pIndices, NULL );
 //   if (hr) {
 //       delete[] v;
 //       HE(hr, "CResourceManager::LoadGrSurface()", "Index buffer can not be created.");
 //   }
 //   WORD  *index;
 //   int ind = 0;
 //   _GrSurface.pIndices->Lock( 0, 0, (LPVOID *) &index, 0 );
	//index[ind++] = 0;
	//index[ind++] = tgFH.iSegsX + 1;
	//for ( int i = 0; i < tgFH.iSegsZ; ) {
 //       for ( int j = 0; j < tgFH.iSegsX; j++ ) {
 //           index[ind++] = i*(tgFH.iSegsX + 1) + j + 1;
 //           index[ind++] = (i + 1)*(tgFH.iSegsX + 1) + j + 1;
 //       }
	//	i++;
	//	if ( i >= tgFH.iSegsZ ) break;
	//	index[ind++] = (i + 1)*(tgFH.iSegsX + 1) + tgFH.iSegsX;

	//	for ( int j = tgFH.iSegsX - 1; j >= 0; j-- ) {
 //           index[ind++] = i*(tgFH.iSegsX + 1) + j;
 //           index[ind++] = (i + 1)*(tgFH.iSegsX + 1) + j;
 //       }
	//	i++;
	//	if ( i >= tgFH.iSegsZ ) break;
	//	index[ind++] = (i + 1)*(tgFH.iSegsX + 1);
 //   }
 //   _GrSurface.pIndices->Unlock();

	// another way - rendering plate as a triangle list
    hr = D3DDevice->CreateIndexBuffer( 6 * tgFH.iSegsX * tgFH.iSegsZ * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &_GrSurface.pIndices, NULL );
    if (hr) {
        delete[] v;
        HE(hr, "CResourceManager::LoadGrSurface()", "Index buffer can not be created.");
    }
    WORD  *index;
    _GrSurface.pIndices->Lock( 0, 0, (LPVOID *) &index, 0 );
    for (int i = 0; i < tgFH.iSegsZ; i++) {
        for (int j = 0; j < tgFH.iSegsX; j++) {
            index[6*i*tgFH.iSegsX + 6*j + 0] = i*(tgFH.iSegsX + 1) + j;
            index[6*i*tgFH.iSegsX + 6*j + 1] = (i+1)*(tgFH.iSegsX + 1) + j;
            index[6*i*tgFH.iSegsX + 6*j + 2] = (i+1)*(tgFH.iSegsX + 1) + j + 1;
            index[6*i*tgFH.iSegsX + 6*j + 3] = (i+1)*(tgFH.iSegsX + 1) + j + 1;
            index[6*i*tgFH.iSegsX + 6*j + 4] = i*(tgFH.iSegsX + 1) + j + 1;
            index[6*i*tgFH.iSegsX + 6*j + 5] = i*(tgFH.iSegsX + 1) + j;
        }
    }
    _GrSurface.pIndices->Unlock();


    _GrSurface.SegsCountX = tgFH.iSegsX;
    _GrSurface.SegsCountZ = tgFH.iSegsZ;
    _GrSurface.VertsCount = (tgFH.iSegsX+1) * (tgFH.iSegsZ+1);

    rcX = CResourceContainer(this, sResourceName, _GrSurface);
    delete[] v;

    return RM_OK;
};

///////////////////////////////////////////////////////////////////////////////
// Loads VertexShader from sFileName - PRIVATE.
///////////////////////////////////////////////////////////////////////////////
LOAD_RESOURCE_HEADER(VertexShader) {
    ShaderLoadParams * _Params = (ShaderLoadParams *) pParam;
    // temporary buffer
    ID3DXBuffer * TempBuf;
    // error buffer
    ID3DXBuffer * ErrorBuf;
    char * ErrorDesc;
    // Shader info struct - TADY DO TOHO SE MA ULOZIT VYSLEDEK
    VertexShader Shader; 
    // compile vertex shader
    HRESULT Result = D3DXCompileShaderFromFile( sFileName, 0, 0, 
						    _Params->MainName, 
						    _Params->ShaderVersion,
						    _Params->Flags, 
						    &TempBuf, &ErrorBuf, &(Shader.ConstTable));
	if (FAILED(Result)) {
		ErrorDesc = (char *) ErrorBuf->GetBufferPointer();
		OUTMSG(ErrorDesc, 1); // Kdyby tohle slo jeste taky primo do error souboru, tak by to bylo skvely
		SAFE_RELEASE(TempBuf);
		SAFE_RELEASE(ErrorBuf);
		HE(Result, "CResourceManager::LoadVertexShader()", ("Unable to compile vertex shader: " + sResourceName) );
	}

	// Create vertex shader
    Result = D3DDevice->CreateVertexShader((DWORD *) TempBuf->GetBufferPointer(), &(Shader._VS) );
    if (FAILED(Result)) {
		SAFE_RELEASE(Shader.ConstTable);
		HE(Result, "CResourceManager::LoadVertexShader()", ("Unable to create vertex shader: " + sResourceName) );
	}
    SAFE_RELEASE(TempBuf);
	SAFE_RELEASE(ErrorBuf);
    rcX = CResourceContainer(this, sResourceName, Shader);
    return Result;
}

///////////////////////////////////////////////////////////////////////////////
// Loads PixeShader from sFileName - PRIVATE.
///////////////////////////////////////////////////////////////////////////////
LOAD_RESOURCE_HEADER(PixelShader) {
    ShaderLoadParams * _Params = (ShaderLoadParams *) pParam;
    // temporary buffer
    ID3DXBuffer * TempBuf;
    // error buffer
    ID3DXBuffer * ErrorBuf;
    char * ErrorDesc;
    // Shader info struct - TADY DO TOHO SE MA ULOZIT VYSLEDEK
    PixelShader Shader; 
    // compile vertex shader
    HRESULT Result = D3DXCompileShaderFromFile( sFileName,
						    0, 0, 
						    _Params->MainName, 
						    _Params->ShaderVersion,
						    _Params->Flags, 
						    &TempBuf, &ErrorBuf, &(Shader.ConstTable));
	if (FAILED(Result)) 	{
		ErrorDesc = (char *) ErrorBuf->GetBufferPointer();
		OUTMSG(ErrorDesc, 1); // Kdyby tohle slo jeste taky primo do error souboru, tak by to bylo skvely
		SAFE_RELEASE(TempBuf);
		SAFE_RELEASE(ErrorBuf);
		HE(Result, "CResourceManager::LoadPixelShader()", ("Unable to compile pixel shader: " + sResourceName) );
	}

	// Create vertex shader
	Result = D3DDevice->CreatePixelShader((DWORD *) TempBuf->GetBufferPointer(), &(Shader._PS) );
    if (FAILED(Result) ) {
		SAFE_RELEASE(Shader.ConstTable);
		HE(Result, "CResourceManager::LoadPixelShader()", ("Unable to create pixel shader: " + sResourceName) );
	}
    SAFE_RELEASE(TempBuf);
    SAFE_RELEASE(ErrorBuf);	
    rcX = CResourceContainer(this, sResourceName, Shader);
    return Result;
}


///////////////////////////////////////////////////////////////////////////////
// Loads Road from sFileName - PRIVATE.
///////////////////////////////////////////////////////////////////////////////

#define ROAD_LIST_LOAD(TYPE)\
    fscanf_s(fr, "%u", &(_Road.ui##TYPE##Count));\
    _Road.p##TYPE##IDs = new RESOURCEID[_Road.ui##TYPE##Count];\
    if (!_Road.p##TYPE##IDs) HE(ERROUTOFMEMORY, "CResourceManager::LoadRoad()", "Not enough memory for p"RES_NAME_##TYPE"IDs!");\
    for (i = 0; i < _Road.ui##TYPE##Count; i++) {\
        if (!fgets(buffer, MAX_FILENAME_SIZE, fr)) break;\
        buffer[strlen(buffer)-1] = 0;\
        res = LoadResource(buffer, RES_##TYPE);\
        if (res >= 0) {\
            _Road.p##TYPE##IDs[i] = res;\
            pvInheritedResources->push_back(res);\
        } else break;\
    }\
    if (i < _Road.ui##TYPE##Count) {\
        ErrorHandler.HandleError(ERRGENERIC, "CResourceManager::LoadRoad()", CAtlString("Could not load "RES_NAME_##TYPE" '") + CAtlString(buffer) + "'.");\
        _Road.ui##TYPE##Count = i;\
        for (j = 0; j < _Road.uiTextureCount; j++)\
            ReleaseResource(_Road.pTextureIDs[j]);\
        for (j = 0; j < _Road.uiPhysicalTextureCount; j++)\
            ReleaseResource(_Road.pPhysicalTextureIDs[j]);\
        delete[] _Road.pTextureIDs;\
        delete[] _Road.pPhysicalTextureIDs;\
        delete[] _Road.pRoadPlateInfo;\
        fclose(fr);\
        return ERRGENERIC;\
    }


LOAD_RESOURCE_HEADER(Road) {
    Road _Road;
    FILE *fr;
    char buffer[MAX_FILENAME_SIZE];
    RoadPlateInfo _rpi;
    UINT i, j, n;
    CResourceContainer _rcX;
    HRESULT res;
      
    
    if (fopen_s(&fr, sFileName, "r")) HE(ERRFILENOTFOUND, "CResourceManager::LoadRoad()", sResourceName);
    
    fscanf_s(fr, "%u %u\n", &(_Road.uiSizeX), &(_Road.uiSizeY));
    
    _Road.pRoadPlateInfo =  new RoadPlateInfo[_Road.uiSizeX*_Road.uiSizeY];
    if (!_Road.pRoadPlateInfo) {
        fclose(fr);
        HE(ERROUTOFMEMORY, "CResourceManager::LoadRoad()", "Not enough memory for pRoadPlateInfo!");
    }

    _Road.uiPhysicalTextureCount = 0;
    _Road.uiTextureCount = 0;


    ROAD_LIST_LOAD(Texture)
    ROAD_LIST_LOAD(PhysicalTexture)

    n = 3;
    for (i = 0; i < _Road.uiSizeY && n == 3; i++)
        for (j = 0; j < _Road.uiSizeX; j++) {
            n = fscanf_s(fr, "%u %u %u %u ", &(_rpi.uiTextureIndex), &(_rpi.uiPhysicalTextureIndex), &(_rpi.uiRotation));
            if (n < 3) {
                ErrorHandler.HandleError(ERRFILECORRUPTED, "CResourceManager::LoadRoad()", sResourceName);
                for (j = 0; j < _Road.uiTextureCount; j++)
                    ReleaseResource(_Road.pTextureIDs[j]);
                for (j = 0; j < _Road.uiPhysicalTextureCount; j++)
                    ReleaseResource(_Road.pPhysicalTextureIDs[j]);
                delete[] _Road.pTextureIDs;
                delete[] _Road.pPhysicalTextureIDs;
                delete[] _Road.pRoadPlateInfo;
                fclose(fr);
                return ERRGENERIC;    
            } else _Road.pRoadPlateInfo[i*_Road.uiSizeX+j] = _rpi;
        };
    if (n < 3) {
    }

    rcX = CResourceContainer(this, sResourceName, _Road);
    fclose(fr);

    return RM_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Loads GrPlate from sFileName - PRIVATE.
///////////////////////////////////////////////////////////////////////////////

LOAD_RESOURCE_HEADER(GrPlate) {
    GrPlate _GrPlate;
    char buffer[MAX_FILENAME_SIZE];
    FILE *fr;
    HRESULT res;
    int iLoDCount = (pParam ? *((int *) pParam) : 0), i;
    
    if (fopen_s(&fr, sFileName, "r")) HE(ERRFILENOTFOUND, "CResourceManager::LoadGrPlate()", sFileName);
    
    for (i = 0; i < TERRAIN_LOD_COUNT; i++) _GrPlate.LoD[i] = -1;

    fscanf_s(fr, "%u\n", &(_GrPlate.uiLoDCount));

    if (!iLoDCount) iLoDCount = (int) _GrPlate.uiLoDCount;

    if (_GrPlate.uiLoDCount < (UINT) iLoDCount) {
        fclose(fr);
        HE(ERRGENERIC, "CResourceManager::LoadGrPlate()", "File '"+sFileName+"'contains less LoDs than requested!");
    }

    for (res = 1, i = 0; res && i < iLoDCount; i++) {
        fgets(buffer, MAX_FILENAME_SIZE, fr);
        buffer[strlen(buffer)-1] = 0;
        res = LoadResource(CAtlString(buffer), RES_GrSurface);
        if (res >= 0) {
            _GrPlate.LoD[i] = res;
            pvInheritedResources->push_back(res);
        } else break;
    }
    
    if (res < 0) {
        for (i = 0; i < iLoDCount; i++)
            if (_GrPlate.LoD[i] != -1) ReleaseResource(_GrPlate.LoD[i]);
        fclose(fr);
        HE(ERRGENERIC, "CResourceManager::LoadGrPlate()", "Could not load GrPlate '"+CAtlString(buffer)+"'!");
    }

	fscanf_s(fr, "%u\n%u", &(_GrPlate.bPlanar), &(_GrPlate.bConcave));

    rcX = CResourceContainer(this, sResourceName, _GrPlate);
    fclose(fr);

    return RM_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Loads PhysicalTexture from sFileName - PRIVATE.
///////////////////////////////////////////////////////////////////////////////

LOAD_RESOURCE_HEADER(PhysicalTexture) {
    PhysicalTexture _PhysicalTexture;
    FILE *fr;
    UINT i, j, n;

    if (fopen_s(&fr, sFileName, "r")) HE(ERRFILENOTFOUND, "CResourceManager::LoadPhysicalTexture()", sFileName);

	for (i = PHYSICAL_TEXTURE_SIZE; i > 0; i--) 
        for (j = 0; j < PHYSICAL_TEXTURE_SIZE; j++) {
            n = fscanf_s(fr, "%i", &(_PhysicalTexture.pPhysicalTexture[j][i-1]/*.iCoefIndex*/));
            if (n < 1) {
                fclose(fr);
                HE(ERRFILECORRUPTED, "CResourceManager::LoadPhysicalTexture()", sFileName);
            }
            //if (_PhysicalTexture.pPhysicalTexturePoint[i][j].iCoefIndex < 0) {
            //    _PhysicalTexture.pPhysicalTexturePoint[i][j].bTransparent = true;
            //    _PhysicalTexture.pPhysicalTexturePoint[i][j].iCoefIndex *= -1;
            //}
        }
        
    rcX = CResourceContainer(this, sResourceName, _PhysicalTexture);
    return RM_OK;
}
 
///////////////////////////////////////////////////////////////////////////////
// Loads LoDMap from sFileName - PRIVATE.
///////////////////////////////////////////////////////////////////////////////

LOAD_RESOURCE_HEADER(LoDMap) {
    FILE *fr;
    LoDMap _LoDMap;
    int i, j, k;
            
    if (fopen_s(&fr, sFileName, "r")) HE(ERRFILENOTFOUND, "CResourceManager::LoadLoDMap()", sFileName);

    fscanf_s(fr, "%i %i", &(_LoDMap.iWidth), &(_LoDMap.iHeight));
    
    _LoDMap.pData = new char[_LoDMap.iWidth*_LoDMap.iHeight];
    if (!_LoDMap.pData) HE(ERROUTOFMEMORY, "CResourceManager::LoadLoDMap()", "Not Enough memory for pData!");

    for (j = 0; j < _LoDMap.iHeight; j++)
        for (i = 0; i < _LoDMap.iWidth; i++) {
            fscanf_s(fr, "%i", &k);
            _LoDMap.pData[_LoDMap.iWidth*j+i] = k;
        }

    fclose(fr);

    rcX = CResourceContainer(this, sResourceName, _LoDMap);
    return RM_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Loads Font from sFileName - PRIVATE.
///////////////////////////////////////////////////////////////////////////////
D3DXFONT_DESC FontDesc;
int iColor = 0x00ffffff, iSimpleDeclaration = 1;

#define MIN(a,b) ((a) < (b) ? (a) : (b))

void CALLBACK LoadFontCallBack( LPNAMEVALUEPAIR pPair )  {
    CAtlString sName = pPair->GetName();
    CAtlString sValue;
    int iLength;

    sName.MakeUpper();
    if (sName == "SIMPLEDECLARATION") iSimpleDeclaration = pPair->GetInt();
    if (sName == "COLOR") iColor = pPair->GetInt();
    if (sName == "HEIGHT") FontDesc.Height = pPair->GetInt();
    if (sName == "WIDTH")  FontDesc.Width  = pPair->GetInt();
    if (sName == "WEIGHT") FontDesc.Weight = pPair->GetInt();
    if (sName == "MIPLEVELS") FontDesc.MipLevels = pPair->GetInt();
    if (sName == "ITALIC") FontDesc.Italic = pPair->GetInt();
    if (sName == "CHARSET") FontDesc.CharSet = pPair->GetInt();
    if (sName == "QUALITY") FontDesc.Quality = pPair->GetInt();
    if (sName == "PITCH") FontDesc.PitchAndFamily = pPair->GetInt();
    if (sName == "FACENAME") {
        sValue = pPair->GetString();
        iLength = sValue.GetLength();
        memcpy(FontDesc.FaceName, sValue.GetString(), MIN(iLength+1, 32));
        FontDesc.FaceName[31] = 0;
    }
};


LOAD_RESOURCE_HEADER(FontDef) {
    HRESULT ret;
    FontDef _FontDef;
    HRESULT	hr;

    FontDesc.Height = 18;
    FontDesc.Width = 0;
    FontDesc.Weight = 400;
    FontDesc.MipLevels = 0;
    FontDesc.Italic = FALSE;
    FontDesc.CharSet = 1;
    FontDesc.Quality = 0;
    FontDesc.PitchAndFamily = 0;
    memcpy(FontDesc.FaceName, "Arial", 6);

    if (ret = NAMEVALUEPAIR::Load( sFileName, LoadFontCallBack, NULL )) return ret;

    _FontDef.iColor = iColor;
    FontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;    

    hr = D3DXCreateFontIndirect(D3DDevice, &FontDesc, &(_FontDef.pFont));
    if (hr != D3D_OK) HE( hr, "CResourceManager::LoadFontDef()", "Font object could not be created.");

    rcX = CResourceContainer(this, sResourceName, _FontDef);
    return RM_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Loads FilePack from sFileName - PRIVATE.
///////////////////////////////////////////////////////////////////////////////
LOAD_RESOURCE_HEADER(FilePack) {
    FilePack _FilePack;
    DWORD dwFileSize, dwOffset, i, dwFileOffset;
    BYTE lpTmp[256];
    FILE *fr;
    
    if (fopen_s(&fr, sFileName, "rb")) HE(ERRGENERIC, "CResourceManager::LoadFilePack()", "File '"+sFileName+"' not found!");
    
    fseek(fr, 0, SEEK_END);
    dwFileSize = ftell(fr);
    fseek(fr, 0, SEEK_SET);

    _FilePack.lpBasePtr =  new BYTE[dwFileSize];
    if (!_FilePack.lpBasePtr) {
        fclose(fr);
        HE(ERRGENERIC, "CResourceManager::LoadFilePack()", "Memory block for file '"+sFileName+"' could not be allocated!");
    }

    if (fread(_FilePack.lpBasePtr, sizeof(BYTE), dwFileSize, fr) < dwFileSize) {
        delete[] _FilePack.lpBasePtr;
        fclose(fr);
        HE(ERRGENERIC, "CResourceManager::LoadFilePack()", "Could not read contents of file '" + sFileName + "'!");
    }

    _FilePack.dwFileCount = *((DWORD *) _FilePack.lpBasePtr);
    _FilePack.lpFPI = new FilePackInfo[_FilePack.dwFileCount];

    if (!(_FilePack.lpFPI)) {
        delete[] _FilePack.lpBasePtr;
        fclose(fr);
        HE(ERRGENERIC, "CResourceManager::LoadFilePack()", "Could allocate memory for FilePackInfos!");
    }
    
    dwOffset = sizeof(DWORD);
    dwFileOffset = sizeof(DWORD) + (sizeof(DWORD) + 256) * _FilePack.dwFileCount;
    for (i = 0; i < _FilePack.dwFileCount && (!i || _FilePack.lpFPI[i-1].dwFileOffset+_FilePack.lpFPI[i-1].dwFileSize <= dwFileSize); i++) {
        ZeroMemory(lpTmp, 256);
        memcpy(lpTmp, _FilePack.lpBasePtr+dwOffset, 256);
        _FilePack.lpFPI[i].sFileName = CAtlString(lpTmp);
        _FilePack.lpFPI[i].sFileName.MakeUpper();
        dwOffset += 256;
        _FilePack.lpFPI[i].dwFileSize = *((DWORD *) (_FilePack.lpBasePtr + dwOffset));
        dwOffset += sizeof(DWORD);
        _FilePack.lpFPI[i].dwFileOffset = dwFileOffset;
        dwFileOffset += _FilePack.lpFPI[i].dwFileSize;
    }

    fclose(fr);
    rcX = CResourceContainer(this, sResourceName, _FilePack); 
    return RM_OK;
}

int FindFile(FilePack * pFP, CAtlString sFileName) { // hleda soubor jmena sFileName ve FilePacku
    DWORD i;
    sFileName.MakeUpper();
    OUTS("Looking for file '" + sFileName + "' in FilePack ... ", 3);
    for (i = 0; i < pFP->dwFileCount && pFP->lpFPI[i].sFileName != sFileName; i++);
    if  (i < pFP->dwFileCount) {
        OUTSN("FOUND!", 3);
        return i;
    } else {
        OUTSN("NOT FOUND!", 3);
        return -1;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Loads Model from sFileName - PRIVATE.
///////////////////////////////////////////////////////////////////////////////
LOAD_RESOURCE_HEADER(Model) {
    Model CM;
    ModelLoadParams _CMLP = *((ModelLoadParams *) pParam);
    MemoryXFileLoadParams _MXFLP;
    FilePack * pFP = NULL;
    DWORD i;
    int len;
    RESOURCEID FPID;
    CResourceManager::CResourceContainer * rcFP;
    DWORD dwXFileCount = 0;

    rcX.uiResType = RES_NoType;

    FPID = LoadResource(sFileName+"$*** FilePack ***", RES_FilePack); //nacteme FilePack
    if (GetResource(FPID, &rcFP)) pFP = &(rcFP->_ResourceData._FilePack); //s touhle strukturou budeme pracovat
 
    if (!pFP) { // neco se pokazilo, konec
        ReleaseResource(FPID);
        HE(ERRGENERIC, "CResourceManager::LoadModel()", "Could not load filepack!");
    }
    for (i = 0; i < pFP->dwFileCount; i++) { // spocitame XFily ve FilePacku
        len = pFP->lpFPI[i].sFileName.GetLength();
        if (len < 3 || pFP->lpFPI[i].sFileName[len-2] != '.' || !(pFP->lpFPI[i].sFileName[len-1] == 'x' || pFP->lpFPI[i].sFileName[len-1] =='X')) 
            break;
        dwXFileCount++;
    }
    
    if (!dwXFileCount) { //zadne jsme nenasli?
        ReleaseResource(FPID);
        HE(ERRGENERIC, "CResourceManager::LoadModel()", "There are no XFiles in filepack '" + sFileName + "'!");
    }

    _MXFLP.pFP = pFP;
    _MXFLP.uiOptions = _CMLP.uiOptions;

    CM.dwXFileCount = _CMLP.uiLODCount;
    CM.pXFileIDs =  new RESOURCEID[_CMLP.uiLODCount];

    if(!CM.pXFileIDs) { // malo pameti?
        ReleaseResource(FPID);
        HE(ERRGENERIC, "CResourceManager::LoadModel()", "Not enough memory for pXFileIDs");
    }

    for (i = 0; i < _CMLP.uiLODCount; i++) { // nahrajeme je vsechny
        _MXFLP.uiIndex = (i < dwXFileCount ? i : (dwXFileCount - 1));
        if ((CM.pXFileIDs[i] = LoadResource(sFileName + "$" + pFP->lpFPI[_MXFLP.uiIndex].sFileName, RES_MemoryXFile, &_MXFLP)) < 0) break;
        pvInheritedResources->push_back(CM.pXFileIDs[i]);
    }

    ReleaseResource(FPID); // FilePack uz nepotrebujem

    if (i < _CMLP.uiLODCount && i > 0) { // nejaky XFile se nenahral? 
        for (; i > 0; i--)
            ReleaseResource(CM.pXFileIDs[i-1]); // uvolni vsechno, ohlas chybu a pryc
        HE(ERRGENERIC, "CResourceManager::LoadModel()", "One of LODs could not be loaded!");
    }
            
    rcX = CResourceContainer(this, sResourceName, CM);
    return RM_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Loads XFile from FilePack in memory - PRIVATE.
///////////////////////////////////////////////////////////////////////////////
LOAD_RESOURCE_HEADER(MemoryXFile) {
    XFile _XFile;
    MemoryXFileLoadParams * _MXFLP = (MemoryXFileLoadParams *) pParam;
    MemoryTextureLoadParams MTLP;
    LPD3DXBUFFER    pMtrlBuf; // zahod
    D3DXMATERIAL*   pMaterial; // tohle zahod
    long int i, index;
    RESOURCEID res;
    rcX.uiResType = RES_NoType;
    if (!_MXFLP || ! _MXFLP->pFP || !_MXFLP->pFP->lpBasePtr || !_MXFLP->pFP->lpFPI)
        HE(ERRGENERIC, "CResourceManager::LoadMemoryXFile()", "One of critical pointers is NULL.");
    //obdoba nacitani z disku, jen se tomu preda pointer a velikost bloku pameti...
    res = D3DXLoadMeshFromXInMemory(_MXFLP->pFP->lpBasePtr + _MXFLP->pFP->lpFPI[_MXFLP->uiIndex].dwFileOffset, _MXFLP->pFP->lpFPI[_MXFLP->uiIndex].dwFileSize, _MXFLP->uiOptions, D3DDevice, NULL, &pMtrlBuf, NULL, &(_XFile.cMtrl), &(_XFile.Mesh));
    if (res == D3DERR_INVALIDCALL) HE(res, "CResourceManager::LoadMemoryXFile()", "Could not load mesh from _XFile '"+sResourceName+"'. Invalid parameters.");
    if (res != D3D_OK) HE(res, "CResourceManager::LoadMemoryXFile()", "Could not load mesh from _XFile '"+sResourceName+"'.");
    pMaterial = (D3DXMATERIAL *) pMtrlBuf->GetBufferPointer();
    
    _XFile.Material = new D3DMATERIAL9[_XFile.cMtrl];
    _XFile.pTextureIDs = new RESOURCEID[_XFile.cMtrl];
    if (!_XFile.pTextureIDs || !_XFile.Material) { // na neci neni dost pameti ...
        delete[] _XFile.pTextureIDs;
        delete[] _XFile.Material;
        HE(ERROUTOFMEMORY, "CResourceManager::LoadMemoryXFile()", "Not Enough memory for pTextureIDs / Material!");
    }
    for (i = 0; (DWORD) i < _XFile.cMtrl; i++) { // tohle je k necemu potreba. Cert (nebo tempsoft) vi k cemu...
        _XFile.Material[i] = pMaterial[i].MatD3D;
        _XFile.Material[i].Ambient = _XFile.Material[i].Diffuse;
    }
    for (i = 0; (DWORD) i < _XFile.cMtrl && res >= 0; i++) {
        if (pMaterial[i].pTextureFilename) { // ma material texturu?
            index = FindFile(_MXFLP->pFP, CAtlString(pMaterial[i].pTextureFilename));
            if (index >= 0) { // textura je ve FilePacku, nacpi do parametru ukazatel na jeji zacatek + velikost
                MTLP.pSrcData = _MXFLP->pFP->lpBasePtr + _MXFLP->pFP->lpFPI[index].dwFileOffset;
                MTLP.uiSrcDataSize = _MXFLP->pFP->lpFPI[index].dwFileSize;
                res = LoadResource((sFileName+"$")+(char *)pMaterial[i].pTextureFilename, RES_MemoryTexture, &MTLP);
            } else { // textura neni ve FilePacku, zkus ji nahrat normalne z disku...
                res = LoadResource(CAtlString(pMaterial[i].pTextureFilename), RES_Texture);
            }
            if (res >= 0) { //ok
                _XFile.pTextureIDs[i] = res; 
                pvInheritedResources->push_back(res);
            } else _XFile.pTextureIDs[i] = -1;           // chyba nacitani (neexisetnce textury, ...)
        } else _XFile.pTextureIDs[i] = -1;    // U tohoho materialu v XFilu neni textura, ignorujeme a nenacitame
    }
    if (res < 0) {
        ErrorHandler.HandleError(ERRGENERIC, "CResourceManager::LoadMemoryXFile()", CAtlString("Could not load texture '") + CAtlString(pMaterial[i].pTextureFilename) + "'.");
        for (i--; i >= 0; i--) if (_XFile.pTextureIDs[i] >= 0) ReleaseResource(_XFile.pTextureIDs[i]); //hezky to vsechno uvolnime
        delete[]_XFile.pTextureIDs;
        delete[]_XFile.Material;   // a smazeme
        return ERRGENERIC;
    }

    rcX = CResourceContainer(this, sResourceName, _XFile); // naplnime ResourceContainer

    SAFE_RELEASE(pMtrlBuf);  // tohle uz nepotrebujeme
    return RM_OK; 
}

///////////////////////////////////////////////////////////////////////////////
// Loads Texture from FilePack in memory - PRIVATE.
///////////////////////////////////////////////////////////////////////////////
LOAD_RESOURCE_HEADER(MemoryTexture) {
    Texture _Texture;
    MemoryTextureLoadParams * _MTLP = (MemoryTextureLoadParams *) pParam;
    HRESULT res = D3DXCreateTextureFromFileInMemory(D3DDevice, _MTLP->pSrcData, _MTLP->uiSrcDataSize, &(_Texture.texture));
    if (res < D3D_OK) HE(res, "CResourceManager::LoadMemoryTexture()", CAtlString("Error while loading MemoryTexture from file '") + sResourceName + "'.");
    if (res > D3D_OK) HE(ERRGENERIC, "CResourceManager::LoadMemoryTexture()", CAtlString("Error while loading MemoryTexture from file '") + sResourceName + "'. (Positive error value)");
    rcX = CResourceContainer(this, sResourceName, _Texture);  
    return RM_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Loads Materials from sFileName - PRIVATE.
///////////////////////////////////////////////////////////////////////////////
LOAD_RESOURCE_HEADER(Materials) {
    Materials _Materials;
    FILE *fr;
    UINT n, i, j;
	char tmpStr1[MAX_FILENAME_SIZE], tmpStr2[MAX_FILENAME_SIZE];
	CAtlString * sMaterialNames;
	bool found_1, found_2;
	
	if(fopen_s(&fr, sFileName, "r"))
		HE(ERRFILENOTFOUND, "CResourceManager::LoadMaterials()", sFileName);

	fscanf_s(fr, "%u\n", &(_Materials.uiMaterialIDsCount));

	SAFE_NEW_ARRAY(sMaterialNames, CAtlString, _Materials.uiMaterialIDsCount, "CResourceManager::LoadMaterials()");
	//SAFE_NEW_ARRAY(_Materials.puiType, UINT ,_Materials.uiMaterialsCount, "CResourceManager::LoadMaterials()");
	//_Materials.puiType[0] = 0; // default material is not a car

	// load material names
	for(i=0; i<_Materials.uiMaterialIDsCount; i++)
	{
		fscanf_s(fr, "%s", tmpStr1, MAX_FILENAME_SIZE);
		sMaterialNames[i] = tmpStr1;
		//n = fscanf_s(fr, "%u\n", &(_Materials.puiType[i]));
		//if(n < 1)
		//{
		//	fclose(fr);
		//	SAFE_DELETE_ARRAY(sMaterialNames);
		//	SAFE_DELETE_ARRAY(_Materials.puiType);
		//	HE(ERRFILECORRUPTED, "CResourceManager::LoadMaterials()", sFileName);
		//}
	}

	fscanf_s(fr, "%u\n", &(_Materials.uiMaterialPairsSize));
	SAFE_NEW_ARRAY(_Materials.pMaterialPairs, TMaterialPairInfo, _Materials.uiMaterialPairsSize, "CResourceManager::LoadMaterials()");
	for(i=0; i<_Materials.uiMaterialPairsSize; i++)
	{
		fscanf_s(fr, "%s", tmpStr1, MAX_FILENAME_SIZE);
		fscanf_s(fr, "%s", tmpStr2, MAX_FILENAME_SIZE);
		CAtlString tmpStr;

		// if both are in materials
		found_1 = found_2 = false;
		for(j=0; j<_Materials.uiMaterialIDsCount; j++)
		{
			tmpStr = tmpStr1;
			if(tmpStr.MakeLower() == sMaterialNames[j].MakeLower())
			{
				_Materials.pMaterialPairs[i].uiMaterialA = j; found_1 = true;
			}
			tmpStr = tmpStr2;
			if(tmpStr.MakeLower() == sMaterialNames[j].MakeLower())
			{
				_Materials.pMaterialPairs[i].uiMaterialB = j; found_2 = true;
			}
		}	
		if(!(found_1 && found_2))
		{
			fclose(fr);
			SAFE_DELETE_ARRAY(sMaterialNames);
			SAFE_DELETE_ARRAY(_Materials.pMaterialPairs);
			HE(ERRFILECORRUPTED, "CResourceManager:: LoadMaterials() - not found", sFileName);
		}
		n = fscanf_s(fr, "%f %f %f %f %u\n",
				&(_Materials.pMaterialPairs[i].coefs.fSoftness),
				&(_Materials.pMaterialPairs[i].coefs.fElasticity),
				&(_Materials.pMaterialPairs[i].coefs.fStaticFriction),
				&(_Materials.pMaterialPairs[i].coefs.fKineticFriction),
				&(_Materials.pMaterialPairs[i].coefs.iCollidable)
				);
		if(n < 5)
		{
			fclose(fr);
			SAFE_DELETE_ARRAY(sMaterialNames);
			//SAFE_DELETE_ARRAY(_Materials.puiType);
			SAFE_DELETE_ARRAY(_Materials.pMaterialPairs);
			HE(ERRFILECORRUPTED, "CResourceManager::LoadMaterials() - less than five", sFileName);
		}

		// load sound effect name
		fscanf_s(fr, "%s", tmpStr1, MAX_FILENAME_SIZE);
		_Materials.pMaterialPairs[i].soundEffect = tmpStr1;
	}
	rcX = CResourceContainer(this, sResourceName, _Materials);
	fclose(fr);

	SAFE_DELETE_ARRAY(sMaterialNames);

	return RM_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Loads TSMRemapTable from sFileName - PRIVATE.
///////////////////////////////////////////////////////////////////////////////
LOAD_RESOURCE_HEADER(TSMRemapTable) {
    TSMRemapTable _TSMRemapTable;
	FILE * fr;
    _TSMRemapTable.pfTable = new float[181];
    if (!_TSMRemapTable.pfTable) HE(ERROUTOFMEMORY, "CResourceManager::LoadTSMRemapTable()", "Out of memory while allocating pfTable");
	
    if (fopen_s(&fr, sFileName, "r")) HE(ERRFILENOTFOUND, "CResourceManager::LoadTSMRemapTable()", sFileName);
	for (int i = 1; i < 180; i++) {
        fscanf_s(fr, "%f\n", &(_TSMRemapTable.pfTable[i]));
		if (ferror(fr)) HE(ERRGENERIC, "CResourceManager::LoadTSMRemapTable()", CAtlString("Error while reading file ") + sFileName);
	}

	_TSMRemapTable.pfTable[0] = _TSMRemapTable.pfTable[1];
	_TSMRemapTable.pfTable[180] = _TSMRemapTable.pfTable[179];

    fclose(fr);
    rcX = CResourceContainer(this, sResourceName, _TSMRemapTable);
    return RM_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Loads GrassItem from sFileName - PRIVATE. 
///////////////////////////////////////////////////////////////////////////////
LOAD_RESOURCE_HEADER(GrassItem) {
	FILE			*fr;
	char			str[MAX_FILENAME_SIZE];
	UINT 			lods, segs, strips;
	HRESULT			hr;
	GrassItem		_GrassItem;
	GrassVertex		*pVertex;
    int				iLoDCount = (pParam ? *((int *) pParam) : 0);
	int				iError = ERRGENERIC;
	
	
	ZeroMemory( &_GrassItem, sizeof(_GrassItem) );
	pVertex = NULL;
		
	if (fopen_s(&fr, sFileName, "rt")) HE(ERRFILENOTFOUND, "CResourceManager::LoadGrassItem()", sFileName);
		
    fgets(str, MAX_FILENAME_SIZE, fr);
    str[strlen(str) - 1] = 0;
	hr = fscanf_s(fr, "%lu\n", &lods);
	if (hr != 1) { 
        iError = ERRFILECORRUPTED; ErrorHandler.HandleError(iError, "CResourceManager::LoadGrassItem()", sFileName); 
        goto ErrorOccured;
    }

	if (lods < (UINT)iLoDCount) { 
        ErrorHandler.HandleError(ERRGENERIC, "CResourceManager::LoadGrassItem()", CAtlString("Too many LODs requested (missing LOD in file '") + sFileName + CAtlString("').")); 
        goto ErrorOccured; 
    }
	
    lods = min(lods, (UINT)iLoDCount);	 

	// load all LODs
	for (UINT lodInd = 0; lodInd < lods; lodInd++) {
		hr = fscanf_s( fr, "%lu %lu\n", &segs, &strips );
		if (hr < 2) { 
            iError = ERRFILECORRUPTED; ErrorHandler.HandleError(iError, "CResourceManager::LoadGrassItem()", sFileName); 
            goto ErrorOccured; 
        }
			
		pVertex = new GrassVertex[(segs+1)*(strips+1)];
		if (!pVertex) { 
            iError = ERROUTOFMEMORY; ErrorHandler.HandleError(iError, "CResourceManager::LoadGrassItem()", "Unable to generate grass vertex data."); 
            goto ErrorOccured; 
        }

		// load vertices data
		for (UINT i = 0; i <= segs; i++)
			for (UINT j = 0; j <= strips; j++) {
				hr = fscanf_s(fr, "%f %f %f %f\n", &pVertex[i * (strips+1) + j].x, &pVertex[i * (strips+1) + j].y, 
								&pVertex[i * (strips+1) + j].u, &pVertex[i * (strips+1) + j].v);
				if (hr != 4) { 
                    iError = ERRFILECORRUPTED; ErrorHandler.HandleError(iError, "CResourceManager::LoadGrassItem()", sFileName); 
                    goto ErrorOccured; 
                }
			}
			
		_GrassItem.pLODs[lodInd].uiSegmentsCount = segs;
		_GrassItem.pLODs[lodInd].uiStripesCount = strips;
		_GrassItem.pLODs[lodInd].pVertices = pVertex;
	}
		
	_GrassItem.uiLODCount = lods;
	 _GrassItem.ridTexture = LoadResource(str, RES_Texture);
	if (_GrassItem.ridTexture < 0) { 
        ErrorHandler.HandleError(ERRGENERIC, "CResourceManager::LoadGrassItem()", "An error occured when loading grass texture."); 
        goto ErrorOccured; 
    }
    pvInheritedResources->push_back(_GrassItem.ridTexture);

	fclose(fr);

    rcX = CResourceContainer(this, sResourceName, _GrassItem);

	return RM_OK;

ErrorOccured:
	if (fr) fclose(fr);
	delete[] pVertex;
	return iError;
}

///////////////////////////////////////////////////////////////////////////////
// Loads LightObject from sFileName - PRIVATE. 
///////////////////////////////////////////////////////////////////////////////
typedef struct __LightObjectLoadParams {
    LightObject * pLightObject;
    CResourceManager * pRM;
    bool bLoadInheritedResources;
    RVec * pvInheritedResources;
} LightObjectLoadParams;

void CALLBACK LightObjectLoadCallBack(NAMEVALUEPAIR *nameValue){
	CAtlString name = nameValue->GetName(); name.MakeLower();
    LightObjectLoadParams * lolp = ((LightObjectLoadParams *) nameValue->ReservedPointer);
    LightObject * pLightObject = lolp->pLightObject;

	if ( name == _T("x"))		{ pLightObject->x = nameValue->GetFloat(); return; };
	if ( name == _T("y"))		{ pLightObject->y = nameValue->GetFloat(); return; }; 
	if ( name == _T("z"))		{ pLightObject->z = nameValue->GetFloat(); return; };
	if ( name == _T("direction.x"         )) {pLightObject->directionX   = nameValue->GetFloat();return; };
	if ( name == _T("direction.y"         )) {pLightObject->directionY   = nameValue->GetFloat();return; };
	if ( name == _T("direction.z"         )) {pLightObject->directionZ   = nameValue->GetFloat();return; };
	if ( name == _T("scale"         ))		 {pLightObject->Scale   = nameValue->GetFloat();return; };
	if ( name == _T("attenuationconstant" )) {pLightObject->Attenuation0 = nameValue->GetFloat();return; };
	if ( name == _T("attenuationlinear"   )) {pLightObject->Attenuation1 = nameValue->GetFloat();return; };
	if ( name == _T("attenuationquadratic")) {pLightObject->Attenuation2 = nameValue->GetFloat();return; };
	if ( name == _T("outerconeangle")) {pLightObject->Phi      = nameValue->GetFloat();return; };
	if ( name == _T("innerconeangle")) {pLightObject->Theta    = nameValue->GetFloat();return; };
	if ( name == _T("falloff"       )) {pLightObject->Falloff  = nameValue->GetFloat();return; };
	if ( name == _T("range"         )) {pLightObject->Range    = nameValue->GetFloat();return; };
	if ( name == _T("diffuse"       )) {pLightObject->Diffuse  = nameValue->GetInt();  return; };
	if ( name == _T("specular"      )) {pLightObject->Specular = nameValue->GetInt();  return; };
	if ( name == _T("ambient"       )) {pLightObject->Ambient  = nameValue->GetInt();  return; };
	if ( name == _T("type"          )) {pLightObject->type     = nameValue->GetInt();  return; };
    if ( name == _T("lightmap"      ) && lolp->bLoadInheritedResources) {
        pLightObject->ridLightMap = lolp->pRM->LoadResource(nameValue->GetString(), RES_Texture, true, NULL);
        if (pLightObject->ridLightMap > 0) 
            lolp->pvInheritedResources->push_back(pLightObject->ridLightMap);
        else pLightObject->ridLightMap = 0;
        return; 
    };
}

LOAD_RESOURCE_HEADER(LightObject) {
    HRESULT ret;
    LightObject _LightObject;
    LightObjectLoadParams lolp;
    _LightObject.x = _LightObject.y = _LightObject.z = 0;
	_LightObject.directionX = _LightObject.directionY = _LightObject.directionZ = 0;
	_LightObject.Attenuation0 = _LightObject.Attenuation1 = _LightObject.Attenuation2 = 0;
    _LightObject.Phi = _LightObject.Theta = _LightObject.Falloff = _LightObject.Range = 0;
    _LightObject.Diffuse = _LightObject.Specular = _LightObject.Ambient = 0;
    _LightObject.type = 0;
	_LightObject.ridLightMap = 0;

    lolp.bLoadInheritedResources = bLoadInheritedResources;
    lolp.pRM = this;
    lolp.pLightObject = & _LightObject;
    lolp.pvInheritedResources = pvInheritedResources;

    //NAMEVALUEPAIR::Load( sFileName, LightObjectLoadCallBack, NULL );
	if (ret = NAMEVALUEPAIR::Load( sFileName, LightObjectLoadCallBack, & lolp )) return ret;
    rcX = CResourceContainer(this, sResourceName, _LightObject);
    return RM_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Loads EngineObject from sFileName - PRIVATE. 
///////////////////////////////////////////////////////////////////////////////
typedef struct {
    int iEngineObjectReadState; // 0 = Searching for "Engine" label, 1 = reading "Engine" label, 2 = reading done
    EngineObject * pEngineObject;
} EngineObjectCallBackParams;

void CALLBACK EngineObjectLoadCallBack(NAMEVALUEPAIR *nameValue) {
    CAtlString value = nameValue->GetName(); value.MakeLower();
    EngineObjectCallBackParams * pParams = (EngineObjectCallBackParams *) nameValue->ReservedPointer;
    
    if (nameValue->GetFlags() & NVPTYPE_LABEL) 
        if (pParams->iEngineObjectReadState) pParams->iEngineObjectReadState = 2;
        else if (!value.CompareNoCase("Engine")) pParams->iEngineObjectReadState = 1;

    if (pParams->iEngineObjectReadState == 1 && (nameValue->GetFlags() & NVPTYPE_VARIABLE)) {
		if ( value == _T("gear_cont"))	{ pParams->pEngineObject->gear_count = nameValue->GetInt(); return; }
        if ( value == _T("gear_1") )	{ pParams->pEngineObject->gear_1 = nameValue->GetFloat(); return; }
        if ( value == _T("gear_2") )	{ pParams->pEngineObject->gear_2 = nameValue->GetFloat(); return; }
        if ( value == _T("gear_3") )	{ pParams->pEngineObject->gear_3 = nameValue->GetFloat(); return; }
        if ( value == _T("gear_4") )	{ pParams->pEngineObject->gear_4 = nameValue->GetFloat(); return; }
        if ( value == _T("gear_5") )	{ pParams->pEngineObject->gear_5 = nameValue->GetFloat(); return; }
        if ( value == _T("gear_6") )	{ pParams->pEngineObject->gear_6 = nameValue->GetFloat(); return; }
        if ( value == _T("gear_r") )	{ pParams->pEngineObject->gear_r = nameValue->GetFloat(); return; }
        if ( value == _T("diff_ratio")) { pParams->pEngineObject->diff_ratio = nameValue->GetFloat(); return; }
		if ( value == _T("efficiency")) { pParams->pEngineObject->efficiency = nameValue->GetFloat(); return; }

		if ( value == _T("curve_points_count") )	{ pParams->pEngineObject->curve_points_count = nameValue->GetInt(); return; }
		if ( value == _T("curve_point_1_rpm") )		{ pParams->pEngineObject->curve_point_1_rpm = nameValue->GetFloat(); return; }
		if ( value == _T("curve_point_1_torque") )	{ pParams->pEngineObject->curve_point_1_torque = nameValue->GetFloat(); return; }
		if ( value == _T("curve_point_2_rpm") )		{ pParams->pEngineObject->curve_point_2_rpm = nameValue->GetFloat(); return; }
		if ( value == _T("curve_point_2_torque") )	{ pParams->pEngineObject->curve_point_2_torque = nameValue->GetFloat(); return; }
		if ( value == _T("curve_point_3_rpm") )		{ pParams->pEngineObject->curve_point_3_rpm = nameValue->GetFloat(); return; }
		if ( value == _T("curve_point_3_torque") )	{ pParams->pEngineObject->curve_point_3_torque = nameValue->GetFloat(); return; }
		if ( value == _T("curve_point_4_rpm") )		{ pParams->pEngineObject->curve_point_4_rpm = nameValue->GetFloat(); return; }
		if ( value == _T("curve_point_4_torque") )	{ pParams->pEngineObject->curve_point_4_torque = nameValue->GetFloat(); return; }
		if ( value == _T("curve_point_5_rpm") )		{ pParams->pEngineObject->curve_point_5_rpm = nameValue->GetFloat(); return; }
		if ( value == _T("curve_point_5_torque") )	{ pParams->pEngineObject->curve_point_5_torque = nameValue->GetFloat(); return; }
		if ( value == _T("curve_point_6_rpm") )		{ pParams->pEngineObject->curve_point_6_rpm = nameValue->GetFloat(); return; }
		if ( value == _T("curve_point_6_torque") )	{ pParams->pEngineObject->curve_point_6_torque = nameValue->GetFloat(); return; }
		if ( value == _T("curve_point_7_rpm") )		{ pParams->pEngineObject->curve_point_7_rpm = nameValue->GetFloat(); return; }
		if ( value == _T("curve_point_7_torque") )	{ pParams->pEngineObject->curve_point_7_torque = nameValue->GetFloat(); return; }
		if ( value == _T("curve_point_8_rpm") )		{ pParams->pEngineObject->curve_point_8_rpm = nameValue->GetFloat(); return; }
		if ( value == _T("curve_point_8_torque") )	{ pParams->pEngineObject->curve_point_8_torque = nameValue->GetFloat(); return; }

		if ( value == _T("shift_up") )		{ pParams->pEngineObject->shift_up = nameValue->GetFloat(); return; }
		if ( value == _T("shift_down") )	{ pParams->pEngineObject->shift_down = nameValue->GetFloat(); return; }
	}		
}

LOAD_RESOURCE_HEADER(EngineObject) {
    HRESULT ret;
	EngineObject _EngineObject;
    EngineObjectCallBackParams eocbParams; eocbParams.pEngineObject = &_EngineObject; eocbParams.iEngineObjectReadState = 0;
	_EngineObject.gear_count = 7;
    _EngineObject.gear_1 = 2.66f;
	_EngineObject.gear_2 = 1.78f;
	_EngineObject.gear_3 = 1.30f;
	_EngineObject.gear_4 = 1.0f;
	_EngineObject.gear_5 = 0.74f;
	_EngineObject.gear_6 = 0.50f;
	_EngineObject.gear_r = 2.20f;
	_EngineObject.diff_ratio = 3.42f;
	_EngineObject.efficiency = 0.7f;

	_EngineObject.curve_points_count = 8;
	_EngineObject.curve_point_1_rpm = 0.0f;
	_EngineObject.curve_point_1_torque = 0.0f;
	_EngineObject.curve_point_2_rpm = 1000.0f;
	_EngineObject.curve_point_2_torque = 430.0f;
	_EngineObject.curve_point_3_rpm = 2000.0f;
	_EngineObject.curve_point_3_torque = 460.0f;
	_EngineObject.curve_point_4_rpm = 3000.0f;
	_EngineObject.curve_point_4_torque = 480.0f;
	_EngineObject.curve_point_5_rpm = 4000.0f;
	_EngineObject.curve_point_5_torque = 500.0f;
	_EngineObject.curve_point_6_rpm = 5000.0f;
	_EngineObject.curve_point_6_torque = 490.0f;
	_EngineObject.curve_point_7_rpm = 6000.0f;
	_EngineObject.curve_point_7_torque = 420.0f;
	_EngineObject.curve_point_8_rpm = 6001.0f;
	_EngineObject.curve_point_8_torque = 0.0f;

	_EngineObject.shift_up = 5500.0f;
	_EngineObject.shift_down = 1500.0f;
 
    if (ret = NAMEVALUEPAIR::Load( sFileName, EngineObjectLoadCallBack, &eocbParams)) return ret;
    rcX = CResourceContainer(this, sResourceName, _EngineObject);
    return RM_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Loads CarObject from sFileName - PRIVATE. 
///////////////////////////////////////////////////////////////////////////////
typedef struct __CarObjectCallBackParams {
    CarObject * pCarObject;
    CResourceManager * pRM;
    CAtlString sFileName;
    bool bLoadInheritedResources;
    int iWheelCount;
    RVec * pvInheritedResources;
} CarObjectCallBackParams;

void CALLBACK CarObjectLoadCallBack(NAMEVALUEPAIR *nameValue) {
    CarObjectCallBackParams * pParams = (CarObjectCallBackParams *) nameValue->ReservedPointer;
    GameObjectLoadParams golp;
	CAtlString sFmt, value = nameValue->GetName(); value.MakeLower();
    RESOURCEID _tmpID;
    if(nameValue->GetFlags() &  NVPTYPE_LABEL){
        if(!value.CompareNoCase("car")){
            golp.sLabelName = value;
            golp.iOrder = 1;
            pParams->pCarObject->ridCarObject = pParams->pRM->LoadResource(pParams->sFileName+"$car", RES_GameObject, pParams->bLoadInheritedResources, &golp);
            pParams->pvInheritedResources->push_back(pParams->pCarObject->ridCarObject);
        } else if(!value.CompareNoCase("wheel")) {
            golp.sLabelName = value;
            golp.iOrder= ++(pParams->iWheelCount);
            if (pParams->iWheelCount >= MAX_CAR_WHEELS) return;
            sFmt.Format("$wheel=%i", golp.iOrder);
            _tmpID = pParams->pRM->LoadResource(pParams->sFileName + sFmt, RES_GameObject, pParams->bLoadInheritedResources, &golp);
            pParams->pvInheritedResources->push_back(_tmpID);
            pParams->pCarObject->pvWheelIDs->push_back(_tmpID);
		} else if(!value.CollateNoCase("engine")){
            pParams->pCarObject->ridEngine = pParams->pRM->LoadResource(pParams->sFileName+"$engine", RES_EngineObject);
            pParams->pvInheritedResources->push_back(pParams->pCarObject->ridEngine);
		}
	}
}

LOAD_RESOURCE_HEADER(CarObject) {
    HRESULT ret;
    CarObject _CarObject;  _CarObject.ridEngine = _CarObject.ridCarObject = -1; _CarObject.pvWheelIDs = new RVec;
    CarObjectCallBackParams cocbParams; cocbParams.pCarObject = &_CarObject; cocbParams.pRM = this; 
    cocbParams.sFileName = sFileName;
    cocbParams.sFileName.Delete(0, ResourcePath[RES_CarObject].GetLength());
    cocbParams.iWheelCount = 0;
    cocbParams.bLoadInheritedResources = bLoadInheritedResources;
    cocbParams.pvInheritedResources = pvInheritedResources;
    if (ret = NAMEVALUEPAIR::Load( sFileName, CarObjectLoadCallBack, &cocbParams)) return ret;
    rcX = CResourceContainer(this, sResourceName, _CarObject);
    return RM_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Loads GameObject from sFileName - PRIVATE. 
///////////////////////////////////////////////////////////////////////////////
typedef struct __GameObjectCallBackParams {
    GameObject * pGameObject;
    bool bLoadInheritedResources;
    CResourceManager * pRM;
    CAtlString sLabelName;
    int iOrder, iGameObjectReadState; // 0 = Searching for label, N > 0 = Nth label found
    RVec * pvInheritedResources;
} GameObjectCallBackParams;


void CALLBACK GameObjectLoadCallBack(NAMEVALUEPAIR *nameValue) {
    GameObjectCallBackParams * pParams = (GameObjectCallBackParams *) nameValue->ReservedPointer;
	CAtlString value = nameValue->GetName(); value.MakeLower();
    RESOURCEID _tmpID;
    CAtlString _tmpFileName;
 
    if (!pParams->iOrder) return;
    if (nameValue->GetFlags() & NVPTYPE_LABEL) {
        if (pParams->iGameObjectReadState == pParams->iOrder || !value.CompareNoCase(pParams->sLabelName)) 
            pParams->iGameObjectReadState++;
    }

    if (pParams->iGameObjectReadState == pParams->iOrder && (nameValue->GetFlags() & NVPTYPE_VARIABLE)) {
		if ( value == _T("sizex"			)) { pParams->pGameObject->sizeX = nameValue->GetInt();					return;}
		if ( value == _T("sizey"			)) { pParams->pGameObject->sizeY = nameValue->GetInt();					return;} 
		if ( value == _T("model"			)) {*pParams->pGameObject->psModel=nameValue->GetString();				return;}
	    if ( value == _T("x"				)) { pParams->pGameObject->x = nameValue->GetFloat();					return;}
		if ( value == _T("y"				)) { pParams->pGameObject->y = nameValue->GetFloat();					return;} 
		if ( value == _T("z"				)) { pParams->pGameObject->z = nameValue->GetFloat();					return;}
        if ( value == _T("posx"				)) { pParams->pGameObject->posX = nameValue->GetInt();					return;}
		if ( value == _T("posy"				)) { pParams->pGameObject->posY = nameValue->GetInt();					return;} 
		if ( value == _T("scale.x"			)) { pParams->pGameObject->scaleX = nameValue->GetFloat();				return;} 
		if ( value == _T("scale.y"			)) { pParams->pGameObject->scaleY = nameValue->GetFloat();				return;}
		if ( value == _T("scale.z"			)) { pParams->pGameObject->scaleZ = nameValue->GetFloat();				return;} 
		if ( value == _T("rotation.x"		)) { pParams->pGameObject->rotationX = nameValue->GetFloat();			return;}
		if ( value == _T("rotation.y"		)) { pParams->pGameObject->rotationY = nameValue->GetFloat();			return;}
		if ( value == _T("rotation.z"		)) { pParams->pGameObject->rotationZ = nameValue->GetFloat();			return;}
		if ( value == _T("cog.x"			)) { pParams->pGameObject->cogX = nameValue->GetFloat();				return;}
		if ( value == _T("cog.y"			)) { pParams->pGameObject->cogY = nameValue->GetFloat();				return;}
		if ( value == _T("cog.z"			)) { pParams->pGameObject->cogZ = nameValue->GetFloat();				return;}
		if ( value == _T("moi.x"			)) { pParams->pGameObject->moiX = nameValue->GetFloat();				return;}
		if ( value == _T("moi.y"			)) { pParams->pGameObject->moiY = nameValue->GetFloat();				return;}
		if ( value == _T("moi.z"			)) { pParams->pGameObject->moiZ = nameValue->GetFloat();				return;}
		if ( value == _T("mass"				)) { pParams->pGameObject->mass = nameValue->GetFloat();				return;}
		if ( value == _T("rotated"			)) { pParams->pGameObject->bRotated = (0 != nameValue->GetInt());		return;}
		if ( value == _T("powered"			)) { pParams->pGameObject->bPowered = (0 != nameValue->GetInt());		return;}
		if ( value == _T("static"			)) { pParams->pGameObject->isStatic = (0 != nameValue->GetInt());		return;}
		if ( value == _T("material"			)) { pParams->pGameObject->material = nameValue->GetInt();				return;}
		if ( value == _T("wheel_material"	)) { pParams->pGameObject->wheel_material = nameValue->GetInt();		return;}
		if ( value == _T("grassallowed"		)) { pParams->pGameObject->bGrassAllowed = (0 != nameValue->GetInt());  return;}
		if ( value == _T("suspension_shock"	)) { pParams->pGameObject->suspensionShock = nameValue->GetFloat();		return;}
		if ( value == _T("suspension_spring")) { pParams->pGameObject->suspensionSpring = nameValue->GetFloat();	return;}
		if ( value == _T("suspension_length")) { pParams->pGameObject->suspensionLength = nameValue->GetFloat();	return;}
		if ( value == _T("suspension_coef"	)) { pParams->pGameObject->suspensionCoef = nameValue->GetFloat();		return;}
		
        if ( value == _T("texture") && pParams->bLoadInheritedResources) { 
            _tmpFileName = nameValue->GetString();
            if (_tmpFileName != "" && pParams->bLoadInheritedResources) {
                _tmpID = (pParams->bLoadInheritedResources) ? pParams->pRM->LoadResource(_tmpFileName, RES_Texture) : -1;
            } else _tmpID = -1;
            if (_tmpID != -1) pParams->pvInheritedResources->push_back(_tmpID);
            pParams->pGameObject->pvTextureFileNames->push_back(_tmpFileName);
            pParams->pGameObject->pvTextureIDs->push_back(_tmpID);
            return;
        }

		if ( value == _T("phtexture") && pParams->bLoadInheritedResources) { 
            _tmpFileName = nameValue->GetString();
            if (_tmpFileName != "" && pParams->bLoadInheritedResources) {              
                _tmpID = pParams->pRM->LoadResource(_tmpFileName, RES_PhysicalTexture);
                if (_tmpID < 0) _tmpID = -1;
            } else _tmpID = -1;
            if (_tmpID > 0) pParams->pvInheritedResources->push_back(_tmpID);
            pParams->pGameObject->pvPhTextureFileNames->push_back(_tmpFileName);
            pParams->pGameObject->pvPhTextureIDs->push_back(_tmpID);
            return;
        }


		if ( value == _T("light")) { 
            _tmpFileName = nameValue->GetString(); 
            if (_tmpFileName != "" && pParams->bLoadInheritedResources) {              
                _tmpID = pParams->pRM->LoadResource(_tmpFileName, RES_LightObject);
                if (_tmpID < 0) _tmpID = -1;
            } else _tmpID = -1;
            if (_tmpID > 0) {
                pParams->pvInheritedResources->push_back(_tmpID);
                pParams->pGameObject->pvLightObjectIDs->push_back(_tmpID);
                pParams->pGameObject->pvLightObjectFileNames->push_back(_tmpFileName);
            }
        }
	}
}

LOAD_RESOURCE_HEADER(GameObject) {
    HRESULT ret;
    GameObject _GameObject;
    GameObjectCallBackParams gocbParams; 
    GameObjectLoadParams * golp;

    if (!pParam) return RM_INVALID_RESOURCE_TYPE;
    golp = (GameObjectLoadParams *) pParam;

    gocbParams.sLabelName = golp->sLabelName.MakeLower();
    gocbParams.pGameObject = &_GameObject; gocbParams.pRM = this; 
    gocbParams.iOrder = golp->iOrder;
	gocbParams.iGameObjectReadState = 0;
    gocbParams.bLoadInheritedResources = bLoadInheritedResources;
    gocbParams.pvInheritedResources = pvInheritedResources;

	_GameObject.posX = _GameObject.posY = _GameObject.material = _GameObject.wheel_material = 0;
    _GameObject.x = _GameObject.y = _GameObject.z = \
    _GameObject.rotationX = _GameObject.rotationY = _GameObject.rotationZ = \
    _GameObject.moiX = _GameObject.moiY = _GameObject.moiZ = \
	_GameObject.cogX = _GameObject.cogY = _GameObject.cogZ = _GameObject.mass = \
	_GameObject.suspensionShock = _GameObject.suspensionSpring = _GameObject.suspensionLength = _GameObject.suspensionCoef = 0.0f;

	_GameObject.sizeX = _GameObject.sizeY = 1;
    _GameObject.scaleX = _GameObject.scaleY = _GameObject.scaleZ = 1.0;
	_GameObject.bRotated = _GameObject.bPowered = _GameObject.isStatic = _GameObject.bGrassAllowed = false;
					
    _GameObject.psModel   = new CAtlString;
    _GameObject.pvLightObjectIDs = new RVec;
    _GameObject.pvTextureIDs     = new RVec;
    _GameObject.pvPhTextureIDs   = new RVec;
	_GameObject.pvTextureFileNames   = new std::vector<CAtlString>;
    _GameObject.pvPhTextureFileNames = new std::vector<CAtlString>;
    _GameObject.pvLightObjectFileNames = new std::vector<CAtlString>;

    if (ret = NAMEVALUEPAIR::Load( sFileName, GameObjectLoadCallBack, &gocbParams)) {
        delete _GameObject.psModel;
        delete _GameObject.pvLightObjectIDs;
        delete _GameObject.pvTextureIDs;
        delete _GameObject.pvPhTextureIDs;
	    delete _GameObject.pvTextureFileNames;
        delete _GameObject.pvPhTextureFileNames;
        delete _GameObject.pvLightObjectFileNames;
        return ret;
    }
    rcX = CResourceContainer(this, sResourceName, _GameObject);
    return RM_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Loads MainObject from sFileName - PRIVATE. 
///////////////////////////////////////////////////////////////////////////////
typedef struct __MainObjectCallBackParams {
    MainObject * pMainObject;
    int iRoadCount;
    int iObjectCount;
    int iWaypointsCount;
    CResourceManager * pRM;
    CAtlString sFileName;
    RVec * pvInheritedResources;
    bool bLoadInheritedResources;
} MainObjectCallBackParams;

void CALLBACK MainObjectLoadCallBack(NAMEVALUEPAIR * nameValue) {
    MainObjectCallBackParams * pParams = (MainObjectCallBackParams *) nameValue->ReservedPointer;
    GameObjectLoadParams golp;
	CAtlString sFmt, value = nameValue->GetName(); value.MakeLower();
    RESOURCEID _tmpID;
    if(nameValue->GetFlags() &  NVPTYPE_LABEL){
        if(!value.CompareNoCase("road")) {
            golp.sLabelName = value;
            golp.iOrder = ++(pParams->iRoadCount);
            sFmt.Format("$road=%i", pParams->iRoadCount);
            _tmpID = pParams->pRM->LoadResource(pParams->sFileName + sFmt, RES_GameObject, pParams->bLoadInheritedResources, &golp);
            pParams->pMainObject->pvRoadIDs->push_back(_tmpID);
            pParams->pvInheritedResources->push_back(_tmpID);
        } else if(!value.CompareNoCase("object")) {
            golp.sLabelName = value;
            golp.iOrder = ++(pParams->iObjectCount);
            sFmt.Format("$object=%i", pParams->iObjectCount);
            _tmpID = pParams->pRM->LoadResource(pParams->sFileName + sFmt, RES_GameObject, pParams->bLoadInheritedResources, &golp);
            pParams->pvInheritedResources->push_back(_tmpID);
            pParams->pMainObject->pvObjectIDs->push_back(_tmpID);
        } else if (!value.CompareNoCase("waypoints")) {
           pParams->iWaypointsCount++;
           sFmt.Format("$waypoints=%i", pParams->iWaypointsCount);
           _tmpID = pParams->pRM->LoadResource(pParams->sFileName + sFmt, RES_Waypoints, pParams->bLoadInheritedResources, &(pParams->iWaypointsCount));
           pParams->pvInheritedResources->push_back(_tmpID);
           pParams->pMainObject->pvWaypointsIDs->push_back(_tmpID);
        }
    } else if (nameValue->GetFlags() & NVPTYPE_VARIABLE) {
        if (!value.CompareNoCase("name"))
			*(pParams->pMainObject->sName) = nameValue->GetString();
        else if (!value.CompareNoCase("icon"))
            *(pParams->pMainObject->sIconName) = nameValue->GetString();
        else if (!value.CompareNoCase("hascheckpoint"))
            pParams->pMainObject->bHasCheckpoint = (0 != nameValue->GetInt());
        else if (!value.CompareNoCase("startingposition"))
            pParams->pMainObject->bStartingPosition = (0 != nameValue->GetInt());

    }
}

LOAD_RESOURCE_HEADER(MainObject) {
    HRESULT ret;
    MainObject _MainObject;  
    MainObjectCallBackParams mocbParams;
    _MainObject.pvRoadIDs   = new RVec;
    _MainObject.pvObjectIDs = new RVec;
    _MainObject.pvWaypointsIDs = new RVec;
    _MainObject.sName = new CAtlString;
    _MainObject.sIconName = new CAtlString;
    _MainObject.bHasCheckpoint = false;
    _MainObject.bStartingPosition = false;
    mocbParams.iObjectCount = mocbParams.iRoadCount = mocbParams.iWaypointsCount = 0;
    mocbParams.pMainObject = &_MainObject;
    mocbParams.pRM = this; mocbParams.sFileName = sFileName;
	mocbParams.sFileName.Delete(0, ResourcePath[RES_MainObject].GetLength());
    mocbParams.pvInheritedResources = pvInheritedResources;
    mocbParams.bLoadInheritedResources = bLoadInheritedResources;
       
    if (ret = NAMEVALUEPAIR::Load( sFileName, MainObjectLoadCallBack, &mocbParams)) {
        delete _MainObject.pvRoadIDs;
        delete _MainObject.pvObjectIDs;
        delete _MainObject.pvWaypointsIDs;
        delete _MainObject.sName;
        delete _MainObject.sIconName;
        return ret;
    }
    rcX = CResourceContainer(this, sResourceName, _MainObject);
    return RM_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Loads Map from sFileName - PRIVATE. 
///////////////////////////////////////////////////////////////////////////////
LOAD_RESOURCE_HEADER(Map) {
    Map _Map;
    char str[MAX_FILENAME_SIZE];
	const int	iLongStrLen = 10000;
	char strlong[iLongStrLen + 2];
    FILE *fr;
    int i, c;
	CAtlString			cstr, cstr2, csPair, csName, csValue;
	WORD				wMajor, wMinor;
	CBase64Coder		B64Coder;
	PlacedObjectMODData	MODData;
	int					semicInd, commaInd, colonInd, equalsInd;
	NAMEVALUEPAIR		NVPair;

	
	if (fopen_s(&fr, sFileName, "rt")) HE(ERRFILENOTFOUND, "CResourceManager::LoadMap() - fopen", sFileName);

    
	////////////////////////////////////////
	//LOAD Map Size, Map name, Format version
	////////////////////////////////////////
	
    if (2 != fscanf_s(fr,"%u %u\n", &(_Map.uiSegsCountX), &(_Map.uiSegsCountZ))) { 
        fclose(fr); 
        HE(ERRFILECORRUPTED, "CResourceManager::LoadMap() - uiSegsCount", sFileName);
    } 
	if (!fgets( str, 256, fr)) {
        fclose(fr); 
        HE(ERRFILECORRUPTED, "CResourceManager::LoadMap() - sMapName", sFileName); 
    }
	if (2 != fscanf_s(fr,"%hu.%hu", &wMajor, &wMinor)) { 
        fclose(fr); 
        HE(ERRFILECORRUPTED, "CResourceManager::LoadMap() - version", sFileName);
    } 
	
	_Map.dwVersion = (((DWORD)wMajor) << 16) + wMinor;

	if ( _Map.dwVersion != MAP_FILE_FORMAT_VERSION ) { 
        fclose(fr); 
		HE(ERRINCOMPATIBLE, "CResourceManager::LoadMap()", "Unsupported map file format (wrong version).");
    } 

	_Map.sMapName = new CAtlString(str);
	_Map.sMapName->Trim("\t \n");
	*(_Map.sMapName) = _Map.sMapName->Left( MAX_MAP_NAME_LENGTH );
	fgets( strlong, iLongStrLen, fr);

	////////////////////////////////////////
	//LOAD Map description
	////////////////////////////////////////
	if (!fgets( strlong, iLongStrLen, fr)) {
        fclose(fr);
		delete _Map.sMapName;
        HE(ERRFILECORRUPTED, "CResourceManager::LoadMap() - sMapDescription", sFileName); 
    }
	_Map.sMapDescription = new CAtlString();
	*(_Map.sMapDescription) = strlong;
	_Map.sMapDescription->TrimRight( "\n\t" );

	////////////////////////////////////////
	//LOAD MOD list
	////////////////////////////////////////
	if (!fgets( strlong, iLongStrLen, fr)) {
        fclose(fr);
		delete _Map.sMapName;
		delete _Map.sMapDescription;
        HE(ERRFILECORRUPTED, "CResourceManager::LoadMap() - pvMODList", sFileName); 
    }
	cstr = strlong;
	cstr += ",";

	_Map.pvSupportedModules = new std::vector<CAtlString>;
	while ( (c = cstr.Find( ',' )) >= 0 )
	{
		cstr2 = cstr.Left( c );
		cstr2 = cstr2.Trim("\n \t").MakeUpper();
		cstr = cstr.Right( cstr.GetLength() - c - 1 );
		if ( cstr2.GetLength() ) _Map.pvSupportedModules->push_back( cstr2 );
	}

	////////////////////////////////////////
	//LOAD Map image
	////////////////////////////////////////
	if (!fgets( str, 256, fr)) {
        fclose(fr); 
        HE(ERRFILECORRUPTED, "CResourceManager::LoadMap() - image name", sFileName); 
    }

	_Map.sMapImageFile = new CAtlString(str);
	_Map.sMapImageFile->Trim("\n \t");

	/////////////////////////////
	//LOAD TERRAIN             
	/////////////////////////////
	
    if (!fgets( str, MAX_FILENAME_SIZE, fr)) {
        fclose(fr);
        delete _Map.sMapName;
		delete _Map.sMapDescription;
		delete _Map.pvSupportedModules;
		delete _Map.sMapImageFile;
        HE(ERRFILECORRUPTED, "CResourceManager::LoadMap() - sTerrainName", sFileName);
    }
    _Map.sTerrainName = new CAtlString(str);
	_Map.sTerrainName->Trim("\n \t");
	
    /////////////////////////////
	//LOAD SKYBOX
	/////////////////////////////

    if(!fgets(str, MAX_FILENAME_SIZE,fr)) {
        fclose(fr);
        delete _Map.sMapName;
		delete _Map.sMapDescription;
		delete _Map.pvSupportedModules;
		delete _Map.sMapImageFile;
        delete _Map.sTerrainName;
        HE(ERRFILECORRUPTED, "CResourceManager::LoadMap() - sSkyBoxName",sFileName);
    }
    _Map.sSkyboxName = new CAtlString(str);
	_Map.sSkyboxName->Trim("\n \t");


	if(!fscanf_s(fr, "%f", &(_Map.fDayTime))) { 
        fclose(fr); 
        delete _Map.sMapName;
		delete _Map.sMapDescription;
		delete _Map.pvSupportedModules;
		delete _Map.sMapImageFile;
        delete _Map.sTerrainName;
        delete _Map.sSkyboxName;
        HE(ERRFILECORRUPTED, "CResourceManager::LoadMap() - fDayTime", sFileName);
    } 

    /////////////////////////////////////
	//LOADING AND SETTING AMBIENT LIGHT & FOG COLOR
	/////////////////////////////////////
	if(!fscanf_s(fr,"%x\n%x\n",&(_Map.uiColor),&(_Map.uiFogColor))) { 
        fclose(fr); 
        delete _Map.sMapName;
		delete _Map.sMapDescription;
		delete _Map.pvSupportedModules;
		delete _Map.sMapImageFile;
        delete _Map.sTerrainName;
        delete _Map.sSkyboxName;
        HE(ERRFILECORRUPTED, "CResourceManager::LoadMap() - uiColor/uiFogColor", sFileName);
    } 
	////////////////////////////////////
	// LOADING START POSITIONS
	////////////////////////////////////
	
    if (!fgets(str, MAX_FILENAME_SIZE, fr)) {
        delete _Map.sMapName;
		delete _Map.sMapDescription;
		delete _Map.pvSupportedModules;
		delete _Map.sMapImageFile;
        delete _Map.sTerrainName;
        delete _Map.sSkyboxName;
        HE(ERRFILECORRUPTED, "CResourceManager::LoadMap() - sStartPosition", sFileName);
    }

    _Map.sStartPositionFileName = new CAtlString(str);
    _Map.sStartPositionFileName->Trim("\n");
    if (bLoadInheritedResources) {
        _Map.ridStartPosition = LoadResource(*_Map.sStartPositionFileName,RES_MainObject, bLoadInheritedResources, NULL); 
        if (_Map.ridStartPosition < 0) {
            delete _Map.sMapName;
			delete _Map.sMapDescription;
			delete _Map.pvSupportedModules;
			delete _Map.sMapImageFile;
            delete _Map.sTerrainName;
            delete _Map.sSkyboxName;
            HE(ERRNOTFOUND, "CResourceManager:LoadMap() - LoadResource(sStartPosition)", sFileName);
        }
    } else _Map.ridStartPosition = -1;

    if (3 != fscanf_s(fr, "%i %i %i\n", &_Map.iStartPositionX, &_Map.iStartPositionZ, &_Map.iStartRotation)) {
            delete _Map.sMapName;
			delete _Map.sMapDescription;
			delete _Map.pvSupportedModules;
			delete _Map.sMapImageFile;
            delete _Map.sTerrainName;
            delete _Map.sSkyboxName;
            delete _Map.sStartPositionFileName;
            if (_Map.ridStartPosition != -1) ReleaseResource(_Map.ridStartPosition);
            delete[] _Map.psObjectNames;
            delete[] _Map.pridObjects;
            fclose(fr);
            HE(ERRFILECORRUPTED, "CResourceManager::LoadMap()", sFileName);
    }

	/////////////////////////////////
	// LOADING OBJETS
	/////////////////////////////////

	if(!fscanf_s(fr,"%u\n",&(_Map.uiObjectsCount))) { 
        fclose(fr); 
        HE(ERRFILECORRUPTED, "CResourceManager::LoadMap()", sFileName);
    }  //number of objects 
	
    _Map.psObjectNames = new CAtlString[_Map.uiObjectsCount];
    _Map.pridObjects = new RESOURCEID[_Map.uiObjectsCount];

    for( i = 0; i < (int) _Map.uiObjectsCount; i++) { //loadObject
		if(!fgets(str,256,fr)) {
            if (bLoadInheritedResources) 
                for (i--; i > -1; i--) 
                    ReleaseResource(_Map.pridObjects[i]);
            delete _Map.sMapName;
			delete _Map.sMapDescription;
			delete _Map.pvSupportedModules;
			delete _Map.sMapImageFile;
            delete _Map.sTerrainName;
            delete _Map.sSkyboxName;
            delete _Map.sStartPositionFileName;
            if (_Map.ridStartPosition != -1) ReleaseResource(_Map.ridStartPosition);
            delete[] _Map.psObjectNames;
            delete[] _Map.pridObjects;
            fclose(fr);
            HE(ERRFILECORRUPTED, "CResourceManager::LoadMap()", sFileName);
        }
        _Map.psObjectNames[i] = CAtlString(str);
		_Map.psObjectNames[i].Trim("\n \t");
		if (bLoadInheritedResources) {
            _Map.pridObjects[i] = LoadResource(_Map.psObjectNames[i], RES_MainObject, pParam);
            pvInheritedResources->push_back(_Map.pridObjects[i]);
        } else _Map.pridObjects[i] = -1;
    }

//	
//	////////////////////////////////////////
//	// LOADING OBJETS AND ROADS POSITIONS 
//	////////////////////////////////////////
//
	if(!fscanf_s(fr, "%u\n", &(_Map.uiPlacedObjectsCount))) { 
        for (i = _Map.uiObjectsCount; i > -1; i--) 
            ReleaseResource(_Map.pridObjects[i]);
        delete _Map.sMapName;
		delete _Map.sMapDescription;
		delete _Map.pvSupportedModules;
		delete _Map.sMapImageFile;
        delete _Map.sTerrainName;
        delete _Map.sSkyboxName;
        delete _Map.sStartPositionFileName;
        if (_Map.ridStartPosition != -1) ReleaseResource(_Map.ridStartPosition);
        delete[] _Map.psObjectNames;
        delete[] _Map.pridObjects;   
        fclose(fr); 
        HE(ERRFILECORRUPTED, "CResourceManager::LoadMap()", sFileName);
    }  //number of positions
    
    _Map.puiPlacedObjectIndexes      = new UINT[_Map.uiPlacedObjectsCount];
    _Map.puiPlacedObjectOrientations = new UINT[_Map.uiPlacedObjectsCount];
    _Map.piPlacedObjectX = new int[_Map.uiPlacedObjectsCount];
    _Map.piPlacedObjectZ = new int[_Map.uiPlacedObjectsCount];
	_Map.pPlacedObjectData = new OBJECTMODDATALIST[_Map.uiPlacedObjectsCount];
	_Map.pbPlacedObjectSpecial = new bool[_Map.uiPlacedObjectsCount];

	// read every object and parse the base64 encoded input data block if present
	for(i=0; i < (int) _Map.uiPlacedObjectsCount; i++) 
	{

		if (4 > fscanf_s(fr,"%u %i %i %u %10000s\n", _Map.puiPlacedObjectIndexes + i,  
													_Map.piPlacedObjectX + i, 
													_Map.piPlacedObjectZ + i,
													_Map.puiPlacedObjectOrientations + i,
													strlong, iLongStrLen + 1 )
                    ) goto ErrorLabel;


		_Map.pbPlacedObjectSpecial[i] = false;
		
		if ( strlen( strlong ) < 4 ) continue; // no MOD data, continue with next item

		// special object is one with some MOD data - which this IS
		_Map.pbPlacedObjectSpecial[i] = true;


		// init the buffer with input data and decode it
		if ( B64Coder.InitBuffer( (BYTE*) strlong, (UINT) strlen(strlong) ) ) 
			goto ErrorLabel;
		if ( B64Coder.Decode() ) 
			goto ErrorLabel;

		// parse the decoded string, one MOD at a time
		cstr.SetString( (char*) B64Coder.GetDecodedBuffer(), B64Coder.GetDecodedBufferSize() );
		cstr += ';';
		while ( (semicInd = cstr.Find( ';' )) != -1 )
		{
			// pick up all the data for this particular MOD
			cstr2 = cstr.Left( semicInd );
			if ( !cstr2.Trim("\n\t ").GetLength() )
			{ // empty item, skip it and continue parsing next one
				cstr = cstr.Right( cstr.GetLength() - semicInd - 1 );
				continue;
			}

			cstr2 += ',';

			// get the MOD ID
			if ( (colonInd = cstr2.Find( ':' )) == -1 ) 
				goto ErrorLabel; // MOD without name - not properly defined
			csName = cstr2.Left( colonInd ).Trim( " \n\t" ).MakeUpper();

			// check whether such a MOD exists in the list of map-supported MODs
			MODData.sModID = "";
			for ( UINT j = 0; j < _Map.pvSupportedModules->size(); j++ )
				if ( csName == (*_Map.pvSupportedModules)[j] ) 
					MODData.sModID = csName;

			if ( !MODData.sModID.GetLength() ) goto ErrorLabel;

			// remove already parsed data
			cstr2 = cstr2.Right( cstr2.GetLength() - colonInd - 1 );

			// parse each name=value pair, one item at a time
			MODData.vProperties.clear();
			while ( (commaInd = cstr2.Find( ',' )) != -1 )
			{
				csPair = cstr2.Left( commaInd ).Trim( "\n\t " );

				// remove already parsed data
				cstr2 = cstr2.Right( cstr2.GetLength() - commaInd - 1 );

				// skip empty fields
				if ( !csPair.GetLength() ) continue;

				if ( (equalsInd = csPair.Find( '=' )) != -1 )
				{ // proper name=value pair
					csName = csPair.Left( equalsInd ).Trim( "\n\t " );
					csValue = csPair.Right( csPair.GetLength() - equalsInd - 1 ).Trim( "\n\t " );
					// empty value is allowed, but not an empty name!			
					if ( !csName.GetLength() ) goto ErrorLabel;
					NVPair.SetName( csName );
					NVPair.SetString( csValue );
					// add the pair into our list
					MODData.vProperties.push_back( NVPair );
				}
				else
				{ // '=' not used; still it can be a 'flag' type property
					// empty name not allowed!
					if ( !csPair.GetLength() ) goto ErrorLabel;
					NVPair.SetName( csPair );
					// consider this no-value item as a SET flag
					NVPair.SetString( "1" );
					// add the pair into our list
					MODData.vProperties.push_back( NVPair );
				}
			} // pairs parsing
			
			// remove already parsed data
			cstr = cstr.Right( cstr.GetLength() - semicInd - 1 );

			// add the MOD data into list
			_Map.pPlacedObjectData[i].push_back( MODData );

		} // MODs parsing 
		
		// no error occured, continue with reading of next item
		continue; 


ErrorLabel: // jump here when any error occurs during the data read or parsing
        for (i = _Map.uiObjectsCount; i > -1; i--) 
            ReleaseResource(_Map.pridObjects[i]);
        delete _Map.sMapName;
		delete _Map.sMapDescription;
		delete _Map.pvSupportedModules;
		delete _Map.sMapImageFile;
        delete _Map.sTerrainName;
        delete _Map.sSkyboxName;
        delete[] _Map.psObjectNames;
        delete[] _Map.pridObjects;
        delete[] _Map.puiPlacedObjectIndexes;
        delete[] _Map.puiPlacedObjectOrientations;
        delete[] _Map.piPlacedObjectX;
        delete[] _Map.piPlacedObjectZ;
        delete[] _Map.pPlacedObjectData;
		delete[] _Map.pbPlacedObjectSpecial;
        delete _Map.sStartPositionFileName;
        if (_Map.ridStartPosition != -1) ReleaseResource(_Map.ridStartPosition);
        fclose(fr); 
        HE(ERRFILECORRUPTED,"CResourceManager::LoadMap()", sFileName);
	} //get positions

    if(!fscanf_s(fr, "%u\n", &(_Map.uiPathPointsCount))) { 
        for (i = _Map.uiObjectsCount; i > -1; i--) 
            ReleaseResource(_Map.pridObjects[i]);
        delete _Map.sMapName;
		delete _Map.sMapDescription;
		delete _Map.pvSupportedModules;
		delete _Map.sMapImageFile;
        delete _Map.sTerrainName;
        delete _Map.sSkyboxName;
        delete[] _Map.psObjectNames;
        delete[] _Map.pridObjects;
        delete[] _Map.puiPlacedObjectIndexes;
        delete[] _Map.puiPlacedObjectOrientations;
        delete[] _Map.piPlacedObjectX;
        delete[] _Map.piPlacedObjectZ;
        delete[] _Map.pPlacedObjectData;
		delete[] _Map.pbPlacedObjectSpecial;
        delete _Map.sStartPositionFileName;
        if (_Map.ridStartPosition != -1) ReleaseResource(_Map.ridStartPosition);
        fclose(fr); 
        HE(ERRFILECORRUPTED, "CResourceManager::LoadMap()", sFileName);
    }  //number of positions

    _Map.puiPathPointIndexes = new UINT[_Map.uiPathPointsCount];
    for (i = 0; i < (int) _Map.uiPathPointsCount; i++) 
        if (!fscanf_s(fr, "%u", _Map.puiPathPointIndexes + i)) {
            for (i = _Map.uiObjectsCount; i > -1; i--) 
                ReleaseResource(_Map.pridObjects[i]);
            delete _Map.sMapName;
			delete _Map.sMapDescription;
			delete _Map.pvSupportedModules;
			delete _Map.sMapImageFile;
            delete _Map.sTerrainName;
            delete _Map.sSkyboxName;
            delete[] _Map.psObjectNames;
            delete[] _Map.pridObjects;
            delete[] _Map.puiPlacedObjectIndexes;
            delete[] _Map.puiPlacedObjectOrientations;
            delete[] _Map.piPlacedObjectX;
            delete[] _Map.piPlacedObjectZ;
            delete[] _Map.pPlacedObjectData;
			delete[] _Map.pbPlacedObjectSpecial;
            delete _Map.sStartPositionFileName;
            if (_Map.ridStartPosition != -1) ReleaseResource(_Map.ridStartPosition);
            fclose(fr); 
            HE(ERRFILECORRUPTED, "CResourceManager::LoadMap()", sFileName);
        }
    fclose(fr);
    rcX = CResourceContainer(this, sResourceName, _Map);
    return RM_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Loads SkySystem from sFileName - PRIVATE. 
///////////////////////////////////////////////////////////////////////////////

// call back load function
void CALLBACK SkySystemLoadCallBack(NAMEVALUEPAIR * Pair)
{
	SkySystem * _SkySystem = (SkySystem *) Pair->ReservedPointer;

	CAtlString Name = Pair->GetName();

	Name.MakeUpper();
	
	// Molecule color
	if (Name == _T("MOLECULECOLOR.R") ) {_SkySystem->MoleculeColor->x = Pair->GetFloat(); return; }
	if (Name == _T("MOLECULECOLOR.G") ) {_SkySystem->MoleculeColor->y = Pair->GetFloat(); return; }
	if (Name == _T("MOLECULECOLOR.B") ) {_SkySystem->MoleculeColor->z = Pair->GetFloat(); return; }
	// Aerosol color
	if (Name == _T("AEROSOLCOLOR.R") ) {_SkySystem->AerosolColor->x = Pair->GetFloat(); return; }
	if (Name == _T("AEROSOLCOLOR.G") ) {_SkySystem->AerosolColor->y = Pair->GetFloat(); return; }
	if (Name == _T("AEROSOLCOLOR.B") ) {_SkySystem->AerosolColor->z = Pair->GetFloat(); return; }
	// Sun texture path and name
	if (Name == _T("SUNTEXTURE") ) {(*_SkySystem->SunTextureName) = Pair->GetString(); return; }
	// Sun colors for hours
	if (Name == _T("SUNCOLOR0.R") ) {_SkySystem->SunColors[0].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR0.G") ) {_SkySystem->SunColors[0].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR0.B") ) {_SkySystem->SunColors[0].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR1.R") ) {_SkySystem->SunColors[1].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR1.G") ) {_SkySystem->SunColors[1].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR1.B") ) {_SkySystem->SunColors[1].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR2.R") ) {_SkySystem->SunColors[2].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR2.G") ) {_SkySystem->SunColors[2].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR2.B") ) {_SkySystem->SunColors[2].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR3.R") ) {_SkySystem->SunColors[3].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR3.G") ) {_SkySystem->SunColors[3].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR3.B") ) {_SkySystem->SunColors[3].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR4.R") ) {_SkySystem->SunColors[4].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR4.G") ) {_SkySystem->SunColors[4].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR4.B") ) {_SkySystem->SunColors[4].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR5.R") ) {_SkySystem->SunColors[5].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR5.G") ) {_SkySystem->SunColors[5].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR5.B") ) {_SkySystem->SunColors[5].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR6.R") ) {_SkySystem->SunColors[6].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR6.G") ) {_SkySystem->SunColors[6].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR6.B") ) {_SkySystem->SunColors[6].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR7.R") ) {_SkySystem->SunColors[7].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR7.G") ) {_SkySystem->SunColors[7].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR7.B") ) {_SkySystem->SunColors[7].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR8.R") ) {_SkySystem->SunColors[8].x = Pair->GetFloat(); return; } 
	if (Name == _T("SUNCOLOR8.G") ) {_SkySystem->SunColors[8].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR8.B") ) {_SkySystem->SunColors[8].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR9.R") ) {_SkySystem->SunColors[9].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR9.G") ) {_SkySystem->SunColors[9].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR9.B") ) {_SkySystem->SunColors[9].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR10.R") ) {_SkySystem->SunColors[10].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR10.G") ) {_SkySystem->SunColors[10].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR10.B") ) {_SkySystem->SunColors[10].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR11.R") ) {_SkySystem->SunColors[11].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR11.G") ) {_SkySystem->SunColors[11].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR11.B") ) {_SkySystem->SunColors[11].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR12.R") ) {_SkySystem->SunColors[12].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR12.G") ) {_SkySystem->SunColors[12].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR12.B") ) {_SkySystem->SunColors[12].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR13.R") ) {_SkySystem->SunColors[13].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR13.G") ) {_SkySystem->SunColors[13].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR13.B") ) {_SkySystem->SunColors[13].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR14.R") ) {_SkySystem->SunColors[14].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR14.G") ) {_SkySystem->SunColors[14].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR14.B") ) {_SkySystem->SunColors[14].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR15.R") ) {_SkySystem->SunColors[15].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR15.G") ) {_SkySystem->SunColors[15].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR15.B") ) {_SkySystem->SunColors[15].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR16.R") ) {_SkySystem->SunColors[16].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR16.G") ) {_SkySystem->SunColors[16].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR16.B") ) {_SkySystem->SunColors[16].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR17.R") ) {_SkySystem->SunColors[17].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR17.G") ) {_SkySystem->SunColors[17].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR17.B") ) {_SkySystem->SunColors[17].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR18.R") ) {_SkySystem->SunColors[18].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR18.G") ) {_SkySystem->SunColors[18].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR18.B") ) {_SkySystem->SunColors[18].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR19.R") ) {_SkySystem->SunColors[19].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR19.G") ) {_SkySystem->SunColors[19].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR19.B") ) {_SkySystem->SunColors[19].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR20.R") ) {_SkySystem->SunColors[20].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR20.G") ) {_SkySystem->SunColors[20].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR20.B") ) {_SkySystem->SunColors[20].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR21.R") ) {_SkySystem->SunColors[21].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR21.G") ) {_SkySystem->SunColors[21].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR21.B") ) {_SkySystem->SunColors[21].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR22.R") ) {_SkySystem->SunColors[22].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR22.G") ) {_SkySystem->SunColors[22].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR22.B") ) {_SkySystem->SunColors[22].z = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR23.R") ) {_SkySystem->SunColors[23].x = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR23.G") ) {_SkySystem->SunColors[23].y = Pair->GetFloat(); return; }
	if (Name == _T("SUNCOLOR23.B") ) {_SkySystem->SunColors[23].z = Pair->GetFloat(); return; }
	// Mountain texture path and names
	if (Name == _T("MOUNTAINNORTH") ) {*(_SkySystem->MountainNorth) = Pair->GetString(); return; }
	if (Name == _T("MOUNTAINWEST") ) {*(_SkySystem->MountainWest) = Pair->GetString(); return; }
	if (Name == _T("MOUNTAINSOUTH") ) {*(_SkySystem->MountainSouth) = Pair->GetString(); return; }
	if (Name == _T("MOUNTAINEAST") ) {*(_SkySystem->MountainEast) = Pair->GetString(); return; }
	// clound texture path and name
	if (Name == _T("CLOUDTEXTURE") ) {*(_SkySystem->CloudTextureName) = Pair->GetString();  return; }
}

// check values from set by setting file
HRESULT SkySystemCheckValues(SkySystem & _SkySystem) {
    CAtlString sFmt;
	// check aerosol color values
	if (_SkySystem.AerosolColor->x < 0 || _SkySystem.AerosolColor->x > 1 || 
		_SkySystem.AerosolColor->y < 0 || _SkySystem.AerosolColor->y > 1 || 
		_SkySystem.AerosolColor->z < 0 || _SkySystem.AerosolColor->z > 1)
		HE(-1, "SkySystemCheckValues()", "Aerosol color either not loaded from sky system setting file or values out of permitted [0, 1] range.");

	// check molecule color values
	if (_SkySystem.MoleculeColor->x < 0 || _SkySystem.MoleculeColor->x > 1 || 
		_SkySystem.MoleculeColor->y < 0 || _SkySystem.MoleculeColor->y > 1 ||
		_SkySystem.MoleculeColor->z < 0 || _SkySystem.MoleculeColor->z > 1)
		HE(-1, "SkySystemCheckValues()", "Molecule color either not loaded from sky system setting file or values out of permitted [0, 1] range.");

	// check sun texture name
	if (*(_SkySystem.SunTextureName) == "")
		HE(-1, "SkySystemCheckValues()", "Sun texture name not specified in sky system setting file.");

	// check sun colors for sky system
	for (int i = 0; i < 24; i++)
		if (_SkySystem.SunColors[i].x < 0 || _SkySystem.SunColors[i].x > 1 ||
			_SkySystem.SunColors[i].y < 0 || _SkySystem.SunColors[i].y > 1 ||
			_SkySystem.SunColors[i].z < 0 || _SkySystem.SunColors[i].z > 1)
		{
            sFmt.Format("Sun color number %i either not loaded from sky system setting file or values out of permitted [0, 1] range.", i);
            HE(-1, "SkySystemCheckValues()", sFmt);
		}

	// check mountain texture names
	if (*(_SkySystem.MountainNorth) == "")
		HE(-1, "SkySystemCheckValues()", "Mountain north texture name not specified in sky system setting file.");

	if (*(_SkySystem.MountainWest) == "")
		HE(-1, "SkySystemCheckValues()", "Mountain west texture name not specified in sky system setting file.");

	if (*(_SkySystem.MountainSouth) == "")
		HE(-1, "SkySystemCheckValues()", "Mountain south texture name not specified in sky system setting file.");

	if (*(_SkySystem.MountainEast) == "")
		HE(-1, "SkySystemCheckValues()", "Mountain east texture name not specified in sky system setting file.");

	// check cloud texture name
	if (*(_SkySystem.CloudTextureName) == "")
		HE(-1, "SkySystemCheckValues()", "Cloud texture name not specified in sky system setting file.");

	return 0;
}

LOAD_RESOURCE_HEADER(SkySystem) {
	HRESULT Result;
    SkySystem _SkySystem;
	
	// Allocate strings and invalidate values
	_SkySystem.CloudTextureName = new CAtlString("");
	_SkySystem.MountainEast = new CAtlString("");
	_SkySystem.MountainNorth = new CAtlString("");
	_SkySystem.MountainSouth = new CAtlString("");
	_SkySystem.MountainWest = new CAtlString("");
	_SkySystem.SunTextureName = new CAtlString("");
	_SkySystem.AerosolColor = new D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
	_SkySystem.MoleculeColor = new D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
	_SkySystem.SunColors = new D3DXVECTOR3[24];
	
	for (UINT i = 0; i < 24; i++)
		_SkySystem.SunColors[i] = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);

	_SkySystem.CloudTextureID = -1;
	_SkySystem.MountainEastTextureID = -1;
	_SkySystem.MountainNorthTextureID = -1;
	_SkySystem.MountainSouthTextureID = -1;
	_SkySystem.MountainWestTextureID = -1;
	_SkySystem.SunTextureID = -1;

	Result = NAMEVALUEPAIR::Load(sFileName, SkySystemLoadCallBack, (void *) &_SkySystem);
	if (FAILED(Result) )
		HE(Result, "CResourceManager::LoadSkySystem()", CAtlString("Could not load sky system filename: ") + sFileName );
	
	Result = SkySystemCheckValues(_SkySystem);
	if (FAILED(Result) )
		HE(Result, "CResourceManager::LoadSkySystem()", CAtlString("Sky system filename: ") + sFileName + CAtlString(" is corrupted.") );

	_SkySystem.CloudTextureID = LoadResource(*_SkySystem.CloudTextureName, RES_Texture, NULL);
	if (_SkySystem.CloudTextureID < 0)
		HE(_SkySystem.CloudTextureID, "CResourceManager::LoadSkySystem()", CAtlString("Unable to load cloud texture filename: ") +
					(*_SkySystem.CloudTextureName) )
    else pvInheritedResources->push_back(_SkySystem.CloudTextureID);
	
	_SkySystem.MountainEastTextureID = LoadResource((*_SkySystem.MountainEast), RES_Texture, NULL);
	if (_SkySystem.MountainEastTextureID < 0)
		HE(_SkySystem.MountainEastTextureID, "CResourceManager::LoadSkySystem()", CAtlString("Unable to load mountain east texture filename: ") +
		(*_SkySystem.MountainEast) )
    else pvInheritedResources->push_back(_SkySystem.MountainEastTextureID);
	
	_SkySystem.MountainNorthTextureID = LoadResource((*_SkySystem.MountainNorth), RES_Texture, NULL);
	if (_SkySystem.MountainNorthTextureID < 0)
		HE(_SkySystem.MountainNorthTextureID, "CResourceManager::LoadSkySystem()", CAtlString("Unable to load mountain north texture filename: ") +
		(*_SkySystem.MountainNorth) )
    else pvInheritedResources->push_back(_SkySystem.MountainNorthTextureID);

	_SkySystem.MountainSouthTextureID = LoadResource((*_SkySystem.MountainSouth), RES_Texture, NULL);
	if (_SkySystem.MountainSouthTextureID < 0)
		HE(_SkySystem.MountainSouthTextureID, "CResourceManager::LoadSkySystem()", CAtlString("Unable to load mountain south texture filename: ") +
		(*_SkySystem.MountainSouth) )
    else pvInheritedResources->push_back(_SkySystem.MountainSouthTextureID);

	_SkySystem.MountainWestTextureID = LoadResource((*_SkySystem.MountainWest), RES_Texture, NULL);
	if (_SkySystem.MountainWestTextureID < 0)
		HE(_SkySystem.MountainWestTextureID, "CResourceManager::LoadSkySystem()", CAtlString("Unable to load mountain west texture filename: ") +
		(*_SkySystem.MountainWest) )
    else pvInheritedResources->push_back(_SkySystem.MountainWestTextureID);

	_SkySystem.SunTextureID = LoadResource((*_SkySystem.SunTextureName), RES_Texture, NULL);
	if (_SkySystem.SunTextureID < 0)
		HE(_SkySystem.SunTextureID, "CResourceManager::LoadSkySystem()", CAtlString("Unable to load sun texture filename: ") +
		(*_SkySystem.SunTextureName) )
    else pvInheritedResources->push_back(_SkySystem.SunTextureID);

    rcX = CResourceContainer(this, sResourceName, _SkySystem);
    return RM_OK;
}

typedef struct __WaypointsCallBackParams {
    int iOrder;
    int iWaypointsCount;
    Waypoints * pWaypoints;
    float x, y, z, min, max;
} WaypointsCallBackParams;

void CALLBACK WaypointsLoadCallBack(NAMEVALUEPAIR * nameValue) {
    WaypointsCallBackParams * pParams = (WaypointsCallBackParams *) nameValue->ReservedPointer;
	CAtlString value = nameValue->GetName(); value.MakeLower();
    if (!(pParams->iOrder)) return;
    if(nameValue->GetFlags() &  NVPTYPE_LABEL)
        if (!value.CompareNoCase("waypoints") || pParams->iOrder == pParams->iWaypointsCount)
           pParams->iWaypointsCount++;
    if (pParams->iOrder == pParams->iWaypointsCount && (nameValue->GetFlags() & NVPTYPE_VARIABLE)) {
        if ( value == _T("x")) { pParams->x = nameValue->GetFloat(); pParams->pWaypoints->x->push_back(pParams->x); return;}
        if ( value == _T("y")) { pParams->y = nameValue->GetFloat(); pParams->pWaypoints->y->push_back(pParams->y); return;}
        if ( value == _T("z")) { pParams->z = nameValue->GetFloat(); pParams->pWaypoints->z->push_back(pParams->z); return;}
        if ( value == _T("min_speed")) { pParams->min = nameValue->GetFloat(); pParams->pWaypoints->min_speed->push_back(pParams->min); return;}
        if ( value == _T("max_speed")) { pParams->max = nameValue->GetFloat(); pParams->pWaypoints->max_speed->push_back(pParams->max); return;}
    }
}

LOAD_RESOURCE_HEADER(Waypoints) {
    HRESULT ret;
    Waypoints _Waypoints;
    WaypointsCallBackParams wcp;
    size_t x, y, z, min_speed, max_speed, max_all;

    _Waypoints.x = new std::vector<float>;
    _Waypoints.y = new std::vector<float>;
    _Waypoints.z = new std::vector<float>;
    _Waypoints.min_speed = new std::vector<float>;
    _Waypoints.max_speed = new std::vector<float>;
    wcp.iOrder = *((int *) pParam);
    wcp.pWaypoints = &_Waypoints;
    wcp.iWaypointsCount = 0;
    wcp.x = 0.0f;
    wcp.y = 0.0f;
    wcp.z = 0.0f;
    wcp.min = 0.0f;
    wcp.max = 0.0f;

    if (ret = NAMEVALUEPAIR::Load(sFileName, WaypointsLoadCallBack, &wcp)) {
        delete _Waypoints.x;
        delete _Waypoints.y;
        delete _Waypoints.z;
        delete _Waypoints.min_speed;
        delete _Waypoints.max_speed;
        return ret; 
    }

    x = _Waypoints.x->size();
    y = _Waypoints.y->size();
    z = _Waypoints.z->size();
    min_speed = _Waypoints.min_speed->size();
    max_speed = _Waypoints.max_speed->size();

    max_all = max(x, max(y, max(z, max(min_speed, max_speed))));

#define VECTOR_INSERT(var, value) if (var != max_all) _Waypoints.var->insert(_Waypoints.var->end(), max_all - var, value)
    
    VECTOR_INSERT(x, wcp.x);
    VECTOR_INSERT(y, wcp.y);
    VECTOR_INSERT(z, wcp.z);
    VECTOR_INSERT(min_speed, wcp.min);
    VECTOR_INSERT(max_speed, wcp.max);

    rcX = CResourceContainer(this, sResourceName, _Waypoints);
    return RM_OK;
}

LOAD_RESOURCE_HEADER(TextureSet) {
    TextureSet _TextureSet;
    FILE * fr;
    char buffer[MAX_FILENAME_SIZE];
    UINT i;
    HRESULT res;

    if (fopen_s(&fr, sFileName, "r")) 
        HE(ERRFILENOTFOUND, "CResourceManager::LoadTextureSet()", sFileName);
    if (!fscanf_s(fr, "%u\n", &(_TextureSet.uiTextureCount)))
        HE(ERRFILECORRUPTED, "CResourceManager::LoadTextureSet()", sFileName);
	
	
	fgets(buffer, MAX_FILENAME_SIZE, fr);
    buffer[strlen(buffer) - 1] = 0;
	_TextureSet.psPhTexture             = new CAtlString(buffer); 
	_TextureSet.pTextureFileNames       = new CAtlString[_TextureSet.uiTextureCount];
    _TextureSet.pTextureIDs             = new RESOURCEID[_TextureSet.uiTextureCount];
    _TextureSet.pTextureProbabilities   = new int       [_TextureSet.uiTextureCount];
    if (!_TextureSet.pTextureFileNames  || !_TextureSet.pTextureProbabilities 
                                        || !_TextureSet.psPhTexture
                                        || !_TextureSet.pTextureIDs) 
    {
        if (_TextureSet.pTextureFileNames    ) delete[] _TextureSet.pTextureFileNames;
        if (_TextureSet.pTextureProbabilities) delete[] _TextureSet.pTextureProbabilities;
        if (_TextureSet.pTextureIDs          ) delete[] _TextureSet.pTextureIDs;
        if (_TextureSet.psPhTexture          ) delete   _TextureSet.psPhTexture;
		HE(ERROUTOFMEMORY, "CResourceManager::LoadTextureSet()", CAtlString("Out of memory!"));
    }
    for (i = 0; i < _TextureSet.uiTextureCount; i++) {
        fgets(buffer, MAX_FILENAME_SIZE, fr);
        buffer[strlen(buffer) - 1] = 0;
        _TextureSet.pTextureFileNames[i] = CAtlString(buffer);
        fscanf_s(fr, "%i\n", &(_TextureSet.pTextureProbabilities[i]));
    }
    
    for (i = 0; i < _TextureSet.uiTextureCount; i++) {
        res = bLoadInheritedResources ? LoadResource(_TextureSet.pTextureFileNames[i], RES_Texture, bLoadInheritedResources, NULL) : -1;
        if (res < 0) _TextureSet.pTextureIDs[i] = -1;
        else {
            _TextureSet.pTextureIDs[i] = res;
            pvInheritedResources->push_back(res);
        }
    }
    
    rcX = CResourceContainer(this, sResourceName, _TextureSet);
    return RM_OK;
}
