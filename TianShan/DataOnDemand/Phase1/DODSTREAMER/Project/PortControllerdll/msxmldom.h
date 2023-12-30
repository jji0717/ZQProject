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

#if !defined(AFX_MARKUPMSXML_H__948A2705_9E68_11D2_A0BF_00105A27C570__INCLUDED_)
#define AFX_MARKUPMSXML_H__948A2705_9E68_11D2_A0BF_00105A27C570__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (disable:4786)

#include <afxtempl.h>

#if defined(MARKUP_MSXML4)
#if _MFC_VER < 0x0700
#include <msxml2.h>
#endif
#import <msxml4.dll>
#define MSXMLNS MSXML2
#elif defined(MARKUP_MSXML3)
#import <msxml3.dll>
#define MSXMLNS MSXML2
#else
#import <msxml.dll>
#define MSXMLNS MSXML
#endif



class CSCMemoryBlock
{
public:
	int m_iBlockNumber;	// -liqing-
public:

	/// Ctor
	CSCMemoryBlock(CHAR* pBlock, INT32 Size)
	{
		m_iBlockNumber = -1;
		m_Block = pBlock;
		m_iBlockSize = Size;
	}

	/// Copy Ctor
	CSCMemoryBlock(const CSCMemoryBlock& rhs)
	{
		m_iBlockNumber = rhs.m_iBlockNumber;
		m_Block = rhs.m_Block;
		m_iBlockSize = rhs.m_iBlockSize;
	}

	CSCMemoryBlock & operator= ( const CSCMemoryBlock & rhs )
	{
		if( this == &rhs )
			return *this;

		m_iBlockNumber = rhs.m_iBlockNumber;
		m_Block = rhs.m_Block;
		m_iBlockSize = rhs.m_iBlockSize;

		return *this;
	}

	/// Dtor
	~CSCMemoryBlock()
	{
		m_Block = NULL;
		m_iBlockSize = 0;
	}

	/// Allocate a memory block.
	/// @param size Bytes to allocate.
	/// @return a void pointer to the allocated space, or NULL.
	static CHAR* AllocBlock(const INT32 size)
	{
		return (CHAR*)malloc(size);
	}

	/// Free a memory block.
	/// @param block previously allocated memory block to be freed.
	static VOID FreeBlock(CHAR* block)
	{
		if( block )
		{
			free(block);
			block = NULL;
		}
	}

	/// Get a memory block pointer.
	/// @return the memory block pointer.
	CHAR* GetBlock() const
	{
		return m_Block;
	}

	/// Get a memory block size.
	/// @return the memory block size.
	INT32 GetSize() const
	{
		return m_iBlockSize;
	}

private:
	CHAR* m_Block;
	INT32 m_iBlockSize;
};



class CMSXmlDOM
{
public:
#if defined(MARKUP_MSXML3) || defined(MARKUP_MSXML4)
	MSXMLNS::IXMLDOMDocument2Ptr m_pDOMDoc;
#else
	MSXMLNS::IXMLDOMDocumentPtr m_pDOMDoc;
#endif

	CMSXmlDOM();
	CMSXmlDOM( LPCTSTR szDoc );
	virtual ~CMSXmlDOM();

	// Navigate

	/// Load a Xml document.
	/// @param szFilename pointer to a null-terminated string that specifies 
	/// the filename to open.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL Load( LPCTSTR szFileName );

	/// Load a memory based Xml document.
	/// @param szDoc pointer to a null-terminated string that specifies the 
	/// memory based Xml document.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL SetDoc( LPCTSTR szDoc );

	/// Find a specified Xml element.
	/// @param szName pointer to a null-terminated string that specifies the 
	/// element.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL FindElem( LPCTSTR szName=NULL );

	/// Find a specified Xml element under current Xml element scope.
	/// @param szName pointer to a null-terminated string that specifies the 
	/// element.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL FindChildElem( LPCTSTR szName=NULL );

	/// Walk into current Xml element.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL IntoElem();

	/// Walk out current Xml element.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL OutOfElem();

	/// Reset child element position.
	void ResetChildPos() { if ( m_pChild ) m_pChild.Release(); };

	/// Reset main position.
	void ResetMainPos() { ResetChildPos(); if ( m_pMain ) m_pMain.Release(); };

	/// Reset Xml document position.
	void ResetPos() { ResetMainPos(); m_pParent = m_pDOMDoc; };

	/// Get a specified Tag name.
	/// @return tag name.
	CString GetTagName() const { return x_GetTagName(m_pMain); };

	/// Get a specified child Tag name.
	/// @return child tag name.
	CString GetChildTagName() const { return x_GetTagName(m_pChild); };

	/// Get a specified element data.
	/// @return the element data.
	CString GetData() const { return x_GetData( m_pMain ); };

	/// Get a child element data under current Xml element section.
	/// @return the element data.
	CString GetChildData() const { return x_GetData(m_pChild); };

	/// Get a specified attribute value.
	/// @param szAttrib pointer to a null-terminated string that specifies the
	/// attribute.
	/// @return the attribute.
	CString GetAttrib( LPCTSTR szAttrib ) const { return x_GetAttrib( m_pMain, szAttrib ); };

