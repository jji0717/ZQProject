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
// Name  : STVPlaylistManager.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-7-12
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/PLAYLISTMOD/STVPlaylistManager.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 3     05-12-27 15:19 Bernie.zhao
// 
// 1     05-08-30 18:30 Bernie.zhao
// ===========================================================================

#include "STVPlaylistManager.h"
#include "../MainCtrl/ScheduleTV.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern ScheduleTV gSTV;

STVPMTimerMan*	STVPlaylistManager::_timerMan	= NULL;
AutoFiller*		STVPlaylistManager::_autoF		= NULL;
BarkerFiller*	STVPlaylistManager::_barkerF	= NULL;
DWORD			STVPlaylistManager::_fillLength	= DEFAULT_FILLLENGTH;
ZQ::common::ComInitializer	STVPlaylistManager::_coInit;

STVPlaylistManager::STVPlaylistManager()
{
	_configName = STVChannel::getBasePath() + DB_CONF_NAME;

	_timerMan	= NULL;
	_autoF		= new AutoFiller();
	_barkerF	= new BarkerFiller();
	
	_isRunning	= false;
	_hasConfig	= false;
	_fillLength = DEFAULT_FILLLENGTH;
}

STVPlaylistManager::~STVPlaylistManager()
{
	if(_isRunning)
		terminate();
	
	if(_timerMan)
	{
		delete _timerMan;
		_timerMan = NULL;
	}
	
	// delete filler confuser
	if(_autoF)
	{
		delete _autoF;
		_autoF = NULL;
	}
	if(_barkerF)
	{
		delete _barkerF;
		_barkerF = NULL;
	}
}

bool STVPlaylistManager::regCh(DWORD chid, const char* ip, const char* port, const char* nodegroup, const char* portid)
{
	ZQ::common::MutexGuard	tmpGd(_channelMutex);

	std::map<DWORD, STVChannel*>::iterator iter = _channelPool.find(chid);
	if(iter != _channelPool.end())
	{	// already has this channel, update it
		if(iter->second == NULL)
		{
			GTRACEERR;
			return false;
		}

		if(iter->second->getIPAddr()==ip 
			&& iter->second->getIPPort()==port 
			&& iter->second->getNodegroup()==nodegroup
			&& iter->second->getPortID()==portid)
		{
			// no change
		}
		else
		{
			// update
			stopCh(chid);
			iter->second->setIPAddr(ip);
			iter->second->setIPPort(port);
			iter->second->setNodegroup(nodegroup);
			iter->second->setPortID(portid);
			startCh(chid);
		}
	}
	else
	{	// not found, create it
		STVChannel* pNewCh = new STVChannel(chid, ip, port, nodegroup, portid);
		_channelPool[chid] = pNewCh;
		glog(ZQ::common::Log::L_DEBUG, "STVPlaylistManager::reg()  New channel [%06d] registered", chid);

		pNewCh->loadLists();
	}

	return true;
}

bool STVPlaylistManager::unregCh(DWORD chid)
{
	ZQ::common::MutexGuard	tmpGd(_channelMutex);

	std::map<DWORD, STVChannel*>::iterator iter = _channelPool.find(chid);
	if(iter != _channelPool.end())
	{	// found this channel, delete it
		STVChannel* pExCh = iter->second;
		delete pExCh;
		iter->second = pExCh = NULL;
		_channelPool.erase(iter);
	}
	else
	{	// not found
		GTRACEERR;
		return false;
	}

	return true;
}

int STVPlaylistManager::startCh(DWORD chid)
{
	// get channel
	STVChannel* pCh = queryCh(chid);
	if(pCh==NULL)
	{
		GTRACEERR;
		return STVINVALIDCHNL;
	}
	
	if(!pCh->isActive())
		pCh->enable();

	return STVSUCCESS;
}

