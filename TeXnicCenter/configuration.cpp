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

#include "stdafx.h"
#include "resource.h"
#include "Configuration.h"
#include "EndOfLineMode.h"
#include "FontOccManager.h"
#include "global.h"
#include "LaTeXDocument.h"
#include "LaTeXView.h"
#include "TeXnicCenter.h"

std::auto_ptr<CConfiguration> CConfiguration::impl_;

CConfiguration::CConfiguration()
: m_nStandardFileFormat(DOSStyleEOLMode)
, show_line_endings_(false)
, word_wrap_(true)
, word_wrap_indent_(false)
, word_wrap_indicators_(0)
, fold_(false)
, show_line_below_fold_(false)
, show_line_below_no_fold_(false)
, show_line_above_fold_(false)
, show_line_above_no_fold_(false)
, use_spaces_(false)
, transparency_(0)
, show_indentation_guides_(false)
, fold_compact_(true)
{
}

#pragma region Serialization

void CConfiguration::Serialize(SERDIRECTION direction)
{
	CString strSection;

#pragma region Options

	strSection = _T("Settings\\Options");
	SerializeProfileStringArray(strSection,_T("ProjectTemplatePaths"),&m_astrProjectTemplatePaths,direction);
	SerializeProfileStringArray(strSection,_T("DocumentTemplatePaths"),&m_astrDocumentTemplatePaths,direction);
	SerializeProfileBool(strSection,_T("SaveBeforeCompilation"),m_bSaveBeforeCompilation,direction,true);
	SerializeProfileBool(strSection,_T("SaveNewDocuments"),m_bSaveNewDocuments,direction,false);

	SerializeProfileInt(strSection,_T("WndMenuMaxEntries"),&m_nWndMenuMaxEntries,direction,10);
	SerializeProfileInt(strSection,_T("ParseInterval"),&m_nParseInterval,direction,1000);
	SerializeProfileInt(strSection,_T("Transparency"),&transparency_,direction,0);

#pragma endregion

#pragma region File

	SerializeProfileBool(strSection,_T("SaveAutomatic"),m_bSaveAutomatic,direction,true);
	SerializeProfileInt(strSection,_T("SaveInterval"),(int*) & m_unSaveInterval,direction,10);
	SerializeProfileInt(strSection,_T("StandardFileFormat"),&m_nStandardFileFormat,direction,DOSStyleEOLMode);
	SerializeProfileString(strSection,_T("DefaultPath"),&m_strDefaultPath,direction);
	SerializeProfileBool(strSection,_T("OpenDocWndMaximized"),m_bOpenDocWndMaximized,direction,false);
	SerializeProfileString(strSection,_T("LastOpenedFolder"),&m_strLastOpenedFolder,direction);
	SerializeProfileInt(strSection,_T("LastTabProjectTemplateDlg"),(int*) & m_nLastTabProjectTemplateDlg,direction,0);
	SerializeProfileInt(strSection,_T("LastTabDocumentTemplateDlg"),(int*) & m_nLastTabDocumentTemplateDlg,direction,0);

#pragma endregion

#pragma region Text outsource

	SerializeProfileInt(strSection,_T("TextOutsourceIncludeType"),&m_TextOutsource_nIncludeType,direction,0);
	SerializeProfileString(strSection,_T("TextOutsourceCmdLeft"),&m_TextOutsource_strUserCmdLeft,direction);
	SerializeProfileString(strSection,_T("TextOutsourceCmdRight"),&m_TextOutsource_strUserCmdRight,direction);
	SerializeProfileBool(strSection,_T("TextOutsourceOpenNewFile"),m_TextOutsource_bOpenNewFile,direction,true);
	SerializeProfileInt(strSection,_T("TextOutsourceOpenNewFileType"),&m_TextOutsource_nOpenNewFileType,direction,0);

#pragma endregion

#pragma region Quotation mark replacement

	SerializeProfileBool(strSection,_T("ReplaceQuotationMarks"),m_bReplaceQuotationMarks,direction,true);
	SerializeProfileString(strSection,_T("OpeningQuotationMark"),&m_strOpeningQuotationMark,direction,_T("\"`"));
	SerializeProfileString(strSection,_T("ClosingQuotationMark"),&m_strClosingQuotationMark,direction,_T("\"'"));

#pragma endregion

#pragma region Session

	strSection = _T("Session");
	SerializeProfileBool(strSection,_T("LoadLastProject"),m_bLoadLastProject,direction,true);
	SerializeProfileString(strSection,_T("LastProject"),&m_strLastProject,direction);

#pragma endregion

#pragma region Editor settings

	strSection = _T("Settings\\Editor");

	if (direction == Load)
	{
		CFont font;
		LOGFONT logFont;

		// Create the default editor font if the loading fails
		LPCTSTR const fontNames[] = {
			_T("Consolas"),			// Preferred monospaced font on Vista or higher
			_T("Lucida Console"),
			_T("Courier New")
		};

		for (int i = 0; i < sizeof(fontNames) / sizeof(*fontNames) &&
			!font.CreatePointFont(120, fontNames[i]); )
			 ++i; // Increment here to prevent compiler warning

		font.GetLogFont(&logFont);

		SerializeProfileData(strSection,_T("EditorFont"),&m_fontEditor,direction,sizeof(logFont),&logFont);
		SerializeProfileData(_T("Settings"),_T("OutputFont"),&m_fontOutput,direction,sizeof(logFont),&logFont);

		font.DeleteObject();

		logFont = GetDisplayFont();

		SerializeProfileData(_T("Settings"),_T("NavigatorFont"),&m_fontNavigator,direction,sizeof(logFont),&logFont);
		font.DeleteObject();
	}
	else
	{
		SerializeProfileData(strSection,_T("EditorFont"),&m_fontEditor,direction,sizeof(m_fontEditor));
		SerializeProfileData(_T("Settings"),_T("OutputFont"),&m_fontOutput,direction,sizeof(m_fontOutput));
		SerializeProfileData(_T("Settings"),_T("NavigatorFont"),&m_fontNavigator,direction,sizeof(m_fontNavigator));
	}

	SerializeProfileBool(strSection,_T("EditorShowLineNumbers"),m_bShowLineNumbers,direction,0);
	SerializeProfileBool(strSection,_T("EditorShowLineEndings"),show_line_endings_,direction,0);
	SerializeProfileBool(strSection,_T("ViewWhitespaces"),m_bViewWhitespaces,direction,false);
	SerializeProfileBool(strSection,_T("EditorWordWrap"),word_wrap_,direction,1);
	SerializeProfileBool(strSection,_T("EditorWordWrapIndent"),word_wrap_indent_,direction,0);
	SerializeProfileInt(strSection,_T("EditorWordWrapIndicators"),&word_wrap_indicators_,direction,0);
	SerializeProfileBool(strSection,_T("EditorShowIndentationGuides"),show_indentation_guides_,direction, show_indentation_guides_);
	SerializeProfileInt(strSection,_T("FixedColumnWrap"),&m_nFixedColumnWrap,direction,80);
	SerializeProfileInt(strSection,_T("VerticalEdgeMode"),&m_nVerticalEdgeMode,direction,0);
	SerializeProfileInt(strSection,_T("VerticalEdgeColumn"),&m_nVerticalEdgeColumn,direction,80);
	SerializeProfileInt(strSection,_T("VerticalEdgeColor"),(int*) &m_aVariableEdgeColor,direction,RGB(192,192,192));

	SerializeProfileBool(strSection,_T("EditorFold"),fold_,direction,0);
	SerializeProfileBool(strSection,_T("EditorShowLineBelowFold"),show_line_below_fold_,direction,0);
	SerializeProfileBool(strSection,_T("EditorShowLineBelowNoFold"),show_line_below_no_fold_,direction,0);
	SerializeProfileBool(strSection,_T("EditorShowLineAboveFold"),show_line_above_fold_,direction,0);
	SerializeProfileBool(strSection,_T("EditorShowLineAboveNoFold"),show_line_above_no_fold_,direction,0);
	SerializeProfileBool(strSection,_T("EditorFoldCompact"),fold_compact_,direction, fold_compact_);

	SerializeProfileInt(strSection,_T("EditorTabWidth"),&m_nTabWidth,direction,2);
	SerializeProfileBool(strSection,_T("EditorUseSpaces"),use_spaces_,direction,0);

	SerializeProfileBool(strSection,_T("CloseToolWindowsOnEscape"), m_bCloseToolWindowsOnEscape, direction, 1);

	for (int i = 0; i < LaTeXView::COLORINDEX_COUNT; i++)
	{
		CString strFormat;
		strFormat.Format(_T("EditorColor%d"),i);
		SerializeProfileInt(strSection,strFormat,(int*) &m_aEditorColors[i],direction,(int) 0xFFFFFFFF);
	}

	SerializeProfileInt(strSection,_T("EditorColorScheme"), &m_nEditorColorScheme, direction, 0);

#pragma region Cursor settings

	SerializeProfileInt(strSection,_T("InsertCaretStyle"), &m_nInsertCaretStyle, direction, 1);
	SerializeProfileInt(strSection,_T("InsertCaretBlinkPeriod"), &m_nInsertCaretBlinkPeriod, direction, 500);
	SerializeProfileBool(strSection,_T("HighlightCaretLine"), m_bHighlightCaretLine, direction, 0);

#pragma endregion

#pragma endregion

#pragma region Find in files settings

	strSection = _T("Settings\\FindInFiles");

	SerializeProfileString(strSection,_T("FileTypes"),&m_strFileFindFileTypes,direction);
	SerializeProfileBool(strSection,_T("FindWholeWords"),m_bFileFindWholeWords,direction,false);
	SerializeProfileBool(strSection,_T("CaseSensitive"),m_bFileFindCaseSensitive,direction,false);
	SerializeProfileBool(strSection,_T("FindRegularExpression"),m_bFileFindRegularExpression,direction,false);
	SerializeProfileBool(strSection,_T("IncludeSubFolders"),m_bFileFindSubFolders,direction,true);
	SerializeProfileInt(strSection,_T("OutputBuffer"),&m_nFileFindOutput,direction,0);

#pragma endregion

#pragma region Language
	strSection = _T("Settings\\Language");

	// help german users from older version to upgrade -- so set german
	// language as default on german systems.
	SerializeProfileString(strSection,_T("GuiLanguage"),&m_strGuiLanguageOnNextStart,direction,
	                       ((GetUserDefaultLangID()&0xFF) == 0x07) ? _T("Deutsch") : _T("English"));

	if (direction == Load)
	{
		m_strGuiLanguage = m_strGuiLanguageOnNextStart;
		SerializeProfileString(strSection,_T("GuiLanguageOnLastSession"),
		                       &m_strGuiLanguageOnLastSession,direction,m_strGuiLanguage);
	}
	else
		SerializeProfileString(strSection,_T("GuiLanguageOnLastSession"),
		                       &m_strGuiLanguage,direction);

#pragma endregion

#pragma region Spelling
	strSection = _T("Settings\\Spelling");

	// Create a default name for the user dictionary (otherwise the user will always loose the added words, if a path is not specified in the options)
	TCHAR PersonalFolderPath[MAX_PATH];
	SHGetSpecialFolderPath(NULL, PersonalFolderPath, CSIDL_PERSONAL, false);

	CPathTool DefaultUserDic(PersonalFolderPath);
	DefaultUserDic.Append(_T("TXCUserDictionary.dic"));

	SerializeProfileString(strSection, _T("PersonalDictionary"),
	                       &m_strSpellPersonalDictionary, direction, DefaultUserDic);

	if (m_strSpellPersonalDictionary.IsEmpty())
		m_strSpellPersonalDictionary = DefaultUserDic;

	//Always take the dictionaries from the subfolder Dictionaries.
	// - If we want more, than we need to add a proper user interface.
	m_strSpellDictionaryPath = CPathTool::Cat(CPathTool::GetDirectory(theApp.GetModuleFileName()), _T("Dictionaries"));

	SerializeProfileString(strSection,_T("Language"),
	                       &m_strLanguageDefault,direction,AfxLoadString(IDS_LANGUAGE));
	SerializeProfileString(strSection,_T("Dialect"),
	                       &m_strLanguageDialectDefault,direction,AfxLoadString(IDS_DIALECT));
	SerializeProfileBool(strSection,_T("SkipComments"),m_bSpellSkipComments,direction,true);
	SerializeProfileBool(strSection,_T("SkipNumbers"),m_bSpellSkipNumbers,direction,true);
	SerializeProfileBool(strSection,_T("SkipTags"),m_bSpellSkipTags,direction,true);
	SerializeProfileBool(strSection,_T("SkipUppercase"),m_bSpellSkipCaps,direction,true);
	SerializeProfileBool(strSection,_T("MainDictionaryOnly"),m_bSpellMainDictOnly,direction,true);
	SerializeProfileBool(strSection,_T("Enable"),m_bSpellEnable,direction,true);

#pragma endregion

#pragma region TextModules

	strSection = _T("Settings\\TextModules");

	//Serialize the currently one and only Group of TextModules
	if (direction == Load)
	{
		m_aTextModules.SerializeFromRegistry(CPathTool::Cat(theApp.m_strRegistryRoot,strSection));
	}
	else
	{
		m_aTextModules.SerializeToRegistry(CPathTool::Cat(theApp.m_strRegistryRoot,strSection));
	}

#pragma endregion

#pragma region File Cleaning
	strSection = "Settings\\FileClean";

	SerializeProfileBool(strSection,_T("Confirm"),m_bFileCleanConfirm,direction,1);
	//Serialize the ItemArray
	if (direction == Load)
	{
		m_aFileCleanItems.SerializeFromRegistry(CPathTool::Cat(theApp.m_strRegistryRoot,strSection));
	}
	else
	{
		m_aFileCleanItems.SerializeToRegistry(CPathTool::Cat(theApp.m_strRegistryRoot,strSection));
	}

#pragma endregion

#pragma region Accessibility
	strSection = "Settings\\Accessibility";

	SerializeProfileBool(strSection,_T("OptimizeMenuForVisuallyHandicappedUsers"),
	                    m_bOptimizeMenuForVisuallyHandicappedUsersOnNextStart,direction,false);

	if (direction == Load)
		m_bOptimizeMenuForVisuallyHandicappedUsers = m_bOptimizeMenuForVisuallyHandicappedUsersOnNextStart;

#pragma endregion

#pragma region Preview
	strSection = "Settings\\Preview";

	SerializeProfileBool(strSection,_T("FastMode"), m_bPreviewFastMode, direction, true);
	SerializeProfileBool(strSection,_T("AutoBuildOnEnter"), m_bPreviewAutoBuildOnEnter, direction, true);
	SerializeProfileBool(strSection,_T("AutoBuildOnSave"), m_bPreviewAutoBuildOnSave, direction, true);
	SerializeProfileString(strSection,_T("PreviewTemplate"), &m_strPreviewTemplate, direction);
	SerializeProfileInt(strSection,_T("DPISetting"), &m_nPreviewDPISetting, direction, 0);
	SerializeProfileInt(strSection,_T("DPIValue"), &m_nPreviewDPIValue, direction, 300);

#pragma endregion

}

