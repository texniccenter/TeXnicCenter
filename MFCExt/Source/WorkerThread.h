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

#if !defined(AFX_WORKERTHREAD_H__B72F25C4_205F_11D5_A222_006097239934__INCLUDED_)
#define AFX_WORKERTHREAD_H__B72F25C4_205F_11D5_A222_006097239934__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/** Class creating worker threads without GUI activity.

	Derive your own class and override the virtual Run()-method, which
	will be called when the thread is started.


@ingroup mfcext

@author Sven Wiegand
*/
class  CWorkerThread
{
//Types
public:
	///Simple structure for defining a message.
	struct CallbackMessage
	{
		CallbackMessage()
			:hWnd(NULL)
			,message(0)
			,wParam(0)
			,lParam(0)
			,bPostMessage(false)
			,bAutoReset(false)
			,bCheckCode(false)
			,nCheckCode(0)
		{}

		virtual ~CallbackMessage(){}

		///Handle to window, which shall recieve the message.
		HWND hWnd;
		///Message ID
		UINT message;
		///WParam of the message
		WPARAM wParam;
		///LParam of the message
		LPARAM lParam;

		///Whether the message shall be posted or sent
		bool bPostMessage;
		///Auto-reset after sending the message, i.e., invalidate the callback
		bool bAutoReset;
		///Whether the given code should be checked
		bool bCheckCode;
		///Desired Code
		int nCheckCode;


		void Set(bool bPost, HWND arghWnd, UINT argmessage, WPARAM argwParam = 0, LPARAM arglParam = 0,
					bool argbAutoReset = false, bool argbCheckCode = false, int argnCheckCode = 0)
		{
			bPostMessage = bPost;
			hWnd = arghWnd;
			message = argmessage;
			wParam = argwParam;
			lParam = arglParam;
			bAutoReset = argbAutoReset;
			bCheckCode = argbCheckCode;
			nCheckCode = argnCheckCode;
		}

		void UnSet()
		{
			hWnd = NULL;
		}

		void SendCallback(int argCode)
		{
			//Are we valid? Is the callbeck set?
			if (!hWnd) return;
			if (!::IsWindow(hWnd)) return;

			//Correct code?
			if (!bCheckCode || nCheckCode == argCode)
			{
				if (bPostMessage)
				{
					::PostMessage(hWnd, message, wParam, lParam);
				}
				else
				{
					::SendMessage(hWnd, message, wParam, lParam);
				}
			}

			//Reset?
			if (bAutoReset) UnSet();
		}
	};

	///Container for CallbackMessage
	class CallbackMessages : protected CArray<CallbackMessage>
	{
	public:
		CallbackMessages() {}
		virtual ~CallbackMessages() {}

	public:
		void AddMessage(bool bPost, HWND arghWnd, UINT argmessage, WPARAM argwParam = 0, LPARAM arglParam = 0,
						bool argbCheckCode = false, int argnCheckCode = 0)
		{
			CallbackMessage Msg;
			Msg.Set(bPost, arghWnd, argmessage, argwParam, arglParam, true, argbCheckCode, argnCheckCode);
			Add(Msg);
		}

		void ClearAllMessages()
		{
			RemoveAll();
		}

