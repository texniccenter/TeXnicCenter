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
#include "LatexProject.h"
#include "Profile.h"
#include "ProfileExchangeDialog.h"
#include "RegistryStack.h"
#include "TeXnicCenter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


std::auto_ptr<CProfileMap> CProfileMap::impl_;

//-------------------------------------------------------------------
// class CProfileArray
//-------------------------------------------------------------------

CProfileMap::CProfileMap()
{
}

CProfileMap::~CProfileMap()
{
	RemoveAll();
}

BOOL CProfileMap::Add(LPCTSTR lpszKey, const CProfile &profile)
{
	CProfile *pProfile = new CProfile(profile);
	if (!pProfile)
		return FALSE;

	if (Exists(lpszKey))
		return FALSE;

	SetAt(lpszKey, pProfile);

	// if there is no active profile till now, select the new one
	if (m_strActiveProfile.IsEmpty())
		m_strActiveProfile = lpszKey;

	return TRUE;
}

void CProfileMap::Remove(LPCTSTR lpszKey)
{
	CProfile *pProfile = NULL;

	if (!Lookup(lpszKey, pProfile))
		return;

	delete pProfile;
	RemoveKey(lpszKey);

	// if deleted profile was the active one, then select the first
	// available profile
	if (m_strActiveProfile == lpszKey)
	{
		POSITION pos = GetStartPosition();
		if (!pos)
		{
			m_strActiveProfile.Empty();
			return;
		}

		CString strKey;
		GetNextAssoc(pos, strKey, pProfile);
		m_strActiveProfile = strKey;
	}
}

void CProfileMap::RemoveAll()
{
	POSITION pos = GetStartPosition();
	CString strKey;
	CProfile *pProfile;

	while (pos)
	{
		GetNextAssoc(pos, strKey, pProfile);
		delete pProfile;
	}

	CMap<CString, LPCTSTR, CProfile*, CProfile*>::RemoveAll();

	m_strActiveProfile.Empty();
}

BOOL CProfileMap::Exists(LPCTSTR lpszKey) const
{
	CProfile *pTemp;
	return Lookup(lpszKey, pTemp);
}

BOOL CProfileMap::Rename(LPCTSTR lpszOldKey, LPCTSTR lpszNewKey)
{
	if (Exists(lpszNewKey))
		return FALSE;
	if (!Exists(lpszOldKey))
		return FALSE;

	CProfile* pTemp = NULL;
	if (Lookup(lpszOldKey, pTemp))
	{
		RemoveKey(lpszOldKey);
		SetAt(lpszNewKey, pTemp);
	}

	// hold active profile
	if (m_strActiveProfile == lpszOldKey)
		m_strActiveProfile = lpszNewKey;

	return TRUE;
}

BOOL CProfileMap::SetActiveProfile(LPCTSTR lpszKey, bool bSetModifiedProject/* = true*/)
{
	if (!Exists(lpszKey))
		return FALSE;

	m_strActiveProfile = lpszKey;
	if (bSetModifiedProject)
	{
		//Set the ModifiedFlag of the project, because this change needs to be saved.
		CLaTeXProject* pLProject = ((CTeXnicCenterApp*)AfxGetApp())->GetProject();
		if (pLProject) pLProject->SetModifiedFlag(true);
	}
	return TRUE;
}

CProfile *CProfileMap::GetActiveProfile()
{
	CProfile *pProfile;
	if (Lookup(m_strActiveProfile, pProfile))
		return pProfile;
	else
		return NULL;
}

CString CProfileMap::GetActiveProfileKey() const
{
	if (!Exists(m_strActiveProfile))
		m_strActiveProfile.Empty();

	return m_strActiveProfile;
}

void CProfileMap::GetKeyList(CStringArray &astrKeys) const
{
	astrKeys.RemoveAll();

	POSITION pos = GetStartPosition();
	CString strKey;
	CProfile *pProfile;

	while (pos)
	{
		GetNextAssoc(pos, strKey, pProfile);
		astrKeys.Add(strKey);
	}
}

