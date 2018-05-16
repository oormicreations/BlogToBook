// DesignService.cpp : implementation file
//

#include "stdafx.h"
#include "BlogToBook.h"
#include "DesignService.h"
#include "afxdialogex.h"
#include "wininet.h"

extern string Utf8Encode(const wstring &wstr);

// CDesignService dialog

IMPLEMENT_DYNAMIC(CDesignService, CDialogEx)

CDesignService::CDesignService(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_DESIGN, pParent)
{

}

CDesignService::~CDesignService()
{
}

void CDesignService::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDesignService, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_DES_SEND, &CDesignService::OnBnClickedButtonDesSend)
	ON_EN_SETFOCUS(IDC_EDIT_DES_FROM, &CDesignService::OnEnSetfocusEditDesFrom)
END_MESSAGE_MAP()


// CDesignService message handlers


//string Utf8Encode(const wstring &wstr)
//{
//	string out;
//	int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);
//	if (len > 0)
//	{
//		out.resize(len);
//		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), &out[0], len, NULL, NULL);
//	}
//	return out;
//}

BOOL CDesignService::SendReq(wstring fdata)
{

	static TCHAR hdrs[] = (_T("Content-Type: application/x-www-form-urlencoded; charset=utf-8"));
	LPCTSTR accept[2] = { _T("*/*"), NULL };

	HINTERNET hInternet = InternetOpen(_T("B2B Showcase"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!hInternet) return FALSE;

	HINTERNET hSession = InternetConnect(hInternet, _T("b2b.oormi.in"), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);
	if (!hSession) return FALSE;

	HINTERNET hReq = HttpOpenRequest(hSession, _T("POST"), _T("b2bdesignreq.php"), NULL, NULL, accept, INTERNET_FLAG_SECURE, 1);
	if (!hReq) return FALSE;

	wstring headers = L"Content-Type: application/x-www-form-urlencoded; charset=utf-8";

	string postData = Utf8Encode(fdata);

	BOOL res = HttpSendRequest(hReq, headers.c_str(), headers.length(), (LPVOID)postData.c_str(), postData.size());

	if (res)
	{
		CHAR szBuffer[2048];
		DWORD dwRead;
		while (InternetReadFile(hReq, szBuffer, sizeof(szBuffer) - 1, &dwRead) == TRUE)
		{
			if (dwRead > 0)
			{
				szBuffer[dwRead] = 0;
				CString str(szBuffer);
				if (str != _T("Message sent!Success")) return FALSE;
			}
			else break;
		}
	}


	InternetCloseHandle(hReq);
	InternetCloseHandle(hSession);
	InternetCloseHandle(hInternet);
	return TRUE;
}

void CDesignService::OnBnClickedButtonDesSend()
{
	CString from, body;
	GetDlgItemText(IDC_EDIT_DES_FROM, from);
	GetDlgItemText(IDC_EDIT_DES_DESC, body);

/*	if (from.IsEmpty() || body.IsEmpty() || (body == m_DesBody) || (from == _T("<Your email address>")))
	{
		AfxMessageBox(_T("Your Email address and requirements are needed!"));
		return;
	}

	int res1 = from.Find(_T("@"));
	int res2 = from.Find(_T("."));
	if ((res1 < 0) || (res2 < 0))
	{
		AfxMessageBox(_T("Your Email address is incorrect!"));
		return;
	}

	int len1 = from.GetLength();
	int len2 = body.GetLength();
	if ((len1 < 6) || (len1 > 500) || (len2 < 20) || (len2 > 2000))
	{
		AfxMessageBox(_T("Some issues were found with the data entered!\r\nEnsure its correct and concise."));
		return;
	}
*/
	from = _T("tarun.pradhaan@gmail.com");
	CString fdata;
	fdata = _T("from=") + from + _T("&body=") + body;

	wstring stra(fdata);

	if (!SendReq(stra))
	{
		AfxMessageBox(_T("Error Request could not be sent.\r\nEnsure you are connected to the net."));
		return;
	}

	AfxMessageBox(_T("Your Cover Design Request was sent!\r\nWe will get back to you asap.\r\nThank you!"));

}


BOOL CDesignService::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	m_DesBody = _T("Hello,\r\n\r\nI need an awesome book cover for my EBook <title> by <author>.\r\n\r\n \
Here is what I need exactly - <requirements>.\r\n\r\nIt should be inspired by these images - <links to images and references>.\r\n\r\n \
Please send me a quote asap. My budget is <range>.\r\n\r\nThank you!\r\n\r\n<your name>");

	SetDlgItemText(IDC_EDIT_DES_TO, _T("oormicreations@gmail.com"));
	SetDlgItemText(IDC_EDIT_DES_FROM, _T("<Your email address>"));
	SetDlgItemText(IDC_EDIT_DES_SUB, _T("Design my book cover"));
	SetDlgItemText(IDC_EDIT_DES_DESC, m_DesBody);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CDesignService::OnEnSetfocusEditDesFrom()
{
	CString from;
	GetDlgItemText(IDC_EDIT_DES_FROM, from);
	if(from == _T("<Your email address>")) 	SetDlgItemText(IDC_EDIT_DES_FROM, _T(""));

}
