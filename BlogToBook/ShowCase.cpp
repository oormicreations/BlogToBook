// ShowCase.cpp : implementation file
//

#include "stdafx.h"
#include "BlogToBook.h"
#include "ShowCase.h"
#include "afxdialogex.h"
#include "wininet.h"
//#include <Wincrypt.h>
#include <memory>
#include <algorithm>
//#pragma comment(lib, "crypt32.lib")

#define BUFSIZE 2048
#define SC_THREAD_NOTIFY (WM_APP + 1)


string g_BookPath;
string g_CoverPath;
wstring g_Form;
BOOL g_Success;

struct ThreadParam
{
	HWND mDlg;
};


static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static inline bool is_base64(unsigned char c)
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}

unsigned int base64_encode(const unsigned char* bytes_to_encode, unsigned int in_len, unsigned char* encoded_buffer, unsigned int& out_len)
{
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3] = { 0, 0, 0 };
	unsigned char char_array_4[4] = { 0, 0, 0, 0 };

	out_len = 0;
	while (in_len--)
	{
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3)
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; i < 4; i++)
			{
				encoded_buffer[out_len++] = base64_chars[char_array_4[i]];
			}
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
		{
			char_array_3[j] = '\0';
		}

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; j < (i + 1); j++)
		{
			encoded_buffer[out_len++] = base64_chars[char_array_4[j]];
		}

		while (i++ < 3)
		{
			encoded_buffer[out_len++] = '=';
		}
	}

	return out_len;
}

unsigned int base64_decode(const unsigned char* encoded_string, unsigned int in_len, unsigned char* decoded_buffer, unsigned int& out_len)
{
	size_t i = 0;
	size_t j = 0;
	int in_ = 0;
	unsigned char char_array_3[3] = { 0, 0, 0 };
	unsigned char char_array_4[4] = { 0, 0, 0, 0 };

	out_len = 0;
	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
	{
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4)
		{
			for (i = 0; i < 4; i++)
			{
				char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));
			}

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; i < 3; i++)
			{
				decoded_buffer[out_len++] = char_array_3[i];
			}
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 4; j++)
		{
			char_array_4[j] = 0;
		}

		for (j = 0; j < 4; j++)
		{
			char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));
		}

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++)
		{
			decoded_buffer[out_len++] = char_array_3[j];
		}
	}
	return out_len;
}




struct FileCloser
{
	typedef HANDLE pointer;

	void operator()(HANDLE h)
	{
		if (h != INVALID_HANDLE_VALUE)
			CloseHandle(h);
	}
};

struct InetCloser
{
	typedef HINTERNET pointer;

	void operator()(HINTERNET h)
	{
		if (h != NULL)
			InternetCloseHandle(h);
	}
};

bool WriteToInternet(HINTERNET hInet, const void *Data, DWORD DataSize)
{
	const BYTE *pData = (const BYTE *)Data;
	DWORD dwBytes;

	while (DataSize > 0)
	{
		if (!InternetWriteFile(hInet, pData, DataSize, &dwBytes))
		{
			DWORD err = GetLastError();
			InternetGetLastResponseInfoA(&err, (LPSTR)pData, &DataSize);
			AfxMessageBox(_T("Write err"));
			return false;
		}
		pData += dwBytes;
		DataSize -= dwBytes;
	}

	return true;
}

