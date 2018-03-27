// MapEditorView.h : interface of the CMapEditorView class
//


#pragma once

#include <list>
class CMapEditorView;

class CRack
{
	
	private:
		CMapEditorDoc * pDoc;
		
		
	
	private:
		 void countImagePlg(BYTE,ORIENTATION,CPoint*);
	//	void countImagePosition(BYTE,ORIENTATION,CRect &);
		void paintStartingPos(CDC *,MAPPOSITION mapPos,CMainObject * pSP,CRect view);	
		void paintWaypoints(CDC * pDC,MAPPOSITION mapPos,CMainObject * pMO,CRect view);
	public:
		
		CRack(UINT _scale=27,UINT px = 100,UINT py = 100);
		UINT px,py;
		UINT scale;
		void init(CMapEditorDoc * _pDoc){ pDoc=_pDoc;}
		//BOOL init(CMapEditorDoc * _data,int _scale = 1);
		void setSize(UINT _px,UINT _py)	{px = _px; py = _py;};
		void rescale(UINT _scale){ scale = _scale; };
		void increaseAltitude(CPoint);//plate position, diference
		void increaseAltitude(UINT x,UINT y);
		void ToIncrease(CPoint point, int (&pole)[3][3]);
		void decreaseAltitude(CPoint, BOOL decreased=FALSE);
		void decreaseAltitude(UINT x,UINT y);
	
		CPoint mouseToMap(CPoint);
		
		int test_border(UINT x,UINT y){ return ((x < px) && (y < py));}
		
		int Zoom(int step);
		int unZoom(int step);
		
		CSize getSize();
		BOOL paint(CDC *,CMapEditorDoc*,CRect,CMapEditorView * pEditorView);
		
};


class CMapEditorView : public CScrollView
{
  public: 
	void Zoom();
	void unZoom();
	
  protected:
	CRack *rack;
	//CMainObject * pSelectedObject;

	
protected: // create from serialization only
	CMapEditorView();
	DECLARE_DYNCREATE(CMapEditorView)
	
// Attributes
public:
	CMapEditorDoc * GetDocument() const;
	
	CPoint downPoint;
	CPoint mousePos;
// Operations
public:

// Overrides
	public:
	virtual void OnDraw(CDC *);  // overridden to draw this view
	virtual void OnActivateView(   BOOL bActivate,CView* pActivateView,CView* pDeactiveView );
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);


protected:

	virtual void OnInitialUpdate();

	//virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	//virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	//virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	
// Implementation
public:
	virtual ~CMapEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif



// Generated message map functions
protected:

	afx_msg void OnChar(UINT,UINT,UINT);
	afx_msg void OnLButtonDown(UINT nFlags,CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags,CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags,CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags,CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags,CPoint point);
	afx_msg void OnMouseMove(UINT nFlags,CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	
	afx_msg BOOL OnMouseWheel(UINT fFlags,short zDelta,CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC );
	DECLARE_MESSAGE_MAP()
	
};


#ifndef _DEBUG  // debug version in MapEditorView.cpp
inline CMapEditorDoc* CMapEditorView::GetDocument() const
   { return reinterpret_cast<CMapEditorDoc*>(m_pDocument); }
#endif


