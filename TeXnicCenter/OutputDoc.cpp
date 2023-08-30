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

#include "stdafx.h"

#include <ddeml.h> // DDE support
#include <dde.h>

#include "resource.h"
#include "OutputDoc.h"
#include "MainFrm.h"
#include "LatexProject.h"
#include "Configuration.h"
#include "BuildView.h"
#include "global.h"
#include "ParseOutputView.h"
#include "ProjectPropertyDialog.h"
#include "Process.h"
#include "TextSourceFile.h"
#include "ItemPropertyDialog.h"
#include "FindInFilesDialog.h"
#include "Profile.h"
#include "OutputBuilder.h"
#include "FileClean.h"
#include "FileCleanConfirmDialog.h"
#include "ErrorListPane.h"
#include "LaTeXDocument.h"
#include "LaTeXView.h"
#include "TeXnicCenter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// standard beep
#define BEEP       //MessageBeep( MB_OK )

//-------------------------------------------------------------------
// class COutputDoc
//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(COutputDoc,CCmdTarget)

COutputDoc::COutputDoc()
		: m_pBuildView(NULL)
		,m_bCanGrep(TRUE)
		,m_bCanRunLatex(TRUE)
		,m_nActiveFileGrep(0)
		,m_pActiveOutputView(NULL)
		,m_pParseView(NULL)
		,m_bActiveFileOperation(FALSE)
		, errorListView_(0)
{
	// initialization
	m_apGrepView[0] = NULL;
	m_apGrepView[1] = NULL;

	m_apaGrepHits[0] = &m_aGrepHits1;
	m_apaGrepHits[1] = &m_aGrepHits2;
}

COutputDoc::~COutputDoc()
{
}

void COutputDoc::SetAllViews(COutputView* pBuildView,COutputView* pGrepView1,
                             COutputView* pGrepView2,COutputView* pParseView)
{
	ASSERT(pBuildView);
	ASSERT(pGrepView1);
	ASSERT(pGrepView2);
	ASSERT(pParseView);
	m_pParseView = pParseView;
	m_pBuildView = pBuildView;
	m_apGrepView[0] = pGrepView1;
	m_apGrepView[1] = pGrepView2;

	SetActiveView(m_pBuildView);
}

BEGIN_MESSAGE_MAP(COutputDoc,CCmdTarget)
	//{{AFX_MSG_MAP(COutputDoc)
	ON_COMMAND(ID_NEXT_ERROR,OnNextError)
	ON_COMMAND(ID_PREV_ERROR,OnPrevError)
	ON_COMMAND(ID_LATEX_VIEW,OnLatexView)
	ON_COMMAND(ID_NEXT_BADBOX,OnNextBadbox)
	ON_UPDATE_COMMAND_UI(ID_NEXT_BADBOX,OnUpdateNextPrevBadbox)
	ON_UPDATE_COMMAND_UI(ID_NEXT_ERROR,OnUpdateNextPrevError)
	ON_COMMAND(ID_NEXT_WARNING,OnNextWarning)
	ON_UPDATE_COMMAND_UI(ID_NEXT_WARNING,OnUpdateNextPrevWarning)
	ON_COMMAND(ID_PREV_BADBOX,OnPrevBadbox)
	ON_COMMAND(ID_PREV_WARNING,OnPrevWarning)
	ON_COMMAND(ID_EDIT_FIND_IN_FILES,OnEditFindInFiles)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_IN_FILES,OnUpdateEditFindInFiles)
	ON_COMMAND(ID_EDIT_NEXTGREPRESULT,OnEditNextGrepResult)
	ON_UPDATE_COMMAND_UI(ID_EDIT_NEXTGREPRESULT,OnUpdateGrepResultStep)
	ON_COMMAND(ID_EDIT_PREVGREPRESULT,OnEditPrevGrepResult)
	ON_COMMAND(ID_LATEX_STOP_BUILD,OnLatexStopBuild)
	ON_UPDATE_COMMAND_UI(ID_LATEX_STOP_BUILD,OnUpdateLatexStopBuild)
	ON_COMMAND(ID_LATEX_RUN,OnLatexRun)
	ON_UPDATE_COMMAND_UI(ID_LATEX_RUN,OnUpdateLatexRun)
	ON_COMMAND(ID_LATEX_BIBTEX,OnBibTex)
	ON_UPDATE_COMMAND_UI(ID_LATEX_BIBTEX,OnUpdateBibTex)
	ON_COMMAND(ID_LATEX_MAKEINDEX,OnMakeIndex)
	ON_UPDATE_COMMAND_UI(ID_LATEX_MAKEINDEX,OnUpdateMakeIndex)
	ON_COMMAND(ID_LATEX_FILECOMPILE,OnFileCompile)
	ON_UPDATE_COMMAND_UI(ID_LATEX_FILECOMPILE,OnUpdateFileCompile)
	ON_COMMAND(ID_LATEX_FILEBIBTEX,OnFileBibTex)
	ON_UPDATE_COMMAND_UI(ID_LATEX_FILEBIBTEX,OnUpdateFileBibTex)
	ON_COMMAND(ID_LATEX_FILEMAKEINDEX,OnFileMakeIndex)
	ON_UPDATE_COMMAND_UI(ID_LATEX_FILEMAKEINDEX,OnUpdateFileMakeIndex)
	ON_COMMAND(ID_LATEX_CLEAN,OnLatexClean)
	ON_UPDATE_COMMAND_UI(ID_LATEX_CLEAN,OnUpdateLatexClean)
	ON_COMMAND(ID_LATEX_RUNANDVIEW,OnLatexRunAndView)
	ON_COMMAND(ID_LATEX_FILECOMPILEANDVIEW,OnLatexFileCompileAndView)
	ON_UPDATE_COMMAND_UI(ID_PREV_BADBOX,OnUpdateNextPrevBadbox)
	ON_UPDATE_COMMAND_UI(ID_PREV_ERROR,OnUpdateNextPrevError)
	ON_UPDATE_COMMAND_UI(ID_PREV_WARNING,OnUpdateNextPrevWarning)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PREVGREPRESULT,OnUpdateGrepResultStep)
	ON_UPDATE_COMMAND_UI(ID_LATEX_RUNANDVIEW,OnUpdateLatexRun)
	ON_UPDATE_COMMAND_UI(ID_LATEX_FILECOMPILEANDVIEW,OnUpdateFileCompile)
	ON_UPDATE_COMMAND_UI(ID_LATEX_VIEW,OnUpdateLatexView)
	ON_COMMAND(ID_BUILD_BUILDPREVIEW, &COutputDoc::OnBuildPreview)
	ON_UPDATE_COMMAND_UI(ID_BUILD_BUILDPREVIEW, &COutputDoc::OnUpdateLatexRun)
	//}}AFX_MSG_MAP
	END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Diagnose COutputDoc

#ifdef _DEBUG
void COutputDoc::AssertValid() const
{
	CCmdTarget::AssertValid();
}

