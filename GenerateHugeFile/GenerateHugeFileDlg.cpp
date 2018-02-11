
// GenerateHugeFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GenerateHugeFile.h"
#include "GenerateHugeFileDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CGenerateHugeFileDlg dialog

UINT CGenerateHugeFileDlg::s_nCommMsg = 0;

CGenerateHugeFileDlg::CGenerateHugeFileDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_GENERATEHUGEFILE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_pGenerator = NULL;
    m_hInputFile = INVALID_HANDLE_VALUE;
    m_hOutputFile = INVALID_HANDLE_VALUE;
    m_hStopEvent = NULL;
}

void CGenerateHugeFileDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDT_INPUT_FILE, m_edtInputFile);
    DDX_Control(pDX, IDC_BTN_INPUT_FILE, m_btnInputFile);
    DDX_Control(pDX, IDC_EDT_OUTPUT_FILE, m_edtOutputFile);
    DDX_Control(pDX, IDC_BTN_OUTPUT_FILE, m_btnOutputFile);
    DDX_Control(pDX, IDC_CHB_LIMIT_OUT_PUT_FILE_SIZE, m_chbLimitOutputSize);
    DDX_Control(pDX, IDC_CHB_APPEND, m_chbAppend);
    DDX_Control(pDX, IDC_EDT_LIMIT_SIZE, m_edtLimitSize);
    DDX_Control(pDX, IDC_BTN_GENERATE, m_btnGenerate);
    DDX_Control(pDX, IDC_CBB_SIZE_UNIT, m_cbbSizeUnit);
    DDX_Control(pDX, IDC_BTN_STOP, m_btnStop);
    DDX_Control(pDX, IDC_PRGB_GENERATING, m_prgbGenerating);
    DDX_Control(pDX, IDC_CHB_REPEAT_READ, m_chbRepeatReadInputFile);
}

BEGIN_MESSAGE_MAP(CGenerateHugeFileDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_CLOSE()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_INPUT_FILE, &CGenerateHugeFileDlg::OnBnClickedBtnInputFile)
    ON_BN_CLICKED(IDC_BTN_OUTPUT_FILE, &CGenerateHugeFileDlg::OnBnClickedBtnOutputFile)
    ON_BN_CLICKED(IDC_CHB_LIMIT_OUT_PUT_FILE_SIZE, &CGenerateHugeFileDlg::OnBnClickedChbLimitOutPutFileSize)
    ON_BN_CLICKED(IDC_BTN_GENERATE, &CGenerateHugeFileDlg::OnBnClickedBtnGenerate)
    ON_BN_CLICKED(IDC_BTN_STOP, &CGenerateHugeFileDlg::OnBnClickedBtnStop)
    ON_EN_CHANGE(IDC_EDT_INPUT_FILE, &CGenerateHugeFileDlg::OnEnChangeEdtInputFile)
    ON_EN_CHANGE(IDC_EDT_OUTPUT_FILE, &CGenerateHugeFileDlg::OnEnChangeEdtOutputFile)
    ON_REGISTERED_MESSAGE(s_nCommMsg, &CGenerateHugeFileDlg::OnGeneratorThreadMessage)
END_MESSAGE_MAP()


// CGenerateHugeFileDlg message handlers

BOOL CGenerateHugeFileDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    m_chbAppend.SetCheck(BST_CHECKED);
    m_mapUnitBases.insert(std::make_pair(_T("Byte(s)"), 1));
    m_mapUnitBases.insert(std::make_pair(_T("KB"), 1024));
    m_mapUnitBases.insert(std::make_pair(_T("MB"), 1024 * 1024));
    m_mapUnitBases.insert(std::make_pair(_T("GB"), (__int64)1024 * (__int64)1024 * (__int64)1024));
    m_mapUnitBases.insert(std::make_pair(_T("TB"), (__int64)1024 * (__int64)1024 * (__int64)1024 * (__int64)1024));
    m_mapUnitBases.insert(std::make_pair(_T("PB"), (__int64)1024 * (__int64)1024 * (__int64)1024 * (__int64)1024 * (__int64)1024));

    std::map<CString, __int64>::const_iterator cIter = m_mapUnitBases.cbegin();
    CString strUnit;
    __int64 nVal = 0;
    for(; cIter != m_mapUnitBases.end();++cIter)
    {
        strUnit = cIter->first;
        nVal = cIter->second;

        m_cbbSizeUnit.AddString(strUnit);
    }
    m_cbbSizeUnit.SetCurSel(0);

    if (0 == s_nCommMsg) {
        CString strMsgID;
        strMsgID.Format(_T("%s_%ld_%ld"), AfxGetAppName(), reinterpret_cast<int>(AfxGetInstanceHandle()), GetCurrentProcessId() );
        s_nCommMsg = RegisterWindowMessage(strMsgID);
    }

    m_hStopEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

    m_prgbGenerating.ModifyStyle(0, PBS_MARQUEE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}


