// ChPubDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ChPub.h"
#include "ChPubDlg.h"
#include "ConnectDlg.h"
#include "ChannelDlg.h"
#include "ItemDlg.h"
#include <typeinfo.h>

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
// CChPubDlg dialog

CChPubDlg::CChPubDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChPubDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChPubDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CChPubDlg::~CChPubDlg()
{
	if(m_isConnected)
	{
		g_Client.disconnect();
		m_isConnected = false;
	}
}

void CChPubDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChPubDlg)
	DDX_Control(pDX, IDC_TREE, m_Channels);
	//}}AFX_DATA_MAP

	// add my own controls here 
	DDX_Control(pDX, IDC_CONNECT, m_btnConnect);
	DDX_Control(pDX, IDC_REFRESH, m_btnRefresh);
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_EDIT, m_btnEdit);
	DDX_Control(pDX, IDC_REMOVE, m_btnRemove);
	DDX_Control(pDX, IDC_INFO, m_btnInfo);
}

BEGIN_MESSAGE_MAP(CChPubDlg, CDialog)
	//{{AFX_MSG_MAP(CChPubDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECT, OnConnect)
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_INFO, OnInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChPubDlg message handlers

BOOL CChPubDlg::OnInitDialog()
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
	
	// initialization for buttons
	short	shBtnColor = 30;
	m_btnConnect.SetIcon(IDI_CONNECT, (int)BTNST_AUTO_GRAY);
	m_btnConnect.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, shBtnColor);
	m_btnConnect.SetTooltipText("Connect");
	m_btnRefresh.SetIcon(IDI_REFRESH, (int)BTNST_AUTO_GRAY);
	m_btnRefresh.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, shBtnColor);
	m_btnRefresh.SetTooltipText("Refresh");
	m_btnAdd.SetIcon(IDI_ADD, (int)BTNST_AUTO_GRAY);
	m_btnAdd.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, shBtnColor);
	m_btnAdd.SetTooltipText("Add");
	m_btnEdit.SetIcon(IDI_EDIT, (int)BTNST_AUTO_GRAY);
	m_btnEdit.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, shBtnColor);
	m_btnEdit.SetTooltipText("Edit");
	m_btnRemove.SetIcon(IDI_REMOVE, (int)BTNST_AUTO_GRAY);
	m_btnRemove.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, shBtnColor);
	m_btnRemove.SetTooltipText("Remove");
	m_btnInfo.SetIcon(IDI_INFO, (int)BTNST_AUTO_GRAY);
	m_btnInfo.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, shBtnColor);
	m_btnInfo.SetTooltipText("Info");
	
	// initialization for tree
	m_treeImgList.Create(16, 16, ILC_COLOR8, 4, 4);
	for (int i=0; i<3; i++)
	{	
		CBitmap bm;	bm.LoadBitmap(IDB_LIST1+i);
		m_treeImgList.Add(&bm, RGB(0, 0, 0));
		bm.DeleteObject();
	}
	m_Channels.SetImageList(&m_treeImgList, TVSIL_NORMAL);
	m_Channels.InsertItem(ROOTSTR,0,0);

	m_isConnected = false;
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CChPubDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CChPubDlg::OnPaint() 
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
HCURSOR CChPubDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CChPubDlg::OnConnect() 
{
	try
	{
		if(!m_isConnected)
		{
			CConnectDlg	connDlg;
			int result = connDlg.DoModal();
			if(result==IDCANCEL)
			{
				return;
			}

			m_isConnected = true;
			m_btnConnect.SetIcon(IDI_DISCONNECT, (int)BTNST_AUTO_GRAY);
			m_btnConnect.SetTooltipText("Disconnect");
			SetForegroundWindow();
			
			::std::string errmsg;
			errmsg = g_Client.list(m_Channels, m_Channels.GetRootItem());
			if(!errmsg.empty())
			{
				CString strError;
				strError.Format("Can not list channels, error:%s!", errmsg.c_str());
				AfxMessageBox((LPCSTR)strError, MB_OK | MB_ICONWARNING);
				return;
			}
		}
		else
		{
			if(IDYES == AfxMessageBox("Disconnect from server? (local server will be shutdown also if it was started)", MB_YESNO | MB_ICONQUESTION))
			{
				// disconnect client
				g_Client.disconnect();
				m_isConnected = false;

				m_Channels.DeleteAllItems();
				m_Channels.InsertItem(ROOTSTR, 0, 0);

				m_btnConnect.SetIcon(IDI_CONNECT, (int)BTNST_AUTO_GRAY);
				m_btnConnect.SetTooltipText("Connect");
			}
		}
	}
	catch(::Ice::Exception e)
	{
		AfxMessageBox("Ice error!", MB_OK | MB_ICONWARNING);
	}
	catch(...) 
	{
	}
}

