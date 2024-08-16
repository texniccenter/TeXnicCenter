#include "stdafx.h"
#include "resource.h"
#include "BibItem.h"
#include "BibView.h"
#include "navigatorview.h"
#include "LatexProject.h"
#include "OleDrop.h"
#include "OutputDoc.h"
#include "SearchToolBarEditButton.h"
#include "SearchToolBarEditCtrl.h"
#include "TeXnicCenter.h"

enum {
	ListID = 100,
	SearchTimerID,
	SearchTimerTimeout = 1000 // 1 second
};

// Generic string tokenizer
template<class Ch, class Container>
void Tokenize(const Ch* first, const Ch* last, const Ch* sep_first, const Ch* sep_last, Container& c)
{
	c.clear();

	typedef typename Container::value_type StringType;

	const Ch* pos1 = first; const Ch* pos2;
	bool stop = false;

	while (!stop) {
		// find first not of
		pos1 = std::find_first_of(pos1,last,sep_first,sep_last,std::not2(std::equal_to<Ch>()));
		pos2 = std::find_first_of(pos1,last,sep_first,sep_last);

		if (pos1 != last) {
			c.push_back(StringType(pos1,pos2 - pos1));
			pos1 = pos2;
		}
		else
			stop = true;
	}
}

/// Splits a string into tokens.
/// 
/// \param text 
///		   String to split
/// \param sep 
///		   Characters used to split the string
/// \param words
///		   A container that will contain the resulting tokens
void Tokenize(const CString& text, const CString& sep, std::vector<CString>& words)
{
	LPCTSTR first = text;
	LPCTSTR last = first + text.GetLength();

	LPCTSTR sep_first = sep;
	LPCTSTR sep_last = sep_first + sep.GetLength();

	Tokenize(first,last,sep_first,sep_last,words);
}

struct BibView::Item {
	int group_id;
	int image;
	StructureItemContainer::size_type structure_item_index;
	BibItem bib;

	Item(int group_id, int image, const BibItem& b);
};

BibView::Item::Item( int group_id, int image, const BibItem& b ) 
: group_id(group_id), bib(b), image(image)
{
}

BibView::PopulateParam::PopulateParam( const PredicateFunctionType& p, BibView* v ) 
: predicate(p), view(v)
{
}

// BibView

IMPLEMENT_DYNAMIC(BibView, WorkspacePaneBase)

BibView::BibView()
: search_timer_enabled_(false)
, dragged_item_(-1)
, search_flags_(0)
, stop_search_(0)
, search_semaphore_(1,1)
, populate_was_search_(false)
{
}

BibView::~BibView()
{
}


BEGIN_MESSAGE_MAP(BibView, WorkspacePaneBase)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	ON_WM_TIMER()
	ON_EN_CHANGE(ID_SEARCH_EDIT, &BibView::OnEnChangeSearch)
	ON_COMMAND(ID_SEARCH_EDIT, &BibView::OnEnChangeSearch)
	ON_COMMAND(ID_CLEAR_SEARCH, &BibView::OnClearSearch)
	ON_COMMAND(ID_START_SEARCH, &BibView::OnStartSearch)
	ON_NOTIFY(LVN_BEGINDRAG, ListID, &BibView::OnLvnBeginDrag)
	ON_COMMAND_RANGE(ID_AUTHOR,ID_PUBLISHER, &BibView::OnSearchOptions)
	ON_COMMAND_RANGE(ID_BIBTEX_KEY, ID_BIBTEX_KEY, &BibView::OnSearchOptions)
	ON_UPDATE_COMMAND_UI_RANGE(ID_AUTHOR,ID_PUBLISHER, &BibView::OnUpdateSearchOptions)
	ON_UPDATE_COMMAND_UI(ID_BIBTEX_KEY, &BibView::OnUpdateSearchOptions)
	ON_NOTIFY(NM_DBLCLK, ListID, &BibView::OnNMDblClk)
	ON_NOTIFY(LVN_ITEMCHANGED, ListID, &BibView::OnLvnItemChanged)
	ON_COMMAND(IDOK, &BibView::OnEnter)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// BibView message handlers

