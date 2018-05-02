#pragma once
class CArticle
{
public:
	CArticle();
	~CArticle();

	CString m_Title;
	CString m_Url;
	CString m_Date;
	CString m_RawFile, m_PreviewFile;
	BOOL m_Included;

};

