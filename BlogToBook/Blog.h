#pragma once
#include "Article.h"
#define MAXARTICLES 200
#define MAXEXTRAPAGES 4

class CBlog
{
public:
	CBlog();
	~CBlog();

	CString m_BlogUrl, m_BlogName, m_BlogAuthor, m_BlogDesc, m_BlogPlublisher, m_BlogPubDate, m_BlogLang, m_BlogModDate, m_BlogISBN;
	CString m_Copyright, m_CopyrightTitle, m_EndPage, m_EndPageTitle, m_Preface, m_PrefaceTitle, m_Dedication, m_DedicationTitle, m_CoverPath, m_CoverTitle;
	int m_YearStart, m_MonthStart, m_YearEnd, m_MonthEnd;
	CArticle m_Articles[MAXARTICLES];
	int m_ArticleCount;
	BOOL m_IncludeB2BRef;

	BOOL GetDateRange(void);
	BOOL CheckDate(CString str, int type);

	void Clear();
	void SetDemoData();
	void SetArticle(int arNum, CString title, CString date, CString url, BOOL enabled, CString rfile, CString pfile);
	void SetArticle(int arNum, CString * data);
	void GetArticle(int arNum, CString * data);
	CString GetFirstName();
	CString GetLastName();

	BOOL SetBlogInfo();
	BOOL GetBlogInfo();
	void SetBookInfo();
	void GetBookInfo();

};

