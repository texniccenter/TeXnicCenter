// BookmarkView.cpp : implementation file
//

#include "stdafx.h"

#include <functional>

#include "resource.h"
#include "BookmarkView.h"
#include "LatexProject.h"
#include "TeXnicCenter.h"


const UINT ListViewID = 1;

// BookmarkView

IMPLEMENT_DYNAMIC(BookmarkView, WorkspacePaneBase)

BookmarkView::BookmarkView()
{

}

BookmarkView::~BookmarkView()
{
}


BEGIN_MESSAGE_MAP(BookmarkView, WorkspacePaneBase)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, ListViewID, &BookmarkView::OnNMDblClk)
	ON_NOTIFY(LVN_ENDLABELEDIT, ListViewID, &BookmarkView::OnLvnEndEditLabel)
	ON_NOTIFY(LVN_DELETEITEM, ListViewID, &BookmarkView::OnLvnDeleteItem)
	ON_NOTIFY(LVN_KEYDOWN, ListViewID, &BookmarkView::OnLvnKeyDown)
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()



// BookmarkView message handlers

int BookmarkView::CompareName(LPARAM l1, LPARAM l2)
{
	const CodeBookmark* b1 = reinterpret_cast<const CodeBookmark*>(l1);
	const CodeBookmark* b2 = reinterpret_cast<const CodeBookmark*>(l2);

	return b1->GetName().Compare(b2->GetName());
}

int BookmarkView::CompareLineNumber(LPARAM l1, LPARAM l2)
{
	const CodeBookmark* b1 = reinterpret_cast<const CodeBookmark*>(l1);
	const CodeBookmark* b2 = reinterpret_cast<const CodeBookmark*>(l2);

	return b1->GetLine() < b2->GetLine() ? -1 : b1->GetLine() > b2->GetLine() ? 1 : 0;
}

int BookmarkView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (WorkspacePaneBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	list_view_.CreateEx(WS_EX_CLIENTEDGE,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|LVS_REPORT|LVS_EDITLABELS,
		CRect(0,0,0,0),this,ListViewID);

	const DWORD style = LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT;
	list_view_.SetExtendedStyle(style);

	list_view_.InsertColumn(0,CString(MAKEINTRESOURCE(IDS_BOOKMARK)),0,200);
	list_view_.InsertColumn(1,CString(MAKEINTRESOURCE(IDS_FILE)),0,350);
	list_view_.InsertColumn(2,CString(MAKEINTRESOURCE(IDS_LINE)),LVCFMT_RIGHT,75);
	
	list_view_.SetColumnCompareFunction(0,CompareName);
	list_view_.SetColumnCompareFunction(2,CompareLineNumber);

	toolbar_.Create(this);
	toolbar_.LoadToolBar(IDR_BOOKMARK_VIEW);

	::SetWindowTheme(list_view_,L"explorer",0);

	return 0;
}

void BookmarkView::OnSize(UINT nType, int cx, int cy)
{
	WorkspacePaneBase::OnSize(nType, cx, cy);
	AdjustLayout(CRect(0,0,cx,cy));
}

void BookmarkView::AdjustLayout(const CRect& rect)
{
	if (toolbar_ && list_view_)	{
		CSize size(0,0);

		if ((toolbar_.GetStyle() & WS_VISIBLE) == WS_VISIBLE) {
			size = toolbar_.CalcFixedLayout(TRUE,TRUE);
			toolbar_.SetWindowPos(0,rect.left,rect.top,
				rect.right - rect.left,rect.top + size.cy,SWP_NOACTIVATE|SWP_NOZORDER);
		}

		const int y = rect.top + size.cy;
		list_view_.SetWindowPos(0,rect.left,y,rect.right,rect.bottom - y,
			SWP_NOACTIVATE|SWP_NOZORDER);
	}
}

//void BookmarkView::OnUpdate(CProjectView* /*sender*/, LPARAM lHint, LPVOID /*p*/)
//{
//	switch (lHint) {
//		case COutputDoc::hintParsingFinished:
//			case COutputDoc::typeCount
//			OnParsingFinished();
//			break;
//	}
//}

void BookmarkView::Clear()
{
	if (list_view_.m_hWnd) list_view_.DeleteAllItems();
}

void BookmarkView::InsertBookmark(int index, const CString& filename, const CodeBookmark& b)
{
	CString text;

	if (b.HasName())
		text = b.GetName();
	else
		text.Format(IDS_UNNAMED_BOOKMARK_FORMAT,index + 1);

	const int pos = list_view_.InsertItem(index,text);

	list_view_.SetItemText(pos,1,filename);

	text.Format(_T("%i"),b.GetLine() + 1);

	list_view_.SetItemText(pos,2,text);
	//TODO: Do not create a thing here. Just store 1 for actual name, and 0 for made-up name. Construct BMark later.
	list_view_.SetItemData(pos,reinterpret_cast<DWORD_PTR>(new CodeBookmark(b)));
}

void BookmarkView::Populate()
{
	list_view_.SetRedraw(FALSE);
	list_view_.DeleteAllItems();

	typedef CLaTeXProject::FileBookmarksContainerType Container;
	const Container& bookmarks = static_cast<CLaTeXProject*>(GetProject())->GetAllBookmarks();

	int index = 0;

	for (Container::const_iterator it = bookmarks.begin(); it != bookmarks.end(); ++it) {
		const BookmarkContainerType& inner = it->second;

		for (BookmarkContainerType::const_iterator it1 = inner.begin(); it1 != inner.end(); ++it1) {
			InsertBookmark(index++,it->first,*it1);
		}
	}

	list_view_.SetRedraw(TRUE);
}

