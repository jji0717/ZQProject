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
// Desc  : implementation of STV playlist Manager
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/PlaylistMod/STVPlaylistManager.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 35    05-07-29 16:14 Bernie.zhao
// 
// 34    05-04-19 21:18 Bernie.zhao
// autobuild modification
// 
// 33    05-03-24 14:52 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 32    05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// 
// 31    05-01-05 17:16 Bernie.zhao
// modified to support STV status feedback
// 
// 30    04-12-16 14:55 Bernie.zhao
// 
// 29    04-12-06 20:54 Bernie.zhao
// mem exception resolved
// 
// 28    04-12-06 11:46 Bernie.zhao
// 
// 27    04-11-30 10:24 Bernie.zhao
// Nov30, after resolving ISSStreamTerminate problem
// 
// 26    04-11-23 10:02 Bernie.zhao
// 
// 25    04-11-03 19:59 Bernie.zhao
// Oct/3 Before Service Release
// 
// 24    04-10-28 18:19 Bernie.zhao
// before trip
// 
// 23    04-10-26 16:08 Bernie.zhao
// 0.1.6 Oct/26
// 
// 22    04-10-25 15:26 Bernie.zhao
// added status report
// 
// 21    04-10-24 19:04 Bernie.zhao
// end of 2004/Oct/24
// 
// 20    04-10-24 11:43 Bernie.zhao
// before 24/Oct
// 
// 19    04-10-21 15:29 Bernie.zhao
// 
// 18    04-10-19 17:21 Bernie.zhao
// for QA release
// 
// 17    04-10-18 18:46 Bernie.zhao
// 
// 16    04-10-18 10:33 Bernie.zhao
// test 1 ok
// 
// 15    04-10-15 16:14 Bernie.zhao
// 
// 14    04-10-14 11:04 Bernie.zhao
// node missing pro fixed
// 
// 13    04-10-13 16:47 Bernie.zhao
// 
// 12    04-10-13 15:50 Bernie.zhao
// 
// 11    04-10-13 15:37 Bernie.zhao
// fixed top&bottom asset error
// ===========================================================================
#include "STVPlaylistManager.h"
#include "../MainCtrl/ScheduleTV.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace ZQ::common;
extern ScheduleTV gSTV;

//##ModelId=4147B84B01E4
STVPlaylistManager::STVPlaylistManager()
{
	_DBpath = _T(DB_DEFAULT_PATH);

	_dbinit = new ZQ::common::ComInitializer();
	
	_ListMutex	= new ZQ::common::Mutex();
	_TimerMan	= new STVPMTimerMan(this);

	_TopAsset	= 0;
	_IsRunning	= FALSE;
	_HasConfig	= FALSE;
	// initialize IdsSession
	//initIdsSession();
	// create filler
	_autoF		= new AutoFiller();
	_barkerF	= new BarkerFiller();
	_FillLength = DEFAULT_FILLLENGTH;

	memset(&_gAEList, 0, sizeof(GlobalAElist));
	
	glog(ZQ::common::Log::L_INFO, "SUCCESS  STVPlaylistManager::STVPlaylistManager()  New Playlist Manager created");
}

//##ModelId=4147B84B01F4
STVPlaylistManager::~STVPlaylistManager()
{
	if(_IsRunning)
		terminate();
	
	if(_ListMutex)
		delete _ListMutex;
	if(_TimerMan)
		delete _TimerMan;
	
	// uninitialize IdsSession and delete it
//	_Builder->unInitialize();
//	if(_Builder)
//		delete _Builder;
	
	// delete filler
	if(_autoF)
		delete _autoF;
	if(_barkerF)
		delete _barkerF;
	if(_dbinit)
		delete _dbinit;

}

bool STVPlaylistManager::getLastNoti(DWORD purchaseID, SSAENotification& retNoti)
{
	ZQ::common::MutexGuard	tmpGd(_mapChnlMutex);
	std::map<DWORD, ChannelInfo>::iterator iter = _mapChnl.find(purchaseID);
	if(iter == _mapChnl.end()) {
		glog(ZQ::common::Log::L_WARNING, "FAILURE  STVPlaylistManager::getLastNoti()  No such channel with purchaseID %d", purchaseID);
		return FALSE;
	}
	
	retNoti.dwAeUID = iter->second.StreamStatus.dwAeUID;
	retNoti.dwPurchaseID = iter->second.StreamStatus.dwPurchaseID;
	retNoti.dwStatus = iter->second.StreamStatus.dwStatus;
	retNoti.wOperation = iter->second.StreamStatus.wOperation;
	
	return TRUE;
}

//##ModelId=4147B84B0203
STVPlaylist* STVPlaylistManager::createList(const wchar_t* mirrorfile/*=NULL*/)
{
	STVPlaylist* pnewlist = new STVPlaylist(this);

	if(mirrorfile==NULL) {
//		glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPlaylistManager::createList()  without mirrorfile");
		return pnewlist;
	}

	// convert to ascii
	char fname[MAX_PATH];
	size_t dwCount = MAX_PATH;
	wcstombs(fname, mirrorfile, dwCount);

	// create list from DB mirror file
	if(!pnewlist->restoreFromDB(mirrorfile)) { // restore failed
			delete pnewlist;
			pnewlist = NULL;
			glog(ZQ::common::Log::L_ERROR, L"FAULURE  STVPlaylistManager::createList()  restore from db %s failed", mirrorfile);
	}
	else {
		// reset playlist playing state
		if(pnewlist->getPLStatus()!=PLSTAT_OUTOFDATE)
			pnewlist->setPLStatus(PLSTAT_IDLE);
		pnewlist->setPlayingAsset(0);
		glog(ZQ::common::Log::L_INFO, L"SUCCESS  STVPlaylistManager::createList()  restore from db %s succeeded", mirrorfile);
	}
	return pnewlist;
}

//##ModelId=4147B84B0213
bool STVPlaylistManager::reg(STVPlaylist *newlist)
{
	if(!newlist) {
		return FALSE;
	}
	
	// lock the list
	_ListMutex->enter();

	if(newlist->getPLType()==LISTTYPE_FILLER && !newlist->isGlobal())	
		_SetPool.push_back(newlist);
	else if(newlist->getPLType()==LISTTYPE_FILLER && newlist->isGlobal())
		_GSetPool.push_back(newlist);
	else if(newlist->getPLType()==LISTTYPE_BARKER)
		_BarkerPool.push_back(newlist);
	else
		_ListPool.push_back(newlist);
	
	// unlock the list
	_ListMutex->leave();
	
	DWORD tmpan = newlist->getTopAsset();
	if( tmpan>_TopAsset )
		_TopAsset=tmpan;

//#ifdef _DEBUG
		glog(ZQ::common::Log::L_INFO, L"SUCCESS  STVPlaylistManager::reg()  playlist %s registered", newlist->getDBfile());
//#endif
	return TRUE;
}

//##ModelId=4147B84B0222
bool STVPlaylistManager::unreg(STVPlaylist *exlist)
{
	bool retval = false;

	// lock the list
	_ListMutex->enter();

	// search for the playlist
	for(int i=0; i<_ListPool.size(); i++) {
		if( _ListPool.at(i)==exlist ) {
			_ListPool.erase(_ListPool.begin()+i);
			retval = true;
			break;
		}		
	}

	if(!retval) {
		for(i=0; i<_BarkerPool.size(); i++) {
			if( _BarkerPool.at(i)==exlist ) {
				_BarkerPool.erase(_BarkerPool.begin()+i);
				retval = true;
				break;
			}		
		}
	}
	
	if(!retval) {
		for(i=0; i<_SetPool.size(); i++) {
			if( _SetPool.at(i)==exlist ) {
				_SetPool.erase(_SetPool.begin()+i);
				retval = true;
				break;
			}		
		}
	}
	
	if(!retval) {
		for(i=0; i<_GSetPool.size(); i++) {
			if( _GSetPool.at(i)==exlist ) {
				_GSetPool.erase(_GSetPool.begin()+i);
				retval = true;
				break;
			}		
		}
	}
	// unlock the list
	_ListMutex->leave();

	// stop stream if playing
	if(exlist->getPLStatus()==PLSTAT_PLAYING) {
	}

	// destruct the playlist here
	if(exlist) {
		// convert to unicode
		std::wstring findstr = exlist->getDBfile();
		
		::DeleteFile(findstr.c_str());
		
		glog(ZQ::common::Log::L_INFO, L"SUCCESS  STVPlaylistManager::unreg()  playlist %s unregistered",exlist->getDBfile());
		delete exlist;
		exlist = NULL;
	}
	return retval;
}

DWORD STVPlaylistManager::getChnlID(ZQ::common::IPreference* chnlPref)
{
	if(chnlPref==NULL)
		return NULL;
	
	char deviceid[16];
	char ipaddr[32];
	char ipport[16];
	DWORD dwDeviceID;
	DWORD dwIpPort;

	//ZQ::common::IPreference* pChnl = chnlPref->firstChild(XML_PORT);
	if(chnlPref==NULL)
		return NULL;

	if( (!chnlPref->has(XML_PORT_IPADRESS))
		||(!chnlPref->has(XML_PORT_IPPORT))
		||(!chnlPref->has(XML_PORT_DEVICEID)) ) {

		//printf("invalid channel");
		//pChnl->free();
		return NULL;
	}
		

	chnlPref->get(XML_PORT_DEVICEID, deviceid);
	dwDeviceID = atol(deviceid);
	chnlPref->get(XML_PORT_IPADRESS, ipaddr);
	chnlPref->get(XML_PORT_IPPORT, ipport);
	dwIpPort = atol(ipport);

	ZQ::common::MutexGuard	tmpGd(_mapChnlMutex);
	std::map<DWORD, ChannelInfo>::const_iterator iter = _mapChnl.begin();
	while(iter!= _mapChnl.end()) {
		if(iter->second.nodegroup == dwDeviceID
			&& iter->second.ipport == dwIpPort
			&& strcmp(iter->second.ipaddr, ipaddr)==0)
		{
			return iter->first;
		}

		iter++;
	}
	
	return NULL;
}

