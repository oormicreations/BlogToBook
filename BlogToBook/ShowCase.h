#pragma once
using namespace std;
#include <string>
#include "afxcmn.h"

// CShowCase dialog

class CShowCase : public CDialog
{
	DECLARE_DYNAMIC(CShowCase)

public:
	CShowCase(CWnd* pParent = NULL);   // standard constructor
	virtual ~CShowCase();

// Dialog Data
	CString /*m_PrintLink, m_Tags, m_Comments,*/ m_Result;
	BOOL m_Agree, m_Success, m_Removed;
	CString m_Data[20];
	int m_DataCount;
	CProgressCtrl m_ProgCtrl;

	//string Utf8Encode(const wstring & wstr);
	//void UploadFile(CString fpath);
	BOOL B64Encode(CString file, CString id, int type);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SHOWCASE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonScVisit();
	afx_msg void OnBnClickedButtonScUpload();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonScRem();
	CListCtrl m_SCListCtrl;
};
