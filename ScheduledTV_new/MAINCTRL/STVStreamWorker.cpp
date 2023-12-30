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
// Name  : STVStreamWorker.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-7-14
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/MAINCTRL/STVStreamWorker.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 3     06-04-30 17:46 Bernie.zhao
// 
// 1     05-08-30 18:29 Bernie.zhao
// ===========================================================================


#include "ScheduleTV.h"
#include "STVStreamWorker.h"

// global STV Main Control object
extern ScheduleTV gSTV;

using namespace ZQ::common;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

STVStreamWorker::STVStreamWorker(DWORD ChnlID)
{
	_chnlID		= ChnlID;
	_scheNO		= "";
	_ieIndex	= -1;
	_bWorking	= true;
	_type		= WORKER_NONE;
	_delayTime	= 0;
	
	CoInitialize(NULL);
}

STVStreamWorker::~STVStreamWorker()
{
	CoUninitialize();
}

bool STVStreamWorker::setStartParam(const char* scheNo, int ieIndex)
{
	if(_type!=WORKER_NONE)
		return false;

	_type		= WORKER_START;
	_scheNO		= scheNo;
	_ieIndex	= ieIndex;

	return true;
}

bool STVStreamWorker::setStopParam()
{
	if(_type!=WORKER_NONE)
		return false;

	_type		= WORKER_STOP;
	_scheNO		= "";
	_ieIndex	= -1;

	return true;
}

int STVStreamWorker::run()
{
	try 
	{
		switch(_type)
		{
		case WORKER_START:
			
			glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::run()  A new worker thread started to work, to start schedule %s with IE %ld", _chnlID, _scheNO.c_str(), _ieIndex);
			
			if(!StartNewStream()) {
				glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::run()  Can not start %s with IE %ld",_chnlID,  _scheNO.c_str(), _ieIndex);
				// fail, increase penalty
				gSTV.getPlayListMan()->getTimer()->penaltyIncr();
				_bWorking = false;
				_delayTime = 0;
				return STVRTSPFAIL;
			}
			glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::run()  Schedule %s with IE %ld started",_chnlID, _scheNO.c_str(), _ieIndex);
		
			break;
		case WORKER_STOP:
	
			glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::run()  A new worker thread started to work",_chnlID);
			
			if(!ShutdownStream()) {
				glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::run()  Can not stop stream",_chnlID);
				_bWorking = false;
				_delayTime = 0;
				return STVRTSPFAIL;
			}
			glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::run()  Stream had been stopped",_chnlID);
			
			break;
		}
		
	}
	catch(std::exception &e) {
		glog(ZQ::common::Log::L_CRIT, "[%06d]STVStreamWorker::run()  Got std exception : %s", _chnlID, e.what());
				
		// exception occurs, change status
		STVChannel* pCh = gSTV.getPlayListMan()->queryCh(_chnlID);
		if(pCh == NULL)
		{
			GTRACEERR;
			glog(ZQ::common::Log::L_ERROR, "[%06d]STVStreamWorker::run()  Could not find channel %ld", _chnlID, _chnlID);
			_bWorking = false;
			_delayTime = 0;
			return -1;
		}

		// set channel status to starting
		pCh->setStatus(STVChannel::STAT_IDLE);
		pCh->clearStubs();
		
		_bWorking = false;
		_delayTime = 0;
		return -1;
	}
	catch(...) {
		glog(ZQ::common::Log::L_CRIT, "[%06d]STVStreamWorker::run()  Got unknown exception", _chnlID);
		
		// exception occurs, change status
		STVChannel* pCh =gSTV.getPlayListMan()->queryCh(_chnlID);
		if(pCh == NULL)
		{
			GTRACEERR;
			glog(ZQ::common::Log::L_ERROR, "[%06d]STVStreamWorker::run()  Could not find channel %ld", _chnlID, _chnlID);
			_bWorking = false;
			_delayTime = 0;
			return -1;
		}

		// set channel status to starting
		pCh->setStatus(STVChannel::STAT_IDLE);
		pCh->clearStubs();
		
		_bWorking = false;
		_delayTime = 0;
		return -1;

	}
	
	glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::run()  A worker thread died gracefully" , _chnlID);
	_bWorking = false;
	return STVSUCCESS;
}

