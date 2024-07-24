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

#pragma once

#include "OutputViewBase.h"
#include "OutputViewVisitor.h"

class COutputDoc;

class COutputView : public CListCtrl, public OutputViewBase
{
	DECLARE_DYNAMIC(COutputView)

public:
	COutputView();
	virtual ~COutputView();

	virtual void Visit(OutputViewVisitor& v) = 0;

// Attribute
public:
	COutputDoc *GetDocument() const;

// Operationen
public:
	/** Creates the view */
	BOOL Create(const RECT &rect, CWnd *pwndParent);

	/** Attaches the specified document to the view */
	void AttachDoc(COutputDoc* pDocument);

	/** Clears the view */
	void Clear();

	/**
	Adds a line to the view.

	@param lpszLine
	        Line to add to the view.
	@param nImage
	        Index of the image to display or -1 if no image should be
	        displayed for the new line.
	@param nIndent
	        Indent level of line. Zero (default value) indicates no indentation
	 */
	void AddLine(LPCTSTR lpszLine, int nImage = -1, int nIndent = 0);

	/** Enables or disables the redrawing of this view.

		When disabled, the view will not redraw.
		If enabled, it will redraw and scroll to the last line
		(if the previously last line was selected).
	*/
	virtual void EnableUpdate(const bool bEnable);

	/**
	Displays the specified image at the beginning of the specified
	line.

	@param nLine
	        Line to change the image for.
	@param nImage
	        Index of the image in the views image list, to display at the
	        beginning of the line.
	 */
	BOOL SetLineImage(int nLine, int nImage);

	/**
	Selects the specified line in the view an ensures, that it is visible.

	@param nLine Line to select.
	@param bPlaceAtTop
	        If set to TRUE, the contents will be scrolled so, that the
	        specified line will be at the top of the list, otherwise the
	        method will only ensure that the line will be visible anywhere
	        in the window.
	 */
	void SelectLine(int nLine, BOOL bPlaceAtTop = FALSE);

	/** A helper function, simply calls the protected OnUpdate member to do the work */
	void DoOnUpdate(COutputView* pSender, LPARAM lHint, CObject* pHint);

	//using CCrystalTextView::GetLineCount;
	int GetLineCount() const;

	/**
	Sets the image list to use

	@return
	        Image list used previously
	 */
	CImageList *SetImageList(CImageList *pImageList);

	/** Returns the image list, currently in use */
	CImageList *GetImageList() const;

// implementation helpers
protected:
	/** Applies the font settings to the window */
	void ApplyFont();

	/**
	Modifies the given string, so that it can be displayed, as expected
	and returns the result.
	 */
	static const CString EnsureStringReadable(LPCTSTR lpszString);

// overridables
protected:
	virtual void OnUpdate(COutputView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// debugging
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generierte Nachrichtenzuordnungsfunktionen
protected:
	LRESULT OnAddLine(WPARAM wParam, LPARAM lParam);
	LRESULT OnReset(WPARAM wParam, LPARAM lParam);
	LRESULT OnAddInfoLine(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysColorChange();
	void OnSetFocus(CWnd* pOldWnd);
	//{{AFX_MSG(COutputView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// attributes
protected:
	/** The views document */
	COutputDoc* m_pOutputDoc;

private:
	/** TRUE if the view has been attached to a document */
	BOOL m_bInitialized;

	/** Font to be used for the view */
	CFont m_font;

	///Index of the last line before we disabled update using EnableUpdate().
	int idxLastLineOnLastUpdateDisable;

protected:
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnKillFocus(CWnd* pNewWnd);
};
