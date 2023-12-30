// AdminCtrl.h : Declaration of the CAdminCtrl

#ifndef __ADMINCTRL_H_
#define __ADMINCTRL_H_
#include "resource.h"       // main symbols
#include <atlctl.h>
#include "mydialog.h"
#include "Draw.h"

#define  SPLIT_MINWIDTH       2
#define  SPLIT_BARWIDTH       2
#define  LISTCOLUMN_NUM       4
#define  LISTLINKCOLUMN_NUM   5

#define  DRAWMENU    1
#define  TEXT_LEN    200

/////////////////////////////////////////////////////////////////////////////
// CAdminCtrlc
class ATL_NO_VTABLE CAdminCtrl : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CStockPropImpl<CAdminCtrl, IAdminCtrl, &IID_IAdminCtrl, &LIBID_ADMINCONTROLLib>,
	public CComControl<CAdminCtrl>,
	public IPersistStreamInitImpl<CAdminCtrl>,
	public IOleControlImpl<CAdminCtrl>,
	public IOleObjectImpl<CAdminCtrl>,
	public IOleInPlaceActiveObjectImpl<CAdminCtrl>,
	public IViewObjectExImpl<CAdminCtrl>,
	public IOleInPlaceObjectWindowlessImpl<CAdminCtrl>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CAdminCtrl>,
	public IPersistStorageImpl<CAdminCtrl>,
	public ISpecifyPropertyPagesImpl<CAdminCtrl>,
	public IQuickActivateImpl<CAdminCtrl>,
	public IDataObjectImpl<CAdminCtrl>,
	public IProvideClassInfo2Impl<&CLSID_AdminCtrl, &DIID__IAdminCtrlEvents, &LIBID_ADMINCONTROLLib>,
	public IPropertyNotifySinkCP<CAdminCtrl>,
	public CComCoClass<CAdminCtrl, &CLSID_AdminCtrl>
{
public:
	CAdminCtrl();
	~CAdminCtrl();
	
DECLARE_REGISTRY_RESOURCEID(IDR_ADMINCTRL)
DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, COLOR_BTNFACE)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAdminCtrl)
	COM_INTERFACE_ENTRY(IAdminCtrl)
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

BEGIN_PROP_MAP(CAdminCtrl)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
//	PROP_ENTRY("Caption", DISPID_CAPTION, CLSID_NULL)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CAdminCtrl)
	CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP()

//用于申明这个组件是安全的
BEGIN_CATEGORY_MAP(CAdminCtrl)
	IMPLEMENTED_CATEGORY(CATID_Insertable)
	IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
END_CATEGORY_MAP()
//用于申明这个组件是安全的


BEGIN_MSG_MAP(CAdminCtrl)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnSelchangedTree)
	NOTIFY_CODE_HANDLER(NM_RCLICK, OnRclickTree)
	NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDblclkList)
	NOTIFY_CODE_HANDLER(NM_CLICK,  OnLclkList)
	MESSAGE_HANDLER(WM_MEASUREITEM,OnMeasureItem)
	MESSAGE_HANDLER(WM_DRAWITEM,OnDrawItem)
	COMMAND_RANGE_HANDLER(ID_FIRST,ID_LAST,OnVerb)
	CHAIN_MSG_MAP(CComControl<CAdminCtrl>)
	DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
	
// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

// IAdminCtrl
public:
	STDMETHOD(SetPathEndPoint)(BSTR bstrIpAddress,  BSTR bstrPort);
	CComBSTR m_bstrCaption;

// MSG FUNCITON
private:
	HRESULT OnDraw(ATL_DRAWINFO& di);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL& bHandled);
	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled);
	LRESULT OnSelchangedTree(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnRclickTree(int idCtrl,LPNMHDR pnmh, BOOL & bHandled);
	LRESULT OnDblclkList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnLclkList(int idCtrl, LPNMHDR pnmh, BOOL & bHandled);
	
	LRESULT OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnDrawItem(UINT , WPARAM , LPARAM , BOOL& bHandled);
	LRESULT OnVerb(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
// PRIVATE FUNCTION
private:
	BOOL  ConnectServe( std::string strEndPoint ="tcp -h 192.168.80.49  -p 10001", std::string strPathEndPoint="tcp -h 192.168.81.107 -p 10001");
	BOOL  ResetCtrl();
	BOOL  InitTreeCtrl();
	BOOL  DeleteListCtrlColumn();
	BOOL  DeleteListCtrlLinkColumn();
	BOOL  DeleteTreeItem(HTREEITEM hitem);
	BOOL  InsertListCtrlColumn(std::string strName);
	BOOL  InsertListCtrlLinkColumn(std::string strName);
	BOOL  ListStorageId(BOOL bUpdate = TRUE);
	BOOL  ListStreamerId(BOOL bUpdata = FALSE);
	BOOL  ListSGId(BOOL bUpdate = FALSE);
	BOOL  ListSite(BOOL bUpdate = FALSE);
	BOOL  ListApp(BOOL bUpdate  = FALSE);
	BOOL  DisplayStreamerInfo(SData *data = NULL);
	BOOL  DisplayStorageInfo(SData *data = NULL);
	BOOL  DisplaySGInfo(SData *data = NULL);
	BOOL  DisplaySiteInfo(SData *data = NULL);
	BOOL  DisplayAppInfo(SData *data = NULL);
	BOOL  DisplayStreamLinks();
	BOOL  DisplayStorageLinks();
	BOOL  RemoveOper(std::string strName,std::string id);
	BOOL  ShowSite(std::string  sitename);
	BOOL  GetProductName(const char* lpszServiceName, char lpszProductName[MAX_PATH+1]);
	BOOL  GetWeiwooEndPoint(char lpszEndPoint[MAX_PATH+1], char lpszPathEndPoint[MAX_PATH+1]);

private:
	CTreeViewCtrl	            m_hTreeCtrl;
	CListViewCtrl               m_hListCtrl;
    CListViewCtrl               m_hListCtrlLinks;
	CSplitterWindow             m_VerSplit;
	CHorSplitterWindow          m_HorSplit;
	CImageList                  m_imagelist;
    HTREEITEM					m_hStorageRoot;
	HTREEITEM					m_hStreamerRoot;
	HTREEITEM					m_hSGRoot;
	HTREEITEM					m_hSiteRoot;
	HTREEITEM					m_hAppRoot;
	HMENU                       m_hMenu;
	BOOL                        m_blistLink;
	BOOL						m_bStorageLink;
	BOOL                        m_bMountApp;
	INT                         m_iListColNum;
	INT                         m_iListLinkNum;
	INT                         m_FirstLayerType;
	CSimpleArray <CString>      m_strMenuString;
	std::string                 m_CurSelText;
	
		
	Ice::CommunicatorPtr		m_Communicator;
	Ice::ObjectAdapterPtr		m_adapter;

	TianShanIce::Site::SiteAdminPrx  m_bizPrx;
	
//	TianShanIce::SRM::SessionAdminPrx  m_sessPrx;
	TianShanIce::Events::EventChannelImpl::Ptr m_sub;
	TianShanIce::Transport::PathAdminPrx m_adminPrx;
		
};

#endif //__ADMINCTRL_H_

