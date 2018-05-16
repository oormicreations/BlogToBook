#pragma once
#include <string>

using namespace std;


// CDesignService dialog

class CDesignService : public CDialogEx
{
	DECLARE_DYNAMIC(CDesignService)

public:
	CDesignService(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDesignService();

	CString m_DesBody;
	BOOL SendReq(wstring fdata);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_DESIGN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonDesSend();
	virtual BOOL OnInitDialog();
	afx_msg void OnEnSetfocusEditDesFrom();
};
