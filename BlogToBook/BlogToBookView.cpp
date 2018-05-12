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
#include "MainFrm.h"

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
	//ON_WM_RBUTTONUP()
	ON_COMMAND(ID_BUTTON_RENDER, &CBlogToBookView::OnButtonRender)
	ON_COMMAND(ID_SLIDER_PAGESCALE, &CBlogToBookView::OnSliderPagescale)
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_BUTTON_FIT, &CBlogToBookView::OnButtonFit)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_BUTTON_LIST, &CBlogToBookView::OnButtonList)
	ON_WM_PARENTNOTIFY()
	ON_NOTIFY(LVN_ITEMCHANGED, ID_ARTICLE_LIST, &CBlogToBookView::OnArListChanged)
	ON_NOTIFY(NM_CUSTOMDRAW, ID_ARTICLE_LIST, &CBlogToBookView::OnArListDraw)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_SPIN_BODY_SIZE, &CBlogToBookView::OnSpinBodySize)
	ON_COMMAND(ID_SPIN_TITLE_SIZE, &CBlogToBookView::OnSpinTitleSize)
	ON_COMMAND(ID_COMBO_FONT_TITLE, &CBlogToBookView::OnComboFontTitle)
	ON_COMMAND(ID_COMBO_FONT_BODY, &CBlogToBookView::OnComboFontBody)
	ON_COMMAND(ID_BUTTON_RESET_FONT, &CBlogToBookView::OnButtonResetFont)
	ON_MESSAGE(FETCH_THREAD_NOTIFY, OnFetchThreadNotify)
END_MESSAGE_MAP()

LRESULT CBlogToBookView::OnFetchThreadNotify(WPARAM wp, LPARAM lp)
{
	CMainFrame * fwnd = (CMainFrame*)AfxGetMainWnd();
	CBlogToBookDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return 0;

	CString str;

	if (wp > 0)
	{
		CString s;
		s.Format(_T("%d"), lp);
		m_ArList.InsertItem(lp, s);
		s.Format(_T("%d-%02d"), wp / 100, wp % 100);
		m_ArList.SetItemText(lp, 2, s);
		s.Format(_T("Found: %d Articles"), lp);
		m_ArList.SetItemText(1, 3, s);

	}
	else
	{
		if (lp == 1) str = _T("Error fetching articles. Check your internet connection or address.");
		if (lp == 2) pDoc->m_Blog.SetBookInfo();
		if (lp == 3)
		{
			Invalidate(); 
			str = _T("Finished fetching articles.");
		}
	}

	fwnd->SetCaption(str);

	return 0;
}

// CBlogToBookView construction/destruction

