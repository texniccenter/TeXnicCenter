#include "stdafx.h"

#include <sstream>
#include <string>
#include <functional>
#include <iterator>
#include <cstring>
#include <cstddef>

#include "resource.h"
#include "CodeDocument.h"
#include "CodeView.h"
#include "gotodialog.h"
#include "configuration.h"
#include "CharType.h"
#include "FindReplaceDlg.h"
#include "EncodingConverter.h"
#include "LaTeXTokenizer.h"
#include "SpellerBackgroundThread.h"
#include "global.h"

#pragma push_macro("max")
#undef max

#pragma region Helper functions

int GetLogFontPointSize(const LOGFONT& lf)
{
	CWindowDC dc(0);

	const LOGFONT& editor_font = lf;

	CFont font;
	font.CreateFontIndirect(&editor_font);

	CFont* oldfont = dc.SelectObject(&font);

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	dc.SelectObject(&oldfont);

	// Convert logical units to points
	return ::MulDiv(tm.tmHeight - tm.tmInternalLeading,72,dc.GetDeviceCaps(LOGPIXELSY));
}


#pragma endregion

// CodeView

IMPLEMENT_DYNAMIC(CodeView, CScintillaView)

CodeView::CodeView()
: hold_count_(0)
, hard_wrap_enabled_(false)
, remove_leading_spaces_(true)
, suppress_speller_(false)
, shadow_(this)
, last_change_pos_(-1)
, topline_from_serialization_(-1)
{
}

CodeView::~CodeView()
{
	ASSERT(hold_count_ == 0);
}

BEGIN_MESSAGE_MAP(CodeView, CScintillaView)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_EDIT_GOTO, &CodeView::OnEditGoto)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CodeView::OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INDICATOR_ENCODING, &CodeView::OnUpdateEncodingIndicator)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INDICATOR_SELECTION, &CodeView::OnUpdateSelectionIndicator)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INDICATOR_POSITION, &CodeView::OnUpdatePositionIndicator)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INDICATOR_CRLF, &CodeView::OnUpdateEOLModeIndicator)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WHITE_SPACE, &CodeView::OnUpdateViewWhiteSpace)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LINE_ENDING, &CodeView::OnUpdateViewShowLineEnding)
	ON_COMMAND(ID_VIEW_WORD_WRAP, &CodeView::OnViewWordWrap)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WORD_WRAP, &CodeView::OnUpdateViewWordWrap)
	ON_COMMAND(ID_VIEW_WORD_WRAP_INDENT, &CodeView::OnViewWordWrapIndent)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WORD_WRAP_INDENT, &CodeView::OnUpdateViewWordWrapIndent)
	ON_COMMAND(ID_VIEW_WORDWRAPINDICATORS_START, &CodeView::OnViewWordWrapIndicatorsStart)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WORDWRAPINDICATORS_START, &CodeView::OnUpdateViewWordWrapIndicators)
	ON_COMMAND(ID_VIEW_WORDWRAPINDICATORS_END, &CodeView::OnViewWordWrapIndicatorsEnd)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WORDWRAPINDICATORS_END, &CodeView::OnUpdateViewWordWrapIndicators)
	ON_COMMAND(ID_VIEW_FOLDMARGIN, &CodeView::OnViewFoldMargin)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FOLDMARGIN, &CodeView::OnUpdateViewFoldMargin)
	ON_COMMAND(ID_VIEW_WHITE_SPACE, &CodeView::OnViewWhiteSpace)
	ON_COMMAND(ID_VIEW_LINE_ENDING, &CodeView::OnViewLineEnding)	
	ON_WM_SYSCOLORCHANGE()
	ON_WM_DESTROY()	
	ON_COMMAND(ID_VIEW_LINE_NUMBERS, &CodeView::OnViewLineNumbers)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LINE_NUMBERS, &CodeView::OnUpdateViewLineNumbers)
	ON_COMMAND(ID_EDIT_GOTO_NEXT_BOOKMARK, &CodeView::OnEditGotoNextBookmark)
	ON_COMMAND(ID_EDIT_GOTO_PREV_BOOKMARK, &CodeView::OnEditGotoPrevBookmark)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GOTO_PREV_BOOKMARK, &CodeView::OnUpdateEditGotoPrevBookmark)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GOTO_NEXT_BOOKMARK, &CodeView::OnUpdateEditGotoNextBookmark)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR_ALL_BOOKMARKS, &CodeView::OnUpdateEditClearAllBookmarks)
	ON_WM_CREATE()
	ON_COMMAND(ID_EDIT_FIND_INCREMENTAL_FORWARD, &CodeView::OnEditFindIncrementalForward)
	ON_COMMAND(ID_EDIT_FIND_INCREMENTAL_BACKWARD, &CodeView::OnEditFindIncrementalBackward)
	ON_COMMAND(ID_SEARCH_FINDNEXTSELECTED, &CodeView::OnSearchFindNextSelected)
	ON_COMMAND(ID_SEARCH_FINDPREVIOUSSELECTED, &CodeView::OnSearchFindPreviousSelected)
	ON_COMMAND(ID_EDIT_GOTO_LAST_CHANGE, &CodeView::OnEditGotoLastChange)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GOTO_LAST_CHANGE, &CodeView::OnUpdateEditGotoLastChange)
	ON_COMMAND(ID_VIEW_INDENTATION_GUIDES, &CodeView::OnViewIndentationGuides)
	ON_UPDATE_COMMAND_UI(ID_VIEW_INDENTATION_GUIDES, &CodeView::OnUpdateViewIndentationGuides)
	ON_COMMAND(ID_EDIT_SEL_BIGGER_BLOCK, &CodeView::OnEditSelBiggerBlock)
	ON_COMMAND(ID_EDIT_SELECTPARAGRAPH, &CodeView::OnEditSelParagraph)
	ON_COMMAND(ID_EDIT_DELETE_LINE, &CodeView::OnEditDeleteLine)
	ON_COMMAND(ID_ADVANCED_DUPLICATE, &CodeView::OnEditDuplicateLine)
	ON_MESSAGE_VOID(AfxUserMessages::CheckForFileChangesMessageID, OnCheckForFileChanges)
	ON_COMMAND(ID_VIEW_HIGHLIGHTACTIVELINE, &CodeView::OnViewHighlightActiveLine)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HIGHLIGHTACTIVELINE, &CodeView::OnUpdateViewHighlightActiveLine)
	ON_MESSAGE(AfxUserMessages::CStringLineTextMessageID, &CodeView::OnGetLineText)
	ON_MESSAGE(WM_COMMANDHELP, &CodeView::OnCommandHelp)
END_MESSAGE_MAP()


// CodeView diagnostics

