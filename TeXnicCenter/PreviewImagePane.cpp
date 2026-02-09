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
	ON_COMMAND(ID_PREVIEW_TEMPLATE_EDIT, &PreviewImagePane::OnTemplateEdit)
	ON_COMMAND(ID_PREVIEW_ZOOM_IN, &PreviewImagePane::OnZoomIn)
	ON_COMMAND(ID_PREVIEW_ZOOM_FIT, &PreviewImagePane::OnZoomFit)
END_MESSAGE_MAP()


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
	return LRESULT();
}

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
	AutoGenMenu.LoadMenu(IDR_PREVIEW_AUTO);	
	CMFCToolBarMenuButton AutoGenMenuButton(~0U, AutoGenMenu.GetSubMenu(0)->GetSafeHmenu(), 0);
	Toolbar.ReplaceButton(ID_PREVIEW_AUTO, AutoGenMenuButton);

	//Replace DPI button with menu on the toolbar
	CMenu DPIMenu;
	DPIMenu.LoadMenu(IDR_PREVIEW_DPI);	
	CMFCToolBarMenuButton DPIMenuButton(~0U, DPIMenu.GetSubMenu(0)->GetSafeHmenu(), 0);
	Toolbar.ReplaceButton(ID_PREVIEW_DPI, DPIMenuButton);

	//Replace Template Button with a drop down list for selecting the template
	CMFCToolBarComboBoxButton TemplateList(ID_PREVIEW_TEMPLATE_SELECT, 1, CBS_DROPDOWNLIST | CBS_SORT);
	//Add templates to combo box
	std::vector<CString> AllTemplates;
	//COutputDoc::GetAllPreviewTemplates();
	for (const auto& Template : AllTemplates) TemplateList.AddItem(Template);
	//TemplateList.SelectItem(idPreferred);
	Toolbar.ReplaceButton(ID_PREVIEW_TEMPLATE_SELECT, TemplateList);

	return 0;
}


void PreviewImagePane::OnTemplateEdit()
{
}

void PreviewImagePane::OnZoomIn()
{
	View.Zoom(1);
}

void PreviewImagePane::OnZoomFit()
{
	View.ZoomFit();
}

