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
// $Log: /ZQProjs/TianShan/CPE/service/CPEClient.cpp $
// 
// 11    8/25/16 11:34a Li.huang
// 
// 10    8/09/16 3:42p Li.huang
// 
// 9     7/06/16 3:18p Li.huang
// 
// 8     5/13/15 2:56p Zhiqiang.niu
// 
// 7     4/29/15 10:29a Zhiqiang.niu
// modify for CPH_AquaLib
// 
// 6     3/09/15 3:02p Li.huang
// 
// 5     3/05/15 5:14p Li.huang
// add method XOR.MediaCluster.CSI
// 
// 4     12/31/13 7:18p Hui.shao
// pGlog
// 
// 3     9/15/11 1:16p Li.huang
// add c2pull method
// 
// 2     4/15/11 10:43a Fei.huang
// * migrated to linux
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 12    10-09-24 17:38 Li.huang
// 
// 11    10-07-30 9:53 Li.huang
// 
// 10    10-01-26 13:15 Xia.chen
// 
// 9     10-01-26 11:38 Xia.chen
// add multiple trick speed argument
// 
// 8     09-07-31 14:13 Xia.chen
// 
// 7     09-02-09 17:00 Yixin.tian
// add test CopyDemo
// 
// 6     09-01-06 14:20 Xia.chen
// 
// 5     08-11-18 11:11 Jie.zhang
// merge from TianShan1.8
// 
// 6     08-11-14 11:36 Xia.chen
// 
// 5     08-09-18 13:56 Xia.chen
// 
// 4     08-08-18 18:49 Xia.chen
// 
// 3     08-08-13 16:19 Xia.chen
// 
// 2     08-08-08 10:53 Jie.zhang
// 
// 1     08-07-31 17:47 Xia.chen
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
#include "strHelper.h"

//#define DEFAULT_ENDPOINT_CPE			"default -p 10010"
//#define DEFAULT_ENDPOINT_CPE			"tcp -h 192.168.81.102 -p 10010"
//#define SERVICE_NAME_ContentProvisionService	"ContentProvision"


extern "C"
{
#include <time.h>
#include <stdio.h>
#ifdef ZQ_OS_MSWIN
#include <direct.h>
#endif
}

#ifdef ZQ_OS_MSWIN
BOOL WINAPI ConsoleHandler(DWORD event);
#endif
bool bQuit = false;

