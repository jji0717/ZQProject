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
// Ident : $Id: McpSubscriber.cpp,v 1.22 2004/08/09 07:57:34 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : implement class McpSubscriber to encapsulate MCP subscriber
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/MCPSession/McpSubscriber.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     05-04-25 15:41 Hongye.gu
// 
// 14    04-12-14 15:00 Jie.zhang
// 
// 13    04-11-26 18:32 Jie.zhang
// 
// 12    04-11-22 19:47 Jie.zhang
// arranged source & added detail log message.
// 
// 11    04-11-22 14:27 Jie.zhang
// 
// 10    04-11-15 11:20 Kaliven.lee
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
// Revision 1.22  2004/08/09 07:57:34  wli
// unicode modified
//
// Revision 1.21  2004/08/03 07:16:35  wli
// no message
//
// Revision 1.20  2004/07/19 02:46:33  wli
// unregister handle when on close
//
// Revision 1.19  2004/07/16 08:43:55  wli
// wsprintf    %s    2    %S
//
// Revision 1.18  2004/07/16 03:09:19  wli
// correct the close function to unregister the handle
//
// Revision 1.17  2004/07/15 09:02:58  wli
// first stab version
//
// Revision 1.15  2004/07/15 03:23:08  wli
// fix the mcp process error. and set all attributes in bind function
//
// Revision 1.13  2004/07/14 09:18:06  jshen
// no message
//
// Revision 1.12  2004/07/14 03:02:43  jshen
// fix some bug
//
// Revision 1.11  2004/07/08 03:53:12  wli
// replace some parameters from string to wstring according MCPAPI
//
// Revision 1.10  2004/07/07 06:33:55  wli
// modified open to set mcast addr
//
// Revision 1.9  2004/07/05 08:52:11  wli
// clean up some unused member and set mcp setting as member
//
// Revision 1.8  2004/07/01 06:28:19  jshen
// after merge
//
// Revision 1.7  2004/06/30 06:47:29  wli
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
// McpSubscriber.cpp: implementation of the McpSubscriber class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "log.h"
#include "tchar.h"
#include "McpSubscriber.h"
#include "McpSession.h"
#include "McpEventsSink.h"
#include "../mcpsdk/McpApi.h"
#include "../mcpsdk/MCPMsgs.h"
#include "../mcpsdk/MCPUtility.h"
#define  _ASYNC_CALL_


#ifdef _ASYNC_CALL_

#include "../common/locks.h"

using namespace ZQ::common;

const int ASYNC_CALL_RPC_TIMEOUT = 10000;

const int MAX_IOSTATUS_NUM = 10;

int				g_nUsePos = 0;
MCP_IOSTATUS	io[MAX_IOSTATUS_NUM];

Mutex g_mcpIoStatus;
extern "C" MCP_IOSTATUS* getMcpIoStatus()
{
	Guard<Mutex> tt(g_mcpIoStatus);

	if (g_nUsePos>=MAX_IOSTATUS_NUM)
		g_nUsePos = 0;

	return &io[g_nUsePos++];
}

extern "C" void mcpReqCBK(MCP_HANDLE hHandle, LPMCP_IOSTATUS pIO)
{
//	int sss = 0;
}

#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern McpEventsSink g_McpSink;
using namespace ZQ::common;


McpSubscriber::McpSubscriber(McpSession* parent,std::wstring Addr,DWORD Port,std::wstring ifaceAddr)
:McpObject(parent,Addr,Port,ifaceAddr)
{
	
	m_sAddr = Addr ;
	m_dwPort = Port;
	m_sIfaceAddr = ifaceAddr;
	m_type = OBJ_TYPE_SUB;
	
	m_handle = MCP_INVALID_HANDLE;
}

McpSubscriber::~McpSubscriber()
{
	close();
//	OutputDebugString(L"~McpSubscriber()\n");
}

