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
// Ident : $Id: TaskAcceptor.h,v 1.0 2005/05/08 16:34:35 Gu Exp $
// Branch: $Name:  $
// Author: Hongye Gu
// Desc  : listen manage node's request for creating works
//
// Revision History: 
// ---------------------------------------------------------------------------

#ifndef _TASKACCEPTOR_H
#define _TASKACCEPTOR_H


#pragma warning(disable : 4786)
#include <vector>
#include <string>
using namespace std;

#include "MPFCommon.h"
using namespace ZQ::MPF::utils;

#include "NativeThread.h"
USE_MPF_NAMESPACE

#include "../XMLRPC/rpcwpvalue.h"
#include "../XMLRPC/rpcwpserver.h"
#include "../XMLRPC/RpcWpServerMethod.h"

#include "WorkFactory.h"
#include "WorknodeInfo.h"

MPF_WORKNODE_NAMESPACE_BEGIN

class Daemon;
/// TaskAcceptor handles task request from manage node
class DLL_PORT TaskAcceptor : public ZQ::rpc::RpcServerMethod
{
	friend class Daemon;
	friend class WorkFactory;
	friend class WorknodeInfo;
public:

	///constructor
	///@param[in]		daemon		-the daemon object attached with
	TaskAcceptor(Daemon& daemon);

	///destructor
	virtual ~TaskAcceptor();

public:
	//////////////////////////////////////////////////////////////////////////
	
	/// register work factory
	///@param[in]		pFactory	-the WorkFactory need registered
	///@return						-True if success, False else
	bool	regFactory(WorkFactory* pFactory);

	/// unregister work factory
	///@param[in]		pFactory	-the WorkFacoty need unregistered
	///@return						-True if success, False else
	bool	unregFactory(WorkFactory* pFactory);

protected:
	/// get attached Daemon object reference
	///@return						-the reference of the Daemon object
	Daemon&			getDaemon() { return _daemon; }

	/// get task type list from all WorkFactory
	///@return						-the list contains all supported work type
	map<string, workCount>	getTaskTypeVector();

	/// derive from RPC method
	void execute(ZQ::rpc::RpcValue& params, ZQ::rpc::RpcValue& result);

	/// derived from RPC method
	char* help(char* strBuffer, int nMax);

	/// query every factory to create a worker
	///@param[in]		tasktype	-task type string
	///@param[in]		sessionURL	-session URL contains manage node information
	///@param[out]		strTaskID	-task id string allocated by work node
	///@return						-True if success, False else
	bool			factoryCreateWork(const char* tasktype, const char* sessionURL, std::string& strTaskID);

	/// query every factory to start a worker
	///@param[in]		taskid		-task id string allocated by work node
	///@return						-True if success, False else
	bool			factoryStartWork(const char* taskid);

	/// query every factory to send a user request to a worker
	///@param[in]		taskid		-task id string allocated by work node
	///@param[in]		useraction	-user defined action code
	///@param[in]		userin		-input for this action
	///@param[out]		userout		-output for this action
	///@return						-True if success, False else
	bool			factoryControlWork(const char* taskid, const char* useraction, RpcValue& userin, RpcValue& userout);

	//////////////////////////////////////////////////////////////////////////
	
	/// list all work type information into output parameter \n
	/// this function is used by WorknodeInfo class to handle sysTray queries
	///@param[out]		output		-the returned information
	///@remarks		-this function returns information about worktype name, running
	/// instance number, available instance number, plug-in location, vendor information
	void			listWorkTypes(RpcValue& output);

	/// list all running task instances information into output parameter \n
	/// this function is used by WorknodeInfo class to handle sysTray queries
	///@param[out]		output		-the returned information
	///@remarks		-this function returns information about task URL, session URL,
	/// work type name, task start time, last update session time, and status
	void			listTaskInstances(RpcValue& output);
	
	// list specified task instances detail information into output parameter \n
	/// this function is used by WorknodeInfo class to handle sysTray queries
	///@param[in]		taskid		-task id string allocated by work node
	///@param[out]		output		-the returned information
	void			listTaskDetails(const char* taskid, RpcValue& output);
private:
	/// reference for daemon object
	Daemon&						_daemon;

	/// pointer to WorknodeInfo instance
	WorknodeInfo*				_wninfo;

	/// pointer to work factories
	vector<WorkFactory*>		_factories;

	/// lock for work factories
	ZQ::common::Mutex			_facLock;

};

MPF_WORKNODE_NAMESPACE_END

#endif