void COutputDoc::Dump(CDumpContext& dc) const
{
	CCmdTarget::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////
// handling output errors

void COutputDoc::ShowError(int nIndex)
{
	if (!(nIndex >= 0 && nIndex < m_aErrors.GetSize()))
		return;

	CMainFrame* f = static_cast<CMainFrame*>(AfxGetMainWnd());
	f->ActivateOutputTab(CMainFrame::outputTabBuildResult);

	TryOpenFile(GetFilePath(m_aErrors[nIndex].GetSourceFile()),m_aErrors[nIndex].GetSourceLine());

	const Nullable<int>& line = m_aErrors[nIndex].GetOutputLine();

	UpdateAllViews(NULL,hintSelectBuildLine,(CObject*)(&line));

	m_nActualErrorIndex = nIndex;
}

void COutputDoc::OnNextError()
{
	if (m_nActualErrorIndex >= m_aErrors.GetSize() - 1 ||
	        m_nActualErrorIndex < 0)
	{
		m_nActualErrorIndex = 0;
		//BEEP;
		ShowError(m_nActualErrorIndex);
	}
	else if (m_nActualErrorIndex < m_aErrors.GetSize() - 1)
		ShowError(++m_nActualErrorIndex);
	else
		m_nActualErrorIndex++;
}

void COutputDoc::OnPrevError()
{
	if (m_nActualErrorIndex > m_aErrors.GetSize() - 1 ||
	        m_nActualErrorIndex <= 0)
	{
		m_nActualErrorIndex = m_aErrors.GetSize() - 1;
		//BEEP;
		ShowError(m_nActualErrorIndex);
	}
	else if (m_nActualErrorIndex > 0)
		ShowError(--m_nActualErrorIndex);
	else
		--m_nActualErrorIndex;
}

void COutputDoc::OnUpdateNextPrevError(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasBuildErrors());
}

/////////////////////////////////////////////////////////////////////
// handling bad boxes

void COutputDoc::ShowBadBox(int nIndex)
{
	if (!(nIndex >= 0 && nIndex < m_aBadBoxes.GetSize()))
		return;

	CMainFrame* f = static_cast<CMainFrame*>(AfxGetMainWnd());
	f->ActivateOutputTab(CMainFrame::outputTabBuildResult);

	TryOpenFile(GetFilePath(m_aBadBoxes[nIndex].GetSourceFile()),m_aBadBoxes[nIndex].GetSourceLine());
	
	const Nullable<int>& line = m_aBadBoxes[nIndex].GetOutputLine();

	UpdateAllViews(NULL,hintSelectBuildLine,(CObject*) &line);
	m_nActualBadBoxIndex = nIndex;
}

void COutputDoc::OnNextBadbox()
{
	if (m_nActualBadBoxIndex >= m_aBadBoxes.GetSize() - 1 ||
	        m_nActualBadBoxIndex < 0)
	{
		m_nActualBadBoxIndex = 0;
		//BEEP;
		ShowBadBox(m_nActualBadBoxIndex);
	}
	else if (m_nActualBadBoxIndex < m_aBadBoxes.GetSize() - 1)
		ShowBadBox(++m_nActualBadBoxIndex);
	else
		m_nActualBadBoxIndex++;
}

void COutputDoc::OnPrevBadbox()
{
	if (m_nActualBadBoxIndex > m_aBadBoxes.GetSize() - 1 ||
	        m_nActualBadBoxIndex <= 0)
	{
		m_nActualBadBoxIndex = m_aBadBoxes.GetSize() - 1;
		//BEEP;
		ShowBadBox(m_nActualBadBoxIndex);
	}
	else if (m_nActualBadBoxIndex > 0)
		ShowBadBox(--m_nActualBadBoxIndex);
	else
		--m_nActualBadBoxIndex;
}

void COutputDoc::OnUpdateNextPrevBadbox(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasBadBoxes());
}

/////////////////////////////////////////////////////////////////////
// handling output warnings

void COutputDoc::ShowWarning(int nIndex)
{
	if (!(nIndex >= 0 && nIndex < m_aWarnings.GetSize()))
		return;

	TryOpenFile(GetFilePath(m_aWarnings[nIndex].GetSourceFile()),m_aWarnings[nIndex].GetSourceLine());

	const Nullable<int>& line = m_aWarnings[nIndex].GetOutputLine();
	UpdateAllViews(NULL,hintSelectBuildLine,(CObject*) &line);

	m_nActualWarningIndex = nIndex;
}

void COutputDoc::OnNextWarning()
{
	if (IsBuildViewActive())
	{
		if ((m_nActualWarningIndex >= m_aWarnings.GetSize() - 1) ||
		        m_nActualWarningIndex < 0)
		{
			m_nActualWarningIndex = 0;
			//BEEP;
			ShowWarning(m_nActualWarningIndex);
		}
		else if (m_nActualWarningIndex < m_aWarnings.GetSize() - 1)
			ShowWarning(++m_nActualWarningIndex);
		else
			m_nActualWarningIndex++;
	}
	else if (m_pActiveOutputView == m_pParseView)
	{
		if ((m_nParseWarningIndex >= m_aParseWarning.GetSize() - 1) ||
		        m_nParseWarningIndex < 0)
		{
			m_nParseWarningIndex = 0;
			//BEEP;
			ShowParseWarning(m_nParseWarningIndex);
		}
		else if (m_nParseWarningIndex < m_aParseWarning.GetSize() - 1)
			ShowParseWarning(++m_nParseWarningIndex);
		else
			m_nParseWarningIndex++;
	}
}

void COutputDoc::OnPrevWarning()
{
	if (IsBuildViewActive())
	{
		if ((m_nActualWarningIndex > m_aWarnings.GetSize() - 1) ||
		        (m_nActualWarningIndex <= 0))
		{
			m_nActualWarningIndex = m_aWarnings.GetSize() - 1;
			//BEEP;
			ShowWarning(m_nActualWarningIndex);
		}
		else if (m_nActualWarningIndex > 0)
			ShowWarning(--m_nActualWarningIndex);
		else
			--m_nActualWarningIndex;
	}
	else if (m_pActiveOutputView == m_pParseView)
	{
		if ((m_nParseWarningIndex > m_aParseWarning.GetSize() - 1) ||
		        (m_nParseWarningIndex <= 0))
		{
			m_nParseWarningIndex = m_aParseWarning.GetSize() - 1;
			//BEEP;
			ShowParseWarning(m_nParseWarningIndex);
		}
		else if (m_nParseWarningIndex > 0)
			ShowParseWarning(--m_nParseWarningIndex);
		else
			--m_nParseWarningIndex;
	}
}

void COutputDoc::OnUpdateNextPrevWarning(CCmdUI* pCmdUI)
{
	ASSERT_NULL_OR_POINTER(m_pActiveOutputView, OutputViewBase);

	if (IsBuildViewActive())
		pCmdUI->Enable(HasBuildWarnings());
	else if (m_pActiveOutputView == m_pParseView)
		pCmdUI->Enable(!m_aParseWarning.IsEmpty());
	else
		pCmdUI->Enable(FALSE);
}

/////////////////////////////////////////////////////////////////////
// handling parse results

void COutputDoc::ShowParseWarning(int nIndex)
{
	if (!(nIndex >= 0 && nIndex < m_aParseWarning.GetSize()))
		return;

	TryOpenFile(GetFilePath(m_aParseWarning[nIndex].GetSourceFile()),m_aParseWarning[nIndex].GetSourceLine());

	const Nullable<int>& line = m_aParseWarning[nIndex].GetOutputLine();

	UpdateAllViews(NULL,hintSelectParseLine,(CObject*) &line);

	m_nParseWarningIndex = nIndex;
}

