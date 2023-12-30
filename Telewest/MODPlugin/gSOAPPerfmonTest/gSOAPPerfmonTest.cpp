// gSOAPPerfmonTest.cpp : Defines the entry point for the console application.
//
// Globals

#include "StdAfx.h"
#include <stdio.h>
#include <map>

#include "../MODAuthorization/ModAuthReporter.h"
#include "SoapRequest.h"

#include "MoDSoapInterfaceMoDSoapInterfaceSoapBindingProxy.h"
#include "ini.h"

//#define SOAP_DEBUG

#define CFG_SECTION_NAME  "CONFIG"

WCHAR g_wszLogFileName[MAX_PATH] = {0};

DWORD g_dwLogFileSize = 64*1024*1024;
DWORD g_dwLogFileTrace = 1000;

// the full path of soap wsdl file
WCHAR g_wszSOAPWSDLFileName[MAX_PATH] = {0};

DWORD g_dwSOAPConnectTimeout = 5000;          // millisecond
DWORD g_dwSOAPRetryCount=1;			          // retry times for soap invoking

WCHAR g_wszAppSitesCfgFileName[MAX_PATH] = {0};     // not used here

WCHAR g_wszProviderID[MAX_PATH] = {0};
WCHAR g_wszProviderAssetID[MAX_PATH] = {0};
WCHAR g_wszMacAddress[MAX_PATH] = {0};

DWORD g_dwTicketID = 0;
DWORD g_dwTicketCount = 1;
DWORD g_dwConcurrentCount = 100;
DWORD g_dwRequestCount = 100;
DWORD g_dwRequestHoldTime = 1000;

long g_dwSUSOAPFailCount = 0;
long g_dwSUAuthPassCount = 0;
long g_dwSUAuthFailCount = 0;

long g_dwTDSOAPFailCount = 0;
long g_dwTDAuthPassCount = 0;
long g_dwTDAuthFailCount = 0;

DWORD g_dwYield = 0;

CModAuthReporter* g_pAuthReporter;

//struct Namespace namespaces[] = 
//{
//	{"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
//	{"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
//	{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
//	{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
//	{"ns2", "http://model.ote.izq.com", NULL, NULL},
//	{"ns3", "http://service.ote.izq.com", NULL, NULL},
//	{NULL, NULL, NULL, NULL}
//};

typedef std::map<int, SOAPRequest*> SOAPRequestMap;


