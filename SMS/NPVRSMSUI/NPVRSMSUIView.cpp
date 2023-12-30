// NPVRSMSUIView.cpp : implementation of the CNPVRSMSUIView class
//

#include "stdafx.h"
#include "NPVRSMSUI.h"

#include "NPVRSMSUIDoc.h"
#include "NPVRSMSUIView.h"

#include "DefineTime.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNPVRSMSUIView

IMPLEMENT_DYNCREATE(CNPVRSMSUIView, CListView)

BEGIN_MESSAGE_MAP(CNPVRSMSUIView, CListView)
	//{{AFX_MSG_MAP(CNPVRSMSUIView)
	ON_COMMAND(ID_DEFINE, OnDefine)
	ON_COMMAND(ID_REFRESH, OnRefresh)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CListView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNPVRSMSUIView construction/destruction

CNPVRSMSUIView::CNPVRSMSUIView()
{
	// TODO: add construction code here

}

CNPVRSMSUIView::~CNPVRSMSUIView()
{
}

BOOL CNPVRSMSUIView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style = m_dwDefaultStyle | LVS_REPORT;
	return CListView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CNPVRSMSUIView drawing

void CNPVRSMSUIView::OnDraw(CDC* pDC)
{
	CNPVRSMSUIDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

void CNPVRSMSUIView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	listSetup();

	if (dbInitail())
	{
		selectAll();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CNPVRSMSUIView printing

BOOL CNPVRSMSUIView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CNPVRSMSUIView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CNPVRSMSUIView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CNPVRSMSUIView diagnostics

#ifdef _DEBUG
void CNPVRSMSUIView::AssertValid() const
{
	CListView::AssertValid();
}

void CNPVRSMSUIView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CNPVRSMSUIDoc* CNPVRSMSUIView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNPVRSMSUIDoc)));
	return (CNPVRSMSUIDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CNPVRSMSUIView message handlers

void CNPVRSMSUIView::listSetup()
{
	CListCtrl& L = GetListCtrl();

	L.InsertColumn(0, "UID");
	L.InsertColumn(1, "CMD");
	L.InsertColumn(2, "PackLen");
	L.InsertColumn(3, "ServiceCode");
	L.InsertColumn(4, "CallNumber");
	L.InsertColumn(5, "SendTime");
	L.InsertColumn(6, "SMSContent");
	L.InsertColumn(7, "SendContent");
	L.InsertColumn(8, "SMSTicpContent");
	L.InsertColumn(9, "LeftContent");
	L.InsertColumn(10, "TicpFinished");
	L.InsertColumn(11, "SmsFinished");

	RECT rect;
	L.GetWindowRect(&rect);
	int wid = rect.right - rect.left;

	L.SetColumnWidth(0, wid*5/100);
	L.SetColumnWidth(1, wid*5/100);
	L.SetColumnWidth(2, wid*10/100);
	L.SetColumnWidth(3, wid*5/100);
	L.SetColumnWidth(4, wid*10/100);
	L.SetColumnWidth(5, wid*15/100);
	L.SetColumnWidth(6, wid*10/100);
	L.SetColumnWidth(7, wid*10/100);
	L.SetColumnWidth(8, wid*10/100);
	L.SetColumnWidth(9, wid*14/100);
	L.SetColumnWidth(10, wid*3/100);
	L.SetColumnWidth(11, wid*3/100);

	L.SetExtendedStyle(LVS_EX_FULLROWSELECT);
}

bool CNPVRSMSUIView::dbInitail()
{
	if (!m_db.ConnectDB("NPVRSMSDB.mdb"))
	{
		return false;
	}
	return true;
}

void CNPVRSMSUIView::selectAll()
{
	CTime current = CTime::GetCurrentTime();
	current = current - CTimeSpan(3, 0, 0, 0);
	
	char time[30];
	memset(time, 0x00, 30*sizeof(char));

	sprintf(time, "%d-%d-%d %d:%d:%d", current.GetYear(),
									   current.GetMonth(),
									   current.GetDay(),
									   0,
									   0,
									   0);

	if (m_db.SelectByTime(time))
	{
		listData();		
	}
}

void CNPVRSMSUIView::listData()
{
	CListCtrl &L = GetListCtrl();

	L.DeleteAllItems();

	int	packageLength;
	int	cmd;
	int	uid;
	char serviceCode[20];
	char callerNumber[20];
	char sendTime[30];
	char SMSContent[100];
	char SendContent[100];
	char leftContent[256];
	char TicpContent[100];
	bool TicpFinished;
	bool SmsFinished;

	memset(serviceCode, 0x00, 20*sizeof(char));
	memset(serviceCode, 0x00, 20*sizeof(char));
	memset(sendTime,	0x00, 30*sizeof(char));
	memset(SMSContent,	0x00, 100*sizeof(char));
	memset(SendContent, 0x00, 100*sizeof(char));
	memset(leftContent, 0x00, 256*sizeof(char));
	memset(TicpContent, 0x00, 100*sizeof(char));

	int index = 0;

	while (m_db.getData(packageLength,
						cmd,
						uid,
						serviceCode,
						callerNumber,
						sendTime,
						SMSContent,
						SendContent,
						leftContent,
						TicpContent,
						TicpFinished,
						SmsFinished))
	{
		char temp[4];
		memset(temp, 0x00, 4*sizeof(char));
		sprintf(temp, "%d", uid);
		L.InsertItem(index, temp);

		memset(temp, 0x00, 4*sizeof(char));
		sprintf(temp, "%d", cmd);
		L.SetItemText(index, 1, temp);

		memset(temp, 0x00, 4*sizeof(char));
		sprintf(temp, "%d", packageLength);
		L.SetItemText(index, 2, temp);

		if (strlen(serviceCode) > 0)
		{
			L.SetItemText(index, 3, serviceCode);
		}

		if (strlen(callerNumber) > 0)
		{
			L.SetItemText(index, 4, callerNumber);
		}

		if (strlen(sendTime) > 0)
		{
			L.SetItemText(index, 5, sendTime);
		}

		if (strlen(SMSContent) > 0)
		{
			L.SetItemText(index, 6, SMSContent);
		}

		if (strlen(SendContent) > 0)
		{
			L.SetItemText(index, 7, SendContent);
		}
		
		if (strlen(leftContent) > 0)
		{
			L.SetItemText(index, 8, leftContent);
		}
		
		if (strlen(TicpContent) > 0)
		{
			L.SetItemText(index, 9, TicpContent);
		}

		if (TicpFinished)
		{
			L.SetItemText(index, 10, "Finished");
		}
		else
		{
			L.SetItemText(index, 10, "UnFinished");
		}
		
		if (SmsFinished)
		{
			L.SetItemText(index, 11, "Finished");
		}
		else
		{
			L.SetItemText(index, 11, "UnFinished");
		}

		// clear bufs
		memset(serviceCode, 0x00, 20*sizeof(char));
		memset(serviceCode, 0x00, 20*sizeof(char));
		memset(sendTime,	0x00, 30*sizeof(char));
		memset(SMSContent,	0x00, 100*sizeof(char));
		memset(SendContent, 0x00, 100*sizeof(char));
		memset(leftContent, 0x00, 256*sizeof(char));
		memset(TicpContent, 0x00, 100*sizeof(char));
	}
}
void CNPVRSMSUIView::OnDefine() 
{
	DefineTime dlg;

	if (dlg.DoModal() == IDOK)
	{
		CTime time = dlg.m_time;

		char ctime[20];
		memset(ctime, 0x00, 20*sizeof(char));

		sprintf(ctime, "%d-%d-%d %d:%d:%d", time.GetYear(),
											time.GetMonth(),
											time.GetDay(),
											0,
											0,
											0);

		if (m_db.SelectByTime(ctime))
		{
			listData();
		}
	}
}

void CNPVRSMSUIView::OnRefresh() 
{
	selectAll();
}