/*
void McpSubscriber::check(void)
{
	if(getCurStatus() == MCP_SUBSCRIBING)
	{
	//	glog(ZQ::common::Log::L_DEBUG,L"HANDLE(%d):start to check Subscriber",(DWORD)m_handle);
//		FILETIME CurTime;
//		GetSystemTimeAsFileTime(&CurTime);

//		DWORD timeinterval = ((CurTime.dwHighDateTime-m_ElapseTime.dwHighDateTime) << 32)/10000 
//			+ (CurTime.dwLowDateTime - m_ElapseTime.dwLowDateTime)/10000;
			
//		if(timeinterval > MCPSESSION_FAILED_CHK_TIMEOUT)
//		{
			MCP_ATTRIBUTE_LIST attList;
			memset(attList, 0, sizeof(attList));
			DWORD attCount = 0;
			MCP_CUR_STATE CurState = MCP_EMPTY;
			MCPAddAttribute(&attList[attCount++], MCP_ATT_CUR_STATE, (LPVOID)&CurState,sizeof(MCP_CUR_STATE));
			
			MCP_STATUS status = MCPGetAttribute(m_handle,m_McpSetting.dwOptionMask,attCount,attList,NULL);

			if(status != MCP_SUCCESS)
			{
				setCurStatus(MCP_FAILED);
				glog(Log::L_ERROR, L"MCPSubscriber(%08X): MCPGetAttribute, Can not read Current state. closing subscriber.", this);

				close();
			}	
			
//			GetSystemTimeAsFileTime(&CurTime);
//			m_ElapseTime = CurTime;
//		}		
	}
}
*/

void McpSubscriber::setMediaInfo(MCPMEDIAINFO& mediaInfo)
{
	m_MediaInfo = mediaInfo;	
}

void McpSubscriber::setMcpSetting(MCPSETTING& mcpSetting)
{
	m_McpSetting = mcpSetting;
}

#include "../stdafx.h"
MCP_STATUS McpSubscriber::subscribe(void)
{
#ifdef _ASYNC_CALL_
	MCP_IOSTATUS*  pIO = getMcpIoStatus();

//	memset(pIO, 0, sizeof(io));
	
	pIO->completionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	pIO->completionRoutine = mcpReqCBK;


	MCP_STATUS status = MCPSubscribe(m_handle,m_McpSetting.dwOptionMask, pIO);
	
	if (status == MCP_PENDING)
	{
		DWORD dwRet = WaitForSingleObject(pIO->completionEvent, ASYNC_CALL_RPC_TIMEOUT);
		CloseHandle(pIO->completionEvent);

		if (dwRet == WAIT_TIMEOUT)
		{
			setCurStatus(MCP_FAILED);

			glog(Log::L_ERROR, L"%s: call subscribe() timeout.", getLogHeader());
			return MCP_EX_SUBSCRIBE_FAILED;
		}

		if (pIO->status == MCP_SUCCESS)
		{
			setCurStatus(MCP_SUBSCRIBING);			
			return MCP_EX_NONE;
		}
		else
		{
			setCurStatus(MCP_FAILED);
			return pIO->extendedStatus;			
		}
	}
	else
	{
		CloseHandle(pIO->completionEvent);
	}
	
#else
	//ATLASSERT(FALSE);
	MCP_STATUS status = MCPSubscribe(m_handle,m_McpSetting.dwOptionMask,NULL);
	
	int nError=MCPGetLastExtendedError();

	if (status == MCP_SUCCESS)
	{
		setCurStatus(MCP_SUBSCRIBING);
		
		return MCP_EX_NONE;
	}

	setCurStatus(MCP_FAILED);
#endif

	return MCPGetLastExtendedError();
}

MCP_STATUS McpSubscriber::close(void)
{
	if (m_handle == MCP_INVALID_HANDLE)
	{
		return MCP_EX_NONE;
	}
	
	glog(Log::L_DEBUG, L"%s: Subscriber start closing", getLogHeader());

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
			glog(Log::L_ERROR, L"%s: Subscriber close timeout.", getLogHeader());
		}
	}
	
	CloseHandle(pIO->completionEvent);

#else
	MCP_STATUS status = MCPClose(m_handle, m_McpSetting.dwOptionMask, NULL);		
#endif

	m_handle = MCP_INVALID_HANDLE;

	glog(Log::L_DEBUG, L"%s: Subscriber closed", getLogHeader());

	return MCP_EX_NONE;	
}


