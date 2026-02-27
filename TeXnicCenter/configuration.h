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

#if !defined(AFX_CONFIGURATION_H__E7A12DE1_107E_11D3_929E_8FAE545EAC4D__INCLUDED_)
#define AFX_CONFIGURATION_H__E7A12DE1_107E_11D3_929E_8FAE545EAC4D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <memory>

#include "FileClean.h"
#include "TextModules.h"
#include "LaTeXView.h"

class CConfiguration
{
	// Singleton
	static std::auto_ptr<CConfiguration> impl_;

	CConfiguration();

	/** TRUE, if line endings shout be displayed as special chars, FALSE otherwise */
	bool show_line_endings_;

	/** TRUE, if word wrap is enabled, FALSE otherwise */
	bool word_wrap_;

	/** TRUE, if wrapped lines are indented, FALSE otherwise */
	bool word_wrap_indent_;

	/** 0 for no indicators, 1 for indicators at the end, 2 for indicators at the beginning, and 3 for both */
	int word_wrap_indicators_;

	///Wether or not folding is enabled.
	bool fold_;

	bool show_line_below_fold_;
	bool show_line_below_no_fold_;
	bool show_line_above_fold_;	
	bool show_line_above_no_fold_;
	bool use_spaces_;
	bool show_indentation_guides_;

	int transparency_;

	bool fold_compact_;

public:
	bool GetShowLineEnding() const;
	void SetShowLineEnding(bool val = true);

	bool IsWordWrapEnabled() const;
	void EnableWordWrap(bool val = true);

	bool IsWordWrapIndentEnabled() const;
	void EnableWordWrapIndent(bool val = true);

	int GetWordWrapIndicators() const;
	void SetWordWrapIndicators( int val = 0 );

	bool IsFoldingEnabled() const;
	void EnableFolding(bool val = true);

	bool GetShowLineBelowFold() const;
	void SetShowLineBelowFold(bool show = true);
	bool GetShowLineAboveNoFold() const;
	void SetShowLineAboveNoFold(bool val);
	bool GetShowLineAboveFold() const;
	void SetShowLineAboveFold(bool val);
	bool GetShowLineBelowNoFold() const;
	void SetShowLineBelowNoFold(bool val);

	bool GetUseSpaces() const;
	void SetUseSpaces(bool val);

	int GetTransparency() const;
	void SetTransparency(int val);

	bool GetShowIndentationGuides() const;
	void SetShowIndentationGuides(bool val);

	bool GetFoldCompact() const;
	void SetFoldCompact(bool val);

	static CConfiguration* GetInstance();

	/** Symbols for the second parameter of Serialize() */
	enum SERDIRECTION
	{
		Load, Save
	};

	/**
	Stores the configuration to the registry or reads it from the registry.

	@direction
	        Specifies, if the function should store the configuration in the registry
	        (CConfiguration::Save) or load it from the registry (CConfiguration::Load)
	 */
	void Serialize(SERDIRECTION direction);

// operations
protected:
	/**
	Writes an integer to or reads an integer from the registry.

	@param szSection
	                Section in the registry, that contains the integer.
	@param szEntry
	                Key of the integer.
	@param pnValue
	                Pointer to a variable that should receive the read integer or that
	                contains the integer to write
	@param bStoring
	                CConfiguration::Save to store pnValue or CConfiguration::Load
	                to read an integer to pnValue.
	@param szDefault
	                If you try to read an integer and the specified key does not exist
	                in the registry, then this default value will be used.
	 */
	void SerializeProfileInt(LPCTSTR szSection, LPCTSTR szEntry,
	                         int* pnValue, SERDIRECTION direction, int nDefault = 0);

	/**
	Writes a string to or reads a string from the registry.

	@param szSection
	                Section in the registry, that contains the string.
	@param szEntry
	                Key of the string
	@param pstrString
	                Pointer to a variable that should receive the read string or that
	                contains the string to write
	@param bStoring
	                CConfiguration::Save to store pstrString or CConfiguration::Load
	                to read a string to pstrString.
	@param szDefault
	                If you try to read a string and the specified key does not exist
	                in the registry, then this default value will be used.
	 */
	void SerializeProfileString(LPCTSTR szSection, LPCTSTR szEntry,
	                            CString *pstrString, SERDIRECTION direction, LPCTSTR szDefault = _T(""));

	void SerializeProfileBool(LPCTSTR szSection, LPCTSTR szEntry, bool& value, SERDIRECTION direction, bool defaultvalue = false);

	/**
	Writes binary data to or read binary data from the registry.

	@param szSection
	                Section in the registry, that contains the data.
	@param szEntry
	                Key of the data.
	@param pbyData
	                Pointer to a buffer that should receive the read data or that
	                contains the data to write.
	@param bStoring
	                CConfiguration::Save to store pbyData or CConfiguration::Load
	                to read data to pbyData.
	@param nDataLen
	                Number of bytes pointed to by pbyData. The method returns FALSE,
	                if data is read and the read data has another size than nDataLen.
	@param szDefault
	                If you try to read data and the specified key does not exist
	                in the registry, then this default value will be used.
	 */
	void SerializeProfileData(LPCTSTR szSection, LPCTSTR szEntry,
	                          LPVOID lpData, SERDIRECTION direction, UINT unDataLen, LPVOID lpDefaulData = NULL);

