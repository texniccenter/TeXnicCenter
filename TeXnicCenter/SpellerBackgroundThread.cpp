/********************************************************************
 *
 * This file is part of the TeXnicCenter-system
 *
 * Copyright (C) 2002-$CurrentYear$ Chris Norris
 * Copyright (C) 2002-$CurrentYear$ ToolsCenter
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
#include "SpellerBackgroundThread.h"

#include "CodeView.h"
#include "Speller.h"
#include "DocumentTokenizer.h"
#include "configuration.h"
#include "EncodingConverter.h"
#include "SpellerSource.h"
#include "global.h"

const int ShortWordLength = 10;

IMPLEMENT_DYNCREATE(SpellerBackgroundThread, CWinThread)

BEGIN_MESSAGE_MAP(SpellerBackgroundThread, CWinThread)
	ON_THREAD_MESSAGE(AfxUserMessages::ID_BG_UPDATE_BUFFER, &SpellerBackgroundThread::OnUpdateBuffer)
	ON_THREAD_MESSAGE(AfxUserMessages::ID_BG_UPDATE_LINE, &SpellerBackgroundThread::OnUpdateLine)
	ON_THREAD_MESSAGE(AfxUserMessages::ID_BG_RESET_SPELLER, &SpellerBackgroundThread::OnGetSpeller)
	ON_THREAD_MESSAGE(AfxUserMessages::ID_BG_ENABLE_SPELLER, &SpellerBackgroundThread::OnEnableSpeller)
	//ON_THREAD_MESSAGE(AfxUserMessages::ID_BG_INVALIDATE_VIEW, &SpellerBackgroundThread::OnInvalidateView)
END_MESSAGE_MAP()


SpellerBackgroundThread::SpellerBackgroundThread()
: speller_(0), spell_enabled_(false)
, processing_(0)
{
}

void SpellerBackgroundThread::OnUpdateBuffer(WPARAM /*wParam*/, LPARAM lParam)
{
	::InterlockedExchange(&processing_,1);

	// Check if there are any redundant message in the message queue.
	// This should really improve speed on slower machines and machines
	// doing background processing.
	MSG msg;

	while (::PeekMessage(&msg, NULL, AfxUserMessages::ID_BG_UPDATE_BUFFER, AfxUserMessages::ID_BG_UPDATE_BUFFER, PM_NOREMOVE) != 0)
	{
		if (msg.lParam == lParam)
		{
			// Remove and discard the duplicate
			::PeekMessage(&msg, NULL, AfxUserMessages::ID_BG_UPDATE_BUFFER, AfxUserMessages::ID_BG_UPDATE_BUFFER, PM_REMOVE);
			continue;
		}
		break;
	}

	CodeView* pTextView = reinterpret_cast<CodeView*>(lParam);
	ASSERT_NULL_OR_POINTER(pTextView,CodeView);

	if (IsViewValid(pTextView)) 
	{
		if (spell_enabled_)
			SpellCheckBuffer(pTextView);
		else
			RemoveBufferAttributes(pTextView);
	}

	::InterlockedExchange(&processing_,0);
}

void SpellerBackgroundThread::OnUpdateLine(WPARAM wParam, LPARAM lParam)
{
	::InterlockedExchange(&processing_,1);

	CodeView *pTextView = (CodeView *)lParam;
	ASSERT_NULL_OR_POINTER(pTextView,CodeView);	

	OnUpdateLine(pTextView,static_cast<int>(wParam));

	::InterlockedExchange(&processing_,0);
}