int STVPlaylistManager::stopCh(DWORD chid)
{
	// get channel
	STVChannel* pCh = queryCh(chid);
	if(pCh==NULL)
	{
		GTRACEERR;
		return STVINVALIDCHNL;
	}

	if(pCh->isActive())
		pCh->disable();

	// query current status
	STVChannel::Stat tmpStat= pCh->getStatus();
	if(tmpStat==STVChannel::STAT_PLAYING || tmpStat==STVChannel::STAT_SUSPENDING)
	{
		// A stream is right now in this channel, so stop it
		int err = 0;
//#pragma message(__INFO__ "move comment")
		err = gSTV.OnStopStream(chid);
		
		if(err!=STVSUCCESS) 
		{
			glog(ZQ::common::Log::L_WARNING, "STVPlaylistManager::stopCh()  Unable to stop stream on channel %06ld : %s", chid, GetSTVErrorDesc(err));
		}
	}
	else if(tmpStat==STVChannel::STAT_STARTING)
	{
		// A stream is right now starting, wait at most 10 seconds and stop it
		for(int i=0; i<10; i++)
		{
			::Sleep(1000);
			tmpStat = pCh->getStatus();
			if(tmpStat!=STVChannel::STAT_STARTING && tmpStat!=STVChannel::STAT_STOPPING)
				break;
		}

		int err =0;
		err = gSTV.OnStopStream((chid));

		if(err!=STVSUCCESS) 
		{
			glog(ZQ::common::Log::L_WARNING, "STVPlaylistManager::stopCh()  Unable to stop stream on channel %06ld : %s", chid, GetSTVErrorDesc(err));
		}
	}
	else if(tmpStat==STVChannel::STAT_STOPPING)
	{
		// A stream is right now stopping, wait at most 10 seconds
		for(int i=0; i<10; i++)
		{
			::Sleep(1000);
			tmpStat = pCh->getStatus();
			if(tmpStat!=STVChannel::STAT_STARTING && tmpStat!=STVChannel::STAT_STOPPING)
				break;
		}
	}

	return STVSUCCESS;
}

STVChannel* STVPlaylistManager::queryCh(DWORD chid)
{
	STVChannel* pRet=NULL;

//	ZQ::common::MutexGuard	tmpGd(_channelMutex);
	std::map<DWORD, STVChannel*>::iterator iter = _channelPool.find(chid);
	if(iter != _channelPool.end())
	{	// found this channel
		pRet = iter->second;
	}
	else
	{	// not found
		pRet = NULL;
	}

	return pRet;
}

bool STVPlaylistManager::start()
{
	if(_isRunning)
		return true;
	
	// create the base directory
	::CreateDirectoryA(STVChannel::getBasePath().c_str(), NULL);
	_isRunning = true;

	// search the database path for config file
	ZQ::common::XMLPrefDoc confDoc(_coInit);
	std::string findstr = STVChannel::getBasePath()+DB_CONF_NAME;
	WIN32_FIND_DATAA finddata;
	HANDLE hfind = ::FindFirstFileA(findstr.c_str(), &finddata);
	if(hfind == INVALID_HANDLE_VALUE ) 
	{	// not found
		glog(ZQ::common::Log::L_DEBUG, "STVPlaylistManager::start()  No previews configuration file found");
	}
	else 
	{	// open existed file
		try
		{
			confDoc.open(findstr.c_str());
			ZQ::common::PrefGuard confGd(confDoc.root());
			if(!confGd.valid())
			{
				glog(ZQ::common::Log::L_WARNING, "STVPlaylistManager::start()  Configuration file not correct");
			}
			else
			{
				ZQ::common::PrefGuard prefGd(confGd.pref()->firstChild());
				if(!prefGd.valid())
				{
					glog(ZQ::common::Log::L_WARNING, "STVPlaylistManager::start()  Configuration file not correct");
				}
				else 
				{
					int nret = OnConfiguration(prefGd.pref(), false);
					if(nret!=STVSUCCESS) 
					{
						glog(ZQ::common::Log::L_WARNING, "STVPlaylistManager::start()  Configuration file parse error: %s", GetSTVErrorDesc(nret));
					}
					else 
					{
						glog(ZQ::common::Log::L_INFO, "STVPlaylistManager::start()  Configuration file had been successfully restored");
						_hasConfig = true;
					}
				}
			}
		}
		catch(ZQ::common::Exception excp) 
		{
			glog(ZQ::common::Log::L_WARNING, "STVPlaylistManager::start()  Got exception, with error: %s", excp.getString());
		}
		catch(...)
		{
			glog(ZQ::common::Log::L_WARNING, "STVPlaylistManager::start()  Got unknown exception");
		}
	}
	confDoc.close();
	::FindClose(hfind);
	
	glog(ZQ::common::Log::L_INFO, "STVPlaylistManager::start()  playlist manager started");

	if(_timerMan == NULL)
		_timerMan	= new STVPMTimerMan(*this);
	
	_timerMan->start();
	
	return true;
}

