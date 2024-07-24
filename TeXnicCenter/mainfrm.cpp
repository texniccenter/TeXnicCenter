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

#include <algorithm>
#include <vector>
#include <functional>

#include "ChildFrm.h"
#include "environmentview.h"
#include "BibView.h"
#include "BookmarkView.h"
#include "FileViewPane.h"
#include "ParseOutputView.h"
#include "OutputDoc.h"
#include "ErrorListPane.h"
#include "StructureView.h"
#include "StructureTreeCtrl.h"
#include "MainFrm.h"
#include "global.h"
#include "BuildView.h"
#include "GrepView.h"
#include "Configuration.h"
#include "Splash.h"
#include "ToolBarsCustomizeDialog.h"
#include "ProfileDialog.h"
#include "LaTeXDocument.h"
#include "TeXnicCenter.h"
#include "TransparencyDlg.h"
#include "UserTool.h"
#include "WorkspacePane.h"
#include "PreviewImageView.h"

// To hold the colours and their names
struct ColourTableEntry
{
	COLORREF crColour;
	LPCTSTR szName;
};

const int MAX_COLOURS = 100;

static ColourTableEntry crColours[] = 
{
	{ RGB(0x00, 0x00, 0x00),    _T("Black")             },
	{ RGB(0xA5, 0x2A, 0x00),    _T("Brown")             },
	{ RGB(0x00, 0x40, 0x40),    _T("Dark Olive Green")  },
	{ RGB(0x00, 0x55, 0x00),    _T("Dark Green")        },
	{ RGB(0x00, 0x00, 0x5E),    _T("Dark Teal")         },
	{ RGB(0x00, 0x00, 0x8B),    _T("Dark Blue")         },
	{ RGB(0x4B, 0x00, 0x82),    _T("Indigo")            },
	{ RGB(0x28, 0x28, 0x28),    _T("Gray-80%")          },

	{ RGB(0x8B, 0x00, 0x00),    _T("Dark Red")          },
	{ RGB(0xFF, 0x68, 0x20),    _T("Orange")            },
	{ RGB(0x8B, 0x8B, 0x00),    _T("Dark Yellow")       },
	{ RGB(0x00, 0x93, 0x00),    _T("Green")             },
	{ RGB(0x38, 0x8E, 0x8E),    _T("Teal")              },
	{ RGB(0x00, 0x00, 0xFF),    _T("Blue")              },
	{ RGB(0x7B, 0x7B, 0xC0),    _T("Blue-Gray")         },
	{ RGB(0x66, 0x66, 0x66),    _T("Gray-50%")          },

	{ RGB(0xFF, 0x00, 0x00),    _T("Red")               },
	{ RGB(0xFF, 0xAD, 0x5B),    _T("Light Orange")      },
	{ RGB(0x32, 0xCD, 0x32),    _T("Lime")              }, 
	{ RGB(0x3C, 0xB3, 0x71),    _T("Sea Green")         },
	{ RGB(51, 204, 204),		_T("Aqua")              },
	{ RGB(0x7D, 0x9E, 0xC0),    _T("Light Blue")        },
	{ RGB(0x80, 0x00, 0x80),    _T("Violet")            },
	{ RGB(0x7F, 0x7F, 0x7F),    _T("Gray-40%")          },

	{ RGB(0xFF, 0xC0, 0xCB),    _T("Pink")              },
	{ RGB(0xFF, 0xD7, 0x00),    _T("Gold")              },
	{ RGB(0xFF, 0xFF, 0x00),    _T("Yellow")            },    
	{ RGB(0x00, 0xFF, 0x00),    _T("Bright Green")      },
	{ RGB(0x00, 0xFF, 0xFF),    _T("Turquoise")         },
	{ RGB(0, 204, 255),			_T("Sky Blue")          },
	{ RGB(234, 128, 102),		_T("Plum")              },
	{ RGB(0xC0, 0xC0, 0xC0),    _T("Gray-25%")          },

	{ RGB(0xFF, 0xE4, 0xE1),    _T("Rose")              },
	{ RGB(255, 254, 153),		_T("Tan")               },
	{ RGB(0xFF, 0xFF, 0xE0),    _T("Light Yellow")      },
	{ RGB(204, 255, 204),		_T("Light Green ")      },
	{ RGB(204, 255, 255),		_T("Light Turquoise")	},
	{ RGB(153, 204, 255),		_T("Pale Blue")         },
	{ RGB(204, 153, 255),		_T("Lavender")          },
	{ RGB(0xFF, 0xFF, 0xFF),    _T("White")             }
};

namespace
{
	template<class ColotButton>
	void DoCreateColorButtonPalette(CPalette& palette, int& colors)
	{
		if (palette.GetSafeHandle() == NULL)
		{
			colors = sizeof(crColours) / sizeof(ColourTableEntry);
			ASSERT(colors <= MAX_COLOURS);

			if (colors > MAX_COLOURS)
				colors = MAX_COLOURS;

			// Create the palette
			struct
			{
				LOGPALETTE LogPalette;
				PALETTEENTRY PalEntry[MAX_COLOURS];
			} pal;

			LOGPALETTE* pLogPalette = (LOGPALETTE*)&pal;
			pLogPalette->palVersion = 0x300;
			pLogPalette->palNumEntries = (WORD)colors;

			for (int i = 0; i < colors; i++)
			{
				pLogPalette->palPalEntry[i].peRed = GetRValue(crColours[i].crColour);
				pLogPalette->palPalEntry[i].peGreen = GetGValue(crColours[i].crColour);
				pLogPalette->palPalEntry[i].peBlue = GetBValue(crColours[i].crColour);
				pLogPalette->palPalEntry[i].peFlags = 0;
			}

			palette.CreatePalette(pLogPalette);
		}

		// Initialize color names:
		for (int i = 0; i < colors; i++)
			ColotButton::SetColorName(crColours[i].crColour, crColours[i].szName);
	}
}

void CreateColorMenuButtonPalette(CPalette& palette, int& colors)
{
	DoCreateColorButtonPalette<CMFCColorMenuButton> (palette, colors);
}

void CreateColorButtonPalette(CPalette& palette, int& colors, int& columns)
{
	DoCreateColorButtonPalette<CMFCColorButton> (palette, colors);
	columns = 8;
}

std::auto_ptr<CMFCColorMenuButton> CreateColorMenuButton(UINT id, UINT tear_off_id,
		CPalette& palette, int& colors, UINT title_id)
{
	CreateColorMenuButtonPalette(palette, colors);

	std::auto_ptr<CMFCColorMenuButton> pColorButton(new CMFCColorMenuButton(id, CString(
		MAKEINTRESOURCE(title_id)), &palette));

	pColorButton->EnableAutomaticButton(CString(MAKEINTRESOURCE(IDS_AUTOMATIC)), RGB(0, 0, 0));
	pColorButton->EnableOtherButton(CString(MAKEINTRESOURCE(STE_COLOR_OTHERS)));
	pColorButton->EnableDocumentColors(_T("Document's Colors"));
	pColorButton->SetColumnsNumber(8);
	pColorButton->EnableTearOff(tear_off_id, 5, 2);

	return pColorButton;
}

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_EXTRAS_CUSTOMIZE, &CMainFrame::OnExtrasCustomize)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_ACTIVATE()
	ON_COMMAND(ID_OPTIONS_CHANGED, &CMainFrame::OnOptionsChanged)
	ON_COMMAND(ID_WINDOW_EDITOR, &CMainFrame::OnWindowEditor)
	ON_COMMAND(ID_WINDOW_LIST, &CMainFrame::OnWindowList)
	ON_COMMAND(ID_HELP_FINDER, &CMainFrame::OnHelpSearch)
	ON_COMMAND(ID_HELP_INDEX, &CMainFrame::OnHelpIndex)
	//ON_COMMAND(ID_HELP, &CMainFrame::WinHelp)
	ON_COMMAND(ID_VIEW_FULLSCREEN, &CMainFrame::OnViewFullScreen)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FULLSCREEN, &CMainFrame::OnUpdateViewFullScreen)
	ON_COMMAND(ID_HELP_KEYMAPPING, &CMainFrame::OnHelpKeyMapping)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_NEXT, &CMainFrame::OnUpdateWindowNext)
	ON_COMMAND(ID_WINDOW_NEXT, &CMainFrame::OnWindowNext)
	ON_COMMAND(ID_WINDOW_PREVIOUS, &CMainFrame::OnWindowPrevious)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_PREVIOUS, &CMainFrame::OnUpdateWindowPrevious)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_HELP_CONTENTS, &CMainFrame::OnHelpContents)
	ON_WM_HELPINFO()
	ON_COMMAND(ID_VIEW_DOCTAB_BOTTOM, &CMainFrame::OnViewDocTabsBottom)
	ON_COMMAND(ID_VIEW_DOCTAB_OFF, &CMainFrame::OnViewDocTabsOff)
	ON_COMMAND(ID_VIEW_DOCTAB_TOP, &CMainFrame::OnViewDocTabsTop)
	ON_COMMAND(ID_VIEW_DOCTAB_ICONS, &CMainFrame::OnViewDocTabsIcons)
	ON_COMMAND(ID_VIEW_DOCTAB_NOTE, &CMainFrame::OnViewDocTabsNote)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DOCTAB_BOTTOM, &CMainFrame::OnUpdateViewDocTabs)
	ON_COMMAND_EX(ID_TOOLS_CANCEL, &CMainFrame::OnToolsCancel)
	ON_COMMAND(ID_WINDOW_CLOSE_SELECTEDTAB, &CMainFrame::OnWindowCloseSelectedTab)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_CLOSE_SELECTEDTAB, &CMainFrame::OnUpdateWindowCloseSelectedTab)
	ON_COMMAND(ID_WINDOW_CLOSE_ALL_BUTACTIVE, &CMainFrame::OnWindowCloseAllButActive)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_CLOSE_ALL_BUTACTIVE, &CMainFrame::OnUpdateWindowCloseAllButActive)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DOCTAB_OFF, &CMainFrame::OnUpdateViewDocTabs)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DOCTAB_TOP, &CMainFrame::OnUpdateViewDocTabs)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DOCTAB_ICONS, &CMainFrame::OnUpdateViewDocTabs)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DOCTAB_NOTE, &CMainFrame::OnUpdateViewDocTabs)
	//ON_COMMAND(ID_HELP_FINDER, &CMDIFrameWndEx::OnHelpFinder)
	//ON_COMMAND(ID_HELP, &CMDIFrameWndEx::OnHelp)
	//ON_COMMAND(ID_CONTEXT_HELP, CMDIFrameWndEx::OnContextHelp)
	//ON_COMMAND(ID_DEFAULT_HELP, CMDIFrameWndEx::OnHelpFinder)
	ON_COMMAND_EX_RANGE(ID_MATH1, ID_MATH16, &CMainFrame::OnToggleCtrlBar)
	ON_COMMAND_EX(ID_VIEW_MATH, &CMainFrame::OnToggleCtrlBar)
	ON_UPDATE_COMMAND_UI_RANGE(ID_MATH1, ID_MATH16, &CMainFrame::OnCheckCtrlBarVisible)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MATH, &CMainFrame::OnCheckCtrlBarVisible)
	ON_REGISTERED_MESSAGE(AFX_WM_RESETTOOLBAR, &CMainFrame::OnResetToolbar)
	ON_REGISTERED_MESSAGE(AFX_WM_CUSTOMIZETOOLBAR, &CMainFrame::OnCustomizationMode)
	ON_UPDATE_COMMAND_UI_RANGE(ID_USER_TOOL_FIRST, ID_USER_TOOL_LAST, &CMainFrame::OnUpdateUserTool)
	ON_COMMAND_RANGE(ID_USER_TOOL_FIRST, ID_USER_TOOL_LAST, &CMainFrame::OnExecuteUserTool)
	ON_COMMAND_RANGE(ID_VIEW_APP_LOOK_WIN_2000,ID_VIEW_APP_LOOK_OFFICE_2007_AQUA, &CMainFrame::OnApplicationLook)
	ON_COMMAND_RANGE(ID_VIEW_APP_LOOK_WIN7, ID_VIEW_APP_LOOK_WIN7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APP_LOOK_WIN_2000, ID_VIEW_APP_LOOK_OFFICE_2007_AQUA, &CMainFrame::OnUpdateApplicationLook)
	ON_UPDATE_COMMAND_UI(ID_VIEW_APP_LOOK_WIN7, &CMainFrame::OnUpdateApplicationLook)
	ON_MESSAGE_VOID(AfxUserMessages::StartPaneAnimationMessageID, OnStartPaneAnimation)
	ON_MESSAGE(AfxUserMessages::StopPaneAnimationMessageID, &CMainFrame::OnStopPaneAnimation)
	ON_REGISTERED_MESSAGE(AFX_WM_ON_GET_TAB_TOOLTIP, &CMainFrame::OnGetTabToolTip)
	ON_COMMAND(ID_VIEW_TRANSPARENCY, &CMainFrame::OnViewTransparency)
	ON_WM_DESTROY()
	ON_MESSAGE_VOID(AfxUserMessages::CheckForFileChangesMessageID, CheckForFileChanges)
	ON_COMMAND(ID_VIEW_TOGGLEBOTTOMDOCKINGBARS, &CMainFrame::OnViewToggleBottomDockingBars)
	ON_COMMAND(ID_VIEW_TOGGLELEFTDOCKINGBARS, &CMainFrame::OnViewToggleLeftDockingBars)
	ON_COMMAND(ID_VIEW_TOGGLERIGHTDOCKINGBARS, &CMainFrame::OnViewToggleRightDockingBars)
	ON_COMMAND(ID_VIEW_TOGGLETOPDOCKINGBARS, &CMainFrame::OnViewToggleTopDockingBars)
	ON_COMMAND_EX(ID_VIEW_STRUCT_PANE, &CMainFrame::OnToggleDockingBar)
	ON_COMMAND_EX(ID_VIEW_ENV_PANE, &CMainFrame::OnToggleDockingBar)
	ON_COMMAND_EX(ID_VIEW_FILES_PANE, &CMainFrame::OnToggleDockingBar)
	ON_COMMAND_EX(ID_VIEW_BIB_ENTRIES_PANE, &CMainFrame::OnToggleDockingBar)
	ON_COMMAND_EX(ID_VIEW_ERROR_LIST_PANE, &CMainFrame::OnToggleDockingBar)
	ON_COMMAND_EX(ID_VIEW_BOOKMARKS_PANE, &CMainFrame::OnToggleDockingBar)
	ON_COMMAND_EX(ID_VIEW_BUILD_PANE, &CMainFrame::OnToggleDockingBar)
	ON_COMMAND_EX(ID_VIEW_PREVIEW_OUTPUT_PANE, &CMainFrame::OnToggleDockingBar)
	ON_COMMAND_EX(ID_VIEW_PREVIEW_IMAGE_PANE, &CMainFrame::OnToggleDockingBar)
	ON_COMMAND_EX(ID_VIEW_GREP_1_PANE, &CMainFrame::OnToggleDockingBar)
	ON_COMMAND_EX(ID_VIEW_GREP_2_PANE, &CMainFrame::OnToggleDockingBar)
	ON_COMMAND_EX(ID_VIEW_PARSE_PANE, &CMainFrame::OnToggleDockingBar)
	ON_MESSAGE(AfxUserMessages::ShowDockingBarID, &CMainFrame::OnShowDockingBar)
	ON_MESSAGE(WM_COMMANDHELP, &CMainFrame::OnCommandHelp)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_CASCADE, &CMainFrame::OnUpdateWindowMDICommands)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_TILE_HORZ, &CMainFrame::OnUpdateWindowMDICommands)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_TILE_VERT, &CMainFrame::OnUpdateWindowMDICommands)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_ARRANGE, &CMainFrame::OnUpdateWindowMDICommands)