int BibView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (WorkspacePaneBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Default search options
	search_flags_ = Label|Author|Title|Year;

	toolbar_.Create(this,AFX_DEFAULT_TOOLBAR_STYLE|WS_TABSTOP,IDR_BIB_SEARCH);
	toolbar_.ModifyStyleEx(0,WS_EX_CONTROLPARENT);
	toolbar_.LoadToolBar(IDR_BIB_SEARCH,0,0,TRUE);
	toolbar_.AdjustStyle();

	toolbar_.SetRouteCommandsViaFrame(FALSE);

	SearchToolBarEditButton edit(ID_SEARCH_EDIT);

	search_button_ = static_cast<CMFCToolBarEditBoxButton*>(toolbar_.GetButton(toolbar_.ReplaceButton(ID_SEARCH_EDIT,edit)));
	
	ENSURE(search_button_);
	ASSERT(search_button_->m_nID == ID_SEARCH_EDIT);

	CMenu menu;
	menu.LoadMenu(IDR_BIB_SEARCH_OPTIONS);
	
	CMFCToolBarMenuButton m(~0U,menu.GetSubMenu(0)->GetSafeHmenu(),0);
	toolbar_.ReplaceButton(ID_SEARCH_OPTIONS,m);

	search_button_->GetEditBox()->SetCueBanner(CString(MAKEINTRESOURCE(IDS_SEARCH)));

	list_view_.CreateEx(WS_EX_CLIENTEDGE,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_TABSTOP|LVS_REPORT|LVS_SHOWSELALWAYS,
		CRect(0,0,0,0),this,ListID);

	::SetWindowTheme(list_view_,L"explorer",0);

	list_view_.SetExtendedStyle(LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|
		LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP);

	list_view_.InsertColumn(0,CString(MAKEINTRESOURCE(IDS_LABEL)),0,150);
	list_view_.InsertColumn(1,CString(MAKEINTRESOURCE(IDS_AUTHOR)),0,250);
	list_view_.InsertColumn(2,CString(MAKEINTRESOURCE(IDS_TITLE)),0,300);
	list_view_.InsertColumn(3,CString(MAKEINTRESOURCE(IDS_YEAR)),LVCFMT_RIGHT,50);
	list_view_.InsertColumn(4,CString(MAKEINTRESOURCE(IDS_PUBLICATION_TYPE)),0,100);
	list_view_.InsertColumn(5,CString(MAKEINTRESOURCE(IDS_PUBLISHER)),0,150);

	CImageList il;
	il.Attach(::ImageList_LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDB_ITEM_TYPES),16,1,
		RGB(255,0,255),IMAGE_BITMAP,LR_CREATEDIBSECTION));
	ASSERT(il);

	list_view_.SetImageList(&il,LVSIL_SMALL);
	il.Detach(); // Will be destroyed by the list view

	using namespace std;
	using namespace placeholders;

	MapKeyStateToFormat(0,CLaTeXProject::FormatRef); // No addition key pressed while dragging, output \cite{key}
	MapKeyStateToFormat(MK_CONTROL,bind(&StructureItem::GetLabel,_1)); // Just the label
	
	MapKeyStateToMessage(0,ID_ITEM_INSERT_REF);
	MapKeyStateToMessage(MK_CONTROL,ID_ITEM_INSERT_LABEL);

	list_view_.SetColumnCompareFunction(0,bind(TranslateCompareFunction,_1,_2,this,CompareLabel));
	list_view_.SetColumnCompareFunction(1,bind(TranslateCompareFunction,_1,_2,this,CompareAuthor));
	list_view_.SetColumnCompareFunction(2,bind(TranslateCompareFunction,_1,_2,this,CompareTitle));
	list_view_.SetColumnCompareFunction(3,bind(TranslateCompareFunction,_1,_2,this,CompareYear));
	list_view_.SetColumnCompareFunction(4,bind(TranslateCompareFunction,_1,_2,this,ComparePublicationType));
	list_view_.SetColumnCompareFunction(5,bind(TranslateCompareFunction,_1,_2,this,ComparePublisher));

	AdjustLayout();

	return 0;
}

void BibView::OnSize(UINT nType, int cx, int cy)
{
	WorkspacePaneBase::OnSize(nType, cx, cy);
	AdjustLayout(CRect(0,0,cx,cy));
}

CLaTeXProject* BibView::GetProject()
{
	ASSERT_KINDOF(CLaTeXProject,CProjectView::GetProject());
	return static_cast<CLaTeXProject*>(CProjectView::GetProject());
}

void BibView::OnUpdate(CProjectView* /*sender*/, LPARAM lHint, LPVOID /*p*/)
{
	switch (lHint) {
		case COutputDoc::hintParsingFinished:
			OnParsingFinished();
			break;
	}
}

