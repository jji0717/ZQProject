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
// Ident : $Id:  $
// Branch: $Name:  $
// Author: Bernie Zhao (Tianbin Zhao)
// Desc  : rtsp client connection implementation
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/RtspClient/RtspClientConnection.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 25    05-07-29 16:15 Bernie.zhao
// 
// 24    05-04-19 21:18 Bernie.zhao
// autobuild modification
// 
// 23    05-03-24 14:51 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 22    05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// 
// 21    05-01-05 17:16 Bernie.zhao
// modified to support STV status feedback
// 
// 20    04-12-16 14:55 Bernie.zhao
// 
// 19    04-12-06 20:55 Bernie.zhao
// socket reset improved
// 
// 18    04-11-30 10:24 Bernie.zhao
// Nov30, after resolving ISSStreamTerminate problem
// 
// 17    04-11-23 10:02 Bernie.zhao
// 
// 16    04-11-03 19:59 Bernie.zhao
// Oct/3 Before Service Release
// 
// 15    04-10-28 18:19 Bernie.zhao
// before trip
// 
// 14    04-10-26 16:08 Bernie.zhao
// 0.1.6 Oct/26
// 
// 13    04-10-24 19:04 Bernie.zhao
// end of 2004/Oct/24
// 
// 12    04-10-24 11:43 Bernie.zhao
// before 24/Oct
// 
// 11    04-10-22 16:37 Bernie.zhao
// 
// 10    04-10-21 15:28 Bernie.zhao
// 
// 9     04-10-18 10:33 Bernie.zhao
// test 1 ok
// 
// 8     04-10-15 16:14 Bernie.zhao
// 
// 7     04-10-14 14:00 Bernie.zhao
// 
// 6     04-10-12 9:55 Bernie.zhao
// pragma warning 4786
// 
// 5     04-10-07 16:52 Bernie.zhao
// added connection query interface with purchase id
// 
// 4     04-10-07 16:01 Bernie.zhao
// fixed TEARDOWN problem
// 
// 3     04-09-22 9:29 Bernie.zhao
// 
// 2     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// ===========================================================================
//#include "StdAfx.h"
#include ".\rtspclientconnection.h"
#include <process.h>

#include "RtspRecvSink.h"

#include "../MainCtrl/ScheduleTV.h"

extern ScheduleTV gSTV;

RtspClientConnection::RtspClientConnection(RtspConnectionManager* manager)
{
	_Heartbeat		=	DEFAULT_KEEPALIVESEC;
	_CSeq			=	0;
	_State			=	RTSP_DISCONNECT;
	_isBlocking		=	FALSE;
	_SetupSession	=	"";

	_ActiveConn		=	false;
	_sd				=	-1;
	_CManager		=	manager;
	_pCurrrentList	=	NULL;
	
	// create recv signal
	_hSentOrFail[0]	= ::CreateEvent(0,0,0,0);
	_hSentOrFail[1]	= ::CreateEvent(0,0,0,0);
	_hGotOrFail[0]	= ::CreateEvent(0,0,0,0);
	_hGotOrFail[1]	= ::CreateEvent(0,0,0,0);

}

RtspClientConnection::~RtspClientConnection(void)
{
	// stop socket
	if(_ActiveConn)
		cease();

	// remove recovery info
	_SetupSession	=	"";
	_SetupMsg.clearMessage();

	// delete RTSPSock
	if(_pRtspSd)
		delete _pRtspSd;

	// delete event handles
	::CloseHandle( _hSentOrFail[0]);
	::CloseHandle( _hSentOrFail[1]);
	::CloseHandle( _hGotOrFail[0]);
	::CloseHandle( _hGotOrFail[1]);
}

bool RtspClientConnection::init(SOCKET sd, const char* hostname, const char* hostport, int nsec /*= DEFAULT_NSEC*/)
{
	// attributes initializing
	_NSec			=	nsec;
	_HostPort		=	hostport;
	_HostName		=	hostname;
	
	SOCKET tempsd;

	// open socket
	_pRtspSd = new RtspSock (sd);
	tempsd = _pRtspSd->openClient(hostname, hostport, nsec);

	// test socket validity
	if(tempsd == INVALID_SOCKET) {
#ifdef _DEBUG
		printf("RtspClientConnection::init()  Invalid socket %d with error code %d\n",sd,::WSAGetLastError());
#endif
		glog(ZQ::common::Log::L_ERROR, "FAILURE  [%06d]RtspClientConnection::init()  Can not connect socket %d to server, with error code %d", _PurchaseID, sd, ::WSAGetLastError() );
		//throw errnum;
		//throw ZQ::common::IOException(errorstr);
		return false;
	}

	_sd = tempsd;

	// connection active
	_ActiveConn		=	true;
	_State = RTSP_IDLE;

	return true;
}