bool STVPlaylistManager::terminate()
{
	if(!_isRunning)
		return true;
	
	// kill timer man
	if(_timerMan)
	{
		_timerMan->OnTerminate();

		if(_timerMan->isRunning()) 
		{
			bool timerret=_timerMan->waitHandle(DEFAULT_SLEEPTIME*3);
		}
		_isRunning = false;
		
		delete _timerMan;
		_timerMan = NULL;
	}
	
	// delete channels
	ZQ::common::MutexGuard	tmpGd(_channelMutex);
	std::map<DWORD, STVChannel*>::iterator iter =0;
	for(iter=_channelPool.begin(); iter!=_channelPool.end(); iter++)
	{
		STVChannel* pExCh = iter->second;
		if(pExCh)
		{
			delete pExCh;
			iter->second = NULL;
		}
	}
	_channelPool.clear();

	glog(ZQ::common::Log::L_INFO, "STVPlaylistManager::terminate()  playlist manager terminated");
	
	return true;
}

DWORD STVPlaylistManager::scan(bool keepslow/* =false */)
{
	DWORD	minSleep = DEFAULT_SLEEPTIME;
	bool	sleepABit = false;
	
	ZQ::common::MutexGuard	poolGd(_channelMutex);
	std::map<DWORD, STVChannel*>::iterator iter = 0;

	gSTV.getStreamUnit()->resetDelay();
	
	// scan for all channels
	for(iter=_channelPool.begin(); iter!=_channelPool.end(); iter++)
	{
//		if(sleepABit && keepslow)
//			::Sleep(DEFAULT_SLEEPTIME);
//		else if(sleepABit)
//			::Sleep(50);

		DWORD		chid = iter->first;
		STVChannel* pCh	 = iter->second;
		if(pCh == NULL)
			continue;

		if(!pCh->isActive())
		{
			// this channel is currently disabled, skip it
			continue;
		}
		
		STVChannel::Stat tmpStat = pCh->getStatus();
		if(tmpStat == STVChannel::STAT_STARTING || tmpStat == STVChannel::STAT_STOPPING)
		{
			// this channel is currently handle a stream operation, skip it
			continue;
		}

		std::string	pendingSchNO = "";
		int			pendingIeIdx = -1;
		int			pendingType = LISTTYPE_UNKNOWN;
		DWORD tmpwait = pCh->extract(pendingSchNO, pendingIeIdx, pendingType);
		minSleep = (tmpwait<minSleep)? tmpwait:minSleep;

		if(!pendingSchNO.empty() && pendingIeIdx==-1)
		{
			// this channel is streaming just what it should stream, skip it
			continue;
		}
		else if(!pendingSchNO.empty() && pendingIeIdx!=-1)
		{
			// this channel has a new ie to start
			int err = 0;
// #pragma message(__INFO__ "move comment")

			// if filler, delay 0.5 second between each filler start time
			if(BASICTYPE(pendingType)==LISTTYPE_FILLER)
				gSTV.getStreamUnit()->enableDelay();
			else
				gSTV.getStreamUnit()->disableDelay();

			err = gSTV.OnStartStream(chid, pendingSchNO.c_str(), pendingIeIdx);
			if(err==STVSUCCESS)
			{
				sleepABit = true;
				glog(ZQ::common::Log::L_DEBUG, "STVPlaylistManager::scan()  Channel %06ld is about to start stream schedule %s ie %d", chid, pendingSchNO.c_str(), pendingIeIdx);
			}
			else
			{
				glog(ZQ::common::Log::L_DEBUG, "STVPlaylistManager::scan()  Could not start stream schedule %s ie %d on Channel %06ld : %s", pendingSchNO.c_str(), pendingIeIdx, chid, GetSTVErrorDesc(err));
			}
		}
		else if(tmpStat==STVChannel::STAT_PLAYING || tmpStat == STVChannel::STAT_SUSPENDING)
		{
			// this channel has nothing to stream at current moment, stop it
			int err = 0;
//#pragma message(__INFO__ "move comment")
			err = gSTV.OnStopStream(chid);
			if(err==STVSUCCESS)
			{
				sleepABit = true;
				glog(ZQ::common::Log::L_DEBUG, "STVPlaylistManager::scan()  Channel %06ld is about to stop stream", chid);
			}
			else
			{
				glog(ZQ::common::Log::L_DEBUG, "STVPlaylistManager::scan()  Could not stop stream on Channel %06ld : %s", chid, GetSTVErrorDesc(err));
			}
		}
	}

	return minSleep;
}

