#include "stdafx.h"

#include <algorithm>

#include "resource.h"
#include "CodeDocument.h"
#include "global.h"
#include "SpellerBackgroundThread.h"
#include "CodeBookmark.h"
#include "EncodingConverter.h"
#include "configuration.h"
#include "EndOfLineMode.h"
#include "textfilesavedialog.h"
#include "TeXnicCenter.h"

int ShowSaveTaskDialog(LPCTSTR prompt)
{
	TASKDIALOGCONFIG tdc = {sizeof(TASKDIALOGCONFIG)};
	tdc.dwCommonButtons = TDCBF_CANCEL_BUTTON;

	USES_CONVERSION;
	tdc.pszMainInstruction = T2CW(prompt);

	const CStringW save(MAKEINTRESOURCE(IDS_DO_SAVE));
	const CStringW dont_save(MAKEINTRESOURCE(IDS_DO_NOT_SAVE));

	const TASKDIALOG_BUTTON btns[] = {
		IDYES,save,
		IDNO,dont_save
	};

	tdc.cButtons = sizeof(btns) / sizeof(*btns);
	tdc.pButtons = btns;

	// disable windows for modal dialog
	theApp.DoEnableModeless(FALSE);

	HWND hWndTop;
	HWND hWnd = CWnd::GetSafeOwner_(NULL, &hWndTop);

	// re-enable the parent window, so that focus is restored
	// correctly when the dialog is dismissed.
	if (hWnd != hWndTop)
		EnableWindow(hWnd, TRUE);

	tdc.hwndParent = hWnd;

	const CStringW title(MAKEINTRESOURCE(IDR_MAINFRAME));
	tdc.pszWindowTitle = title;

	int button = 0;
	::TaskDialogIndirect(&tdc,&button,0,0); // AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE)

	// re-enable windows
	if (hWndTop != NULL)
		::EnableWindow(hWndTop, TRUE);

	theApp.DoEnableModeless(TRUE);

	return button;
}

int DetectScintillaEOLMode(const char* text, std::size_t size)
{
	const char* le = GetLineEnding(text,size);
	int mode;

	if (std::strcmp(LineEndingTraits<char>::CarriageReturn(),le) == 0)
		mode = SC_EOL_CR;
	else if (std::strcmp(LineEndingTraits<char>::LineFeed(),le) == 0)
		mode = SC_EOL_LF;
	else
		mode = SC_EOL_CRLF;

	return mode;
}

namespace {
	namespace BOM {
		const BYTE utf8[] = {0xef,0xbb,0xbf};
		const BYTE utf16le[] = {0xff,0xfe};
		const BYTE utf16be[] = {0xfe,0xff};
		const BYTE utf32le[] = {0xff,0xfe,0x00,0x00};
		const BYTE utf32be[] = {0x00,0x00,0xfe,0xff};
	}

	void SwapCodePoint(WCHAR& ch)
	{
		ch = (ch & 0xff) << 8 | ch >> 8 & 0xff;
	}

	template<class T, std::size_t S>
	const T* AssignByteOrderMark(const T (&p)[S], SIZE_T& s)
	{
		s = S;
		return p;
	}
}

const TCHAR ModifiedMarker = _T('*');

#pragma region Line ending mode conversion functions

int ToScintillaMode(int m)
{
	int mode = SC_EOL_CRLF;

	switch (m) {
		case UnixStyleEOLMode: mode = SC_EOL_LF; break;
		case MacStyleEOLMode: mode = SC_EOL_CR; break;
	}

	return mode;
}

int FromScintillaMode(int m)
{
	int mode = DOSStyleEOLMode;

	switch (m) {
		case SC_EOL_CR: mode = MacStyleEOLMode; break;
		case SC_EOL_LF: mode = UnixStyleEOLMode; break;
	}

	return mode;
}

#pragma endregion

TextDocument::TextDocument( bool use_bom, CodeDocument::Encoding e )
: use_bom_(use_bom), encoding_(e)
{
}

TextDocument::TextDocument(CodeDocument::Encoding e, UINT codepage)
: use_bom_(false)
, encoding_(e)
{
	SetCodePage(codepage);
}

TextDocument::TextDocument()
: encoding_(CodeDocument::ANSI)
, code_page_(::GetACP())
, use_bom_(false)
{
}

TextDocument::TextDocument(CodeDocument* document)
{
	if (document)
	{
		SetEncoding(document->GetEncoding());
		SetUseBOM(document->GetUseBOM());
		SetCodePage(document->GetCodePage());
	}
	else
	{
		SetEncoding(CodeDocument::ANSI);
		SetUseBOM(false);
		SetCodePage(::GetACP());
	}
}

CodeDocument::Encoding TextDocument::GetEncoding() const
{
	return encoding_;
}

