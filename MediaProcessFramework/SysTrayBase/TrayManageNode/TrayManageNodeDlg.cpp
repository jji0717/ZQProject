// TrayManageNodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TrayManageNode.h"
#include "TrayManageNodeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TIMER_ID 1

char* gHost = "192.168.80.74";
int gPort = 10000;
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTrayManageNodeDlg dialog

CTrayManageNodeDlg::CTrayManageNodeDlg(CWnd* pParent /*=NULL*/)
	: CInfoDlg(CTrayManageNodeDlg::IDD, pParent),
	  infoThread(gHost,gPort), nTimeInterval(30)
{
	//{{AFX_DATA_INIT(CTrayManageNodeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTrayManageNodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTrayManageNodeDlg)
	DDX_Control(pDX, IDC_TAB_SELECT, m_sheet);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTrayManageNodeDlg, CDialog)
	//{{AFX_MSG_MAP(CTrayManageNodeDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_MENU_EXIT, OnMenuExit)
	ON_COMMAND(ID_MENU_REFRESH, OnMenuRefresh)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_MESSAGE(WM_TRAYMESSAGE,OnTrayMessage)
	ON_WM_TIMER()
	ON_MESSAGE(WM_AUTOREFRESH_MESSAGE, OnAutoRefresh) 
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTrayManageNodeDlg message handlers

BOOL CTrayManageNodeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_sheet.AddPage("General Info", &infoDlg, IDD_DLG_GENERALINFO);
	m_sheet.AddPage("Work Nodes", &workndeDlg, IDD_DLG_WORKNODE);
	m_sheet.AddPage("Sessions", &sessionDlg, IDD_DLG_SESSIONS);
	m_sheet.AddPage("Managed Work Instances", &workinsDlg, IDD_DLG_MANAGED_WORKINS);
	m_sheet.Show();

	infoThread.AddUpdateDlg(&infoDlg,GETINFO_METHOD);
	infoThread.AddUpdateDlg(&workndeDlg,GETINFO_METHOD);
	infoThread.AddUpdateDlg(&sessionDlg,GETINFO_METHOD);
	infoThread.AddUpdateDlg(&workinsDlg,GETINFO_METHOD);
	infoThread.SetNotifer(this);

	// Start infoThread
	infoThread.SetFreq(nTimeInterval*1000);
	infoThread.start();

	AddSysTray();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTrayManageNodeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTrayManageNodeDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTrayManageNodeDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CTrayManageNodeDlg::AddSysTray()
{
	tnid.cbSize = sizeof(NOTIFYICONDATA); 
    tnid.hWnd = m_hWnd; 
    tnid.uID = IDR_MAINFRAME; 
    tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; 
    tnid.uCallbackMessage = WM_TRAYMESSAGE; 
    tnid.hIcon = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
	strcpy(tnid.szTip, "SysTray Work Node");

	Shell_NotifyIcon(NIM_ADD, &tnid); 
}

void CTrayManageNodeDlg::OnTrayMessage(WPARAM wParam,LPARAM lParam)
{
	UINT uMouseMsg = (UINT) lParam;
		
	switch (uMouseMsg)
	{
	case WM_LBUTTONDOWN : 
		ShowWindow(SW_SHOW);
		break;
	case WM_RBUTTONDOWN :
	{	
		CPoint point;
		GetCursorPos(&point);
		CMenu* pMenu = new CMenu;
		pMenu->LoadMenu(IDR_TRAY_MENU);
		CMenu* pPopMenu = pMenu->GetSubMenu(0);
		pPopMenu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, 
			point.y, this);

		::SetForegroundWindow((HWND)pPopMenu->m_hMenu);
		delete pMenu;
	}
	break;

	default: break;
	}
}

void CTrayManageNodeDlg::OnMenuExit() 
{
	// TODO: Add your command handler code here
	if (isWindowShown)
		this->ShowWindow(SW_HIDE);   // Hide current Dlg

	infoThread.stopThread();
	int nwait=infoThread.waitHandle(1000);	// wait for thread to exit. if fail, kill it
	
	// Clean sys tray
	tnid.cbSize=sizeof(NOTIFYICONDATA);
	tnid.hWnd=this->m_hWnd;
	tnid.uID=IDR_MAINFRAME;
	Shell_NotifyIcon(NIM_DELETE,&tnid);

	// If the dlg is setuped by doModal(), it must be eliminated by EndDialog()
	// DestroyWindow() would not fit MFC code.
	EndDialog(10); // Any value is ok as parameter

}

void CTrayManageNodeDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	this->ShowWindow(SW_HIDE);
	isWindowShown = false;
}

void CTrayManageNodeDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if(m_sheet.m_hWnd)
	{
		CRect rect;
		GetClientRect(&rect);
		rect.top+=10;
		rect.left+=10;
		rect.bottom-=10;
		rect.right-=10;
		m_sheet.MoveWindow(&rect);

		//(this->GetDlgItem(IDC_LIST_TASKTYPE))->MoveWindow(&rect,true);
	}
}

void CTrayManageNodeDlg::OnMenuRefresh() 
{
	infoThread.startQuery();
	infoDlg.UpdateData(false);
	workndeDlg.UpdateData(false);
	sessionDlg.UpdateData(false);
	workinsDlg.UpdateData(false);
}

LRESULT CTrayManageNodeDlg::OnAutoRefresh(WPARAM wParam, LPARAM lParam)
{
	infoDlg.UpdateData(false);
	workndeDlg.UpdateData(false);
	sessionDlg.UpdateData(false);
	workinsDlg.UpdateData(false);
	
	UpdateData(false);
	return 0;
}

RpcValue& CTrayManageNodeDlg::GenRpcValue()
{
	// TODO: add data params if you have something to show on the main dialog
	return params;
}

void CTrayManageNodeDlg::UpdateDlgList(RpcValue& result)
{
	// TODO: add data update if you have something to show on the main dialog
}
