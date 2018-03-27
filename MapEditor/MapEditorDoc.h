// MapEditorDoc.h : interface of the CMapEditorDoc class
//


#pragma once
#include "stdafx.h"
#include "storageClasses.h"
#include  "..\globals\NameValuePair.h"
#include "..\globals\Configuration.h"
#include "..\ResourceManager\ResourceManager.h"

#define STATE_MAP_EDIT 0 
#define STATE_TERRAIN_EDIT 1 
#define STATE_PATH_EDIT 2



class CMapEditorDoc : public CDocument
{
	protected: // create from serialization only
		CMapEditorDoc();
		DECLARE_DYNCREATE(CMapEditorDoc)
	

	public: 
		
		afx_msg void OnFileSave();
		afx_msg void OnFileSaveAs();
		afx_msg void OnMyFileOpen();
		afx_msg void OnTerrainFileOpen();
		afx_msg void OnTerrainEdit();
		afx_msg void OnMapEdit();
		afx_msg void OnPathEdit();
		afx_msg void OnTerrainEditUpdate(CCmdUI*);
		afx_msg void OnMapEditUpdate(CCmdUI*);
		afx_msg void OnPathEditUpdate(CCmdUI*);
		afx_msg void OnMapProperties();
		afx_msg void OnFileEditCar();
		
		afx_msg void OnFileEditLight();
		afx_msg void OnFileEditModeldef();
		afx_msg void OnFileEditPhTexture();
		afx_msg void OnFileEditTextureSet();
		afx_msg void OnFileEditSkySystem();
		afx_msg void OnFileEditDlgDecl();
		afx_msg void OnFileEditDlgDef();
		afx_msg void OnFileEditCursor();
		afx_msg void OnFileEditGrass();
		afx_msg void OnFileEditShrub();
		afx_msg void OnFileEditHelp();
		afx_msg void OnHelpHelp();
		virtual BOOL OnNewDocument();
		virtual BOOL OnOpenDocument(LPCTSTR);
		virtual BOOL OnSaveDocument(LPCTSTR);
		
	
	private: 
		
		int first;

	public:
		CGameMap    *pGameMap;
		CMapStringToPtr  objectsMap;
		resManNS::CResourceManager ResourceManager;
		//CString		sDocumentName;
		BYTE bEditState;
		BOOL		bMapNotSaved;
		BOOL		bTrackChanged;
		ORIENTATION selObjectOr;
		CMainObject * pSelObject;
		CPoint selObjectPos;
		CEditorTree editorTree;

			
		
		

// Overrides


	
	// Implementation
	public:
		
		virtual ~CMapEditorDoc();
		void SetTitle();
		virtual void SetTitle(LPCTSTR lpzsTitle);
		virtual void OnCloseDocument();
	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
	#endif

	protected:

	// Generated message map functions
	protected:
		DECLARE_MESSAGE_MAP()
};


