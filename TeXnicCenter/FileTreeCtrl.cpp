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

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

#include "ShellUtilXP.h"
#include "resource.h"
#include "FileTreeCtrl.h"
#include "FontOccManager.h"
#include "ItemIDList.h"
#include "LatexProject.h"
#include "OleDrop.h"
#include "OutputDoc.h"
#include "global.h"

CString FormatInput(const StructureItem& item)
{
	CString text;
	text.Format(_T("\\input{%s}"), (LPCTSTR)CPathTool::GetFileTitle(item.GetTitle()));

	return text;
}

CString FormatInclude(const StructureItem& item)
{
	CString text;
	text.Format(_T("\\include{%s}"), (LPCTSTR)CPathTool::GetFileTitle(item.GetTitle()));

	return text;
}

namespace {
	CString ExtractFileTitle(const CString& text)
	{
		return CPathTool::GetFileTitle(text);
	}
}

struct FileTreeCtrl::Entry
{
	explicit Entry(ItemIDList&& pidl, const CString& text)
		: pidl(std::forward<ItemIDList>(pidl))
		, text(text)
	{
	}

	virtual ~Entry()
	{
	}

	bool IsMissing() const
	{
		return pidl.IsEmpty();
	}

	virtual int CompareTo(const Entry& other) const;

	ItemIDList pidl;
	ItemIDList oldPidl;
	CString text;
};

struct FileTreeCtrl::FileEntry
	: Entry
{
	explicit FileEntry(ItemIDList&& pidl,
		StructureItemContainer::size_type index, const CString& text)
		: Entry(std::forward<ItemIDList>(pidl), text)
		, itemIndex(index)
	{
	}

	int CompareTo(const Entry& other) const;

	static int GetExtensionPriority(const CString& ext)
	{
		int result;

		if (ext.CompareNoCase(_T("sty")) == 0)
			result = 5;
		else if (ext.CompareNoCase(_T("tex")) == 0)
			result = 4;
		else if (ext.CompareNoCase(_T("bib")) == 0)
			result = 3;
		else if (IsImage(ext))
			result = 2;
		else
			result = 1;

		return result;
	}

	//! Indicates whether the specified @a extension belongs to an image.
	static bool IsImage(const CString& extension)
	{
		// Note: the following array has to be kept sorted
		LPCTSTR const extentions[] = {
			_T("bmp"),
			_T("eps"),
			_T("gif"),
			_T("jpeg"),
			_T("jpg"),
			_T("pdf"),
			_T("png"),
			_T("ps"),
			_T("tif"),
			_T("tiff")
		};

		return std::binary_search(std::begin(extentions), std::end(extentions),
			extension, [](const CString& value, LPCTSTR ext)
		{
			return value.CompareNoCase(ext) < 0;
		});
	}

	StructureItemContainer::size_type itemIndex;
};

int FileTreeCtrl::Entry::CompareTo(const Entry& other) const
{
	// Note: The StrCmpLogicalW function used below allows to sort strings
	// according to their natural order instead of the lexicographical one. For
	// instance, the lexicographically sorted file names 1, 10, 100, 2 will be
	// sorted as 1, 2, 10, 100.

	const CString& ext1 = CPathTool::GetFileExtension(text);
	const CString& ext2 = CPathTool::GetFileExtension(other.text);

	int result;

	if (ext1.CompareNoCase(ext2) == 0) // extensions are equal
		result = StrCmpLogicalW(CPathTool::GetFileTitle(text),
			CPathTool::GetFileTitle(other.text));
	else {
		// Perform chained comparison
		int prio1 = FileEntry::GetExtensionPriority(ext1);
		int prio2 = FileEntry::GetExtensionPriority(ext2);

		result = prio2 - prio1;

		if (result == 0) {
			// same priorities: compare by file title
			result = StrCmpLogicalW(CPathTool::GetFileTitle(text),
				CPathTool::GetFileTitle(other.text));

			if (result == 0) {
				// same file titles: compare by extension
				result = StrCmpLogicalW(ext1, ext2);
			}
		}
	}

	return result;
}

