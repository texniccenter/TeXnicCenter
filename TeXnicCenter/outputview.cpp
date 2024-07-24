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
#include "OutputView.h"
#include "OutputDoc.h"
#include "configuration.h"
#include "global.h"

IMPLEMENT_DYNAMIC(COutputView,CListCtrl)

BEGIN_MESSAGE_MAP(COutputView,CListCtrl)
	ON_MESSAGE(AfxUserMessages::OPW_ADD_LINE, &COutputView::OnAddLine)
	ON_MESSAGE(AfxUserMessages::OPW_RESET, &COutputView::OnReset)
	ON_MESSAGE(AfxUserMessages::OPW_ADD_INFOLINE, &COutputView::OnAddInfoLine)
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
	ON_WM_SETFOCUS()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SETFOCUS()
	ON_NOTIFY_REFLECT(NM_DBLCLK, &COutputView::OnNMDblclk)
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()


COutputView::COutputView()
		: m_bInitialized(FALSE)
		, m_pOutputDoc(NULL)
		, idxLastLineOnLastUpdateDisable(0)
{
	// create dummy font, so that m_font.DestroyObject() wont make any
	// problems
	m_font.CreatePointFont(100,_T("Courier New"));
}

COutputView::~COutputView()
{
}

#ifdef _DEBUG

void COutputView::AssertValid() const
{
	CListCtrl::AssertValid();
}

void COutputView::Dump(CDumpContext& dc) const
{
	CListCtrl::Dump(dc);
}
#endif //_DEBUG

BOOL COutputView::Create(const RECT &rect, CWnd *pwndParent)
{
	ASSERT(pwndParent && IsWindow(pwndParent->m_hWnd));

	return CWnd::CreateEx(WS_EX_CLIENTEDGE,WC_LISTVIEW,0,
		WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SHOWSELALWAYS | LVS_SINGLESEL,rect,pwndParent,0);
}

void COutputView::AttachDoc(COutputDoc *pDocument)
{
	m_pOutputDoc = pDocument;
	m_bInitialized = TRUE;
}

COutputDoc *COutputView::GetDocument() const
{
	ASSERT(!m_pOutputDoc || m_pOutputDoc->IsKindOf(RUNTIME_CLASS(COutputDoc)));
	return m_pOutputDoc;
}

int COutputView::GetLineCount() const
{
	return GetItemCount();
}

CImageList *COutputView::SetImageList(CImageList *pImageList)
{
	return CListCtrl::SetImageList(pImageList,LVSIL_SMALL);
}

CImageList *COutputView::GetImageList() const
{
	return CListCtrl::GetImageList(LVSIL_SMALL);
}

LRESULT COutputView::OnReset(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	Clear();
	return 0;
}

void COutputView::Clear()
{
	DeleteAllItems();
}

LRESULT COutputView::OnAddLine(WPARAM wParam, LPARAM/*lParam*/)
{
	AddLine((LPCTSTR)wParam);
	return 0L;
}

LRESULT COutputView::OnAddInfoLine(WPARAM wParam, LPARAM lParam)
{
	/////////////////
	//Get the Infos
	// - Image
	WORD nImage = HIWORD(wParam);
	// - Level
	WORD nLevel = LOWORD(wParam);
	// - Message
	TCHAR* buffer = (TCHAR*)lParam;

	if (buffer != NULL)
	{
		//Add InfoLine
		AddLine(buffer,nImage,nLevel);

		//We need to free the TCHAR buffer
		delete[] buffer;
	}
	else
	{
		//Add empty line
		AddLine(_T(""));
	}

	return 0L;
}

void COutputView::AddLine(LPCTSTR lpszLine,int nImage /*= -1*/,int nIndent /*= 0*/)
{
	int nItem = GetItemCount();
	CString strLine(EnsureStringReadable(lpszLine));
	LVITEM lvItem;

	lvItem.mask = LVIF_INDENT | LVIF_TEXT;
	lvItem.iItem = nItem;
	lvItem.iSubItem = 0;
	//lvItem.state
	lvItem.stateMask = 0;
	lvItem.iIndent = nIndent;
	lvItem.cchTextMax = strLine.GetLength();
	lvItem.pszText = strLine.GetBuffer(0);
	lvItem.iImage = nImage;
	//lvItem.lParam

	if (nImage != -1)
		lvItem.mask |= LVIF_IMAGE;

	nItem = InsertItem(&lvItem);
	strLine.ReleaseBuffer();

	SetColumnWidth(0,LVSCW_AUTOSIZE);

	//First line? Always select the first one, since this allows us to follow the output.
	if (nItem == 0)
	{
		SelectLine(nItem);
	}
	//Follow other lines only if the update is enabled. See also EnableUpdate().
	else if (bUpdateIsEnabled)
	{
		// was previous line the selected one?
		// => then make the new line the selected one and ensure, that it
		//    is visible,
		// => otherwise don't scroll anywhere
		POSITION pos = GetFirstSelectedItemPosition();
		if (pos && GetNextSelectedItem(pos) == nItem - 1)
			SelectLine(nItem);
	}
}