CBlogToBookView::CBlogToBookView()
{
	// TODO: add construction code here
	m_Render = FALSE;
	m_PageScale = 1.0;
	m_Margins = 1.0; //inches
	m_PixPerInch = 96; //overwritten by getdevicecaps

	m_CharsDrawn = 0;
	m_TotalCharsDrawn = 0;

	m_PreRect.SetRectEmpty();
	m_NxtRect.SetRectEmpty();

	for (int i = 0; i < 1000; i++)
	{
		m_PageOffset[i] = 0;
	}

	m_PageNum = 0;
	m_ChapterNum = 0;

	m_bIsDragging = FALSE;
	m_TopY = m_LastY = 10;

	m_BodyFontSize = 14;
	m_TitleFontSize = 18;
	m_BodyFont = _T("Garamond");
	m_TitleFont = _T("Garamond");
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
CRect CBlogToBookView::DrawPage(CDC * pDC)
{
	CBrush backBrush;
	backBrush.CreateSolidBrush(RGB(200, 200, 200));
	CBrush pageBrush;
	pageBrush.CreateSolidBrush(RGB(255, 255, 255));

	CRect clientRect;
	GetClientRect(&clientRect);
	pDC->FillRect(&clientRect, &backBrush);

	int pxPerInchX = pDC->GetDeviceCaps(LOGPIXELSX);
	int pxPerInchY = pDC->GetDeviceCaps(LOGPIXELSY);

	double pageWidthInch = 8.5 * m_PageScale;
	double pageHeightInch = 11.0 * m_PageScale;

	int pageWidthPixels = (int)(pxPerInchX*pageWidthInch);
	int pageHeightPixels = (int)(pxPerInchY*pageHeightInch);


	CRect pageRect;

	pageRect.left = ((clientRect.right - clientRect.left) / 2) - (pageWidthPixels / 2);
	pageRect.right = ((clientRect.right - clientRect.left) / 2) + (pageWidthPixels / 2);

	//pan up and down
	pageRect.top = clientRect.top + m_TopY;// 10;
	pageRect.bottom = pageHeightPixels + m_TopY;// 10;

	//restrict pan
	if (pageRect.bottom < (clientRect.bottom -10))
	{
		pageRect.bottom = clientRect.bottom - 10;
		pageRect.top = pageRect.bottom - pageHeightPixels;
	}
	if (pageRect.top > (clientRect.top + 10))
	{
		pageRect.top = clientRect.top + 10;
		pageRect.bottom = pageHeightPixels;
	}

	pDC->FillRect(&pageRect, &pageBrush);

	int shadowWidth = 4;
	CPen shadowPen(PS_SOLID, shadowWidth, RGB(150, 150, 150));
	CPen * oldPen = pDC->SelectObject(&shadowPen);

	shadowWidth--;
	pDC->MoveTo(pageRect.right + shadowWidth, pageRect.top + shadowWidth);
	pDC->LineTo(pageRect.right + shadowWidth, pageRect.bottom + shadowWidth);
	pDC->LineTo(pageRect.left + shadowWidth, pageRect.bottom + shadowWidth);

	pDC->SelectObject(oldPen);
	shadowPen.DeleteObject();
	backBrush.DeleteObject();
	pageBrush.DeleteObject();

	m_PixPerInch = pxPerInchX;
	return pageRect;
}

int CBlogToBookView::DrawChaperTitle(CDC * pDC, CString str, CRect * rect, int num)
{
	CBlogToBookDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return 0;

	int chars = 0;

	if (str.GetLength() > 0)
	{
		DRAWTEXTPARAMS dtParams;
		dtParams.cbSize = sizeof(DRAWTEXTPARAMS);
		dtParams.iLeftMargin = 0;
		dtParams.iRightMargin = 0;
		dtParams.iTabLength = 1;

		//draw chap num
		if ((num > 0)/* && (num < pDoc->m_ChapterCount-MAXEXTRAPAGES)*/)
		{
			CString cNum;
			cNum.Format(_T("Chapter %d"), num);

			CFont renderFont;
			renderFont.CreatePointFont((int)(m_BodyFontSize * 10 * m_PageScale), m_BodyFont);

			CFont * oldFont = pDC->SelectObject(&renderFont);

			int ht1 = pDC->DrawTextEx(cNum.GetBuffer(), -1, rect, DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL, &dtParams);
			chars = dtParams.uiLengthDrawn;

			renderFont.DeleteObject();

			rect->top += ht1 * 2;
		}

		//draw chap title
		CFont chapterFont;
		chapterFont.CreatePointFont((int)(m_TitleFontSize * 10 * m_PageScale), m_TitleFont + _T(" Bold"));

		CFont * oldFont2 = pDC->SelectObject(&chapterFont);
		int ht2 = pDC->DrawTextEx(str.GetBuffer(), -1, rect, DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL, &dtParams);
		chars += dtParams.uiLengthDrawn + 21;

		rect->top += ht2 * 2;

		pDC->SelectObject(oldFont2);
		chapterFont.DeleteObject();

	}

	return chars;
}

void CBlogToBookView::PopulateList()
{
	CBlogToBookDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;

	m_ArList.DeleteAllItems();

	CHeaderCtrl* pHeader = (CHeaderCtrl*)m_ArList.GetHeaderCtrl();
	int colCount = pHeader->GetItemCount();

	CString line, token;
	int start1 = 0;
	int row = 0;

	while (start1 >= 0)
	{
		line = pDoc->m_Index.Tokenize(_T("\r\n"), start1);
		if (!line.IsEmpty())
		{
			int col = 1;
			int start2 = 0;

			token = line.Tokenize(_T("|"), start2);
			if (start2 >= 0)
			{
				CString str;
				str.Format(_T("%d"), row+1);
				m_ArList.InsertItem(row, str);
				m_ArList.SetItemText(row, col, token);
				col++;
				if(token==_T("Enabled")) m_ArList.SetCheck(row);
			}

			while (start2 >= 0)
			{
				token = line.Tokenize(_T("|"), start2);
				if(start2 >= 0) m_ArList.SetItemText(row, col, token);
				if (col == 3)pDoc->m_Titles[row] = token;
				col++;
				if (col > 6)break;
			}
		}
		row++;
		if (row > MAXARTICLES)break;
	}

	OnButtonList();
}

void CBlogToBookView::DrawNavButtons(CDC * pDC, CRect rect)
{
	int szNav = 40;
	int szNavSep = 10;

	CBrush navBrush;
	navBrush.CreateSolidBrush(RGB(220, 220, 220));

	//CRect m_PreRect;
	m_PreRect.top = 20;// rect.top - m_TopY;
	m_PreRect.left = rect.left - szNav - szNavSep;
	m_PreRect.right = rect.left - szNavSep;
	m_PreRect.bottom = rect.Height() - 20;// rect.bottom;

	pDC->FillRect(&m_PreRect, &navBrush);

	//CRect m_NxtRect;
	m_NxtRect.top = 20;// rect.top - m_TopY;
	m_NxtRect.right = rect.right + szNav + szNavSep;
	m_NxtRect.left = rect.right + szNavSep;
	m_NxtRect.bottom = rect.Height() - 20;//rect.bottom;

	pDC->FillRect(&m_NxtRect, &navBrush);


	CBrush arrBrush;
	arrBrush.CreateSolidBrush(RGB(230, 230, 230));
	CBrush * oldbrush = pDC->SelectObject(&arrBrush);

	CPen arrPen(PS_SOLID, 2, RGB(240, 240, 240));
	CPen* pOldPen = pDC->SelectObject(&arrPen);

	int mid = (m_PreRect.bottom - m_PreRect.top) / 2;
	int ds = 5, ht = 50;

	CPoint arrowLeft[3];
	arrowLeft[0].x = m_PreRect.right - ds;
	arrowLeft[0].y = mid - ht;
	arrowLeft[1].x = m_PreRect.left + ds;
	arrowLeft[1].y = mid;
	arrowLeft[2].x = m_PreRect.right - ds;
	arrowLeft[2].y = mid + ht;

	CPoint arrowRight[3];
	arrowRight[0].x = m_NxtRect.left + ds;
	arrowRight[0].y = mid - ht;
	arrowRight[1].x = m_NxtRect.right - ds;
	arrowRight[1].y = mid;
	arrowRight[2].x = m_NxtRect.left + ds;
	arrowRight[2].y = mid + ht;

	pDC->Polygon(arrowLeft, 3);
	pDC->Polygon(arrowRight, 3);

	pDC->SelectObject(&oldbrush);
	pDC->SelectObject(pOldPen);
	arrBrush.DeleteObject();
	navBrush.DeleteObject();

}

void CBlogToBookView::DrawCover(CDC * pDC, CString imagePath, CRect pageRect)
{
	CImage image;
	HRESULT hr = image.Load(imagePath);
	if (hr == S_OK)
	{
		int h = image.GetHeight();
		int w = image.GetWidth();
		
		double ratio =  (double)pageRect.Height()/ (double)h;
		if (w > h) ratio = (double)pageRect.Width()/(double)w;

		int iNewWidth = (int)(ratio*(double)w);
		int iNewHeight = (int)(ratio*(double)h);

		int iPosX = pageRect.left + (pageRect.Width() - iNewWidth) / 2;
		int iPosY = pageRect.top + (pageRect.Height() - iNewHeight) / 2;

		CDC *screenDC = GetDC();
		CDC *pMDC = new CDC;
		pMDC->CreateCompatibleDC(screenDC);

		CBitmap *pb = new CBitmap;
		pb->CreateCompatibleBitmap(screenDC, iNewWidth, iNewHeight);

		CBitmap *pob = pMDC->SelectObject(pb);
		SetStretchBltMode(pMDC->GetSafeHdc(), COLORONCOLOR);

		image.StretchBlt(pMDC->m_hDC, 0, 0, iNewWidth, iNewHeight, 0, 0, image.GetWidth(), image.GetHeight(), SRCCOPY);
		pMDC->SelectObject(pob);

		CImage new_image;
		new_image.Attach((HBITMAP)(*pb));
		//new_image.Save(_T("c:\\NewImage.jpg"));
		new_image.Draw(pDC->GetSafeHdc(), iPosX, iPosY);

		new_image.Detach();
		ReleaseDC(screenDC);
		pMDC->DeleteDC();
		pb->DeleteObject();
		new_image.Destroy();
		delete pb;
		delete pMDC;
	}

	m_ChapterRendered = TRUE;
}


void CBlogToBookView::OnDraw(CDC* pDC)
{
	CBlogToBookDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
	
	if (m_Render)
	{
		CRect pageRect;
		pageRect = DrawPage(pDC);
		DrawNavButtons(pDC, pageRect);

		CRect fullRect(pageRect);
		m_PageRect = fullRect;

		int marginInPix = (int)(m_Margins*m_PixPerInch*m_PageScale);
		pageRect.DeflateRect(marginInPix, marginInPix);

		CString str;

		m_TotalCharsDrawn = 0;
		for (int i = 0; i < m_PageNum; i++)
		{
			m_TotalCharsDrawn += m_PageOffset[i];
		}

		str = pDoc->GetPreview(pDoc->m_ChapterList[m_ChapterNum]);

		//b2bref
		if (pDoc->m_ChapterList[m_ChapterNum] == pDoc->m_TitleCount-1)
		{
			CMainFrame * fwnd = (CMainFrame *)AfxGetMainWnd();
			if (fwnd->m_Ref) str = str + pDoc->m_B2BRefPre;
		}

		if (pDoc->m_ChapterList[m_ChapterNum] == 0)
		{
			DrawCover(pDC, str, fullRect);
			return;
		}

		str = str.Right(str.GetLength() - m_TotalCharsDrawn);

		if (m_PageNum == 0)
		{
			CString title = pDoc->GetArTitle(pDoc->m_ChapterList[m_ChapterNum]);
			DrawChaperTitle(pDC, title, &pageRect, pDoc->m_ChapterNumDisp[m_ChapterNum]);
		}

		//convert to utf-8
		//str = CA2CT(str.GetBuffer(), CP_UTF8);


		CFont renderFont;
		renderFont.CreatePointFont((int)(m_BodyFontSize * 10 * m_PageScale), m_BodyFont);
		CFont * oldfont = pDC->SelectObject(&renderFont);

		DRAWTEXTPARAMS dtParams;
		dtParams.cbSize = sizeof(DRAWTEXTPARAMS);
		dtParams.iLeftMargin = 0;
		dtParams.iRightMargin = 0;
		dtParams.iTabLength = 1;

		pDC->DrawTextEx(str.GetBuffer(), -1, &pageRect, DT_LEFT| DT_WORDBREAK| DT_NOPREFIX| DT_EDITCONTROL, &dtParams);
		m_CharsDrawn = dtParams.uiLengthDrawn;

		m_PageOffset[m_PageNum] = m_CharsDrawn;

		if (str.GetLength() == m_CharsDrawn)
		{
			m_ChapterRendered = TRUE;
		}

		//TRACE(_T("--------chars = %d, off=%d, total = %d, pg = %d\n"), m_CharsDrawn, m_PageOffset[m_PageNum], m_TotalCharsDrawn, m_PageNum);

		pDC->SelectObject(oldfont);
		renderFont.DeleteObject();

		pDoc->m_BFontSz = m_BodyFontSize;
		pDoc->m_TFontSz = m_TitleFontSize;
		pDoc->m_BFont = m_BodyFont;
		pDoc->m_TFont = m_TitleFont;

	}
	else
	{
		if (pDoc->m_ListEdited)
		{
			pDoc->SetAllArticles();
			pDoc->m_ListEdited = FALSE;
			pDoc->m_bListChanged = TRUE;
		}

		//Populate List
		if (pDoc->m_bListChanged)
		{
			CString str;
			CString data[6];

			m_ArList.DeleteAllItems();

			CHeaderCtrl* pHeader = (CHeaderCtrl*)m_ArList.GetHeaderCtrl();
			int colCount = pHeader->GetItemCount() - 1; //exclude first

			int entris = 0;
			for (UINT i = 0; i < MAXARTICLES; i++)
			{
				str.Format(_T("%d"), i + 1);
				pDoc->m_Blog.GetArticle(i, data);
				if (data[5].IsEmpty()) break;

				m_ArList.InsertItem(i, str);
				for (int j = 0; j < colCount; j++)
				{
					m_ArList.SetItemText(i, j+1, data[j]);
				}
				BOOL res = data[0] == _T("Enabled");
				if(res) m_ArList.SetCheck(i);
				entris++;
			}

			pDoc->m_TitleCount = entris;
			pDoc->m_Blog.m_ArticleCount = entris;
			pDoc->m_bListChanged = FALSE;
			OnButtonList();
		}

	}

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

//void CBlogToBookView::OnRButtonUp(UINT /* nFlags */, CPoint point)
//{
//	ClientToScreen(&point);
//	OnContextMenu(this, point);
//}

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


void CBlogToBookView::OnButtonRender()
{
	m_TopY = 10;

	CBlogToBookDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;

	if (!pDoc->m_IsProjectLoaded) return;

	pDoc->UpdateB2BData();//extra pages are not updated, just fields in the ribbon

	m_Render = TRUE;
	m_ChapterRendered = FALSE;
	m_ChapterNum = 0;
	m_CharsDrawn = 0;
	m_TotalCharsDrawn = 0;
	m_PageNum = 0;

	for (int i = 0; i < MAXDISPPAGES; i++)
	{
		m_PageOffset[i] = 0;
	}

	for (int i = 0; i < MAXARTICLES; i++)
	{
		pDoc->m_ChapterList[i] = 0;
		pDoc->m_ChapterNumDisp[i] = 0;
	}

	//update articles to be included
	UpdateChapterList();

	m_ArList.ShowWindow(SW_HIDE);
	Invalidate();
	pDoc->ShowCaption(_T("This is an approx render. Click left arrow to flip pages, right arrow to flip chapters. Drag mouse to scroll. Select your own cover or change styles. Edit Book Attributes."));
}

void CBlogToBookView::UpdateChapterList()
{
	CBlogToBookDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;

	pDoc->m_ChapterCount = 0;
	int ndisp = 0;
	for (int i = 0; i < m_ArList.GetItemCount(); i++)
	{
		BOOL chk = m_ArList.GetCheck(i);
		pDoc->m_Blog.m_Articles[i].m_Included = chk;
		if (chk)
		{
			pDoc->m_ChapterList[pDoc->m_ChapterCount] = i;
			if ((i > MAXEXTRAPAGES - 1) && ((UINT)i < pDoc->m_TitleCount-1))
			{
				ndisp++;
				pDoc->m_ChapterNumDisp[pDoc->m_ChapterCount] = ndisp;
			}
			pDoc->m_ChapterCount++;
		}
	}

}

void CBlogToBookView::OnSliderPagescale()
{
	CMFCRibbonBar* pRibbon = ((CFrameWndEx*)AfxGetMainWnd())->GetRibbonBar();
	ASSERT_VALID(pRibbon);
	CMFCRibbonSlider* ribbonSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, pRibbon->FindByID(ID_SLIDER_PAGESCALE));

	m_PageScale = (double)(ribbonSlider->GetPos())/100.0;

	CMFCRibbonEdit* ribbonLabel1 = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_ZOOM));
	CString str;
	str.Format(_T("%d%%"), ribbonSlider->GetPos());
	ribbonLabel1->SetEditText(str);
	Invalidate();
}


