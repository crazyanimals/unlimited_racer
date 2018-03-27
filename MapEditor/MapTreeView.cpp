// MapListView.cpp : implementation file
//

#include "stdafx.h"
#include "MapEditor.h"

#include "MapEditorDoc.h"
#include "MapTreeView.h"
#include "MapPictureView.h"
#include "..\globals\Configuration.h"
#include "MapDialogs.h"

extern CConfig	Configuration;

// CMapTreeView

IMPLEMENT_DYNCREATE(CMapTreeView, CTreeView)



BEGIN_MESSAGE_MAP(CMapTreeView, CTreeView)
	ON_WM_CREATE()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_NOTIFY_REFLECT (TVN_ENDLABELEDIT,OnEndLabelEdit)
	ON_COMMAND(ID_TREE_ADDNEWITEM, OnAddNewItem)
	ON_COMMAND(ID_TREE_ADDNEWSUBITEM, OnAddNewSubItem)
	ON_COMMAND(ID_TREE_CUT, OnCut)
	ON_COMMAND(ID_TREE_PASTE, OnPaste )
	ON_COMMAND(ID_TREE_DELETE, OnDelete)
	ON_COMMAND(ID_TREE_RENAME, OnRename)
	ON_COMMAND(ID_TREE_PROPERTIES, OnProperties)
	//ON_MESSAGE(TVN_ENDLABELEDIT,OnEndLabelEdit)

END_MESSAGE_MAP()





CMapTreeView::CMapTreeView()
{
}

CMapTreeView::~CMapTreeView()
{
	
}
// CMapTreeView drawing

void PopulateTree(){
	
};

int CMapTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	lpCreateStruct->style = TVS_HASLINES | TVS_HASBUTTONS | TVS_EDITLABELS  ;
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;
 	menu.LoadMenu(IDR_TREE_MENU);
	//ctlTree.SetImageList(&m_ctlImage);
	
	//m_dropTarget.Register(this);

	return 0;
};


void CMapTreeView::OnInitialUpdate(){
	HRESULT hr;
	HTREEITEM child;
	pDocument = (CMapEditorDoc  *) GetDocument();
	if(!pDocument->editorTree.pTreeCtrl){
		pTreeCtrl = (CMapTreeCtrl *) &(GetTreeCtrl());
		pDocument->editorTree.setTreeCtrl(pTreeCtrl);
		CString path=Configuration.EditorPath;
		
		hr = pDocument->editorTree.LoadTree(CONFIG_EDITORPATH + TREE_FILE);
		if(hr) { 
			pTreeCtrl->InsertItem("Unlimited Racer",0,0,0,NULL);
			
		}
		HTREEITEM root =pTreeCtrl->GetRootItem();
		pTreeCtrl->Expand(root,TVE_EXPAND);
		child=pTreeCtrl->GetChildItem(root);
		pTreeCtrl->Expand(child,TVE_EXPAND);
		while(child=pTreeCtrl->GetNextSiblingItem(child)){
			pTreeCtrl->Expand(child,TVE_EXPAND);
		}
	}
};

void CMapTreeView::OnDraw(CDC* pDC)
{
	//CDocument* pDoc = GetDocument();
	
	// TODO: add draw code here

}
void CMapTreeView::OnLButtonDown(UINT nFlags,CPoint point){
	CTreeView::OnLButtonDown(nFlags,point);
	CMapEditorDoc * pDoc = (CMapEditorDoc *) GetDocument();
	pTreeCtrl = (CMapTreeCtrl *) &(GetTreeCtrl());
	HTREEITEM item = pTreeCtrl->GetSelectedItem();
	pDoc->selObjectOr=up;
	pDoc->pSelObject = (CMainObject *) pTreeCtrl->GetItemData(item);
	pDoc->bEditState = STATE_MAP_EDIT;
	
	GetParent()->Invalidate(0);
	
}
void CMapTreeView::OnRButtonDown(UINT nFlags,CPoint point){
	//CTreeView::OnLButtonDown(nFlags,point);
	CTreeView::OnRButtonDown(nFlags, point);	
		CMapEditorDoc * pDoc = (CMapEditorDoc *) GetDocument();
	UINT res;
	//OnLButtonDown(nFlags,point);
	//CPoint pos;
	pTreeCtrl = (CMapTreeCtrl *) &(GetTreeCtrl());
	HTREEITEM item = pTreeCtrl->HitTest(point,&res);
	//pTreeCtrl->GetItem(
	if ((item != NULL) && (TVHT_ONITEM & res))
	{
		pTreeCtrl->SelectItem(item);
	}
	item = pTreeCtrl->GetSelectedItem();
	pDoc->selObjectOr=up;
	pDoc->pSelObject = (CMainObject *) pTreeCtrl->GetItemData(item);
	pDoc->bEditState = STATE_MAP_EDIT;
	GetParent()->Invalidate(0);
	ClientToScreen(&point);
	CMenu *right_menu =menu.GetSubMenu(0);
	right_menu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x,point.y, this);
	

}


