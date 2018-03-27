#include "stdafx.h"
#include "MapDialogs.h"


void CNewMapDialog::OnOK(){
	char c[256];
	height = GetDlgItemInt(IDC_NEW_HEIGHT);
	width = GetDlgItemInt(IDC_NEW_WIDTH);
	 GetDlgItemText(IDC_NEW_NAME,c,255);
	name = c;
	CDialog::OnOK();
}

BOOL CNewMapDialog::OnInitDialog(){
	
	height = 50;
	width = 50;
	CDialog::OnInitDialog();
	SetDlgItemInt(IDC_NEW_HEIGHT,height,0);
	SetDlgItemInt(IDC_NEW_WIDTH,width,0);
	SetDlgItemText(IDC_NEW_NAME,"New map");

	return TRUE;
}
void CNewMapDialog::DoDataExchange(CDataExchange *pDX){
	DDX_Text(pDX,IDC_NEW_HEIGHT,height);
	DDX_Text(pDX,IDC_NEW_WIDTH,width);
	DDX_Text(pDX,IDC_NEW_NAME,name);

	

	DDV_MinMaxByte(pDX,height,5,99);
	DDV_MinMaxByte(pDX,width,5,99);
	
	CDialog::DoDataExchange(pDX);
}
/*
void CFileEditDialog::DoDataExchange(CDataExchange *pDX){
	
	DDX_Text(pDX,IDC_SET_NAME_EDIT,strSetName);
	DDX_Text(pDX,IDC_OBJECT_NAME_EDIT,strObjectName);
	DDX_Text(pDX,IDC_OBJECT_SIZEX_EDIT, bObjectSizeX);
	DDX_Text(pDX,IDC_OBJECT_SIZEY_EDIT,	bObjectSizeY);
	DDX_Text(pDX,IDC_OBJECT_MAIN_POSX_EDIT,bPositionX);
	DDX_Text(pDX,IDC_OBJECT_MAIN_POSY_EDIT,bPositionY);
	DDX_Text(pDX,IDC_PHYSICS_NAME_EDIT,strPhysicsName);
	DDX_Text(pDX,IDC_MODEL_NAME_EDIT,strModelName);
	DDX_Text(pDX,IDC_OBJECT_POSITIONX_EDIT,uiObjectPositionX);
	DDX_Text(pDX,IDC_OBJECT_POSITIONY_EDIT,uiObjectPositionY);
	DDX_Text(pDX,IDC_OBJECT_POSITIONZ_EDIT,uiObjectPositionZ);
	DDX_Text(pDX,IDC_OBJECT_ROTATIONX_EDIT,fObjectRotationX);
	DDX_Text(pDX,IDC_OBJECT_ROTATIONY_EDIT,fObjectRotationY);
	DDX_Text(pDX,IDC_OBJECT_ROTATIONZ_EDIT,fObjectRotationZ);
	DDX_Text(pDX,IDC_OBJECT_SCALEX_EDIT,fObjectScaleX);
	DDX_Text(pDX,IDC_OBJECT_SCALEY_EDIT,fObjectScaleY);
	DDX_Text(pDX,IDC_OBJECT_SCALEZ_EDIT,fObjectScaleZ);
	DDX_Text(pDX,IDC_ROAD_EDIT,strRoadName);
	
	
	DDV_MinMaxByte(pDX,bObjectSizeX,0,99);
	DDV_MinMaxByte(pDX,bObjectSizeY,0,99);
	DDV_MinMaxByte(pDX,bPositionX,0,99);
	DDV_MinMaxByte(pDX,bPositionY,0,99);
	DDV_MinMaxInt(pDX,uiObjectPositionX,0,1000);
	DDV_MinMaxInt(pDX,uiObjectPositionY,0,1000);
	DDV_MinMaxInt(pDX,uiObjectPositionZ,0,1000);
	DDV_MinMaxDouble(pDX,fObjectRotationX,-6.28,6.28);
	DDV_MinMaxDouble(pDX,fObjectRotationY,-6.28,6.28);
	DDV_MinMaxDouble(pDX,fObjectRotationZ,-6.28,6.28);
	DDV_MinMaxDouble(pDX,fObjectScaleX,0.00001,1000);
	DDV_MinMaxDouble(pDX,fObjectScaleY,0.00001,1000);
	DDV_MinMaxDouble(pDX,fObjectScaleZ,0.00001,1000);
	CDialog::DoDataExchange(pDX);*/
