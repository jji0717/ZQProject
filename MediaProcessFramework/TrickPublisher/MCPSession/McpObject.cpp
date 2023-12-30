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
// Ident : $Id: McpObject.cpp,v 1.8 2004/07/16 03:08:13 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : implement class McpObject for MCP object
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/MCPSession/McpObject.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     05-04-25 15:41 Hongye.gu
// 
// 2     04-12-14 15:00 Jie.zhang
// Revision 1.8  2004/07/16 03:08:13  wli
// init m_handle as MCP_INVALID_HANDLE
//
// Revision 1.7  2004/07/15 09:00:10  wli
// first stab version.
//
// Revision 1.6  2004/07/08 03:53:12  wli
// replace some parameters from string to wstring according MCPAPI
//
// Revision 1.5  2004/07/05 08:04:43  wli
// no message
//
// Revision 1.4  2004/07/05 06:24:21  wli
// modify construct function to get mcp setting and mediainfo defaultly
//
// Revision 1.3  2004/06/29 05:52:13  wli
// some parament errors after a process review
//
// Revision 1.2  2004/06/28 05:38:38  wli
// Modified to be compatible to the MBCS instead of UNICODE in project setting
//
// Revision 1.1  2004/06/24 11:15:59  wli
// Create to replace the bindevents class
//




#include "McpObject.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

McpObject::McpObject(McpSession *parent,std::wstring addr,DWORD port,std::wstring ifaceAddr)
{
	m_parentSession = parent;
	m_sAddr = addr;
	m_dwPort = port;
	m_sIfaceAddr = ifaceAddr;
	_bIsClose = true;
	m_handle = MCP_INVALID_HANDLE;
	_swLogHeader[0] = L'\0';
}

McpObject::~McpObject()
{
	
}
 

MCP_CUR_STATE McpObject::getCurStatus(void)
{
	return m_tCurStatus;
}


 void McpObject::setCurStatus(MCP_CUR_STATE tCurStatus)
 {
	 m_tCurStatus = tCurStatus;
 }
