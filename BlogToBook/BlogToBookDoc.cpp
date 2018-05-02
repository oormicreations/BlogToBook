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

// BlogToBookDoc.cpp : implementation of the CBlogToBookDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "BlogToBook.h"
#endif

#include "BlogToBookDoc.h"
#include "BlogToBookView.h"

#include <propkey.h>

#include "wininet.h"
#include "MainFrm.h"
#include "AttribDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CBlogToBookDoc

IMPLEMENT_DYNCREATE(CBlogToBookDoc, CDocument)

BEGIN_MESSAGE_MAP(CBlogToBookDoc, CDocument)
	ON_COMMAND(ID_BUTTON_FETCH, &CBlogToBookDoc::OnButtonFetch)
	ON_COMMAND(ID_BUTTON_SAVEEPUB, &CBlogToBookDoc::OnButtonSaveepub)
	ON_COMMAND(ID_EDIT_FETCHFROM, &CBlogToBookDoc::OnEditfetchfrom)
	ON_COMMAND(ID_EDIT_FETCHTO, &CBlogToBookDoc::OnEditfetchto)
	ON_COMMAND(ID_EDIT_ADDR, &CBlogToBookDoc::OnEditAddr)
	ON_COMMAND(ID_EDIT_BNAME, &CBlogToBookDoc::OnEditbname)
	ON_COMMAND(ID_EDIT_AUTH, &CBlogToBookDoc::OnEditAuth)
	ON_COMMAND(ID_BUTTON_DEMO, &CBlogToBookDoc::OnButtonDemo)
	ON_COMMAND(ID_BUTTON_SHOWFOLDER, &CBlogToBookDoc::OnButtonShowfolder)
	ON_COMMAND(ID_BUTTON_SHOWCASE, &CBlogToBookDoc::OnButtonShowcase)
	ON_COMMAND(ID_BUTTON_VALID, &CBlogToBookDoc::OnButtonValid)
	ON_COMMAND(ID_EDIT_BOOK_DESC, &CBlogToBookDoc::OnEditBookDesc)
	ON_COMMAND(ID_EDIT_BOOK_PUB, &CBlogToBookDoc::OnEditBookPub)
	ON_COMMAND(ID_EDIT_BOOK_PUBDATE, &CBlogToBookDoc::OnEditBookPubdate)
	ON_COMMAND(ID_EDIT_BOOK_LANG, &CBlogToBookDoc::OnEditBookLang)
	ON_COMMAND(ID_EDIT_BOOK_ISBN, &CBlogToBookDoc::OnEditBookIsbn)
	ON_COMMAND(ID_BUTTON_COVER, &CBlogToBookDoc::OnButtonCover)
	ON_COMMAND(ID_BUTTON_PREFACE, &CBlogToBookDoc::OnButtonPreface)
	ON_COMMAND(ID_BUTTON_LIC, &CBlogToBookDoc::OnButtonLic)
	ON_COMMAND(ID_BUTTON_ENDPAGE, &CBlogToBookDoc::OnButtonEndpage)
	ON_COMMAND(ID_BUTTON_DED, &CBlogToBookDoc::OnButtonDed)
	ON_COMMAND(ID_BUTTON_EDLIST, &CBlogToBookDoc::OnButtonEdlist)
	ON_COMMAND(ID_BUTTON_CALIBRE, &CBlogToBookDoc::OnButtonCalibre)
	ON_COMMAND(ID_BUTTON_HELP, &CBlogToBookDoc::OnButtonHelp)
	ON_COMMAND(ID_BUTTON_UPDATE, &CBlogToBookDoc::OnButtonUpdate)
END_MESSAGE_MAP()


// CBlogToBookDoc construction/destruction

CBlogToBookDoc::CBlogToBookDoc()
{
	// TODO: add one-time construction code here
	m_IsInstalled = m_IsProjectLoaded = m_bListChanged = FALSE;
	m_IsCancelled = m_ListEdited = m_IsFetched = FALSE;
	m_TitleCount = 0;
	m_ChapterCount = 0;

	//m_UserPath = _T("");// GetUserDocumentPath(B2B_FOLDER) + _T("\\");
	//m_BlankPath = _T("");// m_UserPath + _T("Blank");
	//m_ProjectName = _T("");// _T("Project1");
	//m_ProjectPath = _T("");// m_UserPath + m_ProjectName;
	//m_RawDataPath = _T("");// m_ProjectPath + _T("\\Raw\\");

	m_B2BVersion = 1;
	m_B2BRef = _T("<br /><br />This EBook was created using the Free and Open Sourced windows application: <a href=\"http://github.com/oormicreations/BlogToBook\">Blog to Book</a> by <a href=\"http://oormi.in\">Oormi Creations</a>.");
	m_B2BRefPre = _T("\r\n\r\nThis EBook was created using the Free and Open Sourced windows application: Blog to Book by Oormi Creations.");

	for (int i = 0; i < MAXARTICLES; i++)
	{
		m_ChapterList[i] = 0;
		m_ChapterNumDisp[i] = 0;
	}

	LoadSettings();
	if (!m_IsInstalled)
	{
		m_IsInstalled = TRUE;
		SaveSettings();
		ReportUsage();
	}

	ReportUsage();//test

}

CBlogToBookDoc::~CBlogToBookDoc()
{
}

BOOL CBlogToBookDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	CNewProjectDlg projectDlg;

	if (IDOK == projectDlg.DoModal())
	{
		m_ProjectName = projectDlg.m_Name;
		m_ProjectPath = projectDlg.m_Folder + _T("\\") + m_ProjectName + _T("\\");

		if (projectDlg.m_Open)
		{
			CMainFrame * fwnd = (CMainFrame *)AfxGetMainWnd();
			fwnd->PostMessage(WM_COMMAND, ID_FILE_OPEN, 0);
			return TRUE;
		}

		Clear();

		SetModifiedFlag(TRUE);

		ShowCaption(_T("New project created! Fetch your blog contents."));
		SetTitle(m_ProjectName);
	
		m_IsProjectLoaded = TRUE;
		m_IsCancelled = FALSE;
	}
	else
	{
		if(!m_IsProjectLoaded) ShowCaption(_T("Create a New project and Fetch your blog contents. Or Open an existing project."));
		SetTitle(m_ProjectName);
		m_IsCancelled = TRUE;
	}

	return TRUE;
}

