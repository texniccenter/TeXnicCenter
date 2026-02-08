#pragma once

#include "WorkspacePaneBase.h"
#include "PrivateToolBar.h"

/**
 * @brief Dockable workspace pane with an optional toolbar and child client
 *		  window.
 */
class WorkspacePane : 
	public WorkspacePaneBase
{
	DECLARE_DYNAMIC(WorkspacePane)
public:
	WorkspacePane();
	
	/**
	 * @brief Returns the dockable pane client.
	 */
	CWnd* GetClient() const;

	/**
	 * @brief Sets the specified window as dockable pane client.
	 * 
	 * @param p The pointer to the window that should be the client or @c NULL.
	 */
	void SetClient(CWnd* p);

	/**
	 * @brief Sets properties of a toolbar and assigns it to this dockable pane instance.
	 *
	 * The ownership of the toolbar is held by the caller.
	 * 
	 * @param TB Reference to the toolbar.
	 * @param id The toolbar ID.
	 * @param style The toolbar style.
	 * 
	 * @return void
	 */
	void SetupToolBar(PrivateToolBar& TB, UINT id, UINT style = AFX_DEFAULT_TOOLBAR_STYLE);

protected:
	DECLARE_MESSAGE_MAP()

	virtual void AdjustLayout(CRect& rc);
	void AdjustLayout(void);
	void SetClientFocus();
	
	afx_msg void OnSetFocus(CWnd* pOldWnd);	
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	void Focus();

private:
	CWnd* client_;
	CMFCToolBar* toolBar_;
};