CodeDocument::Encoding TextDocument::TestForUnicode(const BYTE* data, SIZE_T size)
{
	CodeDocument::Encoding encoding = CodeDocument::ANSI;

	INT flags = IS_TEXT_UNICODE_UNICODE_MASK;

	if (::IsTextUnicode(data,size,&flags)) {
		if (flags & (IS_TEXT_UNICODE_REVERSE_ASCII16|IS_TEXT_UNICODE_REVERSE_STATISTICS))
			encoding = CodeDocument::UTF16BE;
		else
			encoding = CodeDocument::UTF16LE;
	}
	else {
		// From Wikipedia:
		// As a consequence of the design of UTF-8, the following properties of multi-byte sequences hold:

		//  * The most significant bit of a single-byte character is always 0.
		//	* The most significant bits of the first byte of a multi-byte sequence determine the length of
		//	  the sequence. These most significant bits are 110 for two-byte sequences; 1110 for three-byte
		//	  sequences, and so on.
		//	* The remaining bytes in a multi-byte sequence have 10 as their two most significant bits.
		//	* A UTF-8 stream contains neither the byte FE nor FF. This makes sure that a UTF-8 stream never
		//	  looks like a UTF-16 stream starting with U+FEFF (Byte-order mark)

		bool utf8 = true;
		bool ascii = true;
		std::size_t consecutive_bytes = 0;
		std::size_t i = 0;

		for ( ; i < size && utf8; ++i) {
			// Might be pure ASCII
			ascii &= data[i] < 128;

			if (data[i] == 0xfe || data[i] == 0xff)
				utf8 = false;

			if (consecutive_bytes == 0)
				GetUTF8CharBytes(data[i],consecutive_bytes);
			else if (consecutive_bytes == 0 && data[i] >> 7 & 1 ||
				consecutive_bytes > 0 && data[i] >> 6 != 2)
				utf8 = false; // Either MSB is not 0 or MSB of a byte in a
			// multi-part sequence is not 10

			if (consecutive_bytes > 0)
				--consecutive_bytes;
		}

		if (utf8 || ascii && i == size)
			encoding = CodeDocument::UTF8;
	}

	return encoding;
}

CodeDocument::Encoding TextDocument::DetectEncoding(const BYTE* data, SIZE_T& pos, SIZE_T size)
{
	// By default we assume ANSI text
	CodeDocument::Encoding encoding = CodeDocument::ANSI;
	pos = 0;

	if (size >= 2) {// minimal BOM size
		// Try to detect Unicode using the byte order mark
		if (size >= sizeof(BOM::utf8) && std::memcmp(data,BOM::utf8,sizeof(BOM::utf8)) == 0) {
			encoding = CodeDocument::UTF8;
			pos += sizeof(BOM::utf8);
		}
		// Try UTF-32LE instead of UTF-16 since BOM's first 2 bytes are for both encodings the same
		else if (size >= sizeof(BOM::utf32le) && std::memcmp(data,BOM::utf32le,sizeof(BOM::utf32le)) == 0) {
			encoding = CodeDocument::UTF32LE;
			pos += sizeof(BOM::utf32le);
		}
		else if (size >= sizeof(BOM::utf32be) && std::memcmp(data,BOM::utf32be,sizeof(BOM::utf32be)) == 0) {
			encoding = CodeDocument::UTF32BE;
			pos += sizeof(BOM::utf32be);
		}
		else if (std::memcmp(data,BOM::utf16le,sizeof(BOM::utf16le)) == 0) {
			encoding = CodeDocument::UTF16LE;
			pos += sizeof(BOM::utf16le);
		}
		else if (std::memcmp(data,BOM::utf16be,sizeof(BOM::utf16be)) == 0) {
			encoding = CodeDocument::UTF16BE;
			pos += sizeof(BOM::utf16be);
		}		
	}

	return encoding;
}

DWORD TextDocument::Write( ATL::CAtlFile& f, const char* text, std::size_t n)
{
	DWORD result = ERROR_SUCCESS;

	if (encoding_ != CodeDocument::ANSI && use_bom_) {
		const BYTE* bom = 0;
		SIZE_T size = 0;

		switch(encoding_) {
			case CodeDocument::UTF8: bom = AssignByteOrderMark(BOM::utf8,size); break;
			case CodeDocument::UTF16LE: bom = AssignByteOrderMark(BOM::utf16le,size); break;
			case CodeDocument::UTF16BE: bom = AssignByteOrderMark(BOM::utf16be,size); break;
			case CodeDocument::UTF32LE: bom = AssignByteOrderMark(BOM::utf32le,size); break;
			case CodeDocument::UTF32BE: bom = AssignByteOrderMark(BOM::utf32be,size); break;
		}

		if (FAILED(f.Write(bom,size)))
			result = ::GetLastError();
	}

	if (result == ERROR_SUCCESS && n > 0) {
		if (FAILED(f.Write(text, static_cast<DWORD>(n))))
			result = ::GetLastError();
	}

	return result;
}

bool TextDocument::Read(LPCTSTR pszFileName, CStringW& string)
{
	DWORD result = ERROR_SUCCESS;
	ATL::CAtlFile file;

	if (SUCCEEDED(file.Create(pszFileName,GENERIC_READ,FILE_SHARE_READ,OPEN_EXISTING))) {
		ATL::CAtlFileMapping<BYTE> mapping;

		if (SUCCEEDED(mapping.MapFile(file))) {
			const BYTE* data = mapping;
			SIZE_T size = mapping.GetMappingSize();
			SIZE_T pos = 0;

			CodeDocument::Encoding encoding = DetectEncoding(data,pos,size);

			if (encoding == CodeDocument::ANSI) { // ANSI? Still might by Unicode without BOM
				encoding = TestForUnicode(data,size);
			}

			UINT code_page = code_page_; // Default user code page for ANSI text

			std::vector<wchar_t> buffer;
			const wchar_t* text = 0;
			std::size_t n = 0;

			switch (encoding) {
				case CodeDocument::ANSI:
					ANSItoUTF16(reinterpret_cast<const char*>(data + pos),size - pos,buffer,code_page);
					break;
				case CodeDocument::UTF8:
					UTF8toUTF16(reinterpret_cast<const char*>(data + pos),size - pos,buffer);					
					break;
				case CodeDocument::UTF16LE:
				case CodeDocument::UTF16BE:
					// No conversion needed
					text = reinterpret_cast<const wchar_t*>(data + pos);
					n = size - pos; // Number of bytes

					if (encoding == CodeDocument::UTF16BE) {
						buffer.assign(text,text + n / sizeof(wchar_t));
						std::for_each(buffer.begin(),buffer.end(),SwapCodePoint);

						text = 0; n = 0; // Will be reassigned
					}
					use_bom_ = true; // We use always a BOM for UTF16
					break;
				case CodeDocument::UTF32LE:
				case CodeDocument::UTF32BE:
					UTF32toUTF16(reinterpret_cast<const char*>(data + pos),size - pos,
						buffer,encoding == CodeDocument::UTF32LE);
					use_bom_ = true; // We use always a BOM for UTF32
					break;
			}

			if (!text && !buffer.empty()) {
				text = &buffer[0];
				n = buffer.size();

				string.SetString(text, static_cast<int>(n));
			}

			this->encoding_ = encoding;

			if (!use_bom_ && pos != 0)
				use_bom_ = true;
		}
		else {
			//The file cannot be memory-mapped, but it exists on disk.
			//An example is an empty file.
			// => We will not report an error for this.
			//result = ::GetLastError();
		}
	}
	else
		result = ::GetLastError();

	return result == ERROR_SUCCESS;
}

