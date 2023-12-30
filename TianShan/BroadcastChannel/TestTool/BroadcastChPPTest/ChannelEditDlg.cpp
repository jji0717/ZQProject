// ChannelEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BroadcastChPPTest.h"
#include "ChannelEditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChannelEditDlg dialog


CChannelEditDlg::CChannelEditDlg(BOOL bNVOD, CWnd* pParent /*=NULL*/)
	: CDialog(CChannelEditDlg::IDD, pParent), m_Interval(0), m_Iteration(0)
	, m_bandwidth(0)
	, m_destMac(_T(""))
	, m_destIp(_T(""))
	, m_destPort(0)
	, m_sPersistent(_T(""))
{
	//{{AFX_DATA_INIT(CChannelEditDlg)
	if(bNVOD)
		m_sChnlName = _T("NVOD");
	else
		m_sChnlName = _T("Broad");
	m_sDesc = _T("");
	m_sNetIds = _T("NetId");
	m_sPersistent = _T("0");
	//}}AFX_DATA_INIT
	editMode = FALSE;
	m_bNVOD = bNVOD;
}

void CChannelEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChannelEditDlg)
	DDX_Control(pDX, IDC_ChannelName, m_cChnlName);
	DDX_Text(pDX, IDC_ChannelName, m_sChnlName);
	DDX_Text(pDX, IDC_Description, m_sDesc);
	DDX_Text(pDX, IDC_NetIds, m_sNetIds);	
	DDX_Text(pDX, IDC_Interval, m_Interval);
	DDX_Text(pDX, IDC_Iteration, m_Iteration);	
	DDX_Text(pDX, IDC_bandwidth, m_bandwidth);
	//}}AFX_DATA_MAP

	DDX_Control(pDX, IDC_Interval, m_cInterval);
	DDX_Control(pDX, IDC_Iteration, m_cIteration);

	DDX_Text(pDX, IDC_DestMac, m_destMac);
	DDX_Text(pDX, IDC_DestIp, m_destIp);
	DDX_Text(pDX, IDC_DestPort, m_destPort);
	DDX_Control(pDX, IDC_bandwidth, m_cBandwidth);
	DDX_Control(pDX, IDC_DestMac, m_cDestMac);
	DDX_Control(pDX, IDC_DestIp, m_cDestIp);
	DDX_Control(pDX, IDC_DestPort, m_cDestPort);
	DDX_Control(pDX, IDC_Persistent, m_cPersistent);
	DDX_Text(pDX, IDC_Persistent, m_sPersistent);
}


BEGIN_MESSAGE_MAP(CChannelEditDlg, CDialog)
	//{{AFX_MSG_MAP(CChannelEditDlg)
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_EDIT1, &CChannelEditDlg::OnEnChangeEdit1)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChannelEditDlg message handlers

void CChannelEditDlg::OnOK() 
{
	// TODO: Add extra validation here
	if (!UpdateData(TRUE))
		return;

	m_sChnlName.TrimLeft();
	m_sDesc.TrimLeft();
	m_sNetIds.TrimLeft();
	m_sPersistent.TrimLeft();

	m_sChnlName.TrimRight();
	m_sDesc.TrimRight();
	m_sNetIds.TrimRight();
	m_sPersistent.TrimRight();

	if (m_sChnlName.IsEmpty())
	{
		AfxMessageBox("channel name is empty");
		return;
	}

	if (m_sDesc.IsEmpty())
	{
		AfxMessageBox("Channel description is empty");
		return;
	}
	if(m_Interval <= 0 && m_bNVOD)
	{
		AfxMessageBox("Interval must be above 0");
		return;
	}

	if(m_Iteration <= 0 && m_bNVOD)
	{
		AfxMessageBox("Iteration must be above 0");
		return;
	}
	if(m_bandwidth <= 0 & !editMode)
	{
		AfxMessageBox("Bandwidth must be above 0");
		return;
	}
	if(m_destPort > 65535 || m_destPort < 0)
	{
		AfxMessageBox("Bandwidth must be above 0 and below 63336");
		return;
	}
	if (m_sPersistent.IsEmpty())
	{
		m_sPersistent = "0";
		UpdateData(TRUE);
	}
	CDialog::OnOK();
}

void CChannelEditDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}


BOOL CChannelEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_cChnlName.EnableWindow(!editMode);
	m_cInterval.EnableWindow(!editMode);
	m_cIteration.EnableWindow(!editMode);

	m_cDestPort.EnableWindow(!editMode);
	m_cDestIp.EnableWindow(!editMode);
	m_cDestMac.EnableWindow(!editMode);
	m_cBandwidth.EnableWindow(!editMode);
	m_cPersistent.EnableWindow(!editMode);

	if(!m_bNVOD)
	{
		m_cInterval.EnableWindow(FALSE);
		m_cIteration.EnableWindow(FALSE);
	}
	
	UpdateData(TRUE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CChannelEditDlg::OnEnChangeEdit1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