void CBlogToBookView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bIsDragging = FALSE;

	if (m_Render)
	{
		CBlogToBookDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc)
			return;

		if (m_PreRect.PtInRect(point))//show previous chapter, this will not show the previous page
		{
			m_PageNum = 0;
			m_ChapterNum--;
			if (m_ChapterNum < 0)
			{
				m_ChapterNum = pDoc->m_ChapterCount-1;
			}
			m_ChapterRendered = FALSE;
			m_CharsDrawn = 0;
			m_TotalCharsDrawn = 0;
			for (int i = 0; i < MAXDISPPAGES; i++)
			{
				m_PageOffset[i] = 0;
			}
			Invalidate();
		}

		if (m_NxtRect.PtInRect(point))
		{
			m_PageNum++;
			if (m_PageNum >= MAXDISPPAGES)m_PageNum = MAXDISPPAGES-1;
			if (m_ChapterRendered)
			{
				m_ChapterNum++;
				if (m_ChapterNum >= (int)pDoc->m_ChapterCount) m_ChapterNum = 0;//rotate
				m_ChapterRendered = FALSE;
				m_CharsDrawn = 0;
				m_TotalCharsDrawn = 0;
				m_PageNum = 0;
				for (int i = 0; i < MAXDISPPAGES; i++)
				{
					m_PageOffset[i] = 0;
				}
			}
			Invalidate();
		}
	}


	CView::OnLButtonUp(nFlags, point);
}


