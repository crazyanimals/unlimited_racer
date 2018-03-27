
#pragma once

#include <vector>
#include <windows.h>

#define ABS(a) (a) < 0 ? -(a) : (a)
#define CMP(a,b) (((a) < (b)) ? 1 : ((a) == (b) ? 0 : -1))



// Value is a value to be stored, 
// Key is a class with overloaded operators < and ==

#define RefVec std::vector<ReferenceVectorData *>

template<typename Key, typename Value> class CReferenceVector {
private:
    class ReferenceVectorData {
      public :
        Value       _Value;
        Key         _Key;
        int        iReference; 
                    ReferenceVectorData(Key &K, Value &V) {
                        _Value = V;
                        _Key = K;
                        iReference = 1;
                    }

    };

    RefVec vData;

public :
            CReferenceVector();
           ~CReferenceVector();
  HRESULT   CreateNode      (Key &K, Value &V);
  int       NodeExists      (Key &K) const;
  void      DeleteNode      (Key &K);
  bool      IsEmpty         () const;
  HRESULT   AcquireValue    (Key &K, Value ** ppV) const;
};

///////////////////////////////////////////////////////////////////////////////
//    Constructor
///////////////////////////////////////////////////////////////////////////////

template<typename Key, typename Value> 
CReferenceVector<Key, Value>::CReferenceVector() {

}

///////////////////////////////////////////////////////////////////////////////
//    Destructor
///////////////////////////////////////////////////////////////////////////////

template<typename Key, typename Value>
CReferenceVector<Key, Value>::~CReferenceVector() {
    ReferenceVectorData * prvd;
    RefVec::iterator vDataIter = vData.begin();
    if (vDataIter == vData.end()) return;
    do {
        prvd = (*vDataIter);
        vDataIter = vData.erase(vDataIter);
        delete prvd;
    } while (vDataIter != vData.end());
}

///////////////////////////////////////////////////////////////////////////////
// Creates node with key _K and Value V. If node with this key already
// exists, increases reference to this node. It DOES NOT check, whether
// it has the same value as the original node! You should manage 
// this by yourself... And assign different key if possible 
///////////////////////////////////////////////////////////////////////////////

template<typename Key, typename Value> 
HRESULT CReferenceVector<Key, Value>::CreateNode(Key & K, Value & V) {
    ReferenceVectorData * rvd_new;// = new ReferenceVectorData(K, V);
    RefVec::iterator vDataIter = vData.begin();
    while (vDataIter != vData.end() && CMP((*vDataIter)->_Key, K) == 1) 
        vDataIter++;
    if (vDataIter != vData.end() && !CMP((*vDataIter)->_Key, K)) {
        (*vDataIter)->iReference++;
    } else {
        rvd_new = new ReferenceVectorData(K, V);
        if (!rvd_new) HE(ERROUTOFMEMORY, "CReferenceVector::CreateNode", "");
        vData.insert(vDataIter, rvd_new);
    }
    return ERRNOERROR;   
}

///////////////////////////////////////////////////////////////////////////////
//    Removes saved data associated with certain key
///////////////////////////////////////////////////////////////////////////////

template<typename Key, typename Value> 
void CReferenceVector<Key, Value>::DeleteNode(Key &K) {
    ReferenceVectorData * prvd;
    RefVec::iterator vDataIter = vData.begin();
    while (vDataIter != vData.end() && CMP((*vDataIter)->_Key, K) == 1) 
        vDataIter++;
    if (vDataIter != vData.end() && !CMP((*vDataIter)->_Key, K)) {
        (*vDataIter)->iReference--;
        if (!(*vDataIter)->iReference) {
            prvd = *vDataIter;
            vData.erase(vDataIter);
            delete prvd;
        }
    } else 
        ErrorHandler.HandleError(ERRGENERIC, "CReferenceVector::DeleteNode", "Node not found");
}

///////////////////////////////////////////////////////////////////////////////
// You can use this function to acquire a value associated with certain
// key (_K). If the key is found, the value will be stored in pV and 
// this function will return 'true'. Otherwise, function returns 'false'.
///////////////////////////////////////////////////////////////////////////////

template<typename Key, typename Value> 
HRESULT CReferenceVector<Key, Value>::AcquireValue(Key &K, Value ** ppV) const {
    RefVec::const_iterator vDataIter = vData.begin();
    while (vDataIter != vData.end() && CMP((*vDataIter)->_Key, K) == 1) 
        vDataIter++;
    if (vDataIter != vData.end() && !CMP((*vDataIter)->_Key, K)) {
        *ppV = &((*vDataIter)->_Value);
        return ERRNOERROR;
    } else  HE(BT_NODE_NOT_FOUND, "CReferenceVector::AcquireValue()", "Node with specified key was not found!");
}

///////////////////////////////////////////////////////////////////////////////
// Returns true, if the tree is empty
///////////////////////////////////////////////////////////////////////////////

template<typename Key, typename Value>
bool CReferenceVector<Key, Value>::IsEmpty() const {
    return !(vData.size());
}

///////////////////////////////////////////////////////////////////////////////
// Checks existence of node with specified key
///////////////////////////////////////////////////////////////////////////////

template<typename Key, typename Value>
int CReferenceVector<Key, Value>::NodeExists(Key &K) const {
    RefVec::const_iterator vDataIter = vData.begin();
    if (!vData.size()) return 0;
    while (vDataIter != vData.end() && CMP((*vDataIter)->_Key, K) == 1) 
        vDataIter++;
    return (vDataIter != vData.end() && !CMP((*vDataIter)->_Key, K)) ? (*vDataIter)->iReference : 0;
}