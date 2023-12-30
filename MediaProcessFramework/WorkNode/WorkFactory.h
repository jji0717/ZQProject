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
// Name  : WorkFactory.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-5-9
// Desc  : factory is responsible for creating, managing and deleting run-time works (tasks)
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/WorkNode/WorkFactory.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 12    05-06-08 16:24 Bernie.zhao
// 
// 11    05-06-08 14:07 Bernie.zhao
// 
// 10    05-06-07 18:08 Bernie.zhao
// added sysTray support
// 
// 9     05-06-02 18:23 Bernie.zhao
// 
// 8     05-05-25 17:18 Bernie.zhao
// 
// 7     05-05-19 19:14 Bernie.zhao
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

#ifndef __WORKFACTORY_H__
#define __WORKFACTORY_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable : 4786)
#include <vector>
#include <string>
#include <map>
#include <algorithm>

using namespace std;

#include "BaseWork.h"

#include "Locks.h"
using namespace ZQ::common;

MPF_WORKNODE_NAMESPACE_BEGIN

/// structure containing work count information
typedef struct _workCount
{
	/// total count for a work type
	DWORD	totalCount;	
	/// available count for a work type
	DWORD	availCount;	
	
	_workCount	operator+(const _workCount& _right)
	{
		_workCount ret;
		ret.availCount = availCount + _right.availCount;
		ret.totalCount = totalCount + _right.totalCount;
		return ret;
	}
	
	_workCount	operator-(const _workCount& _right)
	{
		_workCount ret;
		ret.availCount = availCount - _right.availCount;
		ret.totalCount = totalCount - _right.totalCount;
		return ret;
	}
	
}	workCount;

class TaskAcceptor;

/// WorkFactory is responsible for supported work type task create and control
class DLL_PORT WorkFactory  
{
public:
	/// constructor
	WorkFactory(TaskAcceptor* pTaskAcpt);

	/// destructor
	virtual ~WorkFactory();

public:
	/// create a work according to specified work type, must be derived
	/// if requested work type has been registered in this factory, return the newly create work
	/// else, return NULL
	///@param[in]	workType	the name of the work type that need create
	///@param[in]	sessionURL	the URL of the session
	///@return					pointer to the created work, or NULL if failed
	virtual BaseWork*	createWork(const char* workType, const char* sessionURL)=0;

	/// start work with specified task id
	///@param[in]	taskId		the task id of the work
	///@return					success or fail
	virtual bool		startWork(const char* taskId);
	
	/// remove work with specified task id
	///@param[in]	taskId		the task id of the work
	///@return					success or fail
	virtual bool		removeWork(const char* taskId);

	/// send request to work with specified task id
	///@param[in]	taskId		the task id of the work
	///@param[in]	useraction  the request name user specified
	///@param[in]	userin		the input parameter user passed in
	///@param[out]	userout		the output parameter user will obtain
	///@return					success or fail
	bool				controlWork(const char* taskId, const char* useraction, RpcValue& userin, RpcValue& userout);

	/// check if work exists with specified task id
	///@param[in]	taskId		the task id of the work
	///@return					TRUE if has the work, else FALSE
	bool				hasWork(const char* taskId);

	/// append all running task info into output \n
	/// this function is called when sysTray queries running tasks
	///@param[in,out]	output		the array receives info
	///@param[in,out]	currentNum	the current entry number in this array
	///@return						the number appended in this call
	///@remarks			this function do not remove data existed in the output array, instead, 
	/// it append running tasks of this factory to the output array, starting from the index
	/// currentNum+1.  For example: the output array contains 5 entries BEFORE this function call.
	/// In this function call, it appends 4 entries into the array, starting from output[5]-output[8],
	/// and the currentNum parameter turns to be 9 after this function call.
	int					appendTaskInfo(RpcValue& output, int& currentNum);

	/// get specified running task details
	///@param[in]	taskId		the task id of the work
	///@param[out]	output		the output parameter to user-defined receive details
	///@return					success or fail
	bool				getTaskDetails(const char* taskId, RpcValue& output);

	/// get the vector for task types
	///@return					the map pointer contains all task type
	map<string, workCount>		getTaskTypeVector();

	/// get this workfactory plugin path
	///@return					the plugin path string
	const char*			getPluginPath() { return _pluginPathStr.c_str(); }

	/// get this workfactory plugin vendor info
	///@return					the plugin vendor info, such as version, provider, etc.
	const char*			getPluginVendor() { return _pluginVendorStr.c_str(); }
private:
	//////////////////////////////////////////////////////////////////////////
	// Only BaseWork is responsible for reg() and unreg() itself
	// When a work is registered (unregistered), the available count for this
	// work type will be decreased (increased).
	friend class BaseWork;

	/// register running task into this
	///@param[in]	param_Work		-the pointer to the task
	///@return						-True if success, False else
	bool reg(BaseWork*	param_Work);

	/// unregister running task from this
	///@param[in]	param_Work		-the pointer to the task
	///@return						-True if success, False else
	bool unreg(BaseWork*  param_Work);

protected:
	//////////////////////////////////////////////////////////////////////////
	// for derived class use, to manage type stack

	/// check if the required task type still has available number
	///@param[in]		pTaskType	-the type to check
	///@return			True if still get available count, False else
	bool	workTypeAvailable(const char* pTaskType);

	/// register a certain work type
	///@param[in]		pTaskType	-the task type to register
	///@param[in]		totalNum	-the tatol available number of this work type
	void	workTypeRegister(const char* pTaskType, DWORD totalNum=1);

	//////////////////////////////////////////////////////////////////////////

private:
	/// get work node ID of which this daemon runs on
	///@return					-the work node ID string
	const char* getWorkNodeID();

	/// get the URL which stands for this work node, usually 'MPF:://<ip>:<port>/...'
	///@return					-the work node URL string
	const char* getWorkNodeURL();

protected:
	/// work types that are registered
	map<string, workCount>		_asscTypes;

	/// lock for works
	Mutex				_wrkLock;

	/// works that are created
	vector<BaseWork*>	_workers;

	/// pointer of the taskAcceptor
	TaskAcceptor*		_pTaskAcpt;

	/// plugin path
	std::string			_pluginPathStr;

	/// plugin vendor info
	std::string			_pluginVendorStr;
};

MPF_WORKNODE_NAMESPACE_END

#endif // !define __WORKFACTORY_H__
