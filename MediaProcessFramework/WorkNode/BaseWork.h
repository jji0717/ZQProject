// ===========================================================================
// Copyright (c) 2005 by
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
// Ident : $Id: BaseWork.h,v 1.0 2005/05/08 16:34:35 Gu Exp $
// Branch: $Name:  $
// Author: Hongye Gu
// Desc  : define task creation APIs
//
// Revision History: 
// ---------------------------------------------------------------------------

#ifndef __BASEWORK_H__
#define __BASEWORK_H__

#pragma warning(disable : 4786)
#include <vector>
#include <string>

//rpc header
#include "RpcWpValue.h"
#include "RpcWpClient.h"
using namespace ZQ::rpc;


//MPF common file
#include "MPFCommon.h"
using namespace ZQ::MPF::utils;


//parent thread
#include "NativeThread.h"

MPF_WORKNODE_NAMESPACE_BEGIN

class WorkFactory;
// -----------------------------
// class BaseWork
// -----------------------------
/// base class is a thread that is responsible for actual task execution
class DLL_PORT BaseWork : public ZQ::common::NativeThread
{
	/// status for BaseWork
	typedef enum _state
	{	/// instance created, not registered
		WS_Idle,	
		/// instance registered, not yet init()
		WS_Setup,	
		/// initialized, ready to run
		WS_Ready,	
		/// running
		WS_Run,		
		/// pausing
		WS_Pause,	
		/// stopped
		WS_Stop		
	} state_e;

protected:
	friend class WorkFactory;
	
	/// constructor is protected, only WorkFactory is able to give birth to it
	///@param[in]	factory			-the pointer of WorkFactory class object which gives birth to it
	///@param[in]	TaskTypename	-the task type string of this work
	///@param[in]	sessionURL		-the session URL string which contains the manager node information who requests this work
	BaseWork(WorkFactory* factory, const char* TaskTypename= "NULL", const char* sessionURL=NULL);
	
	///destruct
	virtual ~BaseWork();

public:
	//////////////////////////////////////////////////////////////////////////
	
	/// get the work type string for this task
	///@return			-the string containing the work type
	const char* type();

	/// get the session URL manager node passed to this task
	///@return			-the session URL string
	const char* getSessionURL();

	/// get the IP address of manager node which requests this task
	///@return			-the IP string of the manager node
	const char* getManagementNode();

	/// get the port of manager node which requests this task
	///@return			-the listening port
	const int   getManagementPort();

	/// get session identifier string of this task
	///@return			-the session identifier string
	const char* getSessionId();

	/// check if this task is created with dummy session (no manager node)
	///@return			-True if with dummy session, False else
	bool		isDummySession();

	/// get the task id of this task
	///@return			-the task id string
	const char* id() const;

	/// get start time of this task
	///@return			-the task start time string
	const char* getStartTime();

	/// get last updateSession time of this task
	///@return			-the task last updateSession time
	const char* getLastUpdateTime();

	/// get user defined task details
	///@param[out]		output		-the user defined details
	virtual void getDetails(RpcValue& output){}

public:
	//////////////////////////////////////////////////////////////////////////
	
	/// kill this task
	/// if user derived class want to do something extra, override this function
	/// just remember to 'delete this' in the end
	virtual void free(void);

protected:
	//////////////////////////////////////////////////////////////////////////
	
	/// add an excepted attributes to the parameter list
	/// these parameters will be filled into 'ExpAttr' when sending "UpdateSession[TaskInit]" to mangenode
	///@param[in]	attrname		-the attribute name
	void addExpectedSessionAttr(const char* attrname);

	/// clear all attributes in expected parameter list
	void clearExpectedSessionAttrs();

	/// clear all attributes in report attribute list
	void clearReportAttrs();

	/// get the specific value of the key in returned expected parameter list
	///@param[in]	key				-the key string of the attribute
	///@return						-the value of the key
	const ZQ::rpc::RpcValue getParameter(const char* key);

	/// set the specific value of the key in expected parameter list
	///@param[in]	key				-the key string of the attribute
	///@param[in]	value			-the value of the key
	///@return						-True if success, False else
	bool setParameter(const char* key, const ZQ::rpc::RpcValue& value);

	//////////////////////////////////////////////////////////////////////////
	
	/// notify the managenode of "updateSession" communication
	/// there are now 3 types of it, [TaskInit],[TaskProgress],and [TaskFinal]
	/// please refer to <XML_RPC Interface.doc> for details
	///@param[in]		action			-the type of this communication
	///@param[in]		attrs			-the attributes to report
	///@param[in]		expectedAttr	-the expected attributes
	///@param[out]		result			-the result value
	///@return							-True if success, False else
	bool updateSession(const char* action, ZQ::rpc::RpcValue& attrs, ZQ::rpc::RpcValue& expectedAttr, ZQ::rpc::RpcValue& result);

	/// an optional function to help send progress information
	/// notify the managenode of "updateSession[TaskProgress]"
	/// with the progress information got from OnGetProgress function
	///@param[out]		result			-the result value of this communication
	///@return							-True if success, False else
	bool reportProgress(ZQ::rpc::RpcValue& result);

