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
// Name  : PlaylistModel.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-1-12
// Desc  : impl for PlaylistModel class
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/PlaylistAS/PLAYLISTSOAPUDTMAPPER/PlaylistModel.cpp $
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
// PlaylistModel.cpp : Implementation of CPlaylistModel
#include "stdafx.h"
#include "PlaylistSoapUDTMapper.h"
#include "PlaylistModel.h"

/////////////////////////////////////////////////////////////////////////////
// CPlaylistModel

STDMETHODIMP CPlaylistModel::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IPlaylistModel
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (::ATL::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CPlaylistModel::get_PlaylistID(BSTR *pVal)
{
	*pVal = m_strPlaylistID.Copy();
	return S_OK;
}

STDMETHODIMP CPlaylistModel::put_PlaylistID(BSTR newVal)
{
	m_strPlaylistID = newVal;
	return S_OK;
}

STDMETHODIMP CPlaylistModel::get_Elements(IElementArray **pVal)
{
	return m_pElements.CopyTo(pVal);
}

