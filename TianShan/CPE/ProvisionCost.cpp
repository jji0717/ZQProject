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
// Ident : $Id: ProvisionCost.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/ProvisionCost.cpp $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 7     08-12-15 17:54 Yixin.tian
// 
// 6     08-11-18 11:09 Jie.zhang
// merge from TianShan1.8
// 
// 5     08-05-14 22:07 Jie.zhang
// 
// 4     08-05-13 11:31 Jie.zhang
// 
// 3     08-04-25 18:05 Jie.zhang
// 
// 2     08-04-09 13:26 Hui.shao
// 
// 1     08-04-09 13:24 Hui.shao
// ---------------------------------------------------------------------------

#include "ProvisionCost.h"
#include "ProvisionFactory.h"
#include "TimeUtil.h"

#define TIMECMP(_X, _Y) ((Ice::Long)(_X) - (Ice::Long)(_Y))
#define  CHAIN_TRACE

#if !defined(_DEBUG)
#  undef  CHAIN_TRACE
#endif // _DEBUG

#ifdef CHAIN_TRACE
#undef CHAIN_TRACE
#define CHAIN_TRACE	{ printf("\n%s(%3d)\n", __FILE__, __LINE__);\
					for (BookChain::iterator cit = chain.begin(); cit < chain.end(); cit++) \
					printf("NODE{st=%lld; et=%lld; instance=%d; bandwidth=%d} ", cit->st, cit->et, cit->instance, cit->bandwidth); }
#else
#define CHAIN_TRACE
#endif

namespace ZQTianShan {
namespace CPE {

ProvisionCost::ProvisionCost(CPEEnv& env, const std::string& methodType, Ice::Long timeStart, Ice::Long timeEnd)
 : _env(env), _methodTypeToBook(methodType), _bDirty(true)
{
	_itemToBook.st = timeStart;
	_itemToBook.et = timeEnd;
	_itemToBook.instance = 1;
	_itemToBook.bandwidth = 0;
	_itemToBook.cost = 0;

	refreshSessionList();
}

void ProvisionCost::refreshSessionList()
{
	if (!_bDirty)
		return;

	// enumerate all the provision session, then try to push into the bookMap
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ProvisionCost, "enumerate all the sessions for book"));
	IdentCollection identities;
	try	{
//		ZQ::common::MutexGuard gd(_env._lockProvisionSession);		
		{
			::Freeze::EvictorIteratorPtr itptr = _env._eProvisionSession->getIterator("", 300);
			while (itptr->hasNext())
				identities.push_back(itptr->next());
		}		
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(CPE, "failed to list all the existing sessions"));
	}
	
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(CPE, "%d session(s) found"), identities.size());
	
	for (IdentCollection::iterator it = identities.begin(); it !=identities.end(); it ++)
	{
		try {
			::TianShanIce::ContentProvision::ProvisionSessionExPrx sess =IdentityToObjEnv(_env, ProvisionSessionEx, *it);
#ifdef _DEBUG
			std::string sessid = it->name;
#endif // _DEBUG
			
			::TianShanIce::ContentProvision::ProvisionState state = sess->getState();
			if (state < ::TianShanIce::ContentProvision::cpsAccepted && state >= ::TianShanIce::ContentProvision::cpsStopped)
				continue;
			
			std::string startTimeUTC, endTimeUTC;
			sess->getScheduledTime(startTimeUTC, endTimeUTC);
			Ice::Long timeStart = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());
			Ice::Long timeEnd = ZQTianShan::ISO8601ToTime(endTimeUTC.c_str());

			if (timeEnd < _itemToBook.st || timeStart > _itemToBook.et)
				continue;
			
			std::string methodType = sess->getMethodType();
			uint32 bandwidth = (uint32) sess->getBandwidth();

			if (bandwidth>0)
				pushBookedItem(methodType, timeStart, timeEnd, bandwidth);
		}
		catch(...) {}
	}

	_bDirty = false;
}

void ProvisionCost::pushBookedItem(const std::string& methodType, Ice::Long timeStart, Ice::Long timeEnd, uint32 bandwidth)
{
	// ignore those would not affect this booking
	if (_methodTypeToBook.empty() || methodType.empty())
		return;

	BookItem bn;
	bn.st = timeStart;
	bn.et = timeEnd;
	bn.instance = 1;
	bn.bandwidth = bandwidth;
	bn.cost = 0;

/*
	if (_bookMap.end() == _bookMap.find(methodType))
	{
		// initialize with an empty chain for the method type
		BookChain chain;
		_bookMap.insert(methodType, chain);
	}
*/
	merge(methodType, bn, _bookMap[methodType], _env._provisionFactory->findHelper(methodType.c_str()));
}

