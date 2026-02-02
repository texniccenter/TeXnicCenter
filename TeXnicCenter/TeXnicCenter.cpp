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

#include <vector>

#include <locale.h>
#include <direct.h>
#include <Propkey.h>
#include <Propvarutil.h>

#include "resource.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "global.h"
#include "Configuration.h"
#include "DocumentNewDialog.h"
#include "ProjectNewDialog.h"
#include "OptionDialog.h"
#include "ActiveProfileDialog.h"
#include "Splash.h"
#include "UserTool.h"
#include "Profile.h"
#include "ProfileDialog.h"
#include "PackageScanProgress.h"
#include "AboutBox.h"
#include "OutputWizard.h"
#include "FontOccManager.h"
#include "LaTeXDocument.h"
#include "LaTeXView.h"
#include "Speller.h"
#include "BibTeXView.h"
#include "BibTeXDocument.h"
#include "MetaPostView.h"
#include "MetaPostDocument.h"
#include "MultiExtensionDocTemplate.h"
#include "SpellerBackgroundThread.h"
#include "SpellerSource.h"
#include "tipdlg.h"
#include "TeXnicCenter.h"


class TeXnicCenterAppSpellerSource
	: public SpellerSource
{
public:
	TeXnicCenterAppSpellerSource(CTeXnicCenterApp* app)
		: app(app)
	{
	}

	Speller* GetSpeller()
	{
		return app->GetSpeller();
	}

private:
	CTeXnicCenterApp* const app;
};

class CTCCommandLineInfo : public CCommandLineInfo
{
	// Construction/destruction
	bool bLastWasDdeCommandFlag;
	bool bLastWasLineFlag;
	bool exit_;
	bool register_shell_types_;

public:
	CTCCommandLineInfo();

	bool Exit() const { return exit_; }
	bool RegisterShell() const { return register_shell_types_; }

	// Overridings
protected:
	virtual void ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast);

	// Attributes
public:
	/**
	Line number, if one was specified with the -line or -l parameter
	or -1, if no line has been specified.
	 */
	int m_nDocumentLine;

	/**
	DDE-command, if one was specified, otherwise an empty string.
	 */
	CString m_strDdeCommand;
};

CTCCommandLineInfo::CTCCommandLineInfo()
		: m_nDocumentLine(-1)
		, bLastWasDdeCommandFlag(false)
		, bLastWasLineFlag(false)
		, exit_(false)
		, register_shell_types_(false)
{
}

void CTCCommandLineInfo::ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast)
{
	CString strParam(lpszParam);

	// handle line number
	if (bLastWasLineFlag)
	{
		if (!bFlag)
			m_nDocumentLine = _ttol(lpszParam);
		else
			TRACE0("Line number expected on command line\n");

		bLastWasLineFlag = false;
	}
	else if (strParam.CompareNoCase(_T("line")) == 0 || strParam.CompareNoCase(_T("l")) == 0)
	{
		bLastWasLineFlag = true;
		return;
	}  // handle request to send dde command
	else if (bLastWasDdeCommandFlag)
	{
		if (!bFlag)
		{
			// store dde-command
			m_strDdeCommand = lpszParam;
			m_strDdeCommand.TrimLeft(_T('"'));
			m_strDdeCommand.TrimRight(_T('"'));
			m_strDdeCommand.Replace(_T('\''), _T('"'));
		}
		else
			TRACE0("DDE-command expected on command line\n");

		bLastWasDdeCommandFlag = false;
	}
	else if (strParam.CompareNoCase(_T("ddecmd")) == 0)
		bLastWasDdeCommandFlag = true;
	else if (strParam.CompareNoCase(_T("nosplash")) == 0)
		m_bShowSplash = FALSE;
	else if (!strParam.CompareNoCase(_T("regs")))
		register_shell_types_ = true;
	else if (!strParam.CompareNoCase(_T("exit")))
		exit_ = true;
	else
		CCommandLineInfo::ParseParam(lpszParam, bFlag, bLast);
}


//-------------------------------------------------------------------
// class CTeXnicCenterApp
//-------------------------------------------------------------------

// the one and only
CTeXnicCenterApp theApp;

BEGIN_MESSAGE_MAP(CTeXnicCenterApp, CProjectSupportingWinApp)
	ON_COMMAND(CG_IDS_TIPOFTHEDAY, ShowTipOfTheDay)
	//{{AFX_MSG_MAP(CTeXnicCenterApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_TIP, OnTip)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_SAVE_ALL, OnFileSaveAll)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, OnDisableStdCmd)
	ON_COMMAND(ID_PROJECT_OPEN, OnProjectOpen)
	ON_UPDATE_COMMAND_UI(ID_PROJECT_CLOSE, OnUpdateProjectClose)
	ON_COMMAND(ID_LATEX_NEW, OnLatexNew)
	ON_COMMAND(ID_EXTRAS_OPTIONS, OnExtrasOptions)
	ON_COMMAND(ID_LATEX_PROFILE, OnLaTeXProfile)
	ON_COMMAND(ID_DOCUMENT_SAVED, OnDocumentSaved)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE1, OnUpdateFileMRU)
	ON_COMMAND(ID_LATEX_PROFILE_SEL, OnLatexProfileSel)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_APP_LICENSE, OnAppLicense)
	ON_COMMAND(ID_PROJECT_SAVE, OnProjectSave)
	ON_UPDATE_COMMAND_UI(ID_PROJECT_SAVE, OnUpdateProjectSave)
	ON_COMMAND(ID_LATEX_EDITPROFILES, OnLatexEditProfiles)
	ON_COMMAND(ID_WINDOW_CLOSE_ALL, OnWindowCloseAll)
	ON_COMMAND(ID_HELP, OnHelp)
	ON_COMMAND(ID_BG_UPDATE_PROJECT, OnUpdateProject)
	ON_COMMAND(ID_PROJECT_NEW_FROM_FILE, OnProjectNewFromFile)
	ON_UPDATE_COMMAND_UI(ID_PROJECT_NEW_FROM_FILE, OnUpdateProjectNewFromFile)
	ON_COMMAND(ID_PACKAGE_SETUP, OnPackageSetup)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_ALL, OnUpdateDoForAllOpenWindows)
	//ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnDisableStdCmd)
	//ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnDisableStdCmd)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_CLOSE_ALL, OnUpdateDoForAllOpenWindows)
	ON_COMMAND(ID_BIBTEX_NEW, &CTeXnicCenterApp::OnBibTeXNew)
	ON_COMMAND(ID_METAPOST_NEW, &CTeXnicCenterApp::OnMetaPostNew)
	//}}AFX_MSG_MAP
	// Dateibasierte Standard-Dokumentbefehle
	//ON_COMMAND(ID_FILE_NEW, CProjectSupportingWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CProjectSupportingWinApp::OnFileOpen)
	// Standard-Druckbefehl "Seite einrichten"
	ON_COMMAND(ID_FILE_PRINT_SETUP, CProjectSupportingWinApp::OnFilePrintSetup)
	// handle most recently used projects
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_PROJECT_FIRST, OnUpdateFileMRUProjectList)
	ON_COMMAND_RANGE(ID_FILE_MRU_PROJECT_FIRST, ID_FILE_MRU_PROJECT_LAST, OnFileMRUProject)
	// handle most recently used files
	//	ON_COMMAND_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE10, OnFileMRUFile)
	ON_UPDATE_COMMAND_UI_RANGE(ID_URL_AUTHOR, ID_URL_TCUSERSGROUP, &CTeXnicCenterApp::OnUpdateURL)
	ON_COMMAND_RANGE(ID_URL_AUTHOR, ID_URL_TCUSERSGROUP, &CTeXnicCenterApp::OnURL)
END_MESSAGE_MAP()

const int MaxRecentProjects = 10;
const int MaxRecentDocuments = MaxRecentProjects;
const int RecentFileListMaxDisplayLength = 75;

/////////////////////////////////////////////////////////////////////////////
// CTeXnicCenterApp Konstruktion

CTeXnicCenterApp::CTeXnicCenterApp()
	:m_bEndSession(FALSE)
	,m_bSavingAll(FALSE)
	,m_recentProjectList(1, _T("Recent Project List"), _T("Project%d"), MaxRecentProjects,RecentFileListMaxDisplayLength)
	,bibtex_doc_template_(0)
	,metapost_doc_template_(0)
	,m_nApplicationLook(ID_VIEW_APP_LOOK_VS_2005)
	,m_bAltKeyDownWithMouseButton(false)
{
	m_eHelpType = afxHTMLHelp;

	//Disable the display of thumbnails in the Windows 7 taskbar
	// for every MDI child frame. The thumbnail for the whole app is still rendered.
	EnableTaskbarInteraction(false);

	spellerSource_.reset(new TeXnicCenterAppSpellerSource(this));
}

bool CTeXnicCenterApp::CanUseRecentFiles()
{
	BOOL bNoRecentDocs = FALSE;
	GetSysPolicyValue(_AFX_SYSPOLICY_NORECENTDOCHISTORY, &bNoRecentDocs);

	return !bNoRecentDocs;
}

BOOL CALLBACK TxcEnumResourceLanguages(HANDLE /*hModule*/, LPCTSTR lpszType, LPCTSTR /*lpszName*/, WORD wIdLang, LONG_PTR lParam)
{
	ASSERT(lpszType == RT_VERSION);
	UNUSED_ALWAYS(lpszType);

	*((WORD*)lParam) = wIdLang;

	// we are only interested in the first language, so return FALSE here
	return FALSE;
}

const CString CTeXnicCenterApp::GetDDEServerName() const
{
	CString strShortName;
	AfxGetModuleShortFileName(AfxGetInstanceHandle(), strShortName);
	return CPathTool::GetFileTitle(strShortName);
}

