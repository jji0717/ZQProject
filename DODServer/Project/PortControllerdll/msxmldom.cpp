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
// ===========================================================================

#include "stdafx.h"
#include "afxconv.h"
#include "msxmldom.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CMSXmlDOM::CMSXmlDOM()
{
	CoInitialize(NULL);
	x_CreateInstance();
}

CMSXmlDOM::CMSXmlDOM( LPCTSTR szDoc )
{
	CoInitialize(NULL);
	if ( SUCCEEDED(x_CreateInstance()) )
		SetDoc( szDoc );
}

CMSXmlDOM::~CMSXmlDOM()
{
	// Release COM interfaces, in case CoUninitialize is being called the last time
	if ( m_pChild )
		m_pChild.Release();
	if ( m_pMain )
		m_pMain.Release();
	if ( m_pParent )
		m_pParent.Release();
	if ( m_pDOMDoc )
		m_pDOMDoc.Release();
	CoUninitialize();
}

HRESULT CMSXmlDOM::x_CreateInstance()
{
	// Release any reference to a previous instance
	if ( m_pParent )
		m_pParent.Release();
	if ( m_pDOMDoc )
		m_pDOMDoc.Release();

	// Create new instance
#if defined( MARKUP_MSXML4 )
	HRESULT hr = m_pDOMDoc.CreateInstance( __uuidof(MSXML2::DOMDocument40) );
#elif defined( MARKUP_MSXML3 )
	HRESULT hr = m_pDOMDoc.CreateInstance( __uuidof(MSXML2::DOMDocument) );
#else
	HRESULT hr = m_pDOMDoc.CreateInstance( "Microsoft.XMLDOM" );
#endif
	if ( FAILED(hr) )
	{
		if ( hr == REGDB_E_CLASSNOTREG )
			AfxMessageBox( _T("MSXML not registered") );
		else
			AfxMessageBox( _T("Unable to create MSXML instance") );
	}
	// m_pDOMDoc->PutpreserveWhiteSpace( TRUE );
	m_pParent = m_pDOMDoc;
	return hr;
}

BOOL CMSXmlDOM::x_ParseError()
{
	MSXMLNS::IXMLDOMParseErrorPtr pDOMParseError;
	m_pDOMDoc->get_parseError( &pDOMParseError );
	m_csError = (LPCTSTR)pDOMParseError->Getreason();
	m_csError.TrimRight( _T("\r\n") );
	return FALSE;
}

BOOL CMSXmlDOM::SetDoc( LPCTSTR szDoc )
{
	ResetPos();

	// If szDoc is empty, clear it
	if ( ! szDoc || ! szDoc[0] )
		return SUCCEEDED( x_CreateInstance() );

#ifndef _UNICODE
#if defined(MARKUP_MSXML3) || defined(MARKUP_MSXML4)
	VARIANT_BOOL bResult = m_pDOMDoc->loadXML( szDoc );
#else
	USES_CONVERSION;
	_bstr_t bstrDoc(A2BSTR(szDoc),false);
	VARIANT_BOOL bResult = m_pDOMDoc->loadXML( bstrDoc );
#endif
#else
	_bstr_t bstrDoc(szDoc);
	VARIANT_BOOL bResult = m_pDOMDoc->loadXML( bstrDoc );
#endif
	if ( ! bResult )
		return x_ParseError();
	return TRUE;
};

BOOL CMSXmlDOM::Load( LPCTSTR szFileName )
{
	_variant_t varName;
#ifdef _UNICODE
	varName.vt = VT_BSTR;
	varName.bstrVal = SysAllocString(szFileName);
#else
	varName.SetString(szFileName);
#endif
	VARIANT_BOOL bResult = m_pDOMDoc->load( varName );
	ResetPos();
	if ( ! bResult )
		return x_ParseError();
	return TRUE;
}

BOOL CMSXmlDOM::Save( LPCTSTR szFileName )
{
	_variant_t varName;
#ifdef _UNICODE
	varName.vt = VT_BSTR;
	varName.bstrVal = SysAllocString(szFileName);
#else
	varName.SetString(szFileName);
#endif
	HRESULT hr = m_pDOMDoc->save( varName );
	if ( hr )
		return FALSE;
	return TRUE;
}