int UploadFile(string filename)
{
	//filename = "C:\\Users\\Sanjeev\\Documents\\Oormi Creations\\Blog To Book\\B2B Project 01\\Raw\\book_cover.txt";

	string szContent = "------974767299852498929531610575\r\nContent-Disposition: form-data; name=\"uploadedfile\"; filename=\"" 
						+ filename + "\"\r\nContent-Type: application/octet-stream\r\n\r\n";
	char *szHeaders = ("Content-Type: multipart/form-data; boundary=----974767299852498929531610575");
	char *szEndData = ("\r\n------974767299852498929531610575--\r\n");
	LPCSTR accept[] = { ("*/*"), NULL };

	std::unique_ptr<HANDLE, FileCloser> hIn(CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL));
	if (hIn.get() == INVALID_HANDLE_VALUE)
	{
		return 1;
	}

	DWORD dwFileSize = GetFileSize(hIn.get(), NULL);
	if (dwFileSize == INVALID_FILE_SIZE)
	{
		return 2;
	}

	std::unique_ptr<HINTERNET, InetCloser> io(InternetOpenA("B2B Showcase", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0));
	if (io.get() == NULL)
	{
		return 3;
	}

	std::unique_ptr<HINTERNET, InetCloser> ic(InternetConnectA(io.get(), "b2b.oormi.in", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1));
	if (ic.get() == NULL)
	{
		return 4;
	}

	std::unique_ptr<HINTERNET, InetCloser> hreq(HttpOpenRequestA(ic.get(), "POST", ("upload.php"), NULL, NULL, accept, INTERNET_FLAG_SECURE | INTERNET_FLAG_KEEP_CONNECTION, 1));
	if (hreq.get() == NULL)
	{
		return 5;
	}

	if (!HttpAddRequestHeadersA(hreq.get(), szHeaders, -1, HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD))
	{
		return 6;
	}

	size_t sContentSize = strlen(szContent.c_str());
	size_t sEndDataSize = strlen(szEndData);

	INTERNET_BUFFERSA bufferIn = {};
	bufferIn.dwStructSize = sizeof(INTERNET_BUFFERSA);
	bufferIn.dwBufferTotal = sContentSize + dwFileSize + sEndDataSize;

	if (!HttpSendRequestExA(hreq.get(), &bufferIn, NULL, HSR_INITIATE, 0))
	{
		return 7;
	}

	if (!WriteToInternet(hreq.get(), szContent.c_str(), sContentSize))
	{
		return 8;
	}

	BYTE szData[BUFSIZE];
	DWORD dw = 0, dwBytes;

	while (dw < dwFileSize)
	{
		if (!ReadFile(hIn.get(), szData, sizeof(szData), &dwBytes, NULL))
		{
			return 9;
		}

		//DWORD nDestinationSize = 0;
		//if (CryptBinaryToStringA(reinterpret_cast<const BYTE*> (szData), BUFSIZE, CRYPT_STRING_BASE64| CRYPT_STRING_NOCRLF, nullptr, &nDestinationSize))
		//{
		//	LPVOID pszDestination = /*static_cast<LPSTR>*/ (HeapAlloc(GetProcessHeap(), HEAP_NO_SERIALIZE, nDestinationSize /** sizeof(CHAR)*/));
		//	if (pszDestination)
		//	{
		//		if (CryptBinaryToStringA(reinterpret_cast<const BYTE*> (szData), BUFSIZE, CRYPT_STRING_BASE64| CRYPT_STRING_NOCRLF, (LPSTR)pszDestination, &nDestinationSize))
		//		{
					// Succeeded: 'pszDestination' is 'pszSource' encoded to base64.
					//if (!WriteToInternet(hreq.get(), pszDestination, nDestinationSize))
					if (!WriteToInternet(hreq.get(), szData, dwBytes))
					{
						return 10;
					}

					dw += dwBytes;
					TRACE("dwB=%d\r\n", dw);
		//		}
		//	}
		//	HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pszDestination);
		//}


	}


	if (!WriteToInternet(hreq.get(), szEndData, sEndDataSize))
	{
		return 11;
	}


	if (!HttpEndRequest(hreq.get(), NULL, HSR_INITIATE, 0))
	{
		return 12;
	}

	return 0;
}


// CShowCase dialog

IMPLEMENT_DYNAMIC(CShowCase, CDialog)

CShowCase::CShowCase(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_SHOWCASE, pParent)
{
	m_Success = m_Agree = FALSE;
	m_DataCount = 0;
}

CShowCase::~CShowCase()
{
}

void CShowCase::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_SC, m_ProgCtrl);
	DDX_Control(pDX, IDC_LIST_SCDATA, m_SCListCtrl);
}


BEGIN_MESSAGE_MAP(CShowCase, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SC_VISIT, &CShowCase::OnBnClickedButtonScVisit)
	ON_BN_CLICKED(IDC_BUTTON_SC_UPLOAD, &CShowCase::OnBnClickedButtonScUpload)
	ON_BN_CLICKED(IDOK, &CShowCase::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_SC_REM, &CShowCase::OnBnClickedButtonScRem)
	ON_MESSAGE(SC_THREAD_NOTIFY, OnSCThreadNotify)
END_MESSAGE_MAP()


// CShowCase message handlers
LRESULT CShowCase::OnSCThreadNotify(WPARAM wp, LPARAM lp)
{
	m_ProgCtrl.SetPos((int)wp);

	if ((int)lp == 3) SetDlgItemText(IDC_EDIT_SCMSG, _T("Sending data"));
	if ((int)lp == 5) SetDlgItemText(IDC_EDIT_SCMSG, _T("Data uploaded"));
	if ((int)lp == 7) SetDlgItemText(IDC_EDIT_SCMSG, _T("Cover image uploaded"));
	if ((int)lp == 9) SetDlgItemText(IDC_EDIT_SCMSG, _T("EBook uploaded. Its Done!"));

	if ((int)lp == 10) ShellExecute(NULL, _T("open"), _T("http://b2b.oormi.in"), NULL, NULL, SW_SHOWNORMAL);

	return 0;
}

void CShowCase::OnBnClickedButtonScVisit()
{
	ShellExecute(NULL, _T("open"), _T("http://b2b.oormi.in"), NULL, NULL, SW_SHOWNORMAL);
}


