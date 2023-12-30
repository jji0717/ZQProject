// PropertyView.cpp : implementation file
//

#include "stdafx.h"
#include "CodMan.h"
#include "PropertyView.h"
#include "CodManDoc.h"
#include "ChildFrm.h"
#include "ChannelEditDlg.h"
#include "ChnlItemEditDlg.h"
#include "stroprt.h"
#include "ChannelTV.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropertyView

IMPLEMENT_DYNCREATE(CPropertyView, CListView)

CPropertyView::CPropertyView()
{
}

CPropertyView::~CPropertyView()
{
}


BEGIN_MESSAGE_MAP(CPropertyView, CListView)
	//{{AFX_MSG_MAP(CPropertyView)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_COMMAND(IDM_PushItem, OnPushItem)
	ON_COMMAND(IDM_InsertItem, OnInsertItem)
	ON_COMMAND(IDM_ModifyItem, OnModifyItem)
	ON_COMMAND(IDM_ReplaceItem, OnReplaceItem)
	ON_NOTIFY_REFLECT(NM_RETURN, OnReturn)
	ON_COMMAND(IDM_ModifyChannel, OnModifyChannel)
	ON_COMMAND(IDM_NewChannel, OnNewChannel)
	ON_COMMAND(IDM_RemoveChannel, OnRemoveChannel)
	ON_COMMAND(IDM_RemoveItem, OnRemoveItem)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertyView drawing

void CPropertyView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CPropertyView diagnostics

#ifdef _DEBUG
void CPropertyView::AssertValid() const
{
	CListView::AssertValid();
}

void CPropertyView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPropertyView message handlers

void CPropertyView::ShowChannels(const std::vector<std::string>& chnlIds)
{
	CString szBuf;
	CListCtrl& listCtrl = GetListCtrl();
	CHeaderCtrl* pHeader = listCtrl.GetHeaderCtrl();
	for (unsigned int i = 0, count = pHeader->GetItemCount(); i < count; i ++)
	{
		listCtrl.DeleteColumn(0);
	}
	listCtrl.InsertColumn(0, "ChannelName", LVCFMT_LEFT, 120);
	listCtrl.InsertColumn(1, "OnDemandName", LVCFMT_LEFT, 120);
	listCtrl.InsertColumn(2, "MaxBitrate", LVCFMT_LEFT, 120);
	listCtrl.InsertColumn(3, "NetIds", LVCFMT_LEFT, 120);
	listCtrl.InsertColumn(4, "Description", LVCFMT_LEFT, 120);
	listCtrl.DeleteAllItems();

	CSplitterWnd* pSplit = (CSplitterWnd*)GetParent();
	ASSERT(pSplit);

	CChildFrame* pFrame = (CChildFrame*)pSplit->GetParent();
	ASSERT(pFrame);

	CCodManDoc* pDoc = (CCodManDoc*)pFrame->GetActiveDocument();
	ASSERT(pDoc);

	for (unsigned int i = 0, count = chnlIds.size(); i < count; i ++)
	{
		NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) chnlPub = NULL;
		::TianShanIce::Application::PublishPointPrx pubPrx = NULL;
		TianShanIce::StrValues chnlItems, netIds;
		std::string chnlDesc, onDmdName;
		Ice::Int maxBit;
		try
		{
#ifdef USE_OLD_NS
			chnlPub = pDoc->_publisherPrx->open(chnlIds[i]);
#else
			pubPrx = pDoc->_publisherPrx->open(chnlIds[i]);
			chnlPub = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS
			chnlItems = chnlPub->getItemSequence();
			chnlDesc = chnlPub->getDesc();
			maxBit = chnlPub->getMaxBitrate();
			netIds = chnlPub->listReplica();
			onDmdName = chnlPub->getOnDemandName();
		}
		catch (const TianShanIce::BaseException& ex)
		{
			szBuf.Format("open channel [%s] caught %s: %s", chnlIds[i].c_str(), ex.ice_name().c_str(), ex.message.c_str());
			AfxMessageBox(szBuf);
			continue;
		}
		catch (const Ice::Exception& ex)
		{
			szBuf.Format("open channel [%s] caught %s", chnlIds[i].c_str(), ex.ice_name().c_str());
			AfxMessageBox(szBuf);
			continue;
		}
		int istPos = listCtrl.GetItemCount();
		if (listCtrl.InsertItem(istPos, chnlIds[i].c_str()) != -1)
		{
			listCtrl.SetItemText(istPos, 1, onDmdName.c_str());
			szBuf.Format("%d", maxBit);
			listCtrl.SetItemText(istPos, 2, szBuf);
			szBuf = "";
			for (unsigned int cur = 0, cur_count = netIds.size(); cur < cur_count; cur ++)
			{
				szBuf += netIds[cur].c_str();
				szBuf += "#";
			}
			listCtrl.SetItemText(istPos, 3, szBuf);
			listCtrl.SetItemText(istPos, 4, chnlDesc.c_str());
			DWORD type = ListItem_Channel;
			listCtrl.SetItemData(istPos, type);
		}
	}
	_viewType = ChannelType;
}