END_MESSAGE_MAP()

const UINT BuildAnimationPane = 1;


static UINT indicators[] =
{
	ID_SEPARATOR, // Statusleistenanzeige
	IDB_BUILD_ANIMATION, // Build animation
	ID_EDIT_INDICATOR_SELECTION,
	ID_EDIT_INDICATOR_POSITION,
	ID_EDIT_INDICATOR_ENCODING,
	ID_EDIT_INDICATOR_CRLF,
	ID_INDICATOR_OVR,
	ID_EDIT_INDICATOR_READ,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

CMainFrame::CMainFrame()
	: m_pContextMenuTargetWindow(NULL)
	, structure_view_(new StructurePane)
	, file_view_pane_(new FileViewPane)
	, env_view_pane_(new WorkspacePane)
	, env_view_(new CEnvironmentView)
	, bib_view_pane_(new BibView)
	, bookmark_view_pane_(new BookmarkView)
	, build_view_pane_(new WorkspacePane)
	, build_view_(new CBuildView)
	, preview_output_view_pane_(new WorkspacePane)
	, preview_output_view_(new CBuildView)
	, preview_image_view_pane_(new WorkspacePane)
	, preview_image_view_(new CPreviewImageView)
	, error_list_view_(new ErrorListPane)
	, grep_view_1_pane_(new WorkspacePane)
	, grep_view_1_(new CGrepView)
	, grep_view_2_(new CGrepView)
	, grep_view_2_pane_(new WorkspacePane)
	, parse_view_pane_(new WorkspacePane)
	, output_doc_(new COutputDoc)
	, parse_view_(new CParseOutputView)
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	EnableMDITabsLastActiveActivation(true); //Does not work as expected, see http://blogs.msdn.com/b/dsvc/archive/2012/10/09/cmdiframewndex-enablemditabslastactiveactivation-does-not-work-as-expected-in-mdi-tabbed-style-application.aspx
	OnApplicationLook(theApp.GetApplicationLook());

	// load user images
	if (!m_userImages.Load(theApp.GetWorkingDir() + _T("\\UserImages.bmp")))
	{
		TRACE(_T("Failed to load user images\n"));
		//ASSERT(FALSE);
	}
	else
	{
		CMFCToolBar::SetUserImages(&m_userImages);
	}

	// create menu bar
	if (!CConfiguration::GetInstance()->m_bOptimizeMenuForVisuallyHandicappedUsers)
	{
		if (!m_wndMenuBar.Create(this))
		{
			TRACE0("Failed to create menubar\n");
			return -1;
		}

		m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS
				| CBRS_FLYBY);
	}
	else
	{
		// load menu bars
		m_stdMenu.LoadMenu(IDR_MAINFRAME);
		m_projectMenu.LoadMenu(IDR_LATEXPROJECTDOCTYPE);
		MDISetMenu(&m_stdMenu, NULL);
	}

	// create status bar
	if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(indicators,
		sizeof(indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1; // creation failed
	}

	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Allow disabled menu items to be highlighted...
	//CMFCMenuBar::HighlightDisabledItems();
	CMFCToolBar::EnableQuickCustomization();
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	// create main tool bar
	if (!CreateToolBar(&m_wndToolBar, IDR_MAINFRAME, STE_TB_MAINFRAME)) return -1;

	// create find bar
	if (!CreateToolBar(&m_wndFindBar, IDR_FIND, STE_TB_FIND, false)) return -1;

	// create insert bar
	if (!CreateToolBar(&m_wndInsertBar, IDR_INSERT, STE_TB_INSERT)) return -1;

	// create format recently used tool bar
	if (!CreateToolBar(&m_wndFormatRUBar, IDR_FORMAT_RECENTLY_USED, STE_TB_FORMAT_RECENTLY_USED)) return -1;

	// create format tool bar
	if (!CreateToolBar(&m_wndFormatBar, IDR_FORMAT, STE_TB_FORMAT, false)) return -1;

	// create latex tool bar
	if (!CreateToolBar(&m_wndLatexBar, IDR_LATEX, STE_TB_LATEX)) return -1;

	// create math tool bar
	if (!CreateToolBar(&m_wndMathBar, IDR_MATH, STE_TB_MATH)) return -1;

	// create all math bars
	for (int i = 0; i < MATHBAR_COUNT; i++)
	{
		if (!CreateToolBar(&m_awndMathBar[i], IDR_MATHBAR1 + i, IDR_MATH + i, false)) return -1;
	}

	// create preview tool bar
	if (!CreateToolBar(&m_wndPreviewBar, IDR_LATEX, STE_TB_LATEX)) return -1;

	// load user defined toolbars
	InitUserToolbars(NULL, IDR_USER_TOOLBAR_FIRST, IDR_USER_TOOLBAR_LAST);

	// initialization
	CMFCToolBar::AddToolBarForImageCollection(IDR_IMAGES);
	CMFCToolBar::AddToolBarForImageCollection(IDR_ADDITIONAL_COMMANDS);
	CMFCToolBar::AddToolBarForImageCollection(IDR_NAVIGATION_BAR);
	CMFCToolBar::AddToolBarForImageCollection(IDR_OUTPUT_BAR);

	// dock the bars
	if (!CConfiguration::GetInstance()->m_bOptimizeMenuForVisuallyHandicappedUsers)
		m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndFindBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndInsertBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndFormatRUBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndFormatBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndLatexBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndMathBar.EnableDocking(CBRS_ALIGN_ANY);

	EnableFullScreenMode(ID_VIEW_FULLSCREEN);
	EnableDocking(CBRS_ALIGN_ANY);

	for (int i = 0; i < MATHBAR_COUNT; i++)
	{
		m_awndMathBar[i].EnableDocking(CBRS_ALIGN_ANY);
	}

	if (!CConfiguration::GetInstance()->m_bOptimizeMenuForVisuallyHandicappedUsers)
	{
		DockPane(&m_wndMenuBar);
	}

	DockPane(&m_wndLatexBar);
	DockPaneLeftOf(&m_wndToolBar, &m_wndLatexBar);

	DockPane(&m_wndMathBar);
	DockPaneLeftOf(&m_wndFormatRUBar, &m_wndMathBar);
	DockPaneLeftOf(&m_wndInsertBar, &m_wndFormatRUBar);

	//NOTE: All bars need to be docked. Even if they shall be invisible at first start!
	DockPane(&m_wndFindBar);
	DockPane(&m_wndFormatBar);
	for (int i = 0; i < MATHBAR_COUNT; ++i)
	{
		DockPane(m_awndMathBar + i);
	}

	CreateToolWindows();

	// Enable window list manager...
	if (!CConfiguration::GetInstance()->m_bOptimizeMenuForVisuallyHandicappedUsers)
	{
		EnableWindowsDialog(ID_WINDOW_LIST, ID_WINDOW_LIST_MENU, TRUE);
	}
	else
	{
		// add entry for window list to the end of window menu
		CMenu *pmnuWindowMenu = m_stdMenu.GetSubMenu(m_stdMenu.GetMenuItemCount() - 2);
		pmnuWindowMenu->AppendMenu(MF_SEPARATOR);
		pmnuWindowMenu->AppendMenu(MF_BYPOSITION, ID_WINDOW_LIST, CString(
			(LPCTSTR)ID_WINDOW_LIST_MENU));
	}

	EnablePaneMenu(TRUE, ID_EXTRAS_CUSTOMIZE, CString(MAKEINTRESOURCE(IDS_TOOLBAR_CUSTOMIZE)),
		ID_VIEW_TOOLBAR, FALSE, TRUE);

	build_animation_.m_hImageList = ::ImageList_LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(
		IDB_BUILD_ANIMATION), 16, 1, RGB(192, 192, 192), IMAGE_BITMAP, LR_CREATEDIBSECTION);

	// Recalculate layout
	m_wndStatusBar.SetPaneText(BuildAnimationPane, 0);

	m_wndStatusBar.SetPaneAnimation(BuildAnimationPane, build_animation_);
	m_wndStatusBar.SetPaneAnimation(BuildAnimationPane, 0);

	int cx, cy;

	if (::ImageList_GetIconSize(build_animation_, &cx, &cy))
		m_wndStatusBar.SetPaneWidth(BuildAnimationPane, cx); // Adjust the width

	m_wndStatusBar.SetPaneStyle(BuildAnimationPane, SBPS_NOBORDERS);

	//CList<UINT, UINT> lstBasicCommands;

	//lstBasicCommands.AddTail(ID_FILE_NEW);
	//lstBasicCommands.AddTail(ID_FILE_OPEN);
	//lstBasicCommands.AddTail(ID_FILE_SAVE);
	//lstBasicCommands.AddTail(ID_FILE_PRINT);
	//lstBasicCommands.AddTail(ID_APP_EXIT);
	//lstBasicCommands.AddTail(ID_EDIT_CUT);
	//lstBasicCommands.AddTail(ID_EDIT_PASTE);
	//lstBasicCommands.AddTail(ID_EDIT_UNDO);
	//lstBasicCommands.AddTail(ID_APP_ABOUT);
	//lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
	//lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);

	//CMFCToolBar::SetBasicCommands(lstBasicCommands);

	int value = CConfiguration::GetInstance()->GetTransparency();

	if (value > 0)
		TransparencyDlg::SetTransparency(this, value);

	SendMessage(WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEACCEL | UISF_HIDEFOCUS));

	if (SUCCEEDED(taskBarList_.CoCreateInstance(CLSID_TaskbarList)))
	{
		if (FAILED(taskBarList_->HrInit())) taskBarList_.Release();
	}

	//LoadToolBarsState();

	return 0;
}

