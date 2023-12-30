// CodManDoc.cpp : implementation of the BroadcastChPPTestDoc class
//

#include "stdafx.h"
#include "BroadcastChPPTest.h"

#include "BroadcastChPPTestDoc.h"
#include "BroadcastChPPTestView.h"
#include "ChildFrm.h"

#include "ChannelTV.h"
#include "PropertyView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// BroadcastChPPTestDoc

IMPLEMENT_DYNCREATE(BroadcastChPPTestDoc, CDocument)

BEGIN_MESSAGE_MAP(BroadcastChPPTestDoc, CDocument)
	//{{AFX_MSG_MAP(BroadcastChPPTestDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BroadcastChPPTestDoc construction/destruction

BroadcastChPPTestDoc::BroadcastChPPTestDoc()
{
	// TODO: add one-time construction code here
	_BcastEndPoint = _T("");
	_publisherPrx = NULL;
}

BroadcastChPPTestDoc::~BroadcastChPPTestDoc()
{
}

/////////////////////////////////////////////////////////////////////////////
// BroadcastChPPTestDoc serialization

void BroadcastChPPTestDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// BroadcastChPPTestDoc diagnostics

#ifdef _DEBUG
void BroadcastChPPTestDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void BroadcastChPPTestDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// BroadcastChPPTestDoc commands

BOOL BroadcastChPPTestDoc::OnNewDocument() 
{
	// TODO: Add your specialized code here and/or call the base class
	char szBuf[1024];
	szBuf[sizeof(szBuf) - 1] = '\0';

	INT_PTR ret = _openCod.DoModal();
	if (ret != IDOK)
		return FALSE;

	_BcastEndPoint = _openCod.m_strBcastChEndPoint;

	SetTitle((LPCTSTR)_BcastEndPoint);

	// 获得publisher代理对象
	try
	{
		_snprintf(szBuf, sizeof(szBuf) - 1, "%s\0", _openCod.m_strBcastChEndPoint);
		std::string bcastendpoint = szBuf;
		::Ice::CommunicatorPtr& com = (((BroadcastChPPTestApp*)AfxGetApp())->_communicator);
		::Ice::ObjectPrx objprx = com->stringToProxy(bcastendpoint);
		_publisherPrx = TianShanIce::Application::Broadcast::BcastPublisherPrx::checkedCast(objprx);
	}
	catch (const Ice::Exception& ex)
	{
		_snprintf(szBuf, sizeof(szBuf) - 1, "connect broadcast channel publisher caught %s", ex.ice_name().c_str());
		AfxMessageBox(szBuf, MB_OK);
		return FALSE;
	}
	catch(...)
	{
		int nret = GetLastError();
		return FALSE;
	}

	// 获得视图对象
	BroadcastChPPTestView* pView = NULL;
	POSITION pos = GetFirstViewPosition();
	if (NULL != pos)
		pView = (BroadcastChPPTestView*)GetNextView(pos);
	if (NULL == pView)
	{
		AfxMessageBox("没有与文档关联的视图对象");
		return FALSE;
	}
	
	// 用视图对象进而获得框架对象
	CChildFrame* pFrame = NULL;
	pFrame = (CChildFrame*)pView->GetParentFrame();
	if (NULL == pFrame)
	{
		AfxMessageBox("没有与文档关联的框架对象");
		return FALSE;
	}

	CChannelTV* pChnlView = pFrame->GetChannelView();
	CTreeCtrl& treeCtrl = pChnlView->GetTreeCtrl();

	// 初始化tree ctrl
	TVINSERTSTRUCT tvInsert;
	tvInsert.hParent = NULL;
	tvInsert.hInsertAfter = NULL;
	tvInsert.item.mask = TVIF_TEXT;
	tvInsert.item.pszText = _T("BroadcastChannels");
	HTREEITEM hRoot = treeCtrl.InsertItem(&tvInsert);
	ASSERT(hRoot);
	DWORD type = TreeItem_Root;
	treeCtrl.SetItemData(hRoot, type);

	return CDocument::OnNewDocument();
}

bool BroadcastChPPTestDoc::RefreshChannels()
{
	char szBuf[1024];
	memset(szBuf, 0, sizeof(szBuf));
	_chnlIds.clear();
	try
	{
		_chnlIds = _publisherPrx->list();
	}
	catch (const TianShanIce::ServerError& ex)
	{
		_sntprintf(szBuf, sizeof(szBuf) - 1, "get channel list caught %s: %s", 
			ex.ice_name().c_str(), ex.message.c_str());
		AfxMessageBox(szBuf);
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		_sntprintf(szBuf, sizeof(szBuf) - 1, "get channel list caught %s", 
			ex.ice_name().c_str());
		AfxMessageBox(szBuf);
		return false;
	}
	return true;
}

