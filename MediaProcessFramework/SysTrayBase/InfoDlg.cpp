// InfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InfoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInfoDlg dialog


CInfoDlg::CInfoDlg(DWORD _IDD, CWnd* pParent /*=NULL*/)
	: CDialog(_IDD, pParent)
{
	//{{AFX_DATA_INIT(CInfoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CInfoDlg::~CInfoDlg()
{
}

void CInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInfoDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

void CInfoDlg::startWindowUpdate()
{
	SendNotifyMessage(WM_AUTOREFRESH_MESSAGE, 0, 0);
}

BEGIN_MESSAGE_MAP(CInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CInfoDlg)
	ON_MESSAGE(WM_AUTOREFRESH_MESSAGE, OnAutoRefresh) 
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInfoDlg message handlers
LRESULT CInfoDlg::OnAutoRefresh(WPARAM wParam, LPARAM lParam)
{
	UpdateData(false);
	return 0;
}