#ifdef _DEBUG
void CodeView::AssertValid() const
{
	CScintillaView::AssertValid();
}

#ifndef _WIN32_WCE
void CodeView::Dump(CDumpContext& dc) const
{
	CScintillaView::Dump(dc);
}
#endif
#endif //_DEBUG


long LongFromTwoShorts(short a,short b)
{
	return (a) | ((b) << 16);
}

void AssignScintillaKey(CScintillaCtrl& rCtrl, int key, int mods, int cmd)
{
	rCtrl.AssignCmdKey(LongFromTwoShorts(static_cast<short>(key), static_cast<short>(mods)),
						cmd, 1);
}

void ClearScintillaKey(CScintillaCtrl& rCtrl, int key, int mods)
{
	rCtrl.ClearCmdKey(LongFromTwoShorts(static_cast<short>(key), static_cast<short>(mods)), 1);
}

// CodeView message handlers

int CodeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CScintillaView::OnCreate(lpCreateStruct) == -1)
		return -1;

	shadow_.SubclassWindow(GetCtrl());

	theme_.SubclassWindow(m_hWnd);
	theme_.Initialize();

	//Get a shorthand
	CScintillaCtrl& rCtrl = GetCtrl();

	// The lexer has to be set before fold settings are modified
	rCtrl.SetLexer(GetLexer());

	EnableFolding(CConfiguration::GetInstance()->IsFoldingEnabled()); // CodeView::OnSettingsChanged depends on whether folding is enabled or not
	OnSettingsChanged(); // Make sure derived class setup the settings such as fonts, lexers etc. first

	rCtrl.SetViewWS(CConfiguration::GetInstance()->m_bViewWhitespaces ? SCWS_VISIBLEALWAYS : SCWS_INVISIBLE);
	rCtrl.SetViewEOL(CConfiguration::GetInstance()->GetShowLineEnding());
	rCtrl.SetWrapMode(CConfiguration::GetInstance()->IsWordWrapEnabled() ? SC_WRAP_WORD : SC_WRAP_NONE);
	rCtrl.SetWrapIndentMode(CConfiguration::GetInstance()->IsWordWrapIndentEnabled() ? SC_WRAPINDENT_SAME : SC_WRAPINDENT_FIXED);
	rCtrl.SetWrapVisualFlags(CConfiguration::GetInstance()->GetWordWrapIndicators());
	rCtrl.SetWrapStartIndent(0); //That is our default; we do not put it in the UI.

	UpdateLineNumberMargin();

	//Selection Margin
	rCtrl.SetMarginWidthN(2, 16);
	rCtrl.SetMarginTypeN(2, SC_MARGIN_BACK);

#pragma region Markers

	rCtrl.SetMarginSensitiveN(1, TRUE); // React on clicks
	rCtrl.MarkerDefine(CodeDocument::Errormark, SC_MARK_ARROW);
	rCtrl.MarkerDefine(CodeDocument::Bookmark, SC_MARK_SMALLRECT);

#pragma endregion

#pragma region Caret

	HighlightActiveLine(CConfiguration::GetInstance()->m_bHighlightCaretLine);

#pragma endregion

#pragma region Multiple Selections

	//We enable multiple selections by default.
	//This is not optional, since the single and multiple modes are easy to distinguish for the user.
	rCtrl.SetMultipleSelection(true);
	rCtrl.SetAdditionalSelectionTyping(true);
	rCtrl.SetMultiPaste(SC_MULTIPASTE_EACH);
	rCtrl.SetAdditionalCaretsBlink(false);

#pragma endregion

#pragma region Folding

	const int folding_margin_num = GetFoldingMargin();

	rCtrl.SetMarginSensitiveN(folding_margin_num,TRUE); // Receive clicks
	rCtrl.SetMarginTypeN(folding_margin_num,SC_MARGIN_SYMBOL);
	rCtrl.SetMarginMaskN(folding_margin_num,SC_MASK_FOLDERS);

#pragma endregion

#pragma region Misc Scintilla Options

	// Indicator for spell checker errors
	rCtrl.SetIndicatorCurrent(0); // Default is a squiggly line, which we want to use

	rCtrl.SetIndent(0); //Indent same as tab size
	ShowIndentationGuides(CConfiguration::GetInstance()->GetShowIndentationGuides());

	//Size of the white space dots. Default is 1, but that is too small.
	rCtrl.SetWhitespaceSize(2);

	//Virtual Space is only enabled for rectangular selections.
	//It could be an option in general, but I don't see its value in a LaTeX editor.
	rCtrl.SetVirtualSpaceOptions(SCVS_RECTANGULARSELECTION);

	//rCtrl.SetWordChars: We take the default definition of what makes up a word.
	//rCtrl.SetWhitespaceChars: Use default.
	// Users are used to this from other editors.

	rCtrl.SetPasteConvertEndings(TRUE); // Convert line endings

	rCtrl.SetEdgeColumn(CConfiguration::GetInstance()->m_nVerticalEdgeColumn);
	rCtrl.SetEdgeMode(CConfiguration::GetInstance()->m_nVerticalEdgeMode);
	rCtrl.SetEdgeColour(CConfiguration::GetInstance()->m_aVariableEdgeColor);

	//We handle the context menu ourselves
	rCtrl.UsePopUp(FALSE);

#pragma endregion

#pragma region Scintilla Keyboard Assignments

	//Wrap-aware home/end keys
	AssignScintillaKey(rCtrl, SCK_HOME, 0, SCI_VCHOMEWRAP);
	AssignScintillaKey(rCtrl, SCK_HOME, SCMOD_SHIFT, SCI_VCHOMEWRAPEXTEND);
	AssignScintillaKey(rCtrl, SCK_HOME, SCMOD_SHIFT | SCMOD_ALT, SCI_VCHOMERECTEXTEND);
	AssignScintillaKey(rCtrl, SCK_END, 0, SCI_LINEENDWRAP);
	AssignScintillaKey(rCtrl, SCK_END, SCMOD_SHIFT, SCI_LINEENDWRAPEXTEND);

	//Take away the shortcuts for Upper/Lowercase, since we want the user to be able to customize that
	// I found, that we do not need to do this, since we process the keyboard shortcuts first.
	// And if we defined Ctrl-U with something, Scintilla will just not get it.
	// So let's leave the defaults.
	//ClearScintillaKey(rCtrl, 'U', SCMOD_CTRL);
	//ClearScintillaKey(rCtrl, 'U', SCMOD_CTRL | SCMOD_SHIFT);
	//Similar for: LineCut, SelectionDuplicate

#pragma endregion

	return 0;
}


