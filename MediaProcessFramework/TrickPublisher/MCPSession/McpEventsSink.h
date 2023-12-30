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
// Ident : $Id: McpEventsSink.h,v 1.6 2004/07/01 07:46:19 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : Define   class McpEventsSink  to sink and dispatch object events
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/MCPSession/McpEventsSink.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     05-04-25 15:41 Hongye.gu
// Revision 1.6  2004/07/01 07:46:19  wli
// change the order of include file to correct reinclude windows.h
//
// Revision 1.5  2004/07/01 06:28:19  jshen
// after merge
//
// Revision 1.4  2004/06/28 05:38:38  wli
// Modified to be compatible to the MBCS instead of UNICODE in project setting
//
// Revision 1.3  2004/06/24 11:16:36  wli
// modify the include file
//
// Revision 1.2  2004/06/23 07:24:07  wli
// Add comment file head
//
// Revision 1.1  2004/06/23 06:42:41  wli
// file created
// ===========================================================================
// McpEventsSink.h: interface for the McpEventsSink class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__ZQ_MCPEVENTS_SINK_H__)
#define __ZQ_MCPEVENTS_SINK_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "McpObject.h"
#include "../mcpsdk/MCPUtility.h"
#include "ObjectMap.h"


class McpEventsSink:public ZQ::common::ObjectMap<MCP_HANDLE,McpObject>  
{
public:
	McpEventsSink();
	virtual ~McpEventsSink();
//	static  void processEvent(MCP_HANDLE handle,LPMCP_IOSTATUS pStat);
	static  void dispatchEvents(MCP_HANDLE handle,LPMCP_IOSTATUS pStat);
};


#endif // !defined(__ZQ_MCPEVENTS_SINK_H__)