	/// Get a child attribtue data under current Xml element section.
	/// @param szAttrib pointer to a null-terminated string that specifies the 
	/// attribute.
	/// @return the attribute.
	CString GetChildAttrib( LPCTSTR szAttrib ) const { return x_GetAttrib( m_pChild, szAttrib ); };

	/// Get attribute naem.
	/// @param n attribute index.
	/// @return the attribute name.
	CString GetAttribName( int n ) const;

	/// Get an error description.
	/// @return error description.
	CString GetError() const { return m_csError; };

	enum MarkupNodeType
	{
		MNT_ELEMENT					= 1,  // 0x01
		MNT_TEXT					= 2,  // 0x02
		MNT_WHITESPACE				= 4,  // 0x04
		MNT_CDATA_SECTION			= 8,  // 0x08
		MNT_PROCESSING_INSTRUCTION	= 16, // 0x10
		MNT_COMMENT					= 32, // 0x20
		MNT_DOCUMENT_TYPE			= 64, // 0x40
		MNT_EXCLUDE_WHITESPACE		= 123,// 0x7b
	};

	// Create

	/// Save a Xml document.
	/// @param szFilename pointer to a null-terminated string that specifies 
	/// the filename to open.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL Save( LPCTSTR szFileName );

	/// Get a memory based Xml document.
	/// @return Xml document presented by CString.
	CString GetDoc() const;

	/// Add a Xml element.
	/// @param szName pointer to a null-terminated string that specifies the 
	/// element.
	/// @param szData null-terminated string that specifies the element data.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL AddElem( LPCTSTR szName, LPCTSTR szData=NULL ) { return x_AddElem(szName,szData,false,false); };

	/// Insert a Xml element.
	/// @param szName pointer to a null-terminated string that specifies the 
	/// element.
	/// @param szData null-terminated string that specifies the element data.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL InsertElem( LPCTSTR szName, LPCTSTR szData=NULL ) { return x_AddElem(szName,szData,true,false); };

	/// Add a Xml element into current Xml element section.
	/// @param szName pointer to a null-terminated string that specifies the 
	/// element.
	/// @param szData null-terminated string that specifies the element data.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL AddChildElem( LPCTSTR szName, LPCTSTR szData=NULL ) { return x_AddElem(szName,szData,false,true); };

	/// Insert a Xml element into current Xml element section.
	/// @param szName pointer to a null-terminated string that specifies the 
	/// element.
	/// @param szData null-terminated string that specifies the element data.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL InsertChildElem( LPCTSTR szName, LPCTSTR szData=NULL ) { return x_AddElem(szName,szData,true,true); };

	/// Add a Xml attribute.
	/// @param szAttrib pointer to a null-terminated string that specifies the 
	/// attribute.
	/// @param szValue pointer to a null-terminated string that specifies the 
	/// attribute value.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL AddAttrib( LPCTSTR szAttrib, LPCTSTR szValue ) { return x_SetAttrib(m_pMain,szAttrib,szValue); };

	/// Add a Xml attribute under current Xml element section.
	/// @param szAttrib pointer to a null-terminated string that specifies the 
	/// attribute.
	/// @param szValue pointer to a null-terminated string that specifies the 
	/// attribute value.
	/// @return TRUE if the function succeeds, otherwise return FALSE.	
	BOOL AddChildAttrib( LPCTSTR szAttrib, LPCTSTR szValue ) { return x_SetAttrib(m_pChild,szAttrib,szValue); };

	/// Add a Xml attribute.
	/// @param szAttrib pointer to a null-terminated string that specifies the 
	/// attribute.
	/// @param nValue int value of the attribute data.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL AddAttrib( LPCTSTR szAttrib, int nValue ) { return x_SetAttrib(m_pMain,szAttrib,nValue); };

	/// Add a Xml attribute under current Xml element section.
	/// @param szAttrib pointer to a null-terminated string that specifies the
	/// attribute.
	/// @param nValue int value of the attribute data.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL AddChildAttrib( LPCTSTR szAttrib, int nValue ) { return x_SetAttrib(m_pChild,szAttrib,nValue); };

	/// Add a sub Xml document into current Xml document.
	/// @param szSubDoc pointer to a null-terminated string that specifies the 
	/// document.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL AddSubDoc( LPCTSTR szSubDoc ) { return x_AddSubDoc(szSubDoc,false,false); };

	/// Insert a sub Xml document into current Xml document.
	/// @param szSubDoc pointer to a null-terminated string that specifies the 
	/// document.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL InsertSubDoc( LPCTSTR szSubDoc ) { return x_AddSubDoc(szSubDoc,true,false); };

	/// Get the sub Xml document.
	/// @return the sub document.
	CString GetSubDoc() const { return x_GetSubDoc(m_pMain); };

	/// Add a sub Xml document into current Xml element.
	/// @param szSubDoc pointer to a null-terminated string that specifies the 
	/// document.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL AddChildSubDoc( LPCTSTR szSubDoc ) { return x_AddSubDoc(szSubDoc,false,true); };

