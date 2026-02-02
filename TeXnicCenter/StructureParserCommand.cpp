#include "stdafx.h"
#include "StructureParserCommand.h"

bool StructureParserCommand::Match(LPCTSTR lpText, LPCTSTR lpTextEnd, std::match_results<LPCTSTR>& MatchResult) const
{
	return (regex_search(lpText, lpTextEnd, MatchResult, RegularExpression, nFlags) && IsCmdAt(lpText, MatchResult[0].first - lpText));
}

bool StructureParserCommand::IsCmdAt(LPCTSTR lpText, int nPos) const
{
	// count number of backslashes before command (including command backslash)
	int nCount = 0;
	for (; nPos >= 0 && *(lpText + nPos) == _T('\\'); nCount++, nPos--)
		;
	return !(nCount % 2 == 0);
}
