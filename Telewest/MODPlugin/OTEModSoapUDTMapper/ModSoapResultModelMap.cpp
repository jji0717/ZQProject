// ModSoapResultModelMap.cpp : Implementation of CModSoapResultModelMap
#include "stdafx.h"
#include "OTEModSoapUDTMapper.h"
#include "ModSoapResultModelMap.h"
#include "SaveXPathNamespaces.h"

/////////////////////////////////////////////////////////////////////////////
// CModSoapResultModelMap
const CComBSTR CModSoapResultModelMap::HREF_ATTRIBUTE = L"href";
const CComBSTR CModSoapResultModelMap::ELEMENT_RESULT = L"result";
const CComBSTR CModSoapResultModelMap::ELEMENT_PRICE = L"price";
const CComBSTR CModSoapResultModelMap::ELEMENT_RENTALDURATION = L"rentalDuration";


STDMETHODIMP CModSoapResultModelMap::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IModSoapResultModelMap
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (::ATL::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


STDMETHODIMP CModSoapResultModelMap::Init(
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
	
	// Get mapper for result element.
    hr = pSchemaNode->selectSingleNode(CComBSTR(L"xsd:sequence/xsd:element[@name='result']"), &pNode);
	if( FAILED(hr) || pNode == NULL ) 
		return Error(L"Cannot find result element in ModSoapResultModel schema.", hr);

	hr = pFactory->GetElementMapper(pNode, &m_pResultMapper);
	if( FAILED(hr) ) 
		return Error(L"Cannot get mapper for ModSoapResultModel result element.", hr);

	pNode = NULL;


	// Get mapper for price element.
	hr = pSchemaNode->selectSingleNode(CComBSTR(L"xsd:sequence/xsd:element[@name='price']"), &pNode);
	if( FAILED(hr) || pNode == NULL ) 
		return Error(L"Cannot find price element in ModSoapResultModel schema.", hr);

	hr = pFactory->GetElementMapper(pNode, &m_pPriceMapper);
	if( FAILED(hr) ) 
		return Error(L"Cannot get mapper for ModSoapResultModel price element.", hr);

	pNode = NULL;

	// Get mapper for rentalDuration element.
	hr = pSchemaNode->selectSingleNode(CComBSTR(L"xsd:sequence/xsd:element[@name='rentalDuration']"), &pNode);
	if( FAILED(hr) || pNode == NULL ) 
		return Error(L"Cannot find rentalDuration element in ModSoapResultModel schema.", hr);

	hr = pFactory->GetElementMapper(pNode, &m_pRentalDurationMapper);
	if( FAILED(hr) ) 
		return Error(L"Cannot get mapper for ModSoapResultModel rentalDuration element.", hr);

	pNode = NULL;

	// All Done

	return S_OK;

}

/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CModSoapResultModelMap::Read(
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
	CComPtr<IModSoapResultModel> pModSoapResultModel;
	CComVariant varTemp;

	_ASSERT(pNode != NULL);

	// Create Addr That Will Be Read

	hr = pModSoapResultModel.CoCreateInstance(__uuidof(ModSoapResultModel));
	if( FAILED(hr) ) 
		return Error(L"Cannot create ModSoapResultModel object.", hr);


	// SOAP Encoding rules allow a node to have an href attribute that
    // identifies the node that actually contains the value. Each type
    // mapper must check for this attribute and locate the referenced
    // node.
/*	
	CComPtr<MSXML2::IXMLDOMDocument> pSoapDoc;
	hr = pNode->get_ownerDocument(&pSoapDoc);
	if( FAILED(hr) ) 
		return hr;
	_variant_t varString = _T("C:\\ITV\\Log\\ModSoapResultModel.xml");
	hr = pSoapDoc->save(varString);
	if( FAILED(hr) ) 
		return hr;
*/
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
		CComBSTR txt;
		pNode->get_text(&txt);
	}

    // Read price Property
	pTempNode = NULL;
	hr = pNode->selectSingleNode(ELEMENT_PRICE, &pTempNode);
	if( FAILED(hr) || pTempNode == NULL ) 
		return Error(L"Cannot find ModSoapResultModel price element.", hr);
		
	hr = m_pPriceMapper->Read(pSoapReader, pTempNode, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return Error(L"Cannot map ModSoapResultModel price element value.", hr);

	hr = pModSoapResultModel->put_Price(varTemp.dblVal);
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();
	pTempNode = NULL;

    // Read rentalDuration Property
	hr = pNode->selectSingleNode(ELEMENT_RENTALDURATION, &pTempNode);
	if( FAILED(hr) || pTempNode == NULL ) 
		return Error(L"Cannot find ModSoapResultModel rentalDuration element.", hr);
		
	hr = m_pRentalDurationMapper->Read(pSoapReader, pTempNode, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return Error(L"Cannot map ModSoapResultModel rentalDuration element value.", hr);

	hr = pModSoapResultModel->put_RentalDuration(varTemp.lVal);
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();
	pTempNode = NULL;

    // Read result Property
	hr = pNode->selectSingleNode(ELEMENT_RESULT, &pTempNode);
	if( FAILED(hr) || pTempNode == NULL ) 
		return Error(L"Cannot find ModSoapResultModel result element.", hr);
		
	hr = m_pResultMapper->Read(pSoapReader, pTempNode, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return Error(L"Cannot map ModSoapResultModel result element value.", hr);

	hr = pModSoapResultModel->put_Result(varTemp.lVal);
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();
	pTempNode = NULL;
	
	// Set Return Value
	pvar->vt = VT_DISPATCH;
	pvar->pdispVal = pModSoapResultModel.Detach();


	// All Done
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CModSoapResultModelMap::Write(
	MSSOAPLib30::ISoapSerializer* pSoapSerializer, 
	BSTR bstrEncoding, 
	MSSOAPLib30::enEncodingStyle encodingMode, 
	LONG lFlags, 
	VARIANT * pvar)
{
	HRESULT hr;
	CComPtr<IModSoapResultModel> pModSoapResultModel;
	CComVariant varTemp;


	// Get IAddr interface of object to write.

	_ASSERTE(pvar != NULL);
	_ASSERTE(pvar->vt == VT_DISPATCH);
	_ASSERTE(pvar->pdispVal != NULL);

	hr = pvar->pdispVal->QueryInterface(&pModSoapResultModel); 
	if(FAILED(hr)) return hr;

	// Write price Property
	varTemp.vt = VT_R8;
	hr = pModSoapResultModel->get_Price(&varTemp.dblVal);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->StartElement(ELEMENT_PRICE, NULL, NULL , NULL);
	if( FAILED(hr) ) 
		return hr;

	hr = m_pPriceMapper->Write(pSoapSerializer, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->EndElement();
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();
	
	// Write rentalDuration Property
	varTemp.vt = VT_I4;
	hr = pModSoapResultModel->get_RentalDuration(&varTemp.lVal);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->StartElement(ELEMENT_RENTALDURATION, NULL, NULL , NULL);
	if( FAILED(hr) ) 
		return hr;

	hr = m_pRentalDurationMapper->Write(pSoapSerializer, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->EndElement();
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();

	// Write result Property
	//varTemp.vt = VT_BOOL;
	//hr = pModSoapResultModel->get_Result(&varTemp.boolVal);
	varTemp.vt = VT_I4;
	hr = pModSoapResultModel->get_Result(&varTemp.lVal);
	if( FAILED(hr) ) 
		return hr;

	hr = pSoapSerializer->StartElement(ELEMENT_RESULT, NULL, NULL , NULL);
	if( FAILED(hr) ) 
		return hr;

	hr = m_pResultMapper->Write(pSoapSerializer, bstrEncoding, encodingMode, lFlags, &varTemp);
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
STDMETHODIMP CModSoapResultModelMap::VarType(LONG * pvtType)
{
	if (!pvtType)
        return E_INVALIDARG;
	
	*pvtType = VT_DISPATCH;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CModSoapResultModelMap::Iid(BSTR * pIID)
{
	
	const GUID_STRING_SIZE = 39; // with terminating null
	OLECHAR GUIDString[GUID_STRING_SIZE];

	if (!pIID)
		return E_INVALIDARG;

	StringFromGUID2(IID_IModSoapResultModel, GUIDString, GUID_STRING_SIZE);

	*pIID = CComBSTR(GUIDString).Detach();

	return S_OK;

}


/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CModSoapResultModelMap::SchemaNode(MSXML2::IXMLDOMNode** par_SchemaNode)
{

	if (!par_SchemaNode)
		return E_INVALIDARG;

	*par_SchemaNode = m_pSchemaNode;
	(*par_SchemaNode)->AddRef();

	return S_OK;

}


/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CModSoapResultModelMap::XsdType(MSSOAPLib30::enXSDType* par_xsdType)
{

	if (!par_xsdType)
		return E_INVALIDARG;

	*par_xsdType = MSSOAPLib30::enXSDUndefined;

	return S_OK;

}


/////////////////////////////////////////////////////////////////////////////
HRESULT CModSoapResultModelMap::Error(LPOLESTR pMessage, HRESULT hr)
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
