// SessionViewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SessionView.h"
#include "SessionViewDlg.h"

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
// CSessionViewDlg dialog


CSessionViewDlg::CSessionViewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSessionViewDlg::IDD, pParent), _pSessionView(NULL), _pCommunicator(NULL), totalSize(0), pageSize(20), currPage(1), totalPage(0), clientId(-1)
{
	//{{AFX_DATA_INIT(CSessionViewDlg)
	m_sEndPoint = _T("SessionView:tcp -h 192.168.81.99 -p 6743");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSessionViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSessionViewDlg)
	DDX_Control(pDX, IDC_BUTTON_PREVIOUS, m_btnPrev);
	DDX_Control(pDX, IDC_BUTTON_NEXT, m_btnNext);
	DDX_Control(pDX, IDC_BUTTON_LAST, m_btnLast);
	DDX_Control(pDX, IDC_BUTTON_BEGIN, m_btnBegin);
	DDX_Control(pDX, IDC_LIST_SESSIONVIEW, m_lstViewSess);
	DDX_Text(pDX, IDC_EDIT_CONNECTSTRING, m_sEndPoint);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSessionViewDlg, CDialog)
	//{{AFX_MSG_MAP(CSessionViewDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_BEGIN, OnButtonBegin)
	ON_BN_CLICKED(IDC_BUTTON_PREVIOUS, OnButtonPrevious)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, OnButtonNext)
	ON_BN_CLICKED(IDC_BUTTON_LAST, OnButtonLast)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSessionViewDlg message handlers

BOOL CSessionViewDlg::OnInitDialog()
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
	
	// DO: Add extra initialization here
	m_lstViewSess.SetExtendedStyle(m_lstViewSess.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_lstViewSess.InsertColumn(0, "RtspSession", LVCFMT_LEFT, 100, -1);
	m_lstViewSess.InsertColumn(1, "WeiwooSession", LVCFMT_LEFT, 160, -1);
	m_lstViewSess.InsertColumn(2, "StreamID", LVCFMT_LEFT, 190, -1);
	m_lstViewSess.InsertColumn(3, "PurchaseID", LVCFMT_LEFT, 190, -1);
	m_btnBegin.EnableWindow(FALSE);
	m_btnNext.EnableWindow(FALSE);
	m_btnPrev.EnableWindow(FALSE);
	m_btnLast.EnableWindow(FALSE);

	// DO: init ice run-time
	int i=0;
	Ice::PropertiesPtr props = Ice::createProperties(i, NULL);
	if (NULL != props)
	{
	}
	Ice::InitializationData idt;
	idt.properties = props;
	_pCommunicator=Ice::initialize(idt);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSessionViewDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSessionViewDlg::OnPaint() 
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
HCURSOR CSessionViewDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSessionViewDlg::ShowData(const TianShanS1::SessionDatas& datas)
{
	m_lstViewSess.DeleteAllItems();
	for (int i = 0; i < (int)datas.size(); i ++)
	{
		m_lstViewSess.InsertItem(0, datas[i].ident.name.c_str());
		m_lstViewSess.SetItemText(0, 1, datas[i].srvrSessPrxID.c_str());
		m_lstViewSess.SetItemText(0, 2, datas[i].streamPrxID.c_str());
		m_lstViewSess.SetItemText(0, 3, datas[i].purchasePrxID.c_str());
	}
}

void CSessionViewDlg::ChangeButtonStatus()
{
	if (currPage == 1)
		m_btnBegin.EnableWindow(FALSE);
	else 
		m_btnBegin.EnableWindow(TRUE);

	if (currPage == totalPage)
		m_btnLast.EnableWindow(FALSE);
	else 
		m_btnLast.EnableWindow(TRUE);

	if (currPage + 1 <= totalPage)
		m_btnNext.EnableWindow(TRUE);
	else 
		m_btnNext.EnableWindow(FALSE);

	if (currPage - 1 >= 1)
		m_btnPrev.EnableWindow(TRUE);
	else 
		m_btnPrev.EnableWindow(FALSE);
}

void CSessionViewDlg::OnButtonBegin()
{
	TianShanS1::SessionDatas datas;
	try
	{
		currPage = 1;
		datas = _pSessionView->getRange(pageSize * (currPage - 1) + 1, pageSize * (currPage - 1) + pageSize <= totalSize ? pageSize * (currPage - 1) + pageSize : totalSize, clientId);
	}
	catch (const TianShanS1::ErrorBase& ex)
	{
		char errBuf[200];
		memset(errBuf, 0, sizeof(errBuf));
		_snprintf(errBuf, sizeof(errBuf) - 1, "ConnectServer caught %s: %s", ex.ice_name().c_str(), ex.message.c_str());
		AfxMessageBox(errBuf);
		return;
	}
	catch (const Ice::Exception& ex)
	{
		char errBuf[200];
		memset(errBuf, 0, sizeof(errBuf));
		_snprintf(errBuf, sizeof(errBuf) - 1, "ConnectServer caught %s", ex.ice_name().c_str());
		AfxMessageBox(errBuf);
		return;
	}
	ShowData(datas);
	ChangeButtonStatus();
}

void CSessionViewDlg::OnButtonPrevious()
{
	TianShanS1::SessionDatas datas;
	try
	{
		currPage --;
		if (currPage < 1)
			currPage = 1;
		datas = _pSessionView->getRange(pageSize * (currPage - 1) + 1, pageSize * (currPage - 1) + pageSize <= totalSize ? pageSize * (currPage - 1) + pageSize : totalSize, clientId);
	}
	catch (const TianShanS1::ErrorBase& ex)
	{
		char errBuf[200];
		memset(errBuf, 0, sizeof(errBuf));
		_snprintf(errBuf, sizeof(errBuf) - 1, "ConnectServer caught %s: %s", ex.ice_name().c_str(), ex.message.c_str());
		AfxMessageBox(errBuf);
		return;
	}
	catch (const Ice::Exception& ex)
	{
		char errBuf[200];
		memset(errBuf, 0, sizeof(errBuf));
		_snprintf(errBuf, sizeof(errBuf) - 1, "ConnectServer caught %s", ex.ice_name().c_str());
		AfxMessageBox(errBuf);
		return;
	}
	ShowData(datas);
	ChangeButtonStatus();
}

void CSessionViewDlg::OnButtonNext()
{
	TianShanS1::SessionDatas datas;
	try
	{
		currPage ++;
		if (currPage > totalPage)
			currPage = totalPage;
		datas = _pSessionView->getRange(pageSize * (currPage - 1) + 1, pageSize * (currPage - 1) + pageSize <= totalSize ? pageSize * (currPage - 1) + pageSize : totalSize, clientId);
	}
	catch (const TianShanS1::ErrorBase& ex)
	{
		char errBuf[200];
		memset(errBuf, 0, sizeof(errBuf));
		_snprintf(errBuf, sizeof(errBuf) - 1, "ConnectServer caught %s: %s", ex.ice_name().c_str(), ex.message.c_str());
		AfxMessageBox(errBuf);
		return;
	}
	catch (const Ice::Exception& ex)
	{
		char errBuf[200];
		memset(errBuf, 0, sizeof(errBuf));
		_snprintf(errBuf, sizeof(errBuf) - 1, "ConnectServer caught %s", ex.ice_name().c_str());
		AfxMessageBox(errBuf);
		return;
	}
	ShowData(datas);
	ChangeButtonStatus();
}

void CSessionViewDlg::OnButtonLast()
{
	TianShanS1::SessionDatas datas;
	try
	{
		currPage = totalPage;
		datas = _pSessionView->getRange(pageSize * (currPage - 1) + 1, totalSize, clientId);
	}
	catch (const TianShanS1::ErrorBase& ex)
	{
		char errBuf[200];
		memset(errBuf, 0, sizeof(errBuf));
		_snprintf(errBuf, sizeof(errBuf) - 1, "ConnectServer caught %s: %s", ex.ice_name().c_str(), ex.message.c_str());
		AfxMessageBox(errBuf);
		return;
	}
	catch (const Ice::Exception& ex)
	{
		char errBuf[200];
		memset(errBuf, 0, sizeof(errBuf));
		_snprintf(errBuf, sizeof(errBuf) - 1, "ConnectServer caught %s", ex.ice_name().c_str());
		AfxMessageBox(errBuf);
		return;
	}
	ShowData(datas);
	ChangeButtonStatus();
}

void CSessionViewDlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData();

	if (ConnectServer(m_sEndPoint))
	{
		Ice::Int oIdent;
		totalSize = _pSessionView->getAllContext(clientId, oIdent);
		clientId = oIdent;
		pageSize = 5;
		totalPage = totalSize / pageSize;
		if (totalSize % pageSize != 0)
			totalPage ++;
		currPage = 1;
		OnButtonBegin();
	}
	
//	CDialog::OnOK();
}

void CSessionViewDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	try
	{
	if (NULL != _pSessionView)
	{
		_pSessionView->unregister(clientId);
	}
	}
	catch (TianShanS1::ErrorBase& ex)
	{
		char errBuf[200];
		memset(errBuf, 0, sizeof(errBuf));
		_snprintf(errBuf, sizeof(errBuf) - 1, "unregister caught %s: %s", ex.ice_name().c_str(), ex.message.c_str());
		AfxMessageBox(errBuf);
	}
	catch (Ice::Exception& ex)
	{
		char errBuf[200];
		memset(errBuf, 0, sizeof(errBuf));
		_snprintf(errBuf, sizeof(errBuf) - 1, "unregister caught %s", ex.ice_name().c_str());
		AfxMessageBox(errBuf);
	}
	
	CDialog::OnCancel();
}

bool CSessionViewDlg::ConnectServer(const CString& connStr)
{
	char connBuf[200];
	memset(connBuf, 0, sizeof(connBuf));
	_snprintf(connBuf, sizeof(connBuf) - 1, "%s", connStr);
	try
	{
	_pSessionView = TianShanS1::SessionViewPrx::checkedCast(_pCommunicator->stringToProxy(connBuf));
	}
	catch (const Ice::Exception& ex)
	{
		char errBuf[200];
		memset(errBuf, 0, sizeof(errBuf));
		_snprintf(errBuf, sizeof(errBuf) - 1, "ConnectServer caught %s", ex.ice_name().c_str());
		AfxMessageBox(errBuf);
		return false;
	}
	return true;
}

