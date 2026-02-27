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
#include "ChildFrm.h"
#include "MainFrm.h"
#include "PlaceHolder.h"
#include "TeXnicCenter.h"
#include "configuration.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//-------------------------------------------------------------------
// global functions
//-------------------------------------------------------------------

CString AfxExpandPlaceholders(LPCTSTR lpszStringWithPlaceholders,
                              LPCTSTR lpszMainPath /*= NULL*/,
                              LPCTSTR lpszCurrentPath /*= NULL*/,
                              long lCurrentLine /*= -1*/,
                              LPCTSTR lpszCurrentSelection,/*= NULL*/
                              bool bExpandPlaceholderSets,/*= false*/
                              LPCTSTR lpszWorkingDir /*= NULL*/)
{
	CString strCmdLine(lpszStringWithPlaceholders);
	CString strLine;

	//Getting the WorkingDir of the project
	CString PathToWorkingDir(lpszWorkingDir);
	CLaTeXProject* pLProject = ((CTeXnicCenterApp*)AfxGetApp())->GetProject();
	if (pLProject && !lpszWorkingDir)
	{
		PathToWorkingDir = pLProject->GetWorkingDirectory();
	}

	CString slashPathToWorkingDir = CPathTool::GetSlashPath(PathToWorkingDir);
	CString shortPathToWorkingDir = CPathTool::GetShortPath(PathToWorkingDir);
	CString shortslashPathToWorkingDir = CPathTool::GetShortPath(slashPathToWorkingDir);

	if (lCurrentLine > -1)
		strLine.Format(_T("%d"),lCurrentLine);



	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// format command line

	// store "%%"
	strCmdLine.Replace(_T("%%"),_T("\a"));

	// replace place holders
	if (lpszMainPath)
	{
		CPathTool main(lpszMainPath);

		main.EnsureBackslashPath();
		main.EnsureLongPath();
		strCmdLine.Replace(_T("%pm"),main.GetPath());
		strCmdLine.Replace(_T("%dm"),main.GetDirectory());
		strCmdLine.Replace(_T("%nm"),main.GetFile());
		strCmdLine.Replace(_T("%tm"),main.GetFileTitle());
		strCmdLine.Replace(_T("%em"),main.GetFileExtension());
		strCmdLine.Replace(_T("%bm"),CPathTool::Cat(main.GetDirectory(),main.GetFileTitle()));
		strCmdLine.Replace(_T("%rm"),main.GetDrive());
		strCmdLine.Replace(_T("%wm"),CPathTool::GetRelativePath(PathToWorkingDir,main.GetPath(),true,false));

		main.EnsureSlashPath();
		strCmdLine.Replace(_T("%Pm"),main.GetPath());
		strCmdLine.Replace(_T("%Dm"),main.GetDirectory());
		strCmdLine.Replace(_T("%Bm"),CPathTool::Cat(main.GetDirectory(),main.GetFileTitle()));
		strCmdLine.Replace(_T("%Wm"),CPathTool::GetRelativePath(slashPathToWorkingDir,main.GetPath(),true,false));

		main.EnsureBackslashPath();
		main.EnsureShortPath();
		strCmdLine.Replace(_T("%spm"),main.GetPath());
		strCmdLine.Replace(_T("%sdm"),main.GetDirectory());
		strCmdLine.Replace(_T("%snm"),main.GetFile());
		strCmdLine.Replace(_T("%stm"),main.GetFileTitle());
		strCmdLine.Replace(_T("%sem"),main.GetFileExtension());
		strCmdLine.Replace(_T("%sbm"),CPathTool::Cat(main.GetDirectory(),main.GetFileTitle()));
		strCmdLine.Replace(_T("%swm"),CPathTool::GetRelativePath(shortPathToWorkingDir,main.GetPath(),true,false));

		main.EnsureSlashPath();
		strCmdLine.Replace(_T("%sPm"),main.GetPath());
		strCmdLine.Replace(_T("%sDm"),main.GetDirectory());
		strCmdLine.Replace(_T("%sBm"),CPathTool::Cat(main.GetDirectory(),main.GetFileTitle()));
		strCmdLine.Replace(_T("%sWm"),CPathTool::GetRelativePath(shortslashPathToWorkingDir,main.GetPath(),true,false));
	}

	if (lpszCurrentPath)
	{
		CPathTool current(lpszCurrentPath);

		current.EnsureBackslashPath();
		current.EnsureLongPath();
		strCmdLine.Replace(_T("%pc"),current.GetPath());
		strCmdLine.Replace(_T("%dc"),current.GetDirectory());
		strCmdLine.Replace(_T("%nc"),current.GetFile());
		strCmdLine.Replace(_T("%tc"),current.GetFileTitle());
		strCmdLine.Replace(_T("%ec"),current.GetFileExtension());
		strCmdLine.Replace(_T("%bc"),CPathTool::Cat(current.GetDirectory(),current.GetFileTitle()));
		strCmdLine.Replace(_T("%rc"),current.GetDrive());
		strCmdLine.Replace(_T("%wc"),CPathTool::GetRelativePath(PathToWorkingDir,current.GetPath(),true,false));

		current.EnsureSlashPath();
		strCmdLine.Replace(_T("%Pc"),current.GetPath());
		strCmdLine.Replace(_T("%Dc"),current.GetDirectory());
		strCmdLine.Replace(_T("%Bc"),CPathTool::Cat(current.GetDirectory(),current.GetFileTitle()));
		strCmdLine.Replace(_T("%Wc"),CPathTool::GetRelativePath(slashPathToWorkingDir,current.GetPath(),true,false));

		current.EnsureBackslashPath();
		current.EnsureShortPath();
		strCmdLine.Replace(_T("%spc"),current.GetPath());
		strCmdLine.Replace(_T("%sdc"),current.GetDirectory());
		strCmdLine.Replace(_T("%snc"),current.GetFile());
		strCmdLine.Replace(_T("%stc"),current.GetFileTitle());
		strCmdLine.Replace(_T("%sec"),current.GetFileExtension());
		strCmdLine.Replace(_T("%sbc"),CPathTool::Cat(current.GetDirectory(),current.GetFileTitle()));
		strCmdLine.Replace(_T("%swc"),CPathTool::GetRelativePath(shortPathToWorkingDir,current.GetPath(),true,false));

		current.EnsureSlashPath();
		strCmdLine.Replace(_T("%sPc"),current.GetPath());
		strCmdLine.Replace(_T("%sDc"),current.GetDirectory());
		strCmdLine.Replace(_T("%sBc"),CPathTool::Cat(current.GetDirectory(),current.GetFileTitle()));
		strCmdLine.Replace(_T("%sWc"),CPathTool::GetRelativePath(shortslashPathToWorkingDir,current.GetPath(),true,false));
	}

	if (lCurrentLine > -1)
		strCmdLine.Replace(_T("%l"),strLine);

	if (lpszCurrentSelection != NULL)
		strCmdLine.Replace(_T("%s"),lpszCurrentSelection);

	//DPI setting for preview
	CString strDPI;
	strDPI.Format(_T("%d"), CConfiguration::GetInstance()->m_nPreviewDPIValue);
	strCmdLine.Replace(_T("%DPI"), strDPI);

	// restore "%"
	strCmdLine.Replace(_T("\a"),_T("%"));

	//Expand the Sets
	if (bExpandPlaceholderSets)
	{
		CPlaceholderSets ps(pLProject);

		return ps.ExpandAllSets(strCmdLine);
	}

	return strCmdLine;
}