void CPropertyView::ShowItems(const std::vector<std::string>& chnlItems, const std::string& chnlName)
{
	CListCtrl& listCtrl = GetListCtrl();
	CHeaderCtrl* pHeader = listCtrl.GetHeaderCtrl();
	for (unsigned int i = 0, count = pHeader->GetItemCount(); i < count; i ++)
	{
		listCtrl.DeleteColumn(0);
	}
	listCtrl.InsertColumn(0, "ItemName", LVCFMT_LEFT, 160);
	listCtrl.InsertColumn(1, "BroadCastTime", LVCFMT_LEFT, 130);
	listCtrl.InsertColumn(2, "Expiration", LVCFMT_LEFT, 130);
	listCtrl.InsertColumn(3, "Playable", LVCFMT_LEFT, 60);
	listCtrl.InsertColumn(4, "ForceNormalSpeed", LVCFMT_LEFT, 110);
	listCtrl.InsertColumn(5, "InTimeOffset", LVCFMT_LEFT, 90);
	listCtrl.InsertColumn(6, "OutTimeOffset", LVCFMT_LEFT, 90);
	listCtrl.InsertColumn(7, "SpliceIn", LVCFMT_LEFT, 60);
	listCtrl.InsertColumn(8, "SpliceOut", LVCFMT_LEFT, 70);
	listCtrl.DeleteAllItems();

	CSplitterWnd* pSplit = (CSplitterWnd*)GetParent();
	ASSERT(pSplit);

	CChildFrame* pFrame = (CChildFrame*)pSplit->GetParent();
	ASSERT(pFrame);

	CCodManDoc* pDoc = (CCodManDoc*)pFrame->GetActiveDocument();
	ASSERT(pDoc);

	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) chnlPub = NULL;
	::TianShanIce::Application::PublishPointPrx pubPrx = NULL;
	try
	{
#ifdef USE_OLD_NS
		chnlPub = pDoc->_publisherPrx->open(chnlName);
#else
		pubPrx = pDoc->_publisherPrx->open(chnlName);
		chnlPub = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS
	}
	catch (const TianShanIce::BaseException& ex)
	{
		CString szBuf;
		szBuf.Format("open channel [%s] caught %s: %s", 
			chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		AfxMessageBox(szBuf);
		return;
	}
	catch (const Ice::Exception& ex)
	{
		CString szBuf;
		szBuf.Format("open channel [%s] caught %s", chnlName.c_str(), ex.ice_name().c_str());
		AfxMessageBox(szBuf);
		return;
	}

	for (unsigned int i = 0, count = chnlItems.size(); i < count; i ++)
	{
		CI_NS_PREFIX(ChannelItem) chnlItem;
		try
		{
			chnlItem = chnlPub->findItem(chnlItems[i]);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			CString szBuf;
			szBuf.Format("find channel item [%s] caught %s: %s", 
				chnlItems[i].c_str(), ex.ice_name().c_str(), ex.message.c_str());
			AfxMessageBox(szBuf);
			continue;
		}
		catch (const Ice::Exception& ex)
		{
			CString szBuf;
			szBuf.Format("find channel item [%s] caught %s", chnlItems[i].c_str(), ex.ice_name().c_str());
			AfxMessageBox(szBuf);
			continue;
		}
		CString szBuf;
		szBuf.Format("%s#%s", chnlName.c_str(), chnlItem.contentName.c_str());
		int istPos = listCtrl.GetItemCount();
		if (listCtrl.InsertItem(istPos, szBuf) != -1)
		{
			listCtrl.SetItemText(istPos, 1, chnlItem.broadcastStart.c_str());
			listCtrl.SetItemText(istPos, 2, chnlItem.expiration.c_str());
			szBuf.Format("%d", chnlItem.playable);
			listCtrl.SetItemText(istPos, 3, szBuf);
			szBuf.Format("%d", chnlItem.forceNormalSpeed);
			listCtrl.SetItemText(istPos, 4, szBuf);
			szBuf.Format("%lld", chnlItem.inTimeOffset);
			listCtrl.SetItemText(istPos, 5, szBuf);
			szBuf.Format("%lld", chnlItem.outTimeOffset);
			listCtrl.SetItemText(istPos, 6, szBuf);
			szBuf.Format("%d", chnlItem.spliceIn);
			listCtrl.SetItemText(istPos, 7, szBuf);
			szBuf.Format("%d", chnlItem.spliceOut);
			listCtrl.SetItemText(istPos, 8, szBuf);
			DWORD type = ListItem_ChannelItem;
			listCtrl.SetItemData(istPos, type);
		}
	}
	_viewType = ItemType;
}

