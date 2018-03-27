// MapEditorDoc.cpp : implementation of the CMapEditorDoc class
//

#include "stdafx.h"
#include "MapEditor.h"
#include "..\\globals\\NameValuePair.h"
#include "..\\globals\\ErrorHandler.h"
#include "MapEditorDoc.h"
#include "StorageClasses.h"
#include "MapDialogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMapEditorDoc


using namespace resManNS;

IMPLEMENT_DYNCREATE(CMapEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(CMapEditorDoc, CDocument)
	ON_COMMAND(ID_MYFILE_SAVE,OnFileSave)
	ON_COMMAND(ID_MYFILE_SAVE_AS,OnFileSaveAs)
	ON_COMMAND(ID_MYFILE_OPEN, OnMyFileOpen)
	ON_COMMAND(ID_MYFILE_TERRAINOPEN,OnTerrainFileOpen)
	
	ON_COMMAND(ID_FILE_EDIT_CAR,OnFileEditCar)	
	ON_COMMAND(ID_FILE_EDIT_LIGHT,OnFileEditLight )
	ON_COMMAND(ID_FILE_EDIT_MODELDEF,OnFileEditModeldef )
	ON_COMMAND(ID_FILE_EDIT_PHTEXTURE,OnFileEditPhTexture )
	ON_COMMAND(ID_FILE_EDIT_TEXTURESET,OnFileEditTextureSet )
	ON_COMMAND(ID_FILE_EDIT_SKYSYSTEM,OnFileEditSkySystem )
	ON_COMMAND(ID_FILE_EDIT_DLGDECL,OnFileEditDlgDecl )
	ON_COMMAND(ID_FILE_EDIT_DGDEF,OnFileEditDlgDef )
	ON_COMMAND(ID_FILE_EDIT_CURSOR,OnFileEditCursor )
	ON_COMMAND(ID_FILE_EDIT_GRASS,OnFileEditGrass )
	ON_COMMAND(ID_FILE_EDIT_SHRUB,OnFileEditShrub )
	ON_COMMAND(ID_FILE_EDIT_HELP,OnFileEditHelp )
	
	ON_COMMAND(ID_HELP_HELP,OnHelpHelp)

	ON_COMMAND(ID_EDIT_TERRAINEDIT,OnTerrainEdit)
	ON_COMMAND(ID_EDIT_MAPEDIT,OnMapEdit)
	ON_COMMAND(ID_EDIT_PATHEDIT,OnPathEdit)
	ON_COMMAND(ID_MAP_PROPERTIES,OnMapProperties)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TERRAINEDIT,OnTerrainEditUpdate)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MAPEDIT,OnMapEditUpdate)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PATHEDIT,OnPathEditUpdate)

END_MESSAGE_MAP()


CMapEditorDoc::CMapEditorDoc()
{
	first = 1;
	pGameMap = NULL;
	Configuration.Load( CONFIGFILENAME );
	// TODO: add one-time construction code here
	CString path = Configuration.EditorPath;
	pSelObject = NULL;
	//pTreeCtrl = NULL;
	
	pGameMap = NULL;
	bEditState = STATE_MAP_EDIT;
	ResourceManager.SetResourcePath( CONFIG_MODELSPATH, RES_Model );
	ResourceManager.SetResourcePath( CONFIG_MODELSPATH, RES_XFile );
	ResourceManager.SetResourcePath( CONFIG_MAPSPATH, RES_Terrain );
	ResourceManager.SetResourcePath( CONFIG_TERRAINSPATH, RES_GrSurface );
	ResourceManager.SetResourcePath( CONFIG_TERRAINSPATH, RES_GrPlate );
	ResourceManager.SetResourcePath( CONFIG_TEXTURESPATH, RES_Texture );
	ResourceManager.SetResourcePath( CONFIG_LODDEFINITIONSPATH, RES_LoDMap );
	ResourceManager.SetResourcePath( CONFIG_DIALOGSPATH, RES_DlgDeclaration );
	ResourceManager.SetResourcePath( CONFIG_DIALOGSPATH, RES_DlgDefinition );
	ResourceManager.SetResourcePath( CONFIG_FONTSPATH, RES_FontDef );
	ResourceManager.SetResourcePath( CONFIG_SHADERSPATH, RES_PixelShader );
	ResourceManager.SetResourcePath( CONFIG_SHADERSPATH, RES_VertexShader );
	ResourceManager.SetResourcePath( CONFIG_MAPSPATH, RES_Map );
	ResourceManager.SetResourcePath( CONFIG_PHTEXTURESPATH, RES_PhysicalTexture );
    ResourceManager.SetResourcePath( CONFIG_OBJECTSPATH, RES_EngineObject  );
	ResourceManager.SetResourcePath( CONFIG_CARSPATH, RES_CarObject );
	ResourceManager.SetResourcePath( CONFIG_OBJECTSPATH, RES_Waypoints);
	ResourceManager.SetResourcePath( CONFIG_OBJECTSPATH, RES_MainObject);
	ResourceManager.SetResourcePath( CONFIG_OBJECTSPATH, RES_GameObject);
	ResourceManager.SetResourcePath( CONFIG_LIGHTSPATH, RES_LightObject);
	ResourceManager.SetResourcePath( CONFIG_SKYSYSTEMPATH, RES_SkySystem);
	ResourceManager.SetResourcePath( CONFIG_TEXTURESETPATH, RES_TextureSet);
	editorTree.Init(&ResourceManager,&objectsMap);
}