struct FileTreeCtrl::DirectoryEntry
	: Entry
{
	explicit DirectoryEntry(ItemIDList&& pidl, const CString& text)
		: Entry(std::forward<ItemIDList>(pidl), text)
	{
	}

	int CompareTo(const Entry& other) const;
};

int FileTreeCtrl::DirectoryEntry::CompareTo(const Entry& other) const
{
	int result;

	if (dynamic_cast<const FileEntry*>(&other))
		result = -1;
	else
		result = Entry::CompareTo(other);

	return result;
}

int FileTreeCtrl::FileEntry::CompareTo(const Entry& other) const
{
	int result;

	if (dynamic_cast<const DirectoryEntry*>(&other))
		result = 1;
	else
		result = Entry::CompareTo(other);

	return result;
}

FileTreeCtrl::FolderItemIDListLessPredicate::FolderItemIDListLessPredicate
		(CComPtr<IShellFolder> shellFolder)
	: shellFolder(shellFolder)
{
}

FileTreeCtrl::FolderItemIDListLessPredicate::FolderItemIDListLessPredicate()
{
	HRESULT code = SHGetDesktopFolder(&shellFolder);

	if (FAILED(code)) {
		TRACE0("Failed to get the shell folder interface of the desktop\n");
	}
}

bool FileTreeCtrl::FolderItemIDListLessPredicate::operator()
		(const ItemIDList& lhs, const ItemIDList& rhs) const
{
	return static_cast<short>
		(HRESULT_CODE(shellFolder->CompareIDs(0, lhs, rhs))) < 0;
}

IMPLEMENT_DYNAMIC(FileTreeCtrl, NavigatorTreeCtrl)

BEGIN_MESSAGE_MAP(FileTreeCtrl, NavigatorTreeCtrl)
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, &FileTreeCtrl::OnTvnDeleteitem)
	ON_NOTIFY_REFLECT(TVN_GETINFOTIP, &FileTreeCtrl::OnTvnGetInfoTip)
	ON_MESSAGE(AfxUserMessages::ShellUpdateMessageID, &FileTreeCtrl::OnShellChange)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &FileTreeCtrl::OnNMCustomdraw)
END_MESSAGE_MAP()

FileTreeCtrl::FileTreeCtrl()
	: shellNotifyId_(0)
	, missingItemColor_(RGB(255, 0, 0)) // Red
{
	SHGetDesktopFolder(&shellFolder_);
	itemIdTreeItem_.key_comp().shellFolder = shellFolder_;
}

FileTreeCtrl::~FileTreeCtrl()
{
}

int FileTreeCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (NavigatorTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetupImageLists();

	ClearKeyStateToFormat();
	ClearKeyStateToMessage();

	using namespace std;
	using namespace placeholders;

	MapKeyStateToFormat(0, FormatInput);
	MapKeyStateToFormat(MK_CONTROL, FormatInclude);
	MapKeyStateToFormat(MK_SHIFT, bind(ExtractFileTitle, bind(&StructureItem::GetTitle, _1)));

	const DWORD style = TVS_EX_AUTOHSCROLL;
	SetExtendedStyle(style, style);

	return 0;
}

void FileTreeCtrl::OnUpdate(CProjectView* /*pSender*/, LPARAM lHint, LPVOID /*pHint*/)
{
	switch (lHint)
	{
		case COutputDoc::hintParsingFinished :
			OnParsingFinished();
			break;
	}
}

void FileTreeCtrl::OnParsingFinished()
{
	SetRedraw(FALSE);
	ForceClear();

	Initialize();
	Populate();

	SetRedraw();
	Invalidate();
}