void BibView::Clear(void)
{
	list_view_.DeleteAllItems();
	list_view_.RemoveAllGroups();
}

void BibView::AdjustLayout(const CRect& rect)
{
	if (toolbar_ && list_view_) {
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

void BibView::AdjustLayout()
{
	CRect rect;
	GetClientRect(&rect);
	AdjustLayout(rect);
}

void BibView::OnSetFocus(CWnd* pOldWnd)
{
	WorkspacePaneBase::OnSetFocus(pOldWnd);

	if (search_button_ && search_button_->GetEditBox())
		search_button_->GetEditBox()->SetFocus();
	else
		list_view_.SetFocus();
}

void BibView::OnParsingFinished()
{
	::InterlockedExchange(&stop_search_,1);

	search_semaphore_.Lock();
	bib_items_.clear();

	const StructureItemContainer &a = GetProject()->m_aStructureItems;

	list_view_.SetRedraw(FALSE);
	list_view_.RemoveAllGroups();

	//Tino: Check this.
	// Windows XP specific: after we removed all the groups we need
	// to reenable the group view
	list_view_.EnableGroupView(TRUE);

	list_view_.SetRedraw();

	std::hash<std::basic_string<TCHAR> > hasher;

	LVGROUP lvg = {};
	lvg.mask = LVGF_HEADER|LVGF_GROUPID;

	lvg.cbSize = sizeof(LVGROUP);
	lvg.mask |= LVGF_STATE;
	lvg.state = LVGS_COLLAPSIBLE;

	for (StructureItemContainer::const_iterator it = a.begin(); it != a.end(); ++it) {
		const StructureItem &si = *it;

		switch (si.GetType()) {
			case StructureItem::bibFile :
			case StructureItem::missingBibFile :
				{
					// Windows XP or higher required for grouping
					const CString& title = si.GetTitle();
					lvg.iGroupId = static_cast<int>(hasher(std::basic_string<TCHAR>(title)));

					if (!list_view_.HasGroup(lvg.iGroupId)) {
						const_cast<LPCTSTR&>(lvg.pszHeader) = title;
						lvg.cchHeader = title.GetLength();

						VERIFY(list_view_.InsertGroup(list_view_.GetGroupCount(),&lvg) != -1);
					}
				}
				break;
			case StructureItem::bibItem :
				if (const BibItem* info = dynamic_cast<const BibItem*>(it->GetItemInfo())) {
					ASSERT(lvg.iGroupId); // Must be a valid group id
					Item item(lvg.iGroupId,si.GetType(),*info);
					item.structure_item_index = std::distance(a.begin(),it);
					bib_items_.push_back(item);
				}
				else
					ASSERT(it->GetItemInfo() == NULL);

				break;
		}
	}

	search_semaphore_.Unlock();

	Populate(EveryItem);
}

void BibView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	ShowContextMenu(point);
}

void BibView::ShowContextMenu(CPoint& point)
{
	// handle only if generated by key (context menu key on win95 keyboard)
	// (mouse is handled by OnRButtonDown() -- problems with messages from CCJTabCtrlBar!)
	if (point.x == -1 && point.y == -1) {
		
		int item = list_view_.GetNextItem(-1,LVNI_ALL|LVNI_SELECTED);
		CRect rect;

		if (item != -1 && list_view_.GetItemRect(item,rect,LVIR_LABEL)) {
			list_view_.ClientToScreen(rect);
			theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_ITEM,rect.right,rect.bottom,&list_view_);
		}
	}
	else {
		CPoint pt = point;
		list_view_.ScreenToClient(&pt);
		UINT flags = LVHT_ONITEM;
		int item = list_view_.HitTest(pt,&flags);	

		if (item != -1) {
			list_view_.SetRedraw(FALSE);
			list_view_.SetItemState(-1,LVIS_SELECTED,0);
			list_view_.SetItemState(item,LVIS_SELECTED,LVIS_SELECTED);
			list_view_.SetRedraw();
			list_view_.Invalidate();

			theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_ITEM,point.x,point.y,&list_view_);
		}
		else
			theApp.m_pMainWnd->SendMessage(AFX_WM_TOOLBARMENU, 0,MAKELPARAM(point.x,point.y));
	}
}

void BibView::OnSearchTimerElapsed()
{
	StartSearch();
}

void BibView::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
		case SearchTimerID:
			KillTimer(nIDEvent);
			search_timer_enabled_ = false;
			OnSearchTimerElapsed();
			break;
	}

	WorkspacePaneBase::OnTimer(nIDEvent);
}