#pragma endregion

void CConfiguration::SerializeProfileInt(LPCTSTR szSection,LPCTSTR szEntry,
        int* pnValue,SERDIRECTION direction,int nDefault)
{
	if (direction == Save)
		AfxGetApp()->WriteProfileInt(szSection,szEntry,*pnValue);
	else
		*pnValue = (int) AfxGetApp()->GetProfileInt(szSection,szEntry,nDefault);
}

void CConfiguration::SerializeProfileString(LPCTSTR szSection,LPCTSTR szEntry,
        CString *pstrString,SERDIRECTION direction,LPCTSTR szDefault)
{
	if (direction == Save)
		AfxGetApp()->WriteProfileString(szSection,szEntry,*pstrString);
	else
		*pstrString = AfxGetApp()->GetProfileString(szSection,szEntry,szDefault);
}

void CConfiguration::SerializeProfileData(LPCTSTR szSection,LPCTSTR szEntry,
        LPVOID lpData,SERDIRECTION direction,UINT unDataLen,LPVOID lpDefaultData /*= NULL*/)
{
	CString strSection = theApp.m_strRegistryRoot + _T("\\") + szSection;

	if (direction == Save)
	{
		HKEY hKey;
		DWORD dwDisposition;
		if (
		    RegCreateKeyEx(HKEY_CURRENT_USER,strSection,0,_T("Binary"),0,KEY_WRITE,
		                   NULL,&hKey,&dwDisposition) != ERROR_SUCCESS)
			return;

		RegSetValueEx(hKey,szEntry,0,REG_BINARY,(BYTE*) lpData,unDataLen);
		RegCloseKey(hKey);
	}
	else
	{
		HKEY hKey;
		if (
		    RegOpenKeyEx(HKEY_CURRENT_USER,strSection,0,KEY_READ,&hKey) != ERROR_SUCCESS)
		{
			memcpy(lpData,lpDefaultData,unDataLen);
			return;
		}

		DWORD dwType;

		if (RegQueryValueEx(hKey,szEntry,NULL,&dwType,(BYTE*) lpData,(LPDWORD) & unDataLen) != ERROR_SUCCESS)
		{
			memcpy(lpData,lpDefaultData,unDataLen);
			return;
		}
	}
}