int main(int argc, char* argv[])
{
	char configFile[MAX_PATH];
	if(argc == 2)
	{
		strcpy(configFile, argv[1]);
	}
	else
	{
		GetModuleFileNameA(NULL, configFile, MAX_PATH-1);
		for(int i=strlen(configFile)-1; i>=0; i--)
		{
			if(configFile[i] == '\\')
			{
				configFile[i+1]='\0';
				break;
			}
		}
		strcat(configFile, "gSoapPerfmonCfg.txt");
	}

	HANDLE hFile = CreateFileA(configFile, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		printf("Config file %s does not exist\n", configFile);
		
		return 0;
	}
	CloseHandle(hFile);

	IniFile cfgFile(configFile);

	std::string value;

	// get Connection Timeout
	value = cfgFile.ReadKey(CFG_SECTION_NAME, "ConnTimeout");
	if(value != "")
	{
		g_dwSOAPConnectTimeout = atoi(value.c_str());
	}
	printf("ConnTimeout\t = %d\n", g_dwSOAPConnectTimeout);
	
	// get WSDL
	value = cfgFile.ReadKey(CFG_SECTION_NAME, "WSDL");
	if(value != "")
	{
		mbstowcs(g_wszSOAPWSDLFileName, value.c_str(), MAX_PATH-1);
	}
	else
	{
		printf("WSDL was not specified in Config file: %s\n", configFile);
		return 0;
	}
	wprintf(L"WSDL\t\t = %s\n", g_wszSOAPWSDLFileName);

	// get ProviderID
	value = cfgFile.ReadKey(CFG_SECTION_NAME, "ProviderID");
	if(value != "")
	{
		mbstowcs(g_wszProviderID, value.c_str(), MAX_PATH-1);
	}
	else
	{
		printf("ProviderID was not specified in Config file: %s\n", configFile);
		return 0;
	}
	wprintf(L"ProviderID\t = %s\n", g_wszProviderID);

	// get ProviderAssetID
	value = cfgFile.ReadKey(CFG_SECTION_NAME, "ProviderAssetID");
	if(value != "")
	{
		mbstowcs(g_wszProviderAssetID, value.c_str(), MAX_PATH-1);
	}
	else
	{
		printf("ProviderAssetID was not specified in Config file: %s\n", configFile);
		return 0;
	}
	wprintf(L"ProviderAssetID\t = %s\n", g_wszProviderAssetID);

	// get MacAddress
	value = cfgFile.ReadKey(CFG_SECTION_NAME, "MacAddress");
	if(value != "")
	{
		mbstowcs(g_wszMacAddress, value.c_str(), MAX_PATH-1);
	}
	else
	{
		printf("MacAddress was not specified in Config file: %s\n", configFile);
		return 0;
	}
	wprintf(L"MacAddress\t = %s\n", g_wszMacAddress);

	// get Ticket count
	value = cfgFile.ReadKey(CFG_SECTION_NAME, "TicketCount");
	if(value != "")
	{
		g_dwTicketCount = atoi(value.c_str());
		if(g_dwTicketCount < 1)
		{
			g_dwTicketCount = 1;
		}
	}
	printf("TicketCount\t = %d\n", g_dwTicketCount);
	
	// get TicketID
	value = cfgFile.ReadKey(CFG_SECTION_NAME, "TicketID");
	if(value != "")
	{
		g_dwTicketID = atoi(value.c_str());
	}
	else
	{
		printf("TicketID was not specified in Config file: %s\n", configFile);
		return 0;
	}
	printf("TicketID\t = %d\n", g_dwTicketID);

	// get ConcurrentCount
	value = cfgFile.ReadKey(CFG_SECTION_NAME, "ConcurrentCount");
	if(value != "")
	{
		g_dwConcurrentCount = atoi(value.c_str());
	}
	else
	{
		printf("ConcurrentCount was not specified in Config file: %s\n", configFile);
		return 0;
	}
	printf("ConcurrentCount\t = %d\n", g_dwConcurrentCount);
	
	// get RequestCount
	value = cfgFile.ReadKey(CFG_SECTION_NAME, "RequestCount");
	if(value != "")
	{
		g_dwRequestCount = atoi(value.c_str());
	}
	else
	{
		printf("RequestCount was not specified in Config file: %s\n", configFile);
		return 0;
	}
	printf("RequestCount\t = %d\n", g_dwRequestCount);

	// get RequestHoldTime
	value = cfgFile.ReadKey(CFG_SECTION_NAME, "RequestHoldTime");
	if(value != "")
	{
		g_dwRequestHoldTime = atoi(value.c_str());
	}
	else
	{
		printf("RequestHoldTime was not specified in Config file: %s\n", configFile);
		return 0;
	}
	printf("RequestHoldTime\t = %d\n", g_dwRequestHoldTime);

	
	// get RequestHoldTime
	value = cfgFile.ReadKey(CFG_SECTION_NAME, "Yield");
	if(value != "")
	{
		g_dwYield = atoi(value.c_str());
	}
	printf("Yield\t = %d\n", g_dwYield);
			
	printf("\n\n");

	swprintf(g_wszLogFileName, L"gSoapPerfmon.log");

    swprintf(g_wszAppSitesCfgFileName, L"");
	
	g_pAuthReporter= new CModAuthReporter();
   
	if (g_dwLogFileSize)
        g_pAuthReporter->SetSvcLogMaxFileSize( g_dwLogFileSize );

    g_pAuthReporter->SetLogLevel( ALL_LOGS, g_dwLogFileTrace ); // set the log level
    RPTSTATUS rptRet = g_pAuthReporter->Init( g_wszLogFileName );
    
	rptRet = g_pAuthReporter->Register(L"gSOAPPerfmonTest", L"gSOAPPerfmonTest");

	g_pAuthReporter->ReportLog(REPORT_DEBUG, L"********** SOAP Performance Testing began, ConcurrentCount=%d, RequestCount=%d, HoldTime=%d ms *********", 
		                       g_dwConcurrentCount, g_dwRequestCount, g_dwRequestHoldTime);

	SOAPRequestMap reqests;

	ZQ::common::NativeThreadPool reqPool(g_dwConcurrentCount);

	int index = 0;
	for(int i=0; i<g_dwRequestCount; i++)
	{
		DWORD myticket = 0;
		if(index < g_dwTicketCount)
		{
			myticket = g_dwTicketID + index;
			index ++;
		}
		else
		{
			index = 0;
			myticket = g_dwTicketID + index;
			index ++;
		}
		
		SOAPRequest* req = new SOAPRequest(reqPool, myticket, i+1);
		reqests.insert(SOAPRequestMap::value_type(i+1, req));

		printf("NO.%d SOAP request was put to queue, TicketID = %d\n", i+1, myticket);

		req->start();
		
		Sleep(1);
	}
	
	bool bStop = true;
	char input[10];
	
	while(bStop)
	{
		int activeCount = reqPool.activeCount();
		if(activeCount > 0)
		{
			printf("There are still %d requests not completed\n", activeCount);
			Sleep(1000);
		}
		else
		{
			DWORD setupSoapFailCount = 0;
			DWORD setupAuthPassCount = 0;
			DWORD setupAuthFailCount = 0;

			DWORD teardownSoapFailCount = 0;
			DWORD teardownAuthPassCount = 0;
			DWORD teardownAuthFailCount = 0;

			DWORD setupAuthTotalTime = 0;
			DWORD teardownAuthTotalTime = 0;

			printf("\n\n");
			
			printf("ReqID    Setup(ms)     status    Teardown(ms)    status\n");
			printf("-----    ---------     ------    ------------    ------\n");
			SOAPRequestMap::iterator it = reqests.begin();
			for(; it != reqests.end(); it++)
			{
				int reqNo = (int)it->first;
				SOAPRequest* req = (SOAPRequest*)it->second;
				
				SOAPRequest::SOAPSTATUS setupstat = req->getSetupStatus();
				SOAPRequest::SOAPSTATUS teardownstat = req->getSetupStatus();

				switch(setupstat)
				{
				case SOAPRequest::SOAP_FAIL:
					setupSoapFailCount ++;
					break;
				case SOAPRequest::AUTH_PASS:
					setupAuthPassCount ++;
					setupAuthTotalTime += req->getSetupTimeConsumption();
					break;
				case SOAPRequest::AUTH_FAIL:
					setupAuthFailCount ++;
					setupAuthTotalTime += req->getSetupTimeConsumption();
					break;
				}

				switch(teardownstat)
				{
				case SOAPRequest::SOAP_FAIL:
					teardownSoapFailCount ++;
					break;
				case SOAPRequest::AUTH_PASS:
					teardownAuthPassCount ++;
					teardownAuthTotalTime += req->getTeardownTimeConsumption();
					break;
				case SOAPRequest::AUTH_FAIL:
					teardownAuthFailCount ++;
					teardownAuthTotalTime += req->getTeardownTimeConsumption();
					break;
				}
				printf("%d\t %d\t      %s\t %d\t\t%s\n", 
						reqNo, req->getSetupTimeConsumption(), SOAP_STATUS_TXT[req->getSetupStatus()],
						       req->getTeardownTimeConsumption(), SOAP_STATUS_TXT[req->getTeardownStatus()]);
			}

			printf("\n-------------------------------------------------------------------------------\n");
			printf("Total SetupRequest   =%d, SOAPFail=%d, AuthPass=%d, AuthFail=%d, AuthAvgTime=%d ms\n", 
				g_dwRequestCount, setupSoapFailCount, setupAuthPassCount, setupAuthFailCount, 
				setupAuthTotalTime/(setupAuthPassCount+setupAuthFailCount));
			
			printf("Total TearDwonRequest=%d, SOAPFail=%d, AuthPass=%d, AuthFail=%d, AuthAvgTime=%d ms\n", 
				g_dwRequestCount, teardownSoapFailCount, teardownAuthPassCount, teardownAuthFailCount, 
				teardownAuthTotalTime/(teardownAuthPassCount+teardownAuthFailCount));
			printf("--------------------------------------------------------------------------------\n");			


			break;
		}

//		printf("\n-----------------------------------------\n");
//		printf("To exit from the processing, input 'quit'\n");
//		printf("-----------------------------------------\n\n");
//		fgets(input, 10, stdin);
//		input[strlen(input)-1] = '\0';
//		if(strcmp(input, "quit") == 0)
//		{
//			if(reqPool.activeCount() == 0)
//			{
//				printf("\ngSoapPerfmon Test Completed\n");
//
//
//				bStop = false;
//			}
//			else
//			{
//				printf("\ngSoapPerfmon Test NOT Complete yet, do you really want to quit?\n");
//				char yn = getchar();
//				if(yn == 'Y' || yn == 'y')
//				{
//					bStop = false;
//				}
//			}
//		}
	}

	delete g_pAuthReporter;
	
	return 0;
}

