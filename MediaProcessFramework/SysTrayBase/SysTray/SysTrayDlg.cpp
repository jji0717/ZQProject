// SysTrayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SysTray.h"
#include "SysTrayDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
// CSysTrayDlg dialog

CSysTrayDlg::CSysTrayDlg(CWnd* pParent /*=NULL*/)
	: CInfoDlg(CSysTrayDlg::IDD, pParent),
	m_nInfoDlgCount(0),	m_pInfoThread(NULL), 
	isRereshEnable(FALSE)
{
	//{{AFX_DATA_INIT(CSysTrayDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bWait = FALSE;

	memset(m_pInfoDlg,0,sizeof(m_pInfoDlg));
}

void CSysTrayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSysTrayDlg)
	DDX_Control(pDX, IDC_TAB_SELECT, m_sheet);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSysTrayDlg, CDialog)
	//{{AFX_MSG_MAP(CSysTrayDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_AUTOREFRESH_MESSAGE, OnAutoRefresh)
	ON_MESSAGE(WM_TRAYMESSAGE, OnSysTrayMessage)
	ON_COMMAND(ID_MENUITEM_EXIT, OnMenuitemExit)
	ON_COMMAND(ID_MENUITEM_REFRESH, OnMenuitemRefresh)
	ON_COMMAND(ID_MENUITEM_CONFIG, OnMenuitemConfig)
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSysTrayDlg message handlers

