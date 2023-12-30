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
// Ident : $Id: McpPublisher.cpp,v 1.25 2004/08/09 07:57:34 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : implement class McpPublisher to encapsulate the Publisher object in 
//		   MCP processing
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/MCPSession/McpPublisher.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     05-04-25 15:41 Hongye.gu
// 
// 13    04-12-14 15:00 Jie.zhang
// 
// 12    04-11-26 18:32 Jie.zhang
// 
// 11    04-11-22 19:47 Jie.zhang
// arranged source & added detail log message.
// 
// 10    04-11-22 14:27 Jie.zhang
// 
// 9     04-11-09 14:02 Kaliven.lee
// cast handle to dword for print
// 
// 7     04-11-04 18:44 Kaliven.lee
// 
// 6     04-11-04 9:53 Kaliven.lee
// after unit test and modified
// 
// 5     04-10-28 16:38 Kaliven.lee
// 
// 4     04-08-26 8:45 Kaliven.lee
// add exception catch
// Revision 1.25  2004/08/09 07:57:34  wli
// unicode modified
//
// Revision 1.24  2004/07/19 02:46:38  wli
// unregister handle when on close
//
// Revision 1.23  2004/07/16 08:43:28  wli
// wsprintf    %s    2    %S
//
// Revision 1.21  2004/07/15 09:02:03  wli
// first stab version.
//
// Revision 1.20  2004/07/15 03:23:03  wli
// fix the mcp process error. and set all attributes in bind function
//
// Revision 1.18  2004/07/14 09:18:06  jshen
// no message
//
// Revision 1.17  2004/07/14 03:02:43  jshen
// fix some bug
//
// Revision 1.16  2004/07/13 04:10:35  jshen
// for mca session test
//
// Revision 1.15  2004/07/12 06:44:47  jshen
// because of the IP change of the CVS server
//
// Revision 1.14  2004/07/08 03:53:12  wli
// replace some parameters from string to wstring according MCPAPI
//
// Revision 1.13  2004/07/07 06:32:38  wli
// modified open to set mcast addr
//
// Revision 1.12  2004/07/05 08:52:04  wli
// clean up some unused member and set mcp setting as member
//
// Revision 1.11  2004/07/02 05:46:43  wli
// no message
//
// Revision 1.10  2004/07/01 07:46:19  wli
// change the order of include file to correct reinclude windows.h
//
// Revision 1.9  2004/07/01 06:27:00  jshen
// after merge
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
// McpPublisher.cpp: implementation of the McpPublisher class.
//
//////////////////////////////////////////////////////////////////////
#include "log.h"
#include "tchar.h"
#pragma warning(disable:4786)
#include "McpPublisher.h"
#include "McpSession.h"
#include "../TThread.h"
#include "McpEventsSink.h"

#include <string>

using namespace ZQ::common;

#define  _ASYNC_CALL_


#ifdef _ASYNC_CALL_
const int ASYNC_CALL_RPC_TIMEOUT = 10000;
extern "C" MCP_IOSTATUS* getMcpIoStatus();
extern "C" void mcpReqCBK(MCP_HANDLE hHandle, LPMCP_IOSTATUS pIO);
#endif

#pragma warning(disable: 4786)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//extern ZQ::common::ScLog * gpScLog;
extern McpEventsSink g_McpSink;

McpPublisher::McpPublisher(McpSession* parent ,std::wstring addr,DWORD port,std::wstring ifaceAddr)
			:McpObject(parent,addr,port,ifaceAddr)
{
	m_handle = MCP_EX_INVALID_HANDLE;
}

McpPublisher::~McpPublisher()
{
	if(m_handle != MCP_INVALID_HANDLE)
	{
		close();
	}

//	OutputDebugString(L"~McpPublisher()\n");
}


void McpPublisher::setMediaInfo(MCPMEDIAINFO& mediainfo)
{
	m_MediaInfo = mediainfo;	
}


std::wstring McpPublisher::getPubGSID(void)
{
	return m_GSID;
}

void McpPublisher::setMcpSetting(MCPSETTING& mcpSetting)
{
	m_McpSetting = mcpSetting;
}

