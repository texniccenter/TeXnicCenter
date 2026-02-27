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

#pragma once

/** Handles drawing the preview image.
*/
class CPreviewImageView : public CWnd
{
//Friends
//Types
public:
	friend class PreviewImagePane;

// construction/destruction
public:
	CPreviewImageView();
	virtual ~CPreviewImageView(){};

// operations
public:
	BOOL Create(const RECT& rect, CWnd* pwndParent);

//Overrides and messages
protected:
	DECLARE_MESSAGE_MAP()
	//Double-buffered drawing, fitting image to window
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();

	//Implements pan/zoom interface
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

//Methods
protected:
	/** Zooms into or out of the image.
	* 
	* @param nSteps
	* If the sign of @c nSteps is negative, then we zoom out.
	* Otherwise, we zoom in. Each step represents 10 percent of @c PreviousDrawRect.
	* 
	* @param GravityPoint
	* Adjust the position of the image such that this point coincides with the same pixel.
	*/
	void Zoom(const int nSteps, CPoint GravityPoint = CPoint(-1, -1));

	///Activates a mode in which the image will always be fit fully into the window.
	void ZoomFit();

	///Fills the given data structures with the rectangle describing the window, and a padded version for drawing.
	void GetViewRects(CRect& FullClientRect, CRect& WinRect);

private:
	///Returns a scaled source rectangle fitted and centered into target.
	CRect FitRectIntoRect(const CRect& Target, const CRect& Source);

	///Make sure the source overlaps with the target. If not, adjust position or even size.
	CRect CoverRectByRect(const CRect& Target, const CRect& Source);

	///Adjusts the range [c, d] such that it covers [a, b], while keeping (d-c) constant.
	void Cover1D(const LONG& a, const LONG& b, LONG& c, LONG& d);

//Attributes
protected:
	///The image to be shown.
	CImage PreviewImage;

	///Whether or not to always fit the image into the window size
	bool bFitImageToWindow;

private:
	///The rectangle describing the scaling and positioning
	///of the current/previous image.
	CRect PreviousDrawRect;

	///Padding we would like to have around the edges.
	int Padding;

	///Previous position of the mouse cursor; used for panning.
	CPoint PreviousMousePos;
	bool bValidPreviousMousePos;

	///A factor indicating how much smaller/larger the DPI needs to be
	//such that 1 pixel in the image covers 1 pixel on screen.
	int BestDPI;
};