bool RtspClientConnection::cease(void)
{
	if(_ActiveConn) {
		_ActiveConn		=	false;
		_CSeq			=	0;
		_State			=	RTSP_DISCONNECT;
		
		//::shutdown(_sd, SD_BOTH);
		_pRtspSd->closeSock();
		
		_sd	= INVALID_SOCKET;
		_CManager->updateSock(this);
		

		// clear msg
		_msgLock.enter();
		_RecvMsg.clearMessage();
		_SentMsg.clearMessage();
		_msgLock.leave();
	}

	return true;
}

bool RtspClientConnection::reset(void)
{
	if(!_pRtspSd->resetSock()) {
#ifdef _DEBUG
		printf("RtspClientConnection::reset()  Could not reset RtspSock");
#endif
		glog(ZQ::common::Log::L_ERROR, "RtspClientConnection::reset()  [%06d]Could not reset RtspSock", _PurchaseID);
		cease();
		return FALSE;
	}
	
	_sd = _pRtspSd->getSock();
	_CManager->updateSock(this);

	_CSeq			=	0;
	_isBlocking		=	FALSE;
	_SetupSession	=	"";
	
	_State = RTSP_IDLE;
	_ActiveConn = TRUE;
	return TRUE;
}

bool RtspClientConnection::sendMSG(RtspRequest	reqmsg, RtspResponse	&feedback/*,  int timeout*/)
{
	// check if Sink is working
	if(!_CManager->getListener()->isRunning()) {
		glog(ZQ::common::Log::L_WARNING, "FAILURE  [%06d]RtspClientConnection::sendMSG()  RecvSink is no longer running, restarting", _PurchaseID);
		_CManager->getListener()->start();
	}
	
	std::string msgtype = reqmsg.getCommandtype();

	// fill "CSeq" and "Session" fields
	char acseq[16];
	int seq = getCSeq();
	itoa(seq, acseq, 10);
	reqmsg.setHeaderField(KEY_CSEQ,acseq);
	if(!_SetupSession.empty())
		reqmsg.setHeaderField(KEY_SESSION, _SetupSession);

	// back up SETUP message for reestablish
	if(msgtype == "SETUP")
		_SetupMsg = reqmsg;

	// prepare for response catch
	_RecvMsg.clearMessage();

	// backup sent message
	_WaitMsg.clearMessage();
	_WaitMsg = reqmsg;

	// signal sending request event
	_CManager->getListener()->waitingListPush(_sd);
	_CManager->getListener()->OnSetWillSend();
	
	//////////////////////////////////////////////////////////////////////////
	// catch response
	
	// see if sent
	int respstatus = WaitForMultipleObjects(2, _hSentOrFail, FALSE, _NSec);
	if (respstatus == WAIT_TIMEOUT)	// timeout, not sent
	{
		glog(ZQ::common::Log::L_ERROR, "FAILURE  [%06d]RtspClientConnection::sendMSG()  Can not send request to server with socket %d", _PurchaseID, _sd);
	}
	else if(respstatus == WAIT_OBJECT_0) // sent
	{
	}
	else if(respstatus == WAIT_OBJECT_0+1) // not sent
	{
		glog(ZQ::common::Log::L_ERROR, "FAILURE  [%06d]RtspClientConnection::sendMSG()  Can not send request to server with socket %d", _PurchaseID, _sd);
		cease();
		_isBlocking = FALSE;
		return FALSE;
	}
	else	// wait error
	{
		respstatus = GetLastError();
		_isBlocking = FALSE;
		glog(ZQ::common::Log::L_ERROR, "FAILURE  [%06d]RtspClientConnection::sendMSG()  ::WaitForMultipleObjects() error %d\n", _PurchaseID, respstatus);
	}

	// see if got
	respstatus = WaitForMultipleObjects(2, _hGotOrFail, FALSE, _NSec);
	if (respstatus == WAIT_TIMEOUT)	// timeout, no response
	{
		glog(ZQ::common::Log::L_ERROR, "FAILURE  [%06d]RtspClientConnection::sendMSG()  Can not receive response from server with socket %d", _PurchaseID, _sd);
		cease();
		_isBlocking = FALSE;
		return false;
	}
	else if (respstatus == WAIT_OBJECT_0) {	// got response signal
		if(_RecvMsg.isEmpty()) {
			// connection disconnected
			cease();
			_isBlocking = FALSE;
			if(msgtype=="TEARDOWN") {
				// last request is TEARDOWN, no feedback actually
				return TRUE;
			}
			else {
				return FALSE;
			}
		}

		else {
			// got response
			feedback = _RecvMsg;
		}
	}
	else if (respstatus == WAIT_OBJECT_0+1)	// got fail
	{
		glog(ZQ::common::Log::L_ERROR, "FAILURE  [%06d]RtspClientConnection::sendMSG()  Can not receive response from server with socket %d", _PurchaseID, _sd);
		cease();
		_isBlocking = FALSE;
		return false;
	}
	else	// wait error
	{
		respstatus = GetLastError();
		_isBlocking = FALSE;
		glog(ZQ::common::Log::L_ERROR, "FAILURE  [%06d]RtspClientConnection::sendMSG()  ::WaitForMultipleObjects() error %d\n", _PurchaseID, respstatus);
	}

	//////////////////////////////////////////////////////////////////////////
	// get feedback's status
	
	std::string status = feedback.getCommandstatus();
		

	if(status == "200")	 {// response says "OK"
		// change state machine and increase CSeq
		if(msgtype=="SETUP"){
			// get session_id and time-out
			std::string sessionstring = "";
			std::string tmpsession	= "";
			int	keepalive;

			sessionstring= feedback.getHeaderField(KEY_SESSION);
			size_t	semicolon = sessionstring.find_first_of(';');

			if(semicolon != std::string.npos) {	// with timeout
				tmpsession = sessionstring.substr(0,semicolon);
				RtspMsgHeader timeoutH;
				RtspMsgHeader::parseNewHeader("Session",sessionstring.c_str(), timeoutH);
				keepalive = atoi( timeoutH.getSubHeaderField("timeout").c_str());
				// update heartbeat second to a smaller one
				if(keepalive < _Heartbeat) {
					updateKeepAlive(keepalive);

					// update the manager heartbeat
					if( _Heartbeat < _CManager->getHeartbeat()) {
						_CManager->setHeartbeat( _Heartbeat );
					}
				}
			}
			else	// without timeout
				tmpsession = sessionstring;

			feedback.setHeaderField("Session", tmpsession);
			_SetupMsg.setHeaderField("Session", tmpsession);
			//tmpsession = feedback.getHeaderField("Session");

			if(!tmpsession.empty()) {
				// update state
				updateState(RTSP_READY);
				// increase CSeq according to session id
				increCSeq();
			}
			
			// back up SETUP message for reestablish
			_SetupMsg = reqmsg;
			_SetupSession = tmpsession;
		}
		else if(msgtype== "PLAY"){
			// get session_id 
			std::string tmpsession	= "";
			tmpsession = feedback.getHeaderField("Session");

			//////////////////////////////////////////////////////////////////////////
			if(!tmpsession.empty()) {
				// increase CSeq according to session id
				increCSeq();
			}
			// update state
			updateState(RTSP_PLAYING);
		}
		else if(msgtype== "PAUSE"){
			// get session_id 
			std::string tmpsession	= "";
			tmpsession = feedback.getHeaderField("Session");

			if(!tmpsession.empty()) {
				// update state
				updateState(RTSP_READY);
				// increase CSeq according to session id
				increCSeq();
			}
		}
		else if(msgtype== "GET_PARAMETER"){
			// get session_id 
			std::string tmpsession	= "";
			tmpsession = feedback.getHeaderField("Session");

			if(!tmpsession.empty()) {
				// increase CSeq according to session id
				increCSeq();
			}
		}
		else if(msgtype== "TEARDOWN"){
		// get session_id 
			std::string tmpsession	= "";
			tmpsession = feedback.getHeaderField("Session");

			if(!tmpsession.empty()) {
				// update state
				updateState(RTSP_IDLE);
				// increase CSeq according to session id
				increCSeq();
			}
		}
		
		_isBlocking = FALSE;
		return true;
	}
	else {				// response says failed
		_isBlocking = FALSE;
		return false;
	}
}