void CMapTreeView::OnMButtonDown(UINT nFlags,CPoint point){
	
	CMapEditorDoc* pDoc = (CMapEditorDoc*) GetDocument();
	if (!pDoc)		return;
	CTreeView::OnMButtonDown(nFlags, point);	
	

	switch(pDoc->bEditState){
		case STATE_MAP_EDIT:
			
			if(pDoc->pSelObject){
			pDoc->selObjectOr = (ORIENTATION) (((int) pDoc->selObjectOr +1)%4) ;//(pDoc->selObjectOr +1)%4;
			GetParent()->Invalidate(0);
			};
		break;
			
	};
	

};

// CMapTreeView diagnostics
////////////////////////////////////////////
// TREE MENU 
////////////////////////////////////////////
void CMapTreeView::OnEndLabelEdit(NMHDR * pNmhdr, LRESULT * pResult){
	 *pResult = TRUE;
	
}

void CMapTreeView::OnAddNewItem(){

	HTREEITEM item=pTreeCtrl->GetSelectedItem();
	if(!pTreeCtrl->ItemHasChildren(item)){
		item = pTreeCtrl->GetParentItem(item);
	}
	pDocument->editorTree.AddItem(item);
	
};
void CMapTreeView::OnAddNewSubItem(){
	HTREEITEM item=pTreeCtrl->GetSelectedItem();
	if(!pTreeCtrl->GetItemData(item)){
		pDocument->editorTree.AddItem(item);
		pTreeCtrl->Expand(item,TVE_EXPAND);
	}

};
void CMapTreeView::OnCut(){
	
	hCutItem = pTreeCtrl->GetSelectedItem();
	menu.EnableMenuItem(ID_TREE_PASTE,MF_ENABLED);
};

void CMapTreeView::OnDelete(){
	HTREEITEM item = pTreeCtrl->GetSelectedItem();
	pDocument->editorTree.deleteItemFromTree(item);
	
};
void CMapTreeView::OnPaste(){

	HTREEITEM item=pTreeCtrl->GetSelectedItem();
	if(pTreeCtrl->GetItemData(item)){
		item = pTreeCtrl->GetParentItem(item);
	}
	pTreeCtrl->CopyBranch(hCutItem,item,TVI_SORT);
	pTreeCtrl->DeleteItem(hCutItem);
	hCutItem = NULL;
	menu.EnableMenuItem(ID_TREE_PASTE,MF_GRAYED);
};

void CMapTreeView::OnRename(){
	pTreeCtrl->EditLabel(pTreeCtrl->GetSelectedItem());
};
void CMapTreeView::OnProperties(){
	HTREEITEM item = pTreeCtrl->GetSelectedItem();
	INT_PTR res;
	DWORD_PTR pOld;
	CString name;
	CMainObject * pMO,*_pMO;
	CEditFileDialog dialog("",OBJECT_EXT,CONFIG_OBJECTSPATH);
	if(!pTreeCtrl->ItemHasChildren(item)){
		CMainObject * pObj = (CMainObject *) pTreeCtrl->GetItemData(item);
		
		if(pObj)
			dialog.sFileName = pObj->getFileName();	
		res=dialog.DoModal();
		if(res==IDOK){
			pOld=0;
			pMO = pDocument->editorTree.LoadMainObject(dialog.sFileName);		
			if(pMO){
				if (pDocument->objectsMap[dialog.sFileName]){
					_pMO = (CMainObject *) pDocument->objectsMap[dialog.sFileName]; 
					pDocument->pGameMap->ChangeMainObject(_pMO,pMO);
					SAFE_DELETE(_pMO);
				}
				pOld = (DWORD_PTR) pDocument->objectsMap[dialog.sFileName];
				pDocument->objectsMap[dialog.sFileName]=pMO;
				pDocument->pSelObject = pMO;
				GetParent()->Invalidate(0);

			}else{
				if (pDocument->objectsMap[dialog.sFileName]){
					_pMO = (CMainObject *) pDocument->objectsMap[dialog.sFileName]; 
					pDocument->objectsMap.RemoveKey(dialog.sFileName);
					SAFE_DELETE(_pMO);
					pDocument->pSelObject = NULL;
				}
			}
			
			pTreeCtrl->SetItemData(item,(DWORD_PTR) pMO);
			if(pOld) pTreeCtrl->ChangePointer(pTreeCtrl->GetRootItem(),pOld,(DWORD_PTR) pMO);
			
			if(pMO) pTreeCtrl->SetItemText(item,pMO->sName);
			else{ 
				name = dialog.sFileName;
				name.Delete((int)(name.GetLength()-strlen(OBJECT_EXT)),(int)strlen(OBJECT_EXT));
				pTreeCtrl->SetItemText(item,name);
			}
		}

	};	
};


#ifdef _DEBUG
void CMapTreeView::AssertValid() const
{
	CView::AssertValid();
}

void CMapTreeView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG


// CMapTreeView message handlers
