// NodeConfigure.cpp : implementation file
//

#include "stdafx.h"
#include "SysTray.h"
#include "SysTrayDlg.h"
#include "NodeConfigure.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNodeConfigure dialog
CString  CNodeConfigure::gHost        = "192.168.80.140";
int      CNodeConfigure::gPort        = 12000;
DWORD    CNodeConfigure::gTimeInteval = 30000;

CNodeConfigure::CNodeConfigure(CWnd* pParent /*=NULL*/)
	: CInfoDlg(CNodeConfigure::IDD, pParent),
	_mStop(::CreateEvent(NULL, FALSE, FALSE, NULL) ),
	m_nNodeType(0)
{
	//{{AFX_DATA_INIT(CNodeConfigure)
	m_host = _T("");
	m_time = _T("");
	m_port = _T("");
	//}}AFX_DATA_INIT
}

CNodeConfigure::~CNodeConfigure()
{
	::CloseHandle(_mStop);
}

void CNodeConfigure::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNodeConfigure)
	DDX_Text(pDX, IDC_EDIT_HOST, m_host);
	DDX_Text(pDX, IDC_EDIT_TIMEINTERVAL, m_time);
	DDX_Text(pDX, IDC_EDIT_PORT, m_port);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNodeConfigure, CDialog)
	//{{AFX_MSG_MAP(CNodeConfigure)
	ON_BN_CLICKED(IDC_BUT_RESET, OnButReset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNodeConfigure message handlers

RpcValue& CNodeConfigure::GenRpcValue()
{
	params.setStruct(INFOTYPE_KEY, RpcValue(INFOTYPE_NODETYPE));
	params.setStruct(INFOPARAM_KEY, RpcValue(NULL));
	return params;
}

void CNodeConfigure::UpdateDlgList(RpcValue& result)
{
 	switch ((int)result)
	{
		case NODE_TYPE_MANAGERNODE:
		{
			SetNodeType(NODE_TYPE_MANAGERNODE);
			::SetEvent(_mStop);
		}
		break;
		case NODE_TYPE_WORKNODE:
		{
			SetNodeType(NODE_TYPE_WORKNODE);
			::SetEvent(_mStop);
		}
		break;
		case NODE_TYPE_MIXED:
		{
			SetNodeType(NODE_TYPE_MIXED);
			::SetEvent(_mStop);
		}
		break;
		case NODE_TYPE_UNKNOWN:
			::SetEvent(_mStop);
		break;
		default:
			break;
	}
}

void CNodeConfigure::OnOK() 
{
	UpdateData();
	
	// AutoStop==true, should not delete thread pointer
	InfoQueryThread *pInfoTemp = new InfoQueryThread(m_host, atoi(m_port),true);
	pInfoTemp->AddUpdateDlg(this);
	pInfoTemp->start();

	if(WAIT_OBJECT_0 == ::WaitForSingleObject(_mStop, 3000)) // Wait within 3s
	{
		CDialog::OnOK();
	}
		
	else // Time out
	{
		AfxMessageBox("Can not access your destination, please check and reset.");
	}
		
}

void CNodeConfigure::OnButReset() 
{
	// TODO: Add your control notification handler code here
	m_host.Empty();
	m_port.Empty();
	m_time.Empty();
	UpdateData(false);
}

void CNodeConfigure::SetUerConfig(const CString strHost, const int nPort, 
								  const DWORD unTime)
{
	m_host = strHost;
	char buff[32];
	if (nPort>0)
		m_port = itoa(nPort,buff,10);
	if (unTime>0)
		m_time = itoa(unTime/1000,buff,10);
}

int CNodeConfigure::GetNodeType() const
{
	return m_nNodeType;
}

void CNodeConfigure::SetNodeType(int _type)
{
	m_nNodeType = _type;
}

CString CNodeConfigure::GetHost() const
{
	return m_host;
}

int CNodeConfigure::GetPort() const
{
	return atoi(m_port);
}

DWORD CNodeConfigure::GetTimeInteval() const
{
	return atoi(m_time)*1000;
}