string Utf8Encode(const wstring &wstr)
{
	string out;
	int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);
	if (len > 0)
	{
		out.resize(len);
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), &out[0], len, NULL, NULL);
	}
	return out;
}

UINT B2BDataProc(LPVOID param)
{
	ThreadParam* p = static_cast<ThreadParam*> (param);

	//LPCWSTR pdata = (TCHAR*)param;
	//string stra;
	//stra = CT2A(pdata);

	wstring fdata;
	fdata = g_Form;
	//fdata.append((wchar_t*)param);

	static TCHAR hdrs[] = (_T("Content-Type: application/x-www-form-urlencoded; charset=utf-8"));
	LPCTSTR accept[2] = { _T("*/*"), NULL };

	HINTERNET hInternet = InternetOpen(_T("B2B Showcase"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	HINTERNET hSession = InternetConnect(hInternet, _T("b2b.oormi.in"), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);

	HINTERNET hReq = HttpOpenRequest(hSession, _T("POST"), _T("b2bsubmit.php"), NULL, NULL, accept, INTERNET_FLAG_SECURE, 1);

	wstring headers = L"Content-Type: application/x-www-form-urlencoded; charset=utf-8";

	string postData = Utf8Encode(fdata);//(L"b2bver=1.0.0&b2bid=12345678");

	::SendMessage(p->mDlg, SC_THREAD_NOTIFY, 30, 3);

	BOOL res = HttpSendRequest(hReq, headers.c_str(), headers.length(), (LPVOID)postData.c_str(), postData.size());
	//BOOL res = HttpSendRequest(hReq, headers.c_str(), headers.length(), (LPVOID)stra.c_str(), stra.size());

	if (res)
	{
		CHAR szBuffer[32];
		DWORD dwRead;
		while (InternetReadFile(hReq, szBuffer, sizeof(szBuffer) - 1, &dwRead) == TRUE)
		{
			if (dwRead > 0)
			{
				szBuffer[dwRead] = 0;
				CString str(szBuffer);
				if (str != _T("Success"))
				{
					AfxMessageBox(_T("Error sending book data!"));
					return 1;
				}
			}
			else break;
		}
	}


	InternetCloseHandle(hReq);
	InternetCloseHandle(hSession);
	InternetCloseHandle(hInternet);

	::SendMessage(p->mDlg, SC_THREAD_NOTIFY, 50, 5);

	CString str;
	int upres = UploadFile(g_CoverPath);
	if (upres)
	{
		str.Format(_T("Error Upload Cover File: %d"), upres);
		AfxMessageBox(str);
		return 1;
	}
	//else AfxMessageBox(_T("Uploaded"));
	::SendMessage(p->mDlg, SC_THREAD_NOTIFY, 75, 7);

	upres = UploadFile(g_BookPath);
	if (upres)
	{
		str.Format(_T("Error Upload Book File: %d"), upres);
		AfxMessageBox(str);
		return 1;
	}
	::SendMessage(p->mDlg, SC_THREAD_NOTIFY, 90, 9);
	Sleep(1000);
	::SendMessage(p->mDlg, SC_THREAD_NOTIFY, 100, 10);

	g_Success = TRUE;

	delete p;
	return 0;
}

BOOL CShowCase::B64Encode(CString sfilename, CString id, int type)
{
	CFile sfile;
	if (sfile.Open(sfilename, CFile::modeRead))

	{
		UINT slen = (UINT)sfile.GetLength();
		if (slen > 0)
		{
			CString tfilename = sfilename;
			CString sname = sfile.GetFileName();
			tfilename.Replace(sname , id + _T("---") + sname + _T(".txt"));

			if (type == 1) g_CoverPath = CT2A(tfilename);
			if (type == 2) g_BookPath = CT2A(tfilename);

			CFile tfile;
			if (tfile.Open(tfilename, CFile::modeWrite | CFile::modeCreate))
			{
				unsigned char sbuf[300];//sz must be multiple of 3
				ZeroMemory(sbuf, 300);
				unsigned char tbuf[400];//sz must be multiple of 4
				ZeroMemory(tbuf, 400);

				UINT nread = 0;
				UINT nenc = 0;
				do {
					nread = sfile.Read(sbuf, 300);
					if (nread > 0)
					{
						base64_encode(sbuf, nread, tbuf, nenc);
						tfile.Write(tbuf, nenc);
					}
				} while (nread > 0);

				tfile.Close();
				sfile.Close();
				return TRUE;
			}
		}

	}

	return FALSE;
}


void CShowCase::OnBnClickedButtonScUpload()
{
	if (g_Success)
	{
		AfxMessageBox(_T("You have already uploaded the EBook and data successfully!\r\nVisit the site to view it."));
		return;
	}

	GetDlgItemText(IDC_EDIT_SC_PLINK, m_Data[12]);
	GetDlgItemText(IDC_EDIT_SC_TAGS, m_Data[13]);
	GetDlgItemText(IDC_EDIT_SC_COMMENTS, m_Data[14]);

	CButton *chk = (CButton*)GetDlgItem(IDC_CHECK_TERMS);
	if (!chk->GetCheck())
	{
		AfxMessageBox(_T("You need to agree to the T&C by checking the check box!"));
		return;
	}

	m_Result = _T("No Connection");
	g_Success = FALSE;

	CString formData;// = _T("b2bver=1.0.0&b2bid=12345678");
	CString params[] = {
	_T("b2bver="),
	_T("&b2bid="),
	_T("&b2bname="),
	_T("&b2bauthor="),
	_T("&b2burl="),
	_T("&b2bdesc="),
	_T("&b2bpub="),
	_T("&b2bpubdate="),
	_T("&b2blang="),
	_T("&b2bisbn="),
	_T("&b2bcover="),
	_T("&b2blic="),
	_T("&b2bprintlink="),
	_T("&b2btags="),
	_T("&b2bcomments="),
	_T("&b2bformat=") };

	for (int i = 0; i < m_DataCount; i++)
	{
		formData = formData + params[i] + m_Data[i];
	}

	SetDlgItemText(IDC_EDIT_SCMSG, _T("Encoding data"));
	m_ProgCtrl.SetPos(10);

	BOOL res1 = B64Encode(m_Data[10], m_Data[1], 1);
	BOOL res2 = B64Encode(m_Data[16], m_Data[1], 2);

	if (!res1 || !res2)
	{
		AfxMessageBox(_T("Error encoding Ebook data:\r\n"));
		return;
	}

	SetDlgItemText(IDC_EDIT_SCMSG, _T("Connecting to server"));
	m_ProgCtrl.SetPos(20);

	wstring stra(formData);
	g_Form = stra;//not working when passed via thread

	ThreadParam* param = new ThreadParam;
	param->mDlg = m_hWnd;  // A handle, not a dangerous 'this'

	CWinThread* hTh1 = AfxBeginThread(B2BDataProc, param);// (LPVOID)stra.c_str()/*formData.GetBuffer()*//*B2BDataProc receives this as param */, THREAD_PRIORITY_NORMAL);

	param = 0; // The other thread shall delete it

	//m_Success =	m_Result == _T("Success");

	//if (!m_Success)
	//{
	//	AfxMessageBox(_T("Error uploading Ebook data:\r\n") + m_Result);
	//}

}


BOOL CShowCase::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	SetDlgItemText(IDC_EDIT_SC_PLINK, m_Data[12]);
	SetDlgItemText(IDC_EDIT_SC_TAGS, m_Data[13]);
	SetDlgItemText(IDC_EDIT_SC_COMMENTS, m_Data[14]);

	m_ProgCtrl.SetRange(0, 100);
	m_Removed = FALSE;

	CString attribs[] = { 
	_T("ID"),
	_T("Name"),
	_T("Author"),
	_T("Url"),
	_T("Description"),
	_T("Publisher"),
	_T("Date Published"),
	_T("Language"),
	_T("ISBN") };

	m_SCListCtrl.InsertColumn(0, _T("Attribute"), LVCFMT_LEFT, 100);
	m_SCListCtrl.InsertColumn(1, _T("Value"), LVCFMT_LEFT, 350);

	for (int i = 0; i < 9; i++)
	{
		m_SCListCtrl.InsertItem(i, attribs[i]);
		m_SCListCtrl.SetItemText(i, 1, m_Data[i+1]);
	}

	m_SCListCtrl.SetExtendedStyle(m_SCListCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES);

	SetDlgItemText(IDC_EDIT_SCMSG, _T("This is a free service.You can show off your awesome EBook online."));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CShowCase::OnBnClickedOk()
{
	GetDlgItemText(IDC_EDIT_SC_PLINK, m_Data[12]);
	GetDlgItemText(IDC_EDIT_SC_TAGS, m_Data[13]);
	GetDlgItemText(IDC_EDIT_SC_COMMENTS, m_Data[14]);
	CDialog::OnOK();
}


void CShowCase::OnBnClickedButtonScRem()
{
	if (m_Data[11].IsEmpty())
	{
		AfxMessageBox(_T("Error: Password not found.\r\nPlease contact the webmaster for manual deletion."));
		ShellExecute(NULL, _T("open"), _T("https://b2b.oormi.in"), NULL, NULL, SW_SHOWNORMAL);
		return;
	}

	ShellExecute(NULL, _T("open"), _T("https://b2b.oormi.in/remove.php?b2blic=") + m_Data[11] + _T("&b2bid=") + m_Data[1], NULL, NULL, SW_SHOWNORMAL);
	m_Removed = TRUE;
	g_Success = FALSE;

}

