// ============================================================================================
// Copyright (c) 2006, 2007 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Ken Qian
// Desc  : Implement the Content Process Graph and Filter Base class
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/TianShan/DataOnDemand/Phase2/DODContentStore/GraphFilter.cpp 2     12/12/13 1:52p Hui.shao $
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase2/DODContentStore/GraphFilter.cpp $
// 
// 2     12/12/13 1:52p Hui.shao
// %lld
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
// 
// 1     08-12-08 11:11 Li.huang
// 
// 1     08-10-30 16:45 Ken.qian
// Move files from /ZQProjs/Generic/ContentProcess to local folder, since
// files at ContentProcess were never used by others components. And
// remove the pacing codes from NTFSIORender to indepent on Vstrm DLL
// 
// 42    08-05-16 13:58 Fei.huang
// 
// 41    08-01-16 15:20 Fei.huang
// add: adjust session time 
// 
// 40    08-01-02 18:35 Ken.qian
// 1. Add minBuffCount for Graph and as the parameter for BufferPool
// 2. move minAvailBuffCount from SourceFilter to Graph, and change the
// name to reservedCount
// 
// 39    07-12-17 14:44 Ken.qian
// add :: at the front of _vsnprintf to avoid compiling error:
// '_vsnprintf' : ambiguous call to overloaded
// 
// 38    07-12-14 14:01 Ken.qian
// roll back the code changed by Huangli for temporary using
// 
// 37    07-12-14 13:57 Ken.qian
// use _vsnprintf to replace _vsprintf, and adjust the head include order
// to avoid compiling error: '_vsnprintf' : ambiguous call to overloaded
// function
// 
// 36    07-12-14 11:42 Li.huang
// 
// 35    07-11-19 18:07 Fei.huang
// fix the issue of expected execution time not right
// 
// 34    07-11-19 16:21 Fei.huang
// 
// 33    07-11-19 15:20 Ken.qian
// calcute exepected execution time with __int64
// 
// 32    07-11-19 14:43 Ken.qian
// change the logic of expected exection duration in watiforcompletion()
// 
// 31    07-11-19 14:17 Ken.qian
// make the expected execute time calculcated in side graph
// 
// 30    07-11-16 19:13 Ken.qian
// Graph support statechange event, such as Streamable
// 
// 29    07-11-15 15:15 Ken.qian
// when reach the provision end time, invoke source filter endofstream()
// instead of stopProvision()
// 
// 28    07-11-14 14:09 Ken.qian
// add startTime and endTime
// 
// 27    07-11-08 9:26 Ken.qian
// 
// 26    07-10-31 14:28 Ken.qian
// waitForCompletion() return false if program exit, and the invoker check
// the return value
// 
// 25    07-10-23 18:28 Ken.qian
// add mutex in report completion logic for issue of NGOD62
// 
// 24    07-09-17 15:29 Ken.qian
// 1. replace getUnallocatedCount()  with getUsedCount()
// 2. freeUsedBuffers() before a provision
// 
// 23    07-09-11 14:27 Ken.qian
// add waitHandle in Graph quit
// 
// 22    07-08-16 12:46 Ken.qian
// 
// 21    07-08-14 15:50 Ken.qian
// 
// 20    07-08-14 14:31 Ken.qian
// 
// 19    07-08-09 17:10 Ken.qian
// 
// 18    07-08-02 17:07 Ken.qian
// 
// 17    07-07-26 18:08 Ken.qian
// fix vstrm io vvx issue
// 
// 16    07-07-24 15:42 Ken.qian
// 
// 15    07-07-23 15:28 Ken.qian
// 
// 14    07-07-18 19:36 Ken.qian
// fix buffpool issue
// 
// 13    07-07-17 14:18 Ken.qian
// 
// 12    07-06-27 15:13 Ken.qian
// Change kbps to bps
// 
// 11    07-06-14 14:53 Cary.xiao
// 
// 10    07-06-06 15:37 Ken.qian
// 
// 11    07-06-05 21:15 Ken.qian
// add cancel in GraphPool
// 
// 10    07-06-05 18:25 Ken.qian
// add the detail errorcode
// 
// 9     07-05-14 16:32 Ken.qian
// 
// 8     07-04-26 19:50 Ken.qian
// 
// 7     07-04-20 16:33 Ken.qian
// 
// 6     07-04-20 10:36 Ken.qian
// 
// 5     07-04-19 17:31 Ken.qian
// 
// 4     07-04-17 10:53 Ken.qian
// 
// 3     07-04-16 11:04 Ken.qian
// 
// 2     07-04-10 19:17 Ken.qian
// GraphPool Unitest Pass
// 
// 1     07-03-30 19:26 Ken.qian
// 
// 8     06-09-19 11:55 Ken.qian
// 
// 7     06-09-06 15:27 Ken.qian
// 
// 6     06-08-24 11:02 Ken.qian
// 
// 5     06-07-12 18:56 Ken.qian
// 
// 4     06-07-12 12:12 Ken.qian
// 
// 3     06-07-04 10:34 Ken.qian
// 
// 2     06-06-30 11:32 Ken.qian
// Change the abort processing workflow
// 
// 1     06-06-26 14:49 Ken.qian
// Initial Implementation
// 