MCP_STATUS McpSubscriber::open(void)
{
	// array of IP addresses we're connecting to ... in this case
	// it'll only be "localhost"

	// the address must be NULL-terminate string 
	LPCWSTR nodeNames[] = {0, 0};
	wchar_t buf[256];
	wmemcpy(buf, m_sAddr.data(), m_sAddr.length());
//	mbstowcs(buf,m_McpSetting.sMcastAddr.data(),m_McpSetting.sMcastAddr.length());
//	buf[m_sAddr.length()] = '\0';
	buf[m_sAddr.length()] = L'\0';
	nodeNames[0] = buf;


	MCP_STATUS status = MCPOpen(&m_handle, m_McpSetting.dwOptionMask , MCP_OPEN_TYPE_SUB, nodeNames , NULL);
	if(MCP_SUCCESS == status)
	{	
		g_McpSink.reg(m_handle, this);

		setClosed(false);

		return MCP_EX_NONE;
	}	


	return MCPGetLastExtendedError();
}
/*
    Function:       OnEvent
    Description:    retrieve event attributes
    Parameter:      LPMCP_IOSTATUS		pStat		a pointer to MCP IO status
    Return:         void
    Remark:         reserved for further develop
*/


bool McpSubscriber::OnEvent(LPMCP_IOSTATUS pStat)
{
	//this function is reserved to retrieve the attribute for the events
	//and is fired by dispatchEvents function.
	//and it is reserved for further develop
	PMCP_EVENT pEvent = (PMCP_EVENT)pStat->userData;
	PMCP_ATTRIBUTE pAtt =(PMCP_ATTRIBUTE)pEvent->pEventData;

	ULONGLONG *pUlonglong = NULL;
	MCP_CUR_STATE CurState;


	_dwLastCheckTime = GetTickCount();

	switch(pAtt->attCode)
	{
	case MCP_ATT_BYTES_SENT:		// total bytes sent (ULONGLONG)
//		pUlonglong = (ULONGLONG *)pAtt->pAttData;
//		wsprintf(buf,L"HANDLE(%d):Subscriber had sent %d Bytes.",(DWORD)m_handle,*pUlonglong);

//		gpScLog->writeMessage(buf,ZQ::common::Log::L_DEBUG);
		break;
	case MCP_ATT_BYTES_RECD:		// total bytes received (ULONGLONG)
//		pUlonglong = (ULONGLONG *)pAtt->pAttData;
//		wsprintf(buf,L"HANDLE(%d):Subscriber had received %d Bytes.",(DWORD)m_handle,*pUlonglong);

//		gpScLog->writeMessage(buf,ZQ::common::Log::L_DEBUG);
		break;
	case MCP_ATT_OBJ_SIZE:		// total object size (ULONGLONG)

//		pUlonglong = (ULONGLONG *)pAtt->pAttData;

		//wprintf(L"ULONGLONG data: %I64d\n", *pUlonglong);

		break;

	case MCP_ATT_CUR_STATE:		// current server state (ENUM32)

		CurState = *((PMCP_CUR_STATE)pAtt->pAttData);

		if (CurState == MCP_SUBSCRIBING ||
			CurState == MCP_FAILED ||
			CurState == MCP_STOPPED ||
			CurState == MCP_DONE)
		{
			if (CurState != MCP_SUBSCRIBING)
			{
				if (CurState == MCP_FAILED)
				{
					MCP_ATTRIBUTE_LIST attList;
					memset(attList, 0, sizeof(attList));
					DWORD attCount = 0;
					MCP_CUR_STATE getCurState = MCP_EMPTY;
					MCPAddAttribute(&attList[attCount++], MCP_ATT_CUR_STATE, (LPVOID)&getCurState,sizeof(MCP_CUR_STATE));
					
					MCP_STATUS status = MCPGetAttribute(m_handle,m_McpSetting.dwOptionMask,attCount,attList,NULL);

					if(status == MCP_SUCCESS)
					{		
						glog(Log::L_DEBUG, _T("%s: call back set mcp_fail(%d), getcurstatus is %d."), getLogHeader(), CurState, getCurState);

						/*if (getCurState == MCP_SUBSCRIBING)
						{
							return true;
						}*/
					}
				}

				setCurStatus(CurState);

				return false;
			}
		}
		else
		{
			glog(Log::L_DEBUG, L"%s: OnEvent, CurStatus(%d).", getLogHeader(), CurState);
		}
		
//		wsprintf(buf,L"HANDLE(%d):Subscriber current status is %s.",((DWORD)m_handle),MCPStateAsStr(getCurStatus()));
//		gpScLog->writeMessage(buf,ZQ::common::Log::L_DEBUG);
		//wprintf(L"MCP_CUR_STATE data: %s\n", MCPStateAsStr(*pCurState));

		break;

	default:
		break;
	}

	return true;
}

