
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
// Dev  : Microsoft Developer Studio
// Name  : TaskRequestPoster.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-5-11
// Desc  : task request poster
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/RequestPoster/TaskRequestPoster.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 11    05-06-21 10:00p Daniel.wang
// 
// 10    05-06-20 2:40p Daniel.wang
// ===========================================================================

#ifndef _ZQ_TASKREQUESTPOSTER_H_
#define _ZQ_TASKREQUESTPOSTER_H_

#include "MPFCommon.h"
#include "xmlrpc.h"
#include "comextra/ZqCommon.h"

#define REQPOST_BEGIN namespace ZQ{namespace MPF{namespace REQPOST{
#define REQPOST_END }}}

#define POST_ERR_NOTALLOCATED -1
#define POST_ERR_OK 0
#define POST_ERR_PARAMETER 1
#define POST_ERR_NETWORK 2
#define POST_ERR_USER 100
#define POST_ERR_PROTOCOL POST_ERR_USER+1
#define POST_ERR_UNKNOWN_ACTION POST_ERR_USER+2
#define POST_ERR_RESPONS_ERR POST_ERR_USER+100


namespace ZQ{namespace rpc{class RpcClient;}}

REQPOST_BEGIN

///TaskRequestPoster
///task requester sender
class DLL_PORT TaskRequestPoster
{
protected:
	ZQ::MPF::utils::URLStr	m_worknode;
	std::string		m_strMgmnode;
	rpc::RpcClient	m_client;
	std::string		m_strTaskId;
	std::string		m_strTaskType;
	std::string		m_strWorkNodeId;

public:
	///constructor
	///@param worknodeurl - the URL string of work node
	///@param mgmnodeurl - the URL string of manager node
	///@param responsTimeout - respons time out while post task request to remote machine
	TaskRequestPoster(const char* worknodeurl, const char* mgmnodeurl, 
	int responsTimeout = NET_SEND_TIME_OUT);

	///destructor
	virtual ~TaskRequestPoster();

	///post a request to remote machine
	///@param tasktype - the task type to do in work node
	///@param result[out] - process return
	///@return - post error result
	int postSetup(const char* tasktype, rpc::RpcValue& result);
	
	///post a request to remote machine
	///@param useractionid - user defined action id
	///@param param - process parameters
	///@param result[out] - process return
	///@return - post error result
	int postUser(const char* useractionid, const rpc::RpcValue& param, rpc::RpcValue& result);

	///@return - task id
	const char* getTaskId() const;

	///@return - work node id
	const char* getWorkNodeId() const;
};


REQPOST_END

#endif//_ZQ_TASKREQUESTPOSTER_H_
