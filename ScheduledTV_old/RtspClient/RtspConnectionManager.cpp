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
// Desc  : rtsp connection control class implementation
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/RtspClient/RtspConnectionManager.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 24    05-06-27 18:05 Bernie.zhao
// 
// 23    05-04-19 21:18 Bernie.zhao
// autobuild modification
// 
// 22    05-03-24 14:51 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 21    05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// 
// 20    04-12-06 20:55 Bernie.zhao
// socket reset improved
// 
// 19    04-11-30 10:24 Bernie.zhao
// Nov30, after resolving ISSStreamTerminate problem
// 
// 18    04-11-23 10:02 Bernie.zhao
// 
// 17    04-11-03 19:59 Bernie.zhao
// Oct/3 Before Service Release
// 
// 16    04-10-26 16:08 Bernie.zhao
// 0.1.6 Oct/26
// 
// 15    04-10-22 17:41 Bernie.zhao
// NT4 connect() problem fixed
// 
// 14    04-10-22 16:37 Bernie.zhao
// 
// 13    04-10-21 15:29 Bernie.zhao
// 
// 12    04-10-19 17:20 Bernie.zhao
// mem leak?
// 
// 11    04-10-18 18:46 Bernie.zhao
// 
// 10    04-10-18 10:33 Bernie.zhao
// test 1 ok
// 
// 9     04-10-15 16:14 Bernie.zhao
// 
// 8     04-10-14 14:00 Bernie.zhao
// 
// 7     04-10-12 9:55 Bernie.zhao
// pragma warning 4786
// 
// 6     04-10-07 16:52 Bernie.zhao
// added connection query interface with purchase id
// 
// 5     04-10-07 16:01 Bernie.zhao
// fixed TEARDOWN problem
// 
// 4     04-09-21 11:14 Bernie.zhao
// get rid of hash_map
// 
// 3     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// 2     04-09-09 11:46 Bernie.zhao
// event handle bug fixed
// 
// ===========================================================================
//#include "StdAfx.h"
#include ".\rtspconnectionmanager.h"

///@mainpage Scheduled STV: RtspClient Module
///@section Background
/// The Scheduled TV(STV) system is intended to deliver uninterrupted linear scheduled streams in ITV VOD configurations. It extends and enhances the ITV platform by delivering the capabilities and functions most commonly found in Near Video On Demand (NVOD) systems and, provides an enhancement of the current SeaChange VOD Barker system. A schedule in STV system, represented by a series of streaming assets, is logically defined as a playlist. A single playlist contains asset elements that being delivered to subscribers in a certain span of time. The time duration varies from hours to months, basically depending on practical server requirement and customers' plan. Dozens of playlists constitute a single channel, while the STV service maintains control of different channels. This structure gives birth to a manager-functioned logical unit for all playlists, the playlist management. 
///@section purpose
/// The purpose of RtspClient Module is to provide Rtsp Stream control operations for STV corresponding fuctions.  RtspClient is responsible for stream setup, play, pause, forward front, forward rear, query and terminate operations.
// listening sink over signal event handle
RtspConnectionManager::RtspConnectionManager(void)
{
	_ConnNum	=	0;
	_HeartbeatSec	= DEFAULT_KEEPALIVESEC;
	_IsRunning	=	false;

	_Listener	= new RtspRecvSink(this);
	
	_ConnPool.clear();
	_SockPool.clear();
	glog(ZQ::common::Log::L_DEBUG, L"SUCCESS  RtspConnectionManager::RtspConnectionManager()  New RTSP manager created");
}

RtspConnectionManager::~RtspConnectionManager(void)
{
	if(_IsRunning)
		terminate();
	
	if(_Listener!=NULL)
		delete _Listener;
}

bool RtspConnectionManager::eraseSock( SOCKET sd)
{
	bool ret = false;

	MutexGuard	tmpGd(_ConnMutex);
	
	for(size_t i = 0; i< _SockPool.size(); i++) {
		if( _SockPool.at(i) == sd) {
			_SockPool.erase(_SockPool.begin()+i);
			ret = true;
		}
	}
	
	return ret;
}

