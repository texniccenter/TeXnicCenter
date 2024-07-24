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
#include "PreviewImageView.h"
#include "mainfrm.h"
#include "OutputDoc.h"
#include "global.h"

BEGIN_MESSAGE_MAP(CPreviewImageView, CWnd)
	ON_WM_PAINT()
	ON_MESSAGE(AfxUserMessages::PreviewImageViewUpdate, &CPreviewImageView::Update)
	ON_MESSAGE(AfxUserMessages::PreviewImageViewStartProgressAnimation, &CPreviewImageView::StartProgress)
	ON_MESSAGE(AfxUserMessages::PreviewImageViewStopProgressAnimation, &CPreviewImageView::StopProgress)
END_MESSAGE_MAP()


LRESULT CPreviewImageView::Update(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	//Invalidate the view to force a redraw.
	Invalidate();

	//Get image path and load it
	CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	if (pMainFrame && pMainFrame->GetOutputDoc())
	{
		CString Path = pMainFrame->GetOutputDoc()->GetPreviewImagePath();
		if (!CPathTool::Exists(Path)) return 0L;

		PreviewImage.Destroy();
		HRESULT res = PreviewImage.Load(Path);
		if (FAILED(res)) return 0L;
	}

	return 0L;
}

LRESULT CPreviewImageView::StartProgress(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return LRESULT();
}

LRESULT CPreviewImageView::StopProgress(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return LRESULT();
}

BOOL CPreviewImageView::Create(const RECT& rect, CWnd* pwndParent)
{
	ASSERT(pwndParent && IsWindow(pwndParent->m_hWnd));

	//We redraw everything on horizontal/vertical size changes, not just the new parts,
	//since we re-position and re-size the preview image.
	UINT ClassStyle = 0;
	ClassStyle |= CS_VREDRAW;
	ClassStyle |= CS_HREDRAW;

	return CWnd::CreateEx(WS_EX_CLIENTEDGE,
						  AfxRegisterWndClass(ClassStyle, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
						  0, WS_CHILD | WS_VISIBLE, rect, pwndParent, 0);
}


void CPreviewImageView::OnPaint()
{
	//CImage Img;
	//HRESULT res = Img.Load(_T("D:\\Temp\\TXCLenseTest\\txcpreview\\preview.png"));
	//if (FAILED(res)) return;

	CPaintDC dc(this); // device context for painting
	CRect FullClientRect;
	GetClientRect(&FullClientRect);

	// Fill the entire dc with white
	dc.FillSolidRect(&FullClientRect, RGB(255, 255, 255));

	//Draw the image with correct size.
	if (!PreviewImage.IsNull())
	{
		//Padding. We fix it here and see how that will work out.
		const int Padding = 10;
		CRect DestRect(FullClientRect);
		DestRect.DeflateRect(Padding, Padding);

		//Take care of the aspect ratio wrt. the padded client rect
		const int ImgWidth = PreviewImage.GetWidth();
		const int ImgHeight = PreviewImage.GetHeight();
		const double AspectRatio = double(ImgWidth) / double(ImgHeight);
		int ScaledWidth, ScaledHeight;
		int StartX = Padding;
		int StartY = Padding;
		if (DestRect.Width() / AspectRatio <= DestRect.Height())
		{
			//We are bound by the width.
			ScaledWidth = DestRect.Width();
			ScaledHeight = static_cast<int>(ScaledWidth / AspectRatio);
			StartY = (DestRect.Height() - ScaledHeight) / 2;
		}
		else
		{
			//We are bound by the height.
			ScaledHeight = DestRect.Height();
			ScaledWidth = static_cast<int>(ScaledHeight * AspectRatio);
			StartX = (DestRect.Width() - ScaledWidth) / 2;
		}

		//Draw the scaled image onto the device context
		dc.SetStretchBltMode(HALFTONE);
		PreviewImage.Draw(dc.m_hDC, StartX, StartY, ScaledWidth, ScaledHeight);
		//PreviewImage.StretchBlt(dc.m_hDC, 0, 0, ScaledWidth, ScaledHeight, SRCCOPY);
	}
}
