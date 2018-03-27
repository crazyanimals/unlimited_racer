#include "stdafx.h"
#include "ResourceManager.h"
#include <string.h>
#include "Crc32Static.h"

using namespace resManNS;

///////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////
CResourceManager::CResourceManager() {
    Init(HASH_TABLE_SIZE);
};

CResourceManager::CResourceManager(UINT uiHashTableSize) {
    Init(uiHashTableSize); 
};

void CResourceManager::Init(UINT uiHTS) {
   UINT i;
    IdCounter = 1; 
    D3DDevice = NULL;
	OUTTIME(1);
    uiHashTableSize = uiHTS;
    OUTS(" : Initializing ResourceManager ... ", 1);         
    DataHashTable         = new CReferenceVector < RESOURCEID, CResourceContainer > [uiHTS];
    ResourceInfoHashTable = new CReferenceVector < CAtlString,  ResourceInfo      > [uiHTS];
    if (!DataHashTable || !ResourceInfoHashTable) {
        ErrorHandler.HandleError(ERROUTOFMEMORY, "CResourceManager::CResourceManager()", "Could not create HashTable.");
        OUTSN("Failed!", 1);
        if (DataHashTable        ) delete[] DataHashTable;
        if (ResourceInfoHashTable) delete[] ResourceInfoHashTable;
        delete this;
        return;
    }
    for (i = 0; i < RESOURCE_CNT; i++) 
        ResourcePath[i] = "";
    OUTSN("OK!", 1);
};

///////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////
CResourceManager::~CResourceManager() {
    UINT i;

    OUTTIME(1);
    OUTSN(" : Destroying ResourceManager...", 1);
    for (i = 0; i < uiHashTableSize; i++) {
        if (!DataHashTable[i].IsEmpty()) {
            OUTS(" DataHashTable at index '", 3);
            OUTI(i, 3);
            OUTSN("' is not empty!", 3);
        }
        if (!ResourceInfoHashTable[i].IsEmpty()) {
            OUTS(" DataHashTable at index '", 3);
            OUTI(i, 3);
            OUTSN("' is not empty!", 3);
        }
    }
    delete[] DataHashTable;
    delete[] ResourceInfoHashTable;
    OUTTIME(1);
	OUTSN(" : ResourceManager was destroyed.", 1);
};

///////////////////////////////////////////////////////////////////////////////
// inits D3D device this object
///////////////////////////////////////////////////////////////////////////////
void CResourceManager::InitD3DDevice(LPDIRECT3DDEVICE9 D3DDevice) {
    this->D3DDevice = D3DDevice;
};

