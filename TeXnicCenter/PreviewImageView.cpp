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

BEGIN_MESSAGE_MAP(CPreviewImageView, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_XBUTTONDOWN()
	ON_WM_XBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

CPreviewImageView::CPreviewImageView()
	:Padding(10)
	,bFitImageToWindow(true)
	,PreviousMousePos(-1, -1)
	,bValidPreviousMousePos(false)
	,CurrentZoomFactor(1.0)
{
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


BOOL CPreviewImageView::OnEraseBkgnd(CDC* /*pDC*/)
{
	//We say with TRUE that we erased the background, but we didn't.
	//We will actually override later in OnPaint.
	//This seems necessary for flicker-free, double-buffered drawing.
	return TRUE;
}


void CPreviewImageView::OnPaint()
{
	//The device context for this window, and its size
	CPaintDC dc(this);
	CRect FullClientRect, WinRect;
	GetViewRects(FullClientRect, WinRect);

	//A compatible memory device context for double-buffered drawing
	CDC MemoryDC;
	if (!MemoryDC.CreateCompatibleDC(&dc)) return;
	CBitmap Bitmap;
	Bitmap.CreateCompatibleBitmap(&dc, FullClientRect.Width(), FullClientRect.Height());
	MemoryDC.SelectObject(&Bitmap);

	// Fill the entire dc with white
	MemoryDC.FillSolidRect(&FullClientRect, RGB(255, 255, 255));

	//Draw the image with correct size.
	if (!PreviewImage.IsNull())
	{
		//Get infos about image and window
		CRect CurrentSourceRect(0, 0, PreviewImage.GetWidth(), PreviewImage.GetHeight());
		CRect DrawRect; 

		//Allow zooming into the image, or fit it perfectly?
		if (bFitImageToWindow)
		{
			//Fit image into view
			DrawRect = FitRectIntoRect(WinRect, CurrentSourceRect);
		}
		else
		{
			//Fit image into the previous drawing rectangle, which gets manipulated by pan/zoom outside this function.
			DrawRect = FitRectIntoRect(PreviousDrawRect, CurrentSourceRect);
			//Make sure it overlaps with the actual window. If not, adjust position or even zoom.
			DrawRect = CoverRectByRect(WinRect, DrawRect);
		}

		//Safety and record keeping
		if (DrawRect.Width() <= 0 || DrawRect.Height() <= 0) DrawRect.InflateRect(Padding, Padding); //Draw() does not like empty rectangles.
		// - keep track of how we map the image, so we can do something similar in the next call.
		PreviousDrawRect = DrawRect;

		//Figure out the current scale factor. We can show it to the user
		//and we need it for proper translation when zooming with the mouse wheel below.
		CurrentZoomFactor = (double)CurrentSourceRect.Width() / DrawRect.Width();

		//Draw the scaled image onto the memory device context with proper anti-aliasing
		MemoryDC.SetStretchBltMode(HALFTONE);
		PreviewImage.Draw(MemoryDC.m_hDC, DrawRect);
	}

	//Copy prepared bitmap in one step to the screen device context
	dc.BitBlt(0, 0, FullClientRect.Width(), FullClientRect.Height(), &MemoryDC, 0, 0, SRCCOPY);
}


void CPreviewImageView::GetViewRects(CRect& FullClientRect, CRect& WinRect)
{
	GetClientRect(&FullClientRect);
	WinRect = FullClientRect;
	//Padding. We fix it here and see how that will work out.
	WinRect.DeflateRect(Padding, Padding);
}


CRect CPreviewImageView::FitRectIntoRect(const CRect& Target, const CRect& Source)
{
	//> 1 means the rectangle is wider than high. < 1 means it is higher than wide.
	const double AspectTarget = (double)Target.Width() / Target.Height();
	const double AspectSource = (double)Source.Width() / Source.Height();

	//Adjust based on width or height
	const double Factor = (AspectSource > AspectTarget) ? ((double)Target.Width() / Source.Width()) : ((double)Target.Height() / Source.Height());

	//Scale according to factor
	const int NewWidth = round(Factor * Source.Width());
	const int NewHeight = round(Factor * Source.Height());

	//Center it
	const int XShift = (Target.Width() - NewWidth) / 2;
	const int YShift = (Target.Height() - NewHeight) / 2;
	return CRect(Target.left + XShift, Target.top + YShift,
					Target.left + XShift + NewWidth, Target.top + YShift + NewHeight);
}


CRect CPreviewImageView::CoverRectByRect(const CRect& Target, const CRect& Source)
{
	//Do we need to adjust the zoom? That is basically a fit!
	if (Source.Width() < Target.Width() && Source.Height() < Target.Height())
	{
		return FitRectIntoRect(Target, Source);
	}

	//Ensure covering in both dimensions by adjusting the position.
	CRect CoverRect(Source);
	Cover1D(Target.left, Target.right, CoverRect.left, CoverRect.right);
	Cover1D(Target.top, Target.bottom, CoverRect.top, CoverRect.bottom);

	return CoverRect;
}


void CPreviewImageView::Cover1D(const LONG& a, const LONG& b, LONG& c, LONG& d)
{
	if (d-c < b-a) //Source is smaller
	{
		//Center it
		const LONG cd(d-c);
		c = a + ((b-a) - (d-c))/2;
		d = c + cd;
	}
	else //Source is larger
	{
		if (c > a) //Leaves space on the left
		{
			d -= c-a;
			c = a;
		}

		if (d < b) //Leaves space on the right
		{
			c += b-d;
			d = b;
		}
	}
}


void CPreviewImageView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (nFlags & MK_LBUTTON)
	{
		if (bValidPreviousMousePos)
		{
			//Compute translation vector
			PreviousDrawRect.OffsetRect(point - PreviousMousePos);
		}

		//Save current position to have a translation vector for next call
		PreviousMousePos = point;
		bValidPreviousMousePos = true;
		//TODO: Consider to change the mouse cursor to a hand.

		Invalidate(); //Force a redraw.
	}
	else
	{
		bValidPreviousMousePos = false;
	}
}


BOOL CPreviewImageView::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint pt)
{
	//Determine where the cursor is, so Zoom() can adjust the position of the image
	ScreenToClient(&pt);
	Zoom(zDelta / WHEEL_DELTA, pt);
	return TRUE;
}


