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

#include "LatexProject.h"


/** Holds information from throughout TeXnicCenter for expanding placeholders.
* 
*   Use FillWithInformation() to put useful information into this class.
*   Then pass it on when calling LaTeX and friends.
*   Classes like PProcessor use this to call AfxExpandPlacerHolders().
* 
* @author Tino Weinkauf
*/
class CPlaceholderInfo
{
//Friends
//Types
public:

//Construction / Deconstruction
public:
	CPlaceholderInfo(){};
	virtual ~CPlaceholderInfo() = default;

//Methods
public:
	void FillWithInformation();

//Attributes
public:
	///Path to the project's main file.
	CString strMainPath;

	///Path to the current file in the editor.
	CString strCurrentPath;

	///Path to the template file for compiling the preview.
	CString strPreviewTemplatePath;

	///Working directory used for relative path calculations.
	CString strWorkingDir;

	///Line index of the cursor in the current file.
	long lCurrentLine{-1};

	///Selected text in the current document.
	CString strCurrentSelection;

	///DPI value used for preview generation.
	int nPreviewDPIValue{300};

	///Whether to expand placeholder sets like "$qrAPF".
	bool bExpandPlaceholderSets{true};

	///If true, use the current path as if it was the main path. Ignores the main path completely.
	bool bUseCurrentPathAsMainPath{false};
};


/**
Expands the placeholders in the given string and returns the result.

@param lpszStringWithPlaceholders
        String that contains the placeholders to expand.
@param Info
        Object containing information for expanding the placeholders.

@author Sven Wiegand
@author Tino Weinkauf
 */
CString AfxExpandPlaceholders(LPCTSTR lpszStringWithPlaceholders, const CPlaceholderInfo& Info);


/**
Returns true, if lpszPattern contains Wildcards like "*" and "?"

@author Tino Weinkauf
 */
bool AfxContainsWildcards(LPCTSTR lpszPattern);

/**
Returns true, if lpszPattern contains Placeholders starting with %

@author Tino Weinkauf
 */
bool AfxContainsPlaceholders(LPCTSTR lpszPattern);

/**
Returns true, if lpszPattern contains Placeholder sets starting with $

@author Tino Weinkauf
 */
bool AfxContainsPlaceholderSets(LPCTSTR lpszPattern);


/**
Expands a given Wildcard-Pattern to a List/String of Files.
If the pattern does not contain a drive, this function evaluates
the pattern in the given folder.

If the given StringList is NULL, a string with all filenames will be returned:
Each quoted and separated by a space.

If the given StringList is not NULL, it will be filled instead of the string.
The String will be empty in this case.

If lpszStringWithWildcard does not contain a wildcard but a filename,
this one searched in the given folder.

If lpszFolder is NULL, the current directory is used.

@author Tino Weinkauf
@author Sven Wiegand
 */
CString AfxExpandWildcard(LPCTSTR lpszStringWithWildcard,
                          bool bRecursive,
                          LPCTSTR lpszFolder = NULL,
                          CUniqueStringList* pStrList = NULL);


///Defines for Placeholder sets
const int TXC_PLACEHOLDERSET_TEXFILES = 1;
const int TXC_PLACEHOLDERSET_BIBTEXFILES = 2;
const int TXC_PLACEHOLDERSET_GRAPHICFILES = 4;
const int TXC_PLACEHOLDERSET_TXCFILES = 8;
const int TXC_PLACEHOLDERSET_CURRENTLYOPENEDFILES = 16;
///Defines for sets of Placeholder sets
const int TXC_PLACEHOLDERSET_ALLPROJECTFILES = 15;
const int TXC_PLACEHOLDERSET_ALLFILESETS = 0xFFFF;
///Defines for names of Placeholder sets
LPCTSTR const TXC_PLACEHOLDERSETNAME_TEXFILES = _T("TPF");
LPCTSTR const TXC_PLACEHOLDERSETNAME_BIBTEXFILES = _T("BPF");
LPCTSTR const TXC_PLACEHOLDERSETNAME_GRAPHICFILES = _T("GPF");
LPCTSTR const TXC_PLACEHOLDERSETNAME_TXCFILES = _T("TXC");
LPCTSTR const TXC_PLACEHOLDERSETNAME_CURRENTLYOPENEDFILES = _T("COF");
LPCTSTR const TXC_PLACEHOLDERSETNAME_ALLPROJECTFILES = _T("APF");
LPCTSTR const TXC_PLACEHOLDERSETNAME_ALLFILESETS = _T("AFS");