void CMainFrame::OnClose()
{
	// If we are still in full screen mode, restore normal mode before closing
	// to ensure proper control position saving.
	if (IsFullScreen())
		ShowFullScreen();

	theApp.EndSession();

	CMDIFrameWndEx::OnClose();
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	//Do not add the title of the MDI child to the frame window title. We do this ourselves.
	cs.style &= ~FWS_ADDTOTITLE;

	if (!CMDIFrameWndEx::PreCreateWindow(cs))
		return FALSE;
	return TRUE;
}

bool CMainFrame::CreateToolBar(CMFCToolBar* pToolBar, UINT unID, UINT unTitleID, bool bVisible /*= true*/)
{
	DWORD style = AFX_DEFAULT_TOOLBAR_STYLE;

	if (!bVisible)
		style &= ~WS_VISIBLE;

	if (!pToolBar->Create(this, style, unID) || !pToolBar->LoadToolBar(unID))
	{
		TRACE1("!Failed to create %s tool bar\n", (LPCTSTR)AfxLoadString(unTitleID));
		return false; // creation failed
	}

	pToolBar->SetWindowText(AfxLoadString(unTitleID));
	pToolBar->EnableTextLabels(FALSE);
	pToolBar->EnableCustomizeButton(TRUE, ID_EXTRAS_CUSTOMIZE, IDS_TOOLBAR_CUSTOMIZE);

	return true;
}


CBasePane* CMainFrame::GetControlBarByCmd(UINT unID)
{
	CBasePane* result;

	if (unID >= ID_MATH1 && unID <= ID_MATH16)
		result = &m_awndMathBar[unID - ID_MATH1];
	else if (unID >= ID_VIEW_USER_TOOLBAR_FIRST && unID <= ID_VIEW_USER_TOOLBAR_LAST)
		result = GetUserToolBarByIndex(unID - ID_VIEW_USER_TOOLBAR_FIRST);
	else if (unID == ID_VIEW_MATH)
		result = &m_wndMathBar;
	else
		result = GetPane(unID);

	return result;
}

void CMainFrame::GetAllPanes(std::vector< CBasePane* >& pAllPanes, bool bNavigatorPanes /*= true*/, bool bOutputPanes /*= true*/)
{
	pAllPanes.clear();

	//Note that the order of the panes is important.
	// See enum tagOutputTabs and ActivateOutputTab().
	// The last pane will also be the active one after ToggleDockingBars().

	if (bNavigatorPanes)
	{
		pAllPanes.push_back(bookmark_view_pane_.get());
		pAllPanes.push_back(preview_image_view_pane_.get());
		pAllPanes.push_back(env_view_pane_.get());
		pAllPanes.push_back(file_view_pane_.get());
		pAllPanes.push_back(bib_view_pane_.get());
		pAllPanes.push_back(structure_view_.get());
	}

	if (bOutputPanes)
	{
		pAllPanes.push_back(preview_output_view_pane_.get());
		pAllPanes.push_back(parse_view_pane_.get());
		pAllPanes.push_back(grep_view_2_pane_.get());
		pAllPanes.push_back(grep_view_1_pane_.get());
		pAllPanes.push_back(error_list_view_.get());
		pAllPanes.push_back(build_view_pane_.get());
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame Diagnose

#ifdef _DEBUG

void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}

#endif //_DEBUG
/////////////////////////////////////////////////////////////////////////////
// CMainFrame Nachrichten-Handler

BOOL CMainFrame::DestroyWindow()
{
	return CMDIFrameWndEx::DestroyWindow();
}

void CMainFrame::OnExtrasCustomize()
{
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// create dialog object

	ToolBarsCustomizeDialog* pDlgCust = new ToolBarsCustomizeDialog(this, TRUE,
		AFX_CUSTOMIZE_MENU_SHADOWS | AFX_CUSTOMIZE_TEXT_LABELS | AFX_CUSTOMIZE_MENU_ANIMATIONS
				| AFX_CUSTOMIZE_NOHELP);

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// enable create/delete of user defined toolbars
	pDlgCust->EnableUserDefinedToolbars();

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// add menus, toolbars and commands
	pDlgCust->AddMenu(IDR_LATEXPROJECTDOCTYPE);
	pDlgCust->AddMenu(IDR_MAINFRAME);
	pDlgCust->AddMenu(IDR_HIDDEN_CMDS);
	pDlgCust->AddMenu(IDR_POPUP_ITEM);

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// create combobox for output typess

	CMFCToolBarComboBoxButton profileButton(ID_LATEX_PROFILE_SEL, 1,
	//CImageHash::GetImageOfCommand( ID_LATEX_PROFILE_SEL, FALSE ),
		CBS_DROPDOWNLIST | CBS_SORT);

	// add available outputtypes to combo box
	CStringArray astrProfiles;
	CProfileMap::GetInstance()->GetKeyList(astrProfiles);

	for (int i = 0; i < astrProfiles.GetSize(); i++)
		profileButton.AddItem(astrProfiles[i]);

	profileButton.SelectItem(CProfileMap::GetInstance()->GetActiveProfileKey());

	// replace button
	pDlgCust->ReplaceButton(ID_LATEX_PROFILE_SEL, profileButton);

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// display dialog
	pDlgCust->Create();

	theApp.UpdateLaTeXProfileSel();
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
		case timerSave: // save and parse project
			if (CConfiguration::GetInstance()->m_unSaveInterval < 1)
				CConfiguration::GetInstance()->m_unSaveInterval = 1;

			theApp.SaveAllModifiedWithoutPrompt();
			break;
	}

	CMDIFrameWndEx::OnTimer(nIDEvent);
}

void CMainFrame::CheckForFileChanges()
{
	// Check if files have changed
	CMultiDocTemplate *pTemplate = theApp.GetLatexDocTemplate();
	ASSERT_NULL_OR_POINTER(pTemplate, CMultiDocTemplate);

	if (pTemplate)
	{
		POSITION pos = pTemplate->GetFirstDocPosition();

		while (pos)
		{
			LaTeXDocument *pDoc = dynamic_cast<LaTeXDocument*> (pTemplate->GetNextDoc(pos));

			if (pDoc)
				pDoc->CheckForFileChanges();
		}
	}
}

void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CMDIFrameWndEx::OnActivate(nState, pWndOther, bMinimized);

	switch (nState)
	{
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			CheckForFileChangesAsync();
			break;
		case WA_INACTIVE:
			if (!animating_)
				HideTaskbarProgress();
			break;
	}
}

void CMainFrame::OnOptionsChanged()
{
	// install/uninstall timer for automatic saving
	KillTimer( timerSave);

	if (CConfiguration::GetInstance()->m_bSaveAutomatic)
		SetTimer(timerSave, CConfiguration::GetInstance()->m_unSaveInterval * 1000 * 60, NULL);
}


BOOL CMainFrame::OnToggleCtrlBar(UINT nIDEvent)
{
	CBasePane* pCtrlBar = GetControlBarByCmd(nIDEvent);
	ASSERT(pCtrlBar);
	if (pCtrlBar)
	{
		ASSERT(IsWindow(pCtrlBar->m_hWnd));
		ShowPane(pCtrlBar, !pCtrlBar->IsPaneVisible(), FALSE, !pCtrlBar->IsPaneVisible());
		return true;
	}

	return false;
}


BOOL CMainFrame::OnToggleDockingBar(UINT nIDEvent)
{
	//It doesn't seem to work with GetControlBarByCmd()
	CBasePane* pCtrlBar = NULL;
	switch (nIDEvent)
	{
		case ID_VIEW_STRUCT_PANE:
			pCtrlBar = structure_view_.get();
			break;

		case ID_VIEW_ENV_PANE:
			pCtrlBar = env_view_pane_.get();
			break;

		case ID_VIEW_FILES_PANE:
			pCtrlBar = file_view_pane_.get();
			break;

		case ID_VIEW_BIB_ENTRIES_PANE:
			pCtrlBar = bib_view_pane_.get();
			break;

		case ID_VIEW_ERROR_LIST_PANE:
			pCtrlBar = error_list_view_.get();
			break;

		case ID_VIEW_BOOKMARKS_PANE:
			pCtrlBar = bookmark_view_pane_.get();
			break;

		case ID_VIEW_BUILD_PANE:
			pCtrlBar = build_view_pane_.get();
			break;

		case ID_VIEW_PREVIEW_OUTPUT_PANE:
			pCtrlBar = preview_output_view_pane_.get();
			break;

		case ID_VIEW_PREVIEW_IMAGE_PANE:
			pCtrlBar = preview_image_view_pane_.get();
			break;

		case ID_VIEW_GREP_1_PANE:
			pCtrlBar = grep_view_1_pane_.get();
			break;

		case ID_VIEW_GREP_2_PANE:
			pCtrlBar = grep_view_2_pane_.get();
			break;

		case ID_VIEW_PARSE_PANE:
			pCtrlBar = parse_view_pane_.get();
			break;
	}

	if (pCtrlBar)
	{
		ShowPaneEnsureVisibility(pCtrlBar, !pCtrlBar->IsPaneVisible(), FALSE, !pCtrlBar->IsPaneVisible());
		return true;
	}

	return false;
}


LRESULT CMainFrame::OnShowDockingBar(WPARAM wParam, LPARAM /*lParam*/)
{
	CBasePane* pCtrlBar = GetControlBarByCmd(wParam);
	ASSERT(pCtrlBar);
	if (pCtrlBar)
	{
		ASSERT(IsWindow(pCtrlBar->m_hWnd));
		ShowPane(pCtrlBar, TRUE, FALSE, TRUE);
		return true;
	}

	return 0L;
}