void BibView::OnEnChangeSearch()
{
	if (search_timer_enabled_) {
		KillTimer(SearchTimerID);
		search_timer_enabled_ = false;
	}

	if (IsAttached()) { // A project has to be attached to this view
		SetTimer(SearchTimerID,SearchTimerTimeout,0);
		search_timer_enabled_ = true;
	}		
}

void BibView::OnClearSearch()
{
	if (search_button_ && search_button_->GetEditBox())
		search_button_->GetEditBox()->SetWindowText(0);
}

bool BibView::EveryItem( const BibItem& /*item*/ )
{
	return true;
}

bool BibView::Contains(const CString& text, const CString& search)
{
	temp_ = text;
	temp_.MakeLower();
	return temp_.Find(search) != -1;
}

bool BibView::ContainsAny( const CString& text, std::vector<CString>& tokens )
{
	std::vector<CString>::iterator it = tokens.begin();
	pos_buffer_.clear();

	// We need to find all matches. If we stop after the first match
	// then e.g. a search for an item with more than one author will fail
	// in case the input string contains partial names for a couple of authors
	for ( ; it != tokens.end() && tokens.size() != pos_buffer_.size() && !stop_search_; ++it) {
		if (Contains(text,*it))
			pos_buffer_.push_back(it);
	}

	bool found = !pos_buffer_.empty();

	// and remove these
	for (std::vector<std::vector<CString>::iterator>::reverse_iterator rit = pos_buffer_.rbegin();
		rit != pos_buffer_.rend() && !stop_search_; ++rit)
	{
		tokens.erase(*rit);
	}

	return found && !stop_search_;
}

bool BibView::ItemContains( const BibItem& item, const std::vector<CString>& search_tokens )
{
	std::vector<CString>& tokens = token_buffer_;
	tokens = search_tokens;

	if (search_flags_ & Title)
		ContainsAny(item.GetTitle(),tokens);

	if (!tokens.empty() && search_flags_ & Label)
		ContainsAny(item.GetLabel(),tokens);

	if (!tokens.empty() && search_flags_ & Author)
		ContainsAny(item.GetAuthor(),tokens);

	if (!tokens.empty() && search_flags_ & Year) {
		const Nullable<int>& year = item.GetYear();

		if (year.HasValue()) {
			CString text;
			text.Format(_T("%i"), year.GetValue());

			ContainsAny(text, tokens);
		}
	}

	if (!tokens.empty() && search_flags_ & Type)
		ContainsAny(item.GetTypeString(),tokens);

	if (!tokens.empty() && search_flags_ & Publisher)
		ContainsAny(item.GetPublisher(),tokens);

	return tokens.empty(); // If there are no remaining tokens then this item is a match
}

void BibView::Populate( const PredicateFunctionType& predicate )
{
	stop_search_ = 0;
	AfxBeginThread(PopulateThread,new PopulateParam(predicate,this));
}

void BibView::Populate()
{
	Populate(EveryItem);
}

void BibView::OnLvnBeginDrag(NMHDR* nm, LRESULT*)
{
	NMLISTVIEW* p = reinterpret_cast<NMLISTVIEW*>(nm);
	ASSERT(p->iItem != -1);

	BibItemContainerType::size_type pos = static_cast<BibItemContainerType::size_type>
		(list_view_.GetItemData(p->iItem));
	const StructureItem& item = GetProject()->GetStructureItems()[bib_items_[pos].structure_item_index];

	SetDraggedItem(new StructureItem(item));
	DWORD result;
	DoDragDrop(DROPEFFECT_COPY,&result);
}

void BibView::ToggleSearchOption(unsigned opt)
{
	if (IsSearchOptionSet(opt))
		search_flags_ &= ~opt;
	else
		search_flags_ |= opt;
}

bool BibView::IsSearchOptionSet(unsigned opt) const
{
	return (search_flags_ & opt) == opt;
}

void BibView::OnSearchOptions(UINT id)
{
	switch (id) {
		case ID_BIBTEX_KEY:
			ToggleSearchOption(Label);
			break;
		case ID_AUTHOR:
			ToggleSearchOption(Author);
			break;
		case ID_TITLE:
			ToggleSearchOption(Title);
			break;
		case ID_YEAR:
			ToggleSearchOption(Year);
			break;
		case ID_PUBLICATION_TYPE:
			ToggleSearchOption(Type);
			break;
		case ID_PUBLISHER:
			ToggleSearchOption(Publisher);
			break;
	}
}