void CodeView::GoToLine( int line, bool direct /*= true*/ )
{
	//Go to the given line and put it into display such that it is not directly at the borders.
	GetCtrl().SetYCaretPolicy(CARET_SLOP | CARET_STRICT, 5, direct);
	GetCtrl().EnsureVisibleEnforcePolicy(line, direct); //If folding is on, this is needed.
	GetCtrl().GotoLine(line, direct);
	GetCtrl().SetYCaretPolicy(CARET_EVEN, 1, direct); //Restore default policy
}

int CodeView::GetLineLength( int line, bool direct /*= true*/ )
{
	return GetCtrl().GetLineEndPosition(line, direct) -
		GetCtrl().PositionFromLine(line, direct);
}

int CodeView::GetLineCount( bool direct /*= true*/ )
{
	return GetCtrl().GetLineCount(direct);
}

int CodeView::GetCurrentLine( bool direct /*= true*/ )
{
	return GetCtrl().LineFromPosition(GetCtrl().GetCurrentPos(direct),direct);
}

CString CodeView::GetLineText(int line, bool direct /*= true*/)
{
	CString strLine;

	if (direct) {
		const int length = GetCtrl().GetLine(line, NULL, direct);

		if (length > 0) {
			CStringA temp;

			GetCtrl().GetLine(line, temp.GetBuffer(length), direct);
			temp.ReleaseBuffer(length);

#ifdef UNICODE
			std::vector<wchar_t> buffer;
			UTF8toUTF16(temp, temp.GetLength(), buffer);
#else
			std::vector<char> buffer;
			UTF8toANSI(temp, temp.GetLength(), buffer);
#endif

			if (!buffer.empty())
				strLine.SetString(&buffer[0], static_cast<int>(buffer.size()));
		}
	}
	else
		SendMessage(CStringLineTextMessageID, line, reinterpret_cast<LPARAM>(&strLine));

	return strLine;
}

void CodeView::InsertText( const CString& text )
{
	long pos = GetCtrl().GetCurrentPos();
	GetCtrl().InsertText(pos,text);
	GetCtrl().GotoPos(pos + text.GetLength());
}

