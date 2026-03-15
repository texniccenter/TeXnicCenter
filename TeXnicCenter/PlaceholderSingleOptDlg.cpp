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
#include "PlaceholderSingleOptDlg.h"
#include "Placeholder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlaceholderSingleOptDlg dialog

CPlaceholderSingleOptDlg::CPlaceholderSingleOptDlg(CWnd* pParent /*=NULL*/)
		: CDialog(CPlaceholderSingleOptDlg::IDD,pParent)
{
	//{{AFX_DATA_INIT(CPlaceholderSingleOptDlg)
	m_bForwardSlash = FALSE;
	m_bShortFormat = FALSE;
	//}}AFX_DATA_INIT

	m_bAllowOptions = true;
}

void CPlaceholderSingleOptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlaceholderSingleOptDlg)
	DDX_Control(pDX,IDC_PLACEHOLDER_SINGLE_SHORT,m_ShortBtn);
	DDX_Control(pDX,IDC_PLACEHOLDER_SINGLE_FORWARD,m_ForwardBtn);
	DDX_Control(pDX,IDC_PLACEHOLDER_SINGLE_PHTYPE,m_PHTypeCombo);
	DDX_Control(pDX,IDC_PLACEHOLDER_SINGLE_WHAT,m_WhatStatic);
	DDX_Control(pDX,IDC_PLACEHOLDER_SINGLE_RESULT,m_ResultStatic);
	DDX_Control(pDX,IDC_PLACEHOLDER_SINGLE_EXAMPLE,m_ExampleEdit);
	DDX_Check(pDX,IDC_PLACEHOLDER_SINGLE_FORWARD,m_bForwardSlash);
	DDX_Check(pDX,IDC_PLACEHOLDER_SINGLE_SHORT,m_bShortFormat);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPlaceholderSingleOptDlg,CDialog)
	//{{AFX_MSG_MAP(CPlaceholderSingleOptDlg)
	ON_BN_CLICKED(IDC_PLACEHOLDER_SINGLE_FORWARD,ShowExample)
	ON_BN_CLICKED(IDC_PLACEHOLDER_SINGLE_SHORT,ShowExample)
	ON_CBN_SELCHANGE(IDC_PLACEHOLDER_SINGLE_PHTYPE,OnPHTypeChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlaceholderSingleOptDlg message handlers

void CPlaceholderSingleOptDlg::OnOK()
{
	ConstructPlaceholder();

	CDialog::OnOK();
}

void CPlaceholderSingleOptDlg::OnCancel()
{
	//The user pressed cancel, so we will return nothing

	strPlaceholder = "";

	CDialog::OnCancel();
}

void CPlaceholderSingleOptDlg::SetPlaceholderBase(LPCTSTR lpszPHBase)
{
	ASSERT(lpszPHBase);

	m_strPlaceholderBase = lpszPHBase;
	ASSERT(CorrectPlaceholderBase());

	return;
}

void CPlaceholderSingleOptDlg::SetAllowOptions(bool bEnable /*= true*/)
{
	m_bAllowOptions = bEnable;
}

void CPlaceholderSingleOptDlg::ConstructPlaceholder()
{
	if (!CorrectPlaceholderBase())
	{
		strPlaceholder = "";
		return;
	}

	UpdateData(true);

	//Get type of reference
	CString strPHTypes = _T("pbwdnter");
	int nPHType = m_PHTypeCombo.GetCurSel();
	if ((nPHType < 0) || (nPHType > 7))
	{
		strPlaceholder = "";
		return;
	}

	//Get the possible options for this type of reference
	bool bForward,bShort;
	GetPossibleOptions(&bForward,&bShort);

	if (m_bForwardSlash && bForward)
	{
		strPHTypes.MakeUpper();
	}

	//Construct
	strPlaceholder.Format(_T("%%%s%c%s"),
	                      (m_bShortFormat && bShort) ? _T("s") : _T(""),
	                      strPHTypes[nPHType],
	                      (LPCTSTR)m_strPlaceholderBase);
}

BOOL CPlaceholderSingleOptDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	strPlaceholder.Empty();

	//Load and show description
	CString strDescr;

	if (m_strPlaceholderBase == _T("m"))
	{
		strDescr.LoadString(STE_MAINFILE_DESCRIPTION);
	}
	else if (m_strPlaceholderBase == _T("c"))
	{
		strDescr.LoadString(STE_CURRENTDOC_DESCRIPTION);
	}
	else ASSERT(false); //Should never happen; Error in Ressource

	m_WhatStatic.SetWindowText(strDescr);

	//Activate the first entry
	m_PHTypeCombo.SetCurSel(0);

	//Update Options
	m_ForwardBtn.EnableWindow(m_bAllowOptions);
	m_ShortBtn.EnableWindow(m_bAllowOptions);

	//Init the example
	ShowExample();

	return TRUE;
}