void usage()
{	
	printf("Usage: CPEClient -e [-p] -m -s -f -b [-t] [-d] [-i] [-c]\n");
	printf("\t-e   cpesvc ip\n");
	printf("\t-p   the port of cpesvc service, the default value is 10500\n");
	printf("\t-m   the provision method for ingest.\n");
	printf("	      1  SeaChange.MediaCluster.RDS \n");  
	printf("	      2  SeaChange.MediaCluster.RTFRDS\n");
	printf("	      3  SeaChange.MediaCluster.RTI\n");
	printf("	      4  SeaChange.MediaCluster.FSCOPY\n");
	printf("	      5  SeaChange.MediaCluster.NTFSRTF\n");
	printf("	      6  SeaChange.MediaCluster.NASCOPY\n");
	printf("	      7  SeaChange.MediaCluster.FTPRTF\n");
	printf("	      8  SeaChange.NAS.RTI\n");
	printf("	      9  SeaChange.MediaCluster.NTFSRTF.H264\n");
	printf("	      10 SeaChange.MediaCluster.FTPRTF.H264\n");
	printf("	      11 SeaChange.MediaCluster.RTI.H264\n");
	printf("	      12 SeaChange.MediaCluster.FTPPropagation\n");
	printf("	      13 Cancel session\n");
	printf("	      14 SeaChange.NPVR\n");
	printf("	      15 CopyDemo\n");
	printf("	      16 SeaChange.CDN.FTPPropagation\n");
	printf("	      17 SeaChange.CDN.FTPRTF\n");
	printf("	      18 SeaChange.CDN.HTTPPropagation\n");
	printf("	      20 SeaChange.CDN.C2Pull\n");
	printf("	      21 SeaChange.CDN.C2Pull.H264\n");
	printf("	      22 XOR.MediaCluster.CSI\n");
	printf("	      23 XOR.Aqua.FTPRTF\n");
	printf("	      24 XOR.Aqua.FTPRTF.H264\n");
	printf("	      25 XOR.Aqua.NTFSRTF\n");
	printf("	      26 XOR.Aqua.NTFSRTF.H264\n");
	printf("	      27 XOR.Aqua.RTI\n");
	printf("	      28 XOR.Aqua.RTI.H264\n");
	printf("	      29 SeaChange.CDN.NTFSRTF\n");
	printf("	      30 SeaChange.CDN.NTFSRTF.H264\n");
	printf("	      31 XOR.Raw.RTI\n");
	printf("\t-s   the source URL for ingest\n");
	printf("\t-f   the output file name for ingest\n");
	printf("\t-b   the bandwidth for ingest\n");
	printf("\t-t   start time (UTC) for ingest\n");
	printf("\t-d   duration(seconds) for ingest\n");
	printf("\t-i   interval(seconds) used for back to back provision sessions\n");
	printf("\t-c   session count (default 1)used for back to back provision sessions\n");	
	printf("\t-v   multiple trick speed, default is 7.5, eg:\"15,30\"\n");
	printf("\t-pid    provideId\n");
	printf("\t-paid   provide asset id\n");
	printf("\t-A   packets of the listed PID were added after CBR mux\n");
	printf("\t-n   index type, eg:\"VVC\" \"VVX\"\n");
	printf("\t-noTrick   noTrickspeed, eg:\"0\" \"1\"\n");
	printf("\t-h   display this help\n");
	printf("Example\n");
	printf("CPEClient -e 10.15.10.240 -m 3 -s udp://225.25.25.1:1234 -f test -b 3750000\n");
	printf("CPEClient -e 10.15.10.240 -p 10500 -m 7 -s ftp://192.168.81.102:21/aa -f test -b 3750000\n");
	printf("CPEClient -e 10.15.10.240 -p 10500 -m 5 -s file://192.168.81.102/aa -f test -b 3750000\n");
	printf("CPEClient -e 10.15.10.240 -p 10500 -m 6 -s file://10.15.10.240/aa -f test -b 3750000\n");
	printf("CPEClient -e 10.15.10.240 -p 10500 -m 13 -f test\n");
	printf("CPEClient -e 10.15.10.240 -p 10500 -m 18 -s c2http://10.15.10.50 -f test -b 3750000 -pid zq.com -paid cdntest1234567895588\n");
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
		printf("\n%s {content:%s; storage:%s; volume:%s} progress: "FMT64" of "FMT64, ZQTianShan::TimeToUTC(timeStamp, buf, sizeof(buf)-2),
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
	std::string endpoint;
	//	int traceLevel = ZQ::common::Log::L_DEBUG;	
	//	char*	pLogPath=NULL;
	int nBandwidth = 3750000;
	int nDurationSecs = 30*60;	//half an hour
	std::string filename;
	int interval = 10;
	int count = 1;
	//	int nStartDelayTime = 10;
	char cTemp[MAX_PATH];
	std::string methodType;
	std::string sourceUrl;
	std::string startTime;
	int method;
	std::string port="10500";
	bool bgetExportUrl = false;
	std::string pro = "ftp";
	int spent;
	std::string netId = "";
	std::string vectorSpeed;
	::TianShanIce::ContentProvision::TrickSpeedCollection trickcol;
	std::string strPID;
	std::string strPAID;
	std::string strAugmentationIds;
	std::string indexType = "";
	std::string strNoTrickSpeed = "0";

	if (argc ==1 || argv[1][1]=='h')
	{
		usage();
		return 1;
	}
	else
	{
		for(int i=1; i<argc; i+=2 )
		{
			if( argv[ i ][ 0 ] == '-' )
			{
				strcpy(cTemp,argv[i+1]);
				if(stricmp(argv[i], "-pid") == 0)
				{
					strPID = argv[i+1];
					continue;
				}
				else if(stricmp(argv[i], "-paid") == 0)
				{
					strPAID = argv[i+1];
					continue;
				}
				else if(stricmp(argv[i], "-noTrick") == 0)
				{
					strNoTrickSpeed = argv[i+1];
					continue;
				}

				switch( argv[ i ][ 1 ] )
				{
				case 'A':
					strAugmentationIds = cTemp;
					break;
				case 'e':
					endpoint = cTemp;
					break;
				case 'p':
					port = cTemp;
					break;
				case 'm':
					method = atoi(cTemp);
					switch (method)
					{
					case 1:
						methodType = METHODTYPE_RDSVSVSTRM;
						break;
					case 2:
						methodType = METHODTYPE_RTFRDSVSVSTRM;
						break;
					case 3:
						methodType = METHODTYPE_RTIVSVSTRM;
						break;
					case 4:
						methodType = METHODTYPE_FSCOPYVSVSTRM;
						break;
					case 5:
						methodType = METHODTYPE_NTFSRTFVSVSTRM;
						break;
					case 6:
						methodType = METHODTYPE_NASCOPYVSVSTRM;
						break;
					case 7:
						methodType = METHODTYPE_FTPRTFVSVSTRM;
						break;
					case 8:
						methodType = METHODTYPE_RTIVSNAS;
						break;
					case 9:
						methodType = METHODTYPE_NTFSRTFH264VSVSTRM;
						break;
					case 10:
						methodType = METHODTYPE_FTPRTFH264VSVSTRM;
						break;
					case 11:
						methodType = METHODTYPE_RTIH264VSVSTRM;
						break;
					case 12:
						methodType = METHODTYPE_FTPPropagation;
						break;
					case 14:
						methodType = METHODTYPE_NPVRVSVSTRM;
						break;
					case 15:
						methodType = "CopyDemo";
						break;
					case 16:
						methodType = METHODTYPE_CDN_FTPPropagation;
						break;
					case 17:
						methodType = METHODTYPE_CDN_FTPRTF;
						break;
					case 18:
						methodType = METHODTYPE_CDN_HTTPPropagation;
						break;
					case 19:
						methodType = METHODTYPE_CDN_FTPRTFH264;
						break;
					case 20:
						methodType = METHODTYPE_CDN_C2Pull;
						break;
					case 21:
						methodType = METHODTYPE_CDN_C2PullH264;
						break;
					case 22:
						methodType = METHODTYPE_CSI;
						break;
					case 23:
						methodType = METHODTYPE_AQUA_FTPRTF;
						break;
					case 24:
						methodType = METHODTYPE_AQUA_FTPRTFH264;
						break;
					case 25:
						methodType = METHODTYPE_AQUA_NTFSRTF;
						break;
					case 26:
						methodType = METHODTYPE_AQUA_NTFSRTFH264;
						break;
					case 27:
						methodType = METHODTYPE_AQUA_RTI;
						break;
					case 28:
						methodType = METHODTYPE_AQUA_RTIH264;
						break;
					case 29:
						methodType = METHODTYPE_CDN_NTFSRTF;
						break;
					case 30:
						methodType = METHODTYPE_CDN_NTFSRTFH264;
						break;
					case 31:
						methodType = METHODTYPE_RTIRAW;
						break;
					}
					break;
				case 's':
					sourceUrl = cTemp;
					break;
				case 'f':
					filename = cTemp;
					break;
				case 'b':
					nBandwidth = atoi(cTemp);
					break;
				case 't':
					startTime = cTemp;
					break;
				case 'd':
					nDurationSecs = atoi(cTemp);
					break;
				case 'i':
					interval = atoi(cTemp);
					break;
				case 'c':
					count = atoi(cTemp);
					break;
				case 'g':
					bgetExportUrl = atoi(cTemp);
					break;
				case 'v':
					vectorSpeed = cTemp;
					break;
				case 'n':
					indexType = cTemp;
					break;
				}
			}
			else
			{
				printf("Error input parameter format.\n");
				return 1;
			}
		}

	}

	if (vectorSpeed.empty())
	{
		trickcol.push_back(7.5);
	}
	else
	{
		std::vector<std::string> strVec;
		strVec = ZQ::common::stringHelper::split(vectorSpeed,',');

		for (std::vector<std::string>::iterator it = strVec.begin();it != strVec.end(); it++)
		{
			trickcol.push_back(atof((*it).c_str()));
		}

	}

	int i =0;
	Ice::CommunicatorPtr ic = Ice::initialize(i, NULL);
	printf("Connect CPE at \"%s\"\n", endpoint.c_str());

	TianShanIce::ContentProvision::ContentProvisionServicePrx cpePrx;
	TianShanIce::ContentProvision::ProvisionSessionBindPrx bindPrx;
	std::string strContentProvisionSvc = std::string(SERVICE_NAME_ContentProvisionService ":" )+std::string("tcp -h ") + endpoint+ std::string(" -p ")+port;
	try {
#if 0
		ProvisionSessionBindImpl::Ptr bind = new ProvisionSessionBindImpl();
		::Ice::ObjectAdapterPtr testAdapter = ic->createObjectAdapterWithEndpoints("testAdapter", "default -p 11111");
		printf("successfully create adapter\n");
		Ice::Identity identBind;
		identBind.name = identBind.category = "testBind";
		testAdapter->add(bind, identBind);
		testAdapter->activate();
		bindPrx= ::TianShanIce::ContentProvision::ProvisionSessionBindPrx::checkedCast(testAdapter->createProxy(identBind));
		printf("successfully checkedCast\n");
#endif
		cpePrx = TianShanIce::ContentProvision::ContentProvisionServicePrx::checkedCast(ic->stringToProxy(strContentProvisionSvc));
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		printf("Connect to CPE[%s] caught exception[%s]: %s\n", strContentProvisionSvc.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		ic->destroy();
		return 0;
	}
	catch(const ::Ice::Exception& ex)
	{
		printf("Connect to CPE[%s] caught exception[%s]\n", strContentProvisionSvc.c_str(), ex.ice_name().c_str());
		ic->destroy();
		return 0;
	}

	printf("CPE[%s] connected\n", strContentProvisionSvc.c_str());

	if (bgetExportUrl)
	{
		char buf[32];
		TianShanIce::ContentProvision::ProvisionContentKey contentKey;
		if (filename.size() == 0)
			filename = sourceUrl.substr(sourceUrl.find_last_of('/')+1,sourceUrl.size()-sourceUrl.find_last_of('/')-1);
		strcpy(buf, filename.c_str());

		contentKey.content = buf;
		contentKey.contentStoreNetId = netId;

		int bb;
		std::string url = cpePrx->getExportURL(pro,contentKey,nBandwidth,spent,bb);
		printf("Url is %s\n",url.c_str());
		printf("permittedBitrate is %d\n",bb);
		printf("nTTL is %d\n",spent);

		ic->destroy();
		return 0;
	}
	if (method == 3||method==8||method == 11||method == 14 || method==27 || method==28 || method==31)
	{
		::Ice::Long  lastendtime = 0; 
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
				if (filename.size() == 0)
					filename = sourceUrl.substr(sourceUrl.find_last_of('/')+1,sourceUrl.size()-sourceUrl.find_last_of('/')-1);
				strcpy(buf, filename.c_str());
				strcat(buf, index);
				strcat(buf, indexlast);
				TianShanIce::ContentProvision::ProvisionContentKey contentKey;
				if (method == 14 || method == 31)
					contentKey.content = filename;
				else
					contentKey.content = buf;
				contentKey.contentStoreNetId = netId;
				sess = cpePrx->createSession(contentKey, methodType, TianShanIce::ContentProvision::potDirect, NULL, NULL);
				printf("session[%s] created\n", contentKey.content.c_str());

				if(method == 31)
				{
					sess->setProperty(CPHPM_PROVIDERID, strPID);
					sess->setProperty(CPHPM_PROVIDERASSETID, strPAID);
				}

				sess->setProperty(CPHPM_INDEXTYPE, indexType);
				if(strNoTrickSpeed == "1")
					sess->setProperty(CPHPM_NOTRICKSPEEDS, strNoTrickSpeed);
				TianShanIce::ValueMap resVars;
				TianShanIce::Variant	var;
				var.type = TianShanIce::vtStrings;
				var.bRange = false;
				var.strs.clear();
				var.strs.push_back(contentKey.content);
				resVars[CPHPM_FILENAME] = var;
				var.strs.clear();
				var.strs.push_back(sourceUrl);
				resVars[CPHPM_SOURCEURL] = var;
				var.strs.clear();
				sess->addResource(::TianShanIce::SRM::rtURI, resVars);

				resVars.clear();		
				var.type = TianShanIce::vtLongs;
				var.lints.push_back(nBandwidth); // 2Mbps
				resVars[CPHPM_BANDWIDTH] = var;
				sess->addResource(::TianShanIce::SRM::rtProvisionBandwidth, resVars);

				sess->setTrickSpeedCollection(trickcol);


				::Ice::Long n = c + interval*1000;	//delay 10 seconds
				std::string startTimeUTC = ZQTianShan::TimeToUTC(n, buf, sizeof(buf) -2);
				//				::Ice::Long n2 = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());

				std::string endTimeUTC = ZQTianShan::TimeToUTC(n+nDurationSecs*1000, buf, sizeof(buf) -2);
				lastendtime = n+nDurationSecs*1000;
				sess->setup(startTimeUTC, endTimeUTC);
				sess->commit();
				printf("session[%s] commited\n", contentKey.content.c_str());
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
	if (method == 5  || method == 6  || method == 7  || method == 9  || method == 10  || method == 12  ||
		method == 16 || method == 17 || method == 18 || method == 19 || method == 20  || method == 21  || 
		method == 22 || method == 23 || method == 24 || method == 25 || method == 26  )
	{
		::Ice::Long  lastendtime = 0; 
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
				sprintf(buf, FMT64, c++);
				TianShanIce::ContentProvision::ProvisionContentKey contentKey;
				if (filename.size() == 0)
				{
					if(method != 18)
						filename = sourceUrl.substr(sourceUrl.find_last_of('/')+1,sourceUrl.size()-sourceUrl.find_last_of('/')-1);
					else
					{
						filename = strPAID + strPID;
					}
				}
				strcpy(buf, filename.c_str());
				strcat(buf, index);
				strcat(buf, indexlast);
				contentKey.content = buf;
				contentKey.contentStoreNetId = netId;
				sess = cpePrx->createSession(contentKey, methodType, TianShanIce::ContentProvision::potDirect, NULL, NULL);
				printf("session[%s] created\n", contentKey.content.c_str());
				if(method == 18 || method ==20|| method ==21)
				{
					sess->setProperty(CPHPM_PROVIDERID, strPID);
					sess->setProperty(CPHPM_PROVIDERASSETID, strPAID);
				}
				sess->setProperty(CPHPM_AUGMENTATIONPIDS, strAugmentationIds);
				sess->setProperty(CPHPM_INDEXTYPE, indexType);
				if(strNoTrickSpeed == "1")
					sess->setProperty(CPHPM_NOTRICKSPEEDS, strNoTrickSpeed);

				TianShanIce::ValueMap resVars;
				TianShanIce::Variant	var;
				var.type = TianShanIce::vtStrings;
				var.bRange = false;
				var.strs.clear();
				var.strs.push_back(contentKey.content);
				resVars[CPHPM_FILENAME] = var;
				var.strs.clear();
				var.strs.push_back(sourceUrl);
				resVars[CPHPM_SOURCEURL] = var;
				sess->addResource(::TianShanIce::SRM::rtURI, resVars);

				resVars.clear();		
				var.type = TianShanIce::vtLongs;
				var.lints.push_back(nBandwidth); // 2Mbps
				resVars[CPHPM_BANDWIDTH] = var;
				sess->addResource(::TianShanIce::SRM::rtProvisionBandwidth, resVars);

				sess->setTrickSpeedCollection(trickcol);

				::Ice::Long n = c + interval*1000;	//delay 10 seconds
				std::string startTimeUTC = ZQTianShan::TimeToUTC(n, buf, sizeof(buf) -2);
				//				::Ice::Long n2 = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());

				std::string endTimeUTC = ZQTianShan::TimeToUTC(n+nDurationSecs*1000, buf, sizeof(buf) -2);
				lastendtime = n+nDurationSecs*1000;
				sess->setup(startTimeUTC, endTimeUTC);
				sess->commit();
				printf("session[%s] commited\n", contentKey.content.c_str());
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
	if (method ==1 || method ==2)
	{
		std::string strPushUrl;
		::Ice::Long c = ZQTianShan::now();
		char buf[32];
		try
		{
			TianShanIce::ContentProvision::ProvisionSessionPrx sess;
			sprintf(buf, FMT64, c++);
			TianShanIce::ContentProvision::ProvisionContentKey contentKey;
			contentKey.content = buf;
			contentKey.contentStoreNetId = netId;
			sess = cpePrx->createSession(contentKey, methodType, TianShanIce::ContentProvision::potDirect, NULL, NULL);

			sess->setProperty(CPHPM_INDEXTYPE, indexType);
			if(strNoTrickSpeed == "1")
				sess->setProperty(CPHPM_NOTRICKSPEEDS, strNoTrickSpeed);
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

			sess->setTrickSpeedCollection(trickcol);

			::Ice::Long n = ZQTianShan::now() + 10*1000;	//delay 10 seconds
			std::string startTimeUTC = ZQTianShan::TimeToUTC(n, buf, sizeof(buf) -2);
			//			::Ice::Long n2 = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());

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
	if (method == 13)
	{
		try
		{
			TianShanIce::ContentProvision::ProvisionSessionPrx sess;
			TianShanIce::ContentProvision::ProvisionContentKey contentKey;
			contentKey.content = filename;
			contentKey.contentStoreNetId = netId;
			sess = cpePrx->openSession(contentKey);

			sess->cancel(0,"cancel by client tool");
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

	if(method == 15)//add for simple copy demo
	{
		::Ice::Long c = ZQTianShan::now();
		char buf[256];
		try
		{
			TianShanIce::ContentProvision::ProvisionSessionPrx sess;
			TianShanIce::ContentProvision::ProvisionContentKey contentKey;
			if (filename.size() == 0)
				filename = sourceUrl.substr(sourceUrl.find_last_of('/')+1,sourceUrl.size()-sourceUrl.find_last_of('/')-1);
			strcpy(buf, filename.c_str());
			contentKey.content = buf;
			contentKey.contentStoreNetId = netId;
			sess = cpePrx->createSession(contentKey, methodType, TianShanIce::ContentProvision::potDirect, NULL, NULL);
			printf("session[%s] created\n", contentKey.content.c_str());

			TianShanIce::ValueMap resVars;
			TianShanIce::Variant	var;
			var.type = TianShanIce::vtStrings;
			var.bRange = false;
			var.strs.clear();
			var.strs.push_back(contentKey.content);
			resVars[CPHPM_FILENAME] = var;
			var.strs.clear();
			var.strs.push_back(sourceUrl);
			resVars[CPHPM_SOURCEURL] = var;
			sess->addResource(::TianShanIce::SRM::rtURI, resVars);

			resVars.clear();		
			var.type = TianShanIce::vtLongs;
			var.lints.push_back(nBandwidth);
			resVars[CPHPM_BANDWIDTH] = var;
			sess->addResource(::TianShanIce::SRM::rtProvisionBandwidth, resVars);

			sess->setTrickSpeedCollection(trickcol);


			::Ice::Long n = c + interval*1000;	//delay 10 seconds
			std::string startTimeUTC = ZQTianShan::TimeToUTC(n, buf, sizeof(buf) -2);
			ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());

			std::string endTimeUTC = ZQTianShan::TimeToUTC(n+nDurationSecs*1000, buf, sizeof(buf) -2);
			sess->setup(startTimeUTC, endTimeUTC);
			sess->commit();
			printf("session[%s] commited\n", contentKey.content.c_str());

		}
		catch(...){}
	}

	ic->destroy();

	return 1;
}

#ifdef ZQ_OS_MSWIN
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
#endif