void CChPubDlg::OnRefresh() 
{
	try
	{
		if(m_isConnected)
		{
			::std::string errmsg;
			errmsg = g_Client.list(m_Channels, m_Channels.GetRootItem());
			if(!errmsg.empty())
			{
				CString strError;
				strError.Format("Can not list channels, error:%s!", errmsg.c_str());
				AfxMessageBox((LPCSTR)strError, MB_OK | MB_ICONWARNING);
				return;
			}
		}
	}
	catch(::Ice::Exception e)
	{
		AfxMessageBox("Ice error!", MB_OK | MB_ICONWARNING);
	}
	catch(...) 
	{
	}
}

void CChPubDlg::OnAdd() 
{
	try
	{
		if(!m_isConnected)
			return;

		HTREEITEM hCurr = m_Channels.GetSelectedItem();
		if(hCurr==NULL)
			return;

		if(!g_IsValidNode(m_Channels, hCurr))
			return;

		HTREEITEM hParent = m_Channels.GetParentItem(hCurr);
		HTREEITEM hRoot = m_Channels.GetRootItem();
		if(hParent==hRoot)
		{
			// current item is a channel
			CItemDlg itemDlg(FALSE);
			int result = itemDlg.DoModal();
			if(result==IDOK && !itemDlg.m_valContent.IsEmpty() && !itemDlg.m_valStart.IsEmpty() && !itemDlg.m_valExpiration.IsEmpty())
			{
				::std::string errmsg;
				::std::string start, expiration;
				BOOL playable, forceNormalSpeed, spliceIn, spliceOut;
				long inTimeOffset, outTimeOffset;
				start = itemDlg.m_valStart;
				expiration = itemDlg.m_valExpiration;
				playable = itemDlg.m_valPlayable;
				forceNormalSpeed = itemDlg.m_valForceNormalSpeed;
				spliceIn = itemDlg.m_valSpliceIn;
				spliceOut = itemDlg.m_valSpliceOut;
				inTimeOffset = itemDlg.m_valInTimeOffSet;
				outTimeOffset = itemDlg.m_valOutTimeOffSet;
				errmsg = g_Client.appendItem(m_Channels, hCurr, (LPCSTR)itemDlg.m_valContent, start, expiration, playable, forceNormalSpeed, inTimeOffset, outTimeOffset, spliceIn, spliceOut);
				if(!errmsg.empty())
				{
					CString strError;
					strError.Format("Can not add program item, error:%s!", errmsg.c_str());
					AfxMessageBox((LPCSTR)strError, MB_OK | MB_ICONWARNING);
					return;
				}

			}
		}
		else if(hParent==NULL)
		{
			// current item is the root
			CChannelDlg chDlg(FALSE);
			int result = chDlg.DoModal();
			if(result==IDOK && !chDlg.m_valName.IsEmpty() && !chDlg.m_valDesc.IsEmpty())
			{
				::std::string errmsg;
				errmsg = g_Client.publish(m_Channels, m_Channels.GetRootItem(), (LPCSTR)chDlg.m_valName, (LPCSTR)chDlg.m_valDesc);
				if(!errmsg.empty())
				{
					CString strError;
					strError.Format("Can not add channel, error:%s!", errmsg.c_str());
					AfxMessageBox((LPCSTR)strError, MB_OK | MB_ICONWARNING);
					return;
				}

				HTREEITEM hFirstCh = m_Channels.GetChildItem(hRoot);
				if(!g_IsValidNode(m_Channels, hFirstCh))
					m_Channels.DeleteItem(hFirstCh);
			}
		}
		else if(m_Channels.GetRootItem()==m_Channels.GetParentItem(hParent))		
		{
			// current item is a program item
		}
	}
	catch(::Ice::Exception e)
	{
		AfxMessageBox("Ice error!", MB_OK | MB_ICONWARNING);
	}
	catch(...) 
	{
	}
}

