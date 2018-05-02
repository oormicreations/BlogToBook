#pragma once


// CNewProjectDlg dialog

class CNewProjectDlg : public CDialog
{
	DECLARE_DYNAMIC(CNewProjectDlg)

public:
	CNewProjectDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNewProjectDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_NEW_PROJECT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_Name;
	CString m_Folder;
	CString m_ProjPath;
	BOOL m_Open;

	afx_msg void OnBnClickedButtonProjBrowse();
	afx_msg void OnBnClickedButtonProjopen();
	virtual BOOL OnInitDialog();
	CString GetB2BPath();
	afx_msg void OnBnClickedOk();
};
