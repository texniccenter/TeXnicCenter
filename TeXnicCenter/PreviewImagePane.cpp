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
#include "PreviewImagePane.h"
#include "mainfrm.h"
#include "OutputDoc.h"
#include "global.h"
#include "resource.h"
#include "configuration.h"

IMPLEMENT_DYNAMIC(PreviewImagePane, WorkspacePane)

PreviewImagePane::PreviewImagePane()
{
}

//////////////////// MESSAGE MAP ////////////////////

BEGIN_MESSAGE_MAP(PreviewImagePane, WorkspacePane)
	ON_WM_CREATE()
	ON_MESSAGE(AfxUserMessages::PreviewImageViewUpdate, &PreviewImagePane::Update)
	ON_MESSAGE(AfxUserMessages::PreviewImageViewStartProgressAnimation, &PreviewImagePane::StartProgress)
	ON_MESSAGE(AfxUserMessages::PreviewImageViewStopProgressAnimation, &PreviewImagePane::StopProgress)
	ON_UPDATE_COMMAND_UI(ID_PREVIEW_FASTMODE, &PreviewImagePane::OnUpdateFastMode)
	ON_COMMAND(ID_PREVIEW_FASTMODE, &PreviewImagePane::OnFastMode)

	ON_COMMAND(ID_PREVIEW_AUTO_ENTER, &PreviewImagePane::OnAutoBuildOnEnter)
	ON_UPDATE_COMMAND_UI(ID_PREVIEW_AUTO_ENTER, &PreviewImagePane::OnUpdateAutoBuildOnEnter)

	ON_UPDATE_COMMAND_UI(ID_PREVIEW_AUTO_SAVE, &PreviewImagePane::OnUpdateAutoBuildOnSave)
	ON_COMMAND(ID_PREVIEW_AUTO_SAVE, &PreviewImagePane::OnAutoBuildOnSave)

	ON_COMMAND(ID_PREVIEW_TEMPLATE_EDIT, &PreviewImagePane::OnTemplateEdit)
	ON_COMMAND(ID_PREVIEW_TEMPLATE_CREATE, &PreviewImagePane::OnTemplateCreate)
	ON_COMMAND(ID_PREVIEW_TEMPLATE_SCAN, &PreviewImagePane::OnTemplateScan)

	ON_COMMAND(ID_PREVIEW_ZOOM_IN, &PreviewImagePane::OnZoomIn)
	ON_COMMAND(ID_PREVIEW_ZOOM_OUT, &PreviewImagePane::OnZoomOut)
	ON_COMMAND(ID_PREVIEW_ZOOM_FIT, &PreviewImagePane::OnZoomFit)
	//ON_UPDATE_COMMAND_UI(ID_PREVIEW_ZOOM_IN, &PreviewImagePane::OnZoomUpdateUI)
	ON_UPDATE_COMMAND_UI(ID_PREVIEW_ZOOM_OUT, &PreviewImagePane::OnUpdateZoomOut)
	ON_UPDATE_COMMAND_UI(ID_PREVIEW_ZOOM_FIT, &PreviewImagePane::OnUpdateZoomFit)
	ON_COMMAND(ID_PREVIEW_REFRESH, &PreviewImagePane::OnRefresh)

	ON_COMMAND_RANGE(ID_PREVIEW_DPI_AUTO, ID_PREVIEW_DPI_1200, &PreviewImagePane::OnDPI)
	ON_UPDATE_COMMAND_UI(ID_PREVIEW_TEMPLATE_SELECT, &PreviewImagePane::OnUpdateTemplateSelect)
	ON_COMMAND(ID_PREVIEW_TEMPLATE_SELECT, &PreviewImagePane::OnTemplateSelect)
	ON_CBN_SELCHANGE(ID_PREVIEW_TEMPLATE_SELECT, &PreviewImagePane::OnTemplateSelect)
END_MESSAGE_MAP()



