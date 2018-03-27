// MapEditorView.cpp : implementation of the CMapEditorView class
//

#include "stdafx.h"
#include "MapEditor.h"

#include "MapEditorDoc.h"
#include "MapEditorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMapEditorView

IMPLEMENT_DYNCREATE(CMapEditorView, CScrollView)

BEGIN_MESSAGE_MAP(CMapEditorView, CScrollView)
	// Standard printing commands
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_ZOOM,Zoom)
	ON_COMMAND(ID_UNZOOM,unZoom)
	//ON_WM_MOUSEMOVE()
	/*ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	*/
END_MESSAGE_MAP()



void CMapEditorView::Zoom(){
	CRect rect;
	CSize size;
	CPoint point;
	int z = rack->Zoom(7);
	if ( z != 0){
				
		this->GetClientRect(&rect);
		point = this->GetScrollPosition();
		
	
		point.x += z * (point.x +  rect.Width()  /2) /rack->scale ;
		point.y += z * (point.y +  rect.Height() /2) /rack->scale ;
		
		Invalidate(false);
		SetScrollSizes(MM_TEXT,rack->getSize());
		ScrollToPosition((POINT) point);
		Invalidate(false);
		
	}
};

void CMapEditorView::unZoom(){
	

	
	CSize size;
	CPoint point;
	CRect rect;
	int z = rack->unZoom(7);
	if ( z != 0){
				
		GetClientRect(&rect);
		point = GetScrollPosition();

		point.x = max(0,(int) point.x + z * (int )((point.x +  rect.Width()  /2) /rack->scale));//neni to presne.. melo by se udelat stejne jako zoom
		point.y = max(0, (int) point.y + z * (int )((point.y +  rect.Height() /2) /rack->scale));
		
		Invalidate(false);
		SetScrollSizes(MM_TEXT,rack->getSize());
		ScrollToPosition((POINT) point);
		Invalidate(false);
		
	}

};


// CMapEditorView construction/destruction

void CMapEditorView::OnActivateView(   BOOL bActivate,CView* pActivateView,CView* pDeactiveView ){
	CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);
	downPoint.x=-1;
	downPoint.y=-1;
}

void CMapEditorView::OnInitialUpdate(){
	
	CScrollView::OnInitialUpdate();
	CMapEditorDoc* pDoc = GetDocument();
	rack->init(pDoc);
	int sc;
	sc = 27;
	downPoint.x=-1;
	downPoint.y=-1;
	rack->setSize(pDoc->pGameMap->sizeX,pDoc->pGameMap->sizeY);
	rack->rescale(sc);
	SetScrollSizes(MM_TEXT,rack->getSize(),CSize(100,100),CSize(100,100));
	//this->ActivateTopParent();
	SetActiveWindow();
	//this->ActivateTopParent();
	//SetActiveWindow();
	//EnableActiveAccessibility();
	GetParentFrame()->SetActiveView(this,1);
	
}

CMapEditorView::CMapEditorView()
{
	// TODO: add construction code here

}

CMapEditorView::~CMapEditorView()
{
	SAFE_DELETE(rack);
}


BOOL CMapEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	rack = new CRack();

	return CView::PreCreateWindow(cs);
}

// CMapEditorView drawing

void CMapEditorView::OnDraw(CDC *pDC)
{
	SetActiveWindow();
	CMapEditorDoc* pDoc = GetDocument();
	pDoc->SetTitle();
	if (!pDoc)		return;
	rack->setSize(pDoc->pGameMap->sizeX,pDoc->pGameMap->sizeY);
	SetScrollSizes(MM_TEXT,rack->getSize());
	CSize size; 
	CPoint pos(0,0);
	CBitmap bmp;
	CDC dcMemory;
	CRect rect;	
	int ret;
	
	OnPrepareDC(pDC);
	pDC->DPtoLP(&pos);

    //
	
	//pDC->GetClipBox(&rect);
	ret = pDC->GetClipBox(&rect);
	
	GetClientRect(&rect);
	rect=rect+pos;
	if(ret==ERROR){
		ret=-100;
//		GetWindowRect(&rect);
//		ScreenToClient(&rect);
	}
	if(ret==COMPLEXREGION){
		ret=-100;
	}
	if(ret==SIMPLEREGION){
		ret=-100;
	}
	if(ret==NULLREGION ){
		ret=-100;
		}
	bmp.CreateCompatibleBitmap(pDC,rect.Width(),rect.Height());//rect.Width(),rect.Height());

	
	dcMemory.CreateCompatibleDC(pDC);
	dcMemory.SelectObject(&bmp);
	
	rack->paint(&dcMemory,pDoc,rect,this);	
	pDC->BitBlt(rect.left,rect.top,rect.Width(),rect.Height(), &dcMemory,0, 0, SRCCOPY);

	
	/*
	GetClientRect(&rect);
	rack->setSize(pDoc->pGameMap->sizeX,pDoc->pGameMap->sizeY);
	rack->paint(pDC,pDoc,&rect);	
	*/
}


// CMapEditorView printing
/*
BOOL CMapEditorView::OnPreparePrinting(CPrintInfo* pInfo)
{	
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CMapEditorView::OnBeginPrinting(CDC* , CPrintInfo* )
{
	// TODO: add extra initialization before printing
}

void CMapEditorView::OnEndPrinting(CDC* , CPrintInfo* )
{
	// TODO: add cleanup after printing
}

*/
// CMapEditorView diagnostics

void CMapEditorView::OnChar(UINT c,UINT n,UINT f){
	CMapEditorDoc* pDoc = GetDocument();
	if (!pDoc)		return;
	switch(c){
	case '+':
		Zoom();
		break;

	case '-':
		unZoom();
		break;
	case ' ':
	case 'R':
	case 'r':
		if(pDoc->pSelObject){
			pDoc->selObjectOr = (ORIENTATION) (((int) pDoc->selObjectOr +1)%4) ;//(pDoc->selObjectOr +1)%4;
			GetParent()->Invalidate();
			Invalidate(false);
			};
		break;
	}
	
};

BOOL CMapEditorView::OnMouseWheel(UINT fFlags,short zDelta,CPoint point){
	CMapEditorDoc* pDoc = GetDocument();
	if (!pDoc)		return TRUE;


		if(pDoc->pSelObject){
				if(zDelta>0)	
					pDoc->selObjectOr = (ORIENTATION) (((int) pDoc->selObjectOr +1)%4) ;
				if(zDelta<0)
					pDoc->selObjectOr = (ORIENTATION) (((int) pDoc->selObjectOr +3)%4) ;
				GetParent()->Invalidate();
		};
	return TRUE;

}