int CodeView::Lock(bool exclusive /*= false */)
{
	ASSERT(hold_count_ >= 0);

	if (exclusive) {
		MSG msg;
		
		// Adjust the min/max range if new message are added to Scintilla
		const UINT min = SCI_START;
		const UINT max = SCI_SHOWCURSOR;

		while (hold_count_ > 0) { // atomic operation, no lock needed
			// Let the thread process remaining Scintilla messages
			// sent by the speller otherwise a dead lock will occur.
			while (::PeekMessage(&msg,GetCtrl(),min,max,PM_REMOVE) > 0) {
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}

		::InterlockedExchange(&hold_count_,1);
	}
	else
		::InterlockedIncrement(&hold_count_);

	ASSERT(hold_count_ > 0);
	return hold_count_;
}

int CodeView::Unlock(bool exclusive /*= false */)
{
	ASSERT(hold_count_ > 0);
	int hold_count;

	if (exclusive) {
		// Lock( true ) was not called
		ASSERT(hold_count_ == 1);

		::InterlockedExchange(&hold_count_,0);
		hold_count = 0;
	}
	else {
		::InterlockedDecrement(&hold_count_);
		hold_count = hold_count_;
	}

	return hold_count;
}

void CodeView::OnFilePrintPreview()
{
	AFXPrintPreview(this);
}

void CodeView::OnUpdateEncodingIndicator( CCmdUI* pCmdUI )
{
	LPCTSTR text = 0;

	switch (GetDocument()->GetEncoding()) {
		case CodeDocument::UTF8: text = _T("UTF-8"); break;
		case CodeDocument::UTF16LE: text = _T("UTF-16"); break;
		case CodeDocument::UTF16BE: text = _T("UTF-16BE"); break;
		case CodeDocument::UTF32LE: text = _T("UTF-32"); break;
		case CodeDocument::UTF32BE: text = _T("UTF-32BE"); break;
		case CodeDocument::ANSI: text = _T("ANSI"); break;
		default: ASSERT(FALSE);
	}

	pCmdUI->SetText(text);
	pCmdUI->Enable();
}

void CodeView::OnUpdateEOLModeIndicator( CCmdUI* pCmdUI )
{
	LPCTSTR text = 0;
	int mode = GetCtrl().GetEOLMode();

	switch (mode) {
		case SC_EOL_CRLF: text = _T("CR+LF"); break;
		case SC_EOL_LF: text = _T("LF"); break;
		case SC_EOL_CR: text = _T("CR"); break;
		default: ASSERT(FALSE); break;
	}

	pCmdUI->SetText(text);
	pCmdUI->Enable();
}

void CodeView::OnUpdatePositionIndicator( CCmdUI* pCmdUI )
{
	CScintillaCtrl& rCtrl = GetCtrl();
	const int pos = rCtrl.GetCurrentPos();
	const int line = rCtrl.LineFromPosition(pos);
	const int col = rCtrl.GetColumn(pos);

	CString text;
	text.Format(IDS_CODE_VIEW_INDICATOR, line + 1, col + 1, pos + 1);

	pCmdUI->SetText(text);
	pCmdUI->Enable();
}

void CodeView::OnUpdateSelectionIndicator( CCmdUI* pCmdUI )
{
	CScintillaCtrl& rCtrl = GetCtrl();
	const int nSel = rCtrl.GetSelections();

	CString text;
	if (nSel > 1)
	{
		text.Format(IDS_CODE_VIEW_INDICATOR_SELECTION, nSel);
	}

	pCmdUI->SetText(text);
	pCmdUI->Enable();
}

void CodeView::TextNotFound(LPCTSTR /*lpszFind*/, BOOL /*bNext*/, BOOL /*bCase*/, BOOL /*bWord*/, BOOL /*bRegularExpression*/, BOOL /*bReplaced*/)
{
}

DocumentTokenizer* CodeView::GetTokenizer() const
{
	// Create the tokenizer only on demand
	if (!tokenizer_.get())
		tokenizer_.reset(NewDocumentTokenizer());

	return tokenizer_.get();
}

void CodeView::OnDestroy()
{
	if (GetDocument()->IsSpellerThreadAttached()) {
		CWaitCursor wc;
		// Inform the background thread we're being destroyed
		GetDocument()->GetSpellerThread()->InvalidateView(this);
		// Wait for the background thread to stop
		Lock(true);	
		Unlock(true);
	}

	CScintillaView::OnDestroy();
}

void CodeView::OnViewWhiteSpace()
{
	bool visible = GetCtrl().GetViewWS() != SCWS_INVISIBLE;
	int mode = visible ? SCWS_INVISIBLE : SCWS_VISIBLEALWAYS;

	CConfiguration::GetInstance()->m_bViewWhitespaces = !visible;

	using namespace std;
	using namespace placeholders;

	// Chained update: call for every view GetCtrl().SetViewWS(!visible,TRUE)
	ForEveryView(bind(bind(&CScintillaCtrl::SetViewWS,_1,mode,TRUE),
		bind(&CodeView::GetCtrl,_1)));
}

void CodeView::OnViewLineEnding()
{
	bool visible = !GetCtrl().GetViewEOL();
	CConfiguration::GetInstance()->SetShowLineEnding(visible);

	using namespace std;
	using namespace placeholders;

	// Chained update: call for every view GetCtrl().SetViewEOL(visible,TRUE)
	ForEveryView(bind(bind(&CScintillaCtrl::SetViewEOL,_1,visible,TRUE),
		bind(&CodeView::GetCtrl,_1)));
}

void CodeView::OnViewWordWrap()
{
#if 0 // Hard wrap test code
	long s = GetCtrl().GetSelectionStart();
	long e = GetCtrl().GetSelectionEnd();

	if (s != e)
		HardWrapRange(s,e);
#endif // 0

	bool wrap = GetCtrl().GetWrapMode() != SC_WRAP_WORD;
	CConfiguration::GetInstance()->EnableWordWrap(wrap);

	int mode = wrap ? SC_WRAP_WORD : SC_WRAP_NONE;

	using namespace std;
	using namespace placeholders;

	// Chained update: call every view
	ForEveryView(bind(bind(&CScintillaCtrl::SetWrapMode,_1,mode,TRUE),
		bind(&CodeView::GetCtrl,_1)));
}

void CodeView::OnViewWordWrapIndent()
{
	const bool indent = GetCtrl().GetWrapIndentMode() != SC_WRAPINDENT_SAME;
	CConfiguration::GetInstance()->EnableWordWrapIndent(indent);

	int mode = indent ? SC_WRAPINDENT_SAME : SC_WRAPINDENT_FIXED;

	using namespace std;
	using namespace placeholders;

	// Chained update: call every view
	ForEveryView(bind(bind(&CScintillaCtrl::SetWrapIndentMode,_1,mode,TRUE),
		bind(&CodeView::GetCtrl,_1)));
}

void CodeView::OnViewWordWrapIndicatorsStart()
{
	const int CurrentFlags = GetCtrl().GetWrapVisualFlags();
	const int NewFlags = (CurrentFlags & 2) ? CurrentFlags - 2 : CurrentFlags + 2;
	ASSERT(NewFlags >= 0 && NewFlags <= 3);
	
	CConfiguration::GetInstance()->SetWordWrapIndicators(NewFlags);

	using namespace std;
	using namespace placeholders;

	// Chained update: call every view
	ForEveryView(bind(bind(&CScintillaCtrl::SetWrapVisualFlags,_1,NewFlags,TRUE),
		bind(&CodeView::GetCtrl,_1)));
}

void CodeView::OnViewWordWrapIndicatorsEnd()
{
	const int CurrentFlags = GetCtrl().GetWrapVisualFlags();
	const int NewFlags = (CurrentFlags & 1) ? CurrentFlags - 1 : CurrentFlags + 1;
	ASSERT(NewFlags >= 0 && NewFlags <= 3);
	
	CConfiguration::GetInstance()->SetWordWrapIndicators(NewFlags);

	using namespace std;
	using namespace placeholders;

	// Chained update: call every view
	ForEveryView(bind(bind(&CScintillaCtrl::SetWrapVisualFlags,_1,NewFlags,TRUE),
		bind(&CodeView::GetCtrl,_1)));
}

void CodeView::OnUpdateViewWordWrapIndicators(CCmdUI *pCmdUI)
{
	const int CurrentFlags = GetCtrl().GetWrapVisualFlags();

	switch (pCmdUI->m_nID)
	{
		case ID_VIEW_WORDWRAPINDICATORS_START:
			pCmdUI->SetCheck(CurrentFlags >= 2);
			break;
		case ID_VIEW_WORDWRAPINDICATORS_END:
			pCmdUI->SetCheck(CurrentFlags & 1);
			break;
		default:
			ASSERT(false);
	}
}

void CodeView::OnViewFoldMargin()
{
	bool enable = !IsFoldingEnabled();

	using namespace std::placeholders;
	EnableFolding(!IsFoldingEnabled());

	// Chained update
	ForEveryView(std::bind(&CodeView::EnableFolding, _1, enable));
}

void CodeView::OnUpdateViewFoldMargin(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(IsFoldingEnabled());
}

void CodeView::OnViewLineNumbers()
{
	CConfiguration::GetInstance()->m_bShowLineNumbers =
		!CConfiguration::GetInstance()->m_bShowLineNumbers;

	// Chained update
	ForEveryView(std::mem_fun(&CodeView::UpdateLineNumberMargin));
}

void CodeView::OnUpdateViewWhiteSpace(CCmdUI *pCmdUI)
{
	bool visible = GetCtrl().GetViewWS() != SCWS_INVISIBLE;
	pCmdUI->SetCheck(visible);
}

void CodeView::OnUpdateViewShowLineEnding(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(GetCtrl().GetViewEOL());
}

void CodeView::OnEditDeleteLine()
{
	GetCtrl().LineCut();
}

void CodeView::OnEditDuplicateLine()
{
	GetCtrl().SelectionDuplicate();
}

void CodeView::OnInitialUpdate()
{
	CScintillaView::OnInitialUpdate();
}

void CodeView::OnSettingsChanged()
{
	// User changed application's settings: react here

	UpdateFoldMargin();
	UpdateFoldSettings();

	GetCtrl().SetUseTabs(!CConfiguration::GetInstance()->GetUseSpaces());
	GetCtrl().SetTabWidth(CConfiguration::GetInstance()->m_nTabWidth);
	GetCtrl().SetIndent(GetCtrl().GetTabWidth());
	GetCtrl().SetEdgeColumn(CConfiguration::GetInstance()->m_nVerticalEdgeColumn);
	GetCtrl().SetEdgeMode(CConfiguration::GetInstance()->m_nVerticalEdgeMode);
	GetCtrl().SetEdgeColour(CConfiguration::GetInstance()->m_aVariableEdgeColor);

	GetCtrl().SetCaretStyle(CConfiguration::GetInstance()->m_nInsertCaretStyle);
	GetCtrl().SetCaretPeriod(CConfiguration::GetInstance()->m_nInsertCaretBlinkPeriod);
}

void CodeView::OnSysColorChange()
{
	CScintillaView::OnSysColorChange();
	OnSettingsChanged();
}

void CodeView::OnUpdateViewWordWrap(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(GetCtrl().GetWrapMode() == SC_WRAP_WORD);
}

void CodeView::OnUpdateViewWordWrapIndent(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(GetCtrl().GetWrapIndentMode() == SC_WRAPINDENT_SAME);
	pCmdUI->Enable(GetCtrl().GetWrapMode() == SC_WRAP_WORD);
}

void CodeView::OnEditGoto()
{
	CGotoDialog dlg(GetLineCount(),this);
	dlg.m_nLine = GetCurrentLine() + 1;

	if (dlg.DoModal() == IDOK)
		GoToLine(dlg.m_nLine - 1);
}

void CodeView::OnUpdateViewLineNumbers(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(IsLineMarginVisible());
}

void CodeView::UpdateLineNumberMargin()
{
	CScintillaCtrl& rCtrl = GetCtrl();

	if (CConfiguration::GetInstance()->m_bShowLineNumbers) {
		int line_count = rCtrl.GetLineCount();

		// Calculate the number of digits
		// used to display the line_count value
		int digits = 0;

		while (line_count) {
			line_count /= 10;
			++digits;
		}

		std::string number(std::max(digits,2),'9'); // Put 9 line_count times to measure the width
		number.insert(number.begin(),'_'); // Some padding

		const int width = rCtrl.TextWidth(STYLE_LINENUMBER,number.c_str());
		rCtrl.SetMarginWidthN(0,width);
	}
	else {
		rCtrl.SetMarginWidthN(0,0); // Margin invisible
	}
}

bool CodeView::IsLineMarginVisible()
{
	return GetCtrl().GetMarginWidthN(0) > 0;
}

void CodeView::OnCharAdded(SCNotification* n)
{
	if (!suppress_speller_) {
		if (GetDocument()->IsSpellerThreadAttached())
			GetDocument()->GetSpellerThread()->RecheckSingleLineSpelling(GetCurrentLine(),this);
	}

	// Hard wrapping
	if (hard_wrap_enabled_) {
		if (n->ch != _T('\n') && n->ch != _T('\r')) {
			GetCtrl().BeginUndoAction();

			long pos = GetCtrl().GetCurrentPos();
			int line = GetCtrl().LineFromPosition(pos);
			long s = GetCtrl().PositionFromLine(line);
			
			if (GetCtrl().GetCurrentPos() - s >= CConfiguration::GetInstance()->m_nFixedColumnWrap) {
				long e = GetCtrl().GetLineEndPosition(line);
				bool stop = false;

				while (e >= s && !stop) {
					if (CharTraitsT::IsSpace(GetCtrl().GetCharAt(e))) {
						GetCtrl().GotoPos(e);
						// Advance to the next line
						GetCtrl().NewLine(); ++line;

						long new_line_end = GetCtrl().GetLineEndPosition(line);
						long new_line_start = GetCtrl().PositionFromLine(line);
						long new_line_pos = new_line_start;

						if (remove_leading_spaces_) {
							// Count the number of leading spaces
							while (new_line_pos < new_line_end &&
								GetCtrl().GetCharAt(new_line_pos) == _T(' ')) {
								++new_line_pos;
							}

							// Remove if non zero
							if (new_line_pos - new_line_start > 0) {
								GetCtrl().SetSel(new_line_start,new_line_pos);
								GetCtrl().ReplaceSel("");
							}
						}

						new_line_end = GetCtrl().GetLineEndPosition(line);
						GetCtrl().GotoPos(new_line_end);

						stop = true;
					}

					--e;
				}
			}

			GetCtrl().EndUndoAction();
		}		
	}
}

void CodeView::EnableHardWrap( bool e /*= true*/ )
{
	hard_wrap_enabled_ = e;
}

bool CodeView::IsHardWrapEnabled() const
{
	return hard_wrap_enabled_;
}

void CodeView::SetRemoveLeadingSpaces( bool e /*= true*/ )
{
	remove_leading_spaces_ = e;
}

bool CodeView::GetRemoveLeadingSpaces() const
{
	return remove_leading_spaces_;
}

void CodeView::HardWrapRange( long s, long e )
{
	const int col = CConfiguration::GetInstance()->m_nFixedColumnWrap;

	if (e - s > col) {
		GetCtrl().BeginUndoAction();

		long pos = s; bool move, new_line_started = false;
		int oldpos, newpos, lastcrlfpos = -1, count;

		while (pos <= e) {
			move = false;
			int ch = GetCtrl().GetCharAt(pos);

			// Remove CR+LF and leading spaces for newly started bookmarks
			if (ch == '\r' || ch == '\n' || ch == ' ' && new_line_started) {
				if (ch == '\r' || ch == '\n')
					lastcrlfpos = pos;

				count = 1;

				if (pos + 1 <= e && GetCtrl().GetCharAt(pos + 1) == '\n')
					++count;

				GetCtrl().SetSel(pos,pos + count);
				GetCtrl().ReplaceSel("");
				e -= count;
			}
			else if (ch != ' ' && new_line_started)
				new_line_started = false;

			if ((pos - GetCtrl().PositionFromLine(GetCtrl().LineFromPosition(pos))) >= col) {
				oldpos = GetCtrl().WordStartPosition(pos,TRUE);

				if (oldpos > 0)
					--oldpos;

				if (GetCtrl().GetCharAt(oldpos) == ' ') {
					new_line_started = true;
					GetCtrl().GotoPos(oldpos); GetCtrl().NewLine();

					newpos = GetCtrl().PositionFromLine(GetCtrl().LineFromPosition(oldpos) + 1);
					e += newpos - oldpos; pos = newpos;					

					move = false;
				}
				else
					move = true;
			}
			else
				move = true;
			
			if (move)
				++pos;
		}

		GetCtrl().EndUndoAction();
	}
}

int CodeView::GetPreviousBookmark()
{
	return GetCtrl().MarkerPrevious(GetCurrentLine() - 1,1 << CodeDocument::Bookmark);
}

int CodeView::GetNextBookmark()
{
	return GetCtrl().MarkerNext(GetCurrentLine() + 1,1 << CodeDocument::Bookmark);
}

void CodeView::OnEditGotoNextBookmark()
{
	int line = GetNextBookmark();

	if (line >= 0) GoToLine(line);
}

void CodeView::OnEditGotoPrevBookmark()
{
	int line = GetPreviousBookmark();

	if (line >= 0) GoToLine(line);
}

void CodeView::OnUpdateEditGotoPrevBookmark(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetPreviousBookmark() >= 0);
}