void CPropertyView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class
	::SetWindowLong(GetSafeHwnd(), GWL_STYLE, ::GetWindowLong(GetSafeHwnd(), GWL_STYLE) | LVS_REPORT);
	CListCtrl& listCtrl = GetListCtrl();
	listCtrl.SetExtendedStyle(listCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_SIMPLESELECT);
	
}

void CPropertyView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CString szBuf;
	CListCtrl& listCtrl = GetListCtrl();
	POSITION pos = listCtrl.GetFirstSelectedItemPosition();
	if (pos == NULL)
		return;

	int selCur = listCtrl.GetNextSelectedItem(pos);	
	DWORD type = listCtrl.GetItemData(selCur);
	if (type == ListItem_Channel)
	{
		ModifyChannel();
	}
	else if (type == ListItem_ChannelItem)
	{
		ModifyChannelItem();
	}

	*pResult = 0;
}

void CPropertyView::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	LPNMLISTVIEW pNM = (LPNMLISTVIEW)pNMHDR;
	if (pNM == NULL)
		return;

	CListCtrl& listCtrl = GetListCtrl();
	listCtrl.SetHotItem(pNM->iItem);

	CPoint point;
	::GetCursorPos(&point);

	CMenu mnu;
	mnu.LoadMenu(IDRM_PropertyView);
	
	if (_viewType == ItemType)
	{
		CMenu* pSub = mnu.GetSubMenu(0);
		pSub->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,this);
	}
	else if (_viewType == ChannelType)
	{
		CMenu* pSub = mnu.GetSubMenu(1);
		pSub->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,this);
	}
	
	*pResult = 0;
}

void CPropertyView::OnPushItem() 
{
	// TODO: Add your command handler code here
	CString szBuf;
	CListCtrl& listCtrl = GetListCtrl();

	CSplitterWnd* pSplit = (CSplitterWnd*)GetParent();
	ASSERT(pSplit);

	CChildFrame* pFrame = (CChildFrame*)pSplit->GetParent();
	ASSERT(pFrame);

	CCodManDoc* pDoc = (CCodManDoc*)pFrame->GetActiveDocument();
	ASSERT(pDoc);

	CChannelTV* pTreeView = pFrame->GetChannelView();
	ASSERT(pTreeView);

	ASSERT(_viewType == ItemType);

	CChnlItemEditDlg itemDlg;
	INT_PTR iRet = itemDlg.DoModal();
	if (iRet != IDOK)
		return;
	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) chnlPub = NULL;
	::TianShanIce::Application::PublishPointPrx pubPrx = NULL;
	CI_NS_PREFIX(ChannelItem) itemCtx;		
	try
	{
#ifdef USE_OLD_NS
		chnlPub = pDoc->_publisherPrx->open(pDoc->_curChnl);
#else
		pubPrx = pDoc->_publisherPrx->open(pDoc->_curChnl);
		chnlPub = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS
		itemCtx.contentName = itemDlg.m_sItemName;
		itemCtx.broadcastStart = itemDlg.m_sBroadcast;
		itemCtx.expiration = itemDlg.m_sExpiration;
		itemCtx.playable = itemDlg.m_bPlayable;
		itemCtx.forceNormalSpeed = itemDlg.m_bForceSpeed;
		itemCtx.inTimeOffset = itemDlg.m_inTimeOffset;
		itemCtx.outTimeOffset = itemDlg.m_outTimeOffset;
		itemCtx.spliceIn = itemDlg.m_bSpliceIn;
		itemCtx.spliceOut = itemDlg.m_bSpliceOut;
		chnlPub->appendItem(itemCtx);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		szBuf.Format("append channel item [%s] caught %s: %s", 
			itemDlg.m_sItemName, ex.ice_name().c_str(), ex.message.c_str());
		AfxMessageBox(szBuf);
		return;
	}
	catch (const Ice::Exception& ex)
	{
		szBuf.Format("append channel [%s] caught %s", itemDlg.m_sItemName, ex.ice_name().c_str());
		AfxMessageBox(szBuf);
		return;
	}
	int istPos = listCtrl.GetItemCount();
	szBuf.Format("%s#%s", pDoc->_curChnl.c_str(), itemDlg.m_sItemName);
	if (listCtrl.InsertItem(istPos, szBuf) != -1)
	{
		listCtrl.SetItemText(istPos, 1, itemDlg.m_sBroadcast);
		listCtrl.SetItemText(istPos, 2, itemDlg.m_sExpiration);
		szBuf.Format("%d", itemDlg.m_bPlayable);
		listCtrl.SetItemText(istPos, 3, szBuf);
		szBuf.Format("%d", itemDlg.m_bForceSpeed);
		listCtrl.SetItemText(istPos, 4, szBuf);
		szBuf.Format("%d", itemDlg.m_inTimeOffset);
		listCtrl.SetItemText(istPos, 5, szBuf);
		szBuf.Format("%d", itemDlg.m_outTimeOffset);
		listCtrl.SetItemText(istPos, 6, szBuf);
		szBuf.Format("%d", itemDlg.m_bSpliceIn);
		listCtrl.SetItemText(istPos, 7, szBuf);
		szBuf.Format("%d", itemDlg.m_bSpliceOut);
		listCtrl.SetItemText(istPos, 8, szBuf);
		DWORD type = ListItem_ChannelItem;
		listCtrl.SetItemData(istPos, type);
	}

	CTreeCtrl& treeCtrl = pTreeView->GetTreeCtrl();
	HTREEITEM hRoot = treeCtrl.GetRootItem();
	ASSERT(hRoot);
	HTREEITEM hCurChnl = pTreeView->GetChildItem(hRoot, pDoc->_curChnl.c_str());
	ASSERT(hCurChnl);
	TVINSERTSTRUCT tvChnlItem;
	tvChnlItem.hParent = hCurChnl;
	tvChnlItem.hInsertAfter = NULL;
	tvChnlItem.item.mask = TVIF_TEXT;
	tvChnlItem.item.pszText = (char*)itemCtx.contentName.c_str();
	HTREEITEM hChnlItem = treeCtrl.InsertItem(&tvChnlItem);
	ASSERT(hChnlItem);
	DWORD type = TreeItem_ChannelItem;
	treeCtrl.SetItemData(hChnlItem, type);
	treeCtrl.Expand(hCurChnl, TVE_EXPAND);
}

