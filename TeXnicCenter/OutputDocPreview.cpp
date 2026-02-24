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
#include "Profile.h"
#include "OutputBuilder.h"
#include "ErrorListPane.h"
#include "LaTeXDocument.h"
#include "LaTeXView.h"
#include "TeXnicCenter.h"
#include "PlaceHolder.h"
#include "PreviewImagePane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CString COutputDoc::GetPreviewImagePath() const
{
	//Shorthands
	CProfile* pProfile = CProfileMap::GetInstance()->GetActiveProfile();
	ASSERT(pProfile);
	if (!pProfile) return CString();

	// Collect necessary information
	// - project information
	CLaTeXProject* pProject = theApp.GetProject();
	if (!pProject) return CString();
	CString strMainPath = pProject->GetMainPath();

	//Replace any placeholders. We only allow placeholders based on the project.
	CString ExpandedPath = AfxExpandPlaceholders(pProfile->GetPreviewImagePath(), strMainPath);
	//Get an absolute path
	ExpandedPath = CPathTool::GetAbsolutePath(pProject->GetWorkingDirectory(), ExpandedPath, true);

	return ExpandedPath;
}


CString COutputDoc::GetPreviewDir() const
{
	return CPathTool::GetDirectory(GetPreviewImagePath());
}


int COutputDoc::GetPreviewText(CString& PreviewText, CodeDocument* pPreviewDoc)
{
	//Shorthands
	// - current project
	CLaTeXProject* pProject = theApp.GetProject();
	if (!pProject) return -1;
	// - current editor view and document
	CodeView* pView = theApp.GetActiveCodeView();
	if (!pView) return -1;
	CScintillaCtrl& SCtrl = pView->GetCtrl();
	CodeDocument* pDoc = pView->GetDocument();
	if (!pDoc) return -1;

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
			CodeDocument* pBackgroundDoc = static_cast<CodeDocument*>(theApp.OpenLatexDocument(BMarks.first, FALSE, -1, FALSE, FALSE, TRUE));
			if (!pBackgroundDoc) return -1;

			//Get the positions for these bookmarks
			CodeView* pBackgroundView = dynamic_cast<CodeView*>(pBackgroundDoc->GetView());
			if (!pBackgroundView) return -1;
			// - text area for preview
			PreviewRange.first = pBackgroundView->GetCtrl().PositionFromLine(BMarks.second[0].GetLine());
			PreviewRange.second = pBackgroundView->GetCtrl().GetLineEndPosition(BMarks.second[1].GetLine());
			pPreviewView = pBackgroundView;

			//Re-activate this view, if we opened another one. TODO: Does this work and do we even want this?
			if (pView != pBackgroundView)
			{
				CFrameWnd* pFrame = pView->GetParentFrame();
				if (pFrame) pFrame->ActivateFrame();
			}
		}
		else
		{
			//Nothing selected and no previous bookmarks.
			// Note: The menu entry must be active independent of a selection, because of the bookmarks.
			return 0;
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
		PreviewText = pPreviewView->GetText(PreviewRange.first, PreviewRange.second, false, false);
		pPreviewDoc = pPreviewView->GetDocument();
		return 1;
	}

	return -1;
}


bool COutputDoc::WritePreviewText(const CString& PreviewContentPath)
{
	//Get the preview text. Check validity.
	CString PreviewText;
	CodeDocument* pPreviewDoc = NULL;
	const int nValidPreviewText = GetPreviewText(PreviewText, pPreviewDoc);
	if (nValidPreviewText < 1)
	{
		if (nValidPreviewText == 0) AfxMessageBox(STE_PREVIEW_NOSELECTION);
		return false;
	}

	//Write preview text; same encoding as original file
	TextDocument td(pPreviewDoc); //ok to be NULL
	td.Write(PreviewContentPath, PreviewText);

	return true;
}


CString COutputDoc::GetPreviewGeneratedTemplateFileName() const
{
	return _T("Template ") + AfxLoadString(STE_PREVIEW_GENERATEDTEMPLATE) + _T(".tex");
}