BOOL CConfiguration::SerializeProfileWndPlacement(LPCTSTR szSection,
        WINDOWPLACEMENT* lpWndPlc,SERDIRECTION direction)
{
	if (direction == Save)
	{
		ASSERT(lpWndPlc->length);

		bool bIconified = lpWndPlc->showCmd == SW_SHOWMINIMIZED;
		bool bMaximized = lpWndPlc->showCmd == SW_SHOWMAXIMIZED;

		SerializeProfileData(szSection,_T("Place"),lpWndPlc,Save,sizeof(WINDOWPLACEMENT));
		SerializeProfileBool(szSection,_T("Iconified"),bIconified,Save);
		SerializeProfileBool(szSection,_T("Maximized"),bMaximized,Save);

		return true;
	}
	else
	{
		bool bIconified;
		bool bMaximized;

		lpWndPlc->length = 0; // mark invalid
		SerializeProfileData(szSection,_T("Place"),lpWndPlc,Load,sizeof(WINDOWPLACEMENT),lpWndPlc);
		if (!lpWndPlc->length)
			return false;

		SerializeProfileBool(szSection,_T("Iconified"),bIconified,Load);
		SerializeProfileBool(szSection,_T("Maximized"),bMaximized,Load);

		if (bIconified)
		{
			lpWndPlc->showCmd = SW_SHOWMINNOACTIVE;
			if (bMaximized)
				lpWndPlc->flags = WPF_RESTORETOMAXIMIZED;
			else
				lpWndPlc->flags = WPF_SETMINPOSITION;
		}
		else if (bMaximized)
		{
			lpWndPlc->showCmd = SW_SHOWMAXIMIZED;
			lpWndPlc->flags = WPF_RESTORETOMAXIMIZED;
		}
		else
		{
			lpWndPlc->showCmd = SW_NORMAL;
			lpWndPlc->flags = WPF_SETMINPOSITION;
		}

		return true;
	}
}

