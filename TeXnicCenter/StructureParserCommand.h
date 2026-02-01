#pragma once
#include "stdafx.h"

class StructureParserCommand
{
public:
	StructureParserCommand()
		:nFlags(std::regex_constants::match_default)
	{}

	~StructureParserCommand()
	{}


	/** Parses the given text.
	
		@returns True, if the regular expression found a match
		and we actually have a LaTeX command here.
	*/
	bool Match(LPCTSTR lpText, LPCTSTR lpTextEnd, std::match_results<LPCTSTR>& MatchResult);

	///Name of this command.
	CString Name;

	///Description for this command.
	CString Description;

	///Defines how to parse LaTeX to find the command and its arguments.
	tregex RegularExpression;

	///Flags to use for matching.
	std::regex_constants::match_flag_type nFlags;

protected:
	/**	Checks, if there is a LaTeX-command at the specified position.

	The method simply counts the number of backslashes from the
	given position to the beginning of the string. If the number
	modular 2 is zero, then there is no command, otherwise there
	is one.

	@param lpText
	        String that contains the possible command
	        (not guaranteed to be terminated by null-character).
	@param nPos
	        Zero-based index of the first character of the possible
	        command (the backslash).

	@return
	        TRUE if there is a command at nPos, FALSE otherwise.
	 */
	static bool IsCmdAt(LPCTSTR lpText, int nPos);
};

///A shared pointer to a structure parser command
typedef std::shared_ptr<StructureParserCommand> StructureParserCommandPtr;

template<typename T>
StructureParserCommandPtr CreateNewStructureParserCommand()
{
	return std::make_shared<T>();
}

/** A regular expression parser for input-like commands of tex-files.
*/
class StructureParserCommandTeXFile : public StructureParserCommand
{
public:
	StructureParserCommandTeXFile()
		:StructureParserCommand()
		,idxMatchGroup(1)
	{
	}

	~StructureParserCommandTeXFile()
	{
	}

	/// Returns index of group capturing the filename in the regular expression.
	const int GetFileNameIndex() const
	{
		return idxMatchGroup;
	}

public:
	///Which match group holds the filename.
	int idxMatchGroup;
};


class StructureParserCommandList : public std::vector<StructureParserCommandPtr>
{
public:
	StructureParserCommandList()
	{}

	virtual ~StructureParserCommandList()
	{}

	///Parses all commands, stops at the first match.
	///Returns Null, if no command matches, otherwise the pointer to the matching command.
	StructureParserCommandPtr Match(LPCTSTR lpText, LPCTSTR lpTextEnd, std::match_results<LPCTSTR>& MatchResult);
};