uint16 ProvisionCost::merge(const std::string& methodType, const BookItem& in, BookChain& chain, ZQTianShan::ContentProvision::ICPHelper* pHelper, bool maxCostMode)
{
	chain.clear();

	// find the helper
	if (!maxCostMode && NULL == pHelper)
		return MAX_LOAD_VALUE;

	// locate where to start insert;
	BookChain::iterator i = chain.begin();
	for (; i < chain.end(); i++)
	{
		if (TIMECMP(in.st, i->et) <0)
			break;
	}
	
	BookItem node;
	node.st = in.st;
	node.et = in.et;
	node.instance = in.instance;
	node.bandwidth = in.bandwidth;
	uint16 maxCost = 0;
	
	while(TIMECMP(node.st, in.et) <0 && maxCost < MAX_LOAD_VALUE)
	{
		if (i == chain.end() || TIMECMP(node.et, i->st) <=0)
		{
			// simply insert a node here
			chain.insert(i, node);
			CHAIN_TRACE;
			break;
		}
		
		if (TIMECMP(node.st, i->st) <0)
		{
			//				node.value = in.value;
			node.instance = in.instance;
			node.bandwidth = in.bandwidth;
			node.et = i->st;

			try {
				node.cost = maxCostMode ? in.cost : pHelper->evaluateCost(methodType.c_str(), node.bandwidth /1000, node.instance);
			}catch(...) { maxCost = node.cost = MAX_LOAD_VALUE; }
			maxCost = max(maxCost, node.cost);

			i = chain.insert(i, node); // insert front
			i++;
			node.st = node.et;
			node.et = in.et;
			CHAIN_TRACE;
			continue;
		}
		
		if (TIMECMP(node.st, i->st) >0)
		{
			node.et = i->et;
			i->et = node.st;
			//				node.value = i->value + in.value;
			node.instance = i->instance + in.instance;
			node.bandwidth = i->bandwidth + in.bandwidth;
			
			try {
				node.cost = maxCostMode ? (i->cost + in.cost)
					       : pHelper->evaluateCost(methodType.c_str(), node.bandwidth /1000, node.instance);
			} catch(...) { return MAX_LOAD_VALUE; }
			maxCost = max(maxCost, node.cost);
			
			if (TIMECMP(in.et, node.et) >=0)
			{
				i = chain.insert(i+1, node); // insert back
				i++;
				node.st = node.et;
				//					node.value = in.value;
				node.instance = in.instance;
				node.bandwidth = in.bandwidth;
				try {
					node.cost = maxCostMode ? (in.cost)
						: pHelper->evaluateCost(methodType.c_str(), node.bandwidth /1000, node.instance);
				} catch(...) { return MAX_LOAD_VALUE; }
				maxCost = max(maxCost, node.cost);

				node.et = in.et;
			}
			else
			{
				Ice::Long et = node.et;
				node.et = in.et;
				i = chain.insert(i+1, node);
				i++;
				node.st = in.et;
				node.et = et;
				//					node.value -= in.value;
				node.instance -= in.instance;
				node.bandwidth -= in.bandwidth;
				try {
					node.cost = maxCostMode ? (node.cost - in.cost)
						: pHelper->evaluateCost(methodType.c_str(), node.bandwidth /1000, node.instance);
				} catch(...) { return MAX_LOAD_VALUE; }
				maxCost = max(maxCost, node.cost);

				i = chain.insert(i, node);
				i++;
			}
			
			node.et = in.et;
			continue;
		}
		
		if (TIMECMP(node.st, i->st) ==0 && TIMECMP(in.et, i->st) >=0)
		{
			if (TIMECMP(in.et, i->et) >=0)
			{
				//					i->value += in.value;
				i->instance += in.instance;
				i->bandwidth += in.bandwidth;
				try {
					i->cost = maxCostMode ? (i->cost - in.cost)
						: pHelper->evaluateCost(methodType.c_str(), i->bandwidth /1000, i->instance);
				} catch(...) { return MAX_LOAD_VALUE; }
				maxCost = max(maxCost, i->cost);

				node.st = i->et;
			}
			else
			{
				//					node.value = i->value;
				node.instance = i->instance;
				node.bandwidth = i->bandwidth;
				try {
					node.cost = maxCostMode ? (i->cost)
						: pHelper->evaluateCost(methodType.c_str(), node.bandwidth /1000, node.instance);
				}catch(...) { node.cost = MAX_LOAD_VALUE; }
				node.et = i->et;
				node.st = in.et;
				i->et = in.et;
				//					i->value += in.value;
				i->instance += in.instance;
				i->bandwidth += in.bandwidth;
				try {
					i->cost = maxCostMode ? (i->cost + in.cost)
						: pHelper->evaluateCost(methodType.c_str(), i->bandwidth /1000, i->instance);
				} catch(...) { return MAX_LOAD_VALUE; }
				maxCost = max(maxCost, i->cost);

				chain.insert(i+1, node); // insert behind
				CHAIN_TRACE;
				break;
			}
			
		}
		
		node.et = in.et;
		i++;
	}
	// end of build up chainput chain;

	CHAIN_TRACE;
	return maxCost;
}

bool ProvisionCost::book(const uint32 bandwidth)
{
	if (_methodTypeToBook.empty() || bandwidth <=0)
		return false;

	BookChain chainOfMethod, chainOfCost;

//	if (_bookMap.end() != _bookMap.find(methodType))
		chainOfMethod = _bookMap[_methodTypeToBook];
//	else
//		_bookMap.insert(BookMap::value_type(methodType, chainOfMethod));

	// step 1. try merge this new item into the chain of the method
	_itemToBook.bandwidth = bandwidth;
	if (merge(_methodTypeToBook, _itemToBook, chainOfMethod, _env._provisionFactory->findHelper(_methodTypeToBook.c_str())) >= MAX_LOAD_VALUE)
		return false;

	// step 2. perform integrated booking via cost only mode
	chainOfCost = chainOfMethod;
	for (BookMap::const_iterator itMap = _bookMap.begin(); itMap != _bookMap.end(); itMap++ )
	{
		// skip those of the same method type
		if (0 == _methodTypeToBook.compare(itMap->first))
			continue;

		const BookChain& chain = itMap->second;

		for (BookChain::const_iterator it = chain.begin(); it < chain.end(); it++)
		{
			// ignore those not in this range
			if (it->st < _itemToBook.st || it->et > _itemToBook.et)
				continue;

			if (merge(itMap->first, _itemToBook, chainOfCost, NULL, true) >= MAX_LOAD_VALUE)
				return false;
		}
	}

	_bookMap[_methodTypeToBook] = chainOfMethod;

	return true;
}


}}