	/// Insert a sub Xml document into current Xml element.
	/// @param szSubDoc pointer to a null-terminated string that specifies the 
	/// document.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL InsertChildSubDoc( LPCTSTR szSubDoc ) { return x_AddSubDoc(szSubDoc,true,true); };

	/// Get the sub Xml document under current Xml element section.
	/// @return the sub document.
	CString GetChildSubDoc() const { return x_GetSubDoc(m_pChild); };

	// Modify

	/// Remove current Xml element.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL RemoveElem();

	/// Remove Xml element under current Xml element section.
	/// @return TRUE if the function succeeds, otherwise return FALSE.
	BOOL RemoveChildElem();

	/// Set a Xml attribute.
	/// @param szAttrib pointer to a null-terminated string that specifies the 
	/// attribute.
	/// @param szValue pointer to a null-terminated string that specifies the 
	/// attribute value.
	/// @return TRUE if the function succeeds, otherwise return FALSE.	
	BOOL SetAttrib( LPCTSTR szAttrib, LPCTSTR szValue ) { return x_SetAttrib(m_pMain,szAttrib,szValue); };

	/// Set a Xml attribute under current Xml element section.
	/// @param szAttrib pointer to a null-terminated string that specifies the 
	/// attribute.
	/// @param szValue pointer to a null-terminated string that specifies the 
	/// attribute value.
	/// @return TRUE if the function succeeds, otherwise return FALSE.		
	BOOL SetChildAttrib( LPCTSTR szAttrib, LPCTSTR szValue ) { return x_SetAttrib(m_pChild,szAttrib,szValue); };

	/// Set a Xml attribute.
	/// @param szAttrib pointer to a null-terminated string that specifies the 
	/// attribute.
	/// @param nValue int value of the attribute data.
	/// @return TRUE if the function succeeds, otherwise return FALSE.	
	BOOL SetAttrib( LPCTSTR szAttrib, int nValue ) { return x_SetAttrib(m_pMain,szAttrib,nValue); };

	/// Set a Xml attribute under current Xml element section.
	/// @param szAttrib pointer to a null-terminated string that specifies the
	/// attribute.
	/// @param nValue int value of the attribute data.
	/// @return TRUE if the function succeeds, otherwise return FALSE.	
	BOOL SetChildAttrib( LPCTSTR szAttrib, int nValue ) { return x_SetAttrib(m_pChild,szAttrib,nValue); };

	/// Set data.
	/// @param szData pointer to a null-terminated string that specifies the
	/// data value.
	/// @return TRUE if the function succeeds, otherwise return FALSE.	
	BOOL SetData( LPCTSTR szData, int nCDATA=0 ) { return x_SetData(m_pMain,szData,nCDATA); };

	/// Set data under current Xml element.
	/// @param szData pointer to a null-terminated string that specifies the
	/// data value.
	/// @return TRUE if the function succeeds, otherwise return FALSE.	
	BOOL SetChildData( LPCTSTR szData, int nCDATA=0 ) { return x_SetData(m_pChild,szData,nCDATA); };

protected:
	MSXMLNS::IXMLDOMNodePtr m_pParent;
	MSXMLNS::IXMLDOMNodePtr m_pMain;
	MSXMLNS::IXMLDOMNodePtr m_pChild;
	CString m_csError;

	HRESULT x_CreateInstance();
	BOOL x_ParseError();
	MSXMLNS::IXMLDOMNodePtr x_FindElem( MSXMLNS::IXMLDOMNodePtr pParent, MSXMLNS::IXMLDOMNodePtr pNode, LPCTSTR szPath );
	CString x_GetTagName( MSXMLNS::IXMLDOMNodePtr pNode ) const;
	CString x_GetData( MSXMLNS::IXMLDOMNodePtr pNode ) const;
	CString x_GetAttrib( MSXMLNS::IXMLDOMNodePtr pNode, LPCTSTR szAttrib ) const;
	void CMSXmlDOM::x_Insert( MSXMLNS::IXMLDOMNodePtr pParent, MSXMLNS::IXMLDOMNodePtr pNext, MSXMLNS::IXMLDOMNodePtr pNew );
	BOOL x_AddElem( LPCTSTR szName, LPCTSTR szData, BOOL bInsert, BOOL bAddChild );
	CString x_GetSubDoc( MSXMLNS::IXMLDOMNodePtr pNode ) const;
	BOOL x_AddSubDoc( LPCTSTR szSubDoc, BOOL bInsert, BOOL bAddChild );
	BOOL x_SetAttrib( MSXMLNS::IXMLDOMNodePtr pNode, LPCTSTR szAttrib, LPCTSTR szValue );
	BOOL x_SetAttrib( MSXMLNS::IXMLDOMNodePtr pNode, LPCTSTR szAttrib, int nValue );
	BOOL x_SetData( MSXMLNS::IXMLDOMNodePtr& pNode, LPCTSTR szData, int nCDATA );
};

#endif // !defined(AFX_MARKUPMSXML_H__948A2705_9E68_11D2_A0BF_00105A27C570__INCLUDED_)