void CBlogToBookView::OnButtonFit()
{
	CDC *pDC = GetDC();
	int pxPerInchX = pDC->GetDeviceCaps(LOGPIXELSX);
	int pxPerInchY = pDC->GetDeviceCaps(LOGPIXELSY);
	CRect clientRect;
	GetClientRect(&clientRect);

	int ht = clientRect.Height();
	m_PageScale = 0.975*(double)ht / (double)(pxPerInchY * 11);

	m_TopY = m_LastY = 10;

	CMFCRibbonBar* pRibbon = ((CFrameWndEx*)AfxGetMainWnd())->GetRibbonBar();
	ASSERT_VALID(pRibbon);
	CMFCRibbonSlider* ribbonSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, pRibbon->FindByID(ID_SLIDER_PAGESCALE));
	ribbonSlider->SetPos((int)(m_PageScale * 100.0));

	CMFCRibbonEdit* ribbonLabel1 = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_ZOOM));
	CString str;
	str.Format(_T("%d%%"), (int)(m_PageScale * 100.0));
	ribbonLabel1->SetEditText(str);

	Invalidate();
}


int CBlogToBookView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	CRect crect;
	GetClientRect(&crect);
	CreateArList(crect);

	return 0;
}


BOOL CBlogToBookView::CreateArList(CRect rc)
{

	if(!m_ArList.Create(WS_CHILD | WS_VISIBLE | LVS_REPORT, rc, this, ID_ARTICLE_LIST)) return FALSE;
	m_ArList.SetExtendedStyle(m_ArList.GetStyle() | LVS_EX_CHECKBOXES);
	
	int nColWidth = 100;
	m_ArList.InsertColumn(0, _T("#"), LVCFMT_LEFT, nColWidth);
	m_ArList.InsertColumn(1, _T("Include"), LVCFMT_LEFT, nColWidth);
	m_ArList.InsertColumn(2, _T("Date"), LVCFMT_LEFT, nColWidth * 2);
	m_ArList.InsertColumn(3, _T("Article"), LVCFMT_LEFT, nColWidth * 3);
	m_ArList.InsertColumn(4, _T("Url"), LVCFMT_LEFT, nColWidth * 4);
	m_ArList.InsertColumn(5, _T("Raw File"), LVCFMT_LEFT, nColWidth);
	m_ArList.InsertColumn(6, _T("Preview File"), LVCFMT_LEFT, nColWidth);

	return TRUE;
}