void CPropertyView::OnInsertItem() 
{
	// TODO: Add your command handler code here
	CString szBuf;
	CListCtrl& listCtrl = GetListCtrl();

	CSplitterWnd* pSplit = (CSplitterWnd*)GetParent();
	ASSERT(pSplit);

	CChildFrame* pFrame = (CChildFrame*)pSplit->GetParent();
	ASSERT(pFrame);

	CCodManDoc* pDoc = (CCodManDoc*)pFrame->GetActiveDocument();
	ASSERT(pDoc);

	CChannelTV* pTreeView = pFrame->GetChannelView();
	ASSERT(pTreeView);

	ASSERT(_viewType == ItemType);

	POSITION pos = listCtrl.GetFirstSelectedItemPosition();
	if (pos == NULL)
		return;
	int selCur = listCtrl.GetNextSelectedItem(pos);	
	std::string istText = COD::String::getRightStr((LPCTSTR)listCtrl.GetItemText(selCur, 0), "#", true);

	CChnlItemEditDlg itemDlg;
	INT_PTR iRet = itemDlg.DoModal();
	if (iRet != IDOK)
		return;
	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) chnlPub = NULL;
	::TianShanIce::Application::PublishPointPrx pubPrx = NULL;
	CI_NS_PREFIX(ChannelItem) itemCtx;		
	try
	{
#ifdef USE_OLD_NS
		chnlPub = pDoc->_publisherPrx->open(pDoc->_curChnl);
#else
		pubPrx = pDoc->_publisherPrx->open(pDoc->_curChnl);
		chnlPub = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS
		itemCtx.contentName = itemDlg.m_sItemName;
		itemCtx.broadcastStart = itemDlg.m_sBroadcast;
		itemCtx.expiration = itemDlg.m_sExpiration;
		itemCtx.playable = itemDlg.m_bPlayable;
		itemCtx.forceNormalSpeed = itemDlg.m_bForceSpeed;
		itemCtx.inTimeOffset = itemDlg.m_inTimeOffset;
		itemCtx.outTimeOffset = itemDlg.m_outTimeOffset;
		itemCtx.spliceIn = itemDlg.m_bSpliceIn;
		itemCtx.spliceOut = itemDlg.m_bSpliceOut;
		chnlPub->insertItem(istText, itemCtx);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		szBuf.Format("insert channel item [%s] caught %s: %s", 
			itemDlg.m_sItemName, ex.ice_name().c_str(), ex.message.c_str());
		AfxMessageBox(szBuf);
		return;
	}
	catch (const Ice::Exception& ex)
	{
		szBuf.Format("insert channel [%s] caught %s", itemDlg.m_sItemName, ex.ice_name().c_str());
		AfxMessageBox(szBuf);
		return;
	}
	int istPos = selCur;
	szBuf.Format("%s#%s", pDoc->_curChnl.c_str(), itemDlg.m_sItemName);
	if (listCtrl.InsertItem(istPos, szBuf) != -1)
	{
		listCtrl.SetItemText(istPos, 1, itemDlg.m_sBroadcast);
		listCtrl.SetItemText(istPos, 2, itemDlg.m_sExpiration);
		szBuf.Format("%d", itemDlg.m_bPlayable);
		listCtrl.SetItemText(istPos, 3, szBuf);
		szBuf.Format("%d", itemDlg.m_bForceSpeed);
		listCtrl.SetItemText(istPos, 4, szBuf);
		szBuf.Format("%d", itemDlg.m_inTimeOffset);
		listCtrl.SetItemText(istPos, 5, szBuf);
		szBuf.Format("%d", itemDlg.m_outTimeOffset);
		listCtrl.SetItemText(istPos, 6, szBuf);
		szBuf.Format("%d", itemDlg.m_bSpliceIn);
		listCtrl.SetItemText(istPos, 7, szBuf);
		szBuf.Format("%d", itemDlg.m_bSpliceOut);
		listCtrl.SetItemText(istPos, 8, szBuf);
		DWORD type = ListItem_ChannelItem;
		listCtrl.SetItemData(istPos, type);
	}

	CTreeCtrl& treeCtrl = pTreeView->GetTreeCtrl();
	HTREEITEM hRoot = treeCtrl.GetRootItem();
	ASSERT(hRoot);
	HTREEITEM hCurChnl = pTreeView->GetChildItem(hRoot, pDoc->_curChnl.c_str());
	ASSERT(hCurChnl);
	HTREEITEM hIstBf = pTreeView->GetChildItem(hCurChnl, istText.c_str());
	ASSERT(hIstBf);
	TVINSERTSTRUCT tvChnlItem;
	tvChnlItem.hParent = hCurChnl;
	//////////////////////////////////////////////////////////////////////////	
	// get insert after position
	HTREEITEM hIstAf = treeCtrl.GetNextItem(hIstBf, TVGN_PREVIOUS);
	if (hIstAf)
		tvChnlItem.hInsertAfter = hIstAf;
	else 
		tvChnlItem.hInsertAfter = TVI_FIRST;
	tvChnlItem.item.mask = TVIF_TEXT;
	tvChnlItem.item.pszText = (char*)itemCtx.contentName.c_str();
	HTREEITEM hChnlItem = treeCtrl.InsertItem(&tvChnlItem);
	ASSERT(hChnlItem);
	DWORD type = TreeItem_ChannelItem;
	treeCtrl.SetItemData(hChnlItem, type);
	treeCtrl.Expand(hCurChnl, TVE_EXPAND);
}