void FileTreeCtrl::Populate()
{
	itemIdTreeItem_.clear();

	// How many items do we have in this view currently?  If none, i.e. first
	// filled after loading a project, then expand the zeroth level
	const bool bExpandAll = GetCount() == 0;

	// remember expanded items
	CString strSelectedItem = GetItemPath(GetSelectedItem());
	CStringArray astrExpandedItems;
	GetExpandedItems(astrExpandedItems);

	const StructureItemContainer& items = GetProject()->GetStructureItems();

	const CString& mainDirectory = CPathTool::GetDirectory(GetProject()->GetMainPath());
	const CString& projectDirectory = GetProject()->GetDirectory();
	const CString& projectDirectoryName = CPathTool::GetRelativePath
		(CPathTool::GetParentDirectory(projectDirectory), projectDirectory);

	HTREEITEM projectItem = InsertEntry(TVI_ROOT,
		new DirectoryEntry(GetPathItemIDList(projectDirectory),
		projectDirectoryName));

	CString text;
	HTREEITEM parent;

	// Maps the (flat) relative path to a parent tree item
	typedef std::map<CString, HTREEITEM> ParentDirectoryMap;
	ParentDirectoryMap parents;

	typedef std::vector<HTREEITEM> TreeItemVector;
	TreeItemVector missingItems;

	for (StructureItemContainer::const_iterator it = items.begin();
			it != items.end(); ++it)
	{
		const StructureItem& item = *it;

		if (item.IsCopy())
			continue;

		text = CPathTool::GetFile(it->GetPath());

		CString reldir = CPathTool::GetRelativePath(mainDirectory,
			CPathTool::GetParentDirectory(CPathTool::GetAbsolutePath
				(mainDirectory, it->GetPath())), TRUE, TRUE);

		const StructureItemContainer::const_iterator::difference_type index =
			std::distance(items.begin(), it);

		switch (item.GetType())
		{
			case StructureItem::texFile :
			case StructureItem::missingTexFile :
				parent = projectItem;
				break;
			case StructureItem::graphicFile :
			case StructureItem::missingGraphicFile :
				parent = projectItem;
				break;
			case StructureItem::bibFile :
			case StructureItem::missingBibFile :
				parent = projectItem;
				break;
			default:
				parent = 0;
		}

		if (parent)
		{
			if (reldir != _T("."))
			{
				ParentDirectoryMap::iterator pit = parents.find(reldir);

				if (pit != parents.end())
					parent = pit->second;
				else
				{
					CString component, path;
					int idx;

					LPCTSTR const sep = _T("\\/");
					bool stop = false;

					while (!stop)
					{
						idx = reldir.FindOneOf(sep);

						if (idx == -1)
						{
							idx = reldir.GetLength();
							stop = true;
						}

						component = reldir.Left(idx);
						path += component;

						pit = parents.find(path);

						if (pit != parents.end())
							parent = pit->second;
						else
						{
							const CString& absolutePath =
								CPathTool::GetAbsolutePath(mainDirectory, path);

							parent = InsertEntry(parent,
								new DirectoryEntry(GetPathItemIDList(absolutePath),
									component));

							// but store the whole relative path
							parents.insert(std::make_pair(path, parent));
						}

						if (!stop)
						{
							path += reldir[idx]; // add the \ or /
							reldir.Delete(0, idx + 1);
						}
					}
				}
			}

			ItemIDList pidl = GetPathItemIDList(GetProject()->GetFullPath(item));
			std::unique_ptr<FileEntry> entry(new FileEntry(std::move(pidl), index,
				text));

			HTREEITEM hItem = InsertEntry(parent, entry.get(), &item,
				item.IsMainProjectFile());

			if (entry->IsMissing())
				missingItems.push_back(hItem);

			entry.release();
		}
	}

	// Missing items should be visible
	std::for_each(missingItems.begin(), missingItems.end(),
		std::bind(&FileTreeCtrl::EnsureVisible, this, std::placeholders::_1));

	if (!bExpandAll) {
		ExpandItems(astrExpandedItems);
		SelectItem(GetItemByPath(strSelectedItem));
	}
	else
	{
		ExpandItemsByLevel(0);
		EnsureVisible(GetRootItem());
	}

	// Sort children of all items
	parents.insert(std::make_pair(CString(), projectItem));

	std::for_each(parents.begin(), parents.end(),
		[this](ParentDirectoryMap::const_reference item)
	{
		TVSORTCB cb = { item.second, &FileTreeCtrl::SortComparer,
			reinterpret_cast<LPARAM>(this) };
		SortChildrenCB(&cb);
	});
}

