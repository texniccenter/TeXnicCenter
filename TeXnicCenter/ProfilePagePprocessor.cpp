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
#include "resource.h"
#include "ProfilePagePprocessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-------------------------------------------------------------------
// class CProfilePagePprocessor
//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CProfilePagePprocessor, CProfilePage)
	ON_NOTIFY(BCGCELN_AFTERADDITEM, IDC_PPROCESSORLIST, OnItemAction)
	ON_NOTIFY(BCGCELN_BEFOREREMOVEITEM, IDC_PPROCESSORLIST, OnItemAction)
	ON_NOTIFY(BCGCELN_SELECTIONCHANGED, IDC_PPROCESSORLIST, OnItemAction)
	ON_NOTIFY(BCGCELN_AFTERRENAMEITEM, IDC_PPROCESSORLIST, OnItemAction)
	ON_NOTIFY(BCGCELN_AFTERMOVEITEMDOWN, IDC_PPROCESSORLIST, OnItemAction)
	ON_NOTIFY(BCGCELN_AFTERMOVEITEMUP, IDC_PPROCESSORLIST, OnItemAction)
END_MESSAGE_MAP()


CProfilePagePprocessor::CProfilePagePprocessor(UINT unIDTemplate, UINT unIDCaption, CPProcessorArray& (CProfile::*GetPProcessorArray)())
		: CProfilePage(unIDTemplate, unIDCaption),
		m_wndBrowsePprocessor(IDC_EDIT_PPEXE, TRUE, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, CString((LPCTSTR)STE_COMPILER_FILTER)),
		m_wndArgumentsPh(IDR_POPUP_PLACEHOLDER_MAIN_OPT),
		m_wndInputPh(IDR_POPUP_PLACEHOLDER_MAIN_OPT),
		m_wndOutputPh(IDR_POPUP_PLACEHOLDER_MAIN_OPT),
		m_nSelectedItem(-1),
		m_pProfile(NULL),
		m_pGetPProcessorArray(GetPProcessorArray)
{
	//{{AFX_DATA_INIT(CProfilePagePprocessor)
	//}}AFX_DATA_INIT
}

void CProfilePagePprocessor::UpdateControlStates()
{
	int nItem = m_nSelectedItem;

	if (m_pProfile)
		EnableAllControls();

	m_wndTitleExe.EnableWindow(nItem > -1);
	m_wndExe.EnableWindow(nItem > -1);
	m_wndBrowsePprocessor.EnableWindow(nItem > -1);
	m_wndTitleArguments.EnableWindow(nItem > -1);
	m_wndArguments.EnableWindow(nItem > -1);
	m_wndArgumentsPh.EnableWindow(nItem > -1);
	m_wndTitleInput.EnableWindow(nItem > -1);
	m_wndInput.EnableWindow(nItem > -1);
	m_wndInputPh.EnableWindow(nItem > -1);
	m_wndTitleOutput.EnableWindow(nItem > -1);
	m_wndOutput.EnableWindow(nItem > -1);
	m_wndOutputPh.EnableWindow(nItem > -1);

	if (!m_pProfile)
		EnableAllControls(FALSE);
}

void CProfilePagePprocessor::RefillList()
{
	if (!IsWindow(m_wndList.m_hWnd))
		return;

	// clear list
	while (m_wndList.GetCount())
		m_wndList.RemoveItem(0);

	if (!m_pProfile)
		return;

	// fill list
	for (int i = 0; i < (m_pProfile->*m_pGetPProcessorArray)().GetSize(); i++)
		m_wndList.AddItem((m_pProfile->*m_pGetPProcessorArray)().GetAt(i).GetTitle());
}

void CProfilePagePprocessor::OnUpdateDataSet(CProfile *pProfile)
{
	m_pProfile = pProfile;

	if (IsWindow(m_hWnd))
	{
		UpdateControlStates();
		RefillList();
	}
}

void CProfilePagePprocessor::OnAddItem(int nIndex)
{
	if (!m_pProfile)
		return;

	CPProcessor pp(m_wndList.GetItemText(nIndex));
	(m_pProfile->*m_pGetPProcessorArray)().InsertAt(nIndex,pp);
}

BOOL CProfilePagePprocessor::OnRemoveItem(int nIndex)
{
	if (!m_pProfile)
		return TRUE;

	(m_pProfile->*m_pGetPProcessorArray)().RemoveAt(nIndex);
	m_nSelectedItem = -1;
	return TRUE;
}

void CProfilePagePprocessor::OnSelectionChanged()
{
	UpdateData();
	m_nSelectedItem = m_wndList.GetSelItem();
	UpdateControlStates();
	UpdateData(FALSE);
}

void CProfilePagePprocessor::OnRenameItem(int nIndex)
{
	if (!m_pProfile)
		return;

	(m_pProfile->*m_pGetPProcessorArray)()[nIndex].SetTitle(m_wndList.GetItemText(nIndex));
}

void CProfilePagePprocessor::OnMoveItemUp(int nIndex)
{
	if (!m_pProfile)
		return;

	CPProcessor pp0 = (m_pProfile->*m_pGetPProcessorArray)()[nIndex];
	CPProcessor pp1 = (m_pProfile->*m_pGetPProcessorArray)()[nIndex + 1];
	(m_pProfile->*m_pGetPProcessorArray)().SetAt(nIndex, pp1);
	(m_pProfile->*m_pGetPProcessorArray)().SetAt(nIndex + 1, pp0);

	m_nSelectedItem = nIndex;
	UpdateData(FALSE);
}

