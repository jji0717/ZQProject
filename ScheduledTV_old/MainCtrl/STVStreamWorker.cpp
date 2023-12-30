// ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id: STVStreamUnit.h$
// Branch: $Name:  $
// Author: Bernie(Tianbin) Zhao
// Desc  : implementation of STV stream bring up worker thread
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/MainCtrl/STVStreamWorker.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 20    05-06-09 10:16 Bernie.zhao
// 
// 19    05-03-25 14:21 Bernie.zhao
// 
// 18    05-03-24 14:53 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 17    05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// 
// 16    05-01-05 17:16 Bernie.zhao
// modified to support STV status feedback
// 
// 15    04-12-23 17:14 Ken.qian
// 
// 14    04-12-16 14:55 Bernie.zhao
// 
// 13    04-11-30 10:24 Bernie.zhao
// Nov30, after resolving ISSStreamTerminate problem
// 
// 12    04-11-23 10:01 Bernie.zhao
// 
// 11    04-11-03 19:59 Bernie.zhao
// Oct/3 Before Service Release
// 
// 10    04-10-26 16:09 Bernie.zhao
// 0.1.6 Oct/26
// 
// 9     04-10-24 19:04 Bernie.zhao
// end of 2004/Oct/24
// 
// 8     04-10-24 11:43 Bernie.zhao
// before 24/Oct
// 
// 7     04-10-21 15:29 Bernie.zhao
// 
// 6     04-10-19 17:21 Bernie.zhao
// for QA release
// 
// 5     04-10-18 10:33 Bernie.zhao
// test 1 ok
// 
// 4     04-10-15 16:36 Bernie.zhao
// 
// 3     04-10-14 14:56 Bernie.zhao
// 
// 2     04-10-11 15:19 Bernie.zhao
// 
// 1     04-10-07 9:09 Bernie.zhao
// ===========================================================================

#include "ScheduleTV.h"
#include "STVStreamWorker.h"

// global STV Main Control object
extern ScheduleTV gSTV;

using namespace ZQ::common;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

STVStreamWorker::STVStreamWorker(STVStreamUnit* boss, STVPlaylist* pPL, DWORD ChnlID, const char* RtspHostIP, const char* RtspURL, DWORD RtspPort, DWORD RtspNsec, bool isStart)
{
	_Boss		= boss;
	_pPlaylist	= pPL;
	_isStart	= isStart;
	_PurID		= ChnlID;
	strcpy(_szRtspHostIP, RtspHostIP);
	strcpy(_szRtspURL, RtspURL);
	_dwRtspHostPort = RtspPort;
	_dwRtspNsec = RtspNsec;
	CoInitialize(NULL);

	_bWorking = TRUE;
}

STVStreamWorker::~STVStreamWorker()
{
	CoUninitialize();
}

