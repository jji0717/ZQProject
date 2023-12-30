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
// $Log: /ZQProjs/Telewest/MODPlugin/MODAuthorization/ModSoapModel.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 10    05-06-02 17:29 Ken.qian
// 
// 9     05-06-02 13:49 Ken.qian
// 
// 8     05-04-29 13:28 Ken.qian
// 
// 7     05-04-11 14:41 Ken.qian
// 
// 6     05-03-02 18:39 Ken.qian
// 
// 5     05-02-24 17:42 Ken.qian
// 
// 4     05-01-31 18:51 Ken.qian
// 
// 3     05-01-31 13:58 Ken.qian
// 
// 2     05-01-14 21:19 Ken.qian
// 
// 1     04-12-13 17:43 Ken.qian
// Revision 1.1  2004/12/13 Ken Qian
//   definition and implemention
#include "stdafx.h"
#include "ModSoapModel.h"

#include "ModAuthReporter.h"
extern CModAuthReporter* g_pAuthReporter;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModSoapModel::CModSoapModel(const wchar_t* wszProviderID, const wchar_t* wszProviderAssetID,
							 const wchar_t* wszMacAddress, DWORD dwTicketID, 
							 STREAMID& sid, time_t dwPurchaseTime, DWORD dwErrCode)
{
	wcscpy(m_wszProviderID, wszProviderID);
	wcscpy(m_wszProviderAssetID, wszProviderAssetID);
	wcscpy(m_wszDeviceIDAddress, wszMacAddress);
	m_dwTicketID = dwTicketID;
	m_sid.dwStreamIdNumber = sid.dwStreamIdNumber;
	m_dwPurchaseTime = dwPurchaseTime;
	m_dwErrCode = dwErrCode;
}

CModSoapModel::CModSoapModel(const CModSoapModel& rhs)
{
	wcscpy(m_wszProviderID, rhs.m_wszProviderID);
	wcscpy(m_wszProviderAssetID, rhs.m_wszProviderAssetID);
	wcscpy(m_wszDeviceIDAddress, rhs.m_wszDeviceIDAddress);
	m_dwTicketID = rhs.m_dwTicketID;
	m_sid.dwStreamIdNumber = rhs.m_sid.dwStreamIdNumber;
	m_dwPurchaseTime = rhs.m_dwPurchaseTime;
	m_dwErrCode = rhs.m_dwErrCode;	
}

const CModSoapModel& CModSoapModel::operator=(const CModSoapModel& rhs)
{
	if (this == &rhs) return *this;
	
	*this = rhs;
		
	return *this;
}

CModSoapModel::~CModSoapModel()
{
	if(m_pModSoapModel != NULL)
	{
		m_pModSoapModel.Release();
		m_pModSoapModel = NULL;
	}
}

void CModSoapModel::GetModelInterface(IModSoapModelPtr pModelInterface)
{
	m_pModSoapModel = NULL;

	HRESULT hr;
	hr = m_pModSoapModel.CreateInstance(_uuidof(ModSoapModel));

	if( FAILED(hr) ) 
	{
		// log
		m_pModSoapModel = NULL;
		g_pAuthReporter->ReportLog(REPORT_CRITICAL, L"CreateInstance for ModSoapModel failed with hr = %x",
			hr);

		pModelInterface = NULL;
		return;
	}
	char chVal[256];

	// providerID
	m_pModSoapModel->put_ProviderID(_bstr_t(m_wszProviderID));
	
	// providerAssetID
	m_pModSoapModel->put_ProviderAssetID(_bstr_t(m_wszProviderAssetID));

	// mac
	m_pModSoapModel->put_DeviceID(_bstr_t(m_wszDeviceIDAddress));

	// ticketID
	ltoa(m_dwTicketID, chVal, 10);
	m_pModSoapModel->put_TicketID(_bstr_t(chVal));

	// streamID
	sprintf(chVal, "0X%08x", m_sid.dwStreamIdNumber);
	m_pModSoapModel->put_StreamID(_bstr_t(chVal));

	// purchaseTime
	wchar_t wchTime[256];
	struct tm* tmPurTime;
	tmPurTime = gmtime(&m_dwPurchaseTime);
	swprintf(wchTime, L"%4d-%02d-%02d %02d:%02d:%02d", 
		tmPurTime->tm_year+1900, tmPurTime->tm_mon+1, tmPurTime->tm_mday,
		tmPurTime->tm_hour, tmPurTime->tm_min, tmPurTime->tm_sec);
	m_pModSoapModel->put_PurchaseTime(_bstr_t(wchTime));

	// error code
	sprintf(chVal, "0X%08x", m_dwErrCode);
	m_pModSoapModel->put_ErrorCode(_bstr_t(chVal));

	if(m_dwPurchaseTime == 0)  // when invoking [process], MOD plug-in cannot get the purchaseTime.
		g_pAuthReporter->ReportLog(REPORT_DEBUG, L"CreateInstance for CModSoapModel with providerID = %s, providerAssetID = %s, Mac=%s, ticketID=%d, streamID=%d, errorCode=0X%08x",
									m_wszProviderID, m_wszProviderAssetID, m_wszDeviceIDAddress, m_dwTicketID, m_sid.dwStreamIdNumber, m_dwErrCode);
	else
		g_pAuthReporter->ReportLog(REPORT_DEBUG, L"CreateInstance for CModSoapModel with providerID = %s, providerAssetID = %s, Mac=%s, ticketID=%d, streamID=%d, purchaseTime=%s, errorCode=0X%08x",
								    m_wszProviderID, m_wszProviderAssetID, m_wszDeviceIDAddress, m_dwTicketID, m_sid.dwStreamIdNumber, wchTime, m_dwErrCode);

	pModelInterface = m_pModSoapModel;
	
}

//////////////////////////////////////////////////////////////////////
// CModSoapResultModel Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModSoapResultModel::CModSoapResultModel()
{
}

CModSoapResultModel::~CModSoapResultModel()
{
}

