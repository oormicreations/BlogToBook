// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// BlogToBookDoc.h : interface of the CBlogToBookDoc class
//
#include "Blog.h"
#include "NewProjectDlg.h"


#pragma once
//#define MAXTITLES 50 //in a month
#define MAXMONTHS 240 //20 years
#define B2B_FOLDER 1 
#define B2B_FOLDER_BLANK 2 

class CBlogToBookDoc : public CDocument
{
protected: // create from serialization only
	CBlogToBookDoc();
	DECLARE_DYNCREATE(CBlogToBookDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CBlogToBookDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	UINT m_B2BVersion;
	CBlog m_Blog;
	
	CString m_BlogPageRaw, m_BlogPagePreview, m_Status, m_Index, m_B2BRef, m_B2BRefPre;
	CString /*m_UserPath,*/ m_BlankPath, m_ProjectPath, m_ProjectName, m_RawDataPath, m_BookFile, m_B2BFile;

	CString m_Titles[MAXARTICLES];
	CString m_TitleUrls[MAXARTICLES];
	CString m_TitleDates[MAXARTICLES];
	
	BOOL m_IsFetched, m_ListEdited, m_bListChanged, m_IsProjectLoaded, m_IsCancelled, m_IsInstalled;
	
	UINT m_TitleCount, m_ChapterCount;
	int m_ChapterList[MAXARTICLES];
	int m_ChapterNumDisp[MAXARTICLES];
	
	int m_BFontSz, m_TFontSz;
	CString m_BFont, m_TFont;

	CString m_SCEntries[3];
	CString m_SCID, m_SCPass;


	BOOL Fetch(CString url);
	BOOL ParseTitles();
	BOOL SavePages();
	BOOL SaveFile(CString fname, CString path, CString data);

	CString LoadPage(CString fpath);
	BOOL BuildBook();
	CString GetPreview(int npage = 0);
	bool SHCopy(LPCTSTR from, LPCTSTR to);
	bool SHDelete(LPCTSTR from);
	void Clear();
	void ShowCaption(CString str);
	void SetArticleList();
	int GetArticleCount();
	void SetAllArticles();
	CArticle GetArticle(int arNum);
	HRESULT ZipIt(CString strSrc, CString strDest);
	BOOL IsFileLocked(CString path);
	BOOL SaveIndex();
	CString GetArTitle(int arNum);
	CString GetUserDocumentPath(UINT type);
	void SetProjectPaths();
	void EditAttributes(int arNum);
	void SetRenderFonts();

	void UpdateB2BData();
	
	void ReportUsage();
	void SaveSettings();
	void LoadSettings();
	CString GetSCID();
	HRESULT SaveCoverThumb();

	afx_msg void OnButtonFetch();
	afx_msg void OnButtonSaveepub();
	afx_msg void OnEditfetchfrom();
	afx_msg void OnEditfetchto();
	afx_msg void OnEditAddr();
	afx_msg void OnEditbname();
	afx_msg void OnEditAuth();
	afx_msg void OnButtonDemo();
	afx_msg void OnButtonShowfolder();
	afx_msg void OnButtonShowcase();
	afx_msg void OnButtonValid();
	afx_msg void OnEditBookDesc();
	afx_msg void OnEditBookPub();
	afx_msg void OnEditBookPubdate();
	afx_msg void OnEditBookLang();
	afx_msg void OnEditBookIsbn();
	afx_msg void OnButtonCover();
	afx_msg void OnButtonPreface();
	afx_msg void OnButtonLic();
	afx_msg void OnButtonEndpage();
	afx_msg void OnButtonDed();
	afx_msg void OnButtonEdlist();
	afx_msg void OnButtonCalibre();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual POSITION GetFirstViewPosition() const;
	virtual BOOL SaveModified();
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
	virtual void SetTitle(LPCTSTR lpszTitle);
	afx_msg void OnButtonHelp();
	afx_msg void OnButtonUpdate();
};