bool TextDocument::Write( LPCTSTR pszFileName, const CStringW& string )
{
	const int length = string.GetLength();
	DWORD result = ERROR_SUCCESS;

	if (length > 0) {
		const char* text = 0;
		std::size_t n = 0;

		std::vector<char> convbuffer;
		const char* p = reinterpret_cast<const char*>(static_cast<LPCWSTR>(string));
		std::size_t size = string.GetLength() * sizeof(wchar_t);

		switch (encoding_) {
			case CodeDocument::ANSI:
				UTF16toANSI(p,size,convbuffer,code_page_);
				break;
			case CodeDocument::UTF8:
				UTF16toUTF8(p,size,convbuffer);				
				break;
			case CodeDocument::UTF16LE:
			case CodeDocument::UTF16BE:
				if (encoding_ == CodeDocument::UTF16BE) {
					convbuffer.assign(p,p + size);

					if (!convbuffer.empty())
						std::for_each(reinterpret_cast<wchar_t*>(&convbuffer[0]),
							reinterpret_cast<wchar_t*>(&convbuffer[0] + convbuffer.size()),SwapCodePoint);
				}
				else {
					text = p;
					n = size;
				}
				break;
			case CodeDocument::UTF32LE:
			case CodeDocument::UTF32BE:
				UTF32toUTF16(p,size,convbuffer,encoding_ == CodeDocument::UTF32LE);
				break;
		}

		if (!text && !convbuffer.empty()) {
			text = &convbuffer[0];
			n = convbuffer.size();
		}

		ATL::CAtlFile file;

		if (SUCCEEDED(file.Create(pszFileName,GENERIC_WRITE,FILE_SHARE_READ,CREATE_ALWAYS)))
			result = Write(file,text,n);
		else
			result =::GetLastError();
	}

	return result == ERROR_SUCCESS;
}

DWORD TextDocument::WriteUTF8( ATL::CAtlFile& file, const char* utf8, std::size_t size )
{
	const char* text = 0;
	std::size_t n = 0;

	std::vector<char> convbuffer;
	const char* p = utf8;

	switch (encoding_) {
		case CodeDocument::ANSI:
			UTF8toANSI(p,size,convbuffer,code_page_);
			break;
		case CodeDocument::UTF8:
			text = p;
			n = size;
			break;
		case CodeDocument::UTF16LE:
		case CodeDocument::UTF16BE:
			UTF8toUTF16(p,size,convbuffer,encoding_ != CodeDocument::UTF16BE);
			break;
		case CodeDocument::UTF32LE:
		case CodeDocument::UTF32BE:
			UTF8toUTF16(p,size,convbuffer,encoding_ == CodeDocument::UTF32LE);
			break;
	}

	if (!text && !convbuffer.empty()) {
		text = &convbuffer[0];
		n = convbuffer.size();
	}

	return Write(file,text,n);
}

void TextDocument::SetEncoding( CodeDocument::Encoding e )
{
	encoding_ = e;
}

UINT TextDocument::GetCodePage() const
{
	return code_page_;
}

void TextDocument::SetCodePage( UINT codepage )
{
	code_page_ = codepage;
}

bool TextDocument::GetUseBOM() const
{
	return use_bom_;
}

void TextDocument::SetUseBOM( bool use )
{
	use_bom_ = use;
}

// CodeDocument

IMPLEMENT_DYNAMIC(CodeDocument, CScintillaDoc)

CodeDocument::CodeDocument()
: create_backup_file_(false)
, encoding_(ANSI) // Current user code page, see code_page_ data member
, code_page_(::GetACP())
, speller_thread_(theApp.GetSpellerThread())
, error_marker_handle_(-1)
, use_bom_(false)
, save_copy_(false)
, surpressModifiedChange_(false)
{
}

BOOL CodeDocument::OnNewDocument()
{
	if (!CScintillaDoc::OnNewDocument())
		return FALSE;

	// *
	if (m_strPathName.IsEmpty())
		m_strPathName = m_strTitle;

	GetView()->GetCtrl().SetEOLMode(ToScintillaMode(CConfiguration::GetInstance()->m_nStandardFileFormat));

	return TRUE;
}

CodeDocument::~CodeDocument()
{
}


BEGIN_MESSAGE_MAP(CodeDocument, CScintillaDoc)
	ON_COMMAND(ID_EDIT_TOGGLE_BOOKMARK, &CodeDocument::OnEditToggleBookmark)
	ON_COMMAND(ID_EDIT_CLEAR_ALL_BOOKMARKS, &CodeDocument::OnEditClearAllBookmarks)
	ON_COMMAND(ID_EDIT_SPLIT_PARAGRAPH, &CodeDocument::OnEditSplitParagraph)
	ON_COMMAND(ID_EDIT_JOIN_PARAGRAPH, &CodeDocument::OnEditJoinParagraph)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CodeDocument::OnUpdateFileSave)
	ON_COMMAND(ID_FILE_SAVE_COPY_AS, &CodeDocument::OnFileSaveCopyAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, &CodeDocument::OnUpdateFileSaveAs)
	ON_COMMAND(ID_EDIT_MAKELOWERCASE, &CodeDocument::OnEditMakeLowerCase)
	ON_COMMAND(ID_EDIT_MAKEUPPERCASE, &CodeDocument::OnEditMakeUpperCase)