CMapEditorDoc::~CMapEditorDoc()
{

	CMainObject * pGO;
	POSITION pos;
	CString key;
	SAFE_DELETE(pGameMap);
	for( pos = objectsMap.GetStartPosition(); pos != NULL; )
   {
		objectsMap.GetNextAssoc( pos, key, (void *&) pGO );
		SAFE_DELETE((CMainObject *) pGO);
   };
	
	objectsMap.RemoveAll();
	
}

void CMapEditorDoc::OnMyFileOpen(){
	CMyFileDialog fileDlg(CONFIG_MAPSPATH,MAP_EXT,IDD_FILE_OPEN_DIALOG);
	INT_PTR ret = fileDlg.DoModal();

	
	if (ret==IDOK){
		OnOpenDocument(fileDlg.sReturned);
	}
};


void CMapEditorDoc::OnTerrainFileOpen(){
	CMyFileDialog fileDlg(CONFIG_MAPSPATH,TERRAIN_EXT,IDD_FILE_OPEN_DIALOG);
	INT_PTR ret = fileDlg.DoModal();
	if (ret==IDOK){
		
		OnOpenDocument(fileDlg.sReturned);
	}
};


BOOL CMapEditorDoc::OnNewDocument()
{

	CNewMapDialog dlg;
	
	
	if (first){
		pGameMap = new CGameMap(&objectsMap,&ResourceManager,&editorTree);
		if(!pGameMap) return FALSE;
		pGameMap->setSize(50,50);
		first = 0;
	}else{
		
		if (dlg.DoModal() == IDOK){
			pGameMap->clearMap(); 
			pGameMap->setSize(dlg.width,dlg.height);
			pGameMap->sName = dlg.name;
			pGameMap->sFileName="";
		}
	};
	
	SetTitle();

	return TRUE;
};

void CMapEditorDoc::SetTitle(){
	if(pGameMap->sName==""){
		//pGameMap->
		SetTitle(pGameMap->sFileName);

	}else {
		SetTitle(pGameMap->sName);
	}

}

void CMapEditorDoc::SetTitle(LPCTSTR lpzsTitle){
	//CDocument::SetTitle(lpzsTitle);
	CString str,title;
	CWnd * wnd;
	wnd =AfxGetMainWnd();
	if(wnd){
		str.LoadStringA(AFX_IDS_APP_TITLE);
		wnd->SetWindowText(str+": "+lpzsTitle);
	}
}