#include "GraphFilter.h"
#include "GraphPool.h"
#include "TimeUtil.h"
#include <time.h>

namespace ZQ { 
namespace Content { 
/// namespace Process presents the processing to the content, such as reading/writing
namespace Process {

// state string
static char* GRAPH_CONTENT_STATE_STR[] = { "Streamable" };

// bit definition of statechanges
#define STATECHANGE_BIT_STEAMABLE     0x0001



#define MAX_EXPECT_EXEC_TIME          (24*3600*1000)    // one day


// -----------------------------
// class Graph Implementation
// -----------------------------

Graph::Graph(
		ZQ::common::Log* pLog, 
		bool sourceFirst, 
		int buffPoolSize, 
		int buffSize, 
		int minBuffPoolSize, 
		int reservedBuffCount): 
_buffPool(minBuffPoolSize, buffPoolSize, reservedBuffCount, buffSize), 
_orderSourceFirst(sourceFirst), 
_activeSource(NULL), 
_pLog(pLog), 
_bContinue(true),
_provResult(true), 
_progressReporter(NULL), 
_bitStateChange(0), 
_reservedBuffCount(reservedBuffCount),
_expectInterval(INFINITE) {

	_hStop = CreateEvent(NULL, false, false, NULL);
	_hCompleted = CreateEvent(NULL, false, false, NULL);
	_hOtherStateChange = CreateEvent(NULL, false, false, NULL);
	_execIntervalChange = CreateEvent(NULL, false, false, NULL);

	_logHearder = "";
}

Graph::~Graph()
{
	_activeSource = NULL;
	for(SourceFilterMap::iterator it = _sources.begin(); it != _sources.end(); it++)
	{
		SourceFilter* srcFilter = (SourceFilter*)it->second;

		delete srcFilter;
	}
	_sources.clear();

	std::list<Filter*>::iterator iter;
	iter = _renders.begin();
	while(iter != _renders.end())
	{
		std::list<Filter*>::iterator temp = iter;
		iter++;

		delete (*temp);
	}
	_renders.clear();

	if(_hStop != NULL)
	{
		CloseHandle(_hStop);
		_hStop = NULL;
	}

	if(_hCompleted != NULL)
	{
		CloseHandle(_hCompleted);
		_hCompleted = NULL;
	}

	if(_hOtherStateChange)
	{
		CloseHandle(_hOtherStateChange);
		_hOtherStateChange = NULL;
	}

	if(_execIntervalChange) {
		CloseHandle(_execIntervalChange);
		_execIntervalChange = 0;
	}
}

__int64 Graph::getContentSize() 
{ 
	if(_activeSource != NULL) 
		return _activeSource->getTotalStuff(); 
	else
		return 0;
}

SourceFilter* Graph::getSource(std::string protoType)
{
	SourceFilterMap::iterator it = _sources.find(protoType);
	if(it != _sources.end())
	{
		return it->second;
	}

	return _activeSource ;
}

const Filter* Graph::getRender(std::string name)
{
	if("" == name)
		return NULL;

	std::string myName = "";

	// check renders
	std::list<Filter*>::iterator it = _renders.begin();

	for(; it!=_renders.end(); it++)
	{
		myName = (*it)->getMyName();
		if(myName == name)
		{
			return (*it);
		}
	}

	// check source
	if(_activeSource != NULL)
	{
		myName = _activeSource ->getMyName();
		if(myName == name)
		{
			return _activeSource ;
		}
	}
	return NULL;
}

bool Graph::provision(std::string contentURL, std::string contentName, 
					  DWORD maxbps, time_t startTime, time_t endTime,
					  bool orderSrcFirst)
{
	writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "Graph::provision() enter");
	