END_MESSAGE_MAP()


// CodeDocument diagnostics

#ifdef _DEBUG
void CodeDocument::AssertValid() const
{
	CScintillaDoc::AssertValid();
}

#ifndef _WIN32_WCE
void CodeDocument::Dump(CDumpContext& dc) const
{
	CScintillaDoc::Dump(dc);
}
#endif
#endif //_DEBUG

#ifndef _WIN32_WCE
// CodeDocument serialization

void CodeDocument::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
		SaveFile(ar.GetFile()->m_hFile,false);
	else
		LoadFile(ar.GetFile()->m_hFile);
}
#endif

// CodeDocument commands

void CodeDocument::SetModifiedFlag(BOOL modified)
{
	CScintillaDoc::SetModifiedFlag(modified);
}

CodeDocument::Encoding CodeDocument::GetEncoding() const
{
	return encoding_;
}

UINT CodeDocument::GetCodePage() const
{
	return code_page_;
}

#pragma region Reading & writing to/from files

DWORD CodeDocument::LoadFile(LPCTSTR pszFileName)
{
	DWORD result = ERROR_SUCCESS;
	CAtlFile file;

	if (SUCCEEDED(file.Create(pszFileName,GENERIC_READ,FILE_SHARE_READ,OPEN_EXISTING)))
		result = LoadFile(file);
	else
		result = ::GetLastError();

	return result;
}

DWORD CodeDocument::LoadFile(HANDLE file)
{
	DWORD result = ERROR_SUCCESS;
	ATL::CAtlFileMapping<BYTE> mapping;

	if (SUCCEEDED(mapping.MapFile(file))) {
		const BYTE* data = mapping;
		SIZE_T size = mapping.GetMappingSize();
		SIZE_T pos = 0;

		Encoding encoding = TextDocument::DetectEncoding(data,pos,size);

		if (encoding == ANSI) { // ANSI? Still might by Unicode without BOM
			encoding = TextDocument::TestForUnicode(data,size);
		}

		UINT code_page = ::GetACP(); // Default user code page for ANSI text
		
		std::vector<char> buffer;
		const char* text = 0;
		std::size_t n = 0;

		switch (encoding) {
			case ANSI:
				ANSItoUTF8(reinterpret_cast<const char*>(data + pos),size - pos,buffer,code_page);
				break;
			case UTF8:
				// No conversion needed
				text = reinterpret_cast<const char*>(data + pos);
				n = size - pos;
				break;
			case UTF16LE:
			case UTF16BE:
				UTF16toUTF8(reinterpret_cast<const char*>(data + pos),size - pos,
					buffer,encoding == UTF16LE);
				break;
			case UTF32LE:
			case UTF32BE:
				UTF32toUTF8(reinterpret_cast<const char*>(data + pos),size - pos,
					buffer,encoding == UTF32LE);
				break;
		}

		if (!text && !buffer.empty()) {
			text = &buffer[0];
			n = buffer.size();
		}

		// Prevent * to show up quickly in tab's title while the content is
		// reloaded
		surpressModifiedChange_ = true;

		CScintillaDoc::DeleteContents();
		GetView()->GetCtrl().SetCodePage(SC_CP_UTF8);

		if (n) {
			// Detected EOL type
			GetView()->GetCtrl().SetEOLMode(DetectScintillaEOLMode(text, n));
			GetView()->GetCtrl().AppendText(static_cast<int>(n), text);			
		}

		use_bom_ = pos > 0;
		code_page_ = code_page;
		encoding_ = encoding;

		// No undo operation needed so far, we have just opened the document
		GetView()->GetCtrl().EmptyUndoBuffer();

		surpressModifiedChange_ = false;

		SetModifiedFlag(FALSE);
	}
	else
		result = ::GetLastError();

	return result;
}

DWORD CodeDocument::SaveFile(LPCTSTR pszFileName, bool clearModifiedFlag)
{
	DWORD result = ERROR_ACCESS_DENIED;

	const CString& directory = CPathTool::GetDirectory(pszFileName);
	const CString& tempFileName = CPathTool::GetTempFileName(directory, _T("CRE"), 0);

	const DWORD moveFlags = MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH;

	if (!tempFileName.IsEmpty()) {
		ATL::CAtlFile tempFile;

		if (SUCCEEDED(tempFile.Create(tempFileName, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS))) {
			DWORD tempResult = SaveFile(tempFile, false);

			if (create_backup_file_) {
				if (CPathTool::Exists(pszFileName)) {
					const CString& backupFileName = CString(pszFileName) + _T(".bak");
					// File exists: create backup file
					MoveFileEx(pszFileName, backupFileName, moveFlags);
				}
			}

			tempFile.Close();

			// Move temporary file to target name
			if (tempResult == ERROR_SUCCESS) {				
				if (MoveFileEx(tempFileName, pszFileName, moveFlags))
					result = ERROR_SUCCESS;
			}
			else
				result = tempResult;
		}
	}

	if (result == ERROR_SUCCESS) {
		DeleteFile(tempFileName);

		if (clearModifiedFlag)
			SetModifiedFlag(FALSE);
	}
	else
		result = ::GetLastError();

	return result;
}

DWORD CodeDocument::SaveFile( HANDLE file, const char* text, std::size_t n)
{
	CAtlFile f(file);
	
	TextDocument s(use_bom_,encoding_);
	DWORD result = s.Write(f,text,n);

	f.Detach();

	return result;
}

