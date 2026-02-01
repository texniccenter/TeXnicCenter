#pragma once
#include <vector>

class StructureItemInfo;
class StructureItem;

/**	An Array of StructureItem-objects.

@author Sven Wiegand
*/
typedef std::vector<StructureItem> StructureItemContainer;

/**
An object of this class contains information about one item
of a document structure (for example a header).

@author Sven Wiegand
 */
class StructureItem
{
	//Construction/Destruction
	mutable int main_project_file_;
	/// Detail item information, e.g. BibItem for BibTeX entries
	std::shared_ptr<StructureItemInfo> item_info_;
	bool missing_;
	CString prefix_;
	
public:
	enum Type
	{
		none = -1,
		generic = 0,
		header,
		equation,
		quote,
		quotation,
		center,
		verse,
		itemization,
		enumeration,
		description,
		figure,
		table,
		unknownEnv,
		texFile,
		group,
		bibFile,
		graphicFile,
		bibItem,
		missingTexFile,
		missingGraphicFile,
		missingBibFile,
		// If you add a new type, also add a description string to m_sItemNames
		typeCount
	};

	typedef std::vector<CString> LabelContainer;

	bool HasItemInfo() const;
	void SetItemInfo(StructureItemInfo* info = 0);
	StructureItemInfo* GetItemInfo();
	const StructureItemInfo* GetItemInfo() const;

	const CString& GetPrefix() const;
	void SetPrefix(const CString& val);

private:
	LabelContainer labels_;
	int depth_;
	bool isCopy_;
	StructureItemContainer::difference_type original_;

public:
	StructureItem();

	void SetDepth(int depth);
	int GetDepth() const;

	bool HasParent() const;
	void SetMissing(bool missing = true);
	bool IsMissing() const;

//Attributes
public:
	/** Type this item is of. */
	int m_nType;

	int GetType() const;
	void SetType(int val);

	/** Path of file this item occurs in. */
	CString m_strPath;

	const CString& GetPath() const;
	void SetPath(const CString& val);

	/** Line in the file, this item occurs on. */
	int m_nLine;

	int GetLine() const;
	void SetLine(int val);

	/** Caption of this item */
	CString m_strCaption;

	const CString& GetCaption() const;
	void SetCaption(const CString& val);

	/** Label of this item */
	//CString m_strLabel;

	const CString GetLabel() const;
	void SetLabel(const CString& val);

	bool HasLabels() const;

	/** Title of this item */
	CString m_strTitle;

	const CString& GetTitle() const;
	void SetTitle(const CString& val);

	const LabelContainer& GetLabels() const;

	LabelContainer& GetLabels();

	void AddLabel(const CString& l);

	void RemoveLabels();

	/** Comment of this item */
	CString m_strComment;

	const CString& GetComment() const;
	void SetComment(const CString& val);

	const StructureItem* GetParent(const StructureItemContainer& items) const;

	/** Index of the parent item or -1 if this item has no parent item */
	StructureItemContainer::difference_type m_nParent;

	StructureItemContainer::difference_type GetParent() const;
	void SetParent(StructureItemContainer::difference_type val);

	bool IsMainProjectFile() const;

	static bool IsEnvironment(Type type);
	bool IsEnvironment() const;
	bool IsHeader() const;

	/// Gets a value indicating whether this item is a copy of an existing one.
	bool IsCopy() const;

	void SetCopy(bool val = true);
	
	/// If this item is a copy the method return the index of the original item or -1 otherwise.
	StructureItemContainer::difference_type GetOriginal() const;

	void SetOriginal(StructureItemContainer::difference_type val);
};