void CPropertyView::OnModifyItem() 
{
	// TODO: Add your control notification handler code here	
	ModifyChannelItem();
}

void CPropertyView::ModifyChannelItem(BOOL bModifyFlag)
{
	CString szBuf;
	CListCtrl& listCtrl = GetListCtrl();

	CSplitterWnd* pSplit = (CSplitterWnd*)GetParent();
	ASSERT(pSplit);

	CChildFrame* pFrame = (CChildFrame*)pSplit->GetParent();
	ASSERT(pFrame);

	CCodManDoc* pDoc = (CCodManDoc*)pFrame->GetActiveDocument();
	ASSERT(pDoc);

	CChannelTV* pTreeView = pFrame->GetChannelView();
	ASSERT(pTreeView);

	POSITION pos = listCtrl.GetFirstSelectedItemPosition();
	if (pos == NULL)
		return;

	ASSERT(_viewType == ItemType);

	CChnlItemEditDlg itemDlg;
	int selCur = listCtrl.GetNextSelectedItem(pos);	
	CString itemKey = listCtrl.GetItemText(selCur, 0);
	std::string itemName;
	itemName = COD::String::getRightStr((LPCTSTR)itemKey, "#", true);
	itemDlg.m_sItemName = itemName.c_str();
	itemDlg.m_sBroadcast = listCtrl.GetItemText(selCur, 1);
	itemDlg.m_sExpiration = listCtrl.GetItemText(selCur, 2);
	itemDlg.m_bPlayable = atoi(listCtrl.GetItemText(selCur, 3));
	itemDlg.m_bForceSpeed = atoi(listCtrl.GetItemText(selCur, 4));
	itemDlg.m_inTimeOffset = atol(listCtrl.GetItemText(selCur, 5));
	itemDlg.m_outTimeOffset = atol(listCtrl.GetItemText(selCur, 6));
	itemDlg.m_bSpliceIn = atoi(listCtrl.GetItemText(selCur, 7));
	itemDlg.m_bSpliceOut = atoi(listCtrl.GetItemText(selCur, 8));
	itemDlg.editMode = bModifyFlag;
	INT_PTR iRet = itemDlg.DoModal();
	if (iRet != IDOK)
		return;
	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) chnlPub = NULL;
	::TianShanIce::Application::PublishPointPrx pubPrx = NULL;
	try
	{
#ifdef USE_OLD_NS
		chnlPub = pDoc->_publisherPrx->open(pDoc->_curChnl);
#else
		pubPrx = pDoc->_publisherPrx->open(pDoc->_curChnl);
		chnlPub = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS
		CI_NS_PREFIX(ChannelItem) itemCtx;
		itemCtx.contentName = itemDlg.m_sItemName;
		itemCtx.broadcastStart = itemDlg.m_sBroadcast;
		itemCtx.expiration = itemDlg.m_sExpiration;
		itemCtx.playable = itemDlg.m_bPlayable;
		itemCtx.forceNormalSpeed = itemDlg.m_bForceSpeed;
		itemCtx.inTimeOffset = itemDlg.m_inTimeOffset;
		itemCtx.outTimeOffset = itemDlg.m_outTimeOffset;
		itemCtx.spliceIn = itemDlg.m_bSpliceIn;
		itemCtx.spliceOut = itemDlg.m_bSpliceOut;
		chnlPub->replaceItem(itemName, itemCtx);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		szBuf.Format("update channel item [%s] caught %s: %s", 
			itemKey, ex.ice_name().c_str(), ex.message.c_str());
		AfxMessageBox(szBuf);
		return;
	}
	catch (const Ice::Exception& ex)
	{
		szBuf.Format("update channel [%s] caught %s", itemKey, ex.ice_name().c_str());
		AfxMessageBox(szBuf);
		return;
	}
	szBuf.Format("%s#%s", pDoc->_curChnl.c_str(), itemDlg.m_sItemName);
	listCtrl.SetItemText(selCur, 0, szBuf);
	listCtrl.SetItemText(selCur, 1, itemDlg.m_sBroadcast);
	listCtrl.SetItemText(selCur, 2, itemDlg.m_sExpiration);
	szBuf.Format("%d", itemDlg.m_bPlayable);
	listCtrl.SetItemText(selCur, 3, szBuf);
	szBuf.Format("%d", itemDlg.m_bForceSpeed);
	listCtrl.SetItemText(selCur, 4, szBuf);
	szBuf.Format("%d", itemDlg.m_inTimeOffset);
	listCtrl.SetItemText(selCur, 5, szBuf);
	szBuf.Format("%d", itemDlg.m_outTimeOffset);
	listCtrl.SetItemText(selCur, 6, szBuf);
	szBuf.Format("%d", itemDlg.m_bSpliceIn);
	listCtrl.SetItemText(selCur, 7, szBuf);
	szBuf.Format("%d", itemDlg.m_bSpliceOut);
	listCtrl.SetItemText(selCur, 8, szBuf);

	CTreeCtrl& treeCtrl = pTreeView->GetTreeCtrl();
	HTREEITEM hRoot = treeCtrl.GetRootItem();
	ASSERT(hRoot);
	HTREEITEM hCurChnl = pTreeView->GetChildItem(hRoot, pDoc->_curChnl.c_str());
	ASSERT(hCurChnl);
	HTREEITEM hModify = pTreeView->GetChildItem(hCurChnl, itemName.c_str());
	ASSERT(hModify);
	treeCtrl.SetItemText(hModify, itemDlg.m_sItemName);
	treeCtrl.Expand(hCurChnl, TVE_EXPAND);
}