bool CPlaceholderSingleOptDlg::CorrectPlaceholderBase()
{
	if (m_strPlaceholderBase.GetLength() != 1)
		return false;

	if (
	    (m_strPlaceholderBase != _T("m"))
	    && (m_strPlaceholderBase != _T("c"))
	)
		return false;

	return true;
}

void CPlaceholderSingleOptDlg::ShowExample()
{
	//Get the placeholder
	ConstructPlaceholder();
	//Show it to the user
	m_ResultStatic.SetWindowText(strPlaceholder);

	/////////////////////////////////
	//Construct a real world example
	CPlaceholderInfo PInfo;

	//Problem: For constructing a short path using AfxExpandPlaceholders(),
	// the file needs to exist.
	// So we take a different (=short) MainFileName in this case.
	if (m_bShortFormat)
	{
		PInfo.strMainPath.LoadString(STE_PLACEHOLDER_EXAMPLE_MAIN_SHORT);
		PInfo.strCurrentPath.LoadString(STE_PLACEHOLDER_EXAMPLE_CURRENT_SHORT);
		PInfo.strPreviewTemplatePath.LoadString(STE_PLACEHOLDER_EXAMPLE_PREVIEW_SHORT);
	}
	else
	{
		PInfo.strMainPath.LoadString(STE_PLACEHOLDER_EXAMPLE_MAIN);
		PInfo.strCurrentPath.LoadString(STE_PLACEHOLDER_EXAMPLE_CURRENT);
		PInfo.strPreviewTemplatePath.LoadString(STE_PLACEHOLDER_EXAMPLE_PREVIEW);
	}
	PInfo.strWorkingDir = CPathTool::GetDirectory(CPathTool::GetDirectory(PInfo.strMainPath));

	CString strExampleFileName;
	try
	{
		strExampleFileName = AfxExpandPlaceholders(strPlaceholder, PInfo);
		//Show this example
		m_ExampleEdit.SetWindowText(strExampleFileName);
	}
	catch (CMemoryException* e)
	{
		e->ReportError();
		delete e;
	}
}

void CPlaceholderSingleOptDlg::OnPHTypeChange()
{
	//We need to disable some options for some types of Placeholders
	bool bForward,bShort;

	GetPossibleOptions(&bForward,&bShort);
	m_ForwardBtn.EnableWindow(bForward);
	m_ShortBtn.EnableWindow(bShort);

	ShowExample();
}

void CPlaceholderSingleOptDlg::GetPossibleOptions(bool* pbForward,bool* pbShort)
{
	UpdateData(true);

	if (!m_bAllowOptions)
	{
		//Options are not allowed: Just say no.
		*pbForward = false;
		*pbShort = false;
		return;
	}

	//Get the Type of Placeholder
	int nPHType = m_PHTypeCombo.GetCurSel();
	if ((nPHType < 0) || (nPHType > 6))
	{
		return;
	}

	switch (nPHType)
	{
		default:
			*pbForward = true;
			*pbShort = true;
			break;

		case 3: //Name (with extension)
			*pbForward = false;
			*pbShort = true;
			break;

		case 4: //Title (without extension)
			*pbForward = false;
			*pbShort = true;
			break;

		case 5: //Extension (without leading point)
			*pbForward = false;
			*pbShort = true;
			break;

		case 6: //Drive letter followed by colon (':')
			*pbForward = false;
			*pbShort = false;
			break;
	}
}
