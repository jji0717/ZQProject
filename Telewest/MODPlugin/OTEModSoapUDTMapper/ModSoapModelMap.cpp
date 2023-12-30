// ModSoapModelMap.cpp : Implementation of CModSoapModelMap
#include "stdafx.h"
#include "OTEModSoapUDTMapper.h"
#include "ModSoapModelMap.h"
#include "SaveXPathNamespaces.h"



/////////////////////////////////////////////////////////////////////////////
// CModSoapModelMap
const CComBSTR CModSoapModelMap::HREF_ATTRIBUTE = L"href";

const CComBSTR CModSoapModelMap::ELEMENT_PROVIDERID = L"providerID";
const CComBSTR CModSoapModelMap::ELEMENT_PROVIDERASSETID = L"providerAssetID";
const CComBSTR CModSoapModelMap::ELEMENT_DEVICEID = L"deviceID";
const CComBSTR CModSoapModelMap::ELEMENT_TICKETID = L"ticketID";
const CComBSTR CModSoapModelMap::ELEMENT_STEAMID = L"streamID";
const CComBSTR CModSoapModelMap::ELEMENT_PURCHASETIME = L"purchaseTime";
const CComBSTR CModSoapModelMap::ELEMENT_ERRORCODE = L"errorCode";


STDMETHODIMP CModSoapModelMap::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IModSoapModel
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		//if (InlineIsEqualGUID(*arr[i],riid))
		if (*arr[i] == riid)
			return S_OK;
	}
	return S_FALSE;
}


STDMETHODIMP CModSoapModelMap::Init(
	MSSOAPLib30::ISoapTypeMapperFactory* pFactory, 
	MSXML2::IXMLDOMNode* pSchemaNode, 
	MSXML2::IXMLDOMNode* pWSMLNode, 
	MSSOAPLib30::enXSDType xsdType)
{
	HRESULT hr;
	CComPtr<MSXML2::IXMLDOMNode> pNode;


	// Save schema node for return by SchemaNode method.
	m_pSchemaNode = pSchemaNode;

	
	// Set selection language to XPath and define xsd namespace prefix.
	CSaveXPathNamespaces SavedXPathNamespaces(pSchemaNode, _T("xmlns:xsd='http://www.w3.org/2001/XMLSchema'"));

	// Get mapper for providerID element.
    hr = pSchemaNode->selectSingleNode(CComBSTR(L"xsd:sequence/xsd:element[@name='providerID']"), &pNode);
	if( FAILED(hr) || pNode == NULL ) 
		return Error(L"Cannot find providerID element in ModSoapModel schema.", hr);

	hr = pFactory->GetElementMapper(pNode, &m_pProviderIDMapper);
	if( FAILED(hr) ) 
		return Error(L"Cannot get mapper for ModSoapModel providerID element.", hr);

	pNode = NULL;

	// Get mapper for providerAssetID element.
    hr = pSchemaNode->selectSingleNode(CComBSTR(L"xsd:sequence/xsd:element[@name='providerAssetID']"), &pNode);
	if( FAILED(hr) || pNode == NULL ) 
		return Error(L"Cannot find providerAssetID element in ModSoapModel schema.", hr);

	hr = pFactory->GetElementMapper(pNode, &m_pProviderAssetIDMapper);
	if( FAILED(hr) ) 
		return Error(L"Cannot get mapper for ModSoapModel providerAssetID element.", hr);

	pNode = NULL;
	
	// Get mapper for deviceID element.
    hr = pSchemaNode->selectSingleNode(CComBSTR(L"xsd:sequence/xsd:element[@name='deviceID']"), &pNode);
	if( FAILED(hr) || pNode == NULL ) 
		return Error(L"Cannot find deviceID element in ModSoapModel schema.", hr);

	hr = pFactory->GetElementMapper(pNode, &m_pDeviceIDMapper);
	if( FAILED(hr) ) 
		return Error(L"Cannot get mapper for ModSoapModel deviceID element.", hr);

	pNode = NULL;


	// Get mapper for ticketID element.
	hr = pSchemaNode->selectSingleNode(CComBSTR(L"xsd:sequence/xsd:element[@name='ticketID']"), &pNode);
	if( FAILED(hr) || pNode == NULL ) 
		return Error(L"Cannot find ticketID element in ModSoapModel schema.", hr);

	hr = pFactory->GetElementMapper(pNode, &m_pTicketIDMapper);
	if( FAILED(hr) ) 
		return Error(L"Cannot get mapper for ModSoapModel ticketID element.", hr);

	pNode = NULL;

	// Get mapper for streamID element.
	hr = pSchemaNode->selectSingleNode(CComBSTR(L"xsd:sequence/xsd:element[@name='streamID']"), &pNode);
	if( FAILED(hr) || pNode == NULL ) 
		return Error(L"Cannot find streamID element in ModSoapModel schema.", hr);

	hr = pFactory->GetElementMapper(pNode, &m_pStreamIDMapper);
	if( FAILED(hr) ) 
		return Error(L"Cannot get mapper for ModSoapModel streamID element.", hr);

	pNode = NULL;

	// Get mapper for purchaseTime element.
	hr = pSchemaNode->selectSingleNode(CComBSTR(L"xsd:sequence/xsd:element[@name='purchaseTime']"), &pNode);
	if( FAILED(hr) || pNode == NULL ) 
		return Error(L"Cannot find purchaseTime element in ModSoapModel schema.", hr);

	hr = pFactory->GetElementMapper(pNode, &m_pPurchaseTimeMapper);
	if( FAILED(hr) ) 
		return Error(L"Cannot get mapper for ModSoapModel purchaseTime element.", hr);

	pNode = NULL;

	// Get mapper for purchaseTime element.
	hr = pSchemaNode->selectSingleNode(CComBSTR(L"xsd:sequence/xsd:element[@name='errorCode']"), &pNode);
	if( FAILED(hr) || pNode == NULL ) 
		return Error(L"Cannot find errorCode element in ModSoapModel schema.", hr);

	hr = pFactory->GetElementMapper(pNode, &m_pErrorCodeMapper);
	if( FAILED(hr) ) 
		return Error(L"Cannot get mapper for ModSoapModel errorCode element.", hr);

	pNode = NULL;
	// All Done

	return S_OK;

}

