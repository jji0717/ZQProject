// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: CPECmd.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/service/CPEClt.cpp $
// 
// 3     12/31/13 7:18p Hui.shao
// pGlog
// 
// 2     12/12/13 1:48p Hui.shao
// %lld
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 13    08-05-30 15:59 Xia.chen
// 
// 12    08-05-25 19:15 Xia.chen
// 
// 11    08-05-25 18:33 Xia.chen
// 
// 10    08-05-25 16:07 Xia.chen
// 
// 8     08-05-14 16:01 Xia.chen
// 
// 7     08-05-13 17:47 Xia.chen
// 
// 6     08-05-07 14:42 Xia.chen
// 
// 5     08-05-04 17:21 Xia.chen
// 
// 4     08-04-29 14:35 Xia.chen
// 
// 3     08-04-25 14:34 Xia.chen
// 
// 2     08-04-17 11:08 Xia.chen
// 
// 1     08-03-17 20:00 Jie.zhang
// 
// ===========================================================================

#include "Log.h"
#include "getopt.h"
#include "ZQResource.h"
#include "FileLog.h"
#include "CPHInc.h"
#include "TianShanDefines.h"
#include "TsContentProv.h"

#define DEFAULT_ENDPOINT_CPE			"default -p 10010"
//#define DEFAULT_ENDPOINT_CPE			"tcp -h 10.15.10.250 -p 10010"
#define SERVICE_NAME_ContentProvisionService	"ContentProvision"



extern "C"
{
#include <time.h>
#include <stdio.h>
#include <direct.h>
}

BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;

void usage()
{
	printf("Usage: CPEClt [-e \"<endpoint>\"]\n");
	printf("       CPEClt -h\n");
	printf("CPE console mode server demo.\n");
	printf("options:\n");
	printf("\t-e   the local endpoint to bind, default %d\n", DEFAULT_ENDPOINT_CPE);
	printf("\t-b   the bandwidth for ingest\n");
	printf("\t-s   the source file to ingest\n");
	printf("\t-h   display this help\n");
}

const char* stateStr(const ::TianShanIce::ContentProvision::ProvisionState state)
{
#define SWITCH_CASE_STATE(_ST)	case ::TianShanIce::ContentProvision::cps##_ST: return #_ST
	switch(state)
	{
		SWITCH_CASE_STATE(Created);
		SWITCH_CASE_STATE(Accepted);
		SWITCH_CASE_STATE(Wait);
		SWITCH_CASE_STATE(Ready);
		SWITCH_CASE_STATE(Provisioning);
		//		SWITCH_CASE_STATE(ProvisioningStreamable);
		SWITCH_CASE_STATE(Stopped);
	default:
		return "<Unknown>";
	}
#undef SWITCH_CASE_STATE
}


// -----------------------------
// class ProvisionSessImpl
// -----------------------------
class ProvisionSessionBindImpl : public TianShanIce::ContentProvision::ProvisionSessionBind
{
public:
	typedef ::IceInternal::Handle< ProvisionSessionBindImpl> Ptr;

	ProvisionSessionBindImpl() {}

protected:
    virtual void OnProvisionStateChanged(const ::TianShanIce::ContentProvision::ProvisionContentKey& contentKey, ::Ice::Long timeStamp, ::TianShanIce::ContentProvision::ProvisionState prevState, ::TianShanIce::ContentProvision::ProvisionState currentState, const ::TianShanIce::Properties& params, const ::Ice::Current& c)
	{
		char buf[64];
		printf("\n%s {content:%s; storage:%s; volume:%s} prevState: %s(%d); currentState:%s(%d)", ZQTianShan::TimeToUTC(timeStamp, buf, sizeof(buf)-2),
			contentKey.content.c_str(), contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(),
			stateStr(prevState), prevState, stateStr(currentState), currentState);
	}

    virtual void OnProvisionProgress(const ::TianShanIce::ContentProvision::ProvisionContentKey& contentKey, ::Ice::Long timeStamp, ::Ice::Long processed, ::Ice::Long total, const ::TianShanIce::Properties&params, const ::Ice::Current& c)
	{
		char buf[64];
		printf("\n%s {content:%s; storage:%s; volume:%s} progress: %lld of %lld", ZQTianShan::TimeToUTC(timeStamp, buf, sizeof(buf)-2),
			contentKey.content.c_str(), contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(),
			processed, total);
	}