void SpellerBackgroundThread::OnUpdateLine( CodeView * pTextView, int nLine )
{
	// Check if there are any redundant message in the message queue.
	// This should really improve speed on slower machines and machines
	// doing background processing.
	MSG msg;

	while (::PeekMessage(&msg, NULL, AfxUserMessages::ID_BG_UPDATE_BUFFER, AfxUserMessages::ID_BG_UPDATE_LINE, PM_NOREMOVE) != 0)
	{
		if (msg.lParam == reinterpret_cast<LPARAM>(pTextView))
		{
			// This buffer has a message. What could it be?
			if (msg.message == AfxUserMessages::ID_BG_UPDATE_BUFFER)
			{
				// A message to check the whole buffer is in the queue.
				// There is no point checking a single line -- we're done.
				return;
			}
			else if (msg.message == AfxUserMessages::ID_BG_UPDATE_LINE && msg.wParam == static_cast<WPARAM>(nLine))
			{
				// A duplicate message. Remove and discard.
				::PeekMessage(&msg, NULL, AfxUserMessages::ID_BG_UPDATE_LINE, AfxUserMessages::ID_BG_UPDATE_LINE, PM_REMOVE);
				continue;
			}
			// else  We need to handle this message
		}
		break;
	}

	if (IsViewValid(pTextView)) {
		if (spell_enabled_)
		{
			SpellCheckSingleLine(pTextView, nLine);
			Sleep(3000); // Give the typist a chance to get ahead to prevent flicker
		}
	}
}
void SpellerBackgroundThread::OnGetSpeller(WPARAM /*wParam*/, LPARAM lParam)
{
	SpellerSource *pSource = (SpellerSource *)lParam;

	if (pSource) {
		Speller *pSpell = pSource->GetSpeller();
		SetSpeller(pSpell);
	}
}

void SpellerBackgroundThread::OnEnableSpeller(WPARAM wParam, LPARAM lParam)
{
	bool bSpellEnabled = wParam != 0;

	if (bSpellEnabled == spell_enabled_)
		return;

	spell_enabled_ = bSpellEnabled;

	if (lParam != 0)
		OnUpdateBuffer(0, lParam);
}

BOOL SpellerBackgroundThread::InitInstance()
{
	return TRUE;
}

BOOL SpellerBackgroundThread::OnIdle(LONG /*lCount*/)
{
	if (!invalid_views_.empty()) {
		CSingleLock lock(&invalidate_monitor_,TRUE);
		invalid_views_.clear();
	}

	return 0;
}

void SpellerBackgroundThread::SetSpeller(Speller *pSpell)
{
	speller_ = pSpell;
}

void SpellerBackgroundThread::SpellCheckSingleLine(CodeView* pTextView, int nLine)
{
	if (speller_) {
		CScintillaCtrl& c = pTextView->GetCtrl();
		pTextView->Lock();

		pTextView->GetTokenizer()->ResetState();

		if (::IsWindow(c) && nLine < c.GetLineCount(FALSE))
			DoCheckLine(pTextView,nLine);

		pTextView->Unlock();
	}
}

void SpellerBackgroundThread::SpellCheckBuffer(CodeView* pTextView)
{
	if (speller_ == NULL)
		return;

	ASSERT(pTextView);
	pTextView->Lock();

	pTextView->GetTokenizer()->ResetState();

	int line = 0;

	CScintillaCtrl& c = pTextView->GetCtrl();
	const int line_count = c.GetLineCount(FALSE);

	CString text;

	while (line < line_count && IsViewValid(pTextView))
		DoCheckLine(pTextView,line++,text);

	pTextView->Unlock();
}

void SpellerBackgroundThread::RemoveBufferAttributes(CodeView* pTextView)
{
	ASSERT_NULL_OR_POINTER(pTextView,CodeView);
	pTextView->GetCtrl().IndicatorClearRange(0,pTextView->GetCtrl().GetLength(FALSE),FALSE);
}

bool SpellerBackgroundThread::IsViewValid( CodeView* view )
{
	bool result = (view != NULL);

	if (result) {
		CSingleLock lock(&invalidate_monitor_,TRUE);
		
		InvalidViewContainerType::iterator it = invalid_views_.find(static_cast<CodeView*>(view));
		result = it == invalid_views_.end();
	}

	return result;
}

