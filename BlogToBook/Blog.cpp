#include "stdafx.h"
#include "Blog.h"
#include <propkey.h>

#include "wininet.h"
#include "resource.h"       // main symbols
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CBlog::CBlog()
{
	m_YearStart = m_MonthStart = m_YearEnd = m_MonthEnd = 0;
}


CBlog::~CBlog()
{
}


BOOL CBlog::CheckDate(CString str, int type)
{
	if (str.GetLength() != 6) return FALSE;

	if (str.SpanIncluding(_T("0123456789")) == str)
	{
		int y, m;
		CString s = str.Left(4);
		y = _ttoi(s);
		s = str.Right(2);
		m = _ttoi(s);

		if (y<1999) return FALSE;
		if (m<1) return FALSE;
		if (m>12) return FALSE;

		if (type == 1)
		{
			m_YearStart = y;
			m_MonthStart = m;
		}
		if (type == 2)
		{
			m_YearEnd = y;
			m_MonthEnd = m;
		}
	}
	else return FALSE;

	return TRUE;
}

BOOL CBlog::GetDateRange()
{
	CMFCRibbonBar* pRibbon = ((CFrameWndEx*)AfxGetMainWnd())->GetRibbonBar();
	ASSERT_VALID(pRibbon);

	CMFCRibbonEdit* ribbonEditFrom = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_FETCHFROM));
	BOOL res1 = CheckDate(ribbonEditFrom->GetEditText(), 1);

	CMFCRibbonEdit* ribbonEditTo = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_FETCHTO));
	BOOL res2 = CheckDate(ribbonEditTo->GetEditText(), 2);

	BOOL res3 = (m_YearEnd > m_YearStart);
	if (m_YearEnd == m_YearStart)
	{
		res3 = (m_MonthEnd >= m_MonthStart);
	}

	if (res1&&res2&&res3) return TRUE;
	return FALSE;
}

BOOL CBlog::GetBlogInfo()
{
	CMFCRibbonBar* pRibbon = ((CFrameWndEx*)AfxGetMainWnd())->GetRibbonBar();
	ASSERT_VALID(pRibbon);

	CMFCRibbonEdit* ribbonEditUrl = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_ADDR));
	m_BlogUrl = ribbonEditUrl->GetEditText();
	if (!PathIsURL(m_BlogUrl)) return FALSE;

	int len = m_BlogUrl.GetLength() - 1;
	if (m_BlogUrl.Find('/', len) > 0) m_BlogUrl.Truncate(len);


	CMFCRibbonEdit* ribbonEditName = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_BNAME));
	m_BlogName = ribbonEditName->GetEditText();

	CMFCRibbonEdit* ribbonEditAuth = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_AUTH));
	m_BlogAuthor = ribbonEditAuth->GetEditText();

	if (m_BlogName.IsEmpty()) return FALSE;
	if (m_BlogAuthor.IsEmpty()) return FALSE;

	return TRUE;
}

BOOL CBlog::SetBlogInfo()
{
	CMFCRibbonBar* pRibbon = ((CFrameWndEx*)AfxGetMainWnd())->GetRibbonBar();
	ASSERT_VALID(pRibbon);
	if (!pRibbon) return FALSE;

	CMFCRibbonEdit* ribbonEdit = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_ADDR));
	ribbonEdit->SetEditText(m_BlogUrl);

	ribbonEdit = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_BNAME));
	ribbonEdit->SetEditText(m_BlogName);

	ribbonEdit = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_AUTH));
	ribbonEdit->SetEditText(m_BlogAuthor);

	CString str;
	str.Format(_T("%d%d"), m_YearStart, m_MonthStart);
	ribbonEdit = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_FETCHFROM));
	ribbonEdit->SetEditText(str);

	str.Format(_T("%d%d"), m_YearEnd, m_MonthEnd);
	ribbonEdit = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_FETCHTO));
	ribbonEdit->SetEditText(str);

	return TRUE;
}


