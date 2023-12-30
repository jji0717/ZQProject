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
// Name  : WorkFactory.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-5-9
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/WorkNode/WorkFactory.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 10    05-06-10 14:00 Bernie.zhao
// 
// 9     05-06-07 18:08 Bernie.zhao
// added sysTray support
// 
// 8     05-05-25 17:18 Bernie.zhao
// 
// 7     05-05-25 14:03 Bernie.zhao
// 
// 6     05-05-19 11:13 Bernie.zhao
// 
// 5     05-05-17 22:24 Bernie.zhao
// added log method
// 
// 4     05-05-14 12:13 Bernie.zhao
// 
// 3     05-05-11 22:49 Bernie.zhao
// May/11.  Huge modification since Hongye leave
// 
// 2     05-05-11 13:48 Bernie.zhao
// got rid of hardcoded parts
// ===========================================================================


#include "stdafx.h"
#include "WorkFactory.h"
#include "TaskAcceptor.h"
#include "Daemon.h"

#include "MPFLogHandler.h"
using namespace ZQ::common;
		
#include "MPFCommon.h"
using namespace ZQ::MPF::utils;

MPF_WORKNODE_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WorkFactory::WorkFactory(TaskAcceptor* pTaskAcpt)
{
	_pTaskAcpt = pTaskAcpt;
	_pluginPathStr = "N/A";
	_pluginVendorStr = "Embeded default factory";
}

WorkFactory::~WorkFactory()
{
	Guard<Mutex> tmpGd(_wrkLock);
	while(!_workers.empty())
	{
		BaseWork* exWork = _workers.back();
		delete exWork;
		_workers.pop_back();
	}
}

bool WorkFactory::reg(BaseWork* param_Work)
{
	bool bRet=TRUE;

	if(!param_Work || !workTypeAvailable(param_Work->type()) )
	{	// not available for this type
		bRet = FALSE;
		return bRet;
	}

	// reg it into stack
	Guard<Mutex> tmpGd(_wrkLock);
	
	vector<BaseWork*>::const_iterator witer=find(_workers.begin(), _workers.end(), param_Work);
	if(witer!= _workers.end())
	{
		bRet = FALSE;
	}
	
	if(bRet)
	{
		_workers.push_back(param_Work);
	}

	// decrease work count of this type
	map<string, workCount>::iterator iter;
	iter = _asscTypes.find(param_Work->type());
	if(iter != _asscTypes.end())
	{	// found
		iter->second.availCount--;
	}
	
	return bRet;
}

bool WorkFactory::unreg(BaseWork* param_Work)
{
	bool bRet=FALSE;

	// unreg it from stack
	Guard<Mutex> tmpGd(_wrkLock);
	
	vector<BaseWork*>::iterator witer=find(_workers.begin(), _workers.end(), param_Work);
	if(witer!=_workers.end())
	{
		bRet = TRUE;
	}
	
	if(bRet)
	{
		_workers.erase(witer);
	}

	// decrease work count of this type
	map<string, workCount>::iterator iter;
	iter = _asscTypes.find(param_Work->type());
	if(iter != _asscTypes.end())
	{	// found
		iter->second.availCount++;
	}
	
	return bRet;
}

bool WorkFactory::workTypeAvailable(const char* pTaskType)
{
	bool bRet = FALSE;

	map<string, workCount>::const_iterator iter;
	iter = _asscTypes.find(pTaskType);
	if(iter != _asscTypes.end())
	{	// found
		workCount c = iter->second;
		if(c.availCount > 0)
		{	// still has available works
			bRet = TRUE;
		}
	}

	return bRet;
}

void WorkFactory::workTypeRegister(const char *pTaskType, DWORD totalNum/*=1*/)
{
	workCount initCount;
	// at first, all works are available
	initCount.totalCount = initCount.availCount = totalNum;
	
	_asscTypes[pTaskType] = initCount;
}

map<string, workCount> WorkFactory::getTaskTypeVector()
{
	return _asscTypes;
}

bool WorkFactory::startWork(const char* taskId)
{
	bool bRet=FALSE;
	Guard<Mutex> tmpGd(_wrkLock);

	std::string strTaskId=taskId;
	
	for(int i=0; i<_workers.size(); i++)
	{
		if(_workers[i]->id()==strTaskId) {
			bRet = TRUE;
			break;
		}
	}
	
	if(bRet)
	{
		_workers[i]->start();	// found, start it
	}

	return bRet;
}

