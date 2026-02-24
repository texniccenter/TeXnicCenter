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

#pragma once
#include "WorkspacePane.h"
#include "PreviewImageView.h"
#include "PrivateToolBar.h"


/** This pane contains and manages the preview and its toolbar.
*
*/
class PreviewImagePane : public WorkspacePane
{
//Friends
//Types
public:

//Construction / Deconstruction
public:
    PreviewImagePane();
    virtual ~PreviewImagePane() = default;
	DECLARE_DYNAMIC(PreviewImagePane)

//Messages
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	///Triggers reloading the preview image from disk.
	LRESULT Update(WPARAM wParam, LPARAM lParam);

	///Starts the progress animation.
	LRESULT StartProgress(WPARAM wParam, LPARAM lParam);

	///Stops the progress animation.
	LRESULT StopProgress(WPARAM wParam, LPARAM lParam);

	void OnRefresh();

	void OnUpdateFastMode(CCmdUI* pCmdUI);
	void OnFastMode();

	void OnUpdateAutoBuildOnEnter(CCmdUI* pCmdUI);
	void OnAutoBuildOnEnter();
	void OnUpdateAutoBuildOnSave(CCmdUI* pCmdUI);
	void OnAutoBuildOnSave();

	void OnTemplateEdit();
	void OnTemplateCreate();
	void OnTemplateScan();

	void OnZoomIn();
	void OnUpdateZoomOut(CCmdUI* pCmdUI);
	void OnZoomOut();
	void OnUpdateZoomFit(CCmdUI* pCmdUI);
	void OnZoomFit();

	void OnDPI(UINT nID);

	void OnUpdateTemplateSelect(CCmdUI* pCmdUI);
	void OnTemplateSelect();

//Methods
public:
	///Scans for templates in the preview directory and add their names to the drop down.
	void FillTemplateDropDown();

protected:
	///Returns a pointer to the drop down for the templates.
	CMFCToolBarComboBoxButton* GetTemplateDropDown();

//Attributes
public:
protected:
	///The window responsible for showing, zooming, panning the preview image.
	CPreviewImageView View;

	///A toolbar with commands related to preview templates, resolution and other aspects.
	PrivateToolBar Toolbar;
};