#include "../stdafx.h"
MCP_STATUS McpPublisher::bind(DWORD destinePort)
{
	MCP_STATUS status = MCP_EX_NONE;

	MCP_ATTRIBUTE_LIST attList;
	memset(&attList,0,sizeof(MCP_ATTRIBUTE_LIST));
	DWORD attCount = 0;

	LPMCP_IOSTATUS pStatBlk = 0;
	PMCP_EVENT pEventBuf = 0;

	try{
		
		// set attributes
		// So before you bind publisher u must set media info,mcpsetting at first
		//MCPAddAttribute(&attList[attCount++], MCP_ATT_FILENAME, (LPVOID)m_MediaInfo.fileName.data(),
		//				MAX_FILE_NAME_LENGTH * sizeof(wchar_t));
		if(!m_MediaInfo.fileName.empty())
		{	

			MCPAddAttribute(&attList[attCount++],
							MCP_ATT_FILENAME,
							(LPVOID)m_MediaInfo.fileName.c_str(),
							(m_MediaInfo.fileName.length() + 1) * sizeof(wchar_t));
		}
		else
		{			
			glog(Log::L_ERROR, _T("%s: publish file name is empty."), getLogHeader());
			return MCP_EX_INVALID_FILE;
		}

		MCPAddAttribute(&attList[attCount++], MCP_ATT_MCAST_ADDR, (LPVOID)m_McpSetting.sMcastAddr.data(),
				80 * sizeof(wchar_t));
	
		if(!m_sIfaceAddr.empty())
		{
			MCPAddAttribute(&attList[attCount++], MCP_ATT_IFACE_ADDR, (LPVOID)m_sIfaceAddr.data(),
				80 * sizeof(wchar_t));
		}

		MCPAddAttribute(&attList[attCount++], MCP_ATT_SOURCE_PORT, (LPVOID)&m_dwPort,
			sizeof(m_dwPort));

		MCPAddAttribute(&attList[attCount++], MCP_ATT_DEST_PORT, (LPVOID)&destinePort,
			sizeof(destinePort));

		if(m_McpSetting.dwXferTimeOut != 0)
		{
			DWORD msTimeout = m_McpSetting.dwXferTimeOut * 1000;
			MCPAddAttribute(&attList[attCount++], MCP_ATT_LIVE_XFER_TIMEOUT, (LPVOID)&msTimeout,
				sizeof(DWORD));
		}

		MCPAddAttribute(&attList[attCount++], MCP_ATT_MAX_BITRATE, (LPVOID)&m_McpSetting.dwMaxBitRate,
			sizeof(DWORD));

		MCPAddAttribute(&attList[attCount++], MCP_ATT_XFER_TYPE, (LPVOID)&m_McpSetting.XferType,
			sizeof(m_McpSetting.XferType));

		MCPAddAttribute(&attList[attCount++], MCP_ATT_RTE_STREAM_VERSION, (LPVOID)&m_McpSetting.RTEStreamVer,
			sizeof(MCP_RTE_STREAM_VERSION));

		MCPAddAttribute(&attList[attCount++], MCP_ATT_RTE_STREAM_ECO, (LPVOID)&m_McpSetting.RTEStreamEco,
			sizeof(MCP_RTE_STREAM_ECO));

		MCPAddAttribute(&attList[attCount++], MCP_ATT_RTE_ZEROLEN_FAIL, (LPVOID)&m_McpSetting.dwServerSideDeleteOnFail,
			sizeof(DWORD));

		if(m_McpSetting.dwDisableBufDrvThrottle)
		{
			MCPAddAttribute(&attList[attCount++], MCP_ATT_GUARANTEED_WRITE,
				(LPVOID)&m_McpSetting.dwDisableBufDrvThrottle, sizeof(DWORD));
		}
		

		status = MCPSetAttribute(m_handle, 
								m_McpSetting.dwOptionMask, 
								attCount,
								attList, NULL);

		if (status != MCP_SUCCESS)
		{
			// MCP_PARTIAL is fatal here, but the extended error is MCP_EX_NONE...
			// fish out the failure code from the failed attribute and return it
			if(MCP_PARTIAL == status)
			{
				DWORD i;
				for(i = 0; i < attCount; i++)
				{
					if(MCP_FAILURE == attList[i].status)
					{	
						glog(Log::L_ERROR, L"%s: Publisher set attribute %s failed.", getLogHeader(), MCPAttrAsStr( &attList[i]));

						return attList[i].exStatus;					
					}
				}

				if(i == attCount)
				{
					// MCP_PARTIAL return with no attributes set to MCP_FAILURE :(	
					glog(Log::L_ERROR, L"%s: bind(MCPSetAttribute), MCP externed internal error!", getLogHeader());
					
					return MCP_EX_INTERNAL_ERROR;
				}
			}
			else
			{	
				glog(Log::L_ERROR, L"%s: Publisher set attribute externed failed %s", getLogHeader(), MCPStatusAsStr(MCPGetLastExtendedError()));

				return MCPGetLastExtendedError();
			}
		
		}
		// get attributes
		attCount = 0;
		wchar_t pubGSID[MCP_GSID_SIZE];
		memset(attList, 0, sizeof(attList));
		memset(pubGSID, 0, MCP_GSID_SIZE * sizeof(wchar_t));

		MCPAddAttribute(&attList[attCount++], MCP_ATT_PUB_GSI, (LPVOID)pubGSID,
			MCP_GSID_SIZE * sizeof(wchar_t));


		MCPAddAttribute(&attList[attCount++], MCP_ATT_OBJ_SIZE, (LPVOID)&m_MediaInfo.fileSize,
			sizeof(m_MediaInfo.fileSize));

//		ATLASSERT(FALSE);

		wchar_t Iface[256] ;
		memset(Iface,0,sizeof(Iface));
		MCPAddAttribute(&attList[attCount++], MCP_ATT_IFACE_ADDR, (LPVOID)Iface,
			sizeof(Iface));


		status = MCPGetAttribute(m_handle, 
								m_McpSetting.dwOptionMask, 
								attCount, 
								attList,
								NULL);

		if(status != MCP_SUCCESS)
		{
			// MCP_PARTIAL is fatal for us here.
			// close publisher here
			glog(Log::L_ERROR, L"%s: Publisher get attribute  failed :%s", getLogHeader(), MCPStatusAsStr(status));

			return MCPGetLastExtendedError();		
		}	
		
		//set publisher's GSID
		m_GSID = pubGSID;

		// request event notification
		status = MCPRequestEventNotification(
						m_handle, 
						MCP_OPTM_ATTR_ALL | m_McpSetting.dwOptionMask,
						MCP_EVT_ATTR_CHG,
						0,
						NULL,
						m_McpSetting.dwEventInterval,
						0,
						NULL);
		if(status != MCP_SUCCESS)
		{
			return MCPGetLastExtendedError();		
		}

		// begin reading events

		// allocate an async callback data block, it'll be freed
		// in the callback
		pStatBlk = (LPMCP_IOSTATUS)calloc(1, sizeof(MCP_IOSTATUS));
		if(!pStatBlk)
		{
			return MCP_EX_NOMEMORY;	
		}

		// here's the callback address
		pStatBlk->completionRoutine = McpEventsSink::dispatchEvents;

		// allocate an event buffer, it'll be freed in the callback
		pEventBuf = (PMCP_EVENT)calloc(1, sizeof(MCP_EVENT));

		// stash the eventBuf in the IOSTATUS userData field
		pStatBlk->userData = (LPVOID)pEventBuf;


		// async readEvent
		//according to the description of McpApi the return of read event can not be MCP_PENDING
		status = MCPReadEvent(m_handle, m_McpSetting.dwOptionMask, pEventBuf, pStatBlk );

		if(status != MCP_SUCCESS && status != MCP_PENDING)
		{
			if(pStatBlk )
				free (pStatBlk);

			if(pEventBuf)
				free (pEventBuf);

			glog(Log::L_ERROR, _T("%s: MCPReadEvent failed."), getLogHeader());

			return MCPGetLastExtendedError();
		}
	}
	catch(...)
	{
		//added by salien
//		OutputDebugString(L"In McpPublish's bind \n");

		if(pStatBlk )
		{
			free (pStatBlk);
			pStatBlk = NULL;
		}

		if(pEventBuf)
		{
			free (pEventBuf);
			pEventBuf = NULL;
		}
	}

	_dwLastCheckTime = GetTickCount();

	return MCP_EX_NONE;
}

