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
// Ident : $Id: McpSession.h,v 1.24 2004/08/17 02:56:28 jshen Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : Define class McpSession to deal the session of MCP
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/MCPSession/McpSession.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     05-04-25 15:41 Hongye.gu
// 
// 10    04-12-14 15:00 Jie.zhang
// 
// 9     04-11-26 18:32 Jie.zhang
// 
// 8     04-11-22 19:47 Jie.zhang
// arranged source & added detail log message.
// 
// 7     04-11-22 14:27 Jie.zhang
// 
// 6     04-11-09 19:11 Jie.zhang
// 
// 5     04-11-04 18:44 Kaliven.lee
// Revision 1.24  2004/08/17 02:56:28  jshen
// remove the global threadpool
//
// Revision 1.23  2004/08/09 07:57:34  wli
// unicode modified
//
// Revision 1.22  2004/07/22 01:44:59  jshen
// add node_rest_t list
//
// Revision 1.21  2004/07/20 10:33:22  jshen
// no message
//
// Revision 1.20  2004/07/16 08:43:44  wli
// error message
//
// Revision 1.19  2004/07/16 06:43:12  jshen
// no message
//
// Revision 1.18  2004/07/15 09:03:32  wli
// first stab version
//
// Revision 1.17  2004/07/13 04:10:35  jshen
// for mca session test
//
// Revision 1.16  2004/07/08 03:53:12  wli
// replace some parameters from string to wstring according MCPAPI
//
// Revision 1.15  2004/07/07 06:33:45  wli
// add enum define
//
// Revision 1.14  2004/07/06 08:04:35  wli
// modified enum define
//
// Revision 1.13  2004/07/06 08:00:16  wli
// add status  as member
//
// Revision 1.11  2004/07/02 09:45:52  wli
// set mcpsetting as member and fix some allocate error
//
// Revision 1.10  2004/07/02 05:46:39  wli
// no message
//
// Revision 1.9  2004/07/01 07:46:19  wli
// change the order of include file to correct reinclude windows.h
//
// Revision 1.8  2004/07/01 06:30:50  jshen
// resolve reinclude windows.h problem
//
// Revision 1.7  2004/07/01 06:25:11  jshen
// after merge
//
// Revision 1.6  2004/07/01 05:54:39  wli
// add comment
//
// Revision 1.5  2004/06/30 06:45:39  wli
// add comment
//
// Revision 1.3  2004/06/28 05:38:38  wli
// Modified to be compatible to the MBCS instead of UNICODE in project setting
//
// Revision 1.2  2004/06/23 07:24:07  wli
// Add comment file head
//
// Revision 1.1  2004/06/23 06:42:41  wli
// file created
// ===========================================================================
// McpSession.h: interface for the McpSession class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__ZQ_MCPSESSION_H__)
#define __ZQ_MCPSESSION_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "McpPublisher.h"
#include "McpSubscriber.h"
#include <string>
#include <vector>
#include "../TThread.h"


#pragma warning(disable: 4786)

const int CHECK_ALIVE_SPAN = 5000;


typedef /* [v1_enum] */ 
enum _MCPSESSIONSTATUS
{	MCP_SESSION_NONE = 0,
	MCP_SESSION_RUNING = MCP_SESSION_NONE + 1,
	MCP_SESSION_FAILED = MCP_SESSION_RUNING + 1,
	MCP_SESSION_CLOSED = MCP_SESSION_FAILED + 1,
}	MCPSESSIONSTATUS;

class McpSession:public TThread
{
	friend class McpSubscriber;
	friend class McpEventsSink;
public:
	/*
	    Function:       fatalFailed
	    Description:    process the fatal failed event 
	    Parameter:      std::wstring		tWhy	
	    Return:         void
	    Remark:         
	*/
	
	virtual void fatalFailed(const char* tWhy = NULL);
	/*
	    Function:       setPublisher
	    Description:    set publisher for subscriber
	    Parameter:      string			Addr		IP address for publisher
						DWORD			Port		publisher's port default is 5000
						string			IfaceAddr	interface address
	    Return:         void
	    Remark:         this function must be called before mcpsession start. otherwise mcpsession will be failed
	*/
	
	void setPublisher(const wchar_t* Addr,DWORD Port, const wchar_t* IfaceAddr);
	/*
	    Function:       addSubscriber
	    Description:    add subscriber define to session
	    Parameter:      string			
	    Return:         void
	    Remark:         session maitance a subscriber vector.this function insert a subscriber into the vector
	*/
	