BOOL CTeXnicCenterApp::InitInstance()
{
	if (!Scintilla_RegisterClasses(AfxGetInstanceHandle()))
	{
		AfxMessageBox(_T("SciLexer could not be registered."), MB_ICONERROR);
		return FALSE;
	}

	setlocale(LC_ALL,""); // User's default locale

	// parse command line
	CTCCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// check if we should use an existing instance
	if (!CProjectSupportingWinApp::InitInstance())
		return FALSE;

	// enable DDE commands
	EnableShellOpen();

	// handle DDE-command on command line
	if (!cmdInfo.m_strDdeCommand.IsEmpty())
	{
		// try to forward DDE command to an existing instance ...
		if (CDdeCommand::SendCommand(GetDDEServerName(), cmdInfo.m_strDdeCommand, _T("System")))
			// ... if successful, exit this instance
			return TRUE;
	}

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// load configuration from registry

	// set registry key
	SetRegistryKey(_T("ToolsCenter"));
	// - construct path to registry section. Do not change this as it is in sync with the MFC functions.
	m_strRegistryRoot = CString("Software\\") + m_pszRegistryKey;
	m_strRegistryRoot += _T("\\");
	m_strRegistryRoot += m_pszAppName;

	LoadStdProfileSettings(MaxRecentProjects);
	
	if (m_pRecentFileList)
		m_pRecentFileList->m_nMaxDisplayLength = RecentFileListMaxDisplayLength;

	CConfiguration::GetInstance()->Serialize(CConfiguration::Load);

	// loading localized resource DLL for TeXnicCenter itself
	CString strResPath(_T("language\\TxcRes") + CConfiguration::GetInstance()->m_strGuiLanguage + _T(".dll"));
	m_hTxcResources = LoadLibrary((LPCTSTR)strResPath);

	BOOL bResourcesIncompatible = FALSE;

	if (m_hTxcResources)
	{
		WORD wIdLang = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
		EnumResourceLanguages(m_hTxcResources, RT_VERSION, MAKEINTRESOURCE(1), (ENUMRESLANGPROC)TxcEnumResourceLanguages, (LONG_PTR) &wIdLang);

		// check compatibility
		CFileVersionInfo fviResources(m_hTxcResources, MAKELONG(wIdLang, 0x04b0/*UNICODE*/));
		CFileVersionInfo fviTxc((HMODULE)NULL);

		if (fviResources.GetFileVersion() == fviTxc.GetFileVersion())
		{
			AfxSetResourceHandle(m_hTxcResources);
		}
		else
		{
			bResourcesIncompatible = TRUE;
		}
	}


	InitContextMenuManager();
	InitKeyboardManager();
	InitShellManager();

	AfxEnableControlContainer(new FontOccManager);

	// show splash screen
	if (cmdInfo.m_bShowSplash)
	{
		CSplashWnd::EnableSplashScreen(cmdInfo.m_bShowSplash);
		CSplashWnd::ShowSplashScreen(NULL);
	}

	// Standardinitialisierung
	if (!AfxOleInit())
		return FALSE;

	// Get Working Directory (directory, TeXnicCenter was installed in)
	m_strWorkingDir = GetModuleFileName();
	m_strWorkingDir = CPathTool::GetDirectory(m_strWorkingDir);

	// Initialize BCG
	SetRegistryBase(_T("Workspace\\") + CConfiguration::GetInstance()->m_strGuiLanguage);

	// Set the application look after the registry base for the workspace has been set
	const UINT look_not_set = ~0U;
	UINT application_look = GetProfileInt(_T("Settings"),_T("ApplicationLook"), look_not_set);

	if (application_look == look_not_set)
	{
		application_look = ID_VIEW_APP_LOOK_OFFICE_2007_BLUE;
	}

	SetApplicationLook(application_look);

	// Dokumentvorlagen registrieren
	// LaTeX-Document
	m_pLatexDocTemplate = new MultiExtensionDocTemplate(
	    IDR_LATEXDOCTYPE,
	    RUNTIME_CLASS(LaTeXDocument),
	    RUNTIME_CLASS(CChildFrame), // Benutzerspezifischer MDI-Child-Rahmen
	    RUNTIME_CLASS(LaTeXView));
	AddDocTemplate(m_pLatexDocTemplate);

	m_pProjectDocTemplate = new CSingleProjectTemplate(
	    IDR_LATEXPROJECTDOCTYPE,
	    RUNTIME_CLASS(CLaTeXProject),
	    -IDR_LATEXPROJECTDOCTYPE // Icon index within the executable or -ID
		);

	AddProjectTemplate(m_pProjectDocTemplate);

	//Register the above file types with TXC
	RegisterShellFileTypes(TRUE);
	//The file types below will not be registered with us

	// BibTeX-Document
	CMultiDocTemplate* bibtempl = new MultiExtensionDocTemplate(
		IDR_BIBTEXDOCTYPE,
		RUNTIME_CLASS(BibTeXDocument),
		RUNTIME_CLASS(CChildFrame), // Benutzerspezifischer MDI-Child-Rahmen
		RUNTIME_CLASS(BibTeXView));
	AddDocTemplate(bibtempl);

	bibtex_doc_template_ = bibtempl;

	// MetaPost-Document
	CMultiDocTemplate* mptempl = new CMultiDocTemplate(
		IDR_METAPOSTDOCTYPE,
		RUNTIME_CLASS(MetaPostDocument),
		RUNTIME_CLASS(CChildFrame), // Benutzerspezifischer MDI-Child-Rahmen
		RUNTIME_CLASS(MetaPostView));
	AddDocTemplate(mptempl);

	metapost_doc_template_ = mptempl;


	if (cmdInfo.Exit())
		return FALSE;

	EnableUserTools(ID_TOOLS_ENTRY, ID_USER_TOOL_FIRST, ID_USER_TOOL_LAST,
	                RUNTIME_CLASS(UserTool),
	                0, IDR_POPUP_PLACEHOLDER_DIR);

	//Prevent some commands from being customized
	CList<UINT, UINT> m_listProtectedCommands;
	m_listProtectedCommands.AddTail(&const_cast<CList<UINT, UINT>&>(CMFCToolBarButton::GetProtectedCommands()));
	UINT unCommand;

	// - MRU project commands
	for (unCommand = ID_FILE_MRU_PROJECT_FIRST; unCommand <= ID_FILE_MRU_PROJECT_LAST; ++unCommand)
		m_listProtectedCommands.AddTail(unCommand);

	// - TextModules - will be customizable in later versions
	for (unCommand = ID_TEXTMODULES_FIRST; unCommand <= ID_TEXTMODULES_LAST; ++unCommand)
		m_listProtectedCommands.AddTail(unCommand);

	// - Placeholder for the toolbars and windows.
	m_listProtectedCommands.AddTail(ID_VIEW_TOOLBAR);

	// - Pass the info to the BCG
	CMFCToolBarButton::SetProtectedCommands(m_listProtectedCommands);

	// Haupt-MDI-Rahmenfenster erzeugen
	CMainFrame *pMainFrame = new CMainFrame;

	const DWORD frameStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE | FWS_PREFIXTITLE;

	if (!pMainFrame->LoadFrame(IDR_MAINFRAME, frameStyle))
		return FALSE;

	m_pMainWnd = pMainFrame;

	// enable file open via drag & drop
	m_pMainWnd->DragAcceptFiles();

	// do not create new file
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	// distribute command line commands
	CString strProjectExtension;
	m_pProjectDocTemplate->GetProjectString(strProjectExtension, CProjectTemplate::filterExt);

	// 1. Is it a DDE file?
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileDDE)
	{
		CConfiguration::GetInstance()->m_strLastProject.Empty();

		if (!ProcessShellCommand(cmdInfo))
			return FALSE;
	}
	// 2. ... have we got a DDE-command on the command line?
	else if (!cmdInfo.m_strDdeCommand.IsEmpty())
	{
		// forward command to DDE-processor
		//TODO: Why do we not directly call OnDDECommand() here?
		// We know here that this instance will process the stuff.
		// Sending it via DDE causes a long pause. This is a real problem when opening TXC via YAP.
		CDdeCommand::SendCommand(GetDDEServerName(), cmdInfo.m_strDdeCommand, _T("System"));
		//OnDDECommand(const_cast<LPTSTR>((LPCTSTR)cmdInfo.m_strDdeCommand));
	}
	// 3. ...are we to open a file and it is a project type?
	else if ((cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen) &&
	         (cmdInfo.m_strFileName.Right(4) == strProjectExtension))
	{
		CConfiguration::GetInstance()->m_strLastProject.Empty();

		OpenProject(cmdInfo.m_strFileName);
	}
	// 4. ...are we to open a file and it is NOT a project type?
	else if ((cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen) &&
	         (cmdInfo.m_strFileName.Right(4) != strProjectExtension))
	{
		CConfiguration::GetInstance()->m_strLastProject.Empty(); // Prevent a project from opening

		if (!ProcessShellCommand(cmdInfo))
			return FALSE;
	}
	// 5. Finally, do the default handling of files.
	else if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// show main frame
	pMainFrame->RebuildToolsMenu();

	bool start_wizard = CProfileMap::GetInstance()->GetActiveProfileKey().IsEmpty();

	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	// hide the splash, if still there
	if (CSplashWnd::IsActive())
	{
		CSplashWnd::GetInstance()->HideSplashScreen();
	}

	UpdateLaTeXProfileSel();

	// load last project of last session
	if (CConfiguration::GetInstance()->m_bLoadLastProject && !CConfiguration::GetInstance()->m_strLastProject.IsEmpty())
		OpenProject(CConfiguration::GetInstance()->m_strLastProject);

	// update frame window
	m_pMainWnd->SendMessage(WM_COMMAND, ID_OPTIONS_CHANGED);

	// Change extension for help file
	CString strHelpFilePath = m_pszHelpFilePath;
	strHelpFilePath.Replace(_T(".HLP"), _T(".chm"));

	CString strHelpFile = CPathTool::GetFile(strHelpFilePath);
	CString strHelpDir = CPathTool::Cat(CPathTool::GetDirectory(strHelpFilePath), _T("Help"));
	strHelpFilePath = CPathTool::Cat(strHelpDir, strHelpFile);

	free((void*)m_pszHelpFilePath);
	m_pszHelpFilePath = _tcsdup(strHelpFilePath);

	// language specific handling
	if (bResourcesIncompatible)
	{
		CFileVersionInfo fviResources, fviTxc;
		fviResources.Create(m_hTxcResources);
		fviTxc.Create();

		CString strMessage;
		strMessage.Format(
		    STE_RESOURCE_CONFLICT, (LPCTSTR)CConfiguration::GetInstance()->m_strGuiLanguage,
		    (LPCTSTR)fviTxc.GetFileVersion(), (LPCTSTR)fviResources.GetFileVersion());
		AfxMessageBox(strMessage);
	}

	// inform the user about that the settings for all languages are
	// stored separately...
	bool bLanguageAlreadyDefined = GetProfileInt(_T("Workspace\\") + CConfiguration::GetInstance()->m_strGuiLanguage,
	                               _T("LanguageAlreadyUsed"), FALSE);
	if (!bLanguageAlreadyDefined)
	{
		AfxMessageBox(STE_LANGUAGE_NEW, MB_ICONINFORMATION | MB_OK);
		WriteProfileInt(_T("Workspace\\") + CConfiguration::GetInstance()->m_strGuiLanguage,
		                _T("LanguageAlreadyUsed"), TRUE);
	}

	// start output type wizard, if no output types are existing
	if (start_wizard)
	{
		COutputWizard dlg(*CProfileMap::GetInstance(), m_pMainWnd);
		dlg.DoModal();

		UpdateLaTeXProfileSel();
	}

	// Show Tip
	ShowTipAtStartup();

	// fontDefaultGUIUnderline doesn't contain the font
	// that is consistent with all the menus and dialogs: replace it
	afxGlobalData.fontDefaultGUIUnderline.DeleteObject();

	LOGFONT lf = GetDisplayFont();
	lf.lfUnderline = TRUE;

	afxGlobalData.fontDefaultGUIUnderline.CreateFontIndirect(&lf);

	return TRUE;
}

