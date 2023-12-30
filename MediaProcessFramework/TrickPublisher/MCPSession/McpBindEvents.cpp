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
// Ident : $Id: McpBindEvents.cpp,v 1.5 2004/07/01 06:28:18 jshen Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : implement class McpBindEvents  to bind event for MCP object
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/MCPSession/McpBindEvents.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     05-04-25 15:40 Hongye.gu
// Revision 1.5  2004/07/01 06:28:18  jshen
// after merge
//
// Revision 1.4  2004/06/28 05:38:38  wli
// Modified to be compatible to the MBCS instead of UNICODE in project setting
//
// Revision 1.3  2004/06/23 13:00:54  wli
// no message
//
// Revision 1.2  2004/06/23 07:24:07  wli
// Add comment file head
//
// Revision 1.1  2004/06/23 06:42:41  wli
// file created
// ===========================================================================
// McpBindEvents.cpp: implementation of the McpBindEvents class.
//
//////////////////////////////////////////////////////////////////////

#include "McpBindEvents.h"
#include "McpEventsSink.h"
//#include "MCPMsgs.h"
//#include "Mcpsession.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern McpEventsSink g_EventSink;
extern MCPSETTING *g_pMcpSetting;
McpBindEvents::McpBindEvents()
{

}

McpBindEvents::~McpBindEvents()
{

}

