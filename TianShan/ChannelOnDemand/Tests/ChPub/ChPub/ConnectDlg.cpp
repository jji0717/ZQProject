// ConnectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ChPub.h"
#include "ConnectDlg.h"
#include "ChODDefines.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

TestClient g_Client;

/////////////////////////////////////////////////////////////////////////////
// CConnectDlg dialog


CConnectDlg::CConnectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConnectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConnectDlg)
	m_serverEndpoint = _T("");
	//}}AFX_DATA_INIT
	m_serverEndpoint = DEFAULT_ENDPOINT_ChOD;
}


void CConnectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConnectDlg)
	DDX_Text(pDX, IDC_ENDPOINT, m_serverEndpoint);
	//}}AFX_DATA_MAP

	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CConnectDlg, CDialog)
	//{{AFX_MSG_MAP(CConnectDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnectDlg message handlers

void CConnectDlg::OnOK() 
{
	UpdateData();

	::std::string errmsg = "";
	// try to connect
	try
	{
		errmsg = g_Client.connect((LPCSTR)m_serverEndpoint);
		if(!errmsg.empty())
		{
			CString msg = "Can not connect to server \"" + m_serverEndpoint + "\"! Error: " + errmsg.c_str();
			AfxMessageBox(msg, MB_OK|MB_ICONWARNING);
			return;
		}

	}
	catch(...)
	{
		CString msg = "Can not connect to server \"" + m_serverEndpoint + "\"!";
		AfxMessageBox(msg, MB_OK|MB_ICONWARNING);
		return;
	}
	
	CDialog::OnOK();
}

void CConnectDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}


BOOL CConnectDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	short	shBtnColor = 30;
	m_btnOK.SetIcon(IDI_OK, (int)BTNST_AUTO_GRAY);
	m_btnOK.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, shBtnColor);
	m_btnOK.SetTooltipText("OK");
	m_btnCancel.SetIcon(IDI_CANCEL, (int)BTNST_AUTO_GRAY);
	m_btnCancel.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, shBtnColor);
	m_btnCancel.SetTooltipText("Cancel");
	
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
