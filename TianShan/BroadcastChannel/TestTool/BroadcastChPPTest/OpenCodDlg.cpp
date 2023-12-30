// OpenCodDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BroadcastChPPTest.h"
#include "OpenCodDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COpenCodDlg dialog


COpenCodDlg::COpenCodDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COpenCodDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COpenCodDlg)
	m_strBcastChEndPoint = _T("BcastChannelPublisher:tcp -h 192.168.81.102 -p 11700");
	//}}AFX_DATA_INIT
}


void COpenCodDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpenCodDlg)
	DDX_Text(pDX, IDC_BcastCh_EndPoint, m_strBcastChEndPoint);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COpenCodDlg, CDialog)
	//{{AFX_MSG_MAP(COpenCodDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpenCodDlg message handlers

void COpenCodDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void COpenCodDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}