void BibView::OnUpdateSearchOptions(CCmdUI* pCmdUI)
{
	unsigned opt;

	switch (pCmdUI->m_nID) {
		case ID_BIBTEX_KEY:
			opt = Label;
			break;
		case ID_AUTHOR:
			opt = Author;
			break;
		case ID_TITLE:
			opt = Title;
			break;
		case ID_YEAR:
			opt = Year;
			break;
		case ID_PUBLICATION_TYPE:
			opt = Type;
			break;
		case ID_PUBLISHER:
			opt = Publisher;
			break;
		default:
			ASSERT(FALSE);
			opt = 0;
	}

	pCmdUI->SetCheck(IsSearchOptionSet(opt));
}

int BibView::TranslateCompareFunction(LPARAM l1, LPARAM l2, BibView* pane, 
									  const std::function<int (const BibItem& a, const BibItem& b)>& f )
{
	BibItemContainerType::size_type pos1 = static_cast<BibItemContainerType::size_type>(l1);
	BibItemContainerType::size_type pos2 = static_cast<BibItemContainerType::size_type>(l2);

	ASSERT(pos1 < pane->bib_items_.size() && pos2 < pane->bib_items_.size());
	return f(pane->bib_items_[pos1].bib,pane->bib_items_[pos2].bib);
}

int BibView::CompareLabel( const BibItem& a, const BibItem& b )
{
	return a.GetLabel().CompareNoCase(b.GetLabel());
}

int BibView::CompareAuthor( const BibItem& a, const BibItem& b )
{
	return a.GetAuthor().CompareNoCase(b.GetAuthor());
}

int BibView::CompareYear( const BibItem& a, const BibItem& b )
{
	int result;

	if (!a.HasYear() && !b.HasYear())
		result = 0;
	else if (!a.HasYear())
		result = -1;
	else if (!b.HasYear())
		result = 1;
	else {
		int year1 = a.GetYear().GetValueOrDefault();
		int year2 = b.GetYear().GetValueOrDefault();
		
		result = year1 < year2 ? -1 : year1 > year2 ? 1 : 0;
	}

	return result;
}

int BibView::CompareTitle( const BibItem& a, const BibItem& b )
{
	return a.GetTitle().CompareNoCase(b.GetTitle());
}

int BibView::ComparePublicationType( const BibItem& a, const BibItem& b )
{
	return a.GetType() < b.GetType() ? -1 : a.GetType() > b.GetType() ? 1 : 0;
}

int BibView::ComparePublisher( const BibItem& a, const BibItem& b )
{
	return a.GetPublisher().CompareNoCase(b.GetPublisher());
}

void BibView::StartSearch()
{
	if (CEdit* edit = search_button_->GetEditBox()) {
		::InterlockedExchange(&stop_search_,1);

		CString text;
		edit->GetWindowText(text);

		text.Trim();

		if (text.IsEmpty()) {
			// Avoid unnecessary list view refill
			if (list_view_.GetItemCount() != static_cast<int>(bib_items_.size())) {
				Populate(EveryItem);
				static_cast<CFrameWnd*>(AfxGetMainWnd())->SetMessageText(AFX_IDS_IDLEMESSAGE);
			}
		}
		else {
			static_cast<CFrameWnd*>(AfxGetMainWnd())->SetMessageText(IDS_SEARCHING);
			populate_was_search_ = true;

			text.MakeLower();
			using namespace std;
			using namespace placeholders;

			std::vector<CString> words;
			Tokenize(text,_T(" "),words);

			Populate(bind(&BibView::ItemContains,this,_1,words));
		}
	}
}

void BibView::OnStartSearch()
{
	StartSearch();
}

void BibView::OnNMDblClk(NMHDR* /*nm*/, LRESULT*)
{
	GotoItem();
}

void BibView::OnEnter()
{
	bool ctrl_pressed = (::GetKeyState(VK_CONTROL) >> 15 & 1) == 1;

	if (!ctrl_pressed && GetFocus() == search_button_->GetEditBox()) {
		if (search_timer_enabled_) {
			KillTimer(SearchTimerID);
			search_timer_enabled_ = false;
		}

		StartSearch();
	}
	else {
		if (ctrl_pressed) // Insert \cite on Ctrl+Enter
			AfxGetMainWnd()->SendMessage(WM_COMMAND,ID_ITEM_INSERT_REF);
		else // otherwise go to the item and show it in the editor
			GotoItem();
	}
}