void CodeView::OnUpdateEditGotoNextBookmark(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetNextBookmark() >= 0);
}

void CodeView::OnUpdateEditClearAllBookmarks(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetCtrl().MarkerNext(0,1 << CodeDocument::Bookmark) >= 0);
}

void CodeView::DefineMarker(int marker, int markerType, COLORREF fore, COLORREF back)
{
	CScintillaCtrl& rCtrl = GetCtrl();

	rCtrl.MarkerDefine(marker, markerType);
	rCtrl.MarkerSetFore(marker, fore);
	rCtrl.MarkerSetBack(marker, back);
}

bool CodeView::Serialize(CIniFile &ini, LPCTSTR lpszKey, bool write)
{
	LPCTSTR const VAL_VIEWINFO_CURSOR = _T("Cursor");
	LPCTSTR const TopLine = _T("TopLine");

	if (write) {
		ini.SetValue(lpszKey, TopLine, GetCtrl().GetFirstVisibleLine());
		ini.SetValue(lpszKey,VAL_VIEWINFO_CURSOR,GetCtrl().GetCurrentPos());
	}
	else {
		//Go to the cursor position first
		GetCtrl().GotoPos(ini.GetValue(lpszKey,VAL_VIEWINFO_CURSOR,0));

		//Restore the top line of the view in the first OnPainted Notification.
		topline_from_serialization_ = ini.GetValue(lpszKey,TopLine,0);
	}

	return true;
}