//}
///////////////////////////////////////////////////////////////////////
// MAP DIALOG
///////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CMapPropertiesDialog, CDialog)
	ON_COMMAND(IDC_MAP_LOAD_SKYSYSTEM,OnLoadSkySystem)
	ON_COMMAND(IDC_MAP_LOAD_TEXTURESET,OnLoadTerrainTex)
	ON_COMMAND(IDC_MAP_LOAD_GRASS,OnLoadGrass)
	ON_COMMAND(IDC_MAP_LOAD_WALLSET,OnLoadWallSet)
	ON_COMMAND(IDC_MAP_LOAD_LOGO,OnLoadLogo)
	ON_WM_LBUTTONDBLCLK()

	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDOK, &CMapPropertiesDialog::OnBnClickedOk)
END_MESSAGE_MAP()


CMapPropertiesDialog::CMapPropertiesDialog(CGameMap * _pGameMap,CWnd * pParentWnd)
:CDialog(CMapPropertiesDialog::IDD,pParentWnd){
	pGameMap = _pGameMap;
	sMapName = pGameMap->sName;
	sTextureSet=pGameMap->sTextureSet;
	sSkySystem=pGameMap->sSkySystem;
	sGrassType=pGameMap->sGrassType;
	sWallSet = pGameMap->sWallSet;
	uiHours = pGameMap->uiHours;
	uiMinutes = pGameMap->uiMinutes;
	uiFogColor = pGameMap->uiFogColor;
	csDescription = pGameMap->csDescription;
	csImageFileName = pGameMap->csImageFileName;
}

void CMapPropertiesDialog::DoDataExchange(CDataExchange *pDX){
	char	sz[20];
	CString	cstr;
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX,IDC_MAP_NAME,sMapName);
	DDX_Text(pDX,IDC_MAP_TEXTURESET,sTextureSet);
	DDX_Text(pDX,IDC_MAP_WALLSET,sWallSet);
	DDX_Text(pDX,IDC_MAP_SKYSYSTEM,sSkySystem);
	DDX_Text(pDX,IDC_MAP_LOGO,csImageFileName);
	DDX_Text(pDX,IDC_MAP_HOURS,uiHours);
	DDX_Text(pDX,IDC_MAP_MINUTES,uiMinutes);
	DDX_Text(pDX,IDC_MAP_GRASS,sGrassType);
	DDX_Text(pDX,IDC_MAP_DESCRIPTION,csDescription);
	sprintf_s( sz, 20, "%x", uiFogColor );
	cstr = sz;
	cstr = cstr.Right( 6 );
	DDX_Text(pDX,IDC_MAP_FOGCOLOR,cstr);
	sscanf_s( cstr, "%x", &uiFogColor );
	uiFogColor |= 0xff000000;
	//sscanf_s( cstr, "%x", &uiFogColor );
	DDV_MinMaxInt(pDX,uiHours,0,24);
	DDV_MinMaxInt(pDX,uiMinutes,0,60);

	DDX_Control(pDX, IDC_MAP_MODLIST, GameModsListBox);
}

void CMapPropertiesDialog::OnLoadGrass(){
	CMyFileDialog dlgFile(CONFIG_MODELSPATH,GRASS_EXT,IDD_FILE_OPEN_DIALOG);
	INT_PTR res = dlgFile.DoModal();
	if(res == IDOK){
		sGrassType = dlgFile.sReturned;
		SetDlgItemText(IDC_MAP_GRASS,sGrassType);
	}

}

