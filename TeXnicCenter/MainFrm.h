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
#pragma once

#include <set>
#include <list>
#include <memory>

class BibView;
class BookmarkView;
class CBuildView;
class CChildFrame;
class CEnvironmentView;
class FileViewPane;
class CGrepView;
class COutputDoc;
class CParseOutputView;
class CProjectView;
class ErrorListPane;
class StructurePane;
class WorkspacePane;
class LaTeXView;
class COutputBuilder;
class CPreviewImageView;

void CreateColorButtonPalette(CPalette& palette, int& colors, int& columns);

// number of mathematic toolbars
const int MATHBAR_COUNT = 16;

const CMFCTabCtrl::Style DefaultTabStyle = CMFCTabCtrl::STYLE_3D_ONENOTE;

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)

	friend class CLaTeXProject;
	friend class COutputBuilder;

// types
protected:

	enum tagTimers
	{
		timerSave = 1
	};

// construction/destruction
public:
	CMainFrame();
	virtual ~CMainFrame();

// types
public:

	void OnIdle(long count);

	///Enumeration of output panes. Make sure, this matches the code in GetAllPanes()
	enum tagOutputTabs
	{
		outputTabPreviewBuildResult = 0
		, outputTabParse
		, outputTabGrep2
		, outputTabGrep1
		, outputTabErrors
		, outputTabBuildResult
	};

	///Enumeration of navigator panes. Make sure, this matches the code in GetAllPanes()
	enum tagNavigatorTabs
	{
		navigatorTabBookmarks = 0
		, navigatorTabPreviewImage
		, navigatorTabEnvironments
		, navigatorTabFiles
		, navigatorTabReferences
		, navigatorTabStructure
	};

// operations
public:
	void GetControlBarsEx(CArray<CPane*, CPane*>& arrBars);

	/**
	Replaces a toolbar button by another one.

	Allows other classes to modify toolbar buttons without directly
	accessing the toolbar window.

	@param unCmdID
	        Command ID of the toolbar button that should be replaced.
	@param button
	        Toolbar button the specified button shout be replaced by.

	@return
	        TRUE if successful, FALSE otherwise.
	 */
	BOOL ReplaceToolbarButton(UINT unCmdID, const CMFCToolBarButton &button);

	/**
	Returns a pointer to the toolbar button with the specified
	command ID.

	You can get all instances of a button if you use iteration. Set
	pos to NULL to get the first occurrence, after the call the
	referenced pos variable will contain a position information.
	Using this information in the next call, you will get the next
	instance of the button.

	@param unCmdID
	        Command ID of the toolbar button to return.
	@param pos
	        Position marker for iteration.

	@return
	        Pointer to the toolbar button with the specified command ID or
	        NULL if there is no such button.
	 */
	CMFCToolBarButton* GetToolBarButton(UINT unCmdID, int& pos) const;

	/**
	Activates the specified tab on the output bar.

	@param nTab
	        Zero based index of the tab on the output bar to activate.

	@param bSetFocus
	        Whether to set the focus to the specified tab or not.
	 */
	void ActivateOutputTab(int nTab, bool bSetFocus = true);

	/** Shows or hides a pane and makes sure that it is visible,
		if it is about to be shown.
	*/
	void ShowPaneEnsureVisibility(CBasePane* pPane, bool bShow, bool bDelay, bool bActivate = true);

	/**
	Should be called, when the user may have changed the external, user
	defined tools, to update the tools menu.
	 */
	void RebuildToolsMenu();

	/** Builds up list of opened MDI childs.

	        Call this function to obtain an array of pointers to the currently opened MDI Childs.
	        You can later ask each MDI Child for its view or document.
	        The childs can be given sorted by the current window cycling order
	        or sorted as shown by the tabs (default).

	        @param MDIChildArray
	                Retrieves pointers to all MDI Childs.

	        @param bSortByTabs
	                Whether to give the MDI Childs sorted as shown by the tabs or in windows cycling order.

	        @returns Array index of the currently active child.
	 */
	int GetMDIChilds(CArray<CChildFrame*, CChildFrame*>& MDIChildArray, bool bSortByTabs = true);


