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

#if !defined(AFX_OPTIONPAGEPATH_H__FD645161_2E39_11D3_929E_FAB22A26316D__INCLUDED_)
#define AFX_OPTIONPAGEPATH_H__FD645161_2E39_11D3_929E_FAB22A26316D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BrowseButton.h"

/**	Options for Directory handling.

@ingroup dialogs

@author Sven Wiegand
@author Tino Weinkauf
 */
class COptionPagePath : public CMFCPropertyPage
{
	DECLARE_DYNCREATE(COptionPagePath)

// construction/destruction
public:
	COptionPagePath();
	~COptionPagePath();

// overridings
protected:
//{{AFX_VIRTUAL(COptionPagePath)
public:
	virtual void OnOK();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

// message handlers
protected:
	//{{AFX_MSG(COptionPagePath)
	afx_msg void OnAddProjectTemplates();
	afx_msg void OnRemoveProjectTemplates();
	afx_msg void OnSelchangeProjectTemplates();
	afx_msg void OnAddDocumentTemplates();
	afx_msg void OnRemoveDocumentTemplates();
	afx_msg void OnSelchangeDocumentTemplates();
	afx_msg void OnAddPreviewTemplates();
	afx_msg void OnRemovePreviewTemplates();
	afx_msg void OnSelchangePreviewTemplates();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

// dialog data
private:
	//{{AFX_DATA(COptionPagePath)
	enum
	{
		IDD = IDD_OPTIONS_PATHS
	};

	CButton m_wndProjectRemoveButton;
	CListBox m_wndProjectTemplateList;

	CButton m_wndDocumentRemoveButton;
	CListBox m_wndDocumentTemplateList;

	CButton m_wndPreviewRemoveButton;
	CListBox m_wndPreviewTemplateList;

	CBrowseButton m_wndBrowseBtn;
	CString m_strDefaultPath;
	//}}AFX_DATA
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_OPTIONPAGEPATH_H__FD645161_2E39_11D3_929E_FAB22A26316D__INCLUDED_