bool RtspClientConnection::sendKeepAlive()
{
	char buff[8];

	// send a heartbeat
	sprintf(buff, "\r\n");
	
#ifdef _DEBUG
	printf("%s",buff);
#endif
	int hbret = _pRtspSd->sendN( buff, 2);


#ifdef _DEBUG
	if( hbret == -1 ) {
		int hearterr = WSAGetLastError();
		printf("\nsending heartbeat error %d\n", hearterr);
		return false;
	}
#endif

	return true;
}

std::string RtspClientConnection::getDynamicMac(std::string ipaddr)
{
	std::string dynamicMac_str;
	std::string IPAddress = ipaddr;
	std::string dynamicMacHead = "01005E";
	int dynamicMac_int = 0x00000000;
	int ipZone[4];

	// separate ip address by dot
	for(int i=0;i<3;i++) {
		size_t dot = IPAddress.find_first_of('.');
		ipZone[i] = atoi( IPAddress.substr(0,dot).c_str());
		IPAddress = IPAddress.substr(dot+1,IPAddress.size()-dot-1);
	}
	ipZone[3] = atoi( IPAddress.c_str());

	dynamicMac_int = ipZone[0];
	dynamicMac_int = dynamicMac_int << 8;
	dynamicMac_int = dynamicMac_int + ipZone[1];
	dynamicMac_int = dynamicMac_int << 8;
	dynamicMac_int = dynamicMac_int + ipZone[2];
	dynamicMac_int = dynamicMac_int << 8;
	dynamicMac_int = dynamicMac_int + ipZone[3];

	dynamicMac_int = dynamicMac_int & (0x00FFFFFF >> 1);

	char hexstr[16];
	sprintf(hexstr,"%x",dynamicMac_int);
	dynamicMac_str = hexstr;
	dynamicMac_str = "000000" + dynamicMac_str;
	dynamicMac_str = dynamicMac_str.substr(dynamicMac_str.size()- 6,6);
	dynamicMac_str = dynamicMacHead + dynamicMac_str;

	char macstr[16];
	strcpy(macstr, dynamicMac_str.c_str());
	strupr(macstr);
	dynamicMac_str = macstr;

	return dynamicMac_str;
}

