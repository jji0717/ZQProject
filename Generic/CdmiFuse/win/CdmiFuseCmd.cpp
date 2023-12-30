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
// Ident : $Id: CdmiFuseCmd.cpp Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : FUSE implementation for Windows based on Dokan
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/CdmiFuse/win/CdmiFuseCmd.cpp $
// 
// 24    8/17/15 3:19p Li.huang
// 
// 23    2/24/14 1:46p Hui.shao
// to call AquaClient in a DLL
// 
// 22    7/19/13 3:57p Hui.shao
// install/remove driver
// 
// 21    6/27/13 1:24p Li.huang
// 
// 20    6/19/13 4:51p Li.huang
// 
// 19    6/19/13 4:44p Li.huang
// 
// 18    6/19/13 3:36p Li.huang
// 
// 17    6/13/13 6:10p Hui.shao
// flags
// 
// 16    5/21/13 3:39p Li.huang
// 
// 15    5/10/13 4:24p Li.huang
// 
// 14    4/19/13 2:22p Li.huang
// 
// 13    4/19/13 11:31a Hui.shao
// 
// 12    4/18/13 6:03p Hui.shao
// to support multiple mounting
// 
// 11    2/28/13 2:33p Hui.shao
// 
// 10    2/25/13 4:12p Hui.shao
// 
// 9     2/07/13 2:52p Hui.shao
// usage description
// 
// 8     1/23/13 11:29a Hui.shao
// covered dos commands: dir, mkdir, copy, freespace
// 
// 7     1/22/13 11:37a Hui.shao
// 
// 6     1/09/13 4:57p Hui.shao
// 
// 5     1/06/13 4:22p Hui.shao
// drafted some entries for CdmiDokan
// 
// 4     12/26/12 6:18p Hui.shao
// log and DokanNest
// 
// 3     12/26/12 12:24p Hui.shao
// 
// 2     12/24/12 5:41p Hui.shao
// 
// 1     12/24/12 5:10p Hui.shao
// created
// ===========================================================================

#include "CdmiDokan.h"
#include "FileLog.h"
#include "getopt.h"
#include "strHelper.h"


extern "C"
{
#include <stdio.h>
}

void usage(char* progName)
{
	char* p = strrchr(progName, FNSEPC);
	if (NULL !=p) progName=++p;
	printf("Usage: %s [OPTIONS] -m <DriveLetter>\n", progName);
	printf("       %s [-h]\n", progName);
	printf("Windows File System in User Space to CDMI Server\n\n");
	printf("Options:\n");
	printf(	"    -h                display this help\n"
		"    -m <mount-spec>   specify to mount a CDMI drive\n"
		"    -t {2..20}        count of processing threads, i.e. -t 5\n"
		"    -l <logdir>       the path name where the log file(s) to put under,\n"
		"                      default .\\\n"
		"    -d {0..7}         to specify log level, i.e. -d 3\n"
		"    -f                flag Dump MsgBody ,HexDump, i.e: 199653(0x3000f)\n"
		"    -c                flag print curlclient send ,info ,recviver log, i.e: 15(0x0f)\n"
		"\n  where <mount-spec> is in the format of\n"
		"        <DriveLetter> '=' <CdmiURL> ',' <container> [;<mount-spec>]\n"
		"    for example:\n"
		"        m=http://user:password@server1:2364/,MyCDMIContainer/  or\n"
		"        m=http://u1:p1@svr1/,cont1/;n=http://u2:p2@svr2/,share/\n"
		);
}
// DEBUG option: -l d:\temp\ -d 7 -m x=http://cstest:cstest@172.16.20.131:8080/aqua/rest/cdmi/,MyContainerCDMI_HL005
ZQ::common::NativeThreadPool thpool;

// #define CDMI_API_TEST_ONLY

#ifdef CDMI_API_TEST_ONLY
#  include "CdmiFuseTest.cpp"
#endif // CDMI_API_TEST_ONLY

bool bQuit = false;