// CMapEditorDoc serialization
BOOL CMapEditorDoc::OnOpenDocument(LPCTSTR fName){
	
		
	HRESULT hr;
	CString test=fName;
	CString out;
	test = fName;
	test= test.Right(8);

	if( test == TERRAIN_EXT){
		pGameMap->clearMap();
		hr = pGameMap->loadTerrain(fName);
		if (hr){
			out.LoadString(ID_ERROR_TERRAIN_OPEN);
			AfxMessageBox(out);
			return FALSE;
		}
		pGameMap->sFileName="";
	}
	else {
		pGameMap->clearMap();
		hr = pGameMap->loadMap(fName);
		if (hr){
			out.LoadString(ID_ERROR_MAP_OPEN);
			AfxMessageBox(out);
			return FALSE;
		}
		bTrackChanged=FALSE;
		hr = pGameMap->loadTerrain(pGameMap->sTerrain);
		if (hr){
			out.LoadString(ID_ERROR_TERRAIN_OPEN);
			AfxMessageBox(out);
			return FALSE;
		}
		pGameMap->sFileName=fName;

	}
	pSelObject=NULL;
	SetTitle();
	return TRUE;
};


BOOL CMapEditorDoc::OnSaveDocument(LPCTSTR fName){
	
	HRESULT hr;
	CString terrain;
	CString prefix;
	CString out;
	prefix = fName;
	prefix.Delete((int)(prefix.GetLength() - strlen(MAP_EXT)),(int) strlen(MAP_EXT));
	
	pGameMap->sTerrain=prefix + TERRAIN_EXT;
	if(!pGameMap->pMOStartPosition){
		out.LoadString(ID_ERROR_STARTPOS);
		AfxMessageBox(out);
		return FALSE;
	}
	if(bTrackChanged) pGameMap->FindPathFromStart();
	if(pGameMap->GetPathSize()==0){
		out.LoadString(ID_ERROR_PATH);
		AfxMessageBox(out);
		return FALSE;
	}
	hr=pGameMap->saveMap(fName);
	if (hr){
		out.LoadString(ID_ERROR_MAP_SAVE);
		AfxMessageBox(out);
		return FALSE;
	}
	pGameMap->sFileName=fName;
	hr=pGameMap->saveTerrain(pGameMap->sTerrain);
	if (hr){
		out.LoadString(ID_ERROR_TERRAIN_SAVE);
		AfxMessageBox(out);
		return FALSE;
	}
	bMapNotSaved=FALSE;
	SetTitle();
	return TRUE;
};
void CMapEditorDoc::OnHelpHelp(){
	
	ShellExecute( 
            NULL,
            "open",
            HELP_FILE,
            NULL,
            NULL,
            SW_MAXIMIZE);
	
}
void CMapEditorDoc::OnFileSave(){

	if(pGameMap->sFileName==""){
		OnFileSaveAs();
	}
	else{
		OnSaveDocument(pGameMap->sFileName);
	}
	
	
};
void CMapEditorDoc::OnFileSaveAs(){

	CMyFileDialog SaveDlg(CONFIG_MAPSPATH,MAP_EXT,IDD_FILE_SAVE_DIALOG,save);
	
	INT_PTR ret = SaveDlg.DoModal();
	if (ret==IDOK){
		
			OnSaveDocument(SaveDlg.sReturned);
	}
	
};
void CMapEditorDoc::OnMapProperties(){
	CMapPropertiesDialog PropDlg(pGameMap);
	INT_PTR ret=PropDlg.DoModal();
	if(ret ==IDOK){
		pGameMap->sName=PropDlg.sMapName;
		pGameMap->sSkySystem = PropDlg.sSkySystem;
		pGameMap->uiHours = PropDlg.uiHours;
		pGameMap->uiMinutes = PropDlg.uiMinutes;
		pGameMap->sTextureSet = PropDlg.sTextureSet;
		pGameMap->sGrassType = PropDlg.sGrassType;
		pGameMap->sWallSet = PropDlg.sWallSet;
		pGameMap->csDescription = PropDlg.csDescription;
		pGameMap->uiFogColor = PropDlg.uiFogColor;
		pGameMap->csImageFileName = PropDlg.csImageFileName;
		pGameMap->vGameMods.clear();
		for ( int i = 0; i < (int)PropDlg.vGameMods.size(); i++ ) {
			pGameMap->vGameMods.push_back( PropDlg.vGameMods[i] );
		}
	}
	SetTitle();
	CString ahoj=PropDlg.sMapName;

}
void CMapEditorDoc::OnMapEdit(){
	
	bEditState=STATE_MAP_EDIT ;
	UpdateAllViews(NULL);
	//Invalidate(false);
}

