// Test.cpp : Defines the entry point for the console application.
//

#include "afx.h"
#include "FileLog.h"
#include "TestFactoryAndRequest.h"
#include "VstrmIORender.h"
#include "RTFLibFilter.h"
#include "VstrmIORender.h"

using namespace ZQ::common;
using namespace ZQ::Content::Process;

void FTPGraphTest(ZQ::common::FileLog* flog, int repeatTimes, std::string srcURL, std::string outputName, 
				  std::string server, std::string username, std::string password, DWORD transbps)
{
	FTPGraphFactory FTPFactory(1, flog, server, username, password, 50);
	GraphPool gpool(FTPFactory, false);


	for(int i=0; i<repeatTimes; i++)
	{
		TestProvisionRequest* request = new TestProvisionRequest(flog, gpool, true, srcURL, outputName, 1000, INFINITE, transbps);

		DWORD dwStart = GetTickCount();
		request->start();

		std::string err;
		bool ret = true;
		
		ret = request->waitForCompletion(err);

		DWORD dwEnd = GetTickCount();
		if(!ret)
		{
			printf("File uploading failed with reason: %s\n", err.c_str());
		}
		else
		{
			printf("File uploading succeed, spend %d ms\n", dwEnd-dwStart);
		}
	}
}

void VstrmGraphTest(ZQ::common::FileLog* flog, int repeatTimes, std::string srcURL, std::string outputName, DWORD transbps)
{
	VstrmGraphFactory VstrmFactory(1, flog, 4096);
	GraphPool gpool(VstrmFactory, false);

	std::string errmsg;
	ZQ::Content::Process::VstrmIORender::releaseAllBW(TEST_CLIENT_ID, errmsg, INVALID_HANDLE_VALUE);

	for(int i=0; i<repeatTimes; i++)
	{
		TestProvisionRequest* request = new TestProvisionRequest(flog, gpool, true, srcURL, outputName, 1000, INFINITE, transbps);

		DWORD dwStart = GetTickCount();
		request->start();

		std::string err;
		bool ret = true;
		
		ret = request->waitForCompletion(err);

		DWORD dwEnd = GetTickCount();
		if(!ret)
		{
			printf("File copy to node failed with reason: %s\n", err.c_str());
		}
		else
		{
			printf("[%d] File copy to node succeed, spend %d ms\n", i+1, dwEnd-dwStart);
		}
		
		__int64 filesize = request->getContentSize();
		double bitrate = ((double)(filesize*8)) / ((double)((dwEnd-dwStart)*1000)) ;
		printf("bitrate %f Mbps\n", bitrate);
	

	}
}

void RTFGraphTestNTFS(bool sequential, ZQ::common::FileLog* flog, int repeatTimes, std::string srcURL, std::string outputName, DWORD transbps)
{
	DWORD dwGraphCount = 0;
	if(sequential)
	{
		dwGraphCount = 1;
	}
	else
	{
		dwGraphCount = repeatTimes;
	}

	RTFNTFSGraphFactory rtfFactory(dwGraphCount, flog, 500);
	GraphPool gpool(rtfFactory, true);

	ZQ::Content::Process::RTFLibFilter::initRTFLib(8, flog);

	std::vector<TestProvisionRequest*> requests;
	__int64 filesize = 0;
	DWORD dwStart = 0;
	DWORD totalDuration = 0;
	for(int i=0; i<repeatTimes; i++)
	{
		char ext[10];
		sprintf(ext, "_No.%d", i);
		std::string outputNameEx = outputName + ext;
		TestProvisionRequest* request = new TestProvisionRequest(flog, gpool, true, srcURL, outputNameEx, 1000, INFINITE, transbps, false);

		dwStart = GetTickCount();
		request->start();

		std::string err;
		bool ret = true;
	
		if(sequential)
		{
			ret = request->waitForCompletion(err);
			DWORD dwEnd = GetTickCount();

			if(!ret)
			{
				printf("RTFLib trick file generation failed with reason: %s\n", err.c_str());
			}
			else
			{
				totalDuration += (dwEnd-dwStart);
				filesize += request->getContentSize();
				printf("[%d] RTFLib trick file generation succeed, spend %d ms\n", i+1, dwEnd-dwStart);
			}
			request->free();
		}
		else
		{
			requests.push_back(request);
		}
	}

	if(sequential)
	{
		printf("%d sequential processing spent %d ms\n", repeatTimes, totalDuration);
	}
	else
	{
		std::string err;
		bool ret = true;
		for(int i=0; i<repeatTimes; i++)
		{
			ret = requests[i]->waitForCompletion(err);
			DWORD dwEnd = GetTickCount();

			if(!ret)
			{
				printf("RTFLib trick file generation failed with reason: %s\n", err.c_str());
			}
			else
			{
				totalDuration += (dwEnd-dwStart);
				filesize += requests[i]->getContentSize();
				printf("[%d] RTFLib trick file generation succeed, spend %d ms\n", i+1, dwEnd-dwStart);
			}
			requests[i]->free();
		}
		totalDuration = (DWORD)(totalDuration/repeatTimes);
	}
	double bitrate = ((double)(filesize*8)) / ((double)(totalDuration*1000)) ;

	printf("%d parallel processing spent %d ms, bitrate %f Mbps\n", repeatTimes, totalDuration, bitrate);

	ZQ::Content::Process::RTFLibFilter::uninitRTFLib();
}