// implementation helpers
protected:
	/**
	Returns the control bar associated with the specified command ID.

	@param unID
	        The ID of the command that is used to toggle the control bar.

	@return
	        Pointer to the control bar associated with the specified
	        command ID or NULL, if there is no matching control bar.
	 */
	CBasePane* GetControlBarByCmd(UINT unID);


	/** Returns an array of pointers to all panes.

	@param pAllPanes
	        The array of requested panes.

	@param bNavigatorPanes
	        Whether or not to include Navigator panes.

	@param bOutputPanes
	        Whether or not to include Output panes.
	 */
	void GetAllPanes(std::vector< CBasePane* >& pAllPanes, bool bNavigatorPanes = true, bool bOutputPanes = true);


	/**
	Creates the specified tool bar

	@param pToolbar
	        Tool bar to create
	@param unID
	        ID of the toolbar resource to use for this toolbar
	@param unTitleID
	        String resource to use for the title of the bar
	@param bVisible
	        TRUE to make the tool bar initial visible, FALSE to hide it.

	@return
	        TRUE if successful, FALSE otherwise.
	 */
	bool CreateToolBar(CMFCToolBar* pToolBar, UINT unID, UINT unTitleID, bool bVisible = true);

	/** Cares (display/hide) about the Document tabs.
	 */
	void DisplayDocumentTabs();