void CMapEditorView::OnMouseMove(UINT nFlags, CPoint point){
	CMapEditorDoc* pDoc = GetDocument();
	if (!pDoc)		return;
	CPoint absPos=point; 
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);
	CPoint pos;
	CRect rect;
	pos = rack->mouseToMap(point);
	
	GetClientRect(&rect);
	
	
	if(downPoint.x == -1 && (absPos.x < rect.left || absPos.x > rect.right || absPos.y<rect.top || absPos.y>rect.bottom)){
		ReleaseCapture();
		pDoc->selObjectPos.x=-100;
		pDoc->selObjectPos.y=-100;
		Invalidate(false);
	}
	else 
	if(pDoc->selObjectPos.x*rack->scale > (ULONG) point.x || (pDoc->selObjectPos.x+1)*(rack->scale) < (ULONG) point.x ||
		pDoc->selObjectPos.y*rack->scale > (ULONG) point.y || (pDoc->selObjectPos.y+1)*(rack->scale) < (ULONG) point.y){
			pDoc->selObjectPos.x = point.x / rack->scale;
			pDoc->selObjectPos.y = point.y / rack->scale;
			SetCapture();
			Invalidate(false);
		}
	

	if(downPoint.x!=-1){
		mousePos=pos;
		
		Invalidate(false);
	}

}
void CMapEditorView::OnMButtonDown(UINT nFlags,CPoint point){
	
	CMapEditorDoc* pDoc = GetDocument();
	if (!pDoc)		return;
	CScrollView::OnMButtonDown(nFlags, point);	
	
	switch(pDoc->bEditState){
		case STATE_MAP_EDIT:
			
			if(pDoc->pSelObject){
			pDoc->selObjectOr = (ORIENTATION) (((int) pDoc->selObjectOr +1)%4) ;//(pDoc->selObjectOr +1)%4;
			GetParent()->Invalidate();
			};
		break;
			
	};
	

};

void CMapEditorView::OnLButtonDown(UINT nFlags,CPoint point){
	
	
	CMapEditorDoc* pDoc = GetDocument();
	if (!pDoc)		return;
	CScrollView::OnLButtonDown(nFlags, point);
	SetActiveWindow();
	CPoint pos;
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);

	
	pos = rack->mouseToMap(point);
	CMapMainObject * pMMobj;

	switch(pDoc->bEditState){
	case STATE_MAP_EDIT:
		downPoint=pos;
		mousePos = pos;
		if(pDoc->pSelObject){
			if(pDoc->pSelObject->bStartingPosition){
				if(!pDoc->pGameMap->setStartToMap(pos.x,pos.y,pDoc->selObjectOr,pDoc->pSelObject)){
					pDoc->bTrackChanged=TRUE;pDoc->bMapNotSaved=TRUE;	
				};
			}
			else {
				if(pDoc->pGameMap->setObjectToMap(pos.x,pos.y,pDoc->selObjectOr,pDoc->pSelObject)){
					pDoc->bTrackChanged=TRUE;pDoc->bMapNotSaved=TRUE;	
				};
			}
		} else{
			pMMobj = pDoc->pGameMap->getMainObjectInPos(pos.x,pos.y);
			
			if(pDoc->pGameMap->posStart.x == pos.x && pDoc->pGameMap->posStart.y == pos.y){
				pDoc->pSelObject=pDoc->pGameMap->pMOStartPosition;
				pDoc->selObjectOr = pDoc->pGameMap->posStart.or;
				pDoc->pGameMap->pMOStartPosition=NULL;
				pDoc->bTrackChanged=TRUE;pDoc->bMapNotSaved=TRUE;	
			}else if(pMMobj) {
				pDoc->selObjectOr = pDoc->pGameMap->getObjectOrientation(pos.x,pos.y);
				pDoc->pSelObject = pDoc->pGameMap->deleteObjectFromMap(pos.x,pos.y);
				pDoc->bTrackChanged=TRUE;pDoc->bMapNotSaved=TRUE;	
		
			}
		}

		break;
	
	case  STATE_TERRAIN_EDIT:
		rack->increaseAltitude(pos);
		downPoint=pos;
		mousePos = pos;
		break;
	case STATE_PATH_EDIT:
		pDoc->pGameMap->FindPathFromPos(pos);
		pDoc->bMapNotSaved=TRUE;
		break;
	}

	Invalidate(false);
	
}

void CMapEditorView::OnLButtonUp(UINT nFlags,CPoint point){
	CMapEditorDoc* pDoc = GetDocument();
	if (!pDoc)		return;
	ReleaseCapture();
	CScrollView::OnLButtonUp(nFlags, point);
	CPoint pos;
	int i,j;
	UINT alt,alt2;
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);	
	pos = rack->mouseToMap(point);

	switch(pDoc->bEditState){
	
	case STATE_MAP_EDIT:
		if(downPoint.x>=0 && downPoint.y>=0)
		if(pDoc->pSelObject && !pDoc->pSelObject->bStartingPosition){
		for(i=0;i <= abs(downPoint.y-pos.y);i++)
			for(j = 0; j <= abs(downPoint.x-pos.x);j++)
				if(!(i==0 && j ==0)){
					if(pDoc->pGameMap->setObjectToMap(downPoint.x+ (downPoint.x < pos.x ? j:-j),downPoint.y+(downPoint.y< pos.y ? i:-i),pDoc->selObjectOr,pDoc->pSelObject)){
						pDoc->bMapNotSaved=TRUE;
						if(!pDoc->pSelObject->GetWaypointSetSize())
							pDoc->bTrackChanged=TRUE;

					};
				}
		}
		break;
	case  STATE_TERRAIN_EDIT:
		if(downPoint.x>=0 && downPoint.y>=0)
		for(i=0;i <= abs(downPoint.y-pos.y);i++)
			for(j = 0; j <= abs(downPoint.x-pos.x);j++)
				if(!(i==0 && j ==0)){
					alt2=pDoc->pGameMap->getAltitude(downPoint.x+ (downPoint.x < pos.x ? j:-j),downPoint.y+ (downPoint.y < pos.y ? i:-i));
					if(alt > alt2){
						rack->increaseAltitude(downPoint+CPoint(downPoint.x < pos.x ? j:-j,downPoint.y < pos.y ? i:-i));
						pDoc->bMapNotSaved=TRUE;
					}
				}
				else{
					alt=pDoc->pGameMap->getAltitude(downPoint.x,downPoint.y);
				}
				
		break;
	
	}
	downPoint.x=-1;
	downPoint.y=-1;
	Invalidate(false);


}

