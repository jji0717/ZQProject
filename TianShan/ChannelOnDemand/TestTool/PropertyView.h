#if !defined(AFX_PROPERTYVIEW_H__DE637AEB_1904_4907_AC13_EA6679DEEAB6__INCLUDED_)
#define AFX_PROPERTYVIEW_H__DE637AEB_1904_4907_AC13_EA6679DEEAB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertyView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPropertyView view
typedef enum
{
	ItemType = 0, 
	ChannelType
} ViewType;

class CPropertyView : public CListView
{
protected:
	CPropertyView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPropertyView)

// Attributes
public:
	ViewType _viewType;

// Operations
public:
	void ShowChannels(const std::vector<std::string>& chnlIds);
	void ShowItems(const std::vector<std::string>& chnlItems, const std::string& chnlName);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPropertyView();
	void ModifyChannelItem(BOOL bModifyFlag = TRUE);
	void ModifyChannel();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropertyView)
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPushItem();
	afx_msg void OnInsertItem();
	afx_msg void OnModifyItem();
	afx_msg void OnReplaceItem();
	afx_msg void OnReturn(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnModifyChannel();
	afx_msg void OnNewChannel();
	afx_msg void OnRemoveChannel();
	afx_msg void OnRemoveItem();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTYVIEW_H__DE637AEB_1904_4907_AC13_EA6679DEEAB6__INCLUDED_)
