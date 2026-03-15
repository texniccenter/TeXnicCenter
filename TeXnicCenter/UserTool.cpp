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
#include "resource.h"
#include "PlaceHolder.h"
#include "global.h"
#include "UserTool.h"
#include "LaTeXView.h"
#include "LaTeXDocument.h"
#include "process.h"
#include "ProcessCommand.h"
#include "TeXnicCenter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define USERTOOLCURRENTFORMATVERSION 1

IMPLEMENT_SERIAL(UserTool, CUserTool, USERTOOLCURRENTFORMATVERSION | VERSIONABLE_SCHEMA)

#undef USERTOOLCURRENTFORMATVERSION

UserTool::UserTool()
		:	CUserTool(),
		m_bUseOutputWindow(false),
		m_bPromptForArguments(false),
		m_bCloseConsoleWindow(false),
		m_strInputFile(_T("")),
		m_strOutputFile(_T(""))
{}


UserTool::~UserTool()
{}


void UserTool::Serialize(CArchive& ar)
{
	CUserTool::Serialize(ar);

	if (ar.IsLoading())
	{
		m_bUseOutputWindow = false;
		m_bPromptForArguments = false;
		m_bCloseConsoleWindow = false;
		m_strInputFile.Empty();
		m_strOutputFile.Empty();
	}
}

CString UserTool::GetAdvDescription()
{
	CString retval(_T(""));
	CString strTemp;

	if (!m_strInputFile.IsEmpty())
		retval += _T("In: ") + m_strInputFile;

	if (m_bUseOutputWindow)
	{
		if (!retval.IsEmpty()) retval += _T(" | ");
		retval += _T("Out: Window");
	}
	else
	{
		if (!m_strOutputFile.IsEmpty())
		{
			if (!retval.IsEmpty()) retval += _T(" | ");
			retval += _T("Out: ") + m_strOutputFile;
		}

		if (m_bCloseConsoleWindow)
		{
			if (!retval.IsEmpty()) retval += _T(" | ");
			retval += _T("Close");
		}
	}

	if (m_bPromptForArguments)
	{
		if (!retval.IsEmpty()) retval += _T(" | ");
		retval += _T("Prompt");
	}

	return retval;
}


BOOL UserTool::Invoke()
{
	if (m_strCommand.IsEmpty())
	{
		AfxMessageBox(STE_TOOL_NOCMDLINE, MB_ICONINFORMATION|MB_OK);
		return FALSE;
	}

	//Get info for Placeholders
	CPlaceholderInfo PInfo;
	PInfo.FillWithInformation();

	try
	{
		//Be careful with the InitialDirectory: With CreateProcess it is not allowed
		// to be quoted or empty or elsewise non-valid.
		PInfo.bExpandPlaceholderSets = false;
		CString strInitialDirectory = AfxExpandPlaceholders(m_strInitialDirectory, PInfo);
		strInitialDirectory.TrimLeft(_T('\"'));
		strInitialDirectory.TrimRight(_T('\"'));

		//Valid directory?
		if (!CPathTool::Exists(strInitialDirectory))
		{
			//Get a valid one
			strInitialDirectory = AfxGetDefaultDirectory(true);
		};

		//Adjust placeholder info
		PInfo.strWorkingDir = strInitialDirectory;
		PInfo.bExpandPlaceholderSets = true;

		//Execute the tool
		CProcessCommand	pc;
		pc.Set(m_strCommand, m_strArguments); //Args will be expanded by Execute
		CProcess* p = pc.Execute(PInfo);
		if (!p)
		{
			TCHAR systemError[100];
			::FormatMessage(
			    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			    NULL,
			    pc.GetLastError(),
			    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			    systemError,
			    100,
			    NULL
			);

			CString strProcessResult;
			CString strExeArg(pc.GetExecutable() + _T(' ') + pc.GetArguments());
			strProcessResult.Format(STE_LATEX_START_FAILED_EXT, (LPCTSTR)strExeArg, systemError);
			AfxMessageBox(strProcessResult, MB_ICONSTOP|MB_OK);
			throw FALSE;
		}

		//Delete the process object
		delete p;
	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;
}
