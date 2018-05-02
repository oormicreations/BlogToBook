#pragma once
#include "afxwin.h"


// CAttribDlg dialog

class CAttribDlg : public CDialog
{
	DECLARE_DYNAMIC(CAttribDlg)

public:
	CAttribDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAttribDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ATTRIB };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CButton m_IncludeCtrl;
	CString m_AttribHeader;
	CString m_AttContent;
	CString m_Caption;
	BOOL m_Include;

	afx_msg void OnBnClickedCheckAttInculde();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedSave();
};