MCP_STATUS McpPublisher::publish(void)
{
	MCP_STATUS status = MCPPublish(m_handle,m_McpSetting.dwOptionMask,NULL);	

	if (status == MCP_SUCCESS)
	{
		setCurStatus(MCP_PUBLISHING);
		
		return MCP_EX_NONE;
	}

	setCurStatus(MCP_FAILED);
		
	return MCPGetLastExtendedError();
}

MCP_STATUS McpPublisher::close(void)
{
	if (m_handle == MCP_INVALID_HANDLE)
	{
		return MCP_EX_NONE;
	}
	
	glog(Log::L_DEBUG, L"%s: Publisher start closing", getLogHeader());

	setClosed(true); // because it will cause eventcallback, so set the flag now

	if(g_McpSink.find(m_handle) != NULL)
		g_McpSink.unreg(m_handle,this);
	
#ifdef _ASYNC_CALL_
	MCP_IOSTATUS*  pIO = getMcpIoStatus();

//	memset(pIO, 0, sizeof(io));
	
	pIO->completionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	pIO->completionRoutine = mcpReqCBK;

	MCP_STATUS status = MCPClose(m_handle, m_McpSetting.dwOptionMask, pIO);	
	if (status == MCP_PENDING)
	{
		DWORD dwRet = WaitForSingleObject(pIO->completionEvent, ASYNC_CALL_RPC_TIMEOUT);
		if (dwRet == WAIT_TIMEOUT)
		{
			glog(Log::L_ERROR, L"%s: Publisher close timeout.", getLogHeader());
		}
	}
	
	//CloseHandle(pIO->completionEvent);
#else
	MCP_STATUS status = MCPClose(m_handle,m_McpSetting.dwOptionMask,NULL);
		
#endif

	m_handle = MCP_INVALID_HANDLE;

	glog(Log::L_DEBUG, L"%s: Publisher closed", getLogHeader());

	return MCP_EX_NONE;	
}

