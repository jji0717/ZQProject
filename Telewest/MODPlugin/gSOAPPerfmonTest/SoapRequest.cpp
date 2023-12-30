#include "SoapRequest.h"
#include "../MODAuthorization/ModAuthReporter.h"
#include "../MODAuthorization/ModSoapWrapper.h"
#include "itv.h"

#include <time.h>



extern WCHAR g_wszSOAPWSDLFileName[MAX_PATH];

extern DWORD g_dwSOAPConnectTimeout;
extern DWORD g_dwSOAPRetryCount;

extern WCHAR g_wszAppSitesCfgFileName[MAX_PATH];

extern WCHAR g_wszProviderID[MAX_PATH];
extern WCHAR g_wszProviderAssetID[MAX_PATH];
extern WCHAR g_wszMacAddress[MAX_PATH];

extern DWORD g_dwTicketID;
extern DWORD g_dwConcurrentCount;
extern DWORD g_dwRequestCount;
extern DWORD g_dwRequestHoldTime;

extern long g_dwSUSOAPFailCount;
extern long g_dwSUAuthPassCount;
extern long g_dwSUAuthFailCount;

extern long g_dwTDSOAPFailCount;
extern long g_dwTDAuthPassCount;
extern long g_dwTDAuthFailCount;

extern DWORD g_dwYield;

extern CModAuthReporter* g_pAuthReporter;

SOAPRequest::SOAPRequest(ZQ::common::NativeThreadPool& Pool, DWORD myTicket, int nReqNumber)
: ZQ::common::ThreadRequest(Pool), m_dwMyTicketId(myTicket), m_nNumber(nReqNumber)
{
}

SOAPRequest::~SOAPRequest()
{
	delete this;
}
	
bool SOAPRequest::init(void)
{
	return true;
}

int SOAPRequest::run(void)
{
	if(g_dwYield > 0)
	{
		Sleep(1);
	}
	
	STREAMID Sid = {0};
	CModSoapWrapper	pSoapWrapper(g_dwSOAPConnectTimeout, g_wszSOAPWSDLFileName, L"", 
			                    g_wszProviderID, g_wszProviderAssetID, g_wszMacAddress, m_dwMyTicketId, Sid, time(NULL));
	BOOL pbResponse = false;
	double fComputedPrice = 0.0f;
	DWORD dwRentalTime = 0;

	g_pAuthReporter->ReportLog(REPORT_DEBUG, L"No.%d SetupSOAPCommunicate() start", m_nNumber);
	
	DWORD dwStart = GetTickCount();
	BOOL bRet = pSoapWrapper.SetupSOAPCommunicate(pbResponse, fComputedPrice, dwRentalTime);

	m_dwSetupTime = GetTickCount() - dwStart;

	if(bRet)
	{
		if(pbResponse)
		{
			m_setupStatus = AUTH_PASS;
			printf("No.%d SetupSOAPCommunicate() succeed, and auth passed, spend %d ms\n", m_nNumber, m_dwSetupTime);
		}
		else
		{
			m_setupStatus = AUTH_FAIL;
			printf("No.%d SetupSOAPCommunicate() succeed, and auth failed, spend %d ms\n", m_nNumber, m_dwSetupTime);
		}	
	}
	else
	{
		m_setupStatus = SOAP_FAIL;
		printf("No.%d SetupSOAPCommunicate() failed with SOAP issue, spend %d ms\n", m_nNumber, m_dwSetupTime);
	}
	

	g_pAuthReporter->ReportLog(REPORT_DEBUG, L"No.%d SetupSOAPCommunicate() end with %d ms", m_nNumber, m_dwSetupTime);

	if(g_dwRequestHoldTime > 0)
	{
		Sleep(g_dwRequestHoldTime);
	}

	g_pAuthReporter->ReportLog(REPORT_DEBUG, L"No.%d TearDownSOAPCommunicate() start", m_nNumber);

	dwStart = GetTickCount();
	bRet = pSoapWrapper.TearDownSOAPCommunicate(pbResponse);

	m_dwTeardownTime = GetTickCount() - dwStart; 

	if(bRet)
	{
		if(pbResponse)
		{
			m_teardownStatus = AUTH_PASS;
			printf("No.%d TearDownSOAPCommunicate() succeed, and auth passed, spend %d ms\n", m_nNumber, m_dwTeardownTime);
		}
		else
		{
			m_teardownStatus = AUTH_FAIL;
			printf("No.%d TearDownSOAPCommunicate() succeed, and auth failed, spend %d ms\n", m_nNumber, m_dwTeardownTime);
		}	
	}
	else
	{
		m_teardownStatus = SOAP_FAIL;
		printf("No.%d TearDownSOAPCommunicate() failed with SOAP issue, spend %d ms\n", m_nNumber, m_dwTeardownTime);
	}


	g_pAuthReporter->ReportLog(REPORT_DEBUG, L"No.%d TearDownSOAPCommunicate() end with %d ms", m_nNumber, m_dwTeardownTime);

	return 0;
}

void SOAPRequest::final(int retcode, bool bCancelled)
{
}