	/**
	Writes a windows placement to or read it from the registry.

	<b>Remarks:</b> If direction is CConfiguration::Load and the specified section
	does not contain the data, then FALSE will be returned and lpWndPlc->length will be 0.

	@param szSection
	        Section in the registry, that contains the data.
	@param lpWndPlc
	        Pointer to structure to write from or to read to.
	@param direction
	        CConfiguration::Save to store the data, CConfiguration::Load to read the data.

	@return
	        TRUE if successfull, FALSE otherwise.
	 */
	BOOL SerializeProfileWndPlacement(LPCTSTR szSection, WINDOWPLACEMENT* lpWndPlc,
	                                  SERDIRECTION direction);

	/**
	Writes a string array to or reads a string array from the registry.

	@param szSection
	                Section in the registry, that contains the string array .
	@param szEntry
	                Key of the string array
	@param pastrArray
	                Pointer to a variable that should receive the read string array or that
	                contains the string array to write.
	@param bStoring
	                CConfiguration::Save to store pastrArray or CConfiguration::Load
	                to read a string to pastrArray.

	@return
	        TRUE if successfull, FALSE if the array has not been read.
	 */
	BOOL SerializeProfileStringArray(LPCTSTR szSection, LPCTSTR szEntry,
	                                 CStringArray *pastrArray, SERDIRECTION direction);

	/**
	Deletes the specified key from the registry.

	@param lpszSection
	        Path of the key to remove from the registry.
	@param bAdmin
	        TRUE, if lpszPath should be relative to HKEY_LOCAL_MACHINE,
	        FALSE if lpszPath should be relative to HKEY_CURRENT_USER.

	@return
	        TRUE if successful, FALSE otherwise.
	 */
	BOOL RegDeleteSection(LPCTSTR lpszSection, BOOL bAdmin = FALSE);

	/* Attributes */
public:
	/** TRUE, if document-windows should be opened maximized */
	bool m_bOpenDocWndMaximized;

	/** Paths to search for project templates in. */
	CStringArray m_astrProjectTemplatePaths;

	/** Paths to search for document templates in. */
	CStringArray m_astrDocumentTemplatePaths;

	/** Maximal number of entries (editor windows) in the window menu. */
	int m_nWndMenuMaxEntries;

	///////////////////////////////////////////////////////////////////
	// File	Handling

	/** TRUE, if TeXnicCenter should always save newly created files. */
	bool m_bSaveNewDocuments;

	/** TRUE, if TeXnicCenter should save all modified tex-files, before compilation.*/
	bool m_bSaveBeforeCompilation;

	/** TRUE, if TeXnicCenter should save all open files in given intervals. */
	bool m_bSaveAutomatic;

	/** The interval, TeXnicCenter should save all open files in. @see m_bSaveAutomatic. */
	UINT m_unSaveInterval;

	/** Standard file format used to create new documents or to save documents. */
	int m_nStandardFileFormat;

	/** Default path. */
	CString m_strDefaultPath;

	/** Last opened Folder for Load- and Save-Dialogs. */
	CString m_strLastOpenedFolder;

	///////////////////////////////////////////////////////////////////
	// File	Handling - Template dialog params

	/** Last opened tab of the project template dialog */
	int m_nLastTabProjectTemplateDlg;

	/** Last opened tab of the document template dialog */
	int m_nLastTabDocumentTemplateDlg;

	///////////////////////////////////////////////////////////////////
	// Dialog CTextOutsourceDlg

	///Last selected type of inclusion
	int m_TextOutsource_nIncludeType;

	///Left part of the user inclusion command
	CString m_TextOutsource_strUserCmdLeft;

	///Right part of the user inclusion command
	CString m_TextOutsource_strUserCmdRight;

	///Whether to open the new file
	bool m_TextOutsource_bOpenNewFile;

	///Last selected type of opening the new file
	int m_TextOutsource_nOpenNewFileType;

	///////////////////////////////////////////////////////////////////
	// character replacement

	/** TRUE, if the Editor should replace opening and closing quotation marks while typing.*/
	bool m_bReplaceQuotationMarks;

	/** Text, the Editor should replace an opening quotation mark with. @see m_bReplaceQuotationMarks.*/
	CString m_strOpeningQuotationMark;

	/** Text, the Editor should replace an closing quotation mark with. @see m_bReplaceQuotationMarks.*/
	CString m_strClosingQuotationMark;


	/** Interval in milliseconds to parse the project. */
	int m_nParseInterval;

	///////////////////////////////////////////////////////////////////
	// Project specific

	/** TRUE, if we should load the last project at startup. */
	bool m_bLoadLastProject;

	/** Path of the latest project. */
	CString m_strLastProject;

	///////////////////////////////////////////////////////////////////
	// Editor settings

	/** The font, used in the editor */
	LOGFONT m_fontEditor;

