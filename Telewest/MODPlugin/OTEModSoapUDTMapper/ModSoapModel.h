// ModSoapModel.h : Declaration of the CModSoapModel

#ifndef __MODSOAPMODEL_H_
#define __MODSOAPMODEL_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CModSoapModel
class ATL_NO_VTABLE CModSoapModel : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CModSoapModel, &CLSID_ModSoapModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IModSoapModel, &IID_IModSoapModel, &LIBID_OTEMODSOAPUDTMAPPERLib>
{
public:
	CModSoapModel()
	{
		m_strDeviceID = L"";
		m_strTicketID = L"";
		m_strStreamID = L"";
		m_strPurchaseTime = L"";
		m_strErrorCode = L"";
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MODSOAPMODEL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CModSoapModel)
	COM_INTERFACE_ENTRY(IModSoapModel)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IModSoapModel
public:
	STDMETHOD(get_ProviderAssetID)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_ProviderAssetID)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_ProviderID)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_ProviderID)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_ErrorCode)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_ErrorCode)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_PurchaseTime)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_PurchaseTime)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_StreamID)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_StreamID)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_TicketID)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_TicketID)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_DeviceID)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_DeviceID)(/*[in]*/ BSTR newVal);

protected:
	CComBSTR m_strProviderID;
	CComBSTR m_strProviderAssetID;
	CComBSTR m_strDeviceID;
	CComBSTR m_strTicketID;
	CComBSTR m_strStreamID;
	CComBSTR m_strPurchaseTime;
	CComBSTR m_strErrorCode;
};

#endif //__MODSOAPMODEL_H_