void CMapEditorView::OnLButtonDblClk(UINT nFlags,CPoint point){
	CMapEditorDoc* pDoc = GetDocument();
	if (!pDoc)		return;
	CScrollView::OnLButtonDblClk(nFlags, point);
	CPoint pos;
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);
	pos = rack->mouseToMap(point);
	downPoint=pos;

	switch(pDoc->bEditState){
	case STATE_MAP_EDIT:
			CMapMainObject * pMMobj;
			//pDoc->pSelObject=NULL;
			if(pDoc->pGameMap->posStart.x == pos.x && pDoc->pGameMap->posStart.y == pos.y){
				pDoc->pSelObject=pDoc->pGameMap->pMOStartPosition;
				pDoc->selObjectOr = pDoc->pGameMap->posStart.or;
				pDoc->pGameMap->pMOStartPosition=NULL;
				pDoc->bMapNotSaved=TRUE;pDoc->bTrackChanged=TRUE;
			}else{
				pMMobj = pDoc->pGameMap->getMainObjectInPos(pos.x,pos.y);
				if(pMMobj) {
					pDoc->selObjectOr = pDoc->pGameMap->getObjectOrientation(pos.x,pos.y);
					pDoc->pSelObject = pDoc->pGameMap->deleteObjectFromMap(pos.x,pos.y);
					pDoc->bMapNotSaved=TRUE;pDoc->bTrackChanged=TRUE;
					
				}
			}

			GetParent()->Invalidate(false);
			break;
	}
	Invalidate(false);
}

void CMapEditorView::OnRButtonDown(UINT nFlags,CPoint point){
	CMapEditorDoc* pDoc = GetDocument();
	if (!pDoc)		return;
	SetCapture();
	CScrollView::OnRButtonDown(nFlags, point);
	CPoint pos;
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);
	pos = rack->mouseToMap(point);

	switch(pDoc->bEditState){
	case STATE_MAP_EDIT:
			CMapMainObject * pMMobj;
			//pDoc->pSelObject=NULL;
			pMMobj = pDoc->pGameMap->getMainObjectInPos(pos.x,pos.y);
			if(pMMobj) {
				//pDoc->selObjectOr = up;//pDoc->pGameMap->getObjectOrientation(pos.x,pos.y);
				/*pDoc->pSelObject =*/ pDoc->pGameMap->deleteObjectFromMap(pos.x,pos.y);
				//pDoc->pSelObject= NULL;
				pDoc->bMapNotSaved=TRUE;pDoc->bTrackChanged=TRUE;					
			} 
			/*else {
				if(pDoc->pSelObject){
				pDoc->selObjectOr = (ORIENTATION) (((int) pDoc->selObjectOr +1)%4) ;//(pDoc->selObjectOr +1)%4;
				
				}
			}*/
			GetParent()->Invalidate();
			downPoint=pos;
			mousePos = pos;
			break;
	
	case  STATE_TERRAIN_EDIT:		
			
		rack->decreaseAltitude(pos);
		downPoint=pos;
		mousePos = pos;
		break;
	case STATE_PATH_EDIT:
		pDoc->pGameMap->FindPathBack(pos);
		pDoc->bMapNotSaved=TRUE;
		break;
	}
	
	Invalidate(false);
}

void CMapEditorView::OnRButtonUp(UINT nFlags, CPoint point){
		CMapEditorDoc* pDoc = GetDocument();
	if (!pDoc)		return;
	ReleaseCapture();
	CScrollView::OnRButtonUp(nFlags, point);
	CPoint pos;
	int i,j,alt,alt2;
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);	
	pos = rack->mouseToMap(point);

	switch(pDoc->bEditState){
	case STATE_MAP_EDIT:
		if(downPoint.x >=0 && downPoint.y>=0)
		for(i=0;i <= abs(downPoint.y-pos.y);i++)
			for(j = 0; j <= abs(downPoint.x-pos.x);j++)
				
				if(!(i==0 && j ==0)){
					pDoc->pGameMap->deleteObjectFromMap(downPoint.x+(downPoint.x < pos.x ? j:-j),downPoint.y + (downPoint.y < pos.y ? i:-i));	
					pDoc->bMapNotSaved=TRUE;pDoc->bTrackChanged=TRUE;
				}
		break;
	case  STATE_TERRAIN_EDIT:
		if(downPoint.x >=0 && downPoint.y>=0)
		for(i=0;i <= abs(downPoint.y-pos.y);i++)
			for(j = 0; j <= abs(downPoint.x-pos.x);j++)
				
				if(!(i==0 && j ==0)){
					alt2=pDoc->pGameMap->getAltitude(downPoint.x+(downPoint.x < pos.x ? j:-j),downPoint.y + (downPoint.y < pos.y ? i:-i));
					if(alt2 > alt)
						rack->decreaseAltitude(downPoint+CPoint(downPoint.x < pos.x ? j:-j,downPoint.y < pos.y ? i:-i));
				}else{
					alt=pDoc->pGameMap->getAltitude(downPoint.x,downPoint.y);
				}
				
		break;
	}
	downPoint.x=-1;
	downPoint.y=-1;
	Invalidate(false);

}

#ifdef _DEBUG
void CMapEditorView::AssertValid() const
{
	CView::AssertValid();
}

void CMapEditorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMapEditorDoc* CMapEditorView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMapEditorDoc)));
	return (CMapEditorDoc*)m_pDocument;
}
#endif //_DEBUG


// CMapEditorView message handlers

CRack::CRack(UINT _scale,UINT _px,UINT _py){
	scale = _scale;
	px = _px;
	py = _py;
};

int CRack::Zoom(int step){
	
	if (scale < MAX_ZOOM) {
		scale+= step;
		return step;
		
	}else {return 0;}
	
}
int CRack::unZoom(int step){
	
	if ( int (scale-step) > MIN_ZOOM) {
		scale -= step;
		return -step;
		
	}else{ return 0;}
	
}



CSize CRack::getSize(){
	CSize size(px*scale,py*scale);
	return size;
};

CPoint CRack::mouseToMap(CPoint pos){
	CPoint ret;
	
	if (pos.x > (int) (px*scale) ) ret.x=px-1;
	else if(pos.x<0) 
		ret.x=0;
	else 
		ret.x =  pos.x / scale;

	if (pos.y > (int) (py*scale)) ret.y=0;
	else if(pos.y<0) ret.y=py-1;
	else ret.y =  py - pos.y /scale - 1 ;
	return ret;
}

void CRack::increaseAltitude(UINT x,UINT y){
	CPoint point;
	point.x =x;
	point.y = y;
	increaseAltitude(point);


}

