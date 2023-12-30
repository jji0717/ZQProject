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
// Desc  : Define the Content Process GraphPool 
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/TianShan/DataTunnel/source/DataContentStore/GraphPool.cpp 1     10-11-12 16:05 Admin $Log: 
// 


#include "GraphPool.h"


namespace ZQ { 
namespace Content { 
namespace Process {
	
//////////////////////////////////////////////////////////////////////////
//                        Implementation of GraphPool                   //
//////////////////////////////////////////////////////////////////////////

GraphPool::	GraphPool(GraphFactory& graphFactory, ZQ::common::Log* pLog, bool traceDetails)
: _graphFactory(graphFactory), _pLog(pLog), _traceDetails(traceDetails)
{
	// push the graph to pool
	for(int i=0; i<_graphFactory._graphCount; i++)
	{
		Graph* grh = _graphFactory.create();
		if(grh != NULL)
		{
			grh->_traceDetails = _traceDetails;
			grh->_graphPool = this;
			
			_graphs.push_back(grh);

			grh->start();
		}
	}	
}

GraphPool::~GraphPool()
{
	// quit first
	quit();

	// free each graph
	for(Graphs::iterator it = _graphs.begin(); it!=_graphs.end(); it++)
	{
		Graph* pgraph = (Graph*)(*it);
		delete pgraph;
	}
	_graphs.clear();
}

void GraphPool::quit()
{
	ZQ::common::MutexGuard guard(*this);

	// free all the requests
	while(_requests.size() > 0)
	{
		ProvisionRequest* req = _requests.front();
		_requests.pop_front();

		req->final();
	}

	// free the un-freed requests
	RequestsMap::iterator reqit = _unFreedRequests.begin();
	while(reqit != _unFreedRequests.end())
	{
		ProvisionRequest* req = (ProvisionRequest*)reqit->second;

		reqit++;
		
		delete req;
	}
	_unFreedRequests.clear();

	// quit each graph
	for(Graphs::iterator it = _graphs.begin(); it!=_graphs.end(); it++)
	{
		(*it)->quit();
	}
}

void GraphPool::pushRequest(ProvisionRequest& req)
{
	ZQ::common::MutexGuard(*this);
	_requests.push_back(&req);

	post();
}

bool GraphPool::cancel(std::string contentName)
{
	ZQ::common::MutexGuard(*this);

	bool bFound = false;
	std::deque<ProvisionRequest*>::iterator it = _requests.begin();
	while(it != _requests.end())
	{
		ProvisionRequest* req = (ProvisionRequest*)(*it);
		if(req->getContentName() == contentName)
		{
			_requests.erase(it);
			bFound = true;

			req->final();

			writeLog(ZQ::common::Log::L_INFO, "%s does not provision yet, remove it from request pool", contentName.c_str());
			break;
		}
		it++;
	}

	if(!bFound)
	{
		writeLog(ZQ::common::Log::L_DEBUG, "%s is provisioning, find it from GraphPool", contentName.c_str());

		// find the graph
		for(Graphs::iterator it = _graphs.begin(); it!=_graphs.end(); it++)
		{
			if( (*it)->getContentName() == contentName)
			{
				bFound = true;
			
				writeLog(ZQ::common::Log::L_DEBUG, "%s is found, abort the provisioning", contentName.c_str());

				bFound = (*it)->abortProvision();
				break;
			}
		}
		if(!bFound)
		{
			writeLog(ZQ::common::Log::L_INFO, "%s also is not found in GraphPool, must something wrong", contentName.c_str());
		}
	}
	return bFound;
}

int GraphPool::getActiveCount()
{	
	int count = 0;

	for(Graphs::iterator it = _graphs.begin(); it!=_graphs.end(); it++)
	{
		if(!(*it)->hasCompleted())
		{
			count++;
		}
	}	
	return count;
}

Graph* GraphPool::getGraph(const std::string& name) {
	Graphs::iterator iter = _graphs.begin();
	for(; iter != _graphs.end(); ++iter) {
		if((*iter)->getContentName() == name) {
			return (*iter);
		}
	}
	return 0;
}

ProvisionRequest* GraphPool::getRequest(const std::string& name) {
	Requests::iterator iter = _requests.begin();
	for(; iter != _requests.end(); ++iter) {
		if((*iter)->getContentName() == name) {
			return (*iter);
		}
	}
	return 0;
}

void GraphPool::writeLog(ZQ::common::Log::loglevel_t level, char* fmt, ...)
{
	if(_pLog == NULL)
		return;

	char logMsg[510];

	va_list args;
	va_start(args, fmt);
	_vsnprintf(logMsg, 500, fmt, args);
	va_end(args);
	
	char logMsgWID[510];
	_snprintf(logMsgWID, 510, "[0x%08X]: %s", GetCurrentThreadId(), logMsg);
	(*_pLog)(level, logMsgWID);
}
//////////////////////////////////////////////////////////////////////////
//                    Implementation of ProvisionRequest                //
//////////////////////////////////////////////////////////////////////////

ProvisionRequest::ProvisionRequest(GraphPool& pool, bool syncReq, 
								   std::string srcURL, std::string cntName, 
								   time_t startTime, time_t endTime, DWORD maxbps, 
								   DWORD progressRptInterval, bool stopByEndTime, 
								   bool autoFree)
: _graphPool(pool), _syncRequest(syncReq), _sourceURL(srcURL), _contentName(cntName), 
  _startTime(startTime), _endTime(endTime), _maxbps(maxbps), 
  _progressRptInterval(progressRptInterval), _stopByEndTime(stopByEndTime), 
  _autoFree(autoFree),
  _syncEvent(INVALID_HANDLE_VALUE), _reqResult(false), _reqErrCode(0)
{
	if(0 == _progressRptInterval)
	{
		_progressRptInterval = DEFAULT_PROV_PROGRESS_RPT_INTERVAL;
	}
	
	if(_syncRequest)
	{
		_syncEvent = CreateEvent(NULL, false, false, NULL);
	}
}

ProvisionRequest::~ProvisionRequest()
{
	if(INVALID_HANDLE_VALUE != _syncEvent )
	{
		CloseHandle(_syncEvent);
	}
	_syncEvent = INVALID_HANDLE_VALUE;
}

bool ProvisionRequest::start()
{
	_graphPool.pushRequest(*this);
	
	return true;
}

// to sync provision, invoke this function until it return
bool ProvisionRequest::waitForCompletion(std::string& errstr)
{
	if(!_syncRequest)
	{
		errstr = "async content provision, get completion result from OnProvisionCompleted() callback";
		return false;
	}

	DWORD result = WaitForSingleObject(_syncEvent, INFINITE);
	switch(result)
	{
	case WAIT_OBJECT_0:
	case WAIT_TIMEOUT:
		errstr = _reqError;
		return _reqResult;

	default:
		errstr = "Content processing failed";
		return false;
	}
	return false;
}

void ProvisionRequest::final() 
{ 
	if(_autoFree)
	{
		delete this; 
	}
	else
	{
		_graphPool._unFreedRequests.insert(std::make_pair((DWORD)this, this));
	}
}

bool ProvisionRequest::free()
{
	if(!_autoFree)
	{
		RequestsMap::iterator it = _graphPool._unFreedRequests.find((DWORD)this);
		if(it != _graphPool._unFreedRequests.end())
		{
			_graphPool._unFreedRequests.erase(it);

			delete this;

			return true;
		}
	}
	return true;
}

void ProvisionRequest::notifyCompletion(bool bSuccess, int errCode, std::string errStr)
{
	if(_syncRequest)
	{	
		_reqResult = bSuccess;
		_reqErrCode = errCode;
		_reqError = errStr;

		SetEvent(_syncEvent);
	}
}


}}}