	// clear the temp map
	_tmpNotifiedList.clear();

	// set the URL previous all source or renders
	// coz the source or renders are using this URL
	_sourceURL = contentURL;
	_maxbps = maxbps;
	_startTime = startTime;
	_endTime = endTime;

	if(_sources.size() == 0)
	{
		writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "Graph::provision() No source filter specified in the graph");
		return false;
	}
	// get URL proto type to set the active source 
	else if(_sources.size() > 1)
	{
		int colonpos = _sourceURL.find_first_of(':');
		std::string proto = _sourceURL.substr(0, colonpos);
		_activeSource = getSource(proto);
	}
	
	_provResult = true;

	// get the file name from contentName
	int pos = std::string::npos;
	if( (pos = contentName.find_last_of('\\')) != std::string::npos
		|| (pos = contentName.find_last_of('/')) != std::string::npos)
	{
		_logHearder = contentName.substr(pos+1);
	}
	else
	{
		_logHearder = contentName;
	}
	_logHearder = "[" + _logHearder + "]";

	writeLog(ZQ::common::Log::L_INFO, id(), "Before Provision - BufferPool: usage: %d / %d [used/total]", 
			  _buffPool.getUsedCount(), _buffPool.getPoolSize());

	_buffPool.TraceUsedBuff();  // trace to screen

	
	// clear the unused buffer
	if(_buffPool.getUsedCount() > 0)
	{
		writeLog(ZQ::common::Log::L_INFO, id(), "Before Provision - Free all used buffer before provision");
		_buffPool.freeUsedBuffers();
	}

	bool begRet = false;

	if(orderSrcFirst)
	{
		// begin the source
		bool bRet = _activeSource ->begin();
		if(!bRet)
		{
			abortProvision();
			return false;
		}
	}
	
	// begin the renders
	std::list<Filter*>::iterator iter;
	iter = _renders.begin();
	while(iter != _renders.end())
	{
		// start processing
		if(!(*iter)->begin())
		{
			abortProvision();
			return false;
		}
		iter++;
	}
	
	if(!orderSrcFirst)
	{
		// begin the source
		bool bRet = _activeSource ->begin();
		if(!bRet)
		{
			abortProvision();
			return false;
		}
	}

	writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "Graph::provision() leave");
	
	return true;		
}

bool Graph::abortProvision(void)
{
	writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "Graph::abortProvision() enter");

	_provResult = false;

	if(_activeSource  != NULL)
	{
		_activeSource ->abort();
	}

	std::list<Filter*>::iterator iter;
	iter = _renders.begin();
	while(iter != _renders.end())
	{
		(*iter)->abort();
		iter++;
	}
	
	writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "Graph::abortProvision() leave");
	
	return true;
}

bool Graph::stopProvision()
{
	writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "Graph::stopProvision() enter");

	_provResult = false;

	if(_activeSource  != NULL)
	{
		_activeSource ->stop();
	}

	std::list<Filter*>::iterator iter;
	iter = _renders.begin();
	while(iter != _renders.end())
	{
		(*iter)->stop();
		iter++;
	}

	writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "Graph::stopProvision() leave");
	
	return true;
}