void CBlogToBookView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	//CWnd * win = GetWindow(GW_CHILD);//this is the list control
	//if (win != NULL)
	//{
	//	win->SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_SHOWWINDOW);
	//	if (m_Render) win->ShowWindow(SW_HIDE);
	//}

	if (m_ArList.m_hWnd != NULL)
	{
		m_ArList.SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_SHOWWINDOW);
		if (m_Render) m_ArList.ShowWindow(SW_HIDE);
	}
}


void CBlogToBookView::OnButtonList()
{
	CBlogToBookDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;

	m_Render = FALSE;
	Invalidate();
	m_ArList.ShowWindow(SW_SHOW);

	pDoc->ShowCaption(_T("Include/exclude articles from the list. Or Render the pages for a preview."));
}


void CBlogToBookView::OnParentNotify(UINT message, LPARAM lParam)
{
	CView::OnParentNotify(message, lParam);

	// TODO: Add your message handler code here
}


void CBlogToBookView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	//CRect crect;
	//GetClientRect(&crect);
	//CreateArList(crect);
}

void CBlogToBookView::OnArListChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (pNMHDR->idFrom == ID_ARTICLE_LIST)
	{
		CBlogToBookDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc) return;

		int count = m_ArList.GetItemCount();
		for (int i = 0; i < count; i++)
		{
			pDoc->m_Blog.m_Articles[i].m_Included = m_ArList.GetCheck(i);
			pDoc->SetModifiedFlag(TRUE);

			if (m_ArList.GetCheck(i))
			{
				m_ArList.SetItemText(i, 1, _T("Enabled"));
			}
			else
			{
				m_ArList.SetItemText(i, 1, _T("Disabled"));
			}
		}
		
	}
}

