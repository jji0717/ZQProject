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
// Ident : $Id: ProvisionResourceBook.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/ProvisionResourceBook.cpp $
// 
// 2     2/14/11 4:53p Jie.zhang
// methods only share the resource in one plug-in
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
// 5     09-05-05 14:37 Jie.zhang
// 
// 4     09-05-05 13:20 Jie.zhang
// 
// 3     09-03-09 14:54 Yixin.tian
// midify LONGLONG to int64
// 
// 2     09-03-04 21:33 Jie.zhang
// reflactor the provision cost source
// 
// 1     08-11-18 11:24 Jie.zhang
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

#include "IMethodCost.h"
#include "ProvisionResourceBook.h"
#include <assert.h>


//#define  CHAIN_TRACE

#if !defined(_DEBUG)
#  undef  CHAIN_TRACE
#endif // _DEBUG

#ifdef CHAIN_TRACE
#undef CHAIN_TRACE
#define CHAIN_TRACE	{ printf("\n%s(%3d)\n", __FILE__, __LINE__);\
					for (BookChain::iterator cit = _bookChain.begin(); cit < _bookChain.end(); cit++) \
					printf("NODE{st=%d; et=%d; ld=%d} ", cit->st, cit->et, cit->cost); }
#else
#define CHAIN_TRACE
#endif