void CMapPropertiesDialog::OnLoadSkySystem(){
	CMyFileDialog dlgFile(CONFIG_SKYSYSTEMPATH,SKYSYSTEM_EXT,IDD_FILE_OPEN_DIALOG);
	INT_PTR res = dlgFile.DoModal();
	if(res == IDOK){
		sSkySystem = dlgFile.sReturned;
		SetDlgItemText(IDC_MAP_SKYSYSTEM,sSkySystem);
	}
	
}
void CMapPropertiesDialog::OnLoadTerrainTex(){
	CMyFileDialog dlgFile(CONFIG_TEXTURESETPATH+TEXTURESET_TERRAIN_PATH,TEXTURESET_EXT,IDD_FILE_OPEN_DIALOG);
	INT_PTR res = dlgFile.DoModal();
	if(res == IDOK){
		sTextureSet = dlgFile.sReturned;
		SetDlgItemText(IDC_MAP_TEXTURESET,sTextureSet);
	}
}

void CMapPropertiesDialog::OnLoadWallSet(){
	CMyFileDialog dlgFile(CONFIG_TEXTURESETPATH+TEXTURESET_WALL_PATH,TEXTURESET_EXT,IDD_FILE_OPEN_DIALOG);
	INT_PTR res = dlgFile.DoModal();
	if(res == IDOK){
		sWallSet = dlgFile.sReturned;
		SetDlgItemText(IDC_MAP_WALLSET,sWallSet);
	}
}

void CMapPropertiesDialog::OnLoadLogo(){
	CMyFileDialog dlgFile(CONFIG_TEXTURESPATH+TEXTURE_MAPLOGO_PATH,gProp.mExts["texturejpg"],IDD_FILE_OPEN_DIALOG);
	INT_PTR res = dlgFile.DoModal();
	if(res == IDOK){
		csImageFileName = dlgFile.sReturned;
		SetDlgItemText(IDC_MAP_LOGO,csImageFileName);
	}
}
///////////////////////////////////////////////////////////////////////
// FILE DIALOG
///////////////////////////////////////////////////////////////////////


void CMyFileDialog::OnOK(){
	int index;
	CString sOuttext;
	CString test;
	FILE *f;
	sFileName = "";
	if (type==save) GetDlgItemText(IDC_SAVE_FILE_NAME,sFileName);

	index = ctlList.GetCurSel();
	if ( (index!= LB_ERR && ctlList.GetItemData(index) == 1) || sFileName!=""){

		if(sFileName==""){
				ctlList.GetText(index,sOuttext);
				sReturned = ctlList.sActual + sOuttext;
		}
		else{
			
			if(sFileName.GetLength() > (int)strlen(ctlList.sExt)){
				test = sFileName;
				test = test.Right((int)strlen(ctlList.sExt));
				if(test != ctlList.sExt)
					sFileName = sFileName + ctlList.sExt;
			} else{
					sFileName = sFileName + ctlList.sExt;
			}
			sReturned = ctlList.sActual+sFileName;
		}
		
		if(type == save){
			
			if(fopen_s(&f,ctlList.sPath+sReturned,"r")==0){
				fclose(f);		
				test.LoadString(ID_CONFIRM_REPLACE);
				if(AfxMessageBox(test ,MB_YESNO) == IDYES ){
					EndDialog(IDOK);
					
				}
					
			}else
				EndDialog(IDOK);

		}
		else EndDialog(IDOK);
	}
	
}

BOOL CMyFileDialog::OnInitDialog(){
	
	CDialog::OnInitDialog();
	ctlList.SetList();
	SetWindowText(ctlList.sExt);

	return TRUE;
}

void CMyFileDialog::DoDataExchange(CDataExchange *pDX){
	CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CIntListView)
	//DDX_Text(pDX,IDC_SAVE_FILE_NAME, sFileName);
	DDX_Control(pDX,IDC_FILE_LIST, ctlList);
	
};





/////////////////////////////////////////////////////////////////////////////
// MyListBox
/////////////////////////////////////////////////////////////////////////////


BEGIN_MESSAGE_MAP(CMyListBox, CListBox)
	
	ON_WM_LBUTTONDBLCLK()

END_MESSAGE_MAP()