void RTFGraphTestVSTRM(bool sequential, ZQ::common::FileLog* flog, int repeatTimes, std::string srcURL, std::string outputName, DWORD transbps)
{
	DWORD dwGraphCount = 0;
	if(sequential)
	{
		dwGraphCount = 1;
	}
	else
	{
		dwGraphCount = repeatTimes;
	}
	std::string errmsg;
	HANDLE vstreamHandle =  INVALID_HANDLE_VALUE; //ZQ::Content::Process::VstrmIORender::initVstrm(errmsg);

//	if(vstreamHandle == INVALID_HANDLE_VALUE)
//	{
//		printf("init vstream handle failed\n");
//		return;
//	}

	RTFVstrmGraphFactory rtfFactory(dwGraphCount, flog, vstreamHandle, 512);
	GraphPool gpool(rtfFactory, false);

	ZQ::Content::Process::VstrmIORender::releaseAllBW(TEST_CLIENT_ID, errmsg, INVALID_HANDLE_VALUE);
	ZQ::Content::Process::RTFLibFilter::initRTFLib(dwGraphCount, flog);

	std::vector<TestProvisionRequest*> requests;
	DWORD dwStart = 0;
	DWORD totalDuration = 0;
	__int64 filesize = 0;
	for(int i=0; i<repeatTimes; i++)
	{
		char ext[10];
		sprintf(ext, "_No.%d", i);
		std::string outputNameEx = outputName + ext;
		TestProvisionRequest* request = new TestProvisionRequest(flog, gpool, true, srcURL, outputNameEx, 10000, INFINITE, transbps, false);

		request->start();

		std::string err;
		bool ret = true;
	
		if(sequential)
		{
			ret = request->waitForCompletion(err);

			if(!ret)
			{
				printf("RTFLib trick file generation failed with reason: %s\n", err.c_str());
			}
			else
			{
				DWORD timecon = request->getTimeConsumption();

				totalDuration += timecon;
				filesize += request->getContentSize();
				printf("[%d] RTFLib trick file generation succeed, spend %d ms\n", i+1, timecon);
			}
			request->free();
		}
		else
		{
			requests.push_back(request);
		}
	}

	if(sequential)
	{
		printf("%d sequential processing spent %d ms\n", repeatTimes, totalDuration);
	}
	else
	{
		std::string err;
		bool ret = true;
		for(int i=0; i<repeatTimes; i++)
		{
			ret = requests[i]->waitForCompletion(err);

			if(!ret)
			{
				printf("RTFLib trick file generation failed with reason: %s\n", err.c_str());
			}
			else
			{
				DWORD timecon = requests[i]->getTimeConsumption();

				totalDuration += timecon;

				printf("[%d] RTFLib trick file generation succeed, spend %d ms\n", i+1, timecon);
			}
			filesize += requests[i]->getContentSize();
			requests[i]->free();
		}
		totalDuration = (DWORD)(totalDuration/repeatTimes);
	}
	double bitrate = ((double)(filesize*8)) / ((double)(totalDuration*1000)) ;

	printf("%d processing spent %d ms, bitrate %f Mbps\n", repeatTimes, totalDuration, bitrate);
	
	ZQ::Content::Process::RTFLibFilter::uninitRTFLib();

	ZQ::Content::Process::VstrmIORender::uninitVstrm(vstreamHandle);
}