//##ModelId=4147B84B0232
int STVPlaylistManager::queryByChnl(std::string deviceid, std::string ipaddr, std::string ipport, std::vector<STVPlaylist*> &outlist, int listtype /*= LISTTYPE_PLAYLIST*/)
{
	outlist.clear();
	
	// lock the list
	_ListMutex->enter();

	//printf("Entering queryByChnl with node: %s, ip: %s, port: %s\n", deviceid.c_str(), ipaddr.c_str(), ipport.c_str());

	if(listtype==LISTTYPE_PLAYLIST) {
		for(int i=0; i<_ListPool.size(); i++) {
			if( _ListPool.at(i)->isMyChannel(deviceid, ipaddr, ipport)) {
				outlist.push_back(_ListPool[i]);
			}
		}
	}
	else if(listtype==LISTTYPE_BARKER){
		for(int i=0; i<_BarkerPool.size(); i++) {
			if( _BarkerPool.at(i)->isMyChannel(deviceid, ipaddr, ipport)) {
				outlist.push_back(_BarkerPool[i]);
			}
		}
	}
	else if(listtype==LISTTYPE_FILLER){
		for(int i=0; i<_SetPool.size(); i++) {
			if( _SetPool.at(i)->isMyChannel(deviceid, ipaddr, ipport)) {
				outlist.push_back(_SetPool[i]);
			}
		}
	}

	// unlock the list
	_ListMutex->leave();
	return outlist.size();
}

int STVPlaylistManager::queryByChnl(ZQ::common::IPreference* chnlPref, std::vector<STVPlaylist*> &outlist, int listtype /*= LISTTYPE_PLAYLIST*/)
{
	outlist.clear();
	
	if(chnlPref==NULL)
		return NULL;
	
	char deviceid[16];
	char ipaddr[32];
	char ipport[16];

	//ZQ::common::IPreference* pChnl = chnlPref->firstChild(XML_PORT);
	if(chnlPref==NULL)
		return NULL;

	if( (!chnlPref->has(XML_PORT_IPADRESS))
		||(!chnlPref->has(XML_PORT_IPPORT))
		||(!chnlPref->has(XML_PORT_DEVICEID)) ) {

		//printf("invalid channel");
		//pChnl->free();
		return NULL;
	}
		

	chnlPref->get(XML_PORT_DEVICEID, deviceid);
	chnlPref->get(XML_PORT_IPADRESS, ipaddr);
	chnlPref->get(XML_PORT_IPPORT, ipport);
	//chnlPref->free();
	return queryByChnl(deviceid, ipaddr, ipport, outlist, listtype);
}

int STVPlaylistManager::queryByChnl(DWORD chnlNum, std::vector<STVPlaylist*> &outlist, int listtype /* = LISTTYPE_PLAYLIST */)
{
	outlist.clear();
	
	// lock the list
	_ListMutex->enter();

	if(listtype==LISTTYPE_PLAYLIST) {
		for(int i=0; i<_ListPool.size(); i++) {
			if( (_ListPool.at(i)->getPLchannel()==chnlNum)&&(_ListPool.at(i)->getPLType()==listtype) ) {
				outlist.push_back(_ListPool[i]);
			}
		}
	}
	else if(listtype==LISTTYPE_BARKER) {
		for(int i=0; i<_BarkerPool.size(); i++) {
			if( (_BarkerPool.at(i)->getPLchannel()==chnlNum)&&(_BarkerPool.at(i)->getPLType()==listtype) ) {
				outlist.push_back(_BarkerPool[i]);
			}
		}
	}
	else if(listtype==LISTTYPE_FILLER) {
		for(int i=0; i<_SetPool.size(); i++) {
			if( (_SetPool.at(i)->getPLchannel()==chnlNum)&&(_SetPool.at(i)->getPLType()==listtype) ) {
				outlist.push_back(_SetPool[i]);
			}
		}
	}

	// unlock the list
	_ListMutex->leave();
	return outlist.size();
}

int STVPlaylistManager::queryGlobalFiller(std::vector<STVPlaylist*> &outlist)
{
	outlist.clear();
	
	// lock the list
	_ListMutex->enter();

	for(int i=0; i<_GSetPool.size(); i++) {
		outlist.push_back(_GSetPool[i]);
	}
	
	// unlock the list
	_ListMutex->leave();
	return outlist.size();
}

STVPlaylist* STVPlaylistManager::queryCurrentPL(DWORD chnlNum, int& listtype, int status/* =PLSTAT_PLAYING */)
{
	// lock the list
	_ListMutex->enter();

	for(size_t i=0; i<_ListPool.size(); i++) {
		if( _ListPool.at(i)->getPLchannel()==chnlNum ) 
		{
			if( _ListPool.at(i)->getPLStatus()==status )
			{
				listtype = LISTTYPE_PLAYLIST;
				_ListMutex->leave();
				return _ListPool[i];
			}
			
		}
	}
	
	for(i=0; i<_BarkerPool.size(); i++) {
		if( _BarkerPool.at(i)->getPLchannel()==chnlNum ) 
		{
			if( _BarkerPool.at(i)->getPLStatus()==status)
			{
				listtype = LISTTYPE_BARKER;
				_ListMutex->leave();
				return _BarkerPool[i];
			}
		}
	}

	for(i=0; i<_SetPool.size(); i++) {
		if( _SetPool.at(i)->getPLchannel()==chnlNum ) 
		{
			if( _SetPool.at(i)->getPLStatus()==status)
			{
				listtype = LISTTYPE_FILLER;
				_ListMutex->leave();
				return _SetPool[i];
			}
		}
	}

	// unlock the list
	_ListMutex->leave();
	return NULL;
}

//##ModelId=4147B84B0242
DWORD STVPlaylistManager::extract(unsigned long critAssoci, unsigned long critPlay, std::vector<STVPlaylist*>& outlists)
{
	SYSTEMTIME currtime, listtime; 
	DWORD retval = DEFAULT_SLEEPTIME;
	bool	isMidNight = FALSE;
	
	::GetLocalTime(&currtime);
	if(currtime.wHour==0 && currtime.wMinute==0 && currtime.wSecond<5)
	{
		isMidNight = TRUE;	// it is mid-night, should clear all out-of-date lists
	}


	outlists.clear();

	// lock the list
	_ListMutex->enter();

	for(int i=0; i<_ListPool.size(); i++) 
	{
		if(_ListPool[i]->getPLType()!=LISTTYPE_PLAYLIST)
			continue;

		listtime = _ListPool[i]->getNextTime();
		if(timeComp(listtime, currtime) >=0) 
		{
			// list time has not passed
			// get time difference
			DWORD tspan = timeSub(listtime,currtime);

			switch(_ListPool[i]->getPLStatus())
			{
			case PLSTAT_IDLE:
				if(tspan<=critPlay) {
					// time difference is within criteria
					std::map<DWORD, ChannelInfo>::iterator iter = _mapChnl.find(_ListPool[i]->getPLchannel());
					if( (iter == _mapChnl.end()) 
						|| !iter->second.validation ) {	// invalid channel
						break;
					}
					else {
						_ListPool[i]->setStartFromMid(FALSE);
						outlists.push_back(_ListPool[i]);
					}
				}
				else {
					// time point not reached
					if(retval>tspan*1000)
						retval=tspan*1000-critPlay;
				}
				break;
			case PLSTAT_ASSOCIATED:
				break;
			default:
				break;
			}
			
		}
		else 
		{
			// listtime has passed
			DWORD tspan = timeSub(currtime,listtime);
			if (_ListPool[i]->getPLStatus()==PLSTAT_IDLE && tspan<(_ListPool[i]->getPLLength()-DEFAULT_LAST_CHANCE_NPT))
			{
				// if is scheduled Playlist which missed time point, play it from mid point
				std::map<DWORD, ChannelInfo>::iterator iter = _mapChnl.find(_ListPool[i]->getPLchannel());
				if( (iter != _mapChnl.end()) && iter->second.validation ) 
				{	// valid channel
					_ListPool[i]->setStartFromMid(TRUE);
					_ListPool[i]->setStartPoint(tspan);
					outlists.push_back(_ListPool[i]);
				}
								
			}
			
		}
	}

	if(isMidNight)
	{
		STVPlaylist* pDyingList = NULL;
		std::wstring strFilename = L"";

		// normal list
		for(int j=0; j<_ListPool.size(); j++)
		{
			listtime = _ListPool[j]->getNextTime();
			if(timeComp(listtime, currtime)<0)
			{
				DWORD tspan = timeSub(currtime,listtime);
				if (tspan>_ListPool[j]->getPLLength())
				{
					pDyingList = _ListPool[j];
					_ListPool.erase(_ListPool.begin()+j);

					strFilename = pDyingList->getDBfile();
					::DeleteFile(strFilename.c_str());
					delete pDyingList;
					pDyingList = NULL;
				}
				
			}
		}

		// filler
		for(j=0; j<_SetPool.size(); j++)
		{
			if(!_SetPool[j]->isInEffective(currtime))
			{
				pDyingList = _SetPool[j];
				_SetPool.erase(_SetPool.begin()+j);

				strFilename = pDyingList->getDBfile();
				::DeleteFile(strFilename.c_str());
				delete pDyingList;
				pDyingList = NULL;
			}
		}

		// global filler
		for(j=0; j<_GSetPool.size(); j++)
		{
			if(!_GSetPool[j]->isInEffective(currtime))
			{
				pDyingList = _GSetPool[j];
				_GSetPool.erase(_GSetPool.begin()+j);

				strFilename = pDyingList->getDBfile();
				::DeleteFile(strFilename.c_str());
				delete pDyingList;
				pDyingList = NULL;
			}
		}

		// barker
		for(j=0; j<_BarkerPool.size(); j++)
		{
			if(!_BarkerPool[j]->isInEffective(currtime))
			{
				pDyingList = _BarkerPool[j];
				_BarkerPool.erase(_BarkerPool.begin()+j);

				strFilename = pDyingList->getDBfile();
				::DeleteFile(strFilename.c_str());
				delete pDyingList;
				pDyingList = NULL;
			}
		}

	}
	
	// unlock the list
	_ListMutex->leave();
	return retval;
}