void COutputDoc::ShowParseInfo(int nIndex)
{
	if (!(nIndex >= 0 && nIndex < m_aParseInfo.GetSize()))
		return;

	TryOpenFile(GetFilePath(m_aParseInfo[nIndex].GetSourceFile()),m_aParseInfo[nIndex].GetSourceLine());

	const Nullable<int>& line = m_aParseInfo[nIndex].GetOutputLine();

	UpdateAllViews(NULL,hintSelectParseLine,(CObject*) &line);
	m_nParseInfoIndex = nIndex;
}

/////////////////////////////////////////////////////////////////////
// handling grep results

void COutputDoc::ShowGrepResult(int nIndex)
{
	if (!(nIndex >= 0 && nIndex < m_apaGrepHits[m_nActiveFileGrep]->GetSize()))
		return;

	theApp.OpenLatexDocument(
	    GetFilePath(m_apaGrepHits[m_nActiveFileGrep]->GetAt(nIndex).strPath),FALSE,
	    m_apaGrepHits[m_nActiveFileGrep]->GetAt(nIndex).nLine,TRUE);
	UpdateAllViews(NULL,hintSelectGrep1Line + m_nActiveFileGrep,
	               (CObject*) & m_apaGrepHits[m_nActiveFileGrep]->operator[](nIndex).nOutputLine);

	m_anActiveGrepResult[m_nActiveFileGrep] = nIndex;
}

void COutputDoc::OnEditNextGrepResult()
{
	if ((m_anActiveGrepResult[m_nActiveFileGrep] >=
	        m_apaGrepHits[m_nActiveFileGrep]->GetSize() - 1) ||
	        m_anActiveGrepResult[m_nActiveFileGrep] < 0)
	{
		m_anActiveGrepResult[m_nActiveFileGrep] = 0;
		//BEEP;
		ShowGrepResult(m_anActiveGrepResult[m_nActiveFileGrep]);
	}
	else if (m_anActiveGrepResult[m_nActiveFileGrep] <
	         m_apaGrepHits[m_nActiveFileGrep]->GetSize() - 1)
		ShowGrepResult(++m_anActiveGrepResult[m_nActiveFileGrep]);
	else
		m_anActiveGrepResult[m_nActiveFileGrep]++;
}

void COutputDoc::OnEditPrevGrepResult()
{
	if ((m_anActiveGrepResult[m_nActiveFileGrep] >
	        m_apaGrepHits[m_nActiveFileGrep]->GetSize() - 1) ||
	        m_anActiveGrepResult[m_nActiveFileGrep] <= 0)
	{
		m_anActiveGrepResult[m_nActiveFileGrep] =
		    m_apaGrepHits[m_nActiveFileGrep]->GetSize() - 1;
		//BEEP;
		ShowGrepResult(m_anActiveGrepResult[m_nActiveFileGrep]);
	}
	else if (m_anActiveGrepResult[m_nActiveFileGrep] > 0)
		ShowGrepResult(--m_anActiveGrepResult[m_nActiveFileGrep]);
	else
		m_anActiveGrepResult[m_nActiveFileGrep]--;
}

void COutputDoc::OnUpdateGrepResultStep(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_apaGrepHits[m_nActiveFileGrep]->GetSize());
}

void COutputDoc::SetActiveView(OutputViewBase *pView)
{
	ASSERT(pView);

	m_pActiveOutputView = pView;

	for (int i = 0; i < 2; i++)
	{
		if (pView == m_apGrepView[i])
		{
			m_nActiveFileGrep = i;
			break;
		}
	}
}


/////////////////////////////////////////////////////////////////////
// paths, open, save, etc.

CString COutputDoc::GetMainPath() const
{
	CString result;

	if (m_bActiveFileOperation && GetActiveDocument())
		result = GetActiveDocument()->GetPathName();
	else if (CLaTeXProject* p = theApp.GetProject())
		result = p->GetMainPath();
	// Still nothing known? Try the active document anyway
	else if (CDocument* doc = GetActiveDocument())
		result = doc->GetPathName();

	if (!result.IsEmpty())
	{
		// Canonicalize the path: remove all the dots in an absolute path
		// otherwise e.g. makeindex will try to locate the file extension
		// and stop at the first dot found, resulting in a file not found error
		CString temp;
		::PathCanonicalize(temp.GetBuffer(MAX_PATH),result);
		temp.ReleaseBuffer();

		result = temp;
	}

	return result;
}

bool COutputDoc::AssureExistingMainFile()
{
	CString strPath = GetMainPath();

	if (!strPath.IsEmpty())
	{
		//Everything is fine
		if (CPathTool::Exists(strPath)) return true;
	}

	//Either: proper filename is there, but file is not on disk
	//Or: Empty file name - File not saved before

	//For projects, we just report a missing main file.
	if (!m_bActiveFileOperation)
	{
		AfxMessageBox(STE_PROJECT_MAINFILENOTFOUND,MB_ICONSTOP | MB_OK);
		return false;
	}
	else
	{
		//For single files, we remind to save. It is likely that a new file was just not saved yet.
		//Get the doc
		CDocument* pDoc = GetActiveDocument();
		if (!pDoc) return false;
		//Ask whether to save and proceed OR to cancel the action
		if (AfxMessageBox(STE_DOCUMENT_SAVEBEFOREPROCEED,MB_ICONINFORMATION | MB_OKCANCEL) == IDCANCEL)
			return false;

		//Save it
		if (!pDoc->DoSave(strPath)) return false;
	}

	return true;
};

CString COutputDoc::GetFilePath(LPCTSTR lpszFile)
{
	CString strPath;
	LPTSTR dummy;

	SetCurrentDirectory(GetWorkingDir());
	GetFullPathName(lpszFile,MAX_PATH,strPath.GetBuffer(MAX_PATH),&dummy);
	strPath.ReleaseBuffer();

	return strPath;
}

CString COutputDoc::GetWorkingDir() const
{
	if (m_bActiveFileOperation && GetActiveDocument())
		return CPathTool::GetDirectory(GetActiveDocument()->GetPathName());
	else if (theApp.GetProject())
		return theApp.GetProject()->GetWorkingDirectory();

	//Still nothing known? Try the active document anyway
	if (GetActiveDocument())
		return CPathTool::GetDirectory(GetActiveDocument()->GetPathName());

	return CString(_T(""));
}

CString COutputDoc::GetPreviewDir() const
{
	//TODO: Add UI in Profile Dialog to set preview (sub)dir
	return CString(_T("D:\\Temp\\TXCLenseTest\\txcpreview"));
}

BOOL COutputDoc::GetRunBibTex() const
{
	if (m_bActiveFileOperation)
		return TRUE;
	else
		return theApp.GetProject()->GetRunBibTex();
}

BOOL COutputDoc::GetRunMakeIndex() const
{
	if (m_bActiveFileOperation)
		return TRUE;
	else
		return theApp.GetProject()->GetRunMakeIndex();
}

CDocument* COutputDoc::GetActiveDocument() const
{
	// Obtain a pointer to the main frame window
	CFrameWnd* pFrameWnd = static_cast<CFrameWnd*>(AfxGetMainWnd());
	if (!pFrameWnd || !IsWindow(pFrameWnd->m_hWnd) || !pFrameWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd)))
		return NULL;

	// Now get a pointer to the active frame window
	CFrameWnd* pActiveFrameWnd = pFrameWnd->GetActiveFrame();
	if (pActiveFrameWnd == NULL)
		return NULL;
	return pActiveFrameWnd->GetActiveDocument();
}

