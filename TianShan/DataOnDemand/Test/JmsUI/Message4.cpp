// Message3.cpp : implementation file
//

#include "stdafx.h"
#include "JmsUI.h"
#include "Message4.h"
#include "sys\timeb.h"
#include "time.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMessage4 property page

IMPLEMENT_DYNCREATE(CMessage4, CPropertyPage)

CMessage4::CMessage4() : CPropertyPage(CMessage4::IDD), m_queue2Thd(NULL)
{
	//{{AFX_DATA_INIT(CMessage4)
	m_ExpiredTime = _T("");
	m_SendInterval = _T("");
	m_SendTimes = _T("");
	m_GroupID = _T("");
	//}}AFX_DATA_INIT
}

CMessage4::~CMessage4()
{
}

void CMessage4::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMessage4)
	DDX_Control(pDX, IDC_OPERATION_CODE, m_OperationCode);
	DDX_Control(pDX, IDC_DESTINATION_TYPE, m_DestinationType);
	DDX_Control(pDX, IDC_DESTINATION, m_Destination);
	DDX_Control(pDX, IDC_DATA_TYPE, m_DataType);
	DDX_Text(pDX, IDC_EXPIRED_TIME, m_ExpiredTime);
	DDX_Text(pDX, IDC_SEND_INTERVAL, m_SendInterval);
	DDX_Text(pDX, IDC_SEND_TIMES, m_SendTimes);
	DDX_Text(pDX, IDC_GROUP_ID, m_GroupID);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMessage4, CPropertyPage)
	//{{AFX_MSG_MAP(CMessage4)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_STOP_SEND, OnStopSend)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessage4 message handlers
BOOL CMessage4::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	init();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMessage4::OnOK() 
{
	UpdateData(TRUE);

	if (!validate())
	{
		return;
	}
	
	CString dataType, destination, destinationType, operationCode;
	m_DataType.GetWindowText(dataType);
	m_Destination.GetWindowText(destination);
	m_OperationCode.GetWindowText(operationCode);
	m_DestinationType.GetWindowText(destinationType);	
	
	int sendTimes = atoi(m_SendTimes);
	int sendInterval = atoi(m_SendInterval);
	
	m_queue2Thd->pushMessage4(sendTimes,
							  sendInterval,
							  m_GroupID,
							  dataType,
							  destination,
							  m_ExpiredTime,
							  operationCode,
							  destinationType);
}

bool CMessage4::validate()
{
	if (m_GroupID.IsEmpty())
	{
		AfxMessageBox("Group ID 不能为空");
		return false;
	}

	if (m_SendInterval.IsEmpty())
	{
		AfxMessageBox("Send Interval 不能为空");
		return false;
	}

	if (m_SendTimes.IsEmpty())
	{
		AfxMessageBox("Send Times 不能为空");
		return false;
	}

	if (m_ExpiredTime.IsEmpty())
	{
		AfxMessageBox("Expired Time 不能为空");
		return false;
	}
	
	int expiredTime = atoi(m_ExpiredTime);
	if (expiredTime < 0)
	{
		return false;
	}

	if (m_DataType.GetCurSel() == -1)
	{
		AfxMessageBox("Data Type 不能为空");
		return false;
	}

	if (m_DestinationType.GetCurSel() == -1)
	{
		AfxMessageBox("Destiantion Type 不能为空");
		return false;
	}

	if (m_OperationCode.GetCurSel() == -1)
	{
		AfxMessageBox("OperationCode 不能为空");
		return false;
	}

	if (m_Destination.GetCurSel() == -1)
	{
		AfxMessageBox("Destiantion 不能为空");
		return false;
	}

	return true;
}

void CMessage4::init()
{
	m_DataType.ResetContent();
	for (int i = 0; i < 20; i++)
	{
		char temp[3];
		sprintf(temp, "%d", i + 11);
		m_DataType.AddString(temp);
	}

	m_DestinationType.ResetContent();
	m_DestinationType.InsertString(0, "home");
	m_DestinationType.InsertString(1, "group");
	m_DestinationType.InsertString(2, "device");

	m_OperationCode.ResetContent();
	m_OperationCode.InsertString(0, "1");
	m_OperationCode.InsertString(0, "2");

	m_Destination.ResetContent();
	m_Destination.InsertString(0, "000000");
	m_Destination.InsertString(1, "111111");

	SetTimer(1, 500, 0);

	setButton(false);
}


void CMessage4::OnTimer(UINT nIDEvent) 
{
	setButton(g_Queue2_Busy);
	CPropertyPage::OnTimer(nIDEvent);
}

void CMessage4::setButton(bool bQueue2Busy)
{
	CWnd* p;
	p = (CWnd*)GetDlgItem(IDOK);
	p->EnableWindow(!bQueue2Busy);
	p = (CWnd*)GetDlgItem(IDC_STOP_SEND);
	p->EnableWindow(bQueue2Busy);
}

void CMessage4::OnStopSend() 
{
	m_queue2Thd->stopSend();
}