int STVPlaylistManager::purify(DWORD chnlID, DWORD bottomNO, DWORD topNO, SYSTEMTIME nextTime, int nType)
{
	int retval =0;
	if(nType==LISTTYPE_PLAYLIST) {
		for(int i=0; i<_ListPool.size(); i++) {
			if( (nType==_ListPool[i]->getPLType())&&(chnlID==_ListPool[i]->getPLchannel()) ) {
				DWORD tmpTop = _ListPool[i]->getTopAsset();
				DWORD tmpBot = _ListPool[i]->getBottomAsset();
				SYSTEMTIME tmpTime = _ListPool[i]->getNextTime();

				if( (timeComp(tmpTime, nextTime)>=0)
					&&  (tmpBot<bottomNO) ) {
					// should unreg this list
					unreg(_ListPool[i]);
					i--;	// decrease index, otherwise will skip next list
					retval++;
				}
				else if( (timeComp(tmpTime, nextTime)<=0)
					  &&	(tmpBot>=bottomNO) ) {
					// should return -1, and do not create new list
					return -1;
				}
			}
		}
	}
	else if(nType==LISTTYPE_BARKER){
		for(int i=0; i<_BarkerPool.size(); i++) {
			if( (nType==_BarkerPool[i]->getPLType())&&(chnlID==_BarkerPool[i]->getPLchannel()) ) {
				DWORD tmpTop = _BarkerPool[i]->getTopAsset();
				DWORD tmpBot = _BarkerPool[i]->getBottomAsset();
				SYSTEMTIME tmpTime = _BarkerPool[i]->getNextTime();

				if( (timeComp(tmpTime, nextTime)>=0)
					&&  (tmpBot<bottomNO) ) {
					// should unreg this list
					unreg(_BarkerPool[i]);
					i--;	// decrease index, otherwise will skip next list
					retval++;
				}
				else if( (timeComp(tmpTime, nextTime)<=0)
					  &&	(tmpBot>=bottomNO) ) {
					// should return -1, and do not create new list
					return -1;
				}
			}
		}
	}
	else if(nType==LISTTYPE_FILLER){
		for(int i=0; i<_SetPool.size(); i++) {
			if( (nType==_SetPool[i]->getPLType())&&(chnlID==_SetPool[i]->getPLchannel()) ) {
				DWORD tmpTop = _SetPool[i]->getTopAsset();
				DWORD tmpBot = _SetPool[i]->getBottomAsset();
				SYSTEMTIME tmpTime = _SetPool[i]->getNextTime();

				if( (timeComp(tmpTime, nextTime)>=0)
					&&  (tmpBot<bottomNO) ) {
					// should unreg this list
					unreg(_SetPool[i]);
					i--;	// decrease index, otherwise will skip next list
					retval++;
				}
				else if( (timeComp(tmpTime, nextTime)<=0)
					  &&	(tmpBot>=bottomNO) ) {
					// should return -1, and do not create new list
					return -1;
				}
			}
		}
	}
	return retval;
}

//##ModelId=4147B84B0252
int STVPlaylistManager::getPlaylists(std::vector<STVPlaylist*>& outlists)
{
	outlists.clear();
	outlists.resize(_ListPool.size());
	for(int i=0; i<_ListPool.size(); i++) {
		outlists[i] = _ListPool[i];
	}
	return outlists.size();
}

//##ModelId=4147B84B0261
int STVPlaylistManager::restore()
{
	WIN32_FIND_DATA finddata;
	int retval =0;
	std::wstring findstr = _DBpath+_T("*.")+_T(DB_FILE_EXT);

	// search the database path for mirror files
	HANDLE hfind = ::FindFirstFile(findstr.c_str(), &finddata);
	if(hfind == INVALID_HANDLE_VALUE ) {	// not found
		//throw ZQ::common::Exception(std::string("Can not find ")+findstr);
		return 0;
	}
	// found first file
	if(isValidFile((_DBpath+finddata.cFileName).c_str())) {	// file name valid
		// restore a playlist
		STVPlaylist* pfirstlist = createList((_DBpath+finddata.cFileName).c_str());
		if(pfirstlist!=NULL) {
			if(reg(pfirstlist))
				retval++;
		}
	}
	
	while(::FindNextFile(hfind, &finddata)) {	// search for next
		//retval++;
		if(isValidFile((_DBpath+finddata.cFileName).c_str())) {	// file name valid
			// restore a playlist
			STVPlaylist* pNextlist = createList((_DBpath+finddata.cFileName).c_str());
			if(pNextlist!=NULL) {
				if(reg(pNextlist))
					retval++;
			}
		}
	}
	::FindClose(hfind);
	glog(ZQ::common::Log::L_INFO, "SUCCESS  STVPlaylistManager::restore()  %d existed playlist(s) had been restored", retval);
	return retval;
}

bool STVPlaylistManager::getChnlInfo(DWORD Uid, std::string& deviceid, std::string& ipaddr, std::string& port)
{
	char buffstr[MAX_PATH];

	ZQ::common::MutexGuard	tmpGd(_mapChnlMutex);
	std::map<DWORD, ChannelInfo>::const_iterator iter = _mapChnl.find(Uid);
	if(iter == _mapChnl.end())
	{
		return FALSE;
	}
	
	ChannelInfo gotInfo = iter->second;
	
	
	ultoa( gotInfo.nodegroup, buffstr, 10);
	deviceid = buffstr;
	ultoa( gotInfo.ipport, buffstr, 10);
	port = buffstr;
	ipaddr = gotInfo.ipaddr;

	return TRUE;
}

//##ModelId=4147B84B0271
bool STVPlaylistManager::isValidFile(std::wstring fname)
{
	size_t slashidx=fname.find_last_of(L'\\');
	std::wstring realfile = fname.substr(slashidx+1, fname.length()-1-slashidx);
	//if(realfile.length()!=DB_FILE_NAMELENGTH)
	//	return false;
	//TODO: add other verify ways
	return true;
}

//##ModelId=4147B84B01F6
bool STVPlaylistManager::start()
{
	::CreateDirectory(_DBpath.c_str(),NULL);
	_IsRunning = TRUE;

	// restore configuration
	ZQ::common::XMLPrefDoc* confDoc = new ZQ::common::XMLPrefDoc (*_dbinit);
	std::wstring findstr = _DBpath+_T(DB_CONF_NAME);
	WIN32_FIND_DATA finddata;
	// search the database path for config file
	HANDLE hfind = ::FindFirstFile(findstr.c_str(), &finddata);
	if(hfind == INVALID_HANDLE_VALUE ) {	// not found
		glog(ZQ::common::Log::L_INFO, "NOTIFY   STVPlaylistManager::start()  No previews configuration file found");
		// TODO: should ask SM to send new configuration
		
	}
	else {	// found
		char fname[MAX_PATH];
		size_t dwCount = MAX_PATH;
		wcstombs(fname, findstr.c_str(), dwCount);
		
		// open existed file
		try{
		
			confDoc->open(fname);
			ZQ::common::IPreference* pConf = confDoc->root();
			ZQ::common::IPreference* pConfPref;
			if(!pConf || !(pConfPref = pConf->firstChild()) ) {
				glog(ZQ::common::Log::L_WARNING, "FAILURE  STVPlaylistManager::start()  Configuration file not correct");
				// TODO: should ask SM to send new configuration
			}
			else {
				if(OnConfigration(pConfPref, FALSE)!=STVSUCCESS) {
				// TODO: should ask SM to send new configuration
				}
				else {
					glog(ZQ::common::Log::L_INFO, "NOTIFY   STVPlaylistManager::start()  Configuration file had been successfully restored");
					_HasConfig = TRUE;
				}
				pConfPref->free();
				pConf->free();
			}
		}
		catch(ZQ::common::Exception excp) {
			glog(ZQ::common::Log::L_WARNING, "FAILURE  STVPlaylistManager::start()  Configuration file not correct, with error: %s", excp.getString());
			// TODO: should ask SM to send new configuration
		}
	}
	confDoc->close();
	delete confDoc;
	::FindClose(hfind);
	
#ifdef _DEBUG
	printf("Playlist Man started.\n");
#endif
	glog(ZQ::common::Log::L_INFO, "SUCCESS  STVPlaylistManager::start()  playlist manager started");

	_TimerMan->start();
	
	return TRUE;
}

//##ModelId=4147B84B01F7
bool STVPlaylistManager::terminate()
{
	_TimerMan->OnTerminate();
#ifdef _DEBUG
	printf("Begin wait for timer to end...\n");
#endif

	if(_TimerMan->isRunning()) {
		bool timerret=_TimerMan->wait(DEFAULT_SLEEPTIME*3);
#ifdef _DEBUG
		glog(ZQ::common::Log::L_INFO, L"STVPlaylistManager::terminate Timer already terminated with code %d",timerret);
#endif
	
	}
	_IsRunning = FALSE;
	

	SYSTEMTIME currtime;
	SYSTEMTIME listtime;
	::GetLocalTime(&currtime);
	
	// delete map
	std::map<DWORD, AELIST>::iterator iter;
	_mapAELISTMutex.enter();
	for(iter=_mapAELIST.begin(); iter!=_mapAELIST.end(); iter++) {
		if(iter->second.AELlist!=NULL) {
			delete [] (PAELEMENT)(iter->second.AELlist);
			iter->second.AELlist=NULL;
		}
		
	}
	_mapAELISTMutex.leave();

	// delete playlists
	for(; !_ListPool.empty(); ){
		STVPlaylist* exlist = _ListPool.back();
//		if(exlist->getPLStatus()==PLSTAT_PLAYING)
//			gSTV.OnShutdownStream(exlist);
		
		_ListPool.pop_back();

		listtime = exlist->getNextTime();
		if(timeComp(listtime, currtime)<0)
		{
			DWORD tspan = timeSub(currtime,listtime);
			if (tspan>exlist->getPLLength())
			{
				std::wstring findstr = exlist->getDBfile();
				::DeleteFile(findstr.c_str());
			}
			
		}
		

		glog(ZQ::common::Log::L_DEBUG, L"SUCCESS  STVPlaylistManager::terminate()  playlist %s deleted", exlist->getDBfile());
		if(exlist)
			delete exlist;
		

	}
	// delete Barker set
	for(; !_BarkerPool.empty(); ){
		STVPlaylist* exlist = _BarkerPool.back();
//		if(exlist->getPLStatus()==PLSTAT_PLAYING)
//			gSTV.OnShutdownStream(exlist);
		
		_BarkerPool.pop_back();

		if(exlist && !exlist->isInEffective(currtime))
		{
			std::wstring findstr = exlist->getDBfile();
		
			::DeleteFile(findstr.c_str());
		}
		
		
		glog(ZQ::common::Log::L_DEBUG, L"SUCCESS  STVPlaylistManager::terminate()  barker list %s deleted", exlist->getDBfile());
		if(exlist)
			delete exlist;
		
	}
	// delete filler set
	for(; !_SetPool.empty(); ){
		STVPlaylist* exlist = _SetPool.back();
//		if(exlist->getPLStatus()==PLSTAT_PLAYING)
//			gSTV.OnShutdownStream(exlist);
		
		_SetPool.pop_back();

		if(exlist && !exlist->isInEffective(currtime))
		{
			std::wstring findstr = exlist->getDBfile();
			::DeleteFile(findstr.c_str());
		}
		
		glog(ZQ::common::Log::L_DEBUG, L"SUCCESS  STVPlaylistManager::terminate()  filler list %s deleted", exlist->getDBfile());
		if(exlist)
			delete exlist;
		
	}
	// delete global filler set
	for(; !_GSetPool.empty(); ){
		STVPlaylist* exlist = _GSetPool.back();
		_GSetPool.pop_back();

		if(exlist && !exlist->isInEffective(currtime))
		{
			std::wstring findstr = exlist->getDBfile();
			::DeleteFile(findstr.c_str());
		}
		
		if(exlist)
			delete exlist;
	}

	_mapAELISTMutex.enter();
	_mapAELIST.clear();
	_mapAELISTMutex.leave();

	_ListMutex->enter();
	_ListPool.clear();
	_BarkerPool.clear();
	_SetPool.clear();
	_ListMutex->leave();

	ZQ::common::MutexGuard	tmpGd(_mapChnlMutex);
	_mapChnl.clear();
	
	//_mapStatus.clear();
#ifdef _DEBUG
	printf("Playlist Man terminated.\n");
#endif	
	glog(ZQ::common::Log::L_INFO, "SUCCESS  STVPlaylistManager::terminate()  playlist manager terminated");
	
	return FALSE;
}

