// NewProjectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BlogToBook.h"
#include "NewProjectDlg.h"
#include "afxdialogex.h"
#include "MainFrm.h"


// CNewProjectDlg dialog

IMPLEMENT_DYNAMIC(CNewProjectDlg, CDialog)

CNewProjectDlg::CNewProjectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_NEW_PROJECT, pParent)
	, m_Name(_T(""))
	, m_Folder(_T(""))
	, m_ProjPath(_T(""))
{
	m_Open = FALSE;
}

CNewProjectDlg::~CNewProjectDlg()
{
}

void CNewProjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CNewProjectDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_PROJBROWSE, &CNewProjectDlg::OnBnClickedButtonProjBrowse)
	ON_BN_CLICKED(IDC_BUTTON_PROJOPEN, &CNewProjectDlg::OnBnClickedButtonProjopen)
	ON_BN_CLICKED(IDOK, &CNewProjectDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CNewProjectDlg message handlers


void CNewProjectDlg::OnBnClickedButtonProjBrowse()
{
	CString folderPath;
	CString initialFolder = GetB2BPath();

	CFolderPickerDialog folderPickerDialog(initialFolder, OFN_FILEMUSTEXIST | /*OFN_ALLOWMULTISELECT |*/ OFN_ENABLESIZING, this, sizeof(OPENFILENAME));

	if (folderPickerDialog.DoModal() == IDOK)
	{
		m_Folder = folderPickerDialog.GetPathName();
		SetDlgItemText(IDC_EDIT_PROJFOLDER, m_Folder);
	}

}


void CNewProjectDlg::OnBnClickedButtonProjopen()
{
	m_Open = TRUE;
	CDialog::OnOK();
}


BOOL CNewProjectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_Name = _T("B2B Project 01");
	m_Folder = GetB2BPath();
	m_Open = FALSE;

	SetDlgItemText(IDC_EDIT_PROJFOLDER, m_Folder);
	SetDlgItemText(IDC_EDIT_PROJNAME, m_Name);
	

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

CString CNewProjectDlg::GetB2BPath()
{
	CString str;
	TCHAR my_documents[MAX_PATH];
	HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents);

	if (result == S_OK)
	{
		str.SetString(my_documents);
		str = str + _T("\\Oormi Creations\\Blog To Book");

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
	}
	//path is valid
	return str;
}


void CNewProjectDlg::OnBnClickedOk()
{
	GetDlgItemText(IDC_EDIT_PROJNAME, m_Name);
	GetDlgItemText(IDC_EDIT_PROJFOLDER, m_Folder);
	
	//trailing \ in path
	int res = m_Folder.Find(_T("\\"), m_Folder.GetLength() - 1);
	
	CString str;
	if(res>0) str = m_Folder + m_Name;
	else  str = m_Folder + _T("\\") + m_Name;

	if (GetFileAttributes(str) == INVALID_FILE_ATTRIBUTES)
	{
		if (SHCreateDirectoryEx(NULL, str, NULL) != ERROR_SUCCESS)
		{
			AfxMessageBox(_T("Failed to create the project folder.\r\nEnsure you have rights or choose another location."));
		}
		CDialog::OnOK();
	}
	else
	{
		AfxMessageBox(_T("A project with same name already exists in the same folder.\r\nUse another name or location."));
	}
}