CMyListBox::CMyListBox()
{
	BOOL	bRet = FALSE;
	HICON	hIcon = NULL;
    
	// Create image list
	bRet = ImageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 5, 1);
	ASSERT(bRet == TRUE);
    
	hIcon = AfxGetApp()->LoadIcon(IDR_DIR_ICON);
	ImageList.Add(hIcon);

	hIcon = AfxGetApp()->LoadIcon(IDR_FILE_ICON);
	ImageList.Add(hIcon);
		
}




void CMyListBox::SetList()
{
	_finddata_t			FData;
	intptr_t			FInfoHandle;
	CString name,test = sPath + sExt;
	ResetContent();
	int in=0;
	if ( (FInfoHandle = _findfirst( sPath + sActual +'*', &FData )) != -1 )
	{
		name = FData.name;
			
			if(!(name==".." && sActual=="") && (name!=".") && (FData.attrib & _A_SUBDIR)){
					AddString(FData.name);
					SetItemData(in++,0);
					
			}
		while ( !_findnext( FInfoHandle, &FData ) )
		{
			name = FData.name;
			
			if(!(name==".." && sActual=="") && (FData.attrib & _A_SUBDIR)){
					AddString(FData.name);
					SetItemData(in++,0);
			}
		}
	}
	_findclose( FInfoHandle );
	if ( (FInfoHandle = _findfirst( sPath + sActual + '*' + sExt, &FData )) != -1 )
	{
		if (!((FData.attrib & _A_SUBDIR))){
			AddString(FData.name);
			SetItemData(in++,1);
				/*nw = 18+strlen(FData.name)* 8;
					if(width<nw){
						SetColumnWidth(nw);
						width = nw;
					}*/
		}
		while ( !_findnext( FInfoHandle, &FData ) )
		{	
			if (!((FData.attrib & _A_SUBDIR))){
				AddString(FData.name);
				SetItemData(in++,1);
				
			}
		}
	}
	_findclose( FInfoHandle );

}



/////////////////////////////////////////////////////////////////////////////
// MyListBox message handlers

void CMyListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	//CRect members to store the position of the items
	
	CRect rItem;
	CRect rText;
	CRect rIcon;
	CDC* dc = CDC::FromHandle(lpDrawItemStruct->hDC);
	
	
	if ((int)lpDrawItemStruct->itemID < 0)
	{
		// If there are no elements in the List Box 
		// based on whether the list box has Focus or not 
		// draw the Focus Rect or Erase it,
		if ((lpDrawItemStruct->itemAction & ODA_FOCUS) && 
			(lpDrawItemStruct->itemState & ODS_FOCUS))
		{
			dc->DrawFocusRect(&lpDrawItemStruct->rcItem);
		}
		else if ((lpDrawItemStruct->itemAction & ODA_FOCUS) &&	
			!(lpDrawItemStruct->itemState & ODS_FOCUS)) 
		{
			dc->DrawFocusRect(&lpDrawItemStruct->rcItem); 
		}
		return;
	}


	// String to store the text
	CString strText;

	// Get the item text.
	GetText(lpDrawItemStruct->itemID, strText);

	//Initialize the Item's row
	rItem = lpDrawItemStruct->rcItem;

	rIcon = lpDrawItemStruct->rcItem;
	rIcon.bottom = rIcon.top + 16;
	rIcon.right = rIcon.left + 16;

	//Start drawing the text 2 pixels after the icon
	rText.left = rIcon.right + 2;
	rText.top  = rIcon.top;


	UINT nFormat = DT_LEFT | DT_SINGLELINE | DT_VCENTER;
	if (GetStyle() & LBS_USETABSTOPS)
		nFormat |= DT_EXPANDTABS;


	// If item selected, draw the highlight rectangle.
	// Or if item deselected, draw the rectangle using the window color.
	if ((lpDrawItemStruct->itemState & ODS_SELECTED) &&
		 (lpDrawItemStruct->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		CBrush br(::GetSysColor(COLOR_HIGHLIGHT));
		dc->FillRect(&rItem, &br);
	}
	else if (!(lpDrawItemStruct->itemState & ODS_SELECTED) && 
		(lpDrawItemStruct->itemAction & ODA_SELECT)) 
	{
		CBrush br(::GetSysColor(COLOR_WINDOW));
		dc->FillRect(&rItem, &br);
	}

	// If the item has focus, draw the focus rect.
	// If the item does not have focus, erase the focus rect.
	if ((lpDrawItemStruct->itemAction & ODA_FOCUS) && 
		(lpDrawItemStruct->itemState & ODS_FOCUS))
	{
		dc->DrawFocusRect(&rItem); 
	}
	else if ((lpDrawItemStruct->itemAction & ODA_FOCUS) &&	
		!(lpDrawItemStruct->itemState & ODS_FOCUS))
	{
		dc->DrawFocusRect(&rItem); 
	}


	// To draw the Text set the background mode to Transparent.
	int iBkMode = dc->SetBkMode(TRANSPARENT);

	COLORREF crText;

	if (lpDrawItemStruct->itemState & ODS_SELECTED)
		crText = dc->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
	else if (lpDrawItemStruct->itemState & ODS_DISABLED)
		crText = dc->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
	else
		crText = dc->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));


	CPoint pt(rIcon.left,rIcon.top);
	
	//if( == 0)
	ImageList.Draw(dc,(int) lpDrawItemStruct->itemData ,pt,ILD_NORMAL);

	dc->TextOut(rText.left,rText.top,strText);
	//Draw the Text
	dc->SetTextColor(crText); 


	
}

