#include "stdafx.h"
#include "Daemon.h"
#include "TaskAcceptor.h"
//#include "BaseWork.h"

#include "MPFLogHandler.h"
USE_MPF_NAMESPACE

#include "RpcWpreporter.h"
#include "RpcWpServerMethod.h"
#include "RpcWpServer.h"
using namespace ZQ::rpc;

#include "winsock2.h"
#pragma comment(lib,"ws2_32.lib")


using namespace ZQ::MPF::utils;

#pragma warning(disable : 4786)
#include <vector>
using namespace std;



MPF_WORKNODE_NAMESPACE_BEGIN

bool TaskAcceptor::regFactory(WorkFactory* pFactory)
{
	bool bRet=TRUE;
	Guard<Mutex> tmpGd(_facLock);
	
	for(int i=0; i<_factories.size(); i++)
	{
		if(_factories[i]==pFactory) {
			bRet = FALSE;
			break;
		}
	}
	
	if(bRet)
	{
		_factories.push_back(pFactory);
	}

	return bRet;
}

bool TaskAcceptor::unregFactory(WorkFactory* pFactory)
{
	bool bRet=FALSE;
	Guard<Mutex> tmpGd(_facLock);	

	for(int i=0; i<_factories.size(); i++)
	{
		if(_factories[i]==pFactory) {
			bRet = TRUE;
			break;
		}
	}
	
	if(bRet)
	{
		_factories.erase(_factories.begin()+i);
	}

	return bRet;
}

map<string, workCount> TaskAcceptor::getTaskTypeVector()
{
	map<string, workCount> taskVect;
	map<string, workCount>::iterator taskIter;

	Guard<Mutex> tmpGd(_facLock);
	
	for(int i=0; i<_factories.size(); i++)
	{
		map<string, workCount> Vect = _factories[i]->getTaskTypeVector();
		map<string, workCount>::const_iterator facIter;
		for(facIter=Vect.begin(); facIter!=Vect.end(); facIter++)
		{
			taskIter = taskVect.find(facIter->first);
			if(taskIter == taskVect.end())
			{	// not inserted yet
				taskVect[facIter->first] = facIter->second;
			}
			else
			{	// already has a position
				taskVect[facIter->first] = taskIter->second + facIter->second;
			}
		}
	}

	return taskVect;
}

bool TaskAcceptor::factoryStartWork(const char* taskid)
{
	bool bRet=FALSE;
	Guard<Mutex> tmpGd(_facLock);
	
	for(int i=0; i<_factories.size(); i++)
	{
		if(_factories[i]->startWork(taskid)) {
			bRet = TRUE;
			break;
		}
	}

	return bRet;
}

bool TaskAcceptor::factoryCreateWork(const char* tasktype, const char* sessionURL, std::string& strTaskID)
{
	bool bRet=FALSE;
	BaseWork* tmpWork = NULL;
	
	Guard<Mutex> tmpGd(_facLock);
	
	for(int i=0; i<_factories.size(); i++)
	{
		tmpWork = _factories[i]->createWork(tasktype, sessionURL);
		if(tmpWork) 
		{
			bRet = TRUE;
			break;
		}
	}

	if(bRet)
	{
		strTaskID = tmpWork->id();
	}
	
	return bRet;
}

bool TaskAcceptor::factoryControlWork(const char* taskid, const char* useraction, RpcValue& userin, RpcValue& userout)
{
	bool bRet=FALSE;
	WorkFactory* pFac=NULL;

	Guard<Mutex> tmpGd(_facLock);
	
	for(int i=0; i<_factories.size(); i++)
	{
		if(_factories[i]->hasWork(taskid)) 
		{
			bRet = TRUE;
			pFac = _factories[i];
			break;
		}
	}

	if(bRet && pFac)
	{
		pFac->controlWork(taskid, useraction, userin, userout);
	}
	return bRet;
}

