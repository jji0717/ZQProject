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
// Ident : $Id: STVPlaylist.h$
// Branch: $Name:  $
// Author: Bernie(Tianbin) Zhao
// Desc  : implementation of STV timer thread
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/PlaylistMod/STVPMTimerMan.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 19    05-07-29 16:14 Bernie.zhao
// 
// 18    05-06-09 10:16 Bernie.zhao
// 
// 17    05-03-24 14:52 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 16    05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// 
// 15    05-01-05 17:16 Bernie.zhao
// modified to support STV status feedback
// 
// 14    04-12-16 14:55 Bernie.zhao
// 
// 13    04-12-06 20:54 Bernie.zhao
// mem exception resolved
// 
// 12    04-12-06 11:46 Bernie.zhao
// 
// 11    04-11-30 10:24 Bernie.zhao
// Nov30, after resolving ISSStreamTerminate problem
// 
// 10    04-11-23 10:02 Bernie.zhao
// 
// 9     04-11-03 19:59 Bernie.zhao
// Oct/3 Before Service Release
// 
// 8     04-10-26 16:08 Bernie.zhao
// 0.1.6 Oct/26
// ===========================================================================


#include "STVPMTimerMan.h"
#include "../MainCtrl/ScheduleTV.h"

extern ScheduleTV gSTV;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//##ModelId=4147B84B00FA
STVPMTimerMan::STVPMTimerMan(STVPlaylistManager* paramPM)
{
	_thePM = paramPM;
	_nextsleeptime = DEFAULT_SLEEPTIME;
	_hWakeupOrDie[0]	= ::CreateEvent(0,0,0,0);
	_hWakeupOrDie[1]	= ::CreateEvent(0,0,0,0);
	_dwPenalty = 0;
}

//##ModelId=4147B84B0109
STVPMTimerMan::~STVPMTimerMan()
{
	::CloseHandle(_hWakeupOrDie[0]);
	::CloseHandle(_hWakeupOrDie[1]);
}