//----------------------------------------------------------------

bool AfxContainsWildcards(LPCTSTR lpszPattern)
{
	CString strTest(lpszPattern);

	return ((strTest.Find('*') >= 0) || (strTest.Find('?') >= 0));
};

//----------------------------------------------------------------

bool AfxContainsPlaceholders(LPCTSTR lpszPattern)
{
	CString strTest(lpszPattern);

	//Store "%%"
	strTest.Replace(_T("%%"),_T("\a"));

	return (strTest.Find('%') >= 0);
};

//----------------------------------------------------------------

bool AfxContainsPlaceholderSets(LPCTSTR lpszPattern)
{
	CString strTest(lpszPattern);

	//Store "$$"
	strTest.Replace(_T("$$"),_T("\a"));

	return (strTest.Find('$') >= 0);
};

//----------------------------------------------------------------

CString AfxExpandWildcard(LPCTSTR lpszStringWithWildcard,
                          bool bRecursive,
                          LPCTSTR lpszFolder/* = NULL*/,
                          CUniqueStringList* pStrList/* = NULL*/)
{
	CString retStr;

	////////////////////////////////////////////////////////////
	//Find files
	CFileFind ff;
	BOOL bMoreFiles;

	if ((CPathTool::GetDrive(lpszStringWithWildcard)).IsEmpty())
	{
		bMoreFiles = ff.FindFile(CPathTool::Cat(lpszFolder,lpszStringWithWildcard));
	}
	else
	{
		bMoreFiles = ff.FindFile(lpszStringWithWildcard);
	}

	while (bMoreFiles)
	{
		bMoreFiles = ff.FindNextFile();
		if (pStrList)
			pStrList->AddHead(ff.GetFilePath());
		else
			retStr += _T("\"") + ff.GetFilePath() + _T("\" ");
	}

	//We added one space too much
	retStr.TrimRight();

	ff.Close();

	////////////////////////////////////////////////////////////
	//Recursive through sub folders
	if (bRecursive)
	{
		bMoreFiles = ff.FindFile(CPathTool::Cat(lpszFolder,_T("*.*")));

		while (bMoreFiles)
		{
			bMoreFiles = ff.FindNextFile();
			if (ff.IsDirectory() && !ff.IsDots())
				retStr += _T(' ') + AfxExpandWildcard(lpszStringWithWildcard,bRecursive,ff.GetFilePath(),pStrList);
		}

		ff.Close();
	}

	return retStr;
}

