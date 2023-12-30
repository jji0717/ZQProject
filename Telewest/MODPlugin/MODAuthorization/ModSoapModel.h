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
// Ident : $Id: ModSoapModel.cpp,v 1.1 2004/12/13 Ken Qian $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : Implement the creation of COM Interface for Complex types.
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/MODPlugin/MODAuthorization/ModSoapModel.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 5     05-06-02 13:49 Ken.qian
// 
// 4     05-04-29 13:29 Ken.qian
// 
// 3     05-03-02 18:39 Ken.qian
// 
// 2     05-01-14 21:19 Ken.qian
// 
// 1     04-12-13 17:43 Ken.qian
// Revision 1.1  2004/12/13 Ken Qian
//   definition and implemention

#if !defined(AFX_MODSOAPMODEL_H__29C1448D_366F_487B_8A88_3F4BA332ACC5__INCLUDED_)
#define AFX_MODSOAPMODEL_H__29C1448D_366F_487B_8A88_3F4BA332ACC5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "itv.h"

#define MODEL_MAX_LEGTH 256

class CModSoapModel
{
public:
	CModSoapModel() {};
	CModSoapModel(const wchar_t* wszProviderID, const wchar_t* wszProviderAssetID,
		        const wchar_t* wszDeviceIDAddress, DWORD dwTicketID, 
				STREAMID& sid, time_t dwPurchaseTime, DWORD dwErrCode=0);

	CModSoapModel(const CModSoapModel& rhs);

	const CModSoapModel& operator=(const CModSoapModel& model);

	virtual ~CModSoapModel();

public:
	void GetModelInterface(IModSoapModelPtr pModelInterface);

private:
	IModSoapModelPtr m_pModSoapModel;

	wchar_t  m_wszProviderID[MODEL_MAX_LEGTH];
	wchar_t  m_wszProviderAssetID[MODEL_MAX_LEGTH];
	wchar_t  m_wszDeviceIDAddress[MODEL_MAX_LEGTH];
	DWORD    m_dwTicketID;
	STREAMID m_sid;
	time_t   m_dwPurchaseTime;
	DWORD    m_dwErrCode;
};

class CModSoapResultModel  
{
public:
	CModSoapResultModel();
	virtual ~CModSoapResultModel();

public:
	void   SetPrice(double fPrice) { m_fPrice = fPrice; };
	double GetPrice() { return m_fPrice; };
	
	void  SetRentalDuration(DWORD dwRentalDuration) { m_dwRentalDuration = dwRentalDuration; };
	DWORD GetRentalDuration() { return m_dwRentalDuration; };

private:
	BOOL   m_bResult;
	double m_fPrice;
	DWORD  m_dwRentalDuration;
};
#endif // !defined(AFX_MODSOAPMODEL_H__29C1448D_366F_487B_8A88_3F4BA332ACC5__INCLUDED_)
