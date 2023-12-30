// listSample.h : Declaration of the CSpotExploer

#ifndef __LISTSAMPLE_H_
#define __LISTSAMPLE_H_
#include "resource.h"       // main symbols
#include <atlctl.h>
#include <CommCtrl.h>
#include "MyDialog.h"
#include "event.h"
// -----------------------------
// class CSpotExploer
// -----------------------------
class ATL_NO_VTABLE CSpotExploer : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IlistSample, &IID_IlistSample, &LIBID_ATL_INFOLib>,
	public CComCompositeControl<CSpotExploer>,
	public IPersistStreamInitImpl<CSpotExploer>,
	public IOleControlImpl<CSpotExploer>,
	public IOleObjectImpl<CSpotExploer>,
	public IOleInPlaceActiveObjectImpl<CSpotExploer>,
	public IViewObjectExImpl<CSpotExploer>,
	public IOleInPlaceObjectWindowlessImpl<CSpotExploer>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CSpotExploer>,
	public IPersistStorageImpl<CSpotExploer>,
	public ISpecifyPropertyPagesImpl<CSpotExploer>,
	public IQuickActivateImpl<CSpotExploer>,
	public IDataObjectImpl<CSpotExploer>,
	public IProvideClassInfo2Impl<&CLSID_listSample, &DIID__IlistSampleEvents, &LIBID_ATL_INFOLib>,
	public IPropertyNotifySinkCP<CSpotExploer>,
	public CComCoClass<CSpotExploer, &CLSID_listSample>
{
public:
	CSpotExploer()
	{
		m_bWindowOnly = TRUE;
		CalcExtent(m_sizeExtent);
		m_hRoot = NULL;
		m_hServerRoot = NULL;
		m_nType = NOTYPE;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_LISTSAMPLE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSpotExploer)
	COM_INTERFACE_ENTRY(IlistSample)
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

BEGIN_PROP_MAP(CSpotExploer)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CSpotExploer)
	CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(CSpotExploer)
	CHAIN_MSG_MAP(CComCompositeControl<CSpotExploer>)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	NOTIFY_HANDLER( IDC_TREE1,NM_CLICK, OnClickTree1)
	NOTIFY_HANDLER( IDC_TREE1,NM_DBLCLK, OnDbClickTree1)
	NOTIFY_HANDLER( IDC_TREE1,TVN_SELCHANGED, OnSelchangedTree1)
	MESSAGE_HANDLER(WM_TIMER, OnTimer)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

BEGIN_SINK_MAP(CSpotExploer)
	//Make sure the Event Handlers have __stdcall calling convention
END_SINK_MAP()

	STDMETHOD(OnAmbientPropertyChange)(DISPID dispid)
	{
		if (dispid == DISPID_AMBIENT_BACKCOLOR)
		{
			SetBackgroundColorFromAmbient();
			FireViewChange();
		}
		return IOleControlImpl<CSpotExploer>::OnAmbientPropertyChange(dispid);
	}



// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
	{
		static const IID* arr[] = 
		{
			&IID_IlistSample,
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

// IlistSample

private:
     HWND m_hListCtrl;
	 HWND m_hListCtrl2;
	 HWND m_hTreeCtrl;
	 HWND m_hStatusWindow;

     HTREEITEM m_hRoot;
	 HTREEITEM m_hServerRoot;
	 HIMAGELIST m_hImlIcons;

	 UINT	m_nIDTimer;

	 enum{SPOTIF,ALLIF, NOTYPE};
	 INT m_nType;

	 ZQ::Spot::SpotStatusQuery*  m_pSpotData;
     SpotStatusEvent*  m_pSpotStatus;
// IListCtrl
public:
    ///Init TreeCtrl, display node and interface info
	void InitTreeCtrl();
	///Init listCtrl1
	void InitListCtrl1();
	///Init listCtrl2
	void InitListCtrl2();

	///init Image list of tree view control
	void InitTreeIcon();
    LRESULT OnClickTree1(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnDbClickTree1(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnSelchangedTree1(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
public:
	///Create a statusBar in the Dialog
	void CreateStatusBar();

	///Removes all column from ListCtrl2
	void DeleteListCtrlColumn();

    ///Removes  all children item from a tree view control
    ///@param[in] hitem  handle to the item to delete
	void DeleteTreeItem(HTREEITEM hitem);

    ///get an item text from a tree view control
    ///@param[in] hitem  handle to the item to get
	///@param[out] pstr  an array that receive the item text
	void GetTreeViewText(HTREEITEM hitem, char *pstr);

	///get an interface info and display
	void DisplayIF();

	///get a spot info and display
	void displaySpotInfo();

	///get a node info and display
	void displayNodeInfo();

	///get a spot interface and display
	void DispalySpotIfInfo();

	///get all interface info of spot and add them in tree view control
	void AddSpotInterface();

	///get all spot info of node and add them in tree view control
	void AddNodeSpot();

	///get all interface info and add them in tree view control
	void AddallInterface();

	///get all node info and add them in tree view control
	void AddallNode();

	///get the status query of current spot environment
	void InitSpotStatus();

	///Create a SpotStatusEvente object
    void InitStatusEvent();
 
	enum { IDD = IDD_LISTSAMPLE };
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// TODO : Add Code for message handler. Call DefWindowProc if necessary.
		m_hListCtrl = GetDlgItem(IDC_LIST1);
		m_hListCtrl2 = GetDlgItem(IDC_LIST2);
		m_hTreeCtrl = GetDlgItem(IDC_TREE1);
		m_nIDTimer = ::SetTimer(m_hWnd,1, 1000, NULL);

        InitTreeIcon();
		InitListCtrl1();		
		InitListCtrl2();
        CreateStatusBar();
		
        CMyDialog	dlg;
		dlg.DoModal();
		
		int argc = 3;
		char *argv[3] = {"test.exe",g_Ip, g_Port};
		Init(argc, argv);

		InitSpotStatus();
		
		InitTreeCtrl();	

		InitStatusEvent();
		test();
 
		return 0;
	}
	void test();
	
		LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// TODO : Add Code for message handler. Call DefWindowProc if necessary.
		time_t time_value;
	     time(&time_value);
		::SendMessage(m_hStatusWindow,SB_SETTEXT,2,(LPARAM)TEXT(ctime(&time_value)));
		
		if(m_nType == SPOTIF)
		{
			DispalySpotIfInfo();
		}
		else
			if(m_nType == ALLIF)
			{		
				DisplayIF();	
			}
			
			return 0;
	}
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// TODO : Add Code for message handler. Call DefWindowProc if necessary.
		ImageList_Destroy(m_hImlIcons);
		if (m_nIDTimer)
		{
			KillTimer(m_nIDTimer);
			m_nIDTimer = NULL;
		}

		m_pSpotData->unsinkStatusChangeEvents(m_pSpotStatus);

//		g_ic->waitForShutdown();
		//g_pspotEnv->shutdown();
		//g_ic->destroy();

		if(g_pspotEnv)
			delete g_pspotEnv;

		if(g_Ip)
			delete[] g_Ip;

		if(g_Port)
			delete[] g_Port;

        if(m_pSpotStatus)
			delete m_pSpotStatus;
		
		return 0; 
	}
};
#endif //__LISTSAMPLE_H_
