#pragma once

#define WM_GENERATOR_PRERUN (WM_USER+1)
#define WM_GENERATOR_POSTRUN (WM_USER+2)
#define ERROR_ABORTED   (ERROR_SUCCESS+1)
#define ERROR_READ      (ERROR_SUCCESS+2)
#define ERROR_WRITE     (ERROR_SUCCESS+3)

class GeneratorThread : public CWinThread
{
public:
    GeneratorThread(HANDLE hInputFile, HANDLE hOutputFile, BOOL bAppend = FALSE, __int64 nLimitSize = 0, BOOL bRepeatRead = FALSE, HANDLE hStopEvent = NULL, CWnd* pOwnerWnd = NULL, UINT nMsgID = 0U);
	virtual ~GeneratorThread();
protected:
    BOOL InitInstance();
    BOOL PreRun();
    int Run();
    void PostRun(int nStatus);
    int ExitInstance();
private:
    HANDLE  m_hInputFile;
    HANDLE  m_hOutputFile;
    BOOL    m_bAppend;
    __int64 m_nLimitSize;
    BOOL    m_bRepeatRead;
    UINT    m_nCommMsg;
    int     m_nError;
    DWORD   m_nBufferSize;
    HANDLE  m_hStopEvent;
    CWnd    *m_pWnd;
};