bool Graph::quit()
{
	writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "Graph::quit() enter");

	// set flag for last provision task
	_provResult = false;

	for(SourceFilterMap::iterator it = _sources.begin(); it != _sources.end(); it++)
	{
		SourceFilter* srcFilter = (SourceFilter*)it->second;
		srcFilter->waitHandle(50);
		srcFilter->quit();
	}

	// quit renders
	std::list<Filter*>::iterator iter;
	iter = _renders.begin();
	while(iter != _renders.end())
	{
		(*iter)->quit();
		(*iter)->waitHandle(50);
		iter++;
	}

	// quit myself
	_bContinue = false;
	SetEvent(_hStop);
	waitHandle(100);

	writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "Graph::quit() leave");
	
	return true;
}


bool Graph::addSource(SourceFilter* src)
{
	if(NULL == src)
	{
		return false;
	}

	_sources.insert(SourceFilterMap::value_type(src->getProtoType(), src));
	_activeSource = src;

	return true;
}

bool Graph::addRender(Filter* render)
{
	if(NULL == render)
	{
		return false;
	}
	
	// check whether the render has been added to the graph
	std::list<Filter*>::iterator iter;
	iter = _renders.begin();
	while(iter != _renders.end())
	{
		if((*iter) == render)
			return true;
		iter++;
	}
	
	_renders.push_back(render);
	return true;
}

bool Graph::setProgressReporter(Filter* render)
{
	if(_progressReporter != NULL || NULL == render)
	{
		return false;
	}

	_progressReporter = render;

	return true;
}

void Graph::notifyCompletion()
{
	// notify waitForCompletion() function.
	SetEvent(_hCompleted);

	writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "All filters complete their process, trigger Graph completion.");

	// clear the temp map
	_tmpNotifiedList.clear();	
}

void Graph::notifyCompletion(Filter& filter)
{
	writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "%s notify its completion to Graph", filter.getMyName().c_str());
	
	// remember that this filter has reported completion
	ZQ::common::MutexGuard guard(_tepNotifiedMutex);
	DWORD address = (DWORD)(&filter);
	_tmpNotifiedList[address] = &filter;

	// if all of filters completed, notify completion.
	if(_tmpNotifiedList.size() == _renders.size()+1)
	{
		notifyCompletion();
	}
}

void Graph::notifyStateChange(Filter& filter, GRAPH_CONTENT_STATE state)
{
	writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "%s notify its StateChange[%s] event", 
		filter.getMyName().c_str(), GRAPH_CONTENT_STATE_STR[state]);

	// this function maybe invoke by different threads, need lock this.
	ZQ::common::MutexGuard guard(_bitSCMutex);
	
	switch(state)
	{
	case CNT_STEAMABLE:

		_bitStateChange = _bitStateChange | STATECHANGE_BIT_STEAMABLE;

		SetEvent(_hOtherStateChange);
		break;
	}
}

void Graph::adjustSessionTime(time_t newStart, time_t newEnd) {
	ZQ::common::MutexGuard guard(_intervalMutex);

	char d[100];
	ZQ::common::TimeUtil::Time2Str(newStart, d, 100);
	writeLog(ZQ::common::Log::L_DEBUG, id(),
			"graph: newStart: %u\tnewStart: %s",
			newStart, d);

	_startTime = newStart;
	
	/* postpone */
	if(newEnd > _endTime) {
		writeLog(ZQ::common::Log::L_DEBUG, 
					id(), 
					"stop time of session (%s) is postponed for (%us)", 
					_contentName.c_str(),
					newEnd - _endTime);

		_expectInterval = _expectInterval + (newEnd - _endTime)*1000;
	}
	/* ahead of time */
	else if(newEnd < _endTime) {
		writeLog(ZQ::common::Log::L_DEBUG, 
				id(), 
				"stop time of session (%s) is ahead for (%us)", 
				_contentName.c_str(),
				_endTime - newEnd);

		_expectInterval = _expectInterval - (_endTime - newEnd)*1000;
	}
	else {
		writeLog(ZQ::common::Log::L_INFO,
				id(),
				"stop time of session (%s) not changed",
				_contentName.c_str());
		return;

	}

	_endTime = newEnd;
	
	SetEvent(_execIntervalChange);
}

