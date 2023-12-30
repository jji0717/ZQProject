// listSample.cpp : Implementation of CSpotExploer

#include "stdafx.h"
#include "SPOTEXPLORER.h"
#include "listSample.h"

/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::InitListCtrl1

void CSpotExploer::InitListCtrl1()
{
	ListView_SetExtendedListViewStyle(m_hListCtrl,LVS_EX_FULLROWSELECT| LVS_EX_HEADERDRAGDROP);
	::GetWindowLong(m_hListCtrl,GWL_EXSTYLE | GWL_EXSTYLE & LVS_EX_FULLROWSELECT);

	LV_COLUMN lvCol;
	lvCol.mask = LVCF_TEXT | LVCF_WIDTH |LVIF_STATE;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.iSubItem = 0;
	
	ListView_DeleteColumn(m_hListCtrl,0);
	lvCol.cx = 120;
	lvCol.pszText = "KeyName";
	ListView_InsertColumn(m_hListCtrl,0,&lvCol);
	
	lvCol.cx = 300;
	lvCol.pszText = "Value";
	ListView_InsertColumn(m_hListCtrl,1,&lvCol);
}

/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::InitListCtrl2

void CSpotExploer::InitListCtrl2()
{
	ListView_SetExtendedListViewStyle(m_hListCtrl2,LVS_EX_FULLROWSELECT| LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
	::GetWindowLong(m_hListCtrl2,GWL_EXSTYLE | GWL_EXSTYLE & LVS_EX_FULLROWSELECT);

	LV_COLUMN lvCol;
	lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.iSubItem = 0;
	
	//添加表头
	lvCol.cx = 0;
	lvCol.pszText = "";
	ListView_InsertColumn(m_hListCtrl2,0,&lvCol);	
}

/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::InitTreeIcon

/*void CSpotExploer::InitTreeIcon()
{
	//添加图标
	m_hImlIcons = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 3, 1); 
	HICON hicon;           // handle to icon 
	
	hicon = LoadIcon(_Module.m_hInstResource,MAKEINTRESOURCE(IDI_ICON_ROOT));
	ImageList_AddIcon(m_hImlIcons,hicon);
	DestroyIcon(hicon);
	
	
	hicon = LoadIcon(_Module.m_hInstResource,MAKEINTRESOURCE(IDI_ICON2));
	ImageList_AddIcon(m_hImlIcons,hicon);
	DestroyIcon(hicon);
	
	
	hicon = LoadIcon(_Module.m_hInstResource,MAKEINTRESOURCE(IDI_ICON3));
	ImageList_AddIcon(m_hImlIcons,hicon);
	DestroyIcon(hicon);
	
	hicon = LoadIcon(_Module.m_hInstResource,MAKEINTRESOURCE(IDI_ICON4));
	ImageList_AddIcon(m_hImlIcons,hicon);
    DestroyIcon(hicon);
}*/
void CSpotExploer::InitTreeIcon()
{
	//添加图标
	m_hImlIcons = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 4, 1); 
	HICON hicon;           // handle to icon 
	
	hicon = LoadIcon(_Module.m_hInstResource,MAKEINTRESOURCE(IDI_DOMAIN));
	ImageList_AddIcon(m_hImlIcons,hicon);
	DestroyIcon(hicon);
	
	
	hicon = LoadIcon(_Module.m_hInstResource,MAKEINTRESOURCE(IDI_ROOT));
	ImageList_AddIcon(m_hImlIcons,hicon);
	DestroyIcon(hicon);
	
	
	hicon = LoadIcon(_Module.m_hInstResource,MAKEINTRESOURCE(IDI_SPOT));
	ImageList_AddIcon(m_hImlIcons,hicon);
	DestroyIcon(hicon);
	
	hicon = LoadIcon(_Module.m_hInstResource,MAKEINTRESOURCE(IDI_IF));
	ImageList_AddIcon(m_hImlIcons,hicon);
    DestroyIcon(hicon);

	hicon = LoadIcon(_Module.m_hInstResource,MAKEINTRESOURCE(IDI_ALLIF));
	ImageList_AddIcon(m_hImlIcons,hicon);
    DestroyIcon(hicon);

	hicon = LoadIcon(_Module.m_hInstResource,MAKEINTRESOURCE(IDI_REMOVE));
	ImageList_AddIcon(m_hImlIcons,hicon);
    DestroyIcon(hicon);
}
/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::InitTreeCtrl