void CProfilePagePprocessor::OnMoveItemDown(int nIndex)
{
	if (!m_pProfile)
		return;

	CPProcessor pp0 = (m_pProfile->*m_pGetPProcessorArray)()[nIndex - 1];
	CPProcessor pp1 = (m_pProfile->*m_pGetPProcessorArray)()[nIndex];
	(m_pProfile->*m_pGetPProcessorArray)().SetAt(nIndex - 1, pp1);
	(m_pProfile->*m_pGetPProcessorArray)().SetAt(nIndex, pp0);

	m_nSelectedItem = nIndex;
	UpdateData(FALSE);
}

void CProfilePagePprocessor::DoDataExchange(CDataExchange* pDX)
{
	CString strExecutable, strArguments, strInput, strOutput;
	int nPProcessor = m_nSelectedItem;

	if (m_pProfile && nPProcessor > -1 && nPProcessor < (m_pProfile->*m_pGetPProcessorArray)().GetSize() && !pDX->m_bSaveAndValidate)
	{
		// prepare data for controls
		CPProcessor &pp = (m_pProfile->*m_pGetPProcessorArray)()[nPProcessor];

		strExecutable = pp.GetPath();
		strArguments = pp.GetArguments();
		strInput = pp.GetInputFile();
		strOutput = pp.GetOutputFile();
	}

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProfilePagePprocessor)
	DDX_Control(pDX, IDC_EDIT_PPEXE, m_wndExe);
	DDX_Control(pDX, IDC_TITLE_ARGUMENTS, m_wndTitleArguments);
	DDX_Control(pDX, IDC_TITLE_OUTPUT, m_wndTitleOutput);
	DDX_Control(pDX, IDC_TITLE_INPUT, m_wndTitleInput);
	DDX_Control(pDX, IDC_TITLE_EXE, m_wndTitleExe);
	DDX_Control(pDX, IDC_PPROCESSORLIST, m_wndList);
	DDX_Control(pDX, IDC_INSERTPH_PPOUTPUT, m_wndOutputPh);
	DDX_Control(pDX, IDC_INSERTPH_PPINPUT, m_wndInputPh);
	DDX_Control(pDX, IDC_INSERTPH_PPARGUMENTS, m_wndArgumentsPh);
	DDX_Control(pDX, IDC_EDIT_PPOUTPUT, m_wndOutput);
	DDX_Control(pDX, IDC_EDIT_PPINPUT, m_wndInput);
	DDX_Control(pDX, IDC_EDIT_PPARGUMENTS, m_wndArguments);
	DDX_Control(pDX, IDC_BROWSE_PPROCESSOR, m_wndBrowsePprocessor);
	DDX_Text(pDX, IDC_EDIT_PPEXE, strExecutable);
	DDX_Text(pDX, IDC_EDIT_PPARGUMENTS, strArguments);
	DDX_Text(pDX, IDC_EDIT_PPINPUT, strInput);
	DDX_Text(pDX, IDC_EDIT_PPOUTPUT, strOutput);
	//}}AFX_DATA_MAP

	if (m_pProfile && nPProcessor > -1 && nPProcessor < (m_pProfile->*m_pGetPProcessorArray)().GetSize() && pDX->m_bSaveAndValidate)
	{
		// prepare data for controls
		CPProcessor &pp = (m_pProfile->*m_pGetPProcessorArray)()[nPProcessor];

		pp.SetPath(strExecutable);
		pp.SetArguments(strArguments);
		pp.SetInputFile(strInput);
		pp.SetOutputFile(strOutput);
	}
}

BOOL CProfilePagePprocessor::OnInitDialog()
{
	CProfilePage::OnInitDialog();

	// initialize controls
	m_wndList.SetStandardButtons();
	m_wndArgumentsPh.AttachEditCtrl(&m_wndArguments);
	m_wndInputPh.AttachEditCtrl(&m_wndInput);
	m_wndOutputPh.AttachEditCtrl(&m_wndOutput);

	// fill list
	if (m_pProfile)
		RefillList();

	UpdateControlStates();

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CProfilePagePprocessor::OnItemAction(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMBCGCEDITLISTBOX pHdr = (LPNMBCGCEDITLISTBOX)pNMHDR;

	*pResult = 0;

	switch (pHdr->hdr.code)
	{
		case BCGCELN_AFTERADDITEM:
			OnAddItem(pHdr->iItem);
			break;

		case BCGCELN_BEFOREREMOVEITEM:
			*pResult = !OnRemoveItem(pHdr->iItem);
			break;

		case BCGCELN_SELECTIONCHANGED:
			OnSelectionChanged();
			break;

		case BCGCELN_AFTERRENAMEITEM:
			OnRenameItem(pHdr->iItem);
			break;

		case BCGCELN_AFTERMOVEITEMUP:
			OnMoveItemUp(pHdr->iItem);
			break;

		case BCGCELN_AFTERMOVEITEMDOWN:
			OnMoveItemDown(pHdr->iItem);
			break;

		default:
			ASSERT(FALSE);
	}
}