void CMainFrame::OnCheckCtrlBarVisible(CCmdUI *pCmdUI)
{
	CBasePane* pCtrlBar = GetControlBarByCmd(pCmdUI->m_nID);
	ASSERT(pCtrlBar);
	if (pCtrlBar)
	{
		ASSERT(IsWindow(pCtrlBar->m_hWnd));
		pCmdUI->SetCheck(pCtrlBar->IsPaneVisible());
	}
}

LRESULT CMainFrame::OnToolbarContextMenu(WPARAM /*wp*/, LPARAM lp)
{
	CPoint point(LOWORD(lp), HIWORD(lp));

	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_POPUP_TOOLBAR));

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	// Add user-defined toolbar names to the end
	for (int i = 0; i < (IDR_USER_TOOLBAR_LAST - IDR_USER_TOOLBAR_FIRST + 1); i++)
	{
		CMFCToolBar *pToolBar = GetUserToolBarByIndex(i);
		if (pToolBar != NULL)
		{
			// add title of current tool bar to menu
			CString strToolbarName;
			pToolBar->GetWindowText(strToolbarName);
			pPopup->InsertMenu(pPopup->GetMenuItemCount() - 2, MF_STRING | MF_BYPOSITION,
				ID_VIEW_USER_TOOLBAR_FIRST + i, strToolbarName);
		}
	}

	// display menu
	CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;
	pPopupMenu->Create(this, point.x, point.y, pPopup->Detach());

	return 0;
}

void CMainFrame::OnWindowEditor()
{
	// get active frame
	CFrameWnd *pFrame = GetActiveFrame();

	if (!pFrame)
		return;

	// get active view
	CView *pView = pFrame->GetActiveView();

	if (!pView || !pView->IsKindOf(RUNTIME_CLASS(LaTeXView)))
		return;

	// activate view
	pView->SetFocus();
}

void CMainFrame::ActivateOutputTab(int nTab, bool /*bSetFocus*/)
{
	std::vector< CBasePane* > pAllPanes;
	GetAllPanes(pAllPanes, false, true);

	if (nTab >= 0 && (unsigned int)(nTab) < pAllPanes.size())
	{
		// Special case: Activate the build view only if the error list view isn't active
		if (nTab != outputTabBuildResult || !error_list_view_->IsWindowVisible())
		{
			ShowPaneEnsureVisibility(pAllPanes[nTab], true, false, true);
		}
	}
}


#define PANE_MINIMUM_HEIGHT 100
#define PANE_MINIMUM_WIDTH 100
void CMainFrame::ShowPaneEnsureVisibility(CBasePane* pPane, bool bShow, bool bDelay, bool bActivate /*= true*/)
{
	if (!pPane) return;

	//Call the actual implementation
	pPane->ShowPane(bShow, bDelay, bActivate);

	//Make sure the pane is actually visible
	if (bShow)
	{
		CBasePane* pBase = pPane->GetParentTabbedPane();
		if (!pBase)
		{
			pBase = pPane; //If we are not tabbed, we are on our own.
		}

		//Get its geometry
		CRect Rect;
		pBase->GetWindowRect(&Rect);

		//Check Height
		bool bAdjust(false);
		if (Rect.Height() < PANE_MINIMUM_HEIGHT)
		{
			if (pBase->GetCurrentAlignment() == CBRS_ALIGN_BOTTOM)
			{
				Rect.top = Rect.bottom - PANE_MINIMUM_HEIGHT;
			}
			else
			{
				Rect.bottom = Rect.top + PANE_MINIMUM_HEIGHT;
			}

			bAdjust = true;
		}

		//Check Width
		bool bWorkaroundRightAlignment(false);
		if (Rect.Width() < PANE_MINIMUM_WIDTH)
		{
			if (pBase->GetCurrentAlignment() == CBRS_ALIGN_RIGHT)
			{
				Rect.left = Rect.right - PANE_MINIMUM_WIDTH;
				bWorkaroundRightAlignment = true;
			}
			else
			{
				Rect.right = Rect.left + PANE_MINIMUM_WIDTH;
			}

			bAdjust = true;
		}

		//Adjust, if necessary
		if (bAdjust)
		{
			pBase->SetWindowPos(0, 0, 0, Rect.Width(), Rect.Height(), SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

			//If the pane is right aligned, it will not be properly adjusted the first time. Don't ask why.
			if (bWorkaroundRightAlignment)
			{
				pPane->ShowPane(false, false, false);
				pPane->ShowPane(bShow, bDelay, bActivate);
			}
		}
	}
}


BOOL CMainFrame::ReplaceToolbarButton(UINT unCmdID, const CMFCToolBarButton &button)
{
	// replace the first occurrence of the button
	if (m_wndFindBar.CommandToIndex(unCmdID) > -1)
	{
		m_wndFindBar.ReplaceButton(unCmdID, button);
		return TRUE;
	}

	if (m_wndFormatBar.CommandToIndex(unCmdID) > -1)
	{
		m_wndFormatBar.ReplaceButton(unCmdID, button);
		return TRUE;
	}

	if (m_wndFormatRUBar.CommandToIndex(unCmdID) > -1)
	{
		m_wndFormatRUBar.ReplaceButton(unCmdID, button);
		return TRUE;
	}

	if (m_wndInsertBar.CommandToIndex(unCmdID) > -1)
	{
		m_wndInsertBar.ReplaceButton(unCmdID, button);
		return TRUE;
	}

	if (m_wndLatexBar.CommandToIndex(unCmdID) > -1)
	{
		m_wndLatexBar.ReplaceButton(unCmdID, button);
		return TRUE;
	}

	if (m_wndMathBar.CommandToIndex(unCmdID) > -1)
	{
		m_wndMathBar.ReplaceButton(unCmdID, button);
		return TRUE;
	}

	if (m_wndToolBar.CommandToIndex(unCmdID) > -1)
	{
		m_wndToolBar.ReplaceButton(unCmdID, button);
		return TRUE;
	}

	for (int i = 0; i < MATHBAR_COUNT; i++)
	{
		if (m_awndMathBar[i].CommandToIndex(unCmdID) > -1)
		{
			m_awndMathBar[i].ReplaceButton(unCmdID, button);
			return TRUE;
		}
	}

	// button not found
	return FALSE;
}

CMFCToolBarButton *CMainFrame::GetToolBarButton(UINT unCmdID, int& pos) const
{
	// find and return specified occurence of the button
	int nIndex;
	int nStartBar = pos;

	if (nStartBar < 1 && (nIndex = m_wndFindBar.CommandToIndex(unCmdID)) > -1)
	{
		pos = 1;
		return m_wndFindBar.GetButton(nIndex);
	}

	if (nStartBar < 2 && (nIndex = m_wndFormatBar.CommandToIndex(unCmdID)) > -1)
	{
		pos = 2;
		return m_wndFormatBar.GetButton(nIndex);
	}

	if (nStartBar < 3 && (nIndex = m_wndFormatRUBar.CommandToIndex(unCmdID)) > -1)
	{
		pos = 3;
		return m_wndFormatRUBar.GetButton(nIndex);
	}

	if (nStartBar < 4 && (nIndex = m_wndInsertBar.CommandToIndex(unCmdID)) > -1)
	{
		pos = 4;
		return m_wndInsertBar.GetButton(nIndex);
	}

	if (nStartBar < 5 && (nIndex = m_wndLatexBar.CommandToIndex(unCmdID)) > -1)
	{
		pos = 5;
		return m_wndLatexBar.GetButton(nIndex);
	}

	if (nStartBar < 6 && (nIndex = m_wndMathBar.CommandToIndex(unCmdID)) > -1)
	{
		pos = 6;
		return m_wndMathBar.GetButton(nIndex);
	}

	if (nStartBar < 7 && (nIndex = m_wndToolBar.CommandToIndex(unCmdID)) > -1)
	{
		pos = 7;
		return m_wndToolBar.GetButton(nIndex);
	}

	//Look for the button in the Math-Toolbars
	if (nStartBar < 8 + MATHBAR_COUNT - 1)
	{
		for (int i = 0; i < MATHBAR_COUNT; i++)
		{
			if (nStartBar < (i + 8) && (nIndex = m_awndMathBar[i].CommandToIndex(unCmdID)) > -1)
			{
				pos = (i + 8);
				return m_awndMathBar[i].GetButton(nIndex);
			}
		}
	}

	//Find the Toolbutton in the UserToolbars
	for (int i = 0; i < (IDR_USER_TOOLBAR_LAST - IDR_USER_TOOLBAR_FIRST + 1); i++)
	{
		CMFCToolBar *pToolBar = GetUserToolBarByIndex(i);

		if (!pToolBar) continue;

		if (nStartBar < (i + 8 + MATHBAR_COUNT)
		&& (nIndex = pToolBar->CommandToIndex(unCmdID)) > -1)
		{
			pos = i + 8 + MATHBAR_COUNT;
			return pToolBar->GetButton(nIndex);
		}
	}

	// button not found
	return NULL;
}

LRESULT CMainFrame::OnResetToolbar(WPARAM wParam, LPARAM /*lParam*/)
{
	const UINT id = wParam;
	// modify toolbar buttons

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// create combobox for output types
	CMFCToolBarComboBoxButton profileButton(ID_LATEX_PROFILE_SEL, -1, //CImageHash::GetImageOfCommand( ID_LATEX_PROFILE_SEL, FALSE ),
		CBS_DROPDOWNLIST | CBS_SORT);

	// add available outputtypes to combo box
	CStringArray astrProfiles;
	CProfileMap::GetInstance()->GetKeyList(astrProfiles);

	for (INT_PTR i = 0; i < astrProfiles.GetSize(); i++)
		profileButton.AddItem(astrProfiles[i]);

	profileButton.SelectItem(CProfileMap::GetInstance()->GetActiveProfileKey());

	// replace button
	ReplaceToolbarButton(ID_LATEX_PROFILE_SEL, profileButton);

	if (m_wndFormatRUBar && id == static_cast<UINT>(m_wndFormatRUBar.GetDlgCtrlID()))
	{
		int colors;
		CPalette palette;

		std::auto_ptr<CMFCColorMenuButton> b1(CreateColorMenuButton(ID_FORMAT_TEXT_BACK_COLOR,
			IDR_TEXT_BACK_COLOR, palette, colors, IDS_BACKGROUND_COLORS));

		m_wndFormatRUBar.ReplaceButton(b1->m_nID, *b1);

		std::auto_ptr<CMFCColorMenuButton> b2(CreateColorMenuButton(ID_FORMAT_TEXT_FORE_COLOR,
			IDR_TEXT_FORE_COLOR, palette, colors, IDS_TEXT_COLORS));

		m_wndFormatRUBar.ReplaceButton(b2->m_nID, *b2);
	}
	else if (id == static_cast<UINT>(m_wndToolBar.GetDlgCtrlID()))
	{
		CMenu menu;
		menu.CreatePopupMenu();

		POSITION pos = theApp.GetFirstDocTemplatePosition();

		while (pos)
		{
			CDocTemplate* t = theApp.GetNextDocTemplate(pos);

			CString text;
			t->GetDocString(text, CDocTemplate::regFileTypeName);

			UINT AppendID = ID_LATEX_NEW;

			if (t == theApp.GetBibTeXDocTemplate())
				AppendID = ID_BIBTEX_NEW;
			else if (t == theApp.GetMetaPostDocTemplate())
				AppendID = ID_METAPOST_NEW;

			menu.AppendMenu(MF_STRING, AppendID, text);
		}

		CMFCToolBarButton* pb = m_wndToolBar.GetButton(m_wndToolBar.CommandToIndex(ID_LATEX_NEW));
		CMFCToolBarMenuButton
				tbb(pb->m_nID, menu, pb->GetImage(), pb->m_strText, pb->m_bUserButton);

		m_wndToolBar.ReplaceButton(ID_LATEX_NEW, tbb);
	}

	return 0L;
}

BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// forward notifications from ComboBoxes as command
	if (HIWORD(wParam) == CBN_SELENDOK || HIWORD(wParam) == CBN_EDITCHANGE)
		SendMessage(WM_COMMAND, (WPARAM)LOWORD(wParam));

	return CMDIFrameWndEx::OnCommand(wParam, lParam);
}