DWORD CodeDocument::SaveFile(HANDLE file, bool throw_on_invalid_sequence)
{
	const Nullable<int>& mode = GetSavedEOLMode();

	if (mode.HasValue() && GetView()->GetCtrl().GetEOLMode() != mode) {
		ASSERT(mode == SC_EOL_CR || mode == SC_EOL_CRLF || mode == SC_EOL_LF);

		GetView()->GetCtrl().SetEOLMode(mode.GetValue());
		GetView()->GetCtrl().ConvertEOLs(mode.GetValue());
	}

	return CodeDocument::DoSaveFile(file,throw_on_invalid_sequence);
}

DWORD CodeDocument::DoSaveFile( HANDLE file, bool throw_on_invalid_sequence )
{
	const int length = GetView()->GetCtrl().GetLength();
	DWORD result = ERROR_SUCCESS;

	if (length > 0) {
		std::vector<char> buffer(length + 1);
		GetView()->GetCtrl().GetText(length + 1,&buffer[0]);

		buffer.pop_back(); // Strip the terminating zero

		const char* text = 0;
		std::size_t n = 0;

		std::vector<char> convbuffer;

		switch (encoding_) {
			case ANSI:
				UTF8toANSI(&buffer[0],buffer.size(),convbuffer,code_page_,'?',throw_on_invalid_sequence);
				break;
			case UTF8:
				text = &buffer[0];
				n = buffer.size();
				break;
			case UTF16LE:
			case UTF16BE:
				UTF8toUTF16(&buffer[0],buffer.size(),convbuffer,encoding_ == UTF16LE);
				break;
			case UTF32LE:
			case UTF32BE:
				UTF8toUTF32(&buffer[0],buffer.size(),convbuffer,encoding_ == UTF32LE);
				break;
		}

		if (!text && !convbuffer.empty()) {
			text = &convbuffer[0];
			n = convbuffer.size();
		}

		result = SaveFile(file,text,n);
	}

	return result;
}

#pragma endregion

void CodeDocument::SetSpellerThread( SpellerBackgroundThread* t )
{
	speller_thread_ = t;
}

SpellerBackgroundThread* CodeDocument::GetSpellerThread() const
{
	return speller_thread_;
}

bool CodeDocument::IsSpellerThreadAttached() const
{
	return GetSpellerThread() != 0;
}

bool CodeDocument::ShowSaveDialog(LPCTSTR file) const
{
	CString title(GetTitle());
	title.TrimRight(ModifiedMarker);

	return !file || title == file;
}

void CodeDocument::SetErrorMark(int line)
{
	if (error_marker_handle_ != -1) {
		GetView()->GetCtrl().MarkerDeleteHandle(error_marker_handle_);
		error_marker_handle_ = -1;
	}

	if (line > 0) {
		--line;
		error_marker_handle_ = GetView()->GetCtrl().
			MarkerAdd(line,CodeDocument::Errormark);
		GetView()->GetCtrl().GotoLine(line);
	}
}

bool CodeDocument::HasErrorMark() const
{
	return error_marker_handle_ != -1;
}

BOOL CodeDocument::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CScintillaDoc::OnOpenDocument(lpszPathName))
		return FALSE;

	error_marker_handle_ = -1;
	
	CodeView* view = static_cast<CodeView*>(GetView());
	ASSERT_VALID(view);

	view->SetModified(false);
	::SHAddToRecentDocs(SHARD_PATH, lpszPathName);

	if (CLaTeXProject* p = theApp.GetProject()) {
		BookmarkContainerType bookmarks;

		// Restore the bookmarks, if any
		if (p->GetBookmarks(lpszPathName,bookmarks))
			SetBookmarks(bookmarks.begin(),bookmarks.end());

		FoldingPointContainerType points;

		// Restore the folding
		if (p->GetFoldingPoints(lpszPathName,points))
			SetFoldingPoints(points.begin(),points.end());
	}

	return TRUE;
}

void CodeDocument::OnEditToggleBookmark()
{
	int line = GetView()->GetCtrl().LineFromPosition(GetView()->GetCtrl().GetCurrentPos());
	ToggleBookmark(line);
}

void CodeDocument::OnEditClearAllBookmarks()
{
	RemoveAllBookmarks();
	OnRemovedAllBookmarks();
}

void CodeDocument::OnRemovedAllBookmarks(void)
{
	if (CLaTeXProject* p = theApp.GetProject())
		p->RemoveAllBookmarks(GetFilePath());
}

void CodeDocument::RemoveAllBookmarks()
{
	GetView()->GetCtrl().MarkerDeleteAll(Bookmark);
}

const BookmarkContainerType CodeDocument::GetBookmarks()
{
	BookmarkContainerType bookmarks;
	int line = 0;

	while ((line = GetView()->GetCtrl().MarkerNext(line,1 << CodeDocument::Bookmark)) >= 0) {
		bookmarks.push_back(CodeBookmark(line));
		++line;
	}

	return bookmarks;
}

void CodeDocument::ToggleBookmark( int line )
{
	int marker = GetView()->GetCtrl().MarkerGet(line);

	const ::CodeBookmark b(line);

	if (marker & 1 << Bookmark) { // present
		GetView()->GetCtrl().MarkerDelete(line,Bookmark);
		OnBookmarkRemoved(b);
	}
	else {
		GetView()->GetCtrl().MarkerAdd(line,Bookmark);
		OnBookmarkAdded(b);
	}
}

void CodeDocument::SetEncoding(Encoding e)
{
	encoding_ = e;
}

bool CodeDocument::GetUseBOM() const
{
	return use_bom_;
}

void CodeDocument::SetUseBOM( bool use /*= true*/ )
{
	use_bom_ = use;
}