void CTeXnicCenterApp::PreLoadState()
{
	GetContextMenuManager()->AddMenu(STE_POPUP_EDITOR, IDR_POPUP_EDITOR);
	GetContextMenuManager()->AddMenu(STE_POPUP_ITEM, IDR_POPUP_ITEM);
	GetContextMenuManager()->AddMenu(STE_POPUP_MAINWINDOWAREA, IDR_POPUP_MDICLIENT);
}

BOOL CTeXnicCenterApp::OnIdle(long count)
{
	if (CWnd* main = AfxGetMainWnd())
		static_cast<CMainFrame*>(main)->OnIdle(count);

	return __super::OnIdle(count);
}

void CTeXnicCenterApp::LoadCustomState()
{
	//Document tabs
	m_bMDITabs = GetInt(_T("MDITabs"), TRUE);
	m_bMDITabIcons = GetInt(_T("MDITabIcons"), TRUE);
	m_nMDITabLocation = GetInt(_T("MDITabLocation"), CMFCTabCtrl::LOCATION_TOP);
	m_nMDITabStyle = GetInt(_T("MDITabStyle"), CMFCTabCtrl::STYLE_3D_ONENOTE);

	// Recent projects
	if (CanUseRecentFiles()) // Respect the group policy
	{
		m_recentProjectList.ReadList();
		UpdateJumpList();
	}

	//Output Profiles
	CProfileMap::GetInstance()->SerializeFromRegistry();

	//Load command repository
	FindPackageFiles();
}

void CTeXnicCenterApp::SaveCustomState()
{
	//Document tabs
	WriteInt(_T("MDITabs"), m_bMDITabs);
	WriteInt(_T("MDITabIcons"), m_bMDITabIcons);
	WriteInt(_T("MDITabLocation"), m_nMDITabLocation);
	WriteInt(_T("MDITabStyle"), m_nMDITabStyle);

	// Recent projects
	if (CanUseRecentFiles()) // Respect the group policy
		m_recentProjectList.WriteList();

	//Output Profiles
	CProfileMap::GetInstance()->SerializeToRegistry();

	// store global configuration
	CConfiguration::GetInstance()->Serialize(CConfiguration::Save);

	// speller
	if (m_pSpell && m_pSpell->IsAddedModified() && !CConfiguration::GetInstance()->m_strSpellPersonalDictionary.IsEmpty())
		m_pSpell->SavePersonalDictionary(CConfiguration::GetInstance()->m_strSpellPersonalDictionary);
}

void CTeXnicCenterApp::EndSession()
{
	///////////////////////////////////////////////////////////////////
	// disable file change notifier
	//m_FileChangeNotifier.Destroy();

	///////////////////////////////////////////////////////////////////
	// close project
	theApp.m_bEndSession = TRUE;
	AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_PROJECT_CLOSE);
}

int CTeXnicCenterApp::ExitInstance()
{
	if (CSplashWnd::IsActive())
		CSplashWnd::GetInstance()->DestroyWindow();

	// Shutdown background thread
	if (m_pBackgroundThread)
	{
		m_pBackgroundThread->PostThreadMessage(WM_QUIT, 0, 0);
		
		::WaitForSingleObject(m_pBackgroundThread->m_hThread, INFINITE);

		m_pBackgroundThread.reset();
	}

	ControlBarCleanUp();

	// unload TXC resource DLL
	if (m_hTxcResources)
		FreeLibrary(m_hTxcResources);

	Scintilla_ReleaseResources();

	return CProjectSupportingWinApp::ExitInstance();
}

/*
void CTeXnicCenterApp::ForwardCommands( HCONV hconvServer )
{
        //
        CTCCommandLineInfo	cmdInfo;
        CString							strCmd;
        ParseCommandLine( cmdInfo );

        switch( cmdInfo.m_nShellCommand )
        {
                case CCommandLineInfo::FileOpen:
                        if( cmdInfo.m_nDocumentLine > -1 )
                                strCmd.Format( _T("[goto(\"%s\",\"%d\")]"), cmdInfo.m_strFileName, cmdInfo.m_nDocumentLine );
                        else
                                strCmd.Format( _T("[open(\"%s\")]"), cmdInfo.m_strFileName );

                        SendDdeCommand( strCmd );
                        break;
        }
}
 */


void CTeXnicCenterApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

LaTeXView* CTeXnicCenterApp::GetActiveLaTeXView()
{
	//Get active frame
	if (!m_pMainWnd) return NULL;
	CFrameWnd* pFrame = ((CMDIFrameWnd*)m_pMainWnd)->GetActiveFrame();
	if (!pFrame || !pFrame->IsKindOf(RUNTIME_CLASS(CChildFrame))) return NULL;

	//Get active view and find out about its type
	CView* pView = pFrame->GetActiveView();
	if (!pView || !pView->IsKindOf(RUNTIME_CLASS(LaTeXView))) return NULL;

	return (LaTeXView*)pView;
}

CodeView* CTeXnicCenterApp::GetActiveCodeView()
{
	//Get active frame
	if (!m_pMainWnd) return NULL;
	CFrameWnd* pFrame = ((CMDIFrameWnd*)m_pMainWnd)->GetActiveFrame();
	if (!pFrame || !pFrame->IsKindOf(RUNTIME_CLASS(CChildFrame))) return NULL;

	//Get active view and find out about its type
	CView* pView = pFrame->GetActiveView();
	if (!pView || !pView->IsKindOf(RUNTIME_CLASS(CodeView))) return NULL;

	return (CodeView*)pView;
}

CString CTeXnicCenterApp::GetCurrentWordOrSelection(const bool bDefaultWordChars, const bool bIncludingEscapeChar, const bool bStripEol)
{
	CodeView* pCView = GetActiveCodeView();
	if (pCView) return pCView->GetCurrentWordOrSelection(bDefaultWordChars, bIncludingEscapeChar, bStripEol);
	return CString();
}

CDocument *CTeXnicCenterApp::GetOpenLatexDocument(LPCTSTR lpszFileName, BOOL bReadOnly /*= FALSE*/)
{
	// get the full path name of the file
	TCHAR lpszFilePath[MAX_PATH];

	GetFullPathName(lpszFileName, MAX_PATH, lpszFilePath,0);
	CString strDocPath = lpszFilePath;

	// get the document template
	CDocTemplate *pDocTemplate = m_pLatexDocTemplate;

	// try to find a document that represents the requested file
	CDocument *pDoc = NULL;
	POSITION pos = pDocTemplate->GetFirstDocPosition();
	BOOL bFound = FALSE;

	while (pos)
	{
		pDoc = (LaTeXDocument*)pDocTemplate->GetNextDoc(pos);

		if (pDoc && pDoc->GetPathName().CompareNoCase(strDocPath) == 0 && pDoc->IsKindOf(RUNTIME_CLASS(LaTeXDocument)))
		{
			bFound = TRUE;
			break;
		}
	}

	if (!bFound || !pDoc)
		return NULL;

	// set write protection
	if (pDoc && bReadOnly)
		static_cast<LaTeXDocument*>(pDoc)->GetView()->GetCtrl().SetReadOnly(TRUE);

	return pDoc;
}

CDocument* CTeXnicCenterApp::GetLatexDocument(LPCTSTR lpszFileName,BOOL bReadOnly /*= FALSE*/)
{
	// get the full path name of the file
	TCHAR lpszFilePath[MAX_PATH];

	GetFullPathName(lpszFileName, MAX_PATH, lpszFilePath, 0);
	CString strDocPath = lpszFilePath;

	// get the document template
	CDocTemplate *pDocTemplate = m_pLatexDocTemplate;

	// try to find a document that represents the requested file
	CDocument* pDoc = NULL;
	POSITION pos = pDocTemplate->GetFirstDocPosition();
	BOOL bFound = FALSE;

	while (pos)
	{
		pDoc = (LaTeXDocument*)pDocTemplate->GetNextDoc(pos);
		if (pDoc && pDoc->GetPathName().CompareNoCase(strDocPath) == 0 && pDoc->IsKindOf(RUNTIME_CLASS(LaTeXDocument)))
		{
			bFound = TRUE;
			break;
		}
	}

	// create new document, if we found no one
	if (!bFound)
	{
		pDoc = pDocTemplate->CreateNewDocument();

		if (!pDoc)
			return NULL;

		// open document
		if (!pDoc->IsKindOf(RUNTIME_CLASS(LaTeXDocument)) || !pDoc->OnOpenDocument(strDocPath))
		{
			pDocTemplate->RemoveDocument(pDoc);
			delete pDoc;
			return NULL;
		}

		pDoc->SetPathName(strDocPath);

		if (bReadOnly)
			static_cast<LaTeXDocument*>(pDoc)->GetView()->GetCtrl().SetReadOnly(TRUE);
	}

	if (!pDoc)
		return NULL;

	// set write protection
	if (pDoc && bReadOnly)
		static_cast<LaTeXDocument*>(pDoc)->GetView()->GetCtrl().SetReadOnly(TRUE);

	return pDoc;
}

CDocument* CTeXnicCenterApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	return OpenLatexDocument(lpszFileName, FALSE, -1, FALSE, true, TRUE);
}

CDocument* CTeXnicCenterApp::OpenDocumentFile(LPCTSTR lpszFileName, BOOL bAddToMRU)
{
	return OpenLatexDocument(lpszFileName, FALSE, -1, FALSE, true, bAddToMRU);
}

