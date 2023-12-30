// MN_WorkNodes.cpp : implementation file
//

#include "stdafx.h"
#include "SysTray.h"
#include "MN_WorkNodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MN_WorkNodes dialog


MN_WorkNodes::MN_WorkNodes(CWnd* pParent /*=NULL*/)
	: CInfoDlg(MN_WorkNodes::IDD, pParent)
{
	//{{AFX_DATA_INIT(MN_WorkNodes)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void MN_WorkNodes::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MN_WorkNodes)
	DDX_Control(pDX, IDC_LIST_WORKNODE, m_worknode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MN_WorkNodes, CDialog)
	//{{AFX_MSG_MAP(MN_WorkNodes)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_WORKNODE, OnColumnclickListWorknode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MN_WorkNodes message handlers

BOOL MN_WorkNodes::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	// Set styles
	m_worknode.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_worknode.SetBkColor(RGB(247,247,255));
//	m_worknode.SetTextColor(RGB(0,0,255));
	m_worknode.SetTextBkColor(RGB(247,247,255));
	
	// Add columns
	m_worknode.InsertColumn(0, "Node ID", LVCFMT_LEFT, 100);
	m_worknode.InsertColumn(1, "IP", LVCFMT_LEFT, 100);
	m_worknode.InsertColumn(2, "Port", LVCFMT_LEFT, 60);
	m_worknode.InsertColumn(3, "MPF Version", LVCFMT_LEFT, 150);
	m_worknode.InsertColumn(4, "Last Heartbeat", LVCFMT_LEFT, 100);
	m_worknode.InsertColumn(5, "Lease Term", LVCFMT_LEFT, 100);
	m_worknode.InsertColumn(6, "Hardware", LVCFMT_LEFT, 105);
	
	// Test insertItem
	m_worknode.InsertItem(0,"T12455d");
	m_worknode.SetItemText(0,1,"10.7.0.23");
	m_worknode.SetItemText(0,2,"12000");
	m_worknode.SetItemText(0,3,"0.2build105");
	m_worknode.SetItemText(0,4,"18:23");
	m_worknode.SetItemText(0,5,"30m");
	m_worknode.SetItemText(0,6,"P42.0G/512M");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void MN_WorkNodes::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if(m_worknode.m_hWnd)
	{
		CRect rect;
		GetClientRect(&rect);
		rect.top+=10;
		rect.left+=10;
		rect.bottom-=10;
		rect.right-=10;
		m_worknode.MoveWindow(&rect);
	}
}

void MN_WorkNodes::OnColumnclickListWorknode(NMHDR* pNMHDR, LRESULT* pResult) 
{
	for (int i = 0; i < m_worknode.GetItemCount(); ++i)
	{
		m_worknode.SetItemData(i, i); // Set the item number for sorting
	}
	
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	// Set increase or decrease sorting
	if( pNMListView->iSubItem ==m_worknode.m_nSortedCol )
		m_worknode.m_fAsc = !m_worknode.m_fAsc;
	else
	{
		m_worknode.m_fAsc = TRUE;
		m_worknode.m_nSortedCol = pNMListView->iSubItem;
	}
	// Call the sort function
	m_worknode.SortItems(CSortList::ListCompare, (LPARAM)&m_worknode); 
	
	*pResult = 0;
}

RpcValue& MN_WorkNodes::GenRpcValue()
{
	params.setStruct(INFOTYPE_KEY, RpcValue(INFOTYPE_WORKNODE));
	params.setStruct(INFOPARAM_KEY, RpcValue(NULL));
	return params;
}

void MN_WorkNodes::UpdateDlgList(RpcValue& _result)
{
	// Empty the list to prepare new information
	m_worknode.DeleteAllItems();
	// Insert new information
	char buf[1024];

	for (int i=0; i<_result.size(); i++)
	{
		(_result[i])[INFO_NODEID_KEY].ToString(buf,1024);
		m_worknode.InsertItem(i, buf );
		(_result[i])[INFO_IP_KEY].ToString(buf,1024);
		m_worknode.SetItemText(i, 1,  buf);
		itoa((int)(_result[i])[INFO_PORT_KEY],buf,10);
		m_worknode.SetItemText(i, 2,  buf);
		(_result[i])[INFO_MPFVERSION_KEY].ToString(buf,1024);
		m_worknode.SetItemText(i, 3,  buf);
		(_result[i])[INFO_LASTHB_KEY].ToString(buf,1024);
		m_worknode.SetItemText(i, 4,  buf);
		itoa((int)(_result[i])[INFO_LEASETERM_KEY],buf,10);
		m_worknode.SetItemText(i, 5,  buf);

		(_result[i])[INFO_HARDWARE_KEY].ToString(buf,1024);
		CString strHardware(buf);
		
		removeCR(strHardware);
		m_worknode.SetItemText(i, 6,  strHardware);
	}
}

void MN_WorkNodes::removeCR(CString &strObject)
{
	int pos = strObject.Find('\n');
	while (pos > -1)
	{
		strObject.Delete(pos,1);
		pos = strObject.Find('\n');
	}
}
