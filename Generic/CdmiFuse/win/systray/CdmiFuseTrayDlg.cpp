// CdmiFuseTrayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CdmiFuseTray.h"
#include "CdmiFuseTrayDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_TASKBARICON		1
#define WM_ICONNOTIFY		(WM_USER+101)

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CCdmiFuseTrayDlg dialog




CCdmiFuseTrayDlg::CCdmiFuseTrayDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCdmiFuseTrayDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void CCdmiFuseTrayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCdmiFuseTrayDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_MESSAGE(WM_USER_EXIT,OnExit) //tray exit message
	ON_MESSAGE(WM_USER_SETUP,OnSetup)//tray setup message
END_MESSAGE_MAP()


// CCdmiFuseTrayDlg message handlers

BOOL CCdmiFuseTrayDlg::OnInitDialog()
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
	// Init Tray
	m_Nid.cbSize = sizeof(NOTIFYICONDATA);
	m_Nid.uID = ID_TASKBARICON;
	m_Nid.hWnd = this->GetSafeHwnd();
	ASSERT( NULL != m_Nid.hWnd );
	m_Nid.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
	m_Nid.uCallbackMessage = WM_ICONNOTIFY;
	m_Nid.hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, 0);
	ASSERT( NULL != m_Nid.hIcon );
	
	strcpy_s(m_Nid.szTip,"CdmiFuseTray");
	
	//m_Nid.szTip.LoadString( IDS_TRAYTIP );

	// 添加into系统托盘
	Shell_NotifyIcon(NIM_ADD, &m_Nid);

	// 隐藏对话框
	ModifyStyleEx(WS_EX_APPWINDOW,WS_EX_TOOLWINDOW);//从任务栏中去掉.
	WINDOWPLACEMENT wp;
	wp.length=sizeof(WINDOWPLACEMENT);
	wp.flags=WPF_RESTORETOMAXIMIZED;
	wp.showCmd=SW_HIDE;
	SetWindowPlacement(&wp);

	//initial dialog
	m_dlgSettings.DoModal();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCdmiFuseTrayDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCdmiFuseTrayDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCdmiFuseTrayDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CALLBACK EnumProc( HWND hWnd, LPARAM lParam)
{
	static const TCHAR szAfxOldWndProc[] = _T("AfxOldWndProc423");  // Visual C++ 6.0
	//check for property and unsubclass if necessary
	WNDPROC oldWndProc = (WNDPROC)::GetProp(hWnd, szAfxOldWndProc);
	if (oldWndProc!=NULL)
	{
		SetWindowLong(hWnd, GWL_WNDPROC, (DWORD)oldWndProc);
		RemoveProp(hWnd, szAfxOldWndProc);
	}
	
	return TRUE;
}

LRESULT CCdmiFuseTrayDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	static UINT const WM_TASKBARCREATED = RegisterWindowMessage( _T("TaskbarCreated") );
	if( WM_TASKBARCREATED == message ) // Explorer.exe重新启动
	{
		Shell_NotifyIcon(NIM_ADD, &m_Nid);
		return TRUE;
	}

	// TODO: Add your specialized code here and/or call the base class
	switch( message )
	{
	case WM_ENDSESSION:
		{
		DWORD dwProcessId;
	    DWORD dwThreadId= GetWindowThreadProcessId(m_hWnd,&dwProcessId);
		EnumThreadWindows(dwThreadId, EnumProc,(LPARAM) dwThreadId);
		}
		return TRUE;
	case WM_SYSCOMMAND:
		switch( wParam )
		{
		// 隐藏
		case SC_MINIMIZE:
		case SC_CLOSE:
			//ShowWindow(SW_HIDE);
			EndDialog( 0L );			
			return TRUE;
		}
		break;
	case WM_ICONNOTIFY:
		switch( lParam )
		{
		case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:	// 恢复显示
		      m_dlgSettings.ShowWindow(SW_SHOW);
		      SetForegroundWindow();
			return TRUE;
		case WM_RBUTTONUP:		// 显示菜单
			POINT point;
			HMENU hMenu, hSubMenu;
			GetCursorPos(&point);
			hMenu = LoadMenu(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MENU1));
			hSubMenu = GetSubMenu(hMenu, 0);
			SetForegroundWindow();
			SetMenuDefaultItem(hSubMenu, IDS_TRAYTIP, FALSE);
			TrackPopupMenu(hSubMenu,TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_LEFTALIGN,point.x, point.y, 0, this->GetSafeHwnd(), NULL);
			PostMessage(WM_NULL,0,0);
			DestroyMenu(hMenu);
			return TRUE;
		}
		return TRUE;
	}

	return CDialog::DefWindowProc(message, wParam, lParam);
}


void CCdmiFuseTrayDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	m_Nid.uFlags = NIF_ICON;
	m_Nid.hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_STOPPING), IMAGE_ICON, 16, 16, 0);
	Shell_NotifyIcon(NIM_DELETE, &m_Nid);		// 删除from系统托盘

	// add by shawn 2007-12-12  fix crash preblom when exit servics.
	theApp.m_pMainWnd = NULL;

	CDialog::OnClose();
}


LRESULT CCdmiFuseTrayDlg::OnExit(WPARAM wParam,LPARAM lParam)
{
	m_dlgSettings.SendMessage(WM_USER_CLOSE);

	//
	SendMessage(WM_CLOSE);
	return 0;

}

LRESULT CCdmiFuseTrayDlg::OnSetup(WPARAM wParam,LPARAM lParam)
{
	m_dlgSettings.ShowWindow(SW_SHOW);

	return 0;

}
