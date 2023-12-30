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
// Ident : $Id: IPushTrigger.h $
// Branch: $Name:  $
// Author: Jie Zhang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/IPushTrigger.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 3     08-12-12 13:45 Yixin.tian
// 
// 2     08-02-15 12:23 Jie.zhang
// changes check in
// 
// 1     08-02-13 17:47 Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_IPushTrigger_H__
#define __ZQTianShan_IPushTrigger_H__

namespace ZQTianShan {
namespace ContentProvision {
		
class IPushSource
{
public:
	virtual ~IPushSource(){}
	virtual unsigned int read(void* pBuf, unsigned int nReadBytes) = 0;
	virtual void close(bool succ, const char* szErr = 0) = 0;	
};

}} // namespace

#endif // __ZQTianShan_IPushTrigger_H__