BOOL CodeDocument::SaveModified()
{
	return DoSaveModified();
}

BOOL CodeDocument::DoSave(LPCTSTR lpszPathName, BOOL bReplace /*= TRUE*/)
{
	CString newName(lpszPathName);

	if (ShowSaveDialog(lpszPathName)) {
		CDocTemplate* pTemplate = GetDocTemplate();
		ASSERT(pTemplate != NULL);

		CString extension;
		pTemplate->GetDocString(extension,CDocTemplate::filterExt);
		ASSERT(!extension.IsEmpty() && extension[0] == _T('.'));

		int semicolon = extension.Find(_T(';'));

		if (semicolon != -1)
			extension = extension.Left(semicolon);

		newName = m_strPathName;

		if (bReplace && newName.IsEmpty()) {
			newName = m_strTitle;
			// check for dubious filename
			int index = newName.FindOneOf(_T(" #%;/\\"));

			if (index != -1)
				newName.ReleaseBuffer(index);

			// append the default suffix if there is one
			if (!extension.IsEmpty())
				newName += extension;
		}

		if (CPathTool::GetFileExtension(newName).IsEmpty())
			newName += extension;

		int mode = FromScintillaMode(GetView()->GetCtrl().GetEOLMode());

		if (!EOL_mode_.HasValue()) // Not set yet
			EOL_mode_ = ToScintillaMode(mode);

		CTextFileSaveDialog dlg(
			bReplace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
			extension,newName,
			OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN,
			mode, GetExtensionFilter());

		dlg.SetUseBOM(GetUseBOM());
		dlg.SetEncoding(GetEncoding());

		//Show the dialog
		if (dlg.DoModal() != IDOK) {
			//It was canceled - the PathName is not set.
			// Therefore, we use GetLastOpenedFolder.
			AfxSetLastDirectory(dlg.GetLastOpenedFolder());
			return false;
		}

		AfxSetLastDirectory(CPathTool::GetDirectory(dlg.GetPathName()));
		EOL_mode_ = ToScintillaMode(dlg.GetFileFormat());

		// Delay line ending conversion as long as possible
		// LaTeXView::SaveFile(HANDLE) will perform if it's succeeded so far

		SetEncoding(dlg.GetEncoding());
		SetUseBOM(dlg.GetUseBOM());

		save_copy_ = !bReplace;
		newName = dlg.GetPathName();
	}
	else
		EOL_mode_ = GetView()->GetCtrl().GetEOLMode();

	return CScintillaDoc::DoSave(newName, bReplace);
}

BOOL CodeDocument::DoSaveModified()
{
	BOOL result = TRUE;

	// TODO: Sometimes this function will be called
	// although the attached view has been already destroyed. Find out why.
	if (!IsModified())
		result = TRUE;        // ok to continue
	else {
		// get name/title of document
		CString name;

		if (m_strPathName.IsEmpty()) {
			// get name based on caption
			name = m_strTitle;

			if (name.IsEmpty())
				ENSURE(name.LoadString(AFX_IDS_UNTITLED));
		}
		else {
			extern UINT AFXAPI AfxGetFileTitle(LPCTSTR, LPTSTR, UINT);
			// get name based on file title of path name
			name = m_strPathName;
			AfxGetFileTitle(m_strPathName, name.GetBuffer(MAX_PATH), MAX_PATH);
			name.ReleaseBuffer();
		}

		CString prompt;
		AfxFormatString1(prompt,AFX_IDP_ASK_TO_SAVE,name);

		int button = ShowSaveTaskDialog(prompt);

		switch (button) {
			case IDCANCEL:
				result = FALSE;       // don't continue
				break;

			case IDYES:
				// If so, either Save or Update, as appropriate
				if (!DoFileSave())
					result = FALSE;       // don't continue
				break;

			case IDNO:
				// If not saving changes, revert the document
				result = TRUE;    // keep going
				break;

			case IDOK:
				result = TRUE;    // keep going
				break;

			default:
				ASSERT(FALSE);
				break;
		}
	}

	return result;
}

bool CodeDocument::LineContainsOnlyWhiteSpace(const int Line) const
{
	CScintillaCtrl& Ctrl = GetView()->GetCtrl();

	if (Line < 0 || Line >= Ctrl.GetLineCount()) return false;

	long CurrentPosition = Ctrl.PositionFromLine(Line);
	long EndPosition = Ctrl.GetLineEndPosition(Line);
	for(;CurrentPosition<EndPosition;CurrentPosition++)
	{
		int Character = Ctrl.GetCharAt(CurrentPosition);
		if (Character != _T(' ') && Character != _T('\t')) return false;
	}

	return true;
}

bool CodeDocument::LineContainsComment(const int Line) const
{
	return (LineFindCommentStartPos(Line) >= 0);
}

long CodeDocument::LineFindCommentStartPos(const int Line) const
{
	CScintillaCtrl& Ctrl = GetView()->GetCtrl();

	if (Line < 0 || Line >= Ctrl.GetLineCount()) return -1;

	long CurrentPosition = Ctrl.PositionFromLine(Line);
	long EndPosition = Ctrl.GetLineEndPosition(Line);
	for(;CurrentPosition<EndPosition;CurrentPosition++)
	{
		if (Ctrl.GetStyleAt(CurrentPosition) == SCE_TEX_COMMENT) return CurrentPosition;
	}

	return -1;
}