// overloadings
public:
//{{AFX_VIRTUAL(CMainFrame)
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// debug
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// message handlers
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnExtrasCustomize();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnOptionsChanged();
	afx_msg void OnWindowEditor();
	afx_msg void OnWindowList();
	afx_msg void OnHelpSearch();
	afx_msg void OnHelpIndex();
	afx_msg void OnViewFullScreen();
	afx_msg void OnUpdateViewFullScreen(CCmdUI* pCmdUI);
	afx_msg void OnHelpKeyMapping();
	afx_msg void OnUpdateWindowNext(CCmdUI* pCmdUI);
	afx_msg void OnWindowNext();
	afx_msg void OnWindowPrevious();
	afx_msg void OnUpdateWindowPrevious(CCmdUI* pCmdUI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnHelpContents();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnViewDocTabsBottom();
	afx_msg void OnViewDocTabsOff();
	afx_msg void OnViewDocTabsTop();
	afx_msg void OnViewDocTabsIcons();
	afx_msg void OnViewDocTabsNote();
	afx_msg void OnUpdateViewDocTabs(CCmdUI* pCmdUI);
	afx_msg BOOL OnToolsCancel(UINT);
	afx_msg void OnWindowCloseSelectedTab();
	afx_msg void OnUpdateWindowCloseSelectedTab(CCmdUI* pCmdUI);
	afx_msg void OnWindowCloseAllButActive();
	afx_msg void OnUpdateWindowCloseAllButActive(CCmdUI* pCmdUI);
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateWindowMDICommands(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg BOOL OnToggleCtrlBar(UINT nIDEvent);
	afx_msg BOOL OnToggleDockingBar(UINT nIDEvent);
	afx_msg void OnCheckCtrlBarVisible(CCmdUI *pCmdUI);
	afx_msg LRESULT OnToolbarContextMenu(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnResetToolbar(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCustomizationMode(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateUserTool(CCmdUI *pCmdUI);
	afx_msg void OnExecuteUserTool(UINT nIDEvent);
	afx_msg void OnViewToggleBottomDockingBars();
	afx_msg void OnViewToggleLeftDockingBars();
	afx_msg void OnViewToggleRightDockingBars();
	afx_msg void OnViewToggleTopDockingBars();

	DECLARE_MESSAGE_MAP()

// attributes
private:
	CMFCStatusBar m_wndStatusBar;
	CMFCToolBar m_wndToolBar;
	CMFCToolBar m_wndFormatBar;
	CMFCToolBar m_wndFormatRUBar;
	CMFCToolBar m_wndInsertBar;
	CMFCToolBar m_wndLatexBar;
	CMFCToolBar m_wndMathBar;
	CMFCToolBar m_wndFindBar;
	CMFCToolBar m_awndMathBar[MATHBAR_COUNT];
	CMFCToolBar m_wndPreviewBar;
	CMFCMenuBar m_wndMenuBar;

	CImageList m_navigatorImages;
	CImageList m_outputImages;
	CMenu m_stdMenu;
	CMenu m_projectMenu;
	CMFCToolBarImages m_userImages;

	CView* m_pContextMenuTargetWindow; // Target window from context menu

	CComPtr<ITaskbarList3> taskBarList_;

	///View for the structure of a project
	std::unique_ptr<StructurePane> structure_view_;

	///Pane showing the files of a project
	std::unique_ptr<FileViewPane> file_view_pane_;
	
	///Pane showing the latex environments of a project
	std::unique_ptr<WorkspacePane> env_view_pane_;
	///View for the latex environments of a project
	std::unique_ptr<CEnvironmentView> env_view_;

	///Pane showing the contents of the bibtex files
	std::unique_ptr<BibView> bib_view_pane_;

	///Pane showing all bookmarks
	std::unique_ptr<BookmarkView> bookmark_view_pane_;

	///Pane showing the results of the build process.
	std::unique_ptr<WorkspacePane> build_view_pane_;
	/** View containing the results of the build process. */
	std::unique_ptr<CBuildView> build_view_;

	///Pane showing the results of the preview process.
	std::unique_ptr<WorkspacePane> preview_output_view_pane_;
	/** View containing the results of the preview process. */
	std::unique_ptr<CBuildView> preview_output_view_;

	///Pane for the preview image view.
	std::unique_ptr<WorkspacePane> preview_image_view_pane_;
	/** Preview image view. */
	std::unique_ptr<CPreviewImageView> preview_image_view_;

	///View listing the errors of a compilation
	std::unique_ptr<ErrorListPane> error_list_view_;

	///Pane containing the results of file grep 1.
	std::unique_ptr<WorkspacePane> grep_view_1_pane_;
	/** View containing the results of file grep 1. */
	std::unique_ptr<CGrepView> grep_view_1_;

	///Pane containing the results of file grep 2.
	std::unique_ptr<WorkspacePane> grep_view_2_pane_;
	/** View containing the results of file grep 2. */
	std::unique_ptr<CGrepView> grep_view_2_;

	///Pane for the Structure parser output view.
	std::unique_ptr<WorkspacePane> parse_view_pane_;
	/** Structure parser output view. */
	std::unique_ptr<CParseOutputView> parse_view_;

	std::unique_ptr<COutputDoc> output_doc_; //UPDATE

	CImageList build_animation_;

	///MDI children list in MRU order
	std::list< CFrameWnd* > MDIChildrenMRU;
	///Last activated MDI child
	CFrameWnd* MDITopChild;

private:
	/** Identifier of the timer used to parse the project in given intervals. */
	UINT m_unParseTimer;
	bool CreateToolWindows();
	void HideTaskbarProgress();

protected:
	void OnOpenProject(CLaTeXProject* p);
	void OnCloseProject(CLaTeXProject* p);
	afx_msg LRESULT OnGetTabToolTip(WPARAM, LPARAM);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnViewTransparency();
	
	void OnUpdateApplicationLook(CCmdUI* pCmdUI);

	///Start an animation of the TXC icon in the Windows task bar.
	void OnStartPaneAnimation();
	///Start the animation of the TXC icon in the Windows task bar.
	LRESULT OnStopPaneAnimation(WPARAM, LPARAM);

	/** Shows, not toggles, a docking bar.
	
		@param wParam
		The UINT ID of the docking bar to show.
	*/
	LRESULT OnShowDockingBar(WPARAM wParam, LPARAM lParam);

private:
	bool animating_;

	/** Toogles docking bars with a certain alignment.

		@returns true, if one of them was visible before calling this function.
	*/
	bool ToggleDockingBars(const DWORD dwAlignment, const bool bCloseOnly = false);

public:
	COutputDoc* GetOutputDoc(void);

private:
	const std::vector<CProjectView*> GetViews();
	LRESULT OnDwmSendIconicThumbnail(WPARAM, LPARAM);

protected:
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

	afx_msg void OnDestroy();
	afx_msg void OnWindowRecentlyUsed();
	afx_msg void OnUpdateWindowRecentlyUsed(CCmdUI *pCmdUI);

public:
	void UpdateFrameTitle();

	///Called by child frame on creation
	void RegisterChildFrame(CFrameWnd* frame);
	///Called by child frame on deletion
	void UnregisterChildFrame(CFrameWnd* frame);
	///Called to put the current child at the top of the MRU list
	void SetCurrentChildFrameAsRecentlyUsed();
	///The given child will be at the top of the MRU list.
	void SetRecentlyUsedChildFrame(CFrameWnd* child);
	///Activates the next/previous MDI child based on the MRU list
	void ActivateMDIChildByMRUList(const bool bPrevious);

	void CheckForFileChanges();
	void CheckForFileChangesAsync();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
