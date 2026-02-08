#include "stdafx.h"
#include "navigatorview.h"
#include "WorkspacePane.h"

IMPLEMENT_DYNAMIC(WorkspacePane, WorkspacePaneBase)

WorkspacePane::WorkspacePane()
	: client_(NULL)
	, toolBar_(NULL)
{
}

BEGIN_MESSAGE_MAP(WorkspacePane, WorkspacePaneBase)
	ON_WM_CONTEXTMENU()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// WorkspacePane message handlers

void WorkspacePane::OnSize(UINT nType, int cx, int cy)
{
	WorkspacePaneBase::OnSize(nType, cx, cy);

	CRect rc(0, 0, cx, cy);
	AdjustLayout(rc);
}

void WorkspacePane::SetClient(CWnd* p)
{
	client_ = p;

	if (m_hWnd && client_ && client_->GetParent()->GetSafeHwnd() != m_hWnd) {
		client_->SetParent(this);
		client_->SetOwner(this);
		AdjustLayout();
	}
}

void WorkspacePane::AdjustLayout(CRect& rc)
{
	if (toolBar_ && toolBar_->m_hWnd) {
		CSize size(0, 0);

		if ((toolBar_->GetStyle() & WS_VISIBLE) == WS_VISIBLE) {
			size = toolBar_->CalcFixedLayout(TRUE,TRUE);
			toolBar_->SetWindowPos(0, rc.left, rc.top, rc.right - rc.left,
				rc.top + size.cy, SWP_NOACTIVATE | SWP_NOZORDER);
		}

		rc.top += size.cy;
	}

	if (client_ && client_->m_hWnd)
		client_->SetWindowPos(0, rc.left, rc.top, rc.Width(), rc.Height(),
			SWP_NOACTIVATE | SWP_NOZORDER);
}

void WorkspacePane::AdjustLayout(void)
{
	CRect rect;
	GetClientRect(&rect);

	AdjustLayout(rect);
}

void WorkspacePane::OnSetFocus(CWnd* pOldWnd)
{
	WorkspacePaneBase::OnSetFocus(pOldWnd);
	SetClientFocus();
}

int WorkspacePane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (WorkspacePaneBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (client_ && client_->GetSafeHwnd() && client_->GetParent() != this) {
		client_->SetParent(this);
		AdjustLayout();
	}

	return 0;
}

CWnd* WorkspacePane::GetClient(void) const
{
	return client_;
}

BOOL WorkspacePane::OnEraseBkgnd(CDC* pDC)
{
	return client_ ? 1 : WorkspacePaneBase::OnEraseBkgnd(pDC);
}

void WorkspacePane::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (NavigatorTreeCtrl* t = dynamic_cast<NavigatorTreeCtrl*>(pWnd))
		t->ShowContextMenu(point);
	else
		WorkspacePaneBase::OnContextMenu(pWnd,point);
}

void WorkspacePane::SetClientFocus()
{
	if (client_ && client_->m_hWnd) client_->SetFocus();
}

void WorkspacePane::Focus()
{
	SetClientFocus();
}

void WorkspacePane::SetupToolBar(PrivateToolBar& TB, UINT id, UINT style)
{
	TB.Create(this, style | WS_TABSTOP, id);
	TB.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
	TB.LoadToolBar(id, 0, 0, TRUE);
	TB.AdjustStyle();
	TB.SetRouteCommandsViaFrame(FALSE);

	//We save a reference for future use.
	toolBar_ = &TB;
}
