// ColorBarControl.h : Declaration of the CColorBarControl

#ifndef __COLORBARCONTROL_H_
#define __COLORBARCONTROL_H_

#include "resource.h"    // main symbols
#include <atlctl.h>
#include <comdef.h>      // 是_bstr_t的定义文件
#include "ColorBarDoc.h"
#include "ColorBarView.h"
#include "ColorBar.h"

/////////////////////////////////////////////////////////////////////////////
// CColorBarControl
class ATL_NO_VTABLE CColorBarControl : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IColorBarControl, &IID_IColorBarControl, &LIBID_COLORBARLib>,
	public CComControl<CColorBarControl>,
	public IPersistStreamInitImpl<CColorBarControl>,
	public IOleControlImpl<CColorBarControl>,
	public IOleObjectImpl<CColorBarControl>,
	public IOleInPlaceActiveObjectImpl<CColorBarControl>,
	public IViewObjectExImpl<CColorBarControl>,
	public IOleInPlaceObjectWindowlessImpl<CColorBarControl>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CColorBarControl>,
	public IPersistStorageImpl<CColorBarControl>,
	public ISpecifyPropertyPagesImpl<CColorBarControl>,
	public IQuickActivateImpl<CColorBarControl>,
	public IDataObjectImpl<CColorBarControl>,
	public IProvideClassInfo2Impl<&CLSID_ColorBarControl, &DIID__IColorBarControlEvents, &LIBID_COLORBARLib>,
	public IPropertyNotifySinkCP<CColorBarControl>,
	public CComCoClass<CColorBarControl, &CLSID_ColorBarControl>
{
public:
	CColorBarControl();
	~CColorBarControl();
	
DECLARE_REGISTRY_RESOURCEID(IDR_COLORBARCONTROL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CColorBarControl)
	COM_INTERFACE_ENTRY(IColorBarControl)
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

BEGIN_PROP_MAP(CColorBarControl)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CColorBarControl)
	CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP()

//用于申明这个组件是安全的
BEGIN_CATEGORY_MAP(CColorBarControl)
	IMPLEMENTED_CATEGORY(CATID_Insertable)
	IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
END_CATEGORY_MAP()
//用于申明这个组件是安全的

BEGIN_MSG_MAP(CColorBarControl)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_PAINT,OnPaint)
	MESSAGE_HANDLER(WM_CREATE,OnCreate)
	MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBkgnd)
	
	CHAIN_MSG_MAP(CComControl<CColorBarControl>)
//	DEFAULT_REFLECTION_HANDLER()
	ALT_MSG_MAP(1)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

	// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
	{
		static const IID* arr[] = 
		{
			&IID_IColorBarControl,
		};
		for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++)
		{
			if (::InlineIsEqualGUID(*arr[i], riid))
				return S_OK;
		}
		return S_FALSE;
	}

// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

// IColorBarControl
	STDMETHOD(FillColor)( COLORREF crColor, double dStartPos,double dEndPos, BSTR bstrName );
	STDMETHOD(ShowRange)( double dStartPos, double dEndPos);
	STDMETHOD(CreateCursor)(COLORREF crColor,int *pID);
	STDMETHOD(DrawLine)(int iID,double dCurPos);
	STDMETHOD(DeletePaint)();
	STDMETHOD(DeleteCursor)(int  pID);
public:
	CColorBarView *m_pView; //视图指针
	CColorBarDoc  *m_pDoc; // 文档指针
};
#endif //__COLORBARCONTROL_H_
