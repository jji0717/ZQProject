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
// Name  : SaveXPathNamespaces.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-1-12
// Desc  : this class handle Xpath in soap messages
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/PlaylistAS/PLAYLISTSOAPUDTMAPPER/SaveXPathNamespaces.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 1     05-02-16 11:56 Bernie.zhao
// new mapper
// 
// 1     05-01-14 11:05 Bernie.zhao
// ===========================================================================
// SaveXPathNamespaces.h: interface for the CSaveXPathNamespaces class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SAVEXPATHNAMESPACES_H__C015B495_C62D_4CB1_8153_48F3E2B0311F__INCLUDED_)
#define AFX_SAVEXPATHNAMESPACES_H__C015B495_C62D_4CB1_8153_48F3E2B0311F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSaveXPathNamespaces  
{

public:
	
	inline CSaveXPathNamespaces(MSXML2::IXMLDOMNode* pNode, LPTSTR pNewNamespaces)
	{

		HRESULT hr;

		// Get document that owns the node.
		
		CComPtr<MSXML2::IXMLDOMDocument> pDoc;
		
		hr = pNode->get_ownerDocument(&pDoc);
		_ASSERT(SUCCEEDED(hr));

		hr = pDoc.QueryInterface(&m_pDoc);
		_ASSERT(SUCCEEDED(hr));

		// Save current SelectionLanguage property value, then change
		// to allow selection using XPath.

		hr = m_pDoc->getProperty(SELECTION_LANGUAGE, &m_SavedLanguage);
		_ASSERT(SUCCEEDED(hr));

		hr = m_pDoc->setProperty(SELECTION_LANGUAGE, XPATH);
		_ASSERT(SUCCEEDED(hr));

		// Save the current SelectionNamepsaces property value, then change
		// to allow elements in the XSD namespace to be selected.

		hr = m_pDoc->getProperty(SELECTION_NAMESPACES, &m_SavedNamespaces);
		_ASSERT(SUCCEEDED(hr));

		hr = m_pDoc->setProperty(SELECTION_NAMESPACES, CComVariant(pNewNamespaces));


	}

	inline ~CSaveXPathNamespaces()
	{
		m_pDoc->setProperty(L"SelectionLanguage", m_SavedLanguage);
		m_pDoc->setProperty(L"SelectionNamespaces", m_SavedNamespaces);
	}

private:

	CComPtr<MSXML2::IXMLDOMDocument2> m_pDoc;

	CComVariant m_SavedLanguage;
	CComVariant m_SavedNamespaces;

	static const CComBSTR SELECTION_LANGUAGE;
	static const CComBSTR SELECTION_NAMESPACES;
	static const CComVariant XPATH;

};

#endif // !defined(AFX_SAVEXPATHNAMESPACES_H__C015B495_C62D_4CB1_8153_48F3E2B0311F__INCLUDED_)