bool STVStreamWorker::ShutdownStream()
{
	bool status;
	RtspResponse resBack;
	
	STVChannel* pCh = gSTV.getPlayListMan()->queryCh(_chnlID);
	if(pCh == NULL)
	{
		GTRACEERR;
		glog(ZQ::common::Log::L_ERROR, "[%06d]STVStreamWorker::ShutdownStream()  Could not find channel %ld", _chnlID, _chnlID);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// get last connection and TEARDOWN it
	RtspClient* lastConn = gSTV.getRtspMan()->getClientByPurchase(_chnlID);
	if(lastConn) 
	{	// found last connection

		if(lastConn->status()>=RtspClient::CLIENT_READY)
		{	// has called SETUP or PLAY
			glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::ShutdownStream()  Former client (sd=%d) should be tore down" , _chnlID, lastConn->sd());

			RtspRequest reqTeardown("TEARDOWN", "*");
			// tear down last stream
			status = lastConn->sendMsg(reqTeardown, resBack);
			if(!status) 
			{
				glog(ZQ::common::Log::L_WARNING, "[%06d]STVStreamWorker::ShutdownStream()  error on sending TEARDOWN message" , _chnlID);
				
				::Sleep(500);
				gSTV.getRtspMan()->removeClient(lastConn);
				pCh->setStatus(STVChannel::STAT_IDLE);
				pCh->clearStubs();

				return false;
			}
			
			glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::ShutdownStream()  TEARDOWN message sent" , _chnlID);
		}
			
		::Sleep(500);
		gSTV.getRtspMan()->removeClient(lastConn);
	}

	pCh->setStatus(STVChannel::STAT_IDLE);
	pCh->clearStubs();
	
	glog(ZQ::common::Log::L_INFO, "[%06d]STVStreamWorker::ShutdownStream()  Channel %d had been shut down" , _chnlID, _chnlID);
	return true;
}

bool STVStreamWorker::StartNewStream()
{
	bool status;
	RtspResponse resBack;
	char portbuff[8]={0};
	ultoa(STVStreamUnit::_dwRtspHostPort, portbuff, 10);

	// delay a while
	if(_delayTime>0)
		::Sleep(_delayTime);

	STVChannel* pCh = gSTV.getPlayListMan()->queryCh(_chnlID);
	if(pCh == NULL)
	{
		GTRACEERR;
		glog(ZQ::common::Log::L_ERROR, "[%06d]STVStreamWorker::StartNewStream()  Could not find channel %ld", _chnlID, _chnlID);
		return false;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// get last connection and TEARDOWN it
	RtspClient* lastConn = gSTV.getRtspMan()->getClientByPurchase(_chnlID);
	RtspClient* currConn = NULL;

	int nType = LISTTYPE_UNKNOWN;

	if(lastConn) 
	{	// found last connection

		if(lastConn->status()>=RtspClient::CLIENT_READY) 
		{	// has called SETUP or PLAY
			glog(ZQ::common::Log::L_DEBUG, L"[%06d]STVStreamWorker::StartNewStream()  Former client (sd=%d) should be tore down" ,_chnlID, lastConn->sd());
			
			RtspRequest reqTeardown("TEARDOWN", "*");
			// tear down last stream
			status = lastConn->sendMsg(reqTeardown, resBack);
			
			if(!status) 
			{
				glog(ZQ::common::Log::L_WARNING, "[%06d]STVStreamWorker::StartNewStream()  error on sending TEARDOWN message" ,_chnlID);
				
				::Sleep(500);				
				gSTV.getRtspMan()->removeClient(lastConn);
				pCh->setStatus(STVChannel::STAT_IDLE);
				pCh->clearStubs();
				
				return false;
			}
			else 
			{
				glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::StartNewStream()  TEARDOWN message sent" ,_chnlID);
			}
		}
		
		// reset connection socket, no need to create new connection
		::Sleep(500);		// sleep some time for ISRM to release the connection
		currConn = lastConn;
		SOCKET oldsd = lastConn->sd();
		status = currConn->reset();
		if(!status)
		{
			glog(ZQ::common::Log::L_WARNING, "[%06d]STVStreamWorker::StartNewStream()  client (sd=%d) reset failed" ,_chnlID, lastConn->sd());
			::Sleep(500);
			gSTV.getRtspMan()->removeClient(lastConn);
			pCh->setStatus(STVChannel::STAT_IDLE);
			pCh->clearStubs();
			return false;
		}
		else
		{
			glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::StartNewStream()  client (sd=%d) reset to (sd=%d)" ,_chnlID, oldsd, lastConn->sd());
		}
	}
	else 
	{	// no last connection, create new connection
		glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::StartNewStream()  No former client." ,_chnlID);
		currConn = gSTV.getRtspMan()->createClient(_chnlID, STVStreamUnit::_strRtspHostIP.c_str(), STVStreamUnit::_dwRtspHostPort, STVStreamUnit::_dwRtspNsec);
		if(currConn==NULL) 
		{
			glog(ZQ::common::Log::L_WARNING, "[%06d]STVStreamWorker::StartNewStream()  Can not create RTSP client on %s:%d" ,_chnlID, STVStreamUnit::_strRtspHostIP.c_str(), STVStreamUnit::_dwRtspHostPort);
			pCh->setStatus(STVChannel::STAT_IDLE);
			pCh->clearStubs();
			return false;
		}
		
		glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::StartNewStream()  new client (sd=%d) created" ,_chnlID, currConn->sd());
	}
	
	//////////////////////////////////////////////////////////////////////////
	// send SETUP and PLAY
	std::string desIp, desPort, desNode;

	RtspRequest reqSetup("SETUP",STVStreamUnit::_strRtspURL.c_str());
	resBack.clearMessage();
				
	// get destination info
	desIp		= pCh->getIPAddr();
	desPort		= pCh->getIPPort();
	desNode		= pCh->getNodegroup();

	// make up Transport header
	RtspMsgHeader transport(KEY_TRANSPORT);
	std::string multimac = RtspClient::getMulticastMac(desIp.c_str());
	transport.setSubHeaderField(" MP2T/AVP/UDP","");
	transport.setSubHeaderField(KEY_UNICAST,"");
	transport.setSubHeaderField(KEY_DESTINATION,desIp.c_str());
	transport.setSubHeaderField(KEY_CLIENTPORT,desPort.c_str());
	transport.setSubHeaderField(KEY_CLIENTMAC,multimac.c_str());

	// make up SeaChange-Server-Data header
	RtspMsgHeader serverdata(KEY_SEACHANGESERVERDATA);
	serverdata.setSubHeaderField(KEY_NODEGROUPID,desNode.c_str());
	serverdata.setSubHeaderField(KEY_DEVICEID,multimac.c_str());
	
	// make up SeaChange-Mod-Data header
	RtspMsgHeader moddata(KEY_SEACHANGEMODDATA);
	char purstr[16]={0};
	sprintf(purstr, "%010d", _chnlID);
	moddata.setSubHeaderField(KEY_PURCHASEID, purstr);

	// form the whole SETUP request
	reqSetup.setHeaderField(KEY_SEACHANGEVERSION,"1");
	reqSetup.setHeaderField(transport.getName().c_str(), transport.toString().c_str());
	reqSetup.setHeaderField(KEY_SEACHANGEMAYNOTIFY," ");
	reqSetup.setHeaderField(serverdata.getName().c_str(), serverdata.toString().c_str());
	reqSetup.setHeaderField(moddata.getName().c_str(), moddata.toString().c_str());

	// send SETUP request & PLAY request
	status = currConn->sendMsg(reqSetup, resBack);
	if(!status) 
	{
		glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::StartNewStream()  error on sending SETUP message" ,_chnlID);
			
		::Sleep(500);
		gSTV.getRtspMan()->removeClient(currConn);
		pCh->setStatus(STVChannel::STAT_IDLE);
		pCh->clearStubs();
		return false;						
	}

	glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::StartNewStream()  SETUP message sent" ,_chnlID);
	
//	::Sleep(200);
	
	resBack.clearMessage();
	
	// form the whole PLAY request
	RtspRequest reqPlay("PLAY","*");
	
	// calculate npt if necessary (if npt<=5, we do not care it)
	char nptBuff[32]={0};
	DWORD dwNpt = pCh->getNpt();
	ultoa(dwNpt, nptBuff, 10);
	strcat(nptBuff, ".0-");
	
//	// pre-set status to playing, this is to prevent the ISS AEtransit callback too fast
//	// sometimes the callback gets invoked before RTSP received "200 OK"
//	// in case this happens, we should not ignore this callback
//	pCh->setStatus(STVChannel::STAT_PLAYING);

	RtspMsgHeader nptHeader(KEY_RANGE);
	nptHeader.setSubHeaderField(KEY_RANGE_NPT,nptBuff);
	reqPlay.setHeaderField(nptHeader.getName().c_str(), nptHeader.toString().c_str());
	reqPlay.setHeaderField(KEY_SCALE,"1.0");
	status = currConn->sendMsg(reqPlay, resBack);
	if(!status) 
	{
		glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::StartNewStream()  error on sending PLAY message" ,_chnlID);

		if(currConn->status()>=RtspClient::CLIENT_READY)
		{
			RtspRequest reqDown("TEARDOWN","*");
			resBack.clearMessage();
			currConn->sendMsg(reqDown, resBack);
		}
		
		::Sleep(500);
		gSTV.getRtspMan()->removeClient(currConn);
		pCh->setStatus(STVChannel::STAT_IDLE);
		pCh->clearStubs();
		return false;
	}

	glog(ZQ::common::Log::L_DEBUG, "[%06d]STVStreamWorker::StartNewStream()  PLAY message sent" ,_chnlID);

	pCh->setStatus(STVChannel::STAT_PLAYING);
	return true;
}