bool Graph::hasCompleted()
{
	// begin the renders
	std::list<Filter*>::iterator iter;
	iter = _renders.begin();
	while(iter != _renders.end())
	{
		if((*iter)->isProcessing())
			return false;
		iter++;
	}
	if(_activeSource->isProcessing())
		return false;

	return true;
}

bool Graph::waitForCompletion(ProvisionRequest& request)//, DWORD expectedInterval)
{
	writeLog(ZQ::common::Log::L_DEBUG, id(), "Graph::waitForCompletion() enter");
	
	// get the start time point of this provision
	DWORD dwStart = GetTickCount();
	DWORD dwEnd = dwStart;
	DWORD dwLastProgressReportTime = dwStart;
	
	__int64 processedStuff = 0;
	__int64 totalStuff = 0;

	_bitStateChange = 0;

	HANDLE handles[4] = {_hStop, _hOtherStateChange, _hCompleted, _execIntervalChange};
	
	// set timeout
	DWORD timeout = request._progressRptInterval;
	
	bool bIsStopping = false;
	
	// block here until the task completion
	while(_bContinue)
	{
		DWORD dwStatus = WaitForMultipleObjects(4, handles, FALSE, timeout);
		switch(dwStatus)
		{
		case WAIT_OBJECT_0:
			// quit, return false here. 
			return false;
	
		case WAIT_OBJECT_0 + 1:   // handle _hOtherStateChange was set
			if(_bitStateChange & STATECHANGE_BIT_STEAMABLE)
			{
				// fire stream event
				request.OnProvisionStreamable();

				// reset the timeout
				timeout = 1; 
			}
			break;
			
		case WAIT_OBJECT_0 + 2:   // handle _hCompleted was set
			// provision completed.
			writeLog(ZQ::common::Log::L_INFO, id(), "After Provision - BufferPool: usage: %d / %d [used/total]", 
					  _buffPool.getUsedCount(), _buffPool.getPoolSize());

			_buffPool.TraceUsedBuff();  // trace to screen

			writeLog(ZQ::common::Log::L_DEBUG, id(), "Graph::waitForCompletion() leave");
			return true;

		case WAIT_OBJECT_0 + 3:
			{
			char stopTime[100], startTime[100];
			ZQ::common::TimeUtil::Time2Str(getStartTime(), startTime, 100);
			ZQ::common::TimeUtil::Time2Str(getEndTime(), stopTime, 100);

			writeLog(ZQ::common::Log::L_INFO, 
						id(), 
						"session (%s) time changed [start: (%s) stop: (%s) interval: (%u)]",
						_contentName.c_str(), 
						startTime,
						stopTime, 
						getExecInterval());

			timeout = 0;
			break;
			}
		case WAIT_TIMEOUT:
			//
			// trace details
			//
			writeLog(ZQ::common::Log::L_DEBUG, id(), "BufferPool: usage: %d / %d [used/total]", 
					  _buffPool.getUsedCount(), _buffPool.getPoolSize());
			//
			// check in another way, have the process complete?
			//
//			if(hasCompleted())
//			{
//				notifyCompletion();
//				continue;
//			}

			//
			// Take the chance to update properties if there is, and cleanup after updating.
			//
			{
				ZQ::common::MutexGuard guard(_pryMutex);
				if(_curProperties.size() > 0) 
				{
					request.OnPropertyUpdate(_curProperties);
					_curProperties.clear();
				}
			}


			dwEnd = GetTickCount(); 
			dwEnd = dwEnd > dwStart ? dwEnd : dwStart;
						
			//
			// if specify expectedInterval, means stop the job if it is still in processing  
			// 
			if( (getExecInterval() < MAX_EXPECT_EXEC_TIME || getExecInterval() > 0 ) && !bIsStopping)  
			{
				//
				// does the process time reaches the expected stop time
				//				
				DWORD curInterval = dwEnd - dwStart;

				writeLog(ZQ::common::Log::L_DEBUG, 
							id(),
							"(%ums) passed, (%ums) interval",
							curInterval,
							getExecInterval());
				if(curInterval >= getExecInterval())
				{
					bIsStopping = true;

					writeLog(ZQ::common::Log::L_NOTICE, id(), "Provision is going to actively stop, expected duration: %d ms, actual duration: %d ms", 
						getExecInterval(), (dwEnd - dwStart));

					// stop processing actively
					_activeSource->endOfStream();

					// does not return directly, waiting for all filter's completion of processing
					continue;
				}
				else if( (curInterval + timeout) >= getExecInterval())			
				{
					timeout = getExecInterval() - curInterval ;
				}
				else
				{
					timeout = request._progressRptInterval;
				}
			}
			else
			{
				timeout = request._progressRptInterval;
			}

			// report the progress
			// get current processed stuff.
			if(_progressReporter != NULL)
			{
				processedStuff = _progressReporter->getProcessedStuff();
			}
			totalStuff = _activeSource ->getTotalStuff();

			writeLog(ZQ::common::Log::L_INFO, id(), "Current progress: processed stuff %lld, total stuff %lld, %d ms elapsed", 
					 processedStuff, totalStuff, (dwEnd - dwStart) );
			// in request, OnProvisionProcess() could not block too much time.
			request.OnProvisionProcess(processedStuff, totalStuff);

			dwLastProgressReportTime = dwEnd;
			
			break;
		default:
			break;
		}
	}

	// reset log header
	_logHearder = "";
	
	return true;
}