void CSpotExploer::InitTreeCtrl()
{
	ListView_DeleteAllItems(m_hListCtrl);
	TreeView_DeleteAllItems(m_hTreeCtrl);
	m_hRoot = NULL;
	m_hServerRoot = NULL;

	//增加 TreeCtrl 的 TVS_HASBUTTONS，TVS_HASLINES、TVS_LINESATROOT Style
	DWORD dwStyle = ::GetWindowLong(m_hTreeCtrl,GWL_STYLE);
    dwStyle |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT |TVS_SHOWSELALWAYS;
    ::SetWindowLong(m_hTreeCtrl,GWL_STYLE,dwStyle);
	TreeView_SetImageList(m_hTreeCtrl,m_hImlIcons,TVSIL_NORMAL);
	
	//创建节点
	//父节点
	TVITEM tvi = {0};
	TVINSERTSTRUCT tvs;
	tvs.hInsertAfter = TVI_LAST; 
	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE ;
	
	tvi.pszText = "Domain";
	tvi.iImage = 0;
	tvi.iSelectedImage = 0;
	tvs.hParent = TVI_ROOT;
	tvs.item = tvi;	
	
	m_hRoot = TreeView_InsertItem(m_hTreeCtrl, &tvs);
	
	AddallNode();
	
	tvi.pszText = "All Interface";
	tvi.iImage = 4;
	tvi.iSelectedImage = 4;
	tvs.hParent = m_hRoot;
	tvs.item = tvi;	
	m_hServerRoot = TreeView_InsertItem(m_hTreeCtrl, &tvs);	
}

/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::OnClickTree1