void CProfileMap::CopyFrom(const CProfileMap &profiles, BOOL bEmptyBeforeCopy /* = TRUE */, BOOL bAskUserToOverwrite /* = FALSE */)
{
	// remove all existing items if necessary
	if (bEmptyBeforeCopy)
		RemoveAll();

	POSITION pos = profiles.GetStartPosition();
	while (pos)
	{
		CString strKey;
		CProfile *pProfile = NULL;
		profiles.GetNextAssoc(pos, strKey, pProfile);

		// check if a profile with this name is already defined and
		// ask the user to overwrite it, if necessary
		if (bAskUserToOverwrite && Exists(strKey))
		{
			CString strMsg;
			strMsg.Format(STE_OUTPUTWIZARD_OUTPUTTYPEEXISTS, (LPCTSTR)strKey);
			if (AfxMessageBox(strMsg, MB_YESNO | MB_ICONQUESTION) != IDYES)
				// skip this item
				continue;
		}

		// create a copy and add it
		CProfile *pNewProfile = new CProfile(*pProfile);
		if (pNewProfile)
			SetAt(strKey, pNewProfile);
	}

	// adjust active profile setting for this map if necessary
	if (!Exists(GetActiveProfileKey()) && !IsEmpty())
	{
		pos = GetStartPosition();

		CString strKey;
		CProfile *pProfile;
		GetNextAssoc(pos, strKey, pProfile);

		SetActiveProfile(strKey);
	}
}

void CProfileMap::RemoveDirectorySpecifications()
{
	POSITION pos = GetStartPosition();
	while (pos)
	{
		CString strKey;
		CProfile *pProfile;
		GetNextAssoc(pos, strKey, pProfile);
		if (pProfile)
			pProfile->RemoveDirectorySpecifications();
	}
}

