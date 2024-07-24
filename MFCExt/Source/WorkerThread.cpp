/********************************************************************
*
* This file is part of the MFCExt-Library
*
* Copyright (C) 1999-2000 Sven Wiegand
* Copyright (C) 2000-$CurrentYear$ ToolsCenter
* 
* This library is free software; you can redistribute it and/or
* modify, but leave the headers intact and do not remove any 
* copyrights from the source.
*
* This library does not only contain file from us, but also from
* third party developers. Look at the source file headers to get
* detailed information.
*
* If you have further questions visit our homepage
*
*    http://www.ToolsCenter.org
*
********************************************************************/

/********************************************************************
*
* $Id$
*
********************************************************************/

#include "stdafx.h"
#include "WorkerThread.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static UINT ThreadWrapper(LPVOID lpParam)
{
	ASSERT(lpParam);
	if (!lpParam)
		return -1;

	UINT	unResult = ((CWorkerThread*)lpParam)->Run();
	return ((CWorkerThread*)lpParam)->OnInternalTerminate(unResult);
}


//-------------------------------------------------------------------
// class CWorkerThread
//-------------------------------------------------------------------

CWorkerThread::CWorkerThread(BOOL bAutoDelete)
:	m_bAutoDelete(bAutoDelete),
	m_bAutoCloseHandle(TRUE),
	m_hThread(INVALID_HANDLE_VALUE),
	m_dwThreadId(0xFFFFFFFF)
{}


CWorkerThread::~CWorkerThread()
{}


BOOL CWorkerThread::Create(BOOL bAutoCloseHandle,
													 int nPriority, 
													 UINT nStackSize, DWORD dwCreationFlags, 
													 LPSECURITY_ATTRIBUTES lpSecurityAttrs)
{
	// test if thread is already running
	if (WaitForThread(0) == WAIT_TIMEOUT)
	{
		// you can only create a thread, if it is not running already!
		ASSERT(FALSE);
		return FALSE;
	}

	// create thread
	m_bAutoCloseHandle = bAutoCloseHandle;
	m_hThread = INVALID_HANDLE_VALUE;
	m_dwThreadId = 0xFFFFFFFF;

	if (!OnPreCreate())
		return FALSE;

	CWinThread	*pThread = AfxBeginThread(
		ThreadWrapper, (LPVOID)this, nPriority, nStackSize, 
		dwCreationFlags | CREATE_SUSPENDED , lpSecurityAttrs);
	if (!pThread)
		return FALSE;

	// copy attributes
	DuplicateHandle(
		GetCurrentProcess(), pThread->m_hThread, 
		GetCurrentProcess(), &m_hThread, 
		STANDARD_RIGHTS_REQUIRED|THREAD_ALL_ACCESS, 
		FALSE, 0);
	m_dwThreadId = pThread->m_nThreadID;

	if (!(dwCreationFlags & CREATE_SUSPENDED))
		Resume();

	return TRUE;
}


void CWorkerThread::CloseHandle()
{
	ASSERT(m_hThread != INVALID_HANDLE_VALUE);
	if (m_hThread == INVALID_HANDLE_VALUE)
		return;

	::CloseHandle(m_hThread);
	m_hThread = INVALID_HANDLE_VALUE;
}


BOOL CWorkerThread::IsStillRunning()
{
	if (WaitForThread(0) == WAIT_TIMEOUT)
		return TRUE;
	else
		return FALSE;
}


DWORD CWorkerThread::WaitForThread(DWORD dwTimeout)
{
	if (m_hThread == INVALID_HANDLE_VALUE)
		return WAIT_FAILED;

	return WaitForSingleObject(m_hThread, dwTimeout);
}


BOOL CWorkerThread::GetExitCode(LPDWORD lpdwExitCode)
{
	if (m_hThread == INVALID_HANDLE_VALUE)
		return FALSE;

	return GetExitCodeThread(m_hThread, lpdwExitCode);
}


DWORD CWorkerThread::Suspend()
{
	if (m_hThread == INVALID_HANDLE_VALUE)
		return 0xFFFFFFFF;

	return SuspendThread(m_hThread);
}


DWORD CWorkerThread::Resume()
{
	if (m_hThread == INVALID_HANDLE_VALUE)
		return 0xFFFFFFFF;

	return ResumeThread(m_hThread);
}


BOOL CWorkerThread::SetPriority(int nPriority)
{
	if (m_hThread == INVALID_HANDLE_VALUE)
		return FALSE;

	return SetThreadPriority(m_hThread, nPriority);
}


int CWorkerThread::GetPriority()
{
	if (m_hThread == INVALID_HANDLE_VALUE)
		return THREAD_PRIORITY_ERROR_RETURN;

	return GetThreadPriority(m_hThread);
}


BOOL CWorkerThread::Terminate(DWORD dwExitCode)
{
	if (m_hThread == INVALID_HANDLE_VALUE)
		return FALSE;

	return ::TerminateThread(m_hThread, dwExitCode);
}


BOOL CWorkerThread::OnPreCreate()
{
	return TRUE;
}


UINT CWorkerThread::OnTerminate(UINT unExitCode)
{
	MsgsAfterTermination.SendCallbacks(unExitCode);

	return unExitCode;
}


UINT CWorkerThread::OnInternalTerminate(UINT unExitCode)
{
	UINT	unReturn = OnTerminate(unExitCode);

	if (m_bAutoCloseHandle)
		CloseHandle();
	if (m_bAutoDelete)
		delete this;

	return unReturn;
}