CDocument* CTeXnicCenterApp::OpenLatexDocument(LPCTSTR lpszFileName, BOOL bReadOnly /*= FALSE*/,
        int nLineNumber /*= -1*/, BOOL bError /*= FALSE*/,
        bool bAskForProjectLoad /*= true*/, BOOL bAddToMRU /*= FALSE*/)
{
	TRACE1("Opening LaTeX Doc: %s\n", lpszFileName);

	// get the full path name of the file
	TCHAR lpszFilePath[MAX_PATH];

	GetFullPathName(lpszFileName, MAX_PATH, lpszFilePath,0);
	const CString strDocPath = lpszFilePath;

	// try to find a document that represents the requested file
	CDocument *pDoc = NULL;
	CDocTemplate* pDocTemplate = m_pLatexDocTemplate;
	POSITION pos = pDocTemplate->GetFirstDocPosition();
	BOOL bFound = FALSE;
	while (pos)
	{
		pDoc = pDocTemplate->GetNextDoc(pos);
		if (pDoc && pDoc->GetPathName().CompareNoCase(strDocPath)==0 /*&& pDoc->IsKindOf(RUNTIME_CLASS(CLatexDoc))*/)
		{
			bFound = TRUE;
			break;
		}
	}

	//Open new document, if we could not find one
	// If we found one above, we just activate its view
	if (!bFound)
	{
		bool bLoaded = false;

		//Check for existing project, that may be the project of this file.
		if (bAskForProjectLoad)
		{
			CString strProjectPath = CLaTeXProject::GetProjectFileNameFromDoc(strDocPath);

			//Do we have this project already opened?
			bool bOpened = false;
			CLaTeXProject* pLProject = GetProject();

			if (pLProject)
			{
				//We just care, if the existing project file is different
				if (pLProject->GetPathName() == strProjectPath)
					bOpened = true;
			}

			if (!bOpened && CLaTeXProject::CheckExistingProjectFile(strProjectPath))
			{
				//Yes, load the project
				// ==> But close all other documents before
				// skipping annoying question in this version --
				// just closing the documents.
				// lets see, what the users will say ...
				// save modified documents
				if (theApp.SaveAllModified())
				{
					if (OpenProject(strProjectPath))
					{
						//... and the specified file (in most cases the main file), if not already there
						pDoc = GetOpenLatexDocument(lpszFileName, bReadOnly);

						if (!pDoc)
						{
							pDoc = OpenLatexDocument(lpszFileName, bReadOnly, nLineNumber, bError, false);
						}

						bLoaded = true;
					}
				}
			}
		}

		//Just load the file, if a project was not loaded
		if (!bLoaded)
		{
			pDoc = CWinAppEx::OpenDocumentFile(strDocPath, bAddToMRU); //This may open as tex or bib-file, etc.
		}

		//If no project is loaded, create a new spell checker with default configuration.
		// If the same language has already been loaded, then this call will do nothing.
		if (pDoc && !GetProject())
		{
			NewSpeller(CConfiguration::GetInstance()->m_strLanguageDefault,
						CConfiguration::GetInstance()->m_strLanguageDialectDefault);
		}
	}

	if (!pDoc)
		return NULL;

	if (bAddToMRU)
		theApp.UpdateJumpList();

	// set write protection
	if (pDoc && bReadOnly)
	{
		CodeView* view = static_cast<CodeView*>(static_cast<CodeDocument*>(pDoc)->GetView());
		view->GetCtrl().SetReadOnly(TRUE);
	}

	// set error mark
	if (bError)
		static_cast<CodeDocument*>(pDoc)->SetErrorMark(nLineNumber);

	// activate view and go to specified line
	POSITION viewpos = pDoc->GetFirstViewPosition();
	CView *pView = pDoc->GetNextView(viewpos);

	if (!pView)
		return pDoc;

	CFrameWnd *pFrame = pView->GetParentFrame();

	if (!pFrame)
		return pDoc;

	pFrame->ActivateFrame();
	pView = pFrame->GetActiveView();

	if (!pView)
		return pDoc;

	if (!pView->IsKindOf(RUNTIME_CLASS(CodeView)))
		return pDoc;

	if (nLineNumber >= 0)
	{
		if (nLineNumber > 0)
			--nLineNumber;

		static_cast<CodeView*>(pView)->GoToLine(nLineNumber);
	}

	// give input focus to view
	pView->SetFocus();

	return pDoc;
}

CString CTeXnicCenterApp::GetProjectString(CProjectTemplate::ProjectStringIndex index)
{
	CString strResult;
	m_pProjectDocTemplate->GetProjectString(strResult, index);
	return strResult;
}

CString CTeXnicCenterApp::GetProjectFileFilter()
{
	return GetProjectString(CProjectTemplate::filterName)
	       + _T("|*")
	       + GetProjectString(CProjectTemplate::filterExt)
	       + _T('|')
	       + AfxLoadString(STE_ALL_FILES_FILTER);
}

CString CTeXnicCenterApp::GetLatexString(CDocTemplate::DocStringIndex index)
{
	CString strResult;
	m_pLatexDocTemplate->GetDocString(strResult, index);
	return strResult;
}

CString CTeXnicCenterApp::GetLatexFileFilter()
{
	return
	    GetLatexString(CDocTemplate::filterName)
	    + _T("|*")
	    + GetLatexString(CDocTemplate::filterExt)
	    + _T('|')
	    + AfxLoadString(STE_ALL_FILES_FILTER);
}

void CTeXnicCenterApp::SaveAllModifiedWithoutPrompt(const bool bAskForFilenameIfNeeded /*= false*/)
{
	m_bSavingAll = TRUE;

	// save all projects
	POSITION pos = m_pProjectDocTemplate->GetFirstProjectPosition();

	while (pos)
	{
		CProject *pDoc = m_pProjectDocTemplate->GetNextProject(pos);

		if (pDoc->GetPathName().IsEmpty())
			pDoc->DoFileSave();
		else
			pDoc->Save(pDoc->GetPathName());
	}

	// Save all documents
	pos = GetFirstDocTemplatePosition();
	while (pos)
	{
		CDocTemplate* t = GetNextDocTemplate(pos);
		POSITION pos1 = t->GetFirstDocPosition();

		while (pos1)
		{
			CDocument *pDoc = m_pLatexDocTemplate->GetNextDoc(pos1);

			// We are only interested in modified documents
			if (pDoc->IsModified())
			{
				CPathTool Path(pDoc->GetPathName());

				// File name is ok, if the file actually exists on disk or if its directory exists
				const bool bFileNameIsOK = Path.Exists()
										|| ( CPathTool::Exists(Path.GetDirectory()) && !Path.GetFile().IsEmpty());

				// Default behavior: Never prompt the user, even if we don't have a file name.
				if (bFileNameIsOK)
				{
					// The file already exists or can be created in the existing directory
					pDoc->OnSaveDocument(Path);
				}
				else
				{
					// The file does not yet exist. Ask for a filename, if the calling function wants that.
					if (bAskForFilenameIfNeeded) pDoc->DoFileSave();
				}
			}
		}
	}

	m_bSavingAll = FALSE;
}

