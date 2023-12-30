// CodManDoc.cpp : implementation of the CCodManDoc class
//

#include "stdafx.h"
#include "CodMan.h"

#include "CodManDoc.h"
#include "CodManView.h"
#include "ChildFrm.h"

#include "ChannelTV.h"
#include "PropertyView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCodManDoc

IMPLEMENT_DYNCREATE(CCodManDoc, CDocument)

BEGIN_MESSAGE_MAP(CCodManDoc, CDocument)
	//{{AFX_MSG_MAP(CCodManDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCodManDoc construction/destruction

CCodManDoc::CCodManDoc()
{
	// TODO: add one-time construction code here
	_codEndPoint = _T("");
	_publisherPrx = NULL;
}

CCodManDoc::~CCodManDoc()
{
}

/////////////////////////////////////////////////////////////////////////////
// CCodManDoc serialization

void CCodManDoc::Serialize(CArchive& ar)
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
// CCodManDoc diagnostics

#ifdef _DEBUG
void CCodManDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCodManDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCodManDoc commands

BOOL CCodManDoc::OnNewDocument() 
{
	// TODO: Add your specialized code here and/or call the base class
	char szBuf[1024];
	szBuf[sizeof(szBuf) - 1] = '\0';

	INT_PTR ret = _openCod.DoModal();
	if (ret != IDOK)
		return FALSE;

	_codEndPoint = _openCod.m_strCodEndPoint;

	SetTitle((LPCTSTR)_codEndPoint);

	// 获得publisher代理对象
	try
	{
		_snprintf(szBuf, sizeof(szBuf) - 1, "%s", _openCod.m_strCodEndPoint);
		_publisherPrx = NS_PREFIX(ChannelOnDemand::ChannelPublisherPrx)::checkedCast((((CCodManApp*)AfxGetApp())->_communicator)->stringToProxy(szBuf));
	}
	catch (const Ice::Exception& ex)
	{
		_snprintf(szBuf, sizeof(szBuf) - 1, "connect channel publisher caught %s", ex.ice_name().c_str());
		AfxMessageBox(szBuf, MB_OK);
		return FALSE;
	}

	// 获得视图对象
	CCodManView* pView = NULL;
	POSITION pos = GetFirstViewPosition();
	if (NULL != pos)
		pView = (CCodManView*)GetNextView(pos);
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
	tvInsert.item.pszText = _T("Channels");
	HTREEITEM hRoot = treeCtrl.InsertItem(&tvInsert);
	ASSERT(hRoot);
	DWORD type = TreeItem_Root;
	treeCtrl.SetItemData(hRoot, type);

	return CDocument::OnNewDocument();
}

bool CCodManDoc::RefreshChannels()
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