void CPropertyView::OnReplaceItem() 
{
	// TODO: Add your command handler code here	
	ModifyChannelItem(FALSE); // replace channel item
}

void CPropertyView::OnReturn(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	if (_viewType == ItemType)
		ModifyChannelItem();
	else if (_viewType == ChannelType)
		ModifyChannel();
	
	*pResult = 0;
}

void CPropertyView::ModifyChannel()
{
	CString szBuf;
	CListCtrl& listCtrl = GetListCtrl();

	CSplitterWnd* pSplit = (CSplitterWnd*)GetParent();
	ASSERT(pSplit);
	
	CChildFrame* pFrame = (CChildFrame*)pSplit->GetParent();
	ASSERT(pFrame);

	CCodManDoc* pDoc = (CCodManDoc*)pFrame->GetActiveDocument();
	ASSERT(pDoc);

	POSITION pos = listCtrl.GetFirstSelectedItemPosition();
	if (pos == NULL)
		return;
	int selCur = listCtrl.GetNextSelectedItem(pos);	

	ASSERT(_viewType == ChannelType);

	CChannelEditDlg chnlDlg;
	chnlDlg.m_sChnlName = listCtrl.GetItemText(selCur, 0);
	chnlDlg.m_sOnDmdName = listCtrl.GetItemText(selCur, 1);
	chnlDlg.m_iMaxBit = atoi(listCtrl.GetItemText(selCur, 2));
	chnlDlg.m_sNetIds = listCtrl.GetItemText(selCur, 3);
	chnlDlg.m_sDesc = listCtrl.GetItemText(selCur, 4);
	chnlDlg.editMode = TRUE;
	INT_PTR iRet = chnlDlg.DoModal();
	if (iRet != IDOK)
		return;
	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) chnlPub = NULL;
	::TianShanIce::Application::PublishPointPrx pubPrx = NULL;
	try
	{
#ifdef USE_OLD_NS
		chnlPub = pDoc->_publisherPrx->open((LPCTSTR)(chnlDlg.m_sChnlName));
#else
		pubPrx = pDoc->_publisherPrx->open((LPCTSTR)(chnlDlg.m_sChnlName));
		chnlPub = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS
		chnlPub->setOnDemandName((LPCTSTR)chnlDlg.m_sOnDmdName);
		chnlPub->setDesc((LPCTSTR)chnlDlg.m_sDesc);
		chnlPub->setMaxBitrate(chnlDlg.m_iMaxBit);
		std::vector<std::string> netIds;
		COD::String::splitStr((LPCTSTR)chnlDlg.m_sNetIds, "#", netIds);
		chnlPub->restrictReplica(netIds);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		szBuf.Format("update channel [%s] caught %s: %s", 
			chnlDlg.m_sChnlName, ex.ice_name().c_str(), ex.message.c_str());
		AfxMessageBox(szBuf);
		return;
	}
	catch (const Ice::Exception& ex)
	{
		szBuf.Format("update channel [%s] caught %s", chnlDlg.m_sChnlName, ex.ice_name().c_str());
		AfxMessageBox(szBuf);
		return;
	}
	listCtrl.SetItemText(selCur, 1, chnlDlg.m_sOnDmdName);
	szBuf.Format("%d", chnlDlg.m_iMaxBit);
	listCtrl.SetItemText(selCur, 2, szBuf);
	listCtrl.SetItemText(selCur, 3, chnlDlg.m_sNetIds);
	listCtrl.SetItemText(selCur, 4, chnlDlg.m_sDesc);
}