void BibView::GotoItem()
{
	AfxGetMainWnd()->PostMessage(WM_COMMAND,ID_ITEM_GOTO);
}

void BibView::OnLvnItemChanged(NMHDR* nm, LRESULT*)
{
	LPNMLISTVIEW p = reinterpret_cast<LPNMLISTVIEW>(nm);

	if (p->uChanged & LVIF_STATE && p->iItem != -1 && p->uNewState & LVIS_SELECTED)
		GetProject()->SetCurrentStructureItem(static_cast<int>
			(bib_items_[list_view_.GetItemData(p->iItem)].structure_item_index));
}

BOOL BibView::PreTranslateMessage(MSG* pMsg)
{
	BOOL handled = FALSE;

	if (search_button_ && search_button_->GetEditBox()
		&& search_button_->GetEditBox()->GetSafeHwnd() == pMsg->hwnd)
	{
		if (pMsg->message == WM_KEYDOWN && GetControlSyncState() == 0)
		{
			// No modifiers
			switch (pMsg->wParam)
			{
				case VK_UP:
				case VK_DOWN:
				//case VK_HOME: case VK_END:
					list_view_.SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
					handled = TRUE;
					break;
			}
		}
	}

	if (!handled)
		handled = WorkspacePaneBase::PreTranslateMessage(pMsg);

	return handled;
}

void BibView::DoPopulate( const PredicateFunctionType& predicate )
{
	list_view_.DeleteAllItems();

	LVITEM lvi = {LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM};

	lvi.mask |= LVIF_GROUPID;

	typedef BibItemContainerType::iterator I;

	for (I it = bib_items_.begin(); it != bib_items_.end() && !stop_search_; ++it) {
		if (predicate(it->bib)) {
			const CString& text = it->bib.GetLabel();
			const_cast<LPCTSTR&>(lvi.pszText) = text;
			lvi.cchTextMax = text.GetLength();
			lvi.iGroupId = it->group_id;
			lvi.iItem = std::distance(bib_items_.begin(),it);
			lvi.iImage = it->image;
			lvi.lParam = lvi.iItem;

			const int index = list_view_.InsertItem(&lvi);

			if (index == -1)
				::InterlockedExchange(&stop_search_, 1);
			else {
				list_view_.SetItemText(index,1,it->bib.GetAuthor());
				list_view_.SetItemText(index,2,it->bib.GetTitle());

				if (it->bib.HasYear()) {
					CString& year = temp_;
					year.Format(_T("%i"),it->bib.GetYear().GetValue());

					list_view_.SetItemText(index,3,year);
				}

				list_view_.SetItemText(index,4,it->bib.GetTypeString());
				list_view_.SetItemText(index,5,it->bib.GetPublisher());
			}
		}
	}

	if (!stop_search_) {
		// Sort only if the search hasn't been interrupted
		if (list_view_.IsSorted())
			list_view_.Sort();
	}

	if (populate_was_search_) {
		if (!stop_search_) {
			CString fmt;
			fmt.Format(IDS_ITEMS_FOUND,list_view_.GetItemCount());
			static_cast<CFrameWnd*>(AfxGetMainWnd())->SetMessageText(fmt);
		}
		else
			static_cast<CFrameWnd*>(AfxGetMainWnd())->SetMessageText(AFX_IDS_IDLEMESSAGE);

		populate_was_search_ = false;
	}
}

UINT BibView::PopulateThread( LPVOID p )
{
	std::auto_ptr<PopulateParam> param(static_cast<PopulateParam*>(p));
	param->view->OnPopulateThread(param->predicate);

	return 0;
}

void BibView::OnPopulateThread( const PredicateFunctionType& predicate )
{
	search_semaphore_.Lock();

	list_view_.SetRedraw(FALSE);

	DoPopulate(predicate);

	list_view_.SetRedraw();
	list_view_.Invalidate();

	search_semaphore_.Unlock();
}

void BibView::OnDestroy()
{
	::InterlockedExchange(&stop_search_, 1);

	MSG msg;

	while (::WaitForSingleObject(search_semaphore_, 0) == WAIT_TIMEOUT) {
		// Let the main window process remaining messages sent from
		// OnPopulateThread, otherwise a dead lock may occur.
		while (::PeekMessage(&msg, list_view_.m_hWnd, 0, 0, PM_REMOVE | PM_QS_SENDMESSAGE) > 0) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	WorkspacePaneBase::OnDestroy();
}
