// ChnlItemEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CodMan.h"
#include "ChnlItemEditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChnlItemEditDlg dialog


CChnlItemEditDlg::CChnlItemEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChnlItemEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChnlItemEditDlg)
	m_sBroadcast = _T("");
	m_sExpiration = _T("");
	m_bForceSpeed = FALSE;
	m_sItemName = _T("");
	m_inTimeOffset = 0;
	m_bSpliceIn = FALSE;
	m_bSpliceOut = FALSE;
	m_bPlayable = FALSE;
	m_outTimeOffset = 0;
	//}}AFX_DATA_INIT
	editMode = FALSE;
}


void CChnlItemEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChnlItemEditDlg)
	DDX_Control(pDX, IDC_ItemName, m_cItemName);
	DDX_Text(pDX, IDC_Broadcast, m_sBroadcast);
	DDX_Text(pDX, IDC_Expiration, m_sExpiration);
	DDX_Text(pDX, IDC_ForceNormalSpeed, m_bForceSpeed);
	DDX_Text(pDX, IDC_ItemName, m_sItemName);
	DDX_Text(pDX, IDC_InTimeOffset, m_inTimeOffset);
	DDX_Text(pDX, IDC_SpliceIn, m_bSpliceIn);
	DDX_Text(pDX, IDC_SpliceOut, m_bSpliceOut);
	DDX_Text(pDX, IDC_Playable, m_bPlayable);
	DDX_Text(pDX, IDC_OutTimeOffset, m_outTimeOffset);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChnlItemEditDlg, CDialog)
	//{{AFX_MSG_MAP(CChnlItemEditDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChnlItemEditDlg message handlers

void CChnlItemEditDlg::OnOK() 
{
	// TODO: Add extra validation here
	if (!UpdateData(TRUE))
		return;

	m_sBroadcast.TrimLeft();
	m_sExpiration.TrimLeft();
	m_sItemName.TrimLeft();
	m_sBroadcast.TrimRight();
	m_sExpiration.TrimRight();
	m_sItemName.TrimRight();

	if (m_sItemName.IsEmpty())
	{
		AfxMessageBox("channel item name is empty");
		return;
	}

	if (m_sBroadcast.IsEmpty())
	{
		AfxMessageBox("broadcast time is empty");
		return;
	}

	CDialog::OnOK();
}

void CChnlItemEditDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

BOOL CChnlItemEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_cItemName.EnableWindow(!editMode);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
