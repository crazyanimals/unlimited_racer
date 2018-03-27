#include "stdafx.h"
#include "resource.h"
#include "MapEditorDoc.h"
#include "MapPictureView.h"
#include "MapEditorView.h"
#include "EditorProperties.h"



IMPLEMENT_DYNCREATE(CMapPictureView, CScrollView)
	


BEGIN_MESSAGE_MAP(CMapPictureView, CScrollView)
	ON_WM_SIZE()
	ON_WM_CREATE()
	//ON_WM_PAINT()
	ON_COMMAND(IDC_ROTLEFTBUTTON,OnRotationLeft)
	ON_COMMAND(IDC_ROTRIGHTBUTTON,OnRotationRight)
END_MESSAGE_MAP()





CMapPictureView::CMapPictureView()
{
}

CMapPictureView::~CMapPictureView()
{
}
// CMapPictureView drawing

/*
void CMapPictureView::OnPaint(){
	
}*/
void CMapPictureView::OnInitialUpdate(){
	CScrollView::OnInitialUpdate();
	
	
	
	SIZE s;
	s.cx= gProp.pictureViewSizeX;
	s.cy= gProp.pictureViewSizeY;
	SetScrollSizes(MM_TEXT,s);
}

void CMapPictureView::OnSize(UINT nType,int cx,int cy){
	int x,y;
	CScrollView::OnSize(nType,cx,cy);
	x = cx > gProp.pictureViewSizeX ? cx : gProp.pictureViewSizeX;
	y = cy > gProp.pictureViewSizeY ? cy : gProp.pictureViewSizeY;
	

	leftButton.MoveWindow(x*2/5-12,y-20-24,24,24,1);
	rightButton.MoveWindow(x*3/5-12,y-20-24,24,24,1);

}
void CMapPictureView::countImagePlg(BYTE size, ORIENTATION or,CPoint * points){
	
	UINT sc = size;
	switch(or){
		
		case ORIENTATION::up:
			points[0].x= 0;
			points[0].y= 0;
			points[1].x= sc;
			points[1].y= 0;
			points[2].x= 0;
			points[2].y= sc;
			break;

		case ORIENTATION::down:
			points[0].x= sc-1;
			points[0].y= sc-1;
			points[1].x= -1;
			points[1].y= sc-1;
			points[2].x= sc-1;
			points[2].y= -1;
			break;

		case ORIENTATION::left:
			points[0].x= 0;
			points[0].y= sc;
			points[1].x= 0;
			points[1].y= 0;
			points[2].x= sc;
			points[2].y= sc;
			break;

		case ORIENTATION::right:
			points[0].x= sc;
			points[0].y= 0;
			points[1].x= sc;
			points[1].y= sc;
			points[2].x= 0;
			points[2].y= 0;
			break;

	}
	
}
void CMapPictureView::OnRotationLeft(){
	CMapEditorDoc * pDoc = (CMapEditorDoc *)GetDocument();
	if(pDoc->pSelObject){
		pDoc->selObjectOr = (ORIENTATION) (((int) pDoc->selObjectOr +3)%4) ;//(pDoc->selObjectOr +1)%4;
			GetParent()->Invalidate();
			Invalidate();
		};
}
void CMapPictureView::OnRotationRight(){
	CMapEditorDoc * pDoc = (CMapEditorDoc *)GetDocument();
	if(pDoc->pSelObject){
		pDoc->selObjectOr = (ORIENTATION) (((int) pDoc->selObjectOr +1)%4) ;//(pDoc->selObjectOr +1)%4;
			GetParent()->Invalidate();
			Invalidate();
		};
}
int CMapPictureView::OnCreate (LPCREATESTRUCT lpcs)
{	if(CScrollView::OnCreate(lpcs) == -1) return -1;
	

	int x,y;
	//CBitmap arrow;
	//HBITMAP bmp;
	//arrow.LoadBitmap(MAKEINTRESOURCE(IDB_LEFTARROW));
	
	//arrow.GetBitmap((BITMAP *)bmp);
	
	//arrow.GetBitmap(
	CRect wndRect;
	GetWindowRect(wndRect);
	
	//leftButton.AutoLoad(IDB_LEFTARROW,this);
	x = wndRect.Width () > gProp.pictureViewSizeX ? wndRect.Width() : gProp.pictureViewSizeX;
	y = wndRect.Height() > gProp.pictureViewSizeY ? wndRect.Height() : gProp.pictureViewSizeY;
	leftButton.Create(NULL,WS_VISIBLE | WS_CHILD | WS_TABSTOP |BS_OWNERDRAW,
		CRect( x *2/5 -12, y-20-24,	5*3/5+12, y-20),
		this,IDC_ROTLEFTBUTTON);
	
	rightButton.Create(NULL,WS_VISIBLE | WS_CHILD | WS_TABSTOP |BS_OWNERDRAW,
		CRect(x*3/5-12,y-20-24,x*3/5+12,y-20)
		,this,IDC_ROTRIGHTBUTTON);

	leftButton.LoadBitmaps(IDB_LEFTARROW_UP,IDB_LEFTARROW_DOWN,IDB_LEFTARROW_UP,IDB_LEFTARROW_UP);
	rightButton.LoadBitmaps(IDB_RIGHTARROW_UP,IDB_RIGHTARROW_DOWN,IDB_RIGHTARROW_UP,IDB_RIGHTARROW_UP);
	return 0;

};
void CMapPictureView::paintWaypoints(CDC * pDC,CMainObject * pMO,UINT size,ORIENTATION or,CPoint pos){
	CPen pen,penbig;
	pen.CreatePen(PS_SOLID,1,COLOR_WAYPOINT);
	penbig.CreatePen(PS_SOLID,4,COLOR_WAYPOINT);
	
	
	UINT wsID,wID;
	WAYPOINT waypoint;
	int xs,ys;
	if (pMO){
	
		for(wsID = 0;wsID < pMO->GetWaypointSetSize();wsID++){
			if (pMO->GetWaypointSize(wsID) > 0){
				waypoint = pMO->GetWaypoint(wsID,0,or);		
				xs =(int)(waypoint.x*size/TERRAIN_PLATE_WIDTH/pMO->getSize()); 
				ys =(int)(size - (waypoint.z*size/TERRAIN_PLATE_WIDTH/pMO->getSize()));
				xs += pos.x;
				ys += pos.y;
				pDC->MoveTo(xs,ys);
			}
			pDC->SelectObject(penbig);
			for(wID=0;wID<pMO->GetWaypointSize(wsID);wID++){
				waypoint = pMO->GetWaypoint(wsID,wID,or);		
				xs =(int)(waypoint.x*size/TERRAIN_PLATE_WIDTH/pMO->getSize()); 
				ys =(int)(size - (waypoint.z*size/TERRAIN_PLATE_WIDTH/pMO->getSize()));
				xs += pos.x;
				ys += pos.y;
				pDC->LineTo(xs,ys);
				pDC->SelectObject(pen);
			}

		}
	}

}


