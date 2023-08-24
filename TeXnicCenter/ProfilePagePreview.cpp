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
#include "ProfilePagePreview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-------------------------------------------------------------------
// class CProfilePagePreview
//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CProfilePagePreview, CProfilePagePprocessor)
END_MESSAGE_MAP()


CProfilePagePreview::CProfilePagePreview(UINT unIDTemplate, CPProcessorArray& (CProfile::*GetPProcessorArray)())
	: CProfilePagePprocessor(unIDTemplate, 0, GetPProcessorArray)
	, m_wndPreviewPh(IDR_POPUP_PLACEHOLDER_MAIN_OPT)
{
	//{{AFX_DATA_INIT(CProfilePagePreview)
	//}}AFX_DATA_INIT
}


void CProfilePagePreview::OnUpdateDataSet(CProfile* pProfile)
{
	CProfilePagePprocessor::OnUpdateDataSet(pProfile);

	if (m_pProfile)
	{
		m_strPreviewImagePath = m_pProfile->GetPreviewImagePath();
	}

	if (IsWindow(m_hWnd))
	{
		UpdateData(FALSE);
	}
}


void CProfilePagePreview::DoDataExchange(CDataExchange* pDX)
{
	CProfilePagePprocessor::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CProfilePagePreview)
	//DDX_Control(pDX, IDC_TITLE_PREVIEW, m_wndTitlePreview);
	DDX_Control(pDX, IDC_INSERTPH_PREVIEW, m_wndPreviewPh);
	DDX_Control(pDX, IDC_EDIT_PREVIEW, m_wndPreview);
	DDX_Text(pDX, IDC_EDIT_PREVIEW, m_strPreviewImagePath);
	//}}AFX_DATA_MAP
}


BOOL CProfilePagePreview::OnInitDialog()
{
	CProfilePagePprocessor::OnInitDialog();

	// initialize controls
	m_wndPreviewPh.AttachEditCtrl(&m_wndPreview);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}


BOOL CProfilePagePreview::ApplyChanges()
{
	if (!m_pProfile) return TRUE;

	UpdateData();

	const bool bHasProcessors = (m_pProfile->GetPreviewProcessorArray().GetSize() > 0);

	//We report an error if the image path has not been set, but processors have.
	if (bHasProcessors && m_strPreviewImagePath.IsEmpty())
	{
		m_wndPreview.SetFocus();
		return FALSE;
	}
	m_pProfile->SetPreviewImagePath(m_strPreviewImagePath);

	return TRUE;
}