BOOL CConfiguration::SerializeProfileStringArray(LPCTSTR szSection,LPCTSTR szEntry,
        CStringArray *pastrArray,SERDIRECTION direction)
{
	ASSERT(pastrArray);

	if (direction == Save)
	{
		// store array
		int i;
		int nSize = pastrArray->GetSize();
		CString strSection = CString(szSection) + _T("\\") + szEntry;
		LPCTSTR szFormat = _T("String%d");
		CString strKey;
		CString strElement;

		// store number of elements
		SerializeProfileInt(strSection,_T("Size"),&nSize,Save);

		// store each string
		for (i = 0; i < nSize; i++)
		{
			strKey.Format(szFormat,i);
			strElement = pastrArray->GetAt(i);
			SerializeProfileString(strSection,strKey,&strElement,Save);
		}
	}
	else
	{
		// load array
		int i;
		int nSize;
		CString strSection = CString(szSection) + _T("\\") + szEntry;
		LPCTSTR szFormat = _T("String%d");
		CString strKey;
		CString strElement;

		// load number of elements
		SerializeProfileInt(strSection,_T("Size"),&nSize,Load);
		if (!nSize)
			return false;

		// load each string
		for (i = 0; i < nSize; i++)
		{
			strKey.Format(szFormat,i);
			SerializeProfileString(strSection,strKey,&strElement,Load);
			pastrArray->Add(strElement);
		}
	}

	return true;
}

