#include "stdafx.h"
// -----------------------------
// class SpotStatusEvent
// -----------------------------
class SpotStatusEvent: public ZQ::Spot::SpotStatusChangeBind
{
public:
    HWND m_hStatusWindow;
	HWND m_hTreeCtrl;
	SpotStatusEvent(HWND hstatusWindow,HWND hTree)
		:m_hStatusWindow(hstatusWindow), m_hTreeCtrl(hTree)
	{
		
	}
    /// when a new node is added
	///@param[out]	nodeId addnode id 
	virtual void onNodeAdded(const std::string& nodeId)
	{ 
		std::string strtemp = nodeId + " is Added ! please double click Domain refresh!";
        ::SendMessage(m_hStatusWindow,SB_SETTEXT,1,(LPARAM)TEXT(strtemp.c_str()));
		ChangeLedStatus();
	}
	
	/// when a node is removed
	///@param[out]	nodeId removedNode id info
	virtual void onNodeRemoved(const std::string& nodeId) 
	{
        std::string strtemp = nodeId + " is Removed !";
        ::SendMessage(m_hStatusWindow,SB_SETTEXT,1,(LPARAM)TEXT(strtemp.c_str()));
		ChangeLedStatus();
		ChangeTreeStatus(nodeId,NULL);
	}
	
	/// when a new spot is added
	///@param[out]	nodeid addspot node id  info
	///@param[out]	pid    spot processid  info
	virtual void onSpotAdded(const std::string& nodeId, int pid) 
	{
		char temp[10];
		std::string strtemp = "Node id :"+ nodeId + ",  Spot Id: " + itoa(pid, temp, 10) + " id add !";
        ::SendMessage(m_hStatusWindow,SB_SETTEXT,1,(LPARAM)TEXT(strtemp.c_str()));
        ChangeLedStatus();		
	} 
	
	/// when a  spot is Removed
	///@param[out]	nodeid addspot node id  info
	///@param[out]	pid    spot processid  info
	virtual void onSpotRemoved(const std::string& nodeId, int pid) 
	{
		char temp[10];
		std::string strtemp = "Node id :"+ nodeId + ",  Spot Id: " + itoa(pid, temp, 10) + " id Removed !";
        ::SendMessage(m_hStatusWindow,SB_SETTEXT,1,(LPARAM)TEXT(strtemp.c_str()));
        ChangeLedStatus();
		ChangeTreeStatus(nodeId, pid);
	} 
	
	///change the statusbar info
	void ChangeLedStatus()
	{
		HICON hIcon = (HICON)::LoadImage(_Module.m_hInstResource,MAKEINTRESOURCE(IDI_CHANGE),IMAGE_ICON, 16, 16, LR_SHARED);
		::SendMessage(m_hStatusWindow,SB_SETICON,0,(LPARAM)(HICON)hIcon);
		DestroyIcon(hIcon);
		Beep(900,300);
		::Sleep(2000);
		
		hIcon = (HICON)::LoadImage(_Module.m_hInstResource,MAKEINTRESOURCE(IDI_UNCHANGE),IMAGE_ICON, 16, 16, LR_SHARED);
		::SendMessage(m_hStatusWindow,SB_SETICON,0,(LPARAM)(HICON)hIcon);
        DestroyIcon(hIcon);	
		::SendMessage(m_hStatusWindow,SB_SETTEXT,1,(LPARAM)TEXT(" not changed!"));
	}
	
	void ChangeTreeStatus(const std::string& nodeId, int pid = NULL)
	{  
		char temp[100];
		std::string str,strtemp;
		HTREEITEM hroot, hNode, hSpot; 
		hroot = TreeView_GetRoot(m_hTreeCtrl);

		if( pid != NULL)
		{
			if(hroot)
			{
				hNode =TreeView_GetNextItem(m_hTreeCtrl, hroot,TVGN_CHILD);
				while(hNode)
				{   
					GetTreeViewText(hNode, temp);
					str = temp;
					if(str == nodeId)
						break;
					hNode = TreeView_GetNextItem(m_hTreeCtrl, hNode,TVGN_NEXT);
				}
				if(hNode)
				{
					hSpot =TreeView_GetNextItem(m_hTreeCtrl, hNode,TVGN_CHILD);
					while(hSpot)
					{
						GetTreeViewText(hSpot, temp);
						strtemp = temp;
						str = strtemp.substr(strtemp.find('(') + 1, strtemp.find(')') -1);
						int proId = atoi (str.c_str());
						if(pid == proId)
							break;
						hSpot = TreeView_GetNextItem(m_hTreeCtrl, hSpot,TVGN_NEXT);
					}
					if(hSpot)
					{
					   ///...
						SetTreeViewImag(hSpot);
						DeleteTreeItem(hSpot);
					}
				}
			}
		}
		else
		{
			if(hroot)
			{
				hNode =TreeView_GetNextItem(m_hTreeCtrl, hroot,TVGN_CHILD);
				while(hNode)
				{   
					GetTreeViewText(hNode, temp);
					str = temp;
					if(str == nodeId)
						break;
					hNode = TreeView_GetNextItem(m_hTreeCtrl, hNode,TVGN_NEXT);
				}
				if(hNode)
				{
	                   ///...	
					SetTreeViewImag(hNode);
					DeleteTreeItem(hNode);
				}
			}
		}
	}
	
	void GetTreeViewText(HTREEITEM hitem, char *pstr)
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
	void SetTreeViewImag(HTREEITEM hitem)
	{
	    TVITEM tvi = { 0 };   

		tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.hItem = hitem; 
		tvi.iImage = 5;
		tvi.iSelectedImage = 5;
		TreeView_SetItem(m_hTreeCtrl,&tvi);
	}	
	
	void DeleteTreeItem(HTREEITEM hitem)
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
};