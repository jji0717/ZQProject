#ifndef __MCAST_GRAPH__
#define __MCAST_GRAPH__

#include "GraphPool.h"

#define ZQCP ZQ::Content::Process


namespace {
	const unsigned DEFAULT_BW_CLIENT_ID = 2000;
}


class MCastGraphFactory : public ZQCP::GraphFactory {

public:

	MCastGraphFactory( 
		int graphCount, 
		ZQ::common::Log* logger,
		unsigned int timeout,
		bool dumpFile=false,
		bool oldTrickType = 0,
		unsigned int streamablePlaytime = 10, 
		bool runningOnNode = true,
		int buffPoolSize=DEFAULT_BUFFER_POOL_SIZE, 
		int buffSize=DEFAULT_POOL_BUFFER_SIZE):
		
		ZQCP::GraphFactory(graphCount, logger, buffPoolSize, buffSize),
		_dumpFile(dumpFile),
		_timeout(timeout),
		_streamablePlaytime(streamablePlaytime),
		_oldTrickType(oldTrickType), 
		_runningOnNode(runningOnNode) {
	};
				
	virtual ~MCastGraphFactory(){};

public:
	virtual ZQCP::Graph* create();

private:
	std::string  _localIP;
	bool         _dumpFile;
	unsigned int _timeout;
	bool         _oldTrickType;
	unsigned int _streamablePlaytime;
	bool         _runningOnNode;
};


class MCastRequest : public ZQCP::ProvisionRequest{

public:
	MCastRequest(
			ZQ::common::Log* logger, 
			ZQCP::GraphPool& pool, 
			std::string srcURL, 
			std::string name, 
			time_t start,
			time_t end,
			DWORD maxbps,
			DWORD rptInterval=DEFAULT_PROV_PROGRESS_RPT_INTERVAL,
			bool syncReq=true, 
			bool autoFree=true):
			
			ProvisionRequest(pool, syncReq, srcURL, name, start, end, maxbps, rptInterval, true, autoFree),
			_logger(logger){
	}


protected:
	virtual ~MCastRequest(){};

public:
	virtual bool init(ZQCP::Graph& graph) {
		return true;
	}
	
	
public:
	void OnProvisionStart();

	void OnProvisionStreamable();

	void OnProvisionProcess(__int64 processed, __int64 total);

	void OnPropertyUpdate(std::map<std::string, ZQ::common::Variant> property);

	void OnProvisionCompleted(bool bSuccess, int code, std::string errStr);

private:
	
	ZQ::common::Log* _logger;
};


#endif