int PreviewImagePane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (WorkspacePane::OnCreate(lpCreateStruct) == -1) return -1;

	//Create and set our client area
	CRect rectDummy;
	rectDummy.SetRectEmpty();
	if (!View.Create(rectDummy, this))
	{
		TRACE0("Failed to create preview image view\n");
		return false;
	}
	SetClient(&View);

	//Init toolbar
	SetupToolBar(Toolbar, IDR_PREVIEW);

	//Replace AutoGeneration button with menu on the toolbar
	CMenu AutoGenMenu;
	AutoGenMenu.LoadMenu(IDR_PREVIEW_OPTIONSMENU);	
	CMFCToolBarMenuButton AutoGenMenuButton(~0U, AutoGenMenu.GetSubMenu(0)->GetSafeHmenu(), 1);
	AutoGenMenuButton.SetMessageWnd(this);
	Toolbar.ReplaceButton(ID_PREVIEW_OPTIONS, AutoGenMenuButton);

	//Replace Template Button with a drop down list for selecting the template
	CMFCToolBarComboBoxButton TemplateList(ID_PREVIEW_TEMPLATE_SELECT, 1, CBS_DROPDOWNLIST | CBS_SORT, 200);
	//TemplateList.SetMessageWnd(this);
	Toolbar.ReplaceButton(ID_PREVIEW_TEMPLATE_SELECT, TemplateList);
	//Add templates to combo box
	FillTemplateDropDown();

	return 0;
}


CMFCToolBarComboBoxButton* PreviewImagePane::GetTemplateDropDown()
{
	const int NumButtons = Toolbar.GetCount();
	for(int i(0);i<NumButtons;i++)
	{
		CMFCToolBarButton* pButton = Toolbar.GetButton(i);
		if (pButton && pButton->m_nID == ID_PREVIEW_TEMPLATE_SELECT)
		{
			return dynamic_cast<CMFCToolBarComboBoxButton*>(pButton);
		}
	}

	return NULL;
}


LRESULT PreviewImagePane::Update(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	//Invalidate the view to force a redraw.
	View.Invalidate();

	//Get image path and load it
	CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	if (pMainFrame && pMainFrame->GetOutputDoc())
	{
		CString Path = pMainFrame->GetOutputDoc()->GetPreviewImagePath();
		if (!CPathTool::Exists(Path)) return 0L;

		View.PreviewImage.Destroy();
		HRESULT res = View.PreviewImage.Load(Path);
		if (FAILED(res)) return 0L;
	}

	return 0L;
}


LRESULT PreviewImagePane::StartProgress(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return LRESULT();
}

LRESULT PreviewImagePane::StopProgress(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	//Restart right away, if in fast mode
	if (CConfiguration::GetInstance()->m_bPreviewFastMode)
	{
		CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
		if (pMainFrame && pMainFrame->GetOutputDoc())
		{
			pMainFrame->GetOutputDoc()->DoPreviewRun();
		}
	}

	return LRESULT();
}


void PreviewImagePane::OnRefresh()
{
	CWnd* pMainFrame = AfxGetMainWnd();
	if (pMainFrame) pMainFrame->PostMessage(WM_COMMAND, ID_BUILD_PREVIEW);
}


void PreviewImagePane::OnUpdateFastMode(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(CConfiguration::GetInstance()->m_bPreviewFastMode);
}

void PreviewImagePane::OnFastMode()
{
	//Switch mode
	CConfiguration::GetInstance()->m_bPreviewFastMode = !CConfiguration::GetInstance()->m_bPreviewFastMode;

	//TODO: I am certain we need to cancel an ongoing fast-mode run here.
	//Or we always write to both files 'content.tex' and 'content2.tex'
}


void PreviewImagePane::OnUpdateAutoBuildOnEnter(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(CConfiguration::GetInstance()->m_bPreviewAutoBuildOnEnter);
}

void PreviewImagePane::OnAutoBuildOnEnter()
{
	//Switch mode
	CConfiguration::GetInstance()->m_bPreviewAutoBuildOnEnter = !CConfiguration::GetInstance()->m_bPreviewAutoBuildOnEnter;
}

void PreviewImagePane::OnUpdateAutoBuildOnSave(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(CConfiguration::GetInstance()->m_bPreviewAutoBuildOnSave);
}

void PreviewImagePane::OnAutoBuildOnSave()
{
	//Switch mode
	CConfiguration::GetInstance()->m_bPreviewAutoBuildOnSave = !CConfiguration::GetInstance()->m_bPreviewAutoBuildOnSave;
}


void PreviewImagePane::OnTemplateEdit()
{
	//TODO: Implement
}

