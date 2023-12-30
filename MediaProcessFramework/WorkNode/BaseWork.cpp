// RealWork.cpp: implementation of the RealWork class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "BaseWork.h"

#include "MPFLogHandler.h"
USE_MPF_NAMESPACE;

#include "WorkFactory.h"
#include "RpcWpClient.h"

enum WORK_STATE
{
	state_Ready,
			
	state_Run,
		
	state_Pause,
		
	state_Stop
};


////////////////////////////////////////////////////

///////////////////////////////////////////////////

MPF_WORKNODE_NAMESPACE_BEGIN

// -----------------------------
// class BaseWork
// -----------------------------
BaseWork::BaseWork(WorkFactory* factory, const char* TaskTypename, const char* sessionURL)
:_factory(factory), _typename(TaskTypename?TaskTypename:"BaseWork"), _state(WS_Idle),
 _sessUrl(sessionURL, true)

{
	if (sessionURL == NULL || *sessionURL ==0x00)
	{
		_sessUrl.parse(DUMMY_SESSION_URL);
	}

	_factory->reg(this);
	_state=WS_Setup;
	_startTime = "";
	_lastUpdateTime = "";
}

BaseWork::~BaseWork()
{
	if(_state!=WS_Idle)
		_factory->unreg(this);
}

void BaseWork::free()
{
	 _factory->unreg(this); 
	 _state=WS_Idle; 
	 delete this;
}

void BaseWork::addExpectedSessionAttr(const char* attrname)
{
	if (attrname != NULL && *attrname !=0x00)
		_expectedSessionAttrs.push_back(attrname);
}

void BaseWork::clearExpectedSessionAttrs()
{
	_expectedSessionAttrs.clear();
}

const char* BaseWork::type()
{
	return _typename.c_str();
}

const char* BaseWork::getSessionURL()
{
	return _sessUrl.generate();
}

const char* BaseWork::getManagementNode()
{
	return _sessUrl.getHost();
}

const int   BaseWork::getManagementPort()
{
	return _sessUrl.getPort();
}

const char* BaseWork::getSessionId()
{
	return _sessUrl.getVar(URL_VARNAME_SESSION_ID);
}

bool BaseWork::isDummySession()
{
	std::string idstr=getSessionId();
	return (idstr == URL_VAR_SESSION_DUMMY);
}

const char* BaseWork::id() const
{
	return _id;
}

const char* BaseWork::getStartTime()
{
	return _startTime.c_str();
}

const char* BaseWork::getLastUpdateTime()
{
	return _lastUpdateTime.c_str();
}

void BaseWork::clearReportAttrs()
{
	_reportAttrs.clear();
}

const ZQ::rpc::RpcValue BaseWork::getParameter(const char* key)
{
	static const ZQ::rpc::RpcValue Nil;
	if (key == NULL || *key==0x00 || !_parameters.hasMember(key))
		return Nil;
	
	return _parameters[key];
}

bool BaseWork::setParameter(const char* key, const ZQ::rpc::RpcValue& value)
{
	if (key == NULL || *key==0x00)
		return false;
	
	_parameters[key] = value;
	
	return true;
}

// implementation of Thread
bool BaseWork::init(void)
{
	ZQ::rpc::RpcValue result, expectedAttrs;
	
	// prepare the expected parameters
	int j =0, i=0;
	for (i=0; i< _expectedSessionAttrs.size(); i++)
	{
		if (_expectedSessionAttrs[i].size()>0)
			expectedAttrs.SetArray(j++, RpcValue(_expectedSessionAttrs[i].c_str()));
	}
	
	bool ret = updateSession(TASK_INIT_ACTION, _reportAttrs, expectedAttrs, result);

	// record start time
	time_t curr;
	time(&curr);
	_startTime = getTimeStr(curr);

	// TODO: adjust ret based the return code in result
	
	if(!isDummySession())
	{
	
		if (!ret)
			return false;

		RpcValue expRet = result[SESSION_ATTR_KEY];
		
		// flush the returned attribute into _parameters
		for (i=0, j=0; i< _expectedSessionAttrs.size(); i++)
		{
			std::string key = _expectedSessionAttrs[i];
			if (key.size()<=0)
				continue;
			
			if (expRet.hasMember(key.c_str()))
			{
				_parameters.setStruct(key.c_str(), expRet[key.c_str()]);				
				//_parameters[key.c_str()] = expRet[key.c_str()];
			}
			else
			{
				//TODO: log here: failed to get expected parameter 
			}
		}
	
		_parameters.setStruct(ERROR_CODE_KEY, result[ERROR_CODE_KEY]);
		_parameters.setStruct(COMMENT_KEY, result[COMMENT_KEY]);
	}
	// don't need the initial setup parameters any more
	clearExpectedSessionAttrs();
	
	return OnFabrication();
}