MCP_STATUS McpPublisher::open(void)
{
	//this comment is copied from "multidest" project from SChange Maynard
	// array of IP addresses we're connecting to ... in this case
	// it'll only be "localhost"

	//here is the comment from MCPAPI
	/*	nodeNames
		[in] This parameter is a NULL-terminated array of pointers to strings
		containing the names of nodes to open.  All names specified should refer
		to redundant links on the same machine.  These names may be specified
		as either host names or IP addresses in "dotted-quad" notation.
		A maximum of six addresses are supported, subsequent addresses are
		ignored.
	*/

	//I guess I perhaps is the location for source media or several publishers
	//here some modified to convert the string from MBCS to UNICODE 
	LPCWSTR nodeNames[] = {0, 0};
	wchar_t buf[256];
	//memcpy(buf,m_McpSetting.sMcastAddr.data(),m_McpSetting.sMcastAddr.length());
	wmemcpy(buf, m_sAddr.data(),m_sAddr.length());
	buf[m_sAddr.length()] = L'\0';
	nodeNames[0] =buf;	

	// open handle
	MCP_STATUS status = MCPOpen(&m_handle, m_McpSetting.dwOptionMask, MCP_OPEN_TYPE_PUB, nodeNames , NULL);

	if( status != MCP_SUCCESS)
	{
		return MCPGetLastExtendedError();
	}

	g_McpSink.reg(m_handle, this);
	
	setClosed(false);

	return MCP_EX_NONE;
}

MCP_STATUS McpPublisher::setPrivateData(void)
{
	MCA_BRIEF_PRIVATE_DATA tMcaBrief;
	DWORD briefDatLen = 0;
	if(m_MediaInfo.ulContentBitrate | m_MediaInfo.uiContentHorizRes | m_MediaInfo.uiContentVerticalRes)
	{
		tMcaBrief.magic = MCA_BRIEF_PRIVATE_DATA_MAGIC;
		tMcaBrief.version = MCA_BRIEF_PRIVATE_DATA_VERSION;

		tMcaBrief.bitrateBPS = m_MediaInfo.ulContentBitrate;
		tMcaBrief.usHRes = m_MediaInfo.uiContentHorizRes;
		tMcaBrief.usVRes = m_MediaInfo.uiContentVerticalRes;
		briefDatLen= MAX_USER_BRIEF_PRIVATE_DATA_LENGTH -1;		
		memcpy(tMcaBrief.rawData,m_MediaInfo.briefPrvString.data(),m_MediaInfo.briefPrvString.length());
	}
	
	
	MCP_ATTRIBUTE_LIST attList = {0};
	int i = 0;
	memset((void*)&attList,0,sizeof(MCP_ATTRIBUTE_LIST));

	DWORD dwFullData = (DWORD)m_MediaInfo.fullPrvString.length();
	if(dwFullData)
	{
		
		MCPAddAttribute(&attList[i++], MCP_ATT_PRIVATE_DATA, (LPVOID)m_MediaInfo.fullPrvString.data(), 
			(USHORT)(m_MediaInfo.fullPrvString.length() * sizeof(wchar_t)));
		MCPAddAttribute(&attList[i++], MCP_ATT_PRIVATE_DATA_LENGTH, (LPVOID)&dwFullData, 
			sizeof(DWORD));
	}
	if(briefDatLen)
	{
		MCPAddAttribute(&attList[i++], MCP_ATT_BRIEF_PRIVATE_DATA, (LPVOID)&tMcaBrief.rawData, MCP_BRIEF_PRIVATE_DATA_LENGTH);
	
		MCPAddAttribute(&attList[i++], MCP_ATT_BRIEF_PRIVATE_DATA_LENGTH, &briefDatLen, sizeof(DWORD));
	}
	if(i > 0)
	{
		
		MCP_STATUS status = MCPSetAttribute(m_handle, m_McpSetting.dwOptionMask, i, attList, NULL);
		if(MCP_SUCCESS != status)
		{
			return MCPGetLastExtendedError();
		}
	}
	return MCP_EX_NONE;
}

