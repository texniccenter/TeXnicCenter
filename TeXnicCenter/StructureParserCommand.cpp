#include "stdafx.h"
#include "StructureParserCommand.h"

bool StructureParserCommand::Match(LPCTSTR lpText, LPCTSTR lpTextEnd, std::match_results<LPCTSTR>& MatchResult)
{
	return (regex_search(lpText, lpTextEnd, MatchResult, RegularExpression, nFlags) && IsCmdAt(lpText, MatchResult[0].first - lpText));
}

bool StructureParserCommand::IsCmdAt(LPCTSTR lpText, int nPos)
{
	// count number of backslashes before command (including command backslash)
	int nCount = 0;
	for (; nPos >= 0 && *(lpText + nPos) == _T('\\'); nCount++, nPos--)
		;
	return !(nCount % 2 == 0);
}


StructureParserCommandPtr StructureParserCommandList::Match(LPCTSTR lpText, LPCTSTR lpTextEnd, std::match_results<LPCTSTR>& MatchResult)
{
	for (StructureParserCommandPtr& CmdPtr : *this)
	{
		if (CmdPtr->Match(lpText, lpTextEnd, MatchResult)) return CmdPtr;
	}

	return NULL;
}
