// PerWorkInsPopup.cpp : implementation file
//

#include "stdafx.h"
#include "TrayWorkNode.h"
#include "PerWorkInsPopup.h"
#include <time.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// PerWorkInsPopup dialog


PerWorkInsPopup::PerWorkInsPopup(const char* _strTaskID, CWnd* pParent /*=NULL*/)
	: CInfoDlg(PerWorkInsPopup::IDD)
{
	//{{AFX_DATA_INIT(PerWorkInsPopup)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	params.setStruct(INFOTYPE_KEY, RpcValue(INFOTYPE_TASKDETAIL));
	params.setStruct(INFOPARAM_KEY, RpcValue(_strTaskID));
}


void PerWorkInsPopup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PerWorkInsPopup)
	DDX_Control(pDX, IDC_TREE_RPCVAL, m_treeRpc);
	DDX_Control(pDX, IDC_LIST_PERWORKINS, m_perIns);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(PerWorkInsPopup, CDialog)
	//{{AFX_MSG_MAP(PerWorkInsPopup)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_RPCVAL, OnSelchangedTreeRpcval)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
//ON_NOTIFY(NM_CLICK, IDC_TREE_RPCVAL, OnClickTreeRpcval)
/////////////////////////////////////////////////////////////////////////////
// PerWorkInsPopup message handlers

BOOL PerWorkInsPopup::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Display the first Layer info keys in treeList
	HTREEITEM root = m_treeRpc.InsertItem("_Root_");
	m_perIns.InsertColumn(0, "Index", LVCFMT_LEFT, 200);
	m_perIns.InsertColumn(1, "Value", LVCFMT_LEFT, 300);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

RpcValue& PerWorkInsPopup::GenRpcValue() 
{
	return params;
}

void PerWorkInsPopup::UpdateDlgList(RpcValue& _result)
{
	result = _result;
}

void PerWorkInsPopup::PostNcDestroy() 
{
	// Quit and destroy object in the heap
	CDialog::PostNcDestroy();
	delete this;
}

void PerWorkInsPopup::OnSelchangedTreeRpcval(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(!result.valid())
	{
		m_treeRpc.SelectItem(NULL);
		*pResult = 0;
		return;
	}
	HTREEITEM currentItem = m_treeRpc.GetSelectedItem();

	if ( m_treeRpc.GetItemText(currentItem).Compare("_Root_") == 0 ) 
		if (m_treeRpc.GetNextItem(currentItem,TVGN_CHILD) != NULL)
			return;
		else
		{
			char strKey[256] = {0};
			if (!result.listStruct(strKey, 256, true))
				return;
			do
			{
				m_treeRpc.InsertItem(strKey,currentItem);
				memset(strKey, 0, 256);
			} while ( result.listStruct(strKey, 256) );
			
			UpdateData(false);
			return;
		}

	// Remember the path from root to currentItem
	CString* strPath = new CString[10];
	int nPath = 0;
	strPath[nPath++] = m_treeRpc.GetItemText(currentItem);
	HTREEITEM tempItem = m_treeRpc.GetParentItem(currentItem);
	while ( m_treeRpc.GetItemText(tempItem).Compare("_Root_") )
	{
		strPath[nPath++] = m_treeRpc.GetItemText(tempItem);
		tempItem = m_treeRpc.GetParentItem(tempItem);
	}

	// Get current RpcValue according to path
	RpcValue tempValue(result);
	while(nPath > 0)
	{
		tempValue = tempValue[ strPath[--nPath] ];
	}

	delete [] strPath;
	// Display all subItems and values according to its key
	switch (tempValue.getType()) 
	{
	case RpcValue::TypeStruct: // Struct, expend subItems but not display value
		{
			if (m_treeRpc.GetNextItem(currentItem,TVGN_CHILD) != NULL)
				return;
			else
			{
				char strKey[256] = {0};
				if (!tempValue.listStruct(strKey, 256, true))
					return;
				do
				{
					m_treeRpc.InsertItem(strKey,currentItem);
					memset(strKey, 0, 256);
				} while ( tempValue.listStruct(strKey, 256) );
			}
		}
		break;
	case RpcValue::TypeArray:
		{
			char buf[256] = {0};
			m_perIns.DeleteAllItems();
			for(int i=0; i<tempValue.size(); i++)
			{
				m_perIns.InsertItem( i, itoa(i,buf,10) );
				m_perIns.SetItemText(i,1,tempValue[i].toXml(buf,256));
			}
		}
		break;
	case RpcValue::TypeDateTime:
		{
			char buf[12] = {0}; // a little more than a int number(10 digits)
			m_perIns.DeleteAllItems();
			struct tm *pTime = &static_cast<struct tm>(tempValue);
			m_perIns.InsertItem(0,"Year");
			m_perIns.SetItemText(0,1,itoa((pTime->tm_year),buf,10));
			m_perIns.InsertItem(1,"Month");
			m_perIns.SetItemText(1,1,itoa((pTime->tm_mon),buf,10));
			m_perIns.InsertItem(2,"Day");
			m_perIns.SetItemText(2,1,itoa((pTime->tm_mday),buf,10));
			m_perIns.InsertItem(3,"Hour");
			m_perIns.SetItemText(3,1,itoa((pTime->tm_hour),buf,10));
			m_perIns.InsertItem(4,"Minite");
			m_perIns.SetItemText(4,1,itoa((pTime->tm_min),buf,10));
			m_perIns.InsertItem(5,"Second");
			m_perIns.SetItemText(5,1,itoa((pTime->tm_sec),buf,10));
		}
		break;
	case RpcValue::TypeInt:
		{
			CString strItem(static_cast<int>(tempValue));
			m_perIns.DeleteAllItems();
			m_perIns.InsertItem(0,"Int Type");
			m_perIns.SetItemText(0, 1, strItem);
		}
		break;
	case RpcValue::TypeDouble:
		{
			CString strItem(static_cast<double>(tempValue));
			m_perIns.DeleteAllItems();
			m_perIns.InsertItem(0,"Double Type");
			m_perIns.SetItemText(0, 1, strItem);
		}
		break;
	case RpcValue::TypeBoolean:
		{
			m_perIns.DeleteAllItems();
			m_perIns.InsertItem(0,"Bool Type");
			if (static_cast<bool>(tempValue))
				m_perIns.SetItemText(0,1,"true");
			else
				m_perIns.SetItemText(0,1,"false");
		}
		break;
	case RpcValue::TypeString:
		{
			char buf[256] = {0};
			m_perIns.DeleteAllItems();
			m_perIns.InsertItem(0,"String Type");
			m_perIns.SetItemText(0,1,tempValue.toString(buf,256));
		}
		break;
	// Type can't handle, so recognize it as "Unknown", but trate the value to xml
	default:  
		{
			char buf[256] = {0};
			m_perIns.DeleteAllItems();
			m_perIns.InsertItem(0,"Unknown Type");
			m_perIns.SetItemText(0,1,tempValue.toXml(buf,256));
		}
		break;
	} // end of switch()

	UpdateData(false);
	
	*pResult = 0;
}
