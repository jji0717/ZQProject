#include <iostream>
#include <sstream>
#include <fstream>
#include "MCastGraph.h"
#include "FileLog.h"

#include "CommandLine.h"
#include "RTFLibFilter.h"
#include "MCastIOSource.h"


namespace {
	const std::string LOG_NAME = "RTIDemoUtil.log";
}

int main(int argc, char** argv) {
	CommandLine params;

	if(!params.parse(argc, argv)) {
		std::cerr << params.error() << std::endl;
		params.usage();

		return (-1);
	}
	
	if(params.trace()) {
		params._print();
	}

	
	std::string logFile = params.log().empty() ? LOG_NAME : params.log()+"\\"+LOG_NAME;
	ZQ::common::FileLog* logger = 0;
	try {
		logger = new ZQ::common::FileLog(logFile.c_str(), ZQ::common::Log::L_DEBUG);
	}
	catch(const ZQ::common::FileLogException& ex) {
		std::cerr << ex.getString() << std::endl;

		return (-1);
	}


	if(!ZQCP::RTFLibFilter::initRTFLib(1, logger, INPUT_FILE_BUFFER_BYTES_64, DEF_OUTPUT_FILE_BUFFER_BYTES, true, true)) {
		std::cerr << "failed to init RTFLib" << std::endl;

		delete logger;
		return (-1);
	}

	std::string errstr;
	if(!ZQCP::MCastIOSource::initMCastReceiver(params.localIP(), errstr))
	{
		std::cerr << "failed to init MaticastReceiver on " << params.localIP() <<  "with error:" << errstr;
		delete logger;

		return -1;
	}

	int concurrentCount = params.concurrentCount();
	MCastGraphFactory mcastFactory(concurrentCount, logger, params.timeout(), 
									params.dumpFile(), params.oldTrickType(), 
									params.streamablePlaytime(), params.isRunningOnNode());

	ZQ::Content::Process::GraphPool* pool = 
		new ZQ::Content::Process::GraphPool(mcastFactory, logger, params.trace());

	MCastRequest** reqests = new MCastRequest*[concurrentCount];

	char port[10];
	char fileNo[10];

	for(int i=0; i<concurrentCount; i++)
	{
		sprintf(port, ":%d", params.port()+i);
		std::string url = "udp://" + params.IP() + port;
		
		std::string cntName;
		if(1 == concurrentCount)
			cntName = params.name();
		else
		{
			sprintf(fileNo, "_%d", i+1);
			cntName = params.name() + fileNo;
		}
		time_t start = time(0);
		time_t end = start + params.duration() + params.timeout();
		DWORD execTime = (end - start) * 1000;
		reqests[i] = new MCastRequest(
						logger,
						*pool, 
						url, 
						cntName,
						start,
						end,
						execTime,
						0    // no speed limitation now
						);
		
		printf("[%d] Request created for content: %s, URL: %s\n", i+1, cntName.c_str(), url.c_str());

		reqests[i]->start();
		
		if(concurrentCount > 0)
			Sleep(2000);  // each request interval 2 second
	}

	for(i=0; i<concurrentCount; i++)
	{
		std::string str;
		bool res = reqests[i]->waitForCompletion(str);
		if(res) {
			printf("[%d] Request provision completed\n", i+1);
		}
		else {
			printf("[%d] Request provision failed with error: %s\n", i+1, str.c_str());
 		}
	}
	
	ZQCP::RTFLibFilter::uninitRTFLib();
	ZQCP::MCastIOSource::uninitMCastReceiver();

	delete []reqests;
	delete logger;
	
	return 0;
}


int MCastTest() {

//	MCastCapture catcher;
//	
//	if(!catcher.init()) {
//		std::cout << catcher.getLastError() << std::endl;
//		catcher.close();
//		
//		return (-1);
//	}
//
//	if(!catcher.bind("192.168.81.100")) {
//		std::cout << catcher.getLastError() << std::endl;
//		catcher.close();
//		return (-1);
//	}
//
//	if(!catcher.open("225.25.1.1", 1234)) {
//		std::cout << catcher.getLastError() << std::endl;
//		catcher.close();
//		return (-1);
//	}
//
//	const u_int size = 1500;
//	char* buff = new char[size];
//
//	std::ofstream of;
//	of.open("mcast.mpg", std::ios::binary);
//
//	time_t lastPacket, currPacket;
//	
//	lastPacket = currPacket = time(0);
//
//	u_int len = size;
//	while(true) {
//		currPacket = time(0);
//		if(currPacket - lastPacket >= 10) {
//			std::cout << "timeout " << std::endl;	
//
//			break;
//		}
//
//		int res = catcher.recv(buff, &len);
//		
//		if (res == 0) {
//			std::cout << "ok: " << len << std::endl;
//
//			of.write(buff, len);
//
//			lastPacket = time(0);
//		}
//		else if(res == (1)) {
//			continue;
//		}
//		else if (res < 0) {
//			std::cout << "inner res: " << res << "\nbad: " << catcher.getLastError() << std::endl;
//			break;
//		}
//	}
//	
//	of.close();	
// 	catcher.close();
//	delete[] buff;	
//
	return 0;
}