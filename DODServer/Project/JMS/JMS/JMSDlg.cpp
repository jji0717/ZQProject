// JMSDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "JMS.h"
#include "JMSDlg.h"
#include ".\jmsdlg.h"
#include"IParse.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };



	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CJMSDlg 对话框



CJMSDlg::CJMSDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CJMSDlg::IDD, pParent)
	, m_QueueName(_T("queue/testQueue"))
	, m_TopicName(_T("testtopic1"))
	, m_SendString(_T("Test JMS In C++"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CJMSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_QueueName);
	DDX_Text(pDX, IDC_EDIT2, m_TopicName);
	DDX_Text(pDX, IDC_EDIT3, m_SendString);
}

BEGIN_MESSAGE_MAP(CJMSDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
END_MESSAGE_MAP()


// CJMSDlg 消息处理程序

BOOL CJMSDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将\“关于...\”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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
	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//jms.Init(false);
	//jms.Initialize();
    CString m_ProviderValue="192.168.80.46:1099";
    //CString m_ProviderValue="127.0.0.1:1099";


     CString  m_strQueueConnectionFactoryName="${queue.connectionfactory.name}";

     CString  m_strTopicConnectionFactoryName="${topic.connectionfactory.name}";

     CString m_Providerkey="java.naming.provider.url";
    
	 CString  m_FactoryInitialKey="java.naming.factory.initial";

     CString m_FactoryInitialValue="org.jnp.interfaces.NamingContextFactory";

     CString  m_FactoryurlKey="java.naming.factory.url.pkgs";

     CString  m_FactoryurlValue="org.jboss.naming";
	  
     CString  m_QueueConnectionFactoryKey="queue.connectionfactory.name";

     CString m_QueueConnectionFactoryValue="ConnectionFactory";

     CString m_TopicConnectionFactoryKey="topic.connectionfactory.name";
     
	 CString m_TopicConnectionFactoryValue="ConnectionFactory";
     /*BOOL CActiveJMS::Initialize(CString Providerkey,CString ProviderValue,CString FactoryInitialKey,CString FactoryInitialValue,
		CString QueueConnectionFactoryKey,CString QueueConnectionFactoryValue,CString TopicConnectionFactoryKey, CString TopicConnectionFactoryValue,
		CString strQueueConnectionFactoryName,CString strTopicConnectionFactoryName,CString FactoryurlKey,CString FactoryurlValue)*/
	 
	 /*jms.Initialize(m_Providerkey,m_ProviderValue, 
	                 m_FactoryInitialKey,m_FactoryInitialValue,
                     m_QueueConnectionFactoryKey,m_QueueConnectionFactoryValue,
                     m_TopicConnectionFactoryKey,m_TopicConnectionFactoryValue,
                     m_strQueueConnectionFactoryName,m_strTopicConnectionFactoryName,
                     m_FactoryurlKey,m_FactoryurlValue);*/

     jms.Initialize(m_Providerkey,m_ProviderValue, 
	                 m_FactoryInitialKey,m_FactoryInitialValue,
                     m_QueueConnectionFactoryValue,
                     m_TopicConnectionFactoryValue,
                     m_FactoryurlKey,m_FactoryurlValue);


    MyParse* pMyParse=new  MyParse(&jms);
    MyCommandParse *pMyCommandParse=new MyCommandParse(&jms);
	jms.AddOneQueue("queue/testQueue",pMyParse);
	//jms.AddOneReceiveQueue("queue/testQueue",pMyCommandParse);
	//jms.AddOneQueue("queue/testQueue",pMyCommandParse);


   //MyParse* pMyParse=new  MyParse(&jms);
   // jms.AddOneQueue("queue/testQueue", pMyParse);
  //////////////////////////////////////////////
   //jboss special
//////////////////////////////////////////////////////////////
//jms.Initialize();
    //jms.AddOneQueue("queue/testQueue", pMyParse);
    //queue/testQueue
   // topic/testTopic
    /*jms.AddOneQueue("testqueue1@router1", pMyParse);
	jms.AddOneQueue("testqueue2@router1",pMyParse);
	jms.AddOneTopic("testtopic1",pMyParse);
	jms.AddOneTopic("testtopic2",pMyParse);*/
	//jms.StartConnection(); delete by simin
	//jms.Init(true);
	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CJMSDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CJMSDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CJMSDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CJMSDlg::OnBnClickedButton1()
{
	UpdateData();
	jms.PtopSend(m_SendString.GetBuffer(0),m_QueueName.GetBuffer(0));
}

void CJMSDlg::OnBnClickedButton2()
{
    UpdateData();
    CJMSMessage message(&jms,TRUE,0,MESSAGEMODE_PTOP);
    message.SetDoubleProperty("double",1233);
	message.SetBoolProperty("BOOL",10);
	message.SetFloatProperty("FLOAT",1313.133);
	message.SetIntProperty("INT",123);
	message.SetLongProperty("LONG",1313318);
	message.SetShortProperty("SHORT",131);
	message.SetStringProperty("MESSAGECLASS","COMMAND");
	message.SetRawData(m_SendString.GetBuffer(0));
    jms.PtopSend(&message,"queue/testQueue");
	message.MessageSend("queue/testQueue");

	/*CJMSMessage message(&jms);
    message.CreateQueueMessage();
	message.SetDoubleProperty("double",1233);
	message.SetBoolProperty("BOOL",10);
	message.SetFloatProperty("FLOAT",1313.133);
	message.SetIntProperty("INT",123);
	message.SetLongProperty("LONG",1313318);
	message.SetShortProperty("SHORT",131);
	message.SetStringProperty("STRING","dfdfjdj");
	message.SetRawData(m_SendString.GetBuffer(0));
	jms.PtopSend(&message,"queue/testQueue");
	message.SendMessage("queue/testQueue");
	/*	message.AddDoubleProperty("double",1233);
	message.AddBoolProperty("BOOL",10);
   //message.AddByteProperty("BYTE",'A');
	message.AddFloatProperty("FLOAT",1313.133);
	message.AddIntProperty("INT",123);
	message.AddLongProperty("LONG",1313318);
	message.AddShortProperty("SHORT",131);
	message.AddStringProperty("STRING","dfdfjdj");
	//message.SetRawData(m_SendString.GetBuffer(0));
//	jms.PtopSend(&message,"queue/testQueue");
	//jms.SendToAllTopics(m_SendString.GetBuffer(0));*/

}

void CJMSDlg::OnBnClickedButton3()
{

   UpdateData();
    MyParse* pMyParse=new  MyParse(&jms);
   // jms.AddOneQueue("queue/testQueue", pMyParse);

   // CJMSMessage message(&jms,TRUE,0,MESSAGEMODE_PTOP);

    CJMSMessage message(&jms,"COMMAND","1234");
    message.SetDoubleProperty("double",1233);
	message.SetBoolProperty("BOOL",10);
	message.SetFloatProperty("FLOAT",1313.133);
	message.SetIntProperty("INT",123);
	message.SetLongProperty("LONG",1313318);
	message.SetShortProperty("SHORT",131);
	//message.SetStringProperty("MESSAGECLASS","COMMAND");
	message.SetRawData(m_SendString.GetBuffer(0));
	jms.SyncSend(&message,"queue/testQueue",pMyParse,5);
	//message.SendMessage("queue/testQueue");
	
}
