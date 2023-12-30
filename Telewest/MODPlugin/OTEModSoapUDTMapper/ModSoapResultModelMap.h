// ModSoapResultModelMap.h : Declaration of the CModSoapResultModelMap

#ifndef __MODSOAPRESULTMODELMAP_H_
#define __MODSOAPRESULTMODELMAP_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CModSoapResultModelMap
class ATL_NO_VTABLE CModSoapResultModelMap : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CModSoapResultModelMap, &CLSID_ModSoapResultModelMap>,
	public ISupportErrorInfo,
	public IDispatchImpl<IModSoapResultModelMap, &IID_IModSoapResultModelMap, &LIBID_OTEMODSOAPUDTMAPPERLib>,
	public IDispatchImpl<MSSOAPLib30::ISoapTypeMapper, &MSSOAPLib30::IID_ISoapTypeMapper, &MSSOAPLib30::LIBID_MSSOAPLib30>
{
public:
	CModSoapResultModelMap()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MODSOAPRESULTMODELMAP)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CModSoapResultModelMap)
	COM_INTERFACE_ENTRY(IModSoapResultModelMap)
	COM_INTERFACE_ENTRY2(IDispatch, IModSoapResultModelMap)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(MSSOAPLib30::ISoapTypeMapper)
END_COM_MAP()

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IModSoapResultModelMap
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
	MSSOAPLib30::ISoapTypeMapperPtr m_pResultMapper;
	MSSOAPLib30::ISoapTypeMapperPtr m_pPriceMapper;
	MSSOAPLib30::ISoapTypeMapperPtr m_pRentalDurationMapper;

	HRESULT Error(LPOLESTR pMessage, HRESULT hr = 0);

	static const CComBSTR HREF_ATTRIBUTE;
	static const CComBSTR ELEMENT_RESULT;
	static const CComBSTR ELEMENT_PRICE;
	static const CComBSTR ELEMENT_RENTALDURATION;
};

#endif //__MODSOAPRESULTMODELMAP_H_
