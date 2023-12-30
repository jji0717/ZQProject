// MN_Sessions1.cpp : implementation file
//

#include "stdafx.h"
#include "SysTray.h"
//#include "SysTrayDlg.h"
#include "MN_Sessions.h"
#include "PopSessionDetail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MN_Sessions dialog


MN_Sessions::MN_Sessions(CWnd* pParent /*=NULL*/)
	: CInfoDlg(MN_Sessions::IDD, pParent)
{
	//{{AFX_DATA_INIT(MN_Sessions)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void MN_Sessions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MN_Sessions)
	DDX_Control(pDX, IDC_LIST_SESSIONS, m_sessions);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MN_Sessions, CDialog)
	//{{AFX_MSG_MAP(MN_Sessions)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_SESSIONS, OnColumnclickListSessions)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_SESSIONS, OnDblclkListSessions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MN_Sessions message handlers

BOOL MN_Sessions::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	// Set styles
	m_sessions.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_sessions.SetBkColor(RGB(247,247,255));
	m_sessions.SetTextColor(RGB(0,0,255));
	m_sessions.SetTextBkColor(RGB(247,247,255));
	
	// Add columns
	m_sessions.InsertColumn(0, "Session ID", LVCFMT_LEFT, 100);
	m_sessions.InsertColumn(1, "Created Time", LVCFMT_LEFT, 120);
	m_sessions.InsertColumn(2, "Last Update Time", LVCFMT_LEFT, 150);
	m_sessions.InsertColumn(3, "Lease Term", LVCFMT_LEFT, 120);

	// Add a few data
	m_sessions.InsertItem(0,"T123456789");
	m_sessions.SetItemText(0,1,"17:52");
	m_sessions.SetItemText(0,2,"18:32");
	m_sessions.SetItemText(0,3,"5000");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void MN_Sessions::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if(m_sessions.m_hWnd)
	{
		CRect rect;
		GetClientRect(&rect);
		rect.top+=10;
		rect.left+=10;
		rect.bottom-=10;
		rect.right-=10;
		m_sessions.MoveWindow(&rect);
	}
}

void MN_Sessions::OnColumnclickListSessions(NMHDR* pNMHDR, LRESULT* pResult) 
{
	for (int i = 0; i < m_sessions.GetItemCount(); ++i)
	{
		m_sessions.SetItemData(i, i); // Set the item number for sorting
	}
	
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	// Set increase or decrease sorting
	if( pNMListView->iSubItem ==m_sessions.m_nSortedCol )
		m_sessions.m_fAsc = !m_sessions.m_fAsc;
	else
	{
		m_sessions.m_fAsc = TRUE;
		m_sessions.m_nSortedCol = pNMListView->iSubItem;
	}
	// Call the sort function
	m_sessions.SortItems(CSortList::ListCompare, (LPARAM)&m_sessions); 
	
	*pResult = 0;
}

RpcValue& MN_Sessions::GenRpcValue()
{
	params.setStruct(INFOTYPE_KEY, RpcValue(INFOTYPE_SESSIONS));
	params.setStruct(INFOPARAM_KEY, RpcValue(NULL));
	return params;
}

void MN_Sessions::UpdateDlgList(RpcValue& _result)
{
	// Empty the list to prepare new information
	m_sessions.DeleteAllItems();
	// Insert new information
	char buf[1024];
	for (int i=0; i<_result.size(); i++)
	{
		m_sessions.InsertItem(i, (_result[i])[INFO_SESSIONID_KEY].ToString(buf,1024) );
		m_sessions.SetItemText(i, 1, (_result[i])[INFO_STARTTIME_KEY].ToString(buf,1024) );
		m_sessions.SetItemText(i, 2, (_result[i])[INFO_LASTUPDATE_KEY].ToString(buf,1024) );
		m_sessions.SetItemText(i, 3, itoa((int)(_result[i])[INFO_LEASETERM_KEY],buf,10) );
	}
}

void MN_Sessions::OnDblclkListSessions(NMHDR* pNMHDR, LRESULT* pResult) 
{
	POSITION pos = m_sessions.GetFirstSelectedItemPosition();
	int iItemPos = m_sessions.GetNextSelectedItem(pos);
	if (iItemPos < 0)
		return;

	CString strSessionID(m_sessions.GetItemText(iItemPos,0));

	CPopSessionDetail *pSessionWindow = new CPopSessionDetail(strSessionID);
	pSessionWindow->Create(IDD_DLG_SESSIONDETAIL, this);	
	pSessionWindow->ShowWindow(SW_SHOW);	
	
	*pResult = 0;
}