void CPropertyView::OnModifyChannel() 
{
	// TODO: Add your command handler code here
	ModifyChannel();
}

void CPropertyView::OnNewChannel() 
{
	// TODO: Add your command handler code here
	CString szBuf;
	CListCtrl& listCtrl = GetListCtrl();

	CSplitterWnd* pSplit = (CSplitterWnd*)GetParent();
	ASSERT(pSplit);

	CChildFrame* pFrame = (CChildFrame*)pSplit->GetParent();
	ASSERT(pFrame);

	CCodManDoc* pDoc = (CCodManDoc*)pFrame->GetActiveDocument();
	ASSERT(pDoc);

	CChannelTV* pTreeView = pFrame->GetChannelView();
	ASSERT(pTreeView);

	ASSERT(_viewType == ChannelType);

	CChannelEditDlg chnlDlg;
	INT_PTR iRet = chnlDlg.DoModal();
	if (iRet != IDOK)
		return;

	try
	{
		NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) chnlPub = NULL;
		TianShanIce::Properties props;
#ifdef USE_OLD_NS
		chnlPub = pDoc->_publisherPrx->publishEx((LPCTSTR)chnlDlg.m_sChnlName, (LPCTSTR)chnlDlg.m_sOnDmdName, chnlDlg.m_iMaxBit, props, (LPCTSTR)chnlDlg.m_sDesc);
#else
		::TianShanIce::Application::OnDemandPublishPointPrx pubPrx = NULL;		
		pubPrx = pDoc->_publisherPrx->publishEx((LPCTSTR)chnlDlg.m_sChnlName, (LPCTSTR)chnlDlg.m_sOnDmdName, chnlDlg.m_iMaxBit, props, (LPCTSTR)chnlDlg.m_sDesc);
		chnlPub = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx); 
#endif //USE_OLD_NS
		std::vector<std::string> netIds;
		COD::String::splitStr((LPCTSTR)chnlDlg.m_sNetIds, "#", netIds);
		chnlPub->restrictReplica(netIds);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		szBuf.Format("publishEx channel [%s] caught %s: %s", 
			chnlDlg.m_sChnlName, ex.ice_name().c_str(), ex.message.c_str());
		AfxMessageBox(szBuf);
		return;
	}
	catch (const Ice::Exception& ex)
	{
		szBuf.Format("publishEx channel [%s] caught %s", chnlDlg.m_sChnlName, ex.ice_name().c_str());
		AfxMessageBox(szBuf);
		return;
	}

	int istPos = listCtrl.GetItemCount();
	if (listCtrl.InsertItem(istPos, chnlDlg.m_sChnlName) != -1)
	{
		listCtrl.SetItemText(istPos, 1, chnlDlg.m_sOnDmdName);
		szBuf.Format("%d", chnlDlg.m_iMaxBit);
		listCtrl.SetItemText(istPos, 2, szBuf);
		listCtrl.SetItemText(istPos, 3, chnlDlg.m_sNetIds);
		listCtrl.SetItemText(istPos, 4, chnlDlg.m_sDesc);
	}

	CTreeCtrl& treeCtrl = pTreeView->GetTreeCtrl();
	HTREEITEM hRoot = treeCtrl.GetRootItem();
	ASSERT(hRoot);
	TVINSERTSTRUCT tvChannel;
	tvChannel.hParent = hRoot;
	tvChannel.hInsertAfter = NULL;
	tvChannel.item.mask = TVIF_TEXT;
	tvChannel.item.pszText = (char*)((LPCTSTR)chnlDlg.m_sChnlName);
	HTREEITEM hChannel = treeCtrl.InsertItem(&tvChannel);
	ASSERT(hChannel);
	DWORD type = TreeItem_Channel;
	treeCtrl.SetItemData(hChannel, type);	
	treeCtrl.Expand(hRoot, TVE_EXPAND);
}