DWORD STVPlaylistManager::getChnlID(ZQ::common::IPreference* chnlPref)
{
	if(chnlPref==NULL)
	{
		GTRACEERR;
		return 0;
	}
	
	char buff[256]={0};
	DWORD dwRet=0;

	if( (!chnlPref->has(KEY_PORT_IPADRESS))
		||(!chnlPref->has(KEY_PORT_IPPORT))
		||(!chnlPref->has(KEY_PORT_DEVICEID))
		||(!chnlPref->has(KEY_PORT_CHANNELID)) ) 
	{
		GTRACEERR;
		return 0;
	}
		
	buff[0]='\0';
	chnlPref->get(KEY_PORT_CHANNELID, buff);
	dwRet = atol(buff)*MAX_SUBCHNL;

	if(chnlPref->has(KEY_PORT_SUBCHANNELID))
	{
		buff[0]='\0';
		chnlPref->get(KEY_PORT_SUBCHANNELID, buff);
		dwRet += atol(buff);
	}

	return dwRet;
}

int STVPlaylistManager::OnNewPlayList(const char* scheduleNO, ZQ::common::IPreference* pPortPref, ZQ::common::IPreference* pPlayListPref, int nType)
{	
	char buff[256]={0};
	int nRet = STVSUCCESS;
	
	try
	{
		DWORD tmpch=0;
		if(pPortPref)	// non-global list
		{	
			// get channel id
			tmpch = getChnlID(pPortPref);
			if(tmpch==0)
			{
				glog(ZQ::common::Log::L_WARNING, "STVPlaylistManager::OnNewPlayList()  Invalid channel %06d : %s", tmpch, GetSTVErrorDesc(STVINVALIDCHNL));
				return STVINVALIDCHNL;
			}

			// query channel
			STVChannel* pCh = queryCh(tmpch);
			if(pCh==NULL)
			{
				GTRACEERR;
				glog(ZQ::common::Log::L_WARNING, "STVPlaylistManager::OnNewPlayList()  Invalid channel %06d : %s", tmpch, GetSTVErrorDesc(STVINVALIDCHNL));
				return STVINVALIDCHNL;
			}

			nRet = pCh->createList(pPlayListPref, scheduleNO, nType);
			if(nRet != STVSUCCESS)
			{
				glog(ZQ::common::Log::L_WARNING, "STVPlaylistManager::OnNewPlayList()  Can not create list '%s' for channel %06d : %s", scheduleNO, tmpch, GetSTVErrorDesc(nRet));
				return nRet;
			}
		}
		else			// global list
		{	
			tmpch = 0;
			int globalType = nType | LIST_GLOBAL;

			// create the list for each channel
			ZQ::common::MutexGuard	tmpGd(_channelMutex);
			std::map<DWORD, STVChannel*>::iterator iter = 0;
			for(iter=_channelPool.begin(); iter!=_channelPool.end(); iter++)
			{
				if(iter->second==NULL)
					continue;

				nRet = iter->second->createList(pPlayListPref, scheduleNO, globalType);
				if(nRet != STVSUCCESS)
				{
					glog(ZQ::common::Log::L_WARNING, "STVPlaylistManager::OnNewPlayList()  Can not create list '%s' for channel %06d : %s", scheduleNO, iter->second->id(), GetSTVErrorDesc(nRet));
					continue;
				}
			}
		}
		
		// wake up timer to update time point
		_timerMan->OnWakeup();
		switch(BASICTYPE(nType)) 
		{
		case LISTTYPE_NORMAL:
			if(tmpch)
				glog(ZQ::common::Log::L_INFO, "STVPlaylistManager::OnNewPlayList()  New STV list '%s' created for channel %06d", scheduleNO, tmpch);
			else
				glog(ZQ::common::Log::L_INFO, "STVPlaylistManager::OnNewPlayList()  New global STV list '%s' created for all channels", scheduleNO);
			break;
		case LISTTYPE_FILLER:
			if(tmpch)
				glog(ZQ::common::Log::L_INFO, "STVPlaylistManager::OnNewPlayList()  New filler set '%s' created for channel %06d", scheduleNO, tmpch);
			else
				glog(ZQ::common::Log::L_INFO, "STVPlaylistManager::OnNewPlayList()  New global filler set '%s' created for all channels", scheduleNO);
			break;
		case LISTTYPE_BARKER:
			if(tmpch)
				glog(ZQ::common::Log::L_INFO, "STVPlaylistManager::OnNewPlayList()  New barker list '%s' created for channel %06d", scheduleNO, tmpch);
			else
				glog(ZQ::common::Log::L_INFO, "STVPlaylistManager::OnNewPlayList()  New global barker list '%s' created for all channels", scheduleNO);
			break;
		default:
			break;
		}
		
	}
	catch (ZQ::common::Exception excp) {
		glog(ZQ::common::Log::L_ERROR, "STVPlaylistManager::OnNewPlayList()  Got exception: %s", excp.getString());
		GTRACEERR;
		return STVINVALIDPL;
	}
	catch(...) {
		glog(ZQ::common::Log::L_ERROR, "STVPlaylistManager::OnNewPlayList()  Got unknown exception");
		GTRACEERR;
		return STVINVALIDPL;
	}

	return STVSUCCESS;
}

