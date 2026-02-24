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

#if !defined(AFX_PROFILE_H__6FC6E480_04F2_11D5_A222_006097239934__INCLUDED_)
#define AFX_PROFILE_H__6FC6E480_04F2_11D5_A222_006097239934__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <memory>

#include "PProcessor.h"
#include "ProcessCommand.h"
#include "DDECommand.h"

class RegistryStack;

/**
Profile definition. Specifies how to build the output.

@ingroup profiles

@author Sven Wiegand
 */
class CProfile
{
// types
public:

	/**
	Used to store information about a command -- no matter if it is
	a command line to execute or a DDE-command to send.
	 */
	class CCommand
	{
		// types
	public:

		enum tagType
		{
			typeProcess = 0,
			typeDde
		};

		// construction/destruction
	public:
		CCommand();
		virtual ~CCommand();

		// attribute operations
	public:
		void SetActiveCommand(int nActiveCommand);
		void SetProcessCommand(const CProcessCommand &cmd);
		void SetDdeCommand(const CDdeCommand &cmd);

		int GetActiveCommand() const;
		const CProcessCommand &GetProcessCommand() const;
		const CDdeCommand &GetDdeCommand() const;

		// operations
	public:
		/**
		Removes the directory specifications from all path specifications
		in this command.

		This is usefull when exporting profiles.
		 */
		void RemoveDirectorySpecifications();

		// serialization
	public:
		/**
		Stores its information in the current key of the given registry
		object.

		Will write some values so the key should be reserved only for
		this object.
		 */
		BOOL SerializeToRegistry(RegistryStack &reg) const;

		/**
		Reads its information from the current key of the given registry
		object.
		 */
		BOOL SerializeFromRegistry(RegistryStack &reg);

		/**
		Saves the command properties to the specified XML element.

		@exception CComException
		 */
		void SaveXml(MsXml::CXMLDOMElement xmlCommand) const;

		/**
		Loads the command properties from the specified XML element.
		 */
		void LoadXml(MsXml::CXMLDOMElement xmlCommand);

		// attributes
	protected:
		/**
		Specifies, which of the two commands is the active one.

		One of the values typeProcess or typeDde
		 */
		int m_nActiveCommand;

		/** The process command */
		CProcessCommand m_cmdProcess;

		/** The Dde command */
		CDdeCommand m_cmdDde;
	};

// construction/destruction
public:
	CProfile();
	CProfile(const CProfile &p);
	virtual ~CProfile();

// operators
public:
	CProfile & operator=(const CProfile &p);

// attribute operations
public:
	/**
	Sets the path of the (La)TeX-compiler to use, specifies if (La)teX
	should be used in this profile and whether compilation should stop
	on error or not.
	 */
	void SetLatexPath(LPCTSTR lpszPath, LPCTSTR lpszArguments, BOOL bUseLatex = TRUE, BOOL bStopOnLatexError = FALSE);

	/**
	Sets the path of the BibTeX-executable to use and specifies if
	BibTeX should be used in this profile.
	 */
	void SetBibTexPath(LPCTSTR lpszPath, LPCTSTR lpszArguments, BOOL bUseBibTex = TRUE);

	/**
	Sets the path of the MakeIndex-executable to use and specifies if
	MakeIndex	should be used in this profile.
	 */
	void SetMakeIndexPath(LPCTSTR lpszPath, LPCTSTR lpszArguments, BOOL bUseMakeIndex = TRUE);

	/**
	Gets a reference to the array of the preprocessors, which can be
	used to read from and write to the array.
	 */
	CPProcessorArray &GetPreProcessorArray();

	/**
	Gets a reference to the array of the postprocessors, which can be
	used to read from and write to the array.
	 */
	CPProcessorArray &GetPostProcessorArray();

	/**
	Gets a reference to the array of the preview processors, which can be
	used to read from and write to the array.
	 */
	CPProcessorArray& GetPreviewProcessorArray();

	///Sets the path of the preview image file.
	void SetPreviewImagePath(LPCTSTR lpszPreviewPath);

	///Returns the path of the preview image file.
	CString GetPreviewImagePath() const;

	/**
	Sets the path of the viewer-executable to use to view output,
	generated using this profile.
	 */
	void SetViewerPath(LPCTSTR lpszViewerPath);

	/**
	Specifies the command to execute to view the output.
	 */
	void SetViewProjectCmd(const CCommand &cmd);