MCP_STATUS McpPublisher::getPrivateData(PMCA_BRIEF_PRIVATE_DATA pbrifeData,std::string fullData)
{
	MCP_ATTRIBUTE_LIST attList ;

	int i = 0;
	memset((void*)&attList,0,sizeof(MCP_ATTRIBUTE_LIST));
	char data[2000];

	MCPAddAttribute(&attList[i++], MCP_ATT_PRIVATE_DATA, (LPVOID)data, MCP_FULL_PRIVATE_DATA_LENGTH);
	MCPAddAttribute(&attList[i++], MCP_ATT_BRIEF_PRIVATE_DATA, (LPVOID)pbrifeData->rawData, MCP_BRIEF_PRIVATE_DATA_LENGTH);		
	MCP_STATUS status = MCPGetAttribute(m_handle, m_McpSetting.dwOptionMask, i, attList, NULL);

	fullData = data;
	if(MCP_SUCCESS != status)
	{
		glog(Log::L_ERROR, L"%s: failed to get private data", getLogHeader());
		
		return MCPGetLastExtendedError();
	}

	return MCP_EX_NONE;
}


void McpPublisher::OnClose(MCP_CUR_STATE tState)
{	
	glog(Log::L_DEBUG, L"%s: McpPublisher::OnClose", getLogHeader());	
	setCurStatus(tState);
}


bool McpPublisher::OnEvent(LPMCP_IOSTATUS pStat)
{
	//this function is reserved to retrieve the attribute for the events
	//and is fired by dispatchEvents function.
	//and it is reserved for further develop
	PMCP_EVENT pEvent = (PMCP_EVENT)pStat->userData;
	PMCP_ATTRIBUTE pAtt =(PMCP_ATTRIBUTE)pEvent->pEventData;
	
	ULONGLONG *pUlonglong = NULL;
	MCP_CUR_STATE CurState;

	_dwLastCheckTime = GetTickCount();

	// here's how to pick out the data from the attribute
	switch(pAtt->attCode)
	{
	case MCP_ATT_BYTES_SENT:		// total bytes sent (ULONGLONG)
//		pUlonglong = (ULONGLONG *)pAtt->pAttData;
//		wsprintf(buf,L"HANDLE(%d):Publisher had sent %d byte.",(DWORD)m_handle,*pUlonglong);
//		gpScLog->writeMessage(buf,ZQ::common::Log::L_DEBUG);
		break;
	case MCP_ATT_BYTES_RECD:		// total bytes received (ULONGLONG)
//		pUlonglong = (ULONGLONG *)pAtt->pAttData;
//		wsprintf(buf,L"HANDLE(%d):Publisher had received %d Bytes.",(DWORD)m_handle,*pUlonglong);
//		gpScLog->writeMessage(buf,ZQ::common::Log::L_DEBUG);
		break;
	case MCP_ATT_OBJ_SIZE:		// total object size (ULONGLONG)

//		pUlonglong = (ULONGLONG *)pAtt->pAttData;

		//wprintf(L"ULONGLONG data: %I64d\n", *pUlonglong);

		break;

	case MCP_ATT_CUR_STATE:		// current server state (ENUM32)
		
		CurState = *((PMCP_CUR_STATE)pAtt->pAttData);

		if (CurState == MCP_PUBLISHING ||
			CurState == MCP_FAILED ||
			CurState == MCP_STOPPED ||
			CurState == MCP_DONE)
		{
			if (CurState != MCP_PUBLISHING)
			{
				glog(Log::L_DEBUG, _T("%s: status(%s), setCurStatus(%d) . MCPGetLastExtendedError()=%s. ExtendStaus(%d)"), getLogHeader(), MCPStatusAsStr(pStat->status),CurState,  MCPStatusAsStr(MCPGetLastExtendedError()), pStat->extendedStatus);
				
				setCurStatus(CurState);

				return false;
			}
		}
		else
		{
			glog(Log::L_DEBUG, L"%s: OnEvent, CurStatus(%d).", getLogHeader(), CurState);
		}
//		wsprintf(buf,L"HANDLE(%d):Publisher  current status is %s.",((DWORD)m_handle),MCPStateAsStr(*pCurState));
//		gpScLog->writeMessage(buf,ZQ::common::Log::L_DEBUG);
		//wprintf(L"MCP_CUR_STATE data: %s\n", MCPStateAsStr(*pCurState));

		break;

	default:
		break;
	}
	
	return true;
}