int STVPlaylistManager::OnNewPlayList(ZQ::common::IPreference* pPortPref, ZQ::common::IPreference* pPlayListPref, int nType)
{	
	char buff[64];
	STVPlaylist* pNewPL = NULL;
	try
	{
		// get top&bottom asset no
		DWORD topasset=-1;
		DWORD bottomasset=-1;
		SYSTEMTIME	newtime;
		memset(&newtime, 0, sizeof(SYSTEMTIME));

		// prepare channel id
		DWORD tmpch=0;
		if(pPortPref) {
			pPortPref->get(XML_PORT_CHANNELID, buff);
			tmpch = atol(buff);
			if(pPortPref->has(XML_PORT_SUBCHANNELID)) {
				pPortPref->get(XML_PORT_SUBCHANNELID, buff);
				tmpch = tmpch*_MaxSubChnl+atol(buff);
			}
			else {
				tmpch = tmpch*_MaxSubChnl;
			}
		}
		else {
			tmpch = -1;
		}
		
		ZQ::common::IPreference* pTmpnode= pPlayListPref->firstChild(XML_PLAYLIST_ASSET);
		if(pTmpnode) {
			// get bottom no
			pTmpnode->get(XML_PLAYLIST_ASSET_NO, buff);
			bottomasset = atol(buff);
			// get time attribute
			if(nType == LISTTYPE_PLAYLIST) {	// get begin time
				if(pTmpnode->has(XML_PLAYLIST_ASSET_BEGIN)) {
					pTmpnode->get(XML_PLAYLIST_ASSET_BEGIN, buff);
					
					std::string buffstr = buff;
					newtime.wYear			= atol(buffstr.substr(0,4).c_str());
					newtime.wMonth			= atol(buffstr.substr(4,2).c_str());
					newtime.wDay			= atol(buffstr.substr(6,2).c_str());
					newtime.wHour			= atol(buffstr.substr(9,2).c_str());
					newtime.wMinute			= atol(buffstr.substr(12,2).c_str());
					newtime.wSecond			= atol(buffstr.substr(15,2).c_str());
					newtime.wDayOfWeek		= 7;	// don't calculate weekdays
					newtime.wMilliseconds	= 0;	// don't care about milli seconds
					
				}
			}
			else {	// get effective time
				if(pPlayListPref->has(XML_PLAYLIST_EFFECTTIME)) {
					pPlayListPref->get(XML_PLAYLIST_EFFECTTIME, buff);

					std::string buffstr = buff;
					newtime.wYear			= atol(buffstr.substr(0,4).c_str());
					newtime.wMonth			= atol(buffstr.substr(4,2).c_str());
					newtime.wDay			= atol(buffstr.substr(6,2).c_str());
					newtime.wHour			= atol(buffstr.substr(9,2).c_str());
					newtime.wMinute			= atol(buffstr.substr(12,2).c_str());
					newtime.wSecond			= atol(buffstr.substr(15,2).c_str());
					newtime.wDayOfWeek		= 7;	// don't calculate weekdays
					newtime.wMilliseconds	= 0;	// don't care about milli seconds
				}
			}
			
			// search for last asset element NO
			for(; pPlayListPref->hasNextChild(); pTmpnode = pPlayListPref->nextChild()) {
				pTmpnode->free();
			}
			
			ZQ::common::IPreference* pElem = pTmpnode->firstChild(XML_PLAYLIST_ASSET_AE);
			if(!pElem) {	// no element, so get asset NO instead
				pTmpnode->get(XML_PLAYLIST_ASSET_NO, buff);
				topasset = atol(buff);
			}
			else {	// has element, so search last element to get NO
				for(; pTmpnode->hasNextChild(); pElem = pTmpnode->nextChild()) {
					pElem->free();	
				}
				pElem->get(XML_PLAYLIST_ASSET_AE_NO, buff);
				topasset = atol(buff);
				pElem->free();
			}
			pTmpnode->free();
		}

		if(bottomasset==-1 || topasset==-1) {
#ifdef _DEBUG
			printf("Invalid playlist. error string: %s\n", g_errInfo[STVINVALIDPL]);
#endif
			glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::OnNewPlayList()  Invalid playlist. error string: %s", g_errInfo[STVINVALIDPL]);
			return STVINVALIDPL;
		}

		// delete conflict playlist if this is a modification playlist
		int puriRet=purify(tmpch, bottomasset, topasset, newtime, nType);

		// duplicate playlist with smaller NO, ignore it
		if(puriRet==-1) {
			glog(ZQ::common::Log::L_WARNING, "NOTIFY   STVPlaylistManager::OnNewPlayList()  This playlist is a duplicate playlist with smaller NO, ignore!");
			return STVSUCCESS;
		}

		// create new playlist
		pNewPL = createList(NULL);
		int retval = pNewPL->create(pPortPref, pPlayListPref, nType);
		if(retval!=STVSUCCESS) {
#ifdef _DEBUG
	printf("Invalid playlist. error string: %s\n", g_errInfo[retval-STVERROR]);
#endif
			glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::OnNewPlayList()  Playlist can not be created. error string: %s", g_errInfo[retval-STVERROR]);
			return retval;
		}

		// register
		if(!reg(pNewPL)) {
#ifdef _DEBUG
			printf("Invalid playlist. error string: %s\n", g_errInfo[STVINVALIDPL]);
#endif
			glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::OnNewPlayList()  Invalid playlist. error string: %s", g_errInfo[STVINVALIDPL]);
			return STVINVALIDPL;
		}
	
		// wake up timer to update time point
		_TimerMan->OnWakeup();
	#ifdef _DEBUG
		printf("New playlist created according to SM interface.\n");
	#endif
		switch(nType) {
		case LISTTYPE_PLAYLIST:
			glog(ZQ::common::Log::L_INFO, "SUCCESS  STVPlaylistManager::OnNewPlayList()  New playlist created according to SM interface");
			break;
		case LISTTYPE_FILLER:
			glog(ZQ::common::Log::L_INFO, "SUCCESS  STVPlaylistManager::OnNewPlayList()  New filler set created according to SM interface");
			break;
		case LISTTYPE_BARKER:
			glog(ZQ::common::Log::L_INFO, "SUCCESS  STVPlaylistManager::OnNewPlayList()  New barker playlist created according to SM interface");
			break;
		default:
			break;
		}
		
	}
	catch (ZQ::common::Exception excp) {
#ifdef _DEBUG
		printf("Error(P&PM): Can not create new playlist.  Detail:%s\n", excp.getString());
#endif
		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::OnNewPlayList()  Can not create new playlist.  Detail:%s\n", excp.getString());
		return STVINVALIDPL;
	}
	catch(...) {
		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::OnNewPlayList()  Can not create new playlist.");
		return STVINVALIDPL;
	}

	return STVSUCCESS;
}

int STVPlaylistManager::OnPlayAssetImmediately(ZQ::common::IPreference *pPortPref, ZQ::common::IPreference *pPlayListPref)
{
#ifdef _DEBUG
	printf("STVPlaylistManager::OnPlayAssetImmediately()  command arrived.\n");
#endif
	glog(ZQ::common::Log::L_DEBUG, "PENDING  STVPlaylistManager::OnPlayAssetImmediately()  command arrived");
	// TODO:
	return STVSUCCESS;
}

int STVPlaylistManager::OnPlayImmediately(STVPlaylist* pList, DWORD ChnlID)
{
	HRESULT err=OnIDSquery(pList);
	if(err!= STVSUCCESS) {
		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::OnPlayImmediately()  Can not compose AEList");	
		return err;
	}
	
	pList->setPLStatus(PLSTAT_ASSOCIATED);
	err=gSTV.OnStartStream(pList, ChnlID);
	if(err!= STVSUCCESS) {
		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::OnPlayImmediately()  Can not start stream");	
		return err;
	}
	else {	
		glog(ZQ::common::Log::L_DEBUG, L"SUCCESS  STVPlaylistManager::OnPlayImmediately()  Ready for playing effective list %s", pList->getDBfile());
		return STVSUCCESS;
	}
}

