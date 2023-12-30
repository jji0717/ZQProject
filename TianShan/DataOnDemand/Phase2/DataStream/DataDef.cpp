// DataDef.cpp: implementation of the DataDef class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataDef.h"

namespace DataStream {
	
/*DataConfig::DataConfig()
{
	totalRate = 100 * 1024 * 1024; // 100MB
	higherPriority = false;
	profileFlag = 0;

	readerThreadPoolMinSize = 5;
	readerThreadPoolMaxSize = 150;
	senderThreadPoolMinSize = 10;
	senderThreadPoolMaxSize = 150;

	checkStreamTimeout = 70 * 60;

	char path[MAX_PATH];
	GetModuleFileNameA(NULL, path, _COUNTOF(path));
	char* c = path + lstrlen(path);
	while(c >= path) {
		if (*c == '\\') {
			*c = '\0';
			break;
		}
		
		c --;
	}

	lstrcatA(path, "\\ds_cache");
	strncpy(catchDir, path, MAX_PATH);
	
	strcpy(logFile, "DataStream.log");
	logFileSize = 0x6400000; // 100M
	logLevel = (int) ZQLIB::Log::L_DEBUG;
	
	memset(netId, 0, sizeof(netId));

	memset(netWorkcardIP,0, sizeof(MAX_PATH));

	stdPeriod = DEFAULT_STD_PERIOD;
	stdMaxQueue = DEFAULT_STD_MAXQUEUE;
	stdMinQueue = DEFAULT_STD_MINQUEUE;
}

DataConfig::~DataConfig()
{
	
}

DataConfig& DataConfig::getDataConfig()
{
	static DataConfig theDataConfig;
	return theDataConfig;
}*/

//////////////////////////////////////////////////////////////////////////

std::string getFileNamePart(const std::string& path)
{
	
	std::string::const_iterator it = path.end();
	while(it != path.begin()) {
		it --;
		if (*it == '\\' || *it == '/') {
			return std::string(it + 1, path.end());
		}
	}

	return std::string();
}

} // namespace DataStream {