int Graph::run(void)
{
	writeLog(ZQ::common::Log::L_DEBUG, id(), "Graph::run() enter");
	ProvisionRequest* pRequest = NULL;
	// block here until the task completion
	while(_bContinue)
	{
		//
		// waiting for the request
		//
		while(_bContinue && _graphPool->_requests.empty())
		{
			_graphPool->wait();
		}
		
		//
		// get one request
		//
		{
			ZQ::common::MutexGuard guard(*_graphPool);
			if(!_graphPool->_requests.empty())
			{
				pRequest = _graphPool->_requests.front();
				_graphPool->_requests.pop_front();

				// must set here, since cancel operation need search the processing graph by name
				_contentName = pRequest->_contentName;
			}
			else
			{
				pRequest = NULL;
			}
		}
		
		//
		// check pRequest pointer
		//
		if(NULL == pRequest || !_bContinue)
		{
			continue;
		}

		//
		// request initialize
		//
		if(!pRequest->init(*this))
		{
			writeLog(ZQ::common::Log::L_ERROR, id(), "Provision Request initialize failed.");
			continue;
		}
		//
		// begin provision
		//
		std::string tempContentName = pRequest->_contentName;

		DWORD dwStart = GetTickCount();

		writeLog(ZQ::common::Log::L_INFO, id(), "Provision request: SourceURL=%s, ContentName=%s, maxbps=%d", 
			     pRequest->_sourceURL.c_str(), pRequest->_contentName.c_str(), pRequest->_maxbps);
	
		pRequest->setStartTimeTick(dwStart);

		// notify request callback
		pRequest->OnProvisionStart();

		DWORD expectedExecTime = MAX_EXPECT_EXEC_TIME;

		if(!provision(pRequest->_sourceURL, 
				      pRequest->_contentName, 
					  pRequest->_maxbps, 
					  pRequest->_startTime,
					  pRequest->_endTime,
					  _orderSourceFirst) )
		{			
			// wait for all the filters complete its abort operation
			if(waitForCompletion(*pRequest))//, expectedExecTime))
			{   // check whether the function returned caused by program quit. 
				//
				// notify request that it is done and free the request object
				//
				pRequest->notifyCompletion(false, _errorCode, _lastError);
				pRequest->OnProvisionCompleted(false, _errorCode, _lastError);
				pRequest->final();

				pRequest = NULL;

				writeLog(ZQ::common::Log::L_WARNING, id(), "provision failed at the beginning");

				// reset log header
				_logHearder = "";

				// continue to process next coming request
				continue;
			}
		}

		writeLog(ZQ::common::Log::L_DEBUG, id(), "provision start for content %s", tempContentName.c_str());

		//
		// update properties which triggered at provision starting, and clear current properies map
		//
		{
			ZQ::common::MutexGuard guard(_pryMutex);
			if(_curProperties.size() > 0) 
			{
				pRequest->OnPropertyUpdate(_curProperties);
				_curProperties.clear();
			}
		}
		
		//
		// Wait for the completion of content processing
		//
		time_t tnow = time(0);
		if(pRequest->_stopByEndTime && pRequest->_endTime > tnow)
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			
			__int64 endms = getEndTime()*1000;
			__int64 nowms = tnow*1000+st.wMilliseconds;
			expectedExecTime = (DWORD)(endms - nowms);
			setExecInterval(expectedExecTime);

			writeLog(ZQ::common::Log::L_INFO, id(), 
					"stop time (%lld.000) current time (%lld.%d) duration (%ums)", 
				     getEndTime(), 
					 tnow, 
					 st.wMilliseconds, 
					 expectedExecTime);
		}
		
		if(waitForCompletion(*pRequest))//, (DWORD)expectedExecTime))
		{   // check whether the function returned caused by program quit. 

			pRequest->setEndTimeTick(GetTickCount());

			//
			// update properties, and clear current properies map
			//
			{
				ZQ::common::MutexGuard guard(_pryMutex);
				if(_curProperties.size() > 0) 
				{
					pRequest->OnPropertyUpdate(_curProperties);
					_curProperties.clear();
				}
			}
			// log time consume
			DWORD dwEnd = GetTickCount();
			writeLog(ZQ::common::Log::L_INFO, id(), "Graph provision spend %d ms", dwEnd - dwStart);

			// reset the _contentName
			{
				ZQ::common::MutexGuard guard(*_graphPool);
				_contentName = "";
			}

			//
			// Notify request completion and free the object
			//
			pRequest->notifyCompletion(_provResult, _errorCode, _lastError);
			pRequest->OnProvisionCompleted(_provResult, _errorCode, _lastError);
			pRequest->final();

			if(_provResult)
			{
				writeLog(ZQ::common::Log::L_INFO, id(), "provision completed for content %s successfully", tempContentName.c_str());
			}
			else
			{
				writeLog(ZQ::common::Log::L_INFO, id(), "provision failed for content %s with error: %s", tempContentName.c_str(), _lastError.c_str());
			}
		}
		_logHearder = "";

		pRequest = NULL;
	}
	
	writeLog(ZQ::common::Log::L_DEBUG, id(), "Graph::run() leave");
	return true;
}

