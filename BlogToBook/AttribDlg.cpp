// AttribDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BlogToBook.h"
#include "AttribDlg.h"
#include "afxdialogex.h"


// CAttribDlg dialog

IMPLEMENT_DYNAMIC(CAttribDlg, CDialog)

CAttribDlg::CAttribDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_ATTRIB, pParent)
	, m_AttribHeader(_T(""))
	, m_AttContent(_T(""))
{
	m_Include = TRUE;
}

CAttribDlg::~CAttribDlg()
{
}

void CAttribDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_ATT_INCULDE, m_IncludeCtrl);
	DDX_Text(pDX, IDC_EDIT_ATT_HEAD, m_AttribHeader);
	DDX_Text(pDX, IDC_EDIT_ATT_CONTENT, m_AttContent);

}


BEGIN_MESSAGE_MAP(CAttribDlg, CDialog)
	ON_BN_CLICKED(IDC_CHECK_ATT_INCULDE, &CAttribDlg::OnBnClickedCheckAttInculde)
	ON_BN_CLICKED(IDSAVE, &CAttribDlg::OnBnClickedSave)
END_MESSAGE_MAP()


// CAttribDlg message handlers


void CAttribDlg::OnBnClickedCheckAttInculde()
{
	m_Include = m_IncludeCtrl.GetCheck();
	GetDlgItem(IDC_EDIT_ATT_HEAD)->EnableWindow(m_Include);
	GetDlgItem(IDC_EDIT_ATT_CONTENT)->EnableWindow(m_Include);
}


BOOL CAttribDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_IncludeCtrl.SetCheck(m_Include);
	GetDlgItem(IDC_EDIT_ATT_HEAD)->EnableWindow(m_Include);
	GetDlgItem(IDC_EDIT_ATT_CONTENT)->EnableWindow(m_Include);
	SetWindowText(m_Caption);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CAttribDlg::OnBnClickedSave()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}