CString CMSXmlDOM::GetDoc() const
{
	return (LPCTSTR)m_pDOMDoc->xml;
};

BOOL CMSXmlDOM::FindElem( LPCTSTR szName )
{
	// Change current position only if found
	//
	MSXMLNS::IXMLDOMNodePtr pNode;
	pNode = x_FindElem( m_pParent, m_pMain, szName );
	if ( pNode )
	{
		m_pMain = pNode;
		m_pParent = m_pMain->GetparentNode();
		if ( m_pChild )
			m_pChild.Release();
		return TRUE;
	}
	return FALSE;
}

BOOL CMSXmlDOM::FindChildElem( LPCTSTR szName )
{
	// Change current child position only if found
	//
	// Shorthand: call this with no current main position
	// means find child under root element
	if ( ! ((bool)(m_pParent->GetparentNode())) && ! ((bool)m_pMain) )
		FindElem();
	if ( ! ((bool)m_pMain) )
		return FALSE;

	MSXMLNS::IXMLDOMNodePtr pNode;
	pNode = x_FindElem( m_pMain, m_pChild, szName );
	if ( pNode )
	{
		m_pChild = pNode;
		m_pMain = m_pChild->GetparentNode();
		m_pParent = m_pMain->GetparentNode();
		return TRUE;
	}

	return FALSE;
}

BOOL CMSXmlDOM::IntoElem()
{
	if ( m_pMain )
	{
		m_pParent = m_pMain;
		if ( m_pChild )
		{
			m_pMain = m_pChild;
			m_pChild.Release();
		}
		else
			m_pMain.Release();
		return TRUE;
	}
	return FALSE;
}

BOOL CMSXmlDOM::OutOfElem()
{
	if ( (bool)(m_pParent->GetparentNode()) )
	{
		m_pChild = m_pMain;
		m_pMain = m_pParent;
		m_pParent = m_pMain->GetparentNode();
		return TRUE;
	}
	return FALSE;
}

CString CMSXmlDOM::GetAttribName( int n ) const
{
	CString csAttribName;
	if ( ! (bool)m_pMain )
		return csAttribName;

	// Is it within range?
	if ( n >= 0 && n < m_pMain->Getattributes()->Getlength() )
	{
		MSXMLNS::IXMLDOMNodePtr pAttrib = m_pMain->Getattributes()->item[n];
		csAttribName = (LPCTSTR)pAttrib->GetnodeName();
	}
	return csAttribName;
}

