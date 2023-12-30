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
// Ident : $Id: McpSession.cpp,v 1.28 2004/08/17 02:56:31 jshen Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : implement class McpSession to deal the session of MCP
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/MCPSession/McpSession.cpp $
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
// 10    04-11-09 14:02 Kaliven.lee
// cast handle to dword for print
// 
// 9     04-11-04 18:44 Kaliven.lee
// 
// 8     04-11-04 9:53 Kaliven.lee
// after unit test and modified
// 
// 7     04-10-28 16:38 Kaliven.lee
// 
// 6     04-10-22 11:51 Kaliven.lee
// 
// 5     04-10-11 18:57 Kaliven.lee
// 
// 4     04-10-11 18:15 Kaliven.lee
// add the default interface setting 
// Revision 1.28  2004/08/17 02:56:31  jshen
// remove the global threadpool
//
// Revision 1.27  2004/08/09 07:57:34  wli
// unicode modified
//
// Revision 1.26  2004/08/03 07:16:55  wli
// add subscriber error messages
//
// Revision 1.25  2004/07/20 10:33:25  jshen
// no message
//
// Revision 1.24  2004/07/19 08:21:31  wli
// add MCP_SESSION_FAILED setting
//
// Revision 1.23  2004/07/16 08:43:44  wli
// error message
//
// Revision 1.22  2004/07/16 03:09:54  wli
// correct the close function to unregister the handle
//
// Revision 1.21  2004/07/15 09:03:32  wli
// first stab version
//
// Revision 1.19  2004/07/15 03:19:16  wli
// fix the mcp process error. and set all attributes in bind function
//
// Revision 1.18  2004/07/14 03:02:43  jshen
// fix some bug
//
// Revision 1.17  2004/07/13 04:10:35  jshen
// for mca session test
//
// Revision 1.16  2004/07/12 06:44:47  jshen
// because of the IP change of the CVS server
//
// Revision 1.15  2004/07/08 03:53:12  wli
// replace some parameters from string to wstring according MCPAPI
//
// Revision 1.14  2004/07/07 06:33:45  wli
// add enum define
//
// Revision 1.13  2004/07/06 07:20:43  jshen
// add skeleton for SeaChange AppShell Service
//
// Revision 1.12  2004/07/05 09:01:41  wli
// clean up some unused member and set mcp setting as member
//
// Revision 1.11  2004/07/02 09:45:52  wli
// set mcpsetting as member and fix some allocate error
//
// Revision 1.10  2004/07/01 07:46:19  wli
// change the order of include file to correct reinclude windows.h
//
// Revision 1.9  2004/07/01 05:54:39  wli
// add comment
//
// Revision 1.8  2004/06/30 06:41:42  wli
// no message
//
// Revision 1.7  2004/06/30 06:00:03  wli
// correct vector error
//
// Revision 1.6  2004/06/30 05:51:03  wli
// modified signal function
//
// Revision 1.5  2004/06/29 05:52:13  wli
// some parament errors after a process review
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
// McpSession.cpp: implementation of the McpSession class.
//
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include "log.h"
#include "McpSession.h"
#include "../mcpsdk/MCPMsgs.h"
#include "tchar.h"
#include "../stdafx.h"


//MCPSETTING must be globe because  some of static function need to access
// it to get dwOptionMask option in MCPSETTING

extern McpEventsSink g_McpSink;
using namespace ZQ::common;
using namespace std;

const int  TIMEOUT_PUBFIN_SUBFIN = 60 * 1000;  // timeout for between publish finish and all subscribe finish

// don't know why sometimes, the MCPStatusAsStr return "unknown xxx" when st=MCP_EX_PGM
const wchar_t* my_MCPStatusAsStr(MCP_STATUS st)
{
	static const wchar_t buf[] = L"MCP_EX_PGM - PGM module reported an error";
	if (st == MCP_EX_PGM)
	{
		return buf;
	}
	else
		return MCPStatusAsStr(st);
}

McpSession::McpSession()
{

	m_uiObject = 0;
	m_dwStatus = MCP_SESSION_NONE;
	strcpy(_swErrorMessage, "NOERROR");
	
	_sLogHeader[0] = L'\0';

	_bKeepPolling = false;

	m_pPublisher=NULL;
}