void CBlogToBookDoc::Clear()
{

	m_Status = _T("");
	m_Index = _T("");
	m_BlogPageRaw = _T("");
	m_BlogPagePreview = _T("");
	m_Blog.Clear();
	m_IsFetched = FALSE;
	m_bListChanged = FALSE;

	for (int i = 0; i < MAXARTICLES; i++)
	{
		m_TitleDates[i] = _T("");
		m_Titles[i] = _T("");
		m_TitleUrls[i] = _T("");
		m_ChapterList[i] = 0;
		m_ChapterNumDisp[i] = 0;
	}

	CFrameWnd * fwnd = (CFrameWnd *)AfxGetMainWnd();
	CBlogToBookView * view = (CBlogToBookView*)fwnd->GetActiveView();
	if (view != NULL) view->m_ArList.DeleteAllItems();


}


// CBlogToBookDoc serialization

void CBlogToBookDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
		ar  << m_B2BVersion << m_ProjectName << m_ProjectPath 
			<< m_Blog.m_YearStart << m_Blog.m_YearEnd << m_Blog.m_MonthStart << m_Blog.m_MonthEnd
			<< m_Blog.m_BlogAuthor << m_Blog.m_BlogDesc << m_Blog.m_BlogISBN << m_Blog.m_BlogLang << m_Blog.m_BlogModDate 
			<< m_Blog.m_BlogName << m_Blog.m_BlogPlublisher << m_Blog.m_BlogPubDate << m_Blog.m_BlogUrl
			<< m_Blog.m_IncludeB2BRef << m_BFontSz << m_TFontSz << m_BFont << m_TFont;
	}
	else
	{
		// TODO: add loading code here
		ar  >> m_B2BVersion >> m_ProjectName >> m_ProjectPath 
			>> m_Blog.m_YearStart >> m_Blog.m_YearEnd >> m_Blog.m_MonthStart >> m_Blog.m_MonthEnd
			>> m_Blog.m_BlogAuthor >> m_Blog.m_BlogDesc >> m_Blog.m_BlogISBN >> m_Blog.m_BlogLang >> m_Blog.m_BlogModDate
			>> m_Blog.m_BlogName >> m_Blog.m_BlogPlublisher >> m_Blog.m_BlogPubDate >> m_Blog.m_BlogUrl
			>> m_Blog.m_IncludeB2BRef >> m_BFontSz >> m_TFontSz >> m_BFont >> m_TFont;
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CBlogToBookDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CBlogToBookDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CBlogToBookDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CBlogToBookDoc diagnostics

#ifdef _DEBUG
void CBlogToBookDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CBlogToBookDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CBlogToBookDoc commands


BOOL CBlogToBookDoc::Fetch(CString url)
{
	DWORD flags;
	BOOL connected = InternetGetConnectedState(&flags, NULL);

	if (!connected)
	{
		return FALSE;
	}

	HINTERNET hInet = InternetOpen(_T("Blog to Book App"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
	HINTERNET hUrl = InternetOpenUrl(hInet, url, NULL, -1L,
		INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE |
		INTERNET_FLAG_NO_CACHE_WRITE | WININET_API_FLAG_ASYNC, NULL);

	if (hUrl)
	{
		CHAR szBuffer[2048];
		DWORD dwRead;
		while (InternetReadFile(hUrl, szBuffer, sizeof(szBuffer)-1, &dwRead) == TRUE)
		{
			if (dwRead > 0)
			{
				szBuffer[dwRead] = 0;
				CString str(szBuffer);
				m_BlogPageRaw += str;
			}
			else break;
		}
	}

	InternetCloseHandle(hUrl);
	InternetCloseHandle(hInet);

	if (!m_BlogPageRaw.IsEmpty()) return TRUE;
	return FALSE;
}

BOOL CBlogToBookDoc::ParseTitles()
{
	CString tstart, tend, str;

	int len = m_BlogPageRaw.GetLength();
	int i = 0;
	BOOL hasArticles = FALSE;

	while (TRUE)
	{
		if (i >= MAXARTICLES) break;

		tstart = _T("post-title entry-title");
		tend = _T("a>");
		int n1 = m_BlogPageRaw.Find(tstart);
		if (n1 < 0)
		{
			break;
		}

		str = m_BlogPageRaw.Right(len - n1 - tstart.GetLength());
		m_BlogPageRaw = str;
		len = m_BlogPageRaw.GetLength();

		int n2 = str.Find(tend);
		str = str.Left(n2);

		tstart = _T("http");
		tend = _T("\'");

		n1 = str.Find(tstart);
		str = str.Right(str.GetLength() - n1);
		n2 = str.Find(tend);
		m_TitleUrls[i] = str.Left(n2);

		hasArticles = !m_TitleUrls[i].IsEmpty();

		tstart = _T(">");
		tend = _T("<");

		n1 = str.Find(tstart);
		str = str.Right(str.GetLength() - n1 - 1);
		n2 = str.Find(tend);
		m_Titles[i] = str.Left(n2);

		//attempt date
		tstart = _T("<a class=\'timestamp-link"); 
		tend = _T("</a");

		n1 = m_BlogPageRaw.Find(tstart);
		int n3 = n1;
		if (n1 >= 0)
		{
			n1 = m_BlogPageRaw.Find(_T("<abbr"), n1);
			if(n1<0)n1 = m_BlogPageRaw.Find(_T("<time class=\'published"), n3);

			if (n1 >= 0)
			{
				n1 = m_BlogPageRaw.Find(_T("title="), n1);
				if (n1 >= 0)
				{
					n2 = m_BlogPageRaw.Find(_T(">"), n1);
					str = m_BlogPageRaw.Mid(n1, n2 - n1);
					str.Replace(_T("\""), _T(""));
					str.Replace(_T("\'"), _T(""));
					str.Replace(_T("title="), _T(""));
					m_TitleDates[i] = str;

				}
			}

		}

		i++;
	}

	return hasArticles;
}

CString CBlogToBookDoc::LoadPage(CString fpath)
{
	CFile file;
	BOOL res = file.Open(fpath, CFile::modeRead);
	if (!res)
	{
		AfxMessageBox(fpath + _T("\r\n\r\nError: This file could not be loaded."));
		return _T("");
	}

	UINT len = (UINT)file.GetLength();
	CHAR *buf = new CHAR [len+1];
	ZeroMemory(buf, len + 1);

	file.Read(buf, len);
	file.Close();

	CString str(buf);
	delete[] buf;
	m_BlogPageRaw = str;


	return str;
}

BOOL CBlogToBookDoc::BuildBook()
{
	//LoadPage(_T("G:\\VS2015 Projects\\BlogToBook\\BlogToBook\\memories01.txt"));

	CString bstart = _T("<div class=\'post-body entry-content");
	CString bend = _T("</div>");
	int len = m_BlogPageRaw.GetLength();

	int n1 = m_BlogPageRaw.Find(bstart);
	if (n1>=0)
	{
		m_BlogPageRaw =  m_BlogPageRaw.Right(len - n1);
		int n2 = m_BlogPageRaw.Find(bend);
		m_BlogPageRaw = m_BlogPageRaw.Left(n2);

	}

	//remove tables
	bstart = _T("<table");
	bend = _T("</table>");
	//len = m_BlogPageRaw.GetLength();
	int offset = 0;

	while (TRUE)
	{
		n1 = m_BlogPageRaw.Find(bstart, offset);
		if (n1 >= 0)
		{
			int n2 = m_BlogPageRaw.Find(bend);
			CString s1, s2;
			s1 = m_BlogPageRaw.Mid(n1, n2 - n1 + bend.GetLength());
			m_BlogPageRaw.Replace(s1, _T(""));
			offset = n1 + 1;
		}
		else break;
	}

	//remove html except p, br, b, u and i
	bstart = _T("<");
	bend = _T(">");
	//len = m_BlogPageRaw.GetLength();
	offset = 0;
	m_BlogPageRaw.Replace(_T("\n"), _T(""));
	m_BlogPageRaw.Replace(_T("\r"), _T(""));
	m_BlogPageRaw.Replace(_T("&nbsp"), _T(""));

	m_BlogPagePreview = m_BlogPageRaw;

	CString s1;

	while (TRUE)
	{
		n1 = m_BlogPageRaw.Find(bstart, offset);
		if (n1 >= 0)
		{
			int n2 = m_BlogPageRaw.Find(bend, offset+1);
			s1 = m_BlogPageRaw.Mid(n1, n2 - n1 + 1);

			if ((s1 != _T("<br />")) && (s1 != _T("<b>")) && (s1 != _T("</b>")) 
				&& (s1 != _T("<p>")) && (s1 != _T("</p>")) && (s1 != _T("<i>")) 
				&& (s1 != _T("</i>")) && (s1 != _T("<u>")) && (s1 != _T("</u>")))
			{
				m_BlogPageRaw.Replace(s1, _T(""));
				m_BlogPagePreview.Replace(s1, _T(""));
				offset = n1;
			}
			else
			{
				offset = n2;
				if (s1 == _T("<br />"))
				{
					m_BlogPagePreview.Replace(s1, _T("\r\n")); //replace in preview version of the page
				}
				else
				{
					m_BlogPagePreview.Replace(s1, _T("")); //replace in preview version of the page
				}
			}
		}
		else break;
	}
	//s1.Format(, m_Titles[0]);
	m_BlogPageRaw = _T("<p class=\"c4\" >Chapter b2bchapternum</p><br /><p class=\"chap\" >b2bchaptername</p><br /><br />") + m_BlogPageRaw;
	m_BlogPageRaw.Replace(_T("<br /><br /><br /><br />"), _T("<br />"));
	m_BlogPageRaw.Replace(_T("<br /><br /><br />"), _T("<br />"));
	m_BlogPageRaw.Replace(_T("<br />"), _T("<br />\r\n"));


	return 0;
}

CString CBlogToBookDoc::GetPreview(int arNum)
{
	if (m_Blog.m_Articles[arNum].m_Included)
	{
		return LoadPage(m_RawDataPath + m_Blog.m_Articles[arNum].m_PreviewFile);
	}
	else return _T("");
}

CString CBlogToBookDoc::GetArTitle(int arNum)
{
	return m_Blog.m_Articles[arNum].m_Title;
}


BOOL CBlogToBookDoc::SavePages()
{
	int pSaved = 0;
	CString fname, prename;

	for (int i = 0; i < MAXARTICLES; i++)
	{
		if (!m_TitleUrls[i].IsEmpty())
		{

			if (Fetch(m_TitleUrls[i]))
			{
				BuildBook();

				fname.Format(_T("raw%03d.txt"), m_TitleCount);
				prename.Format(_T("pre%03d.txt"), m_TitleCount);
				if (SaveFile(fname, m_RawDataPath, m_BlogPageRaw) && SaveFile(prename, m_RawDataPath, m_BlogPagePreview))
				{
					m_Index += _T("Enabled|") + m_TitleDates[i] + _T("|") + m_Titles[i] + _T("|") + m_TitleUrls[i] + +_T("|") + fname + _T("|") + prename + _T("\r\n");
					pSaved++;
					m_Blog.SetArticle(m_TitleCount, m_Titles[m_TitleCount], m_TitleDates[m_TitleCount], m_TitleUrls[m_TitleCount], TRUE, fname, prename);
				}
			}
			else
			{
				m_Index += _T("Disabled|") + m_TitleDates[i] + _T("|") + m_Titles[i] + _T("|") + m_TitleUrls[i] + _T("|--|--") + _T("\r\n");
				m_Blog.SetArticle(m_TitleCount, m_Titles[m_TitleCount], m_TitleDates[m_TitleCount], m_TitleUrls[m_TitleCount], FALSE, _T(""), _T(""));
			}

			m_TitleCount++;
			ShowCaption(_T("Fetching... ") + m_Titles[i]);
		}
	}

	return TRUE;
}

BOOL CBlogToBookDoc::SaveFile(CString fname, CString path, CString data)
{

	CFile pageFile;
	BOOL res = pageFile.Open(path + fname, CFile::modeCreate | CFile::modeWrite);
	if (!res)
	{
		AfxMessageBox(path + fname + _T("\r\n\r\nError: This file could not be saved."));
		return FALSE;
	}

	CT2CA outputString(data, CP_UTF8);
	pageFile.Write(outputString, ::strlen(outputString));
	pageFile.Close();

	return TRUE;
}


void CBlogToBookDoc::OnButtonFetch()
{
	if ((m_IsProjectLoaded) && (!m_Index.IsEmpty()))
	{
		OnNewDocument();
		return;
	}

	if (m_ProjectPath.IsEmpty())
	{
		OnNewDocument();
		return;
	}

	if (!m_Blog.GetDateRange())
	{
		AfxMessageBox(_T("There is some problem with start and end dates. Ensure that the format is YYYYMM, year is after 1998 and end date is not before start date."));
		return;
	}

	if (!m_Blog.GetBlogInfo())
	{
		m_Blog.m_BlogUrl = _T("");
		AfxMessageBox(_T("There is some problem with blog name, author or address. Ensure that they are correct and not blank."));
		return;
	}

	//set paths
	SetProjectPaths();

	//copy template
	SHCopy(m_BlankPath + _T("*"), m_ProjectPath);

	//create extra pages
	m_Blog.m_CoverPath = m_RawDataPath + _T("book_cover.jpg");
	m_Blog.m_CopyrightTitle = m_Blog.m_BlogName;

	CString sp[] = { m_Blog.m_CoverPath, m_Blog.m_Copyright, m_Blog.m_Dedication, m_Blog.m_Preface, m_Blog.m_EndPage };
	CString sn[] = { _T("Cover"), _T("Copyright"), _T("Dedication"), _T("Preface"), _T("End Page") };
	CString st[] = { m_Blog.m_CoverTitle, m_Blog.m_CopyrightTitle, m_Blog.m_DedicationTitle, m_Blog.m_PrefaceTitle, m_Blog.m_EndPageTitle };
	CString str;

	m_Index = _T("");
	for (int i = 0; i < MAXEXTRAPAGES; i++)
	{
		str.Format(_T("pre%03d.txt"), i);
		if(!SaveFile(str, m_RawDataPath, sp[i]))return;

		str.Format(_T("raw%03d.txt"), i);
		sp[i].Replace(_T("\r\n"), _T("<br />\r\n"));
		if (!SaveFile(str, m_RawDataPath, _T("<p class=\"chap\" >b2bchaptername</p><br />") + sp[i]))return;

		str.Format(_T("|raw%03d.txt|pre%03d.txt\r\n"), i, i);
		m_Index = m_Index + _T("Enabled|None|") + st[i] + _T("|") + sn[i] + str;
	}

	//get blog data
	ShowCaption(_T("Connecting..."));

	int year = m_Blog.m_YearStart;
	int month = m_Blog.m_MonthStart;

	m_TitleCount = MAXEXTRAPAGES;

	for (int i = 0; i < MAXMONTHS; i++)
	{
		str.Format(_T("%s/%d/%02d"), m_Blog.m_BlogUrl, year, month);

		for (int i = 0; i < MAXARTICLES; i++)
		{
			m_TitleDates[i] = _T("");
			m_Titles[i] = _T("");
			m_TitleUrls[i] = _T("");
		}

		if (Fetch(str))
		{
			if (ParseTitles()) SavePages();

			if (month < 12) month++;
			else
			{
				month = 1;
				year++;
			}
			if (year > m_Blog.m_YearEnd) break;
			if (year == m_Blog.m_YearEnd)
			{
				if (month > m_Blog.m_MonthEnd) break;
			}

			//TRACE(_T("-----------------------y=%d,m=%d\r\n"), year, month);
			str.Format(_T("Getting articles for ... Year:%d Month:%02d. Found: %d"), year, month, m_TitleCount - MAXEXTRAPAGES + 1);
			ShowCaption(str);
		}
		else
		{
			str.Format(_T("Error fetching articles for ... Year:%d Month:%02d. Check your internet connection or address."), year, month);
			ShowCaption(str);
		}

	}


	if (m_TitleCount<MAXEXTRAPAGES+1)
	{
		ShowCaption(_T("Failed! Check your internet connection or address."));
	}
	else
	{
		//create end page
		int i = m_TitleCount ++;
		str.Format(_T("pre%03d.txt"), i);
		if(!SaveFile(str, m_RawDataPath, sp[MAXEXTRAPAGES]))return;

		sp[MAXEXTRAPAGES].Replace(_T("\r\n"), _T("<br />\r\n"));
		str.Format(_T("raw%03d.txt"), i);
		if (!SaveFile(str, m_RawDataPath, _T("<p class=\"chap\" >b2bchaptername</p><br />") + sp[MAXEXTRAPAGES]))return;

		str.Format(_T("|raw%03d.txt|pre%03d.txt\r\n"), i, i);
		m_Index = m_Index + _T("Enabled|None|") + st[MAXEXTRAPAGES] + _T("|") + sn[MAXEXTRAPAGES] + str;
	
		//save index
		if (!SaveFile(_T("index.txt"), m_RawDataPath, m_Index))return;

		SetAllArticles();
		SetModifiedFlag(TRUE);
		m_Blog.SetBookInfo();

		m_IsFetched = TRUE;
		m_bListChanged = TRUE;
		m_Blog.m_ArticleCount = m_TitleCount;

		ShowCaption(_T("Done! Go ahead and Render to see how it looks. Or include/exclude articles from the list."));
	}

	UpdateAllViews(NULL);

}


void CBlogToBookDoc::OnButtonSaveepub()
{
	CFrameWnd * fwnd = (CFrameWnd *)AfxGetMainWnd();
	CBlogToBookView * view = (CBlogToBookView*)fwnd->GetActiveView();
	if (view == NULL)return;
	view->UpdateChapterList();

	if (m_ChapterCount < 1) 
	{
		ShowCaption(_T("There is no content yet!"));
		return;
	}

	m_Blog.GetBlogInfo();
	m_Blog.GetBookInfo();

	ShowCaption(_T("Saving as EPUB: ") + m_Blog.m_BlogName);

	//save cover
	if (m_Blog.m_Articles[0].m_Included)
	{
		SHCopy(m_RawDataPath + _T("book_cover.jpg"), m_ProjectPath + _T("Book\\OEBPS\\images\\book_cover.jpg"));
	}
	else 
	{
		SHCopy(m_RawDataPath + _T("book_cover_none.jpg"), m_ProjectPath + _T("Book\\OEBPS\\images\\book_cover.jpg"));
	}
	
	//save pages
	CString templateName, contentTemplate, cnum;
	templateName = m_BlankPath +  _T("Book\\OEBPS\\p001.xhtml");

	contentTemplate = LoadPage(templateName);
	if (contentTemplate.IsEmpty())return;

	//clean last
	SHDelete(m_ProjectPath + _T("Book\\OEBPS\\p*.xhtml"));
	SHDelete(m_ProjectPath + _T("Book\\OEBPS\\package.opf"));
	SHDelete(m_ProjectPath + _T("Book\\OEBPS\\toc.xhtml"));
	SHDelete(m_ProjectPath + m_Blog.m_BlogName + _T(".epub"));

	//save pages
	for (UINT i = 0; i < m_ChapterCount; i++)
	{
		if (m_ChapterList[i] == 0) continue; //skip cover

		CString xhtmlName, rawName, contentChapter;
		xhtmlName.Format(_T("Book\\OEBPS\\p%03d.xhtml"), i);
		rawName.Format(_T("raw%03d.txt"), m_ChapterList[i]);

		m_BlogPageRaw = LoadPage(m_RawDataPath + rawName);
		if (m_BlogPageRaw.IsEmpty())return;

		contentChapter = contentTemplate;
		contentChapter.Replace(_T("b2bpagetitle"), m_Blog.m_Articles[m_ChapterList[i]].m_Title);
		contentChapter.Replace(_T("b2bcreator1"), m_Blog.m_BlogAuthor);
		contentChapter.Replace(_T("b2bYYYYMMDD"), CTime::GetCurrentTime().Format("%Y%m%d"));

		contentChapter.Replace(_T("b2bcontent"), m_BlogPageRaw);
		cnum.Format(_T("%d"), m_ChapterNumDisp[i]);
		contentChapter.Replace(_T("b2bchapternum"), cnum);
		contentChapter.Replace(_T("b2bchaptername"), m_Blog.m_Articles[m_ChapterList[i]].m_Title);

		//b2b Ref
		if (m_ChapterList[i] == m_TitleCount - 1)
		{
			CMainFrame * fwnd = (CMainFrame *)AfxGetMainWnd();
			if (fwnd->m_Ref) contentChapter.Replace(_T("</body>"), m_B2BRef + _T("\r\n</body>"));
		}

		if (!SaveFile(xhtmlName, m_ProjectPath, contentChapter))return;
	}

	//save package.opf
	templateName.Replace(_T("p001.xhtml"), _T("package.opf"));
	contentTemplate = LoadPage(templateName);
	if (contentTemplate.IsEmpty())return;

	contentTemplate.Replace(_T("b2btitle"), m_Blog.m_BlogName);
	contentTemplate.Replace(_T("b2bdescription"), m_Blog.m_BlogDesc);
	contentTemplate.Replace(_T("b2bcreator1"), m_Blog.m_BlogAuthor);
	contentTemplate.Replace(_T("b2bcreator2"), m_Blog.GetLastName());
	contentTemplate.Replace(_T("b2bcreator3"), m_Blog.GetFirstName());
	contentTemplate.Replace(_T("b2bpublisher"), m_Blog.m_BlogPlublisher);
	contentTemplate.Replace(_T("b2bdateyyyy-mm-dd"), CTime::GetCurrentTime().Format("%Y-%m-%d"));
	contentTemplate.Replace(_T("b2blanguageen"), m_Blog.m_BlogLang);
	contentTemplate.Replace(_T("b2bmodified-yyyy-mm-ddTHH:MM:SSZ"), CTime::GetCurrentTime().Format("%Y-%m-%dT%H:%M:%SZ"));
	contentTemplate.Replace(_T("b2bisbn"), m_Blog.m_BlogISBN);
	contentTemplate.Replace(_T("b2blic"), m_Blog.m_Copyright);

	for (UINT i = 1/*skip cover*/; i < m_ChapterCount; i++)
	{
		cnum.Format(_T("<item id=\"p%03d\" media-type=\"application/xhtml+xml\" href=\"p%03d.xhtml\" />\r\n<!--b2baddpages-->"), i, i);
		contentTemplate.Replace(_T("<!--b2baddpages-->"), cnum);
		cnum.Format(_T("<itemref idref=\"p%03d\" linear=\"yes\"/>\r\n<!--b2badditemref-->"), i);
		contentTemplate.Replace(_T("<!--b2badditemref-->"), cnum);
	}

	if(!SaveFile(_T("Book\\OEBPS\\package.opf"), m_ProjectPath, contentTemplate))return;

	//save toc
	templateName.Replace(_T("package.opf"), _T("toc.xhtml"));
	contentTemplate = LoadPage(templateName);
	if (contentTemplate.IsEmpty())return;

	contentTemplate.Replace(_T("b2btitle"), m_Blog.m_BlogName);
	contentTemplate.Replace(_T("b2bdescription"), m_Blog.m_BlogDesc);
	contentTemplate.Replace(_T("b2bcreator1"), m_Blog.m_BlogAuthor);
	contentTemplate.Replace(_T("b2bYYYYMMDD"), CTime::GetCurrentTime().Format("%Y%m%d"));

	for (UINT i = 0; i < m_ChapterCount; i++)
	{
		if (m_ChapterList[i] == 0) continue; //skip cover
		if (m_ChapterList[i] == 1) continue; //skip copyrt
		if (m_ChapterList[i] == 2) continue; //skip dedic
		if (m_ChapterList[i] == m_ChapterCount - 1) continue; //skip end page

		cnum.Format(_T("<li><a href=\"p%03d.xhtml\">%s</a></li>\r\n<!--b2btoc-->"), i , m_Blog.m_Articles[m_ChapterList[i]].m_Title);
		contentTemplate.Replace(_T("<!--b2btoc-->"), cnum);
	}

	if(!SaveFile(_T("Book\\OEBPS\\toc.xhtml"), m_ProjectPath, contentTemplate))return;

	//save css
	templateName = m_BlankPath + _T("Book\\OEBPS\\css\\style.css");
	contentTemplate = LoadPage(templateName);
	if (contentTemplate.IsEmpty())return;

	cnum.Format(_T("%d"), m_BFontSz);
	contentTemplate.Replace(_T("b2bfontbodysz"), cnum);
	contentTemplate.Replace(_T("b2bfontbody"), m_BFont);

	cnum.Format(_T("%d"), m_TFontSz);
	contentTemplate.Replace(_T("b2bfonttitlesz"), cnum);
	contentTemplate.Replace(_T("b2bfonttitle"), m_TFont);

	if (!SaveFile(_T("Book\\OEBPS\\css\\style.css"), m_ProjectPath, contentTemplate))return;


	//zip
	SHCopy(m_BlankPath + _T("epub.zip"), m_ProjectPath + _T("epub.zip"));

	if (ZipIt(m_ProjectPath + _T("Book\\OEBPS"), m_ProjectPath + _T("epub.zip")) != S_OK)
	{
		AfxMessageBox(_T("Zip save error!"));
		return;
	}

	Sleep(3000);

	if (ZipIt(m_ProjectPath + _T("Book\\META-INF"), m_ProjectPath + _T("epub.zip")) != S_OK)
	{
		AfxMessageBox(_T("Zip save error!"));
		return;
	}


	//wait for zip
	int t = 0;
	while (t < 30)
	{
		Sleep(1000);
		if (!IsFileLocked(m_ProjectPath + _T("epub.zip")) )
		{
			//rename
			CFile::Rename(m_ProjectPath + _T("epub.zip"), m_ProjectPath + m_Blog.m_BlogName + _T(".epub"));
			break;
		}
		else
		{
			t++;
			TRACE(_T(">>>>>Locked. Try=%d\r\n"), t);
		}
	}

	//Done!
	ShowCaption(_T("Your Ebook was saved! Click Show Project Folder button to view the contents."));
}

BOOL CBlogToBookDoc::IsFileLocked(CString path)
{
	CFile file;
	BOOL res = file.Open(path, CFile::modeRead);
	if (res) file.Close();
	return !res;
}

HRESULT CBlogToBookDoc::ZipIt(CString strSrc, CString strDest)
{
	////create empty zip
	//BYTE startBuffer[] = { 80, 75, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	//FILE *f = _wfopen(strDest.GetBuffer(strDest.GetLength()), _T("wb"));
	//strDest.ReleaseBuffer();
	//fwrite(startBuffer, sizeof(startBuffer), 1, f);
	//fclose(f);

	LPCTSTR zipFileName = strDest;
	LPCTSTR outFolderName = strSrc;

	// the shell object
	CComPtr<IShellDispatch> shell;
	HRESULT hr = shell.CoCreateInstance(CLSID_Shell);
	if (FAILED(hr)) {
		return hr;
	}

	// the zip file
	CComPtr<Folder> zipFile;
	hr = shell->NameSpace(CComVariant(zipFileName), &zipFile);
	if (FAILED(hr)) {
		return hr;
	}

	// destination folder
	CComPtr<Folder> destination;
	hr = shell->NameSpace(CComVariant(outFolderName), &destination);
	if (FAILED(hr)) {
		return hr;
	}


	// copy operation
	hr = zipFile->CopyHere(CComVariant(destination),
		CComVariant(2048 | 1024 | 512 | 16 | 4, VT_I4));

	return hr;
	
}

bool CBlogToBookDoc::SHCopy(LPCTSTR from, LPCTSTR to)
{
	SHFILEOPSTRUCT fileOp = { 0 };
	fileOp.wFunc = FO_COPY;
	TCHAR newFrom[MAX_PATH];
	_tcscpy_s(newFrom, from);
	newFrom[_tcsclen(from) + 1] = NULL;
	fileOp.pFrom = newFrom;
	TCHAR newTo[MAX_PATH];
	_tcscpy_s(newTo, to);
	newTo[_tcsclen(to) + 1] = NULL;
	fileOp.pTo = newTo;
	fileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;

	int result = SHFileOperation(&fileOp);

	return result == 0;
}

bool CBlogToBookDoc::SHDelete(LPCTSTR from)
{
	SHFILEOPSTRUCT fileOp = { 0 };
	fileOp.wFunc = FO_DELETE;
	TCHAR newFrom[MAX_PATH];
	_tcscpy_s(newFrom, from);
	newFrom[_tcsclen(from) + 1] = NULL;
	fileOp.pFrom = newFrom;
	fileOp.fFlags = FOF_FILESONLY | FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NORECURSION;

	int result = SHFileOperation(&fileOp);

	return result == 0;
}

void CBlogToBookDoc::OnEditfetchfrom()
{
	// TODO: Add your command handler code here
}


void CBlogToBookDoc::OnEditfetchto()
{
	// TODO: Add your command handler code here
}


void CBlogToBookDoc::OnEditAddr()
{
	// TODO: Add your command handler code here
}


void CBlogToBookDoc::OnEditbname()
{
	// TODO: Add your command handler code here
}


void CBlogToBookDoc::OnEditAuth()
{
	// TODO: Add your command handler code here
}


void CBlogToBookDoc::OnButtonDemo()
{

	if(m_ProjectName.IsEmpty()) OnNewDocument();
	else
	{
		if ((m_IsProjectLoaded) && (!m_Index.IsEmpty())) OnNewDocument();
	}

	if (m_IsCancelled)
	{
		m_IsCancelled = FALSE;
		return;
	}

	m_Blog.SetDemoData();
	ShowCaption(_T("Click the Fetch button to download blog content."));
}


void CBlogToBookDoc::ShowCaption(CString str)
{
	CMainFrame * fwnd = (CMainFrame*)AfxGetMainWnd();
	fwnd->SetCaption(str);
}

void CBlogToBookDoc::SetArticleList()
{
	CFrameWnd * fwnd = (CFrameWnd *)AfxGetMainWnd();
	CBlogToBookView * view = (CBlogToBookView*)fwnd->GetActiveView();
	//view->PopulateList();
	view->OnButtonList();
}

CArticle CBlogToBookDoc::GetArticle(int arNum)
{
	return m_Blog.m_Articles[arNum];
}

int CBlogToBookDoc::GetArticleCount()
{
	return m_Blog.m_ArticleCount;
}

void CBlogToBookDoc::SetAllArticles()
{
	m_Index = LoadPage(m_RawDataPath + _T("index.txt"));
	if (m_Index.IsEmpty())return;

	CString line, token, data[6];
	int start1 = 0;
	int row = 0;

	while (start1 >= 0)
	{
		line = m_Index.Tokenize(_T("\r\n"), start1);
		if (!line.IsEmpty())
		{
			int col = 0;
			int start2 = 0;

			while (start2 >= 0)
			{
				token = line.Tokenize(_T("|"), start2);
				data[col] = token;
				col++;
				if (col > 5)break;
			}
			m_Blog.SetArticle(row, data);
		}

		row++;
		if (row > MAXARTICLES)break;
	}

}

BOOL CBlogToBookDoc::SaveIndex()
{
	CFrameWnd * fwnd = (CFrameWnd *)AfxGetMainWnd();
	CBlogToBookView * view = (CBlogToBookView*)fwnd->GetActiveView();
	view->UpdateChapterList();

	CString str;
	CString data[6];
	for (UINT i = 0; i < m_TitleCount; i++)
	{
		m_Blog.GetArticle(i, data);
		for (int j = 0; j < 5; j++)
		{
			str += data[j] + _T("|");
		}
		str += data[5] + _T("\r\n");

	}

	return SaveFile(_T("index.txt"), m_RawDataPath, str);
}


void CBlogToBookDoc::OnButtonShowfolder()
{
	ShellExecute(NULL, _T("open"), m_ProjectPath, NULL, NULL, SW_SHOW);
}


void CBlogToBookDoc::OnButtonShowcase()
{
	ShellExecute(NULL, _T("open"), _T("https://b2b.oormi.in"), NULL, NULL, SW_SHOWNORMAL);

}


void CBlogToBookDoc::OnButtonValid()
{
	ShellExecute(NULL, _T("open"), _T("http://validator.idpf.org/"), NULL, NULL, SW_SHOWNORMAL);
}


void CBlogToBookDoc::OnEditBookDesc()
{
	// TODO: Add your command handler code here
}


void CBlogToBookDoc::OnEditBookPub()
{
	// TODO: Add your command handler code here
}


void CBlogToBookDoc::OnEditBookPubdate()
{
	// TODO: Add your command handler code here
}


void CBlogToBookDoc::OnEditBookLang()
{
	// TODO: Add your command handler code here
}


void CBlogToBookDoc::OnEditBookIsbn()
{
	// TODO: Add your command handler code here
}


void CBlogToBookDoc::OnButtonCover()
{
	//opens output file select dialog

	CFileDialog DataFileOpenDialog(true, _T("b2b"), _T(""), OFN_HIDEREADONLY, _T("Picture Files (*.jpg)|*.jpg|(*.jpeg)|*.jpeg|(*.png)|*.png|All Files (*.*)|*.*||"));
	DataFileOpenDialog.m_ofn.lpstrTitle = _T("Select a cover image ...");
	DataFileOpenDialog.m_ofn.lpstrInitialDir = m_ProjectPath;
	INT_PTR res = DataFileOpenDialog.DoModal();
	if (res == IDCANCEL) return;
	CString str = DataFileOpenDialog.GetPathName();
	//CString ext = DataFileOpenDialog.GetFileExt();

	CImage image;
	HRESULT hr = image.Load(str);
	if (hr == S_OK)
	{
		int h = image.GetHeight();
		int w = image.GetWidth();

		if ((h < 1500) || (((double)w / (double)h) > 0.8))
		{
			int res = AfxMessageBox(_T("Ensure that your cover image is at least 1500 pixels tall and has an approx 0.76 width to height ratio.\r\nAre you sure you wish to use this image?"), MB_YESNO);
			if (res == IDNO)
			{
				image.Destroy();
				return;
			}
		}

		HRESULT hr = image.Save(m_RawDataPath + _T("book_cover.jpg"), Gdiplus::ImageFormatJPEG);
		image.Destroy();
		if (hr == S_OK)
		{
			ShowCaption(_T("Check Render for cover preview."));
			UpdateAllViews(NULL);
		}
		else
		{
			AfxMessageBox(_T("Error: Image file save error!"));
		}

	}
	else
	{
		AfxMessageBox(_T("Error: Ensure that your cover image is in JPG or PNG formats!"));
	}

}

void CBlogToBookDoc::OnButtonLic()
{
	EditAttributes(1);
}

void CBlogToBookDoc::OnButtonDed()
{
	EditAttributes(2);
}

void CBlogToBookDoc::OnButtonPreface()
{
	EditAttributes(3);
}

void CBlogToBookDoc::OnButtonEndpage()
{
	EditAttributes(m_Blog.m_ArticleCount - 1);
}

void CBlogToBookDoc::EditAttributes(int arNum)
{
	CAttribDlg dialog;
	CString sn[] = { _T("Cover"), _T("Copyright"), _T("Dedication"), _T("Preface"), _T("End Page") };

	dialog.m_AttribHeader = m_Blog.m_Articles[arNum].m_Title;
	dialog.m_AttContent = LoadPage(m_RawDataPath + m_Blog.m_Articles[arNum].m_PreviewFile);
	dialog.m_Include = m_Blog.m_Articles[arNum].m_Included;
	if (arNum < MAXEXTRAPAGES) dialog.m_Caption = sn[arNum];
	else dialog.m_Caption = sn[MAXEXTRAPAGES];

	if (IDCANCEL == dialog.DoModal()) return;

	m_Blog.m_Articles[arNum].m_Title = dialog.m_AttribHeader;
	m_Blog.m_Articles[arNum].m_Included = dialog.m_Include;

	CFrameWnd * fwnd = (CFrameWnd *)AfxGetMainWnd();
	CBlogToBookView * view = (CBlogToBookView*)fwnd->GetActiveView();
	if (view != NULL) view->m_ArList.SetCheck(arNum, m_Blog.m_Articles[arNum].m_Included);

	if (!SaveFile(m_Blog.m_Articles[arNum].m_PreviewFile, m_RawDataPath, dialog.m_AttContent))return;
	if (!SaveIndex())return;

	m_bListChanged = TRUE;
	UpdateAllViews(NULL);

}


void CBlogToBookDoc::OnButtonEdlist()
{
	CFrameWnd * fwnd = (CFrameWnd *)AfxGetMainWnd();
	CBlogToBookView * view = (CBlogToBookView*)fwnd->GetActiveView();
	if (view == NULL)return;
	if(view->m_ArList.GetItemCount() < 1) return;

	if (!SaveIndex())
	{
		AfxMessageBox(_T("Index file save error!"));
		return;
	}

	m_ListEdited = TRUE;
	ShowCaption(_T("Edit the text file and save it. Click Article List button to update view."));
	Sleep(2000);//wait for save before open again
	ShellExecute(NULL, _T("open"), m_RawDataPath + _T("index.txt"), NULL, NULL, SW_SHOWNORMAL);

}


void CBlogToBookDoc::OnButtonCalibre()
{
	ShellExecute(NULL, _T("open"), _T("https://calibre-ebook.com/"), NULL, NULL, SW_SHOWNORMAL);
}


BOOL CBlogToBookDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO:  Add your specialized creation code here
	Clear();

	CFile file;

	if (file.Open(lpszPathName, CFile::modeRead))
	{
		CArchive ar(&file, CArchive::load);
		Serialize(ar);
		ar.Close();
		file.Close();
		if (m_B2BVersion != 1)
		{
			ShowCaption(_T("Error loading Project: Version mismatch. Or the file is not a b2b file. Or it is corrupted."));
			return FALSE;
		}

		SetProjectPaths();
		SetAllArticles();
	}

	if (!m_Index.IsEmpty())
	{
		m_IsFetched = TRUE;
		m_bListChanged = TRUE;

		m_Blog.SetBlogInfo();
		m_Blog.SetBookInfo();
		SetRenderFonts();

		ShowCaption(_T("Project loaded: ") + m_ProjectName);
		SetTitle(m_ProjectName);

		CFrameWnd * fwnd = (CFrameWnd *)AfxGetMainWnd();
		CBlogToBookView * view = (CBlogToBookView*)fwnd->GetActiveView();
		view->m_Render = FALSE;

		m_IsProjectLoaded = TRUE;

		UpdateAllViews(NULL);
	}
	else ShowCaption(_T("Error loading Project: ") + m_ProjectName);

	//SetArticleList();
	return TRUE;
}


BOOL CBlogToBookDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	// TODO: Add your specialized code here and/or call the base class

	CFile file;
	if (file.Open(lpszPathName, CFile::modeCreate | CFile::modeWrite))
	{
		m_Blog.GetBlogInfo();
		m_Blog.GetBookInfo();
		m_Blog.GetDateRange();

		CArchive ar(&file, CArchive::store);
		Serialize(ar);
		ar.Close();

		if (!SaveIndex())
		{
			AfxMessageBox(_T("Index file save error!"));
			return FALSE;
		}

		return TRUE;

	}
	else
	{
		AfxMessageBox(_T("Project file save error!"));
		return FALSE;
	}

	return FALSE;//CDocument::OnSaveDocument(lpszPathName);
}


POSITION CBlogToBookDoc::GetFirstViewPosition() const
{
	// TODO: Add your specialized code here and/or call the base class

	return CDocument::GetFirstViewPosition();
}


BOOL CBlogToBookDoc::SaveModified()
{
	// TODO: Add your specialized code here and/or call the base class

	return CDocument::SaveModified();
}


void CBlogToBookDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
{
	// TODO: Add your specialized code here and/or call the base class

	CDocument::SetPathName(lpszPathName, bAddToMRU);
}


void CBlogToBookDoc::SetTitle(LPCTSTR lpszTitle)
{
	// TODO: Add your specialized code here and/or call the base class

	CDocument::SetTitle(lpszTitle);
}


CString CBlogToBookDoc::GetUserDocumentPath(UINT type)
{
	TCHAR my_documents[MAX_PATH];
	HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents);

	if (result != S_OK)
	{
		return _T("");
	}
	else
	{
		CString str;
		str.SetString(my_documents);
		if (type == B2B_FOLDER_BLANK)
		{
			str = str + _T("\\Oormi Creations\\Blog To Book\\Blank");
		}

		if (type == B2B_FOLDER)
		{
			str = str + _T("\\Oormi Creations\\Blog To Book");
		}

		if (GetFileAttributes(str) == INVALID_FILE_ATTRIBUTES)
		{
			//path doesn't exist, attempt creation
			if (SHCreateDirectoryEx(NULL, str, NULL) == ERROR_SUCCESS)
			{
				return str;
			}
			else
			{
				return _T("");
			}
		}

		//path is valid
		return str;
	}

}

