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
#include "Placeholder.h"

class RegistryStack;

/**
A storage to define a pattern, that matches files to be deleted/protected.

@author Tino Weinkauf
 */
class CFileCleanItem
{
//Item types
public:

	enum tagFileHandling
	{
		none = 0,
		clean,
		protect,
		protectbydefault
	};

//Construction/Destruction
public:
	///Standard Constructor
	CFileCleanItem();

	///Copy Constructor
	CFileCleanItem(const CFileCleanItem &arg)
	{
		*this = arg;
	};

	///Element Constructor
	CFileCleanItem(const LPCTSTR argDescription, const LPCTSTR argPattern,
	               const tagFileHandling argFileHandling,
	               const bool argRecursive);

	virtual ~CFileCleanItem();


//Operators
	/** @{*/
	/** @name Operators*/

public:
	///Assignment - assigns new values to this (left) item
	inline void operator=(const CFileCleanItem &arg)
	{
		strDescription = arg.strDescription;
		strPattern = arg.strPattern;
		tFileHandling = arg.tFileHandling;
		bRecursive = arg.bRecursive;
	};

	///Comparison of Equality - true, if equal
	inline bool operator==(const CFileCleanItem &arg) const
	{
		return ((strDescription == arg.strDescription)
		        && (tFileHandling == arg.tFileHandling));
	};

	///Comparison: Less
	inline bool operator<(const CFileCleanItem &arg) const
	{
		return (
		           (tFileHandling < arg.tFileHandling)
		           || ((tFileHandling == arg.tFileHandling) && (strDescription < arg.strDescription))
		       );
	};

	///Comparison: Greater
	inline bool operator>(const CFileCleanItem &arg) const
	{
		return (
		           (tFileHandling > arg.tFileHandling)
		           || ((tFileHandling == arg.tFileHandling) && (strDescription > arg.strDescription))
		       );
	};

	///Comparison: LessEqual
	inline bool operator<=(const CFileCleanItem &arg) const
	{
		return (
		           (tFileHandling <= arg.tFileHandling)
		           || ((tFileHandling == arg.tFileHandling) && (strDescription <= arg.strDescription))
		       );
	};

	///Comparison: GreaterEqual
	inline bool operator>=(const CFileCleanItem &arg) const
	{
		return (
		           (tFileHandling >= arg.tFileHandling)
		           || ((tFileHandling == arg.tFileHandling) && (strDescription >= arg.strDescription))
		       );
	};

	/* @} */

//Methods
public:
	/**
	Checks, if the pattern is valid.

	@returns true, if the pattern is valid.
	 */
	bool PatternIsValid();

	/**
	Resolves the filenames from the pattern and adds them to the given StringList.
	 */
	bool Expand(CLaTeXProject* argpProject, const CPlaceholderInfo& PInfo, CUniqueStringList* pSList);

	/**
	Saves this Item to the registry.
	 */
	bool SerializeToRegistry(RegistryStack &reg) const;

	/**
	Reads this Item from the registry.
	 */
	bool SerializeFromRegistry(RegistryStack &reg);

//Attributes
private:
	///Just a textual Description of what is defined by strPattern
	CString strDescription;

	///Toggle, whether to scan the directories recursive or not.
	bool bRecursive;

	///Defines, what shall be done with the files defined by strPattern
	tagFileHandling tFileHandling;

	///The Pattern, that defines a set of files using wildcards, Placeholders and Placeholder sets
	CString strPattern;


public:
	const CString& GetDescription() const;
	void SetDescription(const CString& val);	

	const CString& GetPattern() const;
	void SetPattern(const CString& val);	

	CFileCleanItem::tagFileHandling GetFileHandling() const;
	void SetFileHandling(CFileCleanItem::tagFileHandling val);	

	bool IsRecursive() const;
	void SetRecursive(bool val);
};


//typedef CSortArray<CFileCleanItem, CFileCleanItem&> CFileCleanItemArray;

/** An Array to hold the Definitions (Items) for file cleaning.
	Adds 'protectedbydefault'-Values at creation.

	@author Tino Weinkauf
*/
class CFileCleanItemArray : public CSortArray<CFileCleanItem, const CFileCleanItem&>
{
//Constructor / Destructor
public:
	///Overwritten to add 'protectedbydefault'-Values.
	CFileCleanItemArray();
	virtual ~CFileCleanItemArray();

//Methods
public:
	///Adds a minimal number of default Items.
	void AddMinimalDefaultItems();

	///Removes all existing items and adds a large number of default Items.
	void RestoreAllDefaultItems();

	///Serializes the array to the registry.
	bool SerializeToRegistry(LPCTSTR strStartSection);
	///Loads the array from the registry.
	bool SerializeFromRegistry(LPCTSTR strStartSection);
};


/** A Class to delete (clean) Files defined by Wildcards,
	Placeholders and Placeholder sets.
	Supports Protection of Files.

	@author Tino Weinkauf
*/
class CFileClean
{
public:
	///Construction/Destruction
	CFileClean();
	virtual ~CFileClean();

//Methods
public:
	/**
	Resolves the Information stored in ItemArray and builds up the file lists.

	@returns true, if all patterns could be resolved.
	@returns false, if at least one pattern could not be resolved or the pointer is NULL.
	 */
	bool Initialize(CFileCleanItemArray& ItemArray);

	/**
	Deletes the files contained in m_FilesToClean, if they are not in m_FilesToProtect.
	So this works always with Protection.

	@returns true, if all files could be deleted.
	@returns false, if at least one file could not be deleted (for example file is readonly).
	 */
	bool CleanFiles();

	/**
	Returns the files to be deleted.
	Each separated by "\r\n", which is suitable for a edit control.

	@param bProtectionOn
	        Use this to toogle Protection on/off. If true, it returns
	        the list of files, that will be deleted.
	        If false, it returns the list of files that were defined to
	        be deleted.
	 */
	CString GetFilesToClean(bool bProtectionOn) const;

	/**
	Returns the files to be protected.
	Each separated by "\r\n", which is suitable for a edit control.
	 */
	CString GetFilesToProtect() const;

//Attributes
protected:
	///The List of FileNames to be deleted (cleaned) (not interfered/merged with protected files)
	CUniqueStringList m_FilesToClean;
	///The List of FileNames to be protected
	CUniqueStringList m_FilesToProtect;
};