bool COutputDoc::CreatePreviewTemplateFromMainFile(const CString& PreviewDir, const bool bOverwrite)
{
	//Write template text to 'Template Generated from Main File.tex' in the preview folder.
	CString FName = GetPreviewGeneratedTemplateFileName();
	CString PreviewTemplatePath = CPathTool::Cat(PreviewDir, FName, true);

	if (!CPathTool::Exists(PreviewTemplatePath) || bOverwrite)
	{
		//Get main document
		CString strMainPath = GetMainPath();

		//Current editor view
		CodeView* pCurrentView = theApp.GetActiveCodeView();

		//Open the main document in the background
		CodeDocument* pMainDoc = static_cast<CodeDocument*>(theApp.OpenLatexDocument(strMainPath, FALSE, -1, FALSE, FALSE, TRUE));
		if (!pMainDoc) return false;
		CodeView* pMainView = dynamic_cast<CodeView*>(pMainDoc->GetView());
		if (!pMainView) return false;
		CScintillaCtrl& SCtrl = pMainView->GetCtrl();

		//Re-activate the current view, if the main one was not open
		if (pCurrentView && pCurrentView != pMainView)
		{
			CFrameWnd* pFrame = pCurrentView->GetParentFrame();
			if (pFrame) pFrame->ActivateFrame();
		}

		//Find the begin of the document
		SCtrl.SetTargetStart(0);
		SCtrl.SetTargetEnd(SCtrl.GetTextLength());
		CString SearchText(_T("\\begin{document}")); //TODO: maybe we could be more regexp here with \s* and such
		const int posBeginDoc = SCtrl.SearchInTarget(SearchText.GetLength(), SearchText);
		if (posBeginDoc < 0) return false; //not found

		//Get the preamble
		CString Preamble = pMainView->GetText(0, posBeginDoc, false, false);

		//Assemble template
		CString TemplateText = Preamble + _T("\n\n\\begin{document}\n\n\\input{content}\n\n\\end{document}\n");

		//Actual writing of the template
		TextDocument td(pMainDoc);
		td.Write(PreviewTemplatePath, TemplateText);
	}

	return true;
}


bool COutputDoc::CreatePreviewDir(const CString& PreviewDir, const bool bOverwrite,
									const bool bCopyTemplates, const bool bCreateFromMainFile)
{
	//Create the directory 'PreviewDir' recursively using the Windows API
	int nRes = SHCreateDirectoryEx(theApp.GetMainWnd()->GetSafeHwnd(), PreviewDir, NULL);
	if (nRes != ERROR_SUCCESS && nRes != ERROR_FILE_EXISTS && nRes != ERROR_ALREADY_EXISTS)
	{
		CString strMsg;
		strMsg.Format(STE_PREVIEW_FOLDER_CREATE_ERROR,
			(LPCTSTR)PreviewDir,
			(LPCTSTR)AfxFormatSystemString(nRes));

		AfxMessageBox(strMsg, MB_ICONERROR | MB_OK);
		return false;
	}

	//Record whether we could achieve something below regarding templates.
	bool bSomeSuccess(false);

	//Where are our templates?
	CString TemplateDir = CPathTool::Cat(theApp.GetWorkingDir(), _T("Templates\\Preview"));
	//if (!CPathTool::Exists(TemplateDir)) return false;

	//Copy all files from the templates directory to the preview directory
	if (bCopyTemplates)
	{
		CFileFind finder;
		CString strAccumulatedErrorMessages;
		bool bWorking = finder.FindFile(TemplateDir + _T("\\*.*"));
		if (!bWorking) strAccumulatedErrorMessages += AfxFormatSystemString(GetLastError());
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDirectory()) continue;

			//Get the file name
			CString strFileName = finder.GetFileName();
			//Get the full path
			CString strFilePath = finder.GetFilePath();
			//Get the destination path
			CString strDestPath = CPathTool::Cat(PreviewDir, strFileName);

			//Does the file exist? If so, we only copy if the user wants us to overwrite.
			if (!CPathTool::Exists(strDestPath) || bOverwrite)
			{
				//Copy the file, potentially overwrite
				const bool bSuccess = CopyFile(strFilePath, strDestPath, false);
				bSomeSuccess = bSomeSuccess || bSuccess;		
				if (!bSuccess)
				{
					if (!strAccumulatedErrorMessages.IsEmpty()) strAccumulatedErrorMessages += "\n";
					strAccumulatedErrorMessages += AfxFormatSystemString(GetLastError());
				}
			}
			else
			{
				//File exists and we do not want to overwrite. This is a success.
				bSomeSuccess = true;
			}
		}

		//Any errors?
		if (!strAccumulatedErrorMessages.IsEmpty())
		{
			CString strMsg;
			strMsg.Format(STE_PREVIEW_TEMPLATE_COPY_ERRORS,
							(LPCTSTR)TemplateDir,
							(LPCTSTR)PreviewDir,
							(LPCTSTR)strAccumulatedErrorMessages);

			UINT MsgBoxErrorOrInfo = MB_OK;
			if (bSomeSuccess)
			{
				MsgBoxErrorOrInfo |= MB_ICONINFORMATION;
			}
			else
			{
				MsgBoxErrorOrInfo |= MB_ICONERROR;
			}
			AfxMessageBox(strMsg, MsgBoxErrorOrInfo);
		}
	}

	//Create template from main document
	if (bCreateFromMainFile)
	{
		const bool bSuccess = CreatePreviewTemplateFromMainFile(PreviewDir, bOverwrite);
		bSomeSuccess = bSomeSuccess || bSuccess;		
		//if (!bSomeSuccess) <== we could display an error message.
	}

	//The preview pane needs to update its drop down
	if (m_pPreviewImagePane) m_pPreviewImagePane->FillTemplateDropDown();

	return bSomeSuccess;
}