BOOL WINAPI ConsoleHandler(DWORD cEvent)
{
	switch(cEvent) {
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

typedef struct _FuseDescriptor
{
	char driveLetter;
	std::string url;
	DokanFuse*  pfuse;
	ZQ::common::FileLog* plog;

	_FuseDescriptor() : driveLetter(0), pfuse(NULL), plog(NULL) {}

	virtual ~_FuseDescriptor()
	{
		if (pfuse)
			delete pfuse;
		pfuse = NULL;

		if (plog)
			delete plog;
		plog = NULL;
	}

} FuseDescriptor;

typedef std::map<char, FuseDescriptor> FuseMap;

int main(int argc, char* argv[])
{
	uint32 dokanOptions =0;
	uint threadCount =0;
	uint logLevel = 6;

	uint32 cdmiflags =0, curlflags = ZQ::common::CURLClient::sfTraceSend;

	std::string mountSpec;
	std::string logdir =".\\";

	// parse the command options
	if (argc <2)
	{
		usage(argv[0]);
		return -1;
	}

	int ch;
	while((ch = getopt(argc, argv, "hl:t:d:nm:vec:f:ri")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			usage(argv[0]);
			return 0;

		case 'm':
			mountSpec = optarg ? optarg :"";
			break;

		case 'l':
			logdir = optarg ? optarg : ".";
			if (!logdir.empty() && '\\' != logdir[logdir.length()-1])
				logdir +="\\";
			break;

		case 't':
			threadCount = (uint)atoi(optarg);
			break;

		case 'd':
			logLevel = atoi(optarg);
			break;

		case 'n':
			dokanOptions |= DOKAN_OPTION_NETWORK;
			break;

		case 'v':
			dokanOptions |= DOKAN_OPTION_REMOVABLE;
			break;

		case 'e':
			dokanOptions |= DOKAN_OPTION_DEBUG | DOKAN_OPTION_STDERR;
			break;

		case 'f':
			cdmiflags = atoi(optarg);
			break;

		case 'c':
			curlflags = atoi(optarg);
			break;

		case 'i':
			DokanFuse::installDriver();
			return 0;

		case 'r':
			DokanFuse::removeDriver();
			return 0;

		default:
			fprintf(stderr, "unknown option: %c\n", ch);
			return -1;
		}
	}

	dokanOptions |= DOKAN_OPTION_KEEP_ALIVE | DOKAN_OPTION_DEBUG;
	// logdir = "d:\\temp\\";
	// mountSpec = "m=d:\\temp\\cdmiroot\\;n=http://cstest:cstest@172.16.20.131:8080/aqua/rest/cdmi/,MyContainerCDMI_HL005";

	CdmiFuseOps::startCURLenv();
	FuseMap fuseMap;

	// parse the mount spec line
	size_t posToken;
	do {
		std::string mountTokan;
		posToken = mountSpec.find(';');
		if (std::string::npos != posToken)
		{
			mountTokan = mountSpec.substr(0, posToken);
			mountSpec = mountSpec.substr(posToken+1);
		}
		else
		{
			mountTokan = mountSpec;
			mountSpec = "";
		}

		size_t posMapping = mountTokan.find('=');
		if (std::string::npos == posMapping || posMapping <1)
			continue;
		FuseDescriptor fused;
		fused.pfuse = NULL;
		fused.driveLetter = toupper(mountTokan[0]);
		fused.url = mountTokan.substr(posMapping+1);

		if (isalpha(fused.driveLetter))
			MAPSET(FuseMap, fuseMap, fused.driveLetter, fused); 
	} while (std::string::npos != posToken);

#ifdef CDMI_API_TEST_ONLY
	//	CdmiFuseTest fuseTester(log, thpool, mountPoint, "http://10.15.10.50:2364/MyContainerCDMI/", dokanOptions, "", threadCount);
	CdmiFuseTest fuseTester(log, thpool, mountPoint, rootUrl, dokanOptions, "", threadCount);
	fuseTester.testInitMount();
	fuseTester.testDosDir();
	fuseTester.testDosCopyFileInto();
	fuseTester.testDosCopyFileOut();

	return 0;
#endif // CDMI_API_TEST_ONLY

	// CdmiDokan fuse(log, thpool, mountPoint, rootUrl, containerName, dokanOptions, "", threadCount);
	// DokanFuse fuse(log, mountPoint, "d:\\temp\\cdmiroot\\", dokanOptions, "", threadCount);

	// start each fuse instance
	for (FuseMap::iterator it= fuseMap.begin(); it != fuseMap.end(); it++)
	{
		char drive[]="m:";
		drive[0] = it->second.driveLetter;

		char logpath[MAX_PATH];
		try {
			snprintf(logpath, sizeof(logpath)-2, "%sCdmiFuse_%c.log", logdir.c_str(), it->second.driveLetter);
			it->second.plog = new ZQ::common::FileLog(logpath, logLevel);
		}
		catch(const ZQ::common::Exception& ex)
		{
			fprintf(stderr, "skip %s per failed to open logfile[%s]: %s\n", drive, logpath, ex.getString());
		}
		catch(...)
		{
			fprintf(stderr, "skip %s per failed to open logfile[%s]\n", drive, logpath);
		}

		if (NULL == it->second.plog)
			continue;
		(*it->second.plog)(ZQ::common::Log::L_NOTICE, CLOGFMT(CdmiFuse, "===== starting FUSE: MP[%s]=>URL[%s], opt[%x]"), drive, it->second.url.c_str(), dokanOptions);

		if (0 == it->second.url.compare(0, sizeof("http://")-1, "http://"))
		{
			// cdmi fuse mode
			size_t posContainer = it->second.url.find(',');
			std::string rootURL = it->second.url, container;
			if (std::string::npos != posContainer)
			{
				rootURL = it->second.url.substr(0, posContainer);
				container = it->second.url.substr(posContainer+1);
			}

			it->second.pfuse = new CdmiDokan((*it->second.plog), thpool, drive, rootURL, "/cdmi_domains/default/", container, dokanOptions, (cdmiflags<<16) | (curlflags&0xffff), "", threadCount);
		}
		else
			it->second.pfuse = new DokanFuse((*it->second.plog), drive, it->second.url, dokanOptions, "", threadCount);

		if (it->second.pfuse)
			it->second.pfuse->start();

		Sleep(100);
	}

	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE) ConsoleHandler, TRUE)==FALSE)
	{
		fprintf(stderr, "Unable to install handler!\n");
		CdmiFuseOps::stopCURLenv();
		return -1;
	}

	while(!bQuit && DokanFuse::runningCount()>0)
		Sleep(100);

	//	Sleep(10000);
	for (FuseMap::iterator it= fuseMap.begin(); it != fuseMap.end(); it++)
	{
		if (it->second.pfuse)
			it->second.pfuse->stop();
	}

	CdmiFuseOps::stopCURLenv();
	printf("FUSE quits\n");

	return 0;

	/*	int status = fuse.perform();

	switch (status) {
	case DOKAN_SUCCESS:
	log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuse, "FUSE: MP[%s]=>URL[%s] quit"), mountPoint.c_str(), rootUrl.c_str());
	break;

	case DOKAN_ERROR:
	log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuse, "FUSE: MP[%s]=>URL[%s] failed"), mountPoint.c_str(), rootUrl.c_str());
	break;

	case DOKAN_DRIVE_LETTER_ERROR:
	log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuse, "FUSE: MP[%s]=>URL[%s] failed at bad drive letter"), mountPoint.c_str(), rootUrl.c_str());
	break;

	case DOKAN_DRIVER_INSTALL_ERROR:
	log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuse, "FUSE: MP[%s]=>URL[%s] failed at installing driver"), mountPoint.c_str(), rootUrl.c_str());
	break;
	case DOKAN_START_ERROR:
	log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuse, "FUSE: MP[%s]=>URL[%s] failed at driver error"), mountPoint.c_str(), rootUrl.c_str());
	break;
	case DOKAN_MOUNT_ERROR:
	log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuse, "FUSE: MP[%s]=>URL[%s] failed at conflict drive letter"), mountPoint.c_str(), rootUrl.c_str());
	break;
	case DOKAN_MOUNT_POINT_ERROR:
	log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuse, "FUSE: MP[%s]=>URL[%s] failed at bad mount point"), mountPoint.c_str(), rootUrl.c_str());
	break;
	default:
	log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuse, "FUSE: MP[%s]=>URL[%s] failed at err(%d)"), mountPoint.c_str(), rootUrl.c_str(), status);
	break;
	}
	*/

	CdmiFuseOps::stopCURLenv();
	return 0;
}