void CBlog::Clear()
{
	m_YearStart = m_MonthStart = m_YearEnd = m_MonthEnd = 0;
	m_BlogUrl = _T("");
	m_BlogName = _T("Untitled Blog"); 
	m_BlogAuthor = _T("Anonymous Author"); 
	m_BlogDesc = _T("This book was generated using Blog To Book FOSS by Oormi Creations"); 
	m_BlogPlublisher = m_BlogAuthor;
	m_BlogPubDate = CTime::GetCurrentTime().Format("%Y-%m-%d");
	m_BlogLang = _T("English");
	m_BlogModDate = CTime::GetCurrentTime().Format("%Y-%m-%dT%H:%M:%SZ");
	m_BlogISBN = _T("NA");
	
	m_CopyrightTitle = m_BlogName;
	m_Copyright = m_BlogAuthor + _T("\r\n\r\nThis Ebook named ") + m_BlogName + _T(" is copyrighted and is a original work of ") + m_BlogAuthor + _T(". This Ebook may not be published, copied, sold or distributed without the proper consent of the author(s) and publisher(s).");
	
	m_PrefaceTitle = _T("Preface");
	m_Preface = _T("This is the preface for your book.\r\n\r\n") + m_BlogAuthor;
	
	m_DedicationTitle = _T("~ .o. ~");
	m_Dedication = _T("This book is dedicated to someone unknown.");
	
	m_EndPageTitle = _T("~ .o. ~");
	m_EndPage = _T("The End");

	m_CoverPath = _T("book_cover.jpg");
	m_CoverTitle = _T("Cover");

	m_IncludeB2BRef = TRUE;

	CString data[6];
	for (int i = 0; i < MAXARTICLES; i++)
	{
		SetArticle(i, data);
	}
}

void CBlog::SetDemoData()
{
	Clear();
	m_BlogName = _T("Blog To Book");
	m_BlogAuthor = _T("Oormi Creations");
	m_BlogPlublisher = m_BlogAuthor;
	m_Copyright = m_BlogAuthor + _T("\r\n\r\nThis Ebook named ") + m_BlogName + _T(" is copyrighted and is a original work of ") + m_BlogAuthor + _T(". This Ebook may not be published, copied, sold or distributed without the proper consent of the author(s) and publisher(s).");
	m_Preface = _T("This is a demo EBook created using Blog to Book.\r\n\r\nThis demo EBook contains a few articles from the blog Pure Experiences.\r\n\r\nThis application is free and open sourced. Hopefully it will be useful for the readers.\r\n\r\n") + m_BlogAuthor;

	CMFCRibbonBar* pRibbon = ((CFrameWndEx*)AfxGetMainWnd())->GetRibbonBar();
	ASSERT_VALID(pRibbon);

	CMFCRibbonEdit* ribbonEditUrl = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_ADDR));
	ribbonEditUrl->SetEditText(_T("https://oormicreations.blogspot.com/"));

	CMFCRibbonEdit* ribbonEditName = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_BNAME));
	ribbonEditName->SetEditText(m_BlogName);

	CMFCRibbonEdit* ribbonEditAuth = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_AUTH));
	ribbonEditAuth->SetEditText(m_BlogAuthor);

	CMFCRibbonEdit* ribbonEditFrom = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_FETCHFROM));
	ribbonEditFrom->SetEditText(_T("201710"));

	CMFCRibbonEdit* ribbonEditTo = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_FETCHTO));
	ribbonEditTo->SetEditText(_T("201805"));

	SetBookInfo();
}

void CBlog::SetArticle(int arNum, CString title, CString date, CString url, BOOL enabled, CString rfile, CString pfile)
{
	m_Articles[arNum].m_Title = title;
	m_Articles[arNum].m_Date = date;
	m_Articles[arNum].m_Url = url;
	m_Articles[arNum].m_Included = enabled;
	m_Articles[arNum].m_RawFile = rfile;
	m_Articles[arNum].m_PreviewFile = pfile;

}