LRESULT CGenerateHugeFileDlg::OnGeneratorThreadMessage(WPARAM wParam, LPARAM lParam)
{
    LRESULT res = 0L;

    if (WM_GENERATOR_PRERUN == wParam) {
        m_btnGenerate.EnableWindow(FALSE);
        m_btnInputFile.EnableWindow(FALSE);
        m_btnOutputFile.EnableWindow(FALSE);
        m_edtInputFile.EnableWindow(FALSE);
        m_edtOutputFile.EnableWindow(FALSE);
        m_btnStop.EnableWindow(TRUE);
        m_prgbGenerating.ShowWindow(SW_SHOW);
        m_prgbGenerating.SetMarquee(TRUE, 30);
    }

    else if (WM_GENERATOR_POSTRUN == wParam) {
        int _nStatus = (int)lParam;

        m_btnGenerate.EnableWindow((m_edtInputFile.GetWindowTextLength() > 0) && (m_edtOutputFile.GetWindowTextLength() > 0));
        m_btnInputFile.EnableWindow(TRUE);
        m_btnOutputFile.EnableWindow(TRUE);
        m_edtInputFile.EnableWindow(TRUE);
        m_edtOutputFile.EnableWindow(TRUE);
        m_btnStop.EnableWindow(FALSE);
        m_prgbGenerating.ShowWindow(SW_HIDE);

        if (INVALID_HANDLE_VALUE != m_hInputFile) {
            CloseHandle(m_hInputFile);
            m_hInputFile = INVALID_HANDLE_VALUE;
        }

        if (INVALID_HANDLE_VALUE != m_hOutputFile) {
            CloseHandle(m_hOutputFile);
            m_hOutputFile = INVALID_HANDLE_VALUE;
        }

        if (NULL != m_pGenerator) {
            delete m_pGenerator;
            m_pGenerator = NULL;
        }

        if (ERROR_ABORTED == _nStatus) {
            AfxMessageBox(_T("Operation aborted."), MB_ICONINFORMATION);
        }
        else if(ERROR_SUCCESS == _nStatus) {
            AfxMessageBox(_T("Operation completed."), MB_ICONINFORMATION);
        }
        else if ((ERROR_READ == _nStatus) || (ERROR_WRITE == _nStatus)) {
            AfxMessageBox(_T("An error occurred during read/write the files."), MB_ICONEXCLAMATION);
        }
    }

    return res;
}

void CGenerateHugeFileDlg::OnCancel()
{
    if (NULL != m_pGenerator) {
        m_pGenerator->SuspendThread();
    }

    if (GeneratorThreadAlive()) {

        int nConfirm = AfxMessageBox(_T("Are you sure to abort ?"), MB_ICONQUESTION | MB_YESNO);

        if (nConfirm != IDYES) {
            if (NULL != m_pGenerator) {
                m_pGenerator->ResumeThread();
            }
            return;
        }
    }

    if (NULL != m_pGenerator) {
        m_pGenerator->ResumeThread();
    }
    if (NULL != m_hStopEvent) {
        SetEvent(m_hStopEvent);
    }

    if (NULL != m_hStopEvent) {
        ResetEvent(m_hStopEvent);
        CloseHandle(m_hStopEvent);
    }

    CDialogEx::OnCancel();
}

BOOL CGenerateHugeFileDlg::GeneratorThreadAlive()
{
    if (NULL != m_pGenerator) {
        DWORD dwCode = 0;
        if (GetExitCodeThread(m_pGenerator->m_hThread, &dwCode)) {
            if (dwCode == STILL_ACTIVE) {
                return TRUE;
            }
            else {
                return FALSE;
            }
        }
        else {
            return FALSE;
        }
    }
    else {
        return FALSE;
    }
}

