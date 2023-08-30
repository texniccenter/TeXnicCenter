/********************************************************************
 *
 * This file is part of the TeXnicCenter-system
 *
 * Copyright (C) 1999-2000 Sven Wiegand
 * Copyright (C) 2000-$CurrentYear$ ToolsCenter
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * If you have further questions or if you want to support
 * further TeXnicCenter development, visit the TeXnicCenter-homepage
 *
 *    http://www.ToolsCenter.org
 *
 *********************************************************************/

/********************************************************************
 *
 * $Id$
 *
 ********************************************************************/

#pragma once

#include "profile.h"

class COutputDoc;
class COutputView;

/** This class builds the output.

@author Sven Wiegand
 */
class COutputBuilder : public CWorkerThread
{
// construction/destruction
public:
	COutputBuilder();

// operations
public:
	/**
	Starts the build process. And returns immediately.

	@param pDoc
	        The document, errors and warnings should be reported to.
	@param pView
	        Pointer to the COutputView-derived view-object, to write the
	        output to.
	@param lpszWorkingDir
	        Directory to execute the processes in.
	@param lpszMainPath
	        Path of the project's main file.
	@param bRunBibTex
	        If FALSE the output builder won't start BibTeX, otherwise it
	        will start BibTeX, if specified in the active profile.
	@param bRunMakeIndex
	        If FALSE the output builder won't start MakeIndex, otherwise it
	        will start MakeIndex, if specified in the active profile.
	@param nPriority
	        See CWorkerThread::Create() for details.
	 */
	BOOL BuildAll(
	    COutputDoc *pDoc, COutputView *pView,
	    LPCTSTR lpszWorkingDir, LPCTSTR lpszMainPath,
	    BOOL bRunBibTex, BOOL bRunMakeIndex,
	    int nPriority = THREAD_PRIORITY_BELOW_NORMAL);

	/**
	Same as BuildAll, but executes only BibTeX.

	@see #BuildAll
	 */
	BOOL RunBibTex(
	    COutputDoc *pDoc, COutputView *pView,
	    LPCTSTR lpszWorkingDir, LPCTSTR lpszMainPath,
	    int nPriority = THREAD_PRIORITY_BELOW_NORMAL);

	/**
	Same as BuildAll, but executes only MakeIndex.

	@see #BuildAll
	 */
	BOOL RunMakeIndex(
	    COutputDoc *pDoc, COutputView *pView,
	    LPCTSTR lpszWorkingDir, LPCTSTR lpszMainPath,
	    int nPriority = THREAD_PRIORITY_BELOW_NORMAL);

	/** Executes the processors to build a preview.

		@note
		The preview is typically built in its own working directory
		and has its own main LaTeX file.

		@see #BuildAll for further details on the parameters.
	 */
	BOOL BuildPreview(
	    COutputDoc* pDoc, COutputView* pView,
	    LPCTSTR lpszWorkingDir, LPCTSTR lpszMainPath,
	    int nPriority = THREAD_PRIORITY_BELOW_NORMAL);

	/**
	Tries to cancel the build by sending a CTRL_BREAK_EVENT to the
	currently running process.
	 */
	BOOL CancelExecution();

	/**
	Returns whether the last build was canceled or not.
	 */
    bool WasCancelled() const;

// overridings
protected:
	virtual UINT Run();
	virtual UINT OnTerminate(UINT unExitCode);

// types
protected:

	enum tagMode
	{
		modeBuildAll,
		modeRunBibTexOnly,
		modeRunMakeIndexOnly,
		modeBuildPreview
	};

// implementation helpers
protected:
	BOOL Create(
	    int nMode,
	    COutputDoc *pDoc, COutputView *pView,
	    LPCTSTR lpszWorkingDir, LPCTSTR lpszMainPath,
	    BOOL bRunBibTex, BOOL bRunMakeIndex,
	    int nPriority = THREAD_PRIORITY_BELOW_NORMAL);

	/**
	Executes all the preprocessors of the current profile if necessary
	and returns, when all the preprocessors have been executed.

	m_pProfile has to be valid.
	 */
	BOOL RunPreProcessors();

	/**
	Executes the (La)TeX compiler if necessary and returns, when the
	compiler completed.

	m_pProfile has to be valid.
	 */
	BOOL RunLatex();

	/**
	Executes BibTeX if necessary and returns, when BibTeX completed.

	m_pProfile has to be valid.
	 */
	BOOL RunBibTex();

	/**
	Executes MakeIndex if necessary and returns, when MakeIndex
	completed.

	m_pProfile has to be valid.
	 */
	BOOL RunMakeIndex();

	/**
	Executes all the postprocessors of the current profile if necessary
	and returns, when all the postprocessors have been executed.

	m_pProfile has to be valid.
	 */
	BOOL RunPostProcessors();

	/** Executes all the preview processors of the current profile
		and returns, when they all have been executed.

		m_pProfile has to be valid.
	 */
	BOOL RunPreviewProcessors();

// attributes
protected:
	/** document to report errors and warnings to */
	COutputDoc* m_pDoc;

	/** view to write the output to */
	COutputView* m_pView;

	/** output profile to use */
	CProfile* m_pProfile;

	/** main-file path */
	CString m_strMainPath;

	/** current file path*/
	CString m_strWorkingDir;

	/** Should BibTeX be run? */
	BOOL m_bRunBibTex;

	/** Should MakeIndex be run? */
	BOOL m_bRunMakeIndex;

	/**
	Specifies, which commands to execute. one of the values of tagMode
	 */
	int m_nMode;

private:
	/** Process handle of the currently executed build process. */
	HANDLE m_hCurrentProcess;

	/** TRUE, if building should be canceled, FALSE otherwise. */
	bool m_bCancel;

	/** Result string returned by the latex output filter. */
	CString m_strLatexResult;
	
	CString bibtex_result_;

	CString current_process_name_;

	CString transcriptFileName_;

public:
	const CString& GetCurrentProcessName() const;
	const CString& GetTranscriptFileName() const;
};
