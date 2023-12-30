// AdPathUI.h : Declaration of the CAdPathUI

#ifndef __ADPATHUI_H_
#define __ADPATHUI_H_



#include <atlctl.h>
#include "mydialog.h"
#include "resource.h"       // main symbols
#include <Winuser.h>
/////////////////////////////////////////////////////////////////////////////
// CAdPathUI
class ATL_NO_VTABLE CAdPathUI : 
public CComObjectRootEx<CComSingleThreadModel>,
public IDispatchImpl<IAdPathUI, &IID_IAdPathUI, &LIBID_ADPATHEXPLORERLib>,
public CComCompositeControl<CAdPathUI>,
public IPersistStreamInitImpl<CAdPathUI>,
public IOleControlImpl<CAdPathUI>,
public IOleObjectImpl<CAdPathUI>,
public IOleInPlaceActiveObjectImpl<CAdPathUI>,
public IViewObjectExImpl<CAdPathUI>,
public IOleInPlaceObjectWindowlessImpl<CAdPathUI>,
public ISupportErrorInfo,
public IConnectionPointContainerImpl<CAdPathUI>,
public IPersistStorageImpl<CAdPathUI>,
public ISpecifyPropertyPagesImpl<CAdPathUI>,
public IQuickActivateImpl<CAdPathUI>,
public IDataObjectImpl<CAdPathUI>,
public IProvideClassInfo2Impl<&CLSID_AdPathUI, &DIID__IAdPathUIEvents, &LIBID_ADPATHEXPLORERLib>,
public IPropertyNotifySinkCP<CAdPathUI>,
public CComCoClass<CAdPathUI, &CLSID_AdPathUI>
{
public:
	CAdPathUI()
	{
		m_bWindowOnly = TRUE;
		CalcExtent(m_sizeExtent);
	}
	
	DECLARE_REGISTRY_RESOURCEID(IDR_ADPATHUI)
		
		DECLARE_PROTECT_FINAL_CONSTRUCT()
		
		BEGIN_COM_MAP(CAdPathUI)
		COM_INTERFACE_ENTRY(IAdPathUI)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IViewObjectEx)
		COM_INTERFACE_ENTRY(IViewObject2)
		COM_INTERFACE_ENTRY(IViewObject)
		COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
		COM_INTERFACE_ENTRY(IOleInPlaceObject)
		COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
		COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
		COM_INTERFACE_ENTRY(IOleControl)
		COM_INTERFACE_ENTRY(IOleObject)
		COM_INTERFACE_ENTRY(IPersistStreamInit)
		COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
		COM_INTERFACE_ENTRY(ISupportErrorInfo)
		COM_INTERFACE_ENTRY(IConnectionPointContainer)
		COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
		COM_INTERFACE_ENTRY(IQuickActivate)
		COM_INTERFACE_ENTRY(IPersistStorage)
		COM_INTERFACE_ENTRY(IDataObject)
		COM_INTERFACE_ENTRY(IProvideClassInfo)
		COM_INTERFACE_ENTRY(IProvideClassInfo2)
		END_COM_MAP()
		
		BEGIN_PROP_MAP(CAdPathUI)
		PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
		PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
		// Example entries
		// PROP_ENTRY("Property Description", dispid, clsid)
		// PROP_PAGE(CLSID_StockColorPage)
		END_PROP_MAP()
		
		BEGIN_CONNECTION_POINT_MAP(CAdPathUI)
		CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
		END_CONNECTION_POINT_MAP()
		
		BEGIN_MSG_MAP(CAdPathUI)
		CHAIN_MSG_MAP(CComCompositeControl<CAdPathUI>)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_TREE, NM_RCLICK, OnRclickTree)
		
		//	COMMAND_HANDLER(IDC_BUTTON1,BN_CLICKED,OnClickAddStorage)
		NOTIFY_HANDLER(IDC_TREE, TVN_SELCHANGED, OnSelchangedTree)
		NOTIFY_HANDLER(IDC_TREE, NM_CLICK, OnClickTree)
		NOTIFY_HANDLER(IDC_LIST1, NM_DBLCLK, OnDblclkList1)
		COMMAND_HANDLER(IDC_CONNECTBTN, BN_CLICKED, OnClickedConnectbtn)
		NOTIFY_HANDLER(IDC_LIST1, HDN_BEGINTRACK, OnBegintrackList1)
		END_MSG_MAP()
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		
		BEGIN_SINK_MAP(CAdPathUI)
		//Make sure the Event Handlers have __stdcall calling convention
		END_SINK_MAP()
		
		STDMETHOD(OnAmbientPropertyChange)(DISPID dispid)
	{
		if (dispid == DISPID_AMBIENT_BACKCOLOR)
		{
			SetBackgroundColorFromAmbient();
			FireViewChange();
		}
		return IOleControlImpl<CAdPathUI>::OnAmbientPropertyChange(dispid);
	}
	
	
	
	// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
	{
		static const IID* arr[] = 
		{
			&IID_IAdPathUI,
		};
		for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++)
		{
			if (::InlineIsEqualGUID(*arr[i], riid))
				return S_OK;
		}
		return S_FALSE;
	}
	
	// IViewObjectEx
	DECLARE_VIEW_STATUS(0)
		
		// IAdPathUI