bool RtspClientConnection::OnAnnounce(RtspRequest annc)
{
#ifdef _DEBUG
	printf("receiving announce from server!\n");
	printf(annc.toString().c_str());
#endif
	glog(ZQ::common::Log::L_DEBUG, "NOTIFY   [%06d]RtspClientConnection::OnAnnounce()  Receiving announce with socket %d from server:\n%s",_PurchaseID, _sd, annc.toString().c_str());
	std::string NoticeStr = annc.getHeaderField(KEY_SEACHANGENOTICE);
	// skip white space
	while(NoticeStr[0]==' ')
		NoticeStr = NoticeStr.substr(1, NoticeStr.length()-1);
	// get code
	NoticeStr = NoticeStr.substr(0,4);
	int NoticeCode = atoi(NoticeStr.c_str());
	glog(ZQ::common::Log::L_DEBUG, "NOTIFY   [%06d]RtspClientConnection::OnAnnounce()  Stream on channel %d /Announce code is %d\n", _PurchaseID, _PurchaseID, NoticeCode);

	int nType;
	STVPlaylist* pCurrList;
	// update playlist status
	switch(NoticeCode) {
		case 2101:	// "End-of-Stream Reached"
		case 2104:	// "Start-of-stream Reached"
		case 5200:	// "Server Resources Unavailable"
		case 5502:	// "Internal Server Error"
		case 5403:	// "Server Shutting Down"
		
			pCurrList = gSTV.getPlayListMan()->queryCurrentPL(_PurchaseID, nType);
			
			if(!pCurrList) {	// see if it is global filler
				if(gSTV.getPlayListMan()->getChnlState(_PurchaseID)==CHNL_GPLAYING)
				{
					pCurrList = gSTV.getPlayListMan()->getCurrGlobalFiller();
				}
				else
					break;
			}
			
			// send missing status feedback if necessary
			SSAENotification missingNoti;
			if(gSTV.getPlayListMan()->getLastNoti(_PurchaseID, missingNoti))
			{
				if(missingNoti.wOperation== SAENO_PLAY)
				{	// last operation is start, with no stop, so send stop
					missingNoti.wOperation = SAENO_STOP;

					if(pCurrList->isGlobal())
					{
						gSTV.getPlayListMan()->setChnlState(_PurchaseID, CHNL_GSTOPPING);
					}
					else 
					{
						gSTV.getPlayListMan()->setChnlState(_PurchaseID, CHNL_NSTOPPING);
						pCurrList->setPLStatus(PLSTAT_DESTROYING);
					}
						
					gSTV.OnSendAEStatus(_PurchaseID, missingNoti);

				}
			}

			// update list status
			switch(pCurrList->getPLType()) {
			case LISTTYPE_PLAYLIST:
				pCurrList->setPLStatus(PLSTAT_OUTOFDATE);
				pCurrList->setPlayingAsset(0);
				break;
			case LISTTYPE_FILLER:
				pCurrList->setPLStatus(PLSTAT_IDLE);
				break;
			case LISTTYPE_BARKER:
				pCurrList->setPLStatus(PLSTAT_IDLE);
				break;
			}
			
			// update channel status
			gSTV.getPlayListMan()->setChnlState(_PurchaseID, CHNL_NONE);

//			gSTV.getPlayListMan()->setChnlCurrList(_PurchaseID, NULL);
			
			glog(ZQ::common::Log::L_DEBUG, L"NOTIFY   [%06d]RtspClientConnection::OnAnnounce()  List %s has terminated",_PurchaseID, pCurrList->getDBfile());
			break;
	}
	
	return true;
}

