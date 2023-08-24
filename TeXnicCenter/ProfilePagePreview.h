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


#include "ProfilePagePprocessor.h"

/**	Edits the processors for previewing parts of the text.

	@ingroup dialogs
	@ingroup profiles

	@author Tino Weinkauf
*/
class CProfilePagePreview : public CProfilePagePprocessor
{
// construction/destruction
public:
	CProfilePagePreview(UINT unIDTemplate, CPProcessorArray& (CProfile::*GetPProcessorArray)());

// implementation
protected:

// overridings
public:
	void OnUpdateDataSet(CProfile* pProfile);
	BOOL ApplyChanges();

//{{AFX_VIRTUAL(CProfilePagePreview)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// message handlers
protected:
	//{{AFX_MSG(CProfilePagePreview)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

// dialog field data
protected:
	//{{AFX_DATA(CProfilePagePreview)
	enum
	{
		IDD = IDD_PROFILE_PREVIEW
	};

	CEditMenuButtonOpt m_wndPreviewPh;
	CPersistPosEdit m_wndPreview;
	CString m_strPreviewImagePath;

	//}}AFX_DATA

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.