void CPropertyView::OnRemoveChannel() 
{
	// TODO: Add your command handler code here
	CString szBuf;
	CListCtrl& listCtrl = GetListCtrl();

	CSplitterWnd* pSplit = (CSplitterWnd*)GetParent();
	ASSERT(pSplit);

	CChildFrame* pFrame = (CChildFrame*)pSplit->GetParent();
	ASSERT(pFrame);

	CCodManDoc* pDoc = (CCodManDoc*)pFrame->GetActiveDocument();
	ASSERT(pDoc);

	CChannelTV* pTreeView = pFrame->GetChannelView();
	ASSERT(pTreeView);

	ASSERT(_viewType == ChannelType);

	POSITION pos = listCtrl.GetFirstSelectedItemPosition();
	if (pos == NULL)
		return;
	int selCur = listCtrl.GetNextSelectedItem(pos);

	std::string chnlName = (LPCTSTR)listCtrl.GetItemText(selCur, 0);

	try
	{
		NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) chnlPub = NULL;
		::TianShanIce::Application::PublishPointPrx pubPrx = NULL;
#ifdef USE_OLD_NS
		chnlPub = pDoc->_publisherPrx->open(chnlName);
#else
		pubPrx = pDoc->_publisherPrx->open(chnlName);
		chnlPub = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS 
		chnlPub->destroy();
	}
	catch (const TianShanIce::BaseException& ex)
	{
		szBuf.Format("destroy channel [%s] caught %s: %s", 
			chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		AfxMessageBox(szBuf);
		return;
	}
	catch (const Ice::Exception& ex)
	{
		szBuf.Format("destroy channel [%s] caught %s", chnlName.c_str(), ex.ice_name().c_str());
		AfxMessageBox(szBuf);
		return;
	}

	listCtrl.DeleteItem(selCur);

	CTreeCtrl& treeCtrl = pTreeView->GetTreeCtrl();
	HTREEITEM hRoot = treeCtrl.GetRootItem();
	ASSERT(hRoot);
	HTREEITEM hChnlRmv = pTreeView->GetChildItem(hRoot, chnlName.c_str());
	ASSERT(hChnlRmv);
	treeCtrl.DeleteItem(hChnlRmv);
	treeCtrl.Expand(hRoot, TVE_EXPAND);
}

void CPropertyView::OnRemoveItem() 
{
	// TODO: Add your command handler code here
	CString szBuf;
	CListCtrl& listCtrl = GetListCtrl();

	CSplitterWnd* pSplit = (CSplitterWnd*)GetParent();
	ASSERT(pSplit);

	CChildFrame* pFrame = (CChildFrame*)pSplit->GetParent();
	ASSERT(pFrame);

	CCodManDoc* pDoc = (CCodManDoc*)pFrame->GetActiveDocument();
	ASSERT(pDoc);

	CChannelTV* pTreeView = pFrame->GetChannelView();
	ASSERT(pTreeView);

	POSITION pos = listCtrl.GetFirstSelectedItemPosition();
	if (pos == NULL)
		return;
	int selCur = listCtrl.GetNextSelectedItem(pos);
	std::string itemName = COD::String::getRightStr((LPCTSTR)listCtrl.GetItemText(selCur, 0), "#", true);

	ASSERT(_viewType == ItemType);

	try
	{
		NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) chnlPub = NULL;
		::TianShanIce::Application::PublishPointPrx pubPrx = NULL;
#ifdef USE_OLD_NS
		chnlPub = pDoc->_publisherPrx->open(pDoc->_curChnl);
#else
		pubPrx = pDoc->_publisherPrx->open(pDoc->_curChnl);
		chnlPub = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS 
		chnlPub->removeItem(itemName);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		szBuf.Format("remove channel item [%s] caught %s: %s", 
			pDoc->_curChnl.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		AfxMessageBox(szBuf);
		return;
	}
	catch (const Ice::Exception& ex)
	{
		szBuf.Format("remove channel item [%s] caught %s", pDoc->_curChnl.c_str(), ex.ice_name().c_str());
		AfxMessageBox(szBuf);
		return;
	}

	listCtrl.DeleteItem(selCur);

	CTreeCtrl& treeCtrl = pTreeView->GetTreeCtrl();
	HTREEITEM hRoot = treeCtrl.GetRootItem();
	ASSERT(hRoot);
	HTREEITEM hChnl = pTreeView->GetChildItem(hRoot, pDoc->_curChnl.c_str());
	ASSERT(hChnl);
	HTREEITEM hItemRmv = pTreeView->GetChildItem(hChnl, itemName.c_str());
	ASSERT(hItemRmv);
	treeCtrl.DeleteItem(hItemRmv);
	treeCtrl.Expand(hChnl, TVE_EXPAND);
}