	void addSubscriber(const wchar_t* subName, const wchar_t* Addr,DWORD Port, const wchar_t* IfaceAddr);
	/*
	    Function:       setMediaAttr
	    Description:    set media attribute that will be transfered
	    Parameter:      string			fileName				set media file name. the raw file name only
						unsigned long	ulContentBitrate		set media file bitrate
						unsigned short	uiContentHorizRes		set media file horizontal rate
						unsigned short	uiContentVerticalRes	set media file vertical rate
						string			briefPrvStr				set brief private string 
						string			fullPrvStr				set full private string,Max length is 2000
	    Return:         void
	    Remark:         this function must be call before session start;otherwise session will be failed.And if ulContentBitrate
						uiContentHorizRes  uiContentVerticalRes are 0,briefPrvStr and fullPrvStr are NULL, private data will not
						be set for publisher.
	*/
	
	void setMediaAttr(const wchar_t* fileName,		
					  unsigned long ulContentBitrate,
					  unsigned short uiContentHorizRes,
					  unsigned short uiContentVerticalRes,
					  const wchar_t* briefPrvStr,
					  const wchar_t* fullPrvStr);
	/*
	    Function:       setMcpAttr
	    Description:    set MCP attributes 
	    Parameter:      
	    Return:         void
	    Remark:         if this function is not called before session started, Mcpsession will use default values
	*/
	
	void setMcpAttr(const wchar_t* wsMCastAddr,DWORD dwEventInterval,DWORD ulMaxWaitSecs);
	/*
		Function:		setXferAttr
		Description:	Set transfer attribute for MCP session
		Parameter:		MCP_XFER_TYPE		xferType			set transfer type
						DWORD				dwBitRate			set transfer bit rate
						unsigned long		ulRteStreamVersion  set real time encode stream version
						bool				zeroLenFail			disable zeroLenFail
						DWORD				dwXferTimeOut		subscriber live transfer timeout in seconds (default: not set)
		Return:			void
		Remark:			if this function isn't called before the mcp session start, mcp session will use default setting.
	*/
	void setXferAttr(MCP_XFER_TYPE xferType,
						DWORD dwBitRate,
						unsigned long ulRteStreamVersion,
						bool zeroLenFail,
						DWORD dwXferTimeOut);
	
	/*
	Function:		setVODpkgAttr
	Description:	set the attributes of VOD pkg
	Parameter:		bool	bVodpkg					disable client-side vodpkg
					bool	bVodpkgDeleteOnFail		disable vodpkg subscriber delete on fail
					bool    bTxSpeedFiles			transfer only .mpg and .vvx files (default: disabled)
					bool	bVvtTransfer			enable VVT transfer
					bool	bEnableServerSideVodpkgDelete	disable server-side vodpkg delete	
	Remark:
	*/
	void setVODpkgAttr(bool bVodpkg,
					   bool bVodpkgDeleteOnFail,
					   bool bTxSpeedFiles,
					   bool bVvtTransfer,
					   bool bEnableServerSideVodpkgDelete);

	/*
	Function:		terminate
	Description:	Terminate the Mcp session during the process of MCP
	Parameter:		void
	Return:			void
	Remark:			this operation is called by Session Manager to terminate the process.
					And all the related resource and operator will be release. And pending 
					operation will returned by MCP_STATUS or MCP_CANCEL
	*/
	void terminate(void);
	/*
	    Function:       run
	    Description:    this function is main process of MCP.It will be blocked only if the endSession function is called.
	    Parameter:      void
	    Return:         error return 1.otherwise return 0
	    Remark:         this function is called by work thread 
	*/
	
	int run(void);
	
	//Function:			getLatestErrMsg
	//Description:		get latest Error Message for the Session
	//Parameter:		void
	//Return:			std::string		error message 
	//Remark:			
	const char* getLatestErrMsg(void){return _swErrorMessage;};

	//Function:			setErrMsg
	//Description:		set Error Message of the Session
	//Parameter:		std::string		sErrMsg			error message
	//Return:			void
	//Remark:			
	void setErrMsg(const char* sErrMsg){strcpy(_swErrorMessage, sErrMsg);};

	McpSession();
	virtual ~McpSession();
//public here so that mcpObj can set values in its construct function defaultly
	MCPMEDIAINFO m_mediaInfo;
	MCPSETTING m_McpSetting;

	MCPSESSIONSTATUS getCurStatus(void){return m_dwStatus;}

	// used to output log info
	const wchar_t* getLogHeader()
	{
		return _sLogHeader;
	}

	void setLogHeader(const wchar_t* swLog)
	{
		wcscpy(_sLogHeader, swLog);
	}
	

private :	
	char		_swErrorMessage[1024];

	McpPublisher* m_pPublisher;//publisher pointer

	std::vector<McpSubscriber*> m_SubVector;	//subscriber vector 
	unsigned int m_uiObject;//closed or failed object number
	MCPSESSIONSTATUS m_dwStatus;

protected:
	
	wchar_t		_sLogHeader[256];	
	bool		_bKeepPolling;	// main loop keep flag in run	
};

#endif // !defined(__ZQ_MCPSESSION_H__)
