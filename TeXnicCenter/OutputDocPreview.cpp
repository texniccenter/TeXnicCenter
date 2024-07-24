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
#include "PreviewImageView.h"

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


CString COutputDoc::GetPreviewGeneratedTemplateFileName() const
{
	return _T("Template ") + AfxLoadString(STE_PREVIEW_GENERATEDTEMPLATE) + _T(".tex");
}


bool COutputDoc::GetAllPreviewTemplates(std::vector<CString>& AllTemplates, int& idPreferred) const
{
	//Get the path to the preview folder
	CString PreviewDir = GetPreviewDir();

	//Get all template files in the preview folder
	AllTemplates.clear();
	idPreferred = -1;
	CFileFind finder;
	BOOL bWorking = finder.FindFile(CPathTool::Cat(PreviewDir, _T("Template*.tex"), true));
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (!finder.IsDirectory())
		{
			//Add this file to the list
			AllTemplates.push_back(finder.GetFileName());

			////Is this the preferred template?
			//if (finder.GetFileName() == )
			//{
			//	idPreferred = AllTemplates.size() - 1;
			//}
		}
	}
	
	return true;
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
		CString SearchText(_T("\\begin{document}"));
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


bool COutputDoc::CreatePreviewDir(const CString& PreviewDir, const bool bOverwrite)
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


	//Where are our templates?
	CString TemplateDir = CPathTool::Cat(theApp.GetWorkingDir(), _T("Templates\\Preview"));
	//if (!CPathTool::Exists(TemplateDir)) return false;

	//Copy all files from the templates directory to the preview directory
	CFileFind finder;
	CString strAccumulatedErrorMessages;
	bool bSomeSuccess(false);
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

	//Create template from main document
	bSomeSuccess = CreatePreviewTemplateFromMainFile(PreviewDir, bOverwrite);
	//if (!bSomeSuccess) <== we could display an error message.

	return bSomeSuccess;
}


void COutputDoc::OnBuildPreview()
{
	//TODO: Decide on whether this can be run for single active files.
	// Then see also the UI Update for this menu entry
	//m_pPreviewView
	CWnd* pMainWnd = AfxGetMainWnd();
	if (pMainWnd && m_pPreviewImageView)
	{
		PostMessage(pMainWnd->m_hWnd, AfxUserMessages::ShowDockingBarID, ID_VIEW_PREVIEW_IMAGE_PANE, 0);
		PostMessage(m_pPreviewImageView->m_hWnd, AfxUserMessages::PreviewImageViewStartProgressAnimation, 0, 0);

		//After building the preview, the PreviewView needs to reload the image.
		//We do this regardless of success.
		//TODO: Consider displaying the success of the preview generation in the preview window.
		//The builder could send the termination code.
		m_builder.MsgsAfterTermination.AddMessage(true, m_pPreviewImageView->m_hWnd, AfxUserMessages::PreviewImageViewUpdate, 0, 0, false, 0);
		m_builder.MsgsAfterTermination.AddMessage(true, m_pPreviewImageView->m_hWnd, AfxUserMessages::PreviewImageViewStopProgressAnimation, 0, 0, false, 0);
	}

	DoPreviewRun();
}


void COutputDoc::DoPreviewRun()
{
	if (m_builder.IsStillRunning()) return;

	//For previews, we do not scan the errors, warnings, etc.
	//We also do not activate the output.

	//Get the preview text. Check validity.
	CString PreviewText;
	CodeDocument* pPreviewDoc = NULL;
	const int nValidPreviewText = GetPreviewText(PreviewText, pPreviewDoc);
	if (nValidPreviewText < 1)
	{
		if (nValidPreviewText == 0) AfxMessageBox(STE_PREVIEW_NOSELECTION);
		return;
	}
	
	//Get the working directory for the preview.
	const CString PreviewDir = GetPreviewDir();

	//Does it exist? Create it, if necessary.
	if (!CPathTool::Exists(PreviewDir))
	{
		//This generates a template from the main file, and copies generic templates.
		if (!CreatePreviewDir(PreviewDir, false)) return;
	}

	//Write preview text to 'content.tex' in the preview folder; same encoding as original file
	CString PreviewContentPath = CPathTool::Cat(PreviewDir, _T("content.tex"), true);
	TextDocument td(pPreviewDoc); //ok to be NULL
	td.Write(PreviewContentPath, PreviewText);

	//TODO: We will have a menu with more than 1 template. The user can choose one, the choice will be written to the tps file
	// See also GetAllPreviewTemplates()
	CString strPreviewMainPath = CPathTool::Cat(PreviewDir, _T("Template Generated from Main File.tex"), true);

	// build the preview
	//m_builder.BuildPreview(NULL, NULL, /*this, m_pBuildView*/ PreviewDir, strPreviewMainPath);
	m_builder.BuildPreview(this, m_pPreviewView, PreviewDir, strPreviewMainPath);
}