namespace
{
const CString FormatRegex()
{
	CString strRegex;
	strRegex.Format(_T("\\$(q)*(s)*(f)*(r)*(%s|%s|%s|%s|%s|%s|%s)"),
	                TXC_PLACEHOLDERSETNAME_TEXFILES,
	                TXC_PLACEHOLDERSETNAME_BIBTEXFILES,
	                TXC_PLACEHOLDERSETNAME_GRAPHICFILES,
	                TXC_PLACEHOLDERSETNAME_TXCFILES,
	                TXC_PLACEHOLDERSETNAME_CURRENTLYOPENEDFILES,
	                TXC_PLACEHOLDERSETNAME_ALLPROJECTFILES,
	                TXC_PLACEHOLDERSETNAME_ALLFILESETS);
	return strRegex;
}
}

/***************************************************************************
 *	CPlaceholderSets
 *
 *	A class to handle set-placeholders, which define sets of files.
 *
 ***************************************************************************/
CPlaceholderSets::CPlaceholderSets(CLaTeXProject* pProject /*= NULL*/)
		: m_regexPS(FormatRegex())
{
	m_pProject = pProject;
}

CPlaceholderSets::~CPlaceholderSets()
{
}

/*
Evaluates the given String and resolves the desired filenames.
 */
CString CPlaceholderSets::ExpandAllSets(LPCTSTR lpszStringWithPlaceholderSets,
                                        CUniqueStringList* pStrList,/*= NULL*/
                                        bool bIgnoreOptions /*= false*/)
{
	//The return value
	CString retExpandedString;
	//The String to do the search on
	CString SearchString(lpszStringWithPlaceholderSets);

	//Store "$$"
	SearchString.Replace(_T("$$"),_T("\a"));

	//Flags for the regex_search
	std::regex_constants::match_flag_type nFlags = std::regex_constants::match_default;
	//Match Report
	std::match_results<LPCTSTR> what;

	//Init the Search
	LPCTSTR lpStart = (LPCTSTR)SearchString;
	LPCTSTR lpEnd = lpStart + SearchString.GetLength();

	//Start the Search
	while (regex_search(lpStart,lpEnd,what,m_regexPS,nFlags))
	{
		//Now we have the following situation in SearchString:
		// ...(lpStart)[...BeforeMatch...][Match][...AfterMatch...](lpEnd)


		//Copy [...BeforeMatch...] to the expanded string
		if (!pStrList)
			retExpandedString += CString(lpStart,what[0].first - lpStart);


		//Expand [Match] and copy it to the expanded string
		// - get the sets
		unsigned int nMatchedSets = 0;
		CString strMatchedSet(what[5].first,what[5].second - what[5].first);
		nMatchedSets += (
		                    ((strMatchedSet == TXC_PLACEHOLDERSETNAME_TEXFILES) ? TXC_PLACEHOLDERSET_TEXFILES : 0)
		                    +
		                    ((strMatchedSet == TXC_PLACEHOLDERSETNAME_BIBTEXFILES) ? TXC_PLACEHOLDERSET_BIBTEXFILES : 0)
		                    +
		                    ((strMatchedSet == TXC_PLACEHOLDERSETNAME_GRAPHICFILES) ? TXC_PLACEHOLDERSET_GRAPHICFILES : 0)
		                    +
		                    ((strMatchedSet == TXC_PLACEHOLDERSETNAME_TXCFILES) ? TXC_PLACEHOLDERSET_TXCFILES : 0)
		                    +
		                    ((strMatchedSet == TXC_PLACEHOLDERSETNAME_CURRENTLYOPENEDFILES) ? TXC_PLACEHOLDERSET_CURRENTLYOPENEDFILES : 0)
		                    +
		                    ((strMatchedSet == TXC_PLACEHOLDERSETNAME_ALLPROJECTFILES) ? TXC_PLACEHOLDERSET_ALLPROJECTFILES : 0)
		                    +
		                    ((strMatchedSet == TXC_PLACEHOLDERSETNAME_ALLFILESETS) ? TXC_PLACEHOLDERSET_ALLFILESETS : 0)
		                );

		if (!pStrList)
		{
			//StringMode

			// - get the files (options are retrieved from the match of subexpressions)
			CString strMatchedFiles = GetFileSets(nMatchedSets,
			                                      bIgnoreOptions ? false : what[1].matched,
			                                      bIgnoreOptions ? false : what[2].matched,
			                                      bIgnoreOptions ? false : what[3].matched,
			                                      bIgnoreOptions ? false : what[4].matched);

			// - copy to the expanded string
			retExpandedString += strMatchedFiles;
		}
		else
		{
			//StringListMode

			// - get the files (options are retrieved from the match of subexpressions)
			// - copy directly to given StringList
			GetFileSets(nMatchedSets,pStrList,
			            bIgnoreOptions ? false : what[2].matched,
			            bIgnoreOptions ? false : what[3].matched,
			            bIgnoreOptions ? false : what[4].matched);
		}


		//Set pointer to start search after the match in SearchString
		lpStart = what[0].second;
	}

	//Copy the rest of the search string to the expanded string
	if (!pStrList)
		retExpandedString += lpStart;


	//Restore "$"
	if (!pStrList)
		retExpandedString.Replace(_T("\a"),_T("$"));

	return retExpandedString;
}