int STVPlaylistManager::OnPlayAssetImmediately(ZQ::common::IPreference *pPortPref, ZQ::common::IPreference *pPlayListPref, ZQ::common::IPreference* pTimePref)
{
	glog(ZQ::common::Log::L_INFO, "STVPlaylistManager::OnPlayAssetImmediately()  command arrived");
	// !Not implemented
	return STVERROR;
}

int STVPlaylistManager::OnPlayFillerImmediately(ZQ::common::IPreference *pPortPref)
{
	glog(ZQ::common::Log::L_INFO, "STVPlaylistManager::OnPlayFillerImmediately()  command arrived");
	// !Not implemented
	return STVERROR;
}

int STVPlaylistManager::OnPlaySkipToAsset(ZQ::common::IPreference *pPortPref, ZQ::common::IPreference *pPlayListPref)
{
	glog(ZQ::common::Log::L_INFO, "STVPlaylistManager::OnPlaySkipToAsset()  command arrived");
	// !Not implemented
	return STVERROR;
}

int STVPlaylistManager::OnHoldAsset(ZQ::common::IPreference *pPortPref, ZQ::common::IPreference* pAssetPref)
{
	glog(ZQ::common::Log::L_INFO, "STVPlaylistManager::OnHoldAsset()  command arrived");
	// !Not implemented
	return STVERROR;
}

int STVPlaylistManager::OnChannelStartup(ZQ::common::IPreference* pPortPref)
{
	DWORD chid = getChnlID(pPortPref);
	if(chid==0)
	{
		GTRACEERR;
		return STVINVALIDCHNL;
	}
	
	return startCh(chid);
}

int STVPlaylistManager::OnChannelShutdown(ZQ::common::IPreference* pPortPref)
{
	DWORD chid = getChnlID(pPortPref);
	if(chid==0)
	{
		GTRACEERR;
		return STVINVALIDCHNL;
	}
	
	return stopCh(chid);
}