BOOL CGenerateHugeFileDlg::ValidateSettings(HANDLE& hInputFile, HANDLE& hOutputFile, BOOL& bAppend, __int64& nLimitSize)
{
    hInputFile = INVALID_HANDLE_VALUE;
    hOutputFile = INVALID_HANDLE_VALUE;
    bAppend = FALSE;
    nLimitSize = 0;

    CString _strInputFile;
    CString _strOutputFile;
    HANDLE _hInputFile = INVALID_HANDLE_VALUE;
    HANDLE _hOutputFile = INVALID_HANDLE_VALUE;
    BOOL _bAppend = FALSE;
    __int64 _nLimitSize = 0;
    CString _strErrorMsg;

    m_edtInputFile.GetWindowText(_strInputFile);
    m_edtOutputFile.GetWindowText(_strOutputFile);

    if (_strInputFile.CompareNoCase(_strOutputFile) == 0) {
        AfxMessageBox(_T("Input file and output file must be seperated files."), MB_ICONEXCLAMATION);
        return FALSE;
    }

    CString _strFullInputPath;
    DWORD _dwChars = GetFullPathName(_strInputFile.GetBuffer(), MAX_PATH, _strFullInputPath.GetBuffer(MAX_PATH), NULL);
    _strFullInputPath.ReleaseBuffer();

    if (_dwChars <= 0) {
        AfxMessageBox(_T("Invalid input file."), MB_ICONEXCLAMATION);
        return FALSE;
    }

    CString _strFullOutputPath;
    _dwChars = GetFullPathName(_strOutputFile.GetBuffer(), MAX_PATH, _strFullOutputPath.GetBuffer(MAX_PATH), NULL);
    _strFullOutputPath.ReleaseBuffer();

    if (_dwChars <= 0) {
        AfxMessageBox(_T("Invalid output file."), MB_ICONEXCLAMATION);
        return FALSE;
    }

    if (m_chbLimitOutputSize.GetCheck() & BST_CHECKED) {
        CString _strUnit;
        int _nUnit = m_cbbSizeUnit.GetCurSel();
        if (_nUnit == CB_ERR) {
            AfxMessageBox(_T("Invalid size unit."), MB_OK | MB_ICONEXCLAMATION);
            m_cbbSizeUnit.SetFocus();
            return FALSE;
        }
        m_cbbSizeUnit.GetLBText(_nUnit, _strUnit);
        std::map<CString,__int64>::const_iterator cIter = m_mapUnitBases.find(_strUnit);
        if (m_mapUnitBases.end() != cIter) {
            __int64 _nUnitBase = cIter->second;
            CString _strLimitSize;
            m_edtLimitSize.GetWindowText(_strLimitSize);
            if (_strLimitSize.IsEmpty()) {
                AfxMessageBox(_T("Invalid limit size value."), MB_ICONEXCLAMATION);
                m_edtLimitSize.SetFocus();
                return FALSE;
            }
            char _pszLimitSize[255] = { 0 };
            int _nChars = WideCharToMultiByte(CP_ACP, 0, _strLimitSize.GetBuffer(), _strLimitSize.GetLength(), _pszLimitSize, 255, NULL, NULL);
            if (_nChars > 0) {
                _pszLimitSize[_nChars] = 0;
                __int64 _nInputSize = atoll(_pszLimitSize);
                _nLimitSize = _nInputSize * _nUnitBase;
            }
            else {
                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, _strErrorMsg.GetBuffer(255), 255, NULL);
                _strErrorMsg.ReleaseBuffer();
                AfxMessageBox(_strErrorMsg, MB_OK | MB_ICONEXCLAMATION);
                return FALSE;
            }
        }
        else {
            AfxMessageBox(_T("Internal error."), MB_OK | MB_ICONEXCLAMATION);
            return FALSE;
        }
    }

    bAppend = (m_chbAppend.GetCheck() & BST_CHECKED);

    _hInputFile = CreateFile(_strFullInputPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    DWORD _dwError = GetLastError();
    if (INVALID_HANDLE_VALUE == _hInputFile) {
        _strErrorMsg = _T("");
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, _dwError, 0, _strErrorMsg.GetBuffer(255), 255, NULL);
        _strErrorMsg.ReleaseBuffer();
        MessageBox(_strErrorMsg, _T("Input File Error"), MB_ICONEXCLAMATION);
        m_edtInputFile.SetFocus();
        return FALSE;
    }

    _hOutputFile = CreateFile(_strFullOutputPath, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);
    _dwError = GetLastError();
    if (INVALID_HANDLE_VALUE == _hOutputFile) {
        _strErrorMsg = _T("");
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, _dwError, 0, _strErrorMsg.GetBuffer(255), 255, NULL);
        _strErrorMsg.ReleaseBuffer();
        MessageBox(_strErrorMsg, _T("Output File Error"), MB_ICONEXCLAMATION);

        //Close input file
        CloseHandle(_hInputFile);
        _hInputFile = INVALID_HANDLE_VALUE;

        return FALSE;
    }

    hInputFile = _hInputFile;
    hOutputFile = _hOutputFile;
    bAppend = _bAppend;
    nLimitSize = _nLimitSize;

    return TRUE;
}

void CGenerateHugeFileDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}


void CGenerateHugeFileDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CGenerateHugeFileDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CGenerateHugeFileDlg::OnBnClickedBtnInputFile()
{
    CFileDialog dlg(TRUE, NULL, NULL, OFN_EXPLORER | OFN_FILEMUSTEXIST, _T("All files|*.*||"));
    if (IDOK != dlg.DoModal()) {
        return;
    }
    
    m_edtInputFile.SetWindowText(dlg.GetPathName());
}


void CGenerateHugeFileDlg::OnBnClickedBtnOutputFile()
{
    CFileDialog dlg(FALSE, NULL, NULL, OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, _T("All files|*.*||"));
    if (IDOK != dlg.DoModal()) {
        return;
    }

    m_edtOutputFile.SetWindowText(dlg.GetPathName());
}


void CGenerateHugeFileDlg::OnBnClickedChbLimitOutPutFileSize()
{
    BOOL bChecked = (BOOL)(m_chbLimitOutputSize.GetCheck() | BST_CHECKED);
    m_edtLimitSize.EnableWindow(bChecked);
    m_cbbSizeUnit.EnableWindow(bChecked);
}


void CGenerateHugeFileDlg::OnBnClickedBtnGenerate()
{
    HANDLE  hInputFile = INVALID_HANDLE_VALUE;
    HANDLE  hOutputFile = INVALID_HANDLE_VALUE;
    BOOL    bAppend = FALSE;
    __int64 nLimitSize = 0;
    BOOL    bRepeatRead = (BOOL)(m_chbRepeatReadInputFile.GetCheck() & BST_CHECKED);

    if (!ValidateSettings(hInputFile, hOutputFile, bAppend, nLimitSize)) {
        return;
    }

    if (NULL == m_pGenerator) {
        m_pGenerator = new GeneratorThread(hInputFile, hOutputFile, bAppend, nLimitSize, bRepeatRead, m_hStopEvent, this, s_nCommMsg);
    }

    if (! m_pGenerator->CreateThread(CREATE_SUSPENDED)) {
        MessageBox(_T("Unable to create generator thread"), _T("Internal Error"), MB_ICONEXCLAMATION);
        CloseHandle(hInputFile);
        CloseHandle(hOutputFile);
        return;
    }

    if (NULL != m_hStopEvent) {
        ResetEvent(m_hStopEvent);
    }

    m_pGenerator->ResumeThread();

    m_hInputFile = hInputFile;
    m_hOutputFile = hOutputFile;
}


void CGenerateHugeFileDlg::OnBnClickedBtnStop()
{
    if (NULL != m_pGenerator) {
        m_pGenerator->SuspendThread();
    }

    if (GeneratorThreadAlive()) {

        int nConfirm = AfxMessageBox(_T("Are you sure to abort ?"), MB_ICONQUESTION | MB_YESNO);

        if (nConfirm != IDYES) {
            if (NULL != m_pGenerator) {
                m_pGenerator->ResumeThread();
            }
            return;
        }
    }

    if (NULL != m_pGenerator) {
        m_pGenerator->ResumeThread();
    }
    if (NULL != m_hStopEvent) {
        SetEvent(m_hStopEvent);
    }
}


void CGenerateHugeFileDlg::OnEnChangeEdtInputFile()
{
    if (!GeneratorThreadAlive()) {
        m_btnGenerate.EnableWindow((m_edtInputFile.GetWindowTextLength() > 0) && (m_edtOutputFile.GetWindowTextLength() > 0));
    }
}


void CGenerateHugeFileDlg::OnEnChangeEdtOutputFile()
{
    if (!GeneratorThreadAlive()) {
        m_btnGenerate.EnableWindow((m_edtInputFile.GetWindowTextLength() > 0) && (m_edtOutputFile.GetWindowTextLength() > 0));
    }
}