/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CModSoapModelMap::Read(
	MSSOAPLib30::ISoapReader * pSoapReader, 
	MSXML2::IXMLDOMNode * pNode, 
	BSTR bstrEncoding, 
	MSSOAPLib30::enEncodingStyle encodingMode, 
	LONG lFlags, 
	VARIANT * pvar)
{

	HRESULT hr;
	CComPtr<MSXML2::IXMLDOMNode> pTempNode;
	CComPtr<MSXML2::IXMLDOMNamedNodeMap> pAttributes;
	CComPtr<IModSoapModel> pModSoapModel;
	CComVariant varTemp;

	_ASSERT(pNode != NULL);


	// Create Addr That Will Be Read

	hr = pModSoapModel.CoCreateInstance(__uuidof(ModSoapModel));
	if( FAILED(hr) ) 
		return Error(L"Cannot create ModSoapModel object.", hr);


	// SOAP Encoding rules allow a node to have an href attribute that
    // identifies the node that actually contains the value. Each type
    // mapper must check for this attribute and locate the referenced
    // node.

	hr = pNode->get_attributes(&pAttributes);
	if( FAILED(hr) ) 
		return hr;

	hr = pAttributes->getNamedItem(HREF_ATTRIBUTE, &pTempNode);
	if( FAILED(hr) ) 
		return hr;
	if( pTempNode != NULL )
	{
		
		CComBSTR queryString;

		hr = pTempNode->get_nodeValue(&varTemp);
		if( FAILED(hr) ) return hr;

		if( varTemp.bstrVal[0] != L'#' ) return Error(L"href attribute value does not start with '#'.");

		queryString += L"//*[@id='";
		queryString += varTemp.bstrVal + 1; // skip leading '#'
		queryString += L"']";

		CComPtr<MSXML2::IXMLDOMDocument> pDoc;
		hr = pNode->get_ownerDocument(&pDoc);
		if( FAILED(hr) ) 
			return hr;

		hr = pDoc->selectSingleNode(queryString, &pNode);
		if( FAILED(hr) || pNode == NULL ) 
			return Error(L"Cannot find node specified by href attribute.", hr);

	}
    // Read providerID Property
	hr = pNode->selectSingleNode(ELEMENT_PROVIDERID, &pTempNode);
	if( FAILED(hr) || pTempNode == NULL ) 
		return Error(L"Cannot find ModSoapModel providerID element.", hr);
		
	hr = m_pProviderIDMapper->Read(pSoapReader, pTempNode, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return Error(L"Cannot map ModSoapModel providerID element value.", hr);

	hr = pModSoapModel->put_ProviderID(varTemp.bstrVal);
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();
	pTempNode = NULL;

    // Read providerAssetID Property
	hr = pNode->selectSingleNode(ELEMENT_PROVIDERASSETID, &pTempNode);
	if( FAILED(hr) || pTempNode == NULL ) 
		return Error(L"Cannot find ModSoapModel providerAssetID element.", hr);
		
	hr = m_pProviderAssetIDMapper->Read(pSoapReader, pTempNode, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return Error(L"Cannot map ModSoapModel providerAssetID element value.", hr);

	hr = pModSoapModel->put_ProviderAssetID(varTemp.bstrVal);
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();
	pTempNode = NULL;

    // Read errorCode Property
	hr = pNode->selectSingleNode(ELEMENT_ERRORCODE, &pTempNode);
	if( FAILED(hr) || pTempNode == NULL ) 
		return Error(L"Cannot find ModSoapModel errorCode element.", hr);
		
	hr = m_pErrorCodeMapper->Read(pSoapReader, pTempNode, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return Error(L"Cannot map ModSoapModel errorCode element value.", hr);

	hr = pModSoapModel->put_PurchaseTime(varTemp.bstrVal);
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();
	pTempNode = NULL;

    // Read deviceID Property
	hr = pNode->selectSingleNode(ELEMENT_DEVICEID, &pTempNode);
	if( FAILED(hr) || pTempNode == NULL ) 
		return Error(L"Cannot find ModSoapModel deviceID element.", hr);
		
	hr = m_pDeviceIDMapper->Read(pSoapReader, pTempNode, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return Error(L"Cannot map ModSoapModel deviceID element value.", hr);

	hr = pModSoapModel->put_DeviceID(varTemp.bstrVal);
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();
	pTempNode = NULL;

    // Read purchaseTime Property
	hr = pNode->selectSingleNode(ELEMENT_PURCHASETIME, &pTempNode);
	if( FAILED(hr) || pTempNode == NULL ) 
		return Error(L"Cannot find ModSoapModel pruchaseTime element.", hr);
		
	hr = m_pPurchaseTimeMapper->Read(pSoapReader, pTempNode, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return Error(L"Cannot map ModSoapModel pruchaseTime element value.", hr);

	hr = pModSoapModel->put_PurchaseTime(varTemp.bstrVal);
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();
	pTempNode = NULL;

    // Read streamID Property
	hr = pNode->selectSingleNode(ELEMENT_STEAMID, &pTempNode);
	if( FAILED(hr) || pTempNode == NULL ) 
		return Error(L"Cannot find ModSoapModel streamID element.", hr);
		
	hr = m_pStreamIDMapper->Read(pSoapReader, pTempNode, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return Error(L"Cannot map ModSoapModel streamID element value.", hr);

	hr = pModSoapModel->put_StreamID(varTemp.bstrVal);
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();
	pTempNode = NULL;

    // Read ticketID Property
	hr = pNode->selectSingleNode(ELEMENT_TICKETID, &pTempNode);
	if( FAILED(hr) || pTempNode == NULL ) 
		return Error(L"Cannot find ModSoapModel ticketID element.", hr);
		
	hr = m_pTicketIDMapper->Read(pSoapReader, pTempNode, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return Error(L"Cannot map ModSoapModel ticketID element value.", hr);

	hr = pModSoapModel->put_TicketID(varTemp.bstrVal);
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();
	pTempNode = NULL;

	// Set Return Value
	pvar->vt = VT_DISPATCH;
	pvar->pdispVal = pModSoapModel.Detach();


	// All Done
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CModSoapModelMap::Write(
	MSSOAPLib30::ISoapSerializer* pSoapSerializer, 
	BSTR bstrEncoding, 
	MSSOAPLib30::enEncodingStyle encodingMode, 
	LONG lFlags, 
	VARIANT * pvar)
{
	HRESULT hr;
	CComPtr<IModSoapModel> pModSoapModel;
	CComVariant varTemp;


	// Get IAddr interface of object to write.

	_ASSERTE(pvar != NULL);
	_ASSERTE(pvar->vt == VT_DISPATCH);
	_ASSERTE(pvar->pdispVal != NULL);

	hr = pvar->pdispVal->QueryInterface(&pModSoapModel); 
	if(FAILED(hr)) return hr;

	// Write providerID Property
	varTemp.vt = VT_BSTR;
	hr = pModSoapModel->get_ProviderID(&varTemp.bstrVal);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->StartElement(ELEMENT_PROVIDERID, NULL, NULL , NULL);
	if( FAILED(hr) ) 
		return hr;

	hr = m_pProviderIDMapper->Write(pSoapSerializer, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->EndElement();
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();

	// Write providerAssetID Property
	varTemp.vt = VT_BSTR;
	hr = pModSoapModel->get_ProviderAssetID(&varTemp.bstrVal);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->StartElement(ELEMENT_PROVIDERASSETID, NULL, NULL , NULL);
	if( FAILED(hr) ) 
		return hr;

	hr = m_pProviderAssetIDMapper->Write(pSoapSerializer, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->EndElement();
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();

	// Write errorCode Property
	varTemp.vt = VT_BSTR;
	hr = pModSoapModel->get_ErrorCode(&varTemp.bstrVal);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->StartElement(ELEMENT_ERRORCODE, NULL, NULL , NULL);
	if( FAILED(hr) ) 
		return hr;

	hr = m_pErrorCodeMapper->Write(pSoapSerializer, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->EndElement();
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();
	
	// Write deviceID Property
	varTemp.vt = VT_BSTR;
	hr = pModSoapModel->get_DeviceID(&varTemp.bstrVal);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->StartElement(ELEMENT_DEVICEID, NULL, NULL , NULL);
	if( FAILED(hr) ) 
		return hr;

	hr = m_pDeviceIDMapper->Write(pSoapSerializer, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->EndElement();
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();

	// Write pruchaseTime Property
	varTemp.vt = VT_BSTR;
	hr = pModSoapModel->get_PurchaseTime(&varTemp.bstrVal);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->StartElement(ELEMENT_PURCHASETIME, NULL, NULL , NULL);
	if( FAILED(hr) ) 
		return hr;

	hr = m_pPurchaseTimeMapper->Write(pSoapSerializer, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->EndElement();
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();

	// Write streamID Property
	varTemp.vt = VT_BSTR;
	hr = pModSoapModel->get_StreamID(&varTemp.bstrVal);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->StartElement(ELEMENT_STEAMID, NULL, NULL , NULL);
	if( FAILED(hr) ) 
		return hr;

	hr = m_pStreamIDMapper->Write(pSoapSerializer, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->EndElement();
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();

	// Write ticketID Property
	varTemp.vt = VT_BSTR;
	hr = pModSoapModel->get_TicketID(&varTemp.bstrVal);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->StartElement(ELEMENT_TICKETID, NULL, NULL , NULL);
	if( FAILED(hr) ) 
		return hr;

	hr = m_pTicketIDMapper->Write(pSoapSerializer, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->EndElement();
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();
	

	// All Done
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CModSoapModelMap::VarType(LONG * pvtType)
{
	if (!pvtType)
        return E_INVALIDARG;
	
	*pvtType = VT_DISPATCH;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CModSoapModelMap::Iid(BSTR * pIID)
{
	
	const GUID_STRING_SIZE = 39; // with terminating null
	OLECHAR GUIDString[GUID_STRING_SIZE];

	if (!pIID)
		return E_INVALIDARG;

	StringFromGUID2(IID_IModSoapModel, GUIDString, GUID_STRING_SIZE);

	*pIID = CComBSTR(GUIDString).Detach();

	return S_OK;

}


/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CModSoapModelMap::SchemaNode(MSXML2::IXMLDOMNode** par_SchemaNode)
{

	if (!par_SchemaNode)
		return E_INVALIDARG;

	*par_SchemaNode = m_pSchemaNode;
	(*par_SchemaNode)->AddRef();

	return S_OK;

}


/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CModSoapModelMap::XsdType(MSSOAPLib30::enXSDType* par_xsdType)
{

	if (!par_xsdType)
		return E_INVALIDARG;

	*par_xsdType = MSSOAPLib30::enXSDUndefined;

	return S_OK;

}


/////////////////////////////////////////////////////////////////////////////
HRESULT CModSoapModelMap::Error(LPOLESTR pMessage, HRESULT hr)
{
	
	HRESULT _hr;
	CComPtr<IErrorInfo> pErrorInfo;
	CComBSTR PreviousDescription;
	CComBSTR Description;

	Description = pMessage;

	_hr = GetErrorInfo(0, &pErrorInfo);
	if( SUCCEEDED(_hr) && pErrorInfo != NULL )
	{
		_hr = pErrorInfo->GetDescription(&PreviousDescription);
		if( SUCCEEDED(_hr) )
		{
			Description += L" ";
			Description += PreviousDescription;
		}
	}

	if( SUCCEEDED(hr) ) hr = E_FAIL;

	return AtlReportError(GetObjectCLSID(), Description, GUID_NULL, hr);

}
