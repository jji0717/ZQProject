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
// Name  : ElementArray.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-1-12
// Desc  : this class is a container for ElementType objects (asset elements)
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/PlaylistAS/PLAYLISTSOAPUDTMAPPER/ElementArray.h $
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
// ElementArray.h : Declaration of the CElementArray

#ifndef __ELEMENTARRAY_H_
#define __ELEMENTARRAY_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CElementArray
class ATL_NO_VTABLE CElementArray : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CElementArray, &CLSID_ElementArray>,
	public ISupportErrorInfo,
	public IDispatchImpl<IElementArray, &IID_IElementArray, &LIBID_PLAYLISTSOAPUDTMAPPERLib>
{
public:
	CElementArray():
		m_Array(new CComBSTR[10]),
		m_AllocCount(10),
		m_UsedCount(0)
	{
	}

	virtual ~CElementArray()
	{
		delete[] m_Array;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_ELEMENTARRAY)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CElementArray)
	COM_INTERFACE_ENTRY(IElementArray)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IElementArray
public:
	STDMETHOD(get_Item)(/*[in]*/ long Index, /*[out, retval]*/ BSTR *pVal);
	STDMETHOD(RemoveAll)();
	STDMETHOD(Remove)(/*[in]*/ long Index);
	STDMETHOD(Add)(/*[in]*/ BSTR Element);
	STDMETHOD(get_Count)(/*[out, retval]*/ long *pVal);

private:
	CComBSTR* m_Array;
	long m_AllocCount;
	long m_UsedCount;
};

#endif //__ELEMENTARRAY_H_