void CProfileMap::Import()
{
	CFileDialog fileDlg(TRUE, _T("tco"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, CString((LPCTSTR)STE_PROFILE_FILE_FILTER));
	if (fileDlg.DoModal() != IDOK)
		return;

	CProfileMap profiles;
	if (!profiles.LoadXml(fileDlg.GetPathName()))
		return;

	CProfileExchangeDialog dlg(profiles, FALSE);
	if (dlg.DoModal() != IDOK)
		return;

	CopyFrom(profiles, FALSE, TRUE);
}

void CProfileMap::Export() const
{
	CProfileMap profiles;
	profiles.CopyFrom(*this);

	CProfileExchangeDialog dlg(profiles, TRUE);
	if (dlg.DoModal() != IDOK)
		return;

	CFileDialog fileDlg(FALSE, _T("tco"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, CString((LPCTSTR)STE_PROFILE_FILE_FILTER));
	if (fileDlg.DoModal() != IDOK)
		return;

	profiles.SaveXml(fileDlg.GetPathName());
}

BOOL CProfileMap::SerializeToRegistry() const
{
	RegistryStack reg(FALSE, FALSE);
	CString strKey(CPathTool::Cat(theApp.m_strRegistryRoot, _T("Profiles")));
	reg.DeleteKey(strKey);
	reg.CreateKey(strKey);

	reg.Write(_T("ActiveProfile"), m_strActiveProfile);

	POSITION pos = GetStartPosition();
	while (pos)
	{
		CString strTitle;
		CProfile *pTemp;
		GetNextAssoc(pos, strTitle, pTemp);
		reg.PushKey();
		reg.CreateKey(strTitle);
		pTemp->SerializeToRegistry(reg);
		reg.PopKey();
	}

	return TRUE;
}

BOOL CProfileMap::SerializeFromRegistry()
{
	RemoveAll();

	RegistryStack reg(FALSE, TRUE);
	CString strKey(CPathTool::Cat(theApp.m_strRegistryRoot, _T("Profiles")));
	if (!reg.Open(strKey))
		return FALSE;

	CStringArray astrKeys;
	CProfile *pTemp;
	ReadSubKeys(reg.GetRegKey(),astrKeys);
	reg.PushKey();

	reg.Read(_T("ActiveProfile"), m_strActiveProfile);
	if (m_strActiveProfile.IsEmpty() && astrKeys.GetSize())
		m_strActiveProfile = astrKeys[0];

	for (int i = 0; i < astrKeys.GetSize(); i++)
	{
		if (Exists(astrKeys[i]))
			// key already exists -> continue with next element
			continue;

		reg.TopKey();
		reg.Open(astrKeys[i]);
		pTemp = new CProfile;
		pTemp->SerializeFromRegistry(reg);

		SetAt(astrKeys[i], pTemp);
	}

	reg.PopKey();

	return TRUE;
}

BOOL CProfileMap::LoadXml(LPCTSTR lpszPath)
{
	MsXml::CXMLDOMDocumentEx xmlDoc;
	try
	{
		xmlDoc.Create(
		    _T("xmlns:txcop='http://schemas.ToolsCenter.org/TeXnicCenter/OutputProfiles.xsd'"),
		    CPathTool::Cat(theApp.GetWorkingDir(), _T("OutputProfiles.xsd")),
		    _T("http://schemas.ToolsCenter.org/TeXnicCenter/OutputProfiles.xsd"));

		if (!xmlDoc.Load(lpszPath)) AfxThrowComException(S_FALSE);

		RemoveAll();

		//Get the version
		const int Version = xmlDoc.SelectSingleNode(_T("txcop:outputProfiles")).GetAttributes().GetNamedItem(_T("version")).GetNodeTypedValue();

		MsXml::CXMLDOMNodeList xmlProfiles(xmlDoc.SelectNodes(_T("txcop:outputProfiles/outputProfileList/outputProfile")));
		const long lProfiles = xmlProfiles.GetLength();
		for (long lProfile = 0; lProfile < lProfiles; ++lProfile)
		{
			MsXml::CXMLDOMElement xmlProfile(xmlProfiles.GetItem(lProfile).QueryInterface(IID_IXMLDOMElement));
			CProfile *pProfile = new CProfile();
			pProfile->LoadXml(xmlProfile, Version);
			SetAt((_bstr_t)xmlProfile.GetAttribute(_T("name")), pProfile);
		}

		return TRUE;
	}
	catch (CComException *pE)
	{
		bool bReportedError(false);

		//Ask MsXML for error description
		try
		{
			if (xmlDoc && xmlDoc.GetParseError() && xmlDoc.GetParseError().GetErrorCode() != 0)
			{
				MsXml::CXMLDOMParseError xmlError(xmlDoc.GetParseError());
				CString strErrorMsg;
				strErrorMsg.Format(STE_XML_PARSE_ERROR,
				                   xmlError.GetErrorCode(), (LPCTSTR)xmlError.GetReason(),
				                   (LPCTSTR)xmlError.GetUrl(), xmlError.GetLine(), xmlError.GetLinepos(),
				                   (LPCTSTR)xmlError.GetSrcText());

				AfxMessageBox(strErrorMsg, MB_ICONEXCLAMATION | MB_OK);
				bReportedError = true;
			}
		}
		catch (CComException* pENew)
		{
			pENew->Delete();
		}


		//Report Error, if MsXML did not respond
		if (!bReportedError)
		{
			if (pE->GetDescription().IsEmpty())
			{
				//A generic COM error.
				//For example, MsXML not installed.

				//Inform the user, that he needs to install MsXML.
				AfxMessageBox(STE_XML_INSTALLNEEDED, MB_ICONINFORMATION | MB_OK);
			}
			else
			{
				//Some other error. For example, schema file is missing (*.xsd)
				pE->ReportError(MB_ICONEXCLAMATION | MB_OK);
			}
		}

		pE->Delete();
		return FALSE;
	}
}

BOOL CProfileMap::SaveXml(LPCTSTR lpszPath) const
{
	MsXml::CXMLDOMDocumentEx xmlDoc;
	try
	{
		xmlDoc.Create(
		    _T("xmlns:txcop='http://schemas.ToolsCenter.org/TeXnicCenter/OutputProfiles.xsd'"),
		    CPathTool::Cat(theApp.GetWorkingDir(), _T("OutputProfiles.xsd")),
		    _T("http://schemas.ToolsCenter.org/TeXnicCenter/OutputProfiles.xsd"));

		MsXml::CXMLDOMElement xmlRoot(xmlDoc.CreateElement(_T("txcop:outputProfiles")));
		xmlRoot.SetAttribute(_T("xmlns:txcop"), _T("http://schemas.ToolsCenter.org/TeXnicCenter/OutputProfiles.xsd"));
		xmlRoot.SetAttribute(_T("version"), 3.0);
		xmlDoc.AppendChild(xmlRoot);

		MsXml::CXMLDOMElement xmlProfileCollection(xmlDoc.CreateElement(_T("outputProfileList")));
		xmlRoot.AppendChild(xmlProfileCollection);

		// write all profiles to collection
		CStringArray astrProfiles;
		GetKeyList(astrProfiles);
		for (int i = 0; i < astrProfiles.GetSize(); ++i)
		{
			CProfile* pProfile = NULL;
			if (!Lookup(astrProfiles[i], pProfile) || !pProfile)
				continue;

			MsXml::CXMLDOMElement xmlProfile(xmlDoc.CreateElement(_T("outputProfile")));
			xmlProfile.SetAttribute(_T("name"), (LPCTSTR)astrProfiles[i]);
			pProfile->SaveXml(xmlProfile);
			xmlProfileCollection.AppendChild(xmlProfile);
		}

		// write to file
		xmlDoc.SavePretty(lpszPath);

		return TRUE;
	}
	catch (CComException* pE)
	{
		if (pE->GetDescription().IsEmpty())
		{
			//A generic COM error.
			//For example, MsXML not installed.

			//Inform the user, that he needs to install MsXML.
			AfxMessageBox(STE_XML_INSTALLNEEDED, MB_ICONINFORMATION | MB_OK);
		}
		else
		{
			//Some other error. For example, schema file is missing (*.xsd)
			pE->ReportError(MB_ICONEXCLAMATION | MB_OK);
		}

		pE->Delete();
		return FALSE;
	}
}

CProfileMap* CProfileMap::GetInstance()
{
	if (!impl_.get())
		impl_.reset(new CProfileMap);

	return impl_.get();
}

//-------------------------------------------------------------------
// class CProfile
//-------------------------------------------------------------------

CProfile::CProfile()
		: m_bRunLatex(FALSE),
		m_bStopOnLatexError(FALSE),
		m_bRunBibTex(FALSE),
		m_bRunMakeIndex(FALSE),
		m_bCloseBeforeCompilation(FALSE)
{
}

CProfile::CProfile(const CProfile &p)
{
	*this = p;
}

CProfile::~CProfile()
{
}

CProfile &CProfile::operator=(const CProfile &p)
{
	m_aPreProcessors.RemoveAll();
	m_aPreProcessors.Append(p.m_aPreProcessors);
	m_aPostProcessors.RemoveAll();
	m_aPostProcessors.Append(p.m_aPostProcessors);
	m_aPreviewProcessors.RemoveAll();
	m_aPreviewProcessors.Append(p.m_aPreviewProcessors);

	m_bCloseBeforeCompilation = p.m_bCloseBeforeCompilation;
	m_bRunLatex = p.m_bRunLatex;
	m_bStopOnLatexError = p.m_bStopOnLatexError;
	m_bRunBibTex = p.m_bRunBibTex;
	m_bRunMakeIndex = p.m_bRunMakeIndex;
	m_cmdCloseView = p.m_cmdCloseView;
	m_cmdViewCurrent = p.m_cmdViewCurrent;
	m_cmdViewProject = p.m_cmdViewProject;
	m_strLatexPath = p.m_strLatexPath;
	m_strLatexArguments = p.m_strLatexArguments;
	m_strBibTexPath = p.m_strBibTexPath;
	m_strBibTexArguments = p.m_strBibTexArguments;
	m_strMakeIndexPath = p.m_strMakeIndexPath;
	m_strMakeIndexArguments = p.m_strMakeIndexArguments;
	m_strViewerPath = p.m_strViewerPath;
	m_strPreviewImagePath = p.m_strPreviewImagePath;

	return *this;
}

void CProfile::SetLatexPath(LPCTSTR lpszPath, LPCTSTR lpszArguments,
                            BOOL bUseLatex /*= TRUE*/, BOOL bStopOnLatexError /*= FALSE*/)
{
	m_strLatexPath = lpszPath;
	m_strLatexArguments = lpszArguments;
	m_bRunLatex = bUseLatex;
	m_bStopOnLatexError = bStopOnLatexError;
}

void CProfile::SetBibTexPath(LPCTSTR lpszPath, LPCTSTR lpszArguments,
                             BOOL bUseBibTex /*= TRUE*/)
{
	m_strBibTexPath = lpszPath;
	m_strBibTexArguments = lpszArguments;
	m_bRunBibTex = bUseBibTex;
}

void CProfile::SetMakeIndexPath(LPCTSTR lpszPath, LPCTSTR lpszArguments,
                                BOOL bUseMakeIndex /*= TRUE*/)
{
	m_strMakeIndexPath = lpszPath;
	m_strMakeIndexArguments = lpszArguments;
	m_bRunMakeIndex = bUseMakeIndex;
}

void CProfile::SetPreviewImagePath(LPCTSTR lpszPreviewPath)
{
	m_strPreviewImagePath = lpszPreviewPath;
}

void CProfile::SetViewerPath(LPCTSTR lpszViewerPath)
{
	m_strViewerPath = lpszViewerPath;
}

void CProfile::SetViewProjectCmd(const CProfile::CCommand &cmd)
{
	m_cmdViewProject = cmd;
}

void CProfile::SetViewCurrentCmd(const CProfile::CCommand &cmd)
{
	m_cmdViewCurrent = cmd;
}

void CProfile::SetCloseView(BOOL bClose)
{
	m_bCloseBeforeCompilation = bClose;
}

void CProfile::SetViewCloseCmd(const CProfile::CCommand &cmd)
{
	m_cmdCloseView = cmd;
}

void CProfile::RemoveDirectorySpecifications()
{
	m_strLatexPath = CPathTool::GetFile(m_strLatexPath);
	m_strBibTexPath = CPathTool::GetFile(m_strBibTexPath);
	m_strMakeIndexPath = CPathTool::GetFile(m_strMakeIndexPath);

	m_aPreProcessors.RemoveDirectorySpecifications();
	m_aPostProcessors.RemoveDirectorySpecifications();
	m_aPreviewProcessors.RemoveDirectorySpecifications();

	m_strPreviewImagePath = CPathTool::GetFile(m_strPreviewImagePath);
	m_strViewerPath = CPathTool::GetFile(m_strViewerPath);
	m_cmdViewProject.RemoveDirectorySpecifications();
	m_cmdViewCurrent.RemoveDirectorySpecifications();
	m_cmdCloseView.RemoveDirectorySpecifications();
}

BOOL CProfile::SerializeToRegistry(RegistryStack &reg) const
{
	reg.Write(_T("RunLatex"), m_bRunLatex);
	reg.Write(_T("LatexPath"), m_strLatexPath);
	reg.Write(_T("LatexArgs"), m_strLatexArguments);
	reg.Write(_T("LatexStopOnError"), m_bStopOnLatexError);

	reg.Write(_T("RunBibTex"), m_bRunBibTex);
	reg.Write(_T("BibTexPath"), m_strBibTexPath);
	reg.Write(_T("BibTexArgs"), m_strBibTexArguments);

	reg.Write(_T("RunMakeIndex"), m_bRunMakeIndex);
	reg.Write(_T("MakeIndexPath"), m_strMakeIndexPath);
	reg.Write(_T("MakeIndexArgs"), m_strMakeIndexArguments);

	reg.Write(_T("PreviewImagePath"), m_strPreviewImagePath);

	reg.Write(_T("ViewerPath"), m_strViewerPath);
	reg.Write(_T("CloseViewBeforeCompilation"), m_bCloseBeforeCompilation);

	reg.PushKey();
	reg.CreateKey(_T("PreProcessors"));
	m_aPreProcessors.SerializeToRegistry(reg, _T("PreProcessor"));

	reg.TopKey();
	reg.CreateKey(_T("PostProcessors"));
	m_aPostProcessors.SerializeToRegistry(reg, _T("PostProcessor"));

	reg.TopKey();
	reg.CreateKey(_T("PreviewProcessors"));
	m_aPreviewProcessors.SerializeToRegistry(reg, _T("PreviewProcessor"));

	reg.TopKey();
	reg.CreateKey(_T("ViewProjectCmd"));
	m_cmdViewProject.SerializeToRegistry(reg);

	reg.TopKey();
	reg.CreateKey(_T("ViewCurrentCmd"));
	m_cmdViewCurrent.SerializeToRegistry(reg);

	reg.TopKey();
	reg.CreateKey(_T("ViewCloseCmd"));
	m_cmdCloseView.SerializeToRegistry(reg);

	reg.PopKey();

	return TRUE;
}

BOOL CProfile::SerializeFromRegistry(RegistryStack &reg)
{
	reg.Read(_T("RunLatex"), m_bRunLatex);
	reg.Read(_T("LatexPath"), m_strLatexPath);
	reg.Read(_T("LatexArgs"), m_strLatexArguments);
	reg.Read(_T("LatexStopOnError"), m_bStopOnLatexError);

	reg.Read(_T("RunBibTex"), m_bRunBibTex);
	reg.Read(_T("BibTexPath"), m_strBibTexPath);
	reg.Read(_T("BibTexArgs"), m_strBibTexArguments);

	reg.Read(_T("RunMakeIndex"), m_bRunMakeIndex);
	reg.Read(_T("MakeIndexPath"), m_strMakeIndexPath);
	reg.Read(_T("MakeIndexArgs"), m_strMakeIndexArguments);

	reg.Read(_T("PreviewImagePath"), m_strPreviewImagePath);

	reg.Read(_T("ViewerPath"), m_strViewerPath);
	reg.Read(_T("CloseViewBeforeCompilation"), m_bCloseBeforeCompilation);

	reg.PushKey();
	reg.Open(_T("PreProcessors"));
	m_aPreProcessors.SerializeFromRegistry(reg, _T("PreProcessor"));

	reg.TopKey();
	reg.Open(_T("PostProcessors"));
	m_aPostProcessors.SerializeFromRegistry(reg, _T("PostProcessor"));

	reg.TopKey();
	reg.Open(_T("PreviewProcessors"));
	m_aPreviewProcessors.SerializeFromRegistry(reg, _T("PreviewProcessor"));

	reg.TopKey();
	reg.Open(_T("ViewProjectCmd"));
	m_cmdViewProject.SerializeFromRegistry(reg);

	reg.TopKey();
	reg.Open(_T("ViewCurrentCmd"));
	m_cmdViewCurrent.SerializeFromRegistry(reg);

	reg.TopKey();
	reg.Open(_T("ViewCloseCmd"));
	m_cmdCloseView.SerializeFromRegistry(reg);

	reg.PopKey();

	return TRUE;
}

void CProfile::SaveXml(MsXml::CXMLDOMElement &xmlProfile) const
{
	// serialize general settings
	xmlProfile.SetAttribute(_T("stopOnLatexError"), m_bStopOnLatexError ? _T("true") : _T("false"));

	// serialize LaTeX settings
	MsXml::CXMLDOMElement xmlLatexCommand(xmlProfile.GetOwnerDocument().CreateElement(_T("texCommand")));
	xmlLatexCommand.SetAttribute(_T("execute"), m_bRunLatex ? _T("true") : _T("false"));
	xmlLatexCommand.SetAttribute(_T("path"), (LPCTSTR)m_strLatexPath);
	xmlLatexCommand.SetAttribute(_T("arguments"), (LPCTSTR)m_strLatexArguments);
	xmlProfile.AppendChild(xmlLatexCommand);

	// serialize BibTeX settings
	MsXml::CXMLDOMElement xmlBibTexCommand(xmlProfile.GetOwnerDocument().CreateElement(_T("bibTexCommand")));
	xmlBibTexCommand.SetAttribute(_T("execute"), m_bRunBibTex ? _T("true") : _T("false"));
	xmlBibTexCommand.SetAttribute(_T("path"), (LPCTSTR)m_strBibTexPath);
	xmlBibTexCommand.SetAttribute(_T("arguments"), (LPCTSTR)m_strBibTexArguments);
	xmlProfile.AppendChild(xmlBibTexCommand);

	// serialize MakeIndex settings
	MsXml::CXMLDOMElement xmlMakeIndexCommand(xmlProfile.GetOwnerDocument().CreateElement(_T("makeIndexCommand")));
	xmlMakeIndexCommand.SetAttribute(_T("execute"), m_bRunMakeIndex ? _T("true") : _T("false"));
	xmlMakeIndexCommand.SetAttribute(_T("path"), (LPCTSTR)m_strMakeIndexPath);
	xmlMakeIndexCommand.SetAttribute(_T("arguments"), (LPCTSTR)m_strMakeIndexArguments);
	xmlProfile.AppendChild(xmlMakeIndexCommand);

	// serialize preprocessors
	MsXml::CXMLDOMElement xmlPreProcessors(xmlProfile.GetOwnerDocument().CreateElement(_T("preProcessors")));
	m_aPreProcessors.SaveXml(xmlPreProcessors);
	xmlProfile.AppendChild(xmlPreProcessors);

	// serialize postprocessors
	MsXml::CXMLDOMElement xmlPostProcessors(xmlProfile.GetOwnerDocument().CreateElement(_T("postProcessors")));
	m_aPostProcessors.SaveXml(xmlPostProcessors);
	xmlProfile.AppendChild(xmlPostProcessors);

	// serialize preview settings and processors
	MsXml::CXMLDOMElement xmlPreviewProcessors(xmlProfile.GetOwnerDocument().CreateElement(_T("previewProcessors")));
	xmlPreviewProcessors.SetAttribute(_T("imagepath"), (LPCTSTR)m_strPreviewImagePath);
	m_aPreviewProcessors.SaveXml(xmlPreviewProcessors);
	xmlProfile.AppendChild(xmlPreviewProcessors);


	//
	// serialize viewer stuff
	//

	MsXml::CXMLDOMElement xmlViewerDefinition(xmlProfile.GetOwnerDocument().CreateElement(_T("viewer")));
	xmlViewerDefinition.SetAttribute(_T("path"), (LPCTSTR)m_strViewerPath);
	xmlViewerDefinition.SetAttribute(_T("closeBeforeCompilation"), m_bCloseBeforeCompilation ? _T("true") : _T("false"));

	// serialize view project command
	MsXml::CXMLDOMElement xmlViewProjectCommand(xmlProfile.GetOwnerDocument().CreateElement(_T("viewProjectCommand")));
	m_cmdViewProject.SaveXml(xmlViewProjectCommand);
	xmlViewerDefinition.AppendChild(xmlViewProjectCommand);

	// serialize view current command
	MsXml::CXMLDOMElement xmlViewCurrentCommand(xmlProfile.GetOwnerDocument().CreateElement(_T("viewCurrentFileCommand")));
	m_cmdViewCurrent.SaveXml(xmlViewCurrentCommand);
	xmlViewerDefinition.AppendChild(xmlViewCurrentCommand);

	// serialize view close command
	MsXml::CXMLDOMElement xmlViewCloseCommand(xmlProfile.GetOwnerDocument().CreateElement(_T("viewCloseCommand")));
	m_cmdCloseView.SaveXml(xmlViewCloseCommand);
	xmlViewerDefinition.AppendChild(xmlViewCloseCommand);

	xmlProfile.AppendChild(xmlViewerDefinition);
}

void CProfile::LoadXml(MsXml::CXMLDOMElement &xmlProfile, const int Version)
{
	// serialize general settings
	m_bStopOnLatexError = (bool)xmlProfile.GetAttribute(_T("stopOnLatexError"));

	// serialize LaTeX settings
	MsXml::CXMLDOMElement xmlLatexCommand(xmlProfile.SelectSingleNode(_T("texCommand")).QueryInterface(IID_IXMLDOMElement));
	m_bRunLatex = (bool)xmlLatexCommand.GetAttribute(_T("execute"));
	m_strLatexPath = (LPCTSTR)(_bstr_t)xmlLatexCommand.GetAttribute(_T("path"));
	m_strLatexArguments = (LPCTSTR)(_bstr_t)xmlLatexCommand.GetAttribute(_T("arguments"));

	// serialize BibTex settings
	MsXml::CXMLDOMElement xmlBibTexCommand(xmlProfile.SelectSingleNode(_T("bibTexCommand")).QueryInterface(IID_IXMLDOMElement));
	m_bRunBibTex = (bool)xmlBibTexCommand.GetAttribute(_T("execute"));
	m_strBibTexPath = (LPCTSTR)(_bstr_t)xmlBibTexCommand.GetAttribute(_T("path"));
	m_strBibTexArguments = (LPCTSTR)(_bstr_t)xmlBibTexCommand.GetAttribute(_T("arguments"));

	// serialize LaTeX settings
	MsXml::CXMLDOMElement xmlMakeIndexCommand(xmlProfile.SelectSingleNode(_T("makeIndexCommand")).QueryInterface(IID_IXMLDOMElement));
	m_bRunMakeIndex = (bool)xmlMakeIndexCommand.GetAttribute(_T("execute"));
	m_strMakeIndexPath = (LPCTSTR)(_bstr_t)xmlMakeIndexCommand.GetAttribute(_T("path"));
	m_strMakeIndexArguments = (LPCTSTR)(_bstr_t)xmlMakeIndexCommand.GetAttribute(_T("arguments"));

	// serialize preprocessors
	if (Version >= 2)
	{
		MsXml::CXMLDOMElement xmlPreProcessors(xmlProfile.SelectSingleNode(_T("preProcessors")).QueryInterface(IID_IXMLDOMElement));
		m_aPreProcessors.LoadXml(xmlPreProcessors);
	}

	// serialize postprocessors
	MsXml::CXMLDOMElement xmlPostProcessors(xmlProfile.SelectSingleNode(_T("postProcessors")).QueryInterface(IID_IXMLDOMElement));
	m_aPostProcessors.LoadXml(xmlPostProcessors);

	// serialize preview processors
	if (Version >= 3)
	{
		MsXml::CXMLDOMElement xmlPreview(xmlProfile.SelectSingleNode(_T("previewProcessors")).QueryInterface(IID_IXMLDOMElement));
		m_strPreviewImagePath = (LPCTSTR)(_bstr_t)xmlPreview.GetAttribute(_T("imagepath"));
		m_aPreviewProcessors.LoadXml(xmlPreview);
	}

	//
	// serialize viewer stuff
	//

	MsXml::CXMLDOMElement xmlViewerDefinition(xmlProfile.SelectSingleNode(_T("viewer")).QueryInterface(IID_IXMLDOMElement));
	m_strViewerPath = (LPCTSTR)(_bstr_t)xmlViewerDefinition.GetAttribute(_T("path"));
	m_bCloseBeforeCompilation = (bool)xmlViewerDefinition.GetAttribute(_T("closeBeforeCompilation"));

	// serialize view project command
	MsXml::CXMLDOMElement xmlViewProjectCommand(xmlViewerDefinition.SelectSingleNode(_T("viewProjectCommand")).QueryInterface(IID_IXMLDOMElement));
	m_cmdViewProject.LoadXml(xmlViewProjectCommand);

	// serialize view current command
	MsXml::CXMLDOMElement xmlViewCurrentCommand(xmlViewerDefinition.SelectSingleNode(_T("viewCurrentFileCommand")).QueryInterface(IID_IXMLDOMElement));
	m_cmdViewCurrent.LoadXml(xmlViewCurrentCommand);

	// serialize view close command
	MsXml::CXMLDOMElement xmlViewCloseCommand(xmlViewerDefinition.SelectSingleNode(_T("viewCloseCommand")).QueryInterface(IID_IXMLDOMElement));
	m_cmdCloseView.LoadXml(xmlViewCloseCommand);
}


//-------------------------------------------------------------------
// class CProfile::CCommand
//-------------------------------------------------------------------

CProfile::CCommand::CCommand()
		: m_nActiveCommand(typeProcess)
{
}

CProfile::CCommand::~CCommand()
{
}

void CProfile::CCommand::SetActiveCommand(int nActiveCommand)
{
	ASSERT(nActiveCommand == typeProcess || nActiveCommand == typeDde);

	m_nActiveCommand = nActiveCommand;
}

void CProfile::CCommand::SetProcessCommand(const CProcessCommand &cmd)
{
	m_cmdProcess = cmd;
}

void CProfile::CCommand::SetDdeCommand(const CDdeCommand &cmd)
{
	m_cmdDde = cmd;
}

void CProfile::CCommand::RemoveDirectorySpecifications()
{
	m_cmdProcess.RemoveDirectorySpecifications();
	m_cmdDde.RemoveDirectorySpecifications();
}

BOOL CProfile::CCommand::SerializeToRegistry(RegistryStack &reg) const
{
	reg.Write(_T("ActiveType"), m_nActiveCommand);
	m_cmdProcess.SerializeToRegistry(_T("ProcessCmd"), reg);
	m_cmdDde.SerializeToRegistry(_T("DDECmd"), reg);

	return TRUE;
}

BOOL CProfile::CCommand::SerializeFromRegistry(RegistryStack &reg)
{
	CString strPackedInformation;
	reg.Read(_T("ActiveType"), m_nActiveCommand);

	if (!m_cmdProcess.SerializeFromRegistry(_T("ProcessCmd"), reg))
	{
		//Try old-school using a serialized string
		if (reg.Read(_T("ProcessCmd"), strPackedInformation))
			m_cmdProcess.SerializeFromStringDeprecated(strPackedInformation);
	}

	if (!m_cmdDde.SerializeFromRegistry(_T("DDECmd"), reg))
	{
		//Try old-school using a serialized string
		if (reg.Read(_T("DDECmd"), strPackedInformation))
			m_cmdDde.SerializeFromStringDeprecated(strPackedInformation);
	}

	return TRUE;
}

void CProfile::CCommand::SaveXml(MsXml::CXMLDOMElement xmlCommand) const
{
	// active command
	xmlCommand.SetAttribute(_T("type"), (m_nActiveCommand == typeDde) ? _T("dde") : _T("commandLine"));

	// command line
	MsXml::CXMLDOMElement xmlCommandLineCommand(xmlCommand.GetOwnerDocument().CreateElement(_T("commandLineCommand")));
	m_cmdProcess.SaveXml(xmlCommandLineCommand);
	xmlCommand.AppendChild(xmlCommandLineCommand);

	// dde
	MsXml::CXMLDOMElement xmlDdeCommand(xmlCommand.GetOwnerDocument().CreateElement(_T("ddeCommand")));
	m_cmdDde.SaveXml(xmlDdeCommand);
	xmlCommand.AppendChild(xmlDdeCommand);
}

void CProfile::CCommand::LoadXml(MsXml::CXMLDOMElement xmlCommand)
{
	// active command
	if (CString(_T("dde")) == (LPCTSTR)(_bstr_t)xmlCommand.GetAttribute(_T("type")))
		m_nActiveCommand = typeDde;
	else
		m_nActiveCommand = typeProcess;

	// command line
	MsXml::CXMLDOMElement xmlCommandLineCommand(xmlCommand.SelectSingleNode(_T("commandLineCommand")).QueryInterface(IID_IXMLDOMElement));
	m_cmdProcess.LoadXml(xmlCommandLineCommand);

	// dde
	MsXml::CXMLDOMElement xmlDdeCommand(xmlCommand.SelectSingleNode(_T("ddeCommand")).QueryInterface(IID_IXMLDOMElement));
	m_cmdDde.LoadXml(xmlDdeCommand);
}