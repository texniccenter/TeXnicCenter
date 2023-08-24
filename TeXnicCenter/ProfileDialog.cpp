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
#include "ProfileDialog.h"
#include "ProfileNameDialog.h"
#include "OutputWizard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-------------------------------------------------------------------
// class CProfileDialog
//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CProfileDialog, CPropertyDialog)
	//{{AFX_MSG_MAP(CProfileDialog)
	ON_BN_CLICKED(IDC_PROFILE_ADD, OnProfileAdd)
	ON_BN_CLICKED(IDC_PROFILE_REMOVE, OnProfileRemove)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PROFILES, OnItemChanged)
	ON_BN_CLICKED(IDC_PROFILE_COPY, OnProfileCopy)
	ON_BN_CLICKED(IDC_PROFILE_RENAME, OnProfileRename)
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_LIST_PROFILES, OnItemChanging)
	ON_BN_CLICKED(IDC_PROFILE_WIZARD, OnProfileWizard)
	ON_BN_CLICKED(IDC_PROFILE_EXPORT, OnProfileExport)
	ON_BN_CLICKED(IDC_PROFILE_IMPORT, OnProfileImport)
	//}}AFX_MSG_MAP
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_PROFILES, &CProfileDialog::OnLvnEndlabeleditListProfiles)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_PROFILES, &CProfileDialog::OnLvnKeydownListProfiles)
END_MESSAGE_MAP()


CProfileDialog::CProfileDialog(CWnd* pParent /*=NULL*/)
		: CPropertyDialog(CProfileDialog::IDD, IDC_TAB_PROFILEPAGES, pParent),
		m_wndPagePreprocessor(IDD_PROFILE_PPROCESSOR, IDS_PREPROCESSOR, &CProfile::GetPreProcessorArray),
		m_wndPagePostprocessor(IDD_PROFILE_PPROCESSOR, IDS_POSTPROCESSOR, &CProfile::GetPostProcessorArray),
		m_wndPagePreview(IDD_PROFILE_PREVIEW, &CProfile::GetPreviewProcessorArray),
		m_pCurrentProfile(NULL)
{
	AddPage(&m_wndPageLatex);
	AddPage(&m_wndPagePreprocessor);
	AddPage(&m_wndPagePostprocessor);
	AddPage(&m_wndPageViewer);
	AddPage(&m_wndPagePreview);

	// copy global profile-map to local profile map
	POSITION pos = CProfileMap::GetInstance()->GetStartPosition();
	CProfile *pProfile;
	CString strKey;

	while (pos)
	{
		CProfileMap::GetInstance()->GetNextAssoc(pos, strKey, pProfile);
		m_profiles.Add(strKey, *pProfile);
	}

	m_profiles.SetActiveProfile(CProfileMap::GetInstance()->GetActiveProfileKey(), false);

	//{{AFX_DATA_INIT(CProfileDialog)
	//}}AFX_DATA_INIT
}

void CProfileDialog::InternalSortList()
{
	/*
	BOOL			bChanged = FALSE;
	int				nCount = m_wndProfileList.GetCount();
	CString		str, str2;

	for (int i = 0; i < (nCount-1); i++)
	{
	        str = m_wndProfileList.GetItemText(i);
	        str2 = m_wndProfileList.GetItemText(i+1);

	        if (str > str2)
	        {
	                DWORD	dwData = m_wndProfileList.GetItemData(i);
	                m_wndProfileList.RemoveItem(i);
	                m_wndProfileList.AddItem(str, dwData, i+1);

	                bChanged = TRUE;
	        }
	}

	if (bChanged)
	        InternalSortList();
	 */
}

void CProfileDialog::RefillList()
{
	UnselectCurrentItem();

	// empty list
	m_wndProfileList.DeleteAllItems();

	// fill list with profiles
	POSITION pos = m_profiles.GetStartPosition();
	CString strKey;
	CProfile *pProfile;
	while (pos)
	{
		m_profiles.GetNextAssoc(pos, strKey, pProfile);
		m_wndProfileList.InsertItem(-1, strKey);
	}

	UpdateControlStates();
}

void CProfileDialog::SortList()
{
	SetRedraw(FALSE);
	InternalSortList();
	SetRedraw();
}