//////////////////////////////////////////////////////////////////////
//Prohlidne pole okolo bodu a vrati body ke zvyseni v poli. 
/////////////////////////////////////////////////////////////////////
void CRack::ToIncrease(CPoint point, int (&pole)[3][3]){
	
	
	memset(pole,0,9 * sizeof(int));
	UINT alt = pDoc->pGameMap->getAltitude(point.x,point.y);
	//pole[1][1]=1;
	if ( alt+1 < MAX_HEIGHT){ 
			alt = alt  + 1; 
	};
	// TEST verticalne a horizontalne "téèko"
	for(int j = 0; j <4;j++){
			
			for(int i = -1;i <=1 ;i++){

				switch(j){
					case 0:
						if ( pDoc->pGameMap->getAltitude(point.x+i,point.y+2) >= alt && 
							pDoc->pGameMap->getAltitude(point.x,point.y+1) == (alt - 1) &&
							pDoc->pGameMap->getAltitude(point.x+i,point.y+1) == (alt - 1))
							pole[2][1]=1;
						
						break;
						
					case 1:
						if ( pDoc->pGameMap->getAltitude(point.x+2,point.y+i) >= alt && 
							pDoc->pGameMap->getAltitude(point.x+1,point.y) == (alt - 1) &&
							pDoc->pGameMap->getAltitude(point.x+1,point.y+i) == (alt - 1))
							pole[1][2]=1;
						break;
					case 2:
						if ( pDoc->pGameMap->getAltitude(point.x+i,point.y-2) >= alt && 
							pDoc->pGameMap->getAltitude(point.x,point.y-1) == (alt - 1)&&
							pDoc->pGameMap->getAltitude(point.x+i,point.y-1) == (alt - 1))
							pole[0][1]=1;
							
						break;
					case 3:
						if ( pDoc->pGameMap->getAltitude(point.x-2,point.y+i) >= alt && 
							pDoc->pGameMap->getAltitude(point.x-1,point.y) == (alt - 1) &&
							pDoc->pGameMap->getAltitude(point.x-1,point.y+i) == (alt - 1))
							pole[1][0]=1;
							
				}
			}
	}
	///////////////////////////////////////////
	// test diagonalne + elko
	for(int j = 0; j <4;j++){
			switch(j){
				case 0:
						if ( pDoc->pGameMap->getAltitude(point.x+2,point.y+2) >= alt && 
							pDoc->pGameMap->getAltitude(point.x+1,point.y+1) == (alt - 1))
							
							if(pDoc->pGameMap->getAltitude(point.x+1,point.y)>=alt && pDoc->pGameMap->getAltitude(point.x+2,point.y+1)>=alt &&
								pDoc->pGameMap->getAltitude(point.x,point.y+1)>=alt && pDoc->pGameMap->getAltitude(point.x+1,point.y+2)>=alt){
								pole[2][2]=1;
							}else if(!((pDoc->pGameMap->getAltitude(point.x+1,point.y)>=alt && pDoc->pGameMap->getAltitude(point.x+2,point.y+1)>=alt) ||
							(pDoc->pGameMap->getAltitude(point.x,point.y+1)>=alt && pDoc->pGameMap->getAltitude(point.x+1,point.y+2)>=alt)))
								pole[2][2]=1;
						
						break;
							
					break;
				case 1:
						if ( pDoc->pGameMap->getAltitude(point.x+2,point.y-2) >= alt && 
							pDoc->pGameMap->getAltitude(point.x+1,point.y-1) == (alt - 1))

							if(pDoc->pGameMap->getAltitude(point.x,point.y-1)>=alt && pDoc->pGameMap->getAltitude(point.x+1,point.y-2)>=alt &&
								pDoc->pGameMap->getAltitude(point.x+1,point.y)>=alt && pDoc->pGameMap->getAltitude(point.x+2,point.y-1)>=alt){
								pole[0][2]=1;
							}else if(!((pDoc->pGameMap->getAltitude(point.x,point.y-1)>=alt && pDoc->pGameMap->getAltitude(point.x+1,point.y-2)>=alt) ||
							(pDoc->pGameMap->getAltitude(point.x+1,point.y)>=alt && pDoc->pGameMap->getAltitude(point.x+2,point.y-1)>=alt)))
							pole[0][2]=1;
							
					
					break;
				case 2:
							if ( pDoc->pGameMap->getAltitude(point.x-2,point.y-2)>= alt && 
								pDoc->pGameMap->getAltitude(point.x-1,point.y-1) == (alt - 1))
							if (pDoc->pGameMap->getAltitude(point.x-1,point.y)>=alt && pDoc->pGameMap->getAltitude(point.x-2,point.y-1)>=alt &&
								pDoc->pGameMap->getAltitude(point.x,point.y-1)>=alt && pDoc->pGameMap->getAltitude(point.x-1,point.y-2)>=alt){
								pole[0][0]=1;
							}else if(!((pDoc->pGameMap->getAltitude(point.x-1,point.y)>=alt && pDoc->pGameMap->getAltitude(point.x-2,point.y-1)>=alt) ||
							(pDoc->pGameMap->getAltitude(point.x,point.y-1)>=alt && pDoc->pGameMap->getAltitude(point.x-1,point.y-2)>=alt)))
							pole[0][0]=1;
							
					
					break;
				case 3:
						if ( pDoc->pGameMap->getAltitude(point.x-2,point.y+2)>= alt && 
							pDoc->pGameMap->getAltitude(point.x-1,point.y+1) == (alt - 1))
						if (pDoc->pGameMap->getAltitude(point.x-1,point.y)>=alt && pDoc->pGameMap->getAltitude(point.x-2,point.y+1)>=alt &&
							pDoc->pGameMap->getAltitude(point.x,point.y+1)>=alt && pDoc->pGameMap->getAltitude(point.x-1,point.y+2)>=alt){
							pole[2][0]=1;
						}else if(!((pDoc->pGameMap->getAltitude(point.x-1,point.y)>=alt && pDoc->pGameMap->getAltitude(point.x-2,point.y+1)>=alt) ||
							(pDoc->pGameMap->getAltitude(point.x,point.y+1)>=alt && pDoc->pGameMap->getAltitude(point.x-1,point.y+2)>=alt)))
							pole[2][0]=1;
							
					
					break;

			}

			
			
	}
	

}