bool RtspConnectionManager::updateSock(RtspClientConnection* pConn )
{
	bool ret = false;
	
	MutexGuard	tmpGd(_ConnMutex);

	for(size_t i = 0; i< _ConnPool.size(); i++) {
		if( _ConnPool.at(i) == pConn) {
			_SockPool.at(i) = pConn->getSocket();
			ret = true;
		}
	}
	
	return ret;
}

RtspClientConnection* RtspConnectionManager::createConn(const char* hostname, const char* hostport, DWORD purchaseID, int nsec /* = DEFAULT_NSEC */)
{
	RtspClientConnection*	pConn = new RtspClientConnection(this);
	pConn->setPurchase(purchaseID);

	// create a socket
	SOCKET sd = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
#ifdef _DEBUG
		printf("Socket %d created\n", sd);
#endif
	
	if(sd != INVALID_SOCKET) {	// socket succeeded
		try{
			if(!pConn ->init(sd, hostname, hostport, nsec))  {	// connection start
				delete pConn;
#ifdef _DEBUG
				printf("Can not init RtspClient Connection\n");
#endif
				return NULL;
			}
		}
		catch( ZQ::common::Exception excp)
		{
			throw	excp;
			return NULL;
		}

		MutexGuard	tmpGd(_ConnMutex);
		_ConnPool.push_back( pConn);	// add connection into pool
		_SockPool.push_back( sd);		// add socket into pool
		
		_ConnNum++;
		
		
		glog(ZQ::common::Log::L_DEBUG, "SUCCESS  RtspConnectionManager::createConn()  New socket created: %d",sd);
		return pConn;

	}
	glog(ZQ::common::Log::L_ERROR, "FAILURE  RtspConnectionManager::createConn()  Invalid socket: %d",sd);
	return NULL;
}

std::string RtspConnectionManager::createConnEx(RtspRequest setupmsg, const char* hostname, const char* hostport, DWORD purchaseID, int nsec  /*= DEFAULT_NSEC*/)
{
	std::string retstr = "";
	RtspClientConnection* retval;

	try{
		retval = createConn(hostname, hostport, purchaseID, nsec);
	}
	catch( ZQ::common::Exception excp)
	{
		throw	excp;
		return  retstr;
	}

	if(retval!=NULL) {	// create succeeded
		retstr = retval->getSession();	// get session-id
	}
	return retstr;
}

bool RtspConnectionManager::removeConn(std::string session_id)
{
	RtspClientConnection* pConn = NULL;
	MutexGuard	tmpGd(_ConnMutex);

	for(size_t i = 0; i< _ConnPool.size(); i++) {
		if( session_id == _ConnPool.at(i)->getSession()) {
			pConn = _ConnPool.at(i);	// session exist in the connection, get the connection
			_ConnPool.erase(_ConnPool.begin()+i);	// erase this connection from pool
			_SockPool.erase(_SockPool.begin()+i);	// erase socket from pool
			_ConnNum--;
			break;
		}
	}

	if( pConn!=NULL ) {
		// stop stream
		glog(ZQ::common::Log::L_DEBUG, "NOTIFY   RtspConnectionManager::removeConn()  RTSP connection with socket %d, state %d should be killed now", pConn->_sd, pConn->getState());
		int currState = pConn->getState();
		if((currState!=RTSP_DISCONNECT)&&(currState!=RTSP_IDLE)) {
			RtspRequest teardownReq("TEARDOWN", "*");
			RtspResponse backRes;
			bool tearret=pConn->sendMSG(teardownReq, backRes);
		}
		
		pConn->cease();
		delete pConn;
		pConn = NULL;
		
		return true;
	}
	return false;
}

RtspClientConnection* RtspConnectionManager::getConnBySession(std::string session_id)
{
	MutexGuard	tmpGd(_ConnMutex);

	for(size_t i = 0; i< _ConnPool.size(); i++) {
		if( session_id == _ConnPool.at(i)->getSession()) {
			return _ConnPool.at(i);	// session exist in the connection, return the connection
		}
	}

	return NULL;
}