void CMyListBox::OnLButtonDblClk(UINT nFlags,CPoint point){
	int ind,t;

	CString text;
	ind = GetCurSel();
	GetText(ind,text);
	if ( GetItemData(ind) == 0){ 
		if(text == ".."){
			sActual.Delete(sActual.GetLength() -1);
			t = sActual.ReverseFind('\\');
			if(t>=0) sActual.Delete(t+1,sActual.GetLength()-t);
			else sActual="";

		}else{
			sActual+=text+'\\';
			
		}
		SetList();
	}else parent->OnOK();

}
BOOL CMyListBox::OnInitDialog(){
	return 1;
}
void CMyListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	// TODO: Add your code to determine the size of specified item
	lpMeasureItemStruct->itemHeight = 32;

	
}

//////////////////////////////////////////////////////////////////////////////////////////
// CEditFile Dialog
//////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CEditFileDialog, CDialog)
	ON_LBN_DBLCLK(IDC_EDITFILE_LIST,OnListDbl)
	ON_COMMAND(IDC_EDITFILE_LOAD,OnLoad)
	ON_COMMAND(IDC_EDITFILE_SAVE,OnSave)
	ON_COMMAND(IDC_EDITFILE_SAVEAS,OnSaveAs)
	ON_WM_SIZING()
	ON_WM_SIZE()
	
	

END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CEditFileDialog)
    
    EASYSIZE(IDC_EDITFILE_LIST,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_BORDER,0)
	EASYSIZE(IDC_EDITFILE_EDIT,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)
	EASYSIZE(IDC_EDITFILE_VIEW,ES_BORDER,ES_BORDER,ES_BORDER,IDC_EDITFILE_VIEW,0)
    EASYSIZE(IDC_EDITFILE_LOAD,ES_BORDER,ES_KEEPSIZE,ES_KEEPSIZE,ES_BORDER,0)
	EASYSIZE(IDC_EDITFILE_SAVE,ES_BORDER,ES_KEEPSIZE,ES_KEEPSIZE,ES_BORDER,0)
	EASYSIZE(IDC_EDITFILE_SAVEAS,ES_BORDER,ES_KEEPSIZE,ES_KEEPSIZE,ES_BORDER,0)
	EASYSIZE(IDOK,ES_BORDER,ES_KEEPSIZE,ES_KEEPSIZE,ES_BORDER,0)

END_EASYSIZE_MAP

CEditFileDialog::CEditFileDialog(CString filename,CString ext,CString path,CWnd * pParentWnd)
:CDialog(IDD_EDITFILE_DIALOG,pParentWnd){
	sFileName = filename;
	sExt = ext;
	sType = ext;
	sType.Delete(0);
	sPath = path;
	sTitlePrefix = sType;
	sTitlePrefix+=" edit: ";
	
};


