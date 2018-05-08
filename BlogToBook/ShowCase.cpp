// ShowCase.cpp : implementation file
//

#include "stdafx.h"
#include "BlogToBook.h"
#include "ShowCase.h"
#include "afxdialogex.h"
#include "wininet.h"
#include <Wincrypt.h>
#pragma comment(lib, "crypt32.lib")

#define BUFSIZE 300 //multiple of 3

#include <memory>
#include <algorithm>

string g_BookPath;
string g_CoverPath;

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
END_MESSAGE_MAP()


// CShowCase message handlers


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
	wstring fdata;
	fdata.append((wchar_t*)param);

	static TCHAR hdrs[] = (_T("Content-Type: application/x-www-form-urlencoded; charset=utf-8"));
	LPCTSTR accept[2] = { _T("*/*"), NULL };

/*	HINTERNET hInternet = InternetOpen(_T("B2B Showcase"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	HINTERNET hSession = InternetConnect(hInternet, _T("b2b.oormi.in"), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);

	HINTERNET hReq = HttpOpenRequest(hSession, _T("POST"), _T("b2bsubmit.php"), NULL, NULL, accept, INTERNET_FLAG_SECURE, 1);

	wstring headers = L"Content-Type: application/x-www-form-urlencoded; charset=utf-8";

	string postData = Utf8Encode(fdata);//(L"b2bver=1.0.0&b2bid=12345678");

	BOOL res = HttpSendRequest(hReq, headers.c_str(), headers.length(), (LPVOID)postData.c_str(), postData.size());

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
				if (str != _T("Success")) return 1;
			}
			else break;
		}
	}


	InternetCloseHandle(hReq);
	InternetCloseHandle(hSession);
	InternetCloseHandle(hInternet);
*/
	//string f = "C:\\Users\\Sanjeev\\Documents\\Oormi Creations\\Blog To Book\\B2B Project 01\\Raw\\pre001.txt";
	CString str;
	int res = UploadFile(g_CoverPath);
	str.Format(_T("Error Upload Book File: %d"), res);
	if (res)
	{
		AfxMessageBox(str);
		return 1;
	}
	else AfxMessageBox(_T("Uploaded"));

	//res = UploadFile(g_BookPath);
	//str.Format(_T("Error Upload Cover File: %d"), res);
	//if (!res)
	//{
	//	AfxMessageBox(str);
	//	return 1;
	//}


	return 0;
}

BOOL CShowCase::B64Encode(CString sfilename)
{
	CFile sfile;
	if (sfile.Open(sfilename, CFile::modeRead))

	{
		UINT slen = (UINT)sfile.GetLength();
		if (slen > 0)
		{
			CString tfilename = sfilename;
			tfilename.Replace(_T(".jpg"), _T(".jpg.txt"));
			tfilename.Replace(_T(".epub"), _T(".epub.txt"));
			//tfilename.Replace(_T("pre"), _T("epre"));

			CFile tfile;
			if (tfile.Open(tfilename, CFile::modeWrite | CFile::modeCreate))
			{
				unsigned char sbuf[300];
				ZeroMemory(sbuf, 300);
				unsigned char tbuf[400];
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
	GetDlgItemText(IDC_EDIT_SC_PLINK, m_Data[12]);
	GetDlgItemText(IDC_EDIT_SC_TAGS, m_Data[13]);
	GetDlgItemText(IDC_EDIT_SC_COMMENTS, m_Data[14]);

	m_Result = _T("No Connection");
	m_Success = FALSE;

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

	BOOL res1 = B64Encode(m_Data[10]);
	//BOOL res1 = B64Encode(_T("C:\\Users\\Sanjeev\\Documents\\Oormi Creations\\Blog To Book\\B2B Project 01\\Raw\\pre005.txt")/*m_Data[10]*/);
	//BOOL res2 = B64Encode(m_Data[16]);

	//if (!res1 || !res2)
	//{
	//	AfxMessageBox(_T("Error encoding Ebook data:\r\n"));
	//}

	//return;
	m_Data[10].Replace(_T(".jpg"), _T(".jpg.txt"));
	m_Data[16].Replace(_T(".epub"), _T(".epub.txt"));
	g_CoverPath = CT2A(m_Data[10]);
	g_BookPath = CT2A(m_Data[16]);

	m_ProgCtrl.SetPos(10);

	CWinThread* hTh1 = AfxBeginThread(B2BDataProc, formData.GetBuffer()/*B2BDataProc receives this as param */, THREAD_PRIORITY_NORMAL);

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

	CString attribs[] = { _T("Name"),
	_T("Author"),
	_T("Url"),
	_T("Description"),
	_T("Plublisher"),
	_T("Date Published"),
	_T("Language"),
	_T("ISBN") };

	m_SCListCtrl.InsertColumn(0, _T("Attribute"), LVCFMT_LEFT, 100);
	m_SCListCtrl.InsertColumn(1, _T("Value"), LVCFMT_LEFT, 350);

	for (int i = 0; i < 8; i++)
	{
		m_SCListCtrl.InsertItem(i, attribs[i]);
		m_SCListCtrl.SetItemText(i, 1, m_Data[i+2]);
	}

	m_SCListCtrl.SetExtendedStyle(m_SCListCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES);


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
	// TODO: Add your control notification handler code here
}