void CodeView::OnPainted(SCNotification* /*pSCNotification*/)
{
	if (topline_from_serialization_ >= 0)
	{
		//It doesn't restore the first visible line perfectly when word wrap is enabled,
		// but that might just be a bug in Scintilla.
		GetCtrl().SetFirstVisibleLine(topline_from_serialization_);
		topline_from_serialization_ = -1;
	}
}

void CodeView::OnMarginClick(SCNotification* n)
{
	if (n->margin == 1) // Bookmark/Errormark margin
		GetDocument()->ToggleBookmark(GetCtrl().LineFromPosition(n->position));
	else
		CScintillaView::OnMarginClick(n); // Toggle folding
}

void CodeView::OnUpdateUI(SCNotification* n)
{
	UNUSED_ALWAYS(n);
}

void CodeView::OnEditFindIncrementalForward()
{
	shadow_.SetSearchForward();
	shadow_.EnableIncrementalSearch(true);
}

void CodeView::OnEditFindIncrementalBackward()
{
	shadow_.SetSearchForward(false);
	shadow_.EnableIncrementalSearch(true);
}

void CodeView::OnSearchFindNextSelected()
{
	CString sel = GetCurrentWordOrSelection(true, false, true);
	if (sel.GetLength() && sel.Find(_T('\r')) < 0 && sel.Find(_T('\n')) < 0)
	{
		//We would need to set the _scintillaEditState, but that one is local to ScintillaDocView.cpp
		// Now we cannot use F3 after Ctrl-F3, which is not so nice.
		//_scintillaEditState.strFind = sel;
		FindText(sel, true, true, true, false);
	}
}


void CodeView::OnSearchFindPreviousSelected()
{
	CString sel = GetCurrentWordOrSelection(true, false, true);
	if (sel.GetLength() && sel.Find(_T('\r')) < 0 && sel.Find(_T('\n')) < 0)
	{
		//_scintillaEditState.strFind = sel;
		FindText(sel, false, true, true, false);
	}
}


CString CodeView::GetCurrentWordOrSelection(const bool bDefaultWordChars, const bool bIncludingEscapeChar, const bool bStripEol)
{
	const long startPos = GetCtrl().GetSelectionStart();
	const long endPos = GetCtrl().GetSelectionEnd();
	return GetWordAroundRange(startPos, endPos, (startPos == endPos), (startPos == endPos), bDefaultWordChars, bIncludingEscapeChar, bStripEol, true, false);
}


CString CodeView::GetWordAroundRange(const long startPos, const long endPos, const bool bToLeft, const bool bToRight,
										const bool bDefaultWordChars, const bool bIncludingEscapeChar, const bool bStripEol,
										const bool bConvertEncoding, const bool bSelect)
{
	CScintillaCtrl& Ctrl = GetCtrl();

	//Use a different definition of what constitutes a word?
	CString DefaultWordChars;
	if (!bDefaultWordChars)
	{
		//Save default definition
		//const int nRequiredLength = Ctrl.GetWordChars(NULL, NULL);
		//...
		DefaultWordChars = _T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"); //That is the default of Scintilla, but it would be better to retrieve it (once that func is available through our interface).
		CString WordChars(DefaultWordChars);

		//Extend definition
		AddExtendedWordChars(WordChars);

		//Set it temporarily
		Ctrl.SetWordChars(WordChars);
	}

	//Extend range, if wanted
	long RangeStart = bToLeft ? Ctrl.WordStartPosition(startPos, true) : startPos;
	long RangeEnd = bToRight ? Ctrl.WordEndPosition(endPos, true) : endPos;

	//Include escape character, if wanted
	if (bIncludingEscapeChar && RangeStart > 0)
	{
		CString EscapeChars;
		AddEscapeChars(EscapeChars);
		if (EscapeChars.Find(Ctrl.GetCharAt(RangeStart - 1)) >= 0) RangeStart--;
	}

	//Restore default word definition
	if (!bDefaultWordChars)
	{
		Ctrl.SetWordChars(DefaultWordChars);
	}

	//Select the range
	if (bSelect)
	{
		Ctrl.SetSel(RangeStart, RangeEnd);
	}

	return GetText(RangeStart, RangeEnd, bStripEol, bConvertEncoding);
}


CString CodeView::GetText(const long startPos, const long endPos, bool bStripEol, const bool bConvertEncoding)
{
	if (startPos == endPos) return CString();

	//Prepare Range data structure for scinitlla
	Sci_TextRange TRange;
	TRange.chrg.cpMin = startPos;
	TRange.chrg.cpMax = endPos;
	std::vector<char> CharsFromScintilla(abs(TRange.chrg.cpMax - TRange.chrg.cpMin) + 1);
	TRange.lpstrText = &CharsFromScintilla[0];

	//Get the text from scintilla as an array of chars
	GetCtrl().GetTextRange(&TRange);

	//Set the string with possible conversion to a different encoding, see below
	CString SelectedString;
	//IMPORTANT: Convert to proper encoding, if you use this text in the user interface
	if (bConvertEncoding)
	{
		#ifdef UNICODE
			std::vector<wchar_t> TextWithProperEncoding;
			UTF8toUTF16(&CharsFromScintilla[0], CharsFromScintilla.size(), TextWithProperEncoding);
		#else
			std::vector<char> TextWithProperEncoding;
			UTF8toANSI(&CharsFromScintilla[0], CharsFromScintilla.size(), TextWithProperEncoding);
		#endif

		//Set it
		if (!TextWithProperEncoding.empty())
		{
			SelectedString.SetString(&TextWithProperEncoding[0], static_cast<int>(TextWithProperEncoding.size()));
		}
	}
	else
	{
		//No change in the encoding
		SelectedString = &CharsFromScintilla[0];
	}


	//Strip some unwanted characters?
	if (bStripEol)
	{
		//Remove terminating \r, \n, or \r\n.
		int sellen = SelectedString.GetLength();
		if (sellen >= 2 && (SelectedString[sellen - 2] == _T('\r') && SelectedString[sellen - 1] == _T('\n')))
		{
			SelectedString.Left(sellen - 2);
		}
		else if (sellen >= 1 && (SelectedString[sellen - 1] == _T('\r') || SelectedString[sellen - 1] == _T('\n')))
		{
			SelectedString.Left(sellen - 1);
		}
	}

	return SelectedString;
}


