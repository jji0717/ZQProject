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
// Ident : $Id: McpObject.h,v 1.14 2004/08/09 07:57:34 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : Define base  class McpObject  publisher and subscriber
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/MCPSession/McpObject.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     05-04-25 15:41 Hongye.gu
// 
// 7     04-12-14 15:00 Jie.zhang
// 
// 6     04-11-26 18:32 Jie.zhang
// 
// 5     04-11-04 18:44 Kaliven.lee
// 
// 4     04-10-28 16:38 Kaliven.lee
// Revision 1.14  2004/08/09 07:57:34  wli
// unicode modified
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
// Revision 1.9  2004/07/05 08:04:43  wli
// no message
//
// Revision 1.8  2004/07/05 06:24:21  wli
// modify construct function to get mcp setting and mediainfo defaultly
//
// Revision 1.7  2004/07/01 06:30:50  jshen
// resolve reinclude windows.h problem
//
// Revision 1.6  2004/07/01 06:27:44  jshen
// after merge
//
// Revision 1.5  2004/07/01 05:54:39  wli
// add comment
//
// Revision 1.4  2004/06/30 06:55:27  wli
// no message
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
//

// ===========================================================================
// McpObject.h: interface for the McpObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__ZQ_MCPOBJECT_H__)
#define __ZQ_MCPOBJECT_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "../StdAfx.h"
#include <string>
#include "../mcpsdk/McpApi.h"
#include "../mcpsdk/MCPMsgs.h"
#include "../mcpsdk/MCPUtility.h"


#define PRINT_HANDLE(_x) (DWORD)((_x) >> 32), (DWORD)((_x))

#define	MCPSESSION_FAILED_CHK_TIMEOUT		4000
#define MAX_FILE_NAME_LENGTH				1024

typedef enum _tagOBJTYPE{OBJ_TYPE_SUB =0 ,OBJ_TYPE_PUB
}OBJTYPE;
typedef struct _tagMediaInfo
{
	std::wstring fileName;
	std::wstring subName; //subscribe file name
	ULONGLONG fileSize;
	unsigned long ulContentBitrate;
	unsigned short uiContentHorizRes;
	unsigned short uiContentVerticalRes;

	std::string briefPrvString;
	std::string fullPrvString;

	_tagMediaInfo()
	{
		fileSize = 0;	 // always 0 because we're live transfer
		ulContentBitrate = 0;
		uiContentHorizRes = 0;
		uiContentVerticalRes = 0;
	}
}MCPMEDIAINFO;

typedef struct _tagMcpSetting
{
public:
	DWORD dwOptionMask ;
	MCP_XFER_TYPE XferType;
	MCP_RTE_STREAM_VERSION RTEStreamVer;
	MCP_RTE_STREAM_ECO RTEStreamEco;


	DWORD dwRTEZeroLenFail;
	DWORD dwDisableBufDrvThrottle;
	DWORD dwServerSideDeleteOnFail;

	std::wstring sMcastAddr;
	DWORD dwXferTimeOut;
	DWORD dwEventInterval;
	DWORD dwMaxBitRate;
	DWORD dwMaxWaitSecs;

	_tagMcpSetting()
	{
		dwOptionMask = MCP_OPTM_NONE;
		XferType = MCP_XFER_LIVE;
		RTEStreamVer = MCP_RTE_STREAM_VERSION_TWO;
		RTEStreamEco = MCP_RTE_STREAM_ECO_ZERO;

		dwRTEZeroLenFail = 0;
		dwDisableBufDrvThrottle = 0;
		dwServerSideDeleteOnFail = 0;
		dwMaxBitRate = 3750000;

		sMcastAddr = L"225.20.10.5";
		dwXferTimeOut = 0;
		dwEventInterval = 1000;
	}
}MCPSETTING;

class McpSession;
class McpObject  
{
	friend class McpSession;
	friend class McpEventsSink;
public:

	McpObject(McpSession * parent,std::wstring addr,DWORD port,std::wstring ifaceAddr);
	virtual ~McpObject();
	
	MCP_CUR_STATE getCurStatus(void);
	
	bool   isClosed()
	{
		return _bIsClose;
	}

	void	setClosed(bool bClose)
	{
		_bIsClose = bClose;
	}

	const wchar_t* getLogHeader()
	{
		return _swLogHeader;
	}

	void setLogHeader(const wchar_t* swLog)
	{
		wcscpy(_swLogHeader, swLog);
	}
	
	virtual void setCurStatus(MCP_CUR_STATE tCurStatus);
	virtual void OnClose(MCP_CUR_STATE tState) = 0;
	virtual void OnFatalFailed(std::wstring tWhy) = 0;
	virtual bool OnEvent(LPMCP_IOSTATUS pStat) = 0;

	std::wstring m_sAddr;
	std::wstring m_sIfaceAddr;
	DWORD m_dwPort;

	MCP_HANDLE m_handle;

	MCPSETTING m_McpSetting;

	OBJTYPE m_type;
protected:
	MCPMEDIAINFO m_MediaInfo;

	
	McpSession* m_parentSession;	

	private:
	
	bool	_bIsClose;
	
	MCP_CUR_STATE m_tCurStatus;
	
	wchar_t			_swLogHeader[256];
};

#endif // !defined(__ZQ_MCPOBJECT_H__)