int STVStreamWorker::run()
{
	try 
	{
		if(_isStart) 
		{
			glog(ZQ::common::Log::L_DEBUG, L"PENDING  [%06d]STVStreamWorker::run()  A new worker started to work, to start playlist %s", _PurID, _pPlaylist->getDBfile());
			// already filled, start stream
			if(!StartNewStream()) {
				glog(ZQ::common::Log::L_DEBUG, L"FAILURE  [%06d]STVStreamWorker::run()  Can not play the playlist %s on channel %d", _PurID, _pPlaylist->getDBfile(), _PurID);
				// fail, increase penalty
				gSTV.getPlayListMan()->getTimer()->penaltyIncr();
				return STVRTSPFAIL;
			}
			glog(ZQ::common::Log::L_DEBUG, L"SUCCESS  [%06d]STVStreamWorker::run()  The playlist %s had been played on channel %d",_PurID, _pPlaylist->getDBfile(), _PurID);
		}
		else
		{
			glog(ZQ::common::Log::L_DEBUG, L"PENDING  [%06d]STVStreamWorker::run()  A new worker started to work, to stop playlist %s", _PurID, _pPlaylist->getDBfile());
			// shut down stream, and do nothing
			if(!ShutdownStream()) {
				glog(ZQ::common::Log::L_DEBUG, "FAILURE  [%06d]STVStreamWorker::run()  Can not shutdown the current stream on channel %d",_PurID, _PurID);
				return STVRTSPFAIL;
			}
			glog(ZQ::common::Log::L_DEBUG, "SUCCESS  [%06d]STVStreamWorker::run()  The stream had been shutdown on channel %d",_PurID, _PurID);
		}
		
	}
	catch(ZQ::common::Exception excep) {
#ifdef _DEBUG
		wprintf(L"FAILURE  STVStreamWorker::run()  when dealing with playlist %s, an exception occurs, with error string:\n %s", _pPlaylist->getDBfile());
		printf("%s\n",excep.getString());
#endif
		glog(ZQ::common::Log::L_ERROR, L"FAILURE  [%06d]STVStreamWorker::run()  when dealing with playlist %s, an exception occurs, with error string:\n", _PurID, _pPlaylist->getDBfile());
		glog(ZQ::common::Log::L_ERROR, "%s", excep.getString());
		
		// exception occurs, change status
		if(_pPlaylist->getPLStatus()!=PLSTAT_IDLE)
			_pPlaylist->setPLStatus(PLSTAT_IDLE);

		gSTV.getPlayListMan()->setChnlState(_PurID, CHNL_NONE);
//		gSTV.getPlayListMan()->setChnlCurrList(_PurID, NULL);

		_bWorking = FALSE;
		return -1;
	}
	catch(...) {
#ifdef _DEBUG
		wprintf(L"FAILURE  STVStreamWorker::run()  when dealing with playlist %s, an unknown exception occurs", _pPlaylist->getDBfile());
#endif
		glog(ZQ::common::Log::L_ERROR, L"FAILURE  [%06d]STVStreamWorker::run()  when dealing with playlist %s, an unknown exception occurs", _PurID, _pPlaylist->getDBfile());
		
		// exception occurs, change status
		if(_pPlaylist->getPLStatus()!=PLSTAT_IDLE)
			_pPlaylist->setPLStatus(PLSTAT_IDLE);
//		gSTV.getPlayListMan()->setChnlState(_PurID, CHNL_IDLE);

		_bWorking = FALSE;
		return -1;
	}
	
	glog(ZQ::common::Log::L_DEBUG, "NOTIFY   [%06d]STVStreamWorker::run()  A worker thread died gracefully" ,_PurID);
	_bWorking = FALSE;
	return STVSUCCESS;
}

bool STVStreamWorker::ShutdownStream()
{
	bool status;
	RtspResponse resBack;
	
	glog(ZQ::common::Log::L_DEBUG, L"NOTIFY   [%06d]STVStreamWorker::StartNewStream()  Woker begin to stop %s" ,_PurID, _pPlaylist->getDBfile());
	//////////////////////////////////////////////////////////////////////////
	// get last connection and TEARDOWN it
	RtspClientConnection* lastConn = gSTV.getRtspMan()->getConnByPurchase(_PurID);
	if(lastConn) 
	{	// found last connection

		if(lastConn->getState()>=RTSP_READY)
		{	// has called SETUP or PLAY
			glog(ZQ::common::Log::L_DEBUG, "NOTIFY   [%06d]STVStreamWorker::ShutdownStream()  Former stream %d should be tore down" ,_PurID, lastConn);

			RtspRequest reqTeardown("TEARDOWN", "*");
			// tear down last stream
			status = lastConn->sendMSG(reqTeardown, resBack);
			if(!status) {
				glog(ZQ::common::Log::L_WARNING, "FAILURE  [%06d]STVStreamWorker::ShutdownStream()  error on sending TEARDOWN message" ,_PurID);
				_pPlaylist->setPLStatus(PLSTAT_IDLE);
				gSTV.getRtspMan()->removeConn(lastConn->getSession());
				gSTV.getPlayListMan()->setChnlState(_PurID, CHNL_NONE);
//				gSTV.getPlayListMan()->setChnlCurrList(_PurID, NULL);
				return FALSE;
			}
			
			glog(ZQ::common::Log::L_DEBUG, "SUCCESS  [%06d]STVStreamWorker::ShutdownStream()  TEARDOWN message sent" ,_PurID);
		}
			
		gSTV.getRtspMan()->removeConn(lastConn->getSession());
				
		// send missing status feedback if necessary
		SSAENotification missingNoti;
		if(gSTV.getPlayListMan()->getLastNoti(_PurID, missingNoti))
		{
			if(missingNoti.wOperation== SAENO_PLAY)
			{	// last operation is start, with no stop, so send stop
				missingNoti.wOperation = SAENO_STOP;

				_pPlaylist->setPLStatus(PLSTAT_DESTROYING);
				gSTV.OnSendAEStatus(_PurID, missingNoti);
			}

			switch(_pPlaylist->getPLType()) {
			case LISTTYPE_PLAYLIST:
				_pPlaylist->setPLStatus(PLSTAT_IDLE);
				break;
			case LISTTYPE_FILLER:
				_pPlaylist->setPLStatus(PLSTAT_IDLE);
				break;
			case LISTTYPE_BARKER:
				_pPlaylist->setPLStatus(PLSTAT_IDLE);
				break;
			}
		}

		lastConn->setCurrentList(NULL);
		
		
	}

//	gSTV.getPlayListMan()->setChnlCurrList(_PurID, NULL);
	gSTV.getPlayListMan()->setChnlState(_PurID, CHNL_NONE);
	
	glog(ZQ::common::Log::L_INFO, "SUCCESS  [%06d]STVStreamWorker::ShutdownStream()  Channel with purchase id %d had been shut down" ,_PurID, _PurID);
	return TRUE;
}