int BaseWork::run()
{
	return 0;
}

void BaseWork::final()
{
	ZQ::rpc::RpcValue result, expectedAttrs;
	
	bool ret = updateSession(TASK_FINAL_ACTION, _reportAttrs, expectedAttrs, result);

	free();
}

bool BaseWork::reportProgress(ZQ::rpc::RpcValue& result)
{
	ZQ::rpc::RpcValue	attrs, expattrs;
	OnGetProgress(attrs);
	return updateSession(TASK_PROGRESS_ACTION, attrs, expattrs, result);
}

bool BaseWork::updateSession(const char* action, ZQ::rpc::RpcValue& attrs, ZQ::rpc::RpcValue& expectedAttr, ZQ::rpc::RpcValue& result)
{
	try
	{
		//return Attr for RPC call
		RpcValue rAttr;

		if(isDummySession())
		{
			// if it is called by dummy session, skip updateSession call
			return true;
		}

		std::string actionstr = action;
		std::string mnHost = getManagementNode();
		int			mnPort = getManagementPort();
		
		std::string wnid   = _factory->getWorkNodeID();
		
		std::string taskid = (const char*)_id;
		
		std::string emptyEntry;
		bool		hasEmptyEntry=FALSE;
		
		//////////////////////////////////////////////////////////////////////////
		// check validation
		if(mnHost.empty())	{ emptyEntry="Management IP";	hasEmptyEntry=TRUE; }
		else if(mnPort==0)		{ emptyEntry="Management Port";	hasEmptyEntry=TRUE; }
		else if(wnid.empty())	{ emptyEntry="Work Node ID";	hasEmptyEntry=TRUE; }
		else if(taskid.empty())	{ emptyEntry="Task ID";			hasEmptyEntry=TRUE; }

		if(hasEmptyEntry)
		{
			_state=WS_Stop;

			MPFLog(MPFLogHandler::L_WARNING, "BaseWork::updateSession() %s is empty", emptyEntry.c_str());
			return FALSE;
		}
		
		//////////////////////////////////////////////////////////////////////////
		
		RpcValue mergAttr;
		mergAttr.SetStruct(ACTION_ID_KEY, RpcValue(action));
		mergAttr.SetStruct(WORKNODE_ID_KEY, RpcValue(wnid.c_str()));
		mergAttr.SetStruct(TASK_ID_KEY, RpcValue(taskid.c_str()));
		mergAttr.SetStruct(TASK_TYPE_KEY, RpcValue(_typename.c_str()));
		mergAttr.SetStruct(ATTR_KEY, attrs);
		mergAttr.SetStruct(EXP_ATTR_KEY, expectedAttr);

		RpcClient c(mnHost.c_str(), mnPort);
		c.setResponseTimeout(NET_SEND_TIME_OUT);

		RpcValue outParam;
		outParam.SetArray(0, mergAttr);
		
		if (c.execute((LPCSTR)UPDATESESSION_METHOD, outParam ,rAttr))
		{
			// record update time
			time_t curr;
			time(&curr);
			_lastUpdateTime = getTimeStr(curr);

			c.close();

			/*
			if(rAttr.getType()!= RpcValue::TypeArray)
			{
				MPFLog(MPFLogHandler::L_ERROR, "BaseWork::updateSession() Returned value is not an array");
				_state=WS_Stop;

				c.close();
				return FALSE;
			}
			*/

			RpcValue r=rAttr;
			result	 = r;
			RpcValue ErrorCode = r[ERROR_CODE_KEY];
			int nRetCode=ErrorCode;

			if(actionstr==TASK_INIT_ACTION)
			{
				if(nRetCode!=0)
				{
					_state=WS_Stop;
					//return FALSE;
				}
								
				_state=WS_Ready;
			}
		}
		else
		{
			MPFLog(MPFLogHandler::L_WARNING, "BaseWork::updateSession() Execute failed in UpdateSession");

			_state=WS_Stop;

			c.close();
			return FALSE;
		}

		
	}
	catch(...)
	{
		MPFLog(MPFLogHandler::L_WARNING, "BaseWork::updateSession() Got exception");

		_state=WS_Stop;

		return FALSE;
	}

	return TRUE;
}


MPF_WORKNODE_NAMESPACE_END
