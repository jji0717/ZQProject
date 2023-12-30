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
// Name  : PlaylistModel.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-1-12
// Desc  : this class is a user defined type for WSDL type 'PlaylistModel'
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/PlaylistAS/PLAYLISTSOAPUDTMAPPER/PlaylistModel.h $
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
// PlaylistModel.h : Declaration of the CPlaylistModel

#ifndef __PLAYLISTMODEL_H_
#define __PLAYLISTMODEL_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CPlaylistModel
class ATL_NO_VTABLE CPlaylistModel : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPlaylistModel, &CLSID_PlaylistModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IPlaylistModel, &IID_IPlaylistModel, &LIBID_PLAYLISTSOAPUDTMAPPERLib>
{
public:
	CPlaylistModel()
	{
		HRESULT hr;

		m_strPlaylistID = L"";
		
		hr = m_pElements.CoCreateInstance(__uuidof(ElementArray));
		_ASSERT(SUCCEEDED(hr));
	}
	
	virtual ~CPlaylistModel()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_PLAYLISTMODEL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPlaylistModel)
	COM_INTERFACE_ENTRY(IPlaylistModel)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IPlaylistModel
public:
	STDMETHOD(get_Elements)(/*[out, retval]*/ IElementArray* *pVal);
	STDMETHOD(get_PlaylistID)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_PlaylistID)(/*[in]*/ BSTR newVal);

protected:
	CComBSTR m_strPlaylistID;
	CComPtr<IElementArray> m_pElements;
};

#endif //__PLAYLISTMODEL_H_