void usage()
{
	printf("Input command as following specification\n");
	printf("GraphTest -ftp repeatTimes sourceURL outputName serverIP username password trasmitbps\n");
	printf("GraphTest -vstrm repeatTimes sourceURL outputName trasmitbps\n");
	printf("GraphTest -rtfntfs -M<S,P> repeatTimes sourceURL outputName trasmitbps\n");
	printf("GraphTest -rtfvstrm -M<S,P> repeatTimes sourceURL outputName trasmitbps\n");
}

int main(int argc, char* argv[])
{
    // std::string srcURL = "file://192.168.81.108/D$/TestCS/SourceFile/big_term.mpg";

	if(argc < 5)
	{
		printf("Invalid parameter count(%d) !\n\n", argc);

		usage();
		return 0;
	}

	ZQ::common::FileLog* flog = NULL;

	if(strcmp(argv[1], "-ftp") == 0)
	{
		if(argc != 9)
		{
			printf("Invalid parameter count for ftp type!\n\n");
			usage();
			return 0;
		}
		
		int reptimes = atoi(argv[2]);
		if(reptimes <= 0)
		{
			reptimes = 1;
		}
		
		flog = new ZQ::common::FileLog("GraphTest.log", ZQ::common::Log::L_DEBUG, ZQLOG_DEFAULT_FILENUM, ZQLOG_DEFAULT_FILESIZE*10*10);
		
		std::string sourceURL = argv[3];
		std::string outputName = argv[4];
		std::string serverIP = argv[5];
		std::string username = argv[6];
		std::string password = argv[7];
		DWORD transbps = atol(argv[8]);

		FTPGraphTest(flog, reptimes, sourceURL, outputName, serverIP, username, password, transbps);
	}
	else if(strcmp(argv[1], "-vstrm") == 0)
	{
		if(argc != 6)
		{
			printf("Invalid parameter count for vstrm type!\n\n");
			usage();
			return 0;
		}
		int reptimes = atoi(argv[2]);
		if(reptimes <= 0)
		{
			reptimes = 1;
		}
		
		flog = new ZQ::common::FileLog("GraphTest.log", ZQ::common::Log::L_DEBUG);
		
		std::string sourceURL = argv[3];
		std::string outputName = argv[4];
		DWORD transbps = atol(argv[5]);

		VstrmGraphTest(flog, reptimes, sourceURL, outputName, transbps);
	}
	else if(strcmp(argv[1], "-rtfntfs") == 0)
	{
		if(argc != 7)
		{
			printf("Invalid parameter count for rtflib type!\n\n");
			usage();
			return 0;
		}
		
		bool sequential = true;
		if(strcmp(argv[2], "-MP") == 0)
		{
			sequential = false;
		}

		int reptimes = atoi(argv[3]);
		if(reptimes <= 0)
		{
			reptimes = 1;
		}
		
		flog = new ZQ::common::FileLog("GraphTest.log", ZQ::common::Log::L_DEBUG);
		
		std::string sourceURL = argv[4];
		std::string outputName = argv[5];
		DWORD transbps = atol(argv[6]);

		RTFGraphTestNTFS(sequential, flog, reptimes, sourceURL, outputName, transbps);
	}
	else if(strcmp(argv[1], "-rtfvstrm") == 0)
	{
		if(argc != 7)
		{
			printf("Invalid parameter count for rtflib type!\n\n");
			usage();
			return 0;
		}
		
		bool sequential = false;
		if(strcmp(argv[2], "-MS") == 0)
		{
			sequential = true;
		}

		int reptimes = atoi(argv[3]);
		if(reptimes <= 0)
		{
			reptimes = 1;
		}
		
		flog = new ZQ::common::FileLog("GraphTest.log", ZQ::common::Log::L_DEBUG);
		
		std::string sourceURL = argv[4];
		std::string outputName = argv[5];
		DWORD transbps = atol(argv[6]);

		RTFGraphTestVSTRM(sequential, flog, reptimes, sourceURL, outputName, transbps);
	}
	else
	{
		printf("Invalid type!\n\n");
		usage();
		return 0;
	}

	if(flog != NULL)
	{
		delete flog;
	}

	return 0;
}
