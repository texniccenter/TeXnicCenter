#pragma once

#include <vector>
#include <memory>

#include "ThemedCtrl.h"
#include "DocumentTokenizer.h"
#include "CodeBookmark.h"

class SpellerBackgroundThread;
class CodeDocument;

int GetLogFontPointSize(const LOGFONT& lf);

/// Base class for Code editors with support
/// for Unicode loading and saving, spell checking,
/// incremental search, bookmarking and so on
class CodeView : 
	public CScintillaView
{
	ThemedCtrl theme_;
	volatile LONG hold_count_;	
	mutable std::auto_ptr<DocumentTokenizer> tokenizer_;
	
	// Hard wrap
	bool hard_wrap_enabled_;
	bool remove_leading_spaces_;
	bool suppress_speller_;

	long last_change_pos_;

	///The top line from the ini file. It can only be properly restored
	///  in the first OnPainted Notification from Scinitlla.
	int topline_from_serialization_;

	DECLARE_DYNAMIC(CodeView)

protected:
	CodeView();           // protected constructor used by dynamic creation
	~CodeView();

	virtual DocumentTokenizer* NewDocumentTokenizer() const = 0;

public:
	CodeDocument* GetDocument() const;

	void EnableHardWrap(bool e = true);
	bool IsHardWrapEnabled() const;
	void SetRemoveLeadingSpaces(bool e = true);
	bool GetRemoveLeadingSpaces() const;

	static int GetFoldingMargin() { return 3; }

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif	
#endif

	/**
	 * @brief Returns the length of the specified @a line excluding any end of
	 *		  line characters.
	 *
	 * @param line The line number whose length should be returned.
	 * 
	 * @return The line length excluding any end of line characters.
	 */
	int GetLineLength(int line, bool direct = true);
	int GetLineCount(bool direct = true);
	int GetCurrentLine(bool direct = true);

	/**
	 * @brief Returns the text on the specified @a line including any end of
	 *		  line characters.
	 *
	 * @param line The line number whose text should be returned.
	 * 
	 * @return The line text including any end of line characters.
	 */
	CString GetLineText(int line, bool direct = true);
	void InsertText(const CString& text);
	void GoToLine(int line, bool direct = true);

	DocumentTokenizer* GetTokenizer() const;
	int Lock(bool exclusive = false);
	int Unlock(bool exclusive = false);

	bool IsLineMarginVisible();

protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnViewWhiteSpace();
	afx_msg void OnViewLineEnding();
	afx_msg void OnUpdateViewWhiteSpace(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewShowLineEnding(CCmdUI *pCmdUI);
	afx_msg void OnEditDeleteLine();
	afx_msg void OnEditDuplicateLine();
	afx_msg void OnSysColorChange();
	virtual void OnInitialUpdate();
	afx_msg void OnDestroy();
	afx_msg void OnEditGoto();
	afx_msg void OnViewWordWrap();
	afx_msg void OnUpdateViewWordWrap(CCmdUI *pCmdUI);
	afx_msg void OnViewWordWrapIndent();
	afx_msg void OnUpdateViewWordWrapIndent(CCmdUI *pCmdUI);
	afx_msg void OnViewWordWrapIndicatorsStart();
	afx_msg void OnViewWordWrapIndicatorsEnd();
	afx_msg void OnUpdateViewWordWrapIndicators(CCmdUI *pCmdUI);
	afx_msg void OnViewFoldMargin();

	afx_msg void OnUpdateViewFoldMargin(CCmdUI *pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	void OnUpdateSelectionIndicator(CCmdUI* pCmdUI);
	void OnUpdatePositionIndicator(CCmdUI* pCmdUI);
	void OnUpdateEncodingIndicator(CCmdUI* pCmdUI);
	void OnUpdateEOLModeIndicator(CCmdUI* pCmdUI);
	void TextNotFound(LPCTSTR /*lpszFind*/, BOOL /*bNext*/, BOOL /*bCase*/, BOOL /*bWord*/, BOOL /*bRegularExpression*/, BOOL /*bReplaced*/);
	afx_msg void OnViewLineNumbers();
	afx_msg void OnUpdateViewLineNumbers(CCmdUI *pCmdUI);
	void UpdateLineNumberMargin();
	void OnCharAdded(SCNotification* n);
	void OnPainted(SCNotification* pSCNotification);

	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	BOOL InvokeContextHelp(const CString& keyword);

	virtual void OnSettingsChanged();

protected:
	DECLARE_MESSAGE_MAP()	

	void HardWrapRange(long s, long e);

	/**
	 * @brief Executes the specified function object @a f for every @ref
	 *        CodeView.
	 *
	 * @param f The function object to be executed for each view. A pointer to
	 *        the view is a passed as an argument.
	 */
	template<class F>
	void ForEveryView(F f);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEditGotoNextBookmark();
	afx_msg void OnEditGotoPrevBookmark();
	afx_msg void OnUpdateEditGotoPrevBookmark(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditGotoNextBookmark(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditClearAllBookmarks(CCmdUI *pCmdUI);
	int GetPreviousBookmark();
	int GetNextBookmark();
	void DefineMarker(int marker, int markerType, COLORREF fore, COLORREF back);

public:
	virtual bool Serialize(CIniFile &ini, LPCTSTR lpszKey, bool write);
protected:
	void OnMarginClick(SCNotification* n);
	void OnUpdateUI(SCNotification* n);

private:
	void EnableFoldMargin(bool value = true);
	void EnableViewFolding(bool enable = true);
	void UpdateFoldMargin();
	void UpdateFoldSettings();

	class ShadowWindow :
		public CWindowImpl<ShadowWindow>
	{
		CodeView* view;
		long original_pos;
		long match_start_pos, match_end_pos;
		bool found;
		bool incremental_search_enabled;
		CString search_text;
		bool search_forward;
		int rounds;
		bool update_status_bar;
		bool first_match;
		HCURSOR search_forward_cursor, search_backward_cursor;

	public:
		bool autoindent_enabled;

		void SetSearchForward(bool forward = true);

		ShadowWindow(CodeView* v);
		~ShadowWindow();

		BEGIN_MSG_MAP(CodeView::ShadowWindow)
			MESSAGE_HANDLER(WM_CHAR, OnChar)
			MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
			MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
			MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouseButtonDown)
			MESSAGE_HANDLER(WM_RBUTTONDOWN, OnMouseButtonDown)
			MESSAGE_HANDLER(WM_MBUTTONDOWN, OnMouseButtonDown)
			MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		END_MSG_MAP()

		LRESULT OnChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);		

		static bool IsNewLine(TCHAR ch);
		LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnMouseButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);		
		LRESULT OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		void HandleChar(TCHAR ch, bool one_level_recursion = false);
		void EnableIncrementalSearch(bool enable = true);
		void UpdateStatusBarMessage();
		void UpdateCursor();

		bool IsIncrementalSearchEnabled() const;
	};

	ShadowWindow shadow_;

protected:
	afx_msg void OnEditFindIncrementalForward();
	afx_msg void OnEditFindIncrementalBackward();
	afx_msg void OnSearchFindNextSelected();
	afx_msg void OnSearchFindPreviousSelected();
	void GetReplaceAllTarget(long& s, long& e);
	afx_msg void OnEditGotoLastChange();
	afx_msg void OnUpdateEditGotoLastChange(CCmdUI *pCmdUI);
	void OnModified(SCNotification* n);
	void OnSavePointLeft(SCNotification* n);
	void OnSavePointReached(SCNotification* n);
	afx_msg void OnViewHighlightActiveLine();
	afx_msg void OnUpdateViewHighlightActiveLine(CCmdUI *pCmdUI);
	afx_msg void OnViewIndentationGuides();
	afx_msg void OnUpdateViewIndentationGuides(CCmdUI *pCmdUI);
	afx_msg void OnEditSelBiggerBlock();
	afx_msg void OnEditSelParagraph();
	void OnZoom(SCNotification* n);

	virtual int GetLexer() const = 0;

public:
	CScintillaFindReplaceDlg* CreateFindReplaceDialog(void);

	bool IsAutoIndentEnabled() const;
	void EnableAutoIndent(bool enable = true);
	bool IsFoldingEnabled();
	void ShowIndentationGuides(bool show);
	void ToggleFolding();
	void EnableFolding(bool value = true);
	void HighlightActiveLine(bool show);

	void SetModified(bool modified = true);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	/** Returns the word under the cursor, or the selection.

		If nothing is selected, the word under the cursor is returned.
		The definition of a whole word depends on the characters that make up a word.
		LaTeX and BibTeX differ here.
		Furthermore, it may be wanted to include a preceeding backslash or @-sign (escape char).

		@see AddExtendedWordChars()
		@see AddEscapeChars()
	*/
	CString GetCurrentWordOrSelection(const bool bDefaultWordChars, const bool bIncludingEscapeChar, const bool bStripEol);

	///Retrieves the text between the start and end position, possibly with stripped-off EOL characters.
	///The text can be either encoded into the encoding of the program (UTF16 or ANSI),
	/// or it can be kept as it has been given by Scintilla.
	CString GetText(const long startPos, const long endPos, const bool bStripEol, const bool bConvertEncoding);

protected:
	///A derived class shall announce its extra word characters using this function.
	virtual void AddExtendedWordChars(CString& WordChars){UNREFERENCED_PARAMETER(WordChars);}

	///A derived class shall announce its escape character(s) using this function.
	/// This would be a backslash for LaTeX and a @-sign for BibTeX.
	virtual void AddEscapeChars(CString& EscapeChars){UNREFERENCED_PARAMETER(EscapeChars);}

	CString GetWordAroundRange(const long startPos, const long endPos, const bool bToLeft, const bool bToRight,
								const bool bDefaultWordChars, const bool bIncludingEscapeChar, const bool bStripEol,
								const bool bConvertEncoding, const bool bSelect);

private:
	afx_msg LRESULT OnGetLineText(WPARAM wParam, LPARAM lParam);

	static bool TerminatesIncrementalSearch( UINT ch );
	static bool IsDirectionKey( UINT ch );
	void OnCheckForFileChanges();

public:
	void CheckForFileChangesAsync();
	void CheckForFileChanges();
};

template<class F>
inline void CodeView::ForEveryView(F f)
{
	CDocTemplate* t = GetDocument()->GetDocTemplate();
	POSITION pos = t->GetFirstDocPosition();

	while (pos) {
		CDocument* doc = reinterpret_cast<CDocument*>(t->GetNextDoc(pos));

		if (CodeDocument* codedoc = dynamic_cast<CodeDocument*>(doc)) {
			ASSERT(codedoc->GetView()->IsKindOf(RUNTIME_CLASS(CodeView)));
			f(static_cast<CodeView*>(codedoc->GetView()));
		}		
	}
}

