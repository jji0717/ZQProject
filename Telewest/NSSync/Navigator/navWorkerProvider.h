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
// $Log: /ZQProjs/Telewest/NSSync/Navigator/navWorkerProvider.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================

#if !defined(AFX_NAVWORKERPROVIDER_H__89008B6F_B872_4B04_B291_EB1B5462B466__INCLUDED_)
#define AFX_NAVWORKERPROVIDER_H__89008B6F_B872_4B04_B291_EB1B5462B466__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// common include

// local include
#include "navRebuilder.h"
#include "navSplUpdater.h"
#include "navRemover.h"
#include "navRenamer.h"

class nsBuilder;

/// this class uses factory pattern, all specific nav workers are born by this class object
class navWorkerProvider  
{
//////////////////////////////////////////////////////////////////////////
// constructor and destructor
public:
	navWorkerProvider(nsBuilder* theBuilder);
	virtual ~navWorkerProvider();
//////////////////////////////////////////////////////////////////////////
// object reference methods
public:
	/// get pointer to nsBuilder object
	nsBuilder* getBuilder() { return _pTheBuilder; }

//////////////////////////////////////////////////////////////////////////
// operations
public:

	/// create proper navWorker object according to work-queue entry type
	///@param[in]	wqentry		the Work Queue entry which need be handled
	///@return					pointer to a new nav worker object
	navWorker*	provide(WQ_Entry wqentry);

	/// get worker category according to entry
	///@param[in]	source_type		the source_type of wq
	///@param[in]	entry_type		the entry_type of wq
	///@param[in]	operation_type	the operation_type of wq
	///@return						the worker category
	static int getCategory(int source_type, int entry_type, int operation_type);
//////////////////////////////////////////////////////////////////////////
// related objects

	/// pointer to nsBuilder object
	nsBuilder* _pTheBuilder;

//////////////////////////////////////////////////////////////////////////
// db info string

	/// ODBC connect string, for CRecordSet class use
	CString _connectStr;
};

#endif // !defined(AFX_NAVWORKERPROVIDER_H__89008B6F_B872_4B04_B291_EB1B5462B466__INCLUDED_)
