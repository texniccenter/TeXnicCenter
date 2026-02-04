#pragma once

class StructureParserCommand
{
public:
	StructureParserCommand()
		:nFlags(std::regex_constants::match_default)
		,idxMatchGroup(0)
	{}

	~StructureParserCommand()
	{}

	/** Parses the given text.
	
		@returns True, if the regular expression found a match
		and we actually have a LaTeX command here.
	*/
	bool Match(LPCTSTR lpText, LPCTSTR lpTextEnd, std::match_results<LPCTSTR>& MatchResult) const;

	///Name of this command.
	CString Name;

	///Description for this command.
	CString Description;

	///Defines how to parse LaTeX to find the command and its arguments.
	tregex RegularExpression;

	///Flags to use for matching.
	std::regex_constants::match_flag_type nFlags;

	///Which match group holds the desired content, i.e., a filename, title, caption, etc.
	int idxMatchGroup;

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
	bool IsCmdAt(LPCTSTR lpText, int nPos) const;
};


/** A parser command definition for input-like commands.
*/
class StructureParserCommandTeXFile : public StructureParserCommand
{
public:
	StructureParserCommandTeXFile()
		:StructureParserCommand()
	{}

	~StructureParserCommandTeXFile()
	{}

public:
};


/** A parser command definition for section-like commands, i.e., headings.
*/
class StructureParserCommandHeading : public StructureParserCommand
{
public:
	StructureParserCommandHeading()
		:StructureParserCommand()
	{}

	~StructureParserCommandHeading()
	{}

public:
	///Depth of the heading
	int Depth;

public:
	static const std::map<const CString, const int> HeadingTypeToDepth;
};


/** An array of user-defined parser commands of a given type.
* 
*	Call Match() to get the index of the first matching command in the array.
*/
template <typename T>
class StructureParserCommandList : public std::vector<T>
{
public:
	StructureParserCommandList()
	{}

	virtual ~StructureParserCommandList()
	{}

	///Parses all commands, stops at the first match.
	///Returns -1, if no command matches, otherwise the index of the matching command.
	int Match(LPCTSTR lpText, LPCTSTR lpTextEnd, std::match_results<LPCTSTR>& MatchResult) const
	{
		for (int i(0);i<size();i++)
		{
			if ((*this)[i].Match(lpText, lpTextEnd, MatchResult)) return i;
		}

		return -1;
	}

};