void TaskAcceptor::listWorkTypes(RpcValue& output)
{
	Guard<Mutex> tmpGd(_facLock);

	int arrayIndex=0;
	for(int i=0; i<_factories.size(); i++)
	{
		map<string, workCount> Vect = _factories[i]->getTaskTypeVector();
		map<string, workCount>::const_iterator facIter;

		std::string path	= _factories[i]->getPluginPath();
		std::string vendor	= _factories[i]->getPluginVendor();

		for(facIter=Vect.begin(); facIter!=Vect.end(); facIter++)
		{
			RpcValue entry;
			std::string typestr  = facIter->first;
			int			acount	 = facIter->second.availCount;
			int			rcount	 = facIter->second.totalCount - acount;
			
			entry.SetStruct(INFO_TYPENAME_KEY,	RpcValue(typestr.c_str()));
			entry.SetStruct(INFO_INSTANCES_KEY,	RpcValue(rcount));
			entry.SetStruct(INFO_AVAILABLE_KEY,	RpcValue(acount));
			entry.SetStruct(INFO_PLUGIN_KEY,	RpcValue(path.c_str()));
			entry.SetStruct(INFO_VENDOR_KEY,	RpcValue(vendor.c_str()));

			output.SetArray(arrayIndex++, entry);
		}
	}
}

void TaskAcceptor::listTaskInstances(RpcValue& output)
{
	Guard<Mutex> tmpGd(_facLock);

	int arrayIndex=0;
	for(int i=0; i<_factories.size(); i++)
	{
		_factories[i]->appendTaskInfo(output, arrayIndex);
	}
}

void TaskAcceptor::listTaskDetails(const char* taskid, RpcValue& output)
{
	Guard<Mutex> tmpGd(_facLock);
	
	for(int i=0; i<_factories.size(); i++)
	{
		if(_factories[i]->getTaskDetails(taskid, output)) 
		{
			break;
		}
	}
}

char* TaskAcceptor::help(char* strBuffer, int nMax)
{
	if(strBuffer==NULL)
		return NULL;

	ZQ::MPF::utils::sfstrncpy(strBuffer, "This method handles request from manage node.", nMax);
	return strBuffer;
}

