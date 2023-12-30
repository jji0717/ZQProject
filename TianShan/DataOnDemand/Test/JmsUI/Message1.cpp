// Message1.cpp : implementation file
//

#include "stdafx.h"
#include "JmsUI.h"
#include "Message1.h"
#include "sys\timeb.h"
#include "time.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMessage1 property page

IMPLEMENT_DYNCREATE(CMessage1, CPropertyPage)

CMessage1::CMessage1() : CPropertyPage(CMessage1::IDD), m_queue1Thd(NULL)
{
	//{{AFX_DATA_INIT(CMessage1)
	m_SendInterval = _T("");
	m_SendTimes = _T("");
	m_RootPath = _T("");
	m_GroupID = _T("");
	//}}AFX_DATA_INIT
}

CMessage1::~CMessage1()
{
}

void CMessage1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMessage1)
	DDX_Control(pDX, IDC_UPDATE_MODE, m_UpdateMode);
	DDX_Control(pDX, IDC_DATA_TYPE, m_DataType);
	DDX_Text(pDX, IDC_SEND_INTERVAL, m_SendInterval);
	DDX_Text(pDX, IDC_SEND_TIMES, m_SendTimes);
	DDX_Text(pDX, IDC_ROOT_PATH, m_RootPath);
	DDX_Text(pDX, IDC_GROUP_ID, m_GroupID);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMessage1, CPropertyPage)
	//{{AFX_MSG_MAP(CMessage1)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_STOP_SEND, OnStopSend)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessage1 message handlers
BOOL CMessage1::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	init();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMessage1::OnOK() 
{
	UpdateData(TRUE);

	if (!validate())
	{
		return;
	}

	CString dataType, updateMode;
	m_DataType.GetWindowText(dataType);
	m_UpdateMode.GetWindowText(updateMode);
	int sendTimes = atoi(m_SendTimes);
	int sendInterval = atoi(m_SendInterval);

	m_queue1Thd->pushMessage1(sendTimes,
							  sendInterval,
							  m_GroupID,
							  dataType,
							  updateMode,
							  m_RootPath);
}

bool CMessage1::validate()
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

	if (m_RootPath.IsEmpty())
	{
		AfxMessageBox("Root Path 不能为空");
		return false;
	}

	if (m_DataType.GetCurSel() == -1)
	{
		AfxMessageBox("Data Type 不能为空");
		return false;
	}

	if (m_UpdateMode.GetCurSel() == -1)
	{
		AfxMessageBox("Update Mode 不能为空");
		return false;
	}
	
	return true;
}

void CMessage1::init()
{
	m_DataType.ResetContent();
	for (int i = 0; i < 20; i++)
	{
		char temp[3];
		sprintf(temp, "%d", i + 11);
		m_DataType.AddString(temp);
	}

	m_UpdateMode.ResetContent();
	for (i = 0; i <=6; i++)
	{
		char temp[2];
		sprintf(temp, "%d", i);
		m_UpdateMode.AddString(temp);
	}

	SetTimer(1, 500, 0);

	setButton(false);
}

void CMessage1::OnTimer(UINT nIDEvent) 
{
	setButton(g_Queue1_Busy);
	CPropertyPage::OnTimer(nIDEvent);
}

void CMessage1::setButton(bool bQueue1Busy)
{
	CWnd* p;
	p = (CWnd*)GetDlgItem(IDOK);
	p->EnableWindow(!bQueue1Busy);
	p = (CWnd*)GetDlgItem(IDC_STOP_SEND);
	p->EnableWindow(bQueue1Busy);
}

void CMessage1::OnStopSend() 
{
	m_queue1Thd->stopSend();
}
