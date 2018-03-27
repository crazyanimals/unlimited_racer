#pragma once

#define IDC_ROTLEFTBUTTON 101
#define IDC_ROTRIGHTBUTTON 102


class CMapPictureView : public CScrollView
{
	DECLARE_DYNCREATE(CMapPictureView)

protected:
	CMapPictureView();           // protected constructor used by dynamic creation
	virtual ~CMapPictureView();
	virtual void OnInitialUpdate();

private:
	 void countImagePlg(BYTE,ORIENTATION,CPoint*);
	 void paintWaypoints(CDC * pDC,CMainObject * pMO,UINT size,ORIENTATION or,CPoint pos);
public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnSize(UINT nType,int cx, int cy);
	virtual int OnCreate(LPCREATESTRUCT);
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


protected:
	CBitmapButton leftButton; //rotate button
	CBitmapButton rightButton; // rotate button
	afx_msg void OnRotationLeft();
	afx_msg void OnRotationRight();
	//afx_msg void OnPaint();
	
	//int CMapTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct);

protected:

	DECLARE_MESSAGE_MAP()
	
};