bool COutputDoc::PrepareFastPreview(const CString& PreviewDir)
{
	bool bSuccess(true);
	
	//Take care of our helper application for fast mode
	CString ExeName = _T("WaitForFileChange.exe");
	CString PreviewExePath = CPathTool::Cat(PreviewDir, ExeName, true);
	if (!CPathTool::Exists(PreviewExePath))
	{
		//Where is our original preview helper executable?
		CString OrigExe = CPathTool::Cat(theApp.GetWorkingDir(), ExeName);
		if (!CPathTool::Exists(OrigExe))
		{
			//We need to switch off fast mode. Our helper does not exist anywhere.
			CConfiguration::GetInstance()->m_bPreviewFastMode = false;
			return false;
		}

		bSuccess = CopyFile(OrigExe, PreviewExePath, true);
	}

	//Put default text into 'content.tex'
	CString ContentName = _T("content.tex");
	CString ContentPath = CPathTool::Cat(PreviewDir, ContentName, true);
	const CString Content(_T("\\immediate\\write18{WaitForFileChange}\n\\input{content2}\n"));
	TextDocument td(NULL); //ok to be NULL
	bSuccess = bSuccess && td.Write(ContentPath, Content);

	//We record the name of the file that we are waiting on. Good for canceling the process.
	LastWaitForFileChangePath = CPathTool::Cat(PreviewDir, _T("content2.tex"), true);

	return bSuccess;
}


void COutputDoc::OnUpdateBuildPreview(CCmdUI* pCmdUI)
{
	const bool bFastMode = CConfiguration::GetInstance()->m_bPreviewFastMode;

	pCmdUI->Enable(
	    theApp.GetProject()
	    && !CProfileMap::GetInstance()->GetActiveProfileKey().IsEmpty()
	    && (bFastMode || !m_preview_builder.IsStillRunning())
	);
}


void COutputDoc::OnActiveProfileChange()
{
	//Profiles define how previews are done.
	//Any running preview needs to be cancelled.
	CancelBuilds(false, true);

	//Rescan for preview templates in the possibly new preview folder.
	m_pPreviewImagePane->FillTemplateDropDown();
}


void COutputDoc::OnBuildPreview()
{
	//TODO: Decide on whether this can be run for single active files.
	// Then see also the UI Update for this menu entry above.

	//Open tool window to show feedback and preview image itself.
	CWnd* pMainWnd = AfxGetMainWnd();
	if (pMainWnd) PostMessage(pMainWnd->m_hWnd, AfxUserMessages::ShowDockingBarID, ID_VIEW_PREVIEW_IMAGE_PANE, 0);

	if (CConfiguration::GetInstance()->m_bPreviewFastMode)
	{
		// Fast Mode ///////////////////////////////////////////////////
		
		bool bSuccess(true);

		//Start the previewer builder, if it is not running yet
		if (!m_preview_builder.IsStillRunning())
		{
			bSuccess = bSuccess && DoPreviewRun();
		}

		//Write the preview text to file: this triggers the finalization of the preview run
		const CString PreviewDir = GetPreviewDir();
		CString PreviewContentPath = CPathTool::Cat(PreviewDir, _T("content2.tex"), true);
		bSuccess = bSuccess && WritePreviewText(PreviewContentPath);

		//TODO: inform the user!
		if (!bSuccess)
		{
			//Disable fast mode, so we do not run unsuccessfully forever
			CConfiguration::GetInstance()->m_bPreviewFastMode = false;
		}
	}	
	else
	{
		// Regular Mode ////////////////////////////////////////////////
		DoPreviewRun();
	}
}