void Graph::writeLog(ZQ::common::Log::loglevel_t level, DWORD tid, char* fmt, ...)
{
	if(_pLog == NULL)
		return;

	char logMsg[510];

	va_list args;
	va_start(args, fmt);
	::_vsnprintf(logMsg, 500, fmt, args);
	va_end(args);
	
	char logMsgWID[510];
	_snprintf(logMsgWID, 510, "%s [0x%08X]: %s", _logHearder.c_str(), tid, logMsg);
	(*_pLog)(level, logMsgWID);
}

void Graph::traceLog(DWORD tid, char* fmt, ...)
{
	if(_traceDetails)
	{
		if(_pLog == NULL)
			return;

		char logMsg[510];

		va_list args;
		va_start(args, fmt);
		::_vsnprintf(logMsg, 500, fmt, args);
		va_end(args);
		
		char logMsgWID[510];
		_snprintf(logMsgWID, 510, "%s [0x%08X]: --- TRACE ---- %s", _logHearder.c_str(), tid, logMsg);
		(*_pLog)(ZQ::common::Log::L_DEBUG, logMsgWID);
	}
}

void Graph::reportProperty(const std::string fileName, ContentProperty properies)
{
	// currently only save the content file properties
	if(fileName != _contentName)
	{
		return;
	}

	ZQ::common::MutexGuard guard(_pryMutex);

	PropertyMap::iterator it = properies.begin();

	while(it != properies.end())
	{
		_curProperties[it->first] = it->second;
		it++;
	}
}
// -----------------------------
// class Filter Implementation
// -----------------------------