/**
A Class to handle Set-Placeholders (Placeholder Sets), which define sets of files.

This allows the following syntax for a given string (see also ExpandAllSets):

<b><tt>$[q][s][f][r]\<FILESET\></tt></b>

<ul>
        <li><b><tt>$</tt></b> Starts the Placeholder. Use "$$" to get "$".</li>

        <li><b><tt>q</tt></b> Filenames will be quoted using ".</li><br>

        <li><b><tt>s</tt></b> Filenames in 8.3-format.</li><br>

        <li><b><tt>f</tt></b> Forward Slash '/' is used as Directory Separator.</li><br>

        <li><b><tt>r</tt></b> Filenames will be given relative to the WorkingDir of the Project (see also SetProject).</li>

        <li><b><tt>\<FILESET\></tt></b> One of the following:</li>

        <ul>
                <li><b><tt>TPF</tt></b> All TeX-Files reported in <var>m_aStructureItems</var> of the Project (see also SetProject).</li>
                <li><b><tt>BPF</tt></b> All BibTeX-Files reported in <var>m_aStructureItems</var> of the Project (see also SetProject).</li>
                <li><b><tt>GPF</tt></b> All Graphic-Files reported in <var>m_aStructureItems</var> of the Project (see also SetProject).</li>
                <li><b><tt>TXC</tt></b> All Files generated by TXC regarding the Project (see also SetProject). Currently only the .tcp and .tps files.</li>
                <li><b><tt>COF</tt></b> All currently opened Files in the Editor.</li>
                <li><b><tt>APF</tt></b> All Project Files. This is the sum of <b><tt>TPF</tt></b>, <b><tt>BPF</tt></b>, <b><tt>GPF</tt></b> and <b><tt>TXC</tt></b>.</li>
                <li><b><tt>AFS</tt></b> All above Filesets together. This is the sum of <b><tt>TPF</tt></b>, <b><tt>BPF</tt></b>, <b><tt>GPF</tt></b>, <b><tt>TXC</tt></b> and <b><tt>COF</tt></b>.</li>
        </ul>
</ul>


<h4>Examples:</h4>

<ul>
        <li><b><tt>$qTPF</tt></b> Lists all TeX-Files reported by TXC-StructureParser. \n
                                Each quoted and separated from each other using a space, \n
                                like <tt> \verbatim "D:\Temp\test\curvature3d.tex" "D:\Temp\test\definitions\macros.tex" "D:\Temp\test\chaptertwo.tex" \endverbatim </tt></li>

        <li><b><tt>$fBPF</tt></b> Lists all BibTeX-Files reported by TXC-StructureParser. \n
                                The Forward Slash is used as directory separator. \n
                                The Files themselves are separated from each other using a space, \n
                                like <tt> \verbatim D:/Temp/test/xbib.bib D:/Temp/test/morebibs/morexbib.bib \endverbatim </tt></li>

        <li><b><tt>$COF</tt></b> Lists all Files opened in the editor. \n
                                Separated from each other using a space (notice the problem with non-quoting of "Program Files"), \n
                                like <tt> \verbatim D:\Temp\test\curvature3d.tex D:\Program Files\texmf\pdftex\latex\config\pdflatex.ini \endverbatim </tt></li>

        <li><b><tt>$qsrAPF</tt></b> Lists all Files contained in the Project (as reported by TXC-StructureParser). \n
                                Each quoted, Filenames in 8.3-Format and relative to the WorkingDir and separated from each other using a space, \n
                                like <tt> \verbatim "curvat~1.tcp" "curvat~1.tps" "curvat~1.tex" "..\..\Progra~1\texmf\pdftex\latex\config\pdflatex.ini" "xbib.bib" "morebibs\morexb~1.bib" "defini~1\macros.tex" "chapte~1.tex" \endverbatim </tt></li>
</ul>


<h4>Application:</h4>

A good place to use the sets would be:
<tt> \verbatim zip.exe a "%tm.zip" $qsrAPF \endverbatim </tt>


The Set of FileNames returned/expanded by GetFileSets and ExpandAllSets are
always unique - no duplicates of a filename.

@author Tino Weinkauf
 */
