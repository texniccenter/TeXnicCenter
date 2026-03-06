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
#include "OptionPagePath.h"
#include "configuration.h"
#include "global.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-------------------------------------------------------------------
// class COptionPagePath
//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(COptionPagePath,CMFCPropertyPage)

BEGIN_MESSAGE_MAP(COptionPagePath,CMFCPropertyPage)
	//{{AFX_MSG_MAP(COptionPagePath)
	ON_BN_CLICKED(IDC_ADD_PROJECTTEMPLATES,OnAddProjectTemplates)
	ON_BN_CLICKED(IDC_REMOVE_PROJECTTEMPLATES,OnRemoveProjectTemplates)
	ON_LBN_SELCHANGE(IDC_PROJECTTEMPLATES,OnSelchangeProjectTemplates)

	ON_BN_CLICKED(IDC_ADD_DOCUMENTTEMPLATES,OnAddDocumentTemplates)
	ON_BN_CLICKED(IDC_REMOVE_DOCUMENTTEMPLATES,OnRemoveDocumentTemplates)
	ON_LBN_SELCHANGE(IDC_DOCUMENTTEMPLATES,OnSelchangeDocumentTemplates)

	ON_BN_CLICKED(IDC_ADD_PREVIEWTEMPLATES,OnAddPreviewTemplates)
	ON_BN_CLICKED(IDC_REMOVE_PREVIEWTEMPLATES,OnRemovePreviewTemplates)
	ON_LBN_SELCHANGE(IDC_PREVIEWTEMPLATES,OnSelchangePreviewTemplates)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


COptionPagePath::COptionPagePath()
		: CMFCPropertyPage(COptionPagePath::IDD),
		m_wndBrowseBtn(IDC_OPTIONS_DEFAULT_PATH_EDIT,CString((LPCTSTR)STE_GET_PATH))
{
	//{{AFX_DATA_INIT(COptionPagePath)
	//}}AFX_DATA_INIT
	m_strDefaultPath = CConfiguration::GetInstance()->m_strDefaultPath;
}

COptionPagePath::~COptionPagePath()
{
}

void COptionPagePath::DoDataExchange(CDataExchange* pDX)
{
	CMFCPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionPagePath)
	DDX_Control(pDX,IDC_REMOVE_PROJECTTEMPLATES,m_wndProjectRemoveButton);
	DDX_Control(pDX,IDC_PROJECTTEMPLATES,m_wndProjectTemplateList);

	DDX_Control(pDX,IDC_REMOVE_DOCUMENTTEMPLATES,m_wndDocumentRemoveButton);
	DDX_Control(pDX,IDC_DOCUMENTTEMPLATES,m_wndDocumentTemplateList);

	DDX_Control(pDX,IDC_REMOVE_PREVIEWTEMPLATES,m_wndPreviewRemoveButton);
	DDX_Control(pDX,IDC_PREVIEWTEMPLATES,m_wndPreviewTemplateList);

	DDX_Control(pDX,IDC_BROWSE_DEFAULT_PATH,m_wndBrowseBtn);
	DDX_Text(pDX,IDC_OPTIONS_DEFAULT_PATH_EDIT,m_strDefaultPath);
	//}}AFX_DATA_MAP
}

BOOL COptionPagePath::OnInitDialog()
{
	CMFCPropertyPage::OnInitDialog();

	// fill template list
	for (int i = 0; i < CConfiguration::GetInstance()->m_astrProjectTemplatePaths.GetSize(); i++)
	{
		m_wndProjectTemplateList.AddString(CConfiguration::GetInstance()->m_astrProjectTemplatePaths[i]);
	}

	for (int i = 0; i < CConfiguration::GetInstance()->m_astrDocumentTemplatePaths.GetSize(); i++)
	{
		m_wndDocumentTemplateList.AddString(CConfiguration::GetInstance()->m_astrDocumentTemplatePaths[i]);
	}

	for (int i = 0; i < CConfiguration::GetInstance()->m_astrPreviewTemplatePaths.GetSize(); i++)
	{
		m_wndPreviewTemplateList.AddString(CConfiguration::GetInstance()->m_astrPreviewTemplatePaths[i]);
	}

	// disable remove button
	m_wndProjectRemoveButton.EnableWindow(FALSE);
	m_wndDocumentRemoveButton.EnableWindow(FALSE);
	m_wndPreviewRemoveButton.EnableWindow(FALSE);

	return TRUE;
}

