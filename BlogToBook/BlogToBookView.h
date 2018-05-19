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

// BlogToBookView.h : interface of the CBlogToBookView class
//

#pragma once
#define MAXDISPPAGES 1000
#define FETCH_THREAD_NOTIFY (WM_APP + 2)


class CBlogToBookView : public CView
{
protected: // create from serialization only
	CBlogToBookView();
	DECLARE_DYNCREATE(CBlogToBookView)

// Attributes
public:
	CBlogToBookDoc* GetDocument() const;
	BOOL m_Render, m_ChapterRendered;
	double m_PageScale;
	double m_Margins;
	int m_CharsDrawn, m_TotalCharsDrawn/*, m_LastCharsDrawn*/;
	int m_PixPerInch;
	CRect m_PreRect, m_NxtRect, m_PageRect;
	LONG m_PageOffset[1000];
	int m_PageNum, m_ChapterNum;
	CListCtrl m_ArList;
	//int m_ChapterList[MAXARTICLES];

	BOOL m_bIsDragging;
	int m_TopY, m_LastY;
	
	int m_BodyFontSize, m_TitleFontSize;
	CString m_TitleFont, m_BodyFont;

// Operations
public:
	void DrawNavButtons(CDC *pDC, CRect rect);
	CRect DrawPage(CDC *pDC);
	void DrawCover(CDC * pDC, CString imagePath, CRect pageRect);
	int DrawChaperTitle(CDC * pDC, CString str, CRect * rect, int num);
	void PopulateList();
	BOOL CreateArList(CRect crect);
	void UpdateChapterList();
	void UpdateRenderFonts( int BFontSz, int TFontSz, CString BFont, CString TFont);
	bool IsInstalled(LPCTSTR lpszFont);
	//int EnumFontFamExProc(ENUMLOGFONTEX *, NEWTEXTMETRICEX *, int, LPARAM lParam);



// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CBlogToBookView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	//afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnButtonRender();
	afx_msg void OnSliderPagescale();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnButtonFit();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonList();
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	virtual void OnInitialUpdate();
	afx_msg void OnArListChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnArListDraw(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSpinBodySize();
	afx_msg void OnSpinTitleSize();
	afx_msg void OnComboFontTitle();
	afx_msg void OnComboFontBody();
	afx_msg void OnButtonResetFont();
	afx_msg LRESULT OnFetchThreadNotify(WPARAM, LPARAM);

};

#ifndef _DEBUG  // debug version in BlogToBookView.cpp
inline CBlogToBookDoc* CBlogToBookView::GetDocument() const
   { return reinterpret_cast<CBlogToBookDoc*>(m_pDocument); }
#endif