McpSession::~McpSession()
{
//	OutputDebugString(L"~McpSession()\n");
//	ATLASSERT(FALSE);
	for (unsigned int i = 0; i < m_SubVector.size(); i++)
	{
		if (m_SubVector[i])
		{
			delete m_SubVector[i];
			m_SubVector[i] = NULL;
		}
	}
	
	if(m_SubVector.size()>0)
	{
		m_SubVector.clear();
	}
	//release resource 
	//modified by salien
	if(m_pPublisher)
	{
//		ATLASSERT(FALSE);
		delete m_pPublisher;
		m_pPublisher = NULL;

	}

}

/*
	Function:		terminate
	Description:	Terminate the Mcp session during the process of MCP
	Parameter:		void
	Return:			void
	Remark:			this operation is called by Session Manager to terminate the process.
					And all the related resource and operator will be release. And pending 
					operation will returned by MCP_STATUS or MCP_CANCEL.
*/
void McpSession::terminate(void)
{	
	_bKeepPolling = false;
}
/*
	Function:		setVODpkgAttr
	Description:	set the attributes of VOD pkg
	Parameter:		bool	bVodpkg					disable client-side vodpkg
					bool	bVodpkgDeleteOnFail		disable vodpkg subscriber delete on fail
					bool    bTxSpeedFiles			transfer only .mpg and .vvx files (default: disabled)
					bool	bVvtTransfer			enable VVT transfer
					bool	bEnableServerSideVodpkgDelete	disable server-side vodpkg delete	
	Remark:			if this function isn't called before the mcp session start, mcp session will use default setting
*/
void McpSession::setVODpkgAttr(bool bVodpkg,bool bVodpkgDeleteOnFail,bool bTxSpeedFiles,bool bVvtTransfer,bool bEnableServerSideVodpkgDelete)
{
	//bVodpkg must be true before call this function
	if(bVodpkg)
	{
		m_McpSetting.dwOptionMask = MCP_OPTM_VODPKG;
		if(true == bVodpkgDeleteOnFail)
			m_McpSetting.dwOptionMask |= MCP_OPTM_VODPKG_DELETE_ON_FAIL;

		if(false == bTxSpeedFiles)
			m_McpSetting.dwOptionMask |= MCP_OPTM_VODPKG_NO_SPEED_FILES;

		if(true == bVvtTransfer)
			m_McpSetting.dwOptionMask |= MCP_OPTM_VODPKG_ENABLE_VVT;
	}
	
	if(true == bEnableServerSideVodpkgDelete)
		m_McpSetting.dwServerSideDeleteOnFail = 1;
}


/*
Function:       setMediaAttr
Description:    set media attribute that will be transfered
Parameter:      
wstring			fileName				set media file name. the raw file name only
										Note: always 0 because we're live transfer
unsigned long	ulContentBitrate		set media file bitrate
unsigned short	uiContentHorizRes		set media file horizontal rate
unsigned short	uiContentVerticalRes	set media file vertical rate
string			briefPrvStr				set brief private string 
string			fullPrvStr				set full private string,Max length is 2000
Return:         void
Remark:         If this function is not called before MCP , Mcpsession will use default values.
*/
 void McpSession::setMediaAttr(const wchar_t* fileName,			
							  unsigned long ulContentBitrate,
							  unsigned short uiContentHorizRes,
							  unsigned short uiContentVerticalRes,
							  const wchar_t* briefPrvStr,
							  const wchar_t* fullPrvStr)
{
	glog(Log::L_INFO, _T("%s: Set Media %s to be Published"), getLogHeader(), fileName);
	
	m_mediaInfo.fileName = fileName;	
	m_mediaInfo.uiContentHorizRes = uiContentHorizRes;
	m_mediaInfo.uiContentVerticalRes = uiContentVerticalRes;
	m_mediaInfo.ulContentBitrate = ulContentBitrate;

	char tmp[1024];

	if (!fullPrvStr)
		m_mediaInfo.fullPrvString = "";
	else
	{
		WideCharToMultiByte(CP_ACP, NULL, fullPrvStr, -1, tmp, sizeof(tmp), NULL, NULL);
		m_mediaInfo.fullPrvString = tmp;
	}

	if (!briefPrvStr)
		m_mediaInfo.briefPrvString = "";
	else
	{
		WideCharToMultiByte(CP_ACP, NULL, briefPrvStr, -1, tmp, sizeof(tmp), NULL, NULL);
		m_mediaInfo.briefPrvString = tmp;
	}
}
/*
Function:			setXferAttr
Description:		Set transfer attribute for MCP session
Parameter:			MCP_XFER_TYPE		xferType			set transfer type
					DWORD				dwBitRate			set transfer bit rate
					unsigned long		ulRteStreamVersion  set real time encode stream version
					bool				zeroLenFail			disable zeroLenFail
					DWORD				dwXferTimeOut		subscriber live transfer timeout in seconds (default: not set)
Return:				void
Remark:				if this function isn't called before the mcp session start, mcp session will use default setting.
*/
void McpSession::setXferAttr(MCP_XFER_TYPE xferType,DWORD dwBitRate,unsigned long ulRteStreamVersion,bool zeroLenFail,DWORD dwXferTimeOut)
{
	m_McpSetting.XferType = xferType;
	m_McpSetting.dwMaxBitRate = dwBitRate;
	
	if(1 == ulRteStreamVersion)
	{
		m_McpSetting.RTEStreamVer = MCP_RTE_STREAM_VERSION_ONE;
	}
	else
	{
		m_McpSetting.RTEStreamVer = MCP_RTE_STREAM_VERSION_TWO;
	}
	if(true == zeroLenFail)
		m_McpSetting.dwRTEZeroLenFail = 1;
	m_McpSetting.dwXferTimeOut = dwXferTimeOut;
}
/*
Function:       addSubscriber
Description:    add subscriber define to session
Parameter:      string			
Return:         void
Remark:         session maintance a subscriber vector.this function insert a subscriber into the vector
*/

