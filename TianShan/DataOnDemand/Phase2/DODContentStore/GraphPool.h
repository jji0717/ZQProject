
#ifndef __ZQ_GraphPool_H__
#define __ZQ_GraphPool_H__

#pragma warning (disable : 4786)

#include <list>
#include <map>
#include <deque>

#include "GraphFilter.h"

#include "NativeThread.h"

#define DEFAULT_PROV_PROGRESS_RPT_INTERVAL  30000             // 30s

namespace ZQ { 
namespace Content { 
namespace Process {


class ProvisionRequest;
class Graph;
class GraphPool;
/*
*  Inherited GraphFactory and implement create() to build the specified GraphPool
*/
class GraphFactory
{
	friend class GraphPool;
public:
	GraphFactory(int graphCount, ZQ::common::Log* pLog = NULL, 
		         int buffPoolSize = DEFAULT_BUFFER_POOL_SIZE, int buffSize=DEFAULT_POOL_BUFFER_SIZE)
		         : _graphCount(graphCount), _graphLog(pLog), 
		           _buffPoolSize(buffPoolSize), _buffSize(buffSize) {};

	virtual ~GraphFactory() {};

public:
	virtual Graph* create() = 0;

protected:
	int                  _graphCount;

	ZQ::common::Log*     _graphLog;
	
	int                  _buffPoolSize;
	int                  _buffSize;
};

/*
*  This class ONLY manage one type of Graphs. 
*/
typedef std::deque<ProvisionRequest*> Requests;
typedef std::map<DWORD, ProvisionRequest*>   RequestsMap;

class GraphPool : protected ZQ::common::Mutex, ZQ::common::Semaphore
{
	friend class ProvisionRequest;
	friend class Graph;
	friend class GraphFactory;

public:
	GraphPool(GraphFactory& graphFactory, ZQ::common::Log* pLog = NULL, bool traceDetails = false);
	virtual ~GraphPool();

public:
	bool cancel(std::string contentName);
	void quit();

	int getActiveCount();
	
	Graph* getGraph(const std::string& name);
	ProvisionRequest* getRequest(const std::string& name);

protected:
	void pushRequest(ProvisionRequest& req);

	// write log
	void writeLog(ZQ::common::Log::loglevel_t level, char* fmt, ...);

protected:
	typedef std::vector<Graph*>           Graphs;

	GraphFactory&             _graphFactory;

	Graphs					  _graphs;
	Requests                  _requests;

	RequestsMap               _unFreedRequests;
	bool                      _traceDetails;

	ZQ::common::Log*          _pLog;
};


/*
*  This class is the virtual base class for provision request
*  This class and inherited instance could not be access anymore after
*  OnProvisionCompleted() callback happened.
*/
class ProvisionRequest
{
	friend class GraphPool;
	friend class Graph;
public:	
	ProvisionRequest(GraphPool& pool, bool syncReq, std::string srcURL, std::string cntName, 
		             time_t startTime=0, time_t endTime=0, DWORD maxbps=0, 
					 DWORD progressRptInterval = DEFAULT_PROV_PROGRESS_RPT_INTERVAL, bool stopByEndTime = false, 
					 bool autoFree = true);

protected:
	virtual ~ProvisionRequest();

public: 
	bool start();
	virtual bool init(ZQ::Content::Process::Graph& graph) = 0;

	std::string getContentName() { return _contentName; };

	DWORD getTimeConsumption() { return (_provEndTick - _provStartTick); };

public:
	// virtual function for asynchronised callback 
	virtual void OnProvisionStart() = 0;
	virtual void OnProvisionStreamable() = 0;
	virtual void OnProvisionProcess(__int64 processed, __int64 total) = 0;
	virtual void OnPropertyUpdate(std::map<std::string, ZQ::common::Variant> property) = 0;

	virtual void OnProvisionCompleted(bool bSuccess, int errCode, std::string errStr) = 0;

protected:
	// this is the final routine of this request
	void final();

public: 
	// to sync provision, invoke this function until it return
	bool waitForCompletion(std::string& errstr);
	
	// free myself from graph the Graph pool
	bool free();

	void setStartTimeTick(DWORD startTick) { _provStartTick = startTick; };
	void setEndTimeTick(DWORD endTick)  { _provEndTick = endTick; };

	inline void setStartTime(time_t startTime) {
		_startTime = startTime;
	};
	
	inline void setEndTime(time_t endTime) {
		_endTime = endTime;
	};

protected:
	void notifyCompletion(bool bSuccess, int errCode, std::string errStr);
	
protected: 
	GraphPool&        _graphPool;

	std::string       _sourceURL;
	std::string       _contentName;
	DWORD             _maxbps;
	time_t            _startTime;
	time_t            _endTime;

	DWORD             _progressRptInterval;

private:
	bool              _syncRequest;
	HANDLE            _syncEvent;

	bool              _reqResult;
	int               _reqErrCode;
	std::string       _reqError;

	bool              _autoFree;

	DWORD             _provStartTick;
	DWORD             _provEndTick;

	bool              _stopByEndTime;

};


}}}


#endif