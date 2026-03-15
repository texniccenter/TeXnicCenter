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
#include "DdeCommand.h"
#include "PlaceHolder.h"
#include "Process.h"
#include "TeXnicCenter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

HDDEDATA CALLBACK CDdeCommand_DDEProcessMessage(UINT uType,// transaction type
        UINT uFmt,// clipboard data format
        HCONV hconv,// handle to the conversation
        HSZ hsz1,// handle to a string  HSZ hsz2,       // handle to a string
        HDDEDATA hdata,// handle to a global memory object
        DWORD dwData1,// transaction-specific data
        DWORD dwData2 // transaction-specific data
                                               )
{
	UNUSED_ALWAYS(uType);
	UNUSED_ALWAYS(uFmt);
	UNUSED_ALWAYS(hconv);
	UNUSED_ALWAYS(hsz1);
	UNUSED_ALWAYS(hdata);
	UNUSED_ALWAYS(dwData1);
	UNUSED_ALWAYS(dwData2);

	return NULL; //Not doing anything in particular!!!
}


//-------------------------------------------------------------------
// class CDdeCommand
//-------------------------------------------------------------------

CDdeCommand::CDdeCommand()
		: m_strTopic(_T("System"))
{
}

CDdeCommand::~CDdeCommand()
{
}

void CDdeCommand::SetExecutable(LPCTSTR lpszPath)
{
	m_strExecutable = lpszPath;
}

void CDdeCommand::SetServer(LPCTSTR lpszServerName,LPCTSTR lpszTopic /*= _T("System")*/)
{
	ASSERT(lpszServerName && lpszTopic);

	m_strServerName = lpszServerName;
	m_strTopic = lpszTopic;
}

void CDdeCommand::SetCommand(LPCTSTR lpszCmd)
{
	ASSERT(lpszCmd);

	m_strCmd = lpszCmd;
}

BOOL CDdeCommand::SendCommand(const CPlaceholderInfo& PInfo) const
{
	return SendCommandHelper(
	           m_strServerName,
	           AfxExpandPlaceholders(m_strCmd, PInfo),
	           m_strTopic,
	           m_strExecutable.IsEmpty() ? (LPCTSTR) NULL : (LPCTSTR) m_strExecutable,
	           theApp.GetProfileInt(_T("Settings"),_T("DdeConnectTimeOut"),DDE_DEFTIMEOUT));
}

BOOL CDdeCommand::SendCommand(LPCTSTR lpszServer,LPCTSTR lpszCommand,LPCTSTR lpszTopic /*= _T("System")*/,LPCTSTR lpszExecutable /*= NULL*/,DWORD dwTimeOut /*= DDE_DEFTIMEOUT*/)
{
	return SendCommandHelper(lpszServer,lpszCommand,lpszTopic,lpszExecutable,dwTimeOut);
}

void CDdeCommand::RemoveDirectorySpecifications()
{
	m_strExecutable = CPathTool::GetFile(m_strExecutable);
}

BOOL CDdeCommand::Connect(LPCTSTR lpszExecutable,DWORD dwTimeOut,int inst,HSZ serv,HSZ top,HCONV *conv)
{
	DWORD dwOverallTime = 0;

	*conv = DdeConnect(inst,serv,top,NULL);
	if (*conv == 0L && lpszExecutable)
	{
		// try to start the executable
		CProcess p;
		if (p.Create(lpszExecutable))
		{
			do
			{
				Sleep(10);
				dwOverallTime += 10;
				*conv = DdeConnect(inst,serv,top,NULL);
			}
			while ((*conv == 0L) && (dwOverallTime < dwTimeOut));
		}
	}
	return (*conv != 0L);
}