///////////////////////////////////////////////////////////////////////////////
// loads a resource form a file
///////////////////////////////////////////////////////////////////////////////
#define LOAD_CALL(TYPE) \
    case (RES_##TYPE) : {\
                            OUTSN("Loading "RES_NAME_##TYPE" '" + sResourceName + "' ...", 3);\
                            res = Load##TYPE(rcX, sResourceName, sFileName, bLoadInheritedResources, pParam, riNew.pvInheritedResources);\
                            if (res != RC_OK) {\
                                delete riNew.pvInheritedResources;\
                                OUTSN("FAILED!!! "RES_NAME_##TYPE" '" + sResourceName + "' NOT LOADED!", 3);\
                                HE(ERRGENERIC, "CResourceManager::LoadResource()", "Could not load "RES_NAME_##TYPE".");\
                            }\
                            OUTS(RES_NAME_##TYPE" '" + sResourceName + "' LOADED!", 3);\
                            break;\
                        }
RESOURCEID CResourceManager::LoadResource(CAtlString sFileName, UINT uiResType, bool bLoadInheritedResources, void * pParam) {
    RESOURCEID res;
    CResourceContainer rcX = CResourceContainer(this);
    RESOURCEID Id;
    ResourceInfo * priTmp, riNew;
    CAtlString sResourceName;
    int iPos, iRefCount;
    __int64 iHash;

    if (uiResType < RESOURCE_CNT) sFileName = ResourcePath[uiResType] + sFileName;
    sResourceName = sFileName;
    iPos = sFileName.Find('$');
    if (iPos != -1) sFileName.Truncate(iPos);
    sResourceName.MakeUpper();
    sFileName.MakeUpper();
    iHash = ComputeStringHash(sResourceName);
    iRefCount = ResourceInfoHashTable[iHash % uiHashTableSize].NodeExists(sResourceName);
    if (iRefCount) ResourceInfoHashTable[iHash % uiHashTableSize].AcquireValue(sResourceName, &priTmp);
    if (iRefCount && !priTmp) HE(ERRGENERIC, "CResourceManager::LoadResource()", "Resource info could not be acquired although it exists. Strange :-/");
    if (!iRefCount || (bLoadInheritedResources && !priTmp->bLoadInheritedResources)) {
        if (!iRefCount) riNew.pvInheritedResources = new RVec;
        else riNew.pvInheritedResources = priTmp->pvInheritedResources;
        switch (uiResType) {
            EXPAND(LOAD_CALL);
            default           : {
                                    delete riNew.pvInheritedResources;
                                    HE(RM_INVALID_RESOURCE_TYPE, "CResourceManager::LoadResource()", "Invalid resource type to load!");
                                }
        }
        if (res != RM_OK) return res;
        Id = iRefCount ? priTmp->rId : ++IdCounter;
        OUTS(" - Id is ", 3);
        OUTIN(Id, 3);
        riNew.rId = Id;
        riNew.bLoadInheritedResources = bLoadInheritedResources;
        if (!bCheckResourceCRC[uiResType]) riNew.dwCRC = 0xffffffff;
        else if (CCrc32Static::FileCrc32Win32(sFileName, riNew.dwCRC) != NO_ERROR) {
            riNew.dwCRC = 0xffffffff;            
            ErrorHandler.HandleError(ERRFILECORRUPTED, "CResourceManager::LoadResource()", CAtlString("CRC for resource '") + sFileName + "' could not be calculated!");
        }
        
    } else {
        ResourceInfoHashTable[iHash % uiHashTableSize].AcquireValue(sResourceName, &priTmp);
        if (!priTmp) HE(ERRGENERIC, "CResourceManager::LoadResource()", "Resource info could not be acquired although it exists. Strange :-/")
        else {
            Id = priTmp->rId;
            riNew = *priTmp;
        }
        OUTS( "Increasing reference count for resource '" + sResourceName + "' with id '", 4);
        OUTI( Id, 4);
        OUTS( "' to ", 4);
		OUTI( iRefCount+1, 4);
		OUTSN(".", 4);
    }
    res = ResourceInfoHashTable[iHash % uiHashTableSize].CreateNode(sResourceName, riNew); 
    if (res != BT_OK) HE(res, "CResourceManager::LoadResource()", "Could not create binary tree (ResourceInfo) node.");
    res = DataHashTable[Id % uiHashTableSize].CreateNode(Id, rcX);
	if (res != BT_OK) {
        ResourceInfoHashTable[iHash % uiHashTableSize].DeleteNode(sResourceName);
        HE(res, "CResourceManager::LoadResource()", "Could not create binary tree (DataHashTable) node.");    
    }
    return Id;
};

///////////////////////////////////////////////////////////////////////////////
// computes CRCMultiset for specified resource and resources it loads
///////////////////////////////////////////////////////////////////////////////
std::multiset<DWORD> CResourceManager::GetCRCMultiset(RESOURCEID Id) const {
    std::multiset<DWORD> msTmp;
    RecursiveCRCAdd(&msTmp, Id);
    return msTmp;
}

int CResourceManager::RecursiveCRCAdd(std::multiset<DWORD> * pMS, RESOURCEID Id) const {
    ResourceInfo * priTmp;
    CResourceContainer * prcX;
    int ret_code = RM_OK;
    RVec::size_type i;

    if (!GetResource(Id, &prcX)) 
		HE(RM_RESOURCE_NOT_FOUND, "CResourceManager::RecursiveCRCAdd", "Resource not found");
    ResourceInfoHashTable[prcX->iResNameHash % uiHashTableSize].AcquireValue(prcX->ResName, &priTmp);
    pMS->insert(priTmp->dwCRC);
    for (i = 0; i < priTmp->pvInheritedResources->size(); i++) 
        ret_code = RecursiveCRCAdd(pMS, priTmp->pvInheritedResources->at(i));
    return ret_code;    
}

///////////////////////////////////////////////////////////////////////////////
// duplicates specified resource
///////////////////////////////////////////////////////////////////////////////
void CResourceManager::DuplicateResource(RESOURCEID Id) {
    CResourceContainer *prc;
	int iRefCount;
    ResourceInfo riDummy;
	OUTS("Duplicating resource Id '",4);
	OUTI(Id,3);
	if (Id < 0 || !(iRefCount = DataHashTable[Id % uiHashTableSize].NodeExists(Id))) {
		OUTSN("' ... Failed!", 4);
		return;
	}
    DataHashTable[Id % uiHashTableSize].AcquireValue(Id, &prc);
    if (!prc) {
        ErrorHandler.HandleError(ERRNOTFOUND, "CResourceManager::DuplicateResource()", "Something wrong happened, node exists but is NULL...");
        return;
    }
    DataHashTable[Id % uiHashTableSize].CreateNode(Id, CResourceContainer(this));
    ResourceInfoHashTable[prc->iResNameHash % uiHashTableSize].CreateNode(prc->ResName, riDummy);
	OUTS("' ... Ok! (Reference count increased to ", 4);
    OUTI(iRefCount, 4);
    OUTSN(" )", 4);
};

///////////////////////////////////////////////////////////////////////////////
// Gets specified resource
///////////////////////////////////////////////////////////////////////////////
#define GET_RESOURCE_EXPANDER(TYPE) \
    TYPE * CResourceManager::Get##TYPE(RESOURCEID Id) const {\
        CResourceContainer *prc;\
        if (Id < 0 || !DataHashTable[Id % uiHashTableSize].NodeExists(Id)) {\
        OUTS("Could not obtain "RES_NAME_##TYPE" with Id = ",3);\
        OUTIN(Id,3);\
            return NULL;\
        } else DataHashTable[Id % uiHashTableSize].AcquireValue(Id, &prc);\
        if (prc->uiResType != RES_##TYPE) {\
            ErrorHandler.HandleError(RM_INVALID_RESOURCE_TYPE, "CResourceManager::Get"RES_NAME_##TYPE"()", "Not a "RES_NAME_##TYPE"!");\
            return NULL;\
        }\
        return &(prc->_ResourceData._##TYPE);\
    }

    EXPAND(GET_RESOURCE_EXPANDER)

int CResourceManager::GetResource(RESOURCEID Id, CResourceContainer ** ppRC) const {
    if (Id < 0 || !DataHashTable[Id % uiHashTableSize].NodeExists(Id)) {
        ErrorHandler.HandleError( RM_RESOURCE_NOT_FOUND, "CResourceManager::GetResource()", "Could not obtain resource." );
        return 0;
    } else DataHashTable[Id % uiHashTableSize].AcquireValue(Id, ppRC);
    return 1;  
};


///////////////////////////////////////////////////////////////////////////////
// Gets ResourceInfo structure of Resource specified by sResourceName
///////////////////////////////////////////////////////////////////////////////
ResourceInfo * CResourceManager::GetResourceInfo(CAtlString sFileName, UINT uiResType) const {
    ResourceInfo *priTmp;
    CAtlString sResourceName;
    __int64 iHash;

    sResourceName = GetResourcePath(uiResType) + sFileName;
    sResourceName.MakeUpper();
    iHash = ComputeStringHash(sResourceName);
    if (!ResourceInfoHashTable[iHash % uiHashTableSize].NodeExists(sResourceName)) return NULL;
    ResourceInfoHashTable[iHash % uiHashTableSize].AcquireValue(sResourceName, &priTmp);
    return priTmp;
}

///////////////////////////////////////////////////////////////////////////////
// releases specified resource
///////////////////////////////////////////////////////////////////////////////
void CResourceManager::ReleaseResource(RESOURCEID Id) {
    CAtlString _tmp;
    CResourceContainer * prcX = NULL;
    ResourceInfo * priTmp;
    RVec * pvIR;
	int iRefCount;
    if (Id < 0 || !DataHashTable[Id % uiHashTableSize].NodeExists(Id)) {
        OUTS( "Releasing resource with Id '", 3);  OUTI( Id, 3); OUTSN( "' ... NOT FOUND!", 3);
        return;
    }  else DataHashTable[Id % uiHashTableSize].AcquireValue(Id, &prcX);
    if (!prcX) {
        ErrorHandler.HandleError(ERRNOTFOUND, "CResourceManager::ReleaseResource()", "Something wrong happened, node exists but is NULL...");
        return;
    }
    ResourceInfoHashTable[prcX->iResNameHash % uiHashTableSize].AcquireValue(prcX->ResName, &priTmp);
    pvIR = priTmp->pvInheritedResources;
    ResourceInfoHashTable[prcX->iResNameHash % uiHashTableSize].DeleteNode(prcX->ResName);
    prcX->bReleasable = true;
    _tmp = prcX->ResName;
    DataHashTable[Id % uiHashTableSize].DeleteNode(Id);
    if (iRefCount = DataHashTable[Id % uiHashTableSize].NodeExists(Id)) {
        prcX->bReleasable = false;
        OUTS( "Releasing resource with Id '", 4);  OUTI( Id, 4); 
        OUTS( "' Name : '" + _tmp + "' ... DONE! (Reference count decreased to ", 4);
		OUTI( iRefCount, 4);
		OUTSN(" )", 4);
    }
    else {
        delete pvIR;        
        OUTS( "Releasing resource with Id '", 3);  OUTI( Id, 3); 
        OUTSN( "' Name : '" + _tmp + "' ... DONE! (Resource freed)", 3);
    }
};

///////////////////////////////////////////////////////////////////////////////
// Sets default directory for specified type of resource
///////////////////////////////////////////////////////////////////////////////
void CResourceManager::SetResourcePath(CAtlString sPath, UINT uiResType) {
    if (uiResType >= RESOURCE_CNT) {
        ErrorHandler.HandleError(ERRINVALIDRESOURCETYPE, "CResourceManager::SetResourcePath()", "Invalid resource type!");
        return;
    }
    if (sPath[sPath.GetLength()-1] != '\\') sPath += '\\';
    ResourcePath[uiResType] = sPath.MakeUpper(); //Upper just to be sure with case insensitivity.
    OUTS( "Setting path for Resource type ", 3);
    OUTI( uiResType, 3);
    OUTSN( " to '" + sPath + "'", 3);
};

///////////////////////////////////////////////////////////////////////////////
// Gets default directory for specified type of resource
///////////////////////////////////////////////////////////////////////////////
CAtlString CResourceManager::GetResourcePath(UINT uiResType) const {
    if (uiResType >= RESOURCE_CNT) {
        ErrorHandler.HandleError(ERRINVALIDRESOURCETYPE, "CResourceManager::GetResourcePath()", "Invalid resource type!");
        return "";
    }
    return ResourcePath[uiResType];
};

///////////////////////////////////////////////////////////////////////////////
// Gets default directory for specified type of resource
///////////////////////////////////////////////////////////////////////////////
__int64 CResourceManager::ComputeStringHash(ATL::CAtlString str) {
    __int64 iHash = 0;
    int i, len;
    len = str.GetLength();
    for (i = 0; i < len; i++)
        iHash += (__int64) (str[i] % 26);
    return iHash;
}