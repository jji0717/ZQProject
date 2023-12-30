// ChannelEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CodMan.h"
#include "ChannelEditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChannelEditDlg dialog


CChannelEditDlg::CChannelEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChannelEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChannelEditDlg)
	m_sChnlName = _T("");
	m_sDesc = _T("");
	m_sNetIds = _T("");
	m_sOnDmdName = _T("");
	m_iMaxBit = 0;
	//}}AFX_DATA_INIT
	editMode = FALSE;
}


void CChannelEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChannelEditDlg)
	DDX_Control(pDX, IDC_ChannelName, m_cChnlName);
	DDX_Text(pDX, IDC_ChannelName, m_sChnlName);
	DDX_Text(pDX, IDC_Description, m_sDesc);
	DDX_Text(pDX, IDC_NetIds, m_sNetIds);
	DDX_Text(pDX, IDC_OnDemandName, m_sOnDmdName);
	DDX_Text(pDX, IDC_MaxBitrates, m_iMaxBit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChannelEditDlg, CDialog)
	//{{AFX_MSG_MAP(CChannelEditDlg)
	//}}AFX_MSG_MAP
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
	m_sOnDmdName.TrimLeft();
	m_sChnlName.TrimRight();
	m_sDesc.TrimRight();
	m_sNetIds.TrimRight();
	m_sOnDmdName.TrimRight();

	if (m_sOnDmdName.IsEmpty())
	{
		AfxMessageBox("on demand name is empty");
		return;
	}

	if (m_sChnlName.IsEmpty())
	{
		m_sChnlName = m_sOnDmdName;
		UpdateData(FALSE);
	}

	if (m_sDesc.IsEmpty())
	{
		AfxMessageBox("Channel description is empty");
		return;
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
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