void CMainFrame::OnWindowList()
{
	// Display the window list dialog...
	ShowWindowsDialog();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (output_doc_->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	if (bib_view_pane_->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CMDIFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::GetControlBarsEx(CArray<CPane*, CPane*>& arrBars)
{
	if (m_wndMenuBar)
		arrBars.Add(&m_wndMenuBar);
	if (m_wndStatusBar)
		arrBars.Add(&m_wndStatusBar);
	if (m_wndToolBar)
		arrBars.Add(&m_wndToolBar);
	if (m_wndFormatBar)
		arrBars.Add(&m_wndFormatBar);
	if (m_wndFormatRUBar)
		arrBars.Add(&m_wndFormatRUBar);
	if (m_wndInsertBar)
		arrBars.Add(&m_wndInsertBar);
	if (m_wndLatexBar)
		arrBars.Add(&m_wndLatexBar);
	if (m_wndMathBar)
		arrBars.Add(&m_wndMathBar);
	if (m_wndFindBar)
		arrBars.Add(&m_wndFindBar);

	for (int i = 0; i < MATHBAR_COUNT; i++)
	{
		if (m_awndMathBar[i])
			arrBars.Add(&m_awndMathBar[i]);
	}

	// Finally, for the user toolbars
	int nCount = IDR_USER_TOOLBAR_LAST - IDR_USER_TOOLBAR_FIRST + 1;

	for (int j = 0; j < nCount; j++)
	{
		CPane* pToolBar = GetUserToolBarByIndex(j);

		if (pToolBar != NULL)
			arrBars.Add(pToolBar);
	}
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd,
		CCreateContext* pContext)
{
	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	RecalcLayout();

	DisplayDocumentTabs();

	//As suggested in the MSDN, we create default user tools here.
	CUserToolsManager* pUserToolsManager = theApp.GetUserToolsManager();
	if (pUserToolsManager && pUserToolsManager->GetUserTools().IsEmpty())
	{
		CUserTool* pTool1 = pUserToolsManager->CreateNewTool();
		pTool1->m_strLabel = _T("Windows Explorer");
		pTool1->SetCommand(_T("explorer.exe"));
		pTool1->m_strArguments = _T("/n,/e,\"%dc\"");
		pTool1->m_strInitialDirectory = _T("%dc");
	}

	return TRUE;
}

void CMainFrame::OnHelpContents()
{
	HtmlHelp(0, HH_DISPLAY_TOC);
}

void CMainFrame::OnHelpSearch()
{
	HH_FTS_QUERY query = { sizeof(HH_FTS_QUERY) };
	HtmlHelp(reinterpret_cast<DWORD_PTR> (&query), HH_DISPLAY_SEARCH);
}

void CMainFrame::OnHelpIndex()
{
	HtmlHelp(0, HH_DISPLAY_INDEX);
}

LRESULT CMainFrame::OnCommandHelp(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	HtmlHelp(0, HH_DISPLAY_TOC);
	return true;
}


void CMainFrame::DisplayDocumentTabs()
{
	/* NOTE: The code below enables several tabbed groups.
		Now, in general that is a nice feature, but it kills all classic MDI functionality.
		Editor-Windows cannot be cascaded anymore, and so on.
		Furthermore, the current TXC code cannot re-create those tabbed groups.
		Nevertheless, we leave it. Tabbed groups are a nice thing to have.
		MDI functionality is still available, if tabs are not used: theApp.GetShowMDITabs().
		See also:
		http://blogs.msdn.com/b/dsvc/archive/2012/10/09/cmdiframewndex-enablemditabslastactiveactivation-does-not-work-as-expected-in-mdi-tabbed-style-application.aspx
	*/

	CMDITabInfo mdiTabParams;
	mdiTabParams.m_style = (CMFCTabCtrl::Style)theApp.m_nMDITabStyle;
	mdiTabParams.m_bActiveTabCloseButton = TRUE; // set to FALSE to place close button at right of tab area
	mdiTabParams.m_bTabIcons = theApp.m_bMDITabIcons; // set to TRUE to enable document icons on MDI tabs
	mdiTabParams.m_bAutoColor = FALSE; // set to FALSE to disable auto-coloring of MDI tabs
	mdiTabParams.m_bDocumentMenu = TRUE; // enable the document menu at the right edge of the tab area
	mdiTabParams.m_tabLocation = static_cast<CMFCTabCtrl::Location>(theApp.m_nMDITabLocation);
	mdiTabParams.m_nTabBorderSize = 3;
	mdiTabParams.m_bTabCustomTooltips = TRUE; // We will display the path name
	mdiTabParams.m_bEnableTabSwap = true; //We can change the order of the tabs

	EnableMDITabbedGroups(theApp.GetShowMDITabs(), mdiTabParams);


	//Simple MDI tabs: one group. With this, all classic MDI functionality is still available.
	//EnableMDITabs(theApp.GetShowMDITabs()
	//	,theApp.m_bMDITabIcons
	//	,static_cast<CMFCTabCtrl::Location>(theApp.m_nMDITabLocation)
	//	,TRUE // Specifies whether to display tab close buttons
	//	,(CMFCTabCtrl::Style)theApp.m_nMDITabStyle
	//	,TRUE // Specifies whether custom tooltips are enabled.
	//	,TRUE // set to FALSE to place close button at right of tab area
	//	);
}

void CMainFrame::OnViewDocTabsBottom()
{
	theApp.SetShowMDITabs(true);
	theApp.m_nMDITabLocation = CMFCTabCtrl::LOCATION_BOTTOM;
	DisplayDocumentTabs();
}

void CMainFrame::OnViewDocTabsTop()
{
	theApp.SetShowMDITabs();
	theApp.m_nMDITabLocation = CMFCTabCtrl::LOCATION_TOP;
	DisplayDocumentTabs();
}

void CMainFrame::OnViewDocTabsOff()
{
	theApp.SetShowMDITabs(false);
	DisplayDocumentTabs();
}

void CMainFrame::OnViewDocTabsIcons()
{
	theApp.m_bMDITabIcons = !theApp.m_bMDITabIcons;
	DisplayDocumentTabs();
}

void CMainFrame::OnViewDocTabsNote()
{
	if (theApp.m_nMDITabStyle == CMFCTabCtrl::STYLE_3D_SCROLLED)
	{
		theApp.m_nMDITabStyle = CMFCTabCtrl::STYLE_3D_ONENOTE;
	}
	else
	{
		theApp.m_nMDITabStyle = CMFCTabCtrl::STYLE_3D_SCROLLED;
	}

	DisplayDocumentTabs();
}

void CMainFrame::OnUpdateViewDocTabs(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
		case ID_VIEW_DOCTAB_BOTTOM:
			pCmdUI->SetRadio(theApp.GetShowMDITabs() && (theApp.m_nMDITabLocation
					== CMFCTabCtrl::LOCATION_BOTTOM));
			break;

		case ID_VIEW_DOCTAB_TOP:
			pCmdUI->SetRadio(theApp.GetShowMDITabs() && (theApp.m_nMDITabLocation
					== CMFCTabCtrl::LOCATION_TOP));
			break;

		case ID_VIEW_DOCTAB_OFF:
			pCmdUI->SetRadio(!theApp.GetShowMDITabs());
			break;

		case ID_VIEW_DOCTAB_ICONS:
			pCmdUI->SetCheck(theApp.m_bMDITabIcons);
			pCmdUI->Enable(theApp.GetShowMDITabs());
			break;

		case ID_VIEW_DOCTAB_NOTE:
			pCmdUI->SetCheck(theApp.m_nMDITabStyle == CMFCTabCtrl::STYLE_3D_ONENOTE);
			pCmdUI->Enable(theApp.GetShowMDITabs());
			break;

		default:
			ASSERT(false);
			break;
	}
}

void CMainFrame::RebuildToolsMenu()
{
	// we only need to do this, when we do not use the BCG menu bar
	if (!CConfiguration::GetInstance()->m_bOptimizeMenuForVisuallyHandicappedUsers)
		return;

	//
	// Get the name of the tools menu from the reference menu
	//
	// (if MDI-childs are maximized, then the menu of our main frame
	// also contains items for the MDI childs system menu and the
	// minimize, maximize and close buttons for the MDI child. So we
	// cannot determine the tools submenu by index, because the index
	// may change from time to time depending on if the MDI childs are
	// maximized or not).
	const int nToolsMenuIndexOffset = 3;
	CMenu mnuRef;
	VERIFY(mnuRef.LoadMenu(IDR_MAINFRAME));

	CString strToolsMenuTitle;
	mnuRef.GetMenuString(mnuRef.GetMenuItemCount() - nToolsMenuIndexOffset, strToolsMenuTitle,
		MF_BYPOSITION);

	//
	// Get the tools menu from the current menu
	//
	int nToolsMenuIndex = -1;

	for (int nItem = 0; nItem < m_stdMenu.GetMenuItemCount(); ++nItem)
	{
		CString strItem;
		m_stdMenu.GetMenuString(nItem, strItem, MF_BYPOSITION);
		if (strItem == strToolsMenuTitle)
		{
			nToolsMenuIndex = nItem;
			break;
		}
	}
	if (nToolsMenuIndex < 0)
	{
		ASSERT( FALSE);
		return;
	}

	CMenu *pToolsMenu = m_stdMenu.GetSubMenu(nToolsMenuIndex);
	if (!IsMenu(pToolsMenu->GetSafeHmenu()))
	{
		ASSERT( FALSE);
		return;
	}

	// Remove all current tool related entries

	int nLastEntryIndex = pToolsMenu->GetMenuItemCount() - 1;

	while (pToolsMenu->GetMenuItemID(nLastEntryIndex) == ID_TOOLS_ENTRY
			|| (pToolsMenu->GetMenuItemID(nLastEntryIndex) >= ID_USER_TOOL_FIRST
					&& pToolsMenu->GetMenuItemID(nLastEntryIndex) <= ID_USER_TOOL_LAST))
	{
		pToolsMenu->DeleteMenu(nLastEntryIndex, MF_BYPOSITION);
		nLastEntryIndex = pToolsMenu->GetMenuItemCount() - 1;
	}

	// add new tool entries
	const CObList &toolsList = theApp.GetUserToolsManager()->GetUserTools();
	POSITION pos = toolsList.GetHeadPosition();

	while (pos)
	{
		CUserTool *pTool = (CUserTool*)toolsList.GetNext(pos);
		pToolsMenu->AppendMenu(MF_STRING, pTool->GetCommandId(), pTool->m_strLabel);
	}
}

int CMainFrame::GetMDIChilds(CArray<CChildFrame*, CChildFrame*>& MDIChildArray, bool bSortByTabs /*= true*/)
{
	int ActiveChild = -1;

	//Clean up
	MDIChildArray.RemoveAll();

	if (bSortByTabs)
	{
		/* We run over all tabs and collect the child frames.
			If we enabled MDI Tabbed Groups in DisplayDocumentTabs(),
			then we need to visit all their MDI Tab Windows and collect their tabs.
			In this case, there are no tabs in the tab window returned by GetMDITabs().

			If we enabled simple MDI Tabs in DisplayDocumentTabs(),
			then we look for tabs in the tab window returned by GetMDITabs().
			However, that code path is off in DisplayDocumentTabs().

			If tabs are completely off, we find our children the classic way. Not sure, if that is properly sorted, though.

			Sorting works well for everything with tabs.
		*/
		if (AreMDITabs()) //True, if any tabs are enabled (MDI tabs, or MDI tabbed groups)
		{
			if (IsMDITabbedGroup())
			{
				//MDI Tabbed Groups
				for(POSITION pos = m_wndClientArea.GetMDITabGroups().GetHeadPosition();pos != NULL;)
				{
					CMFCTabCtrl* pTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_wndClientArea.GetMDITabGroups().GetNext(pos));
					if (pTabWnd)
					{
						for(int i=0;i<pTabWnd->GetTabsNum();i++)
						{
							CChildFrame* pMDIChildFrame = dynamic_cast< CChildFrame* >(pTabWnd->GetTabWnd(i));
							if (pMDIChildFrame)
							{
								MDIChildArray.Add(pMDIChildFrame);
								if (GetActiveFrame() == pMDIChildFrame) ActiveChild = i;
							}
						}
					}
				}
			}
			else
			{
				//MDI Tabs ==> that code path is off in DisplayDocumentTabs().
				CMFCTabCtrl* pTabWnd = &GetMDITabs();
				if (pTabWnd)
				{
					for(int i=0;i<pTabWnd->GetTabsNum();i++)
					{
						CChildFrame* pMDIChildFrame = dynamic_cast< CChildFrame* >(pTabWnd->GetTabWnd(i));
						if (pMDIChildFrame)
						{
							MDIChildArray.Add(pMDIChildFrame);
							if (GetActiveFrame() == pMDIChildFrame) ActiveChild = i;
						}
					}
				}
			}
		}
		else
		{
			//No tabs, classic code. Probably the order in which we opened them, or the MRU order. Who knows.
			HWND hwndMDIChild = ::GetWindow(m_hWndMDIClient, GW_CHILD);

			for(int i(0);hwndMDIChild;i++)
			{
				CChildFrame* pMDIChildFrame = DYNAMIC_DOWNCAST(CChildFrame, CWnd::FromHandle(hwndMDIChild));
				if (pMDIChildFrame)
				{
					MDIChildArray.Add(pMDIChildFrame);
					if (GetActiveFrame() == pMDIChildFrame) ActiveChild = i;
				}

				hwndMDIChild = ::GetWindow(hwndMDIChild, GW_HWNDNEXT);
			}
		}
	}
	else
	{
		for (auto it=MDIChildrenMRU.begin(); it!=MDIChildrenMRU.end(); it++)
		{
			CChildFrame* pMDIChildFrame = dynamic_cast< CChildFrame* >(*it);
			MDIChildArray.Add(pMDIChildFrame);
		}

		if (!MDIChildArray.IsEmpty()) ActiveChild = 0;
	}

	return ActiveChild;
}

void CMainFrame::OnViewFullScreen()
{
	ShowFullScreen();
}

void CMainFrame::OnUpdateViewFullScreen(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(IsFullScreen());
}

void CMainFrame::OnHelpKeyMapping()
{
	CMFCKeyMapDialog dlg(this, TRUE);
	dlg.DoModal();
}

void CMainFrame::OnContextMenu(CWnd* pWnd, CPoint point)
{
	m_pContextMenuTargetWindow = NULL; //Invalidate. So we do not close the wrong file.

	// We have to use the right window to get right coordinates
	::ScreenToClient(pWnd->GetSafeHwnd(), &point);

	//Show two different popups: (a) for the tabs, (b) for the tool bars
	if (pWnd->m_hWnd == m_hWndMDIClient || pWnd->m_hWnd == m_wndClientArea.GetMDITabs())
	{
		//Show popup for the tabs
		//But first: determine target window for context menu; will be needed to close the right file
		const int nTab = m_wndClientArea.GetMDITabs().GetTabFromPoint(point);

		if (nTab >= 0)
		{
			CChildFrame* wndEdit =
					dynamic_cast<CChildFrame*> (m_wndClientArea.GetMDITabs().GetTabWnd(nTab));
			// save pointer for later use
			if (wndEdit)
			{
				m_pContextMenuTargetWindow = dynamic_cast<CView*> (wndEdit->GetActiveView());
			}
		}

		// translate menu coordinates and show menu
		::ClientToScreen(pWnd->GetSafeHwnd(), &point);
		theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_MDICLIENT, point.x, point.y, this);
	}
	else
	{
		CRect client;
		GetClientRect(&client);

		if (client.PtInRect(point))
		{
			//Show popup for the tool bars: This is never shown, the system has its own one (Tino, 7/8/2013).
			theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_TOOLBAR, point.x, point.y, this);
		}
		else
		{
			// Show caption bar menu
			CFrameWnd::OnContextMenu(pWnd, point);
		}
	}
}

