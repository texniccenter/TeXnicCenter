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
#include "TextSourceFile.h"

#include <string>

#include "CodeDocument.h"

BOOL CTextSourceFile::Create(LPCTSTR lpszFile)
{
	CStringW text;
	TextDocument doc;

	bool result = doc.Read(lpszFile,text);

	if (result)
		is_.str(static_cast<LPCWSTR>(text));
	else
		is_.str(L"");

	return result;
}

void CTextSourceFile::Delete()
{
	delete this;
}

BOOL CTextSourceFile::GetNextLine(LPCTSTR &lpLine, int &nLength)
{
	std::wstring line;
	std::getline(is_, line);
	const bool result = (is_.rdstate() == std::ios_base::goodbit);

	if (result) {
		lpLine = m_strCurrentLine = line.c_str();
		nLength = m_strCurrentLine.GetLength();
	}

	return result;
}