void CMapPictureView::OnDraw(CDC* pDC)
{
	
	//CView::OnDraw(pDC);
	CImage * pImg;
	CImage pomImg;
	CPoint points[3],pos;
	ORIENTATION or;
	UINT size;
	CRect view,rect,maxRect;
	
	int x,y;
	CMapEditorDoc * pDoc = (CMapEditorDoc *)GetDocument();
	
	
	/*int ret = pDC->GetClipBox(&view);
	if(ret){
		GetWindowRect(&view);
		ScreenToClient(&view);
	}*/
	GetClientRect(&view);
	x = view.Width () > gProp.pictureViewSizeX ? view.Width() : gProp.pictureViewSizeX;
	y = view.Height() > gProp.pictureViewSizeY ? view.Height() : gProp.pictureViewSizeY;

	
	

/*	
	leftButton.Create(NULL,WS_VISIBLE | WS_CHILD | WS_TABSTOP,
		CRect( x / 4 -12, y-20-24,	x/4+12, y-20),
		this,IDC_ROTLEFTBUTTON);
	
	rightButton.Create(NULL,WS_VISIBLE | WS_CHILD | WS_TABSTOP,
		CRect(x*3/4-12,y-20-24,x*3/4+12,y-20)
		,this,IDC_ROTRIGHTBUTTON);
*/
	maxRect.SetRect(20,20,x-20,y-60);
	y -= 50;

	if(pDoc->pSelObject && pDoc->pSelObject->pImage){
		pImg = pDoc->pSelObject->pImage;
		
		size = maxRect.Width() < maxRect.Height() ? maxRect.Width() : maxRect.Height();
		size = (UINT)pImg->GetWidth() < size ? (UINT)pImg->GetWidth() : size;
		
		or = pDoc->selObjectOr;
		pomImg.Create(size,size,32);
		countImagePlg(size,or,points);
		pImg->PlgBlt(pomImg.GetDC(),points);
		
		
		//	countImagePos(size,or,position);
		rect.SetRect(maxRect.CenterPoint().x-size/2,maxRect.CenterPoint().y-size/2,
			maxRect.CenterPoint().x+size/2,maxRect.CenterPoint().y+size/2);
		rect.left -= view.left;
		rect.right -=view.left;
		rect.top -= view.top;
		rect.bottom -=view.top;
		//maxRect.SetRect(view.left,view.top,
		pomImg.TransparentBlt(pDC->m_hDC,(RECT &) rect,TRANSPARENT_COLOR);
						//pomImg.TransparentBlt(pDC->m_hDC,(RECT &) rect,0xc536fa);

		CBrush brush;
		brush.CreateSolidBrush(RGB(0,230,0));
		//pDC->SelectObject(brush);
		//pDC->Rectangle(maxRect);
		pomImg.ReleaseDC();
		pomImg.Destroy();
		pos.x=rect.left;
		pos.y=rect.top;
		paintWaypoints(pDC,pDoc->pSelObject,size,or,pos);
	}
	
}


// CMapPictureView diagnostics

#ifdef _DEBUG
void CMapPictureView::AssertValid() const
{
	CView::AssertValid();
}

void CMapPictureView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG


// CMapPictureView message handlers