MCP_STATUS McpSubscriber::bind(std::wstring pubGSI,DWORD pubPort)
{
	MCP_ATTRIBUTE_LIST attList;
	memset(attList, 0, sizeof(attList));
	DWORD attCount = 0;
	LPMCP_IOSTATUS pStatBlk =0;
	PMCP_EVENT pEventBuf = 0;

	glog(Log::L_DEBUG, L"%s: start binding  subscriber", getLogHeader());

	try{
	
		//add MCP setting attribute
		if(m_MediaInfo.subName != L"")
		{
			MCPAddAttribute(&attList[attCount++],
							MCP_ATT_FILENAME,
							(LPVOID)m_MediaInfo.subName.data(),
							MAX_FILE_NAME_LENGTH * sizeof(wchar_t));
		}
		//file size option is not a must when transfer type is MCP_XFER_LIVE
		
		if(m_McpSetting.XferType != MCP_XFER_LIVE)
		{
			//file size is got by publisher
			if(m_MediaInfo.fileSize)
			{
				MCPAddAttribute(&attList[attCount++],MCP_ATT_OBJ_SIZE,(VOID*)&m_MediaInfo.fileSize,sizeof(DWORD));
			}
			else
			{
				glog(Log::L_ERROR, L"%s: if file type is not MCP_XFER_LIVE, should give the fileSize.", getLogHeader());
			}
		}
		//wchar_t iface[64] =L"192.168.80.236";
		if(!m_sIfaceAddr.empty())
		{
		MCPAddAttribute(&attList[attCount++], MCP_ATT_IFACE_ADDR, (LPVOID)m_sIfaceAddr.data(),
			80 * sizeof(wchar_t));
		}

		MCPAddAttribute(&attList[attCount++],MCP_ATT_MCAST_ADDR,(VOID*)m_McpSetting.sMcastAddr.data(),
				80 * sizeof(wchar_t));	
		
		MCPAddAttribute(&attList[attCount++],MCP_ATT_SOURCE_PORT,(VOID*)&pubPort,sizeof(DWORD));	
		MCPAddAttribute(&attList[attCount++],MCP_ATT_DEST_PORT,(VOID*)&m_dwPort,sizeof(DWORD));	

		if(m_McpSetting.dwXferTimeOut != 0)
		{
			DWORD msTimeout = m_McpSetting.dwXferTimeOut * 1000;
			MCPAddAttribute(&attList[attCount++], MCP_ATT_LIVE_XFER_TIMEOUT, (LPVOID)&msTimeout,
				sizeof(DWORD));
		}
		//add incoming attribute 
		MCPAddAttribute(&attList[attCount++],MCP_ATT_PUB_GSI,(VOID*)pubGSI.data(),
			MCP_GSID_SIZE * sizeof(wchar_t));


		MCPAddAttribute(&attList[attCount++],MCP_ATT_XFER_TYPE,(VOID*)&m_McpSetting.XferType,
			sizeof(m_McpSetting.XferType));	


		MCPAddAttribute(&attList[attCount++],MCP_ATT_RTE_STREAM_VERSION,(VOID*)&m_McpSetting.RTEStreamVer,
			sizeof(m_McpSetting.RTEStreamVer));	
		MCPAddAttribute(&attList[attCount++],MCP_ATT_RTE_STREAM_ECO,(VOID*)&m_McpSetting.RTEStreamEco,
			sizeof(m_McpSetting.RTEStreamEco));	

		
		if(m_McpSetting.dwDisableBufDrvThrottle)
		{
			MCPAddAttribute(&attList[attCount++], MCP_ATT_GUARANTEED_WRITE,
				(LPVOID)&m_McpSetting.dwDisableBufDrvThrottle, sizeof(DWORD));
		}
			
		
		if(m_McpSetting.dwServerSideDeleteOnFail)
		{
			MCPAddAttribute(&attList[attCount++], MCP_ATT_DELETE_ON_FAIL,
				(LPVOID)&m_McpSetting.dwServerSideDeleteOnFail, sizeof(DWORD));
		}		

		MCP_STATUS status = MCPSetAttribute(m_handle,m_McpSetting.dwOptionMask,attCount,attList,NULL);

		if( status != MCP_SUCCESS)
		{
			// MCP_PARTIAL is fatal here, but the extended error is MCP_EX_NONE...
			// fish out the failure code from the failed attribute and return it
			DWORD i;
			for(i = 0; i < attCount; i++)
			{
				if(MCP_FAILURE == attList[i].status)
				{
					glog(Log::L_ERROR, L"%s: Subscriber set attribute %s failed.", getLogHeader(), MCPAttrAsStr( &attList[i]));

					return attList[i].exStatus;	
				}
			}

			if(i == attCount)
			{
				glog(Log::L_ERROR, _T("%s: MCP_PARTIAL return status with no failed element"), getLogHeader());

				return MCP_EX_INTERNAL_ERROR;
			}
		}

		// request event notification
		status = MCPRequestEventNotification(m_handle, MCP_OPTM_ATTR_ALL | m_McpSetting.dwOptionMask,
			MCP_EVT_ATTR_CHG, 0, NULL, m_McpSetting.dwEventInterval, 0, NULL);
		if(status != MCP_SUCCESS)
		{	
			glog(Log::L_ERROR, _T("%s: Subscriber failed to MCPRequestEventNotification"), getLogHeader());	

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
		if(!pEventBuf)
		{
			if(pStatBlk)
			{
				free(pStatBlk);
				pStatBlk =NULL;
			}

			return  MCP_EX_NOMEMORY;

		}

		// stash the eventBuf in the IOSTATUS userData field
		pStatBlk->userData = (LPVOID)pEventBuf;


		// async readEvent
		status = MCPReadEvent(m_handle, m_McpSetting.dwOptionMask, pEventBuf, pStatBlk );
		if(status != MCP_PENDING)
		{
			if(pStatBlk)
			{
				free(pStatBlk);
				pStatBlk =NULL;
			}

			if(pEventBuf)
			{
				free(pEventBuf);
				pEventBuf = NULL;
			}

			glog(Log::L_ERROR, _T("%s: Subscriber  is not ready to read event"), getLogHeader());

			return MCPGetLastExtendedError();		
		}
	}
	catch(...)
	{
		//added by salien
//		OutputDebugString(L"Subscriber catch unexpected exception when binding\n");
		glog(Log::L_ERROR, L"%s: Subscriber catch unexpected exception when binding", getLogHeader());
		
		if(pStatBlk)
		{
			free(pStatBlk);
			pStatBlk = NULL;
		}

		if(pEventBuf)
		{
			free(pEventBuf);
			pEventBuf = NULL;
		}
	}

	_dwLastCheckTime = GetTickCount();
	
	return MCP_EX_NONE;
}



void McpSubscriber::OnClose(MCP_CUR_STATE tState)
{
	glog(Log::L_DEBUG, L"%s: OnClose", getLogHeader());
	setCurStatus(tState);	
}

/*
MCP_CUR_STATE McpSubscriber::getCurStatus()
{
	if (m_handle == MCP_INVALID_HANDLE)
	{
		return MCP_STOPPED;
	}

	if (McpObject::getCurStatus() != MCP_SUBSCRIBING)
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
		glog(Log::L_ERROR, L"MCPSubscriber(%08X): MCPGetAttribute, Can not read Current state. closing subscriber.", this);
		
		return MCP_FAILED;		
	}

	McpObject::setCurStatus(CurState);

	return CurState;
}
*/

BOOL McpSubscriber::checkAliving()
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

void McpSubscriber::setCurStatus(MCP_CUR_STATE tCurStatus)
{
	_dwLastCheckTime = GetTickCount();

	if (tCurStatus == MCP_FAILED)
	{
		glog(Log::L_DEBUG, L"%s: setCurStatus MCP_FAILED.", getLogHeader());
	}

	McpObject::setCurStatus(tCurStatus);
}
