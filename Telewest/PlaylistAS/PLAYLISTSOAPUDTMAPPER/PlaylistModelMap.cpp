// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Name  : PlaylistModelMap.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-1-12
// Desc  : impl for PlaylistModelMap class
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/PlaylistAS/PLAYLISTSOAPUDTMAPPER/PlaylistModelMap.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 2     05-04-20 19:03 Bernie.zhao
// 
// 1     05-02-16 11:56 Bernie.zhao
// new mapper
// 
// 1     05-01-14 11:05 Bernie.zhao
// ===========================================================================
// PlaylistModelMap.cpp : Implementation of CPlaylistModelMap
#include "stdafx.h"
#include "PlaylistSoapUDTMapper.h"
#include "PlaylistModelMap.h"
#include "SaveXPathNamespaces.h"

/////////////////////////////////////////////////////////////////////////////
// CPlaylistModelMap
/////////////////////////////////////////////////////////////////////////////
const CComBSTR CPlaylistModelMap::HREF_ATTRIBUTE = L"href";
const CComBSTR CPlaylistModelMap::PLAYLIST_ID = L"playlistID";
const CComBSTR CPlaylistModelMap::ELEMENTS = L"elements";

STDMETHODIMP CPlaylistModelMap::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IPlaylistModelMap
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (::ATL::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CPlaylistModelMap::Init(
	MSSOAPLib30::ISoapTypeMapperFactory* pFactory, 
	MSXML2::IXMLDOMNode* pSchemaNode, 
	MSXML2::IXMLDOMNode* pWSMLNode, 
	MSSOAPLib30::enXSDType xsdType)
{
	HRESULT hr;
	CComPtr<MSXML2::IXMLDOMNode> pNode;


	// Save schema node for return by SchemaNode method.
	m_pSchemaNode = pSchemaNode;

	
	// Set selection language to XPath and define XSD namespace prefix.
	CSaveXPathNamespaces SavedXPathNamespaces(pSchemaNode, _T("xmlns:xsd='http://www.w3.org/2001/XMLSchema'"));
	
	// Get mapper for Asset element.
    hr = pSchemaNode->selectSingleNode(CComBSTR(L"xsd:sequence/xsd:element[@name='elements']"), &pNode);
	if( FAILED(hr) || pNode == NULL ) 
		return Error(L"Cannot find elements element in PlaylistModel schema.", hr);

	hr = pFactory->GetElementMapper(pNode, &m_pElementsMapper);
	if( FAILED(hr) ) 
		return Error(L"Cannot get mapper for PlaylistModel elements element.", hr);

	pNode = NULL;

	// Get mapper for PlaylistID element.
    hr = pSchemaNode->selectSingleNode(CComBSTR(L"xsd:sequence/xsd:element[@name='playlistID']"), &pNode);
	if( FAILED(hr) || pNode == NULL ) 
		return Error(L"Cannot find PlaylistID element in PlaylistModel schema.", hr);

	hr = pFactory->GetElementMapper(pNode, &m_pPlaylistIDMapper);
	if( FAILED(hr) ) 
		return Error(L"Cannot get mapper for PlaylistModel PlaylistID element.", hr);

	pNode = NULL;
	
	// All Done

	return S_OK;

}

/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPlaylistModelMap::Read(
	MSSOAPLib30::ISoapReader * pSoapReader, 
	MSXML2::IXMLDOMNode * pNode, 
	BSTR bstrEncoding, 
	MSSOAPLib30::enEncodingStyle encodingMode, 
	LONG lFlags, 
	VARIANT * pvar)
{

	HRESULT hr;
	CComPtr<MSXML2::IXMLDOMNode> pTempNode;
	CComPtr<MSXML2::IXMLDOMNodeList> pTempNodeList;
	CComPtr<MSXML2::IXMLDOMNamedNodeMap> pAttributes;
	CComPtr<IPlaylistModel> pPlaylistModel;
	CComPtr<IElementArray> pElementArray;
	CComBSTR Element;
	CComVariant varTemp;
	unsigned long ulIndex;
	unsigned long ulCount;

	_ASSERT(pNode != NULL);


	//////////////////////////////////////////////////////////////////////////
	// ** this block is for test only

	// TODO: delete this after stable
#ifdef _DEBUG
	MSXML2::IXMLDOMDocument* tmpDoc;
	CComBSTR bfile("E:\\bernieworkpath\\ZQProjs\\PlaylistAS\\pas.xml");
	CComVariant varString(bfile);

	pNode->get_ownerDocument(&tmpDoc);
	tmpDoc->save(varString);
#endif

	//////////////////////////////////////////////////////////////////////////
	
	// Create Addr That Will Be Read

	hr = pPlaylistModel.CoCreateInstance(__uuidof(PlaylistModel));
	if( FAILED(hr) ) 
		return Error(L"Cannot create PlaylistModel object.", hr);


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

	pTempNode = NULL;

    // Read element Properties
	hr = pNode->selectSingleNode(ELEMENTS, &pTempNode);
	if( FAILED(hr) || pTempNode == NULL) return Error(L"Cannot find PlaylistModel elements element.", hr);

	hr = m_pElementsMapper->Read(pSoapReader, pTempNode, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) return Error(L"Cannot map PlaylistModel elements element value.", hr);

	if( (varTemp.vt != (VT_ARRAY | VT_BSTR)) ) 
	{
		return Error(L"Cannot map PlaylistModel elements element value to an array of string objects.");
	}

	hr = pPlaylistModel->get_Elements(&pElementArray);
	if( FAILED(hr) ) return Error(L"Cannot get elements collection from PlaylistModel object.", hr);

	ulCount = varTemp.parray->rgsabound[0].cElements;
	for(ulIndex = 0; ulIndex < ulCount; ++ulIndex)
	{

		hr = SafeArrayGetElement(varTemp.parray, reinterpret_cast<long*>(&ulIndex), &Element);
		if( FAILED(hr) ) return hr;

		hr = pElementArray->Add(Element);
		if( FAILED(hr) ) return hr;

		Element.Empty();

	}

	varTemp.Clear();
	pTempNode = NULL;
	
	
	// Read PlaylistID Property
	hr = pNode->selectSingleNode(PLAYLIST_ID, &pTempNode);
	if( FAILED(hr) || pTempNode == NULL ) 
		return Error(L"Cannot find PlaylistModel PlaylistID element.", hr);
		
	hr = m_pPlaylistIDMapper->Read(pSoapReader, pTempNode, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) 
		return Error(L"Cannot map PlaylistModel PlaylistID element value.", hr);

	hr = pPlaylistModel->put_PlaylistID(varTemp.bstrVal);
	if( FAILED(hr) ) 
		return hr;

	varTemp.Clear();
	pTempNode = NULL;


    // Set Return Value

	pvar->vt = VT_DISPATCH;
	pvar->pdispVal = pPlaylistModel.Detach();

	// All Done

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPlaylistModelMap::Write(
	MSSOAPLib30::ISoapSerializer* pSoapSerializer, 
	BSTR bstrEncoding, 
	MSSOAPLib30::enEncodingStyle encodingMode, 
	LONG lFlags, 
	VARIANT * pvar)
{
	HRESULT hr;
	CComPtr<IPlaylistModel> pPlaylistModel;
	CComPtr<IElementArray> pElementArray;
	CComBSTR Element;
	CComVariant varTemp;
	long lIndex;
	long lCount;


	// Get IPlaylistModel interface of object to write.

	_ASSERTE(pvar != NULL);
	_ASSERTE(pvar->vt == VT_DISPATCH);
	_ASSERTE(pvar->pdispVal != NULL);

	hr = pvar->pdispVal->QueryInterface(&pPlaylistModel); 
	if(FAILED(hr)) return hr;

	// Write elements Property

	hr = pPlaylistModel->get_Elements(&pElementArray);
	if( FAILED(hr) ) return hr;

	hr = pElementArray->get_Count(&lCount);
	if( FAILED(hr) ) return hr;

	varTemp.vt = VT_ARRAY | VT_DISPATCH;
	varTemp.parray = SafeArrayCreateVectorEx(VT_BSTR, 0, lCount, NULL);
	if( varTemp.parray == NULL ) return E_FAIL;

	for(lIndex = 0; lIndex < lCount; ++lIndex)
	{
		
		hr = pElementArray->get_Item(lIndex, &Element);
		if( FAILED(hr) ) return hr;

		hr = SafeArrayPutElement(varTemp.parray, &lIndex, &Element);
		if( FAILED(hr) ) return hr;

		Element.Empty();

	}

	hr = pSoapSerializer->StartElement(ELEMENTS, NULL, NULL , NULL);
	if( FAILED(hr) ) return hr;

	hr = m_pElementsMapper->Write(pSoapSerializer, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) return hr;

	hr = pSoapSerializer->EndElement();
	if( FAILED(hr) ) return hr;

	varTemp.Clear();

	
	// Write PlaylistID Property
	varTemp.vt = VT_BSTR;
	hr = pPlaylistModel->get_PlaylistID(&varTemp.bstrVal);
	if( FAILED(hr) ) return hr;

	hr = pSoapSerializer->StartElement(PLAYLIST_ID, NULL, NULL , NULL);
	if( FAILED(hr) ) return hr;

	hr = m_pPlaylistIDMapper->Write(pSoapSerializer, bstrEncoding, encodingMode, lFlags, &varTemp);
	if( FAILED(hr) ) return hr;

	hr = pSoapSerializer->EndElement();
	if( FAILED(hr) ) return hr;

	varTemp.Clear();

	// All Done

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPlaylistModelMap::VarType(LONG * pvtType)
{
	if (!pvtType)
        return E_INVALIDARG;
	
	*pvtType = VT_DISPATCH;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPlaylistModelMap::Iid(BSTR * pIID)
{
	
	const GUID_STRING_SIZE = 39; // with terminating null
	OLECHAR GUIDString[GUID_STRING_SIZE];

	if (!pIID)
		return E_INVALIDARG;

	StringFromGUID2(IID_IPlaylistModel, GUIDString, GUID_STRING_SIZE);

	*pIID = CComBSTR(GUIDString).Detach();

	return S_OK;

}


/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPlaylistModelMap::SchemaNode(MSXML2::IXMLDOMNode** par_SchemaNode)
{

	if (!par_SchemaNode)
		return E_INVALIDARG;

	*par_SchemaNode = m_pSchemaNode;
	(*par_SchemaNode)->AddRef();

	return S_OK;

}


/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CPlaylistModelMap::XsdType(MSSOAPLib30::enXSDType* par_xsdType)
{

	if (!par_xsdType)
		return E_INVALIDARG;

	*par_xsdType = MSSOAPLib30::enXSDUndefined;

	return S_OK;

}


/////////////////////////////////////////////////////////////////////////////
HRESULT CPlaylistModelMap::Error(LPOLESTR pMessage, HRESULT hr)
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
