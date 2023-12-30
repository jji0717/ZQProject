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
// Name  : ElementArray.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-1-12
// Desc  : impl for ElementArray class
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/PlaylistAS/PLAYLISTSOAPUDTMAPPER/ElementArray.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 2     05-04-20 19:03 Bernie.zhao
// 
// 1     05-02-16 11:55 Bernie.zhao
// new mapper
// 
// 1     05-01-14 11:05 Bernie.zhao
// ===========================================================================

// ElementArray.cpp : Implementation of CElementArray
#include "stdafx.h"
#include "PlaylistSoapUDTMapper.h"
#include "ElementArray.h"

/////////////////////////////////////////////////////////////////////////////
// CElementArray

STDMETHODIMP CElementArray::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IElementArray
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (::ATL::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CElementArray::get_Count(long *pVal)
{
	*pVal = m_UsedCount;
	return S_OK;
}

STDMETHODIMP CElementArray::Add( BSTR Element)
{
	if( m_UsedCount == m_AllocCount )
	{

		long NewAllocCount = m_AllocCount+5;
		
		CComBSTR* NewArray = new CComBSTR[NewAllocCount];
		if( NewArray == NULL ) return E_OUTOFMEMORY;

		for(long i = 0; i < m_UsedCount; ++i)
		{
			NewArray[i] = m_Array[i];
		}

		delete[] m_Array;

		m_Array = NewArray;
		m_AllocCount = NewAllocCount;

	}

	m_Array[m_UsedCount++] = Element;

	return S_OK;
}

STDMETHODIMP CElementArray::Remove(long Index)
{
	if( Index >= m_UsedCount ) 
		return Error("Invalid index.");

	while( Index < m_UsedCount-1 )
	{
		m_Array[Index] = m_Array[Index+1];
		++Index;
	}

	m_Array[Index].Empty();

	--m_UsedCount;

	return S_OK;
}

STDMETHODIMP CElementArray::RemoveAll()
{
	for( long i = 0; i < m_UsedCount; ++i )
	{
		m_Array[i].Empty();
	}

	m_UsedCount = 0;

	return S_OK;
}

STDMETHODIMP CElementArray::get_Item( long Index,  BSTR *pVal)
{
	if (pVal == NULL) return E_POINTER;

	if( Index >= m_UsedCount )
		return Error("Invalid index.");

	*pVal = m_Array[Index].Copy();

	return S_OK;
}
