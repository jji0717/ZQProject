// WN_TaskType.cpp : implementation file
//

#include "stdafx.h"
#include "TrayWorkNode.h"
#include "WN_TaskType.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int CALLBACK ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

/////////////////////////////////////////////////////////////////////////////
// WN_TaskType dialog

WN_TaskType::WN_TaskType(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(WN_TaskType)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void WN_TaskType::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(WN_TaskType)
	DDX_Control(pDX, IDC_LIST_TASKTYPE, m_taskType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(WN_TaskType, CDialog)
	//{{AFX_MSG_MAP(WN_TaskType)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_TASKTYPE, OnColumnclickListTasktype)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// WN_TaskType message handlers

BOOL WN_TaskType::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	// Set styles
	m_taskType.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_taskType.SetBkColor(RGB(247,247,255));
//	m_taskType.SetTextColor(RGB(0,0,255));
	m_taskType.SetTextBkColor(RGB(247,247,255));
	
	// Add columns
	m_taskType.InsertColumn(0, "Type Name", LVCFMT_LEFT, 120);
	m_taskType.InsertColumn(1, "Instances", LVCFMT_LEFT, 80);
	m_taskType.InsertColumn(2, "Available", LVCFMT_LEFT, 80);
	m_taskType.InsertColumn(3, "Plugin", LVCFMT_LEFT, 120);
	m_taskType.InsertColumn(4, "Version", LVCFMT_LEFT, 120);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void WN_TaskType::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if(m_taskType.m_hWnd)
	{
		CRect rect;
		GetClientRect(&rect);
		rect.top+=10;
		rect.left+=10;
		rect.bottom-=10;
		rect.right-=10;
		m_taskType.MoveWindow(&rect);
	}
}

void WN_TaskType::OnColumnclickListTasktype(NMHDR* pNMHDR, LRESULT* pResult) 
{
	for (int i = 0; i < m_taskType.GetItemCount(); ++i)
	{
		m_taskType.SetItemData(i, i); // Set the item number for sorting
	}
	
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	// Set increase or decrease sorting
	if( pNMListView->iSubItem ==m_taskType.m_nSortedCol )
		m_taskType.m_fAsc = !m_taskType.m_fAsc;
	else
	{
		m_taskType.m_fAsc = TRUE;
		m_taskType.m_nSortedCol = pNMListView->iSubItem;
	}
	// Call the sort function
	m_taskType.SortItems(ListCompare, (LPARAM)&m_taskType); 
	
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

	iCompRes=szComp1.Compare(szComp2); // Compare string only! 
    // Sort with increase or decrease
	if(pV->m_fAsc)
		return iCompRes;
	else
		return -iCompRes;
}

RpcValue& WN_TaskType::GenRpcValue() 
{
	params.setStruct(INFOTYPE_KEY, RpcValue(INFOTYPE_TASK_TYPE));
	params.setStruct(INFOPARAM_KEY, RpcValue(NULL));
	return params;
}

void WN_TaskType::UpdateDlgList(RpcValue& _result)
{
	// Empty the list to prepare new infomation
	m_taskType.DeleteAllItems();
	// Insert new information
	char buf[1024];
	for (int i=0; i<_result.size(); i++)
	{
		m_taskType.InsertItem(i, (_result[i])[INFO_TYPENAME_KEY].ToString(buf,1024) );

		char buff[15];
		m_taskType.SetItemText(i, 1, itoa((int)(_result[i])[INFO_INSTANCES_KEY],buff,10) );
		m_taskType.SetItemText(i, 2, itoa((int)(_result[i])[INFO_AVAILABLE_KEY],buff,10) );

		m_taskType.SetItemText(i, 3, (_result[i])[INFO_PLUGIN_KEY].ToString(buf,1024) );
		m_taskType.SetItemText(i, 4, (_result[i])[INFO_VENDOR_KEY].ToString(buf,1024) );
	}

	//this->UpdateData(false);
}