void COptionPagePath::OnAddProjectTemplates()
{
	CString InitFolder;
	if (m_wndProjectTemplateList.GetCount() > 0)
	{
		m_wndProjectTemplateList.GetText(m_wndProjectTemplateList.GetCount() - 1, InitFolder);
	}

	CFolderSelect dlg(AfxLoadString(STE_GET_PATH), InitFolder.IsEmpty() ? NULL : InitFolder);

	if (dlg.DoModal() == IDOK)
	{
		m_wndProjectTemplateList.AddString(dlg.GetPath());
	}
}

void COptionPagePath::OnRemoveProjectTemplates()
{
	int nItem = m_wndProjectTemplateList.GetCurSel();

	if (nItem != LB_ERR)
		m_wndProjectTemplateList.DeleteString(nItem);
}

void COptionPagePath::OnSelchangeProjectTemplates()
{
	int nIndex = m_wndProjectTemplateList.GetCurSel();

	m_wndProjectRemoveButton.EnableWindow(nIndex != LB_ERR);
}

void COptionPagePath::OnAddDocumentTemplates()
{
	CString InitFolder;
	if (m_wndDocumentTemplateList.GetCount() > 0)
	{
		m_wndDocumentTemplateList.GetText(m_wndDocumentTemplateList.GetCount() - 1, InitFolder);
	}

	CFolderSelect dlg(AfxLoadString(STE_GET_PATH), InitFolder.IsEmpty() ? NULL : InitFolder);

	if (dlg.DoModal() == IDOK)
	{
		m_wndDocumentTemplateList.AddString(dlg.GetPath());
	}
}

void COptionPagePath::OnRemoveDocumentTemplates()
{
	int nItem = m_wndDocumentTemplateList.GetCurSel();

	if (nItem != LB_ERR)
		m_wndDocumentTemplateList.DeleteString(nItem);
}

void COptionPagePath::OnSelchangeDocumentTemplates()
{
	int nIndex = m_wndDocumentTemplateList.GetCurSel();

	m_wndDocumentRemoveButton.EnableWindow(nIndex != LB_ERR);
}

void COptionPagePath::OnAddPreviewTemplates()
{
	CString InitFolder;
	if (m_wndPreviewTemplateList.GetCount() > 0)
	{
		m_wndPreviewTemplateList.GetText(m_wndPreviewTemplateList.GetCount() - 1, InitFolder);
	}

	CFolderSelect dlg(AfxLoadString(STE_GET_PATH), InitFolder.IsEmpty() ? NULL : InitFolder);

	if (dlg.DoModal() == IDOK)
	{
		m_wndPreviewTemplateList.AddString(dlg.GetPath());
	}
}

void COptionPagePath::OnRemovePreviewTemplates()
{
	int nItem = m_wndPreviewTemplateList.GetCurSel();

	if (nItem != LB_ERR)
		m_wndPreviewTemplateList.DeleteString(nItem);
}

void COptionPagePath::OnSelchangePreviewTemplates()
{
	int nIndex = m_wndPreviewTemplateList.GetCurSel();

	m_wndPreviewRemoveButton.EnableWindow(nIndex != LB_ERR);
}

void COptionPagePath::OnOK()
{
	UpdateData();

	// Store settings to configuration
	CString strElement;

	CConfiguration::GetInstance()->m_astrProjectTemplatePaths.RemoveAll();
	for (int i = 0; i < m_wndProjectTemplateList.GetCount(); i++)
	{
		m_wndProjectTemplateList.GetText(i,strElement);
		CConfiguration::GetInstance()->m_astrProjectTemplatePaths.Add(strElement);
	}

	CConfiguration::GetInstance()->m_astrDocumentTemplatePaths.RemoveAll();
	for (int i = 0; i < m_wndDocumentTemplateList.GetCount(); i++)
	{
		m_wndDocumentTemplateList.GetText(i,strElement);
		CConfiguration::GetInstance()->m_astrDocumentTemplatePaths.Add(strElement);
	}

	CConfiguration::GetInstance()->m_astrPreviewTemplatePaths.RemoveAll();
	for (int i = 0; i < m_wndPreviewTemplateList.GetCount(); i++)
	{
		m_wndPreviewTemplateList.GetText(i,strElement);
		CConfiguration::GetInstance()->m_astrPreviewTemplatePaths.Add(strElement);
	}

	//Kill leading and ending spaces
	m_strDefaultPath.TrimLeft();
	m_strDefaultPath.TrimRight();
	//Update Default Path
	CConfiguration::GetInstance()->m_strDefaultPath = m_strDefaultPath;

	CMFCPropertyPage::OnOK();
}
