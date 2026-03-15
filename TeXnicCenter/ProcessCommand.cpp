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
#include "process.h"
#include "ProcessCommand.h"
#include "RegistryStack.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//-------------------------------------------------------------------
// class CProcessCommand
//-------------------------------------------------------------------

CProcessCommand::CProcessCommand() : m_nLastError(0)
{
}

CProcessCommand::~CProcessCommand()
{
}

void CProcessCommand::Set(LPCTSTR lpszExecutable,LPCTSTR lpszArguments)
{
	m_strExecutable = lpszExecutable;
	m_strArguments = lpszArguments;
}

CProcess* CProcessCommand::Execute(const CPlaceholderInfo& PInfo) const
{
	CString strArguments = AfxExpandPlaceholders(m_strArguments, PInfo);
	CProcessCommand * const localThis = (CProcessCommand * const)this;
	CProcess* p = new CProcess;

	if (p->Create(m_strExecutable + _T(' ') + strArguments,
		NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, PInfo.strWorkingDir))
	{
		localThis->m_nLastError = 0;
		return p;
	}

	localThis->m_nLastError = ::GetLastError();

	delete p;
	return NULL;
}

CProcess *CProcessCommand::Execute(HANDLE hOutput, const CPlaceholderInfo& PInfo) const
{
	CString strArguments = AfxExpandPlaceholders(m_strArguments, PInfo);

	CProcessCommand * const localThis = (CProcessCommand * const)this;
	CProcess* p = new CProcess;

	if (p->CreateHiddenConsole(m_strExecutable + _T(' ') + strArguments,
							   INVALID_HANDLE_VALUE, hOutput, hOutput, CREATE_NO_WINDOW, PInfo.strWorkingDir))
	{
		localThis->m_nLastError = 0;
		return p;
	}

	localThis->m_nLastError = ::GetLastError();

	delete p;
	return NULL;
}

void CProcessCommand::RemoveDirectorySpecifications()
{
	m_strExecutable = CPathTool::GetFile(m_strExecutable);
}

CString CProcessCommand::SerializeToStringDeprecated() const
{
	return m_strExecutable + _T('\n') + m_strArguments;
}

BOOL CProcessCommand::SerializeFromStringDeprecated(LPCTSTR lpszPackedInformation)
{
	CString strExecutable;
	CString strArguments;
	if (!AfxExtractSubString(strExecutable,lpszPackedInformation,0,_T('\n')))
		return FALSE;
	if (!AfxExtractSubString(strArguments,lpszPackedInformation,1,_T('\n')))
		return FALSE;

	Set(strExecutable,strArguments);
	return TRUE;
}

bool CProcessCommand::SerializeToRegistry(const CString& ValueBaseName, RegistryStack& reg) const
{
	bool success(true);
	success &= (bool)reg.Write(ValueBaseName + _T("Executable"), m_strExecutable);
	success &= (bool)reg.Write(ValueBaseName + _T("Arguments"), m_strArguments);
	return success;
}

bool CProcessCommand::SerializeFromRegistry(const CString& ValueBaseName, RegistryStack& reg)
{
	bool success(true);
	success &= (bool)reg.Read(ValueBaseName + _T("Executable"), m_strExecutable);
	success &= (bool)reg.Read(ValueBaseName + _T("Arguments"), m_strArguments);
	return success;
}

void CProcessCommand::SaveXml(MsXml::CXMLDOMElement xmlCommand) const
{
	xmlCommand.SetAttribute(_T("path"),(LPCTSTR)m_strExecutable);
	xmlCommand.SetAttribute(_T("arguments"),(LPCTSTR)m_strArguments);
}

void CProcessCommand::LoadXml(MsXml::CXMLDOMElement xmlCommand)
{
	m_strExecutable = (LPCTSTR)(_bstr_t)xmlCommand.GetAttribute(_T("path"));
	m_strArguments = (LPCTSTR)(_bstr_t)xmlCommand.GetAttribute(_T("arguments"));
}