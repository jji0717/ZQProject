// Test.cpp : 定义控制台应用程序的入口点
//

#include "stdafx.h"
#include "Ice/Ice.h"
#include "DataStream.h"
#include <algorithm>
#include <vector>
#include <string>

void printName(const std::string& name)
{
	printf("item: %s\n", name.c_str());

}

void split( const std::string& src, char delimiter, std::vector<std::string>& result)
{
	std::string::const_iterator it, beginPos = src.begin();
	for (it = src.begin(); it != src.end(); it ++) {
		if (*it == delimiter) {
			std::string str(beginPos, it);
			beginPos = it + 1;
			result.push_back(str);
		}
	}

	std::string str(beginPos, it);
	result.push_back(str);
}

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD dw1, dw2;
	dw1 = ~0;
	dw2 = 45;
	int i1 = dw2 - dw2;
	return 0;

	std::vector<std::string> r;
	split("UDP:192.168.80.1:8000;TCP:192.168.80.1:8000", ';', r);
	std::for_each(r.begin(), r.end(), printName);
	std::vector<std::string> r1;
	split(r[0], ':', r1);
	std::for_each(r1.begin(), r1.end(), printName);

	
	Ice::CommunicatorPtr ic = Ice::initialize(argc, argv);
	DataOnDemand::DataStreamServicePrx service = 
		DataOnDemand::DataStreamServicePrx::checkedCast(ic->stringToProxy("DODStreamer:default -p 10000"));
	DataOnDemand::StreamInfo info;	
	DataOnDemand::DataStreamPrx strm, strm2, strm3;

	try {
		strm = service->getStream("stream1");
	} catch(Ice::ObjectNotExistException) {
		strm = service->createStreamByApp(NULL, "stream1", info);
	}

	try {
		strm2 = service->getStream("stream2");
	} catch(Ice::ObjectNotExistException) {
		strm2 = service->createStreamByApp(NULL, "stream2", info);
	}

	try {
		strm3 = service->getStream("strea3");
	} catch(Ice::ObjectNotExistException) {
		strm3 = service->createStreamByApp(NULL, "stream3", info);
	}

	std::string name = strm->getName();
	printf("%s\n", name.c_str());

	DataOnDemand::MuxItemPrx muxItem;
	try {
		muxItem = strm->getMuxItem("item1");
	} catch(Ice::ObjectNotExistException& ) {
		DataOnDemand::MuxItemInfo itemInfo;
		strm->createMuxItem("item1", itemInfo);
	}

	Ice::StringSeq names = strm->listMuxItems();
	std::for_each(names.begin(), names.end(), printName);

	muxItem = strm->getMuxItem("item1");
	name = muxItem->getName();
	printf("%s\n", name.c_str());

	::TianShanIce::Properties props = strm->getProperties();
	printf("stream1 SessionId: %s\n", props["SessionId"].c_str());
	props = strm2->getProperties();
	printf("stream2 SessionId: %s\n", props["SessionId"].c_str());

	props = strm3->getProperties();
	printf("stream3 SessionId: %s\n", props["SessionId"].c_str());
	
	return 0;
}
