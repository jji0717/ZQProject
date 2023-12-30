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
// Name  : PlaylistModelMap.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-1-12
// Desc  : this class is a UDT for WSDL type 'PlaylistModel'
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/PlaylistAS/PLAYLISTSOAPUDTMAPPER/PlaylistModelMap.h $
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
// PlaylistModelMap.h : Declaration of the CPlaylistModelMap

#ifndef __PLAYLISTMODELMAP_H_
#define __PLAYLISTMODELMAP_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CPlaylistModelMap
class ATL_NO_VTABLE CPlaylistModelMap : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPlaylistModelMap, &CLSID_PlaylistModelMap>,
	public ISupportErrorInfo,
	public IDispatchImpl<IPlaylistModelMap, &IID_IPlaylistModelMap, &LIBID_PLAYLISTSOAPUDTMAPPERLib>,
	public IDispatchImpl<MSSOAPLib30::ISoapTypeMapper, &MSSOAPLib30::IID_ISoapTypeMapper, &MSSOAPLib30::LIBID_MSSOAPLib30>
{
public:
	CPlaylistModelMap()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_PLAYLISTMODELMAP)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPlaylistModelMap)
	COM_INTERFACE_ENTRY(IPlaylistModelMap)
	COM_INTERFACE_ENTRY2(IDispatch, IPlaylistModelMap)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(MSSOAPLib30::ISoapTypeMapper)
END_COM_MAP()

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IPlaylistModelMap
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
	MSSOAPLib30::ISoapTypeMapperPtr m_pPlaylistIDMapper;
	MSSOAPLib30::ISoapTypeMapperPtr m_pElementsMapper;

	HRESULT Error(LPOLESTR pMessage, HRESULT hr = 0);

	static const CComBSTR HREF_ATTRIBUTE;
	static const CComBSTR PLAYLIST_ID;
	static const CComBSTR ELEMENTS;
};

#endif //__PLAYLISTMODELMAP_H_
