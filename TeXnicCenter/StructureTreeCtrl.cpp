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
 *	 http://www.ToolsCenter.org
 *
 *********************************************************************/

/********************************************************************
 *
 * $Id: structureview.cpp,v 1.11 2005/07/03 16:49:35 niteria Exp $
 *
 ********************************************************************/

#include "stdafx.h"

#include <vector>

#include "resource.h"
#include "BibItem.h"
#include "StructureTreeCtrl.h"
#include "StructureParser.h"
#include "LatexProject.h"
#include "FontOccManager.h"
#include "OutputDoc.h"
#include "TeXnicCenter.h"
#include "global.h"


IMPLEMENT_DYNCREATE(StructureTreeCtrl, NavigatorTreeCtrl)

BEGIN_MESSAGE_MAP(StructureTreeCtrl, NavigatorTreeCtrl)
	ON_MESSAGE(AfxUserMessages::GotoBibliographyLine, &StructureTreeCtrl::OnGotoBibliographyLine)
END_MESSAGE_MAP()

void StructureTreeCtrl::OnUpdate(CProjectView* /*pSender*/, LPARAM lHint, LPVOID /*pHint*/)
{
	switch (lHint)
	{
		case COutputDoc::hintParsingFinished :
		{
			OnParsingFinished();
		}
		break;
	}
}

void StructureTreeCtrl::OnParsingFinished()
{
	//How many items do we have in this view currently?
	//If none, index.e. first filled after loading a project, then expand the zeroth level
	const bool bExpandAll = (GetCount() == 0);

	// remember expanded items
	CString strSelectedItem = GetItemPath(GetSelectedItem());
	CStringArray astrExpandedItems;
	GetExpandedItems(astrExpandedItems);

	// fill view with parsing results
	const StructureItemContainer &a = GetProject()->GetStructureItems();
	
	typedef std::vector<HTREEITEM> InsertedItemsContainer;
	InsertedItemsContainer insertedItems(a.size());

	//Lock
	SetRedraw(FALSE);
	// empty view
	DeleteAllItems();

	HTREEITEM currentItem;

	// fill view
	for (StructureItemContainer::const_iterator it = a.begin(); it != a.end(); ++it)
	{
		const StructureItem &si = *it;//a.GetAt(index);
		StructureItemContainer::difference_type index = std::distance(a.begin(), it);

		switch (si.GetType())
		{
		case StructureItem::equation :
		case StructureItem::figure :
		case StructureItem::table :
		case StructureItem::header :
		case StructureItem::bibFile:
		case StructureItem::missingBibFile:
			//case CStructureParser::unknownEnv:
			{
				CString title = si.GetTitle();
				StructureItemContainer::difference_type parent = si.GetParent();

				if (si.GetType() == StructureItem::header)
					title.Replace(_T("\\-"),_T("")); // Cleanup \-
				else if (si.GetType() == StructureItem::bibFile || si.GetType() == StructureItem::missingBibFile) 
				{
					CString path = si.GetPath();

					if (!si.GetPrefix().IsEmpty())
						path = si.GetPrefix() + _T(", ") + path;

					title.Format(IDS_BIBLIOGRAPHY, (LPCTSTR)path);
					parent = -1; // Bibliography items are always root nodes
				}

				int imageIndex = si.GetType();

				if (si.GetType() == StructureItem::header && si.IsMissing())
				{
					imageIndex = StructureItem::missingTexFile;
				}

				if (parent <= -1)
					currentItem = InsertItem(title, imageIndex, imageIndex);
				else 
					currentItem = InsertItem(title, imageIndex, imageIndex, insertedItems[parent]);

				insertedItems[index] = currentItem;

				// remember Array-Index
				SetItemData(currentItem, index);

				if (si.IsMissing() || si.HasParent() && si.GetParent(a)->IsMissing())
					EnsureVisible(currentItem);

				if (si.GetType() == StructureItem::bibFile)
				{
					InsertBibItems(si, si.IsCopy() ? si.GetOriginal() : index, currentItem);
				}
			}
			break;
		}
	}

	//try to expand items
	if (!bExpandAll)
	{
		ExpandItems(astrExpandedItems);
		SelectItem(GetItemByPath(strSelectedItem));
	}
	else
	{
		ExpandItemsByLevel(0);
		EnsureVisible(GetNextItem(NULL, TVGN_ROOT));
	}

	//Unlock
	SetRedraw();
	Invalidate();
}

bool StructureTreeCtrl::IsFolder( HTREEITEM ) const
{
	// No folders here
	return false;
}

LRESULT StructureTreeCtrl::OnGotoBibliographyLine( WPARAM w, LPARAM )
{
	StructureItemContainer::difference_type index = static_cast<StructureItemContainer::difference_type>(w);
	const StructureItem& si = GetProject()->GetStructureItems()[index];

	theApp.OpenLatexDocument(si.GetComment(), FALSE, si.GetLine());

	return 0;
}

void StructureTreeCtrl::GotoItem( HTREEITEM item )
{
	StructureItemContainer::difference_type index = GetItemData(item);

	try 
	{
		const StructureItem& si = GetProject()->GetStructureItems().at(index);
		
		if (si.GetType() != StructureItem::bibFile)
			__super::GotoItem(item);
		else
		{
			PostMessage(AfxUserMessages::GotoBibliographyLine, index);
		}
	}
	catch (std::out_of_range&) 
	{ 
	}
}

void StructureTreeCtrl::InsertBibItems(const StructureItem& /*item*/, StructureItemContainer::difference_type index,
									   HTREEITEM parentItem )
{
	const StructureItemContainer& items = GetProject()->GetStructureItems();
	
	// Bib items follow the parent
	StructureItemContainer::const_iterator it = items.begin();
	std::advance(it, index + 1);

	bool stop = false;

	for (; it != items.end() && !stop; ++it)
	{
		if (it->GetType() == StructureItem::bibItem)
		{
			const BibItem* bib = dynamic_cast<const BibItem*>(it->GetItemInfo());

			if (bib)
			{
				int imageIndex = it->GetType();

				InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM,
					bib->GetLabel(), 
					imageIndex, imageIndex, 0, 0, std::distance(items.begin(), it), 
					parentItem, TVI_SORT);
			}
		}
		else
			stop = true;
	}
}

bool StructureTreeCtrl::ShouldBeExpanded( HTREEITEM item ) const
{
	return GetProject()->GetStructureItems()[GetItemData(item)].GetType() != StructureItem::bibFile;
}