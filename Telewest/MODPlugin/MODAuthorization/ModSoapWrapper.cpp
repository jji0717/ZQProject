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
// $Log: /ZQProjs/Telewest/MODPlugin/MODAuthorization/ModSoapWrapper.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 14    08-01-29 17:04 Ken.qian
// Add socket error code in log
// 
// 13    07-01-26 11:21 Ken.qian
// 
// 12    06-03-16 14:57 Ken.qian
// 
// 11    05-11-04 18:58 Ken.qian
// 
// 10    05-07-18 16:11 Ken.qian
// For supporting more than one application id
// 
// 9     05-07-11 17:58 Ken.qian
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

#include "stdafx.h"
#include <stdio.h>
#include "ModSoapWrapper.h"

#include "itv.h"

#include "ModAuthReporter.h"

extern CModAuthReporter* g_pAuthReporter;

// Errorcode definition
#define OTE_RET_SUCCESS							 1
#define OTE_RET_UNEXPECTED						 0
#define OTE_RET_PARAMETER_MISSING				-1
#define OTE_RET_CONNECT_TO_DB_FAILED			-2
#define OTE_RET_TICKET_NOT_FOUND				-3
#define OTE_RET_NOT_A_PREVIEW					-4
#define OTE_RET_REPORT_TO_MEDIATION_FAILED		-5
#define OTE_RET_ASSET_UNBELONGING_TO_TICKET		-6
#define OTE_RET_UPDATE_DB_FAILED				-7
#define OTE_RET_TICKET_EXPIRED					-8
#define OTE_RET_ASSET_NOT_FOUND					-9
#define OTE_RET_TICKET_NOT_EFFECTIVE            -10

