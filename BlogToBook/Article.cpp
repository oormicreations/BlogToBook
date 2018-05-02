#include "stdafx.h"
#include "Article.h"


CArticle::CArticle()
{
	m_Title = _T("");
	m_Url = _T("");
	m_Date = _T("");
	m_Included = TRUE;
}


CArticle::~CArticle()
{
}