void PreviewImagePane::OnTemplateCreate()
{
	CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	if (pMainFrame && pMainFrame->GetOutputDoc())
	{
		COutputDoc* pDoc = pMainFrame->GetOutputDoc();
		const CString PreviewDir = pDoc->GetPreviewDir();
		pDoc->CreatePreviewDir(PreviewDir, true, false, true);
		//TODO: Error handling
		//Add new templates to drop down
		FillTemplateDropDown();
	}
}

void PreviewImagePane::OnTemplateScan()
{
	CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	if (pMainFrame && pMainFrame->GetOutputDoc())
	{
		COutputDoc* pDoc = pMainFrame->GetOutputDoc();
		const CString PreviewDir = pDoc->GetPreviewDir();
		pDoc->CreatePreviewDir(PreviewDir, false, true, false);
		//TODO: Error handling
		//Add new templates to drop down
		FillTemplateDropDown();
	}
}


void PreviewImagePane::OnZoomIn()
{
	View.Zoom(1);
}

void PreviewImagePane::OnUpdateZoomOut(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!View.bFitImageToWindow);
}

void PreviewImagePane::OnZoomOut()
{
	View.Zoom(-1);
}

void PreviewImagePane::OnUpdateZoomFit(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(View.bFitImageToWindow);
}

void PreviewImagePane::OnZoomFit()
{
	View.ZoomFit();
}


void PreviewImagePane::OnDPI(UINT nID)
{

//ID_PREVIEW_DPI_AUTO
//ID_PREVIEW_DPI_75
//ID_PREVIEW_DPI_150
//ID_PREVIEW_DPI_300
//ID_PREVIEW_DPI_600
//ID_PREVIEW_DPI_900
//ID_PREVIEW_DPI_1200        

}

void PreviewImagePane::OnUpdateTemplateSelect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
}

void PreviewImagePane::OnTemplateSelect()
{
	//Get the dropdown list
	CMFCToolBarComboBoxButton* pTemplateList = GetTemplateDropDown();
	if (!pTemplateList) return;

	//Which template is currently selected? If none, then the string is empty.
	CConfiguration::GetInstance()->m_strPreviewTemplate = pTemplateList->GetItem();
}

void PreviewImagePane::FillTemplateDropDown()
{
	//Get the dropdown list
	CMFCToolBarComboBoxButton* pTemplateList = GetTemplateDropDown();
	if (!pTemplateList) return;
	//auto Test = pTemplateList->GetParentWnd();

	//Which template is currently selected? If none, then the string is empty.
	CString strPreviousSelection(pTemplateList->GetItem());
	//TODO: Consider getting the previously selected name from the config / tps

	//Get the path to the preview folder
	CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	if (!pMainFrame || !pMainFrame->GetOutputDoc()) return;
	const CString PreviewDir = pMainFrame->GetOutputDoc()->GetPreviewDir();

	//Get all template files in the preview folder
	CFileFind finder;
	BOOL bWorking = finder.FindFile(CPathTool::Cat(PreviewDir, _T("Template *.tex"), true));
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (!finder.IsDirectory())
		{
			//Remove common parts from name "Template*.tex"
			CString Name = finder.GetFileName();
			if (Name.GetLength() > 13)
			{
				Name = Name.Mid(9, Name.GetLength() - 13);
			}

			//Add this file to the list
			pTemplateList->AddSortedItem(Name);
		}
	}

	//Try to select the previously selected entry
	//We have to do it manually, because the pButton->SelectItem() does not work correctly
	CComboBox* pComboBox = pTemplateList->GetComboBox();
	if (pComboBox && IsWindow(pComboBox->m_hWnd) && !pComboBox->GetDroppedState())
	{
		int nIndex = pComboBox->FindStringExact(-1, strPreviousSelection);
		if (nIndex < 0) nIndex = pComboBox->FindString(-1, AfxLoadString(STE_PREVIEW_GENERATEDTEMPLATE));
		if (nIndex < 0) nIndex = 0;
		pTemplateList->SelectItem(nIndex);
		pComboBox->SetCurSel(nIndex);
		pTemplateList->NotifyCommand(CBN_SELENDOK);
		pComboBox->GetParent()->UpdateWindow();
	}

	//Somehow this is not called through the above code.
	OnTemplateSelect();
}

