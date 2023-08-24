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

#if !defined(AFX_PROFILEPAGEPPROCESSOR_H__5C552766_082E_11D5_A222_006097239934__INCLUDED_)
#define AFX_PROFILEPAGEPPROCESSOR_H__5C552766_082E_11D5_A222_006097239934__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "ProfilePage.h"
#include "BrowseButton.h"
#include "PersistPosEdit.h"
#include "EditMenuButton.h"
#include "EditMenuButtonOpt.h"
#include "NotifyingEditListBox.h"

/**	Edits the processors of output profiles.

@ingroup dialogs
@ingroup profiles

@author Sven Wiegand
 */
class CProfilePagePprocessor : public CProfilePage
{
// construction/destruction
public:
	CProfilePagePprocessor(UINT unIDTemplate, UINT unIDCaption, CPProcessorArray& (CProfile::*GetPProcessorArray)());

// implementation
protected:
	void UpdateControlStates();
	void OnAddItem(int nIndex);
	BOOL OnRemoveItem(int nIndex);
	void OnSelectionChanged();
	void OnRenameItem(int nIndex);
	void OnMoveItemUp(int nIndex);
	void OnMoveItemDown(int nIndex);
	void RefillList();

// overridings
public:
	void OnUpdateDataSet(CProfile *pProfile);

//{{AFX_VIRTUAL(CProfilePagePprocessor)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// message handlers
protected:
	afx_msg void OnItemAction(NMHDR *pNMHDR, LRESULT *pResult);

	//{{AFX_MSG(CProfilePagePprocessor)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

// dialog field data
protected:
	//{{AFX_DATA(CProfilePagePprocessor)
	enum
	{
		IDD = IDD_PROFILE_PPROCESSOR
	};

	CEdit m_wndExe;
	CStatic m_wndTitleArguments;
	CStatic m_wndTitleOutput;
	CStatic m_wndTitleInput;
	CStatic m_wndTitleExe;
	NotifyingEditListBox m_wndList;
	CEditMenuButtonOpt m_wndOutputPh;
	CEditMenuButtonOpt m_wndInputPh;
	CEditMenuButtonOpt m_wndArgumentsPh;
	CPersistPosEdit m_wndOutput;
	CPersistPosEdit m_wndInput;
	CPersistPosEdit m_wndArguments;
	CBrowseButton m_wndBrowsePprocessor;
	//}}AFX_DATA

// attributes
protected:
	CProfile *m_pProfile;
	CPProcessorArray& (CProfile::*m_pGetPProcessorArray)();

	int m_nSelectedItem;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_PROFILEPAGEPPROCESSOR_H__5C552766_082E_11D5_A222_006097239934__INCLUDED_
