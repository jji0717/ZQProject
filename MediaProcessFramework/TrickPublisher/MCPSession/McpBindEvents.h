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
// Ident : $Id: McpBindEvents.h,v 1.3 2004/06/28 05:38:38 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : Define base  class McpBindEvents  for events of mcp objects
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/MCPSession/McpBindEvents.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     05-04-25 15:41 Hongye.gu
// Revision 1.3  2004/06/28 05:38:38  wli
// Modified to be compatible to the MBCS instead of UNICODE in project setting
//
// Revision 1.2  2004/06/23 07:24:07  wli
// Add comment file head
//
// Revision 1.1  2004/06/23 06:42:41  wli
// file created
// ===========================================================================

// McpBindEvents.h: interface for the McpBindEvents class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__ZQ_MCPBINDEVENTS_H__)
#define __ZQ_MCPBINDEVENTS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MCPApi.h"
#include <string>


class McpBindEvents 
{
	public:
		virtual void OnClose(MCP_CUR_STATE tState) = 0;
		virtual void OnFatalFailed(std::string tWhy) = 0;
		virtual void OnEvent(LPMCP_IOSTATUS pStat) = 0;
	McpBindEvents();
	virtual ~McpBindEvents();
	

};

#endif // !defined(__ZQ_MCPBINDEVENTS_H__)
