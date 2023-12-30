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
// Ident : $Id: ProvisionCost.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/ProvisionCost.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 3     08-12-15 17:55 Yixin.tian
// 
// 2     08-04-09 13:26 Hui.shao
// 
// 1     08-04-09 13:24 Hui.shao
// ---------------------------------------------------------------------------

#ifndef __ZQTianShan_CPE_ProvisionCost_H__
#define __ZQTianShan_CPE_ProvisionCost_H__

#include "../common/TianShanDefines.h"
#include "CPEEnv.h"
#include "ICPHelper.h"

namespace ZQTianShan {
namespace CPE {

class ProvisionCost  
{
public:
	ProvisionCost(CPEEnv& env, const std::string& methodType, Ice::Long timeStart, Ice::Long timeStop);

	// id, the returned booked RDS id
	bool book(const uint32 bandwidth);

protected:

	CPEEnv& _env;

	typedef struct _BookItem
	{
		Ice::Long st, et;
		uint32 bandwidth;
		int instance;
		uint16 cost;
	} BookItem;

	std::string _methodTypeToBook;
	BookItem _itemToBook;

	typedef std::vector < BookItem > BookChain;
	typedef std::map <std::string, BookChain> BookMap;

	bool _bDirty;
	BookMap _bookMap;
	

protected:

	void pushBookedItem(const std::string& methodType, Ice::Long timeStart, Ice::Long timeEnd, uint32 bandwidth);
	uint16 merge(const std::string& methodType, const BookItem& in, BookChain& out, ZQTianShan::ContentProvision::ICPHelper* pHelper, bool maxCostMode = false);
	void refreshSessionList();
};


}}

#endif // __ZQTianShan_CPE_ProvisionCost_H__
