// AdPathUI.cpp : Implementation of CAdPathUI
#include "stdafx.h"
#include "AdPathExplorer.h"
#include "AdPathUI.h"

/////////////////////////////////////////////////////////////////////////////
// CAdPathUI
void CAdPathUI::test()
{
/*	::TianShanIce::AccreditedPath::StorageLinks storagelinks = m_client->listStorageLinks();
	
	for (::TianShanIce::AccreditedPath::StorageLinks::iterator it = storagelinks.begin(); it < storagelinks.end(); it++)
	{
		std::string str = (*it)->getType();
		AtlTrace("%d", (*it)->getIdent());
		AtlTrace(" %s\n",str.c_str());
	}*/
}

void CAdPathUI::InitTreeCtrl()
{
	
	DWORD dwStyle = ::GetWindowLong(m_hTreeCtrl,GWL_STYLE);
    dwStyle |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT |TVS_SHOWSELALWAYS;
    ::SetWindowLong(m_hTreeCtrl,GWL_STYLE,dwStyle);
	
	TVITEM tvi = {0};
	TVINSERTSTRUCT tvs;
	tvs.hInsertAfter = TVI_LAST; 
	tvi.mask = TVIF_TEXT | TVIF_SELECTEDIMAGE ;
	
	tvi.pszText = "Storage";
	tvs.hParent = TVI_ROOT;
	tvs.item = tvi;	
	m_hStorageRoot = TreeView_InsertItem(m_hTreeCtrl, &tvs);
    ListStorageId();
	
	tvi.pszText = "Streamer";
	tvs.hParent = TVI_ROOT;
	tvs.item = tvi;	
	m_hStreamerRoot = TreeView_InsertItem(m_hTreeCtrl, &tvs);	
    listStreamerId();
	
	tvi.pszText = "ServiceGroup";
	tvs.hParent = TVI_ROOT;
	tvs.item = tvi;	
	m_hSGRoot = TreeView_InsertItem(m_hTreeCtrl, &tvs);	
	listSGId();

	ListView_DeleteAllItems(m_hListCtrl);
	
	//	TreeView_SelectItem(m_hTreeCtrl, m_hStorageRoot);
}