LRESULT CSpotExploer::OnClickTree1(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{  	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::OnDbClickTree1

LRESULT CSpotExploer::OnDbClickTree1(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	//  MessageBox("test OnDbClickTree");
	HTREEITEM hselect = NULL,hParent = NULL,hrootParent = NULL;
    hselect = TreeView_GetSelection(m_hTreeCtrl);
	if(hselect)
		hParent = TreeView_GetNextItem(m_hTreeCtrl,hselect,TVGN_PARENT);
	if(hParent)
		hrootParent = TreeView_GetNextItem(m_hTreeCtrl,hParent,TVGN_PARENT); 

	if( NULL==hselect || hselect == m_hRoot)//当双击Domain时
	{
		m_nType = NOTYPE;
		InitTreeCtrl();
	}
	else 
		if(hselect == m_hServerRoot)//当双击All interface时
		{
			m_nType = NOTYPE;
			AddallInterface();
		}
		else
			if(hParent == m_hRoot)//当双击某个Node时
			{
				m_nType = NOTYPE;
				AddNodeSpot();
			}
			else
				if(hParent == m_hServerRoot)
				{
					DisplayIF();
				}
				else 
					if(hrootParent == m_hRoot) 
					{
						//MessageBox("double click spot");
						m_nType = NOTYPE;
						AddSpotInterface();
					}
					else
					{
						DispalySpotIfInfo();
					}
					
					return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::OnSelchangedTree1

LRESULT CSpotExploer::OnSelchangedTree1(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{

	HTREEITEM hselect = NULL,hParent = NULL,hrootParent = NULL;
    hselect = TreeView_GetSelection(m_hTreeCtrl);
	if(hselect)
		hParent = TreeView_GetNextItem(m_hTreeCtrl,hselect,TVGN_PARENT);
	if(hParent)
		hrootParent = TreeView_GetNextItem(m_hTreeCtrl,hParent,TVGN_PARENT); 
	
	if( NULL==hselect || hselect == m_hRoot)//当双击Domain时
	{
		m_nType = NOTYPE;
		InitTreeCtrl();
	}
	else 
		if(hselect == m_hServerRoot)//当双击All interface时
		{
			m_nType = NOTYPE;
			AddallInterface();
		}
		else
			if(hParent == m_hRoot)//当双击某个Node时
			{
				m_nType = NOTYPE;
				displayNodeInfo();
			}
			else
				if(hParent == m_hServerRoot)
				{
					DisplayIF();
				}
				else 
					if(hrootParent == m_hRoot) 
					{
						//MessageBox("double click spot");
						m_nType = NOTYPE;
						displaySpotInfo();
					}
					else
					{
						DispalySpotIfInfo();
					}
					
					return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::AddallNode

void CSpotExploer::AddallNode()
{
	LVITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
	lvItem.state = 0;
	
	TVITEM tvi = {0};
	TVINSERTSTRUCT tvs;
	tvs.hInsertAfter = TVI_LAST;
	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE ;
	ListView_DeleteAllItems(m_hListCtrl);
	ListView_DeleteAllItems(m_hListCtrl2);
	DeleteListCtrlColumn();
    ZQ::common::Variant var;
	if (m_pSpotData->getNodeInfo(var, NULL)) 
	{
		//display all node info in ClistCtrl;
		int j = var.size();
		char temp[10];
		lvItem.iItem = 0;
		lvItem.iSubItem = 0;
		lvItem.pszText = "Node count"; 
		ListView_InsertItem(m_hListCtrl, &lvItem);
		ListView_SetItemText(m_hListCtrl,0,1, itoa(j, temp, 10));
		
		//add all node in treeview
		for (int i = 0; i < var.size(); i ++) 
		{
			ZQ::common::Variant& node = var[i];
			const char* keyName = (LPSTR)node.key(1).c_str();
			std::string str = ((tstring)node[keyName]);
			tvi.pszText = (LPSTR)str.c_str();
            tvi.iImage = 1;
			tvi.iSelectedImage = 1;
			tvs.hParent = m_hRoot;
			tvs.item = tvi;
            TreeView_InsertItem(m_hTreeCtrl, &tvs);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::AddNodeSpot

void CSpotExploer::AddNodeSpot()
{
   	HTREEITEM hSelect = TreeView_GetSelection(m_hTreeCtrl);
    DeleteTreeItem(hSelect);
	
	TVITEM tvi = {0};
	TVINSERTSTRUCT tvs;
	tvs.hInsertAfter = TVI_LAST;
	
	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE ;
	
	ListView_DeleteAllItems(m_hListCtrl);
	ListView_DeleteAllItems(m_hListCtrl2);
	DeleteListCtrlColumn();
	
    LVITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE | TVIF_PARAM; 
	lvItem.state = 0;
	
    ZQ::common::Variant var;
	char pNode[100];
	GetTreeViewText(hSelect,pNode);//node id;
	
	//display node info
	if (m_pSpotData->getNodeInfo(var, pNode)) 
	{
		ZQ::common::Variant& node = var[0];
		
		for (int i = 0; i < node.size(); i++ ) 
		{		
			char *keyName = (LPSTR)node.key(i).c_str();
			lvItem.iItem = i;
			lvItem.iSubItem = 0;
			lvItem.pszText = (LPSTR)keyName; 
			ListView_InsertItem(m_hListCtrl, &lvItem);
			std::string str = ((tstring)node[keyName]);
			ListView_SetItemText(m_hListCtrl,i,1, (LPSTR)(LPCTSTR)(str.c_str()));
		}
	}
	//add node spot info in treeview
	if (m_pSpotData->getSpotInfo(var,pNode, NULL)) 
	{
		for (int i = 0; i < var.size(); i ++) 
		{
			ZQ::common::Variant& spot = var[i];
			const char* keyName = (LPSTR)spot.key(0).c_str();
			std::string str = ((tstring)spot[keyName]);
			for(int k = 0; k < spot.size(); k++)
			{
				keyName = (LPSTR)spot.key(k).c_str();
				if(!strcmp(keyName,"processId"))
                  break;
			}
		
			int proId = (int)spot[keyName];
			char temp[10];
			itoa(proId, temp, 10);
			str = str + "(" + temp +")";
			tvi.pszText = (LPSTR)str.c_str();
            tvi.iImage = 2;
			tvi.iSelectedImage = 2; 
			tvs.hParent = hSelect;
			tvs.item = tvi;
            TreeView_InsertItem(m_hTreeCtrl, &tvs);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::AddallInterface

void CSpotExploer::AddallInterface()
{
    DeleteTreeItem(m_hServerRoot);
	
	TVITEM tvi = {0};
	TVINSERTSTRUCT tvs;
	tvs.hInsertAfter = TVI_LAST;
	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE ;
    
	ListView_DeleteAllItems(m_hListCtrl);
	ListView_DeleteAllItems(m_hListCtrl2);
    DeleteListCtrlColumn();
	
	LVITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 	
	lvItem.state = 0;
	
	ZQ::common::Variant var;
	if (m_pSpotData->listInterfaces(var, NULL)) 
	{
		//display all interface info in ClistCtrl
		int j = var.size();
		char temp[10];
		lvItem.iItem = 0;
		lvItem.iSubItem = 0;
		lvItem.pszText = "Interface count"; 
		ListView_InsertItem(m_hListCtrl, &lvItem);
		ListView_SetItemText(m_hListCtrl,0,1, itoa(j, temp, 10));
		
		//add all interface in treeview
		for (int i = 0; i < var.size(); i++) 
		{
			ZQ::common::Variant& InterFace = var[i];
			const char *keyName = (LPSTR)InterFace.key(1).c_str();
			std::string str = ((tstring)InterFace[keyName]);
			tvi.pszText = (LPSTR)str.c_str();
            tvi.iImage = 3;
			tvi.iSelectedImage = 3;
			tvs.hParent = m_hServerRoot;
			tvs.item = tvi;
            TreeView_InsertItem(m_hTreeCtrl, &tvs);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::AddSpotInterface

void CSpotExploer::AddSpotInterface()
{
	
	HTREEITEM hSelect,hParent;
    hSelect = TreeView_GetSelection(m_hTreeCtrl);
	hParent = TreeView_GetNextItem(m_hTreeCtrl, hSelect, TVGN_PARENT );
    DeleteTreeItem(hSelect);
	
	ListView_DeleteAllItems(m_hListCtrl);
	ListView_DeleteAllItems(m_hListCtrl2);
	DeleteListCtrlColumn();
	
	TVITEM tvi = {0};
	TVINSERTSTRUCT tvs;
	tvs.hInsertAfter = TVI_LAST;	
	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE ;
	
    ListView_DeleteAllItems(m_hListCtrl);
    LVITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
	lvItem.state = 0;
	
    ZQ::common::Variant var;
	char pNode[100], pSpot[100];
	GetTreeViewText(hParent, pNode);//node id;
	GetTreeViewText(hSelect, pSpot);//Spot id;
	string strtemp = pSpot;
	string str = strtemp.substr(strtemp.find('(') + 1, strtemp.find(')') -1);
	int proId = atoi (str.c_str());
	//display node spot info
	if (m_pSpotData->getSpotInfo(var, pNode, proId)) 
	{
		ZQ::common::Variant& spot = var[0];
		
		for (int i = 0; i < spot.size() ; i++ )
		{		
			char *keyName = (LPSTR)spot.key(i).c_str();
			std::string str;
			char temp[20];
			int proId;
			lvItem.iItem = i;
			lvItem.iSubItem = 0;
			lvItem.pszText = (LPSTR)keyName;
            ListView_InsertItem(m_hListCtrl, &lvItem);
			
			if(!strcmp(keyName, "processId"))
			{
				proId = (int)spot[keyName];
				str = itoa(proId, temp, 10);
			}
			else
				str = ((tstring)spot[keyName]);			
			ListView_SetItemText(m_hListCtrl,i,1, (LPSTR)(LPCTSTR)(str.c_str()));
		}
		
	}
	// add node spot interface info in treeview
	if (m_pSpotData->getInterfaceInfo(var,pNode, proId, NULL)) 
	{
		for (int i = 0; i < var.size(); i ++) 
		{
			ZQ::common::Variant& spotIf = var[i];
			const char* keyName = (LPSTR)spotIf.key(2).c_str();
			std::string str = ((tstring)spotIf[keyName]);
			tvi.pszText = (LPSTR)str.c_str();
			tvi.iImage = 3;
			tvi.iSelectedImage = 3;
			tvs.hParent = hSelect;
			tvs.item = tvi;
			TreeView_InsertItem(m_hTreeCtrl, &tvs);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::displayNodeInfo

void CSpotExploer::displayNodeInfo()
{
	HTREEITEM hSelect;
    hSelect = TreeView_GetSelection(m_hTreeCtrl);
	ListView_DeleteAllItems(m_hListCtrl);
	ListView_DeleteAllItems(m_hListCtrl2);
	DeleteListCtrlColumn();
	
    LVITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE | TVIF_PARAM; 
	lvItem.state = 0;
	
    ZQ::common::Variant var;
	char pNode[100];
	GetTreeViewText(hSelect,pNode);//node id;
	
	//display node info
	if (m_pSpotData->getNodeInfo(var, pNode)) 
	{
		ZQ::common::Variant& node = var[0];
		
		for (int i = 0; i < node.size(); i++ ) 
		{		
			char *keyName = (LPSTR)node.key(i).c_str();
			lvItem.iItem = i;
			lvItem.iSubItem = 0;
			lvItem.pszText = (LPSTR)keyName; 
			ListView_InsertItem(m_hListCtrl, &lvItem);
			std::string str = ((tstring)node[keyName]);
			ListView_SetItemText(m_hListCtrl,i,1, (LPSTR)(LPCTSTR)(str.c_str()));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::displaySpotInfo

void CSpotExploer::displaySpotInfo()
{
  HTREEITEM hSelect,hParent;
    hSelect = TreeView_GetSelection(m_hTreeCtrl);
	hParent = TreeView_GetNextItem(m_hTreeCtrl, hSelect, TVGN_PARENT );
	
	ListView_DeleteAllItems(m_hListCtrl);
	ListView_DeleteAllItems(m_hListCtrl2);
	DeleteListCtrlColumn();
	
    ListView_DeleteAllItems(m_hListCtrl);
    LVITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
	lvItem.state = 0;
	
    ZQ::common::Variant var;
	char pNode[100], pSpot[100];
	GetTreeViewText(hParent, pNode);//node id;
	GetTreeViewText(hSelect, pSpot);//Spot id;
	string strtemp = pSpot;
	string str = strtemp.substr(strtemp.find('(') + 1, strtemp.find(')') -1);
	int proId = atoi (str.c_str());
	//display node spot info
	if (m_pSpotData->getSpotInfo(var, pNode, proId)) 
	{
		ZQ::common::Variant& spot = var[0];
		
		for (int i = 0; i < spot.size() ; i++ )
		{		
			char *keyName = (LPSTR)spot.key(i).c_str();
			std::string str;
			char temp[20];
			int proId;
			lvItem.iItem = i;
			lvItem.iSubItem = 0;
			lvItem.pszText = (LPSTR)keyName;
            ListView_InsertItem(m_hListCtrl, &lvItem);
			
			if(!strcmp(keyName, "processId"))
			{
				proId = (int)spot[keyName];
				str = itoa(proId, temp, 10);
			}
			else
				str = ((tstring)spot[keyName]);			
			ListView_SetItemText(m_hListCtrl,i,1, (LPSTR)(LPCTSTR)(str.c_str()));
		}
		
	}
}


/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::DispalySpotIfInfo

void CSpotExploer::DispalySpotIfInfo()
{
    HTREEITEM hSelect, hSpotParent, hNodeParent;
    hSelect = TreeView_GetSelection(m_hTreeCtrl);
	hSpotParent = TreeView_GetNextItem(m_hTreeCtrl, hSelect, TVGN_PARENT );
	hNodeParent = TreeView_GetNextItem(m_hTreeCtrl, hSpotParent, TVGN_PARENT );
	
	ListView_DeleteAllItems(m_hListCtrl);
	ListView_DeleteAllItems(m_hListCtrl2);
	DeleteListCtrlColumn();
	
    LVITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
	lvItem.state = 0;
	
    ZQ::common::Variant var;
	char pNode[100], pSpot[100],pIf[100];
	GetTreeViewText(hNodeParent, pNode);//node id;
	GetTreeViewText(hSpotParent, pSpot);//Spot id;
	string strtemp = pSpot;
	string strsub = strtemp.substr(strtemp.find('(') + 1, strtemp.find(')') -1);
	int proId = atoi (strsub.c_str());
    GetTreeViewText(hSelect, pIf);//Spot id;
	
	//display node spot info
	if (m_pSpotData->getInterfaceInfo(var, pNode, proId, pIf)) 
	{
		ZQ::common::Variant& SpotIf = var[0];
		int load;
		std::string str;
		char temp[10];
		for (int i = 0; i < SpotIf.size(); i++ ) 
		{		
			char *keyName = (LPSTR)SpotIf.key(i).c_str();
			lvItem.iItem = i;
			lvItem.iSubItem = 0;
			lvItem.pszText = (LPSTR)keyName; 
			ListView_InsertItem(m_hListCtrl, &lvItem);
			if(!strcmp(keyName, "load"))
			{
				load = (int)SpotIf[keyName];
				str=   itoa(load, temp, 10);
			}
			else	
				str = ((tstring)SpotIf[keyName]);
			ListView_SetItemText(m_hListCtrl,i,1, (LPSTR)(LPCTSTR)(str.c_str()));
		}
	}
	
	m_nType = SPOTIF;
}
/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::DisplayIF

void CSpotExploer::DisplayIF()
{	
	ListView_DeleteAllItems(m_hListCtrl);
	ListView_DeleteAllItems(m_hListCtrl2);
	DeleteListCtrlColumn();
	
	LVITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 	
	lvItem.state = 0;
	
	LV_COLUMN lvCol;
	lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.iSubItem = 0;
	
	HTREEITEM hSelect = TreeView_GetSelection(m_hTreeCtrl);
    ZQ::common::Variant var;
	char pIf[100];
	GetTreeViewText(hSelect,pIf);//Interface name;
	//display node info
	if (m_pSpotData->listInterfaces(var, pIf)) 
	{
        const char* keyName;
		char temp[10];
		std::string str;
		int load;
		int j = var.size();
		lvItem.iItem = 0;
		lvItem.iSubItem = 0;
		lvItem.pszText = "count"; 
		ListView_InsertItem(m_hListCtrl, &lvItem);
		ListView_SetItemText(m_hListCtrl,0,1, itoa(j, temp, 10));
		
		for (int i = 0; i < var[0].size(); i++ ) 
		{
			keyName = var[0].key(i).c_str();
			lvCol.cx = 150;
			lvCol.pszText = (LPSTR)keyName;
			ListView_InsertColumn(m_hListCtrl2,i,&lvCol);
		}
		
		for( j = 0; j < var.size(); j++)
		{
			ZQ::common::Variant& InterFace = var[j];
		    keyName = (LPSTR)InterFace.key(0).c_str();
			str = ((tstring)InterFace[keyName]);
			lvItem.iItem = j;
			lvItem.iSubItem = 0;
			lvItem.pszText = (LPSTR)(LPCTSTR)(str.c_str()); 
			ListView_InsertItem(m_hListCtrl2, &lvItem);
			for (i = 1; i < InterFace.size(); i++ ) 
			{
				keyName = (LPSTR)InterFace.key(i).c_str();
				if(!strcmp(keyName, "load") || !strcmp(keyName, "processId") )
				{
					load = (int)InterFace[keyName];
					str = itoa(load, temp, 10);
				}
				else
					str = ((tstring)InterFace[keyName]);
				
				ListView_SetItemText(m_hListCtrl2,j,i, (LPSTR)(LPCTSTR)(str.c_str()));
			}
		}
		m_nType = ALLIF;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::InitSpotStatus

void CSpotExploer::InitSpotStatus()
{
	m_pSpotData = g_pspotEnv->getStatusQuery();
}

/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::GetTreeViewText

void CSpotExploer::GetTreeViewText(HTREEITEM hitem, char *pstr)
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

/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::DeleteTreeItem

void CSpotExploer::DeleteTreeItem(HTREEITEM hitem)
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

/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::DeleteListCtrlColumn

void CSpotExploer::DeleteListCtrlColumn()
{
	int ncol = 10;
	while(ncol--)
	{
		ListView_DeleteColumn(m_hListCtrl2, 0);
	}
	InitListCtrl2();
}

/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::CreateStatusBar

void CSpotExploer::CreateStatusBar()
{
	m_hStatusWindow=CreateStatusWindow(WS_CHILD|WS_VISIBLE|WS_BORDER,
		TEXT("Status"),m_hWnd, IDS_STATUS);
	
	int pint[3]={60,760,-1};
	
	::SendMessage(m_hStatusWindow,SB_SETPARTS,3,(LPARAM)pint);
	::SendMessage(m_hStatusWindow,SB_SETTEXT,1,(LPARAM)TEXT("    No change !"));

	HICON hIcon = (HICON)::LoadImage(_Module.m_hInstResource,MAKEINTRESOURCE(IDI_UNCHANGE),IMAGE_ICON, 16, 16, LR_SHARED);
	::SendMessage(m_hStatusWindow,SB_SETICON,0,(LPARAM)(HICON)hIcon);
	DestroyIcon(hIcon);
}
/////////////////////////////////////////////////////////////////////////////
// CSpotExploer::InitStatusEvent

void CSpotExploer::InitStatusEvent()
{
	m_pSpotStatus = new SpotStatusEvent(m_hStatusWindow, m_hTreeCtrl);
	if(m_pSpotStatus)
	m_pSpotData->sinkStatusChangeEvents(m_pSpotStatus);
}

void CSpotExploer::test()
{
	DWORD LpTest;
	::CreateThread(NULL,NULL,DataChange,NULL,1,&LpTest);
}