void CChPubDlg::OnEdit() 
{
	try
	{
		if(!m_isConnected)
			return;
		
		HTREEITEM hCurr = m_Channels.GetSelectedItem();
		if(hCurr==NULL)
			return;
		
		if(!g_IsValidNode(m_Channels, hCurr))
			return;
		
		HTREEITEM hParent = m_Channels.GetParentItem(hCurr);
		HTREEITEM hRoot = m_Channels.GetRootItem();
		if(hParent==hRoot)
		{
			// current item is a channel
			::std::string tmpName, tmpDesc;
			::std::string errmsg = g_Client.open(m_Channels, hCurr, tmpName, tmpDesc);
			if(!errmsg.empty())
			{
				CString strError;
				strError.Format("Can not get channel information, error:%s!", errmsg.c_str());
				AfxMessageBox((LPCSTR)strError, MB_OK | MB_ICONWARNING);
				return;
			}

			m_Channels.Expand(hCurr, TVE_COLLAPSE);
			
			CChannelDlg channelDlg(TRUE);
			channelDlg.m_valName = tmpName.c_str();
			channelDlg.m_valDesc = tmpDesc.c_str();
			int result = channelDlg.DoModal();
		}
		else if(hParent==NULL)
		{
			// current item is the root
		}
		else if(m_Channels.GetRootItem()==m_Channels.GetParentItem(hParent))		
		{
			// current item is a program item
			::std::string tmpContent;
			::std::string tmpStart, tmpExpiration;

			BOOL tmpPlayable, tmpForceNormalSpeed, tmpSpliceIn, tmpSpliceOut;
			long tmpInTimeOffset, tmpOutTimeOffset;
//			char strStart[32]={0}, strExpiration[32]={0};
			::std::string errmsg = g_Client.findItem(m_Channels, hCurr, tmpContent, tmpStart, tmpExpiration,
				tmpPlayable,tmpForceNormalSpeed,tmpInTimeOffset,tmpOutTimeOffset,tmpSpliceIn,tmpSpliceOut);
			if(!errmsg.empty())
			{
				CString strError;
				strError.Format("Can not get program information, error:%s!", errmsg.c_str());
				AfxMessageBox((LPCSTR)strError, MB_OK | MB_ICONWARNING);
				return;
			}

			CItemDlg itemDlg(FALSE);
			itemDlg.m_valContent = tmpContent.c_str();
			itemDlg.m_valStart = tmpStart.c_str();
			itemDlg.m_valExpiration = tmpExpiration.c_str();
			itemDlg.m_valPlayable = tmpPlayable;
			itemDlg.m_valForceNormalSpeed = tmpForceNormalSpeed;
			itemDlg.m_valSpliceIn = tmpSpliceIn;
			itemDlg.m_valSpliceOut = tmpSpliceOut;
			itemDlg.m_valInTimeOffSet = tmpInTimeOffset;
			itemDlg.m_valOutTimeOffSet =tmpOutTimeOffset;
			int result = itemDlg.DoModal();
			if(result==IDOK && !itemDlg.m_valContent.IsEmpty() && !itemDlg.m_valStart.IsEmpty() && !itemDlg.m_valExpiration.IsEmpty())
			{
				::std::string errmsg;
				::std::string start, expiration;
				BOOL playable, forceNormalSpeed, spliceIn, spliceOut;
				long inTimeOffset, outTimeOffset;
				start = itemDlg.m_valStart;
				expiration = itemDlg.m_valExpiration;
				playable = itemDlg.m_valPlayable;
				forceNormalSpeed = itemDlg.m_valForceNormalSpeed;
				spliceIn = itemDlg.m_valSpliceIn;
				spliceOut = itemDlg.m_valSpliceOut;
				inTimeOffset = itemDlg.m_valInTimeOffSet;
				outTimeOffset = itemDlg.m_valOutTimeOffSet;
				errmsg = g_Client.replaceItem(m_Channels, hCurr, tmpContent.c_str(), (LPCSTR)itemDlg.m_valContent, start, expiration
					,playable,forceNormalSpeed,inTimeOffset,outTimeOffset,spliceIn,spliceOut);
				if(!errmsg.empty())
				{
					CString strError;
					strError.Format("Can not replace program item, error:%s!", errmsg.c_str());
					AfxMessageBox((LPCSTR)strError, MB_OK | MB_ICONWARNING);
					return;
				}
				
			}
		}
	}
	catch(::Ice::Exception e)
	{
		AfxMessageBox("Ice error!", MB_OK | MB_ICONWARNING);
	}
	catch(...) 
	{
	}
}

void CChPubDlg::OnRemove() 
{
	try
	{
		if(!m_isConnected)
			return;
		
		HTREEITEM hCurr = m_Channels.GetSelectedItem();
		if(hCurr==NULL)
			return;
		
		if(!g_IsValidNode(m_Channels, hCurr))
			return;
		
		HTREEITEM hParent = m_Channels.GetParentItem(hCurr);
		HTREEITEM hRoot = m_Channels.GetRootItem();
		if(hParent==hRoot)
		{
			// current item is a channel
			if(IDYES == AfxMessageBox("Remove channel and all programs within it?", MB_YESNO | MB_ICONQUESTION))
			{
				::std::string errmsg = g_Client.destroy(m_Channels, hCurr);
				if(!errmsg.empty())
				{
					CString strError;
					strError.Format("Can not remove channel, error:%s!", errmsg.c_str());
					AfxMessageBox((LPCSTR)strError, MB_OK | MB_ICONWARNING);
					return;
				}
			}
		}
		else if(hParent==NULL)
		{
			// current item is the root
		}
		else if(m_Channels.GetRootItem()==m_Channels.GetParentItem(hParent))		
		{
			// current item is a program item
			if(IDYES == AfxMessageBox("Remove this program and all programs early than it?", MB_YESNO | MB_ICONQUESTION))
			{
				::std::string errmsg = g_Client.expireItem(m_Channels, hCurr);
				if(!errmsg.empty())
				{
					CString strError;
					strError.Format("Can not remove program, error:%s!", errmsg.c_str());
					AfxMessageBox((LPCSTR)strError, MB_OK | MB_ICONWARNING);
					return;
				}
			}
		}
	}
	catch(::Ice::Exception e)
	{
		AfxMessageBox("Ice error!", MB_OK | MB_ICONWARNING);
	}
	catch(...) 
	{
	}
}

void CChPubDlg::OnInfo() 
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
	
//	for(int i=0;i<100;i++)
//		g_Client.createPurchase();
}

