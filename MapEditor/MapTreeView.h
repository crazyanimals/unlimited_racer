#pragma once
#include "MapEditorDoc.h"

// CMapTreeView view

class CTreePopUp : public CMenu{
	
};


class CMapTreeView : public CTreeView
{
	DECLARE_DYNCREATE(CMapTreeView)

protected:
	CTreePopUp menu;
	CMapEditorDoc * pDocument;
	CMapTreeCtrl	  * pTreeCtrl;
	HTREEITEM hCutItem;
	CMapTreeView();           // protected constructor used by dynamic creation
	virtual ~CMapTreeView();
	virtual void OnInitialUpdate();
	
public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	
	void PopulateTree();
	//int CMapTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct);

protected:
	afx_msg void OnMButtonDown(UINT nFlags,CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags,CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags,CPoint point);
	afx_msg void OnEndLabelEdit(NMHDR * , LRESULT  *);
	afx_msg void  OnAddNewItem();
	afx_msg void  OnAddNewSubItem();
	afx_msg void  OnCut();
	afx_msg void  OnPaste();
	afx_msg void  OnDelete();
	afx_msg void  OnRename();
	afx_msg void  OnProperties();
	DECLARE_MESSAGE_MAP()
};

