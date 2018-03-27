#include "stdafx.h"
#include "LeftFrm.h"
#include "MapTreeView.h"
#include "MapPictureView.h"
#include "EditorProperties.h"

IMPLEMENT_DYNCREATE(CLeftFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CLeftFrame, CFrameWnd)
	ON_WM_CREATE()
	
END_MESSAGE_MAP()

CLeftFrame::CLeftFrame()
{
	// TODO: add member initialization code here
}

BOOL CLeftFrame::OnCreateClient(LPCREATESTRUCT lpcs,
	CCreateContext* pContext){
	
	if (!m_wndSplitter.CreateStatic (this,2 , 1) ||
		!m_wndSplitter.CreateView (0, 0, RUNTIME_CLASS (CMapPictureView),
		CSize (gProp.pictureViewSizeX, gProp.pictureViewSizeY), 
		pContext) ||
		!m_wndSplitter.CreateView (1, 0, RUNTIME_CLASS (CMapTreeView),
			CSize (200, 0), pContext) 
		
			)
			return FALSE; //fail to create


	return TRUE;
}

#ifdef _DEBUG
void CLeftFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CLeftFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG
