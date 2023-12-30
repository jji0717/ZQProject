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
// Ident : $Id: ModSoapWrapper.cpp,v 1.1 2004/12/10 Ken Qian $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : Wrap the SOAPClient and OTE SOAP Inteface routines.
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/MODPlugin/MODAuthorization/ModSoapWrapper.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 8     05-07-04 17:35 Ken.qian
// 
// 7     05-06-23 16:11 Ken.qian
// 
// 6     05-06-20 21:24 Ken.qian
// 
// 5     05-06-02 13:49 Ken.qian
// 
// 4     05-02-25 19:42 Ken.qian
// 
// 3     05-01-14 21:19 Ken.qian
// 
// 1     04-12-13 17:43 Ken.qian
// Revision 1.1  2004/12/10 Ken Qian
//   definition and implemention

#if !defined(AFX_MODSOAPWRAPPER_H__C6B99230_11CE_4012_9C32_480C34E52105__INCLUDED_)
#define AFX_MODSOAPWRAPPER_H__C6B99230_11CE_4012_9C32_480C34E52105__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "itv.h"
#include "MoDSoapInterfaceMoDSoapInterfaceSoapBindingProxy.h"

#define MODEL_MAX_LEGTH 256

class CModSoapWrapper  
{
public:
	CModSoapWrapper(DWORD dwSoapConnTimeout, const wchar_t* pszWSDLFileName, const wchar_t* pszWSMLFileName, 
							 const wchar_t* wszProviderID, const wchar_t* wszProviderAssetID, const wchar_t* wszMacAddress, 
							 DWORD dwTicketID, STREAMID& sid, time_t dwPurchaseTime, DWORD dwErrCode=0);
	virtual ~CModSoapWrapper();

public:

	/// wrapper for SOAP SetupSOAPCommunicate Interface routine
	BOOL SetupSOAPCommunicate(BOOL& bResult, double& fPrice, DWORD& dwRentalTime);
	/// wrapper for SOAP TearDownSOAPCommunicate Interface routine
	BOOL TearDownSOAPCommunicate(BOOL& bResult);
private:
	void ErrorCodeToString(long lErrorCode, wchar_t* wszErrorDesciption);

	void LogSoapErrorMsg(const MoDSoapInterfaceSoapBinding &modSoap);

	DWORD m_dwSoapConnTimeout; // millisecond
	wchar_t m_pszWSDLFileName[MODEL_MAX_LEGTH];
	wchar_t m_pszWSMLFileName[MODEL_MAX_LEGTH];

	char  m_chProviderID[MODEL_MAX_LEGTH];
	char  m_chProviderAssetID[MODEL_MAX_LEGTH];
	char  m_chDeviceID[MODEL_MAX_LEGTH];
	char  m_chTicketID[MODEL_MAX_LEGTH];
	char  m_chSid[MODEL_MAX_LEGTH];
	char  m_chPurchaseTime[MODEL_MAX_LEGTH];
	char  m_chErrCode[MODEL_MAX_LEGTH];
};


#endif // !defined(AFX_MODSOAPWRAPPER_H__C6B99230_11CE_4012_9C32_480C34E52105__INCLUDED_)
