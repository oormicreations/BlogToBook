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

// BlogToBookView.cpp : implementation of the CBlogToBookView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "BlogToBook.h"
#endif

#include "BlogToBookDoc.h"
#include "BlogToBookView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBlogToBookView

IMPLEMENT_DYNCREATE(CBlogToBookView, CView)

BEGIN_MESSAGE_MAP(CBlogToBookView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CBlogToBookView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CBlogToBookView construction/destruction

CBlogToBookView::CBlogToBookView()
{
	// TODO: add construction code here

}

CBlogToBookView::~CBlogToBookView()
{
}

BOOL CBlogToBookView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CBlogToBookView drawing

void CBlogToBookView::OnDraw(CDC* /*pDC*/)
{
	CBlogToBookDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CBlogToBookView printing


void CBlogToBookView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CBlogToBookView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CBlogToBookView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CBlogToBookView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CBlogToBookView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CBlogToBookView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CBlogToBookView diagnostics

#ifdef _DEBUG
void CBlogToBookView::AssertValid() const
{
	CView::AssertValid();
}

void CBlogToBookView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CBlogToBookDoc* CBlogToBookView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBlogToBookDoc)));
	return (CBlogToBookDoc*)m_pDocument;
}
#endif //_DEBUG


// CBlogToBookView message handlers