BOOL CTeXnicCenterApp::OpenProject(LPCTSTR lpszPath, bool addToRecentList)
{
	TRACE1("Opening LaTeX Project: %s\n", lpszPath);

	//Close the current project
	AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_PROJECT_CLOSE);

	CProject* pDoc = m_pProjectDocTemplate->OpenProjectFile(lpszPath);

	// test for success
	if (!pDoc)
	{
		AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_PROJECT_CLOSE);
		return FALSE;
	}
	else
	{
		// Success		
		if (addToRecentList)
		{
			// Add to recent project list
			m_recentProjectList.Add(lpszPath);
			UpdateJumpList();
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CTeXnicCenterApp-Nachrichtenbehandlungsroutinen

void CTeXnicCenterApp::ShowTipAtStartup(void)
{
	CTCCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (
	    (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNothing || cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
	    && cmdInfo.m_bShowSplash && !cmdInfo.m_bRunAutomated && !cmdInfo.m_bRunEmbedded)
	{
		CTipDlg dlg;
		if (dlg.m_bStartup)
			dlg.DoModal();
	}
}

void CTeXnicCenterApp::ShowTipOfTheDay(void)
{
	CTipDlg dlg;
	dlg.DoModal();
}

void CTeXnicCenterApp::OnTip()
{
	ShowTipOfTheDay();
}

void CTeXnicCenterApp::OnFileNew()
{
	// get current project's directory
	CString strDefaultDir;
	CLaTeXProject *pProject = GetProject();

	if (pProject)
		strDefaultDir = CPathTool::GetDirectory(pProject->GetMainPath());
	else
		strDefaultDir = CConfiguration::GetInstance()->m_strDefaultPath;

	// create file from template
	CDocumentNewDialog dlg(AfxGetMainWnd());

	for (int i = 0; i < CConfiguration::GetInstance()->m_astrDocumentTemplatePaths.GetSize(); i++)
		dlg.AddSearchDir(CConfiguration::GetInstance()->m_astrDocumentTemplatePaths[i]);

	dlg.AddTemplateFilter(_T("*.tex"), RUNTIME_CLASS(CFileBasedDocumentTemplateItem));
	dlg.AddTemplateFilter(_T("*.dll"), RUNTIME_CLASS(CWizardBasedDocumentTemplateItem));

	dlg.DoModal();
}

void CTeXnicCenterApp::OnFileSaveAll()
{
	// save all
	SaveAllModifiedWithoutPrompt(true);

	// reparse project
	m_pMainWnd->SendMessage(WM_COMMAND, ID_PROJECT_PARSE);
}

void CTeXnicCenterApp::OnDisableStdCmd(CCmdUI* pCmdUI)
{
	// disables standard document commands for project document
	pCmdUI->Enable(HasOpenDocuments());
}

void CTeXnicCenterApp::OnFileOpen()
{
	CString filter;//((LPCTSTR)STE_FILE_LATEXFILTER)

	POSITION pos = GetFirstDocTemplatePosition();

	while (pos)
	{
		if (CDocTemplate* doc = GetNextDocTemplate(pos))
		{
			filter += GetDocTemplateFilter(doc);
		}
	}

	filter += CString(MAKEINTRESOURCE(STE_ALL_FILES_FILTER));

	CFileDialogEx dlg(
	    TRUE,
	    GetLatexString(CDocTemplate::filterExt), NULL,
	    OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT, filter);

	std::vector<TCHAR> buffer(0x10000);
	dlg.m_ofn.lpstrFile = &buffer[0]; // Use a larger buffer
	dlg.m_ofn.nMaxFile = static_cast<DWORD>(buffer.size());

	//Get default path
	CString strInitialDir = AfxGetDefaultDirectory();
	dlg.m_ofn.lpstrInitialDir = strInitialDir;

	//Show the dialog
	if (dlg.DoModal() != IDOK)
	{
		//It was cancelled - the PathName is not set.
		// Therefore, we use GetLastOpenedFolder.
		AfxSetLastDirectory(dlg.GetLastOpenedFolder());
		return;
	}

	POSITION file = dlg.GetStartPosition();

	while (file)
	{
		const CString path = dlg.GetNextPathName(file);
		// open file
		//NOTE: dlg.GetReadOnlyPref() always return false on my WinXP SP2, Tino, Jan 2007
		OpenLatexDocument(path, dlg.GetReadOnlyPref(), -1, false, true, TRUE);
	}

	AfxSetLastDirectory(CPathTool::GetDirectory(dlg.GetPathName()));	
}

void CTeXnicCenterApp::OnProjectOpen()
{
	// display file dialog
	CFileDialogEx dialog(TRUE, NULL, NULL,
	                     OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
	                     GetProjectFileFilter(), AfxGetMainWnd());

	//Get default path
	CString strInitialDir = AfxGetDefaultDirectory();
	dialog.m_ofn.lpstrInitialDir = strInitialDir;

	//Show the dialog
	if (dialog.DoModal() != IDOK)
	{
		//It was cancelled - the PathName is not set.
		// Therefore, we use GetLastOpenedFolder.
		AfxSetLastDirectory(dialog.GetLastOpenedFolder());
		return;
	}

	AfxSetLastDirectory(CPathTool::GetDirectory(dialog.GetPathName()));
	//open project file
	OpenProject(dialog.GetPathName());
}

/*
void CTeXnicCenterApp::OnProjectClose()
{
        // close latex-documents?

        // skipping annoying question in this version --
        // just closing the documents.
        // lets see, what the users will say ...
        if (!m_pLatexDocTemplate->SaveAllModified())
                return;

        // close project
        POSITION	pos = m_pProjectDocTemplate->GetFirstProjectPosition();
        if( !(BOOL)pos )
                return;

        // close document
        CProject	*pDoc = m_pProjectDocTemplate->GetNextProject( pos );
        if( !pDoc )
                return;

        if( !pDoc->SaveModified() )
                return;

        // remeber last project
        if( m_bEndSession )
                CConfiguration::GetInstance()->m_strLastProject = pDoc->GetPathName();
        else
                CConfiguration::GetInstance()->m_strLastProject.Empty();

        pDoc->OnCloseProject();
        m_pProjectDocTemplate->RemoveProject( pDoc );
        delete pDoc;

        // lets close all the documents
        m_pLatexDocTemplate->CloseAllDocuments(FALSE);
}
 */


void CTeXnicCenterApp::OnUpdateProjectClose(CCmdUI* pCmdUI)
{
	// enabled, if there is an open document
	pCmdUI->Enable(m_pProjectDocTemplate->GetFirstProjectPosition() != NULL);
}

void CTeXnicCenterApp::OnProjectSave()
{
	CLaTeXProject* pProject = GetProject();
	if (pProject)
		pProject->DoProjectSave();
}

void CTeXnicCenterApp::OnUpdateProjectSave(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetProject() ? TRUE : FALSE);
}

void CTeXnicCenterApp::OnLatexNew()
{
	CDocument *pDoc = m_pLatexDocTemplate->OpenDocumentFile(NULL);

	if (pDoc && CConfiguration::GetInstance()->m_bSaveNewDocuments)
		pDoc->DoFileSave();

	//If no project is loaded, create a new spell checker with default configuration.
	// If the same language has already been loaded, then this call will do nothing.
	if (pDoc && !GetProject())
	{
		NewSpeller(CConfiguration::GetInstance()->m_strLanguageDefault,
					CConfiguration::GetInstance()->m_strLanguageDialectDefault);
	}
}

void CTeXnicCenterApp::OnExtrasOptions()
{
	COptionDialog dlg;

	if (dlg.DoModal() != IDOK)
		return;

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// send information about change
	m_pMainWnd->SendMessage(WM_COMMAND, ID_OPTIONS_CHANGED);

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// write options to registry
	CConfiguration::GetInstance()->Serialize(CConfiguration::Save);
}

void CTeXnicCenterApp::OnLatexEditProfiles()
{
	CProfileDialog dlg;

	if (dlg.DoModal() != IDOK)
		return;

	CProfileMap::GetInstance()->SerializeToRegistry();
	UpdateLaTeXProfileSel();
}

void CTeXnicCenterApp::OnLaTeXProfile()
{
	CActiveProfileDialog dlg(m_pMainWnd);
	dlg.DoModal();
	UpdateLaTeXProfileSel();
}

void CTeXnicCenterApp::OnLatexProfileSel()
{
	CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(m_pMainWnd);

	if (!m_pMainWnd || !IsWindow(m_pMainWnd->m_hWnd)) return;

	//Get combo box for selection of the profile
	int pos = 0;
	CMFCToolBarComboBoxButton* pButton = dynamic_cast<CMFCToolBarComboBoxButton*>(pMainFrame->GetToolBarButton(ID_LATEX_PROFILE_SEL, pos));
	if (!pButton) return;
	CComboBox* pBox = pButton->GetComboBox();
	if (!pBox || !IsWindow(pBox->m_hWnd)) return;

	//Get currently selected output type
	CString strActiveProfile;
	pBox->GetLBText(pButton->GetCurSel(), strActiveProfile);
	CProfileMap::GetInstance()->SetActiveProfile(strActiveProfile);

	//Give the focus back to the editor - otherwise, all subsequent keyboard input goes to the combo box
	pMainFrame->SendMessage(WM_COMMAND, ID_WINDOW_EDITOR);
	//If we do not have an open edit window, the above message will not succeed.
	// So we set the focus to the main window.
	if (GetFocus() == pBox->m_hWnd) pMainFrame->SetFocus();
}

void CTeXnicCenterApp::UpdateLaTeXProfileSel()
{
	// get selection button
	CMainFrame *pMainFrame = (CMainFrame*)m_pMainWnd;

	if (!m_pMainWnd || !IsWindow(m_pMainWnd->m_hWnd))
		return;

	// get available profiles and active profile
	CString strActiveProfile = CProfileMap::GetInstance()->GetActiveProfileKey();
	CStringArray astrProfiles;
	CProfileMap::GetInstance()->GetKeyList(astrProfiles);

	// update all instances of this button
	int pos = 0;
	CMFCToolBarButton* pButton = NULL;
	while ((pButton = pMainFrame->GetToolBarButton(ID_LATEX_PROFILE_SEL, pos)) != NULL)
	{
		//Cast properly
		CMFCToolBarComboBoxButton* pComboButton = dynamic_cast<CMFCToolBarComboBoxButton*>(pButton);
		ASSERT(pComboButton); //If this triggers, then another toolbar may have been instantiated with IDR_LATEX, or some other way the toolbars were overwritten.
		if (!pComboButton) continue;

		//Delete all items and refill
		pComboButton->RemoveAllItems();
		for (int i = 0; i < astrProfiles.GetSize(); i++)
		{
			pComboButton->AddItem(astrProfiles[i]);
		}

		//Select item
		CComboBox* pComboBox = pComboButton->GetComboBox();
		if (!pComboBox || !IsWindow(pComboBox->m_hWnd) || pComboBox->GetDroppedState()) continue;

		// we have to do it manually, because the pButton->SelectItem() does not work correctly
		int nIndex = pComboBox->FindStringExact(-1, strActiveProfile);
		pComboButton->SelectItem(nIndex);
		pComboBox->SetCurSel(nIndex);
		pComboButton->NotifyCommand(CBN_SELENDOK);
		pComboBox->GetParent()->UpdateWindow();
	}
}

void CTeXnicCenterApp::OnDocumentSaved()
{
	if (m_bSavingAll)
		return;

	// reparse project
	m_pMainWnd->SendMessage(WM_COMMAND, ID_PROJECT_PARSE);
}

BOOL CTeXnicCenterApp::PreTranslateMessage(MSG* pMsg)
{
	if (CSplashWnd::PreTranslateAppMessage(pMsg))
		return TRUE;

	//// catch cursor blink message
	//if (pMsg->message == 0x0118)
	//{
	//	CWnd *pWnd = CWnd::GetFocus();
	//	if (IsWindow(pWnd->GetSafeHwnd()) && pWnd->PreTranslateMessage(pMsg))
	//		return TRUE;
	//}

	//// Workaround for bug 1222824 (Crash on ALT + ^)
	//if (pMsg->message == 0x104 && pMsg->wParam == 0xDC)
	//{
	//	return TRUE;
	//}

	//Cycling through MDI windows in MRU order: process the release of the CTRL key
	if (pMsg->message == WM_KEYUP && pMsg->wParam == VK_CONTROL)
	{
		CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(m_pMainWnd);
		if (m_pMainWnd && IsWindow(m_pMainWnd->m_hWnd))
		{
			pMainFrame->SetCurrentChildFrameAsRecentlyUsed();
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	// Workaround for the buggy activation of the main menu on F10 or Alt keys.
	// Important for cycling through Warnings, for Ctrl-Alt-Shortcuts, and rectangular selections.
	//==> With this workaround, TXC behaves like a normal Windows application.

	//Eat the WM_SYSKEYUP of the Alt key, if it was pressed togther with the Control key.
	if (pMsg->message == WM_SYSKEYUP && pMsg->wParam == VK_MENU && GetKeyState(VK_CONTROL) < 0)
	{
		return true;
	}

	//Check whether the Alt key is pressed when a mouse button gets pressed too
	if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_MBUTTONDOWN || pMsg->message == WM_RBUTTONDOWN)
	{
		m_bAltKeyDownWithMouseButton = (GetKeyState(VK_MENU) < 0);
	}
	// - and eat the next WM_SYSKEYUP of the Alt key
	// - ==> Rectangular selections are not followed by a menu activation.
	if (m_bAltKeyDownWithMouseButton && pMsg->message == WM_SYSKEYUP && pMsg->wParam == VK_MENU)
	{
		m_bAltKeyDownWithMouseButton = false; //Reset for the next pressed Alt key
		return true;
	}

	//Eat the WM_SYSKEYUP of F10, if we have a shortcut for F10.
	//Otherwise, the menu would be activated together with the F10-command itself, which is not the Windos default.
	//TODO: How to find out, which command is assigned to F10. And whether it is available at the moment.
	if (pMsg->message == WM_SYSKEYUP && pMsg->wParam == VK_F10)
	{
		CKeyboardManager* pKeyboardManager = GetKeyboardManager();
		if (pKeyboardManager && pKeyboardManager->IsKeyHandled(VK_F10, FVIRTKEY + FNOINVERT, ((CMDIFrameWnd*)m_pMainWnd), true))
		{
			return true; //Eat
		}
	}

	// end of Workaround for the buggy activation of the main menu on F10 or Alt keys.
	////////////////////////////////////////////////////////////////////////////////

	//Call the default implementation
	return CProjectSupportingWinApp::PreTranslateMessage(pMsg);
}

BOOL AddDestination(CJumpList& jumpList, CString strCategory, CString strFileName, bool openInSameWindow)
{
	TCHAR szAppPath[MAX_PATH];
	::GetModuleFileName( NULL, szAppPath, MAX_PATH );

	int backslashIndex = strFileName.ReverseFind('\\');
	CString strTitle = backslashIndex >= 0 ? strFileName.Right(strFileName.GetLength() - backslashIndex - 1) : strFileName;
	CString strCommandLineArgs;

	if (openInSameWindow)
	{
		strCommandLineArgs = _T("/ddecmd \"[goto('") + strFileName + _T("', '0')]\"");
	}
	else
	{
		strCommandLineArgs = '"' + strFileName + '"';
	}

	CComPtr<IShellLink> shellLinkPtr;
	if (FAILED(shellLinkPtr.CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER)))
	{
		return FALSE;
	}
	
	shellLinkPtr->SetPath(CString('"') + szAppPath + '"');
	shellLinkPtr->SetDescription(strFileName);
	shellLinkPtr->SetArguments(strCommandLineArgs);

	int iIcon = -1;
	CString strFileExt = strTitle.Right(4);
	if (strFileExt == ".tex")
		iIcon = 1;
	else if (strFileExt == ".bib" || strFileExt == ".bbl")
		iIcon = 2;
	else if (strFileExt == ".tcp")
		iIcon = 3;

	if (iIcon < 0)
		shellLinkPtr->SetIconLocation(_T("%SystemRoot%\\system32\\SHELL32.dll"), 0);
	else
		shellLinkPtr->SetIconLocation(szAppPath, iIcon);

	CComQIPtr<IPropertyStore> propPtr = shellLinkPtr;

	if (propPtr != NULL)
	{
		PROPVARIANT var;
		if (FAILED(InitPropVariantFromString(strTitle, &var)))
		{
			return FALSE;
		}
		if (FAILED(propPtr->SetValue(PKEY_Title, var)))
		{
			PropVariantClear(&var);
			return FALSE;
		}

		HRESULT hr = propPtr->Commit();
		PropVariantClear(&var);
		if (FAILED(hr))
		{
			return FALSE;
		}
	}

	return jumpList.AddDestination(strCategory, shellLinkPtr.Detach());
}

void CTeXnicCenterApp::UpdateJumpList()
{
	CJumpList jumpList;
	jumpList.InitializeList();

	CString strCategory;
	strCategory.LoadString( IDS_MRU_PROJECTS );

	int nProjects = m_recentProjectList.GetSize();
	for (int i = 0; i < nProjects; ++i)
	{
 		if (!m_recentProjectList.m_arrNames[i].IsEmpty())
			AddDestination(jumpList, strCategory, m_recentProjectList.m_arrNames[i], false);
	}

	strCategory.LoadString( IDS_MRU_FILES );

	int nFiles = min(m_pRecentFileList->GetSize(), (int)jumpList.GetMaxSlots()/2);
	for (int i = 0; i < nFiles; ++i)
	{
		if (!m_pRecentFileList->m_arrNames[i].IsEmpty())
			AddDestination(jumpList, strCategory, m_pRecentFileList->m_arrNames[i], true);
	}

	jumpList.CommitList();
}

void CTeXnicCenterApp::UpdateRecentFileList(CCmdUI *pCmdUI, CRecentFileList &recentFileList, UINT unFirstCommandId, UINT unNoFileStringId)
{
	///////////////////////////////////////////////////////////////////
	if (!recentFileList.GetSize() || recentFileList[0].IsEmpty())
	{
		pCmdUI->SetText(AfxLoadString(unNoFileStringId));
		pCmdUI->Enable(FALSE);
		return;
	}

	if (CConfiguration::GetInstance()->m_bOptimizeMenuForVisuallyHandicappedUsers)
	{
		/////////////////////////////////////////////////////////////////
		//
		// handling for normal windows menus
		// (visually handicapped user mode)
		//

		// update message from menu bar?
		if (!IsMenu(pCmdUI->m_pSubMenu->GetSafeHmenu()))
			return;

		CMenu *pMenu = pCmdUI->m_pSubMenu;
		int i;

		for (i = pMenu->GetMenuItemCount(); i >= 0; --i)
			pMenu->DeleteMenu(i, MF_BYPOSITION);

		for (i = 0; i < recentFileList.GetSize(); i++)
		{
			// get project path
			CString strDisplayName;
			CString strCurrentDir;

			GetCurrentDirectory(MAX_PATH, strCurrentDir.GetBuffer(MAX_PATH));
			strCurrentDir.ReleaseBuffer();

			if (!recentFileList.GetDisplayName(
			            strDisplayName,
			            i,
			            strCurrentDir, strCurrentDir.GetLength()))
				break;

			// add number for the first ten files
			if (i < 10)
			{
				CString strFormat;
				if (i == 9)
					strFormat.Format(_T("1&0 %s"), (LPCTSTR)strDisplayName);
				else
					strFormat.Format(_T("&%d %s"), i + 1, (LPCTSTR)strDisplayName);
				strDisplayName = strFormat;
			}

			pMenu->AppendMenu(MF_STRING, unFirstCommandId + i, (LPCTSTR)strDisplayName);
		}
	}
	else
	{
		/////////////////////////////////////////////////////////////////
		//
		// handling for BCG menus
		//

		// update message from menu bar?
		if (!pCmdUI->m_pOther || !pCmdUI->m_pOther->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar)))
			return;

		// insert all entries of recent project list
		CMFCPopupMenuBar *pMenu = (CMFCPopupMenuBar*)pCmdUI->m_pOther;
		BOOL bChange = FALSE;

		for (int i = 0; i < recentFileList.GetSize(); i++)
		{
			// get project path
			CString strDisplayName;
			CString strCurrentDir;

			GetCurrentDirectory(MAX_PATH, strCurrentDir.GetBuffer(MAX_PATH));
			strCurrentDir.ReleaseBuffer();

			if (!recentFileList.GetDisplayName(
			            strDisplayName,
			            i,
			            strCurrentDir, strCurrentDir.GetLength()))
				break;

			// add number for the first ten projects
			if (i < 10)
			{
				CString strFormat;

				if (i == 9)
					strFormat.Format(_T("1&0 %s"), (LPCTSTR)strDisplayName);
				else
					strFormat.Format(_T("&%d %s"), i + 1, (LPCTSTR)strDisplayName);

				strDisplayName = strFormat;
			}

			if (pMenu->GetCount() > i && pMenu->GetButtonText(i) != strDisplayName)
			{
				pMenu->SetButtonText(i, strDisplayName);
				bChange = TRUE;
			}
			else if (pMenu->GetCount() <= i)
			{
				int nIndex = pMenu->InsertButton(CMFCToolBarMenuButton(unFirstCommandId + i, 0, -1, strDisplayName), i);
				ASSERT(nIndex > -1);
				UNUSED_ALWAYS(nIndex);

				bChange = TRUE;
			}
		}

		// repaint menu
		//if( bChange )
		//	pMenu->AdjustLayout();
	}
}