	/** Colors for editor components */
	COLORREF m_aEditorColors[LaTeXView::COLORINDEX_COUNT];

	/** Color scheme for editor components */
	int m_nEditorColorScheme;

	/** Width of a tab in characters */
	int m_nTabWidth;

	/** TRUE, if white spaces shout be displayed as special chars, FALSE otherwise */
	bool m_bViewWhitespaces;

	/** TRUE, if line numbers should be shown */
	bool m_bShowLineNumbers;

	/** Column for hard wrap via Edit | Advanced | Split Paragraphs */
	int m_nFixedColumnWrap;

	/** Mode for vertical edge */
	int m_nVerticalEdgeMode;

	/** Column for vertical edge */
	int m_nVerticalEdgeColumn;

	/** Color for vertical edge */
	COLORREF m_aVariableEdgeColor;

	/** Close ToolWindows on pressing ESC when the editor has the focus. */
	bool m_bCloseToolWindowsOnEscape;

	/** Type of the caret. 0 invisible, 1 line, 2 block. See also Scintilla documentation. */
	int m_nInsertCaretStyle;

	/** Rate of caret blinking in milliseconds. 0 disables blinking. See also Scintilla documentation. */
	int m_nInsertCaretBlinkPeriod;

	/** TRUE, if caret line is highlighted, FALSE otherwise. */
	bool m_bHighlightCaretLine;

	///////////////////////////////////////////////////////////////////
	// Other window settings

	/** The font to use for the navigator */
	LOGFONT m_fontNavigator;

	/** The font to use for the output view */
	LOGFONT m_fontOutput;

	///////////////////////////////////////////////////////////////////
	// find in files settings

	/** file types, the last time was searched for. */
	CString m_strFileFindFileTypes;

	/** TRUE if whole words where searched last time. */
	bool m_bFileFindWholeWords;

	/** TRUE if last search was case sensitive. */
	bool m_bFileFindCaseSensitive;

	/** TRUE if last search was a regular expression. */
	bool m_bFileFindRegularExpression;

	/** TRUE if last time sub folders where searched. */
	bool m_bFileFindSubFolders;

	/** Index of output buffer, where last time the output was written to */
	int m_nFileFindOutput;

	///////////////////////////////////////////////////////////////////
	// Skin initialization

	///////////////////////////////////////////////////////////////////
	// Language and spelling

	/** User interface language currently in use. */
	CString m_strGuiLanguage;

	/** User interface language activated on next start. */
	CString m_strGuiLanguageOnNextStart;

	/** User interface language, that was active in the last session. */
	CString m_strGuiLanguageOnLastSession;

	/** Language of the dictionary. */
	CString m_strLanguageDefault;

	/** Dialect of the dictionary. */
	CString m_strLanguageDialectDefault;

	/** Directory containing dictionaries. */
	CString m_strSpellDictionaryPath;

	/** Path of personal dictionary. */
	CString m_strSpellPersonalDictionary;

	/** Skip comments. */
	bool m_bSpellSkipComments;

	/** Skip words with numbers. */
	bool m_bSpellSkipNumbers;

	/** Skip LaTeX tags. */
	bool m_bSpellSkipTags;

	/** Skip all uppercase words .*/
	bool m_bSpellSkipCaps;

	/** Suggest from main dictionary only. */
	bool m_bSpellMainDictOnly;

	/** Enable spell check. */
	bool m_bSpellEnable;

	///////////////////////////////////////////////////////////////////
	// TextModules

	/** Group that holds the user defined TextModules. */
	CTextModuleGroup m_aTextModules;

	///////////////////////////////////////////////////////////////////
	// File Cleaning

	/** Array to hold the patterns for file cleaning. */
	CFileCleanItemArray m_aFileCleanItems;

	bool m_bFileCleanConfirm;

	///////////////////////////////////////////////////////////////////
	// Accessibility

	/** TRUE to optimize the GUI for visually handicapped users. */
	bool m_bOptimizeMenuForVisuallyHandicappedUsers;

	/**
	TRUE to optimize the GUI for visually handicapped users in the
	next session.
	 */
	bool m_bOptimizeMenuForVisuallyHandicappedUsersOnNextStart;

	///////////////////////////////////////////////////////////////////
	// Preview

	/** TRUE for running preview in fast mode. */
	bool m_bPreviewFastMode{true};

	/** TRUE for running preview when pressing Enter in edit window. */
	bool m_bPreviewAutoBuildOnEnter{false};

	/** TRUE for running preview when saving any file. */
	bool m_bPreviewAutoBuildOnSave{false};

	/** Name of the used preview template. */
	CString m_strPreviewTemplate;

	/** Identifies the DPI setting in the preview image pane. */
	int m_nPreviewDPISetting{0};

	/** Actual DPI value used for preview generation. */
	int m_nPreviewDPIValue{300};
};

// declaration of one and only global configuration object
//extern CConfiguration g_configuration;

#endif // !defined(AFX_CONFIGURATION_H__E7A12DE1_107E_11D3_929E_8FAE545EAC4D__INCLUDED_)