void CBlogToBookView::OnArListDraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	
	switch (pNMCD->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:
			*pResult = CDRF_NOTIFYSUBITEMDRAW;
			break;
		case (CDDS_ITEMPREPAINT | CDDS_SUBITEM):
			pNMCD->clrText = RGB(0, 0, 0);
			if(pNMCD->iSubItem==3) pNMCD->clrText = RGB(0,0,180);//title in blue
			if(!m_ArList.GetCheck(pNMCD->nmcd.dwItemSpec)) pNMCD->clrText = RGB(170, 170, 170);//disabled in grey
			break;
	}
}

void CBlogToBookView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_bIsDragging = TRUE;
	m_LastY = 10;
	CView::OnLButtonDown(nFlags, point);
}


void CBlogToBookView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_PageRect.PtInRect(point))
	{
		if (m_bIsDragging)
		{
			int dy = 0;
			if(m_LastY!=10) dy = point.y - m_LastY;

			m_LastY = point.y;
			m_TopY += dy;
			
			Invalidate(FALSE);
		}
	}
	else
	{
		m_bIsDragging = FALSE;
		m_TopY = m_LastY = 10;
	}
	CView::OnMouseMove(nFlags, point);
}


void CBlogToBookView::OnSpinBodySize()
{
	CMFCRibbonBar* pRibbon = ((CFrameWndEx*)AfxGetMainWnd())->GetRibbonBar();
	ASSERT_VALID(pRibbon);

	CMFCRibbonEdit* ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_SPIN_BODY_SIZE));
	m_BodyFontSize = _ttoi( ribbonItem->GetEditText());

	Invalidate();
}