void CMapEditorDoc::OnTerrainEdit(){
	bEditState=STATE_TERRAIN_EDIT ;
	UpdateAllViews(NULL);
	//Invalidate(false);
}

void CMapEditorDoc::OnPathEdit(){

	bEditState=STATE_PATH_EDIT;
	
	if(bTrackChanged){	
		pGameMap->FindPathFromStart();
		bTrackChanged=FALSE;
	}
	
	UpdateAllViews(NULL);
}
void CMapEditorDoc::OnTerrainEditUpdate(CCmdUI* pCmdUI){
	pCmdUI->SetCheck(bEditState != STATE_MAP_EDIT && bEditState != STATE_PATH_EDIT);
	
}
void CMapEditorDoc::OnMapEditUpdate(CCmdUI* pCmdUI){
	pCmdUI->SetCheck(bEditState != STATE_TERRAIN_EDIT && bEditState != STATE_PATH_EDIT);
	
}
void CMapEditorDoc::OnPathEditUpdate(CCmdUI * pCmdUI){
	pCmdUI->SetCheck(bEditState != STATE_TERRAIN_EDIT && bEditState != STATE_MAP_EDIT);
}

void CMapEditorDoc::OnCloseDocument(){
	
	editorTree.SaveTree(CONFIG_EDITORPATH + TREE_FILE);
	CDocument::OnCloseDocument();
}
void CMapEditorDoc::OnFileEditCar(){

	CEditFileDialog dialog("",gProp.mExts["car"],gProp.mPaths["car"]);
	dialog.DoModal();
}

void CMapEditorDoc::OnFileEditLight(){
	CEditFileDialog dialog("",gProp.mExts["light"],gProp.mPaths["light"]);
	dialog.DoModal();
};
void CMapEditorDoc::OnFileEditModeldef(){
	CEditFileDialog dialog("",gProp.mExts["modeldef"],gProp.mPaths["modeldef"]);
	dialog.DoModal();
};
void CMapEditorDoc::OnFileEditPhTexture(){
	CEditFileDialog dialog("",gProp.mExts["phtexture"],gProp.mPaths["phtexture"]);
	dialog.DoModal();
};
void CMapEditorDoc::OnFileEditTextureSet(){
	CEditFileDialog dialog("",gProp.mExts["textset"],gProp.mPaths["textset"]);
	dialog.DoModal();
};
void CMapEditorDoc::OnFileEditSkySystem(){
	CEditFileDialog dialog("",gProp.mExts["sk"],gProp.mPaths["sk"]);
	dialog.DoModal();
};
void CMapEditorDoc::OnFileEditDlgDecl(){
	CEditFileDialog dialog("",gProp.mExts["dlgdec"],gProp.mPaths["dlgdec"]);
	dialog.DoModal();
};
void CMapEditorDoc::OnFileEditDlgDef(){
	
	CEditFileDialog dialog("",gProp.mExts["dlgdef"],gProp.mPaths["dlgdef"]);
	dialog.DoModal();
};
void CMapEditorDoc::OnFileEditCursor(){
	CEditFileDialog dialog("",gProp.mExts["cur"],gProp.mPaths["cur"]);
	dialog.DoModal();
};
void CMapEditorDoc::OnFileEditGrass(){
	CEditFileDialog dialog("",gProp.mExts["grass"],gProp.mPaths["grass"]);
	dialog.DoModal();
};
void CMapEditorDoc::OnFileEditShrub(){
	CEditFileDialog dialog("",gProp.mExts["shrub"],gProp.mPaths["shrub"]);
	dialog.DoModal();
};
void CMapEditorDoc::OnFileEditHelp(){
	CEditFileDialog dialog("",gProp.mExts["dhlp"],gProp.mPaths["dhlp"]);
	dialog.DoModal();
};
#ifdef _DEBUG
void CMapEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMapEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CMapEditorDoc commands