//##ModelId=4147B84B011A
int STVPMTimerMan::run()
{
	int retval = 0;
	bool bRestAWhile	= FALSE;
	bool bStartup		= TRUE;

	try 
	{
	
		int err;
	#ifdef _DEBUG
		printf("a new timer man started!\n");
	#endif
		glog(ZQ::common::Log::L_INFO, "SUCCESS  STVPMTimerMan::run()  playlist timer init...");
		::Sleep(DEFAULT_SLEEPTIME*10);
		glog(ZQ::common::Log::L_INFO, "SUCCESS  STVPMTimerMan::run()  playlist timer started");
		
		//int err;
		DWORD waitstatus;
		
		for(;;) 
		{
			std::vector<STVPlaylist*> readylists;
			std::vector<DWORD> readyChnls;

			_nextsleeptime = DEFAULT_SLEEPTIME;

			//////////////////////////////////////////////////////////////////////////
			// test penalty, if high, do not care any stream
			if(!penaltyTest())
			{
				glog(ZQ::common::Log::L_DEBUG, "NOTIFY   STVPMTimerMan::run()  Penalty too high: %ld", _dwPenalty);
				Sleep(_nextsleeptime);
				penaltyDecr();
				continue;
			}
			
			//////////////////////////////////////////////////////////////////////////
			// scan lists within STV_TIMECRIT_ASSOCIATE and STV_TIMECRIT_PLAY
			// and then notify main control to IDS query or Play them
			// at the same time decide how long will next sleep take
			
			readylists.clear();
			readyChnls.clear();

			_nextsleeptime = _thePM->extract(STV_TIMECRIT_ASSOCIATE, STV_TIMECRIT_PLAY, readylists);
			if(readylists.size())
				glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPMTimerMan::run()  %d playlist(s) had their/her time point(s) matched", readylists.size());
			
			for(int i=0; i<readylists.size(); i++) 
			{
				std::map<DWORD, ChannelInfo>::iterator iter = _thePM->_mapChnl.find(readylists[i]->getPLchannel());
				if(iter == _thePM->_mapChnl.end()) {	// no such channel
					continue;
				}
				else if( iter->second.validation ) {	// channel valid
	#ifdef _DEBUG
					wprintf(L"this playlist %s need handle\tstatus:%s\n", readylists[i]->getDBfile(), STVPlaylist::statusitoa(readylists[i]->getPLStatus()).c_str() );
	#endif
					_thePM->OnPlayImmediately(readylists[i], iter->first);

					// if it is during first time start up, delay long time to avoid SRM collision
					// otherwise delay 500 milliseconds
					Sleep((bStartup?2000:500));

					readyChnls.push_back(readylists[i]->getPLchannel());
				}
			}

			//////////////////////////////////////////////////////////////////////////
			// scan channel to activate newly added filler/barker
			{
			
				ZQ::common::MutexGuard	tmpGd(_thePM->_mapChnlMutex);
				std::map<DWORD, ChannelInfo>::iterator Chanliter;
				for(Chanliter=_thePM->_mapChnl.begin(); Chanliter!=_thePM->_mapChnl.end(); Chanliter++) 
				{
					if(bRestAWhile)
					{
						// if it is during first time start up, delay long time to avoid SRM collision
						// otherwise delay 500 milliseconds
						Sleep((bStartup?2000:1000));
					}
					
					bRestAWhile = FALSE;
					DWORD chnlID = Chanliter->first;
					ChannelInfo chnlINFO = Chanliter->second;
					std::vector<STVPlaylist*> listset;
					bool bSkip = FALSE;

					// if it is already handled before during extract(), skip it
					for(int jj=0; jj<readyChnls.size(); jj++)
					{
						if(readyChnls[jj] == chnlID)
						{
							bSkip = TRUE;
							break;
						}
					}

					if(bSkip)
						continue;

					//////////////////////////////////////////////////////////////////////////
					// search channel status
					
					if(chnlINFO.validation) 
					{
						listset.clear();
						int nType;

						STVPlaylist* pCurr = NULL;
						STVPlaylist* pNext = NULL;
						
						switch(chnlINFO.status) 
						{
						case CHNL_NONE:
							// channel is empty, start a filler/barker/Gfiller

							if(!_thePM->queryByChnl(chnlID, listset, LISTTYPE_BARKER))
								_thePM->queryByChnl(chnlID, listset, LISTTYPE_FILLER);
							pNext = _thePM->getEffectiveSet(listset);
							if(pNext) 
							{	// should start filler or barker
								glog(ZQ::common::Log::L_DEBUG, L"NOTIFY   STVPMTimerMan::run()  Channel %d should start list %s", chnlID, pNext->getDBfile());
								_thePM->OnPlayImmediately(pNext, chnlID);
								bRestAWhile = TRUE;
							}
							else 
							{
								
								_thePM->queryGlobalFiller(listset);	// get global filler
								pNext = _thePM->getEffectiveSet(listset);
								if(pNext)
								{	
									// should start global filler
									glog(ZQ::common::Log::L_DEBUG, L"NOTIFY   STVPMTimerMan::run()  Channel %d should start GLOBAL filler %s", chnlID, pNext->getDBfile());
									_thePM->OnPlayImmediately(pNext, chnlID);
									bRestAWhile = TRUE;
									
								}
							}
							break;
						
						case CHNL_NSTARTING:
						case CHNL_NSTOPPING:
						case CHNL_GSTARTING:
						case CHNL_GSTOPPING:
							// channel is starting or stopping stream, skip it
							break;
						
						case CHNL_NPLAYING:
						case CHNL_GPLAYING:
							// channel is playing list

							// get current list or global filler
							pCurr =_thePM->queryCurrentPL(chnlID, nType, PLSTAT_PLAYING);
							if(!pCurr)
							{
								pCurr = _thePM->getCurrGlobalFiller();
								nType = LISTTYPE_FILLER;
							}

							if(pCurr) 
							{	
								switch(nType) 
								{
								case LISTTYPE_PLAYLIST:
									break;
								case LISTTYPE_FILLER:
									if(!_thePM->queryByChnl(chnlID, listset, LISTTYPE_BARKER))	// get barker
									{
										if(!_thePM->queryByChnl(chnlID, listset, LISTTYPE_FILLER))	// get filler
										{
										}
									  
									}
									
									pNext = _thePM->getEffectiveSet(listset);

									if( (pNext) && (pNext!=pCurr) ) 
									{	// should modify current filler
										glog(ZQ::common::Log::L_DEBUG, L"NOTIFY   STVPMTimerMan::run()  Filler list %s should be replaced by list %s", pCurr->getDBfile(), pNext->getDBfile());
										pCurr->setPLStatus(PLSTAT_IDLE);
										_thePM->OnPlayImmediately(pNext, chnlID);
										bRestAWhile = TRUE;
									}
									else if(!pNext) 
									{	// no new list
										
										_thePM->queryGlobalFiller(listset);	// get global filler
										pNext = _thePM->getEffectiveSet(listset);
										if(pNext)
										{
											if(chnlINFO.status==CHNL_NONE)
											{
												// has global filler, start it
												glog(ZQ::common::Log::L_DEBUG, L"NOTIFY   STVPMTimerMan::run()  Filler list %s should be replaced by GLOBAL filler list %s", pCurr->getDBfile(), pNext->getDBfile());
												_thePM->OnPlayImmediately(pNext, chnlID);
												bRestAWhile = TRUE;
											}
										}
										else
										{
											// no global filler, stop stream
											glog(ZQ::common::Log::L_DEBUG, L"NOTIFY   STVPMTimerMan::run()  Filler list %s should be stopped", pCurr->getDBfile());
											gSTV.OnShutdownStream(pCurr, chnlID);
											bRestAWhile = TRUE;
										}
									}
									break;
								case LISTTYPE_BARKER:
									_thePM->queryByChnl(chnlID, listset, LISTTYPE_BARKER);
									pNext = _thePM->getEffectiveSet(listset);
									if( (pNext) && (pNext!=pCurr) ) {	// should modify current filler
										glog(ZQ::common::Log::L_DEBUG, L"NOTIFY   STVPMTimerMan::run()  Barker list %s should be replaced by barker list %s", pCurr->getDBfile(), pNext->getDBfile());
										pCurr->setPLStatus(PLSTAT_IDLE);
										_thePM->OnPlayImmediately(pNext, chnlID);
										bRestAWhile = TRUE;
									}
									else if(!pNext) { // barker not effective anymore
										_thePM->queryByChnl(chnlID, listset, LISTTYPE_FILLER);
										pNext = _thePM->getEffectiveSet(listset);
										if(pNext) { // should turn to filler
											glog(ZQ::common::Log::L_DEBUG, L"NOTIFY   STVPMTimerMan::run()  Barker list %s should be replaced by filler list %d", pCurr->getDBfile(), pNext->getDBfile());
											pCurr->setPLStatus(PLSTAT_IDLE);
											_thePM->OnPlayImmediately(pNext, chnlID);
											bRestAWhile = TRUE;
										}
										else if(!pNext) { // filler also not effective

											_thePM->queryGlobalFiller(listset);	// get global filler
											pNext = _thePM->getEffectiveSet(listset);
											if(pNext)
											{	
												if(chnlINFO.status==CHNL_NONE)
												{	// has global filler, start it
													glog(ZQ::common::Log::L_DEBUG, L"NOTIFY   STVPMTimerMan::run()  Barker list %s should be replaced by GLOBAL filler list %s", pCurr->getDBfile, pNext->getDBfile());
													_thePM->OnPlayImmediately(pNext, chnlID);
													bRestAWhile = TRUE;
												}
												
											}
											else
											{	// no global filler, stop stream
												glog(ZQ::common::Log::L_DEBUG, L"NOTIFY   STVPMTimerMan::run()  Barker list %s should be stopped", pCurr->getDBfile());
												gSTV.OnShutdownStream(pCurr, chnlID);
												bRestAWhile = TRUE;
											}
											
										}
									}
									break;
								}
							}
							break;
						default :
							break;
						}

						
					}	// end if(chnlINFO.validation)
				} // end channel scan

			}

			// sleep until time reached or wakeup/terminate signal invoked
	//		glog(ZQ::common::Log::L_DEBUG, "NOTIFY   STVPMTimerMan::run()  ::WaitForSingleObject() begins sleep for %d milli-seconds\n", _nextsleeptime);
			waitstatus = ::WaitForMultipleObjects(2, _hWakeupOrDie, FALSE, _nextsleeptime);
	//		waitstatus = ::WaitForSingleObject(_hSTVTimerWakeup, _nextsleeptime);
			if(waitstatus == WAIT_OBJECT_0) {
				// wake up signal, wake up and re-scan the list
				continue;
			}
			else if(waitstatus == WAIT_OBJECT_0+1) {
				// PM ended, should terminate this thread
				glog(ZQ::common::Log::L_INFO, "SUCCESS  STVPMTimerMan::run()  playlist timer terminated");
				return 0;
			}
			else if(waitstatus == WAIT_TIMEOUT) {
				// timeout, wake up and re-scan the list
				continue;
			}
			else {

				err = ::GetLastError();
	#ifdef _DEBUG
				printf("::WaitForSingleObject() Error - %d \n", err);
	#endif
				glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPMTimerMan::run()  ::WaitForSingleObject() Error - %d \n", err);
			}

			bStartup = FALSE;

		}	// end for(;;)
		
	}	
	catch(ZQ::common::Exception excep) {
#ifdef _DEBUG
		printf("FAILURE  STVPMTimerMan::run()  An exception occurs, with error string: %s", excep.getString());
#endif
		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPMTimerMan::run()  An exception occurs, with error string: %s", excep.getString());
	}
	catch(...) {
#ifdef _DEBUG
		printf("FAILURE  STVPMTimerMan::run()  An unknown exception occurs");
#endif
		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPMTimerMan::run()  An unknown exception occurs");
	}
	return retval;
}
