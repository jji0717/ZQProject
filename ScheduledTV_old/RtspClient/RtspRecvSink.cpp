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
// Desc  : rtsp receive sink thread implementation
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/RtspClient/RtspRecvSink.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 21    05-07-29 16:15 Bernie.zhao
// 
// 20    05-06-27 18:07 Bernie.zhao
// fixed interface error with SM when creating new STV list
// 
// 19    05-06-09 10:16 Bernie.zhao
// 
// 18    05-03-24 14:52 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 17    05-03-08 16:54 Bernie.zhao
// upon version 0.4.0.0
// 
// 16    04-12-06 20:55 Bernie.zhao
// socket reset improved
// 
// 15    04-11-23 10:02 Bernie.zhao
// 
// 14    04-11-03 19:59 Bernie.zhao
// Oct/3 Before Service Release
// 
// 13    04-10-28 18:19 Bernie.zhao
// before trip
// 
// 12    04-10-24 11:43 Bernie.zhao
// before 24/Oct
// 
// 11    04-10-22 16:38 Bernie.zhao
// 
// 10    04-10-19 18:15 Bernie.zhao
// 
// 9     04-10-19 17:20 Bernie.zhao
// mem leak?
// 
// 8     04-10-18 18:46 Bernie.zhao
// 
// 7     04-10-18 10:33 Bernie.zhao
// test 1 ok
// 
// 6     04-10-14 13:54 Bernie.zhao
// enable listening frequency configurable
// 
// 5     04-10-14 11:25 Bernie.zhao
// 
// 4     04-10-07 16:01 Bernie.zhao
// fixed TEARDOWN problem
// 
// 3     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// 2     04-09-14 17:32 Bernie.zhao
// 
// ===========================================================================
//#include "StdAfx.h"
#include ".\rtsprecvsink.h"

RtspRecvSink::RtspRecvSink(RtspConnectionManager* manager, int freq/*=MAX_SELECTTIME*/) 
{
	_man		= manager;
	_Freq		= freq;
	_hOverOrSend[0]	= ::CreateEvent(0,0,0,0);
	_hOverOrSend[1]	= ::CreateEvent(0,TRUE,0,0);
}

RtspRecvSink::~RtspRecvSink(void)
{
	::CloseHandle(_hOverOrSend[0]);
	::CloseHandle(_hOverOrSend[1]);
}

int RtspRecvSink::sockSetUpdate()
{
	int socknum = _man->_ConnNum;
	int maxsock = 0;
	
	if(socknum == 0) {
		return maxsock;
		::Sleep(_Freq);
	}

	FD_ZERO( &_sockSet);
	_man->_ConnMutex.enter();
	for(int j = 0; j< _man->_SockPool.size(); j++) {
		SOCKET tmpSock = _man->_SockPool[j];
		
		if(tmpSock==INVALID_SOCKET)
			continue;
		
		FD_SET( tmpSock, &_sockSet);	// add socket to set
		
		if( (int)tmpSock > maxsock)
			maxsock = (int)tmpSock;
	}
	_man->_ConnMutex.leave();

	return maxsock;
}

int RtspRecvSink::run()
{
	int socknum = _man->_ConnNum;
	int retval, status;
	struct timeval	tv;
	
	int	maxsock;

	int selecttimes;
	int i;

	::ResetEvent(_hOverOrSend[1]);
	
	for(;;)
	{
		try
		{

			if(_man == NULL || _man->_IsRunning == false) {	// manager is not running, end the thread
				return 0;
			}

			for(i=0; ; i++) 
			{	// separate select time span into pieces
			
				//////////////////////////////////////////////////////////////////////////
				// handle request if necessary

				status = ::WaitForMultipleObjects(2, _hOverOrSend,FALSE, 300);
				if(status == WAIT_OBJECT_0) {	// listening over event signaled, end thread
					return 0;
				}
				else if( status == WAIT_OBJECT_0+1 ) {
					int retreq = handleRequest();
					::ResetEvent(_hOverOrSend[1]);
				}
				else if( status!= WAIT_TIMEOUT)
				{
					status = GetLastError();
					return 1;
				}

				//////////////////////////////////////////////////////////////////////////
				// handle response if necessary

				// update socket set
				maxsock = sockSetUpdate();
				if(maxsock==0)
				{
					::Sleep(_Freq);
					continue;
				}
			
				// update keepalive second
				selecttimes = _man->getHeartbeat()*DEFAULT_KEEPALIVEFAC/_Freq;
				tv.tv_sec	= _Freq /1000;
				tv.tv_usec	= _Freq %1000*1000;

				//////////////////////////////////////////////////////////////////////////
				// monitor if any response got
				retval = select(maxsock, &_sockSet, NULL, NULL, &tv);
				if( retval > 0 ) {	// something notified
//					glog(ZQ::common::Log::L_DEBUG, "NOTIFY   Something happens in socket set with %d socket in", socknum);
					handleResponse( &_sockSet);	
					
				}
				else if( retval<0 )
				{
					status = GetLastError();
					glog(ZQ::common::Log::L_DEBUG, "ERROR    select error- %d", status);
				}


				// total timeout reached
				if(i>=(selecttimes-1)) {	
//					glog(ZQ::common::Log::L_DEBUG, "NOTIFY   Should send keep alive after %d seconds", i*_Freq/1000);
					handleHeartbeat();
					break;
				}
			}
		}
		catch(std::exception &e)
		{
			glog(ZQ::common::Log::L_CRIT, "FAILURE  RtspRecvSink::run()  An std exception occurs, with error string:%s", e.what());
		}
		catch(ZQ::common::Exception excep) {
			glog(ZQ::common::Log::L_CRIT, "FAILURE  RtspRecvSink::run()  An exception occurs, with error string:%s", excep.getString());
		}
		catch(...) {
			glog(ZQ::common::Log::L_CRIT, "FAILURE  RtspRecvSink::run()  An unknown exception occurs");
		}

	}
	return -1;
}