void CEditFileDialog::OnSize(UINT nType,int cx,int cy){
	
	CDialog::OnSize(nType,cx,cy);
	UPDATE_EASYSIZE;

}
void CEditFileDialog::OnSizing(UINT nSide,LPRECT lpRect){
	CDialog::OnSizing(nSide,lpRect);
	EASYSIZE_MINSIZE(700,600,nSide,lpRect);
}
DWORD CALLBACK CEditFileDialog::MyStreamInCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	FILE *f;
	f = * ((FILE **) UlongToPtr(dwCookie));
	*pcb =(LONG) fread(pbBuff,sizeof(char),cb,f);
	return 0;
}
void CEditFileDialog::setTitle(){
	SetWindowTextA(sTitlePrefix + sFileName);

}
BOOL CEditFileDialog::OnInitDialog(){
	FILE *f;
	EDITSTREAM es;
	CString name;
	errno_t res;
	
	pEdit = (CRichEditCtrl*)GetDlgItem(IDC_EDITFILE_EDIT);
	pViewEdit = (CRichEditCtrl*)GetDlgItem(IDC_EDITFILE_VIEW);
	pListBox  = (CListBox *) GetDlgItem(IDC_EDITFILE_LIST);
	
	m_pIRichEditOleCallback = NULL;
	m_pIRichEditOleCallback = new IExRichEditOleCallback;
	ASSERT( m_pIRichEditOleCallback != NULL );
	pViewEdit->SetOLECallback( m_pIRichEditOleCallback );
	name = CONFIG_FILESFORMATPATH;
	name += sType+".rtf";
	SetIcon(AfxGetApp()->LoadIcon(IDI_EDIT_FILE_ICON) ,true);
	res = fopen_s(&f,name,"r");
	if(!res){
	
		es.dwCookie =PtrToUlong(&f);
		es.pfnCallback = MyStreamInCallback; 
	    
		pViewEdit->StreamIn(SF_RTF, es);
		fclose(f);
	}
	loadFile();
	saved=FALSE;
	loadListHelp();
	INIT_EASYSIZE;
	setTitle();
	
	return TRUE;
};
void CEditFileDialog::loadFile(){
	FILE * f;
	errno_t res;
	EDITSTREAM es;
	res = fopen_s(&f,sPath+sFileName,"rb");
	if(!res){
		saved=TRUE;
		es.dwCookie = PtrToUlong(&f);
		es.pfnCallback = MyStreamInCallback; 
	    
		pEdit->StreamIn(SF_TEXT, es);
		fclose(f);
		setTitle();
	}
}
void CEditFileDialog::loadListHelp(){
	FILE *f;
	CString loaded,name;
	int res;
	int size,lenght;

	char buffer[MAX_BUFFER_SIZE];
	MYLISTITEM _litem;

	name = CONFIG_FILESFORMATPATH;
	name += sType+DHLP_EXT;
	res = fopen_s(&f,name,"r");
	if(res) return;
	while(fgets(buffer,MAX_BUFFER_SIZE,f)){
		loaded = buffer;
		loaded.Trim("\n");
		if(loaded[0] == '-' && loaded[1]=='-'){
			loaded.Delete(0,2);
			_litem.name=loaded;
			vListItems.push_back(_litem);
			pListBox->AddString(loaded);

		}else if(loaded[0]=='%' && loaded[1]=='%'){
			loaded.Delete(0,2);
			size=(int) vListItems.size();
			if(size){	
				size -= 1;
				lenght=vListItems[size].content.GetLength();
				if(lenght > 0 && vListItems[size].content[lenght-1]=='\n')
					lenght--;
				vListItems[size].sTypesPos.push_back(lenght);
				vListItems[size].sTypes.push_back(loaded);
			}

		}
		else{
			size=(int)vListItems.size();
			if(size) vListItems[size-1].content+=buffer;
		}
	}
	fclose(f);

}
void CEditFileDialog::OnListDbl(){
	//pListBox->OnLButtonDblClk(
	int id=pListBox->GetCurSel();
	int i;
	INT_PTR ret;
	CString ext,path,outstr,index;
	CMyFileDialog openDialog("","",IDD_FILE_OPEN_DIALOG);
	
	
	outstr=vListItems[id].content;
	for(i =(int) vListItems[id].sTypes.size()-1; i >= 0 ;i--){
		index=vListItems[id].sTypes[i];
		ext = gProp.mExts[index];
		if(ext != ""){
			path = gProp.mPaths[index];
			openDialog.init(path,ext);
			ret = openDialog.DoModal();
			if(ret==IDOK){
				outstr.Insert(vListItems[id].sTypesPos[i],openDialog.sReturned);	
			}
		}
	}
	
	pEdit->ReplaceSel(outstr);

}
void CEditFileDialog::DoDataExchange(CDataExchange *pDX){

};


