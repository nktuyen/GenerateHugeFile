
// GenerateHugeFileDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <map>
#include "GeneratorThread.h"
#include "afxcmn.h"

// CGenerateHugeFileDlg dialog
class CGenerateHugeFileDlg : public CDialogEx
{
// Construction
public:
	CGenerateHugeFileDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GENERATEHUGEFILE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
private:
	virtual BOOL OnInitDialog();
    afx_msg void OnCancel();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedBtnInputFile();
    afx_msg void OnBnClickedBtnOutputFile();
    afx_msg void OnBnClickedChbLimitOutPutFileSize();
    afx_msg void OnBnClickedBtnGenerate();
    afx_msg void OnBnClickedBtnStop();
    afx_msg void OnEnChangeEdtInputFile();
    afx_msg void OnEnChangeEdtOutputFile();
    BOOL GeneratorThreadAlive();
    BOOL ValidateSettings(HANDLE& hInputFile, HANDLE& hOutputFile, BOOL& bAppend, __int64& nLimitSize);
    afx_msg LRESULT OnGeneratorThreadMessage(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
private:
    HICON m_hIcon;
    GeneratorThread* m_pGenerator;
    CEdit m_edtInputFile;
    CButton m_btnInputFile;
    CEdit m_edtOutputFile;
    CButton m_btnOutputFile;
    CButton m_chbLimitOutputSize;
    CButton m_chbAppend;
    CEdit m_edtLimitSize;
    CButton m_btnGenerate;
    std::map<CString, __int64> m_mapUnitBases;
    CComboBox m_cbbSizeUnit;
    static UINT s_nCommMsg;
    CButton m_btnStop;
    HANDLE m_hInputFile;
    HANDLE m_hOutputFile;
    HANDLE m_hStopEvent;
    CProgressCtrl m_prgbGenerating;
    CButton m_chbRepeatReadInputFile;
};