/*
Returns a string with the names of all files, that are members
of the sets defined in Sets.
NOTE: These sets can only include files, which TXC <b>REPORTED</b>
to be part of the current project or opened.
 */
CString CPlaceholderSets::GetFileSets(const unsigned int Sets,
                                      bool bQuoted,/*= true*/
                                      bool bShortPaths /*= false*/,
                                      bool bForwardSlash /*= false*/,
                                      bool bRelativePaths /*= false*/) const
{
	//The return value
	CString retAllFiles;
	CUniqueStringList* pStrList = NULL;

	try
	{
		pStrList = new CUniqueStringList();
		if (!pStrList) AfxThrowMemoryException();
		//Let the StringList be filled with the apropriate FileNames
		GetFileSets(Sets,pStrList,bShortPaths,bForwardSlash,bRelativePaths);
	}
	catch (CMemoryException* e)
	{
		//We run out of memory. I would say, we will kill the exception
		//and be quiet.
		e->Delete();
		//But if the list could not be allocated at all, lets end this here.
		if (!pStrList) return retAllFiles;

		//TODO:
		//I know, that the list might not contain all FileNames needed
		//and this might result in less/more files deleted/protected.
		//Or whatever might be done with the list of files.
		//But to make it really clean I would have to throw a new exception and
		//handle it everywhere, where AfxExpandPlaceholders() is called.
		//Maybe someday later.
	}

	//Retrieve the Strings
	POSITION pos = pStrList->GetHeadPosition();
	while (pos != NULL)
	{
		if (bQuoted) retAllFiles += '\"';
		retAllFiles += pStrList->GetAt(pos);
		if (bQuoted)
			retAllFiles += "\" ";
		else
			retAllFiles += ' ';

		//Next
		pStrList->GetNext(pos);
	}

	//We added one space to much
	retAllFiles.TrimRight();

	//We don't need the StringList anymore
	delete pStrList;

	return retAllFiles;
}

