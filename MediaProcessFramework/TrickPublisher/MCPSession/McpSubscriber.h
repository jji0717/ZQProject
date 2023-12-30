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
// Ident : $Id: McpSubscriber.h,v 1.14 2004/08/09 07:57:34 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : define class McpSubscriber to encapsulate MCP subscriber
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/MCPSession/McpSubscriber.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     05-04-25 15:41 Hongye.gu
// 
// 8     04-12-14 15:00 Jie.zhang
// 
// 7     04-11-26 18:32 Jie.zhang
// 
// 6     04-11-22 19:47 Jie.zhang
// arranged source & added detail log message.
// 
// 5     04-11-22 14:27 Jie.zhang
// 
// 4     04-11-04 18:44 Kaliven.lee
// Revision 1.14  2004/08/09 07:57:34  wli
// unicode modified
//
// Revision 1.13  2004/07/15 03:19:26  wli
// fix the mcp process error. and set all attributes in bind function
//
// Revision 1.12  2004/07/08 03:53:12  wli
// replace some parameters from string to wstring according MCPAPI
//
// Revision 1.11  2004/07/05 08:52:11  wli
// clean up some unused member and set mcp setting as member
//
// Revision 1.10  2004/07/01 07:46:08  wli
// change the order of include file to correct reinclude windows.h
//
// Revision 1.9  2004/07/01 06:28:19  jshen
// after merge
//
// Revision 1.8  2004/07/01 05:54:27  wli
// add comment
//
// Revision 1.7  2004/06/30 06:52:28  wli
// no message
//
// Revision 1.5  2004/06/28 05:38:38  wli
// Modified to be compatible to the MBCS instead of UNICODE in project setting
//
// Revision 1.4  2004/06/24 11:17:51  wli
// modified to inherit from class mcpobject
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
// McpSubscriber.h: interface for the McpSubscriber class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__ZQ_MCPSUBSCRIBER_H__)
#define __ZQ_MCPSUBSCRIBER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <string>


#include "McpObject.h"


class McpSession;
class McpSubscriber : public McpObject
{
public:
//fire functions
	
	void OnFatalFailed(std::wstring){};
	void OnClose(MCP_CUR_STATE tState);
	/*
	Function:       OnEvent
	Description:    retrieve event attributes
	Parameter:      LPMCP_IOSTATUS		pStat		a pointer to MCP IO status
	Return:         void
	Remark:         reserved for further develop
	*/
	bool OnEvent(LPMCP_IOSTATUS pStat);

	MCP_STATUS bind(std::wstring pubGSI,DWORD pubPort);
	MCP_STATUS open(void);
	MCP_STATUS close(void);
	MCP_STATUS subscribe(void);

	void setMcpSetting(MCPSETTING& mcpSetting);
	void setMediaInfo(MCPMEDIAINFO& mediaInfo);
	
	virtual void setCurStatus(MCP_CUR_STATE tCurStatus);

	BOOL checkAliving();
	
	McpSubscriber(McpSession* parent,std::wstring Addr,DWORD Port,std::wstring IfaceAddr);
	virtual ~McpSubscriber();	

private:
	DWORD		_dwLastCheckTime;
};

#endif // !defined(__ZQ_MCPSUBSCRIBER_H__)