void CBlogToBookView::OnSpinTitleSize()
{
	CMFCRibbonBar* pRibbon = ((CFrameWndEx*)AfxGetMainWnd())->GetRibbonBar();
	ASSERT_VALID(pRibbon);

	CMFCRibbonEdit* ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_SPIN_TITLE_SIZE));
	m_TitleFontSize = _ttoi(ribbonItem->GetEditText());

	Invalidate();
}


void CBlogToBookView::OnComboFontTitle()
{
	CMFCRibbonBar* pRibbon = ((CFrameWndEx*)AfxGetMainWnd())->GetRibbonBar();
	ASSERT_VALID(pRibbon);

	CMFCRibbonEdit* ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_COMBO_FONT_TITLE));
	m_TitleFont = ribbonItem->GetEditText();

	Invalidate();
}


void CBlogToBookView::OnComboFontBody()
{
	CMFCRibbonBar* pRibbon = ((CFrameWndEx*)AfxGetMainWnd())->GetRibbonBar();
	ASSERT_VALID(pRibbon);

	CMFCRibbonEdit* ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_COMBO_FONT_BODY));
	m_BodyFont = ribbonItem->GetEditText();

	Invalidate();
}


void CBlogToBookView::OnButtonResetFont()
{
	UpdateRenderFonts(14, 18, _T("Garamond"), _T("Garamond"));
}

void CBlogToBookView::UpdateRenderFonts(int BFontSz, int TFontSz, CString BFont, CString TFont)
{
	CMFCRibbonBar* pRibbon = ((CFrameWndEx*)AfxGetMainWnd())->GetRibbonBar();
	ASSERT_VALID(pRibbon);

	m_BodyFontSize = BFontSz;
	m_TitleFontSize = TFontSz;
	m_BodyFont = BFont;
	m_TitleFont = TFont;

	CMFCRibbonEdit* ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_COMBO_FONT_BODY));
	ribbonItem->SetEditText(m_BodyFont);

	ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_COMBO_FONT_TITLE));
	ribbonItem->SetEditText(m_TitleFont);

	ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_SPIN_TITLE_SIZE));
	ribbonItem->SetEditText(_T("18"));

	ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_SPIN_BODY_SIZE));
	ribbonItem->SetEditText(_T("14"));


	Invalidate();
}