public:
    Ice::CommunicatorPtr m_ic;
	TianShanIce::AccreditedPath::PathAdminPrx m_client;
	
	HWND m_hListCtrl;
	HWND m_hListCtrlLinks;
	HWND m_hTreeCtrl;
	//	 HWND m_hStatusWindow;
	HMENU m_hMenu;
	
	HTREEITEM m_hStorageRoot;
	HTREEITEM m_hStreamerRoot;
	HTREEITEM m_hSGRoot;
	BOOL m_blistLink;
	BOOL m_bStorageLink;
	BOOL m_bStreamerLink;
public:
	void ReInitTree();
	///Init ListCtrl2 For PrivateData
	void InitListCtrlForPD();

	///Init ListCtrl Colunm Name
	void InitListCtrlColunmName();

	///Removes all Column from ListCtrl2
	void DeleteListCtrlColumn();

	///Display StreamLinks in ListCtrl2
	void DisplayStreamLinks();

	///Display StorageLinks in ListCtrl2
	void DisplayStorageLinks();

    ///Display ServiceGroup Info in ListCtrl
	///@parme[in]  data the struct receive ServiceGroup info
	///            if data is NULL, not receive ServiceGroup info
	void DisplaySGInfo(SData *data = NULL);

    ///Display Streamer Info in ListCtrl
	///@parme[in]  data the struct receive Streamer info
	///            if data is NULL, not receive Streamer info
	void DisplayStreamerInfo(SData *data = NULL);

	///Display Storage Info in ListCtrl
	///@parme[in]  data the struct receive Storage info
	///            if data is NULL, not receive Storage info
	void DisplayStorageInfo(SData *data = NULL);

	///List ServiceGroup ID in tree view control
	void listSGId();

   ///List Streamer ID in tree view control
	void listStreamerId();

	///List Storage ID in tree view control
	void ListStorageId();

    ///Init listCtrl1
	void InitListCtrl();

	///Init Tree view control
	void InitTreeCtrl();

	///Removes  all children item from a tree view control
    ///@param[in] hitem  handle to the item to delete
	void DeleteTreeItem(HTREEITEM hitem);

	void test();

	///get an item text from a tree view control
    ///@param[in] hitem  handle to the item to get
	///@param[out] pstr  an array that receive the item text
    void GetTreeViewText(HTREEITEM hitem, char *pstr);

	enum { IDD = IDD_ADPATHUI };
	
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// TODO : Add Code for message handler. Call DefWindowProc if necessary.
		return 0;
	}
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// TODO : Add Code for message handler. Call DefWindowProc if necessary.
	    int i = 0;
		m_hTreeCtrl = GetDlgItem(IDC_TREE);
		m_hListCtrl = GetDlgItem(IDC_LIST);
		m_hListCtrlLinks = GetDlgItem(IDC_LIST1);
		InitListCtrl();
		::SetWindowText(GetDlgItem(IDC_EDIT),_T("default -p 10002"));