int STVPlaylistManager::OnConfiguration(ZQ::common::IPreference* pConfigPref, bool dump/*=true*/)
{
	char buff[MAX_PATH];
	bool errormsg=false;
 	
	ZQ::common::MutexGuard		confGd(_configMutex);
	
	// scan channels
	for(ZQ::common::PrefGuard portGd(pConfigPref->firstChild()); portGd.valid(); portGd.pref(pConfigPref->nextChild()) )
	{
		std::string tagname = portGd.pref()->name(buff);
		DWORD	chid=0;
		std::string deviceid="", portid="",  ipaddr="",  ipport="";

		if(!portGd.pref()->has(KEY_PORT_CHANNELID))
		{
			errormsg = true;
			break;
		}

		// channel id
		buff[0]='\0';
		portGd.pref()->get(KEY_PORT_CHANNELID, buff);
		chid = atol(buff)*MAX_SUBCHNL;

		if(tagname == TAG_PORT) 
		{	// no sub-channels, calculate unique channel id and store information
			
			if(!portGd.pref()->has(KEY_PORT_DEVICEID)
				|| !portGd.pref()->has(KEY_PORT_IPADRESS)
				|| !portGd.pref()->has(KEY_PORT_IPPORT)	) {
				errormsg = true;
				break;
			}

			// node-group
			buff[0]='\0';
			portGd.pref()->get(KEY_PORT_DEVICEID, buff);
			deviceid = buff;
			
			// port id
			buff[0]='\0';
			if(portGd.pref()->has(KEY_PORT_PORTID))
				portGd.pref()->get(KEY_PORT_PORTID, buff);
			else
				strcpy(buff, "");
			portid = buff;

			// ip addr
			buff[0]='\0';
			portGd.pref()->get(KEY_PORT_IPADRESS, buff);
			ipaddr = buff;

			// ip port
			buff[0]='\0';
			portGd.pref()->get(KEY_PORT_IPPORT, buff);
			ipport = buff;

			// add channel or update channel info
			regCh(chid, ipaddr.c_str(), ipport.c_str(), deviceid.c_str(), portid.c_str());
		}
		else if(tagname == TAG_CHANNEL) 
		{	// there are sub-channels
			
			for(ZQ::common::PrefGuard subPortGd(portGd.pref()->firstChild(TAG_PORT)); subPortGd.valid(); subPortGd.pref(portGd.pref()->nextChild()) ) 
			{
				DWORD subid = 0;
				
				if(!subPortGd.pref()->has(KEY_PORT_SUBCHANNELID) 
				|| !subPortGd.pref()->has(KEY_PORT_DEVICEID)
				|| !subPortGd.pref()->has(KEY_PORT_IPADRESS)
				|| !subPortGd.pref()->has(KEY_PORT_IPPORT) ) {
					errormsg = true;
					break;
				}

				// sub-channel id
				buff[0]='\0';
				subPortGd.pref()->get(KEY_PORT_SUBCHANNELID, buff);
				subid = chid + atol(buff);

				// node-group
				buff[0]='\0';
				subPortGd.pref()->get(KEY_PORT_DEVICEID, buff);
				deviceid = buff;
				
				// ip
				buff[0]='\0';
				subPortGd.pref()->get(KEY_PORT_IPADRESS, buff);
				ipaddr = buff;

				// port
				buff[0]='\0';
				subPortGd.pref()->get(KEY_PORT_IPPORT, buff);
				ipport = buff;

				// add channel or update channel info
				regCh(subid, ipaddr.c_str(), ipport.c_str(), deviceid.c_str(), portid.c_str());
			}
		}
	}
	
	if(errormsg) 
	{
		glog(ZQ::common::Log::L_ERROR, "STVPlaylistManager::OnConfigration()  configuration format not correct!");
		GTRACEERR;
		return STVINVALIDCHNL;
	}
	
	if(dump)// should dump to disk file
	{	
		ZQ::common::XMLPrefDoc confDoc(_coInit);	// create file
		
		confDoc.open(_configName.c_str(), XMLDOC_CREATE);
		ZQ::common::PrefGuard confRoot(confDoc.newElement(TAG_DBCONFIG));
		confDoc.set_root(confRoot.pref());		// set root tag
		confRoot.pref()->addNextChild(pConfigPref);	// add child
		if(confDoc.save())
			_hasConfig = true;
		confDoc.close();
		
		glog(ZQ::common::Log::L_INFO, "STVPlaylistManager::OnConfigration()  Configuration file %s updated", _configName.c_str());
	}
	
	return STVSUCCESS;
}

int STVPlaylistManager::OnConnected(char* backstream, DWORD* backcount)
{
	if(_hasConfig) 
	{
		sprintf(backstream, "<%s %s=\"0\" />",
			TAG_CONNECT, KEY_CONNECT_FLAG);
	}
	else 
	{
		sprintf(backstream, "<%s %s=\"1\" />",
			TAG_CONNECT, KEY_CONNECT_FLAG);
		
		gSTV.getSMConnector()->SendEnquireConfig();

		// sleep a while to make sure the configuration arrived earlier than ANY other schedule
		::Sleep(1000);
	}

	*backcount = strlen(backstream);

	return STVSUCCESS;
}