int STVPlaylistManager::OnPlayFillerImmediately(ZQ::common::IPreference *pPortPref)
{
#ifdef _DEBUG
	printf("STVPlaylistManager::OnPlayFillerImmediately()  command arrived.\n");
#endif
	char buff[16];
	DWORD purID = 0;
	if(pPortPref->has(XML_PORT_CHANNELID)) {
		pPortPref->get(XML_PORT_CHANNELID, buff);
		purID = atol(buff)*_MaxSubChnl;
	}
	if(pPortPref->has(XML_PORT_SUBCHANNELID)) {
		pPortPref->get(XML_PORT_SUBCHANNELID, buff);
		purID += atol(buff);
	}

	std::map<DWORD, ChannelInfo>::iterator iter = _mapChnl.find(purID);
	if(iter != _mapChnl.end()) {
		iter->second.validation = FALSE;
	}
	else {
		glog(ZQ::common::Log::L_WARNING, "FAILURE  STVPlaylistManager::OnPlayFillerImmediately()  No channel with ID %d found", purID);
		return STVINVALIDCHNL;
	}

	// get current effective filler set
	std::vector<STVPlaylist*> fillerlist;
	if(!queryByChnl(pPortPref, fillerlist, LISTTYPE_FILLER)) {
		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::OnPlayFillerImmediately()  No filler set in this channel");	
		return STVINVALIDCHNL;
	}
	STVPlaylist* pFiller = getEffectiveSet(fillerlist);
	if(!pFiller) {
		glog(ZQ::common::Log::L_WARNING, "FAILURE  STVPlaylistManager::OnPlayFillerImmediately()  No filler set effective currently in this channel");	
		return STVINVALIDCHNL;
	}

	// play filler
	HRESULT err=OnIDSquery(pFiller);
	if(err!= STVSUCCESS) {
		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::OnPlayFillerImmediately()  Can not compose AEList");	
		return err;
	}
	
	pFiller->setPLStatus(PLSTAT_ASSOCIATED);
	err=gSTV.OnStartStream(pFiller, purID);
	if(err!= STVSUCCESS) {
		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::OnPlayFillerImmediately()  Can not start stream");	
		return err;
	}
	else {	
		glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPlaylistManager::OnPlayFillerImmediately()  Playing effective filler set");
		return STVSUCCESS;
	}
	
}

int STVPlaylistManager::OnPlaySkipToAsset(ZQ::common::IPreference *pPortPref, ZQ::common::IPreference *pPlayListPref)
{
#ifdef _DEBUG
	printf("STVPlaylistManager::OnPlaySkipToAsset()  command arrived.\n");
#endif
	glog(ZQ::common::Log::L_DEBUG, "PENDING  STVPlaylistManager::OnPlaySkipToAsset()  command arrived");
	// TODO:
	return STVSUCCESS;
}

int STVPlaylistManager::OnHoldAsset(ZQ::common::IPreference *pPortPref, ZQ::common::IPreference* pAssetPref)
{
#ifdef _DEBUG
	printf("STVPlaylistManager::OnHoldAsset()  command arrived.\n");
#endif
	glog(ZQ::common::Log::L_DEBUG, "PENDING  STVPlaylistManager::OnHoldAsset()  command arrived");
	// TODO:
	return STVSUCCESS;
}

int STVPlaylistManager::OnChannelStartup(ZQ::common::IPreference* pPortPref)
{
	int retval = STVSUCCESS;
#ifdef _DEBUG
	printf("STVPlaylistManager::OnChannelStartup()  command arrived.\n");
#endif
	char buff[16];
	
	// set validation
	DWORD purID = 0;
	if(pPortPref->has(XML_PORT_CHANNELID)) {
		pPortPref->get(XML_PORT_CHANNELID, buff);
		purID = atol(buff)*_MaxSubChnl;
	}
	if(pPortPref->has(XML_PORT_SUBCHANNELID)) {
		pPortPref->get(XML_PORT_SUBCHANNELID, buff);
		purID += atol(buff);
	}

	std::map<DWORD, ChannelInfo>::iterator iter = _mapChnl.find(purID);
	if(iter != _mapChnl.end()) {
		iter->second.validation = TRUE;
	}
	else {
		glog(ZQ::common::Log::L_WARNING, "FAILURE  STVPlaylistManager::OnChannelStartup()  No such channel with purchase ID %d", purID);
		return STVINVALIDCHNL;
	}
	
//	// query current playlist
//	int tmpListType;
//	STVPlaylist* pCurrPL = queryCurrentPL(purID, tmpListType);
//	if(!pCurrPL) {
//		// no stream right now in this channel, so play filler
//		int tmpret=OnPlayFillerImmediately(pPortPref);
//		if(tmpret!=STVSUCCESS) {
//			glog(ZQ::common::Log::L_DEBUG, "FAILURE  STVPlaylistManager::OnChannelStartup()  Some error happens on channel with purchase ID %d", purID);
//			retval = tmpret;
//		}
//		else {
//			glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPlaylistManager::OnChannelStartup()  Channel with purchase ID %d started", purID);
//		}
//	}

	glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPlaylistManager::OnChannelShutdown()  Channel with purchase ID %d started up", purID);
	return retval;
//	return STVINVALIDCHNL;
}

int STVPlaylistManager::OnChannelShutdown(ZQ::common::IPreference* pPortPref)
{
	int retval = STVSUCCESS;
#ifdef _DEBUG
	printf("STVPlaylistManager::OnChannelShutdown()  command arrived.\n");
#endif
	
	char buff[16];
	
	// set validation
	DWORD purID = 0;
	if(pPortPref->has(XML_PORT_CHANNELID)) {
		pPortPref->get(XML_PORT_CHANNELID, buff);
		purID = atol(buff)*_MaxSubChnl;
	}
	if(pPortPref->has(XML_PORT_SUBCHANNELID)) {
		pPortPref->get(XML_PORT_SUBCHANNELID, buff);
		purID += atol(buff);
	}

	std::map<DWORD, ChannelInfo>::iterator iter = _mapChnl.find(purID);
	if(iter != _mapChnl.end()) {
		iter->second.validation = FALSE;
	}
	else {
		glog(ZQ::common::Log::L_WARNING, "FAILURE  STVPlaylistManager::OnChannelShutdown()  No channel with purchase ID %d found", purID);
		return STVINVALIDCHNL;
	}

	// query current playlist
	int tmpListType;
	STVPlaylist* pCurrPL = queryCurrentPL(purID, tmpListType);
	if(pCurrPL) {
		// A stream is right now in this channel, so tear down it
		pCurrPL->setPLStatus(PLSTAT_IDLE);
		int tmpret= gSTV.OnShutdownStream(pCurrPL, purID);
		if(tmpret!=STVSUCCESS) {
			glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::OnChannelShutdown()  Some error happens on channel with purchase ID %d", purID);
			retval = tmpret;
		}
		else {
			glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPlaylistManager::OnChannelShutdown()  Channel with purchase ID %d shutted down, 1 stream had been terminated", purID);
		}
	}
	else {
		glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPlaylistManager::OnChannelShutdown()  Channel with purchase ID %d shutted down", purID);
	}
	return retval;
}

int STVPlaylistManager::OnConfigration(ZQ::common::IPreference* pConfigPref, bool dump/*=TRUE*/)
{
	char buff[MAX_PATH];
	bool errormsg=FALSE;
#ifdef _DEBUG
	printf("STVPlaylistManager::OnConfigration()  command arrived.\n");
#endif
 	glog(ZQ::common::Log::L_DEBUG, "PENDING  STVPlaylistManager::OnConfigration()  command arrived");
	// clear channels
	bool firsttime=FALSE;
	if(_mapChnl.empty())
		firsttime=TRUE;		// if it is the first time, all channels are shut down as default
	_mapChnl.clear();
	
	// store channels
	for(ZQ::common::IPreference* pPort = pConfigPref->firstChild(); pPort; pPort = pConfigPref->nextChild()) {
		std::string tagname = pPort->name(buff);
		DWORD	Uid=0;
		ChannelInfo newInfo;
		if(tagname == XML_PORT) {
			// no sub-channels, calculate unique channel Uid and stores information
			
			// channel id
			if(!pPort->has(XML_PORT_CHANNELID)) {
				errormsg = TRUE;
				break;
			}
			pPort->get(XML_PORT_CHANNELID, buff);
			Uid = atol(buff);

			// sub-channel id
			Uid = Uid*_MaxSubChnl;

			// node-group
			if(!pPort->has(XML_PORT_DEVICEID)) {
				errormsg = TRUE;
				break;
			}
			pPort->get(XML_PORT_DEVICEID, buff);
			newInfo.nodegroup = atol(buff);
			//Uid = Uid*_MaxSubChnl + atol(buff);

			// ip
			if(!pPort->has(XML_PORT_IPADRESS)) {
				errormsg = TRUE;
				break;
			}
			pPort->get(XML_PORT_IPADRESS, buff);
			strcpy(newInfo.ipaddr, buff);

			// port
			if(!pPort->has(XML_PORT_IPPORT)){
				errormsg = TRUE;
				break;
			}
			pPort->get(XML_PORT_IPPORT, buff);
			newInfo.ipport = atol(buff);

			// validation
//			newInfo.validation = (firsttime)?FALSE:TRUE;
			newInfo.validation = TRUE;
			
			// status
			newInfo.status = CHNL_NONE;

			// current list
			newInfo.pCurrList = NULL;

			std::pair<DWORD, ChannelInfo> entry(Uid, newInfo);
			
			ZQ::common::MutexGuard	tmpGd(_mapChnlMutex);
			std::map<DWORD, ChannelInfo>::iterator iter = _mapChnl.find(Uid);
			if(iter!= _mapChnl.end()) {
				// already existed
				_mapChnl.erase(iter);
			}
			_mapChnl.insert(entry);
			

		}
		else if(tagname == XML_CHANNEL) {
			// there are sub-channels
			
			for(ZQ::common::IPreference* pSubPort = pPort->firstChild(XML_PORT); pSubPort; pSubPort = pPort->nextChild()) {
				DWORD subUid = 0;
				ChannelInfo subnewInfo;
				// channel id
				if(!pPort->has(XML_PORT_CHANNELID)){
					errormsg = TRUE;
					break;
				}
				pPort->get(XML_PORT_CHANNELID, buff);
				subUid = atol(buff);

				// sub-channel id
				if(!pSubPort->has(XML_PORT_SUBCHANNELID)){
					errormsg = TRUE;
					break;
				}
				pSubPort->get(XML_PORT_SUBCHANNELID, buff);
				subUid = subUid*_MaxSubChnl + atol(buff);

				// node-group
				if(!pSubPort->has(XML_PORT_DEVICEID)){
					errormsg = TRUE;
					break;
				}
				pSubPort->get(XML_PORT_DEVICEID, buff);
				subnewInfo.nodegroup = atol(buff);
				//subUid = subUid*_MaxSubChnl + atol(buff);

				// ip
				if(!pSubPort->has(XML_PORT_IPADRESS)){
					errormsg = TRUE;
					break;
				}
				pSubPort->get(XML_PORT_IPADRESS, buff);
				strcpy(subnewInfo.ipaddr, buff);

				// port
				if(!pSubPort->has(XML_PORT_IPPORT)){
					errormsg = TRUE;
					break;
				}
				pSubPort->get(XML_PORT_IPPORT, buff);
				subnewInfo.ipport = atol(buff);

				// validation
//				subnewInfo.validation = (firsttime)?FALSE:TRUE;
				subnewInfo.validation = TRUE;
				
				// status
				subnewInfo.status = CHNL_NONE;

				// current list
				subnewInfo.pCurrList = NULL;

				std::pair<DWORD, ChannelInfo> subentry(subUid, subnewInfo);
				
				ZQ::common::MutexGuard	tmpGd(_mapChnlMutex);
				std::map<DWORD, ChannelInfo>::iterator iter = _mapChnl.find(subUid);
				if(iter!= _mapChnl.end()) {
					// already existed
					_mapChnl.erase(iter);
				}
				_mapChnl.insert(subentry);

				pSubPort->free();
			}
		}
		pPort->free();
	}
	if(errormsg) {
		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::OnConfigration()  configuration format not corrent!");
		return STVINVALIDCHNL;
	}
	
	if(dump) {	// should dump to disk file
		// create file
		ZQ::common::XMLPrefDoc* confDoc = new ZQ::common::XMLPrefDoc(*_dbinit);
		std::wstring wfname = _DBpath+_T(DB_CONF_NAME);
		char fname[MAX_PATH];
		size_t dwCount = MAX_PATH;
		wcstombs(fname, wfname.c_str(), dwCount);	// convert to ascii
//		printf("Test config\n");
//		HANDLE hf=::CreateFile(wfname.c_str(), GENERIC_WRITE|GENERIC_READ, 0, NULL, CREATE_ALWAYS, 0, NULL);	// create new file
//		if(INVALID_HANDLE_VALUE ==hf) {
//			::CloseHandle(hf);
//			glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::OnConfigration()  Can not create config file: %s", fname);
//		}
//		else {
//			::CloseHandle(hf);
			glog(ZQ::common::Log::L_DEBUG, "NOTIFY   STVPlaylistManager::OnConfigration()  Should update config file: %s", fname);
			confDoc->open(fname, XMLDOC_CREATE);
			ZQ::common::IPreference* pConf = confDoc->newElement(XML_DBCONFIG);
			confDoc->set_root(pConf);	// set root tag
			pConf->addNextChild(pConfigPref);	// add child
			if(confDoc->save())
				_HasConfig = TRUE;
			confDoc->close();
			pConf->free();
//		}
		delete confDoc;
	}

#ifdef _DEBUG
	printf("Configuration saved.\n");
#endif
	glog(ZQ::common::Log::L_INFO, "SUCCESS  STVPlaylistManager::OnConfigration()  Configuration saved");

	return STVSUCCESS;
}