BOOL CProfileDialog::ApplyChanges()
{
	try
	{
		if (!m_wndPageLatex.ApplyChanges())
			throw 0;
		if (!m_wndPagePreprocessor.ApplyChanges())
			throw 1;
		if (!m_wndPagePostprocessor.ApplyChanges())
			throw 2;
		if (!m_wndPageViewer.ApplyChanges())
			throw 3;
		if (!m_wndPagePreview.ApplyChanges())
			throw 4;
	}
	catch (int nPage)
	{
		ActivatePage(nPage);
		AfxMessageBox(STE_PROFILE_UNCOMPLETE, MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	return TRUE;
}

BOOL CProfileDialog::OnSelectionChanging()
{
	if (!m_pCurrentProfile)
		return TRUE;

	if (ApplyChanges())
		return TRUE;
	else
		return FALSE;
}

void CProfileDialog::OnSelectionChanged()
{
	CProfile* pProfile = NULL;
	CString strProfile = _T("");
	const int nIndex = m_wndProfileList.GetNextItem(-1, LVNI_SELECTED);

	if (nIndex >= 0)
	{
		strProfile = m_wndProfileList.GetItemText(nIndex, 0);
		if (!strProfile.IsEmpty()) m_profiles.Lookup(strProfile, pProfile);
	}

	m_wndPageLatex.OnUpdateDataSet(pProfile);
	m_wndPagePreprocessor.OnUpdateDataSet(pProfile);
	m_wndPagePostprocessor.OnUpdateDataSet(pProfile);
	m_wndPageViewer.OnUpdateDataSet(pProfile);
	m_wndPagePreview.OnUpdateDataSet(pProfile);

	m_pCurrentProfile = pProfile;
	m_profiles.SetActiveProfile(strProfile);

	UpdateControlStates();
}

BOOL CProfileDialog::ValidateLabel(LPCTSTR lpszLabel)
{
	const CString strLabel(lpszLabel);
	return !m_profiles.Exists(strLabel);
}

void CProfileDialog::UnselectCurrentItem()
{
	int nIndex = m_wndProfileList.GetNextItem(-1, LVNI_SELECTED);
	if (nIndex > -1)
		m_wndProfileList.SetItemState(nIndex, 0, LVIS_SELECTED);
}


//Selects the item in the list, which represents the active output profile.

void CProfileDialog::SelectActiveProfile()
{
	LVFINDINFO FindInfo;

	//Init the FindInfo structure
	FindInfo.flags = LVFI_STRING;
	CString strActiveProfile = m_profiles.GetActiveProfileKey();
	FindInfo.psz = strActiveProfile.GetBuffer(1);

	//Find it
	int nIndex = m_wndProfileList.FindItem(&FindInfo);

	//Select it
	if (nIndex != -1)
	{
		//Now select
		m_wndProfileList.SetItemState(nIndex, LVIS_SELECTED, LVIS_SELECTED);
	}
}

void CProfileDialog::UpdateControlStates()
{
	if (m_pCurrentProfile)
	{
		m_wndCopyButton.EnableWindow();
		m_wndRenameButton.EnableWindow();
		m_wndRemoveButton.EnableWindow();
	}
	else
	{
		m_wndCopyButton.EnableWindow(FALSE);
		m_wndRenameButton.EnableWindow(FALSE);
		m_wndRemoveButton.EnableWindow(FALSE);
	}
}

void CProfileDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProfileDialog)
	DDX_Control(pDX, IDC_PROFILE_RENAME, m_wndRenameButton);
	DDX_Control(pDX, IDC_PROFILE_REMOVE, m_wndRemoveButton);
	DDX_Control(pDX, IDC_PROFILE_COPY, m_wndCopyButton);
	DDX_Control(pDX, IDC_LIST_PROFILES, m_wndProfileList);
	//}}AFX_DATA_MAP
}

BOOL CProfileDialog::OnInitDialog()
{
	CPropertyDialog::OnInitDialog();

	::SetWindowTheme(m_wndProfileList,L"explorer",0);

	// fill list with profiles
	RefillList();

	//Select the active output profile
	SelectActiveProfile();

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CProfileDialog::OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pnmv = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	if (pnmv->uChanged == LVIF_STATE &&
	        (pnmv->uOldState & LVIS_SELECTED) != (pnmv->uNewState & LVIS_SELECTED))
	{
		/*
		If we prevent the change, the message will be send four times
		and therefore the dialog box will be showed five times.

		To avoid this, we are using this counter.
		 */
		static int nSelChangeFailCount = 0;

		if (!nSelChangeFailCount)
		{
			*pResult = !OnSelectionChanging();
			if (*pResult)
				nSelChangeFailCount = 4;
		}
		else
		{
			--nSelChangeFailCount;
			*pResult = TRUE;
		}
	}
}

void CProfileDialog::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pnmv = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	if (pnmv->uChanged == LVIF_STATE &&
	        (pnmv->uOldState & LVIS_SELECTED) != (pnmv->uNewState & LVIS_SELECTED))
	{
		OnSelectionChanged();
	}
}