int FileTreeCtrl::GetItemImageIndex(std::size_t index, int flags /*= 0*/)
{
	const StructureItemContainer& si = GetProject()->GetStructureItems();
	const StructureItem& item = si[index];

	const CString& path = GetProject()->GetFullPath(item);

	return GetImageIndex(path, flags);
}

int FileTreeCtrl::GetImageIndex(LPCWSTR path, int flags, DWORD fileAttributes)
{
	SHFILEINFO shfi = { };

	HIMAGELIST systemImageList = reinterpret_cast<HIMAGELIST>
		(::SHGetFileInfo(path, fileAttributes, &shfi, sizeof(shfi), flags |
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_ICON));

	int imageIndex;

	if (systemImageList) {
		imageIndex = shfi.iIcon;
		DestroyIcon(shfi.hIcon);
	}
	else
		imageIndex = -1;

	return imageIndex;
}

int FileTreeCtrl::GetImageIndex(LPCITEMIDLIST pidl, int flags /*= 0*/)
{
	return GetImageIndex(reinterpret_cast<LPCWSTR>(pidl), flags | SHGFI_PIDL);
}

void FileTreeCtrl::Initialize()
{
	if (shellNotifyId_ != 0)
		Unregister();

	const CString& directory = GetProject()->GetDirectory();

	ItemIDList pidl;

	HRESULT code = SHILCreateFromPath(directory, &pidl, NULL);

	if (SUCCEEDED(code)) {
		CComPtr<IShellFolder> shellFolder;
		code = SHBindToObject(shellFolder_, pidl, NULL, IID_IShellFolder,
			reinterpret_cast<void**>(&shellFolder));

		if (SUCCEEDED(code)) {
			projectShellFolder_ = shellFolder;
		}
	}

	Register(directory);
}

void FileTreeCtrl::Register(const CString& path, bool recursive /*= true*/)
{
	ASSERT(shellNotifyId_ == 0);

	ItemIDList pidl = GetPathItemIDList(path);

	SHChangeNotifyEntry e;
	e.pidl = pidl;
	e.fRecursive = recursive;

	const LONG notifications = SHCNE_CREATE | SHCNE_UPDATEITEM |
		SHCNE_RENAMEITEM | SHCNE_DELETE | SHCNE_UPDATEIMAGE;

	shellNotifyId_ = SHChangeNotifyRegister(m_hWnd, SHCNRF_ShellLevel |
		SHCNRF_InterruptLevel, notifications, AfxUserMessages::ShellUpdateMessageID, 1, &e);
}

void FileTreeCtrl::Unregister()
{
	ASSERT(shellNotifyId_);

	SHChangeNotifyDeregister(shellNotifyId_);
	shellNotifyId_ = 0;
}

LRESULT FileTreeCtrl::OnShellChange(WPARAM wParam, LPARAM lParam)
{
	switch (lParam) {
		case SHCNE_CREATE:
			OnShellCreateItem(reinterpret_cast<PIDLIST_ABSOLUTE*>(wParam));
			break;
		case SHCNE_UPDATEITEM:
			OnShellUpdateItem(reinterpret_cast<PIDLIST_ABSOLUTE*>(wParam));
			break;
		case SHCNE_RENAMEITEM:
			OnShellRenameItem(reinterpret_cast<PIDLIST_ABSOLUTE*>(wParam));
			break;
		case SHCNE_DELETE:
			OnShellDeleteItem(reinterpret_cast<PIDLIST_ABSOLUTE*>(wParam));
			break;
		case SHCNE_UPDATEIMAGE:
			OnShellUpdateImage();
			break;
	}

	return 0;
}

void FileTreeCtrl::OnShellCreateItem(PIDLIST_ABSOLUTE* pidls)
{
	MarkItemAsMissing(ItemIDList(pidls[0]), false);
}

void FileTreeCtrl::OnTvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	delete reinterpret_cast<Entry*>(pNMTreeView->itemOld.lParam);

	*pResult = 0;
}

