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
// Ident : $Id: FakeMethodCost.cpp $
// Branch: $Name:  $
// Author: Jie Zhang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/FakeMethodCost.cpp $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 4     09-05-21 17:59 Jie.zhang
// merge from 1.10
// 
// 4     09-05-05 14:37 Jie.zhang
// 
// 3     09-03-06 16:45 Jie.zhang
// 
// 2     09-03-04 21:33 Jie.zhang
// reflactor the provision cost source
// 
// 1     08-11-18 11:24 Jie.zhang
// ---------------------------------------------------------------------------

#include "stdlib.h"
#include "FakeMethodCost.h"


namespace ZQTianShan {
	namespace ContentProvision {


unsigned int FakeMethodCost::evaluateCost(unsigned int bandwidthKbps, unsigned int sessions)
{
	if (bandwidthKbps > _maxBandwidthKbps)
		return MAX_LOAD_VALUE + 1;

	if (sessions > _maxSessions)
		return MAX_LOAD_VALUE + 1;


	int nCost1 = (int)((bandwidthKbps/(float)_maxBandwidthKbps)*MAX_LOAD_VALUE);
	int nCost2 = (int)((sessions/(float)_maxSessions)*MAX_LOAD_VALUE);

	//两种独占的resource, cost以大的为准
	return __max(nCost1, nCost2);
}

bool FakeMethodCostCol::addResource(const std::string& methodType, unsigned int maxBandwidthKbps, unsigned int maxSessions)
{
	if (methodType.empty() || !maxBandwidthKbps || !maxSessions)
		return false;

	MethodResMap::iterator it = _methodRes.find(methodType);
	if(it!=_methodRes.end())
	{
		return false;
	}

	_methodRes[methodType] = new FakeMethodCost(maxBandwidthKbps, maxSessions);
	return true;
}

bool FakeMethodCostCol::removeResource(const std::string& methodType)
{
	MethodResMap::iterator it = _methodRes.find(methodType);
	if(it==_methodRes.end())
	{
		return false;
	}

	if (it->second)
		delete it->second;

	_methodRes.erase(it);
	return true;
}

FakeMethodCost::FakeMethodCost( unsigned int maxBandwidthKbps, unsigned int maxSessions )
{
	_maxBandwidthKbps = maxBandwidthKbps;
	_maxSessions = maxSessions;
}


MethodCost* FakeMethodCostCol::getMethodCost( const std::string& methodType )
{
	MethodResMap::iterator it = _methodRes.find(methodType);
	if(it==_methodRes.end())
	{
		return NULL;
	}

	return (it->second);	
}

FakeMethodCostCol::~FakeMethodCostCol()
{
	MethodResMap::iterator it = _methodRes.begin();
	for(;it!=_methodRes.end();it++)
	{
		if (it->second)
			delete it->second;
	}

	_methodRes.clear();
}

unsigned int ContentProvision::FakeMethodCostCol::evaluateCost( const std::string& methodType, unsigned int bandwidthKbps, unsigned int sessions )
{
	MethodResMap::iterator it = _methodRes.find(methodType);
	if(it==_methodRes.end())
	{
		return NULL;
	}

	return it->second->evaluateCost(bandwidthKbps, sessions);	
}
	
}}