class CPlaceholderSets
{
public:
	//Constructor / Destructor
	CPlaceholderSets(CLaTeXProject* pProject = NULL);
	virtual ~CPlaceholderSets();

//Operations
public:
	/**
	Evaluates the given String and resolves the desired filenames.

	@param lpszStringWithPlaceholderSets
	        The String with the placeholder sets to be expanded.

	@param pStrList
	        See below.

	@param bIgnoreOptions
	        The Options (q, s, f, r) of a set-placeholder are ignored.
	        Example: "$rAPF" is treated as "$APF".

	@returns
	        If pStrList == NULL , then it returns a string, where all placeholder sets have been expanded
	        to their real values.
	@returns
	        If pStrList != NULL , then it returns an empty string and all filenames have been added to the List.
	 */
	CString ExpandAllSets(LPCTSTR lpszStringWithPlaceholderSets,
	                      CUniqueStringList* pStrList = NULL,
	                      bool bIgnoreOptions = false);

	/**
	Returns a string with the names of all files, that are members
	of the sets defined in Sets.

	@param Sets
	        A value to define the set of sets of files to be returned.
	        Can be any combination of (use binary OR "|" to combine):
	                TXC_PLACEHOLDERSET_TEXFILES
	                TXC_PLACEHOLDERSET_BIBTEXFILES
	                TXC_PLACEHOLDERSET_GRAPHICFILES
	                TXC_PLACEHOLDERSET_TXCFILES
	                TXC_PLACEHOLDERSET_CURRENTLYOPENEDFILES

	        You can also use the following predefined set of sets:
	                TXC_PLACEHOLDERSET_ALLPROJECTFILES (== TEX + BIB + GRAPHIC + TXC)
	                TXC_PLACEHOLDERSET_ALLFILESETS (== all sets above together)

	@param bQuoted
	        Filenames are "quoted"

	@param bShortPaths
	        Filenames are in 8.3-Format

	@param bForwardSlash
	        ForwardSlash is used as directory separator

	@param bRelativePaths
	        Filenames will be given relative to the WorkingDir of the Project (see also SetProject)


	NOTE: These sets can only include files, which TXC <b>REPORTED</b>
	to be part of the current project or opened.
	 */
	CString GetFileSets(const unsigned int Sets,
	                    const bool bQuoted = true,
	                    const bool bShortPaths = false,
	                    const bool bForwardSlash = false,
	                    const bool bRelativePaths = false) const;

	/**
	Same as above, but returns a list of strings.

	@param pStrList
	        List to be filled with found filenames. Caller must allocate/create and delete it.

	NOTE: Strings in the list are never quoted.
	 */
	CUniqueStringList* GetFileSets(const unsigned int Sets,
	                               CUniqueStringList* pStrList,
	                               const bool bShortPaths = false,
	                               const bool bForwardSlash = false,
	                               const bool bRelativePaths = false) const;

	/**
	Sets the pointer to the Project to retrieve information from.

	@param argpProject
	        Pointer to Project

	@returns
	        Pointer to former Project given to this PlaceholderSet-Object
	 */
	CLaTeXProject* SetProject(CLaTeXProject* argpProject);

protected:
	///Returns, whether this object has valid attributes or not.

	bool IsValid() const;

	/**
	Converts a string according to the given bool parameters
	and adds it to the given CUniqueStringList-Object.
	It expects to get a full path of the file, i.e. \verbatim "D:\Temp\test.txt" \endverbatim

	@param bCheckExistence
	        Set to true, if you only want to add a Name of a File,
	        which really exists.

	@see GetFileSets for explanation of the other parameters.

	Be aware, that parts of this function might throw a CMemoryException.
	Be aware, that the given String might be altered.
	 */
	void ConvertAndAdd(CString& strToAdd, CUniqueStringList* pStrList, bool bShortPaths, bool bForwardSlash, bool bRelativePaths, bool bCheckExistence) const;


//Attributes
private:
	///Pointer to the Project; needed to get the names of the project files
	CLaTeXProject* m_pProject;

	///Regular Expression to search for the Placeholders
	const tregex m_regexPS;
};