void SpellerBackgroundThread::DoCheckLine(CodeView* view, int line, CString &text)
{
	CScintillaCtrl &c = view->GetCtrl();
	DocumentTokenizer* tokenizer = view->GetTokenizer();

	const long line_start = c.PositionFromLine(line,FALSE);
	const long line_end = c.GetLineEndPosition(line,FALSE);
	const long length = line_end - line_start;

	long last_decoration_end = line_start;

	if (length > 0) {
		text = view->GetLineText(line,false);
		int start = 0, end = 0;

		while (tokenizer->NextWord(text,text.GetLength(),start,end)) {
			int word_length = end - start;

			if (word_length > 0 && word_length < MAXWORDLEN) {
				const CString word(text.Mid(start,word_length));

				if (!speller_->Spell(word)) {
					bool decorate = true;
					// Special case for abbrev. like bzw., bzgl. and so on:
					// Make sure it's short word first and check whether there is
					// one more character after this word
					if (word_length <= ShortWordLength && end + 1 <= length) {
						// Include this character and recheck the spelling
						decorate = !speller_->Spell(word + text[end]);
					}

					if (decorate) {
						// Find word's start and end positions, add 1 otherwise
						// the previous word will be found
						//long s = c.WordStartPosition(line_start + start + 1,TRUE,FALSE);
						//long e = c.WordEndPosition(s,TRUE,FALSE);

						const CString left = text.Left(start);

						// We operate on UTF-8 bytes, thus the conversion
						std::vector<char> buffer1, buffer2;

#ifdef UNICODE
						UTF16toUTF8(left,left.GetLength(),buffer1);
						UTF16toUTF8(word,word.GetLength(),buffer2);
#else
						ANSItoUTF8(left,left.GetLength(),buffer1);
						ANSItoUTF8(word,word.GetLength(),buffer2);
#endif
						const long s = line_start + static_cast<long>(buffer1.size());
						const long e = s + static_cast<long>(buffer2.size());

						// Clear the line indicators from the previous mis-spelled word to here
						c.IndicatorClearRange(last_decoration_end, s - last_decoration_end, false);
						last_decoration_end = e; //For the next clearing

						// Indicate the mis-spelled word using a line decoration
						c.IndicatorFillRange(s, e - s, false);
					}
				}
			}

			start = end;
		}

		// Clear the line indicators from the previous mis-spelled word (or start of the line) to the end of the line
		c.IndicatorClearRange(last_decoration_end, line_end - last_decoration_end, false);
	}
}

void SpellerBackgroundThread::DoCheckLine(CodeView* view, int line)
{
	CString text;
	DoCheckLine(view,line,text);
}

void SpellerBackgroundThread::RecheckSpelling( CodeView* view )
{
	PostThreadMessage(AfxUserMessages::ID_BG_UPDATE_BUFFER,0,reinterpret_cast<LPARAM>(view));
}

void SpellerBackgroundThread::RecheckSingleLineSpelling( int line, CodeView* view )
{
	PostThreadMessage(AfxUserMessages::ID_BG_UPDATE_LINE,static_cast<WPARAM>(line),reinterpret_cast<LPARAM>(view));
}

void SpellerBackgroundThread::EnableSpeller( bool enable /*= true*/ )
{
	PostThreadMessage(AfxUserMessages::ID_BG_ENABLE_SPELLER,enable,0);
}

void SpellerBackgroundThread::ResetSpeller( SpellerSource* s )
{
	PostThreadMessage(AfxUserMessages::ID_BG_RESET_SPELLER,0,reinterpret_cast<LPARAM>(s));
}

void SpellerBackgroundThread::InvalidateView( CodeView* view )
{
	if (processing_) {
		// Invalidate a view on if the buffer is being processed
		CSingleLock lock(&invalidate_monitor_,TRUE);
		invalid_views_.insert(view);
	}
}
