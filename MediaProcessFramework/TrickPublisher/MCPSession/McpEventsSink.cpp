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
// Ident : $Id: McpEventsSink.cpp,v 1.15 2004/08/09 07:57:34 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : implement class McpEventsSink  to sink and dispatch events for MCP object
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/MCPSession/McpEventsSink.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     05-04-25 15:41 Hongye.gu
// 
// 17    04-12-14 15:00 Jie.zhang
// 
// 16    04-11-26 18:32 Jie.zhang
// 
// 15    04-11-22 14:27 Jie.zhang
// 
// 13    04-11-15 11:20 Kaliven.lee
// 
// 12    04-11-09 19:11 Jie.zhang
// 
// 11    04-11-09 14:02 Kaliven.lee
// cast handle to dword for print
// 
// 10    04-11-09 10:39 Kaliven.lee
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
// Revision 1.15  2004/08/09 07:57:34  wli
// unicode modified
//
// Revision 1.14  2004/07/16 08:43:22  wli
// wsprintf    %s    2    %S
//
// Revision 1.13  2004/07/16 03:07:34  wli
// fix memory leak when terminate the session
//
// Revision 1.12  2004/07/15 06:55:37  jshen
// no message
//
// Revision 1.11  2004/07/14 03:02:43  jshen
// fix some bug
//
// Revision 1.10  2004/07/06 07:20:43  jshen
// add skeleton for SeaChange AppShell Service
//
// Revision 1.9  2004/07/05 08:51:03  wli
// no message
//
// Revision 1.8  2004/07/01 06:28:19  jshen
// after merge
//
// Revision 1.7  2004/06/30 05:46:16  wli
// redirect dispatch
//
// Revision 1.6  2004/06/29 05:52:13  wli
// some parament errors after a process review
//
// Revision 1.5  2004/06/28 05:38:38  wli
// Modified to be compatible to the MBCS instead of UNICODE in project setting
//
// Revision 1.4  2004/06/24 11:16:36  wli
// modify the include file
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
// McpEventsSink.cpp: implementation of the McpEventsSink class.
//
//////////////////////////////////////////////////////////////////////

#include "log.h"
#include "tchar.h"
#include "McpEventsSink.h"
//#include "MCPMsgs.h"
//#include "McpSession.h"
//#include "McpApi.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

McpEventsSink g_McpSink;
//extern MCPSETTING* g_pMcpSetting;
using namespace ZQ::common;
McpEventsSink::McpEventsSink()
{

}

McpEventsSink::~McpEventsSink()
{
//	OutputDebugString(L"~McpEventsSink()\n");
}