void CEditFileDialog::OnLoad(){
	CMyFileDialog fileDlg(sPath,sExt,IDD_FILE_OPEN_DIALOG);
	INT_PTR ret = fileDlg.DoModal();
	if (ret==IDOK){
		sFileName = fileDlg.sReturned;
		loadFile();
		
	}
};

void CEditFileDialog::OnSave(){
	
	HRESULT hr;
	if(sFileName == "")
		OnSaveAs();
	else{
		hr = saveFile();
	}

};

void CEditFileDialog::OnSaveAs(){
	CMyFileDialog fileDlg(sPath,sExt,IDD_FILE_SAVE_DIALOG,save);
	INT_PTR ret = fileDlg.DoModal();
	if (ret==IDOK){

		sFileName = fileDlg.sReturned;
		saveFile();
		setTitle();
	}

};
DWORD CALLBACK CEditFileDialog::MyStreamOutCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	FILE *f;
	f = * ((FILE **) UlongToPtr(dwCookie));
	*pcb =(LONG) fwrite(pbBuff,sizeof(char), (size_t)cb,f);
	return 0;
}

HRESULT CEditFileDialog::saveFile(){
	FILE *f;
	errno_t ret;
	EDITSTREAM es;
	ret = fopen_s(&f,sPath+sFileName,"wb");
	if (ret) return ret;
	saved = TRUE;
	
	es.dwCookie = (DWORD) PtrToUlong(&f);
	es.pfnCallback = MyStreamOutCallback; 
	 
	pEdit->StreamOut(SF_TEXT, es);
	fclose(f);
	return ERRNOERROR;
};

void CEditFileDialog::OnOK(){
	if(saved)
		CDialog::OnOK();
	else CDialog::OnCancel();

}

/////////////////////////////////////////////////////////////////////////////
//CEditFileDialog::IExRichEditOleCallback
/////////////////////////////////////////////////////////////////////////////

CEditFileDialog::IExRichEditOleCallback::IExRichEditOleCallback()
{
	pStorage = NULL;
	m_iNumStorages = 0;
	m_dwRef = 0;

	// set up OLE storage

	HRESULT hResult = ::StgCreateDocfile(NULL,
		STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE /*| STGM_DELETEONRELEASE */|STGM_CREATE ,
		0, &pStorage );

	if ( pStorage == NULL ||
		hResult != S_OK )
	{
		AfxThrowOleException( hResult );
	}
}

CEditFileDialog::IExRichEditOleCallback::~IExRichEditOleCallback()
{
}

HRESULT STDMETHODCALLTYPE 
CEditFileDialog::IExRichEditOleCallback::GetNewStorage(LPSTORAGE* lplpstg)
{
	m_iNumStorages++;
	WCHAR tName[50];
	swprintf_s(tName, L"REOLEStorage%d", m_iNumStorages);

	HRESULT hResult = pStorage->CreateStorage(tName, 
		STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE ,
		0, 0, lplpstg );

	if (hResult != S_OK )
	{
		::AfxThrowOleException( hResult );
	}
	return hResult;
}