BOOL CMSXmlDOM::RemoveElem()
{
	if ( m_pMain )
	{
		MSXMLNS::IXMLDOMNodePtr pParent = m_pMain->GetparentNode();
		if ( pParent )
		{
			if ( m_pChild )
				m_pChild.Release();
			MSXMLNS::IXMLDOMNodePtr pPrev = m_pMain->GetpreviousSibling();
			pParent->removeChild( m_pMain );
			m_pMain = pPrev;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CMSXmlDOM::RemoveChildElem()
{
	if ( m_pChild )
	{
		MSXMLNS::IXMLDOMNodePtr pPrev = m_pChild->GetpreviousSibling();
		m_pMain->removeChild( m_pChild );
		m_pChild.Release();
		m_pChild = pPrev;
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////
// Private Methods
//////////////////////////////////////////////////////////////////////

MSXMLNS::IXMLDOMNodePtr CMSXmlDOM::x_FindElem( MSXMLNS::IXMLDOMNodePtr pParent, MSXMLNS::IXMLDOMNodePtr pNode, LPCTSTR szPath )
{
	// If szPath is NULL or empty, go to next sibling element
	// Otherwise go to next sibling element with matching path
	//
	if ( pNode )
		pNode = pNode->GetnextSibling();
	else
		pNode = pParent->GetfirstChild();

	while ( pNode )
	{
		if ( pNode->nodeType == MSXMLNS::NODE_ELEMENT )
		{
			// Compare tag name unless szPath is not specified
			if ( szPath == NULL || !szPath[0] || x_GetTagName(pNode) == szPath )
				break;
		}
		pNode = pNode->GetnextSibling();
	}
	return pNode;

}

CString CMSXmlDOM::x_GetTagName( MSXMLNS::IXMLDOMNodePtr pNode ) const
{
	CString csTagName = (LPCTSTR)pNode->GetnodeName();
	return csTagName;
}

CString CMSXmlDOM::x_GetAttrib( MSXMLNS::IXMLDOMNodePtr pNode, LPCTSTR szAttrib ) const
{
	CString csAttrib;
	MSXMLNS::IXMLDOMNodePtr pAttrib;
	HRESULT hr = pNode->Getattributes()->raw_getNamedItem( _bstr_t(szAttrib), &pAttrib );
	if ( SUCCEEDED(hr) && ((bool)pAttrib) )
	{
		_variant_t varVal = pAttrib->GetnodeValue();
		if ( varVal.vt == VT_BSTR )
			csAttrib = (LPCTSTR)_bstr_t(varVal.bstrVal);
	}
	return csAttrib;
}

void CMSXmlDOM::x_Insert( MSXMLNS::IXMLDOMNodePtr pParent, MSXMLNS::IXMLDOMNodePtr pNext, MSXMLNS::IXMLDOMNodePtr pNew )
{
	if ( pNext )
	{
		VARIANT varRef;
		VariantInit( &varRef );
		varRef.vt = VT_DISPATCH;
		varRef.pdispVal = pNext.GetInterfacePtr();
		pParent->insertBefore( pNew, varRef );
	}
	else
		pParent->appendChild( pNew );
}

BOOL CMSXmlDOM::x_AddElem( LPCTSTR szName, LPCTSTR szData, BOOL bInsert, BOOL bAddChild )
{
	MSXMLNS::IXMLDOMNodePtr pNext, pParent;
	if ( bAddChild )
	{
		if ( ! (bool)m_pMain )
			return FALSE;
		pParent = m_pMain;
		pNext = m_pChild;
	}
	else
	{
		if ( m_pChild )
			m_pChild.Release();
		pParent = m_pParent;
		pNext = m_pMain;
	}
	if ( bInsert )
	{
		if ( ! ((bool)pNext) )
			pNext = pParent->GetfirstChild();
	}
	else
	{
		if ( pNext )
			pNext = pNext->GetnextSibling();
	}

	MSXMLNS::IXMLDOMElementPtr pNew = m_pDOMDoc->createElement( _bstr_t(szName) );
	x_Insert( pParent, pNext, pNew );
	if ( szData && szData[0] )
	{
		MSXMLNS::IXMLDOMNodePtr pText = m_pDOMDoc->createTextNode( _bstr_t(szData) );
		pNew->appendChild( pText );
	}

	if ( bAddChild )
		m_pChild = pNew;
	else
		m_pMain = pNew;
	return TRUE;
}

CString CMSXmlDOM::x_GetSubDoc( MSXMLNS::IXMLDOMNodePtr pNode ) const
{
	if ( (bool)pNode )
		return (LPCTSTR)pNode->xml;
	return _T("");
}

BOOL CMSXmlDOM::x_AddSubDoc( LPCTSTR szSubDoc, BOOL bInsert, BOOL bAddChild )
{
	MSXMLNS::IXMLDOMNodePtr pNext, pParent;
	if ( bAddChild )
	{
		// Add a subdocument under main position, before or after child
		if ( ! (bool)m_pMain )
			return FALSE;
		pParent = m_pMain;
		pNext = m_pChild;
	}
	else
	{
		// Add a subdocument under parent position, before or after main
		if ( ! (bool)m_pParent )
			return FALSE;
		pParent = m_pParent;
		pNext = m_pMain;
	}
	if ( bInsert )
	{
		if ( ! ((bool)pNext) )
			pNext = pParent->GetfirstChild();
	}
	else
	{
		if ( pNext )
			pNext = pNext->GetnextSibling();
	}

#if defined(MARKUP_MSXML4)
	MSXMLNS::IXMLDOMDocument2Ptr pSubDoc;
	pSubDoc.CreateInstance( __uuidof(MSXML2::DOMDocument40) );
#elif defined(MARKUP_MSXML3)
	MSXMLNS::IXMLDOMDocument2Ptr pSubDoc;
	pSubDoc.CreateInstance( __uuidof(MSXML2::DOMDocument) );
#else
	MSXMLNS::IXMLDOMDocumentPtr pSubDoc;
	pSubDoc.CreateInstance( _T("Microsoft.XMLDOM") );
#endif
	// pSubDoc->PutpreserveWhiteSpace( TRUE );
#ifndef _UNICODE
#if defined(MARKUP_MSXML3) || defined(MARKUP_MSXML4)
	VARIANT_BOOL bResult = pSubDoc->loadXML( szSubDoc );
#else
	USES_CONVERSION;
	_bstr_t bstrSubDoc(A2BSTR(szSubDoc),false);
	VARIANT_BOOL bResult = pSubDoc->loadXML( bstrSubDoc );
#endif
#else
	_bstr_t bstrSubDoc(szSubDoc);
	VARIANT_BOOL bResult = pSubDoc->loadXML( bstrSubDoc );
#endif
	if ( ! bResult )
		return FALSE;
	MSXMLNS::IXMLDOMElementPtr pNew = pSubDoc->GetdocumentElement();
	x_Insert( pParent, pNext, pNew );

	if ( bAddChild )
	{
		m_pChild = pNew;
	}
	else
	{
		m_pMain = pNew;
		if ( m_pChild )
			m_pChild.Release();
	}
	return TRUE;
}

BOOL CMSXmlDOM::x_SetAttrib( MSXMLNS::IXMLDOMNodePtr pNode, LPCTSTR szAttrib, int nValue )
{
	_TCHAR szVal[25];
	_itot( nValue, szVal, 10 );
	return x_SetAttrib( pNode, szAttrib, szVal );
}

BOOL CMSXmlDOM::x_SetAttrib( MSXMLNS::IXMLDOMNodePtr pNode, LPCTSTR szAttrib, LPCTSTR szValue )
{
	if ( ! ((bool)pNode) )
		return FALSE;

	MSXMLNS::IXMLDOMNamedNodeMapPtr pAttribs = pNode->Getattributes();
	MSXMLNS::IXMLDOMAttributePtr pAttr = m_pDOMDoc->createAttribute( _bstr_t(szAttrib) );
	if ( pAttr )
	{
		_variant_t varVal;
#ifdef _UNICODE
		varVal.vt = VT_BSTR;
		varVal.bstrVal = SysAllocString(szValue);
#else
		varVal.SetString(szValue);
#endif
		pAttr->put_value( varVal );
		pAttribs->setNamedItem( pAttr );
		return TRUE;
	}
	return FALSE;
}


CString CMSXmlDOM::x_GetData( MSXMLNS::IXMLDOMNodePtr pNode ) const
{
	CString csData;
	if ( ! ((bool)pNode) )
		return csData;


	if ( (bool)(pNode->GetfirstChild()) )
		csData = (LPCTSTR) pNode->Gettext();
	return csData;
}

BOOL CMSXmlDOM::x_SetData( MSXMLNS::IXMLDOMNodePtr& pNode, LPCTSTR szData, int nCDATA )
{
	if ( ! ((bool)pNode) )
		return FALSE;


	// Element
	MSXMLNS::IXMLDOMNodePtr pChild = pNode->GetfirstChild();
	if ( (bool)pChild && pChild->nodeType != MSXMLNS::NODE_TEXT
			&& pChild->nodeType != MSXMLNS::NODE_CDATA_SECTION )
		return FALSE;
	MSXMLNS::IXMLDOMNodePtr pText;
	if ( nCDATA != 0 )
	{
		MSXMLNS::IXMLDOMCDATASectionPtr pData;
		pData = m_pDOMDoc->createCDATASection( _bstr_t(szData) );
		if ( (bool)pChild )
			pNode->replaceChild( pData, pChild );
		else
			pNode->appendChild( pData );
	}
	else
	{
		pText = m_pDOMDoc->createTextNode( _bstr_t(szData) );
		if ( (bool)pChild )
			pNode->replaceChild( pText, pChild );
		else
			pNode->appendChild( pText );
	}
	return TRUE;
}