void McpEventsSink::dispatchEvents(MCP_HANDLE handle, LPMCP_IOSTATUS pStat)
{
	McpObject * pObj = NULL;
	PMCP_EVENT pEvent = NULL;
	PMCP_ATTRIBUTE pAtt = NULL;
	
	try
	{
		bool readAgain = true;
		
		do
		{
			pObj = g_McpSink.find(handle);			
			if(pObj == NULL )
			{
				readAgain = false;
				break;
			}
			

			DWORD dwOptionMask = pObj->m_McpSetting.dwOptionMask;
			if(MCP_SUCCESS != pStat->status)
			{
				// fatal for event reporting - could be MCP_FAILED, MCP_CANCELLED
				if (!pObj->isClosed())
				{
					glog(Log::L_ERROR, _T("%s: eventsink: received failing status: %s, set MCP_FAILED"), pObj->getLogHeader(), MCPStatusAsStr(pStat->status));
					pObj->setCurStatus(MCP_FAILED);
				}
				
				readAgain = false;

				break;
			}
			
		
			pEvent = (PMCP_EVENT)pStat->userData;

			//show the type of event in log
			switch(pEvent->eventClass)
			{
				case MCP_EVT_CANCELLED:					
				case MCP_EVT_SERVICE_FAILED:
					{
						if (!pObj->isClosed())
						{
							glog(Log::L_ERROR, _T("%s: eventsink: received event with eventClass %s, set MCP_FAILED"), pObj->getLogHeader(), MCPEventClassAsStr(pEvent->eventClass));
							pObj->setCurStatus(MCP_FAILED);
						}

						readAgain = false;
					}
					
					break;
				case MCP_EVT_ATTR_EMPTY:				
				case MCP_EVT_OVERFLOW:					
					glog(Log::L_ERROR, _T("%s: eventsink: received event with eventClass %s"), pObj->getLogHeader(), MCPEventClassAsStr(pEvent->eventClass));
					break;					
				default:
					break;
			}

			if (!readAgain)
				break;
		
			// u can fire OnEvent to retrieve attribute here.
			// here's our attribute
			pAtt = (PMCP_ATTRIBUTE)pEvent->pEventData;

			if(pStat&&pObj)
			{
				if (!pObj->OnEvent(pStat))
				{
					readAgain = false;
					break;
				}
			}

			
			//the OnEvent will deal with it.
/*
			if(MCP_ATT_CUR_STATE == pAtt->attCode)
			{
				PMCP_CUR_STATE pState = (PMCP_CUR_STATE)pAtt->pAttData;

				if( MCP_DONE == *pState ||
					MCP_FAILED == *pState ||
					MCP_STOPPED == *pState)
				{
					// don't set up for more event notification if we're
					// MCP_DONE, MCP_FAILED, or MCP_STOPPED
					pObj->setCurStatus(*pState);

					if (*pState == MCP_FAILED)
					{
						glog(Log::L_ERROR, _T("%s: eventsink: MCP_FAILED, pAtt->attCode."), pObj->getLogHeader());
					}

					readAgain = false;
					break;
				}				
			}
*/

			// fire off another async MCPReadEvent request using the previously
			// allocated event buffer and iorequest buffer
			MCP_STATUS status;
			
			status = MCPReadEvent(handle, dwOptionMask, (PMCP_EVENT)pStat->userData, pStat);
			if(status != MCP_PENDING)
			{
				if (!pObj->isClosed())
				{
					// fatal for event reporting on this handle
					glog(Log::L_ERROR, _T("%s: eventsink: MCPReadEvent() request failed: %s"), pObj->getLogHeader(), MCPStatusAsStr(status));
					
					if (pObj->getCurStatus() == MCP_SUBSCRIBING || pObj->getCurStatus() == MCP_PUBLISHING)
						pObj->setCurStatus(MCP_FAILED);

					glog(Log::L_DEBUG, _T("%s: eventsink: MCPReadEvent. pState->status(%s), MCPGetLastExtendedError()=%s."), pObj->getLogHeader(), MCPStatusAsStr(pStat->status), MCPStatusAsStr(MCPGetLastExtendedError()));
				}				
				
				readAgain = false;
				break;
			}
		}while(0);

			// free the ioStat and eventBuf if we're not setting up
		// another MCPReadEvent request
		if(false == readAgain)
		{
			// this thread's done reading events, close it if the app says to
			//(*gpScLog)(Log::L_DEBUG,_T("HANDLE(%d):McpEventsSink:this thread's do not read events, close it if the app says to"),(DWORD)handle);
/*			if (pEvent != NULL)				
			{
				if(pEvent->eventClass != MCP_EVT_CANCELLED &&
					pEvent->eventClass != MCP_EVT_SERVICE_FAILED)
				{
					if (*(PMCP_CUR_STATE)pAtt->pAttData == MCP_DONE || 
						*(PMCP_CUR_STATE)pAtt->pAttData == MCP_FAILED ||
						*(PMCP_CUR_STATE)pAtt->pAttData == MCP_STOPPED)
					{
						if (*(PMCP_CUR_STATE)pAtt->pAttData == MCP_DONE)
							glog(Log::L_DEBUG, _T("%s: eventsink: pObj->OnClose"), pObj);						
						else
							glog(Log::L_DEBUG, _T("%s: eventsink: pObj->OnClose. *(PMCP_CUR_STATE)pAtt->pAttData (%d), MCPGetLastExtendedError()=%s. ExtendStaus(%d)"), pObj->getLogHeader(), (*(PMCP_CUR_STATE)pAtt->pAttData), MCPStatusAsStr(pStat->status), MCPStatusAsStr(MCPGetLastExtendedError()), pStat->extendedStatus);

						pObj->OnClose(*(PMCP_CUR_STATE)pAtt->pAttData);		
					}
				}
			}
*/			if(pStat)
			{
				if(pStat->userData)
				{
					free(pStat->userData);
					pStat->userData = NULL;
				}
				free(pStat);
				pStat = NULL;
			}
		}

		// free the current event data, if there is any
		if(pAtt)
		{
			MCPFreeEventAttribute(pAtt);
		}
	}
	catch(...)
	{
		//added by salien
//		OutputDebugString(L"MCPEventsSink(00000000): catch an unhandle expection, release relate ioStat and eventBuf.\n");
		glog(Log::L_ERROR, _T("MCPEventsSink(00000000): catch an unhandle expection, release relate ioStat and eventBuf."));

		if (pObj)
			pObj->OnClose(MCP_FAILED);
	
		if(pStat)
		{
			free(pStat->userData);
			pStat->userData = NULL;
			free(pStat);
			pStat = NULL;
		}
		if(pAtt)
		{
			MCPFreeEventAttribute(pAtt);
		}
	}

}