int STVPlaylistManager::OnGetAElist(DWORD chnlID, AELIST& OutAEList)
{
	// check if channel existed
	STVChannel* pCh = queryCh(chnlID);
	if(pCh==NULL)
	{
		GTRACEERR;
		return STVINVALIDCHNL;
	}
	
	// fetch the AE list
	int err = pCh->allocAEList(OutAEList);
	
	if(err!=STVSUCCESS) {
		glog(ZQ::common::Log::L_WARNING, "STVPlaylistManager::OnGetAElist()  Could not get AEList for channel %06ld :%s", chnlID, GetSTVErrorDesc(err));
	}

	return err;
}

int STVPlaylistManager::OnFreeAElist(DWORD chnlID)
{
	// check if channel existed
	STVChannel* pCh = queryCh(chnlID);
	if(pCh==NULL)
	{
		GTRACEERR;
		return STVINVALIDCHNL;
	}
	
	// fetch the AE list
	int err = pCh->freeAEList();
	
	if(err!=STVSUCCESS) {
		glog(ZQ::common::Log::L_WARNING, "STVPlaylistManager::OnFreeAElist()  Could not free AEList for channel %06ld :%s", chnlID, GetSTVErrorDesc(err));
	}

	return err;
}

int STVPlaylistManager::OnSetStreamStatus(DWORD chnlID, SSAENotification& notiMsg, char* backstream, DWORD* backcount)
{
	// check if channel existed
	STVChannel* pCh = queryCh(chnlID);
	if(pCh==NULL)
	{
		GTRACEERR;
		return STVINVALIDCHNL;
	}

	STVChannel::Stat tmpStat = pCh->getStatus();
	if(tmpStat==STVChannel::STAT_IDLE)
	{	
		glog(ZQ::common::Log::L_DEBUG, "STVPlaylistManager::OnSetStreamStatus()  channel %06ld not streaming:", chnlID);
		return STVINVALIDCHNL;
	}
	
	// format feedback
	glog(ZQ::common::Log::L_DEBUG, "STVPlaylistManager::OnSetStreamStatus()  AE status on channel %06ld changed:", chnlID);
	glog(ZQ::common::Log::L_DEBUG, "\tAsset Element: 0x%x", notiMsg.dwAeUID);
	glog(ZQ::common::Log::L_DEBUG, "\tOperation: %ld", notiMsg.wOperation);
	glog(ZQ::common::Log::L_DEBUG, "\tStatus: %ld", notiMsg.dwStatus);

	// update ae status
	int err = pCh->updatePlayingElem(notiMsg, backstream, backcount);
		
	return err;
}

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

unsigned long STVPlaylistManager::timeSub(SYSTEMTIME t1, SYSTEMTIME t2)
{
	unsigned long retval = ULONG_MAX;

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

	tm	tb1,tb2;
	time_t	tval1,tval2;

	tb1.tm_year = t1.wYear -1900;
	tb2.tm_year	= t2.wYear -1900;
	tb1.tm_mon	= t1.wMonth -1;
	tb2.tm_mon	= t2.wMonth -1;
	tb1.tm_mday	= t1.wDay;
	tb2.tm_mday	= t2.wDay;
	tb1.tm_hour	= t1.wHour;
	tb2.tm_hour	= t2.wHour;
	tb1.tm_min	= t1.wMinute;
	tb2.tm_min	= t2.wMinute;
	tb1.tm_sec	= t1.wSecond;
	tb2.tm_sec	= t2.wSecond;

	tval1 = mktime(&tb1);
	tval2 = mktime(&tb2);

	retval = (tval1>=tval2)? (tval1 - tval2) : ULONG_MAX;
	
	return retval;
}

void STVPlaylistManager::checkTimer()
{
	if(!_timerMan->isRunning()) 
	{
		_timerMan->OnTerminate();
		delete _timerMan;
		_timerMan = NULL;
		_timerMan = new STVPMTimerMan(*this);
		glog(ZQ::common::Log::L_WARNING, "STVPlaylistManager::checkTimer()  Timer is not running, restarting it");
		_timerMan->start();
	}
}