void CTeXnicCenterApp::OnUpdateFileMRUProjectList(CCmdUI *pCmdUI)
{
	UpdateRecentFileList(pCmdUI, m_recentProjectList, ID_FILE_MRU_PROJECT_FIRST, STE_FILE_MRU_PROJECT_NONE);
}

void CTeXnicCenterApp::OnUpdateFileMRU(CCmdUI* pCmdUI)
{
	// We are handling the MRU-List on our own
	OnUpdateFileMRUFileList(pCmdUI);
}

void CTeXnicCenterApp::OnUpdateFileMRUFileList(CCmdUI *pCmdUI)
{
	if (m_pRecentFileList)
		UpdateRecentFileList(pCmdUI, *m_pRecentFileList, ID_FILE_MRU_FILE1, STE_FILE_MRU_FILE_NONE);
	else
		pCmdUI->Enable(FALSE);
}

void CTeXnicCenterApp::OnFileMRUProject(UINT unID)
{
	int nIndex = unID - ID_FILE_MRU_PROJECT_FIRST;

	ASSERT(nIndex >= 0 && nIndex < m_recentProjectList.GetSize());

	const CString filename = m_recentProjectList[nIndex];

	if (OpenProject(filename, false))
		m_recentProjectList.Add(filename); // Place on top
	else
		m_recentProjectList.Remove(nIndex);

	UpdateJumpList();
}

//void CTeXnicCenterApp::OnFileMRUFile(UINT unID)
//{
//	int	nIndex = unID - ID_FILE_MRU_FILE1;
//
//	ASSERT(nIndex >= 0 && nIndex < m_pRecentFileList->GetSize());
//	if (!OpenLatexDocument((*m_pRecentFileList)[nIndex], false, -1, false, true))
//		m_pRecentFileList->Remove(nIndex);
//}

BOOL CTeXnicCenterApp::OnDDECommand(LPTSTR lpszCommand)
{
	CString strCommand(lpszCommand);
	CString strFileName(lpszCommand);
	CString strProjectExtension;

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// handle goto command => [goto("filepath", "linenumber")]
	if (strCommand.Left(7) == _T("[goto(\""))
	{
		int nStart, nEnd;
		CString strLineNumber;
		int nLineNumber;

		// get file name
		nStart = strCommand.Find(_T('"'));
		nEnd = strCommand.Find(_T('"'), nStart + 1);

		if (nStart == -1 || nEnd == -1 || nEnd - nStart <= 1)
			return FALSE;

		strFileName = strCommand.Mid(nStart + 1, nEnd - (nStart + 1));

		// Some LaTeX engines, such as XeLaTeX, seem to omit the file extension
		// in pdfsync data leading to failures when using inverse search.
		if (!CPathTool::Exists(strFileName) &&
			CPathTool::GetFileExtension(strFileName).IsEmpty())
		{
			const CString tmp = strFileName + _T(".tex");

			if (CPathTool::Exists(tmp))
				strFileName = tmp;
		}

		// get line number
		nStart = strCommand.Find(_T('"'), nEnd + 1);
		nEnd = strCommand.Find(_T('"'), nStart + 1);

		if (nStart == -1 || nEnd == -1 || nEnd - nStart <= 1)
			return FALSE;

		strLineNumber = strCommand.Mid(nStart + 1, nEnd - (nStart + 1));
		nLineNumber = _ttol(strLineNumber);

		//Do not load the last project, if we now open a file/project via dde
		CConfiguration::GetInstance()->m_strLastProject.Empty();

		if (!OpenLatexDocument(strFileName, FALSE, nLineNumber, FALSE, true)) return FALSE;

		return TRUE;
	}

	/*
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// handle project specific stuff
	m_pProjectDocTemplate->GetProjectString( strProjectExtension, CProjectTemplate::filterExt );

	if( strFileName.Left( 7 ) == _T("[open(\"") )
	        strFileName = strFileName.Mid( 7, strFileName.GetLength() - 7 - 3 );
	else if( strFileName.Left( 8 ) == _T("[print(\"") )
	        strFileName = strFileName.Mid( 8, strFileName.GetLength() - 8 - 3 );
	else if( strFileName.Left( 10 ) == _T("[printto(\"") )
	        strFileName = strFileName.Mid( 10, strFileName.GetLength() - 10 - 3 );

	// Handle only project-files here
	if( strFileName.Right( 4 ) == strProjectExtension )
	{
	        // we only support opening for project-files
	        if( strCommand.Left( 7 ) != _T("[open(\"") )
	                return FALSE;

	        // open new project-file
	        OpenProject( strFileName );
	}
	 */

	return CProjectSupportingWinApp::OnDDECommand(lpszCommand);
}

