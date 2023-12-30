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
// Name  : nsOptimizer.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : class for optimize AM work-queue
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/nsOptimizer.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================


#if !defined(AFX_NSOPTIMIZER_H__497F460F_79EF_4833_9020_64F68DD75081__INCLUDED_)
#define AFX_NSOPTIMIZER_H__497F460F_79EF_4833_9020_64F68DD75081__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// local include
#include "ns_def.h"
class nsBuilder;

class nsOptimizer  
{
//////////////////////////////////////////////////////////////////////////
// constructor and destructor
public:
	nsOptimizer(nsBuilder* pBuilder);
	virtual ~nsOptimizer();

//////////////////////////////////////////////////////////////////////////
// object reference methods
public:
	/// get the ne db builder object
	///@return		the pointer to the builder
	nsBuilder* getBuilder() { return _pTheBuilder; }

//////////////////////////////////////////////////////////////////////////
// operations
public:

	/// optimize work queue list in nsBuilder object
	///@return	optimize result, NS_SUCCESS if successfully
	int optimize();

//////////////////////////////////////////////////////////////////////////
// attributes
protected:
	/// pointer to the ne db builder object
	nsBuilder*	_pTheBuilder;
};

#endif // !defined(AFX_NSOPTIMIZER_H__497F460F_79EF_4833_9020_64F68DD75081__INCLUDED_)