void McpSession::addSubscriber(const wchar_t* subName, const wchar_t* Addr,DWORD Port, const wchar_t* IfaceAddr)
{
	wstring wsIfaceAddr;	
	if(IfaceAddr == NULL || IfaceAddr[0] == L'\0')
	{
		wsIfaceAddr = Addr;
	}
	else
		wsIfaceAddr = IfaceAddr;
	
	glog(Log::L_INFO, _T("%s: add a subscriber %s(A),%s(I),%d(P) %s(SubName)"), getLogHeader(), Addr, IfaceAddr,Port,subName);
	
	McpSubscriber* pSub = new McpSubscriber(this,Addr,Port, wsIfaceAddr);
		
	m_mediaInfo.subName = subName;
	
	m_SubVector.push_back(pSub);	

	wchar_t   swText[256];

	swprintf(swText, L"%s: Subscriber(%d)", getLogHeader(), m_SubVector.size());

	pSub->setLogHeader(swText);
}
/*
Function:       setPublisher
Description:    set publisher for subscriber
Parameter:      wstring			Addr		IP address for publisher
DWORD			Port		publisher's port default is 5000
wstring			IfaceAddr	interface address
Return:         void
Remark:         this function must be called before mcpsession start. otherwise mcpsession will be failed
*/
void McpSession::setPublisher(const wchar_t* Addr,DWORD Port, const wchar_t* IfaceAddr)
{
	wstring wsIfaceAddr;
	if(IfaceAddr[0] == L'\0') 
	{
        wsIfaceAddr = Addr;
	}	
	else
		wsIfaceAddr = IfaceAddr;
	
	glog(Log::L_INFO, _T("%s: set publish %s(A),%s(I),%d(P)"), getLogHeader(), Addr, wsIfaceAddr.c_str(),Port);
	
	m_pPublisher = new McpPublisher(this,Addr,Port,IfaceAddr);	

	wchar_t   swText[256];

	swprintf(swText, L"%s: Publish", getLogHeader());

	m_pPublisher->setLogHeader(swText);
	//write here because I afraid of when terminate, the thread not execute to this sentense
	_bKeepPolling = true;
}

/*
Function:       fatalFailed
Description:    process the fatal failed event 
Parameter:      
Return:         void
Remark:         
*/
void McpSession::fatalFailed(const char* sWhy)
{	
	if (m_pPublisher)	
	{
		glog(Log::L_INFO, "%s: fatalFailed, delete publish", getLogHeader());
        m_pPublisher->close();
		delete m_pPublisher;
		m_pPublisher = NULL;
	}

	if(m_SubVector.size() > 0)
	{
		for(int i=0;i<m_SubVector.size();i++)
		{
			if(m_SubVector[i])
			{
				m_SubVector[i]->close();
				delete m_SubVector[i];
				m_SubVector[i] = NULL;
			}
		}
	}	

	m_dwStatus = MCP_SESSION_FAILED;

	if (sWhy)
		strcpy(_swErrorMessage, sWhy);
}

