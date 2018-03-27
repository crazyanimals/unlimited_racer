/*
    ResourceManager:Class managing all game resources stored on disk
    Creation Date:    14.10.2003
    Last Update:      09.01.2005
    Author:            Jirka Sofka

*/

#pragma once

#include "CReferenceVector.h"
#include "RMDataStructs.h"
#include <set>
#include <vector>


#define MAX_FILENAME_SIZE 1024
#define HASH_TABLE_SIZE 100

#define OTHER_MATERIAL_TYPE	0
#define CAR_MATERIAL_TYPE	1

#define CONSTRUCTOR_DECLARATOR(TYPE) \
    CResourceContainer(CResourceManager * Owner, CAtlString Name, TYPE _##TYPE);

#define LOAD_RESOURCE_DECLARATOR(TYPE) \
    HRESULT Load##TYPE(CResourceContainer &rcX, CAtlString sResourceName, CAtlString sFileName, bool bLoadInheritedResources, void * pParam, RVec * pvInheritedResources);

#define GET_RESOURCE_DECLARATOR(TYPE) \
    TYPE * Get##TYPE(RESOURCEID Id) const;

// ResourceManager namespace
namespace resManNS {

    typedef struct __ResourceInfo {
        RESOURCEID rId;
        bool bLoadInheritedResources;
        DWORD dwCRC;
        RVec * pvInheritedResources;
    } ResourceInfo;

// ResourceManager class
    class CResourceManager {
    public:
        class CResourceContainer {
        private :
            CResourceManager *  pOwner;
        public :
            bool                bReleasable;
            UINT                uiResType;
            ResourceData        _ResourceData;
            CAtlString          ResName;
            __int64             iResNameHash;
                                CResourceContainer          (CResourceManager * Owner = NULL);
                                CResourceContainer          (const CResourceContainer & rcX);
                                EXPAND(CONSTRUCTOR_DECLARATOR)
                                ~CResourceContainer         ();
            CResourceContainer  operator=(CResourceContainer & rc);
            friend class CResourceManager;
        };  
                                CResourceManager();
                                CResourceManager(UINT uiHashTableSize);
                               ~CResourceManager(); 
        void                    InitD3DDevice   (LPDIRECT3DDEVICE9 D3DDevice); 
        // Note : when loading 2 resources from the same file with different pParam, only the first
        // one is actually loaded. The second only increases reference count of the resource, setting
        // pParam has no effect...
        RESOURCEID              LoadResource     (CAtlString sFileName, UINT uiResType, bool bLoadInheritedResources, void * pParam); 
        RESOURCEID              LoadResource     (CAtlString sFileName, UINT uiResType, void * pParam = NULL) {
            return LoadResource(sFileName, uiResType, true, pParam);
        }
        void                    DuplicateResource(RESOURCEID Id);
        int                     GetResource      (RESOURCEID Id, CResourceContainer ** pRC) const; 
        ResourceInfo *          GetResourceInfo  (CAtlString sFileName, UINT uiResType) const;
        RESOURCEID              GetResourceID    (CAtlString sFileName, UINT uiResType) const {
            ResourceInfo * priTmp = GetResourceInfo(sFileName, uiResType);
            return priTmp ? priTmp->rId : -1;
        }
       
        EXPAND(GET_RESOURCE_DECLARATOR)

        void                    ReleaseResource  (RESOURCEID Id); 
        void                    SetResourcePath  (CAtlString sPath, UINT uiResType);
        CAtlString              GetResourcePath  (UINT uiResType) const;
        
        std::multiset<DWORD>    GetCRCMultiset(RESOURCEID Id) const;

    private:
        UINT uiHashTableSize;
        CReferenceVector < RESOURCEID, CResourceContainer > * DataHashTable;
        CReferenceVector < CAtlString, ResourceInfo       > * ResourceInfoHashTable;
        LPDIRECT3DDEVICE9       D3DDevice;
        CAtlString              ResourcePath[RESOURCE_CNT];
        RESOURCEID              IdCounter;

        int             RecursiveCRCAdd(std::multiset<DWORD> * pMS, RESOURCEID Id) const;
        void            Init(UINT uiHTS);     
        static __int64  ComputeStringHash(CAtlString str);

        EXPAND(LOAD_RESOURCE_DECLARATOR)    
    } ;
}

//using namespace resManNS;