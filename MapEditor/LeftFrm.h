#pragma once

class CLeftFrame : public CFrameWnd
{
	
protected: // create from serialization only
	
	CLeftFrame();
	DECLARE_DYNCREATE(CLeftFrame);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs,
		CCreateContext* pContext);

// Attributes
public:
	
// Operations
public:

// Overrides
public:
	//virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CSplitterWnd m_wndSplitter;
// Generated message map functions
protected:

	//afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
};