bool STVStreamWorker::StartNewStream()
{
	bool status;
	std::string desIp, desPort, desNode;
	RtspResponse resBack;
	char portbuff[8];
	ultoa(_dwRtspHostPort, portbuff, 10);

	glog(ZQ::common::Log::L_DEBUG, L"NOTIFY   [%06d]STVStreamWorker::StartNewStream()  Woker begin to start %s" ,_PurID, _pPlaylist->getDBfile());

	//////////////////////////////////////////////////////////////////////////
	// get last connection and TEARDOWN it
	RtspClientConnection* lastConn = gSTV.getRtspMan()->getConnByPurchase(_PurID);
	RtspClientConnection* currConn = NULL;

	int nType = -1;
	STVPlaylist* pLastList = NULL;

	if(lastConn) 
	{	// found last connection

		// get current playing list
		pLastList = lastConn->getCurrentList();

		int connState=lastConn->getState();
		if(connState>=RTSP_READY) 
		{	// has called SETUP or PLAY

			// need tear down the stream
			glog(ZQ::common::Log::L_DEBUG, L"NOTIFY   [%06d]STVStreamWorker::StartNewStream()  Former list %s should be tore down" ,_PurID, pLastList->getDBfile());
			
			RtspRequest reqTeardown("TEARDOWN", "*");
			// tear down last stream
			status = lastConn->sendMSG(reqTeardown, resBack);
			
			if(!status) {
				glog(ZQ::common::Log::L_WARNING, "FAILURE  [%06d]STVStreamWorker::StartNewStream()  error on sending TEARDOWN message" ,_PurID);
				_pPlaylist->setPLStatus(PLSTAT_IDLE);
				gSTV.getRtspMan()->removeConn(lastConn->getSession());
				gSTV.getPlayListMan()->setChnlState(_PurID, CHNL_NONE);
//				gSTV.getPlayListMan()->setChnlCurrList(_PurID, NULL);
				return FALSE;
			}
			else {
				glog(ZQ::common::Log::L_DEBUG, "SUCCESS  [%06d]STVStreamWorker::StartNewStream()  TEARDOWN message sent" ,_PurID);
			}
		}
		
		if(pLastList)
		{
			// send missing status feedback if necessary
			SSAENotification missingNoti;
			if(gSTV.getPlayListMan()->getLastNoti(_PurID, missingNoti))
			{
				if(missingNoti.wOperation== SAENO_PLAY)
				{	// last operation is start, with no stop, so send stop
					missingNoti.wOperation = SAENO_STOP;

					pLastList->setPLStatus(PLSTAT_DESTROYING);
					
					if(pLastList->isGlobal())
						gSTV.getPlayListMan()->setChnlState(_PurID, CHNL_GSTOPPING);
					else
						gSTV.getPlayListMan()->setChnlState(_PurID, CHNL_NSTOPPING);

					gSTV.OnSendAEStatus(_PurID, missingNoti);
					
				}
			}
		
			if(nType==LISTTYPE_PLAYLIST)
				pLastList->setPLStatus(PLSTAT_OUTOFDATE);
			else
				pLastList->setPLStatus(PLSTAT_IDLE);

//			gSTV.getPlayListMan()->setChnlCurrList(_PurID, NULL);
		}
		
		// reset connection socket, no need to create new connection
		currConn = lastConn;
		currConn->reset();
	}
	else 
	{	// no last connection, create new connection
		glog(ZQ::common::Log::L_DEBUG, "NOTIFY   [%06d]STVStreamWorker::StartNewStream()  No former stream." ,_PurID);
		currConn = gSTV.getRtspMan()->createConn(_szRtspHostIP, portbuff, _PurID, _dwRtspNsec);
		if(currConn==NULL) 
		{
			glog(ZQ::common::Log::L_WARNING, "FAILURE  [%06d]STVStreamWorker::StartNewStream()  Can not create RTSP connection on %s:%s" ,_PurID,_szRtspHostIP,portbuff);
			_pPlaylist->setPLStatus(PLSTAT_IDLE);
			gSTV.getPlayListMan()->setChnlState(_PurID, CHNL_NONE);
//			gSTV.getPlayListMan()->setChnlCurrList(_PurID, NULL);
			return FALSE;
		}
		
		glog(ZQ::common::Log::L_DEBUG, "SUCCESS  [%06d]STVStreamWorker::StartNewStream()  RTSP connection created on %s:%s" ,_PurID,_szRtspHostIP,portbuff);
	}
	//////////////////////////////////////////////////////////////////////////
	// create new connection for PLAY

	// set status back to starting mode
	if(_pPlaylist->isGlobal())
		gSTV.getPlayListMan()->setChnlState(_PurID, CHNL_GSTARTING);
	else
		gSTV.getPlayListMan()->setChnlState(_PurID, CHNL_NSTARTING);
	
	glog(ZQ::common::Log::L_DEBUG, "NOTIFY   [%06d]STVStreamWorker::StartNewStream()  New stream %x should be brought up" ,_PurID, currConn);

	RtspRequest reqSetup("SETUP",_szRtspURL);
				
	// get destination info
	gSTV.getPlayListMan()->getChnlInfo(_PurID, desNode, desIp, desPort);
		
	// make up Transport header
	RtspMsgHeader transport(KEY_TRANSPORT);
	std::string multimac = RtspClientConnection::getDynamicMac(desIp);
	transport.setSubHeaderField(" MP2T/AVP/UDP","");
	transport.setSubHeaderField("unicast","");
	transport.setSubHeaderField(KEY_DESTINATION,desIp);
	transport.setSubHeaderField(KEY_CLIENTPORT,desPort);
	transport.setSubHeaderField(KEY_CLIENTMAC,multimac);

	// make up SeaChange-Server-Data header
	RtspMsgHeader serverdata(KEY_SEACHANGESERVERDATA);
	serverdata.setSubHeaderField(KEY_NODEGROUPID,desNode);
	//serverdata.setSubHeaderField(KEY_SMARTCARDID,"0000000001");
	serverdata.setSubHeaderField(KEY_DEVICEID,multimac);
	
	// make up SeaChange-Mod-Data header
	RtspMsgHeader moddata(KEY_SEACHANGEMODDATA);
	char purstr[16];
	sprintf(purstr, "%010d", _PurID);
//	moddata.setSubHeaderField(KEY_PURCHASEID, purstr);
	moddata.setSubHeaderField(KEY_HOMEID, purstr);

	// form the whole SETUP request
	reqSetup.setHeaderField(KEY_SEACHANGEVERSION,"1");
	reqSetup.setHeaderField(transport.getName(),transport.toString());
	reqSetup.setHeaderField(KEY_SEACHANGEMAYNOTIFY," ");
	reqSetup.setHeaderField(serverdata.getName(),serverdata.toString());
	reqSetup.setHeaderField(moddata.getName(), moddata.toString());

	// send SETUP request & PLAY request
	status = currConn->sendMSG(reqSetup, resBack);
	if(!status) {
		bool retryresult = FALSE;
		// retry for more times
		for(int retrynum=0; retrynum<DEFAULT_RETRYTIMES; retrynum++) {
			glog(ZQ::common::Log::L_DEBUG, "FAILURE  [%06d]STVStreamWorker::StartNewStream()  error on sending SETUP message" ,_PurID);
			
			// increase retry times tick
			_pPlaylist->retryIncr();
			
			// reset socket object
			if(!currConn->reset()) {
				glog(ZQ::common::Log::L_WARNING, "FAILURE  [%06d]STVStreamWorker::StartNewStream()  Can not reset socket" ,_PurID);
				_pPlaylist->setPLStatus(PLSTAT_IDLE);
				gSTV.getRtspMan()->removeConn(currConn->getSession());
				gSTV.getPlayListMan()->setChnlState(_PurID, CHNL_NONE);
//				gSTV.getPlayListMan()->setChnlCurrList(_PurID, NULL);
				return FALSE;
			}
						
			status = currConn->sendMSG(reqSetup, resBack);
			if(!status) { continue; }
			else { retryresult = TRUE; break; }
		}
		if(!retryresult) {
			if(_pPlaylist->getPLType()==LISTTYPE_PLAYLIST && !_pPlaylist->retryTest())
				_pPlaylist->setPLStatus(PLSTAT_OUTOFDATE);
			else
				_pPlaylist->setPLStatus(PLSTAT_IDLE);

			gSTV.getRtspMan()->removeConn(currConn->getSession());
			gSTV.getPlayListMan()->setChnlState(_PurID, CHNL_NONE);
//			gSTV.getPlayListMan()->setChnlCurrList(_PurID, NULL);
			return FALSE;
		}
	}
	glog(ZQ::common::Log::L_DEBUG, "SUCCESS  [%06d]STVStreamWorker::StartNewStream()  SETUP message sent" ,_PurID);
	
	::Sleep(200);
	
	resBack.clearMessage();
	// form the whole PLAY request
	RtspRequest reqPlay("PLAY","*");
	// calculate npt if necessary (if npt<=5, we do not care it)
	char nptBuff[16];
	if(_pPlaylist->getStartFromMid() && _pPlaylist->getStartPoint()>5) {
		ultoa(_pPlaylist->getStartPoint(), nptBuff, 10);
		strcat(nptBuff, ".0-");
		RtspMsgHeader nptHeader(KEY_RANGE);
		nptHeader.setSubHeaderField(KEY_RANGE_NPT,nptBuff);
		reqPlay.setHeaderField(nptHeader.getName(), nptHeader.toString());
	}
	reqPlay.setHeaderField(KEY_SCALE,"1.0");
	status = currConn->sendMSG(reqPlay, resBack);
	if(!status) {
		bool retryresult = FALSE;
		// retry for 2 more times
		for(int retrynum=0; retrynum<DEFAULT_RETRYTIMES; retrynum++) {
			glog(ZQ::common::Log::L_DEBUG, "FAILURE  [%06d]STVStreamWorker::StartNewStream()  error on sending PLAY message" ,_PurID);

			// increase retry times tick
			_pPlaylist->retryIncr();
			
			status = currConn->sendMSG(reqPlay, resBack);
			if(!status) { continue; }
			else { retryresult = TRUE; break; }
		}
		if(!retryresult) {
			if(_pPlaylist->getPLType()==LISTTYPE_PLAYLIST && !_pPlaylist->retryTest())
				_pPlaylist->setPLStatus(PLSTAT_OUTOFDATE);
			else
				_pPlaylist->setPLStatus(PLSTAT_IDLE);

			gSTV.getRtspMan()->removeConn(currConn->getSession());
			gSTV.getPlayListMan()->setChnlState(_PurID, CHNL_NONE);
//			gSTV.getPlayListMan()->setChnlCurrList(_PurID, NULL);
			return FALSE;
		}
	}

	// set list status
	_pPlaylist->setPLStatus(PLSTAT_PLAYING);

	// reset retry times tick
	_pPlaylist->retryReset();
	
	// set channel status
	if(_pPlaylist->isGlobal())
		gSTV.getPlayListMan()->setChnlState(_PurID, CHNL_GPLAYING);
	else
		gSTV.getPlayListMan()->setChnlState(_PurID, CHNL_NPLAYING);

	// save current list in connection for future use
	currConn->setCurrentList(_pPlaylist);

	// save current list in channel info for future use
//	gSTV.getPlayListMan()->setChnlCurrList(_PurID, _pPlaylist);


	glog(ZQ::common::Log::L_DEBUG, "SUCCESS  [%06d]STVStreamWorker::StartNewStream()  PLAY message sent" ,_PurID);
#ifdef _DEBUG
	printf("Stream started.\n");
#endif	
	glog(ZQ::common::Log::L_INFO, "SUCCESS  [%06d]STVStreamWorker::StartNewStream()  Stream started" ,_PurID);
	return TRUE;
}