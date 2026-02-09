/********************************************************************
 *
 * This file is part of the TeXnicCenter-system
 *
 * Copyright (C) 2002 Chris Norris
 * Copyright (C) 2002-$CurrentYear$ ToolsCenter
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

// ParseOutputView.cpp: implementation of the CParseOutputView class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "MainFrm.h"
#include "ParseOutputView.h"
#include "OutputDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CParseOutputView,COutputView)
	//{{AFX_MSG_MAP(CParseOutputView)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CParseOutputView,COutputView)


CParseOutputView::CParseOutputView()
{
}

CParseOutputView::~CParseOutputView()
{
}

void CParseOutputView::OnUpdate(COutputView* pSender,LPARAM lHint,CObject* pHint) //UPDATE
{
	if (pSender == this)
		return;

	switch ((COutputDoc::HINT)lHint)   //UPDATE
	{
		case COutputDoc::hintSelectParseLine :
			SelectLine(reinterpret_cast<Nullable<int>*>(pHint)->GetValue(), FALSE);
			break;
	}
}

#ifdef _DEBUG

void CParseOutputView::AssertValid() const
{
	COutputView::AssertValid();
}

void CParseOutputView::Dump(CDumpContext& dc) const
{
	COutputView::Dump(dc);
}
#endif //_DEBUG

int CParseOutputView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (COutputView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_images.m_hImageList = ::ImageList_LoadImage(AfxGetResourceHandle(),
	                        MAKEINTRESOURCE(IDB_PARSE_VIEW),16,1,RGB(255,0,255),IMAGE_BITMAP,
	                        LR_CREATEDIBSECTION);

	SetImageList(&m_images);//TODO: we get an access violation on destruction of this class because of images. Difficult to reproduce.

	return 0;
}

void CParseOutputView::Visit(OutputViewVisitor& v)
{
	v.Accept(this);
}