void FileTreeCtrl::OnShellUpdateItem(PIDLIST_ABSOLUTE* pidls)
{
	ItemIDList key(pidls[0]);
	ItemIDListTreeItemMap::const_iterator it = itemIdTreeItem_.find(key);

	if (it == itemIdTreeItem_.end()) {
		it = std::find_if(itemIdTreeItem_.begin(), itemIdTreeItem_.end(),
			[&key] (const ItemIDListTreeItemMap::value_type& item)
			{
				return item.first == key;
			});
	}

	if (it != itemIdTreeItem_.end()) {
		SetRedraw(FALSE);
		UpdateEntryTree(it->second);
		SetRedraw();
	}
}

void FileTreeCtrl::OnShellRenameItem(PIDLIST_ABSOLUTE* pidls)
{
	PIDLIST_ABSOLUTE oldpidl = pidls[0];
	ItemIDList key(oldpidl);

	ItemIDListTreeItemMap::iterator it = itemIdTreeItem_.find(key);

	if (it != itemIdTreeItem_.end()) {
		ItemIDListTreeItemMap::value_type tmp = *it;
		itemIdTreeItem_.erase(it);
		itemIdTreeItem_.insert(std::make_pair(key, it->second));

		MarkItemAsMissing(it->second, true);
	}
}

void FileTreeCtrl::OnShellDeleteItem(PIDLIST_ABSOLUTE* pidls)
{
	PIDLIST_ABSOLUTE oldpidl = pidls[0];
	ItemIDList key(oldpidl);

	MarkItemAsMissing(key);
}

ItemIDList FileTreeCtrl::GetPathItemIDList(const CString& path) const
{
	ItemIDList pidl;

	//SHILCreateFromPath needs a canonical path
	CString CanonicalPath = CPathTool::GetCanonicalPath(path);
	HRESULT code = SHILCreateFromPath(CanonicalPath, &pidl, NULL);

	if (FAILED(code)) {
		TRACE0("CFileView: Failed to convert a path to PIDL\n");
	}

	return pidl;
}

void FileTreeCtrl::UpdateEntryImages(HTREEITEM hItem)
{
	const Entry* entry = reinterpret_cast<const Entry*>(GetItemData(hItem));

	AFXASSUME(entry);

	int normalImageIndex = entry->IsMissing() ? -1 :
		GetImageIndex(entry->pidl, SHGFI_OVERLAYINDEX);
	int stateImageIndex = entry->IsMissing() ? -1 :
		GetImageIndex(entry->pidl);

	SetItem(hItem, TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE, NULL,
		normalImageIndex & 0xffffff, stateImageIndex,
		INDEXTOOVERLAYMASK(normalImageIndex >> 24), TVIS_OVERLAYMASK, 0);
}

void FileTreeCtrl::UpdateEntryTree(HTREEITEM hItem)
{
	// Update the item
	UpdateEntryImages(hItem);

	// Update item's children
	HTREEITEM hChildItem = GetChildItem(hItem);

	while (hChildItem) {
		UpdateEntryImages(hChildItem);
		hChildItem = GetNextSiblingItem(hChildItem);
	}

	// Update item's parents
	HTREEITEM hParent = hItem;

	while ((hParent = GetParentItem(hParent)) != NULL) {
		UpdateEntryImages(hParent);
	}
}

