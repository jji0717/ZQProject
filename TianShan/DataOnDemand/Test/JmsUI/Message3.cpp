// Message3.cpp : implementation file
//

#include "stdafx.h"
#include "JmsUI.h"
#include "Message3.h"
#include "sys\timeb.h"
#include "time.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMessage3 property page

IMPLEMENT_DYNCREATE(CMessage3, CPropertyPage)

CMessage3::CMessage3() : CPropertyPage(CMessage3::IDD), m_queue2Thd(NULL)
{
	//{{AFX_DATA_INIT(CMessage3)
	m_FilePath = _T("");
	m_RootPath = _T("");
	m_GroupID = _T("");
	m_SendInterval = _T("");
	m_SendTimes = _T("");
	//}}AFX_DATA_INIT
}

CMessage3::~CMessage3()
{
}

void CMessage3::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMessage3)
	DDX_Control(pDX, IDC_UPDATE_MODE, m_UpdateMode);
	DDX_Control(pDX, IDC_DATA_TYPE, m_DataType);
	DDX_Text(pDX, IDC_FILE_PATH, m_FilePath);
	DDX_Text(pDX, IDC_ROOT_PATH, m_RootPath);
	DDX_Text(pDX, IDC_GROUP_ID, m_GroupID);
	DDX_Text(pDX, IDC_SEND_INTERVAL, m_SendInterval);
	DDX_Text(pDX, IDC_SEND_TIMES, m_SendTimes);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMessage3, CPropertyPage)
	//{{AFX_MSG_MAP(CMessage3)
	ON_BN_CLICKED(IDC_STOP_SEND, OnStopSend)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessage3 message handlers

BOOL CMessage3::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	init();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CMessage3::OnOK() 
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

	m_queue2Thd->pushMessage3(sendTimes,
							  sendInterval,
							  m_GroupID,
							  dataType,
							  updateMode,
							  m_FilePath,
							  m_RootPath);
}

bool CMessage3::validate()
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

	if (m_FilePath.IsEmpty())
	{
		AfxMessageBox("File Path 不能为空");
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

void CMessage3::init()
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

void CMessage3::OnTimer(UINT nIDEvent) 
{
	setButton(g_Queue2_Busy);
	CPropertyPage::OnTimer(nIDEvent);
}

void CMessage3::setButton(bool bQueue2Busy)
{
	CWnd* p;
	p = (CWnd*)GetDlgItem(IDOK);
	p->EnableWindow(!bQueue2Busy);
	p = (CWnd*)GetDlgItem(IDC_STOP_SEND);
	p->EnableWindow(bQueue2Busy);
}

void CMessage3::OnStopSend() 
{
	m_queue2Thd->stopSend();
}