namespace ZQTianShan {
namespace ContentProvision {


ProvisionResourceBook::ProvisionResourceBook(MethodCostCol* pMethodCost)
	:_pMethodCostCol(pMethodCost)
{
	assert(_pMethodCostCol != 0);

	_startfilter = 0;
	_endfilter = 0;
}

ProvisionResourceBook::~ProvisionResourceBook()
{
	ProvisionChainMap::iterator it = _chainMap.begin();
	for(;it != _chainMap.end();it++)
	{
		if (it->second)
		{
			delete it->second;
		}
	}

	_chainMap.clear();
}

void ProvisionResourceBook::setTimeFilter(unsigned int stfilter, unsigned int etfilter)
{
	_startfilter = stfilter;
	_endfilter = etfilter;
}

bool ProvisionResourceBook::addProvision(const std::string& methodType, unsigned int timeStart, unsigned int timeEnd, unsigned int bandwidthKbps)
{
	if (!bandwidthKbps)
		return true;

	if (_startfilter && timeEnd <= _startfilter)
		return true;

	if (_endfilter && timeStart >= _endfilter)
		return  true;

	unsigned int llStartTime, lllEndTime;
	if (_startfilter && timeStart < _startfilter)
		llStartTime = _startfilter;
	else
		llStartTime = timeStart;

	if (_endfilter && timeEnd > _endfilter)
		lllEndTime = _endfilter;
	else
		lllEndTime = timeEnd;

	return bookProvision(methodType, llStartTime, lllEndTime, bandwidthKbps);
}

bool ProvisionResourceBook::bookProvision(const std::string& methodType, unsigned int timeStart, unsigned int timeEnd, unsigned int bandwidthKbps)
{
	if (methodType.empty()|| !bandwidthKbps)
		return false;

	std::string strMethodCatory;

	// step 1. add to provision chain, and check if over load	
	{
		ProvisionChain*	pProvisionChain = NULL;

		//find the provision chain, create if not exist
		ProvisionChainMap::iterator it = _chainMap.find(methodType);
		if(it == _chainMap.end())
		{
			MethodCost* pMethodCost = _pMethodCostCol->getMethodCost(methodType);
			if (!pMethodCost)
				return false;			// no this methods

			pProvisionChain = new ProvisionChain(pMethodCost);	
			_chainMap[methodType] = pProvisionChain;
		}
		else
		{
			pProvisionChain = it->second;
		}

		ProvisionChain::ProvisionItem itemToBook;
		itemToBook.st = timeStart;
		itemToBook.et = timeEnd;
		itemToBook.bandwidth = bandwidthKbps;
		itemToBook.instance = 1;

		if (!pProvisionChain->add(itemToBook))
		{
			return false;
		}

		strMethodCatory = pProvisionChain->getMethodCategory();
	}

	// step 2. perform integrated booking on all method types
	ProvisionChainMap::iterator it;
	unsigned int nTotalCost = 0;
	for (it = _chainMap.begin(); it != _chainMap.end(); it++ )
	{
		ProvisionChain* pChain = it->second;

		// only methods in one plug-in will perform integrated booking
		if (pChain->getMethodCategory() != strMethodCatory)
			continue;

		nTotalCost += pChain->getMaxCost();
		
		if (_pMethodCostCol->isOverLoad(nTotalCost))
		{
			return false;
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
// ProvisionChain
//

ProvisionChain::ProvisionChain(MethodCost* pMethodCost)
:_pMethodCost(pMethodCost)
{
	assert(_pMethodCost != 0);
	_nMaxCost = 0;
}

bool ProvisionChain::add( const ProvisionItem& in )
{
	// locate where to start insert;
	BookChain::iterator it = _bookChain.begin();
	for (; it < _bookChain.end(); it++)
	{
		if (in.st < it->et)
			break;
	}

	BookItem node;
	node.st = in.st;
	node.et = in.et;
	node.instance = in.instance;
	node.bandwidth = in.bandwidth;
	node.cost = 0;

	while(node.st < in.et && !_pMethodCost->isOverLoad(_nMaxCost))
	{
		if (it == _bookChain.end() || node.et <= it->st)
		{
			// simply insert a node here
			setProvisionCost(node);
			_bookChain.insert(it, node);

			CHAIN_TRACE;
			break;
		}

		if (node.st < it->st)
		{
			node.instance = in.instance;
			node.bandwidth = in.bandwidth;
			node.et = it->st;

			setProvisionCost(node);
			it = _bookChain.insert(it, node); // insert front
			it++;
			node.st = node.et;
			node.et = in.et;
			CHAIN_TRACE;
			continue;
		}

		if (node.st > it->st)
		{
			node.et = it->et;
			it->et = node.st;

			node.instance = it->instance + in.instance;
			node.bandwidth = it->bandwidth + in.bandwidth;

			setProvisionCost(node);

			if (in.et >= node.et)
			{
				it = _bookChain.insert(it+1, node); // insert back
				it++;

				node.st = node.et;
				node.et = in.et;
				node.instance = in.instance;
				node.bandwidth = in.bandwidth;
			}
			else
			{
				unsigned int et = node.et;
				node.et = in.et;
				it = _bookChain.insert(it+1, node);
				it++;
				node.st = in.et;
				node.et = et;
				
				node.instance -= in.instance;
				node.bandwidth -= in.bandwidth;
				setProvisionCost(node);

				it = _bookChain.insert(it, node);
				it++;
			}

			node.et = in.et;
			continue;
		}

		if (node.st == it->st && in.et >= it->st)
		{
			if (in.et >= it->et)
			{
				//					it->value += in.value;
				it->instance += in.instance;
				it->bandwidth += in.bandwidth;
				setProvisionCost(*it);

				node.st = it->et;
			}
			else
			{
				node.instance = it->instance;
				node.bandwidth = it->bandwidth;
				node.et = it->et;
				node.st = in.et;

				it->et = in.et;			
				it->instance += in.instance;
				it->bandwidth += in.bandwidth;
				setProvisionCost(*it);
				
				setProvisionCost(node);
				_bookChain.insert(it+1, node); // insert behind
				CHAIN_TRACE;
				break;
			}
		}

		node.et = in.et;
		it++;
	}
	// end of build up chainput chain;

	CHAIN_TRACE;
	return !_pMethodCost->isOverLoad(_nMaxCost);
}

unsigned int ProvisionChain::getMaxCost()
{
	return _nMaxCost;
}

bool ProvisionChain::isOverLoad()
{
	return _pMethodCost->isOverLoad(_nMaxCost);
}

void ProvisionChain::setProvisionCost( BookItem& node)
{
	node.cost = _pMethodCost->evaluateCost(node.bandwidth, node.instance);

	if (node.cost > _nMaxCost)
		_nMaxCost = node.cost;
}

std::string ProvisionChain::getMethodCategory()
{
	return _pMethodCost->getCategory();
}

}}