HTREEITEM FileTreeCtrl::InsertEntry(HTREEITEM parent, Entry* entry,
	const StructureItem* item /*= NULL*/, bool bold /*= false*/)
{
	AFXASSUME(entry);

	int normalImageIndex = GetImageIndex(entry->pidl, SHGFI_PIDL |
		SHGFI_OVERLAYINDEX);
	int stateImageIndex = GetImageIndex(entry->pidl, SHGFI_PIDL |
		SHGFI_SELECTED);
	bool iconMissing = normalImageIndex == -1;

	if (iconMissing && item) {
		// The file item is missing so there's no way to find out file's
		// specific icon. Hence, extract file's generic icon by using the file
		// extension.

		CStringW extension;

		switch (item->GetType()) {
			case StructureItem::missingBibFile:
			case StructureItem::bibFile:
				extension = L".bib";
				break;
			case StructureItem::missingTexFile:
			case StructureItem::texFile:
				extension = L".tex";
				break;
			case StructureItem::missingGraphicFile:
			case StructureItem::graphicFile:
				extension = CPathTool::GetFileExtension(item->GetTitle());

				if (extension.IsEmpty())
					extension = L".png"; // assume Portable Network Graphics
				else
					extension.Insert(0, L'.'); // prepend the period

				break;
		}

		normalImageIndex = GetImageIndex(extension, SHGFI_OVERLAYINDEX |
			SHGFI_USEFILEATTRIBUTES, FILE_ATTRIBUTE_NORMAL);
		stateImageIndex = GetImageIndex(extension, SHGFI_SELECTED |
			SHGFI_USEFILEATTRIBUTES, FILE_ATTRIBUTE_NORMAL);
	}
	else if (iconMissing && dynamic_cast<const DirectoryEntry*>(entry)) {
		// The folder doesn't exist: query the generic directory icon of the
        // project folder.
		const CStringW& folder = GetProject()->GetMainPath();

		normalImageIndex = GetImageIndex(folder, SHGFI_USEFILEATTRIBUTES,
			FILE_ATTRIBUTE_DIRECTORY);
		stateImageIndex = GetImageIndex(folder, SHGFI_SELECTED |
			SHGFI_USEFILEATTRIBUTES, FILE_ATTRIBUTE_DIRECTORY);
	}

	UINT state = INDEXTOOVERLAYMASK(normalImageIndex >> 24 & 0xff);
	UINT stateMask = TVIS_OVERLAYMASK;

	if (bold) {
		state |= TVIS_BOLD;
		stateMask |= TVIS_BOLD;
	}

	parent = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE |
		TVIF_SELECTEDIMAGE | TVIF_STATE, entry->text,
		normalImageIndex & 0xffffff, stateImageIndex, state, stateMask,
		reinterpret_cast<LPARAM>(entry), parent, TVI_LAST);

	if (!entry->pidl.IsEmpty())
		itemIdTreeItem_.insert(std::make_pair(entry->pidl, parent));

	return parent;
}

std::size_t FileTreeCtrl::GetItemIndex(HTREEITEM hItem) const
{
	const FileEntry* entry = dynamic_cast<const FileEntry*>
		(reinterpret_cast<const Entry*>(GetItemData(hItem)));

	AFXASSUME(entry);

	return entry->itemIndex;
}

bool FileTreeCtrl::IsFolder(HTREEITEM item) const
{
	return dynamic_cast<const DirectoryEntry*>
		(reinterpret_cast<const Entry*>(GetItemData(item))) != NULL;
}

void FileTreeCtrl::OnTvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMTVGETINFOTIP>(pNMHDR);

	const Entry* entry = reinterpret_cast<const Entry*>(pGetInfoTip->lParam);

	AFXASSUME(entry);

	CComPtr<IShellFolder> sf;
	PCUITEMID_CHILD childPidl;

	HRESULT code = SHBindToParent(entry->pidl, IID_IShellFolder,
		reinterpret_cast<void**>(&sf), &childPidl);

	if (FAILED(code))
		return;

	CComPtr<IQueryInfo> queryInfo;

	if (SUCCEEDED(sf->GetUIObjectOf(m_hWnd, 1, &childPidl, IID_IQueryInfo, NULL,
		reinterpret_cast<void**>(&queryInfo)))) {
		LPWSTR text = 0;

		if (SUCCEEDED(queryInfo->GetInfoTip(0, &text))) {
			std::size_t n = std::min<std::size_t>(pGetInfoTip->cchTextMax - 1,
				std::wcslen(text));

			USES_CONVERSION_EX;
			std::memcpy(pGetInfoTip->pszText, W2CT_EX(text, static_cast<UINT>(n)),
				n * sizeof(TCHAR));

			pGetInfoTip->pszText[n] = 0;
			pGetInfoTip->cchTextMax = static_cast<int>(n);

			::CoTaskMemFree(text);
		}
	}

	*pResult = 0;
}