/*
Function:       setMcpAttr
Description:    set MCP attributes 
Parameter:      
Return:         void
Remark:         if this function is not called before session started, Mcpsession will use default values
*/

void McpSession::setMcpAttr(const wchar_t* sMCastAddr, DWORD dwEventInterval,DWORD dwMaxWaitSecs)
{
	m_McpSetting.sMcastAddr = sMCastAddr;
	m_McpSetting.dwEventInterval = dwEventInterval;
	m_McpSetting.dwMaxWaitSecs = dwMaxWaitSecs;
}

/*
Function:       run
Description:    this function is main process of MCP.It will be blocked only if the endSession function is called.
Parameter:      void
Return:         error return 1.otherwise return 0
Remark:         this function is called by work thread 
*/

int McpSession::run(void)
{
//	ATLASSERT(FALSE);

	char buf[1024];
	
	int iFailedTransfers = 0;
	int iSuccessTransfer = 0;
	int iSubCount = m_SubVector.size();	

	if(!m_pPublisher)
	{
		sprintf(buf, "%s: Publisher can't be initialized.Try to Call SetPublisher() at first!", getLogHeader());
		glog(Log::L_ERROR, buf);

		fatalFailed(buf);
		return 1;
	}
	
	if (!iSubCount)
	{
		sprintf(buf, "%s: No subscriber, publish failed.", getLogHeader());
		fatalFailed(buf);
		return 1;
	}

	// here setMcpSetting must be called before open have been called 	
	m_pPublisher->setMcpSetting(m_McpSetting);
	MCP_STATUS status = m_pPublisher->open();
	if(status != MCP_EX_NONE )
	{
		sprintf(buf,"%s: Publisher(%s) open Failed.[%s] ", getLogHeader(), 
					m_pPublisher->m_sAddr.c_str(), my_MCPStatusAsStr(status));

		glog(Log::L_ERROR, buf);
		
		fatalFailed(buf);
		return 1;
	}
	else
		glog(Log::L_DEBUG, _T("%s: Publisher(%s) open success."), getLogHeader(), m_pPublisher->m_sAddr.c_str());
	
	
	m_pPublisher->setMediaInfo(m_mediaInfo);

	
	status = m_pPublisher->bind(m_SubVector[0]->m_dwPort);
	if(status != MCP_EX_NONE )
	{
		sprintf(buf,"%s: MCPSession: Publisher bind Failed.[%s] ", getLogHeader(), my_MCPStatusAsStr(status));

		glog(Log::L_ERROR, _T("%s: Publisher bind Failed.[%s]"), getLogHeader(), my_MCPStatusAsStr(status));

		fatalFailed(buf);
		return 1;
	}

	//before u setPrivateData u must setMediaInfo at first
	status = m_pPublisher->setPrivateData();
	if(status != MCP_EX_NONE )
	{	
		sprintf(buf,"%s: McpSession: Publisher setPrivateData Failed.[%s] ", getLogHeader(), my_MCPStatusAsStr(status));
		glog(Log::L_ERROR, _T("%s: Publisher setPrivateData Failed.[%s]"), getLogHeader(), my_MCPStatusAsStr(status));
	}

	try{
	
	for(unsigned int i = 0;i<iSubCount; i++)
	{		
		m_SubVector[i]->setMcpSetting(m_McpSetting);
		status = m_SubVector[i]->open();
		
		if(status != MCP_EX_NONE)
		{
		
			sprintf(_swErrorMessage, "%s: Open subscribe %s Failed.[%s] \n", getLogHeader(), m_SubVector[i]->m_sAddr.c_str(),my_MCPStatusAsStr(status));
			glog(Log::L_ERROR, _swErrorMessage);
			
			//added by salien 
			OutputDebugString(_swErrorMessage);

			delete m_SubVector[i];
			m_SubVector[i] = NULL;

			iFailedTransfers++;
			
			continue;
		}
		else
		{
			glog(Log::L_DEBUG, L"%s: Open subscriber %s successful", getLogHeader(), m_SubVector[i]->m_sAddr.c_str());
			
		}

		m_SubVector[i]->setMediaInfo(m_mediaInfo);

		status = m_SubVector[i]->bind(m_pPublisher->getPubGSID(),m_pPublisher->m_dwPort);
		if (status != MCP_EX_NONE)
		{
			sprintf(_swErrorMessage, "%s: bind subscribe %s Failed.[%s]\n", getLogHeader(), m_SubVector[i]->m_sAddr.c_str(), my_MCPStatusAsStr(status));
			glog(Log::L_ERROR, _swErrorMessage);
			
			//added by salien
			OutputDebugString(_swErrorMessage);

			delete m_SubVector[i];
			m_SubVector[i] = NULL;
				
			iFailedTransfers++;
	
			continue;
		}
		else
		{
			glog(Log::L_DEBUG, L"%s: bind subscriber %s successful", getLogHeader(), m_SubVector[i]->m_sAddr.c_str());
		}


		status = m_SubVector[i]->subscribe();
		if(status != MCP_EX_NONE)
		{
			sprintf(_swErrorMessage, "%s: subscribe %s subscribe() failed.[%s] \n", getLogHeader(), m_SubVector[i]->m_sAddr.c_str(), my_MCPStatusAsStr(status));

			glog(Log::L_ERROR, _swErrorMessage);

			OutputDebugString(_swErrorMessage);

			iFailedTransfers++;
			delete m_SubVector[i];
			m_SubVector[i] = NULL;
		}
		else
		{
			glog(Log::L_DEBUG, L"%s: subscribe %s subscribe() successful.", getLogHeader(), m_SubVector[i]->m_sAddr.c_str());
		}
	}
	
	glog(Log::L_DEBUG, _T("%s: begin check failed sub before publish."), getLogHeader());

	}
	catch(...)
	{
		glog(Log::L_ERROR, _T("%s: there is a execption in subscrib init."), getLogHeader());
		fatalFailed(_T("there is a execption in subscrib init."));

		//added by salien
		sprintf(_swErrorMessage, "%s: there is a execption in subscrib init.\n", getLogHeader());
		OutputDebugString(_swErrorMessage);

		return 1;
	}

	if(iFailedTransfers == iSubCount)
	{
		fatalFailed(NULL);

		return 1;
	}

	glog(Log::L_DEBUG, _T("%s: begin publish."), getLogHeader());

	status = m_pPublisher->publish();
	if(status != MCP_EX_NONE)
	{
		sprintf(buf, "%s: Publish() error.[%s]!\n", getLogHeader(), my_MCPStatusAsStr(status));
		glog(Log::L_ERROR, buf);
		
		//added by salien
		OutputDebugString(buf);

		fatalFailed(buf);
		return 1;
	}
	else
		glog(Log::L_DEBUG, _T("%s: publish() ok."), getLogHeader());

	m_dwStatus = MCP_SESSION_RUNING;
		

	// exit when finished
	while(_bKeepPolling)
	{
		Sleep(1000);
		
		m_pPublisher->checkAliving();//if failed, it will set the publish's current status.
		
		// query publisher status
		MCP_CUR_STATE pubStatus = m_pPublisher->getCurStatus();
		
		/// when all subscribes failed ,close the publisher 
		//so next loop the status of the publisher will be MCP_CLOSED	
		if(( MCP_DONE == pubStatus))
		{
			glog(Log::L_DEBUG, _T("%s: Publish finish, begin waiting for subscriber exit."), getLogHeader());
			
			DWORD dwTimeWaited = 0;

			int nDone = 0, nSuccess = 0;
			while(_bKeepPolling)
			{
				Sleep(m_McpSetting.dwMaxWaitSecs * 1000);

				dwTimeWaited += m_McpSetting.dwMaxWaitSecs * 1000;

				if (dwTimeWaited >= TIMEOUT_PUBFIN_SUBFIN)
				{
					glog(Log::L_ERROR, _T("%s: Publish finish, but waiting for subscriber exit time out, stop waiting."), getLogHeader());

					_bKeepPolling = false;
					break;
				}

				try{
				
				
				// count the active subscribers, if there's none left, exit
				nDone = 0;
				nSuccess = 0;
				for(unsigned int i = 0; i < iSubCount; i++)
				{					
					McpSubscriber *pSub = m_SubVector.at(i);
					if (pSub == NULL)
					{
						nDone++;
						continue;
					}

					MCP_CUR_STATE subStatus = pSub->getCurStatus();

					if (MCP_DONE == subStatus)				
					{
						nSuccess++;
						nDone++;

						glog(Log::L_DEBUG,_T("%s: MCP_DONE begin to call sub close, nDone(%d), nSuccess(%d). 6"), getLogHeader(), nDone, nSuccess);
						
						pSub->close();
						delete m_SubVector[i];
						m_SubVector[i] = NULL;

						continue;
					}
					
					if( subStatus != MCP_SUBSCRIBING)					
					{
						nDone++;

						glog(Log::L_ERROR,_T("%s: begin to call sub close, nDone(%d), nSuccess(%d). 5"), getLogHeader(), nDone, nSuccess);
						
						pSub->close();
						delete m_SubVector[i];
						m_SubVector[i] = NULL;
					}
					else if (!pSub->checkAliving())
					{
						nDone++;

						glog(Log::L_ERROR,_T("%s: begin to call sub close, nDone(%d), nSuccess(%d). 4"), getLogHeader(), nDone, nSuccess);
						pSub->close();
						delete m_SubVector[i];
						m_SubVector[i] = NULL;
					}
				}

				if(nDone == iSubCount)
				{	
					if(nSuccess)// >0 is success, 
					{
						m_dwStatus = MCP_SESSION_CLOSED;
						glog(Log::L_INFO, _T("%s: MCP session exit with Successful close"), getLogHeader());
					}
					else
					{
						m_dwStatus = MCP_SESSION_FAILED;
						glog(Log::L_ERROR, _T("%s: MCP session exit with all subscribe failed"), getLogHeader());
						
						sprintf(_swErrorMessage, "%s: MCPSession: Publish Done,but all Subscribe failed.", getLogHeader());
					}


					_bKeepPolling = false;	
				}				
			}
			catch(...)
			{
				glog(Log::L_ERROR ,_T("%s: there is a unknown excpetion in run. 1"), getLogHeader());	

				//added by salien
				OutputDebugString(_T("%s: there is a unknown excpetion in run. in while(_bKeepAliving)1\n"));	
				_bKeepPolling = false;
			}
			}


			m_pPublisher->close();
			delete m_pPublisher;
			m_pPublisher = NULL;

			if (nDone != iSubCount)
			{
				if(nSuccess)// >0 is success, 
				{
					m_dwStatus = MCP_SESSION_CLOSED;
					glog(Log::L_INFO, _T("%s: MCP session exit with Successful close"), getLogHeader());
				}
				else
				{
					m_dwStatus = MCP_SESSION_FAILED;
					glog(Log::L_ERROR, _T("%s: MCP session exit with all subscribe failed"), getLogHeader());
					sprintf(_swErrorMessage, "%s: Publish Done,but all Subscribe failed.\n", getLogHeader());

					//added by salien
					OutputDebugString(_swErrorMessage);
				}
			}

			glog(Log::L_DEBUG, _T("%s: end of waiting for subscriber exit, TimeWaited(%ds), nDone(%d), nSuccess(%d), SubCount(%d)."), getLogHeader(), dwTimeWaited/1000, nDone, nSuccess, iSubCount);
		}
		else if( MCP_FAILED == pubStatus)
		{	
			sprintf(buf, "%s: exit with publisher failed.\n", getLogHeader());
			
			glog(Log::L_ERROR, buf);			

			fatalFailed(buf);
			
			//added by salien
			OutputDebugString(buf);

			_bKeepPolling = false;			
		}
		else
		{		
			// check the subscriber if is ok

			int nDone = 0;
			MCP_CUR_STATE  curStatus; 

			for(unsigned int i = 0; i < iSubCount; i++)
			{
				McpSubscriber *pSub = m_SubVector.at(i);
				if (!pSub)
				{
					nDone++;
					continue;
				}

				curStatus = pSub->getCurStatus();
				if( curStatus == MCP_SUBSCRIBING)	
				{
					if (!pSub->checkAliving())
					{
						glog(Log::L_ERROR,_T("%s: begin to call sub close. 2"), getLogHeader());
						
						nDone++;
						pSub->close();
						delete m_SubVector[i];
						m_SubVector[i] = NULL;
					}
				}
				else if (curStatus != MCP_DONE)
				{
					glog(Log::L_ERROR,_T("%s: begin to call sub close. 1"), getLogHeader());

					nDone++;
					pSub->close();
					delete m_SubVector[i];
					m_SubVector[i] = NULL;
				}
			}


			if(nDone == iSubCount)
			{
				sprintf(buf, "%s: All the subscriber failed.\n", getLogHeader());
				glog(Log::L_ERROR, buf);
				
				fatalFailed(buf);
				
				//added by salien
				OutputDebugString(buf);

				//loop will exit when entry the loop next time				
				_bKeepPolling = false;
			}
		}
	}

	glog(Log::L_DEBUG, _T("%s: MCP session exit the run thread."), getLogHeader());

	
	return 0;
}