void CRack::increaseAltitude(CPoint point){
	
	int alt,i,j,alt2;
	CPoint n;
	if( test_border(point.x,point.y)){
		alt = pDoc->pGameMap->getAltitude(point.x,point.y);
		if ( alt+1 < MAX_HEIGHT){ 
			alt = alt  + 1; 
			pDoc->pGameMap->setAltitude(point.x,point.y,alt);
		}

		for(i = -1;i <= 1 ;i++)
			for(j = -1; j<=1;j++){
				
				if( (i!=0 || j!=0) && test_border(point.x + j,point.y+i)){
					
					n.x=point.x + j;n.y=point.y+i;
					alt2= pDoc->pGameMap->getAltitude(point.x + j,point.y+i);
					if( (alt - alt2) > 1)
						increaseAltitude(n);
				}		

			}
		//TEST 
		
		for(j = 0; j <4;j++){
			//nastavim roh
			
			for(i = -1;i <=1 ;i++){

				switch(j){
					case 0:
						if ( pDoc->pGameMap->getAltitude(point.x+i,point.y+2) == alt && 
							pDoc->pGameMap->getAltitude(point.x,point.y+1) == (alt - 1) &&
							pDoc->pGameMap->getAltitude(point.x+i,point.y+1) == (alt - 1))
							increaseAltitude(point.x,point.y+1);
						break;
						
					case 1:
						if ( pDoc->pGameMap->getAltitude(point.x+2,point.y+i) == alt && 
							pDoc->pGameMap->getAltitude(point.x+1,point.y) == (alt - 1) &&
							pDoc->pGameMap->getAltitude(point.x+1,point.y+i) == (alt - 1))
							increaseAltitude(point.x+1,point.y);
						break;
					case 2:
						if ( pDoc->pGameMap->getAltitude(point.x+i,point.y-2) == alt && 
							pDoc->pGameMap->getAltitude(point.x,point.y-1) == (alt - 1)&&
							pDoc->pGameMap->getAltitude(point.x+i,point.y-1) == (alt - 1))
							increaseAltitude(point.x,point.y-1);
						break;
					case 3:
						if ( pDoc->pGameMap->getAltitude(point.x-2,point.y+i) == alt && 
							pDoc->pGameMap->getAltitude(point.x-1,point.y) == (alt - 1) &&
							pDoc->pGameMap->getAltitude(point.x-1,point.y+i) == (alt - 1))
							increaseAltitude(point.x-1,point.y);
				}
			}
		}
		for(j = 0; j <4;j++){
			switch(j){
				case 0:
						if ( pDoc->pGameMap->getAltitude(point.x+2,point.y+2) == alt && 
							pDoc->pGameMap->getAltitude(point.x+1,point.y+1) == (alt - 1)&& 
							!( (pDoc->pGameMap->getAltitude(point.x+1,point.y)==alt && pDoc->pGameMap->getAltitude(point.x+2,point.y+1)==alt) ||
							(pDoc->pGameMap->getAltitude(point.x,point.y+1)==alt && pDoc->pGameMap->getAltitude(point.x+1,point.y+2)==alt))

							)

							increaseAltitude(point.x+1,point.y+1);
					break;
				case 1:
						if ( pDoc->pGameMap->getAltitude(point.x+2,point.y-2) == alt && 
							pDoc->pGameMap->getAltitude(point.x+1,point.y-1) == (alt - 1) && 
							!( (pDoc->pGameMap->getAltitude(point.x,point.y-1)==alt && pDoc->pGameMap->getAltitude(point.x+1,point.y-2)) ||
							(pDoc->pGameMap->getAltitude(point.x+1,point.y)==alt && pDoc->pGameMap->getAltitude(point.x+2,point.y-1))))
							increaseAltitude(point.x+1,point.y-1);
					
					break;
				case 2:
							if ( pDoc->pGameMap->getAltitude(point.x-2,point.y-2) == alt && 
							pDoc->pGameMap->getAltitude(point.x-1,point.y-1) == (alt - 1) && 
							!( (pDoc->pGameMap->getAltitude(point.x-1,point.y)==alt && pDoc->pGameMap->getAltitude(point.x-2,point.y-1)==alt) ||
							(pDoc->pGameMap->getAltitude(point.x,point.y-1)==alt && pDoc->pGameMap->getAltitude(point.x-1,point.y-2)==alt)))
							increaseAltitude(point.x-1,point.y-1);
					
					break;
				case 3:
						if ( pDoc->pGameMap->getAltitude(point.x-2,point.y+2) == alt && 
							pDoc->pGameMap->getAltitude(point.x-1,point.y+1) == (alt - 1) && 
							!((pDoc->pGameMap->getAltitude(point.x-1,point.y)==alt && pDoc->pGameMap->getAltitude(point.x-2,point.y+1)==alt) ||
							(pDoc->pGameMap->getAltitude(point.x,point.y+1)==alt && pDoc->pGameMap->getAltitude(point.x-1,point.y+2)==alt)))
							increaseAltitude(point.x-1,point.y+1);
					
					break;

			}

			
		}
	}
}
//void CRack::decreaseAltitudeCheck(UINT x,UINT y,
void CRack::decreaseAltitude(UINT x,UINT y){
	CPoint point;
	point.x =x;
	point.y = y;
	decreaseAltitude(point);


}

void CRack::decreaseAltitude(CPoint point,BOOL decreased){
	

	/* 
	1)snizim vse okolo bodu s rozdilem vyšek 2
	2)uložím si okolí bodu
	3)Najdu ideální cestu v okolí bodu- cesta která zeèínám bodem, který je nejvíce snížen
	4)projdu okolí bodu podle cesty, každý bod v okolí snížím a pokusím se ho zvednout, pokud zvednutím se zvedne i bod,
	který už byl snížen nechám ho sníženej, jinak vrátím. 
	5)body ke snížení uložím do toDecrease a pak na nì zavolám snížení
	*/
	int alt,j,i,alt2,r,r1,r2,i1,j1,ip,jp;
	CPoint n,n2;

	alt = pDoc->pGameMap->getAltitude(point.x,point.y);	
	int toIncrease[3][3];
	BYTE t[4],tmax,maxid;
	BYTE altArr[3][3];
	BYTE toDecrease[3][3];
	
	int path[8][2]={ {0,-1},{-1,0},{-1,-1},{0,1},{-1,1},{1,0},{1,1},{1,-1}}; //x y
	
	int pathv[4][8][2]={ { {0,-1},{-1,0},{-1,-1},{0,1},{-1,1},{1,0},{1,1},{1,-1} }, //x y
					     { {-1,0},{0,1},{-1,1},{1,0},{1,1},{0,-1},{1,-1},{-1,-1} },
					     { {0,1},{1,0},{1,1},{0,-1},{1,-1},{-1,0},{-1,-1},{-1,1} },
					     { {1,0},{0,-1},{1,-1},{-1,0},{-1,-1},{0,1},{-1,1},{1,1} }
					    };

	if( test_border(point.x,point.y) && ((alt > 0) || (alt >=0 && decreased)) ){
		if(!decreased){
			alt = alt  - 1; 	
			pDoc->pGameMap->setAltitude(point.x,point.y,alt);
		}
		for(r = 0;r < 8 ;r++){
				j = path[r][0];
				i = path[r][1];
				//pole[i+1][j+1] =  pDoc->pGameMap->getAltitude(point.x + j,point.y+i);
				if( test_border(point.x + j,point.y+i)){
				
					n.x=point.x + j;n.y=point.y+i;
					alt2= pDoc->pGameMap->getAltitude(point.x + j,point.y+i);
				
					if(  (alt - alt2) < -1) 
						decreaseAltitude(n);
			
				}
			}
	
	}
	
	memset(toDecrease,0,9 * sizeof(BYTE));
	memset(altArr,0,9 * sizeof(BYTE));
	for(i = -1;i <=1;i++) //naplneni pole ulozeni, co je nahore a co dole
		for(j=-1;j<=1;j++){
			if( test_border(point.x + j ,point.y+i)){
				alt2 = pDoc->pGameMap->getAltitude(point.x+ j,point.y+i);	
				if(alt2 <= alt) {
					altArr[i+1][j+1] = 1;
				}
			}
	}
	
	memset(t,0,4);
	for(r1=0;r1<4;r1++){
		i1=pathv[r1][0][1];j1=pathv[r1][0][0];
		
			for(r2=0;r2<4;r2++){
				ip=pathv[r2][0][1];jp=pathv[r2][0][0];
					if( (i1+ip) >=-1 && (i1+ip)<=1 && 
						(j1+jp) >=-1 && (j1+jp)<=1 &&
						altArr[i1+ip+1][j1+jp+1])
						t[r1]++;

		}
	}
	tmax = 0;
	maxid=0;
	for(r1=0;r1<4;r1++){
		if(t[r1] > tmax){
			maxid = r1;
			tmax = t[r1];
		}
	}


	for(r = 0;r < 8 ;r++){
			j = pathv[maxid][r][0];
			i = pathv[maxid][r][1];
			if( test_border(point.x + j ,point.y+i)){
				alt2 = pDoc->pGameMap->getAltitude(point.x+ j,point.y+i);	
				if(alt2 > alt) {
					pDoc->pGameMap->setAltitude(point.x+ j ,point.y+i,alt);
					n.x=point.x + j;n.y=point.y+i;
					ToIncrease(n,toIncrease);
				

					for(r1=0;r1<=8;r1++){
							i1 =  path[r1][1];j1 = path[r1][0];	
							ip = i+i1;
							jp = j+j1;
							//if(!(j1==0 && i1==0))
							if( ip>=-1 && ip<=1 && jp>=-1 && jp<=1 && toIncrease[i1+1][j1+1] && altArr[ip+1][jp+1]){
								toDecrease[i+1][j+1]=1;
								altArr[i+1][j+1] = 1;
								break;
							}
					}
					
				//		decreaseAltitude(n);
				} 
				if(!toDecrease[i+1][j+1]) pDoc->pGameMap->setAltitude(point.x+ j ,point.y+i,alt2);
					
			}	

	}
			
	for(i = 0;i <= 2;i++)
		for(j = 0; j<=2;j++){
			if(toDecrease[i][j]){
				n.x=point.x + j -1;n.y=point.y+i-1;
				decreaseAltitude(n,true);
			}
		
		
		}

	
}