BOOL CTeXnicCenterApp::OpenURL(LPCTSTR lpszURL)
{
	if (::ShellExecute(NULL, NULL, lpszURL, NULL, NULL, NULL) < (HINSTANCE)32)
		return FALSE;

	return TRUE;
}

void CTeXnicCenterApp::OnUpdateURL(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
}

void CTeXnicCenterApp::OnURL(UINT unID)
{
	// text behind the second '\n'-character in the description of the command
	// is the full URL
	CString strURL;
	strURL.LoadString(unID);
	int nIndex = strURL.Find(_T('\n'));

	if (nIndex < 0)
		return;

	if ((nIndex + 1) < strURL.GetLength() && (nIndex = strURL.Find(_T('\n'), nIndex + 1)) < 0)
		return;

	if (++nIndex >= strURL.GetLength())
		return;

	if (!OpenURL(strURL.Mid(nIndex, strURL.GetLength() - 1)))
		AfxMessageBox(STE_URL_CANNOTOPEN, MB_ICONSTOP | MB_OK);
}

void CTeXnicCenterApp::OnAppLicense()
{
	OpenURL(CPathTool::Cat(GetWorkingDir(), CString((LPCTSTR)STE_LICENSE_FILE)));
}

void CTeXnicCenterApp::OnWindowCloseAll()
{
	// save modified documents
	if (!theApp.SaveAllModified())
		return;

	theApp.CloseAllDocuments(FALSE);
}

void CTeXnicCenterApp::OnHelp()
{
	CWinApp::OnHelp();
}

Speller* CTeXnicCenterApp::GetSpeller()
{
	return m_pSpell.get();
}

void CTeXnicCenterApp::ReleaseSpeller()
{
	ASSERT(m_pSpell && "ReleaseSpeller called although no speller exists.");

	if (CConfiguration::GetInstance()->m_bSpellEnable && m_pBackgroundThread)
	{
		m_pBackgroundThread->EnableSpeller(FALSE);
	}

	m_pSpell.reset(NULL);
}

bool CTeXnicCenterApp::NewSpeller(const CString& strLanguage, const CString& strLanguageDialect)
{
	CSingleLock lock(&m_csLazy);

	// Create dictionary name and path
	CString dicName;
	dicName.Format(_T("%s\\%s_%s.dic"),
	               (LPCTSTR)CConfiguration::GetInstance()->m_strSpellDictionaryPath,
	               (LPCTSTR)strLanguage,
	               (LPCTSTR)strLanguageDialect);
	// Create affix name and path
	CString affName;
	affName.Format(_T("%s\\%s_%s.aff"),
	               (LPCTSTR)CConfiguration::GetInstance()->m_strSpellDictionaryPath,
	               (LPCTSTR)strLanguage,
	               (LPCTSTR)strLanguageDialect);

	//Compare desired language/dialect to the currently active one
	if (m_pSpell && m_pSpell->SameLanguage(affName, dicName))
	{
		//Same language is already loaded. Everything is fine, no further need for action.
		// NOTE: We explicitly ignore the case that the dictionary file may have changed on disk.
		//       You just have to restart TXC, or switch the language twice.
		return true;
	}

	//Release the old one, if it exists.
	if (m_pSpell) ReleaseSpeller();

	//Actually load a new dictionary into a new speller
	CWaitCursor wait;
	try
	{
		if (!::PathFileExists(affName))
			throw AfxFormatString1(STE_DICTIONARY_OPEN_FAIL, affName);

		if (!::PathFileExists(dicName))
			throw AfxFormatString1(STE_DICTIONARY_OPEN_FAIL, dicName);

		m_pSpell.reset(new Speller(affName, dicName));

		// Create the personal dictionary if we have a name
		if (!CConfiguration::GetInstance()->m_strSpellPersonalDictionary.IsEmpty())
			m_pSpell->LoadPersonalDictionary(CConfiguration::GetInstance()->m_strSpellPersonalDictionary);
	}
	catch (const CString& str)
	{
		// One of the dictionary files does not exist.
		CConfiguration::GetInstance()->m_bSpellEnable = FALSE;
		AfxMessageBox(str, MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
		return false;
	}
	catch (...)
	{
		// There was an error while creating the dictionary. This may be due
		// to a corrupted file system or insufficient operating system privileges.
		// Whatever the cause, it deserves a little stronger warning message.
		CConfiguration::GetInstance()->m_bSpellEnable = FALSE;
		AfxMessageBox(AfxFormatString1(STE_DICTIONARY_CREATE_FAIL, _T("")),
		              MB_OK | MB_ICONERROR | MB_TASKMODAL);
		return false;
	}

	if (CConfiguration::GetInstance()->m_bSpellEnable && m_pBackgroundThread)
	{
		ResetSpeller();
		m_pBackgroundThread->EnableSpeller(TRUE);
		AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_BG_UPDATE_PROJECT); // clear or set the line attributes
	}

	return true;
}

SpellerBackgroundThread* CTeXnicCenterApp::GetSpellerThread()
{
    CSingleLock lock(&m_csLazy);

	if (m_pBackgroundThread == NULL)
	{
		m_pBackgroundThread.reset(new SpellerBackgroundThread());
		m_pBackgroundThread->m_bAutoDelete = FALSE;
		m_pBackgroundThread->CreateThread();
		m_pBackgroundThread->SetThreadPriority(THREAD_PRIORITY_LOWEST);

		// Get the background thread to trigger the dictionary creation which
		// may take several seconds. This allows the user to continue working
		// without interruption.
		m_pBackgroundThread->EnableSpeller(CConfiguration::GetInstance()->m_bSpellEnable != 0);

		if (CConfiguration::GetInstance()->m_bSpellEnable)
		{
			ResetSpeller();
		}
	}

	return m_pBackgroundThread.get();
}

void CTeXnicCenterApp::OnUpdateProject()
{
	// Inform the background thread to update all of the active views.
	SpellerBackgroundThread *pBackgroundThread = GetSpellerThread();

	// get the document template
	CDocTemplate *pDocTemplate = m_pLatexDocTemplate;

	CDocument *pDoc = NULL;
	POSITION pos = pDocTemplate->GetFirstDocPosition();

	while (pos)
	{
		pDoc = pDocTemplate->GetNextDoc(pos);
		if (pDoc)
		{
			// Get the first document view
			POSITION ViewPos = pDoc->GetFirstViewPosition();
			if (ViewPos)
			{
				LaTeXView *pView = (LaTeXView *)pDoc->GetNextView(ViewPos);

				if (pView && pView->IsKindOf(RUNTIME_CLASS(LaTeXView)))
					pBackgroundThread->RecheckSpelling(pView);
			}
		}
	}
}

void CTeXnicCenterApp::OnProjectNewFromFile()
{
	LaTeXView* pEdit = GetActiveLaTeXView();
	if (!pEdit) return;

	LaTeXDocument* pDoc = pEdit->GetDocument();
	if (!pDoc) return;

	//Empty (not yet saved) path name?
	CString DocPathName = pDoc->GetPathName();
	if (DocPathName.IsEmpty())
	{
		//Ask whether to save and proceed OR to cancel the action
		if (AfxMessageBox(STE_DOCUMENT_SAVEBEFOREPROCEED, MB_ICONINFORMATION | MB_OKCANCEL) == IDCANCEL)
			return;

		//Save it
		if (!pDoc->DoSave(DocPathName)) return;

		DocPathName = pDoc->GetPathName();
	}

	//Is it a latex file?
	if (CPathTool::GetFileExtension(DocPathName) != _T("tex")) return;

	//Do we have an opened project?
	CLaTeXProject* pLProject = GetProject();
	if (pLProject)
	{
		//We just care, if the existing main file is the same as the new main file
		if (pLProject->GetMainPath() == DocPathName)
		{
			//It is the same ==> the user wants to recreate the current project
			// So we just show him the Properties dialog.
			// This is similar to the behaviour in OnProjectNewFromDocument
			// and saves the user from closing all documents.
			AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_PROJECT_PROPERTIES);
			return;
		}
	}

	m_pProjectManager->OnProjectNewFromDocument(DocPathName);
}

void CTeXnicCenterApp::OnUpdateProjectNewFromFile(CCmdUI* pCmdUI)
{
	bool bEnable = false;

	LaTeXView* pEdit = GetActiveLaTeXView();
	if (pEdit)
	{
		LaTeXDocument* pDoc = pEdit->GetDocument();
		if (pDoc)
		{
			//Is it a latex file?
			bEnable = (CPathTool::GetFileExtension(pDoc->GetPathName()) == _T("tex"));
		}
	}

	pCmdUI->Enable(bEnable);
	//	pCmdUI->Enable(m_pLatexDocTemplate->GetFirstDocPosition() != NULL);
}

void CTeXnicCenterApp::FindPackageFiles()
{
	CString strSearchDir = CString(GetWorkingDir() + _T("\\packages"));
	FindPackageFilesRecursive(strSearchDir);
	// TODO: Issue warning, if no files/commands have been found
	if (!m_AvailableCommands.GetNoOfFiles())
	{

	}
}