void COutputDoc::OnLatexView()
{
	// check if there is an open document
	CodeView* pView = theApp.GetActiveCodeView();
	CodeDocument* pDoc = NULL;
	CString strCurrentPath;
	long lCurrentLine = -1;

	if (pView)
		pDoc = pView->GetDocument();

	if (pDoc)
	{
		strCurrentPath = pDoc->GetPathName();
		lCurrentLine = pView->GetCurrentLine() + 1;
	}

	// execute command
	CProfile *pProfile = CProfileMap::GetInstance()->GetActiveProfile();
	ASSERT(pProfile);
	if (!pProfile)
		return;

	CProfile::CCommand *pCmd = NULL;
	if (strCurrentPath.IsEmpty())
		pCmd = &pProfile->GetViewProjectCmd();
	else
	{
		pCmd = &pProfile->GetViewCurrentCmd();
		if (pCmd->GetActiveCommand() == CProfile::CCommand::typeProcess && pCmd->GetProcessCommand().GetArguments().IsEmpty())
			pCmd = &pProfile->GetViewProjectCmd();
		else if (pCmd->GetActiveCommand() == CProfile::CCommand::typeDde && (pCmd->GetDdeCommand().GetCommand().IsEmpty() || pCmd->GetDdeCommand().GetServerName().IsEmpty() || pCmd->GetDdeCommand().GetTopic().IsEmpty()))
			pCmd = &pProfile->GetViewProjectCmd();
	}

	if (pCmd && pCmd->GetActiveCommand() == CProfile::CCommand::typeProcess)
	{
		CProcess *p = pCmd->GetProcessCommand().Execute(GetWorkingDir(),GetMainPath(),strCurrentPath,lCurrentLine);
		if (p)
			delete p;
		else
		{
			CString strFormat;
			strFormat.Format(STE_LATEX_START_FAILED, (LPCTSTR)pCmd->GetProcessCommand().GetExecutable());
			AfxMessageBox(strFormat,MB_ICONSTOP | MB_OK);
		}
	}
	else if (pCmd && pCmd->GetActiveCommand() == CProfile::CCommand::typeDde)
	{
		if (!pCmd->GetDdeCommand().SendCommand(GetMainPath(),strCurrentPath,lCurrentLine))
		{
			CString strFormat;
			strFormat.Format(STE_LATEX_START_FAILED, (LPCTSTR)pCmd->GetDdeCommand().GetCommand());
			AfxMessageBox(strFormat,MB_ICONSTOP | MB_OK);
		}
	}
}

class OutputViewActivateByLineVisitor :
			public OutputViewVisitor
{
	COutputDoc* doc_;
	int line_;

public:
	OutputViewActivateByLineVisitor(COutputDoc* doc, int line)
			: doc_(doc), line_(line)
	{
	}

	void Accept(CGrepView* /*p*/)
	{
		doc_->ActivateGrepMessageByOutputLine(line_);
	}

	void Accept(CParseOutputView* /*p*/)
	{
		doc_->ActivateParseMessageByOutputLine(line_);
	}

	void Accept(CBuildView* /*p*/)
	{
		doc_->ActivateBuildMessageByOutputLine(line_);
	}
};

void COutputDoc::ActivateMessageByOutputLine(int nLine, COutputView* view)
{
	OutputViewActivateByLineVisitor visitor(this,nLine);
	view->Visit(visitor);
	// not found
	//BEEP;
}

void COutputDoc::OnEditFindInFiles()
{
	// can we grep now?
	if (!m_bCanGrep)
		return;

	//Show dialog
	CFindInFilesDialog dlg;
	dlg.m_strSearchFor = theApp.GetCurrentWordOrSelection(false, true, true);

	if (dlg.DoModal() != IDOK)
		return; // cancel pressed

	// prepare file grep
	m_nWorkingFileGrep = m_nActiveFileGrep = dlg.m_nOutput;

	DWORD dwFlags = CFileGrep::flagOwnThread;

	if (dlg.m_bCaseSensitive)
		dwFlags |= CFileGrep::flagCaseSensitive;
	if (dlg.m_bIncludeSubFolders)
		dwFlags |= CFileGrep::flagIncludeSubFolders;
	if (dlg.m_bRegularExpression)
		dwFlags |= CFileGrep::flagRegularExpression;
	if (dlg.m_bWholeWords)
		dwFlags |= CFileGrep::flagWholeWords;

	// initialize output view
	CString strStart;

	CMainFrame *pwndMainFrame = static_cast<CMainFrame*>(AfxGetMainWnd());

	if (pwndMainFrame)
		pwndMainFrame->ActivateOutputTab(CMainFrame::outputTabGrep1 + m_nWorkingFileGrep,true);

	strStart.Format(STE_GREP_START, (LPCTSTR)dlg.m_strSearchFor);
	ASSERT(m_nWorkingFileGrep == 0 || m_nWorkingFileGrep == 1);
	ASSERT(m_apGrepView[m_nWorkingFileGrep]);
	m_apGrepView[m_nWorkingFileGrep]->SendMessage(OPW_RESET);
	m_apGrepView[m_nWorkingFileGrep]->SendMessage(OPW_ADD_LINE,
	        (WPARAM)(LPCTSTR)strStart);

	// initialize attributes
	m_apaGrepHits[m_nWorkingFileGrep]->RemoveAll();
	m_anActiveGrepResult[m_nWorkingFileGrep] = -1;
	m_nWorkingGrepOutputLine = 1;

	// run grep
	m_bCanGrep = !m_fileGrep.Grep(this,/*this,*/
	                              dlg.m_strSearchFor,dlg.m_strSearchThrough,dlg.m_strSearchIn,
	                              dwFlags);

	if (m_bCanGrep)
	{
		// failed
		m_apGrepView[m_nWorkingFileGrep]->SendMessage(OPW_ADD_LINE,
		        (WPARAM)(LPCTSTR)CString((LPCTSTR)STE_GREP_FAILED));
	}
}

void COutputDoc::OnUpdateEditFindInFiles(CCmdUI* pCmdUI)
{
	if (m_bCanGrep)
	{
		pCmdUI->SetCheck(FALSE);
		pCmdUI->Enable(TRUE);
	}
	else
	{
		pCmdUI->Enable(FALSE);
		pCmdUI->SetCheck(TRUE);
	}
}

/////////////////////////////////////////////////////////////////////
// implementation of CParseOutputHandler virtuals