BOOL CSysTrayDlg::OnInitDialog()
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
	AddSysTray();

	m_menu.LoadMenu(IDR_MENU_SYSTRAY);
	m_menu.EnableMenuItem(ID_MENUITEM_REFRESH, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	SetMenu(&m_menu);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSysTrayDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSysTrayDlg::OnPaint() 
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
HCURSOR CSysTrayDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSysTrayDlg::AddSysTray()
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

void CSysTrayDlg::OnClose() 
{
	ShowWindow(SW_HIDE);
}

LRESULT CSysTrayDlg::OnAutoRefresh(WPARAM wParam, LPARAM lParam)
{
	for (int i=0; i<m_nInfoDlgCount; i++)
		m_pInfoDlg[i]->UpdateData(false);
		
	UpdateData(false);

	m_bWait = FALSE;

	SendNotifyMessage(WM_SETCURSOR,0,0);

	EnableWindow();
	
	return 0;
}

void CSysTrayDlg::SetRefreshEnable(bool _isEnable)
{
	isRereshEnable = _isEnable;
	if (isRereshEnable)
		m_menu.EnableMenuItem(ID_MENUITEM_REFRESH, MF_BYCOMMAND | MF_ENABLED);
	else
		m_menu.EnableMenuItem(ID_MENUITEM_REFRESH, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
}

void CSysTrayDlg::CreateManageNode()
{
	// Close all dialogs in tab sheet.
	for (int i=0; i<m_nInfoDlgCount; i++)
	{
		m_pInfoDlg[i]->DestroyWindow();
		delete m_pInfoDlg[i];
	}
	memset(m_pInfoDlg,0,sizeof(m_pInfoDlg));
	m_nInfoDlgCount = 0;

	// Stop Thread
	if (m_pInfoThread)
	{
		m_pInfoThread->stopThread();
		m_pInfoThread->waitHandle(200);
		delete m_pInfoThread;
		m_pInfoThread = NULL;
	}

	m_sheet.ReSet();
	m_pInfoDlg[m_nInfoDlgCount] = new CNodeGeneralInfo;
	m_sheet.AddPage("General Info", m_pInfoDlg[m_nInfoDlgCount++], IDD_DLG_GENERALINFO);
	m_pInfoDlg[m_nInfoDlgCount] = new MN_WorkNodes;
	m_sheet.AddPage("Work Nodes", m_pInfoDlg[m_nInfoDlgCount++], IDD_DLG_WORKNODE);
	m_pInfoDlg[m_nInfoDlgCount] = new MN_Sessions;
	m_sheet.AddPage("Sessions", m_pInfoDlg[m_nInfoDlgCount++], IDD_DLG_SESSIONS);
	m_pInfoDlg[m_nInfoDlgCount] = new MN_ManagedWorkIns;
	m_sheet.AddPage("Managed Work Instances", m_pInfoDlg[m_nInfoDlgCount++], IDD_DLG_MANAGED_WORKINS);
	m_sheet.Show();

	if (!m_pInfoThread)
	{
		m_pInfoThread = new InfoQueryThread(CNodeConfigure::gHost, CNodeConfigure::gPort);
		for (int i=0; i<m_nInfoDlgCount; i++)
			m_pInfoThread->AddUpdateDlg(m_pInfoDlg[i]);
		m_pInfoThread->SetNotifer(this);
		
		// Start infoThread
		m_pInfoThread->SetFreq(CNodeConfigure::gTimeInteval);
		m_pInfoThread->start();
	}
}

void CSysTrayDlg::CreateWorkNode()
{
	// Close all dialogs in tab sheet.
	for (int i=0; i<m_nInfoDlgCount; i++)
	{
		m_pInfoDlg[i]->DestroyWindow();
		delete m_pInfoDlg[i];
	}
	memset(m_pInfoDlg,0,sizeof(m_pInfoDlg));
	m_nInfoDlgCount = 0;

	// Stop Thread
	if (m_pInfoThread)
	{
		m_pInfoThread->stopThread();
		m_pInfoThread->waitHandle(200);
		delete m_pInfoThread;
		m_pInfoThread = NULL;
	}
	
	m_sheet.ReSet();
	m_pInfoDlg[m_nInfoDlgCount] = new CNodeGeneralInfo;
	m_sheet.AddPage("General Info", m_pInfoDlg[m_nInfoDlgCount++], IDD_DLG_GENERALINFO);
	m_pInfoDlg[m_nInfoDlgCount] = new WN_TaskType;
	m_sheet.AddPage("Task Type", m_pInfoDlg[m_nInfoDlgCount++], IDD_DLG_TASKTYPE);
	m_pInfoDlg[m_nInfoDlgCount] = new WN_WorkInstance;
	m_sheet.AddPage("Work Instance", m_pInfoDlg[m_nInfoDlgCount++], IDD_DLG_WORKINSTANCE);
	m_sheet.Show();

	if (!m_pInfoThread)
	{
		m_pInfoThread = new InfoQueryThread(CNodeConfigure::gHost, CNodeConfigure::gPort);
		for (int i=0; i<m_nInfoDlgCount; i++)
			m_pInfoThread->AddUpdateDlg(m_pInfoDlg[i]);
		m_pInfoThread->SetNotifer(this);
		
		// Start infoThread
		m_pInfoThread->SetFreq(CNodeConfigure::gTimeInteval);
		m_pInfoThread->start();
	}
}

void CSysTrayDlg::CreateMixedNode()
{
	// Close all dialogs in tab sheet.
	for (int i=0; i<m_nInfoDlgCount; i++)
	{
		m_pInfoDlg[i]->DestroyWindow();
		delete m_pInfoDlg[i];
	}
	memset(m_pInfoDlg,0,sizeof(m_pInfoDlg));
	m_nInfoDlgCount = 0;

	// Stop Thread
	if (m_pInfoThread)
	{
		m_pInfoThread->stopThread();
		m_pInfoThread->waitHandle(200);
		delete m_pInfoThread;
		m_pInfoThread = NULL;
	}
	
	m_sheet.ReSet();
	// Manage Node information
	m_pInfoDlg[m_nInfoDlgCount] = new CNodeGeneralInfo;
	m_sheet.AddPage("General Info", m_pInfoDlg[m_nInfoDlgCount++], IDD_DLG_GENERALINFO);
	m_pInfoDlg[m_nInfoDlgCount] = new MN_WorkNodes;
	m_sheet.AddPage("Work Nodes", m_pInfoDlg[m_nInfoDlgCount++], IDD_DLG_WORKNODE);
	m_pInfoDlg[m_nInfoDlgCount] = new MN_Sessions;
	m_sheet.AddPage("Sessions", m_pInfoDlg[m_nInfoDlgCount++], IDD_DLG_SESSIONS);
	m_pInfoDlg[m_nInfoDlgCount] = new MN_ManagedWorkIns;
	m_sheet.AddPage("Managed Work Instances", m_pInfoDlg[m_nInfoDlgCount++], IDD_DLG_MANAGED_WORKINS);
	// Work Node Information
	m_pInfoDlg[m_nInfoDlgCount] = new WN_TaskType;
	m_sheet.AddPage("Task Type", m_pInfoDlg[m_nInfoDlgCount++], IDD_DLG_TASKTYPE);
	m_pInfoDlg[m_nInfoDlgCount] = new WN_WorkInstance;
	m_sheet.AddPage("Work Instance", m_pInfoDlg[m_nInfoDlgCount++], IDD_DLG_WORKINSTANCE);

	m_sheet.Show();

	if (!m_pInfoThread)
	{
		m_pInfoThread = new InfoQueryThread(CNodeConfigure::gHost, CNodeConfigure::gPort);
		for (int i=0; i<m_nInfoDlgCount; i++)
			m_pInfoThread->AddUpdateDlg(m_pInfoDlg[i]);
		m_pInfoThread->SetNotifer(this);
		
		// Start infoThread
		m_pInfoThread->SetFreq(CNodeConfigure::gTimeInteval);
		m_pInfoThread->start();
	}
}

LRESULT CSysTrayDlg::OnSysTrayMessage(WPARAM wParam, LPARAM lParam)
{
	UINT uMouseMsg = (UINT) lParam;
		
	if(!IsWindowEnabled())
	{
		return 0;	// if not active, do not show pop-up
	}

	switch (uMouseMsg)
	{
	case WM_LBUTTONDOWN : 
		ShowWindow(SW_SHOW);
		break;
	case WM_RBUTTONDOWN :
	{	
		CPoint point;
		GetCursorPos(&point);
		CMenu menu;
		menu.LoadMenu(IDR_MENU_SYSTRAY);
		if (isRereshEnable)
			menu.EnableMenuItem(ID_MENUITEM_REFRESH, MF_BYCOMMAND | MF_ENABLED);
		else
			menu.EnableMenuItem(ID_MENUITEM_REFRESH, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		CMenu* pPopMenu = menu.GetSubMenu(0);

		// Active the current dialog, without it, the menu will not auto released.
		SetForegroundWindow();  
		pPopMenu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, 
			point.y, this);

		PostMessage(WM_USER, 0, 0);  
		pPopMenu->DeleteMenu(0,MF_BYPOSITION);
	}
	break;

	default: break;
	}

	return 0;
}

void CSysTrayDlg::OnMenuitemExit() 
{
	if (m_pInfoThread)
	{
		m_pInfoThread->stopThread();
		// wait for thread to exit. if fail, kill it
		m_pInfoThread->waitHandle(200);
		delete m_pInfoThread;
		m_pInfoThread = NULL;
	}

	// destroy menu
	m_menu.DestroyMenu();

	// Clean sys tray
	tnid.cbSize=sizeof(NOTIFYICONDATA);
	tnid.hWnd=this->m_hWnd;
	tnid.uID=IDR_MAINFRAME;
	Shell_NotifyIcon(NIM_DELETE,&tnid);

	// Close all dialogs in tab sheet.
	m_sheet.DeleteAllItems();
	for (int i=0; i<m_nInfoDlgCount; i++)
	{
		m_pInfoDlg[i]->DestroyWindow();
		delete m_pInfoDlg[i];
	}

	// If the dlg is setuped by doModal(), it must be eliminated by EndDialog()
	// DestroyWindow() would not fit MFC code.
	EndDialog(10); // Any value is ok as parameter
}

void CSysTrayDlg::OnMenuitemRefresh() 
{
	m_pInfoThread->startQuery();
	m_bWait	= TRUE;
	EnableWindow(FALSE);

//	for (int i=0; i<m_nInfoDlgCount; i++)
//		m_pInfoDlg[i]->UpdateData(false);
}

RpcValue& CSysTrayDlg::GenRpcValue()
{
	return params;
}

void CSysTrayDlg::UpdateDlgList(RpcValue& result)
{
}

void CSysTrayDlg::OnMenuitemConfig() 
{
	CNodeConfigure config;
	config.SetUerConfig(CNodeConfigure::gHost, CNodeConfigure::gPort, CNodeConfigure::gTimeInteval);
	switch(config.DoModal()) {
	case -1: 
		AfxMessageBox("Dialog box could not be created!");
		break;
    case IDABORT:
		break;
    case IDOK:
	{
		CNodeConfigure::gHost = config.GetHost();
		CNodeConfigure::gPort = config.GetPort();
		CNodeConfigure::gTimeInteval = config.GetTimeInteval();
		
		switch(config.GetNodeType()) {
		case NODE_TYPE_MANAGERNODE:
			SetRefreshEnable(TRUE);
			CreateManageNode();
			break;
		case NODE_TYPE_WORKNODE:
			SetRefreshEnable(TRUE);
			CreateWorkNode();
			break;
		case NODE_TYPE_MIXED:
			SetRefreshEnable(TRUE);
			CreateMixedNode();
			break;
		default:
			SetRefreshEnable(FALSE);
			AfxMessageBox("The connection type is unknown, please check.");
			break;
		}
	}
		break;
    case IDCANCEL:	
		break;
    default:
		break;
	}
}

void CSysTrayDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if(m_sheet.m_hWnd)
	{
		CRect rect;
		GetClientRect(&rect);
		rect.top+=10;
		rect.left+=10;
		rect.bottom-=10;
		rect.right-=10;
		m_sheet.MoveWindow(&rect);
	}
}

BOOL CSysTrayDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_bWait)
	{
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));
		return TRUE;
	}
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