	/**
	Specifies the command to execute to view the output at the position
	belonging to the current file in the editor.
	 */
	void SetViewCurrentCmd(const CCommand &cmd);

	/**
	Specifies if the close-view command should be executed before the
	(La)TeX-compiler will be started.
	 */
	void SetCloseView(BOOL bClose);

	/**
	Sets the command used to close the output in the viewer, before the
	(La)TeX-compiler will be started and specifies, whether to use this
	command or not.
	 */
	void SetViewCloseCmd(const CCommand &cmd);

	/**
	Returns TRUE if the (La)TeX compiler will be used and FALSE
	otherwise.
	 */
	BOOL GetRunLatex() const;

	/**
	Returns TRUE if the compilation should stop on (La)TeX error
	and FALSE otherwise.
	 */
	BOOL GetStopOnLatexError() const;

	/**
	Returns TRUE if the BibTex should be used and FALSE otherwise.
	 */
	BOOL GetRunBibTex() const;

	/**
	Returns TRUE if the MakeIndex should be used and FALSE otherwise.
	 */
	BOOL GetRunMakeIndex() const;

	/**
	Returns TRUE if the view close command will be executed, before the
	(La)TeX compiler will be started.
	 */
	BOOL GetCloseView() const;

	CString GetLatexPath() const;
	CString GetLatexArguments() const;
	CString GetBibTexPath() const;
	CString GetBibTexArguments() const;
	CString GetMakeIndexPath() const;
	CString GetMakeIndexArguments() const;
	CString GetViewerPath() const;
	CCommand& GetViewProjectCmd();
	CCommand& GetViewCurrentCmd();
	CCommand& GetViewCloseCmd();

	void SetLaTeXArguments(LPCTSTR args)
	{
		m_strLatexArguments = args;
	}

// operations
public:
	/**
	Removes the directory specifications from all path specifications
	in this profile.

	This is usefull when exporting profiles.
	 */
	void RemoveDirectorySpecifications();

// serialization
public:
	/**
	Stores the object's information at the current position in the
	given registry object.
	 */
	BOOL SerializeToRegistry(RegistryStack &reg) const;

	/**
	Reads the information from the current position of the given
	registry object.
	 */
	BOOL SerializeFromRegistry(RegistryStack &reg);

	/**
	Writes the profiles properties to the specified XML element.

	@exception CComException
	 */
	void SaveXml(MsXml::CXMLDOMElement &xmlProfile) const;

	/**
	Loads the profiles properties from the specified XML element.

	@exception CComException
	 */
	void LoadXml(MsXml::CXMLDOMElement &xmlProfile, const int Version);

// attributes
protected:
	/**
	TRUE if (La)TeX should be run, FALSE if only the pre- and postprocessors
	should be run.
	 */
	BOOL m_bRunLatex;

	/** Full path of the (La)TeX-compiler to use */
	CString m_strLatexPath;

	/** Arguments to pass to the (La)TeX-compiler */
	CString m_strLatexArguments;

	/**
	TRUE if TXC should stop after the (La)TeX run, if
	an error occured. FALSE if the postprocessors
	should be run nevertheless.
	This is about unserious errors only, i.e. (La)TeX
	generated at least one page of output. Compilation
	will stop, if there were no pages of output and latex
	finished with an error code.
	 */
	BOOL m_bStopOnLatexError;


	/**
	TRUE if BibTeX should be run, FALSE otherwise.
	 */
	BOOL m_bRunBibTex;

	/** Full path of the BibTeX-executable to use */
	CString m_strBibTexPath;

	/** Arguments to pass to the BibTeX-executable */
	CString m_strBibTexArguments;


	/**
	TRUE if MakeIndex should be run, FALSE otherwise.
	 */
	BOOL m_bRunMakeIndex;

	/** Full path of the MakeIndex-executable to use */
	CString m_strMakeIndexPath;

	/** Arguments to pass to the MakeIndex-executable */
	CString m_strMakeIndexArguments;


	/** The preprocessors to run after the (La)TeX-compiler. */
	CPProcessorArray m_aPreProcessors;

	/** The postprocessors to run after the (La)TeX-compiler. */
	CPProcessorArray m_aPostProcessors;

	/** The processors to run for generating a preview. */
	CPProcessorArray m_aPreviewProcessors;

	/** Full path of the preview image */
	CString m_strPreviewImagePath;

	/** Full path of the viewer */
	CString m_strViewerPath;

	/** Command to execute the viewer and view the project */
	CCommand m_cmdViewProject;