void COutputDoc::OnParseLineInfo(COutputInfo &line,int nLevel,int nSeverity)
{
	///////////////////////////////////////////
	//Make the Infos
	//NOTE: We need to copy the string, because we use PostMessage and
	// the original string is out of date when the message is processed.
	//We need to use PostMessage instead of SendMessage, because SendMessage
	// still blocks - I do not know why. SendMessage works for the FileGrep,
	// but for some unknown reason not for this.
	// NOTE: The Message handler is responsible for freeing the memory!!!
	LPTSTR buffer = new TCHAR[line.GetErrorMessage().GetLength() + 1];
	_tcscpy(buffer,(LPCTSTR)line.GetErrorMessage());

	//NOTE: Because we use PostMessage, we can not rely on
	// m_pParseView->GetLineCount() to get the number of issued output lines
	// as not all AddInfoLines-Messages have been processed now.
	// Therefore, we need to count the number of issued lines.
	// This is not very nice, but I do not see another way.
	line.SetOutputLine(m_nOutputLines);

	switch (nSeverity)
	{
		case CParseOutputHandler::none :
			m_aParseInfo.Add(line);
			PostMessage(m_pParseView->m_hWnd,OPW_ADD_INFOLINE,
			            MAKEWPARAM((WORD)nLevel,(WORD)CParseOutputView::imageNone),(LPARAM)buffer);
			m_nOutputLines++;
			break;
		case CParseOutputHandler::information :
			m_aParseInfo.Add(line);
			PostMessage(m_pParseView->m_hWnd,OPW_ADD_INFOLINE,
			            MAKEWPARAM((WORD)nLevel,(WORD)CParseOutputView::imageInfo),(LPARAM)buffer);
			m_nOutputLines++;
			break;

		case CParseOutputHandler::warning :
			m_aParseWarning.Add(line);
			PostMessage(m_pParseView->m_hWnd,OPW_ADD_INFOLINE,
			            MAKEWPARAM((WORD)nLevel,(WORD)CParseOutputView::imageWarning),(LPARAM)buffer);
			m_nOutputLines++;
			break;
		case CParseOutputHandler::error :
			m_aParseWarning.Add(line);
			PostMessage(m_pParseView->m_hWnd,OPW_ADD_INFOLINE,
			            MAKEWPARAM((WORD)nLevel,(WORD)CParseOutputView::imageError),(LPARAM)buffer);
			m_nOutputLines++;
			break;
		default:
		{
			ASSERT(FALSE); //Invalid nSeverity value
			delete[] buffer; //buffer was not sent - so we kill it here
		}
	}
}

void COutputDoc::OnParseBegin(bool bCancelState)
{
	//Add here things, that need to be done even if parser shall be cancelled

	m_aParseWarning.RemoveAll();
	m_aParseInfo.RemoveAll();
	m_nParseInfoIndex = m_nParseWarningIndex = 0;

	//end of: Add here things, that need to be done even if parser shall be cancelled

	//----------------------------------------------------------------------

	if (bCancelState) return;

	//Add here things, that should NOT be done if parser shall be cancelled

	PostMessage(m_pParseView->m_hWnd,OPW_RESET,0,0L);
	m_nOutputLines = 0;

	CString timeStr = AfxLoadString(STE_PARSE_BEGIN);
	int nLength = timeStr.GetLength();

	time_t ltime;
	time(&ltime);

	tm *now = localtime(&ltime);

	TCHAR *timeBuf = timeStr.GetBuffer(100);
	_tcsftime(timeBuf + nLength,100 - nLength,_T("%#c"),now);
	timeStr.ReleaseBuffer();

	// NOTE: The Message handler is responsible for freeing the memory!!!
	TCHAR* buffer = new TCHAR[timeStr.GetLength() + 1];
	_tcscpy(buffer,timeStr);

	PostMessage(m_pParseView->m_hWnd,OPW_ADD_INFOLINE,
	            MAKEWPARAM((WORD)0,(WORD)CParseOutputView::imageNone),(LPARAM)buffer);
	m_nOutputLines++;

	PostMessage(m_pParseView->m_hWnd,OPW_ADD_INFOLINE,0,0L); //Empty line
	m_nOutputLines++;

	//end of: Add here things, that should NOT be done if parser shall be cancelled
}

void COutputDoc::OnParseEnd(bool bResult,int nFiles,int nLines)
{
	//Add here things, that need to be done even after cancelling the parser


	//end of: Add here things, that need to be done even after cancelling the parser

	//----------------------------------------------------------------------

	if (!bResult) return;

	//Add here things, that should NOT be done after cancelling the parser

	ASSERT(::IsWindow(m_pParseView->m_hWnd));

	//Time Info
	CString timeStr = AfxLoadString(STE_PARSE_END);
	int nLength = timeStr.GetLength();

	struct tm *now;
	time_t ltime;
	time(&ltime);
	now = localtime(&ltime);

	TCHAR *timeBuf = timeStr.GetBuffer(100);
	_tcsftime(timeBuf + nLength,100 - nLength,_T("%#c"),now);
	timeStr.ReleaseBuffer();

	// NOTE: The Message handler is responsible for freeing the memory!!!
	TCHAR* buffer = new TCHAR[timeStr.GetLength() + 1];
	_tcscpy(buffer,(LPCTSTR)timeStr);
	PostMessage(m_pParseView->m_hWnd,OPW_ADD_INFOLINE,0,0L); //Empty line
	m_nOutputLines++;
	PostMessage(m_pParseView->m_hWnd,OPW_ADD_INFOLINE,
	            MAKEWPARAM((WORD)0,(WORD)CParseOutputView::imageNone),(LPARAM)buffer);
	m_nOutputLines++;


	//The result of parsing
	CString results;
	results.Format(STE_PARSE_RESULTS,nFiles,nLines);
	// NOTE: The Message handler is responsible for freeing the memory!!!
	TCHAR* buffer2 = new TCHAR[results.GetLength() + 1];
	_tcscpy(buffer2,results);

	PostMessage(m_pParseView->m_hWnd,OPW_ADD_INFOLINE,
	            MAKEWPARAM((WORD)0,(WORD)CParseOutputView::imageNone),(LPARAM)buffer2);
	m_nOutputLines++;

	//end of: Add here things, that should NOT be done after cancelling the parser
}


/////////////////////////////////////////////////////////////////////
// implementation of CFileGrepHandler virtuals

void COutputDoc::OnFileGrepHit(CFileGrep *pFileGrep,LPCTSTR lpszPath,
                               int nLine,LPCTSTR lpszLine)
{
	// identify file grep object
	if (pFileGrep != &m_fileGrep)
		return;

	// Add string to grep view
	CString strHit;
	strHit.Format(STE_GREP_HIT,lpszPath,nLine,lpszLine);

	SendMessage(m_apGrepView[m_nWorkingFileGrep]->m_hWnd,OPW_ADD_LINE,
	            (WPARAM)(LPCTSTR)strHit,0L);

	// add hit
	GREPHIT gh;
	gh.strPath = lpszPath;
	gh.nLine = nLine;
	gh.nOutputLine = m_nWorkingGrepOutputLine++;

	m_apaGrepHits[m_nWorkingFileGrep]->Add(gh);
}

void COutputDoc::OnFileGrepError(CFileGrep *pFileGrep,LPCTSTR lpszPath)
{
	// identify file grep object
	if (pFileGrep != &m_fileGrep)
		return;

	// Add error to grep view
	CString strError;
	strError.Format(STE_GREP_ERROR,lpszPath);
	SendMessage(m_apGrepView[m_nWorkingFileGrep]->m_hWnd,OPW_ADD_LINE,
	            (WPARAM)(LPCTSTR)strError,0L);
	m_nWorkingGrepOutputLine++;
}