/*
Same as above, but returns a list of strings.

NOTE: Strings in the list are never quoted.
 */
CUniqueStringList* CPlaceholderSets::GetFileSets(const unsigned int Sets,
        CUniqueStringList* pStrList,
        const bool bShortPaths /*= false*/,
        const bool bForwardSlash /*= false*/,
        const bool bRelativePaths /*= false*/) const
{
	CString strToAdd;

	//Safety
	if (!pStrList) return pStrList;
	//NOTE: For the COF we do not need a valid m_pProject

	/////////////////////////////////
	//Get the currently opened files
	if (Sets & TXC_PLACEHOLDERSET_CURRENTLYOPENEDFILES)
	{
		//Get the list of MDI Childs
		CWnd* pWnd = AfxGetMainWnd();
		CMainFrame* pMain = dynamic_cast<CMainFrame*>(pWnd);
		if (pMain)
		{
			CArray<CChildFrame*,CChildFrame*> MDIChildArray;
			pMain->GetMDIChilds(MDIChildArray);

			for (int i = 0; i < MDIChildArray.GetSize(); i++)
			{
				//Get the information from this MDI-window
				// absolute path; long filenames; backslashes
				strToAdd = MDIChildArray.GetAt(i)->GetPathNameOfDocument();

				//The Caller must be aware, that the following might throw a CMemoryException
				ConvertAndAdd(strToAdd,pStrList,bShortPaths,bForwardSlash,
				              bRelativePaths,true);
			}
		}
	}

	//Safety - we need a valid m_pProject
	if (!IsValid()) return pStrList;

	////////////////////////////////////
	//Get the Project-related TXC files
	if (Sets & TXC_PLACEHOLDERSET_TXCFILES)
	{
		//Get the path of the tcp-file
		strToAdd = m_pProject->GetPathName();
		//The Caller must be aware, that the following might throw a CMemoryException
		ConvertAndAdd(strToAdd,pStrList,bShortPaths,bForwardSlash,
		              bRelativePaths,true);

		//Get the path of the tps-file
		strToAdd = m_pProject->GetSessionPathName();
		//The Caller must be aware, that the following might throw a CMemoryException
		ConvertAndAdd(strToAdd,pStrList,bShortPaths,bForwardSlash,
		              bRelativePaths,true);

		//Get the path of the tiw-file
		strToAdd = m_pProject->GetIgnoredWordsFileName();
		//The Caller must be aware, that the following might throw a CMemoryException
		ConvertAndAdd(strToAdd,pStrList,bShortPaths,bForwardSlash,
		              bRelativePaths,true);
	}

	////////////////////////////////////
	//Get the Project-related user files
	if (Sets & (TXC_PLACEHOLDERSET_TEXFILES + TXC_PLACEHOLDERSET_BIBTEXFILES + TXC_PLACEHOLDERSET_GRAPHICFILES))
	{
		//Go through the StructureArray and extract all Files
		// that are part of the project.
		// Extract them only, if they are part of any given set.
		//
		// NOTE: If the project has not yet been parsed
		// (for example, large project and this routine is called shortly after TXC started)
		// this function will return no references to the project files.
		// So, functions relying on this here need to ensure, that the project has
		// been parsed already.
		//int i;

		//for (i = 0; i < m_pProject->m_aStructureItems.GetSize(); i++) {
		for (StructureItemContainer::const_iterator it = m_pProject->m_aStructureItems.begin(); it != m_pProject->m_aStructureItems.end(); ++it)
		{
			const StructureItem &si = *it;//m_pProject->m_aStructureItems.GetAt(i);

			if (
			    ((si.GetType() == StructureItem::texFile) && (Sets & TXC_PLACEHOLDERSET_TEXFILES))
			    ||
			    ((si.GetType() == StructureItem::bibFile) && (Sets & TXC_PLACEHOLDERSET_BIBTEXFILES))
			    ||
			    ((si.GetType() == StructureItem::graphicFile) && (Sets & TXC_PLACEHOLDERSET_GRAPHICFILES))
			)
			{
				//Get the absolute path; long filenames; backslashes
				strToAdd = m_pProject->GetFilePath(si.GetPath());

				//Convert to relative path in ConvertAndAdd(), if wanted
				//NOTE: It is not sure, whether	si.m_strPath contains
				// a relative path (relative to the WorkingDir)
				// or a full path. For example, the MainFile is added as a full path to
				// the StructureItemArray by the StructureParser, but other files not.
				// And as I do not want this code too much depend on the Parser,
				// I calculate the relative paths here in this class.

				//The Caller must be aware, that the following might throw a CMemoryException
				ConvertAndAdd(strToAdd,pStrList,bShortPaths,bForwardSlash,
				              bRelativePaths,true);
			}
		}
	}

	return pStrList;
}

