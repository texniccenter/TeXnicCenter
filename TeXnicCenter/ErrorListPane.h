#pragma once

#include <vector>
#include <memory>

#include "BuildView.h"
#include "PrivateToolBar.h"
#include "SortListCtrl.h"
#include "OutputViewBase.h"
#include "WorkspacePaneBase.h"

class COutputDoc;
class COutputInfo;

// ErrorListView

class ErrorListPane : public WorkspacePaneBase, public OutputViewBase
{
	std::auto_ptr<COutputInfo> selectedItem_;

	DECLARE_DYNAMIC(ErrorListPane)

private:
	COutputDoc* doc_;
	CCriticalSection item_monitor_;

	struct Item;

	typedef std::vector<Item> ItemContainer;
	ItemContainer items_;

	PrivateToolBar toolbar_;
	SortListCtrl list_view_;
	CImageList image_list_;
	int errors_, warnings_, bad_boxes_;
	bool show_errors_, show_warnings_, show_bad_boxes_;

public:
	ErrorListPane();
	~ErrorListPane();

protected:
	DECLARE_MESSAGE_MAP()

public:	
	void AdjustLayout(const CRect& rect);
	void AdjustLayout(void);
	void Clear(void);
	void AddMessage(const COutputInfo& info, CBuildView::tagImage t);

	/** Enables or disables the redrawing of this view.

		When disabled, the view will not redraw.
		If enabled, it will redraw.
	*/
	virtual void EnableUpdate(const bool bEnable);

private:
	void UpdateToolBarButton(CBuildView::tagImage t);
	void UpdateToolBarButtons();

protected:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnUpdateShowErrors(CCmdUI *pCmdUI);
	afx_msg void OnUpdateShowWarnings(CCmdUI *pCmdUI);
	afx_msg void OnUpdateShowBadBoxes(CCmdUI *pCmdUI);
	afx_msg void OnShowErrors();
	afx_msg void OnShowWarnings();
	afx_msg void OnShowBadBoxes();
	afx_msg void OnNMDblClk(NMHDR*, LRESULT*);

private:
	int InsertMessage(const COutputInfo& info, CBuildView::tagImage t);
	void Populate(unsigned set);
	void Populate(void);

public:
	void AttachDoc(COutputDoc* doc);

private:
	int CompareType(LPARAM l1, LPARAM l2);
	int CompareOrdinal(LPARAM l1, LPARAM l2);
	int CompareErrorMessage(LPARAM l1, LPARAM l2);
	int CompareSourceLine(LPARAM l1, LPARAM l2);
	int CompareSourceFile(LPARAM l1, LPARAM l2);

protected:
	void Focus();

	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnViewSource();
};