void CBlogToBookDoc::SetProjectPaths()
{
	TCHAR strPathName[MAX_PATH];
	::GetModuleFileName(NULL, strPathName, _MAX_PATH);
	m_BlankPath.SetString(strPathName);
	m_BlankPath.Replace(_T("BlogToBook.exe"), _T("Blank\\"));

	m_RawDataPath = m_ProjectPath + _T("Raw\\");

}

void CBlogToBookDoc::OnButtonHelp()
{
	ShellExecute(NULL, _T("open"), _T("https://b2b.oormi.in/help.html"), NULL, NULL, SW_SHOWNORMAL);
}

void CBlogToBookDoc::SetRenderFonts()
{
	CFrameWnd * fwnd = (CFrameWnd *)AfxGetMainWnd();
	CBlogToBookView * view = (CBlogToBookView*)fwnd->GetActiveView();
	if (view == NULL)return;
	view->UpdateRenderFonts(m_BFontSz, m_TFontSz, m_BFont, m_TFont);

}

void CBlogToBookDoc::OnButtonUpdate()
{
	ShowCaption(_T("Checking for updates ..."));
	m_BlogPageRaw = _T("");

	if (Fetch(_T("https://oormi.in/software/updateb2b.txt")))
	{
		if (!m_BlogPageRaw.IsEmpty())
		{
			CString ver1, ver2, durl;

			AfxExtractSubString(ver1, m_BlogPageRaw, 0, '|'); //major ver num
			AfxExtractSubString(ver2, m_BlogPageRaw, 1, '|'); //minor ver num
			AfxExtractSubString(durl, m_BlogPageRaw, 2, '|'); //downlaod url

			int oldver = (int)m_B2BVersion*10-1;
			int newver = _ttoi(ver1)*10 + _ttoi(ver2);

			if (newver > oldver)
			{
				int res = AfxMessageBox(_T("A new version of Blog to Book is available for download. Would you like to download it now?"), MB_YESNO);
				if (res == IDYES)
				{
					ShellExecute(NULL, _T("open"), durl, NULL, NULL, SW_SHOWNORMAL);
					ShowCaption(_T("Blog to Book download site was opened."));
				}
				else
				{
					ShowCaption(_T("Blog to Book can be updated from https://github.com/oormicreations/BlogToBook"));
				}
			}
			else
			{
				ShowCaption(_T("Your version of Blog to Book is up to date!"));
			}
		}

	}
	else
	{
		ShowCaption(_T("Failed to connect to the internet!"));
	}
}