    virtual void OnProvisionStarted(const ::TianShanIce::ContentProvision::ProvisionContentKey&, ::Ice::Long, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current())
	{
	}

    virtual void OnProvisionStopped(const ::TianShanIce::ContentProvision::ProvisionContentKey&, ::Ice::Long, bool, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current())
	{
	}

    virtual void OnProvisionStreamable(const ::TianShanIce::ContentProvision::ProvisionContentKey& contentKey, ::Ice::Long timeStamp, bool streamable, const ::TianShanIce::Properties& params, const ::Ice::Current& c)
	{
	}

    virtual void OnProvisionDestroyed(const ::TianShanIce::ContentProvision::ProvisionContentKey& contentKey, ::Ice::Long timeStamp, const ::TianShanIce::Properties& params, const ::Ice::Current& c)
	{
	}
};

int main(int argc, char* argv[])
{
	int ch;
	std::string endpoint;
	int traceLevel = ZQ::common::Log::L_DEBUG;	
	char*	pLogPath=NULL;
	int nBandwidth = 3750000;
	int nDurationSecs = 30*60;	//half an hour
	std::string strSrcFile;
	char apptype[10]={0};
	std::string filename;
	std::string ip;
	std::string port;
	int interval;
	int count;
	int nStartDelayTime = 10;


	char path[MAX_PATH] = ".", *p=path;
	if (::GetModuleFileName(NULL, path, MAX_PATH-1)>0)
	{
		char* p = strrchr(path, FNSEPC);
		if (NULL !=p)
		{
			*p='\0';
			p = strrchr(path, FNSEPC);
			if (NULL !=p && (0==stricmp(FNSEPS "bin", p) || 0==stricmp(FNSEPS "exe", p)))
				*p='\0';
		}
	}
	strcat(path, FNSEPS);
	p = path+strlen(path);
	
	while((ch = getopt(argc, argv, "he:b:d:s:")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			usage();
			exit(0);
			
		case 'e':
			endpoint = optarg;
			break;
			
		case 'b':
			nBandwidth = atoi(optarg);
			break;
			
		case 'd':
			traceLevel = atoi(optarg);
			break;
		case 's':
			strSrcFile=optarg;
			break;
		default:
			printf("Error: unknown option %c specified\n", ch);
			exit(1);
		}
	}

	if (argc==1)
	{
		printf("Please input the type of client.\n");
		printf("Types include rds, rti, rtf,list,updateST,nascopy and fscp.\n");
	}
	else
	{
		printf("There should be one parameter.\n");
		return 1;
	}

	cin >> apptype;

	
	std::string serverip ;
	printf("Please input server ip.\n");
	cin >> serverip;
	endpoint = std::string("tcp -h ") + serverip + std::string(" -p 10010");

	int i =0;
	Ice::CommunicatorPtr ic = Ice::initialize(i, NULL);
    printf("Connect CPE at \"%s\"\n", endpoint.c_str());
	
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		printf("Unable to install handler!                      \n");
		return -1;
	}

	std::string strPushUrl;
	TianShanIce::ContentProvision::ContentProvisionServicePrx cpePrx;
	TianShanIce::ContentProvision::ProvisionSessionBindPrx bindPrx;
	try {
		ProvisionSessionBindImpl::Ptr bind = new ProvisionSessionBindImpl();
		::Ice::ObjectAdapterPtr testAdapter = ic->createObjectAdapterWithEndpoints("testAdapter", "default -p 11111");
		printf("successfully create adapter\n");
		Ice::Identity identBind;
		identBind.name = identBind.category = "testBind";
		testAdapter->add(bind, identBind);
		testAdapter->activate();
		bindPrx= ::TianShanIce::ContentProvision::ProvisionSessionBindPrx::checkedCast(testAdapter->createProxy(identBind));
		printf("successfully checkedCast\n");
		std::string strContentProvisionSvc = std::string(SERVICE_NAME_ContentProvisionService ":") + endpoint;
		printf("successfully begin connect\n");
		cpePrx = TianShanIce::ContentProvision::ContentProvisionServicePrx::checkedCast(ic->stringToProxy(strContentProvisionSvc));
		printf("successfully conect\n");
		//TianShanIce::ContentProvision::ProvisionSessionPrx sess;
		if (!cpePrx)
			return 1000;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		printf("caught exception[%s]: %s\n", ex.ice_name().c_str(), ex.message.c_str());
	    ic->destroy();
		return 0;
	}
	catch(const ::Ice::Exception& ex)
	{
		printf("caught exception[%s]\n", ex.ice_name().c_str());
		ic->destroy();
		return 0;
	}

	if(_stricmp(apptype,"updateST")==0)
	{
		std::string sessionName;
		std::string begintime;
		std::string endtime;
		std::string netId;

		printf("Input session name.\n");  
		cin >> sessionName;
		printf("Input contentstoreNetId.\n");  
		cin >> netId;
		printf("Input starttime.\n"); 
		cin >> begintime;
		printf("Input endtime.\n"); 
		cin >> endtime;

		TianShanIce::ContentProvision::ProvisionContentKey contentKey;
		contentKey.content =sessionName;
		contentKey.contentStoreNetId = netId;
		try
		{
			TianShanIce::ContentProvision::ProvisionSessionPrx sess= cpePrx->openSession(contentKey);
			if (!sess)
			{
				printf("can't find the session %s",sessionName.c_str());
				return -1;
			}
			sess->updateScheduledTime(begintime,endtime);
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			printf("caught exception[%s]: %s\n", ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			printf("caught exception[%s]\n", ex.ice_name().c_str());
		}
      
	}
	if(strcmp(apptype,"rti")==0)
	{
		printf("Input szOutputFilename.\n");  
		cin >> filename;
        printf("Input szMulticastUrl.\n"); 
		cin >> ip;
		printf("Input szSessionInterval.\n"); 
		cin >> interval;
		printf("Input szSessionNumber.\n"); 
		cin >> count;
		printf("Input szSessionDuration.\n"); 
		cin >> nDurationSecs;
		printf("Input szBandwidth.\n"); 
		cin >> nBandwidth;	

		::Ice::Long  lastendtime = 0; 
			
		if (count == 0)
		count = 1;


		char index[10];
		itoa(count, index, 10);
		int len = strlen(index);
		char cc[10] = "000000000"; 
		strncpy(index, cc,len);
		int number = 0;

		while (count)
		{	
			number++;
			::Ice::Long c;
			if (lastendtime == 0)
				c = ZQTianShan::now();
			else
				c = lastendtime;
			
			char buf[32];
			char indexlast[10];
			itoa(number, indexlast, 10);
			try
			{
				TianShanIce::ContentProvision::ProvisionSessionPrx sess;
				strcpy(buf, filename.c_str());
				strcat(buf, index);
				strcat(buf, indexlast);
				TianShanIce::ContentProvision::ProvisionContentKey contentKey;
				contentKey.content = buf;
				contentKey.contentStoreNetId = "rti";
				sess = cpePrx->createSession(contentKey, METHODTYPE_RTIVSVSTRM, TianShanIce::ContentProvision::potDirect, NULL, NULL);
				
				TianShanIce::ValueMap resVars;
				TianShanIce::Variant	var;
				var.type = TianShanIce::vtStrings;
				var.bRange = false;
				var.strs.clear();
				var.strs.push_back(contentKey.content);
				resVars[CPHPM_FILENAME] = var;
				var.strs.clear();
				var.strs.push_back(ip);
				resVars[CPHPM_SOURCEURL] = var;
				var.strs.clear();
				sess->addResource(::TianShanIce::SRM::rtURI, resVars);
				
				resVars.clear();		
				var.type = TianShanIce::vtLongs;
				var.lints.push_back(nBandwidth); // 2Mbps
				resVars[CPHPM_BANDWIDTH] = var;
				sess->addResource(::TianShanIce::SRM::rtProvisionBandwidth, resVars);
				
				
				
				::Ice::Long n = c + nStartDelayTime*1000;	//delay 10 seconds
				std::string startTimeUTC = ZQTianShan::TimeToUTC(n, buf, sizeof(buf) -2);
				::Ice::Long n2 = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());
				
				std::string endTimeUTC = ZQTianShan::TimeToUTC(n+nDurationSecs*1000, buf, sizeof(buf) -2);
				lastendtime = n+nDurationSecs*1000;
				sess->setup(startTimeUTC, endTimeUTC);
				sess->commit();
				
			}
			catch(const ::TianShanIce::BaseException& ex)
			{
				printf("caught exception[%s]: %s\n", ex.ice_name().c_str(), ex.message.c_str());
			}
			catch(const ::Ice::Exception& ex)
			{
				printf("caught exception[%s]\n", ex.ice_name().c_str());
			}
			count--;
		}
	}
	if(strcmp(apptype,"rtinas")==0)
	{
		printf("Input szOutputFilename.\n");  
		cin >> filename;
        printf("Input szMulticastUrl.\n"); 
		cin >> ip;
		printf("Input szSessionInterval.\n"); 
		cin >> interval;
		printf("Input szSessionNumber.\n"); 
		cin >> count;
		printf("Input szSessionDuration.\n"); 
		cin >> nDurationSecs;
		printf("Input szBandwidth.\n"); 
		cin >> nBandwidth;	
		
		::Ice::Long  lastendtime = 0; 
		
		if (count == 0)
			count = 1;
		
		
		char index[10];
		itoa(count, index, 10);
		int len = strlen(index);
		char cc[10] = "000000000"; 
		strncpy(index, cc,len);
		int number = 0;
		
		while (count)
		{	
			number++;
			::Ice::Long c;
			if (lastendtime == 0)
				c = ZQTianShan::now();
			else
				c = lastendtime;
			
			char buf[32];
			char indexlast[10];
			itoa(number, indexlast, 10);
			try
			{
				TianShanIce::ContentProvision::ProvisionSessionPrx sess;
				strcpy(buf, filename.c_str());
				strcat(buf, index);
				strcat(buf, indexlast);
				TianShanIce::ContentProvision::ProvisionContentKey contentKey;
				contentKey.content = buf;
				contentKey.contentStoreNetId = "rtinas";
				sess = cpePrx->createSession(contentKey, METHODTYPE_RTIVSNAS, TianShanIce::ContentProvision::potDirect, NULL, NULL);
				
				TianShanIce::ValueMap resVars;
				TianShanIce::Variant	var;
				var.type = TianShanIce::vtStrings;
				var.bRange = false;
				var.strs.clear();
				var.strs.push_back(contentKey.content);
				resVars[CPHPM_FILENAME] = var;
				var.strs.clear();
				var.strs.push_back(ip);
				resVars[CPHPM_SOURCEURL] = var;
				var.strs.clear();
				sess->addResource(::TianShanIce::SRM::rtURI,resVars);
				
				resVars.clear();		
				var.type = TianShanIce::vtLongs;
				var.lints.push_back(nBandwidth); // 2Mbps
				resVars[CPHPM_BANDWIDTH] = var;
				sess->addResource(::TianShanIce::SRM::rtProvisionBandwidth, resVars);
				
				
				
				
				::Ice::Long n = c + nStartDelayTime*1000;	//delay 10 seconds
				std::string startTimeUTC = ZQTianShan::TimeToUTC(n, buf, sizeof(buf) -2);
				::Ice::Long n2 = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());
				
				std::string endTimeUTC = ZQTianShan::TimeToUTC(n+nDurationSecs*1000, buf, sizeof(buf) -2);
				lastendtime = n+nDurationSecs*1000;
				sess->setup(startTimeUTC, endTimeUTC);
				sess->commit();
				
			}
			catch(const ::TianShanIce::BaseException& ex)
			{
				printf("caught exception[%s]: %s\n", ex.ice_name().c_str(), ex.message.c_str());
			}
			catch(const ::Ice::Exception& ex)
			{
				printf("caught exception[%s]\n", ex.ice_name().c_str());
			}
			count--;
		}
	}
	if(strcmp(apptype,"rds")==0)
	{
		::Ice::Long c = ZQTianShan::now();
		char buf[32];
		try
		{
			TianShanIce::ContentProvision::ProvisionSessionPrx sess;
			sprintf(buf, "%lld", c++);
			TianShanIce::ContentProvision::ProvisionContentKey contentKey;
			contentKey.content = buf;
			contentKey.contentStoreNetId = "rds";
			sess = cpePrx->createSession(contentKey, METHODTYPE_RDSVSVSTRM, TianShanIce::ContentProvision::potDirect, NULL, NULL);
			
			TianShanIce::ValueMap resVars;
			TianShanIce::Variant	var;
			var.type = TianShanIce::vtStrings;
			var.bRange = false;
			var.strs.clear();
			var.strs.push_back(contentKey.content);
			resVars[CPHPM_FILENAME] = var;
			var.strs.clear();
			sess->addResource(::TianShanIce::SRM::rtURI, resVars);
			
			resVars.clear();		
			var.type = TianShanIce::vtLongs;
			var.lints.push_back(nBandwidth); // 2Mbps
			resVars[CPHPM_BANDWIDTH] = var;
			sess->addResource(::TianShanIce::SRM::rtProvisionBandwidth, resVars);
			
			::Ice::Long n = ZQTianShan::now() + 10*1000;	//delay 10 seconds
			std::string startTimeUTC = ZQTianShan::TimeToUTC(n, buf, sizeof(buf) -2);
			::Ice::Long n2 = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());
			
			std::string endTimeUTC = ZQTianShan::TimeToUTC(ZQTianShan::now()+1200000, buf, sizeof(buf) -2);
			
			sess->setup(startTimeUTC, endTimeUTC);
			::TianShanIce::Properties props = sess->getProperties();
			strPushUrl = props[PROPTY_PUSHURL];
			printf("Push Url: %s\n", strPushUrl.c_str());
			sess->commit();
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			printf("caught exception[%s]: %s\n", ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			printf("caught exception[%s]\n", ex.ice_name().c_str());
		}
	}
	if(strcmp(apptype,"rtf")==0)
	{
		std::string methodType;
		std::string filename;
		std::string sourcename;
		bool methodFlag = false;
		printf("Input szmethodType(Both push, ftp and ntfs are available.).\n");
 		cin >> methodType;

		printf("Input content name.\n");
		cin >> filename ;
		printf("Input source name(Including filepath).\n");
		cin >> sourcename ;

		::Ice::Long c = ZQTianShan::now();
		char buf[32];
		try
		{
			TianShanIce::ContentProvision::ProvisionSessionPrx sess;
			sprintf(buf, "%lld", c++);
			TianShanIce::ContentProvision::ProvisionContentKey contentKey;

			    contentKey.content = filename;
			contentKey.contentStoreNetId = "rtfrds";

			if (strcmp(methodType.c_str(),"push") == 0)
			    sess = cpePrx->createSession(contentKey, METHODTYPE_RTFRDSVSVSTRM, TianShanIce::ContentProvision::potDirect, NULL, NULL);
			if (strcmp(methodType.c_str(),"ftp") == 0)
				sess = cpePrx->createSession(contentKey, METHODTYPE_FTPRTFVSVSTRM, TianShanIce::ContentProvision::potDirect, NULL, NULL);
			if (strcmp(methodType.c_str(),"ntfs") == 0)
				sess = cpePrx->createSession(contentKey, METHODTYPE_NTFSRTFVSVSTRM, TianShanIce::ContentProvision::potDirect, NULL, NULL);
			
			TianShanIce::ValueMap resVars;
			TianShanIce::Variant	var;
			if (strcmp(methodType.c_str(),"push") == 0)
			{
				var.type = TianShanIce::vtStrings;
				var.bRange = false;
				var.strs.clear();
				var.strs.push_back(contentKey.content);
				resVars[CPHPM_FILENAME] = var;
				sess->addResource(::TianShanIce::SRM::rtURI, resVars);
			}
			else
			{
				var.type = TianShanIce::vtStrings;
				var.bRange = false;
				var.strs.clear();
				var.strs.push_back(contentKey.content);
				resVars[CPHPM_FILENAME] = var;
				var.strs.clear();
				var.strs.push_back(sourcename);
				resVars[CPHPM_SOURCEURL] = var;
				sess->addResource(::TianShanIce::SRM::rtURI, resVars);
			}
			resVars.clear();		
			var.type = TianShanIce::vtLongs;
			var.lints.push_back(nBandwidth); // 2Mbps
			resVars[CPHPM_BANDWIDTH] = var;
			sess->addResource(::TianShanIce::SRM::rtProvisionBandwidth, resVars);
			
			::Ice::Long n = ZQTianShan::now() + 10*1000;	//delay 10 seconds
			std::string startTimeUTC = ZQTianShan::TimeToUTC(n, buf, sizeof(buf) -2);
			::Ice::Long n2 = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());
			
			std::string endTimeUTC = ZQTianShan::TimeToUTC(ZQTianShan::now()+24000000, buf, sizeof(buf) -2);
			
			sess->setup(startTimeUTC, endTimeUTC);
			sess->commit();
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			printf("caught exception[%s]: %s\n", ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			printf("caught exception[%s]\n", ex.ice_name().c_str());
		}
	}
	if(strcmp(apptype,"fscp")==0)
	{
		printf("Input szInputFileName(the file path should be included.).\n");
		cin >> strSrcFile;

		::Ice::Long c = ZQTianShan::now();
		char buf[32];
        try
		{
			TianShanIce::ContentProvision::ProvisionSessionPrx sess;
			sprintf(buf, "%lld", c++);
			TianShanIce::ContentProvision::ProvisionContentKey contentKey;
			contentKey.content = strSrcFile;
			contentKey.contentStoreNetId = "fscp";
			sess = cpePrx->createSession(contentKey, METHODTYPE_FSCOPYVSVSTRM, TianShanIce::ContentProvision::potDirect, NULL, NULL);
			
			TianShanIce::ValueMap resVars;
			TianShanIce::Variant	var;
			var.type = TianShanIce::vtStrings;
			var.bRange = false;
			var.strs.clear();
			var.strs.push_back(contentKey.content);
			resVars[CPHPM_FILENAME] = var;
			var.strs.clear();
			sess->addResource(::TianShanIce::SRM::rtURI, resVars);
			
			resVars.clear();		
			var.type = TianShanIce::vtLongs;
			var.lints.push_back(nBandwidth); // 2Mbps
			resVars[CPHPM_BANDWIDTH] = var;
			sess->addResource(::TianShanIce::SRM::rtProvisionBandwidth, resVars);
			
			::Ice::Long n = ZQTianShan::now() + 10*1000;	//delay 10 seconds
			std::string startTimeUTC = ZQTianShan::TimeToUTC(n, buf, sizeof(buf) -2);
			::Ice::Long n2 = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());
			
			std::string endTimeUTC = ZQTianShan::TimeToUTC(ZQTianShan::now()+2400000, buf, sizeof(buf) -2);
			
			sess->setup(startTimeUTC, endTimeUTC);
			sess->commit();
			
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			printf("caught exception[%s]: %s\n", ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			printf("caught exception[%s]\n", ex.ice_name().c_str());
		}
	}
	if(strcmp(apptype,"nascopy")==0)
	{
		std::string desfilename;
		ULONG64   bitrate;
		printf("Input szInputFileName(the file path should be included.).\n");
		cin >> strSrcFile;
		printf("Input szOutputFileName(Not including path.).\n");
		cin>>desfilename;
		printf("Input bitrate.\n");
		cin>>bitrate;

		::Ice::Long c = ZQTianShan::now();
		char buf[32];
        try
		{
			TianShanIce::ContentProvision::ProvisionSessionPrx sess;
			sprintf(buf, "%lld", c++);
			TianShanIce::ContentProvision::ProvisionContentKey contentKey;
			contentKey.content = desfilename;
			contentKey.contentStoreNetId = "nascopy";
			sess = cpePrx->createSession(contentKey, METHODTYPE_NASCOPYVSVSTRM, TianShanIce::ContentProvision::potDirect, NULL, NULL);
			
			TianShanIce::ValueMap resVars;
			TianShanIce::Variant	var;
			var.type = TianShanIce::vtStrings;
			var.bRange = false;
			var.strs.clear();
			var.strs.push_back(contentKey.content);
			resVars[CPHPM_FILENAME] = var;
			var.strs.clear();
			var.strs.push_back(strSrcFile);
			resVars[CPHPM_SOURCEURL] = var;
			sess->addResource(::TianShanIce::SRM::rtURI, resVars);
			
			resVars.clear();		
			var.type = TianShanIce::vtLongs;
			var.lints.push_back(bitrate); // 2Mbps
			resVars[CPHPM_BANDWIDTH] = var;
			sess->addResource(::TianShanIce::SRM::rtProvisionBandwidth, resVars);

			
			::Ice::Long n = ZQTianShan::now() + 10*1000;	//delay 10 seconds
			std::string startTimeUTC = ZQTianShan::TimeToUTC(n, buf, sizeof(buf) -2);
			::Ice::Long n2 = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());
			
			std::string endTimeUTC = ZQTianShan::TimeToUTC(ZQTianShan::now()+2400000, buf, sizeof(buf) -2);
			
			sess->setup(startTimeUTC, endTimeUTC);
			sess->commit();
			
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			printf("caught exception[%s]: %s\n", ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			printf("caught exception[%s]\n", ex.ice_name().c_str());
		}
	}
    if(strcmp(apptype,"list")==0)
	{
// 		printf("Input szmethodType.\n");
// 		cin >> methodType;
// 		printf("Input szstartId.\n");
// 		cin >> startId;
// 		printf("Input imaxCount.\n");
// 		cin >> maxcount;
		std::string methodType("");
		std::string startId("");
    	int maxcount = 0;

		try
		{
		::TianShanIce::StrValues paramNames;
		std::vector< ::TianShanIce::ContentProvision::ProvisionInfo>::iterator iter;
		TianShanIce::ContentProvision::ProvisionInfo proInfo;
		::TianShanIce::ContentProvision::ProvisionContentKey contentKey;
        ::TianShanIce::Properties params;
		paramNames.push_back(SYS_PROP(contentName));
		paramNames.push_back(SYS_PROP(contentStore));
		paramNames.push_back(SYS_PROP(volume));
		paramNames.push_back(SYS_PROP(scheduledStart));
		paramNames.push_back(SYS_PROP(scheduledEnd));
		
		TianShanIce::ContentProvision::ProvisionInfos listInfo;
		listInfo = cpePrx->listSessions(methodType,paramNames,startId,maxcount);
		
		int i = listInfo.size();
		std::string startTimeUTC, endTimeUTC, volume, netId, content;
		::TianShanIce::ContentProvision::ProvisionState state;
		
		printf("\n");
		if (listInfo.size() == 0)
			printf("There is no session.\n");
		else
		{
			for (iter = listInfo.begin(); iter != listInfo.end();iter++)
			{
				proInfo = *iter;
				content = proInfo.contentKey.content;
				netId = proInfo.contentKey.contentStoreNetId;	
				params = proInfo.params;
				state = proInfo.state;
				
				volume = proInfo.contentKey.volume;
				startTimeUTC = (*params.find(SYS_PROP(scheduledStart))).second;
				endTimeUTC = (*params.find(SYS_PROP(scheduledEnd))).second;
				printf("Provision session info: content:%s; contentStoreNetId:%s; volume:%s; scheduleStartTime:%s; scheduleEndTime:%s.\n",content.c_str(),netId.c_str(),volume.c_str(),startTimeUTC.c_str(),endTimeUTC.c_str());
			}
		}
 		::TianShanIce::ContentProvision::MethodInfos result;
 		::TianShanIce::ContentProvision::MethodInfos::iterator itor;
 		result = cpePrx->listMethods();
 		for (itor = result.begin(); itor != result.end(); itor++)
 		{
 			printf("Provision methodType:%s; allocateKbps:%d; maxKbps:%d; sessions:%d; maxsession:%d.\n",(*itor).methodType.c_str(),(*itor).allocatedKbps,(*itor).maxKbps,(*itor).sessions,(*itor).maxsessions);
 		}
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			printf("caught exception[%s]: %s\n", ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			printf("caught exception[%s]\n", ex.ice_name().c_str());
		}
	}
// 	if (argc != 2)
// 	{
// 		printf("Error number for Input parameters.\n");
// 		return 1;	
// 	}
// 
// 	if(strcmp(argv[1],"?") == 0)
// 	{
// 		printf("cpeclt szInputFileName(the file path should be included.).\b");
// 		return 1;
// 	}
// 	strSrcFile = argv[1];
// 
// 	//Set logPath to current path/////////////////////////////////////////////
// 	char path[MAX_PATH] = ".", *p=path;
// 	if (::GetModuleFileName(NULL, path, MAX_PATH-1)>0)
// 	{
// 		char* p = strrchr(path, FNSEPC);
// 		if (NULL !=p)
// 		{
// 			*p='\0';
// 			p = strrchr(path, FNSEPC);
// 			if (NULL !=p && (0==stricmp(FNSEPS "bin", p) || 0==stricmp(FNSEPS "exe", p)))
// 				*p='\0';
// 		}
// 	}
// 	strcat(path, FNSEPS);
// 	p = path+strlen(path);
// 
// 	while((ch = getopt(argc, argv, "he:b:d:s:")) != EOF)
// 	{
// 		switch (ch)
// 		{
// 		case '?':
// 		case 'h':
// 			usage();
// 			exit(0);
// 
// 		case 'e':
// 			endpoint = optarg;
// 			break;
// 
// 		case 'b':
// 			nBandwidth = atoi(optarg);
// 			break;
// 
// 		case 'd':
// 			traceLevel = atoi(optarg);
// 			break;
// 		case 's':
// 			strSrcFile=optarg;
// 			break;
// 		default:
// 			printf("Error: unknown option %c specified\n", ch);
// 			exit(1);
// 		}
// 	}
// 
// 	int i =0;
// 	Ice::CommunicatorPtr ic = Ice::initialize(i, NULL);
//     printf("Connect CPE at \"%s\"\n", endpoint.c_str());
// 	
// 	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
// 	{
// 		printf("Unable to install handler!                      \n");
// 		return -1;
// 	}
// 
// 	std::string strPushUrl;
// 
// 	try {
// 		ProvisionSessionBindImpl::Ptr bind = new ProvisionSessionBindImpl();
// 		::Ice::ObjectAdapterPtr testAdapter = ic->createObjectAdapterWithEndpoints("testAdapter", "default -p 11111");
// 		Ice::Identity identBind;
// 		identBind.name = identBind.category = "testBind";
// 		testAdapter->add(bind, identBind);
// 		testAdapter->activate();
// 		TianShanIce::ContentProvision::ProvisionSessionBindPrx bindPrx
// 			= ::TianShanIce::ContentProvision::ProvisionSessionBindPrx::checkedCast(testAdapter->createProxy(identBind));
// 
// 		std::string strContentProvisionSvc = std::string(SERVICE_NAME_ContentProvisionService ":") + endpoint;
// 		TianShanIce::ContentProvision::ContentProvisionServicePrx cpePrx = TianShanIce::ContentProvision::ContentProvisionServicePrx::checkedCast(ic->stringToProxy(strContentProvisionSvc));
// 		TianShanIce::ContentProvision::ProvisionSessionPrx sess;
// 		if (!cpePrx)
// 			return 1000;
// 		
// 		::Ice::Long c = ZQTianShan::now();
// 		char buf[32];
// #define TEST_RDS
// 
// #ifdef TEST_RDS
// 		{
// 			TianShanIce::ContentProvision::ProvisionSessionPrx sess;
// 			sprintf(buf, "%lld", c++);
// 			TianShanIce::ContentProvision::ProvisionContentKey contentKey;
// 			contentKey.content = strSrcFile;
// 			contentKey.contentStoreNetId = "rds";
// 			sess = cpePrx->createSession(contentKey, METHODTYPE_FSCPVSVSTRM, TianShanIce::ContentProvision::potDirect, NULL, bindPrx);
// 
// 			TianShanIce::ValueMap resVars;
// 			TianShanIce::Variant	var;
// 			var.type = TianShanIce::vtStrings;
// 			var.bRange = false;
// 			var.strs.clear();
// 			var.strs.push_back(contentKey.content);
// 			resVars[CPHPM_FILENAME] = var;
// 			var.strs.clear();
// 			sess->addResource(::TianShanIce::SRM::rtURI, resVars);
// 			
// 			resVars.clear();		
// 			var.type = TianShanIce::vtLongs;
// 			var.lints.push_back(nBandwidth); // 2Mbps
// 			resVars[CPHPM_BANDWIDTH] = var;
// 			sess->addResource(::TianShanIce::SRM::rtProvisionBandwidth, resVars);
// 			
// 			::Ice::Long n = ZQTianShan::now() + 10*1000;	//delay 10 seconds
// 			std::string startTimeUTC = ZQTianShan::TimeToUTC(n, buf, sizeof(buf) -2);
// 			::Ice::Long n2 = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());
// 			
// 			std::string endTimeUTC = ZQTianShan::TimeToUTC(ZQTianShan::now()+2400000, buf, sizeof(buf) -2);
// 			
// 			sess->setup(startTimeUTC, endTimeUTC);
// 			sess->commit();
// 			
// 		}
// #endif
// 
// 	}
// 	catch(const ::TianShanIce::BaseException& ex)
// 	{
// 		printf("caught exception[%s]: %s\n", ex.ice_name().c_str(), ex.message.c_str());
// 	}
// 	catch(const ::Ice::Exception& ex)
// 	{
// 		printf("caught exception[%s]\n", ex.ice_name().c_str());
// 	}

	ic->destroy();

	//
	// start ftp client to do upload
	//
	{
		

	}
	
	return 0;
}

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
    switch(CEvent)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
		bQuit = true;
        break;
    }
    return TRUE;
}