	/** Command to execute the viewer and view the current file */
	CCommand m_cmdViewCurrent;

	/**
	TRUE, if the m_cmdCloseView command should be executed, before the
	(La)TeX compiler will be started or FALSE otherwise.
	 */
	BOOL m_bCloseBeforeCompilation;

	/** Command to close the file in the viewer */
	CCommand m_cmdCloseView;
};


// inlines

inline
CPProcessorArray& CProfile::GetPreProcessorArray()
{
	return m_aPreProcessors;
}

inline
CPProcessorArray& CProfile::GetPostProcessorArray()
{
	return m_aPostProcessors;
}

inline
CPProcessorArray& CProfile::GetPreviewProcessorArray()
{
	return m_aPreviewProcessors;
}

inline
BOOL CProfile::GetRunLatex() const
{
	return m_bRunLatex;
}

inline
BOOL CProfile::GetStopOnLatexError() const
{
	return m_bStopOnLatexError;
}

inline
BOOL CProfile::GetRunBibTex() const
{
	return m_bRunBibTex;
}

inline
BOOL CProfile::GetRunMakeIndex() const
{
	return m_bRunMakeIndex;
}

inline
BOOL CProfile::GetCloseView() const
{
	return m_bCloseBeforeCompilation;
}

inline
CString CProfile::GetLatexPath() const
{
	return m_strLatexPath;
}

inline
CString CProfile::GetLatexArguments() const
{
	return m_strLatexArguments;
}

inline
CString CProfile::GetBibTexPath() const
{
	return m_strBibTexPath;
}

inline
CString CProfile::GetBibTexArguments() const
{
	return m_strBibTexArguments;
}

inline
CString CProfile::GetMakeIndexPath() const
{
	return m_strMakeIndexPath;
}

inline
CString CProfile::GetMakeIndexArguments() const
{
	return m_strMakeIndexArguments;
}

inline
CString CProfile::GetPreviewImagePath() const
{
	return m_strPreviewImagePath;
}

inline
CString CProfile::GetViewerPath() const
{
	return m_strViewerPath;
}

inline
CProfile::CCommand& CProfile::GetViewProjectCmd()
{
	return m_cmdViewProject;
}

inline
CProfile::CCommand& CProfile::GetViewCurrentCmd()
{
	// if view current command not specified, then return the project view command
	return m_cmdViewCurrent;
	/*
	if (m_cmdViewCurrent.GetActiveCommand() == CCommand::typeProcess && m_cmdViewCurrent.GetProcessCommand().GetArguments().IsEmpty())
	        return m_cmdViewProject;
	else if (m_cmdViewCurrent.GetActiveCommand() == CCommand::typeDde && m_cmdViewCurrent.GetDdeCommand().GetCommand().IsEmpty())
	        return m_cmdViewProject;
	else
	        return m_cmdViewCurrent;
	 */
}

inline
CProfile::CCommand& CProfile::GetViewCloseCmd()
{
	return m_cmdCloseView;
}

inline
int CProfile::CCommand::GetActiveCommand() const
{
	return m_nActiveCommand;
}

inline
const CProcessCommand& CProfile::CCommand::GetProcessCommand() const
{
	return m_cmdProcess;
}

inline
const CDdeCommand& CProfile::CCommand::GetDdeCommand() const
{
	return m_cmdDde;
}



/**
An array of profiles (CProfile-objects).

@ingroup profiles

@see CProfile

@author Sven Wiegand
 */
class CProfileMap : protected CMap<CString, LPCTSTR, CProfile*, CProfile*>
{
	// construction/destruction

	static std::auto_ptr<CProfileMap> impl_;

public:
	using CObject::operator delete;

	CProfileMap();
	virtual ~CProfileMap();

	static CProfileMap* GetInstance();

// make base implementations available
public:
	using CMap<CString, LPCTSTR, CProfile*, CProfile*>::Lookup;
	using CMap<CString, LPCTSTR, CProfile*, CProfile*>::GetStartPosition;
	using CMap<CString, LPCTSTR, CProfile*, CProfile*>::GetNextAssoc;

// operations
public:
	/**
	Adds a profile to the list

	@param lpszKey
	        Title to identify the profile with.
	@param profile
	        Profile to add.

	@return
	        TRUE if successfull, FALSE if a profile with the given name does
	        already exist of if there was not enough memory to create the
	        profile on the heap.
	 */
	BOOL Add(LPCTSTR lpszKey, const CProfile &profile);