int RtspRecvSink::handleResponse(fd_set* socketset)
{
	int retval =0;
	char buff[MAX_BUFFER_SIZE];

	for( int i =0; i< _man->_ConnPool.size(); i++) {
		SOCKET tmpsd = _man->_SockPool.at(i);
		if( FD_ISSET(tmpsd, socketset)) {	// this socket is in set
			retval++;
#ifdef _DEBUG
			printf("Handling data on socket %d\n",tmpsd);
#endif
//			printf("Handling data on socket %d\n",tmpsd);
			memset(buff, NULL, MAX_BUFFER_SIZE);
			RtspMessage	 recvmsg;
			SOCKET	currsd = _man->_SockPool.at(i);
			RtspClientConnection* pConn = _man->_ConnPool.at(i);

			// receive data
			int bytes = pConn->_pRtspSd->recvN(buff, MAX_BUFFER_SIZE);
			char* st = buff;
			if(bytes>0) {
//				glog(ZQ::common::Log::L_DEBUG, "SUCCESS  RtspRecvSink::run()  WSA socket %d got data:\n %s\n", currsd, buff);
				while(st && *st>0) {	// maybe more than one message received once, so handle them sequentially
//					printf("in handleData() 1");
					recvmsg.clearMessage();
//					printf("in handleData() 2");
					RtspMessage::parseNewMessage(st, recvmsg);
//					printf("in handleData() 3");
					int msgcate = recvmsg.isRequOrResp();
//					printf("in handleData() 4");

					
					if(msgcate == RTSP_RESPONSE_MSG){	// is a response
						RtspResponse	response(recvmsg);
						_man->OnResponse(currsd,response);
					}
					else if(msgcate == RTSP_REQUEST_MSG) {	// is a request
						RtspRequest		request(recvmsg);
						if(request.getCommandtype() == "ANNOUNCE") {	// is an ANNOUNCE request
							_man->OnAnnounce(currsd,request);
						}
					}

					
					st = splitNextPart(st); // move position to the start of next message
				}
			}
			else {
				int errnum = WSAGetLastError();
				glog(ZQ::common::Log::L_DEBUG, "FAILURE  RtspRecvSink::handleData()  WSA socket %d has some error with code %d", currsd, errnum);
				if( errnum == 10035)	// no data for recv in non-blocking mode, it's non-fatal error
					continue;
				if( errnum == 10050 || errnum==10053 || errnum == 10054 || errnum ==10060) {	// connection failure
					// 10050 network down, 10054 socket connection failure, 10060 connection timeout
					// Usually it always shows 10054 for almost all error
					if ( pConn->_SentMsg.getCommandtype() != "TEARDOWN")	{
//						if(!_man->OnRecover( currsd,errnum))	// last request is not TEARDOWN, so recover the connection
						_man->OnDestroy(currsd);	// recover failed, so destroy the connection
						pConn->OnSetGotResponse();

					}
					else {	// last request is TEARDOWN, so probably the request is successful 
						// and server closes the connection
						_man->OnDestroy(currsd);
						pConn->OnSetGotResponse();
					}
					
				}
			}
		
		}
	}
	return retval;
}

int RtspRecvSink::handleHeartbeat()
{
	int retval =0;
	for( int i =0; i< _man->_ConnPool.size(); i++) {
		RtspClientConnection* pConn = _man->_ConnPool.at(i);
		if(pConn->getState()==RTSP_READY || pConn->getState()==RTSP_PLAYING) {
			pConn->sendKeepAlive();
			retval++;
		}
	}
	return retval;
}

int RtspRecvSink::handleRequest()
{
	int num = _WaitingList.size();
	int retval = 0;
	for(int i=0; i<num; i++)
	{
		SOCKET sd = waitingListPop();
		if(sd<=0)
			continue;
		if(_man->OnRequest(sd))
		{
			retval++;
		}
	}
	return retval;
}

char* RtspRecvSink::splitNextPart(char* bytes)
{
	char* pos=bytes;
	while(!(*pos==CH_CR && *(pos+1)==CH_LF && *(pos+2)==CH_CR && *(pos+3)==CH_LF)) {
		if(*pos == EOF)
			return NULL;
		pos++;
	}
	pos+=4;

	return pos;
}