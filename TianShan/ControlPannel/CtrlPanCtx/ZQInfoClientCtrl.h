// ZQInfoClientCtrl.h : Declaration of the CZQInfoClientCtrl

#ifndef __ZQINFOCLIENTCTRL_H_
#define __ZQINFOCLIENTCTRL_H_

#include "resource.h"       // main symbols
#include <atlctl.h>
#include "MyTabViewCtrl.h"
#include <Locks.h>
typedef CSimpleArray<string> GRIBDATAARRAY;

/////////////////////////////////////////////////////////////////////////////
// CZQInfoClientCtrl
class ATL_NO_VTABLE CZQInfoClientCtrl : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IZQInfoClientCtrl, &IID_IZQInfoClientCtrl, &LIBID_ZQINFOCLIENTCONTROLLib>,
	public CComControl<CZQInfoClientCtrl>,
	public IPersistStreamInitImpl<CZQInfoClientCtrl>,
	public IOleControlImpl<CZQInfoClientCtrl>,
	public IOleObjectImpl<CZQInfoClientCtrl>,
	public IOleInPlaceActiveObjectImpl<CZQInfoClientCtrl>,
	public IViewObjectExImpl<CZQInfoClientCtrl>,
	public IOleInPlaceObjectWindowlessImpl<CZQInfoClientCtrl>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CZQInfoClientCtrl>,
	public IPersistStorageImpl<CZQInfoClientCtrl>,
	public ISpecifyPropertyPagesImpl<CZQInfoClientCtrl>,
	public IQuickActivateImpl<CZQInfoClientCtrl>,
	public IDataObjectImpl<CZQInfoClientCtrl>,
	public IProvideClassInfo2Impl<&CLSID_ZQInfoClientCtrl, &DIID__IZQInfoClientCtrlEvents, &LIBID_ZQINFOCLIENTCONTROLLib>,
	public IPropertyNotifySinkCP<CZQInfoClientCtrl>,
	public CComCoClass<CZQInfoClientCtrl, &CLSID_ZQInfoClientCtrl>
{
public:
	CZQInfoClientCtrl();
	~CZQInfoClientCtrl();
	

DECLARE_REGISTRY_RESOURCEID(IDR_ZQINFOCLIENTCTRL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CZQInfoClientCtrl)
	COM_INTERFACE_ENTRY(IZQInfoClientCtrl)
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

BEGIN_PROP_MAP(CZQInfoClientCtrl)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CZQInfoClientCtrl)
	CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP()


//用于申明这个组件是安全的
BEGIN_CATEGORY_MAP(CZQInfoClientCtrl)
	IMPLEMENTED_CATEGORY(CATID_Insertable)
	IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
END_CATEGORY_MAP()
//用于申明这个组件是安全的

BEGIN_MSG_MAP(CZQInfoClientCtrl)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	REFLECT_NOTIFICATIONS()
	CHAIN_MSG_MAP(CComControl<CZQInfoClientCtrl>)
//	CHAIN_MSG_MAP_MEMBER(m_TabCtrl)
	REFLECT_NOTIFICATIONS()
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

// IZQInfoClientCtrl
public:
	HRESULT OnDraw(ATL_DRAWINFO& di);
// MESSAGE Fuction
private:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL& bHandled);
	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled);
// Method
private:
	void InitSNMPRegister();
	void InitTabCtrl();
	void ReadXMLFileData();
	
// Member
private:
	CMyTabViewCtrl          m_TabCtrl;
	ZQ::common::Mutex		m_Mutex;
	GRIBDATAARRAY           m_GribTabDataArray;
};


#endif //__ZQINFOCLIENTCTRL_H_
