// DCATestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DCATest.h"
#include "DCATestDlg.h"
#include ".\dcatestdlg.h"
#include "Markup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString g_strEXEPath;
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


// CDCATestDlg dialog



CDCATestDlg::CDCATestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDCATestDlg::IDD, pParent)
	, m_strIPPort(_T("10.20.5.3:1099"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDCATestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_strIPPort);
}

BEGIN_MESSAGE_MAP(CDCATestDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CDCATestDlg message handlers

BOOL CDCATestDlg::OnInitDialog()
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
	m_jms=NULL;
	m_Parse=NULL;

	CString        strCurDir;
	char           sModuleName[MAX_PATH];
	DWORD dSize = GetModuleFileName(NULL,sModuleName,MAX_PATH);
	sModuleName[dSize] = '\0';
	strCurDir = sModuleName;
	int nIndex = strCurDir.ReverseFind('\\');
	strCurDir = strCurDir.Left(nIndex); //end with "\\"

	g_strEXEPath=strCurDir;

	strCurDir=strCurDir+"\\"+"DCATest.log";	
	ClogEstablishSettings(strCurDir, LOGMAXLEVEL, LOGMAXSIZE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDCATestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CDCATestDlg::OnPaint() 
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
HCURSOR CDCATestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDCATestDlg::OnBnClickedButton1()
{

	if (m_jms)
	{
		AfxMessageBox("Connect JBOSS  has successful !");
		return ;
	}
	// connect jboss
	CString sTmp;

	UpdateData();

	sTmp=m_strIPPort;
		//.Format("10.20.5.3:1099");
	m_jms=new CJMS(sTmp.GetBuffer(0));

	if(m_jms->StartConnect()==TRUE)
	{
		Clog( LOG_ERROR,_T("TetsDlg Connect JBOSS successful.(%s)"),sTmp);
	}
	else
	{
		Clog( LOG_DEBUG,_T("Connect JBOSS error.(%s)"),sTmp );
		Sleep(5);
		delete m_jms;
		m_jms=NULL;		

		AfxMessageBox("Connect JBOSS error");
		return ;
	}

	m_Parse=new  CJMSParser(m_jms,NULL);



	sTmp.Format("queue/queue_cf");
	if(m_jms->AddQueueOrTopic(TRUE,sTmp.GetBuffer(0),INT_ONLY_RECEIVE,m_Parse)==FALSE)
	{
		Clog( LOG_ERROR,_T("m_jms.AddOneSendQueue (%s) error"), sTmp);
		delete m_Parse;
		m_Parse=NULL;
		AfxMessageBox("Connect AddOneSendQueue error queue_cf");
		return ;
	}
	Clog( LOG_DEBUG,_T("The Queue named (%s) is registed in jboss \n") ,sTmp);



	sTmp.Format("queue/queue_DOD");
	if(m_jms->AddQueueOrTopic(TRUE,sTmp.GetBuffer(0),INT_ONLY_SEND,m_Parse)==FALSE)
	{
		Clog( LOG_ERROR,_T("m_jms.AddOneSendQueue (%s) error"), sTmp);
		delete m_Parse;
		m_Parse=NULL;
		AfxMessageBox("Connect AddOneSendQueue error queue_DOD");
		return ;

	}
	Clog( LOG_DEBUG,_T("The Queue named (%s) is registed in jboss \n") ,sTmp);
	AfxMessageBox("Connect JBOSS OK");

	SetTimer(0,500,NULL);

}

void CDCATestDlg::OnBnClickedButton2()
{
//send share folder mssage

	if (m_jms == NULL)
	{
		AfxMessageBox("Please Connect JBOSS");
		return ;
	}
	CFileDialog dlg( TRUE );
	if( dlg.DoModal() != IDOK )
		return ;

	CString str;	CString sTmp;

	CMarkup m_XmlDOM;

	CString sFileName = dlg.GetFileName();
	if(m_XmlDOM.Load(sFileName) == FALSE)
	{
		AfxMessageBox("Load sFileName :xml format error");
		return ;
	}
	str=m_XmlDOM.GetDoc();

	sTmp.Format("queue/queue_DOD");
	if (m_jms->SendMsg(TRUE,sTmp.GetBuffer(0),str.GetBuffer(0),"MESSAGECODE",3003,"MESSAGECLASS","NOTIFICATION")==FALSE)
	{
		Clog( LOG_DEBUG,_T("PtopSend( send share folder mssage, error "));
		AfxMessageBox("send share folder mssage, error");
		return ;
	}

	Clog( LOG_DEBUG,_T("PtopSend :send share folder mssage ok"));
	AfxMessageBox("send share folder mssage ok");

}
CString CDCATestDlg::GetCurrDateTime()
{
	SYSTEMTIME time; 
	GetLocalTime(&time);
	CString sTime;
	sTime.Format("%04d%02d%02d %02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	return sTime;
}
void CDCATestDlg::OnBnClickedButton3()
{
	//	send msg_channel message

	if (m_jms == NULL)
	{
		AfxMessageBox("Please Connect JBOSS");
		return ;
	}

	CFileDialog dlg( TRUE );
	if( dlg.DoModal() != IDOK )
		return ;

	CString str;	CString sTmp;

	CMarkup m_XmlDOM;

	CString sFileName = dlg.GetFileName();
	if(m_XmlDOM.Load(sFileName) == FALSE)
	{
		AfxMessageBox("Load sFileName :xml format error");
		return ;
	}
	str=m_XmlDOM.GetDoc();

	sTmp.Format("queue/queue_DOD");
	if (m_jms->SendMsg(TRUE,sTmp.GetBuffer(0),str.GetBuffer(0),"MESSAGECODE",3004,"MESSAGECLASS","NOTIFICATION")==FALSE)
	{
		Clog( LOG_DEBUG,_T("PtopSend( send msg_channel mssage, error "));
		AfxMessageBox("send msg_channel, error");
		return ;
	}

	Clog( LOG_DEBUG,_T("PtopSend :send msg_channel ok"));

	AfxMessageBox("send msg_channel mssage ok");

}

void CDCATestDlg::OnBnClickedCancel()
{
	if(m_jms)
	{
		delete m_jms;
		m_jms=NULL;
	}
	if(m_Parse)
	{
		delete m_Parse;
		m_Parse=NULL;
	}
	Clog(LOG_DEBUG,"delete m_jms ok! exits OK!");
	OnCancel();
}

void CDCATestDlg::OnBnClickedButton4()
{
//get path;

	char szFileName[MAX_PATH];
	char m_szAddress[MAX_PATH];
	char m_szPath[MAX_PATH];

	strcpy(szFileName,"\\\\192.168.80.34\\d$\\Inetpub\\ftproot\\back.mpg");
	SplitURLName( szFileName, m_szAddress, m_szPath ); 

	Clog(LOG_DEBUG,"szFileName :%s ",szFileName);
	Clog(LOG_DEBUG,"m_szAddress :%s ",m_szAddress);
	Clog(LOG_DEBUG,"m_szPath :%s ",m_szPath);


	strcpy(szFileName,"e:\\Inetpub\\ftproot\\back.mpg");
	SplitURLName( szFileName, m_szAddress, m_szPath ); 

	Clog(LOG_DEBUG,"szFileName :%s ",szFileName);
	Clog(LOG_DEBUG,"m_szAddress :%s ",m_szAddress);
	Clog(LOG_DEBUG,"m_szPath :%s ",m_szPath);


	strcpy(szFileName,"\\\\192.168.80.34:8080\\d$\\Inetpub\\ftproot\\back.mpg");
	int nPort=0;
	SplitURLName( szFileName, m_szAddress, m_szPath,&nPort ); 

	Clog(LOG_DEBUG,"szFileName :%s ",szFileName);
	Clog(LOG_DEBUG,"m_szAddress :%s :port=%d",m_szAddress,nPort);
	Clog(LOG_DEBUG,"m_szPath :%s ",m_szPath);


	strcpy(szFileName,"ftp://10.10.1.26:9012/004044fa");

	SplitURLName( szFileName, m_szAddress, m_szPath,&nPort ); 

	Clog(LOG_DEBUG,"szFileName :%s ",szFileName);
	Clog(LOG_DEBUG,"m_szAddress :%s :port=%d",m_szAddress,nPort);
	Clog(LOG_DEBUG,"m_szPath :%s ",m_szPath);

}

bool CDCATestDlg::SplitURLName( char * szUNCName, char * szAddress, char * szDirectory, int* nPort )
{

	if( (szUNCName == NULL) || (szAddress == NULL) || (szDirectory == NULL) )
		return false;

	_tcscpy( szAddress, _T("") );
	_tcscpy( szDirectory, _T("") );

	char szTmp[MAX_PATH];
	int iLen = _tcslen( szUNCName );

	// discard "PATH \\"
	bool bHaveSlash = false;
	int i=0;
	for( ; i<iLen-2; i++ )
	{
		if(( szUNCName[i] == '\\' && szUNCName[i+1] == '\\' ) || ( szUNCName[i] == '/' && szUNCName[i+1] == '/' ))
		{
			strcpy( szTmp, szUNCName+i+2 );
			strcpy( szAddress, szTmp );
			bHaveSlash = true;
			break;
		}
	}
	if(bHaveSlash )
	{		
		//find out szAddress
		iLen = _tcslen( szAddress );
		bHaveSlash = false;
		i=0;
		for( ; i<iLen-1; i++ )
		{
			if(szAddress[i] == '\\' || szAddress[i] == '/' )
			{
				strcpy( szTmp, szAddress+i+1 );
				strcpy( szDirectory, szTmp);
				szAddress[i] = 0;
				Clog( 1, "Enter SplitURLName - szAddress: %s.", szAddress );
				bHaveSlash = true;
				break;
			}
		}
		//find out Directory

		iLen = _tcslen( szDirectory );
		bHaveSlash = false;
		i=iLen-1;
		for( ; i>0; i-- )
		{
			if(szDirectory[i] == '\\' || szDirectory[i] == '/'  )
			{
				//	strcpy( szTmp, szAddress+i+1 );
				szDirectory[i] = 0;
				Clog( 1, "Enter SplitURLName - szDirectory: %s.", szDirectory );
				bHaveSlash = true;
				break;
			}
		}
	}
	else
	{
		//no szAddress	
		//find out Directory
		iLen = _tcslen( szUNCName );
		bHaveSlash = false;
		i=iLen-1;
		for( ; i>0; i-- )
		{
			if(szUNCName[i] == '\\'  || szUNCName[i] == '/')
			{
				strcpy( szDirectory, szUNCName );
				szDirectory[i] = 0;		
				Clog( 1, "Enter SplitURLName no Address - szDirectory: %s.", szDirectory );
				bHaveSlash = true;
				break;
			}
		}
	}
	// split address and port.
	if( nPort != NULL )
	{
		TCHAR szPort[8];
		iLen = _tcslen( szAddress );
		bHaveSlash = false;
		*nPort = -1;
		i=0;
		for( ; i<iLen; i++ )
		{
			if( szAddress[i] == ':' )
			{
				strcpy( szPort, szAddress+i+1);
				szAddress[i] = 0;
				*nPort = _ttoi( szPort );
				bHaveSlash = true;
				break;
			}
		}
	}

	if( szDirectory[strlen(szDirectory)-1] == '\\' 
		|| szDirectory[strlen(szDirectory)-1] == '/' )
		szDirectory[strlen(szDirectory)-1] = 0;
	return true;
}


void CDCATestDlg::OnTimer(UINT nIDEvent)
{

	CString str;	CString sTmp;

	CMarkup m_XmlDOM;

	if(m_XmlDOM.Load("config_sharefolder.xml") == FALSE)
	{
		AfxMessageBox("Load sFileName :xml format error");
		return ;
	}
	str=m_XmlDOM.GetDoc();

	sTmp.Format("queue/queue_DOD");
	if (m_jms->SendMsg(TRUE,sTmp.GetBuffer(0),str.GetBuffer(0),"MESSAGECODE",3003,"MESSAGECLASS","NOTIFICATION")==FALSE)
	{
		Clog( LOG_DEBUG,_T("PtopSend( send share folder mssage, error "));
		AfxMessageBox("send share folder mssage, error");
		return ;
	}

	Clog( LOG_DEBUG,_T("PtopSend :send share folder mssage ok"));

	CDialog::OnTimer(nIDEvent);
}
