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
#include "TeXnicCenter.h"


//Indices in the IDB_PREVIEW for status images for the refresh button
#define BTNREFRESH_NORMAL 0
#define BTNREFRESH_WORKING 3
#define BTNREFRESH_SUCCESS 4
#define BTNREFRESH_ERROR 5

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

	ON_UPDATE_COMMAND_UI(ID_PREVIEW_AUTO_ENTER, &PreviewImagePane::OnUpdateAutoBuildOnEnter)
	ON_COMMAND(ID_PREVIEW_AUTO_ENTER, &PreviewImagePane::OnAutoBuildOnEnter)

	ON_UPDATE_COMMAND_UI(ID_PREVIEW_AUTO_SAVE, &PreviewImagePane::OnUpdateAutoBuildOnSave)
	ON_COMMAND(ID_PREVIEW_AUTO_SAVE, &PreviewImagePane::OnAutoBuildOnSave)

	ON_UPDATE_COMMAND_UI(ID_PREVIEW_CANCEL, &PreviewImagePane::OnUpdateCancel)
	ON_COMMAND(ID_PREVIEW_CANCEL, &PreviewImagePane::OnCancel)

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

	ON_UPDATE_COMMAND_UI_RANGE(ID_PREVIEW_DPI_AUTO, ID_PREVIEW_DPI_1200, &PreviewImagePane::OnUpdateDPI)
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

	//The last buttons are differently colored referesh buttons to communicate status to the user
	Toolbar.RemoveButton(BTNREFRESH_ERROR);
	Toolbar.RemoveButton(BTNREFRESH_SUCCESS);
	Toolbar.RemoveButton(BTNREFRESH_WORKING);

	//Replace AutoGeneration button with menu on the toolbar
	CMenu AutoGenMenu;
	AutoGenMenu.LoadMenu(IDR_PREVIEW_OPTIONSMENU);	
	CMFCToolBarMenuButton AutoGenMenuButton(~0U, AutoGenMenu.GetSubMenu(0)->GetSafeHmenu(), 1);
	AutoGenMenuButton.SetMessageWnd(this);
	Toolbar.ReplaceButton(ID_PREVIEW_OPTIONS, AutoGenMenuButton);

	//Replace Template Button with a drop down list for selecting the template
	CMFCToolBarComboBoxButton TemplateList(ID_PREVIEW_TEMPLATE_SELECT, 1, CBS_DROPDOWNLIST | CBS_SORT, 200);
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
	if (Toolbar.GetButton(BTNREFRESH_NORMAL))
	{
		Toolbar.GetButton(BTNREFRESH_NORMAL)->SetImage(BTNREFRESH_WORKING);
		Toolbar.InvalidateButton(BTNREFRESH_NORMAL);
	}

	return LRESULT();
}

