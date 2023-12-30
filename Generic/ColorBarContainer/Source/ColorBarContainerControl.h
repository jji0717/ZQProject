// ColorBarContainerControl.h : Declaration of the CColorBarContainerControl

#ifndef __COLORBARCONTAINERCONTROL_H_
#define __COLORBARCONTAINERCONTROL_H_

#include "resource.h"       // main symbols
#include "IBMSBaseWnd.h"
#include "DataStruct.h"

#include "StreamEventSinkImpl.h"
#include "StreamProgressSinkImpl.h"
#include "PlaylistEventSinkImpl.h"
#include "EventChannel.h"
#include "ChannelOnDemand.h"
#include "ChannelOnDemandEx.h"
#include "CBarContainer.h"

#ifdef _LOG
// for the log it may to be deleted
enum OperationMode
{
    Normal,
    Nonmutating,
    Idempotent
};
enum State
{
    stNotProvisioned,
    stProvisioned,
    stInService,
    stOutOfService
};
#endif

// for the log 
/////////////////////////////////////////////////////////////////////////////
// CColorBarContainerControl
class ATL_NO_VTABLE CColorBarContainerControl : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IColorBarContainerControl, &IID_IColorBarContainerControl, &LIBID_CBARCONTAINERLib>,
	public CComControl<CColorBarContainerControl>,
	public IPersistStreamInitImpl<CColorBarContainerControl>,
	public IOleControlImpl<CColorBarContainerControl>,
	public IOleObjectImpl<CColorBarContainerControl>,
	public IOleInPlaceActiveObjectImpl<CColorBarContainerControl>,
	public IViewObjectExImpl<CColorBarContainerControl>,
	public IOleInPlaceObjectWindowlessImpl<CColorBarContainerControl>,
	public IConnectionPointContainerImpl<CColorBarContainerControl>,
	public IPersistStorageImpl<CColorBarContainerControl>,
	public ISpecifyPropertyPagesImpl<CColorBarContainerControl>,
	public IQuickActivateImpl<CColorBarContainerControl>,
	public IDataObjectImpl<CColorBarContainerControl>,
	public IProvideClassInfo2Impl<&CLSID_ColorBarContainerControl, &DIID__IColorBarContainerControlEvents, &LIBID_CBARCONTAINERLib>,
	public IPropertyNotifySinkCP<CColorBarContainerControl>,
	public CComCoClass<CColorBarContainerControl, &CLSID_ColorBarContainerControl>
{
public:
	CColorBarContainerControl();
	~CColorBarContainerControl();

DECLARE_REGISTRY_RESOURCEID(IDR_COLORBARCONTAINERCONTROL)
DECLARE_PROTECT_FINAL_CONSTRUCT()
BEGIN_COM_MAP(CColorBarContainerControl)
	COM_INTERFACE_ENTRY(IColorBarContainerControl)
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
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY(IQuickActivate)
	COM_INTERFACE_ENTRY(IPersistStorage)
	COM_INTERFACE_ENTRY(IDataObject)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()


//用于申明这个组件是安全的
BEGIN_CATEGORY_MAP(CColorBarContainerControl)
	IMPLEMENTED_CATEGORY(CATID_Insertable)
	IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
END_CATEGORY_MAP()
//用于申明这个组件是安全的


BEGIN_PROP_MAP(CColorBarContainerControl)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CColorBarContainerControl)
	CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(CColorBarContainerControl)
	MESSAGE_HANDLER(WM_CREATE,OnCreate)
	MESSAGE_HANDLER(WM_PAINT,OnPaint)
	MESSAGE_HANDLER(WM_SIZE,OnSize)
	CHAIN_MSG_MAP(CComControl<CColorBarContainerControl>)
	DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

// IColorBarContainerControl
public:
	STDMETHOD(SetChanelIpAddress)(/*[in]*/ BSTR  bstrIpAddress,BSTR bstrPort);
	STDMETHOD(SetStreamIPAddress)(/*[in]*/ BSTR bstrIpAddress,BSTR bstrPort);

public:
	 HRESULT  CreateContainMap(string  strStreamId); // create the contain map by the StreamId
	 HRESULT  CreateUserCursorMap(string strStreamId);// create the user cursor map by the StreamId,被CreateCursorAndDrawLine取代
	 HRESULT  UserDrawLine(string strStreamId,double dRatio,string strComment,int iCursorId);//draw the line by the params，被CreateCursorAndDrawLine取代
	 HRESULT  CreateCursorAndDrawLine(string strStreamId,string strComment,double dRatio);//该函数综合和CreateUserCursorMap与UserDrawLine的函数，因为原来的那两个函数有缺陷，为保留代码的完整性，不删除他们
	 bool     IsExistByStreamId(string strStreamId);
private:
	
	HRESULT OnStreamEventInit(void);
	HRESULT OnStreamEventStart(void);
	HRESULT OnStreamEventUnInit(void);

	// Chanel Publish
	HRESULT OnChanelStart(void);
	HRESULT OnChanelUnInit(void);
	HRESULT GetChanelName(void);

	// CreateWindow Proce
	HRESULT CreateContainWinow(void);
	
	HRESULT GetRegionTimeData(string strStreamId,string strComment);

	double  CalcTimeDiff(string strStartTime,string strEndTime);
	double  ConvertToHour(string strTime);
	// Stream Event Sink
	bool SubscribeStreamEventSink(); // Subscribe Stream Event Sink
	bool SubscribeProgressSink();    // Subscribe Progress Event Sink
	bool SubscribePlaylistEventSink();//Subscribe Playlist Event Sink
	
	string  MidString(string strSource,int iStartPos, int iEndPos);
	
private:
	CIBMSBaseWnd * m_pContainWin; // 控件的容器
	CComPtr<IColorBarControl>  *m_pColorBarInterface; //控件的接口

	// Stream Event
	Ice::CommunicatorPtr	m_Communicator;
	Ice::ObjectAdapterPtr   m_StreamAdapter;
	TianShanIce::Events::EventChannelImpl::Ptr m_eventChannel;

	

	// Channel data
//	Ice::ObjectAdapterPtr   m_ChanelAdapter;
	
	::ChannelOnDemand::ChannelPublisherExPrx   m_publisherExPrx;
	::ChannelOnDemand::ChannelPublishPointPrx  m_channelPrx;

	/// IceStorm configuration for Stream Event
	string       m_tsTopicManagerEndpoint_Stream;
	string       m_tsSubscribeEndpoint_Stream;

	// IceStorm configuration for Chanel
	string       m_tsTopicManagerEndpoint_Chanel;
	string       m_tsSubscribeEndpoint_Chanel;

	size_t      m_WindowNum;
	double      *m_pContainControlWidth;
	double      m_dStartPos;
	double      m_dEndPos;
	COLORREF    m_CursorColor;

	// the Chanel Data map
	CHANELDATAMAP   m_ChanelMap; //节目的Map列表
	CONTAINERMAP    m_ContainerMap;   //组件的Map列表
	USERDRAWLINEMAP m_UserCursorDataMap; //用户数据的Map列表

};

#endif //__COLORBARCONTAINERCONTROL_H_