LRESULT FileTreeCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	switch (message) {
		case WM_MENUCHAR:
		case WM_INITMENUPOPUP:
		case WM_DRAWITEM:
		case WM_MEASUREITEM:
			{
				bool handled = false;

				if (contextMenu3_)
					handled = SUCCEEDED(contextMenu3_->HandleMenuMsg2(message,
						wParam, lParam, &result));
				else if (contextMenu2_)
					handled = SUCCEEDED(contextMenu2_->HandleMenuMsg(message,
						wParam, lParam));
				else
					handled = false;

			if (handled)
				break;
			}
		default:
			result = NavigatorTreeCtrl::WindowProc(message, wParam, lParam);
	}

	return result;
}

void FileTreeCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	ShowContextMenu(point);
}

void FileTreeCtrl::ShowContextMenu(CPoint& point)
{
	HTREEITEM hItem;

	if (point.x == -1 && point.y == -1) {
		// The display of the context menu has been issue using the keyboard:
		// display the menu below the bounding rectangle of the selected item
		hItem = GetSelectedItem();
	}
	else {
		CPoint tmp(point);
		ScreenToClient(&tmp);

		hItem = HitTest(tmp);

		if (hItem)
			SelectItem(hItem);
		else
			hItem = GetSelectedItem();
	}

	if (!hItem)
		return;

	CRect rect;

	GetItemRect(hItem, &rect, TRUE);
	ClientToScreen(&rect);

	const Entry* entry = reinterpret_cast<const Entry*>(GetItemData(hItem));

	AFXASSUME(entry);

	CComPtr<IShellFolder> sf;
	PCUITEMID_CHILD childPidl;

	HRESULT code = SHBindToParent(entry->pidl, IID_IShellFolder,
		reinterpret_cast<void**>(&sf), &childPidl);

	if (FAILED(code))
		return;

	CComPtr<IContextMenu> cm;

	code = sf->GetUIObjectOf(m_hWnd, 1, &childPidl, IID_IContextMenu, NULL,
		reinterpret_cast<void**>(&cm));

	if (FAILED(code))
		return;

	contextMenu2_ = cm;
	contextMenu3_ = cm;

	CMenu menu;
	menu.CreatePopupMenu();

	UINT queryFlags = CMF_EXPLORE;

	if (GetKeyState(VK_SHIFT) < 0)
		queryFlags |= CMF_EXTENDEDVERBS;

	if (FAILED(cm->QueryContextMenu(menu, 0, 1, 0x7fff, queryFlags)))
		return;

	TPMPARAMS tpmp = { sizeof(TPMPARAMS), rect };

	UINT id = menu.TrackPopupMenuEx(TPM_VERTICAL | TPM_LEFTALIGN | TPM_TOPALIGN |
		TPM_RETURNCMD, rect.left, rect.bottom, this, &tpmp);

	if (contextMenu3_)
		contextMenu3_.Release();

	if (contextMenu2_)
		contextMenu2_.Release();

	if (id) {
		CMINVOKECOMMANDINFO cmi = { sizeof(CMINVOKECOMMANDINFO) };
		cmi.hwnd = m_hWnd;
		cmi.lpVerb = MAKEINTRESOURCEA(id - 1);
		cmi.nShow = SW_SHOWDEFAULT;

		if (GetKeyState(VK_SHIFT) < 0)
			cmi.fMask |= CMIC_MASK_SHIFT_DOWN;

		if (GetKeyState(VK_CONTROL) < 0)
			cmi.fMask |= CMIC_MASK_CONTROL_DOWN;

		cm->InvokeCommand(&cmi);
	}
}

void FileTreeCtrl::Refresh()
{
	HTREEITEM hItem = GetSelectedItem();

	if (hItem) {
		const Entry* entry = reinterpret_cast<const Entry*>(GetItemData(hItem));

		AFXASSUME(entry);

		SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_IDLIST | SHCNF_FLUSH |
			SHCNF_NOTIFYRECURSIVE, entry->pidl, NULL);
	}
}

void FileTreeCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMTVCUSTOMDRAW>(pNMHDR);

	switch (pNMCD->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:
			{
				const Entry* entry = reinterpret_cast<const Entry*>
					(pNMCD->nmcd.lItemlParam);

				if (entry->IsMissing())
					pNMCD->clrText = missingItemColor_;

				*pResult = CDRF_DODEFAULT;
			}
			break;
		default:
			*pResult = CDRF_DODEFAULT;
	}
}

void FileTreeCtrl::MarkItemAsMissing(HTREEITEM hItem, bool missing /*= true*/)
{
	Entry* entry = reinterpret_cast<Entry*>(GetItemData(hItem));

	AFXASSUME(entry);

	if (entry->pidl.IsEmpty() && !missing || !entry->pidl.IsEmpty() && missing)
		std::swap(entry->pidl, entry->oldPidl);

	if (missing) {
		const UINT noImage = ~0U;

		SetItem(hItem, TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE, NULL,
			noImage, noImage, static_cast<UINT>(INDEXTOOVERLAYMASK(-1)),
			TVIS_OVERLAYMASK, 0);
	}
	else
		UpdateEntryImages(hItem);

	CRect rect;
	GetItemRect(hItem, &rect, FALSE);

	InvalidateRect(&rect);
}

void FileTreeCtrl::MarkItemAsMissing(const ItemIDList& key, bool missing)
{
	ItemIDListTreeItemMap::iterator it = itemIdTreeItem_.find(key);

	if (it == itemIdTreeItem_.end()) {
		TRACE0("FileTreeCtrl: Search using IShellFolder::CompareIDs failed. ");
		TRACE0("Using binary comparison to find the PIDL of the deleted file.\n");

		// The search using IShellFolder::CompareIDs will likely fail for files
		// that are being deleted. Try to find an ItemIDList (binary) equal to
		// the key

		it = std::find_if(itemIdTreeItem_.begin(), itemIdTreeItem_.end(),
			[&key] (const ItemIDListTreeItemMap::value_type& item)
			{
				return item.first == key;
			});
	}
	else
		TRACE0("FileTreeCtrl: Found deleted item using IShellFolder::CompareIDs.\n");

	if (it != itemIdTreeItem_.end()) {
		HTREEITEM hItem = it->second;
		MarkItemAsMissing(hItem, missing);
	}
}

void FileTreeCtrl::Clear()
{
	NavigatorTreeCtrl::Clear();
	ForceClear();
}

void FileTreeCtrl::SetMissingItemColor(COLORREF value, bool redraw /*= true*/)
{
	missingItemColor_ = value;

	if (m_hWnd && redraw)
		Invalidate();
}

COLORREF FileTreeCtrl::GetMissingItemColor() const
{
	return missingItemColor_;
}

void FileTreeCtrl::ForceClear()
{
	DeleteAllItems();
	itemIdTreeItem_.clear();
}

void FileTreeCtrl::OnShellUpdateImage()
{
	SetupImageLists();
}

void FileTreeCtrl::SetupImageLists()
{
	SHFILEINFO shfi = { };

	HIMAGELIST imageList;

	imageList = reinterpret_cast<HIMAGELIST>(::SHGetFileInfo(_T(""), 0, &shfi,
		sizeof(shfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_ICON));

	TreeView_SetImageList(m_hWnd, imageList, TVSIL_NORMAL);

	imageList = reinterpret_cast<HIMAGELIST>(::SHGetFileInfo(_T(""), 0, &shfi,
		sizeof(shfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_ICON |
		SHGFI_SELECTED));

	TreeView_SetImageList(m_hWnd, imageList, TVSIL_STATE);
}

int CALLBACK FileTreeCtrl::SortComparer(LPARAM l1, LPARAM l2, LPARAM p)
{
	return reinterpret_cast<FileTreeCtrl*>(p)->SortComparer(l1, l2);
}

int FileTreeCtrl::SortComparer(LPARAM l1, LPARAM l2)
{
	const Entry* lhs = reinterpret_cast<const Entry*>(l1);
	const Entry* rhs = reinterpret_cast<const Entry*>(l2);

	AFXASSUME(lhs);
	AFXASSUME(rhs);

	return lhs->CompareTo(*rhs);
}
