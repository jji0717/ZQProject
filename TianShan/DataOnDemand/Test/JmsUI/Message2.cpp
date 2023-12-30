// Message2.cpp : implementation file
//

#include "stdafx.h"
#include "JmsUI.h"
#include "Message2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMessage2 property page

IMPLEMENT_DYNCREATE(CMessage2, CPropertyPage)

CMessage2::CMessage2() : CPropertyPage(CMessage2::IDD), m_queue1Thd(NULL)
{
	//{{AFX_DATA_INIT(CMessage2)
	m_ExpiredTime = _T("");
	m_SendInterval = _T("");
	m_SendTimes = _T("");
	m_GroupID = _T("");
	m_MessageID = _T("");
	//}}AFX_DATA_INIT
}

CMessage2::~CMessage2()
{
}

void CMessage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMessage2)
	DDX_Control(pDX, IDC_DESTINATION_TYPE, m_DestinationType);
	DDX_Control(pDX, IDC_DESTINATION, m_Destination);
	DDX_Control(pDX, IDC_DATA_TYPE, m_DataType);
	DDX_Text(pDX, IDC_EXPIRED_TIME, m_ExpiredTime);
	DDX_Text(pDX, IDC_SEND_INTERVAL, m_SendInterval);
	DDX_Text(pDX, IDC_SEND_TIMES, m_SendTimes);
	DDX_Text(pDX, IDC_GROUP_ID, m_GroupID);
	DDX_Text(pDX, IDC_MESSAGE_ID, m_MessageID);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMessage2, CPropertyPage)
	//{{AFX_MSG_MAP(CMessage2)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_STOP_SEND, OnStopSend)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessage2 message handlers

BOOL CMessage2::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	init();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMessage2::OnOK() 
{
	UpdateData(TRUE);

	if (!validate())
	{
		return;
	}

	CString dataType, destination, destinationType;
	m_DataType.GetWindowText(dataType);
	m_Destination.GetWindowText(destination);
	m_DestinationType.GetWindowText(destinationType);
	
	int sendTimes = atoi(m_SendTimes);
	int sendInterval = atoi(m_SendInterval);
	
	if (m_MessageID.IsEmpty())
	{
		m_queue1Thd->pushMessage2(sendTimes, 
								  sendInterval,
								  m_GroupID,
								  dataType,
								  destination,
								  destinationType,
								  m_ExpiredTime);
	}
	else
	{
		m_queue1Thd->pushMessage2(sendTimes, 
								  sendInterval,
								  m_GroupID,
								  dataType,
								  destination,
								  destinationType,
								  m_ExpiredTime,
								  m_MessageID);
	}
}

bool CMessage2::validate()
{
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

	if (m_DestinationType.GetCurSel() == -1)
	{
		AfxMessageBox("Destiantion Type 不能为空");
		return false;
	}

	if (m_Destination.GetCurSel() == -1)
	{
		AfxMessageBox("Destiantion 不能为空");
		return false;
	}
	
	return true;
}

void CMessage2::init()
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

	m_Destination.ResetContent();
	m_Destination.InsertString(0, "000000");
	m_Destination.InsertString(1, "111111");

	SetTimer(1, 500, 0);
	
	setButton(false);
}

void CMessage2::OnTimer(UINT nIDEvent) 
{
	setButton(g_Queue1_Busy);
	CPropertyPage::OnTimer(nIDEvent);
}

void CMessage2::setButton(bool bQueue1Busy)
{
	CWnd* p;
	p = (CWnd*)GetDlgItem(IDOK);
	p->EnableWindow(!bQueue1Busy);
	p = (CWnd*)GetDlgItem(IDC_STOP_SEND);
	p->EnableWindow(bQueue1Busy);
}

void CMessage2::OnStopSend() 
{
	m_queue1Thd->stopSend();
}
