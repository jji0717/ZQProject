// WN_WorkInstance.cpp : implementation file
//

#include "stdafx.h"
#include "SysTray.h"
//#include "SysTrayDlg.h"
#include "WN_WorkInstance.h"
#include <MPFCommon.h>
#include "PerWorkInsPopup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// WN_WorkInstance dialog


WN_WorkInstance::WN_WorkInstance(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(WN_WorkInstance)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void WN_WorkInstance::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(WN_WorkInstance)
	DDX_Control(pDX, IDC_LIST_WORKINSTANCE, m_instance);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(WN_WorkInstance, CDialog)
	//{{AFX_MSG_MAP(WN_WorkInstance)
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_WORKINSTANCE, OnDblclkListWorkinstance)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_WORKINSTANCE, OnColumnclickListWorkinstance)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// WN_WorkInstance message handlers

BOOL WN_WorkInstance::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_instance.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_instance.SetBkColor(RGB(247,247,255));
	m_instance.SetTextColor(RGB(0,0,255));
	m_instance.SetTextBkColor(RGB(247,247,255));
	
	// Add columns
	m_instance.InsertColumn(0, "Type Name", LVCFMT_LEFT, 80);
	m_instance.InsertColumn(1, "Task URL", LVCFMT_LEFT, 120);
	m_instance.InsertColumn(2, "Session URL", LVCFMT_LEFT, 160);
	m_instance.InsertColumn(3, "Start Time", LVCFMT_LEFT, 100);
	m_instance.InsertColumn(4, "Last Update Time", LVCFMT_LEFT, 120);
	m_instance.InsertColumn(5, "Status", LVCFMT_LEFT, 60);

	// Test insert data
	m_instance.InsertItem(0, "T123456d");
	m_instance.SetItemText(0, 1, "MPF:\\192.168.0.140:12000?id=135454623");
	m_instance.SetItemText(0, 2, "MPF:\\192.168.0.140:12000?id=135454623");
	m_instance.SetItemText(0, 3, "17:59");
	m_instance.SetItemText(0, 4, "15:59");
	m_instance.SetItemText(0, 5, "start");
	
	m_instance.InsertItem(1, "B223456d");
	m_instance.SetItemText(1, 1, "MPF:\\292.168.0.140:12000?id=135454623");
	m_instance.SetItemText(1, 2, "MPF:\\292.168.0.140:12000?id=135454623");
	m_instance.SetItemText(1, 3, "18:59");
	m_instance.SetItemText(1, 4, "19:59");
	m_instance.SetItemText(1, 5, "initial");

	m_instance.InsertItem(2, "T323456d");
	m_instance.SetItemText(2, 1, "MPF:\\092.168.0.140:12000?id=135454623");
	m_instance.SetItemText(2, 2, "MPF:\\092.168.0.140:12000?id=135454623");
	m_instance.SetItemText(2, 3, "13:59");
	m_instance.SetItemText(2, 4, "21:59");
	m_instance.SetItemText(2, 5, "complete");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void WN_WorkInstance::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if(m_instance.m_hWnd)
	{
		CRect rect;
		GetClientRect(&rect);
		rect.top+=10;
		rect.left+=10;
		rect.bottom-=10;
		rect.right-=10;
		m_instance.MoveWindow(&rect);
	}
}

void WN_WorkInstance::OnDblclkListWorkinstance(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Get the selected item number
	POSITION pos = m_instance.GetFirstSelectedItemPosition();
	int iItemPos = m_instance.GetNextSelectedItem(pos);
	if (iItemPos < 0)
		return;
	
	char taskID[128] = {0};
	CString strTemp(m_instance.GetItemText(iItemPos,1));
	ZQ::MPF::utils::URLStr strUrl(strTemp.GetBuffer(256));
	strncpy(taskID, strUrl.getVar("tid"), 128);

	PerWorkInsPopup *p = new PerWorkInsPopup(taskID);
	p->Create(IDD_POPUP_PERWORKINS, this);
	p->ShowWindow(SW_SHOW);	
	
	*pResult = 0;
}

void WN_WorkInstance::OnColumnclickListWorkinstance(NMHDR* pNMHDR, LRESULT* pResult) 
{
	for (int i = 0; i < m_instance.GetItemCount(); ++i)
	{
		m_instance.SetItemData(i, i); // Set the item number for sorting
	}
	
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	// Set increase or decrease sorting
	if( pNMListView->iSubItem ==m_instance.m_nSortedCol )
		m_instance.m_fAsc = !m_instance.m_fAsc;
	else
	{
		m_instance.m_fAsc = TRUE;
		m_instance.m_nSortedCol = pNMListView->iSubItem;
	}
	// Call the sort function
	m_instance.SortItems(CSortList::ListCompare, (LPARAM)&m_instance); 
	
	*pResult = 0;
}

RpcValue& WN_WorkInstance::GenRpcValue() 
{
	params.setStruct(INFOTYPE_KEY, RpcValue(INFOTYPE_TASK));
	params.setStruct(INFOPARAM_KEY, RpcValue(NULL));
	return params;
}

void WN_WorkInstance::UpdateDlgList(RpcValue& _result)
{
	// Empty the list to prepare new infomation
	m_instance.DeleteAllItems();
	// Insert new information
	char buf[1024];
	for (int i=0; i<_result.size(); i++)
	{
		m_instance.InsertItem(i, (_result[i])[INFO_TYPENAME_KEY].ToString(buf,1024) );
		m_instance.SetItemText(i, 1, (_result[i])[INFO_TASKURL_KEY].ToString(buf,1024) );
		m_instance.SetItemText(i, 2, (_result[i])[INFO_SESSIONURL_KEY].ToString(buf,1024) );
		m_instance.SetItemText(i, 3, (_result[i])[INFO_STARTTIME_KEY].ToString(buf,1024) );
		m_instance.SetItemText(i, 4, (_result[i])[INFO_LASTUPDATE_KEY].ToString(buf,1024) );
		m_instance.SetItemText(i, 5, (_result[i])[INFO_STATUS_KEY].ToString(buf,1024) );
	}

	//this->UpdateData(false);
}