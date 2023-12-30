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
// Name  : navRebuilder.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : worker for rename entry
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/navRenamer.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 3     12/09/06 5:21a Bernie.zhao
// modified architechture to support QA Navigation.
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================
#pragma once

// local include
#include "navworker.h"

class navRenamer :
	public navWorker
{
protected:
	friend class navWorkerProvider;

	/// protected constructor, use navWorkerProvider to create instance
	navRenamer(WQ_Entry& wqentry, CString connectStr);

	/// protected destructor, call free() to delete self
	~navRenamer(void);

public:
	/// free method
	virtual void free();

	/// overrided function
	///@return	work result, NS_SUCCESS if successfully
	virtual int work();
};