BOOL CMainFrame::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	return TRUE; //CMDIFrameWndEx::OnHelpInfo(pHelpInfo);
}

LRESULT CMainFrame::OnCustomizationMode(WPARAM wParam, LPARAM /*lParam*/)
{
	if (!wParam)
	{
		// customization mode is about to be deactivated
		RebuildToolsMenu();
	}

	return 0L;
}

void CMainFrame::OnUpdateUserTool(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
}

void CMainFrame::OnExecuteUserTool(UINT nIDEvent)
{
	theApp.GetUserToolsManager()->InvokeTool(nIDEvent);
}

BOOL CMainFrame::OnToolsCancel(UINT)
{
	//Dependent on the current mode, we do some fancy stuff here.
	// Current edit view does NOT have focus: It gets it.
	// old: Current edit view is in incremental search mode: Cancel search.
	// Current edit view does have focus: Close tool windows at bottom and the build output window.
	// In any case: close the instant advice tip.

	//Get active frame
	CFrameWnd* pFrame = GetActiveFrame();

	if (!pFrame)
		return FALSE;

	//Get active view/edit
	CView* pView = pFrame->GetActiveView();
	if (!pView) return false;

	//Close instant advice tip
	if (dynamic_cast<LaTeXView*>(pView))
	{
		dynamic_cast<LaTeXView*>(pView)->HideAdvice();
	}

	CodeView* pEdit = dynamic_cast<CodeView*>(pView);
	if (!pEdit) return false;

	//Focus to the editor
	if (GetFocus() != &pEdit->GetCtrl())
	{
		//Activate view
		pEdit->SetFocus();
		return true;
	}

	//Close the bottom tool windows to get more space for the editor.
	if (CConfiguration::GetInstance()->m_bCloseToolWindowsOnEscape)
	{
		//Close the bottom tool windows
		bool bClosedSomething = ToggleDockingBars(CBRS_ALIGN_BOTTOM, true);
		//...and we make sure that the build output is closed as well, since we open it with every compilation.
		bClosedSomething = bClosedSomething || build_view_pane_.get()->IsVisible();
		ShowPaneEnsureVisibility(build_view_pane_.get(), false, false, false);

		if (bClosedSomething) return true;
	}

	//Abort selections
	const long CurrentPos = pEdit->GetCtrl().GetCurrentPos();
	pEdit->GetCtrl().ClearSelections();
	pEdit->GetCtrl().SetAnchor(CurrentPos);
	pEdit->GetCtrl().SetCurrentPos(CurrentPos);

	return true;
}

void CMainFrame::OnWindowCloseSelectedTab()
{
	//Close the tab where the user pressed the right mouse button, i.e., where the context menu was/is displayed for
	if (m_pContextMenuTargetWindow && m_pContextMenuTargetWindow->GetDocument())
	{
		if (m_pContextMenuTargetWindow->GetDocument()->SaveModified())
		{
			m_pContextMenuTargetWindow->GetDocument()->OnCloseDocument();
		}
	}

	m_pContextMenuTargetWindow = NULL;
}

void CMainFrame::OnUpdateWindowCloseSelectedTab(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pContextMenuTargetWindow != NULL);
}

void CMainFrame::OnWindowCloseAllButActive()
{
	CArray<CChildFrame*, CChildFrame*> MDIChildArray;
	const int nActiveFrame = GetMDIChilds(MDIChildArray);

	for (int i = 0; i < MDIChildArray.GetSize(); i++)
	{
		if (i != nActiveFrame)
		{
			CView* pView = dynamic_cast<CView*> (MDIChildArray.GetAt(i)->GetActiveView());

			if (pView && pView->GetDocument() && pView->GetDocument()->SaveModified())
			{
				pView->GetDocument()->OnCloseDocument();
			}
		}
	}
}

void CMainFrame::OnUpdateWindowCloseAllButActive(CCmdUI* pCmdUI)
{
	CArray<CChildFrame*, CChildFrame*> MDIChildArray;
	GetMDIChilds(MDIChildArray);
	pCmdUI->Enable(MDIChildArray.GetSize() > 1);
}


CString GetCaption(UINT nID)
{
	CString FullString = AfxLoadString(nID);
	return FullString.Right(FullString.GetLength() - FullString.ReverseFind(_T('\n')) - 1);;
}