void COutputDoc::OnFileGrepFinished(CFileGrep *pFileGrep,int nHints)
{
	// identify file grep object
	if (pFileGrep != &m_fileGrep)
		return;

	// Add string to grep view
	CString strFinished;

	if (nHints > 0)
		strFinished.Format(STE_GREP_FINISHED,nHints);
	else
		strFinished.LoadString(STE_GREP_FINISHEDNOHIT);

	SendMessage(m_apGrepView[m_nWorkingFileGrep]->m_hWnd,OPW_ADD_LINE,
	            (WPARAM)(LPCTSTR)strFinished,0L);
	m_nWorkingGrepOutputLine++;

	// accoustic information
	//MessageBeep( MB_ICONINFORMATION );

	// allow grep
	m_bCanGrep = TRUE;
}

void COutputDoc::UpdateAllViews(COutputView* pSender,LPARAM lHint,CObject* pHint)
{
	// walk through all views
	ASSERT(pSender == NULL);
	// must have views if sent by one of them

	ASSERT_VALID(m_pBuildView);
	if (m_pBuildView != pSender)
		m_pBuildView->DoOnUpdate(pSender,lHint,pHint);
	ASSERT_VALID(m_apGrepView[0]);
	if (m_apGrepView[0] != pSender)
		m_apGrepView[0]->DoOnUpdate(pSender,lHint,pHint);
	ASSERT_VALID(m_apGrepView[1]);
	if (m_apGrepView[1] != pSender)
		m_apGrepView[1]->DoOnUpdate(pSender,lHint,pHint);
	ASSERT_VALID(m_pParseView);
	if (m_pParseView != pSender)
		m_pParseView->DoOnUpdate(pSender,lHint,pHint);
}

void COutputDoc::OnLatexRun()
{
	m_bActiveFileOperation = FALSE;
	DoLaTeXRun();
}

void COutputDoc::OnUpdateLatexRun(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(
	    theApp.GetProject()
	    && !CProfileMap::GetInstance()->GetActiveProfileKey().IsEmpty()
	    && !m_builder.IsStillRunning()
	);
}

void COutputDoc::OnBibTex()
{
	m_bActiveFileOperation = FALSE;
	DoBibTexRun();
}

void COutputDoc::OnUpdateBibTex(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(
	    !m_builder.IsStillRunning() &&
	    theApp.GetProject() &&
	    !CProfileMap::GetInstance()->GetActiveProfileKey().IsEmpty() && CProfileMap::GetInstance()->GetActiveProfile()->GetRunBibTex());
}

void COutputDoc::OnMakeIndex()
{
	m_bActiveFileOperation = FALSE;
	DoMakeIndexRun();
}

void COutputDoc::OnUpdateMakeIndex(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(
	    !m_builder.IsStillRunning() &&
	    theApp.GetProject() &&
	    !CProfileMap::GetInstance()->GetActiveProfileKey().IsEmpty() && CProfileMap::GetInstance()->GetActiveProfile()->GetRunMakeIndex());
}

void COutputDoc::OnFileCompile()
{
	m_bActiveFileOperation = TRUE;
	DoLaTeXRun();
}

void COutputDoc::OnUpdateFileCompile(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetActiveDocument() != NULL && !m_builder.IsStillRunning());
}

void COutputDoc::OnFileBibTex()
{
	m_bActiveFileOperation = TRUE;
	DoBibTexRun();
}

void COutputDoc::OnUpdateFileBibTex(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(
	    GetActiveDocument() != NULL && !m_builder.IsStillRunning() &&
	    !CProfileMap::GetInstance()->GetActiveProfileKey().IsEmpty() && CProfileMap::GetInstance()->GetActiveProfile()->GetRunBibTex());
}

void COutputDoc::OnFileMakeIndex()
{
	m_bActiveFileOperation = TRUE;
	DoMakeIndexRun();
}

void COutputDoc::OnUpdateFileMakeIndex(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(
	    GetActiveDocument() != NULL && !m_builder.IsStillRunning() &&
	    !CProfileMap::GetInstance()->GetActiveProfileKey().IsEmpty() && CProfileMap::GetInstance()->GetActiveProfile()->GetRunMakeIndex());
}

void COutputDoc::DoLaTeXRun()
{
	if (m_builder.IsStillRunning()) return;

	//Save all modified files
	//NOTE: This saves only files, that have been saved before
	if (CConfiguration::GetInstance()->m_bSaveBeforeCompilation)
		theApp.SaveAllModifiedWithoutPrompt();

	//Save main file, even if not saved before
	if (!AssureExistingMainFile()) return;

	// remove all error marks
	CMultiDocTemplate* pDocTemplate = theApp.GetLatexDocTemplate();

	if (pDocTemplate)
	{
		POSITION pos = pDocTemplate->GetFirstDocPosition();

		while (pos)
		{
			LaTeXDocument *pDoc = dynamic_cast<LaTeXDocument*>(pDocTemplate->GetNextDoc(pos));
			if (pDoc) pDoc->SetErrorMark(-1);
		}
	}

	// close viewer if necessary
	CProfile* pProfile = CProfileMap::GetInstance()->GetActiveProfile();

	if (pProfile && pProfile->GetCloseView())
	{
		// remember windows that has the input focus
		CWnd* pwnd = CWnd::GetFocus();

		CProfile::CCommand &cmd = pProfile->GetViewCloseCmd();

		if (cmd.GetActiveCommand() == CProfile::CCommand::typeProcess)
		{
			CProcess* p = cmd.GetProcessCommand().Execute(GetWorkingDir(),GetMainPath());
			if (p) delete p;
		}
		else
		{
			// prevent executable from being started, if not already running
			CDdeCommand dde = cmd.GetDdeCommand();
			dde.SetExecutable(_T(""));
			dde.SendCommand(GetMainPath());
		}

		// set focus back to window that had the focus before
		if (pwnd && IsWindow(pwnd->m_hWnd))
			pwnd->SetFocus();
	}

	// initialize members
	m_aErrors.RemoveAll();
	m_aWarnings.RemoveAll();
	m_aBadBoxes.RemoveAll();
	m_nActualErrorIndex = -1;
	m_nActualWarningIndex = -1;
	m_nActualBadBoxIndex = -1;

	// activate output bar / tab
	CMainFrame* pwndMainFrame = static_cast<CMainFrame*>(AfxGetMainWnd());

	if (pwndMainFrame)
		pwndMainFrame->ActivateOutputTab(CMainFrame::outputTabBuildResult,false);

	// run latex
	m_builder.BuildAll(
	    this,m_pBuildView,
	    GetWorkingDir(),GetMainPath(),
	    GetRunBibTex(),GetRunMakeIndex());
}

void COutputDoc::DoBibTexRun()
{
	if (m_builder.IsStillRunning())
		return;

	//Save all modified files
	//NOTE: This saves only files, that have been saved before
	if (CConfiguration::GetInstance()->m_bSaveBeforeCompilation)
		theApp.SaveAllModifiedWithoutPrompt();

	//Save main file, even if not saved before
	if (!AssureExistingMainFile()) return;

	// initialize members
	m_aErrors.RemoveAll();
	m_aWarnings.RemoveAll();
	m_aBadBoxes.RemoveAll();
	m_nActualErrorIndex = -1;
	m_nActualWarningIndex = -1;
	m_nActualBadBoxIndex = -1;

	// activate output bar / tab
	CMainFrame* pwndMainFrame = (CMainFrame*)AfxGetMainWnd();
	if (pwndMainFrame)
		pwndMainFrame->ActivateOutputTab(CMainFrame::outputTabBuildResult,false);

	// run latex
	m_builder.RunBibTex(this,m_pBuildView,GetWorkingDir(),GetMainPath());
}

