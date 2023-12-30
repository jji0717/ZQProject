// MN_ManagedWorkIns.cpp : implementation file
//

#include "stdafx.h"
#include "TrayManageNode.h"
#include "MN_ManagedWorkIns.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int CALLBACK ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
/////////////////////////////////////////////////////////////////////////////
// MN_ManagedWorkIns dialog


MN_ManagedWorkIns::MN_ManagedWorkIns(CWnd* pParent /*=NULL*/)
	: CInfoDlg(MN_ManagedWorkIns::IDD, pParent)
{
	//{{AFX_DATA_INIT(MN_ManagedWorkIns)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void MN_ManagedWorkIns::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MN_ManagedWorkIns)
	DDX_Control(pDX, IDC_LIST_WORKINS, m_workIns);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MN_ManagedWorkIns, CDialog)
	//{{AFX_MSG_MAP(MN_ManagedWorkIns)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_WORKINS, OnColumnclickListWorkins)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MN_ManagedWorkIns message handlers

BOOL MN_ManagedWorkIns::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	// Set styles
	m_workIns.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_workIns.SetBkColor(RGB(247,247,255));
	m_workIns.SetTextColor(RGB(0,0,255));
	m_workIns.SetTextBkColor(RGB(247,247,255));
	
	// Add columns
	m_workIns.InsertColumn(0, "Type Name", LVCFMT_LEFT, 80);
	m_workIns.InsertColumn(1, "Task URL", LVCFMT_LEFT, 120);
	m_workIns.InsertColumn(2, "Session URL", LVCFMT_LEFT, 160);
	m_workIns.InsertColumn(3, "Start Time", LVCFMT_LEFT, 100);
	m_workIns.InsertColumn(4, "Last Update Time", LVCFMT_LEFT, 120);
	m_workIns.InsertColumn(5, "Status", LVCFMT_LEFT, 60);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void MN_ManagedWorkIns::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if(m_workIns.m_hWnd)
	{
		CRect rect;
		GetClientRect(&rect);
		rect.top+=10;
		rect.left+=10;
		rect.bottom-=10;
		rect.right-=10;
		m_workIns.MoveWindow(&rect);
	}
}

void MN_ManagedWorkIns::OnColumnclickListWorkins(NMHDR* pNMHDR, LRESULT* pResult) 
{
	for (int i = 0; i < m_workIns.GetItemCount(); ++i)
	{
		m_workIns.SetItemData(i, i); // Set the item number for sorting
	}
	
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	// Set increase or decrease sorting
	if( pNMListView->iSubItem ==m_workIns.m_nSortedCol )
		m_workIns.m_fAsc = !m_workIns.m_fAsc;
	else
	{
		m_workIns.m_fAsc = TRUE;
		m_workIns.m_nSortedCol = pNMListView->iSubItem;
	}
	// Call the sort function
	m_workIns.SortItems(ListCompare, (LPARAM)&m_workIns); 
	
	*pResult = 0;
}

int CALLBACK ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CSortList * pV=(CSortList *)lParamSort;
	// Get selected item data
	CString szComp1,szComp2;
	int iCompRes;
	szComp1=pV->GetItemText(lParam1,pV->m_nSortedCol);
	szComp2=pV->GetItemText(lParam2,pV->m_nSortedCol);

	iCompRes=szComp1.Compare(szComp2); // Only compare string
	// Sort with increase or decrease
	if(pV->m_fAsc)
		return iCompRes;
	else
		return -iCompRes;
}

RpcValue& MN_ManagedWorkIns::GenRpcValue()
{
	params.setStruct(INFOTYPE_KEY, RpcValue(INFOTYPE_TASK));
	params.setStruct(INFOPARAM_KEY, RpcValue(NULL));
	return params;
}

void MN_ManagedWorkIns::UpdateDlgList(RpcValue& _result)
{
	// Empty the list to prepare new information
	m_workIns.DeleteAllItems();
	// Insert new information
	char buf[1024];
	for (int i=0; i<_result.size(); i++)
	{
		m_workIns.InsertItem(i, (_result[i])[INFO_TYPENAME_KEY].ToString(buf,1024) );
		m_workIns.SetItemText(i, 1, (_result[i])[INFO_TASKURL_KEY].ToString(buf,1024) );
		m_workIns.SetItemText(i, 2, (_result[i])[INFO_SESSIONURL_KEY].ToString(buf,1024) );
		m_workIns.SetItemText(i, 3, (_result[i])[INFO_STARTTIME_KEY].ToString(buf,1024) );
		m_workIns.SetItemText(i, 4, (_result[i])[INFO_LASTUPDATE_KEY].ToString(buf,1024) );
		m_workIns.SetItemText(i, 5, (_result[i])[INFO_STATUS_KEY].ToString(buf,1024) );
	}
}