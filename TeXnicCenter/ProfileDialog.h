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

#if !defined(AFX_PROFILEDIALOG_H__5C552764_082E_11D5_A222_006097239934__INCLUDED_)
#define AFX_PROFILEDIALOG_H__5C552764_082E_11D5_A222_006097239934__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropertyDialog.h"
#include "ProfilePage.h"
#include "ProfilePageLatex.h"
#include "ProfilePagePprocessor.h"
#include "ProfilePageViewer.h"
#include "ProfilePagePreview.h"

/**	Dialog for defining output profiles.

	@ingroup dialogs
	@ingroup profiles

	@see CProfilePageLaTeX
	@see CProfilePagePreprocessor
	@see CProfilePagePostprocessor
	@see CProfilePageViewer
	@see CProfilePagePreview

	@author Sven Wiegand
	@author Tino Weinkauf
 */
class CProfileDialog : public CPropertyDialog
{
// construction/destruction
public:
	CProfileDialog(CWnd* pParent = NULL);

// overridings
protected:
//{{AFX_VIRTUAL(CProfileDialog)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

// implementation
protected:
	/**
	Refills the list with the currently defined profiles.
	 */
	void RefillList();

	/**
	Sorts the elements in list with ascending titles.
	 */
	void SortList();

	/**
	Implements the recursive sort alorithm.
	 */
	void InternalSortList();

	/**
	Requests the pages to apply the made changes.

	@return
	        TRUE if all pages have applied their changes, FALSE if at least
	        one page failed.
	 */
	BOOL ApplyChanges();

	/**
	Checks the label of the specified item and returns TRUE if it is
	valid and FALSE otherwise.
	 */
	BOOL ValidateLabel(LPCTSTR lpszLabel);

	/**
	Unselects the current selected item in the list, if there is one.
	 */
	void UnselectCurrentItem();

	/**
	Selects the item in the list, which represents the active output profile.
	 */
	void SelectActiveProfile();

	BOOL OnSelectionChanging();
	void OnSelectionChanged();
	void UpdateControlStates();

// message handlers
protected:
	//{{AFX_MSG(CProfileDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnProfileAdd();
	afx_msg void OnProfileRemove();
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnProfileCopy();
	afx_msg void OnProfileRename();
	afx_msg void OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnProfileWizard();
	afx_msg void OnProfileExport();
	afx_msg void OnProfileImport();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

// dialog field data
public:
	//{{AFX_DATA(CProfileDialog)
	enum
	{
		IDD = IDD_PROFILE
	};
	CButton m_wndRenameButton;
	CButton m_wndRemoveButton;
	CButton m_wndCopyButton;
	CListCtrl m_wndProfileList;
	//}}AFX_DATA

// attributes
protected:
	CProfilePageLaTeX m_wndPageLatex;
	CProfilePagePprocessor m_wndPagePreprocessor;
	CProfilePagePprocessor m_wndPagePostprocessor;
	CProfilePageViewer m_wndPageViewer;
	CProfilePagePreview m_wndPagePreview;

	CProfileMap m_profiles;

	/**
	Pointer to the profile currently selected or NULL, if none is
	selected.
	 */
	CProfile *m_pCurrentProfile;
public:
	afx_msg void OnLvnEndlabeleditListProfiles(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnKeydownListProfiles(NMHDR *pNMHDR, LRESULT *pResult);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_PROFILEDIALOG_H__5C552764_082E_11D5_A222_006097239934__INCLUDED_
