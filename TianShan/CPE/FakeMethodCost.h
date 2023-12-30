// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: FakeMethodCost.h $
// Branch: $Name:  $
// Author: Jie Zhang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/FakeMethodCost.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 3     09-05-21 17:59 Jie.zhang
// merge from 1.10
// 
// 3     09-05-05 14:37 Jie.zhang
// 
// 2     09-03-04 21:33 Jie.zhang
// reflactor the provision cost source
// 
// 1     08-11-18 11:24 Jie.zhang
// ---------------------------------------------------------------------------

#ifndef __ZQTianShan_CPE_FakeMethodCost_H__
#define __ZQTianShan_CPE_FakeMethodCost_H__


#include "IMethodCost.h"

#include <string>
#include <map>

namespace ZQTianShan {
	namespace ContentProvision {

class FakeMethodCost: public MethodCost
{
public:
	FakeMethodCost(unsigned int maxBandwidthKbps, unsigned int maxSessions);
	virtual unsigned int evaluateCost(unsigned int bandwidthKbps, unsigned int sessions);

protected:
	unsigned int _maxBandwidthKbps;
	unsigned int _maxSessions;
};

class FakeMethodCostCol: public MethodCostCol
{
public:
	~FakeMethodCostCol();

	virtual unsigned int evaluateCost(const std::string& methodType, unsigned int bandwidthKbps, unsigned int sessions);

	virtual bool addResource(const std::string& methodType, unsigned int maxBandwidthKbps, unsigned int maxSessions);

	virtual bool removeResource(const std::string& methodType);

protected:
	virtual MethodCost* getMethodCost(const std::string& methodType);

	typedef std::map<std::string, FakeMethodCost*> MethodResMap;
	MethodResMap	_methodRes;
};


}}


#endif