/*
void McpPublisher::OnFatalFailed(std::wstring tWhy)
{	
	McpObject::setStatus(MCP_FAILED) ;
	McpObject::setCurStatus(MCP_FAILED);
	//log 
	// Fatal failed	
	
	m_parentSession->setErrMsg(tWhy.c_str());
	if(m_handle!= MCP_INVALID_HANDLE)
	{
		glog(Log::L_ERROR, L"%s: Publisher On Fatal Failed, start closing", this);
		
		close();
	}
}
*/

/*
MCP_CUR_STATE McpPublisher::getCurStatus()
{
	if (m_handle == MCP_INVALID_HANDLE)
	{
		return MCP_STOPPED;
	}
	
	return McpObject::getCurStatus();	

	if (McpObject::getCurStatus() != MCP_PUBLISHING)
	{
		return McpObject::getCurStatus();
	}
	
	MCP_ATTRIBUTE_LIST attList;
	memset(attList, 0, sizeof(attList));
	DWORD attCount = 0;
	MCP_CUR_STATE CurState = MCP_EMPTY;
	MCPAddAttribute(&attList[attCount++], MCP_ATT_CUR_STATE, (LPVOID)&CurState,sizeof(MCP_CUR_STATE));
	
	MCP_STATUS status = MCPGetAttribute(m_handle,m_McpSetting.dwOptionMask,attCount,attList,NULL);

	if(status != MCP_SUCCESS)
	{		
		glog(Log::L_ERROR, L"%s: MCPGetAttribute, Can not read Current state.", this);
		
		return MCP_FAILED;		
	}

	McpObject::setCurStatus(CurState);

	return CurState;
}*/

BOOL McpPublisher::checkAliving()
{
	DWORD dwNow = GetTickCount();

	if(dwNow - _dwLastCheckTime < CHECK_ALIVE_SPAN)
		return TRUE;

	MCP_ATTRIBUTE_LIST attList;
	memset(attList, 0, sizeof(attList));
	DWORD attCount = 0;
	MCP_CUR_STATE CurState = MCP_EMPTY;
	MCPAddAttribute(&attList[attCount++], MCP_ATT_CUR_STATE, (LPVOID)&CurState,sizeof(MCP_CUR_STATE));
	
	MCP_STATUS status = MCPGetAttribute(m_handle,m_McpSetting.dwOptionMask,attCount,attList,NULL);

	if(status != MCP_SUCCESS)
	{		
		glog(Log::L_ERROR, L"%s: MCPGetAttribute, Can not read Current state.", getLogHeader());
		
		setCurStatus(MCP_FAILED);

		return FALSE;
	}
	setCurStatus(CurState);

	glog(Log::L_DEBUG, L"%s: checkAliving ok, MCP_CUR_STATE=[%d].", getLogHeader(), CurState);

	_dwLastCheckTime = dwNow;

	return TRUE;
}

void McpPublisher::setCurStatus(MCP_CUR_STATE tCurStatus)
{
	_dwLastCheckTime = GetTickCount();

	McpObject::setCurStatus(tCurStatus);
}