bool CMainFrame::CreateToolWindows()
{
	const DWORD pane_style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
			| CBRS_FLOAT_MULTI | CBRS_HIDE_INPLACE;

	const CSize size(350, 350);
	const CSize bottom_pane_size(350, 350);

	//Left windows
	structure_view_->Create(GetCaption(ID_VIEW_STRUCT_PANE), this,
		CRect(CPoint(0, 0), size), TRUE, ID_VIEW_STRUCT_PANE, pane_style | CBRS_LEFT);
	env_view_pane_->Create(GetCaption(ID_VIEW_ENV_PANE), this,
		CRect(CPoint(0, 0), size), TRUE, ID_VIEW_ENV_PANE, pane_style | CBRS_LEFT);
	file_view_pane_->Create(GetCaption(ID_VIEW_FILES_PANE), this,
		CRect(CPoint(0, 0), size), TRUE, ID_VIEW_FILES_PANE, pane_style | CBRS_LEFT);
	//Right windows
	bookmark_view_pane_->Create(GetCaption(ID_VIEW_BOOKMARKS_PANE), this,
		CRect(CPoint(0, 0), size), TRUE, ID_VIEW_BOOKMARKS_PANE, pane_style | CBRS_RIGHT);
	preview_image_view_pane_->Create(GetCaption(ID_VIEW_PREVIEW_IMAGE_PANE), this,
		CRect(CPoint(0, 0), size), TRUE, ID_VIEW_PREVIEW_IMAGE_PANE, pane_style | CBRS_RIGHT);
	//Bottom windows
	error_list_view_->Create(GetCaption(ID_VIEW_ERROR_LIST_PANE), this,
		CRect(CPoint(0, 0), bottom_pane_size), TRUE, ID_VIEW_ERROR_LIST_PANE, pane_style | CBRS_BOTTOM);
	build_view_pane_->Create(GetCaption(ID_VIEW_BUILD_PANE), this,
		CRect(CPoint(0, 0), bottom_pane_size), TRUE, ID_VIEW_BUILD_PANE, pane_style | CBRS_BOTTOM);
	preview_output_view_pane_->Create(GetCaption(ID_VIEW_PREVIEW_OUTPUT_PANE), this,
		CRect(CPoint(0, 0), bottom_pane_size), TRUE, ID_VIEW_PREVIEW_OUTPUT_PANE, pane_style | CBRS_BOTTOM);
	grep_view_1_pane_->Create(GetCaption(ID_VIEW_GREP_1_PANE), this,
		CRect(CPoint(0, 0), bottom_pane_size), TRUE, ID_VIEW_GREP_1_PANE, pane_style | CBRS_BOTTOM);
	grep_view_2_pane_->Create(GetCaption(ID_VIEW_GREP_2_PANE), this,
		CRect(CPoint(0, 0), bottom_pane_size), TRUE, ID_VIEW_GREP_2_PANE, pane_style | CBRS_BOTTOM);
	parse_view_pane_->Create(GetCaption(ID_VIEW_PARSE_PANE), this,
		CRect(CPoint(0, 0), bottom_pane_size), TRUE, ID_VIEW_PARSE_PANE, pane_style | CBRS_BOTTOM);
	//Auto-Hide windows
	bib_view_pane_->Create(GetCaption(ID_VIEW_BIB_ENTRIES_PANE), this,
		CRect(CPoint(250, 100), CPoint(800, 500)), TRUE, ID_VIEW_BIB_ENTRIES_PANE, pane_style | CBRS_BOTTOM);

	//Create views
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!env_view_->Create(env_view_pane_.get()))
	{
		TRACE0("Failed to create environment view\n");
		return false;
	}

	if (!build_view_->Create(rectDummy, this))
	{
		TRACE0("Failed to create build output view\n");
		return false;
	}

	if (!preview_output_view_->Create(rectDummy, this))
	{
		TRACE0("Failed to create preview output view\n");
		return false;
	}

	if (!preview_image_view_->Create(rectDummy, this))
	{
		TRACE0("Failed to create preview image view\n");
		return false;
	}

	if (!grep_view_1_->Create(rectDummy, this))
	{
		TRACE0("Failed to create find 1 output view\n");
		return false;
	}
	
	grep_view_1_->SetIndex(0);

	if (!grep_view_2_->Create(rectDummy, this))
	{
		TRACE0("Failed to create find 2 output view\n");
		return false;
	}

	grep_view_2_->SetIndex(1);

	if (!parse_view_->Create(rectDummy, this))
	{
		TRACE0("Failed to create parse output view\n");
		return false;
	}
	// - and set them
	env_view_pane_->SetClient(env_view_.get());
	output_doc_->SetAllViews(build_view_.get(), grep_view_1_.get(), 
							 grep_view_2_.get(), parse_view_.get(),
							 preview_output_view_.get(), preview_image_view_.get());
	build_view_->AttachDoc(output_doc_.get());
	preview_output_view_->AttachDoc(output_doc_.get());
	grep_view_1_->AttachDoc(output_doc_.get());
	grep_view_2_->AttachDoc(output_doc_.get());
	parse_view_->AttachDoc(output_doc_.get());
	error_list_view_->AttachDoc(output_doc_.get());
	output_doc_->SetErrorListView(error_list_view_.get());

	//Set clients
	build_view_pane_->SetClient(build_view_.get());
	preview_output_view_pane_->SetClient(preview_output_view_.get());
	preview_image_view_pane_->SetClient(preview_image_view_.get());
	grep_view_1_pane_->SetClient(grep_view_1_.get());
	grep_view_2_pane_->SetClient(grep_view_2_.get());
	parse_view_pane_->SetClient(parse_view_.get());

	//Set toolbars
	//preview_image_view_pane_->SetToolBar(&m_wndFindBar);
	//PrivateToolBar* pPTool = new PrivateToolBar();
	//preview_image_view_pane_->CreateToolBar(pPTool->GetRuntimeClass(), IDR_FIND);

	//Enable Docking
	structure_view_->EnableDocking(CBRS_ALIGN_ANY);
	env_view_pane_->EnableDocking(CBRS_ALIGN_ANY);
	file_view_pane_->EnableDocking(CBRS_ALIGN_ANY);
	bib_view_pane_->EnableDocking(CBRS_ALIGN_ANY);
	bookmark_view_pane_->EnableDocking(CBRS_ALIGN_ANY);
	error_list_view_->EnableDocking(CBRS_ALIGN_ANY);
	build_view_pane_->EnableDocking(CBRS_ALIGN_ANY);
	preview_output_view_pane_->EnableDocking(CBRS_ALIGN_ANY);
	preview_image_view_pane_->EnableDocking(CBRS_ALIGN_ANY);
	grep_view_1_pane_->EnableDocking(CBRS_ALIGN_ANY);
	grep_view_2_pane_->EnableDocking(CBRS_ALIGN_ANY);
	parse_view_pane_->EnableDocking(CBRS_ALIGN_ANY);

	//Dock the major panes
	DockPane(structure_view_.get());
	DockPane(build_view_pane_.get());
	DockPane(bookmark_view_pane_.get());
	DockPane(bib_view_pane_.get());
	bib_view_pane_->SetAutoHideMode(TRUE, CBRS_BOTTOM, 0, FALSE);

	//Adjust the size of the Literature pane
	CRect BibRect;
	bib_view_pane_->GetWindowRect(&BibRect);
	if (BibRect.Height() < bottom_pane_size.cy)
	{
		BibRect.top = BibRect.bottom - bottom_pane_size.cy;
		bib_view_pane_->SetWindowPos(0, 0, 0, BibRect.Width(), BibRect.Height(), SWP_NOZORDER | SWP_NOMOVE
				| SWP_NOACTIVATE);
	}

	//Add the other panes to the major ones
	CDockablePane* pDockablePane = NULL;

	//Structure view and its other windows
	env_view_pane_->AttachToTabWnd(structure_view_.get(), DM_STANDARD, TRUE, &pDockablePane);
	file_view_pane_->AttachToTabWnd(structure_view_.get(), DM_STANDARD, FALSE);

	//Bookmark view and its other windows
	pDockablePane = NULL;
	preview_image_view_pane_->AttachToTabWnd(bookmark_view_pane_.get(), DM_STANDARD, TRUE, &pDockablePane);

	//Set Images
	HIMAGELIST himl = ::ImageList_LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(
		IDB_NAVIGATION_BAR), 16, 1, RGB(192, 192, 192), IMAGE_BITMAP, LR_CREATEDIBSECTION);
	structure_view_->SetIcon(::ImageList_ExtractIcon(0, himl, 0), FALSE);
	env_view_pane_->SetIcon(::ImageList_ExtractIcon(0, himl, 1), FALSE);
	file_view_pane_->SetIcon(::ImageList_ExtractIcon(0, himl, 2), FALSE);
	bib_view_pane_->SetIcon(::ImageList_ExtractIcon(0, himl, 3), FALSE);
	bookmark_view_pane_->SetIcon(::ImageList_ExtractIcon(0, himl, 4), FALSE);
	preview_image_view_pane_->SetIcon(::ImageList_ExtractIcon(0, himl, 5), FALSE);
	::ImageList_Destroy(himl);
	
	if (CBaseTabbedPane* pane = dynamic_cast<CBaseTabbedPane*>(pDockablePane))
	{
		pane->GetUnderlyingWindow()->SetActiveTab(0);
	}

	//Build output and its other windows
	pDockablePane = NULL;
	error_list_view_->AttachToTabWnd(build_view_pane_.get(), DM_STANDARD, TRUE, &pDockablePane);
	grep_view_1_pane_->AttachToTabWnd(build_view_pane_.get(), DM_STANDARD, FALSE);
	grep_view_2_pane_->AttachToTabWnd(build_view_pane_.get(), DM_STANDARD, FALSE);
	parse_view_pane_->AttachToTabWnd(build_view_pane_.get(), DM_STANDARD, FALSE);
	preview_output_view_pane_->AttachToTabWnd(build_view_pane_.get(), DM_STANDARD, FALSE);

	//Set images
	himl = ::ImageList_LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(
		IDB_OUTPUT_BAR), 16, 1, RGB(192, 192, 192), IMAGE_BITMAP, LR_CREATEDIBSECTION);
	build_view_pane_->SetIcon(::ImageList_ExtractIcon(0, himl, 0), FALSE);
	preview_output_view_pane_->SetIcon(::ImageList_ExtractIcon(0, himl, 0), FALSE);
	grep_view_1_pane_->SetIcon(::ImageList_ExtractIcon(0, himl, 1), FALSE);
	grep_view_2_pane_->SetIcon(::ImageList_ExtractIcon(0, himl, 2), FALSE);
	parse_view_pane_->SetIcon(::ImageList_ExtractIcon(0, himl, 3), FALSE);
	error_list_view_->SetIcon(::ImageList_ExtractIcon(0, himl, 4), FALSE);
	::ImageList_Destroy(himl);

	if (CBaseTabbedPane* pane = dynamic_cast<CBaseTabbedPane*>(pDockablePane))
	{
		pane->GetUnderlyingWindow()->SetActiveTab(0);
	}

	//Adjust the size of the bottom panes
	//The size parameter of the CDockablePane::Create function doesn't seem to be honored.
	CRect rect;
	pDockablePane->GetWindowRect(&rect);

	if (rect.Height() < bottom_pane_size.cy)
	{
		rect.top = rect.bottom - size.cy;
		pDockablePane->SetWindowPos(0, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	//Do not show all of them at first startup
	ShowPaneEnsureVisibility(bookmark_view_pane_.get(), false, false, false);
	ShowPaneEnsureVisibility(build_view_pane_.get(), false, false, false);
	ShowPaneEnsureVisibility(preview_output_view_pane_.get(), false, false, false);
	ShowPaneEnsureVisibility(preview_image_view_pane_.get(), false, false, false);
	ShowPaneEnsureVisibility(error_list_view_.get(), false, false, false);
	ShowPaneEnsureVisibility(grep_view_1_pane_.get(), false, false, false);
	ShowPaneEnsureVisibility(grep_view_2_pane_.get(), false, false, false);
	ShowPaneEnsureVisibility(parse_view_pane_.get(), false, false, false);

	return true;
}

const std::vector<CProjectView*> CMainFrame::GetViews()
{
	std::vector<CProjectView*> views;

	views.push_back(structure_view_->GetProjectView());
	views.push_back(file_view_pane_->GetProjectView());
	views.push_back(env_view_.get());
	views.push_back(bib_view_pane_.get());
	views.push_back(bookmark_view_pane_.get());

	return views;
}

void CMainFrame::OnOpenProject(CLaTeXProject* p)
{
	const std::vector<CProjectView*>& views = GetViews();

	using namespace std::placeholders;
	std::for_each(views.begin(), views.end(), std::bind(&CLaTeXProject::AddView, p, _1));
}

void CMainFrame::OnCloseProject(CLaTeXProject* p)
{
	HideTaskbarProgress();

	const std::vector<CProjectView*>& views = GetViews();

	using namespace std::placeholders;
	std::for_each(views.begin(), views.end(), std::bind(&CLaTeXProject::RemoveView, p, _1));

	output_doc_->ClearMessages(); // Clear all the warnings, errors etc.
}

COutputDoc* CMainFrame::GetOutputDoc(void)
{
	return output_doc_.get();
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.SetApplicationLook(id);

	switch (theApp.GetApplicationLook())
	{
		case ID_VIEW_APP_LOOK_WIN_2000:
			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS(CMFCVisualManager));
			break;
		case ID_VIEW_APP_LOOK_OFFICE_XP:
			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
			break;
		case ID_VIEW_APP_LOOK_WIN_XP:
			CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS(CMFCVisualManagerWindows));
			break;
		case ID_VIEW_APP_LOOK_OFFICE_2003:
			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS(CMFCVisualManagerOffice2003));
			CDockingManager::SetDockingMode( DT_SMART);
			break;
		case ID_VIEW_APP_LOOK_VS_2005:
			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS(CMFCVisualManagerVS2005));
			CDockingManager::SetDockingMode(DT_SMART);
			break;
		case ID_VIEW_APP_LOOK_WIN7:
			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS(CMFCVisualManagerWindows7));
			CDockingManager::SetDockingMode(DT_SMART);
			break;
		default:
			switch (theApp.GetApplicationLook())
			{
				case ID_VIEW_APP_LOOK_OFFICE_2007_BLUE:
					CMFCVisualManagerOffice2007::SetStyle(
						CMFCVisualManagerOffice2007::Office2007_LunaBlue);
					break;
				case ID_VIEW_APP_LOOK_OFFICE_2007_BLACK:
					CMFCVisualManagerOffice2007::SetStyle(
						CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
					break;
				case ID_VIEW_APP_LOOK_OFFICE_2007_SILVER:
					CMFCVisualManagerOffice2007::SetStyle(
						CMFCVisualManagerOffice2007::Office2007_Silver);
					break;
				case ID_VIEW_APP_LOOK_OFFICE_2007_AQUA:
					CMFCVisualManagerOffice2007::SetStyle(
						CMFCVisualManagerOffice2007::Office2007_Aqua);
					break;
			}

			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS(CMFCVisualManagerOffice2007));
			CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME
			| RDW_ERASE);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	if (pCmdUI->m_nID == ID_VIEW_APP_LOOK_WIN_XP)
		pCmdUI->Enable();

	pCmdUI->SetRadio(theApp.GetApplicationLook() == pCmdUI->m_nID);
}