void CTeXnicCenterApp::FindPackageFilesRecursive(CString dir)
{
	CFileFind finder;
	BOOL bWorking = finder.FindFile(dir + _T("\\*"));

	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		CString name(finder.GetFileName());

		if (finder.IsDirectory() && !finder.IsDots())
		{
			FindPackageFilesRecursive(dir + _T("\\") + name);

		}
		else
		{
			CString p(finder.GetFilePath());
			CString ext = CPathTool::GetFileExtension(name);

			if (!ext.CompareNoCase(_T("xml")))
			{
				TRACE1("Adding package file: %s...\n", (LPCTSTR)CPathTool::GetFileTitle(p));
				m_AvailableCommands.LoadFromXML(p, true);
			}
		}
	}
}

void CTeXnicCenterApp::OnPackageSetup()
{
	CString title(AfxLoadString(IDS_SAVE_PACKAGE_AS));
	CString initialDir(GetWorkingDir() + _T("\\packages"));

	CFolderSelect fsel(AfxLoadString(IDS_SELECT_PACKAGE_DIR) /*, "D:\\Programme\\texmf\\tex"*/);

	if (fsel.DoModal() == IDOK)
	{
		CStyleFileContainer NewCommands;
		NewCommands.ClearSearchPath();
		NewCommands.AddSearchPath(CString(fsel.GetPath()));

		CPackageScanProgress prg;
		NewCommands.SetEventListener(&prg);

		prg.ShowWindow(SW_SHOW);
		prg.RedrawWindow(); //Just to be sure that it gets drawn at the beginning.
		const bool bResult = NewCommands.FindStyleFiles();
		//prg.CloseWindow();

		//Canceled?
		if (!bResult) return;

		//Needs to be implemented: m_AvailableCommands.Merge(NewCommands);

		//What about something like this:
		//if (MsgBox("n files have been scanned and m commands have been detected. They are available for this session.\
		// If they shall be available in future session, you need to save them. Do you want to save them?"))

		CFileDialog fselxml(FALSE,
		                    _T("xml"),
		                    _T("packages.xml"),
		                    OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		                    AfxLoadString(STE_FILE_XMLFILTER),
		                    NULL);

		fselxml.m_ofn.lpstrTitle = (LPCTSTR)title;
		fselxml.m_ofn.lpstrInitialDir = (LPCTSTR)initialDir;

		if (fselxml.DoModal() == IDOK)
		{
			CString s = fselxml.GetPathName();
			NewCommands.SaveAsXML(s);
		}
	}
}

void CTeXnicCenterApp::OnUpdateDoForAllOpenWindows(CCmdUI* pCmdUI)
{
	CMultiDocTemplate* pDocTemplate = GetLatexDocTemplate();
	if (!pDocTemplate)
	{
		pCmdUI->Enable(FALSE);
	}
	else
	{
		pCmdUI->Enable(pDocTemplate->GetFirstDocPosition() != NULL);
	}
}

UINT CTeXnicCenterApp::GetApplicationLook() const
{
	return m_nApplicationLook;
}

void CTeXnicCenterApp::SetApplicationLook(UINT val)
{
	m_nApplicationLook = val;
	WriteProfileInt(_T("Settings"),_T("ApplicationLook"),m_nApplicationLook);
}

const CString& CTeXnicCenterApp::GetModuleFileName() const
{
	if (module_name_.IsEmpty())
	{
		::GetModuleFileName(0,module_name_.GetBuffer(MAX_PATH),MAX_PATH);
		module_name_.ReleaseBuffer();
	}

	return module_name_;
}

int CTeXnicCenterApp::DoMessageBox(LPCTSTR prompt, UINT nType, UINT nIDPrompt)
{
	int result(0);

	// disable windows for modal dialog
	DoEnableModeless(FALSE);

	HWND hWndTop;
	HWND hWnd = CWnd::GetSafeOwner_(NULL, &hWndTop);

	// re-enable the parent window, so that focus is restored
	// correctly when the dialog is dismissed.
	if (hWnd != hWndTop)
		EnableWindow(hWnd, TRUE);

	// set help context if possible
	DWORD* pdwContext = NULL;

	DWORD dwWndPid=0;
	GetWindowThreadProcessId(hWnd,&dwWndPid);

	if (hWnd != NULL && dwWndPid==GetCurrentProcessId() )
	{
		// use app-level context or frame level context
		LRESULT lResult = ::SendMessage(hWnd, WM_HELPPROMPTADDR, 0, 0);
		if (lResult != 0)
			pdwContext = (DWORD*)lResult;
	}

	// for backward compatibility use app context if possible
	if (pdwContext == NULL)
		pdwContext = &m_dwPromptContext;

	DWORD dwOldPromptContext = 0;

	if (pdwContext != NULL)
	{
		// save old prompt context for restoration later
		dwOldPromptContext = *pdwContext;
		if (nIDPrompt != 0)
			*pdwContext = HID_BASE_PROMPT+nIDPrompt;
	}

	// determine icon based on type specified
	if ((nType & MB_ICONMASK) == 0)
	{
		switch (nType & MB_TYPEMASK)
		{
		case MB_OK:
		case MB_OKCANCEL:
			nType |= MB_ICONEXCLAMATION;
			break;

		case MB_YESNO:
		case MB_YESNOCANCEL:
			nType |= MB_ICONQUESTION;
			break;

		case MB_ABORTRETRYIGNORE:
		case MB_RETRYCANCEL:
			// No default icon for these types, since they are rarely used.
			// The caller should specify the icon.
			break;
		}
	}

#ifdef _DEBUG
		if ((nType & MB_ICONMASK) == 0)
			TRACE(traceAppMsg, 0, "Warning: no icon specified for message box.\n");
#endif

	VERIFY(DoTaskDialog(hWnd,prompt,nType,result));

	// restore prompt context if possible
	if (pdwContext != NULL)
		*pdwContext = dwOldPromptContext;

	// re-enable windows
	if (hWndTop != NULL)
		::EnableWindow(hWndTop, TRUE);

	DoEnableModeless(TRUE);

	return result;
}

bool CTeXnicCenterApp::DoTaskDialog( HWND hWnd, LPCTSTR prompt, UINT nType, int& result)
{
	TASKDIALOGCONFIG tdc = {sizeof(TASKDIALOGCONFIG)};
	
	const CStringW window_title(MAKEINTRESOURCE(IDR_MAINFRAME)); // Title: TeXnicCenter

	tdc.pszWindowTitle = window_title;
	tdc.hwndParent = hWnd;

	switch (nType & MB_TYPEMASK)
	{
		case MB_OK:
			tdc.dwCommonButtons |= TDCBF_OK_BUTTON;
			break;
		case MB_OKCANCEL:
			tdc.dwCommonButtons |= TDCBF_OK_BUTTON|TDCBF_CANCEL_BUTTON;
			break;
		case MB_YESNO:
			tdc.dwCommonButtons |= TDCBF_YES_BUTTON|TDCBF_NO_BUTTON;
			break;
		case MB_YESNOCANCEL:
			tdc.dwCommonButtons |= TDCBF_YES_BUTTON|TDCBF_NO_BUTTON|TDCBF_CANCEL_BUTTON;
			break;
		case MB_ABORTRETRYIGNORE: // Shouldn't be used at all
			tdc.dwCommonButtons |= TDCBF_RETRY_BUTTON|TDCBF_CANCEL_BUTTON|TDCBF_OK_BUTTON;
			break;
		default:
			ASSERT(FALSE);
	}

#pragma region Icon translation

	switch (nType & MB_ICONMASK)
	{
		case MB_ICONERROR:
			tdc.pszMainIcon = TD_ERROR_ICON;
			break;
		case MB_ICONEXCLAMATION:
			tdc.pszMainIcon = TD_WARNING_ICON;
			break;
		case MB_ICONQUESTION:
			tdc.pszMainIcon =
#ifdef UNICODE
				IDI_QUESTION;
#else
				MAKEINTRESOURCEW(32514);
#endif
			break;
		case MB_ICONINFORMATION:
			tdc.pszMainIcon = TD_INFORMATION_ICON;
			break;
	}

#pragma endregion

	CStringW title, body;	

	// Now, try to split up the text into a title and body by
	// searching for the first occurrence of a double new line
	if (LPCTSTR b = _tcsstr(prompt,_T("\n\n")))
	{
#ifdef UNICODE
		title.SetString(prompt,b - prompt);
		body.SetString(b + 2);
#else
		title = CStringA(prompt,b - prompt);
		body = CStringA(b + 2);
#endif
	}
	else
		title = prompt;

	tdc.pszMainInstruction = title;

	if (!body.IsEmpty())
		tdc.pszContent = body;

	return SUCCEEDED(::TaskDialogIndirect(&tdc,&result,0,0));
}

void CTeXnicCenterApp::OnBibTeXNew()
{
	bibtex_doc_template_->OpenDocumentFile(0);
}

void CTeXnicCenterApp::OnMetaPostNew()
{
	metapost_doc_template_->OpenDocumentFile(0);
}

CDocTemplate* CTeXnicCenterApp::GetBibTeXDocTemplate() const
{
	return bibtex_doc_template_;
}

CDocTemplate* CTeXnicCenterApp::GetMetaPostDocTemplate() const
{
	return metapost_doc_template_;
}

bool CTeXnicCenterApp::GetShowMDITabs() const
{
	return m_bMDITabs != 0;
}

void CTeXnicCenterApp::SetShowMDITabs( bool val )
{
	m_bMDITabs = val;
}

bool CTeXnicCenterApp::HasOpenDocuments() const
{
	bool found = false;
	POSITION pos = GetFirstDocTemplatePosition();

	while (pos && !found)
	{
		if (CDocTemplate* t = GetNextDocTemplate(pos))
		{
			POSITION pos1 = t->GetFirstDocPosition();

			if (pos1 && t->GetNextDoc(pos1))
				found = true;
		}
	}

	return found;
}

const CString CTeXnicCenterApp::GetDocTemplateFilter( CDocTemplate* doc)
{
	CString text;

	doc->GetDocString(text, CDocTemplate::filterName);

	CString filter = text + _T('|');

	doc->GetDocString(text, CDocTemplate::filterExt);

	int index;
	CString ext;

	do
	{
		index = text.Find(_T(';'));

		if (index == -1)
			index = text.GetLength();

		ext = text.Left(index);
		filter += _T('*') + ext;

		if (index < text.GetLength())
		{
			filter += _T(';');
			text = text.Mid(index + 1);
		}
		else
			text.Empty();

	}
	while (!text.IsEmpty());

	filter += _T('|');	
	
	return filter;
}

void CTeXnicCenterApp::ResetSpeller()
{
	GetSpellerThread()->ResetSpeller(spellerSource_.get());
}
