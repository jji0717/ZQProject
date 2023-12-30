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
// Ident : $Id: IMethodCost.h $
// Branch: $Name:  $
// Author: Jie Zhang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/IMethodCost.h $
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
// 5     09-05-22 17:42 Yixin.tian
// 
// 4     09-05-21 17:59 Jie.zhang
// merge from 1.10
// 
// 5     09-05-05 13:20 Jie.zhang
// 
// 4     09-04-30 14:04 Jie.zhang
// 
// 3     09-03-09 14:53 Yixin.tian
// 
// 2     09-03-04 21:33 Jie.zhang
// reflactor the provision cost source
// 
// 1     08-11-18 11:24 Jie.zhang
// 
// ---------------------------------------------------------------------------

#ifndef __ZQTianShan_CPE_MethodCost_H__
#define __ZQTianShan_CPE_MethodCost_H__

#include <string>


#define MAX_LOAD_VALUE (10000)


namespace ZQTianShan {
	namespace ContentProvision {


		class MethodCost
		{
		public:
			virtual ~MethodCost(){}
			/// evaluate the cost per given session count and total allocated bandwidth
			///@param[in] bandwidthKbps to specify the allocated bandwidth in Kbps
			///@param[in] sessions to specify the allocated session instances
			///@return a cost in the range of [0, MAX_LOAD_VALUE+] at the given load level:
			///		0 - fully available
			///		MAX_LOAD_VALUE	 - just available and no more could accept
			///		MAX_LOAD_VALUE+	 - not available
			virtual unsigned int evaluateCost(unsigned int bandwidthKbps, unsigned int sessions) =0;

			bool isOverLoad(unsigned int nCost)
			{
				return (nCost > MAX_LOAD_VALUE);
			}

			//
			// return the provision method category, methods in one plug-in are in same category
			//
			virtual std::string getCategory() = 0;
		};

		class MethodCostCol
		{
		public:
			virtual ~MethodCostCol(){}
			virtual MethodCost* getMethodCost(const std::string& methodType) = 0;
			
			bool isOverLoad(unsigned int nCost)
			{
				return (nCost > MAX_LOAD_VALUE);
			}
		};

	}
}


#endif