void CMainFrame::OnStartPaneAnimation()
{
	animating_ = true;
	// Start the animation
	m_wndStatusBar.SetPaneAnimation(BuildAnimationPane, build_animation_, 250);

	if (taskBarList_)
	{
		// Show a marquee progress in the taskbar if available
		taskBarList_->SetProgressState(m_hWnd, TBPF_INDETERMINATE);
	}
}

LRESULT CMainFrame::OnStopPaneAnimation(WPARAM, LPARAM)
{
	// Stop the animation
	m_wndStatusBar.SetPaneAnimation(BuildAnimationPane, 0);

	// Release the taskbar
	if (taskBarList_)
	{
		taskBarList_->SetProgressValue(m_hWnd, 100, 100); // 100% complete
		taskBarList_->SetProgressState(m_hWnd, output_doc_->HasBuildErrors() ? TBPF_ERROR
				: TBPF_NORMAL);
	}

	animating_ = false;

	return 0;
}

void CMainFrame::OnIdle(long /*count*/)
{
#if 0
	if (!animating_)
	{
		// Remove the taskbar progress when idle
		HideTaskbarProgress();
	}
#endif
}

void CMainFrame::HideTaskbarProgress()
{
	if (taskBarList_)
	{
		taskBarList_->SetProgressState(m_hWnd, TBPF_NOPROGRESS);
	}
}

LRESULT CMainFrame::OnGetTabToolTip(WPARAM, LPARAM l)
{
	CMFCTabToolTipInfo* ti = reinterpret_cast<CMFCTabToolTipInfo*> (l);
	ASSERT_POINTER(ti, CMFCTabToolTipInfo);

	CChildFrame* f = dynamic_cast<CChildFrame*> (ti->m_pTabWnd->GetTabWnd(ti->m_nTabIndex));
	if (f)
	{
		ASSERT_KINDOF(CChildFrame, f);
		ti->m_strText = f->GetPathNameOfDocument();
	}

	return 0;
}

void CMainFrame::OnViewTransparency()
{
	TransparencyDlg dlg(this);
	dlg.DoModal();
}

void CMainFrame::OnUpdateFrameTitle(BOOL /*bAddToTitle*/)
{
	//We use our own version to get consistent results regarding projects and single files.
	UpdateFrameTitle();
	//CMDIFrameWndEx::OnUpdateFrameTitle(bAddToTitle);
}

void CMainFrame::UpdateFrameTitle()
{
	CString title(MAKEINTRESOURCE(IDR_MAINFRAME));

	CLaTeXProject* project = theApp.GetProject();
	if (project)
	{
		//Add the project title: "Project - TeXnicCenter"
		const CString& tmp = project->GetTitle();
		if (!tmp.IsEmpty()) title = tmp + _T(" - ") + title;
	}
	else
	{
		//Add the file name: "File - TeXnicCenter"
		CFrameWnd *pFrame = GetActiveFrame();
		if (pFrame)
		{
			//Get active document
			CDocument* pDoc = pFrame->GetActiveDocument();
			if (pDoc)
			{
				const CString& tmp = CPathTool::GetFile(pDoc->GetPathName());
				if (!tmp.IsEmpty()) title = tmp + _T(" - ") + title;
			}
		}
	}

	//Same as before? If not, set it.
	CString OldTitle;
	GetWindowText(OldTitle);
	if (title != OldTitle)
	{
		SetWindowText(title);
	}
}

void CMainFrame::OnDestroy()
{
	CMDIFrameWndEx::OnDestroy();

	if (taskBarList_)
		taskBarList_.Release();
}

void CMainFrame::CheckForFileChangesAsync()
{
	PostMessage(AfxUserMessages::CheckForFileChangesMessageID);
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	BOOL result = FALSE;

	if (pMsg->message == WM_MBUTTONUP && AreMDITabs())
	{
		CWnd* pWnd = FromHandle(pMsg->hwnd);
		CMFCTabCtrl* tabGroup = dynamic_cast<CMFCTabCtrl*> (pWnd);

		if (tabGroup)
		{
			//clicked middle button on a tab group.
			//was it on a tab?
			CPoint point = pMsg->pt;
			tabGroup->ScreenToClient(&point);

			int tabIndex = tabGroup->GetTabFromPoint(point);

			if (tabIndex != -1)
			{
				//clicked middle button on a tab.
				//send a WM_CLOSE message to it
				CWnd* pTab = tabGroup->GetTabWnd(tabIndex);

				if (pTab && pTab->IsKindOf(RUNTIME_CLASS(CChildFrame)))
				{
					pTab->SendMessage(WM_CLOSE);
					result = TRUE;
				}
			}
		}
	}

	if (!result)
		result = CMDIFrameWndEx::PreTranslateMessage(pMsg);

	return result;
}


bool CMainFrame::ToggleDockingBars(const DWORD dwAlignment, const bool bCloseOnly /*= false*/)
{
	std::vector< CBasePane* > pAllPanes;
	GetAllPanes(pAllPanes, true, true);

	//If any of them is visible, we hide them all
	//Otherwise, we show them
	bool bOneIsVisible(false);
	for(unsigned int i=0;i<pAllPanes.size();i++)
	{
		if (!pAllPanes[i]->m_hWnd) continue;

		if (pAllPanes[i]->GetCurrentAlignment() == dwAlignment
			&& pAllPanes[i]->IsVisible()
			&& !pAllPanes[i]->IsAutoHideMode()
			&& !pAllPanes[i]->IsMDITabbed())
		{
			bOneIsVisible = true;
			break;
		}
	}

	for(unsigned int i=0;i<pAllPanes.size();i++)
	{
		if (!pAllPanes[i]->m_hWnd) continue;

		if (pAllPanes[i]->GetCurrentAlignment() == dwAlignment
			&& !pAllPanes[i]->IsAutoHideMode()
			&& !pAllPanes[i]->IsMDITabbed())
		{
			ShowPaneEnsureVisibility(pAllPanes[i], !bOneIsVisible && !bCloseOnly, false, !bOneIsVisible && !bCloseOnly);
		}
	}

	return bOneIsVisible;
}


void CMainFrame::OnViewToggleBottomDockingBars()
{
	ToggleDockingBars(CBRS_ALIGN_BOTTOM);
}

void CMainFrame::OnViewToggleLeftDockingBars()
{
	ToggleDockingBars(CBRS_ALIGN_LEFT);
}

void CMainFrame::OnViewToggleRightDockingBars()
{
	ToggleDockingBars(CBRS_ALIGN_RIGHT);
}

void CMainFrame::OnViewToggleTopDockingBars()
{
	ToggleDockingBars(CBRS_ALIGN_TOP);
}


#pragma region MRU cycling of MDI windows

void CMainFrame::RegisterChildFrame(CFrameWnd* frame)
{
	ENSURE_ARG(frame);

	//Add a new MDI Child to the front of the MRU list of MDI children
	MDIChildrenMRU.push_front(frame);
}

void CMainFrame::UnregisterChildFrame(CFrameWnd* frame)
{
	ENSURE_ARG(frame);

	//Remove a MDI Child from the MRU list of MDI children
	for (auto it=MDIChildrenMRU.begin(); it!=MDIChildrenMRU.end(); it++)
	{
		if (*it == frame)
		{
			MDIChildrenMRU.erase(it);
			break;
		}
	}
}

void CMainFrame::SetCurrentChildFrameAsRecentlyUsed()
{
	SetRecentlyUsedChildFrame(GetActiveFrame());
}


void CMainFrame::SetRecentlyUsedChildFrame(CFrameWnd* child)
{
	//TRACE(_T("SetRecentlyUsedChildFrame called with child: %s\n"), CPathTool::GetFileTitle(((CChildFrame*)child)->GetPathNameOfDocument()));
	//DbgMRUList("SetRecentlyUsedChildFrame::Start");

	if (!MDIChildrenMRU.empty() && child && MDIChildrenMRU.front() != child)
	{
		auto it = std::find(MDIChildrenMRU.begin(), MDIChildrenMRU.end(), child);

		if (it != MDIChildrenMRU.end())
		{
			// Put the child at the top of the MRU list of MDI children
			MDIChildrenMRU.erase(it);
			MDIChildrenMRU.push_front(child);
		}
	}

	//DbgMRUList("SetRecentlyUsedChildFrame::End");
}


//void CMainFrame::DbgMRUList(const char* Msg)
//{
//	TRACE("%s:", Msg);
//	int idx(0);
//	for (auto it=MDIChildrenMRU.begin(); it!=MDIChildrenMRU.end(); it++, idx++)
//	{
//		CChildFrame* pMDIChildFrame = dynamic_cast< CChildFrame* >(*it);
//		TRACE(_T("  %s"), CPathTool::GetFileTitle(pMDIChildFrame->GetPathNameOfDocument()));
//	}
//	TRACE("\n");
//}

void CMainFrame::ActivateMDIChildByMRUList(const bool bPrevious)
{
	//Current window
	CFrameWnd* pCurrentFrame = GetActiveFrame();
	CFrameWnd* pNextFrame = NULL;

	//Get next or previous window
	if (bPrevious)
	{
		//DbgMRUList("ActivateMDIChildByMRUList::Next");

		//Forward in the list - this is what we're usually doing
		for (auto it=MDIChildrenMRU.begin(); it!=MDIChildrenMRU.end(); it++)
		{
			if (*it == pCurrentFrame)
			{
				it++;
				if (it==MDIChildrenMRU.end())
				{
					pNextFrame = *MDIChildrenMRU.begin();
				}
				else
				{
					pNextFrame = *it;
				}
				break;
			}
		}
	}
	else
	{
		//DbgMRUList("ActivateMDIChildByMRUList::Previous");

		//Backward in the list - on the first run, the next frame is the LEAST recently used one
		for (auto it=MDIChildrenMRU.begin(); it!=MDIChildrenMRU.end(); it++)
		{
			if (*it == pCurrentFrame)
			{
				if (it==MDIChildrenMRU.begin())
				{
					pNextFrame = *MDIChildrenMRU.rbegin();
				}
				else
				{
					//pNextFrame = *(it);
					pNextFrame = *(--it);
				}
				break;
			}
		}
	}

	//Activate!
	if (pNextFrame)
	{
		//TRACE(_T("Activating: %s\n"), CPathTool::GetFileTitle(((CChildFrame*)pNextFrame)->GetPathNameOfDocument()));
		pNextFrame->ActivateFrame();
	}
}

void CMainFrame::OnUpdateWindowNext(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(MDIGetActive() != NULL);
}

void CMainFrame::OnWindowNext()
{
	ActivateMDIChildByMRUList(false);
	//::SendMessage(m_hWndMDIClient, WM_MDINEXT, 0, FALSE);
}

void CMainFrame::OnUpdateWindowPrevious(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(MDIGetActive() != NULL);
}

void CMainFrame::OnWindowPrevious()
{
	ActivateMDIChildByMRUList(true);
	//::SendMessage(m_hWndMDIClient, WM_MDINEXT, 0, TRUE);
}

void CMainFrame::OnUpdateWindowMDICommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!IsMDITabbedGroup());
}

#pragma endregion 