void TaskAcceptor::execute(RpcValue& params, RpcValue& result)
{
	try
	{

		if(params.getType()!=params.TypeArray)
		{
#ifdef _DEBUG
			printf("params type is not array!\n");
#endif

			result.SetStruct(ERROR_CODE_KEY,RpcValue(1));
			result.SetStruct(COMMENT_KEY,RpcValue("params type is not array!"));
			
			return;
		}

		RpcValue rParam;
		rParam=params[0];

		if(rParam.getType()!=rParam.TypeStruct)
		{
#ifdef _DEBUG
			printf("params type is not struct!\n");
#endif
			result.SetStruct(ERROR_CODE_KEY,RpcValue(1));
			result.SetStruct(COMMENT_KEY,RpcValue("params type is not struct!"));
			
			return;
		}


		//////////////////////////////////////////////////////////////////////////
		// begin parse
		
		char szActionID[64];
		char szType[512];
		char szSesURL[512];
		char szTaskID[512];
				
		std::string strActionID;
			
		ZeroMemory(szActionID, 64);
		ZeroMemory(szType,512);
		ZeroMemory(szSesURL,512);
		
		// ActionID : string
		rParam[ACTION_ID_KEY].ToString(szActionID,64);
		strActionID=szActionID;

		// RequestAttr : structure
		RpcValue rRequest = rParam[REQUEST_ATTR_KEY];
		if(rRequest.getType()!=rRequest.TypeStruct)
		{
			MPFLog(MPFLogHandler::L_ERROR, "TaskAcceptor::execute() RequestAttr type is not struct");
			
			result.SetStruct(ERROR_CODE_KEY,RpcValue(1));
			result.SetStruct(COMMENT_KEY,RpcValue("RequestAttr type is not struct!"));
			
			return;
		}
		
		if(strActionID==REQUEST_SETUP)
		{
			// task type : string
			rRequest[TASK_TYPE_KEY].ToString(szType, 512);
			
			// Mgm session URL : string
			rRequest[MGM_SESSION_URL_KEY].ToString(szSesURL, 512);

			// do some extra work
			std::string strTaskID;
			if(factoryCreateWork(szType, szSesURL ,strTaskID))
			{
				MPFLog(MPFLogHandler::L_DEBUG, "TaskAcceptor::execute() Work with task id '%s' created", strTaskID.c_str());
				
				result.SetStruct(TASK_ID_KEY, RpcValue(strTaskID.c_str()));
				result.SetStruct(ERROR_CODE_KEY, RpcValue(0));
				result.SetStruct(COMMENT_KEY, RpcValue("Task Request Successfully!"));
				
			}
			else
			{
				MPFLog(MPFLogHandler::L_ERROR, "TaskAcceptor::execute() Can not create specified '%s' task", szType);

				
				result.SetStruct(ERROR_CODE_KEY,RpcValue(1));
				result.SetStruct(COMMENT_KEY, RpcValue("Can not create specified type task!"));
			}
		}
		else if(strActionID==REQUEST_PLAY)
		{
			// task id : string
			rRequest[TASK_ID_KEY].ToString(szTaskID, 512);
			
			// start work
			if(factoryStartWork(szTaskID))
			{
				MPFLog(MPFLogHandler::L_DEBUG, "TaskAcceptor::execute() Work with task id '%s' started", szTaskID);

				result.SetStruct(ERROR_CODE_KEY,RpcValue(0));
				result.SetStruct(COMMENT_KEY,RpcValue("Task start successfully!"));
			}
			else
			{
				MPFLog(MPFLogHandler::L_ERROR, "TaskAcceptor::execute() Can not start specified task with task id '%s'", szTaskID);

				result.SetStruct(ERROR_CODE_KEY,RpcValue(1));
				result.SetStruct(COMMENT_KEY,RpcValue("Task start failed!"));
			}
			
		}
		else if(strActionID==REQUEST_USER)
		{
			char szUserActionID[512];
			ZeroMemory(szUserActionID, 512);

			// task id : string
			rRequest[TASK_ID_KEY].ToString(szTaskID, 512);

			// user action id : string
			rRequest[USER_ACTION_ID_KEY].ToString(szUserActionID, 512);

			// user attr (input)
			RpcValue userAttr = rRequest[USER_ATTR_KEY];

			factoryControlWork(szTaskID, szUserActionID, userAttr, result);
			
		}
		else 
		{
		}

		
	}
	catch(...)
	{
#ifdef _DEBUG
		printf("throw exception during creating work!\n");
#endif

		result.SetStruct(ERROR_CODE_KEY, RpcValue(8));//exec		
		result.SetStruct(COMMENT_KEY, RpcValue("throw exception during task accept!"));
		MPFLog(MPFLogHandler::L_DEBUG, "TaskAcceptor::execute() Got Exception");
	}
		
}

TaskAcceptor::TaskAcceptor(Daemon& daemon)
:_daemon(daemon), ZQ::rpc::RpcServerMethod((LPCSTR)TASKREQUEST_METHOD, NULL)
{
	_daemon.regAcceptor(this);
	_daemon.addMethod(this);
	

	URLStr tmpURL(_daemon.getWorkNodeURL());
	_wninfo = new WorknodeInfo(*this, daemon, tmpURL.getHost());
	_daemon.addMethod(_wninfo);

	_daemon.enableIntrospection(true);
}

TaskAcceptor::~TaskAcceptor() 
{
	if(_wninfo)
	{
		delete _wninfo;
		_wninfo = NULL;
	}

}

MPF_WORKNODE_NAMESPACE_END

