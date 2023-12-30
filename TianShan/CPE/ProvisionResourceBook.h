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
// Ident : $Id: ProvisionResourceBook.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/ProvisionResourceBook.h $
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
// 2     08-04-09 13:26 Hui.shao
// 
// 1     08-04-09 13:24 Hui.shao
// ---------------------------------------------------------------------------

#ifndef __ZQTianShan_CPE_ProvisionCost_H__
#define __ZQTianShan_CPE_ProvisionCost_H__

#include "ZQ_common_conf.h"
#include <string>
#include <deque>
#include <map>
#include "IMethodCost.h"


namespace ZQTianShan {
namespace ContentProvision {



class ProvisionChain
{
public:
	struct ProvisionItem
	{
		unsigned int st, et;
		unsigned int bandwidth;			//in Kbps
		unsigned int instance;
	};

	ProvisionChain(MethodCost* pMethodCost);

	//return false if over load, else return true
	bool add(const ProvisionItem& in);

	unsigned int getMaxCost();
	
	bool isOverLoad();

	std::string getMethodCategory();

protected:

	struct BookItem : public ProvisionItem
	{
		unsigned int cost;
	};

	typedef std::deque< BookItem > BookChain;

	//update the cost in BookItem,
	//and update the max cost
	void setProvisionCost(BookItem& node);

protected:

	BookChain		_bookChain;
	unsigned int	_nMaxCost;
	MethodCost*		_pMethodCost;
};

class ProvisionResourceBook  
{
public:	
	ProvisionResourceBook(MethodCostCol* pMethodCostCol);
	~ProvisionResourceBook();

	void setTimeFilter(unsigned int stfilter, unsigned int etfilter);

	//
	// caution, bandwidth is in Kbps, 
	//
	bool addProvision(const std::string& methodType, unsigned int timeStart, unsigned int timeEnd, unsigned int bandwidthKbps);

	bool bookProvision(const std::string& methodType, unsigned int lstart, unsigned int lend, unsigned int bandwidthKbps);

protected:

	unsigned int				_startfilter;
	unsigned int				_endfilter;

	typedef std::map <std::string, ProvisionChain*>	 ProvisionChainMap;
	ProvisionChainMap		_chainMap;
	MethodCostCol*			_pMethodCostCol;
};


}}

#endif // __ZQTianShan_CPE_ProvisionCost_H__