UINT UsageProc(LPVOID param)
{
#ifndef _DEBUG
	static TCHAR frmdata[] = _T("appinstall=b2b_install&ver=1.0.0");
#else
	static TCHAR frmdata[] = _T("appinstall=b2b_install_D&ver=1.0.0");
#endif

	static TCHAR hdrs[] = (_T("Content-Type: application/x-www-form-urlencoded"));
	LPCTSTR accept[2] = { _T("*/*"), NULL };

	HINTERNET hInternet = InternetOpen(_T("B2B Usage"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	HINTERNET hSession = InternetConnect(hInternet, _T("oormi.in"), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);

	HINTERNET hReq = HttpOpenRequest(hSession, _T("POST"), _T("software/selftalkmsg/selftalkstat01.php"), NULL, NULL, accept, INTERNET_FLAG_SECURE, 1);

	DWORD x = _tcslen(hdrs);
	//not working with unicode version
	//BOOL res = HttpSendRequest(hReq, _T("Content-Type: application/x-www-form-urlencoded"), _tcslen(hdrs), _T("appinstall=b2b_install&ver=1.0.0"), _tcslen(frmdata));
	BOOL res = HttpSendRequestA(hReq, ("Content-Type: application/x-www-form-urlencoded"), _tcslen(hdrs), ("appinstall=b2b_install&ver=1.0.0"), _tcslen(frmdata));

	InternetCloseHandle(hReq);
	InternetCloseHandle(hSession);
	InternetCloseHandle(hInternet);
	return 0;
}

void CBlogToBookDoc::ReportUsage()
{
	CWinThread* hTh1 = AfxBeginThread(UsageProc, 0/*UsageProc receives this as param */, THREAD_PRIORITY_NORMAL);
}


void CBlogToBookDoc::SaveSettings()
{
	CFile SettFile;
	BOOL m_IsSettFileOpen = SettFile.Open(GetUserDocumentPath(B2B_FOLDER) + _T("\\B2BSettings.set"), CFile::modeCreate | CFile::modeWrite);
	if (!m_IsSettFileOpen) { return; }

	CArchive archive(&SettFile, CArchive::store);
	archive << m_B2BVersion << m_IsInstalled;

	archive.Close();
	SettFile.Close();
}

void CBlogToBookDoc::LoadSettings()
{
	CFile SettFile;
	BOOL m_IsSettFileOpen = SettFile.Open(GetUserDocumentPath(B2B_FOLDER) + "\\B2BSettings.set", CFile::modeRead);
	if (!m_IsSettFileOpen) { return; }
	CArchive archive(&SettFile, CArchive::load);

	int ver, type;
	archive >> ver;

	if (ver == m_B2BVersion)
	{
		archive >> m_IsInstalled;
	}

	archive.Close();
	SettFile.Close();

}