BOOL CConfiguration::RegDeleteSection(LPCTSTR lpszSection,BOOL bAdmin /*= false*/)
{
	ASSERT(lpszSection);

	CString strSection = theApp.m_strRegistryRoot + _T("\\") + lpszSection;

	LONG lReturn = ::RegDeleteKey(bAdmin ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
	                              strSection);

	if (lReturn == ERROR_SUCCESS)
		return true;

	return false;
}

void CConfiguration::SerializeProfileBool( LPCTSTR szSection, LPCTSTR szEntry, bool& value,
									  SERDIRECTION direction, bool defaultvalue /*= false*/ )
{
	int temp = value != 0;

	SerializeProfileInt(szSection,szEntry,&temp,direction,defaultvalue);

	if (direction == Load)
		value = temp != 0;
}

CConfiguration* CConfiguration::GetInstance()
{
	if (!impl_.get())
		impl_.reset(new CConfiguration);

	return impl_.get();
}

bool CConfiguration::GetShowLineEnding() const
{
	return show_line_endings_ != 0;
}

void CConfiguration::SetShowLineEnding( bool val /*= true*/ )
{
	show_line_endings_ = val;
}

bool CConfiguration::IsWordWrapEnabled() const
{
	return word_wrap_ != 0;
}

void CConfiguration::EnableWordWrap( bool val /*= true*/ )
{
	word_wrap_ = val;
}

