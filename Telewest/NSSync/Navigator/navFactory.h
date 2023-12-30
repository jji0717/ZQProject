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
// Name  : navWorkerProvider.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : class for navWorker object factory
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/navFactory.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 2     06-12-12 14:13 Ken.qian
// 
// 1     12/09/06 5:20a Bernie.zhao
// factory class works as thread to keep some worker in the factory
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================
#if _MSC_VER > 1000
#	pragma once
#else
#	ifndef	_NAV_FACOTRY_H_
#	define	_NAV_FACOTRY_H_
#	endif
#endif // _MSC_VER > 1000
#include "nativethread.h"

enum FactoryType
{
	FactoryNAV_t =0 ,
	FactoryQA_t
};

class _boolGuard
{
public:
	_boolGuard(bool& value,bool resetVal = false)
		:_value(value),_resetValue(resetVal) 
		{ _value=!_resetValue; }

	~_boolGuard()
		{ _value=_resetValue; }
private:
	bool&	_value;
	bool	_resetValue;
	
	_boolGuard(const _boolGuard &);
	_boolGuard &operator=(const _boolGuard &);
};

class nsBuilder;

class navFactory :
	public ZQ::common::NativeThread
{
public:
	navFactory(nsBuilder& theBuilder, FactoryType type = FactoryNAV_t);
	~navFactory(void);

public:
	//////////////////////////////////////////////////////////////////////////
	// overrided functions
	virtual int run();

public:
	/// get the status of factory, see if worker is working
	bool isWorking() { return _isWorking; }

	/// signal event that terminate the thread
	void signalTerm() { ::SetEvent(_hTerminate); }

	/// signal event that wake up from sleeping
	void signalWakeup() { ::SetEvent(_hWakeup); }
	

protected:
	bool isQA() { return (_type==FactoryQA_t); }

protected:
	/// pointer to nsBuilder object
	nsBuilder&	_theBuilder;

	/// signal for stop thread
	HANDLE	_hTerminate;

	/// signal for wakeup
	HANDLE	_hWakeup;

	/// type
	FactoryType	_type;

	/// status, if a worker working in the factory, means it is updating NE tree
	bool	_isWorking;
};
