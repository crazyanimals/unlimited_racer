#pragma once

#include "stdafx.h"
#include "resource.h"
#include "StorageClasses.h"
#include "EasySize.h"
#include <richole.h>
#include "afxwin.h"
#include "..\GameMods\ModWorkshop.h"


typedef  enum  __MyDialogType {load=0,save} MyDialogType;
class CMyFileDialog;
/////////////////////////////////////////////////////////////////////////////
// MyListBox window

class CMyListBox:public CListBox
{
// Construction
public:
	CMyListBox();

	//{{AFX_VIRTUAL(MyListBox)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void OnLButtonDblClk(UINT nFlags,CPoint point);
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL
	
	CMyFileDialog * parent;
	CImageList ImageList;
	CString sPath;
	CString sActual;
	CString sExt;
	
	void SetList();
	void init(CMyFileDialog * _parent,CString path, CString ext){ parent=_parent;sPath=path; sExt =ext;};

private:
	
protected:

	DECLARE_MESSAGE_MAP()
};
class CMyFileDialog:public CDialog{
	public:
		CMyFileDialog::CMyFileDialog(CString _path,CString _ext,int resource_id,MyDialogType _type=load,CWnd * pParentWnd = NULL):CDialog(resource_id,pParentWnd)
			{ctlList.init(this,_path,_ext);sFileName="";type = _type;};
		
		CString sReturned;
		virtual void OnOK();
		void init(CString path,CString ext){ctlList.init(this,path,ext);};
	private:
	CString sFileName;
	CMyListBox ctlList;
	MyDialogType type;
	virtual void DoDataExchange(CDataExchange *pDX);
	virtual BOOL OnInitDialog();
	
	/*protected:

	DECLARE_MESSAGE_MAP()*/

};




class CNewMapDialog:public CDialog{
	public:
		CNewMapDialog::CNewMapDialog(CWnd * pParentWnd = NULL):CDialog(IDD_NEW_DIALOG,pParentWnd){};
	
		int width;
		int height;
		CString name;

	private:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void DoDataExchange(CDataExchange *pDX);

};

struct MYLISTITEM{
		CString name;
		CString content;
		std::vector<CString> sTypes;
		std::vector<int> sTypesPos;
	};

class CEditFileDialog:public CDialog{
	


	public:
			CEditFileDialog(CString filename,CString ext,CString path,CWnd * pParentWnd = NULL);
			enum {IDD=IDD_EDITFILE_DIALOG};
		

			CString sFileName;
	private:
		
		CString sExt;
		CString sType;
		CString sPath;
		CRichEditCtrl * pViewEdit;
		CRichEditCtrl * pEdit;
		CListBox	  *	pListBox; 
		CString sTitlePrefix;
		BOOL saved;
		interface IExRichEditOleCallback;	// forward declaration (see below in this header file)
		IExRichEditOleCallback* m_pIRichEditOleCallback;
		
		std::vector<MYLISTITEM> vListItems; 
		void loadListHelp();
		void loadFile();
		void setTitle();
		HRESULT saveFile();
		
		virtual BOOL OnInitDialog();
		virtual void DoDataExchange(CDataExchange *pDX);
		virtual void OnSizing(UINT nSide,LPRECT lpRect);
		virtual void OnSize(UINT nType,int cx,int cy);
	
	public:
		static DWORD CALLBACK MyStreamInCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);
		static DWORD CALLBACK MyStreamOutCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);
		DECLARE_MESSAGE_MAP()
		DECLARE_EASYSIZE
		
		virtual void OnOK();
		afx_msg void OnEnChangeEditfileEdit();
		afx_msg void OnListDbl();
		afx_msg void OnLoad();
		afx_msg void OnSave();
		afx_msg void OnSaveAs();
	
	private:
		interface IExRichEditOleCallback : public IRichEditOleCallback
		{
		public:
			IExRichEditOleCallback();
			virtual ~IExRichEditOleCallback();
			int m_iNumStorages;
			IStorage* pStorage;
			DWORD m_dwRef;

			virtual HRESULT STDMETHODCALLTYPE GetNewStorage(LPSTORAGE* lplpstg);
			virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject);
			virtual ULONG STDMETHODCALLTYPE AddRef();
			virtual ULONG STDMETHODCALLTYPE Release();
			virtual HRESULT STDMETHODCALLTYPE GetInPlaceContext(LPOLEINPLACEFRAME FAR *lplpFrame,
				LPOLEINPLACEUIWINDOW FAR *lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo);
 			virtual HRESULT STDMETHODCALLTYPE ShowContainerUI(BOOL fShow);
 			virtual HRESULT STDMETHODCALLTYPE QueryInsertObject(LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp);
 			virtual HRESULT STDMETHODCALLTYPE DeleteObject(LPOLEOBJECT lpoleobj);
 			virtual HRESULT STDMETHODCALLTYPE QueryAcceptData(LPDATAOBJECT lpdataobj, CLIPFORMAT FAR *lpcfFormat,
				DWORD reco, BOOL fReally, HGLOBAL hMetaPict);
 			virtual HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL fEnterMode);
 			virtual HRESULT STDMETHODCALLTYPE GetClipboardData(CHARRANGE FAR *lpchrg, DWORD reco, LPDATAOBJECT FAR *lplpdataobj);
 			virtual HRESULT STDMETHODCALLTYPE GetDragDropEffect(BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect);
 			virtual HRESULT STDMETHODCALLTYPE GetContextMenu(WORD seltyp, LPOLEOBJECT lpoleobj, CHARRANGE FAR *lpchrg,
				HMENU FAR *lphmenu);
		};

};

class CMapPropertiesDialog:public CDialog{
	public:
		CMapPropertiesDialog(CGameMap * _pGameMap= NULL,CWnd * pParentWnd = NULL);
		
	   enum {IDD = IDD_MAP_PROPERTIES };		
	private:

		CGameMap * pGameMap;	

		virtual void DoDataExchange(CDataExchange *pDX);
	public:
		CString sWallSet;
		CString sMapName;
		CString csDescription;
		CString csImageFileName;
		CString sTextureSet;
		CString sSkySystem;
		CString sGrassType;
		UINT uiHours, uiMinutes;
		UINT uiFogColor;
		CListBox GameModsListBox;
		std::vector<CString> vGameMods; // list of mods supported by this map
			
	private:
		afx_msg void OnLoadSkySystem();
		afx_msg void OnLoadTerrainTex();
		afx_msg void OnLoadGrass();
		afx_msg void OnLoadWallSet();
		afx_msg void OnLoadLogo();

		DECLARE_MESSAGE_MAP()
		// multi-select listbox containing the list of available GameMods
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
		afx_msg void OnBnClickedOk();
};