void CBlog::SetArticle(int arNum, CString * data)
{
	m_Articles[arNum].m_Included = (data[0] == _T("Enabled"));
	m_Articles[arNum].m_Date = data[1];
	m_Articles[arNum].m_Title = data[2];
	m_Articles[arNum].m_Url = data[3];
	m_Articles[arNum].m_RawFile = data[4];
	m_Articles[arNum].m_PreviewFile = data[5];

}

void CBlog::GetArticle(int arNum, CString * data)
{
	data[0] = _T("Disabled");
	if(m_Articles[arNum].m_Included) (data[0] = _T("Enabled"));
	data[1] = m_Articles[arNum].m_Date;
	data[2] = m_Articles[arNum].m_Title;
	data[3] = m_Articles[arNum].m_Url;
	data[4] = m_Articles[arNum].m_RawFile;
	data[5] = m_Articles[arNum].m_PreviewFile;

}


CString CBlog::GetFirstName()
{
	int s = m_BlogAuthor.Find(_T(" "));
	if (s < 1)return m_BlogAuthor;
	return m_BlogAuthor.Left(s);
}

CString CBlog::GetLastName()
{
	int s = m_BlogAuthor.Find(_T(" "));
	if (s < 1)return m_BlogAuthor;
	return m_BlogAuthor.Right(s-3).Trim();
}

void CBlog::SetBookInfo()
{
	CMFCRibbonBar* pRibbon = ((CFrameWndEx*)AfxGetMainWnd())->GetRibbonBar();
	ASSERT_VALID(pRibbon);

	CMFCRibbonEdit* ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_BOOK_DESC));
	ribbonItem->SetEditText(m_BlogDesc);

	ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_BOOK_PUB));
	ribbonItem->SetEditText(m_BlogPlublisher);

	ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_BOOK_PUBDATE));
	ribbonItem->SetEditText(m_BlogPubDate);

	ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_BOOK_LANG));
	ribbonItem->SetEditText(m_BlogLang);

	ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_BOOK_ISBN));
	ribbonItem->SetEditText(m_BlogISBN);

	CMainFrame * fwnd = (CMainFrame*)AfxGetMainWnd();
	//fwnd->m_Ref = m_IncludeB2BRef;

	//Cannot set the checkbox in ribbon
	CMFCRibbonCheckBox* ribbonItemChk = DYNAMIC_DOWNCAST(CMFCRibbonCheckBox, pRibbon->FindByID(ID_CHECK_REF));
	if (ribbonItemChk->IsChecked() != m_IncludeB2BRef)
	{
		//does not work at app launch
		//PostMessage(fwnd->GetSafeHwnd(), WM_COMMAND, MAKEWPARAM(ID_CHECK_REF, BN_CLICKED), (LPARAM)NULL);
	}


}

void CBlog::GetBookInfo()
{
	CMFCRibbonBar* pRibbon = ((CFrameWndEx*)AfxGetMainWnd())->GetRibbonBar();
	ASSERT_VALID(pRibbon);

	CMFCRibbonEdit* ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_BOOK_DESC));
	m_BlogDesc = ribbonItem->GetEditText();

	ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_BOOK_PUB));
	m_BlogPlublisher = ribbonItem->GetEditText();

	ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_BOOK_PUBDATE));
	m_BlogPubDate = ribbonItem->GetEditText();

	ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_BOOK_LANG));
	m_BlogLang = ribbonItem->GetEditText();

	ribbonItem = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pRibbon->FindByID(ID_EDIT_BOOK_ISBN));
	m_BlogISBN = ribbonItem->GetEditText();

	CMainFrame * fwnd = (CMainFrame*)AfxGetMainWnd();
	m_IncludeB2BRef = fwnd->m_Ref;

}