// ModSoapModelMap.h : Declaration of the CModSoapModelMap

#ifndef __MODSOAPMODELMAP_H_
#define __MODSOAPMODELMAP_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CModSoapModelMap
class ATL_NO_VTABLE CModSoapModelMap : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CModSoapModelMap, &CLSID_ModSoapModelMap>,
	public ISupportErrorInfo,
	public IDispatchImpl<IModSoapModelMap, &IID_IModSoapModelMap, &LIBID_OTEMODSOAPUDTMAPPERLib>,
	public IDispatchImpl<MSSOAPLib30::ISoapTypeMapper, &MSSOAPLib30::IID_ISoapTypeMapper, &MSSOAPLib30::LIBID_MSSOAPLib30>
{
public:
	CModSoapModelMap()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MODSOAPMODELMAP)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CModSoapModelMap)
	COM_INTERFACE_ENTRY(IModSoapModelMap)
	COM_INTERFACE_ENTRY2(IDispatch, IModSoapModelMap)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(MSSOAPLib30::ISoapTypeMapper)
END_COM_MAP()

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IModSoapModelMap
public:
	STDMETHOD(Init)(MSSOAPLib30::ISoapTypeMapperFactory * par_Factory, MSXML2::IXMLDOMNode * par_Schema, MSXML2::IXMLDOMNode * par_WSMLNode, MSSOAPLib30::enXSDType par_xsdType);
	STDMETHOD(Read)(MSSOAPLib30::ISoapReader * par_SoapReader, MSXML2::IXMLDOMNode * par_node, BSTR par_encoding, MSSOAPLib30::enEncodingStyle par_encodingMode, LONG par_flags, VARIANT * par_var);
	STDMETHOD(Write)(MSSOAPLib30::ISoapSerializer * par_SoapSerializer, BSTR par_encoding, MSSOAPLib30::enEncodingStyle par_encodingMode, LONG par_flags, VARIANT * par_var);
	STDMETHOD(VarType)(LONG * par_Type);
	STDMETHOD(Iid)(BSTR * par_IIDAsString);
	STDMETHOD(SchemaNode)(MSXML2::IXMLDOMNode** par_SchemaNode);
	STDMETHOD(XsdType)(MSSOAPLib30::enXSDType* par_xsdType);

private:

	MSXML2::IXMLDOMNodePtr m_pSchemaNode;
	MSSOAPLib30::ISoapTypeMapperPtr m_pProviderIDMapper;
	MSSOAPLib30::ISoapTypeMapperPtr m_pProviderAssetIDMapper;
	MSSOAPLib30::ISoapTypeMapperPtr m_pDeviceIDMapper;
	MSSOAPLib30::ISoapTypeMapperPtr m_pTicketIDMapper;
	MSSOAPLib30::ISoapTypeMapperPtr m_pStreamIDMapper;
	MSSOAPLib30::ISoapTypeMapperPtr m_pPurchaseTimeMapper;
	MSSOAPLib30::ISoapTypeMapperPtr m_pErrorCodeMapper;

	HRESULT Error(LPOLESTR pMessage, HRESULT hr = 0);

	static const CComBSTR HREF_ATTRIBUTE;

	static const CComBSTR ELEMENT_PROVIDERID;
	static const CComBSTR ELEMENT_PROVIDERASSETID;
	static const CComBSTR ELEMENT_DEVICEID;
	static const CComBSTR ELEMENT_TICKETID;
	static const CComBSTR ELEMENT_STEAMID;
	static const CComBSTR ELEMENT_PURCHASETIME;
	static const CComBSTR ELEMENT_ERRORCODE;
};

#endif //__MODSOAPMODELMAP_H_
