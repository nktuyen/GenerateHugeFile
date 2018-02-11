// GeneratorThread.cpp : implementation file
//

#include "stdafx.h"
#include "GenerateHugeFile.h"
#include "GeneratorThread.h"


GeneratorThread::GeneratorThread(HANDLE hInputFile, HANDLE hOutputFile, BOOL bAppend /* = FALSE */, __int64 nLimitSize /* = 0 */, BOOL bRepeatRead /* = FALSE */, HANDLE hStopEvent /* = NULL */, CWnd* pOwnerWnd /* = NULL */, UINT nMsgID /* = 0U */)
{
    m_bAutoDelete = FALSE;
    m_hInputFile = hInputFile;
    m_hOutputFile = hOutputFile;
    m_bAppend = bAppend;
    m_nLimitSize = nLimitSize;
    m_bRepeatRead = bRepeatRead;
    m_hStopEvent = hStopEvent;
    m_nError = ERROR_SUCCESS;
    m_nBufferSize = 1048576;
    m_pWnd = pOwnerWnd;
    m_nCommMsg = nMsgID;
}

GeneratorThread::~GeneratorThread()
{
}

BOOL GeneratorThread::InitInstance()
{
    CWinThread::InitInstance();

    return TRUE;
}

int GeneratorThread::ExitInstance()
{
    CWinThread::ExitInstance();

    return m_nError;
}

BOOL GeneratorThread::PreRun()
{
    if (NULL != m_pWnd) {
        if (NULL != m_pWnd->GetSafeHwnd()) {
            m_pWnd->PostMessage(m_nCommMsg, WM_GENERATOR_PRERUN, 0);
        }
    }
    return TRUE;
}

int GeneratorThread::Run()
{
    if (!PreRun()) {
        return m_nError;
    }

    LARGE_INTEGER   _li;
    _li.QuadPart = 0;

    __int64 _nOriginSize = 0;
    __int64 _nWritenBytes = 0;
    if (GetFileSizeEx(m_hOutputFile, &_li)) {
        _nOriginSize = _li.QuadPart;
    }

    BYTE*   _pBuffer = new BYTE[m_nBufferSize];
    DWORD   _dwReadBytes = 0;
    DWORD   _dwWritenBytes = 0;
    int    _nStatus = ERROR_SUCCESS;

    SetFilePointer(m_hOutputFile, 0, NULL, FILE_END);

    while ( (0 == m_nLimitSize) || ((_nOriginSize + _nWritenBytes) < m_nLimitSize) )
    {
        if (WAIT_OBJECT_0 == WaitForSingleObject(m_hStopEvent, 0)) {
            _nStatus = ERROR_ABORTED;
            break;
        }

        ZeroMemory(_pBuffer, 255);
        _dwReadBytes = 0;
        if (!ReadFile(m_hInputFile, _pBuffer, m_nBufferSize, &_dwReadBytes, NULL)) {
            _nStatus = ERROR_READ;
            break;
        }

        if (0 == _dwReadBytes) {
            DWORD dwCode = GetLastError();
            if (ERROR_SUCCESS == dwCode) {  //EOF
                if (m_bRepeatRead) {
                    SetFilePointer(m_hInputFile, 0, NULL, FILE_BEGIN);
                    continue;
                }
                else {
                    _nStatus = ERROR_SUCCESS;
                    break;
                }
            }
            else {
                TCHAR szMsg[255] = { 0 };
                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwCode, 0, szMsg, 255, NULL);
                _nStatus = ERROR_READ;  //An error occurred
                break;
            }
        }

        if (WAIT_OBJECT_0 == WaitForSingleObject(m_hStopEvent, 0)) {
            _nStatus = ERROR_ABORTED;
            break;
        }

        _dwWritenBytes = 0;
        if (!WriteFile(m_hOutputFile, _pBuffer, _dwReadBytes, &_dwWritenBytes, NULL)) {
            _nStatus = ERROR_WRITE;
            break;
        }

        if (0 == _dwWritenBytes) {
            _nStatus = ERROR_WRITE;
            break;
        }

        if (_dwReadBytes != _dwWritenBytes) {
            break;
        }

        _nWritenBytes += (__int64)_dwWritenBytes;
    }
    delete[] _pBuffer;

    PostRun(_nStatus);
    return m_nError;
}

void GeneratorThread::PostRun(int nStatus)
{
    if (NULL != m_pWnd) {
        if (NULL != m_pWnd->GetSafeHwnd()) {
            m_pWnd->PostMessage(m_nCommMsg, WM_GENERATOR_POSTRUN, (LPARAM)nStatus);
        }
    }
}