//		::SetWindowText(GetDlgItem(IDC_EDIT),_T("default -h 10.11.0.250 -p 10002"));
		m_ic = Ice::initialize(i, NULL); 
		return 0;
	}
	LRESULT OnRclickTree(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		// TODO : Add Code for control notification handler.
		POINT pos;
		::GetCursorPos(&pos);
		// Retreive the coordinates of the treecrtl viwndow
		RECT cRect ;
		::GetWindowRect(m_hTreeCtrl, &cRect);
		
		pos.x += -cRect.left;
		pos.y += -cRect.top;
		
		TVHITTESTINFO ht;
		ht.pt = pos;
		HTREEITEM hSelect, hParent;
		hSelect =TreeView_HitTest(m_hTreeCtrl, &ht ) ;
		if ( hSelect ) 
		{
			char strText[50];
			TreeView_SelectItem(m_hTreeCtrl, hSelect ) ;
			if(hSelect)
				hParent = TreeView_GetNextItem(m_hTreeCtrl,hSelect, TVGN_PARENT);
			
			GetTreeViewText(hSelect, strText);
			if(!strcmp(strText, "Storage Links"))
			{ 
				m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_STORAGELINK));
			}
			else if(!strcmp(strText, "Streamer Links"))
			{
			  m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_STREAMERLINK));		
			}
			if(hParent == NULL)
			{
				if(hSelect == m_hStreamerRoot )
				{
					m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_ADDSTREAMER));
				}
				else 
					if(hSelect == m_hSGRoot)
					{
						m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_ADDSG));
					}
					else
						if(hSelect == m_hStorageRoot)
							m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_ADDSTORAGE));
			}
			else
			{
				if(hParent == m_hStreamerRoot )
				{
					m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_UPSTREAMER));
				}
				else 
					if(hParent == m_hSGRoot)
					{
						m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_UPSG));
					}
					else
						if(hParent == m_hStorageRoot)
							m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_UPSTORAGE));
			}
			
			m_hMenu = ::GetSubMenu (m_hMenu, 0) ;		
			POINT pt; 
			GetCursorPos(&pt);	
			BOOL btest = ::TrackPopupMenu (m_hMenu, TPM_RIGHTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0,m_hTreeCtrl, NULL) ;
            SData sdata;
			sdata.flag = FALSE;

			int nflag = -1;
			if(btest)
			{
				switch(btest)
				{
				case ID_ADDSTORAGE:
					nflag = 0;
					break;
				case ID_ADDSTREAMER:
					nflag = 1;
					break;
				case ID_ADDSG:
					nflag = 2;
					break;
				case ID_LINKSTORAGE:
					nflag = 3;
					break;
				case ID_LINKSTREAMER:
					nflag = 4;
					break;
				case ID_UPSTORAGE:
					DisplayStorageInfo(&sdata);
					nflag = 0;
					break;
				case ID_UPSTREAMER:
					DisplayStreamerInfo(&sdata);
					nflag = 1;
					break;
				case ID_UPSG:
					DisplaySGInfo(&sdata);
					nflag = 2;
					break;
				default:
					break;
				}			
			
              CAddDataDlg dlg(m_client, nflag, sdata);
              dlg.DoModal();
			}
		}
		return 0;
	}
	LRESULT OnSelchangedTree(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		// TODO : Add Code for control notification handler.
			HTREEITEM hSelect =NULL, hParent = NULL, hRoot = NULL; 
		hSelect = TreeView_GetSelection(m_hTreeCtrl);
		if(hSelect)
			hParent = TreeView_GetNextItem(m_hTreeCtrl,hSelect, TVGN_PARENT);
		if(hParent)
			hRoot = TreeView_GetNextItem(m_hTreeCtrl,hParent, TVGN_PARENT);
		
		if(hSelect == m_hStreamerRoot && hParent == NULL)
		{
			DeleteTreeItem(m_hStreamerRoot);
			listStreamerId();		
			return 0;
		}
		else 
		{
			if(hSelect == m_hStorageRoot)				
			{
				DeleteTreeItem(m_hStorageRoot);
				ListStorageId();
				return 0;
			}
			else
			{
				if(hSelect == m_hSGRoot )			
				{
					DeleteTreeItem(m_hSGRoot);
					listSGId();
					return 0;
				}
			}
		}
		if(hParent == m_hStreamerRoot )
		{
			DisplayStreamerInfo();
			return 0;
		}
		else 
			if(hParent == m_hSGRoot)
			{
				DisplaySGInfo();
				return 0;
			}
			else
				if(hParent == m_hStorageRoot)
				{
					DisplayStorageInfo();
					return 0;
				}
				
				if(hRoot == m_hStreamerRoot )
				{
					DisplayStreamLinks();
					return 0;
				}
				else 
					if(hRoot == m_hSGRoot)
					{
						DisplayStreamLinks();
						return 0;
					}
					else
						if(hRoot == m_hStorageRoot)
						{
							DisplayStorageLinks();
							return 0;
						}
		return 0;
	}
	LRESULT OnClickTree(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		// TODO : Add Code for control notification handler.
/*		POINT pos;
		::GetCursorPos(&pos);
		// Retreive the coordinates of the treecrtl viwndow
		RECT cRect ;
		::GetWindowRect(m_hTreeCtrl, &cRect);
		
		pos.x += -cRect.left;
		pos.y += -cRect.top;
		
		TVHITTESTINFO ht;
		ht.pt = pos;
		HTREEITEM hSelect =TreeView_HitTest(m_hTreeCtrl, &ht ) ;
		if ( hSelect ) 
		{
		HTREEITEM hSelect =NULL, hParent = NULL, hRoot = NULL; 
		hSelect = TreeView_GetSelection(m_hTreeCtrl);
		if(hSelect)
			hParent = TreeView_GetNextItem(m_hTreeCtrl,hSelect, TVGN_PARENT);
		if(hParent)
			hRoot = TreeView_GetNextItem(m_hTreeCtrl,hParent, TVGN_PARENT);
		
		if(hSelect == m_hStreamerRoot && hParent == NULL)
		{
			DeleteTreeItem(m_hStreamerRoot);
			listStreamerId();		
			return 0;
		}
		else 
		{
			if(hSelect == m_hStorageRoot)				
			{
				DeleteTreeItem(m_hStorageRoot);
				ListStorageId();
				return 0;
			}
			else
			{
				if(hSelect == m_hSGRoot )			
				{
					DeleteTreeItem(m_hSGRoot);
					listSGId();
					return 0;
				}
			}
		}
		if(hParent == m_hStreamerRoot )
		{
			DisplayStreamerInfo();
			return 0;
		}
		else 
			if(hParent == m_hSGRoot)
			{
				DisplaySGInfo();
				return 0;
			}
			else
				if(hParent == m_hStorageRoot)
				{
					DisplayStorageInfo();
					return 0;
				}	
		}*/
		return 0;
	}
	LRESULT OnDblclkList1(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		// TODO : Add Code for control notification handler.
		std::string strtemp;
		char str[200];
		char strproxy[650] = "";
		LinkInfo linkinfo;
        
		if(m_blistLink)
		{
           int nCount = ListView_GetSelectedCount(m_hListCtrlLinks);

		   if(nCount != 1)
		   {
              return 0;
		   }

           int nRow = ListView_GetSelectionMark(m_hListCtrlLinks);
		   if(m_bStorageLink)
		   {
			   linkinfo.bStorage = 1;
			   ListView_GetItemText(m_hListCtrlLinks,nRow , 1, str, 200);//Type;
			   linkinfo.linktype = str;
			   memset(str,0,200);
			   ListView_GetItemText(m_hListCtrlLinks,nRow, 2, str, 200);//storageId;
			   linkinfo.storageNetId = str;
			   memset(str,0,200);
			   ListView_GetItemText(m_hListCtrlLinks,nRow, 3, str, 200);//streamerId;
			   linkinfo.streamerNetId = str;
			   memset(str,0,200);
			   ListView_GetItemText(m_hListCtrlLinks,nRow, 4, strproxy, 650);//proxystring;
			   try
			   {
				   linkinfo.storagelinkPrx  = TianShanIce::AccreditedPath::StorageLinkPrx::checkedCast(m_ic->stringToProxy(strproxy));
			   }
			   catch (const Ice::Exception& ex)
			   {
				   MessageBox(_T(ex.ice_name().c_str()),"connect error");
				   ReInitTree();
				   return 0;
			   }

			   CAddDataDlg dlg(m_client,3,linkinfo);
			   dlg.DoModal();
		   }

		   else
		   {
			   linkinfo.bStorage = 0;
			   ListView_GetItemText(m_hListCtrlLinks,nRow , 1, str, 200);//Type;
			   linkinfo.linktype = str;
			   memset(str,0,200);
			   ListView_GetItemText(m_hListCtrlLinks,nRow, 2, str, 200);//streamerId;
			   linkinfo.streamerNetId = str;
			   memset(str,0,200);
			   ListView_GetItemText(m_hListCtrlLinks,nRow, 3, str, 200);//SGId;
			   linkinfo.SGId = atoi(str);
			   memset(str,0,200);
			   ListView_GetItemText(m_hListCtrlLinks,nRow, 4, strproxy, 650);//proxystring;
			   try
			   {
				   linkinfo.streamlinkprx = TianShanIce::AccreditedPath::StreamLinkPrx::checkedCast(m_ic->stringToProxy(strproxy));
			   }
			   catch (const Ice::Exception& ex)
			   {
				   MessageBox(_T(ex.ice_name().c_str()),"connect error");
				   ReInitTree();
				   return 0;
			   }

			   CAddDataDlg dlg(m_client,4,linkinfo);
			   dlg.DoModal();
		   }
		}
		return 0;
	}
	LRESULT OnClickedConnectbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		// TODO : Add Code for control notification handler.
		char strEndpoint[100];
		::GetWindowText(GetDlgItem(IDC_EDIT), strEndpoint, 100);
	     ReInitTree();
		try
		{
			std::string endpoint = strEndpoint;
//			m_client = TianShanIce::AccreditedPath::PathAdminPrx::checkedCast(m_ic->stringToProxy("PathManager:default -p 10002"));
			m_client = TianShanIce::AccreditedPath::PathAdminPrx::checkedCast(m_ic->stringToProxy(std::string(ADAPTER_NAME_PathManager) + ":" + endpoint));
		}
		catch (const Ice::Exception& ex)
		{
			MessageBox(_T(ex.ice_name().c_str()),"connect error");
			return 0;
		}
	  InitTreeCtrl();
		return 0;
	}
	LRESULT OnBegintrackList1(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		// TODO : Add Code for control notification handler.
		return 0;
	}
};

#endif //__ADPATHUI_H_
