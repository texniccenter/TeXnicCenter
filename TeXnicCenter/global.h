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

// Global Functions
CString AfxLoadString(UINT nID);
CString AfxFormatString1(UINT nID, LPCTSTR lpszText);
CString AfxFormatSystemString(DWORD dwMessageId);

/** Returns the default directory for open/save-dialogs.
        The return value depends on the current setup and opened files/project.

        @param bForceNonEmpty
                Forces the return value to be non-empty

    @param bNewProject
            New Project or not
 */
CString AfxGetDefaultDirectory(bool bForceNonEmpty = true, bool bNewProject = false);

/** Sets the last opened directory for open/save-dialogs.

        @param strLastFolder
                Last opened folder
 */
void AfxSetLastDirectory(const CString& strLastFolder);

typedef CArray<CString, const CString&> StringArray;

/** Parse the current dictionaries installed

        @param aLanguage
                Languages installed

        @param aDialect
                Dialects installed

        Remark: the files should be properly set (no error handling)
 */
void AfxFindDictionaries(StringArray &aLanguage,
                         StringArray &aDialect);


/** The following messages can be sent to different parts of TeXnicCenter.

    Some of these messages are rather local to a window,
    but we collect them here to keep the IDs unique
    throughout the application.
*/
enum AfxUserMessages
{
	 StartPaneAnimationMessageID = WM_USER
	,StopPaneAnimationMessageID
	,CheckForFileChangesMessageID
    ,CStringLineTextMessageID
    /** Show, not toggle, a docking bar. The wParam is the ID of the docking bar to show. */
	,ShowDockingBarID
    ,UpdateToolBarButtonMessageID
    ,ShellUpdateMessageID
	,ID_BG_UPDATE_BUFFER
	,ID_BG_UPDATE_LINE
	,ID_BG_RESET_SPELLER
	,ID_BG_ENABLE_SPELLER
	,ID_BG_INVALIDATE_VIEW
    ,GotoBibliographyLine
    ,OPW_ADD_LINE
    ,OPW_RESET
    ,OPW_ADD_INFOLINE

    /** Triggers a reload of the preview image from disk. */
    ,PreviewImageViewUpdate
    /** Starts the animation of the progress bar for the preview creation. */
    ,PreviewImageViewStartProgressAnimation
    /** Stops the animation of the progress bar for the preview creation. */
    ,PreviewImageViewStopProgressAnimation
};