struct Namespace namespaces[] = 
{
	{"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
	{"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
	{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
	{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
	{"ns2", "http://model.ote.izq.com", NULL, NULL},
	{"ns3", "http://service.ote.izq.com", NULL, NULL},
	{NULL, NULL, NULL, NULL}
};
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CModSoapWrapper::CModSoapWrapper(DWORD dwSoapConnTimeout, const wchar_t* pszWSDLFileName, const wchar_t* pszWSMLFileName, 
							 const wchar_t* wszProviderID, const wchar_t* wszProviderAssetID, const wchar_t* wszMacAddress, 
							 DWORD dwTicketID, STREAMID& sid, time_t dwPurchaseTime, DWORD dwErrCode)
{
	if(dwSoapConnTimeout == 0)
		dwSoapConnTimeout = 30;
	m_dwSoapConnTimeout = dwSoapConnTimeout / 1000;

	wcscpy(m_pszWSDLFileName, pszWSDLFileName);
	wcscpy(m_pszWSMLFileName, pszWSMLFileName);

	// wcstombs(m_chProviderID, wszProviderID, MODEL_MAX_LEGTH);
	WideCharToMultiByte(CP_UTF8, 0, wszProviderID, -1, m_chProviderID, sizeof(m_chProviderID), 0, 0);
	// wcstombs(m_chProviderAssetID, wszProviderAssetID, MODEL_MAX_LEGTH);	
	WideCharToMultiByte(CP_UTF8, 0, wszProviderAssetID, -1, m_chProviderAssetID, sizeof(m_chProviderAssetID), 0, 0);

	// wcstombs(m_chDeviceID, wszMacAddress, MODEL_MAX_LEGTH);
	WideCharToMultiByte(CP_UTF8, 0, wszMacAddress, -1, m_chDeviceID, sizeof(m_chDeviceID), 0, 0);

	ltoa(dwTicketID, m_chTicketID, 10);
	sprintf(m_chSid, "0X%08x", sid.dwStreamIdNumber);
	sprintf(m_chErrCode, "0X%08x", dwErrCode);

	struct tm* tmPurTime;
	tmPurTime = gmtime(&dwPurchaseTime);
	sprintf(m_chPurchaseTime, "%4d-%02d-%02d %02d:%02d:%02d", 
		tmPurTime->tm_year+1900, tmPurTime->tm_mon+1, tmPurTime->tm_mday,
		tmPurTime->tm_hour, tmPurTime->tm_min, tmPurTime->tm_sec);

	wchar_t wchTime[MODEL_MAX_LEGTH];
	mbstowcs(wchTime, m_chPurchaseTime, MODEL_MAX_LEGTH);

	if(dwPurchaseTime == 0)  // when invoking [process], MOD plug-in cannot get the purchaseTime.
		g_pAuthReporter->ReportLog(REPORT_DEBUG, L"providerID = %s, providerAssetID = %s, Mac=%s, ticketID=%d, streamID=%d, errorCode=0X%08x",
									wszProviderID, wszProviderAssetID, wszMacAddress, dwTicketID, sid.dwStreamIdNumber, dwErrCode);
	else
		g_pAuthReporter->ReportLog(REPORT_DEBUG, L"providerID = %s, providerAssetID = %s, Mac=%s, ticketID=%d, streamID=%d, purchaseTime=%s, errorCode=0X%08x",
								    wszProviderID, wszProviderAssetID, wszMacAddress, dwTicketID, sid.dwStreamIdNumber, wchTime, dwErrCode);
}

CModSoapWrapper::~CModSoapWrapper()
{
}

BOOL CModSoapWrapper::SetupSOAPCommunicate(BOOL& bResult, double& fPrice, DWORD& dwRentalTime)
{
	bResult = FALSE;

	g_pAuthReporter->ReportLog(REPORT_DEBUG, L"Entering CModSoapWrapper::SetupSOAPCommunicate()");

	// prepare for the parameters.
	std::string strProviderID = m_chProviderID;
	std::string strProviderAssetID = m_chProviderAssetID;
	std::string strDevID = m_chDeviceID;
	std::string strTicketID = m_chTicketID;
	std::string strStreamID = m_chSid;
	std::string strPurchaseTime = m_chPurchaseTime;
	std::string strErrorCode = m_chErrCode;

	ns2__MoDSoapModel model;

	model.deviceID = &strDevID;
	model.providerAssetID = &strProviderAssetID;
	model.errorCode = &strErrorCode;
	model.providerID = &strProviderID;
	model.purchaseTime = &strPurchaseTime;
	model.streamID = &strStreamID;
	model.ticketID = &strTicketID;

	char chWSDLFileName[MODEL_MAX_LEGTH];

	wcstombs(chWSDLFileName, m_pszWSDLFileName, MODEL_MAX_LEGTH);

	ns3__setupSOAPCommunicateResponse result;

	MoDSoapInterfaceSoapBinding modSoap;

	// set soap property
	modSoap.endpoint = chWSDLFileName;
	modSoap.soap->connect_timeout = m_dwSoapConnTimeout;
	modSoap.soap->send_timeout = m_dwSoapConnTimeout;
	modSoap.soap->recv_timeout = m_dwSoapConnTimeout;

	// call soap interface
	if (modSoap.ns3__setupSOAPCommunicate(&model, result) == 0) 
	{
		long lResult = result._setupSOAPCommunicateReturn->result;
		if(1 == lResult)
		{
			fPrice = result._setupSOAPCommunicateReturn->price;
			dwRentalTime = result._setupSOAPCommunicateReturn->rentalDuration;  // in minute

			g_pAuthReporter->ReportLog(REPORT_DEBUG, L"SetupSOAPCommunicate succeed, Asset's Computed Price=%f, RentalTime=%d minutes",
										fPrice, dwRentalTime);
			g_pAuthReporter->ReportLog(REPORT_DEBUG, L"SetupSOAPCommunicate succeed, and the request for mod was allowed");

			bResult = TRUE;
		}
		else
		{
			wchar_t wszErrorString[256];

			ErrorCodeToString(lResult, wszErrorString);

			g_pAuthReporter->ReportLog(REPORT_DEBUG, L"SetupSOAPCommunicate succeed, and the request for mod was not allowed. Reason:(%d) %s", lResult, wszErrorString);

			bResult = FALSE;
		}
		g_pAuthReporter->ReportLog(REPORT_DEBUG, L"Leaving CModSoapWrapper::SetupSOAPCommunicate()");
		return TRUE;
	}
	else
	{ 
		g_pAuthReporter->ReportLog(REPORT_CRITICAL, L"SetupSOAPCommunicate() failed");
		
		LogSoapErrorMsg(modSoap);

		g_pAuthReporter->ReportLog(REPORT_DEBUG, L"Leaving CModSoapWrapper::SetupSOAPCommunicate()");
		bResult = FALSE;
		return FALSE;
	}
}

BOOL CModSoapWrapper::TearDownSOAPCommunicate(BOOL& bResult)
{
	bResult = FALSE;

	g_pAuthReporter->ReportLog(REPORT_DEBUG, L"Entering CModSoapWrapper::TearDownSOAPCommunicate()");

	// prepare for the parameters.
	std::string strProviderID = m_chProviderID;
	std::string strProviderAssetID = m_chProviderAssetID;
	std::string strDevID = m_chDeviceID;
	std::string strTicketID = m_chTicketID;
	std::string strStreamID = m_chSid;
	std::string strPurchaseTime = m_chPurchaseTime;
	std::string strErrorCode = m_chErrCode;

	ns2__MoDSoapModel model;

	model.deviceID = &strDevID;
	model.providerAssetID = &strProviderAssetID;
	model.errorCode = &strErrorCode;
	model.providerID = &strProviderID;
	model.purchaseTime = &strPurchaseTime;
	model.streamID = &strStreamID;
	model.ticketID = &strTicketID;

	char chWSDLFileName[MODEL_MAX_LEGTH];

	wcstombs(chWSDLFileName, m_pszWSDLFileName, MODEL_MAX_LEGTH);

	ns3__tearDownSOAPCommunicateResponse result;

	MoDSoapInterfaceSoapBinding modSoap;

	modSoap.endpoint = chWSDLFileName;
	modSoap.soap->connect_timeout = m_dwSoapConnTimeout;
	modSoap.soap->send_timeout = m_dwSoapConnTimeout;
	modSoap.soap->recv_timeout = m_dwSoapConnTimeout;
	
	if (modSoap.ns3__tearDownSOAPCommunicate(&model, result) == 0) 
	{
		long lResult = result._tearDownSOAPCommunicateReturn->result;
		if(1 == lResult)
		{
			double fPrice = result._tearDownSOAPCommunicateReturn->price;
			DWORD dwRentalDuration = result._tearDownSOAPCommunicateReturn->rentalDuration;

			g_pAuthReporter->ReportLog(REPORT_DEBUG, L"TearDownSOAPCommunicate succeed, price=%f, rentalTime=%d minutes",
										fPrice, dwRentalDuration);
			g_pAuthReporter->ReportLog(REPORT_DEBUG, L"TearDownSOAPCommunicate succeed, and the request for mod was allowed");

			bResult = TRUE;
		}
		else
		{
			wchar_t wszErrorString[256];

			ErrorCodeToString(lResult, wszErrorString);

			g_pAuthReporter->ReportLog(REPORT_DEBUG, L"TearDownSOAPCommunicate succeed, and the request for mod was not allowed. Reason:(%d) %s", lResult, wszErrorString);

			bResult = FALSE;
		}
		g_pAuthReporter->ReportLog(REPORT_DEBUG, L"Leaving CModSoapWrapper::TearDownSOAPCommunicate()");
		return TRUE;
	}
	else
	{ 
		g_pAuthReporter->ReportLog(REPORT_CRITICAL, L"TearDownSOAPCommunicate() failed");

		LogSoapErrorMsg(modSoap);

		g_pAuthReporter->ReportLog(REPORT_DEBUG, L"Leaving CModSoapWrapper::TearDownSOAPCommunicate()");
		bResult = FALSE;
		return FALSE;
	}
}

void CModSoapWrapper::LogSoapErrorMsg(const MoDSoapInterfaceSoapBinding &modSoap)
{
	if(modSoap.soap->error)
	{ 
		// following codes from soap_print_fault(modSoap.soap, stderr);
		const char **s;
		if (!*soap_faultcode(modSoap.soap))
			soap_set_fault(modSoap.soap);
		char chFaultMsg[1024];
		wchar_t wszFaultMsg[512];

		sprintf(chFaultMsg, "SOAP FAULT: SocketErrNo=%d ErrorCode=%d  FaultCode=<%s> FaultString=<%s>", modSoap.soap->errnum, modSoap.soap->error, *soap_faultcode(modSoap.soap), *soap_faultstring(modSoap.soap));
		mbstowcs(wszFaultMsg, chFaultMsg, 1024);

		g_pAuthReporter->ReportLog(REPORT_CRITICAL, L"%s", wszFaultMsg);

		s = soap_faultdetail(modSoap.soap);
		if (s && *s)
		{
			sprintf(chFaultMsg, "Detail: %s", *s);

			mbstowcs(wszFaultMsg, chFaultMsg, 1024);

			g_pAuthReporter->ReportLog(REPORT_CRITICAL, L"%s", wszFaultMsg);
		}
		// soap_print_fault_location(modSoap.soap, stderr);
	}
}

void CModSoapWrapper::ErrorCodeToString(long lErrorCode, wchar_t* wszErrorDesciption)
{
	if (wszErrorDesciption == NULL)
		return;

	switch(lErrorCode)
	{
	case OTE_RET_SUCCESS:
		wcscpy(wszErrorDesciption, L"Succeed");
		break;
	case OTE_RET_UNEXPECTED:		
		wcscpy(wszErrorDesciption, L"Unexpected error");
		break;
	case OTE_RET_PARAMETER_MISSING:	
		wcscpy(wszErrorDesciption, L"Parameter missing");
		break;
	case OTE_RET_CONNECT_TO_DB_FAILED:		
		wcscpy(wszErrorDesciption, L"OTE failed to connect to DB");
		break;
	case OTE_RET_TICKET_NOT_FOUND:			
		wcscpy(wszErrorDesciption, L"TicketID not found in DB");
		break;
	case OTE_RET_NOT_A_PREVIEW:
		wcscpy(wszErrorDesciption, L"Requested asset is not for preview");
		break;
	case OTE_RET_REPORT_TO_MEDIATION_FAILED:
		wcscpy(wszErrorDesciption, L"OTE failed to report to medaition");
		break;
	case OTE_RET_ASSET_UNBELONGING_TO_TICKET:		
		wcscpy(wszErrorDesciption, L"Asset unbelong to ticket");
		break;
	case OTE_RET_UPDATE_DB_FAILED:
		wcscpy(wszErrorDesciption, L"Failed to update the DB");
		break;
	case OTE_RET_TICKET_EXPIRED:
		wcscpy(wszErrorDesciption, L"Failed for the ticket id has expired");
		break;
	case OTE_RET_ASSET_NOT_FOUND:
		wcscpy(wszErrorDesciption, L"Failed for the asset was not found");
		break;
	case OTE_RET_TICKET_NOT_EFFECTIVE:
		wcscpy(wszErrorDesciption, L"The ticket is not effective now");
		break;
	default:
		wcscpy(wszErrorDesciption, L"Unknow error");
		break;
	}
}