int STVPlaylistManager::OnConnected(char* backstream, DWORD* backcount)
{
	// TODO: log
	if(_HasConfig) {
		strcpy(backstream, "<Connect Flag=\"0\"/>");
	}
	else {
		strcpy(backstream, "<Connect Flag=\"1\"/>");
		gSTV.getSMConnector()->SendEnquireConfig();
	}
	*backcount = 17;

	return STVSUCCESS;
}

int STVPlaylistManager::OnIDSquery(STVPlaylist* pl)
{

	AELIST AES;
	ASSETS	playlistAS;
	ASSETS	fillerAS;
	PASSETLIST filledList;
	int retval1,retval2;

#ifdef _DEBUG
	printf("STVPlaylistManager::OnIDSquery()  command arrived.\n");
#endif
	if(pl->isGlobal() && _gAEList.pGFiller==pl) {
		// no need to update IDS
		glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPlaylistManager::OnIDSquery()  global AEList existed");
		return STVSUCCESS;
	}
	
	glog(ZQ::common::Log::L_DEBUG, "PENDING  STVPlaylistManager::OnIDSquery()  Begin AEList construct");
	
	playlistAS.clear();
	fillerAS.clear();
	filledList.clear();
	retval1=retval2=STVSUCCESS;
	//////////////////////////////////////////////////////////////////////////
	// compute playlist AE count and bitrate
	retval1 = pl->OnPLIdsSession(&playlistAS);
	
	if(retval1!=STVSUCCESS) {
		glog(ZQ::common::Log::L_ERROR, L"FAILURE  STVPlaylistManager::OnIDSquery()  Can not make up AEList of playlist %s",pl->getDBfile());
		return retval1;
	}
	glog(ZQ::common::Log::L_DEBUG, L"SUCCESS  STVPlaylistManager::OnIDSquery()  AE of playlist %s constructed",pl->getDBfile());
	//////////////////////////////////////////////////////////////////////////
	//printf("list type: %d\n", pl->getPLType());
	if(pl->getPLType()==LISTTYPE_PLAYLIST) {
	
//		// compute filler list AE count and bitrate
//		std::vector<STVPlaylist*>	currfiller;
//		queryByChnl(pl->getPLchannel(), currfiller, LISTTYPE_FILLER);
//		STVPlaylist* qualiSet = getEffectiveSet(currfiller);
//
//		if(qualiSet) {	// effective filler exist
//			glog(ZQ::common::Log::L_DEBUG, "NOTIFY   STVPlaylistManager::OnIDSquery()  Use filler set %d", qualiSet);
//			retval2 = qualiSet->OnPLIdsSession(&fillerAS);
//			if(retval2!=STVSUCCESS) {
//				glog(ZQ::common::Log::L_DEBUG, "FAILURE  STVPlaylistManager::OnIDSquery()  Can not make up AEList");
//				return retval2;
//			}
//			// fill
//			_autoF->setFillers(&fillerAS);
//			_autoF->fill(&filledList, _FillLength, qualiSet->getFillType());
//			glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPlaylistManager::OnIDSquery()  Filler set %d process completed", qualiSet);
//			
//			///debug
//			PAELEMENT paele = (PAELEMENT)_mapAELIST[pl->getPLchannel()].AELlist;
//			for(int i=0; i<_mapAELIST[pl->getPLchannel()].AECount; i++) {
//				printf("%x\n", paele[i].AEUID);
//			}
//		}
//		else {
//			glog(ZQ::common::Log::L_DEBUG, "WARNING  STVPlaylistManager::OnIDSquery()  Currently no filler set available");
//		}
//		composeAE(pl, &AES, &playlistAS, &filledList);
		composeAE(pl, &AES, &playlistAS);
		
	}

	else if(pl->getPLType()==LISTTYPE_FILLER) {
		// pure filler, fill it and compose
		_autoF->setFillers(&playlistAS);
		_autoF->fill(&filledList, _FillLength, pl->getFillType());
		composeAE(pl, &AES, &filledList);
	}
	
	else if(pl->getPLType()==LISTTYPE_BARKER) {
		// barker, fill it and compose
		_barkerF->setFillers(&playlistAS);
		_barkerF->fill(&filledList, _FillLength, pl->getFillType());
		composeAE(pl, &AES, &filledList);
	}
	
	playlistAS.clear();
	fillerAS.clear();
	filledList.clear();
	
	if(retval1!=STVSUCCESS) {
		//printf("ret1 error\n");
		return retval1;
	}
	else if(retval2!=STVSUCCESS) {
		//printf("ret2 error\n");
		return retval2;
	}
	else {
#ifdef _DEBUG
	printf("Asset Elements ready.\n");
#endif
	glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPlaylistManager::OnIDSquery()  AEList ready");
		return STVSUCCESS;
	}
}

int STVPlaylistManager::OnGetAElist(DWORD chnlID, AELIST& OutAEList)
{
	bool bSuc = FALSE;
	bool bValidChnl = FALSE;
	bool bIsGlobalAE = FALSE;

#ifdef _DEBUG
	printf("STVPlaylistManager::OnGetAElist()  command arrived.\n");
#endif
	glog(ZQ::common::Log::L_DEBUG, "PENDING  STVPlaylistManager::OnGetAElist()  command arrived");
	
	// check if channel existed
//	ZQ::common::MutexGuard	tmpGd(_mapChnlMutex);

	std::map<DWORD, ChannelInfo>::const_iterator chnlIter = _mapChnl.find(chnlID);
	if(chnlIter == _mapChnl.end())
	{
		bValidChnl = FALSE;
	}
	else
	{
		bValidChnl = TRUE;
		if(chnlIter->second.status <= CHNL_NPLAYING)	// normal list
			bIsGlobalAE = FALSE;
		else	// global filler
			bIsGlobalAE = TRUE;
	}

	// get aelist for this channel
	_mapAELISTMutex.enter();

	std::map<DWORD, AELIST>::const_iterator iter = _mapAELIST.find(chnlID);
	
	if(iter== _mapAELIST.end()) {
		bSuc = FALSE;
	}
	else 
	{
		if(iter->second.AELlist==NULL) {
			bSuc = FALSE;
		}
		else {
			bSuc = TRUE;
			OutAEList = iter->second;
		}
	}

	_mapAELISTMutex.leave();
	
	// if no list for it, search global AEList
	if(bValidChnl==TRUE && bSuc==FALSE && bIsGlobalAE==TRUE && _gAEList.gAE.AELlist!=NULL) {
		bSuc = TRUE;
		OutAEList = _gAEList.gAE;
		glog(ZQ::common::Log::L_DEBUG, "NOTIFY   STVPlaylistManager::OnGetAElist()  Use global AEList for channel %d", chnlID);
	}
	
	
	if(bSuc) {
		glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPlaylistManager::OnGetAElist()  AEList sent");
		return STVSUCCESS;
	}
	else {
		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::OnGetAElist()  No AEList available for channel %d", chnlID);
		return STVINVALIDCHNL;
	}
}

int STVPlaylistManager::OnFreeAElist(DWORD chnlID)
{
#ifdef _DEBUG
	printf("STVPlaylistManager::OnFreeAElist()  command arrived.\n");
#endif

	_mapAELISTMutex.enter();

	std::map<DWORD, AELIST>::iterator iter = _mapAELIST.find(chnlID);
	if(iter== _mapAELIST.end()) {
//		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::OnFreeAElist()  No AEList associated with channel %d", chnlID);
		_mapAELISTMutex.leave();
		return STVIDSERROR;
	}
	// free the Array of element
	if(iter->second.AELlist) {
		delete [] (PAELEMENT)iter->second.AELlist;
		iter->second.AELlist = NULL;
	}

	_mapAELISTMutex.leave();

	return STVSUCCESS;
}