bool CConfiguration::IsWordWrapIndentEnabled() const
{
	return word_wrap_indent_ != 0;
}

void CConfiguration::EnableWordWrapIndent( bool val /*= true*/ )
{
	word_wrap_indent_ = val;
}

int CConfiguration::GetWordWrapIndicators() const
{
	if (word_wrap_indicators_ >= 0 && word_wrap_indicators_ <= 3)
	{
		return word_wrap_indicators_;
	}

	return 0;
}

void CConfiguration::SetWordWrapIndicators( int val /*= 0*/ )
{
	word_wrap_indicators_ = val;
}

bool CConfiguration::IsFoldingEnabled() const
{
	return fold_;
}

void CConfiguration::EnableFolding(bool val /*= true*/)
{
	fold_ = val;
}

bool CConfiguration::GetShowLineBelowFold() const
{
	return show_line_below_fold_ != 0;
}

void CConfiguration::SetShowLineBelowFold( bool show /*= true*/ )
{
	show_line_below_fold_ = show;
}

bool CConfiguration::GetShowLineAboveNoFold() const
{
	return show_line_above_no_fold_;
}

void CConfiguration::SetShowLineAboveNoFold( bool val )
{
	show_line_above_no_fold_ = val;
}

bool CConfiguration::GetShowLineAboveFold() const
{
	return show_line_above_fold_;
}

void CConfiguration::SetShowLineAboveFold( bool val )
{
	show_line_above_fold_ = val;
}

bool CConfiguration::GetShowLineBelowNoFold() const
{
	return show_line_below_no_fold_;
}

void CConfiguration::SetShowLineBelowNoFold( bool val )
{
	show_line_below_no_fold_ = val;
}

bool CConfiguration::GetUseSpaces() const
{
	return use_spaces_ != 0;
}

void CConfiguration::SetUseSpaces( bool val )
{
	use_spaces_ = val;
}

int CConfiguration::GetTransparency() const
{
	return transparency_;
}

void CConfiguration::SetTransparency( int val )
{
	transparency_ = val;
}

bool CConfiguration::GetShowIndentationGuides() const
{
	return show_indentation_guides_;
}

void CConfiguration::SetShowIndentationGuides( bool val )
{
	show_indentation_guides_ = val;
}

bool CConfiguration::GetFoldCompact() const
{
	return fold_compact_;
}

void CConfiguration::SetFoldCompact( bool val )
{
	fold_compact_ = val;
}
