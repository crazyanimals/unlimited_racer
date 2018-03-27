#include "stdafx.h"
#include "ResourceManager.h"

using namespace resManNS;

#define CONSTRUCTOR_EXPANDER(TYPE) \
    CResourceManager::CResourceContainer::CResourceContainer(CResourceManager * Owner, CAtlString Name, TYPE _##TYPE) {\
        uiResType = RES_##TYPE; \
        bReleasable = false; \
        ResName = Name; \
        _ResourceData._##TYPE = _##TYPE; \
        pOwner = Owner;\
        iResNameHash = CResourceManager::ComputeStringHash(ResName);\
    }

///////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////
CResourceManager::CResourceContainer::CResourceContainer(CResourceManager * Owner) {
    uiResType = RES_NoType;
    bReleasable = false;
    ResName = "";
    iResNameHash = 0;
    pOwner = Owner;
}

CResourceManager::CResourceContainer::CResourceContainer(const CResourceContainer & rcX) {
    this->_ResourceData = rcX._ResourceData;
    this->bReleasable   = false;
    this->pOwner        = rcX.pOwner;
    this->ResName       = rcX.ResName;
    this->uiResType     = rcX.uiResType;
    this->iResNameHash  = rcX.iResNameHash;
}
    EXPAND(CONSTRUCTOR_EXPANDER)

///////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////