void CodeView::GetReplaceAllTarget(long& s, long& e)
{
	CScintillaCtrl& c = GetCtrl();
	bool do_default = true;

	if (FindReplaceDlg* d = dynamic_cast<FindReplaceDlg*>(CScintillaFindReplaceDlg::GetFindReplaceDlg())) {
		d->UpdateData();

		if (d->ReplaceSelection()) {
			s = c.GetSelectionStart();
			e = c.GetSelectionEnd();
			do_default = false;
		}
	}

	if (do_default)
		CScintillaView::GetReplaceAllTarget(s, e);
}

CScintillaFindReplaceDlg* CodeView::CreateFindReplaceDialog(void)
{
	return new FindReplaceDlg;
}

bool CodeView::IsAutoIndentEnabled() const
{
	return shadow_.autoindent_enabled;
}

void CodeView::EnableAutoIndent( bool enable /*= true*/ )
{
	shadow_.autoindent_enabled = enable;
}

void CodeView::OnModified(SCNotification* n)
{
	CScintillaView::OnModified(n);

	// Update the last changed position only if text has been inserted or deleted
	if (n->modificationType & (SC_MOD_INSERTTEXT|SC_MOD_DELETETEXT))
		last_change_pos_ = GetCtrl().GetCurrentPos();

	if (n->linesAdded) {
		// Number of lines changed, update margin's width
		UpdateLineNumberMargin();
	}
}

void CodeView::OnEditGotoLastChange()
{
	GetCtrl().GotoPos(last_change_pos_);
}

void CodeView::OnUpdateEditGotoLastChange(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(last_change_pos_ != -1);
}

void CodeView::EnableViewFolding( bool enable /*= true*/ )
{
	GetCtrl().SetProperty("fold", enable ? "1" : "0");
}

bool CodeView::IsFoldingEnabled()
{
	return GetCtrl().GetPropertyInt("fold") == 1;
}

void CodeView::HighlightActiveLine(bool show)
{
	CConfiguration::GetInstance()->m_bHighlightCaretLine = show;

	//Chained update
	using namespace std::placeholders;
	ForEveryView(bind(bind(&CScintillaCtrl::SetCaretLineVisible,_1,show,TRUE),
		bind(&CodeView::GetCtrl,_1)));
}

void CodeView::OnViewHighlightActiveLine()
{
	const bool bEnable = !GetCtrl().GetCaretLineVisible();
	HighlightActiveLine(bEnable);
}

void CodeView::OnUpdateViewHighlightActiveLine(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(CConfiguration::GetInstance()->m_bHighlightCaretLine);
}

void CodeView::ShowIndentationGuides( bool show )
{
	GetCtrl().SetIndentationGuides(show ? SC_IV_LOOKBOTH : SC_IV_NONE);
}

void CodeView::OnViewIndentationGuides()
{
	bool enable = GetCtrl().GetIndentationGuides() == SC_IV_NONE;
	ShowIndentationGuides(enable);
	CConfiguration::GetInstance()->SetShowIndentationGuides(enable);
}

void CodeView::OnUpdateViewIndentationGuides(CCmdUI *pCmdUI)
{
	bool check = CConfiguration::GetInstance()->GetShowIndentationGuides();
	pCmdUI->SetCheck(check);
}

void CodeView::OnZoom( SCNotification* /*n*/ )
{
	if (CConfiguration::GetInstance()->m_bShowLineNumbers)
		UpdateLineNumberMargin();

	UpdateFoldMargin();
}

void CodeView::SetModified( bool modified /*= true*/ )
{
	if (!modified)
		last_change_pos_ = -1;
}

bool CodeView::ShadowWindow::IsNewLine(TCHAR ch)
{
	return ch == VK_RETURN || ch == _T('\n');
}

BOOL CodeView::PreTranslateMessage(MSG* pMsg)
{
	BOOL result = FALSE;

	if (pMsg->message == WM_KEYDOWN && shadow_.IsIncrementalSearchEnabled()) {
		UINT ch = pMsg->wParam;

		if (TerminatesIncrementalSearch(ch)) {
			shadow_.EnableIncrementalSearch(false);

			if (!IsDirectionKey(static_cast<UINT>(pMsg->wParam)))
				result = TRUE; // Not a direction key; surpress it.
		}
	}

	if (!result)
		result = CScintillaView::PreTranslateMessage(pMsg);

	return result;
}

bool CodeView::TerminatesIncrementalSearch( UINT ch )
{
	return ch != VK_BACK && ch != VK_SHIFT && ch != VK_CONTROL && ch != VK_MENU &&
		(ch == VK_RETURN || IsDirectionKey(ch) ||
			!CharTraitsT::IsPrint(ch));
}


bool IsOpeningBrace(TCHAR ch)
{
	return ch == _T('{') || ch == _T('(') || ch == _T('[');
}

bool IsClosingBrace(TCHAR ch)
{
	return ch == _T('}') || ch == _T(')') || ch == _T(']');
}