bool RtspClientConnection::OnResponse(RtspResponse resp)
{
	if(_isBlocking)
	{
	
		_msgLock.enter();
		_RecvMsg = resp;
		_msgLock.leave();
		
#ifdef _DEBUG
		printf("receiving response from server:\n%s",resp.toString().c_str());
#endif
		glog(ZQ::common::Log::L_DEBUG, "NOTIFY   [%06d]RtspClientConnection::OnResponse()  Receiving response with socket %d from server:\n%s",_PurchaseID, _sd, resp.toString().c_str());
	
	::SetEvent(_hGotOrFail[0]);
	}
	return true;
}

bool RtspClientConnection::OnRequest()
{
	if(!_isBlocking)
	{
		
		_msgLock.enter();
		_isBlocking = TRUE;
		
		int bytes = _WaitMsg.sendMessage(_pRtspSd);
		if(bytes<=0) {
			int errnum = WSAGetLastError();

			_isBlocking = FALSE;
			::SetEvent(_hSentOrFail[1]);

			_msgLock.leave();
			return false;
		}

#ifdef _DEBUG
		printf("sending request from client:\n%s", _WaitMsg.toString().c_str());
#endif		
		glog(ZQ::common::Log::L_DEBUG, "NOTIFY   [%06d]RtspClientConnection::OnRequest()  Sending request with socket %d from client:\n%s",_PurchaseID, _sd, _WaitMsg.toString().c_str());
		_SentMsg = _WaitMsg;
			
		::SetEvent(_hSentOrFail[0]);
		
		_msgLock.leave();
	}
	
	return true;
}

bool RtspClientConnection::OnRecover(int errornum)
{
	SOCKET oldsock = _sd;	// back up old socket

	if(reset()) {
		
// 		_SetupMsg.setHeaderField(KEY_SESSION, _SetupSession);	// add a "Session" field to the setup message
		
		int retbytes = _SetupMsg.sendMessage(_pRtspSd);
		if( retbytes>0) {
#ifdef _DEBUG
			printf("sending recovery request from client:\n%s",_SetupMsg.toString().c_str());
#endif
			glog(ZQ::common::Log::L_DEBUG, "NOTIFY   [%06d]RtspClientConnection::OnRecover()  Sending recovery request with socket %d from client:\n%s",_PurchaseID, _sd, _SetupMsg.toString().c_str());
			return true;
		}
	}

	Sleep(_NSec);	//wait for a timeout span
	return false;
}

bool RtspClientConnection::OnDestroy(void)
{
	return cease();
}