void CPreviewImageView::Zoom(const int nSteps, CPoint GravityPoint /*= CPoint(-1, -1)*/)
{
	if (nSteps == 0) return;

	//We increase/decrease the rect by steps of 10 percent
	const double Factor = 0.1 * nSteps;
	const int HorizontalAdjust = round(Factor * PreviousDrawRect.Width());
	const int VerticalAdjust = round(Factor * PreviousDrawRect.Height());

	//When we zoom out and we hit the lower limit, we snap back into fitting mode
	CRect FullClientRect, WinRect;
	GetViewRects(FullClientRect, WinRect);
	if (PreviousDrawRect.Width() + HorizontalAdjust <= WinRect.Width()
		&& PreviousDrawRect.Height() + VerticalAdjust <= WinRect.Height())
	{
		bFitImageToWindow = true;
	}
	else
	{
		//////////// Actual Zooming ////////////
		bFitImageToWindow = false;

		//The center of the window is the default gravity point for the zoom.
		if (GravityPoint.x < 0 || GravityPoint.y < 0)
		{
			GravityPoint = FullClientRect.CenterPoint();
		}

		//Where is the gravity point wrt. to the drawing rectangle?
		const double GravityPercentage[] =
		{
			(double)(GravityPoint.x - PreviousDrawRect.left) / (PreviousDrawRect.right - PreviousDrawRect.left),
			(double)(GravityPoint.y - PreviousDrawRect.top) / (PreviousDrawRect.bottom - PreviousDrawRect.top)
		};

		//How much we adjust on either side depends on the gravity
		const int LeftAdjust = GravityPercentage[0] * HorizontalAdjust;
		const int RightAdjust = HorizontalAdjust - LeftAdjust;
		const int TopAdjust = GravityPercentage[1] * VerticalAdjust;
		const int BottomAdjust = VerticalAdjust - TopAdjust;

		//Change the drawing rectangle, i.e., zoom in or out
		PreviousDrawRect.left -= LeftAdjust;
		PreviousDrawRect.top -= TopAdjust;
		PreviousDrawRect.right += RightAdjust;
		PreviousDrawRect.bottom += BottomAdjust;
	}

	Invalidate(); //Force a redraw.
}


void CPreviewImageView::ZoomFit()
{
	bFitImageToWindow = true;
	
	Invalidate(); //Force a redraw.
}