void CodeView::OnEditSelBiggerBlock()
{
	//Shorthand
	CScintillaCtrl& ctrl = GetCtrl();

	const long BeginSelectionStart = ctrl.GetSelectionStart();
	const long BeginSelectionEnd = ctrl.GetSelectionEnd();
	const bool bHadSelectionAtBeginning = (BeginSelectionStart != BeginSelectionEnd);

	//This is arbitrary, but we cannot use the current cursor position
	// as it might be inside the selection after a call to this function.
	// And we want to be able to grow the selection using this function.
	long CurrentPosition = BeginSelectionStart;

	//TODO: What about $ signs as opening/closing braces?
	//A little bit difficult to handle, I assume,
	//since there is no easy, character-based distinction between opening and closing.
	//Also, Scintilla does not natively match them using BraceMatch().

	//Search for the next opening brace.
	long StackCounter(0); //Obviously, the current position is at zero-th level.
	//We start with the character before the cursor, searching to the left.
	CurrentPosition--;
	while (CurrentPosition >= 0 && StackCounter != 1)
	{
		//Take care of the the style, such that we do not match braces in comments.
		if (ctrl.GetStyleAt(CurrentPosition) != SCE_TEX_COMMENT)
		{
			TCHAR ch = ctrl.GetCharAt(CurrentPosition);

			if (IsOpeningBrace(ch))
			{
				StackCounter++;
			}
			else if (IsClosingBrace(ch))
			{
				StackCounter--;
			}
		}

		//Next char!
		CurrentPosition--;
	}

	//Are we leveled out, i.e., did we find a proper opening brace or did we just hit the end?
	if (StackCounter == 1)
	{
		//The starting point for the new selection
		// - we had one "pos--" too much above
		// - and we actually do not want to select the brace itself
		// - therefore the + 2
		long NewSelectionStart = CurrentPosition + 2;

		//Let Scintilla find the closing brace, starting from the found opening brace.
		long NewSelectionEnd = ctrl.BraceMatch(NewSelectionStart - 1);

		//Did we find something?
		if (NewSelectionEnd >= 0)
		{
			//Did anything change? If not, then we extend the selection by one char in each direction,
			//since we have actually already selected a brace block.
			if (BeginSelectionStart == NewSelectionStart && BeginSelectionEnd == NewSelectionEnd)
			{
				NewSelectionStart--; //Scintilla will be graceful
				NewSelectionEnd++;   //if this gets us out of bounds
			}

			//We explicitly do not change the caret position.
			// I want people to be able to keep the cursor where it is.
			ctrl.SetSelectionStart(NewSelectionStart);
			ctrl.SetSelectionEnd(NewSelectionEnd);
		}
	}
	else if (bHadSelectionAtBeginning)
	{
		//We had a selection at the beginning, but could not extend it in any way.
		//Lets remove the selection.
		//By going up/down, we might actually keep the original cursor position,
		// if the selection is still in one line. If we wanted to have this behavior
		// in all situations, we needed to save the cursor position from the first call
		// to this function, i.e., Ctrl-M.
		ctrl.LineUp();
		ctrl.LineDown();
	}
}


void CodeView::OnEditSelParagraph()
{
	CodeDocument* pDoc = GetDocument();
	if (pDoc)
	{
		CScintillaCtrl& c = GetCtrl();
		//No previous selection: get the current paragraph respecting comments
		//Previous selection: get the paragraph ignoring comments
		//This allows to grow paragraph selections by pressing Ctrl-P twice.
		const std::pair<long,long> range = pDoc->GetParagraphRangePos(c.GetCurrentPos(), c.GetSelectionStart() == c.GetSelectionEnd());

		//Select full lines
		long FullLineStartPos = c.PositionFromLine(c.LineFromPosition(range.first));
		long FullLineEndPos = c.GetLineEndPosition(c.LineFromPosition(range.second));
		//If the given pos is the first char in a line, we go back to the last char of the previous line.
		if (range.first != range.second && range.second == c.PositionFromLine(c.LineFromPosition(range.second)))
		{
			FullLineEndPos = c.GetLineEndPosition(c.LineFromPosition(range.second) - 1);
		}

		c.SetSelectionStart(FullLineStartPos);
		c.SetSelectionEnd(FullLineEndPos);
	}
}


bool CodeView::IsDirectionKey( UINT ch )
{
	return ch == VK_LEFT || ch == VK_RIGHT || ch == VK_UP || ch == VK_DOWN;
}

void CodeView::OnSetFocus(CWnd* pOldWnd)
{
	CScintillaView::OnSetFocus(pOldWnd);
	CheckForFileChangesAsync();
}

void CodeView::CheckForFileChangesAsync()
{
	PostMessage(AfxUserMessages::CheckForFileChangesMessageID);
}

void CodeView::OnCheckForFileChanges()
{
	CheckForFileChanges();
}

void CodeView::CheckForFileChanges()
{
	CodeDocument* doc = dynamic_cast<CodeDocument*>(GetDocument());

	if (doc)
		doc->CheckForFileChanges();
}

void CodeView::ToggleFolding()
{
	EnableFolding(!IsFoldingEnabled());
}

void CodeView::EnableFolding(bool value)
{
	UpdateFoldSettings();

	EnableFoldMargin(value);
	EnableViewFolding(value);

	CConfiguration::GetInstance()->EnableFolding(value);
}

void CodeView::EnableFoldMargin(bool value)
{
	// Folding margin width: 2px + 1.5ex + 2px
	const int width = value ? GetCtrl().
		TextWidth(STYLE_DEFAULT, "9") + 2 * 2 : 0;

	GetCtrl().SetMarginWidthN(GetFoldingMargin(), width);
}

void CodeView::UpdateFoldSettings()
{
	int flags = 0;

	if (CConfiguration::GetInstance()->GetShowLineBelowFold())
		flags |= 16;

	if (CConfiguration::GetInstance()->GetShowLineBelowNoFold())
		flags |= 8;

	if (CConfiguration::GetInstance()->GetShowLineAboveFold())
		flags |= 4;

	if (CConfiguration::GetInstance()->GetShowLineAboveNoFold())
		flags |= 2;

	GetCtrl().SetFoldFlags(flags);
	GetCtrl().SetProperty("fold.compact",
		CConfiguration::GetInstance()->GetFoldCompact() ? "1" : "0");
}

void CodeView::UpdateFoldMargin()
{
	EnableFoldMargin(CConfiguration::GetInstance()->IsFoldingEnabled());
}

void CodeView::OnSavePointLeft(SCNotification* /*n*/)
{
	GetDocument()->MarkTitleAsModified(true);
}

void CodeView::OnSavePointReached(SCNotification* /*n*/)
{
	GetDocument()->MarkTitleAsModified(false);
}

LRESULT CodeView::OnGetLineText(WPARAM wParam, LPARAM lParam)
{
	*reinterpret_cast<CString*>(lParam) = GetLineText(static_cast<int>(wParam));
	return 0;
}

LRESULT CodeView::OnCommandHelp(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return InvokeContextHelp(GetCurrentWordOrSelection(true, true, true));	
}


BOOL CodeView::InvokeContextHelp(const CString& keyword)
{
	if (keyword.IsEmpty())
	{
		HtmlHelp(0L, HH_DISPLAY_TOC);
	}
	else
	{
		HtmlHelp(0L, HH_DISPLAY_TOC);

		HH_AKLINK link;
		link.cbStruct = sizeof(HH_AKLINK);
		link.fReserved = FALSE;
		link.pszKeywords = (LPCTSTR)keyword;
		link.pszUrl = NULL;
		link.pszMsgText = NULL;
		link.pszWindow = NULL;
		link.fIndexOnFail = TRUE;
		HtmlHelp((DWORD_PTR) &link, HH_KEYWORD_LOOKUP);
	}

	return TRUE;
}


#pragma pop_macro("max")