void CProfileDialog::OnProfileAdd()
{
	CString strName;
	BOOL bValidLabel = FALSE;

	while (!bValidLabel)
	{
		CProfileNameDialog dlg(strName);

		if (dlg.DoModal() != IDOK)
			return;

		strName = dlg.GetName();
		bValidLabel = ValidateLabel(strName);
		if (!bValidLabel)
			AfxMessageBox(STE_PROFILE_ALREADYEXISTS, MB_OK | MB_ICONSTOP);
	}

	m_profiles.Add(strName, CProfile());
	int nIndex = m_wndProfileList.InsertItem(-1, strName);
	m_wndProfileList.SetItemState(nIndex, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
}

void CProfileDialog::OnProfileRemove()
{
	int nIndex = m_wndProfileList.GetNextItem(-1, LVNI_SELECTED);
	if (nIndex < 0)
		return;

	if (AfxMessageBox(STE_PROFILE_REMOVE, MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;

	CString strProfile = m_wndProfileList.GetItemText(nIndex, 0);
	UnselectCurrentItem();
	m_wndProfileList.DeleteItem(nIndex);
	m_profiles.Remove(strProfile);
}

void CProfileDialog::OnProfileCopy()
{
	int nIndex = m_wndProfileList.GetNextItem(-1, LVNI_SELECTED);
	if (nIndex < 0)
		return;

	if (!ApplyChanges())
		return;

	CString strName = m_wndProfileList.GetItemText(nIndex, 0);
	CProfile *pProfile;
	if (!m_profiles.Lookup(strName, pProfile))
		return;

	BOOL bValidLabel = FALSE;
	while (!bValidLabel)
	{
		CProfileNameDialog dlg(strName);

		if (dlg.DoModal() != IDOK)
			return;

		strName = dlg.GetName();
		bValidLabel = ValidateLabel(strName);
		if (!bValidLabel)
			AfxMessageBox(STE_PROFILE_ALREADYEXISTS, MB_OK | MB_ICONSTOP);
	}

	m_profiles.Add(strName, *pProfile);
	nIndex = m_wndProfileList.InsertItem(-1, strName);
	m_wndProfileList.SetItemState(nIndex, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
}

void CProfileDialog::OnProfileRename()
{
	int nIndex = m_wndProfileList.GetNextItem(-1, LVNI_SELECTED);
	if (nIndex < 0)
		return;

	CString strOldName = m_wndProfileList.GetItemText(nIndex, 0);
	CString strName(strOldName);
	BOOL bValidLabel = FALSE;
	while (!bValidLabel)
	{
		CProfileNameDialog dlg(strName);

		if (dlg.DoModal() != IDOK)
			return;

		strName = dlg.GetName();
		bValidLabel = ValidateLabel(strName);
		if (!bValidLabel)
			AfxMessageBox(STE_PROFILE_ALREADYEXISTS, MB_OK | MB_ICONSTOP);
	}

	m_profiles.Rename(strOldName, strName);
	m_wndProfileList.DeleteItem(nIndex);
	nIndex = m_wndProfileList.InsertItem(-1, strName);
	m_wndProfileList.SetItemState(nIndex, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
}

void CProfileDialog::OnOK()
{
	if (m_wndProfileList.GetNextItem(-1, LVNI_SELECTED) >= 0)
	{
		if (!ApplyChanges())
			return;
	}

	// copy local map to global map
	CProfileMap::GetInstance()->RemoveAll();

	POSITION pos = m_profiles.GetStartPosition();
	CString strKey;
	CProfile *pProfile;

	while (pos)
	{
		m_profiles.GetNextAssoc(pos, strKey, pProfile);
		CProfileMap::GetInstance()->Add(strKey, *pProfile);
	}
	CProfileMap::GetInstance()->SetActiveProfile(m_profiles.GetActiveProfileKey());

	CPropertyDialog::OnOK();
}

void CProfileDialog::OnProfileWizard()
{
	UnselectCurrentItem();

	// start wizard
	COutputWizard dlg(m_profiles, this);
	dlg.DoModal();
	RefillList();
}

void CProfileDialog::OnProfileExport()
{
	m_profiles.Export();
}

void CProfileDialog::OnProfileImport()
{
	// remember selected item
	int nSelectedItem = m_wndProfileList.GetSelectionMark();
	CString strSelectedItem = (nSelectedItem>-1) ? m_wndProfileList.GetItemText(nSelectedItem, 0) : _T("");

	// import items
	m_profiles.Import();

	RefillList();

	// try to select same item as before
	LVFINDINFO lvfi;
	ZeroMemory(&lvfi, sizeof(lvfi));
	lvfi.flags = LVFI_STRING;
	lvfi.psz = (LPCTSTR)strSelectedItem;
	nSelectedItem = m_wndProfileList.FindItem(&lvfi);
	if (nSelectedItem > -1)
		m_wndProfileList.SetItemState(nSelectedItem, LVIS_SELECTED, LVIS_SELECTED);
}

void CProfileDialog::OnLvnEndlabeleditListProfiles(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	if (pDispInfo->item.pszText) // User didn't cancel editing
	{
		CString strOldName = m_wndProfileList.GetItemText(pDispInfo->item.iItem,0);
		
		if (!ValidateLabel(pDispInfo->item.pszText)) 
		{
			AfxMessageBox(STE_PROFILE_ALREADYEXISTS, MB_OK | MB_ICONSTOP);
			*pResult = 0;
		}
		else 
		{
			m_profiles.Rename(strOldName, pDispInfo->item.pszText);
			*pResult = 1; // Success
		}
	}	
}

void CProfileDialog::OnLvnKeydownListProfiles(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	
	switch (pLVKeyDow->wVKey)
	{
	case VK_F2:
		{
			int item = m_wndProfileList.GetNextItem(-1,LVNI_ALL|LVNI_SELECTED);

			if (item != -1)
				m_wndProfileList.EditLabel(item);
		}
		break;
	}

	*pResult = 0;
}
