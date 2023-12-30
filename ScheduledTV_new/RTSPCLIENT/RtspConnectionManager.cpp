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
// ===========================================================================
//#include "StdAfx.h"
#include ".\rtspconnectionmanager.h"

///@mainpage Scheduled STV: RtspClient Module
///@section Background
/// The Scheduled TV(STV) system is intended to deliver uninterrupted linear scheduled streams in ITV VOD configurations. It extends and enhances the ITV platform by delivering the capabilities and functions most commonly found in Near Video On Demand (NVOD) systems and, provides an enhancement of the current SeaChange VOD Barker system. A schedule in STV system, represented by a series of streaming assets, is logically defined as a playlist. A single playlist contains asset elements that being delivered to subscribers in a certain span of time. The time duration varies from hours to months, basically depending on practical server requirement and customers' plan. Dozens of playlists constitute a single channel, while the STV service maintains control of different channels. This structure gives birth to a manager-functioned logical unit for all playlists, the playlist management. 
///@section purpose
/// The purpose of RtspClient Module is to provide Rtsp Stream control operations for STV corresponding fuctions.  RtspClient is responsible for stream setup, play, pause, forward front, forward rear, query and terminate operations.
// listening sink over signal event handle
RtspConnectionManager::RtspConnectionManager(int maxdaemon, bool trace/* =true */)
{
	_IsRunning	=	false;
	_maxDaemonNum = maxdaemon;
	_trace = trace;
	_daemons.clear();
	
	glog(ZQ::common::Log::L_DEBUG, "RtspConnectionManager::RtspConnectionManager()  New RTSP manager created");
}

RtspConnectionManager::~RtspConnectionManager(void)
{
	if(_IsRunning)
		terminate();
	
	glog(ZQ::common::Log::L_DEBUG, L"RtspConnectionManager::RtspConnectionManager()  RTSP manager deleted");
}

RtspClient* RtspConnectionManager::createClient(DWORD purchaseID, const char* hostname, int hostport/* =RTSP_DEFAULT_PORT */, int nsec/* =RTSP_DEFAULT_NSEC */, bool createnew/* =false */)
{
	RtspClient*	pClient = NULL;

	ZQ::common::MutexGuard	tmpGd(_lkDaemons);
	
	RtspDaemon*	pDaemon = allocateDaemon();

	if(NULL == pDaemon)
	{
		glog(ZQ::common::Log::L_ERROR, "RtspConnectionManager::createClient()  Failed to allocate RtspDaemon object for new RtspClient");
		return pClient;
	}
	
	// create client
	pClient = pDaemon->createClient(purchaseID, createnew);

	if(NULL == pClient)
	{
		glog(ZQ::common::Log::L_ERROR, "RtspConnectionManager::createClient()  Failed to create new RtspClient object");
		return pClient;
	}

	// set trace flag
	// TODO: this value should comes from registry
	pClient->traceFlag() = _trace;

	// connect client
	if(!pClient->open(hostname, hostport, nsec))
	{
		glog(ZQ::common::Log::L_ERROR, "RtspConnectionManager::createClient()  Failed to connect RtspClient object");
		pDaemon->destroyClient(pClient);
		pClient = NULL;
		return pClient;
	}
	
	return pClient;
}

bool RtspConnectionManager::removeClient(RtspClient* exclient)
{
	if(!exclient)
		return false;

	ZQ::common::MutexGuard	tmpGd(_lkDaemons);

	glog(ZQ::common::Log::L_DEBUG, "RtspConnectionManager::removeClient()  Client (sd=%d) is about to be deleted.", exclient->sd());
	std::vector<RtspDaemon*>::iterator iter = NULL;
	for(iter = _daemons.begin(); iter!=_daemons.end(); iter++)
	{
		if((*iter)->destroyClient(exclient))
		{	// found
			return true;
		}
	}

	return false;
}

RtspClient* RtspConnectionManager::getClientByPurchase(DWORD pid)
{
	RtspClient*	pClient = NULL;

	ZQ::common::MutexGuard	tmpGd(_lkDaemons);

	std::vector<RtspDaemon*>::const_iterator iter = NULL;
	for(iter = _daemons.begin(); iter!=_daemons.end(); iter++)
	{
		if(pClient=(*iter)->queryClientByPurchase(pid))
		{	// found
			return pClient;
		}
	}

	return NULL;
}

RtspClient* RtspConnectionManager::getClientBySocket(SOCKET fd)
{
	RtspClient*	pClient = NULL;

	ZQ::common::MutexGuard	tmpGd(_lkDaemons);

	std::vector<RtspDaemon*>::const_iterator iter = NULL;
	for(iter = _daemons.begin(); iter!=_daemons.end(); iter++)
	{
		if(pClient=(*iter)->queryClientBySocket(fd))
		{	// found
			return pClient;
		}
	}

	return NULL;
}

RtspDaemon*	RtspConnectionManager::allocateDaemon()
{
	RtspDaemon*	pDaemon = NULL;
	
	ZQ::common::MutexGuard	tmpGd(_lkDaemons);

	std::vector<RtspDaemon*>::iterator iter = NULL;
	int	minNum = -1;
	for(iter = _daemons.begin(); iter!=_daemons.end(); iter++)
	{
		int currNum = (*iter)->getClientNum();

		if(minNum==-1)	
		{	// init min value
			minNum = currNum;
			pDaemon = (*iter);
		}

		if(currNum == 0)
		{	// no client bound, choose this of course
			return (*iter);
		}
		else if(currNum < minNum)
		{	// record this daemon
			minNum	= currNum;
			pDaemon = (*iter);
		}
	}

	if(_daemons.size()<_maxDaemonNum)
	{	// still have seat for new daemon, create a new one
		int id = _daemons.size();
		pDaemon = new RtspDaemon(id);
		_daemons.push_back(pDaemon);
		pDaemon->start();
		return pDaemon;
	}
	else
	{	// no more seat, return the daemon with least client
		return pDaemon;
	}
}

bool RtspConnectionManager::start(void)
{
	bool retval = true;

	_IsRunning	=   true;

	glog(ZQ::common::Log::L_INFO, "RtspConnectionManager::start()  RTSP Connection Manager started");
	
	return retval;
}

bool RtspConnectionManager::terminate(void)
{
	ZQ::common::MutexGuard	tmpGd(_lkDaemons);

	// first, stop each daemon
	std::vector<RtspDaemon*>::iterator iter = NULL;
	for(iter = _daemons.begin(); iter!=_daemons.end(); iter++)
	{
		(*iter)->signalStop();
		(*iter)->waitHandle(1000);
	}

	// second, delete each daemon object
	while(!_daemons.empty())
	{
		RtspDaemon* pEx = _daemons.back();
		if(pEx)
		{
			delete pEx;
			pEx = NULL;
		}
		_daemons.pop_back();
	}

	_IsRunning	=	false;
	
	glog(ZQ::common::Log::L_DEBUG, "RtspConnectionManager::terminate()  RTSP Connection Manager terminated");
	
	return true;
}