void BookmarkView::OnBookmarkAdded(CLaTeXProject* /*sender*/, const BookmarkEventArgs& args)
{
	InsertBookmark(list_view_.GetItemCount(),args.GetFileName(),args.GetBookmark());
}

void BookmarkView::OnBookmarkRemoved(CLaTeXProject* /*sender*/, const BookmarkEventArgs& args)
{
	int index = FindBookmark(args.GetBookmark());

	if (index != -1)
		list_view_.DeleteItem(index);
}

//TODO: Add filename here. Just find the index, nothing else necessary, I think.
int BookmarkView::FindBookmark(const CodeBookmark b, CodeBookmark** p)
{
	const int count = list_view_.GetItemCount();
	int result = -1;

	for (int i = 0; i < count && result == -1; ++i) {
		CodeBookmark* data = reinterpret_cast<CodeBookmark*>(list_view_.GetItemData(i));

		if (*data == b) {
			result = i;

			if (p)
				*p = data;
		}
	}

	return result;
}

void BookmarkView::OnOpeningProject()
{
	using namespace std::placeholders;

	CLaTeXProject* p = static_cast<CLaTeXProject*>(GetProject());

	p->AddBookmarkAddedHandler(std::bind(&BookmarkView::OnBookmarkAdded,this,_1,_2));
	p->AddBookmarkRemovedHandler(std::bind(&BookmarkView::OnBookmarkRemoved,this,_1,_2));

	Populate();
}

void BookmarkView::OnClosingProject()
{
	using namespace std::placeholders;

	//CLaTeXProject* p = static_cast<CLaTeXProject*>(GetProject());

	//p->RemoveBookmarkAddedHandler(std::bind(&BookmarkView::OnBookmarkAdded,this,_1,_2));
	//p->RemoveBookmarkRemovedHandler(std::bind(&BookmarkView::OnBookmarkRemoved,this,_1,_2));
}

void BookmarkView::OnNMDblClk(NMHDR* /*hdr*/, LRESULT*)
{
	const int index = list_view_.GetNextItem(-1,LVNI_SELECTED|LVNI_ALL);
	
	if (index != -1) {
		const CString filename = list_view_.GetItemText(index,1);
		const CString name = CPathTool::GetRelativePath(static_cast<CLaTeXProject*>(GetProject())->
			GetDirectory(),filename,TRUE,FALSE);
		
		theApp.OpenLatexDocument(filename,FALSE,_ttoi(list_view_.GetItemText(index,2)));
	}
}

void BookmarkView::OnLvnEndEditLabel(NMHDR* hdr, LRESULT* result)
{
	NMLVDISPINFO* disp = reinterpret_cast<NMLVDISPINFO*>(hdr);

	if (disp->item.pszText) {
		CodeBookmark* b = reinterpret_cast<CodeBookmark*>(list_view_.GetItemData(disp->item.iItem));

		LPCTSTR name = disp->item.pszText;
		b->SetName(name);

		b = static_cast<CLaTeXProject*>(GetProject())->GetBookmark(list_view_.GetItemText(disp->item.iItem,1),b->GetLine());

		if (b)
			b->SetName(name);

		*result = 1;
	}
}

void BookmarkView::OnLvnDeleteItem(NMHDR* hdr, LRESULT*)
{
	NMLISTVIEW* nm = reinterpret_cast<NMLISTVIEW*>(hdr);
	delete reinterpret_cast<CodeBookmark*>(list_view_.GetItemData(nm->iItem));
}

void BookmarkView::OnLvnKeyDown(NMHDR* hdr, LRESULT*)
{
	NMLVKEYDOWN * key = reinterpret_cast<NMLVKEYDOWN*>(hdr);
	
	switch (key->wVKey) {
		case _T('A'):
			if (::GetKeyState(VK_CONTROL) >> 15 & 1) {
				list_view_.SetItemState(-1,LVIS_SELECTED,LVIS_SELECTED);
			}
			break;
		case VK_F2:
			{
				int index = list_view_.GetNextItem(-1,LVNI_SELECTED|LVNI_ALL);

				if (index != -1)
					list_view_.EditLabel(index);
			}
			break;
		case VK_DELETE:
			list_view_.SetRedraw(FALSE);

			if (list_view_.GetSelectedCount() == static_cast<UINT>(list_view_.GetItemCount()))
				list_view_.DeleteAllItems();
			else {
				int index = -1;

				while ((index = list_view_.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL)) != -1)
				{
					//list_view_.DeleteItem(index);
					CLaTeXProject* p = static_cast<CLaTeXProject*>(GetProject());
					if (p)
					{
						CodeBookmark* data = reinterpret_cast<CodeBookmark*>(list_view_.GetItemData(index));
						CString FName = list_view_.GetItemText(index, 1);
						p->RemoveBookmark(FName, *data);
					}
				}
			}

			list_view_.SetRedraw(TRUE);
			break;
	}
}

BOOL BookmarkView::PreTranslateMessage( MSG* msg )
{
	BOOL handled = msg->message == WM_KEYDOWN && (msg->hwnd == list_view_.GetSafeHwnd() && msg->wParam != VK_ESCAPE || 
		::GetParent(msg->hwnd) == list_view_.GetSafeHwnd() && msg->wParam == VK_ESCAPE);

	if (!handled)
		handled = WorkspacePaneBase::PreTranslateMessage(msg);
	else
		::DispatchMessage(msg);

	return handled;
}

void BookmarkView::OnSetFocus(CWnd* pOldWnd)
{
	__super::OnSetFocus(pOldWnd);

	list_view_.SetFocus();
}