/*
Sets the pointer to the Project to retrieve information from.
 */
CLaTeXProject* CPlaceholderSets::SetProject(CLaTeXProject* argpProject)
{
	CLaTeXProject* pOld = m_pProject;
	m_pProject = argpProject;
	//	ASSERT(IsValid());
	return pOld;
}

/*
Converts a string according to the given bool parameters
and adds it to the given CUniqueStringList-Object.
It expects to get a full path of the file, i.e. "D:\Temp\test.txt".

Be aware, that parts of this function might throw a CMemoryException.
Be aware, that the given String might be altered.
 */
void CPlaceholderSets::ConvertAndAdd(CString& strToAdd, CUniqueStringList* pStrList, bool bShortPaths, bool bForwardSlash, bool bRelativePaths, bool bCheckExistence) const
{
	//Is the String empty?
	if (strToAdd.IsEmpty()) return;

	//Does the File exists?
	if (bCheckExistence && (!CPathTool::Exists(strToAdd)))
		return;

	//Convert to relative path, if wanted
	if (bRelativePaths)
	{
		strToAdd = CPathTool::GetRelativePath(m_pProject->GetWorkingDirectory(),strToAdd);
	}

	//Convert to short path, if wanted
	if (bShortPaths)
	{
		strToAdd = CPathTool::GetShortPath(strToAdd);
	}

	//Convert to path with forward slashes, if wanted
	if (bForwardSlash)
	{
		strToAdd = CPathTool::GetSlashPath(strToAdd);
	}

	//Trim and Add
	strToAdd.TrimLeft();
	strToAdd.TrimRight();
	//The Caller must be aware, that the following might throw a CMemoryException
	pStrList->AddHead(strToAdd);
}

bool CPlaceholderSets::IsValid() const
{
	return (m_pProject != NULL);
}