// JmsUIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "JmsUI.h"
#include "JmsUIDlg.h"
#include "Queue1Thread.h"
#include "Queue2Thread.h"
#include "Message1.h"
#include "Message2.h"
#include "Message3.h"
#include "Message4.h"
#include "Log.h"

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
// CJmsUIDlg dialog

CJmsUIDlg::CJmsUIDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CJmsUIDlg::IDD, pParent), 
	  m_JmsPublisherMgr(NULL),
	  m_Queue1Thd(NULL),
	  m_Queue2Thd(NULL)
{
	//{{AFX_DATA_INIT(CJmsUIDlg)
	m_Queue1_DEST = _T("");
	m_Queue2_DEST = _T("");
	m_Queue_IP = _T("");
	m_Queue_Port = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CJmsUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJmsUIDlg)
	DDX_Control(pDX, IDC_TAB1, m_tab);
	DDX_Text(pDX, IDC_QUEUE1_DEST, m_Queue1_DEST);
	DDX_Text(pDX, IDC_QUEUE2_DEST, m_Queue2_DEST);
	DDX_Text(pDX, IDC_QUEUE_IP, m_Queue_IP);
	DDX_Text(pDX, IDC_QUEUE_PORT, m_Queue_Port);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CJmsUIDlg, CDialog)
	//{{AFX_MSG_MAP(CJmsUIDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SHARE_FOLDER, OnShareFolder)
	ON_BN_CLICKED(IDC_MESSAGE, OnMessage)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnSelchangeTab1)
	ON_BN_CLICKED(IDC_CLOSE, OnClose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJmsUIDlg message handlers

BOOL CJmsUIDlg::OnInitDialog()
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
	
	m_tab.InsertItem(0, "Config");
	m_tab.InsertItem(1, "Message Class Select");
	
	init();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CJmsUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CJmsUIDlg::OnPaint() 
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
HCURSOR CJmsUIDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CJmsUIDlg::OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int sel = m_tab.GetCurSel();
	
	switch(sel)
	{
	case 0:
		ShowTab2(false);
		ShowTab1(true);
		EnableTab1(m_bShowFolderConnect, m_bMessageConnect);
		break;
	case 1:
		ShowTab1(false);
		ShowTab2(true);
		EnableTab2(m_bShowFolderConnect, m_bMessageConnect);
		break;
	default:
		break;
	}

	*pResult = 0;
}

bool CJmsUIDlg::ConnectJBoss()
{
	if (!m_bConnectJBoss)
	{
		int port = atoi(m_Queue_Port);
		m_JmsPublisherMgr = new JMSPublisherManager(m_Queue_IP, port);
		m_bConnectJBoss = true;
	}

	m_JmsPublisherMgr->createPublisher(m_Queue1_DEST);
	m_JmsPublisherMgr->createPublisher(m_Queue2_DEST);
	m_JmsPublisherMgr->setLogger(m_FileReport);
	m_JmsPublisherMgr->start();
	
	return true;
}

void CJmsUIDlg::OnShareFolder() 
{
	CPropertySheet sheet("Share Folder");
	CMessage1 msg1;
	CMessage2 msg2;
	sheet.AddPage(&msg1);
	sheet.AddPage(&msg2);
	msg1.m_queue1Thd = m_Queue1Thd;
	msg2.m_queue1Thd = m_Queue1Thd;
	sheet.DoModal();
}

void CJmsUIDlg::OnMessage() 
{
	CPropertySheet sheet("Messages");
	CMessage3 msg3;
	CMessage4 msg4;
	sheet.AddPage(&msg3);
	sheet.AddPage(&msg4);
	msg3.m_queue2Thd = m_Queue2Thd;
	msg4.m_queue2Thd = m_Queue2Thd;
	sheet.DoModal();
}

void CJmsUIDlg::init()
{	
	m_FileReport = new FileReport("FileLog", true);
	m_FileReport->initialize("E:\\VSS\\ZQProjs\\JmsUI\\Log\\JmsUI.log",
							 DEFAULT_LOG_FILE_SIZE,
							 DEFAULT_LOG_FILE_BUF_SIZE,
							 100);
	m_FileReport->setUnixLogFileCount(100);
	m_FileReport->start();

	m_bShowFolderConnect = false;
	m_bMessageConnect = false;
	m_bConnectJBoss = false;
	ShowTab1(true);
	ShowTab2(false);
}

void CJmsUIDlg::ShowTab1(bool bShow)
{
	CWnd* p;
	p = (CWnd*)GetDlgItem(IDC_QUEUE_PORT);
	p->ShowWindow(bShow);
	p = (CWnd*)GetDlgItem(IDC_QUEUE_IP);
	p->ShowWindow(bShow);
	p = (CWnd*)GetDlgItem(IDOK);
	p->ShowWindow(bShow);
	p = (CWnd*)GetDlgItem(IDC_QUEUE1);
	p->ShowWindow(bShow);
	p = (CWnd*)GetDlgItem(IDC_QUEUE2);
	p->ShowWindow(bShow);
	p = (CWnd*)GetDlgItem(IDC_IP);
	p->ShowWindow(bShow);
	p = (CWnd*)GetDlgItem(IDC_PORT);
	p->ShowWindow(bShow);
	p = (CWnd*)GetDlgItem(IDC_QUEUE1_DEST);
	p->ShowWindow(bShow);
	p = (CWnd*)GetDlgItem(IDC_QUEUE2_DEST);
	p->ShowWindow(bShow);
	p = (CWnd*)GetDlgItem(IDC_DEST);
	p->ShowWindow(bShow);
}