bool WorkFactory::removeWork(const char* taskId)
{
	bool bRet=FALSE;
	Guard<Mutex> tmpGd(_wrkLock);
	
	std::string strTaskId=taskId;
	
	for(int i=0; i<_workers.size(); i++)
	{
		if(_workers[i]->id()==strTaskId) {
			bRet = TRUE;
			break;
		}
	}
	
	if(bRet)
	{
		BaseWork* pex = _workers[i];	// found, remove it
		delete pex;
		_workers.erase(_workers.begin()+i);
	}

	return bRet;
}

bool WorkFactory::controlWork(const char* taskId, const char* useraction, RpcValue& userin, RpcValue& userout)
{
	bool bRet=FALSE;
	BaseWork* pWork=NULL;
	Guard<Mutex> tmpGd(_wrkLock);
	
	std::string strTaskId=taskId;
	
	for(int i=0; i<_workers.size(); i++)
	{
		if(_workers[i]->id()==strTaskId) {
			bRet = TRUE;
			pWork = _workers[i];
			break;
		}
	}
	
	if(bRet && pWork)
	{
		// found, send request to it
		pWork->OnUserRequest(useraction, userin, userout);
	}

	return bRet;
}

bool WorkFactory::hasWork(const char* taskId)
{
	bool bRet = FALSE;
	Guard<Mutex> tmpGd(_wrkLock);
	
	std::string strTaskId=taskId;
	
	for(int i=0; i<_workers.size(); i++)
	{
		if(_workers[i]->id()==strTaskId) {
			bRet = TRUE;
			break;
		}
	}
	
	return bRet;
}

int WorkFactory::appendTaskInfo(RpcValue& output, int& currentNum)
{
	int backNum = currentNum;
	Guard<Mutex> tmpGd(_wrkLock);
	
	for(int i=0; i<_workers.size(); i++)
	{
		if(_workers[i]==NULL)
			continue;

		RpcValue entry;
		std::string typeName, sessUrl, startTime, lastTime, status;
		URLStr taskUrl(getWorkNodeURL());
		taskUrl.setPath(URL_PATH_TASK);
		taskUrl.setVar(URL_VARNAME_TASK_ID, _workers[i]->id());
				
		typeName	= _workers[i]->type();
		sessUrl		= _workers[i]->getSessionURL();
		startTime	= _workers[i]->getStartTime();
		lastTime	= _workers[i]->getLastUpdateTime();
		//status		= _worker[i]->			TODO: add Status

		entry.SetStruct(INFO_TYPENAME_KEY,	RpcValue(typeName.c_str()));
		entry.SetStruct(INFO_TASKURL_KEY,	RpcValue(taskUrl.generate()));
		entry.SetStruct(INFO_SESSIONURL_KEY,RpcValue(sessUrl.c_str()));
		entry.SetStruct(INFO_STARTTIME_KEY,	RpcValue(startTime.c_str()));
		entry.SetStruct(INFO_LASTUPDATE_KEY,RpcValue(lastTime.c_str()));
		entry.SetStruct(INFO_STATUS_KEY,	RpcValue(status.c_str()));

		output.SetArray(currentNum++, entry);
		
	}
	
	return currentNum-backNum;
}

bool WorkFactory::getTaskDetails(const char* taskId, RpcValue& output)
{
	bool bRet = FALSE;
	Guard<Mutex> tmpGd(_wrkLock);
	
	std::string strTaskId=taskId;
	
	for(int i=0; i<_workers.size(); i++)
	{
		if(_workers[i]->id()==strTaskId) {
			bRet = TRUE;
			_workers[i]->getDetails(output);
			break;
		}
	}
	
	return bRet;
}

const char* WorkFactory::getWorkNodeID()
{
	return _pTaskAcpt->getDaemon().getWorkNodeID();
}

const char* WorkFactory::getWorkNodeURL()
{
	return _pTaskAcpt->getDaemon().getWorkNodeURL();
}

MPF_WORKNODE_NAMESPACE_END