RtspClientConnection* RtspConnectionManager::getConnByPurchase(DWORD pid)
{
	MutexGuard	tmpGd(_ConnMutex);

	for(size_t i =0; i< _ConnPool.size(); i++) {
		if( pid == _ConnPool[i]->getPurchase()) {
			return _ConnPool[i];	// purchase exist in the connection
		}
	}

	return NULL;
}

RtspClientConnection* RtspConnectionManager::getConnBySocket(SOCKET sd)
{
	MutexGuard	tmpGd(_ConnMutex);
	
	for(size_t i = 0; i< _SockPool.size(); i++) {
		if( _SockPool.at(i) == sd) {
			return _ConnPool.at(i);	// connection socket description match, return the connection
		}
	}

	return NULL;
}

bool RtspConnectionManager::OnAnnounce(SOCKET sd, RtspRequest reqmsg)
{
	RtspClientConnection* pConn = getConnBySocket(sd);
	if(!pConn) {
		return FALSE;
	}
	return (pConn->OnAnnounce(reqmsg));
}

bool RtspConnectionManager::OnResponse(SOCKET sd, RtspResponse resmsg)
{
	RtspClientConnection* pConn = getConnBySocket(sd);
	if(!pConn) {
		return FALSE;
	}
	return (pConn->OnResponse(resmsg));
}

bool RtspConnectionManager::OnRequest(SOCKET sd)
{
	RtspClientConnection* pConn = getConnBySocket(sd);
	if(!pConn) {
		return FALSE;
	}
	return (pConn->OnRequest());
}

bool RtspConnectionManager::OnRecover(SOCKET sd, int errnum)
{
	bool ret = false;

	RtspClientConnection* pConn = getConnBySocket(sd);
	
	// retry for several times
	for( int i=0 ;i<_HeartbeatSec*1000/DEFAULT_NSEC; i++) {
			ret = pConn->OnRecover(errnum);
			if(ret)
				break;
	}
	return ret;
}

bool RtspConnectionManager::OnDestroy(SOCKET sd)
{
	RtspClientConnection* pConn = getConnBySocket(sd);
	glog(ZQ::common::Log::L_DEBUG, "NOTIFY   RtspConnectionManager::OnDestroy()  RTSP connection with socket %d should be stopped now", sd);
	
	if(!pConn) {
		return FALSE;
	}
	else {
		return (pConn->cease());
	}
}

bool RtspConnectionManager::start(void)
{
	bool retval = true;

	_IsRunning	=   true;

	retval = _Listener->start();	// start listening thread

	glog(ZQ::common::Log::L_INFO, "NOTIFY   RtspConnectionManager::start()  RTSP Connection Manager started");
	return retval;
}

bool RtspConnectionManager::terminate(void)
{
	while(!_ConnPool.empty()) {
		RtspClientConnection* pConn = _ConnPool.front();
		// tear down all stream
		RtspRequest teardownReq("TEARDOWN", "*");
		RtspResponse backRes;
		DWORD purID = pConn->getPurchase();
		bool tearret=pConn->sendMSG(teardownReq, backRes);
		if(pConn=getConnByPurchase(purID))
			removeConn(pConn->getSession());
	}
	glog(ZQ::common::Log::L_DEBUG, "NOTIFY   RtspConnectionManager::terminate()  All connection removed");
	_ConnPool.clear();
	_SockPool.clear();

	_Listener->OnSetListenOver();	// signal listening over
	if(_Listener->isRunning()) {
		bool listenRet=_Listener->wait(10000);
		glog(ZQ::common::Log::L_DEBUG, "NOTIFY   RtspConnectionManager::terminate()  RTSP Receive Sink closed with code %d", listenRet);
	}
	else {
		glog(ZQ::common::Log::L_DEBUG, "NOTIFY   RtspConnectionManager::terminate()  RTSP Receive Sink already closed");
	}
	_ConnNum	=	0;
	_HeartbeatSec	= DEFAULT_KEEPALIVESEC;
	_IsRunning	=	false;

	
	glog(ZQ::common::Log::L_INFO, "NOTIFY   RtspConnectionManager::terminate()  RTSP Connection Manager closed");
	return true;
}