bool COutputDoc::DoPreviewRun()
{
	//Get the working directory for the preview.
	const CString PreviewDir = GetPreviewDir();

	//Does it exist? Create it, if necessary.
	if (!CPathTool::Exists(PreviewDir))
	{
		//This generates a template from the main file, and copies generic templates.
		if (!CreatePreviewDir(PreviewDir, false, true, true)) return false;
	}

	//The following steps need to be all successful. If not, fast mode is disabled, and we leave here.
	bool bSuccess(true);

	//Kill a still running preview run.
	if (m_preview_builder.IsStillRunning())
	{
		CancelBuilds(false, true);
		bSuccess = bSuccess && !m_preview_builder.IsStillRunning();
	}

	if (CConfiguration::GetInstance()->m_bPreviewFastMode)
	{
		// Fast Mode ///////////////////////////////////////////////////
		bSuccess = bSuccess && PrepareFastPreview(PreviewDir);
	}	
	else
	{
		// Regular Mode ////////////////////////////////////////////////
		//Write the preview text to file
		CString PreviewContentPath = CPathTool::Cat(PreviewDir, _T("content.tex"), true);
		bSuccess = bSuccess && WritePreviewText(PreviewContentPath);
		LastWaitForFileChangePath.Empty();
	}

	//TODO: inform user on error! bSuccess
	if (!bSuccess)
	{
		//Always switch off fast mode in case of errors. We do not want to run unsuccessfully over and over again.
		CConfiguration::GetInstance()->m_bPreviewFastMode = false;
		return false;
	}

	//Get the selected template.
	CString strPreviewMainPath = CPathTool::Cat(PreviewDir,
		_T("Template ") + CConfiguration::GetInstance()->m_strPreviewTemplate + _T(".tex"),
		true);

	//Does the template actually exist? The config may even be empty.
	//Is any template selected? Force a scan. We do know that the preview directory exists, see above.
	if (!CPathTool::Exists(strPreviewMainPath))
	{
		//Let us scan based on the current settings.
		if (m_pPreviewImagePane) m_pPreviewImagePane->FillTemplateDropDown();

		//Construct the template name.
		strPreviewMainPath = CPathTool::Cat(PreviewDir,
			_T("Template ") + CConfiguration::GetInstance()->m_strPreviewTemplate + _T(".tex"),
			true);

		//Is the filename still invalid? Then give up.
		if (!CPathTool::Exists(strPreviewMainPath)) return false;
		//TODO: Inform user. They should scan for templates, create from main file, or manually add some. See manual.
	}

	//Reset old message settings
	//TODO: check necessity
	m_preview_builder.MsgsAfterTermination.ClearAllMessages();

	//Messages to open tool windows, provide feedback, and update content after preview run.
	CWnd* pMainWnd = AfxGetMainWnd();
	if (pMainWnd && m_pPreviewImagePane)
	{
		//PostMessage(pMainWnd->m_hWnd, AfxUserMessages::ShowDockingBarID, ID_VIEW_PREVIEW_IMAGE_PANE, 0);
		PostMessage(m_pPreviewImagePane->m_hWnd, AfxUserMessages::PreviewImageViewStartProgressAnimation, 0, 0);

		//After building the preview, the PreviewView needs to reload the image.
		//We do this regardless of success.
		//TODO: Consider displaying the success of the preview generation in the preview window.
		//The builder could send the termination code.
		m_preview_builder.MsgsAfterTermination.AddMessage(true, m_pPreviewImagePane->m_hWnd, AfxUserMessages::PreviewImageViewUpdate, 0, 0, false, 0);
		m_preview_builder.MsgsAfterTermination.AddMessage(true, m_pPreviewImagePane->m_hWnd, AfxUserMessages::PreviewImageViewStopProgressAnimation, 0, 0, false, 0);
	}

	//Build the preview
	//For previews, we do not scan the errors, warnings, etc.
	//We also do not activate the output.
	//m_builder.BuildPreview(this, m_pPreviewView, PreviewDir, strPreviewMainPath);
	return m_preview_builder.BuildPreview(this, m_pPreviewView, PreviewDir, strPreviewMainPath);
}