const std::pair<long,long> CodeDocument::GetParagraphRangePos(const int StartPos, const bool bRespectComments /*= true*/,
																const bool bNoLastNewLine /*= true*/) const
{
	CScintillaCtrl& Ctrl = GetView()->GetCtrl();
	const int NumLines = Ctrl.GetLineCount();
	const int StartLine = Ctrl.LineFromPosition(StartPos);

	if (LineContainsOnlyWhiteSpace(StartLine)) return std::make_pair(StartPos, StartPos);

	int CommentStartInStartLine(-1);
	if (bRespectComments)
	{
		CommentStartInStartLine = LineFindCommentStartPos(StartLine);
		if (CommentStartInStartLine >= 0 && CommentStartInStartLine <= StartPos)
		{
			//We do not treat comments as parts of a paragraph
			return std::make_pair(StartPos, StartPos);
		}
	}

	int UpLine = StartLine;
	int DownLine = StartLine;

	//Find the start of the paragraph
	do
	{
		UpLine--;
	}
	while (UpLine >= 0 && !LineContainsOnlyWhiteSpace(UpLine) && (!bRespectComments || !LineContainsComment(UpLine)));

	//Current UpLine did not match, i.e., contains whites or comments or is below zero
	UpLine++;
	const int UpPos = Ctrl.PositionFromLine(UpLine);


	//Find the end of the paragraph; if the start line does not have a comment.
	if (CommentStartInStartLine < 0)
	{
		do
		{
			DownLine++;
		}
		while (DownLine < NumLines && !LineContainsOnlyWhiteSpace(DownLine) && (!bRespectComments || !LineContainsComment(DownLine)));

		//Current DownLine did not match, i.e., contains whites or comments or is above maximal num
		if (DownLine >= NumLines || LineContainsOnlyWhiteSpace(DownLine))
		{
			DownLine--;
		}
	}

	long DownPos = Ctrl.GetLineEndPosition(DownLine);
	if (bRespectComments)
	{
		const long CommentStartInLastLine = LineFindCommentStartPos(DownLine);
		if (CommentStartInLastLine >= 0)
		{
			DownPos = CommentStartInLastLine;
		}
	}

	//If the given pos is the first char in a line, we go back to the last char of the previous line.
	if (bNoLastNewLine && UpPos != DownPos && DownPos == Ctrl.PositionFromLine(Ctrl.LineFromPosition(DownPos)))
	{
		DownPos = Ctrl.GetLineEndPosition(Ctrl.LineFromPosition(DownPos) - 1);
	}

	return std::make_pair(UpPos, DownPos);
}

void CodeDocument::OnEditSplitParagraph()
{
	CScintillaCtrl& c = GetView()->GetCtrl();

	std::pair<long,long> Range;
	Range.first = c.GetSelectionStart();
	Range.second = c.GetSelectionEnd();

	//Nothing selected by the user?
	if (Range.first == Range.second)
	{
		Range = GetParagraphRangePos(c.GetCurrentPos());
		//Anything useful from the automatic detection?
		if (Range.first == c.GetCurrentPos() && Range.second == c.GetCurrentPos()) return;
	}

	//Is our end in the middle of a line?
	if (c.GetLineEndPosition(c.LineFromPosition(Range.second)) != Range.second)
	{
		//Insert a line break
		c.InsertText(Range.second, _T("\n"));
	}

	c.SetTargetStart(Range.first);
	c.SetTargetEnd(Range.second);

	//Get the width for the wrapping
	const int width = c.TextWidth(STYLE_DEFAULT,"_") * CConfiguration::GetInstance()->m_nFixedColumnWrap;

	//Call LinesSplit()
	c.LinesSplit(width);

	/* Would be nice, if this worked properly. Press Ctrl-K more than once and you'll see.
	//NOTE: This is a workaround for a the current tex lexer, which makes underscores
	// and other characters to symbols, i.e., gives them a special style.
	// Scintilla wraps between styles or at spaces/tabs.
	// By disabling all styles here, we allow breaks only at spaces/tabs.
	// - get old lexer
	const int idOldLexer = c.GetLexer();
	// - set null lexer
	c.SetLexer(SCLEX_NULL);
	c.ClearDocumentStyle();
	// - actual splitting
	c.LinesSplit(width);
	// - reset old lexer
	c.SetLexer(idOldLexer);
	c.ClearDocumentStyle();
	c.Colourise(0, -1);
	*/
}

void CodeDocument::OnEditJoinParagraph()
{
	CScintillaCtrl& c = GetView()->GetCtrl();

	std::pair<long,long> Range;
	Range.first = c.GetSelectionStart();
	Range.second = c.GetSelectionEnd();

	//Nothing selected by the user?
	if (Range.first == Range.second)
	{
		Range = GetParagraphRangePos(c.GetCurrentPos());
		//Anything useful from the automatic detection?
		if (Range.first == c.GetCurrentPos() && Range.second == c.GetCurrentPos()) return;
	}

	c.SetTargetStart(Range.first);
	c.SetTargetEnd(Range.second);

	c.LinesJoin();
}

const FoldingPointContainerType CodeDocument::GetFoldingPoints( bool contracted /*= false*/ )
{
	FoldingPointContainerType points;

	CScintillaCtrl& c = GetView()->GetCtrl();

	const int lines = c.GetLineCount();
	int level;
	bool folded;

	for (int i = 0; i < lines; ++i) {
		level = c.GetFoldLevel(i);
		folded = c.GetFoldExpanded(i) == 0;

		if (level & SC_FOLDLEVELHEADERFLAG && (!contracted || folded)) // SC_FOLDLEVELHEADERFLAG indicates a fold point
			points.push_back(FoldingPoint(i,folded));
	}

	return points;
}

void CodeDocument::OnCloseDocument()
{
	if (GetView()) { // Is the view attached?
		if (CLaTeXProject* p = theApp.GetProject())
			p->SetFoldingPoints(GetPathName(),GetContractedFoldingPoints());
	}

	// Destroying big documents takes a great amount of time.
	// While destroying the document can be found in
	// template's list. If the user tries to reopen the *same* document rapidly
	// it won't be shown, because CTeXnicCenterApp::OpenLatexDocument
	// will find a valid pointer to the document that is being destroyed and
	// return it. So we need to remove the document from the template list
	// *before* DestroyWindow is called.
	GetDocTemplate()->RemoveDocument(this);
	// DestroyWindow etc.
	CScintillaDoc::OnCloseDocument();
}