void COutputDoc::DoMakeIndexRun()
{
	if (m_builder.IsStillRunning())
		return;

	//Save all modified files
	//NOTE: This saves only files, that have been saved before
	if (CConfiguration::GetInstance()->m_bSaveBeforeCompilation)
		theApp.SaveAllModifiedWithoutPrompt();

	//Save main file, even if not saved before
	if (!AssureExistingMainFile()) return;

	// initialize members
	m_aErrors.RemoveAll();
	m_aWarnings.RemoveAll();
	m_aBadBoxes.RemoveAll();
	m_nActualErrorIndex = -1;
	m_nActualWarningIndex = -1;
	m_nActualBadBoxIndex = -1;

	// activate output bar / tab
	CMainFrame* pwndMainFrame = (CMainFrame*)AfxGetMainWnd();

	if (pwndMainFrame)
		pwndMainFrame->ActivateOutputTab(CMainFrame::outputTabBuildResult,false);

	// run makeindex
	m_builder.RunMakeIndex(this,m_pBuildView,GetWorkingDir(),GetMainPath());
}

void COutputDoc::DoPreviewRun()
{
	if (m_builder.IsStillRunning())
		return;

	//Save all modified files
	//NOTE: This saves only files, that have been saved before
	//if (CConfiguration::GetInstance()->m_bSaveBeforeCompilation)
	//	theApp.SaveAllModifiedWithoutPrompt();

	//Save main file, even if not saved before
	//if (!AssureExistingMainFile()) return;

	//For previews, we do not scan the errors, warnings, etc.
	//We also do not activate the output.

	//Shorthands
	// - current project
	CLaTeXProject* pProject = theApp.GetProject();
	if (!pProject) return;
	// - current editor view and document
	CodeView* pView = theApp.GetActiveCodeView();
	if (!pView) return;
	CScintillaCtrl& SCtrl = pView->GetCtrl();
	CodeDocument* pDoc = pView->GetDocument();
	if (!pDoc) return;

	//Range of the LaTeX text to be used for compiling the preview
	std::pair<long, long> PreviewRange(-1, -1);
	CodeView* pPreviewView = NULL;

	//Start and end of selection, if any.
	std::pair<long, long> SelectionRange(SCtrl.GetSelectionStart(), SCtrl.GetSelectionEnd());

	//Did the user select something?
	if (SelectionRange.first == SelectionRange.second)
	{
		//Nothing has been selected.

		//Do we have Preview-Bookmarks from before?
		CLaTeXProject::FileBookmarksContainerType BMarkFiles;
		const int nBookmarksFound = pProject->FindBookmarksByName(_T("TXCPreview"), BMarkFiles);
		//We are only interested if we found exactly 2 bookmarks in 1 file
		if (nBookmarksFound == 2 && BMarkFiles.size() == 1)
		{
			//Make sure the corresponding doc is open (possibly open it in background)
			const auto& BMarks = *(BMarkFiles.begin());
			CodeDocument* pPreviewDoc = static_cast<CodeDocument*>(theApp.OpenLatexDocument(BMarks.first, FALSE, -1, FALSE, FALSE, TRUE));
			if (!pPreviewDoc) return;

			//Get the positions for these bookmarks
			CodeView* pBackgroundView = dynamic_cast<CodeView*>(pPreviewDoc->GetView());
			if (!pBackgroundView) return;
			// - text area for preview
			PreviewRange.first = pBackgroundView->GetCtrl().PositionFromLine(BMarks.second[0].GetLine());
			PreviewRange.second = pBackgroundView->GetCtrl().GetLineEndPosition(BMarks.second[1].GetLine());
			pPreviewView = pBackgroundView;

			//Re-activate this view, if we opened another one
			if (pView != pBackgroundView)
			{
				CFrameWnd* pFrame = pView->GetParentFrame();
				if (pFrame) pFrame->ActivateFrame();
			}
		}
		else
		{
			//Nothing selected and no previous bookmarks.
			//Inform the user to select something and return doing nothing else.
			// TODO: AfxMessageBox here
			// Note: The menu entry must be active independent of a selection, because of the bookmarks.
			return;
		}
	}
	else
	{
		//Something has been selected by the user.
		//The selected text is used for preview.
		pPreviewView = pView;

		//We distinguish between selecting several lines, or some text within a line.
		const int StartLine = SCtrl.LineFromPosition(SelectionRange.first);
		const int EndLine = SCtrl.LineFromPosition(SelectionRange.second);
		if (EndLine - StartLine > 0)
		{
			//Multiple lines have been selected: (Re-)Define Preview-Bookmarks
			//Do we have Preview-Bookmarks from before?
			CLaTeXProject::FileBookmarksContainerType BMarkFiles;
			pProject->FindBookmarksByName(_T("TXCPreview"), BMarkFiles);
			//Delete found bookmarks; everything matching our search term will be deleted.
			// We will establish new bookmarks below.
			for each (const auto& FileBMarks in BMarkFiles)
			{
				for each (const CodeBookmark& BMark in FileBMarks.second)
				{
					pProject->RemoveBookmark(FileBMarks.first, BMark);
				}
			}

			//Create new bookmarks
			CodeBookmark NewPreviewBMarkStart(StartLine);
			NewPreviewBMarkStart.SetName(_T("TXCPreviewStart"));
			CodeBookmark NewPreviewBMarkEnd(EndLine);
			NewPreviewBMarkEnd.SetName(_T("TXCPreviewEnd"));
			pProject->AddBookmark(pDoc->GetFilePath(), NewPreviewBMarkStart);
			pProject->AddBookmark(pDoc->GetFilePath(), NewPreviewBMarkEnd);

			//Text area for preview
			PreviewRange.first = SCtrl.PositionFromLine(StartLine);
			PreviewRange.second = SCtrl.GetLineEndPosition(EndLine);
		}
		else
		{
			PreviewRange = SelectionRange;
		}
	}

	//If we have a valid preview range, then extract the text, and create the preview
	if (pPreviewView && (PreviewRange.second - PreviewRange.first > 0))
	{
		CString PreviewText = pPreviewView->GetText(PreviewRange.first, PreviewRange.second, false, false);

		//Write preview text to 'content.tex' in the preview folder; same encoding as original file
		CString PreviewContentPath = CPathTool::Cat(GetPreviewDir(), _T("content.tex"), true);
		TextDocument td(pPreviewView->GetDocument());
		td.Write(PreviewContentPath, PreviewText);

		//TODO: We will have a menu with more than 1 template. The user can choose one, the choice will be written to the tps file
		CString strPreviewMainPath = CPathTool::Cat(GetPreviewDir(), _T("template.tex"), true);

		// build the preview
		m_builder.BuildPreview(NULL, NULL, /*this, m_pBuildView*/ GetPreviewDir(), strPreviewMainPath);
	}
}

void COutputDoc::OnUpdateLatexStopBuild(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_builder.IsStillRunning());
}

void COutputDoc::OnLatexStopBuild()
{
	//Cancel the build
	m_builder.CancelExecution();
}

void COutputDoc::ClearMessages()
{
	ClearBuildMessages();
	ClearParseMessages();
}

