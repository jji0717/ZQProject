// ChannelDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ChPub.h"
#include "ChannelDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChannelDlg dialog


CChannelDlg::CChannelDlg(BOOL readonly /* = TRUE */, CWnd* pParent /* = NULL */)
	: m_ReadOnly(readonly), CDialog(CChannelDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChannelDlg)
	m_valName = _T("");
	m_valDesc = _T("");
	//}}AFX_DATA_INIT
}


void CChannelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChannelDlg)
	DDX_Control(pDX, IDC_DESC, m_ctlDesc);
	DDX_Control(pDX, IDC_NAME, m_ctlName);
	DDX_Text(pDX, IDC_NAME, m_valName);
	DDX_Text(pDX, IDC_DESC, m_valDesc);
	//}}AFX_DATA_MAP

	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CChannelDlg, CDialog)
	//{{AFX_MSG_MAP(CChannelDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChannelDlg message handlers

void CChannelDlg::OnOK() 
{
	UpdateData();
	if(m_valName.IsEmpty() || m_valDesc.IsEmpty())
	{
		AfxMessageBox("All fields must not be empty!", MB_OK | MB_ICONWARNING);
		return;
	}
	CDialog::OnOK();
}

void CChannelDlg::OnCancel() 
{
	CDialog::OnCancel();
}

BOOL CChannelDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// init buttons
	short	shBtnColor = 30;
	m_btnOK.SetIcon(IDI_OK, (int)BTNST_AUTO_GRAY);
	m_btnOK.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, shBtnColor);
	m_btnOK.SetTooltipText("OK");
	m_btnCancel.SetIcon(IDI_CANCEL, (int)BTNST_AUTO_GRAY);
	m_btnCancel.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, shBtnColor);
	m_btnCancel.SetTooltipText("Cancel");

	// set read-only controls
	if(m_ReadOnly)
	{
		m_btnCancel.EnableWindow(FALSE);
		m_ctlName.SetReadOnly(TRUE);
		m_ctlDesc.SetReadOnly(TRUE);
	}
	else
	{
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