void CAdPathUI::InitListCtrl()
{
   	ListView_SetExtendedListViewStyle(m_hListCtrl,LVS_EX_FULLROWSELECT| LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
	::GetWindowLong(m_hListCtrl,GWL_EXSTYLE | GWL_EXSTYLE & LVS_EX_FULLROWSELECT);
	
	 ListView_SetExtendedListViewStyle(m_hListCtrlLinks,LVS_EX_FULLROWSELECT| LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
	::GetWindowLong(m_hListCtrlLinks,GWL_EXSTYLE | GWL_EXSTYLE & LVS_EX_FULLROWSELECT);
	 LV_COLUMN lvCol;

	lvCol.mask = LVCF_TEXT | LVCF_WIDTH |LVIF_STATE;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.iSubItem = 0;
	
	lvCol.cx = 150;
	lvCol.pszText = "Key";
	ListView_InsertColumn(m_hListCtrl,0,&lvCol);
	
	lvCol.cx = 250;
	lvCol.pszText = "Value";
	ListView_InsertColumn(m_hListCtrl,1,&lvCol);


	lvCol.cx = 0;
	lvCol.pszText = "";
	ListView_InsertColumn(m_hListCtrlLinks,0,&lvCol);
}

void CAdPathUI::DeleteTreeItem(HTREEITEM hitem)
{
    HTREEITEM hdel = TreeView_GetChild(m_hTreeCtrl, hitem);
	HTREEITEM htemp;
	while(hdel)
	{   
		htemp = hdel;
		hdel = TreeView_GetNextItem(m_hTreeCtrl, htemp,TVGN_NEXT);
		TreeView_DeleteItem(m_hTreeCtrl, htemp);
	}
}

void CAdPathUI::ListStorageId()
{
       HTREEITEM hTemp;
	   TVITEM tvi = {0};
	   TVINSERTSTRUCT tvs;
	   tvs.hInsertAfter = TVI_LAST; 
	   tvi.mask = TVIF_TEXT  | TVIF_SELECTEDIMAGE ;

        ListView_DeleteAllItems(m_hListCtrl);
	    DeleteListCtrlColumn();	
		m_blistLink = FALSE;
  ::TianShanIce::AccreditedPath::Storages stores;
	try
	{
      stores = m_client->listStorages();
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		ReInitTree();
		return;
	}

	   int size = stores.size();
	   
	   for(int j = 0; j < size; j ++)
	   { 
		   tvs.hInsertAfter = TVI_LAST; 
		   tvi.pszText = (LPSTR)(stores[j].netId).c_str();//itoa(stores[j].id, temp, 10);
		   tvs.hParent = m_hStorageRoot;
		   tvs.item = tvi;	
		   hTemp = TreeView_InsertItem(m_hTreeCtrl, &tvs);

		   tvs.hInsertAfter = TVI_LAST; 
		   tvi.pszText = "Storage Links";//itoa(streamers[j].id, temp, 10);
		   tvs.hParent = hTemp;
		   tvs.item = tvi;
		   hTemp = TreeView_InsertItem(m_hTreeCtrl, &tvs);
		  
	   }

	    LVITEM lvItem;
		char temp[20];
	   lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
	   lvItem.state = 0;
	   lvItem.iSubItem = 0;	  
	   
	   lvItem.iItem = 0;
	   lvItem.pszText = "Storage Count"; 
	   ListView_InsertItem(m_hListCtrl, &lvItem);		
	   ListView_SetItemText(m_hListCtrl,0,1,(LPSTR)itoa(size, temp , 10));

	   stores.clear();
	   TreeView_Expand(m_hTreeCtrl, m_hStorageRoot, TVE_EXPAND);
}

void CAdPathUI::listStreamerId()
{
	   HTREEITEM hTemp;
	   TVITEM tvi = {0};
	   TVINSERTSTRUCT tvs;
	   tvs.hInsertAfter = TVI_LAST; 
	   tvi.mask = TVIF_TEXT  | TVIF_SELECTEDIMAGE ;
	   
	   ListView_DeleteAllItems(m_hListCtrl);
	   DeleteListCtrlColumn();
	   m_blistLink = FALSE;
	   ::TianShanIce::AccreditedPath::Streamers streamers;
	try
	{
	   streamers = m_client->listStreamers();
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		ReInitTree();
		return;
	}

	   int size = streamers.size();
	   
	   for( int j = 0; j < size; j ++)
	   { 
		   tvs.hInsertAfter = TVI_LAST; 
		   tvi.pszText = (LPSTR)(streamers[j].netId).c_str();//itoa(streamers[j].id, temp, 10);
		   tvs.hParent = m_hStreamerRoot;
		   tvs.item = tvi;	
		   hTemp = TreeView_InsertItem(m_hTreeCtrl, &tvs);

		   tvs.hInsertAfter = TVI_LAST; 
		   tvi.pszText = "Storage Links";//itoa(streamers[j].id, temp, 10);
		   tvs.hParent = hTemp;
		   tvs.item = tvi;
		   TreeView_InsertItem(m_hTreeCtrl, &tvs);

		   tvs.hInsertAfter = TVI_LAST; 
		   tvi.pszText = "Streamer Links";//itoa(streamers[j].id, temp, 10);
		   tvs.hParent = hTemp;
		   tvs.item = tvi;
		   TreeView_InsertItem(m_hTreeCtrl, &tvs);
	   }
	  
		   LVITEM lvItem;
		   char temp[20];
		   lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
		   lvItem.state = 0;
		   lvItem.iSubItem = 0;	  
		   
		   lvItem.iItem = 0;
		   lvItem.pszText = "Streamer Count"; 
		   ListView_InsertItem(m_hListCtrl, &lvItem);		
		   ListView_SetItemText(m_hListCtrl,0,1,(LPSTR)itoa(size, temp , 10));

	   streamers.clear();	
	   TreeView_Expand(m_hTreeCtrl, m_hStreamerRoot, TVE_EXPAND);	   
}

void CAdPathUI::listSGId()
{      
       HTREEITEM hTemp;
	   char temp[20];
	   TVITEM tvi = {0};
	   TVINSERTSTRUCT tvs;
	   tvs.hInsertAfter = TVI_LAST; 
	   tvi.mask = TVIF_TEXT | TVIF_SELECTEDIMAGE ;

	   ListView_DeleteAllItems(m_hListCtrl);
	   DeleteListCtrlColumn();	
	   m_blistLink = FALSE;

 ::TianShanIce::AccreditedPath::ServiceGroups servicegroups;
	try
	{
	   servicegroups = m_client->listServiceGroups();
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		ReInitTree();
		return;
	}

	   int size = servicegroups.size();
	   
	   for( int j = 0; j < size; j ++)
	   { 
		   tvs.hInsertAfter = TVI_LAST; 		
		   tvi.pszText = itoa(servicegroups[j].id, temp, 10);
		   tvs.hParent = m_hSGRoot;
		   tvs.item = tvi;	
		   hTemp = TreeView_InsertItem(m_hTreeCtrl, &tvs);

		   tvs.hInsertAfter = TVI_LAST; 
		   tvi.pszText = "Streamer Links";//itoa(streamers[j].id, temp, 10);
		   tvs.hParent = hTemp;
		   tvs.item = tvi;
		   TreeView_InsertItem(m_hTreeCtrl, &tvs);
	   }
     
	   LVITEM lvItem;
	   lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
	   lvItem.state = 0;
	   lvItem.iSubItem = 0;	  
	   
	   lvItem.iItem = 0;
	   lvItem.pszText = "ServiceGroup Count"; 
	   ListView_InsertItem(m_hListCtrl, &lvItem);		
	   ListView_SetItemText(m_hListCtrl,0,1,(LPSTR)itoa(size, temp , 10));

	   servicegroups.clear();	
	   TreeView_Expand(m_hTreeCtrl, m_hSGRoot, TVE_EXPAND);
}

void CAdPathUI::GetTreeViewText(HTREEITEM hitem, char *pstr)
{
   	TVITEM tvi = { 0 };   
	char buf[100];
	tvi.mask = LVIF_TEXT;
    tvi.pszText=buf;
    tvi.cchTextMax=sizeof(buf);
	tvi.hItem = hitem; 
    TreeView_GetItem(m_hTreeCtrl,&tvi);
    strcpy(pstr, tvi.pszText);
}


void CAdPathUI::DisplayStorageInfo(SData *data)
{
   	HTREEITEM hSelect = TreeView_GetSelection(m_hTreeCtrl);
	char temp[50], *key;
	char strText[150];
	std::string strtemp;
	
	DeleteListCtrlColumn();	

	GetTreeViewText(hSelect, temp);
	strtemp = temp;
	m_blistLink = FALSE;
	::TianShanIce::AccreditedPath::Storages storages;
	try
	{	 
    storages = m_client->listStorages();
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		ReInitTree();
		return;
	}

	   int size = storages.size();
	   
	   for( int j = 0; j < size; j ++)
	   { 		
		   if(storages[j].netId == strtemp)
			   break;		   
	   }
	   if( j == size)
		   return;
	   
	   ListView_DeleteAllItems(m_hListCtrl);
	   
	   LVITEM lvItem;
	   lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
	   lvItem.state = 0;
	   lvItem.iSubItem = 0;
	   int i = -1;
	   int num = 5;
	   
	   lvItem.iItem = 0;
	   lvItem.pszText = "storage netId"; 
	   ListView_InsertItem(m_hListCtrl, &lvItem);		
	   ListView_SetItemText(m_hListCtrl,0,1,(LPSTR)(storages[j].netId).c_str());
	   
	   lvItem.iItem = 1;
	   lvItem.pszText = "storage Type"; 
	   ListView_InsertItem(m_hListCtrl, &lvItem);		
	   ListView_SetItemText(m_hListCtrl,1,1,(LPSTR)(storages[j].type).c_str());
	   
	   lvItem.iItem = 2;
	   lvItem.pszText = "storage EndPoint"; 
	   ListView_InsertItem(m_hListCtrl, &lvItem);		
	   ListView_SetItemText(m_hListCtrl,2,1,(LPSTR)(storages[j].ifep).c_str());
	   
	   lvItem.iItem = 3;
	   lvItem.pszText = "storage Desc"; 
	   ListView_InsertItem(m_hListCtrl, &lvItem);		
	   ListView_SetItemText(m_hListCtrl,3,1,(LPSTR)(storages[j].desc).c_str());
	 
	   ::TianShanIce::ValueMap vale =  storages[j].privateData;
	   
	   ::TianShanIce::ValueMap::iterator pos = vale.begin();
        InitListCtrlForPD();
	   while(pos != vale.end())
	   {
		   if(!GetPrivateDataStr(strText,&(pos->second)))
		   {
			   MessageBox("GetPrivateDataStr error!!");
			   return;	
		   } 
		   lvItem.iItem = ++i;
		   key = (LPSTR) pos->first.c_str();	  
		   lvItem.pszText =(LPSTR)key; 
		   ListView_InsertItem(m_hListCtrlLinks, &lvItem);	 
		   ListView_SetItemText(m_hListCtrlLinks,i,1,(LPSTR)strText);
		   
		   ++pos;
	   }
      
	   if(data)	
	   {
		   data->netId = storages[j].netId;
		   data->desc = storages[j].desc;
		   data->type = storages[j].type;
		   data->ifep = storages[j].ifep;
		   data->flag = TRUE;
	   }
	   storages.clear();	
}
void CAdPathUI::DisplayStreamerInfo(SData *data)
{
   	HTREEITEM hSelect = TreeView_GetSelection(m_hTreeCtrl);
	char temp[50], *key;
	char strText[150];
	std::string strTemp;
	
	DeleteListCtrlColumn();	
	m_blistLink = FALSE;

	GetTreeViewText(hSelect, temp);
	strTemp = temp;
	::TianShanIce::AccreditedPath::Streamers streamers;
	try
	{
		streamers = m_client->listStreamers();
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		ReInitTree();
		return;
	}

	   int size = streamers.size();
	   
	   for( int j = 0; j < size; j ++)
	   { 		
		   if(streamers[j].netId == strTemp)
			   break;		   
	   }
	   if( j == size)
		   return;
	   
	   ListView_DeleteAllItems(m_hListCtrl);
	   
	   LVITEM lvItem;
	   lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
	   lvItem.state = 0;
	   lvItem.iSubItem = 0;
	   int i = -1;
	   int num = 5;
	   
	   lvItem.iItem = 0;
	   lvItem.pszText = "streamer netId"; 
	   ListView_InsertItem(m_hListCtrl, &lvItem);		
	   ListView_SetItemText(m_hListCtrl,0,1,(LPSTR)(streamers[j].netId).c_str());
	   
	   lvItem.iItem = 1;
	   lvItem.pszText = "streamer Type"; 
	   ListView_InsertItem(m_hListCtrl, &lvItem);		
	   ListView_SetItemText(m_hListCtrl,1,1,(LPSTR)(streamers[j].type).c_str());
	   
	   lvItem.iItem = 2;
	   lvItem.pszText = "streamer EndPoint"; 
	   ListView_InsertItem(m_hListCtrl, &lvItem);		
	   ListView_SetItemText(m_hListCtrl,2,1,(LPSTR)(streamers[j].ifep).c_str());
	   
	   lvItem.iItem = 3;
	   lvItem.pszText = "streamer Desc"; 
	   ListView_InsertItem(m_hListCtrl, &lvItem);		
	   ListView_SetItemText(m_hListCtrl,3,1,(LPSTR)(streamers[j].desc).c_str());
	   
	
	   ::TianShanIce::ValueMap vale =  streamers[j].privateData;
	   
	   ::TianShanIce::ValueMap::iterator pos = vale.begin();

	   InitListCtrlForPD();
	   while(pos != vale.end())
	   {
		   if(!GetPrivateDataStr(strText,&(pos->second)))
		   {
			   MessageBox("GetPrivateDataStr error!!");
			   return;	
		   } 
		   lvItem.iItem = ++i;
		   key = (LPSTR) pos->first.c_str();	  
		   lvItem.pszText =(LPSTR)key; 
		   ListView_InsertItem(m_hListCtrlLinks, &lvItem);	 
		   ListView_SetItemText(m_hListCtrlLinks,i,1,(LPSTR)strText);
		   
		   ++pos;
	   }

		 if(data)
	   {
		   data->netId = streamers[j].netId;
		   data->desc = streamers[j].desc;
		   data->type = streamers[j].type;
		   data->ifep = streamers[j].ifep;	
		   data->flag = TRUE;
	   }
	   streamers.clear();
}
void CAdPathUI::DisplaySGInfo(SData *data)
{
	HTREEITEM hSelect = TreeView_GetSelection(m_hTreeCtrl);
	char temp[50];
	int nId;
	
	DeleteListCtrlColumn();	
    m_blistLink = FALSE;
	GetTreeViewText(hSelect, temp);
	nId = atoi(temp);
	::TianShanIce::AccreditedPath::ServiceGroups servicegroups; 
	try
	{
	 servicegroups = m_client->listServiceGroups();
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		ReInitTree();
		return ;
	}

	   int size = servicegroups.size();
	   
	   for( int j = 0; j < size; j ++)
	   { 		
		   if(servicegroups[j].id == nId)
			   break;		   
	   }
	   
	   if( j == size)
		   return;
       ListView_DeleteAllItems(m_hListCtrl);
	   
	   LVITEM lvItem;
	   lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
	   lvItem.state = 0;
	   lvItem.iSubItem = 0;
	   int i = -1;
	   int num = 5;
	   
	   lvItem.iItem = 0;
	   lvItem.pszText = "ServiceGroup ID"; 
	   ListView_InsertItem(m_hListCtrl, &lvItem);		
	   ListView_SetItemText(m_hListCtrl,0,1,(LPSTR)temp);
	   
	   lvItem.iItem = 1;
	   lvItem.pszText = "ServiceGroup Desc"; 
	   ListView_InsertItem(m_hListCtrl, &lvItem);		
	   ListView_SetItemText(m_hListCtrl,1,1,(LPSTR)(servicegroups[j].desc).c_str());
	   
	   if(data)
	   {
		   data->id = nId;
		   data->desc = servicegroups[j].desc;
		   data->flag = TRUE;
	   }
	   servicegroups.clear();		   
}

void CAdPathUI::DeleteListCtrlColumn()
{
    ListView_DeleteAllItems(m_hListCtrlLinks);
	int ncol = 10;
	while(ncol--)
	{
		ListView_DeleteColumn(m_hListCtrlLinks, 0);
	}

	LV_COLUMN lvCol;

	lvCol.mask = LVCF_TEXT | LVCF_WIDTH |LVIF_STATE;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.iSubItem = 0;
	
	lvCol.cx = 0;
	lvCol.pszText = "";
	ListView_InsertColumn(m_hListCtrlLinks,0,&lvCol);
}

void CAdPathUI::InitListCtrlColunmName()
{
	HTREEITEM hSelect ; 
	char strTreeText[50];
	hSelect = TreeView_GetSelection(m_hTreeCtrl);
	GetTreeViewText(hSelect, strTreeText);
	ListView_DeleteAllItems(m_hListCtrl);
	DeleteListCtrlColumn();
    ListView_DeleteColumn(m_hListCtrlLinks, 0);
	
	   LVITEM lvItem;
	   lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
	   lvItem.state = 0;
	   lvItem.iSubItem = 0;	  
	   lvItem.iItem = 0;

	   LV_COLUMN lvCol;
	   
	   lvCol.mask = LVCF_TEXT | LVCF_WIDTH |LVIF_STATE;
	   lvCol.fmt = LVCFMT_LEFT;
	   lvCol.iSubItem = 0;
	   

	
	lvCol.cx = 230;
	lvCol.pszText = "Ident";
	ListView_InsertColumn(m_hListCtrlLinks,0,&lvCol);
	
	lvCol.cx = 150;
	lvCol.pszText = "Type";
	ListView_InsertColumn(m_hListCtrlLinks,1,&lvCol);

   if(!strcmp(strTreeText, "Storage Links"))
   {
    m_bStorageLink = TRUE;
	m_bStreamerLink = FALSE;

	lvCol.cx = 80;
	lvCol.pszText = "StorageId";
	ListView_InsertColumn(m_hListCtrlLinks,2,&lvCol);
	
	lvCol.cx = 80;
	lvCol.pszText = "StreamerId";
	ListView_InsertColumn(m_hListCtrlLinks,3,&lvCol);
	
	lvItem.pszText = "StorageLink Count"; 
	ListView_InsertItem(m_hListCtrl, &lvItem);		
	   
   }
   else
   {
	m_bStorageLink = FALSE;
	m_bStreamerLink = TRUE;

   	lvCol.cx = 80;
	lvCol.pszText = "StreamerId";
	ListView_InsertColumn(m_hListCtrlLinks,2,&lvCol);
	
	lvCol.cx = 80;
	lvCol.pszText = "ServiceGroupId";
	ListView_InsertColumn(m_hListCtrlLinks,3,&lvCol);

	lvItem.pszText = "StreamLink Count"; 
	ListView_InsertItem(m_hListCtrl, &lvItem);		
	   
   }
	lvCol.cx = 0;
	lvCol.pszText = "ProxyString";
	ListView_InsertColumn(m_hListCtrlLinks,4,&lvCol);  
}
void CAdPathUI::DisplayStorageLinks()
{
	char temp[15];
    InitListCtrlColunmName();
 
   	HTREEITEM hSelect ,hParent; 
	char strTreeText[50];
	hSelect = TreeView_GetSelection(m_hTreeCtrl);
    hParent = TreeView_GetNextItem(m_hTreeCtrl,hSelect, TVGN_PARENT);
	
	GetTreeViewText(hParent, strTreeText);
    m_blistLink = TRUE;

	LVITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
    lvItem.state = 0;
	lvItem.iSubItem = 0;
    int i = -1;
	std::string strtemp;
	::TianShanIce::AccreditedPath::StorageLinks storagelinks;
	try
	{
		storagelinks = m_client->listStorageLinksByStorage(strTreeText);
		
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		ReInitTree();
		return;
	}

	 ListView_SetItemText(m_hListCtrl,0,1,(LPSTR)itoa(storagelinks.size(), temp , 10));
 	for (::TianShanIce::AccreditedPath::StorageLinks::iterator it = storagelinks.begin(); it < storagelinks.end(); it++)
	{
/*		std::string str = (*it)->getType();
		AtlTrace("%s", (*it)->getIdent().name.c_str());
		AtlTrace("Type %s ,",str.c_str());
		AtlTrace("StorageId %s ,",(*it)->getStorageId().c_str());
		AtlTrace("StreamerId %s,",(*it)->getStreamerId().c_str());
        AtlTrace("BandWitdth %d\n",(*it)->getBandwidth());*/

	   lvItem.iItem = ++i;
	   strtemp = (*it)->getIdent().name;
	   lvItem.pszText = (LPSTR)strtemp.c_str(); 
	   ListView_InsertItem(m_hListCtrlLinks, &lvItem);	
	   strtemp =(*it)->getType() ;
	   ListView_SetItemText(m_hListCtrlLinks,i,1,(LPSTR)strtemp.c_str());
	   strtemp = (*it)->getStorageId() ;
	   ListView_SetItemText(m_hListCtrlLinks,i,2,(LPSTR)strtemp.c_str());
	   strtemp = (*it)->getStreamerId();
	   ListView_SetItemText(m_hListCtrlLinks,i,3,(LPSTR)strtemp.c_str());
	   	strtemp = m_ic->proxyToString(*it);
		int len = strtemp.length();
       ListView_SetItemText(m_hListCtrlLinks,i,4,(LPSTR)strtemp.c_str());
	}
}

void CAdPathUI::DisplayStreamLinks()
{
	char temp[15];
	InitListCtrlColunmName();
	HTREEITEM hSelect ,hParent,hRoot; 
	char strTreeText[50];
	hSelect = TreeView_GetSelection(m_hTreeCtrl);
	hParent = TreeView_GetNextItem(m_hTreeCtrl,hSelect, TVGN_PARENT);
	hRoot = TreeView_GetNextItem(m_hTreeCtrl,hParent, TVGN_PARENT); 
	GetTreeViewText(hSelect, strTreeText);
	m_blistLink = TRUE;

	LVITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
    lvItem.state = 0;
	lvItem.iSubItem = 0;
    int i = -1;
	std::string strtemp;

	if(!strcmp(strTreeText,"Storage Links"))
	{
		GetTreeViewText(hParent, strTreeText);	
		::TianShanIce::AccreditedPath::StorageLinks storagelinks ;
		try
		{
		storagelinks = m_client->listStorageLinksByStreamer(strTreeText);
		}
		catch (const Ice::Exception& ex)
		{
			MessageBox(_T(ex.ice_name().c_str()),"connect error");
	      ReInitTree();
			return ;
		}

		ListView_SetItemText(m_hListCtrl,0,1,(LPSTR)itoa(storagelinks.size(), temp , 10));
		for (::TianShanIce::AccreditedPath::StorageLinks::iterator it = storagelinks.begin(); it < storagelinks.end(); it++)
		{
			lvItem.iItem = ++i;
			strtemp = (*it)->getIdent().name;
			lvItem.pszText = (LPSTR)strtemp.c_str(); 
			ListView_InsertItem(m_hListCtrlLinks, &lvItem);	
			strtemp =(*it)->getType() ;
			ListView_SetItemText(m_hListCtrlLinks,i,1,(LPSTR)strtemp.c_str());
			strtemp = (*it)->getStorageId() ;
			ListView_SetItemText(m_hListCtrlLinks,i,2,(LPSTR)strtemp.c_str());
			strtemp = (*it)->getStreamerId();
			ListView_SetItemText(m_hListCtrlLinks,i,3,(LPSTR)strtemp.c_str());
			strtemp = m_ic->proxyToString(*it);
            ListView_SetItemText(m_hListCtrlLinks,i,4,(LPSTR)strtemp.c_str());
		}
	}
	else 
		if(!strcmp(strTreeText,"Streamer Links")&& hRoot == m_hStreamerRoot)
		{
			GetTreeViewText(hParent, strTreeText);	
			::TianShanIce::AccreditedPath::StreamLinks streamlinks;
			try
			{
			 streamlinks = m_client->listStreamLinksByStreamer(strTreeText);	
			}
			catch (const Ice::Exception& ex)
			{
				MessageBox(_T(ex.ice_name().c_str()),"connect error");
					 ReInitTree();
				return;
			}

			ListView_SetItemText(m_hListCtrl,0,1,(LPSTR)itoa(streamlinks.size(), temp , 10));
			for (::TianShanIce::AccreditedPath::StreamLinks::iterator it = streamlinks.begin(); it < streamlinks.end(); it++)
			{
				
				lvItem.iItem = ++i;
				strtemp = (*it)->getIdent().name;
				lvItem.pszText = (LPSTR)strtemp.c_str(); 
				ListView_InsertItem(m_hListCtrlLinks, &lvItem);	
				strtemp =(*it)->getType() ;
				ListView_SetItemText(m_hListCtrlLinks,i,1,(LPSTR)strtemp.c_str());
				strtemp = (*it)->getStreamerId() ;
				ListView_SetItemText(m_hListCtrlLinks,i,2,(LPSTR)strtemp.c_str());
				ListView_SetItemText(m_hListCtrlLinks,i,3,(LPSTR)itoa((*it)->getServiceGroupId(),temp, 10));
				strtemp = m_ic->proxyToString(*it);
                ListView_SetItemText(m_hListCtrlLinks,i,4,(LPSTR)strtemp.c_str());
				
			}
		}
			else
			{
				GetTreeViewText(hParent, strTreeText);
				::TianShanIce::AccreditedPath::StreamLinks streamlinks;
				try
				{
					streamlinks = m_client->listStreamLinksByServiceGroup(atoi(strTreeText));	
				}
				catch (const Ice::Exception& ex)
				{
					MessageBox(_T(ex.ice_name().c_str()),"connect error");
					ReInitTree();
					return;
				}
	
				ListView_SetItemText(m_hListCtrl,0,1,(LPSTR)itoa(streamlinks.size(), temp , 10));
				for (::TianShanIce::AccreditedPath::StreamLinks::iterator it = streamlinks.begin(); it < streamlinks.end(); it++)
				{
					
					lvItem.iItem = ++i;
					strtemp = (*it)->getIdent().name;
					lvItem.pszText = (LPSTR)strtemp.c_str(); 
					ListView_InsertItem(m_hListCtrlLinks, &lvItem);	
					strtemp =(*it)->getType() ;
					ListView_SetItemText(m_hListCtrlLinks,i,1,(LPSTR)strtemp.c_str());
					strtemp = (*it)->getStreamerId() ;
					ListView_SetItemText(m_hListCtrlLinks,i,2,(LPSTR)strtemp.c_str());
					ListView_SetItemText(m_hListCtrlLinks,i,3,(LPSTR)itoa((*it)->getServiceGroupId(),temp, 10));
					strtemp = m_ic->proxyToString(*it);
                    ListView_SetItemText(m_hListCtrlLinks,i,4,(LPSTR)strtemp.c_str());
				}
			}
}

void CAdPathUI::InitListCtrlForPD()
{
	LV_COLUMN lvCol;

	lvCol.mask = LVCF_TEXT | LVCF_WIDTH |LVIF_STATE;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.iSubItem = 0;
	ListView_DeleteColumn(m_hListCtrlLinks, 0);
	lvCol.cx = 150;
	lvCol.pszText = "PrivateData Key";
	ListView_InsertColumn(m_hListCtrlLinks,0,&lvCol);
	
	lvCol.cx = 250;
	lvCol.pszText = "Value";
	ListView_InsertColumn(m_hListCtrlLinks,1,&lvCol);
}

void CAdPathUI::ReInitTree()
{
	if(m_client)
	{
		m_client = NULL;
	}
	m_hStorageRoot = NULL;
	m_hStreamerRoot = NULL;
	m_hSGRoot = NULL;
	m_blistLink = FALSE;
	m_bStorageLink = FALSE;
	m_bStreamerLink = FALSE;
	ListView_DeleteAllItems(m_hListCtrlLinks);
	ListView_DeleteAllItems(m_hListCtrl);
	DeleteListCtrlColumn();
	TreeView_DeleteAllItems(m_hTreeCtrl);
}
