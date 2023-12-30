// AppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "App.h"
#include "AppDlg.h"

#include "clog.h"
//#include "DeviceAgent.h"
#include "DODClientAgent.h"
#include ".\appdlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static UINT WM_DODTRAY = ::RegisterWindowMessage("DODCreateTrayIcon");
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
// CAppDlg dialog

CAppDlg::CAppDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAppDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAppDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAppDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAppDlg, CDialog)
	//{{AFX_MSG_MAP(CAppDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_WM_TIMER()
	ON_REGISTERED_MESSAGE(WM_DODTRAY, OnTrayMessage)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON5, OnButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnButton6)
	ON_BN_CLICKED(IDC_BUTTON7, OnButton7)
	ON_BN_CLICKED(IDC_BUTTON8, OnButton8)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_CLOSE_EXIT, OnCloseExit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAppDlg message handlers

BOOL CAppDlg::OnInitDialog()
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
	m_bEnableChannel=TRUE;
	MoveWindow(0,0,1,1);	

	SetTimer(1,1000,NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CAppDlg::OnPaint() 
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
HCURSOR CAppDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}
CDODClientAgent *g_pDCAgent=NULL;
void CAppDlg::OnButton1() 
{
	if (g_pDCAgent==NULL)
	{	
		ClogEstablishSettings("DODClineAgent.log",LOG_DEBUG,0x400000 );
		g_pDCAgent = new CDODClientAgent();	
		g_pDCAgent->create();
	}
}
CAppDlg::~CAppDlg()	// standard constructor
{
	if(g_pDCAgent)
	{
		g_pDCAgent->Destroy();
		delete g_pDCAgent;
		g_pDCAgent=NULL;

	}
}

void CAppDlg::OnOK() 
{

	CDialog::OnOK();
}

void CAppDlg::OnButton2() 
{
	if (g_pDCAgent==NULL)
		return ;
	g_pDCAgent->SetMessage();
}


void CAppDlg::TrayIcon(BOOL add)
{
	HICON Icon;
	Icon = (HICON) AfxGetApp()->LoadIcon(IDR_ICON1);
	NOTIFYICONDATA nid;
	CString pcstr;
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hIcon =	Icon;
	nid.hWnd = AfxGetMainWnd()->GetSafeHwnd();
	nid.uCallbackMessage = WM_DODTRAY;
	nid.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
	nid.uID = 1;
	AfxGetMainWnd()->GetWindowText(pcstr);
	_tcscpy(nid.szTip, pcstr);
	::Shell_NotifyIcon(add ? NIM_ADD : NIM_DELETE, &nid);
}
void CAppDlg::OnTimer(UINT nIDEvent) 
{

	if (nIDEvent==1)
	{
		KillTimer(1);
		TrayIcon(true);
		ShowWindow(0);
		SetTimer(2,50,NULL);
	}
	else
		if (nIDEvent==2)
		{
			KillTimer(2);
			OnButton1();
		}
	CDialog::OnTimer(nIDEvent);
}

void CAppDlg::OnButton3() 
{
if (g_pDCAgent==NULL)
		return ;
	g_pDCAgent->Updatecatelog();
	
}

void CAppDlg::OnButton4() 
{
if (g_pDCAgent==NULL)
		return ;
	g_pDCAgent->Stop();
}

void CAppDlg::OnClose() 
{
	if(g_pDCAgent)
	{
		g_pDCAgent->Destroy();
		delete g_pDCAgent;
		g_pDCAgent=NULL;
	}
	CDialog::OnClose();
}

void CAppDlg::OnButton5() 
{

if (g_pDCAgent==NULL)
		return ;
	int nState=0;
	nState=g_pDCAgent->GetState(0);	
	if(nState==-1)
	{
		//clog(3,"PORTindex=%d,getStateError=%d",nState);
		return ;
	}
	
	CString str;
	switch(nState) {
	case 0:
		str="initialized";
		break;
	case 1:
		str="port opened ";
		break;
	case 2:
		str="Running ";
		break;
	case 3:
		str="Stop ";
		break;
	case 4:
		str="Close ";
		break;
	case 5:
		str="Error ";
		break;
	default:
		break;
	}
	
//	AfxMessageBox(str);
}

void CAppDlg::OnButton6() 
{
	if (g_pDCAgent==NULL)
		return ;
	g_pDCAgent->ClosePort();
	
}

void CAppDlg::OnButton7() 
{
	if (g_pDCAgent==NULL)
		return ;
	CString str="Channel monitor path : ";
	str+=g_pDCAgent->GetPort();
//	AfxMessageBox(str);
}

void CAppDlg::OnButton8() 
{
	if (g_pDCAgent==NULL)
		return ;
		m_bEnableChannel=!m_bEnableChannel;
	g_pDCAgent->EnableChannel(m_bEnableChannel);

}


LRESULT CAppDlg::OnTrayMessage(WPARAM wParam, LPARAM lParam)
{
	UINT msg = (UINT)lParam;
	CWnd* pTarget = AfxGetMainWnd();

	switch (msg)
	{
/*	case WM_LBUTTONDBLCLK:
		//	CWnd* pTadrgeat = AfxGetMainWnd();
		pTarget->SetForegroundWindow(); 
		pTarget->PostMessage(WM_COMMAND,ID_MENUITEM32771, 0);	

		break;*/
	case WM_RBUTTONUP:
		CPoint pos;
		CMenu theMenu;
		CMenu menu;
		theMenu.LoadMenu(IDR_MENU1);
		CMenu *subMenu = (CMenu*)theMenu.GetSubMenu(0);
		GetCursorPos(&pos);
		menu.LoadMenu(IDR_MENU1);
		::SetMenuDefaultItem(menu, 0, TRUE);
		pTarget->SetForegroundWindow(); 
		::TrackPopupMenu(subMenu->GetSafeHmenu(), 0, pos.x, pos.y, 0, 
			pTarget->GetSafeHwnd(), NULL);
		pTarget->PostMessage(WM_NULL, 0, 0);
		break;
	}
	return 1;
}
void CAppDlg::OnCloseExit()
{
	TrayIcon(FALSE);
	PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0L);
}