int STVPlaylistManager::OnSetStreamStatus(DWORD chnlID, SSAENotification notiMsg, char* backstream, DWORD* backcount)
{
	bool bShoudUpdateStat = FALSE;
	// update channel status
	{	
		ZQ::common::MutexGuard	tmpGd(_mapChnlMutex);

		std::map<DWORD, ChannelInfo>::iterator iter = _mapChnl.find(chnlID);
		if(iter == _mapChnl.end()) {
			glog(ZQ::common::Log::L_INFO, "FAILURE  STVPlaylistManager::OnSetStreamStatus()  No such channel with purchaseID %d", chnlID);
			return STVINVALIDCHNL;
		}

		if(notiMsg.dwAeUID == UNKNOWN_UID && iter->second.StreamStatus.wOperation == SAENO_PLAY)
		{	// if it is reported by ISS StateChange Callback, hence no AeUID available, set its AeUID first
			notiMsg.dwAeUID = iter->second.StreamStatus.dwAeUID;
			bShoudUpdateStat = TRUE;
		}
		else if(notiMsg.dwAeUID == UNKNOWN_UID && iter->second.StreamStatus.wOperation!=SAENO_PLAY)
		{	// if an out-of-date report, skip it
			*backcount = 0;
			return STVSUCCESS;
		}
		
		iter->second.StreamStatus.dwPurchaseID  = chnlID;
		iter->second.StreamStatus.dwAeUID		= notiMsg.dwAeUID;
		iter->second.StreamStatus.dwStatus		= notiMsg.dwStatus;
		iter->second.StreamStatus.wOperation	= notiMsg.wOperation;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// search for this playlist

	STVPlaylist* pCurrPL = NULL;
	int currtype = -1;

	switch(getChnlState(chnlID)) {
	case CHNL_NONE:
		pCurrPL = NULL;
		break;
	case CHNL_NSTARTING:
		// 1. try list that is starting
		pCurrPL=queryCurrentPL(chnlID, currtype, PLSTAT_ALLOCATING);
		break;
	case CHNL_NSTOPPING:
		// 2. try list that is dying
		pCurrPL = queryCurrentPL(chnlID, currtype, PLSTAT_DESTROYING);
		bShoudUpdateStat = TRUE;	// dying stream, also update status
		break;
	case CHNL_NPLAYING:
		// 3. try list that is playing
		pCurrPL = queryCurrentPL(chnlID, currtype, PLSTAT_PLAYING);
		break;
	case CHNL_GSTARTING:
		// 4. try global filler that is starting
		pCurrPL = getCurrGlobalFiller();
		currtype = LISTTYPE_FILLER;
		break;
	case CHNL_GSTOPPING:
		// 5. try global filler that is dying
		pCurrPL = getCurrGlobalFiller();
		currtype = LISTTYPE_FILLER;
		bShoudUpdateStat = TRUE;	// dying stream, also update status
		break;
	case CHNL_GPLAYING:
		// 6. try global filler that is playing
		pCurrPL = getCurrGlobalFiller();
		currtype = LISTTYPE_FILLER;
		break;
	default :
		break;
	}
	
//	pCurrPL		= getChnlCurrList(chnlID);
//	currtype	= pCurrPL->getPLType();
	
	if(!pCurrPL || currtype==-1) {
#ifdef _DEBUG
		printf("Currently no stream related with this channel.\n");
#endif
		glog(ZQ::common::Log::L_DEBUG, "NOTIFY   STVPlaylistManager::OnSetStreamStatus()  Currently no stream related with this channel( ID = %d)", chnlID);
		*backcount = 0;
		return STVSUCCESS;
	}

	//////////////////////////////////////////////////////////////////////////
	// update playlist status if necessary
	if(bShoudUpdateStat)
	{
		// channel status
		setChnlState(chnlID, CHNL_NONE);

		// channel current list
//		setChnlCurrList(chnlID, NULL);

		// list status
		switch(pCurrPL->getPLType()) {
		case LISTTYPE_PLAYLIST:
			pCurrPL->setPLStatus(PLSTAT_IDLE);
			break;
		case LISTTYPE_FILLER:
			pCurrPL->setPLStatus(PLSTAT_IDLE);
			break;
		case LISTTYPE_BARKER:
			pCurrPL->setPLStatus(PLSTAT_IDLE);
			break;
		}
	}
	

	//////////////////////////////////////////////////////////////////////////
	// format feedback
	glog(ZQ::common::Log::L_DEBUG, "NOTIFY   STVPlaylistManager::OnSetStreamStatus()  AE status on channel %d changed:", chnlID);
	glog(ZQ::common::Log::L_DEBUG, "NOTIFY   \tAsset Element: %x", notiMsg.dwAeUID);
	glog(ZQ::common::Log::L_DEBUG, "NOTIFY   \tOperation: %d", notiMsg.wOperation);
	glog(ZQ::common::Log::L_DEBUG, "NOTIFY   \tStatus: %d", notiMsg.dwStatus);

	// update ae status
	if(currtype==LISTTYPE_PLAYLIST) {
		// playlist, should update current playing element
		if(!pCurrPL->updatePlayingElemEx(notiMsg, backstream, backcount)) {
	#ifdef _DEBUG
			printf("Unable to update playlist cursor.\n");
	#endif
			glog(ZQ::common::Log::L_DEBUG, L"FAILURE  STVPlaylistManager::OnSetStreamStatus()  Unable to update playlist %s cursor", pCurrPL->getDBfile());
			return STVINVALIDCHNL;
		}
	}
	else {
		// filler or barker, no NOT need update current playing element
		if(!pCurrPL->updatePlayingElem(notiMsg, backstream, backcount)) {
			glog(ZQ::common::Log::L_ERROR, L"FAILURE  STVPlaylistManager::OnSetStreamStatus()  Unable to update fillerlist/barkerlist %s cursor", pCurrPL->getDBfile());
			return STVINVALIDCHNL;
		}
	}
#ifdef _DEBUG
	printf("Status feedback saved.\n");
#endif
	glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPlaylistManager::OnSetStreamStatus()  Status feedback updated");
	return STVSUCCESS;
}

//##ModelId=4147B84B0280
int STVPlaylistManager::timeComp(SYSTEMTIME t1, SYSTEMTIME t2)
{
	WORD* p1 = (WORD*)&t1;
	WORD* p2 = (WORD*)&t2;
	WORD curr1 = *p1;
	WORD curr2 = *p2;

	// compare year and month
	for(int i=0; i<2; i++) {
		curr1 = *(p1+i);
		curr2 = *(p2+i);
		if(curr1 < curr2)	
			return -1;
		else if(curr1 > curr2)
			return 1;
	}

	// compare day, hour, min, and sec
	for(i=3; i<7; i++) {
		curr1 = *(p1+i);
		curr2 = *(p2+i);
		if(curr1 < curr2)	
			return -1;
		else if(curr1 > curr2)
			return 1;

	}
	return 0;
}

//##ModelId=4147B84B0291
unsigned long STVPlaylistManager::timeSub(SYSTEMTIME t1, SYSTEMTIME t2)
{
	unsigned long retval = 0xFFFFFFFF;
	unsigned long factor;

	WORD* p1 = (WORD*)&t1;
	WORD* p2 = (WORD*)&t2;
	WORD curr1 = *p1;
	WORD curr2 = *p2;

	int retcp = timeComp(t1,t2);
	// return if t1 is earlier than t2 or t1 equals t2
	
	if( retcp<0 )
		return retval;
	else if ( retcp ==0 )
		return 0;

	// if difference greater than 1 month, return 0xFFFFFFFF
	for(int i=0; i<2; i++) {
		curr1 = *(p1+i);
		curr2 = *(p2+i);
		if(curr1 != curr2) {
			retval = 0xFFFFFFFF;
			return retval;
		}
	}

	retval = 0;
	// calculate difference
	for(i=3; i<7; i++) {
		curr1 = *(p1+i);
		curr2 = *(p2+i);
		
		switch(i) {
		case 3:
			factor = 3600*24;
			break;
		case 4:
			factor = 3600;
			break;
		case 5:
			factor = 60;
			break;
		case 6:
			factor = 1;
			break;
		}

		retval += (curr1-curr2)*factor;
	}
	return retval;
}

//bool STVPlaylistManager::initIdsSession()
//{
//	wchar_t ipaddr[16];
//	wchar_t username[32];
//	// get IDS server address and user name, maybe from registry
//	// TODO:
//	wcscpy(ipaddr, L"192.168.12.12");
//	wcscpy(username, L"ITV");
//
//	_Builder = new IdsSession();
//	return _Builder->initialize(ipaddr, username);
//}

STVPlaylist* STVPlaylistManager::getEffectiveSet(std::vector<STVPlaylist*> inlist)
{
	STVPlaylist* pRet = NULL;
	SYSTEMTIME currentt;
	::GetLocalTime(&currentt);
	for(int i=0; i<inlist.size(); i++) {
		if( (inlist[i]->isInEffective(currentt)) && (inlist[i]->getPLType()!=LISTTYPE_PLAYLIST) ) {
			if(pRet) {
				if(timeComp(inlist[i]->getNextTime(), pRet->getNextTime())>0)
					pRet = inlist[i];
			}
			else {
				pRet = inlist[i];
			}
		}
	}
	return pRet;
}

bool STVPlaylistManager::composeAE(STVPlaylist* pl, PAELIST pAE, PASSETS pASp)
{
	glog(ZQ::common::Log::L_DEBUG, "PENDING  STVPlaylistManager::composeAE()  begin composing AE, type 1");
	PAELEMENT	pAElements;

	LONG	maxBitRate	=0;
	LONG	countAE		=0;
	LONG	indxAE	=0;
	int i,j;
	//////////////////////////////////////////////////////////////////////////
	// compute total asset element number
	try
	{
		for(i=0; i<pASp->size(); i++) {
			for(j=0; j<pASp->at(i).AssetElements.size(); j++) {
				countAE++;
				if(pASp->at(i).AssetElements[j].dwBitRate>maxBitRate)
					maxBitRate = pASp->at(i).AssetElements[j].dwBitRate;
			}
		}
		//////////////////////////////////////////////////////////////////////////
		// compose the AE list for ISS
		pAElements = new AELEMENT[countAE];
		glog(ZQ::common::Log::L_DEBUG, "NOTIFY   STVPlaylistManager::composeAE()  Altogether %d Asset elements", countAE);
		
		for(i=0; i<pASp->size(); i++) {
			for(j=0; j<pASp->at(i).AssetElements.size(); j++) {
				pAElements[indxAE].AEUID	= pASp->at(i).AssetElements[j].dwAEUID;
				pAElements[indxAE].BitRate	= pASp->at(i).AssetElements[j].dwBitRate;
				pAElements[indxAE].StartPos.seconds			= pASp->at(i).AssetElements[j].dwCueIn;
				pAElements[indxAE].StartPos.microseconds	= 0;
				pAElements[indxAE].EndPos.seconds			= pASp->at(i).AssetElements[j].dwCueOut;
				pAElements[indxAE].EndPos.microseconds		= 0;
				pAElements[indxAE].PlayoutEnabled	= TRUE;
				pAElements[indxAE].EncryptionVendor	= ITV_ENCRYPTION_VENDOR_None;
				pAElements[indxAE].EncryptionDevice	= ITV_ENCRYPTION_DEVICE_None;
				indxAE++;
			}
		}
		pAE->AssetUID		= pASp->at(0).dwAssetUID;
		pAE->BitRate		= maxBitRate;
		pAE->AECount		= countAE;
		pAE->AELlist		= (PAEARRAY)pAElements;

		// push it into the IDS map for ISS
		std::pair<DWORD, AELIST> entry(pl->getPLchannel(), *pAE);

		_mapAELISTMutex.enter();

		std::map<DWORD, AELIST>::iterator iter = _mapAELIST.find(pl->getPLchannel());
		if(iter != _mapAELIST.end()) {
			// already exist one
			if(iter->second.AELlist) {
				delete [] (PAELEMENT)iter->second.AELlist;
				iter->second.AELlist = NULL;
			}
			_mapAELIST.erase(iter);
		}
		_mapAELIST.insert(entry);

		_mapAELISTMutex.leave();

	}
	catch(...) {
		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::composeAE()  Some sort of unknown error occurs");
	}
	
	//delete []pAElements;	//???
	return TRUE;
}

bool STVPlaylistManager::composeAE(STVPlaylist* pl, PAELIST pAE, PASSETS pASp, PASSETLIST* filledlist)
{
	glog(ZQ::common::Log::L_DEBUG, "PENDING  STVPlaylistManager::composeAE()  begin composing AE, type 2");
	PAELEMENT	pAElements;

	LONG	maxBitRate	=0;
	LONG	countAE		=0;
	LONG	indxAE	=0;
	int i,j;
	//////////////////////////////////////////////////////////////////////////
	// compute total asset element number
	try
	{
		for(i=0; i<pASp->size(); i++) {
			for(j=0; j<pASp->at(i).AssetElements.size(); j++) {
				countAE++;
				if(pASp->at(i).AssetElements[j].dwBitRate>maxBitRate)
					maxBitRate = pASp->at(i).AssetElements[j].dwBitRate;
			}
		}
		for(i=0; i<filledlist->size(); i++) {
			for(j=0; j<filledlist->at(i)->AssetElements.size(); j++) {
				countAE++;
				if(filledlist->at(i)->AssetElements[j].dwBitRate>maxBitRate)
					maxBitRate = filledlist->at(i)->AssetElements[j].dwBitRate;
			}
		}
		//////////////////////////////////////////////////////////////////////////
		// compose the AE list for ISS
		pAElements = new AELEMENT[countAE];
		glog(ZQ::common::Log::L_DEBUG, "NOTIFY   STVPlaylistManager::composeAE()  Altogether %d Asset elements", countAE);
		
		for(i=0; i<pASp->size(); i++) {
			for(j=0; j<pASp->at(i).AssetElements.size(); j++) {
				pAElements[indxAE].AEUID	= pASp->at(i).AssetElements[j].dwAEUID;
				pAElements[indxAE].BitRate	= pASp->at(i).AssetElements[j].dwBitRate;
				pAElements[indxAE].StartPos.seconds			= pASp->at(i).AssetElements[j].dwCueIn;
				pAElements[indxAE].StartPos.microseconds	= 0;
				pAElements[indxAE].EndPos.seconds			= pASp->at(i).AssetElements[j].dwCueOut;
				pAElements[indxAE].EndPos.microseconds		= 0;
				pAElements[indxAE].PlayoutEnabled	= TRUE;
				pAElements[indxAE].EncryptionVendor	= ITV_ENCRYPTION_VENDOR_None;
				pAElements[indxAE].EncryptionDevice	= ITV_ENCRYPTION_DEVICE_None;
				indxAE++;
			}
		}
		for(i=0; i<filledlist->size(); i++) {
			for(j=0; j<filledlist->at(i)->AssetElements.size(); j++) {
				pAElements[indxAE].AEUID	= filledlist->at(i)->AssetElements[j].dwAEUID;
				pAElements[indxAE].BitRate	= filledlist->at(i)->AssetElements[j].dwBitRate;
				pAElements[indxAE].StartPos.seconds			= filledlist->at(i)->AssetElements[j].dwCueIn;
				pAElements[indxAE].StartPos.microseconds	= 0;
				pAElements[indxAE].EndPos.seconds			= filledlist->at(i)->AssetElements[j].dwCueOut;
				pAElements[indxAE].EndPos.microseconds		= 0;
				pAElements[indxAE].PlayoutEnabled	= TRUE;
				pAElements[indxAE].EncryptionVendor	= ITV_ENCRYPTION_VENDOR_None;
				pAElements[indxAE].EncryptionDevice	= ITV_ENCRYPTION_DEVICE_None;
				indxAE++;
			}
		}

		pAE->AssetUID		= pASp->at(0).dwAssetUID;
		pAE->BitRate		= maxBitRate;
		pAE->AECount		= countAE;
		pAE->AELlist		= (PAEARRAY)pAElements;

		// push it into the IDS map for ISS
		std::pair<DWORD, AELIST> entry(pl->getPLchannel(), *pAE);

		_mapAELISTMutex.enter();

		std::map<DWORD, AELIST>::iterator iter = _mapAELIST.find(pl->getPLchannel());
		if(iter != _mapAELIST.end()) {
			// already exist one
			if(iter->second.AELlist) {
				delete [] (PAELEMENT)iter->second.AELlist;
				iter->second.AELlist = NULL;
			}
			_mapAELIST.erase(iter);
		}
		_mapAELIST.insert(entry);

		_mapAELISTMutex.leave();

	}
	catch(...) {
		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylistManager::composeAE()  Some sort of unknown error occurs");
	}
	
	//delete []pAElements;	//???
	return TRUE;
}

bool STVPlaylistManager::composeAE(STVPlaylist* pl, PAELIST pAE, PASSETLIST* filledlist)
{
	glog(ZQ::common::Log::L_DEBUG, "PENDING  STVPlaylistManager::composeAE()  begin composing AE, type 3");
	PAELEMENT	pAElements;

	LONG	maxBitRate	=0;
	LONG	countAE		=0;
	LONG	indxAE	=0;
	int i,j;
	//////////////////////////////////////////////////////////////////////////
	// compute total asset element number
	for(i=0; i<filledlist->size(); i++) {
		for(j=0; j<filledlist->at(i)->AssetElements.size(); j++) {
			countAE++;
			if(filledlist->at(i)->AssetElements[j].dwBitRate>maxBitRate)
				maxBitRate = filledlist->at(i)->AssetElements[j].dwBitRate;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// compose the AE list for ISS
	pAElements = new AELEMENT[countAE];
	
	for(i=0; i<filledlist->size(); i++) {
		for(j=0; j<filledlist->at(i)->AssetElements.size(); j++) {
			pAElements[indxAE].AEUID	= filledlist->at(i)->AssetElements[j].dwAEUID;
			pAElements[indxAE].BitRate	= filledlist->at(i)->AssetElements[j].dwBitRate;
			pAElements[indxAE].StartPos.seconds			= filledlist->at(i)->AssetElements[j].dwCueIn;
			pAElements[indxAE].StartPos.microseconds	= 0;
			pAElements[indxAE].EndPos.seconds			= filledlist->at(i)->AssetElements[j].dwCueOut;
			pAElements[indxAE].EndPos.microseconds		= 0;
			pAElements[indxAE].PlayoutEnabled	= TRUE;
			pAElements[indxAE].EncryptionVendor	= ITV_ENCRYPTION_VENDOR_None;
			pAElements[indxAE].EncryptionDevice	= ITV_ENCRYPTION_DEVICE_None;
			indxAE++;
		}
	}

	pAE->AssetUID		= filledlist->at(0)->dwAssetUID;
	pAE->BitRate		= maxBitRate;
	pAE->AECount		= countAE;
	pAE->AELlist		= (PAEARRAY)pAElements;

	
	if(pl->isGlobal())
	{	
		// push it into global filler if necessary
		if(_gAEList.gAE.AELlist) {	
			// already exist one
			delete [] (PAELEMENT)_gAEList.gAE.AELlist;
			_gAEList.gAE.AELlist = NULL;
		}
		_gAEList.gAE = *pAE;
		_gAEList.pGFiller = pl;
	}
	else
	{	
		// push it into the IDS map for ISS
		std::pair<DWORD, AELIST> entry(pl->getPLchannel(), *pAE);

		_mapAELISTMutex.enter();

		std::map<DWORD, AELIST>::iterator iter = _mapAELIST.find(pl->getPLchannel());
		if(iter != _mapAELIST.end()) {
			// already exist one
			if(iter->second.AELlist) {
					delete [] (PAELEMENT)iter->second.AELlist;
					iter->second.AELlist = NULL;
			}
			_mapAELIST.erase(iter);
		}
		_mapAELIST.insert(entry);

		_mapAELISTMutex.leave();
	}

	return TRUE;
}

void STVPlaylistManager::checkTimer()
{
	if(!_TimerMan->isRunning()) {
		glog(ZQ::common::Log::L_WARNING, "FAILURE  STVPlaylistManager::checkTimer()  Timer is not running, restarting");
		_TimerMan->start();
	}
}