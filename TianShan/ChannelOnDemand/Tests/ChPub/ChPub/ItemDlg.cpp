// ItemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ChPub.h"
#include "ItemDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CItemDlg dialog


CItemDlg::CItemDlg(BOOL edit /* = TRUE */, CWnd* pParent /* = NULL */)
	: m_bEdit(edit), CDialog(CItemDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CItemDlg)
	m_valContent = _T("");
	m_valExpiration = _T("");
	m_valStart = _T("");
	m_valSpliceOut = 0;
	m_valSpliceIn = 0;
	m_valForceNormalSpeed = FALSE;
	m_valPlayable = FALSE;
	m_valOutTimeOffSet = FALSE;
	m_valInTimeOffSet = FALSE;
	//}}AFX_DATA_INIT
}


void CItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CItemDlg)
	DDX_Control(pDX, IDC_START, m_ctlStart);
	DDX_Control(pDX, IDC_EXPIRATION, m_ctlExpiration);
	DDX_Control(pDX, IDC_CONTENT, m_ctlContent);
	DDX_Text(pDX, IDC_CONTENT, m_valContent);
	DDX_Text(pDX, IDC_EXPIRATION, m_valExpiration);
	DDX_Text(pDX, IDC_START, m_valStart);
	DDX_Text(pDX, IDC_SPLICEOUT, m_valSpliceOut);
	DDX_Text(pDX, IDC_SPLICEIN, m_valSpliceIn);
	DDX_Text(pDX, IDC_FORCENORMALSPEED, m_valForceNormalSpeed);
	DDX_Text(pDX, IDC_PLAYABLE, m_valPlayable);
	DDX_Text(pDX, IDC_OUTTIMEOFFSET, m_valOutTimeOffSet);
	DDX_Text(pDX, IDC_INTIMEOFFSET, m_valInTimeOffSet);
	//}}AFX_DATA_MAP

	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CItemDlg, CDialog)
	//{{AFX_MSG_MAP(CItemDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CItemDlg message handlers

BOOL CItemDlg::OnInitDialog() 
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
	if(m_bEdit)
	{
		m_ctlContent.SetReadOnly(TRUE);
	}
	else
	{
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CItemDlg::OnOK() 
{
	UpdateData();
	if(m_valContent.IsEmpty() || m_valStart.IsEmpty() || m_valExpiration.IsEmpty())
	{
		AfxMessageBox("All fields must not be empty!", MB_OK | MB_ICONWARNING);
		return;
	}
	CDialog::OnOK();
}

void CItemDlg::OnCancel() 
{
	CDialog::OnCancel();
}