const FoldingPointContainerType CodeDocument::GetContractedFoldingPoints()
{
	return GetFoldingPoints(true);
}

const Nullable<int>& CodeDocument::GetSavedEOLMode() const
{
	return EOL_mode_;
}

const CString CodeDocument::GetExtensionFilter() const
{
	CString text = CTeXnicCenterApp::GetDocTemplateFilter(GetDocTemplate());	
	text += CString(MAKEINTRESOURCE(STE_ALL_FILES_FILTER));

	return text;
}

void CodeDocument::OnUpdateFileSave(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsModified());
}

void CodeDocument::OnFileSaveCopyAs()
{
	DoSave(NULL,FALSE);
}

void CodeDocument::OnUpdateFileSaveAs(CCmdUI *pCmdUI)
{
	UNUSED_ALWAYS(pCmdUI);
}

void CodeDocument::OnEditMakeLowerCase()
{
	if (GetView() && GetView()->GetCtrl())
	{
		GetView()->GetCtrl().LowerCase();
	}
}

void CodeDocument::OnEditMakeUpperCase()
{
	if (GetView() && GetView()->GetCtrl())
	{
		GetView()->GetCtrl().UpperCase();
	}
}

void CodeDocument::UpdateReadOnlyFlag()
{
	CFileStatus fs;

	if (CFile::GetStatus(GetPathName(),fs))
		GetView()->GetCtrl().SetReadOnly(fs.m_attribute & CFile::readOnly);
}

void CodeDocument::CheckForFileChanges()
{
	WORD wChanges = GetFileChanges();

	if (wChanges & chReadOnly)
		UpdateReadOnlyFlag();
	else if (wChanges)
		UpdateTextBufferOnExternalChange();
}

void CodeDocument::UpdateTextBufferOnExternalChange()
{
	CString strMsg;
	int nResult;

	if (IsModified()) {
		strMsg.Format(STE_FILE_EXTERNALCHANGEEX, (LPCTSTR)GetPathName());
		nResult = AfxMessageBox(strMsg,MB_ICONEXCLAMATION | MB_YESNO);
	}
	else {
		strMsg.Format(STE_FILE_EXTERNALCHANGE, (LPCTSTR)GetPathName());
		nResult = AfxMessageBox(strMsg,MB_ICONINFORMATION | MB_YESNO);
	}

	if (nResult == IDYES) {
		DWORD dwResult = LoadFile(GetPathName());

		if (dwResult != ERROR_SUCCESS) {
			strMsg.Format(STE_FILE_INUSE,
				(LPCTSTR)AfxLoadString(IDS_OPEN),
				(LPCTSTR)GetPathName(),
				(LPCTSTR)AfxFormatSystemString(dwResult));
			AfxMessageBox(strMsg,MB_ICONINFORMATION | MB_OK);
			GetView()->GetCtrl().SetReadOnly(TRUE);
		} else {
			//Trigger analysis - parse project
			AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_PROJECT_PARSE);
		}
	}
}

void CodeDocument::DeleteContents()
{
	SetFilePath(NULL);

	if (CScintillaView* v = GetView())
		v->GetCtrl().ClearAll();

	current_line_ = 0;

	CScintillaDoc::DeleteContents();
}

void CodeDocument::Delete()
{
}

BOOL CodeDocument::GetNextLine(LPCTSTR &lpLine, int &nLength)
{
	AssertValid();

	if (current_line_ >= GetView()->GetCtrl().GetLineCount())
		return FALSE;

	static_cast<CodeView*>(GetView())->GetLineText(current_line_++);
	lpLine = line_;
	nLength = line_.GetLength();

	return TRUE;
}

void CodeDocument::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
{
	SetFilePath(lpszPathName);
	CScintillaDoc::SetPathName(lpszPathName,bAddToMRU);
}

void CodeDocument::OnBookmarkAdded(const CodeBookmark& b)
{
	if (CLaTeXProject* p = theApp.GetProject())
		p->AddBookmark(GetFilePath(),b);
}

void CodeDocument::OnBookmarkRemoved(const CodeBookmark& b)
{
	if (CLaTeXProject* p = theApp.GetProject())
		p->RemoveBookmark(GetFilePath(),b);
}

BOOL CodeDocument::OnSaveDocument(LPCTSTR lpszPathName)
{
	DWORD result = SaveFile(lpszPathName,!save_copy_);
	BOOL b;

	if (result != ERROR_SUCCESS) {
		CString strMsg;
		strMsg.Format(STE_FILE_INUSE,
			(LPCTSTR)AfxLoadString(IDS_SAVE),
			lpszPathName,
			(LPCTSTR)AfxFormatSystemString(result));
		AfxMessageBox(strMsg,MB_ICONEXCLAMATION | MB_OK);

		save_copy_ = false;
		b = FALSE;
	}
	else {
		b = TRUE;

		save_copy_ = false;
		theApp.m_pMainWnd->SendMessage(WM_COMMAND,ID_DOCUMENT_SAVED);
	}

	SnapFileState();

	return TRUE;
}

void CodeDocument::MarkTitleAsModified(bool modified)
{
	if (!surpressModifiedChange_) {
		const TCHAR ch = ModifiedMarker;
		CString title = GetTitle();

		if (modified)
			title += ch;
		else
			title.TrimRight(ch);

		SetTitle(title);
	}
}

BOOL CodeDocument::IsModified()
{
	return GetView()->GetCtrl().GetModify();
}