	//////////////////////////////////////////////////////////////////////////
	
	/// allows user derived class to do their own progress calculation
	///@param[out]		attrs			-the returned progress attribute
	///@return							-True if success, False else
	virtual bool OnGetProgress(ZQ::rpc::RpcValue& attrs)
	{
		// add your code here in the child
		return true;
	}
		
	/// allows user derived class to do some extra work during work init phase
	/// child class derived from the BaseWork must construct its execution structure
	/// based on the member variables and those parameters gained in _parameters
	///@return							-True if success, False else
	virtual bool OnFabrication()
	{
		// add your code here in the child
		return true;
	}

	/// allows user dericed class to do its own message dispatch instead of the standard way BaseWork provided
	/// This method is NOT encouraged to override
	///@param[in]		useraction			-the user defined message string
	///@param[in]		userin				-the input parameter of this message
	///@param[out]		userout				-the output parameter of this message
	///@return							-True if success, False else
	///@remarks 
	/// child class derived from the BaseWork could respond to user defined request from manage node \n
	/// based on the user request name in 'useraction', with input and output in 'userin' and 'userout' \n
	/// This is the recommended solution \n
	/// you can use	'DECLARE_USRMSG_MAP()' \n
	///				'BEGIN_USERMSG_MAP()' \n
	///				'ON_USERMSG()' \n
	///				'END_USERMSG_MAP' \n
	/// macros to map your own response functions such as 'OnTest()'to the request map \n
	/// Check test project 'WordNodeApp' for help \n\n
	/// If you do not want BaseWork to dispatch the user request for you \n
	/// you can override this function and do your own request \n
	/// \n
	/// !NOTICE: do NOT block in your own response functions! \n
	/// if you have extra work to do which may take long time \n
	/// it is better to create a new thread to handle it and return this code as soon as possible
	virtual bool OnUserRequest(const char* useraction, RpcValue& userin, RpcValue& userout)
	{
		
		/// !NOTICE: do NOT block in your own response functions!
		/// if you have extra work to do which may take long time
		/// it is better to create a new thread to handle it and return this code as soon as possible
		_dispatchUserMsg(useraction, userin, userout);
		return true;
	}

	/// this dispatch method is the default dispatch method BaseWork provides
	/// you can use	'DECLARE_USRMSG_MAP()' \n
	///				'BEGIN_USERMSG_MAP()' \n
	///				'ON_USERMSG()' \n
	///				'END_USERMSG_MAP' \n
	/// macros to override this
	virtual void _dispatchUserMsg(const char* msgName, RpcValue& in, RpcValue& out)
	{}

protected:
	/// implementation of Thread
	virtual bool init(void);

	/// implementation of Thread
	virtual int run(void);

	/// implementation of Thread
	virtual void final(void);

protected:

	/// work type string
	std::string					_typename;
	
	/// task id
	ZQ::MPF::utils::UniqueId	_id;

	/// status
	state_e						_state;

	/// task start time
	std::string					_startTime;

	/// last updateSession time
	std::string					_lastUpdateTime;

	/// session URL string
	ZQ::MPF::utils::URLStr		_sessUrl;

	/// pointer to WorkFactory which gives birth to this
	WorkFactory*				_factory;

	/// attributes that is about to report to manage node
	ZQ::rpc::RpcValue	 _reportAttrs;

	/// attributes that is obtained from manage node
	ZQ::rpc::RpcValue	 _parameters;

	/// attribute keys that are expected when send "updateSession[TaskInit]"
	std::vector < std::string >	 _expectedSessionAttrs;
};

//////////////////////////////////////////////////////////////////////////
// following are for user message dispatch macros
typedef void (BaseWork::*LPUSERMSGCALLBACK)(RpcValue& in, RpcValue& out);

typedef struct {
	const char*			strMsgName;
	LPUSERMSGCALLBACK	pCallback;
}User_MsgMap_Entry;

#define DECLARE_USRMSG_MAP() \
private: \
	static 	User_MsgMap_Entry _UserMsgMap[]; \
protected: \
	virtual void _dispatchUserMsg(const char* msgName, RpcValue& in, RpcValue& out); \

#define BEGIN_USERMSG_MAP(theClass) \
void theClass::_dispatchUserMsg(const char* msgName, RpcValue& in, RpcValue& out) \
{ \
	for(int i=0; _UserMsgMap[i].strMsgName!=0; i++) \
	{ \
		if(strcmp(_UserMsgMap[i].strMsgName,msgName)==0) \
		{	LPUSERMSGCALLBACK p= _UserMsgMap[i].pCallback; \
			(this->*p)(in, out); break;	} \
	} \
} \
User_MsgMap_Entry theClass::_UserMsgMap[] = \
{ \
	

#define END_USERMSG_MAP() \
	{0,NULL} \
}; \


#define ON_USERMSG(msgname, memberFxn) \
{msgname, (LPUSERMSGCALLBACK)memberFxn},


// end of user message dispatch macros
//////////////////////////////////////////////////////////////////////////
MPF_WORKNODE_NAMESPACE_END

#endif 
