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
// Ident : $Id: McpPublisher.h,v 1.14 2004/08/09 07:57:34 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : Define class McpPublisher to encapsulate the Publisher object in 
//		   MCP processing
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/MCPSession/McpPublisher.h $
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
// Revision 1.13  2004/07/15 03:19:08  wli
// fix the mcp process error. and set all attributes in bind function
//
// Revision 1.12  2004/07/14 09:18:06  jshen
// no message
//
// Revision 1.11  2004/07/13 04:10:35  jshen
// for mca session test
//
// Revision 1.10  2004/07/08 03:53:12  wli
// replace some parameters from string to wstring according MCPAPI
//
// Revision 1.9  2004/07/05 08:52:04  wli
// clean up some unused member and set mcp setting as member
//
// Revision 1.8  2004/07/01 05:54:39  wli
// add comment
//
// Revision 1.7  2004/06/30 05:48:08  wli
// rewrite callback to mcpsession
//
// Revision 1.6  2004/06/29 05:52:13  wli
// some parament errors after a process review
//
// Revision 1.5  2004/06/28 05:38:38  wli
// Modified to be compatible to the MBCS instead of UNICODE in project setting
//
// Revision 1.4  2004/06/24 11:17:40  wli
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
// McpPublisher.h: interface for the McpPublisher class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__ZQ_MCPPUBLISHER_H__)
#define __ZQ_MCPPUBLISHER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include <string>
#include "McpObject.h"

#define MAX_USER_BRIEF_PRIVATE_DATA_LENGTH	12


//this segment code is from project "multiDest" of Maynard
// brief private data stuff taken from MCA.h
#define	MCA_BRIEF_PRIVATE_DATA_VERSION			(1)
#define	MCA_BRIEF_PRIVATE_DATA_MAGIC			(0xBD)
#pragma pack( 1 )
typedef union _MCA_BRIEF_PRIVATE_DATA
{
	unsigned char rawData[ MAX_USER_BRIEF_PRIVATE_DATA_LENGTH -1 ];
	struct
	{
		unsigned char magic;
		unsigned char version;
		unsigned long bitrateBPS;
		unsigned short usHRes;
		unsigned short usVRes;
	};
} MCA_BRIEF_PRIVATE_DATA, *PMCA_BRIEF_PRIVATE_DATA;
#pragma pack()

class McpSession;

class McpPublisher:public McpObject  
{
	public:
		
		BOOL checkAliving();

	//	static void processEvent(MCP_HANDLE handle, LPMCP_IOSTATUS pStat);
		MCP_STATUS getPrivateData(PMCA_BRIEF_PRIVATE_DATA pbrifeData,std::string fullData);
		MCP_STATUS setPrivateData(void);
		MCP_STATUS open(void);
		MCP_STATUS close(void);
		MCP_STATUS publish(void);
		MCP_STATUS bind(DWORD destinePort);
		std::wstring getPubGSID(void);

		virtual void setCurStatus(MCP_CUR_STATE tCurStatus);

		void setMediaInfo(MCPMEDIAINFO& mediainfo);
		void setMcpSetting(MCPSETTING&  mcpSetting);
	
		McpPublisher(McpSession* parent,std::wstring addr,DWORD port,std::wstring ifaceAddr);
		void OnClose(MCP_CUR_STATE tState);
		bool OnEvent(LPMCP_IOSTATUS pStat);
		void OnFatalFailed(std::wstring tWhy){}
		virtual ~McpPublisher();
	private:
		std::wstring m_GSID;

		DWORD		_dwLastCheckTime;
};

#endif // !defined(__ZQ_MCPPUBLISHER_H__)