		void SendCallbacks(int argCode)
		{
			for(int i=0;i<=GetUpperBound();i++)
			{
				GetAt(i).SendCallback(argCode);
			}

			ClearAllMessages();
		}
	};

// construction/destruction
public:
	/**
	Constructs a worker thread object.

	@param bAutoDelete
		If set to TRUE the object will delete itself, when the
		Run()-method terminates. Only set this to TRUE if the object has
		been created on the heap.
	*/
	CWorkerThread(BOOL bAutoDelete = FALSE);
	virtual ~CWorkerThread();

// operations
public:
	/**
	Creates the thread -- makes it run.

	If you call Create() a second time for an object, you have to
	ensure, that the previous thread's handle has been closed, by
	either calling Create() with bAutoCloseHandle set to TRUE or
	calling CloseHandle().

	See MSDN-description of AfxBeginThread() for detailed parameter
	description.

	@param bAutoCloseHandle
		Set this value to TRUE, if you would like the threads handle
		be released automatically, when the Run()-method terminates. If
		FALSE is specified here, you have to close the handle manually
		calling CloseHandle().

	@return
		TRUE on success, FALSE if the creation of the thread failed.
	*/
	BOOL Create(
		BOOL bAutoCloseHandle = TRUE,
		int nPriority = THREAD_PRIORITY_NORMAL,
		UINT nStackSize = 0, DWORD dwCreationFlags = 0,
		LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

	/**
	You have to call this method if you do not need the thread's handle
	anymore.
	*/
	void CloseHandle();

	/**
	Returns TRUE, if the thread is still running and FALSE otherwise.
	*/
	BOOL IsStillRunning();

	/**
	Waits until this thread terminates or the specified time expired.

	Will only work, when m_hThread is valid.

	@param dwTimeout
		Timeout in milliseconds of INFINITE to wait forever.

	@return
		WAIT_OBJECT_0 if the thread terminated, WAIT_TIMEOUT if the
		specified timeout-interval elapsed or WAIT_FAILED if an error
		occured.
	*/
	DWORD WaitForThread(DWORD dwTimeout = INFINITE);

	/**
	Retrieves the termination status of the thread.

	Will only work, when m_hThread is valid.

	See documentation of ::GetExitCodeThread for detailed information.

	@return
		TRUE on success, FALSE otherwise.
	*/
	BOOL GetExitCode(LPDWORD lpdwExitCode);

	/**
	Suspends the thread.

	Will only work, when m_hThread is valid.

	See documentation of ::SuspendThread for detailed information.
	*/
	DWORD Suspend();

	/**
	Decrements the thread's suspend counter and resumes it, if the
	counter value becomes zero.

	Will only work, when m_hThread is valid.

	See documentation of ::ResumeThread for detailed information.
	*/
	DWORD Resume();

	/**
	Sets the thread's priority.

	Will only work, when m_hThread is valid.

	See documentation of ::SetPriority for detailed information.
	*/
	BOOL SetPriority(int nPriority);

	/**
	Returns the thread's priority.

	Will only work, when m_hThread is valid.

	See documentation of ::GetThreadPriority() for detailed
	information.
	*/
	int GetPriority();

	/**
	Call to force the thread to terminate. DANGEROUS!

	This method should only be used, when really necessary.

	Will only work, when m_hThread is valid.

	See documentation of ::TerminateThread() for detailed information.
	*/
	BOOL Terminate(DWORD dwExitCode);

// overridables
protected:
	/**
	This virtual method is called before the thread is created, from
	within the thread, that called the Create()-method.

	This base implementation just returns TRUE.

	@return
		Return TRUE to allow the thread to be started and FALSE
		otherwise.
	*/
	virtual BOOL OnPreCreate();

	/**
	This virtual method is the thread method.

	The return value is the threads return value.
	*/
	virtual UINT Run() = 0;

	/**
	This virtual method is called, when the Run()-method has
	terminated.

	This base implementation simply calls the @c MsgAfterTermination
	message callback and returns the unExitCode parameter.

	@param unExitCode
		Value returned by the Run()-method

	@return
		Return the threads return value here. In most case you will
		simply return the unExitCode parameter.
	*/
	virtual UINT OnTerminate(UINT unExitCode);

// implementation helpers
private:
	/**
	Called when the Run()-method terminates.

	@param unExitCode
		Value returned by the Run()-method.

	@return
		Value that should be returned by the thread.
	*/
	UINT OnInternalTerminate(UINT unExitCode);

// attributes
public:
	/**
	Handle to the current thread.

	If you sepcified bAutoCloseHandle with the Create()-method, this
	handle will only be valid for the time, the thread is running.
	*/
	HANDLE m_hThread;

	/** Thread ID */
	DWORD m_dwThreadId;

	///Definition of Callbacks after Termination
	CallbackMessages MsgsAfterTermination;

private:
	BOOL m_bAutoDelete;
	BOOL m_bAutoCloseHandle;

// friends
private:
	friend UINT ThreadWrapper(LPVOID lpParam);
};

#endif // !defined(AFX_WORKERTHREAD_H__B72F25C4_205F_11D5_A222_006097239934__INCLUDED_)