void COutputDoc::ClearBuildMessages()
{
	m_aErrors.RemoveAll();
	m_aWarnings.RemoveAll();
	m_aBadBoxes.RemoveAll();
	m_pBuildView->Clear();

	if (errorListView_)
		errorListView_->Clear();
}

void COutputDoc::AddError(COutputInfo& error)
{
	m_aErrors.Add(error);
	m_pBuildView->SetLineImage(error.GetOutputLine().GetValueOrDefault(),CBuildView::imageError);

	if (errorListView_)
		errorListView_->AddMessage(error,CBuildView::imageError);
}

void COutputDoc::AddWarning(COutputInfo& warning)
{
	m_aWarnings.Add(warning);
	m_pBuildView->SetLineImage(warning.GetOutputLine().GetValueOrDefault(),CBuildView::imageWarning);

	if (errorListView_)
		errorListView_->AddMessage(warning,CBuildView::imageWarning);
}

void COutputDoc::AddBadBox(COutputInfo& badbox)
{
	m_aBadBoxes.Add(badbox);
	m_pBuildView->SetLineImage(badbox.GetOutputLine().GetValueOrDefault(),CBuildView::imageBadBox);

	if (errorListView_)
		errorListView_->AddMessage(badbox,CBuildView::imageBadBox);
}

void COutputDoc::EnableUpdateOfViews(const bool bEnable)
{
	//Enable/disable update of build output
	if (m_pBuildView)
	{
		m_pBuildView->EnableUpdate(bEnable);
	}

	//Enable/disable update of error list
	if (errorListView_)
	{
		errorListView_->EnableUpdate(bEnable);
	}
}

void COutputDoc::OnLatexClean()
{
	CFileClean fc;
	bool bCleanIt = true;

	fc.Initialize(CConfiguration::GetInstance()->m_aFileCleanItems);

	if (CConfiguration::GetInstance()->m_bFileCleanConfirm)
	{
		CFileCleanConfirmDialog fcDlg;
		fcDlg.m_bShowDialog = true;
		fcDlg.m_CleanedFiles = fc.GetFilesToClean(true);
		fcDlg.m_ProtectedFiles = fc.GetFilesToProtect();

		if (fcDlg.DoModal() == IDCANCEL)
		{
			bCleanIt = false;
		}
		else
		{
			//bCleanIt = true;
			CConfiguration::GetInstance()->m_bFileCleanConfirm = fcDlg.m_bShowDialog;
		}
	}

	if (bCleanIt)
	{
		if (!fc.CleanFiles())
			AfxMessageBox(STE_FILECLEAN_ERROR);
	}
}

void COutputDoc::OnUpdateLatexClean(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(
	    !m_builder.IsStillRunning() &&
	    theApp.GetProject() &&
	    !CProfileMap::GetInstance()->GetActiveProfileKey().IsEmpty());
}

void COutputDoc::OnLatexRunAndView()
{
	m_builder.MsgAfterTermination.Set(false,AfxGetMainWnd()->m_hWnd,WM_COMMAND,ID_LATEX_VIEW,0,true,true,0);
	OnLatexRun();
}

void COutputDoc::OnLatexFileCompileAndView()
{
	m_builder.MsgAfterTermination.Set(false,AfxGetMainWnd()->m_hWnd,WM_COMMAND,ID_LATEX_VIEW,0,true,true,0);
	OnFileCompile();
}

bool COutputDoc::TryOpenFile(LPCTSTR lpszFilename, const Nullable<int>& nLineNumber)
{
	bool result;

	if (nLineNumber.HasValue() && CPathTool::IsFile(lpszFilename))
		result = theApp.OpenLatexDocument(lpszFilename,FALSE, nLineNumber.GetValueOrDefault(-1),TRUE) != NULL;
	else 
	{
		MessageBeep(MB_ICONSTOP);
		result = false;
	}

	return result;
}

void COutputDoc::OnUpdateLatexView(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(/* !m_builder.IsStillRunning() && */ //Don't do this, since this will de-active the CompileAndView Commands
	    (GetActiveDocument() != NULL || theApp.GetProject() != NULL));
}

void COutputDoc::ActivateBuildMessageByOutputLine(int nLine)
{
	int i;

	// search errors for specified line
	for (i = 0; i < m_aErrors.GetSize(); i++)
	{
		if (m_aErrors[i].GetOutputLine() == nLine)
		{
			ShowError(i);
			return;
		}
	}

	// search warnings for specified line
	for (i = 0; i < m_aWarnings.GetSize(); i++)
	{
		if (m_aWarnings[i].GetOutputLine() == nLine)
		{
			ShowWarning(i);
			return;
		}
	}

	// search bad boxes for specified line
	for (i = 0; i < m_aBadBoxes.GetSize(); i++)
	{
		if (m_aBadBoxes[i].GetOutputLine() == nLine)
		{
			ShowBadBox(i);
			return;
		}
	}
}


void COutputDoc::ActivateParseMessageByOutputLine(int nLine)
{
	for (int i = 0; i < m_aParseWarning.GetSize(); i++)
	{
		if (m_aParseWarning[i].GetOutputLine() == nLine)
		{
			ShowParseWarning(i);
			return;
		}
	}

	for (int i = 0; i < m_aParseInfo.GetSize(); i++)
	{
		if (m_aParseInfo[i].GetOutputLine() == nLine)
		{
			ShowParseInfo(i);
			return;
		}
	}
}

void COutputDoc::ActivateGrepMessageByOutputLine(int nLine)
{
	for (int i = 0; i < m_apaGrepHits[m_nActiveFileGrep]->GetSize(); i++)
	{
		if (m_apaGrepHits[m_nActiveFileGrep]->GetAt(i).nOutputLine == nLine)
		{
			ShowGrepResult(i);
			return;
		}
	}
}

const CString COutputDoc::GetCurrentProcessName() const
{
	return m_builder.GetCurrentProcessName();
}

int COutputDoc::GetBuildErrorCount() const
{
	return m_aErrors.GetSize();
}

int COutputDoc::GetBuildBadBoxCount() const
{
	return m_aBadBoxes.GetSize();
}

int COutputDoc::GetBuildWarningCount() const
{
	return m_aWarnings.GetSize();
}

bool COutputDoc::HasBuildErrors() const
{
	return GetBuildErrorCount() > 0;
}

bool COutputDoc::HasBadBoxes() const
{
	return GetBuildBadBoxCount() > 0;
}

bool COutputDoc::HasBuildWarnings() const
{
	return GetBuildWarningCount() > 0;
}

void COutputDoc::ClearParseMessages()
{
	m_aParseWarning.RemoveAll();
	m_aParseInfo.RemoveAll();

	if (m_pParseView) {
		ASSERT_VALID(m_pParseView);
		m_pParseView->Clear();
	}
}

bool COutputDoc::IsBuildViewActive() const
{
	return m_pActiveOutputView == m_pBuildView || m_pActiveOutputView == errorListView_;
}

const CString COutputDoc::GetTranscriptFilePath() const
{
	return CPathTool::Cat(GetWorkingDir(), m_builder.GetTranscriptFileName());
}

void COutputDoc::OnBuildPreview()
{
	//m_bActiveFileOperation = FALSE;

	//TODO: Decide on whether this can be run for single active files.
	// Then see also the UI Update for this menu entry

	DoPreviewRun();
}