#define DESTRUCTOR(TYPE) \
    void Destructor(CResourceManager * pOwner, TYPE _##TYPE)

#define DESTRUCTOR_FORWARD_DECLARATOR(TYPE) \
    DESTRUCTOR(TYPE);
    
    EXPAND(DESTRUCTOR_FORWARD_DECLARATOR)

DESTRUCTOR(Texture) {
    SAFE_RELEASE(_Texture.texture);
}

DESTRUCTOR(XFile) {
    UINT i;
    for (i = 0; i < _XFile.cMtrl; i++)
        if (_XFile.pTextureIDs[i] != -1) pOwner->ReleaseResource(_XFile.pTextureIDs[i]);
    delete[] _XFile.pTextureIDs;
    delete[] _XFile.Material;
    SAFE_RELEASE(_XFile.Mesh)
}

DESTRUCTOR(Terrain) {
    UINT j;
    for (j = 0; j < _Terrain.uiTextureCount; j++)
        pOwner->ReleaseResource(_Terrain.pTextureIDs[j]);
    for (j = 0; j < _Terrain.uiPhysicalTextureCount; j++)
        pOwner->ReleaseResource(_Terrain.pPhysicalTextureIDs[j]);
    for (j = 0; j < 115; j++)
        if (_Terrain.pGrPlateIDs[j] != -1) pOwner->ReleaseResource(_Terrain.pGrPlateIDs[j]);
    if (_Terrain.ridWallTextureSet != -1) pOwner->ReleaseResource(_Terrain.ridWallTextureSet);
    delete[] _Terrain.pGrPlateIDs;
    delete[] _Terrain.pPhysicalTextureIDs;
    delete[] _Terrain.pTextureIDs;
    delete[] _Terrain.pTerrainPlateInfo;
    delete[] _Terrain.pGrassFileNames;
    delete[] _Terrain.pPhysicalTextureFileNames;
    delete[] _Terrain.pTextureFileNames;
	delete   _Terrain.sTextureSet;
    delete   _Terrain.sWallTextureSet;
}

DESTRUCTOR(GrSurface) {
    SAFE_RELEASE(_GrSurface.pIndices);
    SAFE_RELEASE(_GrSurface.pVertices);
}

DESTRUCTOR(VertexShader) {
    SAFE_RELEASE(_VertexShader._VS);
    SAFE_RELEASE(_VertexShader.ConstTable);
}

DESTRUCTOR(PixelShader) {
    SAFE_RELEASE(_PixelShader._PS);
    SAFE_RELEASE(_PixelShader.ConstTable);
}

DESTRUCTOR(Road) {
    UINT j;
    for (j = 0; j < _Road.uiTextureCount; j++)
        pOwner->ReleaseResource(_Road.pTextureIDs[j]);
    for (j = 0; j < _Road.uiPhysicalTextureCount; j++)
        pOwner->ReleaseResource(_Road.pPhysicalTextureIDs[j]);
    delete[] _Road.pTextureIDs;
    delete[] _Road.pPhysicalTextureIDs;
    delete[] _Road.pRoadPlateInfo;
}

DESTRUCTOR(GrPlate) {
    UINT i;
    for (i = 0; i < _GrPlate.uiLoDCount; i++)
        if (_GrPlate.LoD[i] != -1) pOwner->ReleaseResource(_GrPlate.LoD[i]);
}

DESTRUCTOR(PhysicalTexture) {
}

DESTRUCTOR(LoDMap) {
    delete[] _LoDMap.pData;
    _LoDMap.pData = NULL;
}

DESTRUCTOR(FontDef) {
    SAFE_RELEASE(_FontDef.pFont);    
}

DESTRUCTOR(FilePack) {
    delete[] _FilePack.lpFPI;
    delete[] _FilePack.lpBasePtr;
}

DESTRUCTOR(Model) {
    for (DWORD i = 0; i < _Model.dwXFileCount; i++)
        pOwner->ReleaseResource(_Model.pXFileIDs[i]);
    delete[] _Model.pXFileIDs;
}

DESTRUCTOR(MemoryXFile) {

}

DESTRUCTOR(MemoryTexture) {
}

DESTRUCTOR(Materials) {
	delete[] _Materials.pMaterialPairs;
}

DESTRUCTOR(TSMRemapTable) {
    delete[] _TSMRemapTable.pfTable;
}

DESTRUCTOR(GrassItem) {
	pOwner->ReleaseResource( _GrassItem.ridTexture );
	for ( UINT i = 0; i < _GrassItem.uiLODCount; i++ ) 
		delete[] _GrassItem.pLODs[i].pVertices;
}

DESTRUCTOR(EngineObject) {
}


DESTRUCTOR(LightObject) {
    if (_LightObject.ridLightMap > 0) pOwner->ReleaseResource(_LightObject.ridLightMap);
}

DESTRUCTOR(CarObject) {
    RESOURCEID  _tmpID;
    RVec::iterator Iter;
    pOwner->ReleaseResource(_CarObject.ridEngine);
    pOwner->ReleaseResource(_CarObject.ridCarObject);
    if (!_CarObject.pvWheelIDs->empty()) {
        Iter = _CarObject.pvWheelIDs->begin();
        do {
            _tmpID = *(Iter++);
            pOwner->ReleaseResource(_tmpID);
        } while (Iter != _CarObject.pvWheelIDs->end()); 
    }
    delete _CarObject.pvWheelIDs;
}

DESTRUCTOR(GameObject) {
    RVec::iterator ridIter;
    RESOURCEID _tmpID;
    
    if (!_GameObject.pvLightObjectIDs->empty()) {
        ridIter = _GameObject.pvLightObjectIDs->begin();
        do {
            _tmpID = *(ridIter++);
            pOwner->ReleaseResource(_tmpID);
        } while (ridIter != _GameObject.pvLightObjectIDs->end());
    }
    
    if (!_GameObject.pvTextureIDs->empty()) {
        ridIter = _GameObject.pvTextureIDs->begin();
        do {
            _tmpID = *(ridIter++);
            pOwner->ReleaseResource(_tmpID);
        } while (ridIter != _GameObject.pvTextureIDs->end());
    }

    if (!_GameObject.pvPhTextureIDs->empty()) {
        ridIter = _GameObject.pvPhTextureIDs->begin();
        do {
            _tmpID = *(ridIter++);
            pOwner->ReleaseResource(_tmpID);
        } while (ridIter != _GameObject.pvPhTextureIDs->end());
    }
    delete _GameObject.pvLightObjectIDs;
    delete _GameObject.pvTextureIDs;
    delete _GameObject.pvPhTextureIDs;
	delete _GameObject.pvTextureFileNames;
	delete _GameObject.pvPhTextureFileNames;
    delete _GameObject.psModel;
	delete _GameObject.pvLightObjectFileNames;
}

DESTRUCTOR(MainObject) {
    RVec::iterator ridIter;
    RESOURCEID _tmpID;
    
    if (!_MainObject.pvObjectIDs->empty()) {
        ridIter = _MainObject.pvObjectIDs->begin();
        do {
            _tmpID = *(ridIter++);
            pOwner->ReleaseResource(_tmpID);
        } while (ridIter != _MainObject.pvObjectIDs->end());
    }

    if (!_MainObject.pvRoadIDs->empty()) {
        ridIter = _MainObject.pvRoadIDs->begin();
        do {
            _tmpID = *(ridIter++);
            pOwner->ReleaseResource(_tmpID);
        } while (ridIter != _MainObject.pvRoadIDs->end());
    }

    if (!_MainObject.pvWaypointsIDs->empty()) {
        ridIter = _MainObject.pvWaypointsIDs->begin();
        do {
            _tmpID = *(ridIter++);
            pOwner->ReleaseResource(_tmpID);
        } while (ridIter != _MainObject.pvWaypointsIDs->end());
    }

    delete _MainObject.pvRoadIDs;
    delete _MainObject.pvObjectIDs;
    delete _MainObject.pvWaypointsIDs;
    delete _MainObject.sName;
    delete _MainObject.sIconName;
}

DESTRUCTOR(Map) {
    int i;
    delete _Map.sMapName;
    delete _Map.sTerrainName;
    delete _Map.sSkyboxName;
    delete _Map.sStartPositionFileName;
    for (i = 0; i < (int) _Map.uiObjectsCount; i++)
        if (_Map.pridObjects[i] != -1)
            pOwner->ReleaseResource(_Map.pridObjects[i]);
    if (_Map.ridStartPosition != -1) pOwner->ReleaseResource(_Map.ridStartPosition);
    delete[] _Map.pridObjects;
    delete[] _Map.psObjectNames;
    delete[] _Map.puiPlacedObjectIndexes;
    delete[] _Map.puiPlacedObjectOrientations;
    delete[] _Map.piPlacedObjectX;
    delete[] _Map.piPlacedObjectZ;
    delete[] _Map.puiPathPointIndexes;  
}

DESTRUCTOR(SkySystem) {
	SAFE_DELETE(_SkySystem.CloudTextureName);
	SAFE_DELETE(_SkySystem.MountainEast);
	SAFE_DELETE(_SkySystem.MountainNorth);
	SAFE_DELETE(_SkySystem.MountainSouth);
	SAFE_DELETE(_SkySystem.MountainWest);
	SAFE_DELETE(_SkySystem.SunTextureName);
	SAFE_DELETE(_SkySystem.AerosolColor);
	SAFE_DELETE(_SkySystem.MoleculeColor);
	SAFE_DELETE_ARRAY(_SkySystem.SunColors);

	if (_SkySystem.CloudTextureID >= 0)
		pOwner->ReleaseResource(_SkySystem.CloudTextureID);
	if (_SkySystem.MountainEastTextureID >= 0)
		pOwner->ReleaseResource(_SkySystem.MountainEastTextureID);
	if (_SkySystem.MountainNorthTextureID >= 0)
		pOwner->ReleaseResource(_SkySystem.MountainNorthTextureID);
	if (_SkySystem.MountainSouthTextureID >= 0)
		pOwner->ReleaseResource(_SkySystem.MountainSouthTextureID);
	if (_SkySystem.MountainWestTextureID >= 0)
		pOwner->ReleaseResource(_SkySystem.MountainWestTextureID);
	if (_SkySystem.SunTextureID >= 0)
		pOwner->ReleaseResource(_SkySystem.SunTextureID);
}

DESTRUCTOR(Waypoints) {
    delete _Waypoints.x;
    delete _Waypoints.y;
    delete _Waypoints.z;
    delete _Waypoints.min_speed;
    delete _Waypoints.max_speed;
}

DESTRUCTOR(TextureSet) {
    UINT i;
    for (i = 0; i < _TextureSet.uiTextureCount; i++)
        if (_TextureSet.pTextureIDs[i] != -1) pOwner->ReleaseResource(_TextureSet.pTextureIDs[i]);
	delete _TextureSet.psPhTexture;
    delete[] _TextureSet.pTextureFileNames;
    delete[] _TextureSet.pTextureProbabilities;
    delete[] _TextureSet.pTextureIDs;
}

DESTRUCTOR(WaveBank) {
	SAFE_DESTROY( _WaveBank.pWaveBank );
}

DESTRUCTOR(SoundBank) {
	SAFE_DESTROY( _SoundBank.pSoundBank );
}


#define DESTRUCTOR_CALL(TYPE) \
    case (RES_##TYPE) : { Destructor(pOwner, _ResourceData._##TYPE); break;}

CResourceManager::CResourceContainer::~CResourceContainer() {
    if (!bReleasable) return;
    switch (uiResType) {
        EXPAND(DESTRUCTOR_CALL)
    }
}

///////////////////////////////////////////////////////////////////////////////
// To correct releasing in CBinaryTree<RESOURCEID, CResourceContainer>... 
///////////////////////////////////////////////////////////////////////////////
CResourceManager::CResourceContainer CResourceManager::CResourceContainer::operator =(CResourceContainer & rc) {
    this->bReleasable   = false; // <--- HERE
    this->_ResourceData = rc._ResourceData;
    this->pOwner        = rc.pOwner;
    this->ResName       = rc.ResName;
    this->uiResType     = rc.uiResType;
    this->iResNameHash  = rc.iResNameHash;
    return rc;
}

