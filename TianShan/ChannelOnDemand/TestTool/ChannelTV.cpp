// ChannelTV.cpp : implementation file
//

#include "stdafx.h"
#include "CodMan.h"
#include "ChannelTV.h"
#include "CodManDoc.h"
#include "ChildFrm.h"
#include "PropertyView.h"
#include "ChannelEditDlg.h"
#include "stroprt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChannelTV

IMPLEMENT_DYNCREATE(CChannelTV, CTreeView)

CChannelTV::CChannelTV()
{
}

CChannelTV::~CChannelTV()
{
}


BEGIN_MESSAGE_MAP(CChannelTV, CTreeView)
	//{{AFX_MSG_MAP(CChannelTV)
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChannelTV drawing

void CChannelTV::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CChannelTV diagnostics

#ifdef _DEBUG
void CChannelTV::AssertValid() const
{
	CTreeView::AssertValid();
}

void CChannelTV::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChannelTV message handlers

HTREEITEM CChannelTV::GetChildItem(HTREEITEM hParent, CString findText)
{
	CTreeCtrl& treeCtrl = GetTreeCtrl();
	HTREEITEM hRet = treeCtrl.GetNextItem(hParent, TVGN_CHILD);
	while (hRet)
	{
		CString text = treeCtrl.GetItemText(hRet);
		if (text == findText)
		{
			break;
		}
		else 
		{
			HTREEITEM hTemp = hRet;
			hRet = NULL;
			hRet = treeCtrl.GetNextItem(hTemp, TVGN_NEXT);
		}
	}
	return hRet;
}

void CChannelTV::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class
	::SetWindowLong(GetSafeHwnd(), GWL_STYLE, ::GetWindowLong(GetSafeHwnd(), GWL_STYLE) | TVS_HASLINES/* | TVS_HASBUTTONS*/);
	
}

void CChannelTV::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	CSplitterWnd* pSplit = (CSplitterWnd*)GetParent();
	ASSERT(pSplit);

	CChildFrame* pFrame = (CChildFrame*)pSplit->GetParent();
	ASSERT(pFrame);

	CCodManDoc* pDoc = (CCodManDoc*)pFrame->GetActiveDocument();
	ASSERT(pDoc);

	CPropertyView* pPropView = pFrame->GetPropertyView();
	ASSERT(pPropView);

	CTreeCtrl& treeCtrl = GetTreeCtrl();

	CPoint point;
	::GetCursorPos(&point);
	ScreenToClient(&point);
	UINT uFlags;
	HTREEITEM hSelItem = treeCtrl.HitTest(point, &uFlags);
	treeCtrl.Select(hSelItem, TVGN_CARET);
	hSelItem = treeCtrl.GetSelectedItem();
	if (NULL == hSelItem)
		return;
	CString selText = treeCtrl.GetItemText(hSelItem);
	// Delete all of the children of current selected tree item.
	HTREEITEM hSelChild = treeCtrl.GetChildItem(hSelItem);
	while (hSelChild)
	{
		HTREEITEM hNextChild = treeCtrl.GetNextItem(hSelChild, TVGN_NEXT);
		treeCtrl.DeleteItem(hSelChild);
		hSelChild = hNextChild;
	}

	DWORD type;
	type = treeCtrl.GetItemData(hSelItem);
	if (type == TreeItem_Root)
	{
		pDoc->RefreshChannels();
		for (unsigned int i = 0; i < pDoc->_chnlIds.size(); i ++)
		{
			TVINSERTSTRUCT tvChnl;
			tvChnl.hParent = hSelItem;
			tvChnl.hInsertAfter = NULL;
			tvChnl.item.mask = TVIF_TEXT;
			tvChnl.item.pszText = _T((char*)pDoc->_chnlIds[i].c_str());
			HTREEITEM hChnl = treeCtrl.InsertItem(&tvChnl);
			ASSERT(hChnl);
			type = TreeItem_Channel;
			treeCtrl.SetItemData(hChnl, type);
		}
		pPropView->ShowChannels(pDoc->_chnlIds);
		treeCtrl.Expand(hSelItem, TVE_EXPAND);
	}
	else if (type == TreeItem_Channel)
	{
		TianShanIce::StrValues chnlItems;
		NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) chnlPub = NULL;
		::TianShanIce::Application::PublishPointPrx pubPrx = NULL;
		try
		{
#ifdef USE_OLD_NS
			chnlPub = pDoc->_publisherPrx->open((LPCTSTR)selText);
#else
			pubPrx = pDoc->_publisherPrx->open((LPCTSTR)selText);
			chnlPub = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS
			chnlItems = chnlPub->getItemSequence();
		}
		catch (const TianShanIce::BaseException& ex)
		{
			CString szBuf;
			szBuf.Format("open channel [%s] caught %s: %s", 
				selText, ex.ice_name().c_str(), ex.message.c_str());
			AfxMessageBox(szBuf);
			return;
		}
		catch (const Ice::Exception& ex)
		{
			CString szBuf;
			szBuf.Format("open channel [%s] caught %s", selText, ex.ice_name().c_str());
			AfxMessageBox(szBuf);
			return;
		}
		for (unsigned int i = 0, count = chnlItems.size(); i < count; i ++)
		{
			TVINSERTSTRUCT tvChnlItem;
			tvChnlItem.hParent = hSelItem;
			tvChnlItem.hInsertAfter = NULL;
			tvChnlItem.item.mask = TVIF_TEXT;
			tvChnlItem.item.pszText = _T((char*)chnlItems[i].c_str());
			HTREEITEM hChnlItem = treeCtrl.InsertItem(&tvChnlItem);
			ASSERT(hChnlItem);
			type = TreeItem_ChannelItem;
			treeCtrl.SetItemData(hChnlItem, type);
		}
		pPropView->ShowItems(chnlItems, (LPCTSTR)selText);
		treeCtrl.Expand(hSelItem, TVE_EXPAND);
		pDoc->_curChnl = (LPCTSTR)selText;
	}
	else if (type == TreeItem_ChannelItem)
	{
	}

	*pResult = 0;
}