HRESULT STDMETHODCALLTYPE 
CEditFileDialog::IExRichEditOleCallback::QueryInterface(REFIID iid, void ** ppvObject)
{
	HRESULT hr = S_OK;
 
	*ppvObject = NULL;
	
	if ( iid == IID_IUnknown ||
		iid == IID_IRichEditOleCallback )
	{
		*ppvObject = this;
		AddRef();
		hr = NOERROR;
	}
	else
	{
		hr = E_NOINTERFACE;
	}

	return hr;
}



ULONG STDMETHODCALLTYPE 
CEditFileDialog::IExRichEditOleCallback::AddRef()
{
	return ++m_dwRef;
}



ULONG STDMETHODCALLTYPE 
CEditFileDialog::IExRichEditOleCallback::Release()
{
	if ( --m_dwRef == 0 )
	{
		delete this;
		return 0;
	}

	return m_dwRef;
}


HRESULT STDMETHODCALLTYPE 
CEditFileDialog::IExRichEditOleCallback::GetInPlaceContext(LPOLEINPLACEFRAME FAR *lplpFrame,
	LPOLEINPLACEUIWINDOW FAR *lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	return S_OK;
}


HRESULT STDMETHODCALLTYPE 
CEditFileDialog::IExRichEditOleCallback::ShowContainerUI(BOOL fShow)
{
	return S_OK;
}



HRESULT STDMETHODCALLTYPE 
CEditFileDialog::IExRichEditOleCallback::QueryInsertObject(LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp)
{
	return S_OK;
}


HRESULT STDMETHODCALLTYPE 
CEditFileDialog::IExRichEditOleCallback::DeleteObject(LPOLEOBJECT lpoleobj)
{
	return S_OK;
}



HRESULT STDMETHODCALLTYPE 
CEditFileDialog::IExRichEditOleCallback::QueryAcceptData(LPDATAOBJECT lpdataobj, CLIPFORMAT FAR *lpcfFormat,
	DWORD reco, BOOL fReally, HGLOBAL hMetaPict)
{
	return S_OK;
}


HRESULT STDMETHODCALLTYPE 
CEditFileDialog::IExRichEditOleCallback::ContextSensitiveHelp(BOOL fEnterMode)
{
	return S_OK;
}



HRESULT STDMETHODCALLTYPE 
CEditFileDialog::IExRichEditOleCallback::GetClipboardData(CHARRANGE FAR *lpchrg, DWORD reco, LPDATAOBJECT FAR *lplpdataobj)
{
	return S_OK;
}


HRESULT STDMETHODCALLTYPE 
CEditFileDialog::IExRichEditOleCallback::GetDragDropEffect(BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE 
CEditFileDialog::IExRichEditOleCallback::GetContextMenu(WORD seltyp, LPOLEOBJECT lpoleobj, CHARRANGE FAR *lpchrg,
	HMENU FAR *lphmenu)
{
	return S_OK;
}






int CMapPropertiesDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	
	return 0;
}

void CMapPropertiesDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if ( bShow ) {
		// clear the listbox
		while ( GameModsListBox.GetCount() > 0 ) 
			GameModsListBox.DeleteString( 0 );
		// fill the listbox again
		for ( int i = 0; i < gProp.ModWorkshop.GetModCount(); i++ )
			GameModsListBox.AddString( gProp.ModWorkshop[i]->GetModulePrimaryID() );
		// make supported mods (items in listbox) selected
		for ( int i = 0; i < (int)pGameMap->vGameMods.size(); i++ ) {
			int index = gProp.ModWorkshop.FindMODIndex( pGameMap->vGameMods[i] );
			if ( index >= 0 ) GameModsListBox.SetSel( index, 1 );
		}
	}
}

void CMapPropertiesDialog::OnBnClickedOk()
{
	vGameMods.clear();
	for ( int i = 0; i < GameModsListBox.GetCount(); i++ ) {
		if ( GameModsListBox.GetSel( i ) ) {
			CString	cstr;
			GameModsListBox.GetText( i, cstr );
			vGameMods.push_back( cstr );
		}
	}

	OnOK();
}