void COutputView::EnableUpdate(const bool bEnable)
{
	bUpdateIsEnabled = bEnable;

	if (bEnable)
	{
		SetRedraw(1);
		//Invalidate();
		//UpdateWindow();

		//Which line is currently selected?
		POSITION pos = GetFirstSelectedItemPosition();
		if (pos && GetNextSelectedItem(pos) == idxLastLineOnLastUpdateDisable)
		{
			//The selected line was also selected when we disabled update.
			// ==> So we follow the output to the currently last line.
			SelectLine(GetLineCount() - 1);
		}
	}
	else
	{
		//We follow the output, if the currently last line is selected when we re-enable the view update.
		idxLastLineOnLastUpdateDisable = GetLineCount() - 1;
		SetRedraw(0);
	}
}

BOOL COutputView::SetLineImage(int nLine,int nImage)
{
	LVITEM item;

	item.iItem = nLine;
	item.iSubItem = 0;
	item.mask = LVIF_IMAGE;
	item.iImage = nImage;

	return CListCtrl::SetItem(&item);
}

void COutputView::SelectLine(int nLine,BOOL bPlaceAtTop /*= FALSE*/)
{
	ASSERT(nLine >= 0 && nLine < GetLineCount());

	SetItemState(nLine,LVIS_FOCUSED | LVIS_SELECTED,LVIS_FOCUSED | LVIS_SELECTED);
	EnsureVisible(nLine,FALSE);

	if (bPlaceAtTop)
	{
		CRect rect;
		GetItemRect(nLine,rect,LVIR_BOUNDS);
		Scroll(CSize(0,rect.top));
	}
}

int COutputView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// set font
	ApplyFont();
	InsertColumn(0,_T(""));

	return 0;
}

BOOL COutputView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CListCtrl::PreCreateWindow(cs);
}

void COutputView::OnShowWindow(BOOL bShow,UINT nStatus)
{
	CListCtrl::OnShowWindow(bShow,nStatus);

	if (bShow && GetDocument())
		GetDocument()->SetActiveView(this);
}

void COutputView::OnSetFocus(CWnd* pOldWnd)
{
	if (COutputDoc* doc = GetDocument())
		GetDocument()->SetActiveView(this);

	CListCtrl::OnSetFocus(pOldWnd);
}

void COutputView::DoOnUpdate(COutputView *pSender,LPARAM lHint,CObject *pHint)
{
	OnUpdate(pSender,lHint,pHint);
}

void COutputView::OnUpdate(COutputView* /*pSender*/ ,LPARAM /*lHint*/, CObject* /*pHint*/)
{
}

void COutputView::ApplyFont()
{
	m_font.DeleteObject();
	m_font.CreateFontIndirect(&CConfiguration::GetInstance()->m_fontOutput);
	SetFont(&m_font);
}

const CString COutputView::EnsureStringReadable(LPCTSTR lpszString)
{
	CString strResult;

	while (lpszString && *lpszString)
	{
		if (*lpszString == _T('\t'))
			strResult += _T("  ");
		else
			strResult += *lpszString;

		++lpszString;
	}

	return strResult;
}

void COutputView::OnSysColorChange()
{
	CListCtrl::OnSysColorChange();

	ApplyFont();
}

void COutputView::OnNMDblclk(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POSITION pos = GetFirstSelectedItemPosition();

	if (!pos)
		return;

	GetDocument()->ActivateMessageByOutputLine(GetNextSelectedItem(pos),this);
}

void COutputView::OnKillFocus(CWnd* pNewWnd)
{
	__super::OnKillFocus(pNewWnd);
}