void CJmsUIDlg::ShowTab2(bool bShow)
{
	CWnd* p;
	p = (CWnd*)GetDlgItem(IDC_SHARE_FOLDER);
	p->ShowWindow(bShow);
	p = (CWnd*)GetDlgItem(IDC_MESSAGE);
	p->ShowWindow(bShow);
}

void CJmsUIDlg::EnableTab1(bool bQueue1Connect, bool bQueue2Connect)
{
	CWnd* p;
	p = (CWnd*)GetDlgItem(IDC_QUEUE_PORT);
	p->EnableWindow(!bQueue1Connect && !bQueue2Connect);
	p = (CWnd*)GetDlgItem(IDC_QUEUE_IP);
	p->EnableWindow(!bQueue1Connect && !bQueue2Connect);
	
	p = (CWnd*)GetDlgItem(IDC_QUEUE1_DEST);
	p->EnableWindow(!bQueue1Connect);
	
	p = (CWnd*)GetDlgItem(IDC_QUEUE2_DEST);
	p->EnableWindow(!bQueue2Connect);

	p = (CWnd*)GetDlgItem(IDOK);
	p->EnableWindow(!bQueue1Connect || !bQueue2Connect);
}

void CJmsUIDlg::EnableTab2(bool bQueue1Connect, bool bQueue2Connect)
{
	CWnd* p;
	p = (CWnd*)GetDlgItem(IDC_SHARE_FOLDER);
	p->EnableWindow(bQueue1Connect);
	p = (CWnd*)GetDlgItem(IDC_MESSAGE);
	p->EnableWindow(bQueue2Connect);
}

bool CJmsUIDlg::ValidIP(CString& queueIp)
{
	char ip[30] = {0};
	strcpy(ip, queueIp);

	char part[4] = {0};
	int iPart;

	// 第一部分
	char* juhao1 = ip;
	char* juhao2 = strstr(ip, ".");
	if (juhao2 == NULL)
	{
		AfxMessageBox("IP地址出错！");
		return false;
	}
	strncpy(part, juhao1, juhao2 - juhao1);
	iPart = atoi(part);
	if (iPart > 255 || iPart < 1)
	{
		AfxMessageBox("IP第一部分出错错误");
		return false;
	}

	// 第二部分
	juhao1 = juhao2 + 1;
	juhao2 = strstr(juhao2 + 1, ".");
	strnset(part, 0, 4);
	if (juhao2 == NULL)
	{
		AfxMessageBox("IP地址出错！");
		return false;
	}
	strncpy(part, juhao1, juhao2 - juhao1);
	iPart = atoi(part);
	if (iPart > 255 || iPart < 0)
	{
		AfxMessageBox("IP第二部分出错错误");
		return false;
	}

	// 第三部分
	juhao1 = juhao2 + 1;
	juhao2 = strstr(juhao2 + 1, ".");
	strnset(part, 0, 4);
	if (juhao2 == NULL)
	{
		AfxMessageBox("IP地址出错！");
		return false;
	}
	strncpy(part, juhao1, juhao2 - juhao1);
	iPart = atoi(part);
	if (iPart > 255 || iPart < 0)
	{
		AfxMessageBox("IP第三部分出错错误");
		return false;
	}

	// 第四部分
	juhao1 = juhao2 + 1;
	iPart = atoi(juhao1);
	if (iPart > 255 || iPart < 1)
	{
		AfxMessageBox("IP第四部分出错错误");
		return false;
	}
	return true;
}

void CJmsUIDlg::OnClose() 
{
	if (m_Queue1Thd)
	{
		m_Queue1Thd->stopSend();
		m_Queue1Thd->stop();
		Sleep(1000);
		delete m_Queue1Thd;
		m_Queue1Thd = NULL;
	}

	if (m_Queue2Thd)
	{
		m_Queue2Thd->stopSend();
		m_Queue2Thd->stop();
		Sleep(1000);
		delete m_Queue2Thd;
		m_Queue1Thd = NULL;
	}

	if (m_JmsPublisherMgr)
	{
		m_JmsPublisherMgr->stop();
		Sleep(3000);	
		delete m_JmsPublisherMgr;
		m_JmsPublisherMgr = NULL;
	}

	delete m_FileReport;
	Sleep(3000);
	
	CDialog::OnCancel();
}

void CJmsUIDlg::OnOK() 
{
	UpdateData(TRUE);
	if (!ValidIP(m_Queue_IP))
	{
		return;
	}

	if (m_Queue_Port.IsEmpty())
	{
		AfxMessageBox("Port 不能为空");
		return;
	}

	int port = atoi(m_Queue_Port);
	if (port < 1024)
	{
		AfxMessageBox("端口不能小于1024");
		return;
	}

	if (!m_Queue1_DEST.IsEmpty())
	{
		if (m_Queue1_DEST == m_Queue2_DEST)
		{
			AfxMessageBox("Queue1 Destination 不能 Queue2 Destination 相同");
			return;
		}
		m_bShowFolderConnect = true;
	}
	else
	{
		AfxMessageBox("Queue1 Destination 不能空");
		return;
	}

	if (!m_Queue2_DEST.IsEmpty())
	{
		if (m_Queue2_DEST == m_Queue1_DEST)
		{
			AfxMessageBox("Queue2 Destination 不能 Queue1 Destination 相同");
			return;
		}
		m_bMessageConnect = true;
	}
	else
	{
		AfxMessageBox("Queue2 Destination 不能空");
		return;
	}

	EnableTab1(m_bShowFolderConnect, m_bMessageConnect);

	ConnectJBoss();

	// 启动2个线程
	m_Queue1Thd = new Queue1Thread(m_Queue1_DEST, m_JmsPublisherMgr);
	m_Queue1Thd->start();

	m_Queue2Thd = new Queue2Thread(m_Queue2_DEST, m_JmsPublisherMgr);
	m_Queue2Thd->start();
}