void CRack::countImagePlg(BYTE sc, ORIENTATION or,CPoint * points){
	
	//UINT sc = (scale)*size;
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
void CRack::paintWaypoints(CDC * pDC,MAPPOSITION mapPos,CMainObject * pMO,CRect view){
	CPen pen,penbig;
	pen.CreatePen(PS_SOLID,1,COLOR_WAYPOINT);
	penbig.CreatePen(PS_SOLID,4,COLOR_WAYPOINT);

	UINT wsID,wID;
	WAYPOINT waypoint;
	int xs,ys;
	if (pMO){
		for(wsID = 0;wsID < pMO->GetWaypointSetSize();wsID++){
			if (pMO->GetWaypointSize(wsID) > 0){
				waypoint = pMO->GetWaypoint(wsID,0,mapPos.or);		
				xs =(int)(waypoint.x*scale/TERRAIN_PLATE_WIDTH); 
				ys =(int)(pMO->size*scale - (waypoint.z*scale/TERRAIN_PLATE_WIDTH));
		//		ys = py*scale - ys;
				xs += scale * (mapPos.x-pMO->getShift())-view.left;
				ys += scale * (mapPos.y-(pMO->size-1-pMO->getShift()))-view.top;
				pDC->MoveTo(xs,ys);
			}
			pDC->SelectObject(penbig);
			for(wID=0;wID<pMO->GetWaypointSize(wsID);wID++){
				waypoint = pMO->GetWaypoint(wsID,wID,mapPos.or);		
				xs =(int)( waypoint.x*scale/TERRAIN_PLATE_WIDTH); 
				ys =(int)( pMO->size*scale- (waypoint.z*scale/TERRAIN_PLATE_WIDTH));
		//		ys = py*scale - ys;
				xs += scale * (mapPos.x-pMO->getShift())-view.left;
				ys += scale * (mapPos.y-( pMO->size-pMO->getShift()-1))-view.top;
				
				pDC->LineTo(xs,ys);
				pDC->SelectObject(pen);
				
			}
		}
	}

}
void CRack::paintStartingPos(CDC * pDC,MAPPOSITION mapPos,CMainObject * pSP,CRect view){
	CPen   redpen;
	redpen.CreatePen(PS_SOLID,1,RGB(255,255,0));
	CGameObject * pGOStartPos;
	UINT i;
	
	int xp,yp,xs,ys,xt,yt;
	float r,t;
	float x,z,rot;
	pDC->SelectObject(redpen);
	if(mapPos.x >= 0 && mapPos.x < px && mapPos.y>=0 && mapPos.y < py){
		for(i = 0;i < pSP->obj.size();i++){
			pGOStartPos=&pSP->obj[i];
			switch(mapPos.or){
				case 0:
					x = (float) scale * mapPos.x + pGOStartPos->x*scale/TERRAIN_PLATE_WIDTH;
					z = (float) scale * mapPos.y + pGOStartPos->z*scale/TERRAIN_PLATE_WIDTH;
					break;
				case 1:
					x = (float)scale* mapPos.x +  pGOStartPos->z*scale/TERRAIN_PLATE_WIDTH;
					z = (float)scale* mapPos.y + scale - pGOStartPos->x*scale/TERRAIN_PLATE_WIDTH;
					break;
				case 2:
					x = (float)scale* mapPos.x +scale - pGOStartPos->x*scale/TERRAIN_PLATE_WIDTH;
					z = (float)scale* mapPos.y +scale - pGOStartPos->z*scale/TERRAIN_PLATE_WIDTH;
					break;
				case 3:
					x = (float)scale* mapPos.x + scale -  pGOStartPos->z*scale/TERRAIN_PLATE_WIDTH;
					z = (float)scale* mapPos.y + pGOStartPos->x*scale/TERRAIN_PLATE_WIDTH; 
					break;

			}
			rot = D3DX_PI/ 2 *mapPos.or + pGOStartPos->rotationY;
			
			
			r = scale*2 / TERRAIN_PLATE_WIDTH_M;
			
			xs =(int)( x-view.left); 
			ys =(int)((py*scale - z)-view.top);
			//pDC->Rectangle(xp,yp,xp+4,yp+2);ROUND
			pDC->MoveTo(xs,ys);
			t = cos(rot);
			xt = (int)(r*cos(rot));
			yt = (int)(r*sin(rot));
			
			xp = xs+xt;yp = ys+yt;
			pDC->LineTo(xp,yp);
			pDC->MoveTo(xs,ys);
			xp = xs-xt;yp = ys-yt;
			pDC->LineTo(xp,yp);
			pDC->MoveTo(xs,ys);
			xt = (int)(r*cos(rot+D3DX_PI/ 2));
			yt = (int)(r*sin(rot+D3DX_PI/ 2));
			xp = xs+xt;yp = ys+yt;
			pDC->LineTo(xp,yp);
		}
	}
	
	

	//Rot = D3DX_PI/ 2 * MapObj->iStartRotation + pGOStartPos->rotationY;
		

}
BOOL CRack::paint(CDC * pDC,CMapEditorDoc* pDoc,CRect view,CMapEditorView * pEditorView){
	
	
	CBrush rbrush(TRANSPARENT_COLOR);
	CBrush pathbrush(COLOR_PATHEDIT);
	CPen   pathpen;
	CPen   tpen;
	CPen   rpen;
	
	
	rpen.CreatePen(PS_DASHDOT,1,COLOR_RECTANGLE_SELECT);
	pathpen.CreatePen(PS_SOLID,0,COLOR_PATHEDIT);
	tpen.CreatePen(PS_SOLID,1,TRANSPARENT_COLOR);

	CImage * img;
	CImage pomImg,tmpImg,numberImg;
	CDC* pImgDC;
	CFont font;
	CMapMainObject * pMMO;
	MAPPOSITION mp;
	ORIENTATION or;
	CString number;
	UINT alt,size,posX,posY,palt;
	CRect rect,src;
	UINT i,j;
	int k;
	int corners[4];
	int shift;
	int edgex[4]= { 0,1,0,-1};
	int edgey[4]= { 1,0,-1,0};
	int cornx[4]= { -1,1,1,-1}; 
	int corny[4]={ 1,1,-1,-1};
	CPoint points[3];
	CRect position;
	
	//////////////////////////////////////////////////////////////////////////////////
	// Painting terrain
	//////////////////////////////////////////////////////////////////////////////////
	
	for(i = 0 ;i < py;i++)
		for(j= 0; j < px;j++){
			
			if(pDoc->pGameMap->getMainObjectInPos(j,py-i-1)){
				pDoc->pGameMap->unsetPaintedForObject(j,py-i-1);
			}
			alt = pDoc->pGameMap->getAltitude(j, py - i -1 );//x y
			
			for(k=0;k<4;k++) 
				corners[k]=0;
			for(k = 0; k < 4;k++){		
				palt = pDoc->pGameMap->getAltitude(j+edgex[k], py - i -1 + edgey[k]);
				if (alt < palt) 
					corners[k]=1;
				if (alt < palt) 
					corners[(k+1)%4]=1;
			}
			for(k = 0; k < 4;k++){		
				palt = pDoc->pGameMap->getAltitude(j+cornx[k], py - i -1 + corny[k]);
				if (alt < palt) 
					corners[k]=1;
			
			}			
			rect.SetRect(j*scale,i*scale,(j+1)*(scale)-1,(i + 1)*(scale)-1);
			rect.left -= view.left;	rect.right -=view.left;
			rect.top -= view.top; rect.bottom -=view.top;
			gProp.plates[alt][corners[0]][corners[1]][corners[2]][corners[3]]
				.TransparentBlt(pDC->m_hDC,(RECT &) rect,TRANSPARENT_COLOR);
			
	}
	//
	//////////////////////////////////////////////////////////////////////////////////////////
	//Painting objects
	/////////////////////////////////////////////////////////////////////////////////////////
	for( i = 0 ;i < py;i++)
		for( j= 0; j < px;j++){
			if( !pDoc->pGameMap->isObjectPainted(j,i)){ 
				
				if(	img = pDoc->pGameMap->getImage(j,i)){
					size = pDoc->pGameMap->getObjectSize(j,i);
					or = pDoc->pGameMap->getObjectOrientation(j,i);

					posX = pDoc->pGameMap->getObjectPosX(j,i);
					posY = py - pDoc->pGameMap->getObjectPosY(j,i) -1;
					mp.x = posX;mp.y = posY;mp.or = or;
					pomImg.Create(img->GetWidth(),img->GetHeight(),24);
					
					countImagePlg(img->GetWidth(),or,points);
					img->PlgBlt(pomImg.GetDC(),points);
					
					rect.SetRect(posX*scale,(posY-size+1)*scale,size*scale+posX*scale-1,size*scale+(posY-size+1)*scale-1);
					shift=(size-1)/2*scale;
					rect.left -= view.left+shift;
					rect.right -=view.left+shift;
					rect.top -= view.top-shift;
					rect.bottom -=view.top-shift;
					
					pomImg.TransparentBlt(pDC->m_hDC,(RECT &) rect,TRANSPARENT_COLOR);
					
					pomImg.ReleaseDC();
					pomImg.Destroy();
					if(pDoc->pGameMap->getMainObjectInPos(j,i)&&pDoc->pGameMap->getMainObjectInPos(j,i)->pMainObject)
						paintWaypoints(pDC,mp,pDoc->pGameMap->getMainObjectInPos(j,i)->pMainObject,view);
					pDoc->pGameMap->setPaintedForObject(j,i);
				}
			}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	//painting under mouse
	/////////////////////////////////////////////////////////////////////////////////////////////
		
	if (pDoc->bEditState == STATE_MAP_EDIT ){
		
		/*HTREEITEM item = pDoc->pTreeCtrl->GetSelectedItem();*/
		//CMainObject * pMainObject = pDoc->pSelObject;

		/*if(!pDoc->pTreeCtrl->ItemHasChildren(item)){
			pMainObject = (CMainObject *) pDoc->pTreeCtrl->GetItemData(item);
		*/	
			
		if(	pDoc->pSelObject && pDoc->pSelObject->size &&(img = pDoc->pSelObject->pImage)){
					
			size = pDoc->pSelObject->size;
			or = pDoc->selObjectOr;
			
			pomImg.Create(img->GetWidth(),img->GetHeight(),24);
			countImagePlg(img->GetWidth(),or,points);
			img->PlgBlt(pomImg.GetDC(),points);
			
			rect.SetRect(pDoc->selObjectPos.x*scale,(pDoc->selObjectPos.y-size+1)*scale,
				pDoc->selObjectPos.x*scale-1+size*scale,(pDoc->selObjectPos.y-size+1)*scale-1+size*scale);																														
			shift=(size-1)/2*scale;
			rect.left -= (view.left + shift);																										
			rect.right -= (view.left + shift);
			rect.top -=  (view.top - shift) ;
			rect.bottom -=(view.top - shift);
		
			pomImg.TransparentBlt(pDC->m_hDC, (RECT) rect,TRANSPARENT_COLOR);
			pomImg.ReleaseDC();
			pomImg.Destroy();
			
			mp.or=or;
			mp.x=pDoc->selObjectPos.x;
			mp.y=pDoc->selObjectPos.y;

			paintWaypoints(pDC,mp,pDoc->pSelObject,view);
			
			if(pDoc->pSelObject->bStartingPosition){
				MAPPOSITION startp;
				startp.or=pDoc->selObjectOr;startp.x=pDoc->selObjectPos.x,startp.y=py - 1 - pDoc->selObjectPos.y;
				paintStartingPos(pDC,startp,pDoc->pSelObject,view);
			}

		}
		
	
	}
	//////////////////////////////////////////////////////////////////////////////
	//painting start position
	//////////////////////////////////////////////////////////////////////////////
	if(	pDoc->pGameMap->pMOStartPosition && pDoc->pGameMap->pMOStartPosition->pImage){
					img = pDoc->pGameMap->pMOStartPosition->pImage;
					or = pDoc->pGameMap->posStart.or;

					posX = pDoc->pGameMap->posStart.x;
					posY = py - pDoc->pGameMap->posStart.y -1;

					CPoint points[3];
					CRect position;


					pomImg.Create(img->GetWidth(),img->GetHeight(),24);
					
					countImagePlg(img->GetWidth(),or,points);
					img->PlgBlt(pomImg.GetDC(),points);
					
					rect.SetRect(posX*scale,posY*scale,scale+posX*scale-1,scale+posY*scale-1);
					rect.left -= view.left;
					rect.right -=view.left;
					rect.top -= view.top;
					rect.bottom -=view.top;
					
					pomImg.TransparentBlt(pDC->m_hDC,(RECT &) rect,TRANSPARENT_COLOR);

					pomImg.ReleaseDC();
					pomImg.Destroy();
					paintStartingPos(pDC,pDoc->pGameMap->posStart,pDoc->pGameMap->pMOStartPosition,view);
					//pDoc->pGameMap->setPaintedForObject(j,i);
	}
	//////////////////////////////////////////////////////////////////////////////
	// Painting road course
	//////////////////////////////////////////////////////////////////////////////
	if(pDoc->bEditState == STATE_PATH_EDIT){
		
		for(i = 0; i < pDoc->pGameMap->vPath.size();i++)
			
			for(j = 0; j < pDoc->pGameMap->vPath[i].size();j++){
				pMMO = pDoc->pGameMap->GetObjectOnPath(i,j);
				if(!pMMO->pMainObject) break;
				pomImg.Create(10,10,24);
				pImgDC = CDC::FromHandle(pomImg.GetDC());
				pImgDC->SelectObject(pathbrush);pImgDC->SelectObject(pathpen);
				
				pImgDC->Rectangle(0,0,10,10);
				src.SetRect(0,0,10,10);				
		
				size = pMMO->pMainObject->size;
				
				or = pMMO->objOrient;
				posX = pMMO->posX;
				posY = py - pMMO->posY -1;
				//mp.x = posX;mp.y = posY;mp.or = or;
				
				rect.SetRect(posX*scale,(posY-size+1)*scale,size*scale+posX*scale-1,size*scale+(posY-size+1)*scale-1);
				shift=(size-1)/2*scale;
				rect.left -= view.left+shift;
				rect.right -=view.left+shift;
				rect.top -= view.top-shift;
				rect.bottom -=view.top-shift;
					
				pomImg.AlphaBlend(pDC->m_hDC, rect,src,70);
				pomImg.ReleaseDC();
				pomImg.Destroy();
				/////////////////////////
				// Paint number
				///////////////////////
				if(j == 0) {
					numberImg.Create(MAX_ZOOM*size,MAX_ZOOM*size,24);
					src.SetRect(0,0,MAX_ZOOM*size,MAX_ZOOM*size);
					pImgDC = CDC::FromHandle(numberImg.GetDC());
					font.CreateFont(MAX_ZOOM*size,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
						DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,NULL);
					
					pImgDC->SetBkColor(TRANSPARENT_COLOR);	
					pImgDC->SelectObject(tpen);pImgDC->SelectObject(rbrush);
					pImgDC->SetTextColor(COLOR_PATHEDIT_FONT);
					pImgDC->SelectObject(font);
					pImgDC->Rectangle(src);
					number.Format("%d",i+1 > 99 ? 0 : i+1);
					pImgDC->DrawText(number.GetString(),number.GetLength(),&src,DT_VCENTER | DT_CENTER |DT_SINGLELINE);
					numberImg.TransparentBlt(pDC->m_hDC,rect,TRANSPARENT_COLOR);
					font.DeleteObject();
					numberImg.ReleaseDC();
					numberImg.Destroy();
				}
				//pomImg.TransparentBlt(pDC->m_hDC,rect,TRANSPARENT_COLOR);
				
			}
		
		
		
	}
	//////////////////////////////////////////////////////////////////////////////
	// paint border 
	//////////////////////////////////////////////////////////////////////////////
	if((pDoc->bEditState == STATE_TERRAIN_EDIT || pDoc->bEditState == STATE_MAP_EDIT) && pEditorView->downPoint.x != -1 ){
		
		
		CPoint points[3];
		CRect position;
		CPoint polSize;
		int shiftx,shifty;
		int objshift,objsize=1;
		objshift=0;
		if(	pDoc->pSelObject && pDoc->pSelObject->size && pDoc->bEditState==STATE_MAP_EDIT){
			objshift=pDoc->pSelObject->getShift();
			objsize = pDoc->pSelObject->size;
		}

		polSize.x = abs(pEditorView->downPoint.x - pEditorView->mousePos.x );
		polSize.y = abs(pEditorView->downPoint.y - pEditorView->mousePos.y );
		if(!(polSize.x==0 && polSize.y==0)){
			polSize += CPoint(objsize,objsize);
			pomImg.Create(polSize.x*scale+1,polSize.y*scale+1,32);
			CDC* pRDC = CDC::FromHandle(pomImg.GetDC());
			pRDC->SelectObject(&rbrush);
			pRDC->SelectObject(&rpen);
			pRDC->SetBkColor(TRANSPARENT_COLOR);

			pRDC->Rectangle(0,0,pomImg.GetWidth(),pomImg.GetHeight());
			
			rect.SetRect(pEditorView->downPoint.x*scale-1,(py-pEditorView->downPoint.y-1)*scale-1,
					 (pEditorView->downPoint.x+polSize.x)*scale,(py - pEditorView->downPoint.y+polSize.y -1)*scale);
			shiftx= pEditorView->downPoint.x > pEditorView->mousePos.x ? (polSize.x-(objsize-objshift-1))*scale-scale : objshift*scale;
			shifty= pEditorView->downPoint.y < pEditorView->mousePos.y ? (polSize.y-objshift)*scale-scale :(objsize-objshift-1)*scale;
			rect.left -= (view.left+ shiftx);																										
			rect.right -= (view.left + shiftx);
			rect.top -=  (view.top + shifty) ;
			rect.bottom -=(view.top+ shifty);
			
			pomImg.TransparentBlt(pDC->m_hDC, (RECT) rect,TRANSPARENT_COLOR);
			pomImg.ReleaseDC();
			pomImg.Destroy();
		}
	}
	return true;
};

BOOL CMapEditorView::OnEraseBkgnd(CDC* pDC ){

	return false;
}