Filter::Filter(ZQ::Content::Process::Graph& graph, std::string myName)
:_graph(graph), _pool(graph.getBuffPool()), _myName(myName), _copyUplinkDataBuff(true)
{
	// set the initial status
	_processStatus = STOPPED;
}

Filter::~Filter(void)
{
}

void Filter::deliverBuffer(ZQ::Content::BufferData* buff)
{
	std::list<Filter*>::iterator iter = _desObjs.begin();
	while(iter != _desObjs.end())
	{
		(*iter)->receive(this, buff);
		iter++;
	}
}

bool Filter::releaseBuffer(ZQ::Content::BufferData* buff, Filter* render)
{
	// release buff according specified filter
	bool bReleased = false;
	if(render != NULL)
	{
		if(this == render || render->_copyUplinkDataBuff)
		{
			_pool.free(buff);
			bReleased = true;
		}
		return bReleased;
	}

	bool hasDownLink = false;
	
	bool bNeedRelease = true;
	// release buff according the downlink filters
	std::list<Filter*>::iterator iter = _desObjs.begin();
	while(iter != _desObjs.end())
	{
		hasDownLink = true;

		if( !(*iter)->_copyUplinkDataBuff )
		{
			bNeedRelease = false;
		}
		iter++;
	}
	// if everybody connected are using copying data mode, just release it.
	// if this filter does not have downlink filters, just release it.
	if(bNeedRelease || !hasDownLink)
	{
		_pool.free(buff);
		bReleased = true;
	}
	
	return bReleased;
}


void Filter::notifyEndOfStream(void)
{
	std::list<Filter*>::iterator iter = _desObjs.begin();
	while(iter != _desObjs.end())
	{
		(*iter)->endOfStream();
		iter++;
	}
}

void Filter::deliverContentProperty(std::string property, ZQ::common::Variant value)
{
	std::list<Filter*>::iterator iter = _desObjs.begin();
	while(iter != _desObjs.end())
	{
		(*iter)->setProperty(property, value);
		iter++;
	}
}

bool Filter::connectTo(Filter* obj, bool copyBuff)
{
	if(NULL == obj)
	{
		return false;
	}
	std::list<Filter*>::iterator iter = _desObjs.begin();
	while(iter != _desObjs.end())
	{
		// check whether this obj has been connected before
		if(obj == (*iter))
		{
			return false;
		}
		iter++;
	}
	obj->_copyUplinkDataBuff = copyBuff; 
	_desObjs.push_back(obj);

	// the destination object to Graph
	_graph.addRender(obj);

	return true;
}

void Filter::disconnect(Filter* obj)
{
	if(NULL == obj)
	{
		return;
	}
	std::list<Filter*>::iterator iter = _desObjs.begin();
	while(iter != _desObjs.end())
	{
		// check whether this obj has been connected before
		if(obj == (*iter))
		{
			_desObjs.remove(obj);
		}
		iter++;
	}	
}

bool Filter::isActive()
{
	if(_processStatus == ACTIVE)
		return true;
	return false;
}

bool  Filter::isProcessing()
{
	if(_processStatus != STOPPED && _processStatus != ABORTED)
		return true;
	return false;
}

void Filter::setProperty(std::string property, ZQ::common::Variant value)
{
	_srcProperty[property] = value;
}

bool Filter::getProperty(std::string property, ZQ::common::Variant& value)
{
	PropertyMap::iterator it;
	if( (it = _srcProperty.find(property)) != _srcProperty.end())
	{
		value = it->second;
		return true;
	}

	return false;
}

SourceFilter::SourceFilter(ZQ::Content::Process::Graph& graph, std::string protoType, std::string myName)
: Filter(graph, myName)
{
	_protoType = protoType;

	// add this source to graph
	graph.addSource(this);
}

SourceFilter::~SourceFilter()
{
}

}}}