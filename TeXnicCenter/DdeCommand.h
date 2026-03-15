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

#pragma once


#include "RegistryStack.h"
#include "PlaceHolder.h"


/**
Default DDE Timeout. 10 sec. is enough to start AR 6 on a
P III 500 MHz with 256 MB
 */
#define DDE_DEFTIMEOUT 10000

/**
Class for storing properties of an DDE command and to execute DDE
commands.

This class can be used to pass simple execution commands to a
DDE-server.

@author Sven Wiegand
 */
class CDdeCommand
{
// construction/destruction
public:
	CDdeCommand();
	virtual ~CDdeCommand();

// attribute operations
public:
	/**
	Sets the full path of the executable, that implements the
	dde-server to communicate with.

	Pass an empty string, to reset the executable-setting.
	 */
	void SetExecutable(LPCTSTR lpszPath);

	/**
	Sets the DDE server to send the command to and the topic the
	command belongs to.
	 */
	void SetServer(LPCTSTR lpszServerName, LPCTSTR lpszTopic = _T("System"));

	/**
	Command string to send to the server. The string may contain
	placeholders as specified by AfxExpandPlaceholders.

	@see AfxExpandPlaceholders
	 */
	void SetCommand(LPCTSTR lpszCmd);

	CString GetExecutable() const;
	CString GetServerName() const;
	CString GetTopic() const;
	CString GetCommand() const;

// operations
public:
	/**
	Tries to connect to the DDE-server and sends the execution command.

	Before the command is send, the placeholder in the command string
	will be expanded by using the appropriate parts of the given
	information.

	If SetExecutable() has been called before with a non-empty string
	and the first try to send the command fails, the method will try
	to start the specified executable and try to send the command
	again.

	@param PInfo
	        Information for expanding placeholders.

	@return
	        TRUE on success, FALSE oterhwise.
	 */
	BOOL SendCommand(const CPlaceholderInfo& PInfo) const;

	/**
	Removes the directory specifications from all path specifications
	of this command.

	This is usefull when exporting profiles.
	 */
	void RemoveDirectorySpecifications();

// static operations
public:
	/**
	Tries to connect to the specified DDE-server using the given topic
	and sends the execution command.

	@param lpszServer
	        The name of the DDE-server to send the command to.
	@param lpszCommand
	        Command to send.
	@param lpszTopic
	        Topic to communicate on.
	@param lpszExecutable
	        Path to the executable that implements the specified DDE-server.
	        If this parameter is specified and the initialization of the
	        connection fails, the method will start the specified executable
	        and try again to send the command.

	@return
	        TRUE on success, FALSE oterhwise.
	 */
	static BOOL SendCommand(LPCTSTR lpszServer, LPCTSTR lpszCommand, LPCTSTR lpszTopic = _T("System"), LPCTSTR lpszExecutable = NULL, DWORD dwTimeOut = DDE_DEFTIMEOUT);

// static implementation helpers
protected:
	static BOOL SendCommandHelper(LPCTSTR lpszServer, LPCTSTR lpszCommand, LPCTSTR lpszTopic = _T("System"), LPCTSTR lpszExecutable = NULL, DWORD dwTimeOut = DDE_DEFTIMEOUT);
	static BOOL Connect(LPCTSTR lpszExecutable, DWORD dwTimeOut, int inst, HSZ serv, HSZ top, HCONV *conv);

// string serialization
public:
	/**
	Packs the information of this command into one string and returns
	this string.
	 */
	CString SerializeToStringDeprecated() const;

	/**
	Sets the attributes with the information received from the given
	string.

	@return
	        TRUE on success, FALSE if the given string does not have the
	        right format.
	 */
	BOOL SerializeFromStringDeprecated(LPCTSTR lpszPackedInformation);

	///Writes the command to the registry
	bool SerializeToRegistry(const CString& ValueBaseName, RegistryStack& reg) const;

	///Reads the command from the registry
	bool SerializeFromRegistry(const CString& ValueBaseName, RegistryStack& reg);

	/**
	Saves the command properties to the specified XML element.

	@exception CComException
	 */
	void SaveXml(MsXml::CXMLDOMElement xmlCommand) const;

	/**
	Loads the command properties from the specified XML element.
	 */
	void LoadXml(MsXml::CXMLDOMElement xmlCommand);

// attributes
protected:
	/** Name of the DDE-server to send commands to */
	CString m_strServerName;

	/** Topic to talk about with the server */
	CString m_strTopic;

	/** Command to send to the server */
	CString m_strCmd;

	/** Full Path of the application to communicate with */
	CString m_strExecutable;
};


// inlines

inline
CString CDdeCommand::GetExecutable() const
{
	return m_strExecutable;
}

inline
CString CDdeCommand::GetServerName() const
{
	return m_strServerName;
}

inline
CString CDdeCommand::GetTopic() const
{
	return m_strTopic;
}

inline
CString CDdeCommand::GetCommand() const
{
	return m_strCmd;
}

