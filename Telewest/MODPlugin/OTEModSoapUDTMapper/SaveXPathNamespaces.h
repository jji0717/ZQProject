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