LRESULT PreviewImagePane::StopProgress(WPARAM wParam, LPARAM /*lParam*/)
{
	//Error or Cancelled?
	const bool bError = (wParam != 0);

	//Set image on refresh button to indicate success and status
	if (Toolbar.GetButton(BTNREFRESH_NORMAL))
	{
		int nImageID = BTNREFRESH_SUCCESS;
		if (bError)
		{
			nImageID = (wParam == 1) ? BTNREFRESH_NORMAL : BTNREFRESH_ERROR; //User Cancel vs. Build Error
		}

		Toolbar.GetButton(BTNREFRESH_NORMAL)->SetImage(nImageID);
		Toolbar.InvalidateButton(BTNREFRESH_NORMAL);
	}

	//Restart right away, if in fast mode
	if (!bError && CConfiguration::GetInstance()->m_bPreviewFastMode)
	{
		CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
		if (pMainFrame && pMainFrame->GetOutputDoc())
		{
			//This is the one call in the system where fast mode renews itself, i.e., starts a new run
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
	//Cancel an ongoing fast-mode run, otherwise it will wait forever
	CancelPreviewBuild(true);

	//Switch mode
	CConfiguration::GetInstance()->m_bPreviewFastMode = !CConfiguration::GetInstance()->m_bPreviewFastMode;
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


void PreviewImagePane::OnUpdateCancel(CCmdUI* pCmdUI)
{
	bool bEnable(false);

	CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	if (pMainFrame && pMainFrame->GetOutputDoc())
	{
		bEnable = pMainFrame->GetOutputDoc()->IsPreviewRunning();
	}

	pCmdUI->Enable(bEnable);
}

void PreviewImagePane::OnCancel()
{
	CancelPreviewBuild(false);
}


void PreviewImagePane::OnTemplateEdit()
{
	CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	if (pMainFrame && pMainFrame->GetOutputDoc())
	{
		CString strPreviewMainPath = pMainFrame->GetOutputDoc()->GetPreviewTemplatePath();
		theApp.OpenLatexDocument(strPreviewMainPath, false, -1, false, false, true);
	}
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

void PreviewImagePane::OnUpdateDPI(CCmdUI* pCmdUI)
{
	//Convert from the enum to the resource ID. Works only if resource IDs are continuous and AUTO is first.
	const unsigned int nModeID = (unsigned int)CConfiguration::GetInstance()->m_nPreviewDPISetting + ID_PREVIEW_DPI_AUTO;

	pCmdUI->SetRadio(pCmdUI->m_nID == nModeID);
}

void PreviewImagePane::OnDPI(UINT nID)
{
	auto pConfig = CConfiguration::GetInstance();

	switch (nID)
	{
		case ID_PREVIEW_DPI_AUTO:
		default:
		{
			pConfig->m_nPreviewDPISetting = (int)DPISettings::Auto;
			break;
		}
		case ID_PREVIEW_DPI_75:
		{
			pConfig->m_nPreviewDPISetting = (int)DPISettings::DPI75;
			break;
		}
		case ID_PREVIEW_DPI_150:
		{
			pConfig->m_nPreviewDPISetting = (int)DPISettings::DPI150;
			break;
		}
		case ID_PREVIEW_DPI_300:
		{
			pConfig->m_nPreviewDPISetting = (int)DPISettings::DPI300;
			break;
		}
		case ID_PREVIEW_DPI_600:
		{
			pConfig->m_nPreviewDPISetting = (int)DPISettings::DPI600;
			break;
		}
		case ID_PREVIEW_DPI_900:
		{
			pConfig->m_nPreviewDPISetting = (int)DPISettings::DPI900;
			break;
		}
		case ID_PREVIEW_DPI_1200:
		{
			pConfig->m_nPreviewDPISetting = (int)DPISettings::DPI1200;
			break;
		}
	}

	//Set the actual value in a different function, which we can call from other places as well.
	SetDPIValue();
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

	//Actually anything new?
	LPCTSTR NewName = pTemplateList->GetItem();
	if (NewName && NewName != CConfiguration::GetInstance()->m_strPreviewTemplate)
	{
		//Set the currently selected template as active? If none, then the string is empty.
		CConfiguration::GetInstance()->m_strPreviewTemplate = NewName;

		//Changing the template requires us to terminate the preview build.
		//We can restrict this to fast mode here.
		CancelPreviewBuild(true);
	}
}

void PreviewImagePane::FillTemplateDropDown()
{
	//Get the dropdown list
	CMFCToolBarComboBoxButton* pTemplateList = GetTemplateDropDown();
	if (!pTemplateList) return;

	//Which template is currently selected? If none, then the string is empty.
	CString strPreviousSelection(pTemplateList->GetItem());
	//If empty, then we get the previously selected name from the config
	strPreviousSelection = CConfiguration::GetInstance()->m_strPreviewTemplate;

	//Get the path to the preview folder
	CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	if (!pMainFrame || !pMainFrame->GetOutputDoc()) return;
	const CString PreviewDir = pMainFrame->GetOutputDoc()->GetPreviewDir();

	//Clear the drop down
	pTemplateList->RemoveAllItems();

	//Get all template files in the preview folder
	CFileFind finder;
	BOOL bWorking = finder.FindFile(CPathTool::Cat(PreviewDir, _T("Template *.tex"), true));
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (!finder.IsDirectory())
		{
			//Remove common parts from name "Template *.tex"
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


void PreviewImagePane::CancelPreviewBuild(const bool bOnlyWhenFastMode)
{
	if (!bOnlyWhenFastMode || CConfiguration::GetInstance()->m_bPreviewFastMode)
	{
		CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
		if (pMainFrame && pMainFrame->GetOutputDoc())
		{
			pMainFrame->GetOutputDoc()->CancelBuilds(false, true);
		}
	}
}

void PreviewImagePane::SetDPIValue()
{
	auto pConfig = CConfiguration::GetInstance();

	switch (pConfig->m_nPreviewDPISetting)
	{
		case DPISettings::Auto:
		default:
		{
			pConfig->m_nPreviewDPIValue = View.BestDPI;
			break;
		}
		case DPISettings::DPI75:
		{
			pConfig->m_nPreviewDPIValue = 75;
			break;
		}
		case DPISettings::DPI150:
		{
			pConfig->m_nPreviewDPIValue = 150;
			break;
		}
		case DPISettings::DPI300:
		{
			pConfig->m_nPreviewDPIValue = 300;
			break;
		}
		case DPISettings::DPI600:
		{
			pConfig->m_nPreviewDPIValue = 600;
			break;
		}
		case DPISettings::DPI900:
		{
			pConfig->m_nPreviewDPIValue = 900;
			break;
		}
		case DPISettings::DPI1200:
		{
			pConfig->m_nPreviewDPIValue = 1200;
			break;
		}
	}
}