	/**
	Renames the profile with the specified old title to the given new
	title.

	@return
	        TRUE if successfull, FALSE if the given profile does not exist
	        or the new name does alread exist.
	 */
	BOOL Rename(LPCTSTR lpszOldKey, LPCTSTR lpszNewKey);

	/**
	Removes the profile with the given key from the list.
	 */
	void Remove(LPCTSTR lpszKey);

	/**
	Remove all profiles from the list.
	 */
	void RemoveAll();

	/**
	Returns TRUE if a profile with the given name does already exist
	in the map and FALSE otherwise.
	 */
	BOOL Exists(LPCTSTR lpszKey) const;

	/**
	Activates the profile with the specified key.
	May set the active project to be modified (bSetModifiedProject == true)
	and may inform the OutputDoc of the change.

	@return
	        FALSE, if there does not exist a profile with the specified key.
	@return
	        TRUE, if the specified key could be found in the list of profiles.
	                The ModifiedFlag of the active project will be set only,
					if this profile was not selected before.
	 */
	BOOL SetActiveProfile(LPCTSTR lpszKey, bool bSetModifiedProject = true,
							bool bInformOutputDoc = true);

	/**
	Returns a pointer to the active profile or NULL if there are no
	profiles in the map.
	 */
	CProfile *GetActiveProfile();

	/**
	Returns the key to the active profile or an empty string, if there
	is no active profile.
	 */
	CString GetActiveProfileKey() const;

	/**
	Fills the specified string array with the keys of the profiles
	stored in this map.

	The keys are not sorted.
	 */
	void GetKeyList(CStringArray &astrKeys) const;

	/**
	Makes this copy a deep copy of the specified map.

	This method does not copy the CProfile-pointers stored in the other
	map, instead it creates a copy of each profile stored in profiles
	add adds it to this map.

	@param profiles
	        Source map to copy from.
	@param bEmptyBeforeCopy
	        If TRUE, all items from this map will be removed before the new
	        ones are copied. Otherwise the new items will be added and maybe
	        overwrite the existing items.
	@param bAskUserToOverwrite
	        If bEmptyBeforeCopy is FALSE and the method tries to overwrite
	        an already defined profile, this flag specifies whether the user
	        should be asked to overwrite or not. If the user is not asked,
	        items will be overwritten silently.
	 */
	void CopyFrom(const CProfileMap &profiles, BOOL bEmptyBeforeCopy = TRUE, BOOL bAskUserToOverwrite = FALSE);

	/**
	Removes the directory specifications from all path specifications
	in all profiles.

	This is usefull when exporting profiles.
	 */
	void RemoveDirectorySpecifications();

	/**
	Allows the user to import output profiles to this map.
	 */
	void Import();

	/**
	Allows the user to export output profiles of this map.
	 */
	void Export() const;

// serialization
public:
	/**
	Stores the object's information in the registry.

	The information will be stored under the key, specified by the
	m_pszRegistryKey -member of the CWinApp-derived application class.
	 */
	BOOL SerializeToRegistry() const;

	/**
	Reads the object's information from the registry.

	The information will be read from the key, specified by the
	m_pszRegistryKey -member of the CWinApp-derived application class.
	 */
	BOOL SerializeFromRegistry();

	/**
	Loads the profile map from the specified XML file, which must follow
	the XML-schema
	'http://schemas.ToolsCenter.org/TeXnicCenter/OutputProfiles.xsd'.

	Before loading, all currently existing profiles are removed from
	the map.

	In case of an error the method will display a message to the user
	and return FALSE.

	@param lpszPath
	        Path to load the definitions from.

	@return
	        TRUE on success, FALSE otherwise.
	 */
	BOOL LoadXml(LPCTSTR lpszPath);

	/**
	Saves the profile map to the specified XML file.

	The created file follows the rules specified in the XML-schema
	'http://schemas.ToolsCenter.org/TeXnicCenter/OutputProfiles.xsd'.

	In case of an error the method will display a message to the user
	and return FALSE.

	@param lpszPath
	        Path of the file to write the profile definitions to. If the file
	        already exists, it will be overwritten.

	@return
	        TRUE on success, FALSE otherwise.
	 */
	BOOL SaveXml(LPCTSTR lpszPath) const;

// attributes
private:
	/** Title of the active profile */
	mutable CString m_strActiveProfile;
};

#endif // !defined(AFX_PROFILE_H__6FC6E480_04F2_11D5_A222_006097239934__INCLUDED_)