BOOL CDdeCommand::SendCommandHelper(LPCTSTR lpszServer,LPCTSTR lpszCommand,LPCTSTR lpszTopic /*= _T("System")*/,LPCTSTR lpszExecutable /*= NULL*/,DWORD dwTimeOut /*= DDE_DEFTIMEOUT*/)
{
	ASSERT(lpszServer && lpszCommand && lpszTopic);

	DWORD dwId = 0;

	if (DdeInitialize(&dwId,(PFNCALLBACK) CDdeCommand_DDEProcessMessage,APPCMD_CLIENTONLY,0) != DMLERR_NO_ERROR)
		return FALSE;

	HSZ hszServerName = NULL;
	HSZ hszTopic = NULL;
	HCONV hConversation = NULL;
	TCHAR *szCommand = NULL;

	try
	{
		// create handles
		hszServerName = DdeCreateStringHandle(dwId,lpszServer,0);
		if (!hszServerName)
			throw FALSE;
		hszTopic = DdeCreateStringHandle(dwId,lpszTopic,0);
		if (!hszTopic)
			throw FALSE;

		// establish conversation
		if (!Connect(lpszExecutable,dwTimeOut,dwId,hszServerName,hszTopic,&hConversation))
			throw FALSE;

		// ensure byte string
		size_t nLen = _tcslen(lpszCommand);
		szCommand = new TCHAR[nLen + 1];

		if (!szCommand)
			throw FALSE;

		_tcsncpy(szCommand,lpszCommand,nLen);

		// send command
		if (!DdeClientTransaction(reinterpret_cast<BYTE*>(szCommand),
			static_cast<DWORD>((nLen + 1) * sizeof(TCHAR)),
			hConversation,NULL,
#ifdef UNICODE
			CF_UNICODETEXT
#else
			CF_TEXT
#endif
			,XTYP_EXECUTE,dwTimeOut,NULL))
			throw FALSE;

		// clean up
		delete[] szCommand;

		DdeDisconnect(hConversation);
		DdeFreeStringHandle(dwId,hszServerName);
		DdeFreeStringHandle(dwId,hszTopic);
		//DdeUninitialize(dwId);
	}
	catch (...)
	{
		delete[] szCommand;
		if (hConversation)
			DdeDisconnect(hConversation);
		if (hszServerName)
			DdeFreeStringHandle(dwId,hszServerName);
		if (hszTopic)
			DdeFreeStringHandle(dwId,hszTopic);

		DdeUninitialize(dwId);

		return FALSE;
	}

	// successfull
	return TRUE;
}

CString CDdeCommand::SerializeToStringDeprecated() const
{
	return m_strServerName + _T('\n') + m_strTopic + _T('\n') + m_strCmd + _T('\n') + m_strExecutable;
}

BOOL CDdeCommand::SerializeFromStringDeprecated(LPCTSTR lpszPackedInformation)
{
	CString strServerName,strTopic,strCmd,strExecutable;
	if (!AfxExtractSubString(strServerName,lpszPackedInformation,0,_T('\n')))
		return FALSE;
	if (!AfxExtractSubString(strTopic,lpszPackedInformation,1,_T('\n')))
		return FALSE;
	if (!AfxExtractSubString(strCmd,lpszPackedInformation,2,_T('\n')))
		return FALSE;
	AfxExtractSubString(strExecutable,lpszPackedInformation,3,_T('\n'));

	SetExecutable(strExecutable);
	SetServer(strServerName,strTopic);
	SetCommand(strCmd);
	return TRUE;
}

bool CDdeCommand::SerializeToRegistry(const CString& ValueBaseName, RegistryStack& reg) const
{
	bool success(true);
	success &= (bool)reg.Write(ValueBaseName + _T("ServerName"), m_strServerName);
	success &= (bool)reg.Write(ValueBaseName + _T("Topic"), m_strTopic);
	success &= (bool)reg.Write(ValueBaseName + _T("Command"), m_strCmd);
	success &= (bool)reg.Write(ValueBaseName + _T("Executable"), m_strExecutable);
	return success;
}

bool CDdeCommand::SerializeFromRegistry(const CString& ValueBaseName, RegistryStack& reg)
{
	bool success(true);
	success &= (bool)reg.Read(ValueBaseName + _T("ServerName"), m_strServerName);
	success &= (bool)reg.Read(ValueBaseName + _T("Topic"), m_strTopic);
	success &= (bool)reg.Read(ValueBaseName + _T("Command"), m_strCmd);
	success &= (bool)reg.Read(ValueBaseName + _T("Executable"), m_strExecutable);
	return success;
}

void CDdeCommand::SaveXml(MsXml::CXMLDOMElement xmlCommand) const
{
	xmlCommand.SetAttribute(_T("path"),(LPCTSTR) m_strExecutable);
	xmlCommand.SetAttribute(_T("server"),(LPCTSTR) m_strServerName);
	xmlCommand.SetAttribute(_T("topic"),(LPCTSTR) m_strTopic);
	xmlCommand.SetAttribute(_T("command"),(LPCTSTR) m_strCmd);
}

void CDdeCommand::LoadXml(MsXml::CXMLDOMElement xmlCommand)
{
	m_strExecutable = (LPCTSTR)(_bstr_t) xmlCommand.GetAttribute(_T("path"));
	m_strServerName = (LPCTSTR)(_bstr_t) xmlCommand.GetAttribute(_T("server"));
	m_strTopic = (LPCTSTR)(_bstr_t) xmlCommand.GetAttribute(_T("topic"));
	m_strCmd = (LPCTSTR)(_bstr_t) xmlCommand.GetAttribute(_T("command"));
}
