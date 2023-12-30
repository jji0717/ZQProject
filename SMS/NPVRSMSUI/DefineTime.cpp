// DefineTime.cpp : implementation file
//

#include "stdafx.h"
#include "NPVRSMSUI.h"
#include "DefineTime.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DefineTime dialog


DefineTime::DefineTime(CWnd* pParent /*=NULL*/)
	: CDialog(DefineTime::IDD, pParent)
{
	//{{AFX_DATA_INIT(DefineTime)
	m_time = 0;
	//}}AFX_DATA_INIT
}


void DefineTime::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DefineTime)
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER1, m_time);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DefineTime, CDialog)
	//{{AFX_MSG_MAP(DefineTime)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DefineTime message handlers

BOOL DefineTime::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_time = CTime::GetCurrentTime();

	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void DefineTime::OnOK() 
{
	UpdateData(TRUE);

	CDialog::OnOK();
}
