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
// Desc  : implementation of STV playlist
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/PlaylistMod/STVPlaylist.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 26    05-06-27 18:04 Bernie.zhao
// June.27/2005.  Fixed interface error with SM when creating new STV list
// 
// 25    05-03-26 16:19 Bernie.zhao
// 
// 24    05-03-24 14:52 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 23    05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// 
// 22    05-01-05 17:16 Bernie.zhao
// modified to support STV status feedback
// 
// 21    04-12-16 14:55 Bernie.zhao
// 
// 20    04-12-06 20:54 Bernie.zhao
// mem exception resolved
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
// 15    04-10-25 15:26 Bernie.zhao
// added status report
// 
// 14    04-10-24 19:04 Bernie.zhao
// end of 2004/Oct/24
// 
// 13    04-10-18 18:46 Bernie.zhao
// 
// 12    04-10-18 10:33 Bernie.zhao
// test 1 ok
// 
// 11    04-10-15 16:14 Bernie.zhao
// ===========================================================================
#include <string.h>

#include "STVPlaylist.h"
#include "STVPlaylistManager.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//##ModelId=4147B84B033C
STVPlaylist::STVPlaylist(STVPlaylistManager*	mgm)
{
	_mgm = mgm;
	
	wcscpy(_dbfilename,_T(""));
	wcscpy(_dbdir,_T(""));

	_PLstatus		= PLSTAT_IDLE;
	_PLchannel		= -1;

//	_SetIDS=FALSE;
	_PlayingAsset=0;
	_StartFromMid=FALSE;
	_StartPoint = 0;

	_Filltype = FILLTYPE_SERIAL;
	_IsGlobal = FALSE;

	_RetryTimes = 0;
}


//##ModelId=4147B84B033D
int STVPlaylist::create(ZQ::common::IPreference* channelnode, ZQ::common::IPreference* listnode, int listtype)
{
	
	int retdb=createDBName(channelnode, listnode, listtype);
	if(retdb!=STVSUCCESS)
		return retdb;

	if(!init())
		return STVINVALIDPL;

	// import playlist info from SM interface
	ZQ::common::IPreference* dbIpref = _dbdoc->root();

	if(channelnode)	dbIpref->addNextChild(channelnode);
	if(listnode) dbIpref->addNextChild(listnode);
	if(dbIpref)	dbIpref->free();

	if(!updateToDB())
		return STVPLDBFAIL;
	
	return STVSUCCESS;
}

//##ModelId=4147B84B034D
STVPlaylist::~STVPlaylist()
{
	if(_dbdoc) {
		//_dbdoc->close();
		delete _dbdoc;
	}
}

//##ModelId=4147B84B03C8
bool STVPlaylist::init(int typerestore /* =0 */)
{
	_dbdoc  = new ZQ::common::XMLPrefDoc (*(_mgm->getComInit()));
	ZQ::common::IPreference* dbIpref=NULL;
	ZQ::common::IPreference* dbStatusPref=NULL;
	ZQ::common::IPreference* dbCurrent=NULL;
	if(typerestore ==0) 
	{
		// set status
		_PLstatus = PLSTAT_IDLE;
		
		// convert to ascii
		char fname[MAX_PATH];
		size_t dwCount = MAX_PATH;
		wcstombs(fname, _dbfilename, dwCount);

		// create new file
		_dbdoc->open(fname, XMLDOC_CREATE);
		dbIpref = _dbdoc->newElement(XML_DBMIRROR);
					
		// add a status node
		dbStatusPref =_dbdoc->newElement(XML_STATUS);
		
		// add current status flag in status node
		dbCurrent = _dbdoc->newElement(XML_STATUS_CURRENT);
		
		dbCurrent->set(XML_STATUS_CURRENT_STATE, statusitoa(_PLstatus).c_str());
		
		dbStatusPref->addNextChild(dbCurrent);
				
		dbIpref->addNextChild(dbStatusPref);

		_dbdoc->set_root(dbIpref);
	}
	else 
	{
		char fname[MAX_PATH];
		size_t dwCount = MAX_PATH;
		wcstombs(fname, _dbfilename, dwCount);
		
		// open existed file
		_dbdoc->open(fname);
		dbIpref = _dbdoc->root();
		dbStatusPref = dbIpref->firstChild(XML_STATUS);

		// get status
		std::string statstr;
		if(getStatus(XML_STATUS_CURRENT, XML_STATUS_CURRENT_STATE, statstr))
			_PLstatus = statusatoi(statstr);

	}

	if(dbIpref)	dbIpref->free();
	if(dbStatusPref) dbStatusPref->free();
	if(dbCurrent) dbCurrent->free();
	return true;
}

//##ModelId=4147B84B036D
bool STVPlaylist::updateToDB()
{
	char fname[MAX_PATH];
	size_t dwCount = MAX_PATH;
	wcstombs(fname, _dbfilename, dwCount);
	bool bRet=FALSE;
	_AttribLock.enter();
	try
	{
		bRet= _dbdoc->save(fname);
	}
	catch (ZQ::common::Exception excep) {
		glog(ZQ::common::Log::L_WARNING, "FAILURE  STVPlaylist::updateToDB()  An exception occurs when saving file, with error string: %s", excep.getString());
	}
	_AttribLock.leave();
	return bRet;
}

//##ModelId=4147B84B037A
bool STVPlaylist::updateToDB(const wchar_t* filename)
{
	// extract the directory first
	wchar_t* pos=wcschr(filename, L'\\');
	if(pos == NULL)
		return false;
	// create the directory
	wchar_t dir[MAX_PATH];
	wcscpy(dir,filename);
	dir[pos-filename+1]=L'\0';
	
	::CreateDirectory(dir,NULL);

	char fname[MAX_PATH];
	size_t dwCount = MAX_PATH;
	wcstombs(fname, _dbfilename, dwCount);
	// save to both mirror file
	if(!_dbdoc->save(fname))
		return FALSE;
	wcstombs(fname, filename, dwCount);
	return(_dbdoc->save(fname));
		
}

//##ModelId=4147B84B038A
bool STVPlaylist::restoreFromDB(const wchar_t* dbfilename)
{
	char buff[64];
	std::string buffstr;
	ZQ::common::IPreference* dbIpref;
	ZQ::common::IPreference* channelnode;
	ZQ::common::IPreference* listnode;
	ZQ::common::IPreference* pTmpAsset;
	ZQ::common::IPreference* pTmpElement;
	
	wcscpy(_dbfilename, dbfilename);
	std::wstring fname=_dbfilename;
	size_t slashidx=fname.find_last_of(L'\\');
	wchar_t prefix=fname[slashidx+1];
	switch(prefix) {
	case L'B':
		_PLtype = LISTTYPE_BARKER;
		break;
	case L'F':
		_PLtype = LISTTYPE_FILLER;
		break;
	case L'G':
		_PLtype = LISTTYPE_FILLER;
		_IsGlobal = TRUE;
		break;
	case L'P':
		_PLtype = LISTTYPE_PLAYLIST;
		break;
	default:
		return false;
	}
	bool retval= init(INIT_RESTORE);
	if(!retval)
		return retval;
	
	dbIpref = _dbdoc->root();
	if(!dbIpref) {
		return FALSE;
	}

	channelnode = dbIpref->firstChild(XML_PORT);
	if(!channelnode && !_IsGlobal) {
		if(dbIpref)	dbIpref->free();
		return FALSE;
	}
	
	if(_PLtype!=LISTTYPE_FILLER) {
		listnode = dbIpref->firstChild(XML_PLAYLIST);
	}
	else {
		listnode = dbIpref->firstChild(XML_FILLERLIST);
	}
	if(!listnode) {
		if(channelnode)	channelnode->free();
		if(dbIpref)	dbIpref->free();
		return FALSE;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// set channel id, fill mode
	
	// prepare channel id
	DWORD tmpch=0;
	if(_IsGlobal)
	{
		tmpch = -1;
	}
	else
	{
		if(!channelnode->has(XML_PORT_CHANNELID)) {
			if(channelnode) channelnode->free();
			if(listnode) listnode->free();
			if(dbIpref) dbIpref->free();
			return FALSE;
		}
		channelnode->get(XML_PORT_CHANNELID, buff);
		tmpch = atol(buff);
		if(channelnode->has(XML_PORT_SUBCHANNELID)) {
			channelnode->get(XML_PORT_SUBCHANNELID, buff);
			tmpch = tmpch*(_mgm->getMaxSubChnl())+atol(buff);
		}
		else {
			tmpch = tmpch*(_mgm->getMaxSubChnl());
		}
	}

	setPLchannel(tmpch);
	
	// prepare fill type
	if(_PLtype!=LISTTYPE_PLAYLIST) {
		if(listnode->has(XML_PLAYLIST_PLAYMODE)) {
			listnode->get(XML_PLAYLIST_PLAYMODE, buff);
			if(strcmp(buff, "S")==0)
				_Filltype = FILLTYPE_SERIAL;
			else if(strcmp(buff, "R")==0)
				_Filltype = FILLTYPE_RANDOM;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// set time attributes
	pTmpAsset= listnode->firstChild(XML_PLAYLIST_ASSET);
	if(!pTmpAsset) {
		if(listnode) listnode->free();
		if(channelnode) channelnode->free();
		if(dbIpref) dbIpref->free();
		return FALSE;
	}
	
	// prepare begin time attributes
	if(_PLtype==LISTTYPE_PLAYLIST)
		pTmpAsset->get(XML_PLAYLIST_ASSET_BEGIN, buff);
	else
		listnode->get(XML_PLAYLIST_EFFECTTIME, buff);

	buffstr = buff;
	_BeginTime.wYear		= atol(buffstr.substr(0,4).c_str());
	_BeginTime.wMonth		= atol(buffstr.substr(4,2).c_str());
	_BeginTime.wDay			= atol(buffstr.substr(6,2).c_str());
	_BeginTime.wHour		= atol(buffstr.substr(9,2).c_str());
	_BeginTime.wMinute		= atol(buffstr.substr(12,2).c_str());
	_BeginTime.wSecond		= atol(buffstr.substr(15,2).c_str());
	_BeginTime.wDayOfWeek= 7;	// don't calculate weekdays
	_BeginTime.wMilliseconds = 0;	// don't care about milli seconds
	
	// prepare end time ( for filler/barker only)
	if(_PLtype!=LISTTYPE_PLAYLIST) {
		listnode->get(XML_PLAYLIST_ENDTIME, buff);
		buffstr = buff;
		_EndTime.wYear			= atol(buffstr.substr(0,4).c_str());
		_EndTime.wMonth			= atol(buffstr.substr(4,2).c_str());
		_EndTime.wDay			= atol(buffstr.substr(6,2).c_str());
		_EndTime.wHour			= atol(buffstr.substr(9,2).c_str());
		_EndTime.wMinute		= atol(buffstr.substr(12,2).c_str());
		_EndTime.wSecond		= atol(buffstr.substr(15,2).c_str());
		_EndTime.wDayOfWeek	= 7;	// don't calculate weekdays
		_EndTime.wMilliseconds = 0;	// don't care about milli seconds
	}

	if(pTmpAsset) pTmpAsset->free();
	
	//////////////////////////////////////////////////////////////////////////
	// prepare top&bottom ae no , and total duration
	
	DWORD tmpba=0;
	DWORD tmpta=0;
	DWORD tmpLength=0;
	for(pTmpAsset=listnode->firstChild(XML_PLAYLIST_ASSET); pTmpAsset; pTmpAsset=listnode->nextChild())
	{
		for(pTmpElement=pTmpAsset->firstChild(XML_PLAYLIST_ASSET_AE); pTmpElement; pTmpElement=pTmpAsset->nextChild())
		{
			if(!pTmpElement->get(XML_PLAYLIST_ASSET_AE_NO, buff))
			{ 
				if(pTmpElement) pTmpElement->free();
				if(pTmpAsset) pTmpAsset->free();
				if(listnode) listnode->free();
				if(channelnode) channelnode->free();
				if(dbIpref) dbIpref->free();
				return FALSE;
			}
			if(tmpba==0)
				tmpba=atol(buff);	// set bottom ae NO
			if(tmpta<atol(buff))
				tmpta=atol(buff);	// set current top ae NO

			if(!pTmpElement->get(XML_PLAYLIST_ASSET_AE_CUEDURATION, buff))
			{ 
				if(pTmpElement) pTmpElement->free(); 
				if(pTmpAsset) pTmpAsset->free();	
				if(listnode) listnode->free(); 
				if(channelnode) channelnode->free(); 
				if(dbIpref) dbIpref->free();	
				return FALSE;
			}
			tmpLength+= STVPlaylist::Cue2Sec(buff);	// add length

			if(pTmpElement) pTmpElement->free();
		}
		if(pTmpAsset) pTmpAsset->free();
	}
		
	// set total playlist length, bottom asset, top asset
	_PLLength = tmpLength;
	_BottomAsset = tmpba;
	_TopAsset = tmpta;
	
	if(listnode) listnode->free();
	if(channelnode)	channelnode->free();
	if(dbIpref)	dbIpref->free();
	return retval;
}

//##ModelId=4147B84B03AA
int STVPlaylist::createDBName(ZQ::common::IPreference* channelnode, ZQ::common::IPreference* listnode, int listtype)
{
	// default db directory
	char buff[MAX_PATH];
	std::string buffstr;
	wchar_t namebuff[MAX_PATH];
	bool retval = FALSE;
	ZQ::common::IPreference* pTmpAsset;
	ZQ::common::IPreference* pTmpElement;

	
	if(wcscmp(_dbdir,L"")==0)
		wcscpy(_dbdir, _mgm->getDBpath().c_str());
		
	::CreateDirectory(_dbdir,NULL);
	
	//////////////////////////////////////////////////////////////////////////
	// set list type, channel id, and fill mode

	// prepare list type
	_PLtype = listtype;
	
	// prepare channel id
	DWORD tmpch=0;
	if(!channelnode) {
		_IsGlobal = TRUE;
		setPLchannel(-1);
	}
	else 
	{
		channelnode->get(XML_PORT_CHANNELID, buff);
		tmpch = atol(buff);
		if(channelnode->has(XML_PORT_SUBCHANNELID)) {
			channelnode->get(XML_PORT_SUBCHANNELID, buff);
			tmpch = tmpch*(_mgm->getMaxSubChnl())+atol(buff);
		}
		else {
			tmpch = tmpch*(_mgm->getMaxSubChnl());
		}
		setPLchannel(tmpch);
	}
	
	// prepare fill type ( for filler/barker only)
	if(listtype!=LISTTYPE_PLAYLIST) {
		if(listnode->has(XML_PLAYLIST_PLAYMODE)) {
			listnode->get(XML_PLAYLIST_PLAYMODE, buff);
			if(strcmp(buff, "S")==0)
				_Filltype = FILLTYPE_SERIAL;
			else if(strcmp(buff, "R")==0)
				_Filltype = FILLTYPE_RANDOM;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// set time attributes
	pTmpAsset= listnode->firstChild(XML_PLAYLIST_ASSET);
	if(!pTmpAsset) {
		return STVERRFORMAT;
	}
	
	// prepare begin time attributes
	if(_PLtype==LISTTYPE_PLAYLIST)
	{
		if(!pTmpAsset->has(XML_PLAYLIST_ASSET_BEGIN))
		{
			if(pTmpAsset) pTmpAsset->free();
			return STVERRFORMAT;
		}
		pTmpAsset->get(XML_PLAYLIST_ASSET_BEGIN, buff);
	}
	else
	{
		if(!listnode->has(XML_PLAYLIST_EFFECTTIME))
		{
			if(pTmpAsset) pTmpAsset->free();
			return STVERRFORMAT;
		}
		listnode->get(XML_PLAYLIST_EFFECTTIME, buff);
	}

	buffstr = buff;
	mbstowcs(namebuff, buff, MAX_PATH);
	_BeginTime.wYear		= atol(buffstr.substr(0,4).c_str());
	_BeginTime.wMonth		= atol(buffstr.substr(4,2).c_str());
	_BeginTime.wDay			= atol(buffstr.substr(6,2).c_str());
	_BeginTime.wHour		= atol(buffstr.substr(9,2).c_str());
	_BeginTime.wMinute		= atol(buffstr.substr(12,2).c_str());
	_BeginTime.wSecond		= atol(buffstr.substr(15,2).c_str());
	_BeginTime.wDayOfWeek= 7;	// don't calculate weekdays
	_BeginTime.wMilliseconds = 0;	// don't care about milli seconds
	
	// prepare end time ( for filler/barker only)
	if(_PLtype!=LISTTYPE_PLAYLIST) {
		if(!listnode->has(XML_PLAYLIST_ENDTIME))
		{
			if(pTmpAsset) pTmpAsset->free();
			return STVERRFORMAT;
		}
		listnode->get(XML_PLAYLIST_ENDTIME, buff);
		buffstr = buff;
		_EndTime.wYear			= atol(buffstr.substr(0,4).c_str());
		_EndTime.wMonth			= atol(buffstr.substr(4,2).c_str());
		_EndTime.wDay			= atol(buffstr.substr(6,2).c_str());
		_EndTime.wHour			= atol(buffstr.substr(9,2).c_str());
		_EndTime.wMinute		= atol(buffstr.substr(12,2).c_str());
		_EndTime.wSecond		= atol(buffstr.substr(15,2).c_str());
		_EndTime.wDayOfWeek	= 7;	// don't calculate weekdays
		_EndTime.wMilliseconds = 0;	// don't care about milli seconds
	}

	if(pTmpAsset) pTmpAsset->free();
	
	//////////////////////////////////////////////////////////////////////////
	// prepare top&bottom ae no , and total duration
	
	DWORD tmpba=0;
	DWORD tmpta=0;
	DWORD tmpLength=0;

	pTmpAsset=listnode->firstChild(XML_PLAYLIST_ASSET);
	if(pTmpAsset==NULL)		// check if has asset
	{
		return STVERRFORMAT;
	}

	for(; pTmpAsset; pTmpAsset=listnode->nextChild())
	{
		pTmpElement=pTmpAsset->firstChild(XML_PLAYLIST_ASSET_AE);
		if(pTmpElement==NULL)	// check if has element
		{
			if(pTmpAsset) pTmpAsset->free();
			return STVERRFORMAT;
		}

		for(; pTmpElement; pTmpElement=pTmpAsset->nextChild())
		{
			if(!pTmpElement->get(XML_PLAYLIST_ASSET_AE_NO, buff))
			{ 
				if(pTmpElement) pTmpElement->free();
				if(pTmpAsset) pTmpAsset->free();
				return STVERRFORMAT;
			}
			if(tmpba==0)
				tmpba=atol(buff);	// set bottom ae NO
			if(tmpta<atol(buff))
				tmpta=atol(buff);	// set current top ae NO

			if(!pTmpElement->get(XML_PLAYLIST_ASSET_AE_CUEDURATION, buff))
			{ 
				if(pTmpElement) pTmpElement->free(); 
				if(pTmpAsset) pTmpAsset->free();	
				return STVERRFORMAT;
			}
			tmpLength+= STVPlaylist::Cue2Sec(buff);	// add length

			if(pTmpElement) pTmpElement->free();
		}
		if(pTmpAsset) pTmpAsset->free();
	}
		
	// set total playlist length, bottom asset, top asset
	_PLLength = tmpLength;
	_BottomAsset = tmpba;
	_TopAsset = tmpta;
	
	//////////////////////////////////////////////////////////////////////////
	// and make up the DB file name
	if(_IsGlobal)
	{
		wcscpy(_dbfilename,L"G");
	}
	else 
	{
		switch(listtype) {
		case LISTTYPE_BARKER:
			wcscpy(_dbfilename,L"B");
			break;
		case LISTTYPE_FILLER:
			wcscpy(_dbfilename,L"F");
			break;
		case LISTTYPE_PLAYLIST:
			wcscpy(_dbfilename,L"P");
			break;
		}
	}
	
	_dbfilename[1]=namebuff[2];
	_dbfilename[2]=namebuff[3];
	_dbfilename[3]=namebuff[4];
	_dbfilename[4]=namebuff[5];
	_dbfilename[5]=namebuff[6];
	_dbfilename[6]=namebuff[7];
	_dbfilename[7]=namebuff[9];
	_dbfilename[8]=namebuff[10];
	_dbfilename[9]=namebuff[12];
	_dbfilename[10]=namebuff[13];
	_dbfilename[11]=namebuff[15];
	_dbfilename[12]=namebuff[16];
	
	if((int)_PLchannel>=0) {
		wchar_t buf[16];
		_ultow(_PLchannel, buf, 10);
		for(int ci=0; ci<wcslen(buf); ci++)
			_dbfilename[13+ci]=buf[ci];
		_dbfilename[13+ci]=L'\0';
	}
	else {
		_dbfilename[13]=L'\0';
	}
	
	wcscat(_dbfilename, L".");
	wcscat(_dbfilename, _T(DB_FILE_EXT));
	
	// create the xml directory and file
	wcscat(_dbdir, _dbfilename);
	wcscpy(_dbfilename, _dbdir);

	// check if duplicate playlist already existed
	WIN32_FIND_DATA finddata;
	int suffix =1;
	wchar_t fixbuff[8];
	HANDLE hfind= ::FindFirstFile(_dbfilename, &finddata);
	if(hfind!=INVALID_HANDLE_VALUE) {	// duplicate file existed, add suffix to file name
		::FindClose(hfind);
		int extLen = strlen(DB_FILE_EXT);
		_dbfilename[wcslen(_dbfilename)-extLen-1]=L'\0';
		wcscat(_dbfilename, L"_");
		wcscat(_dbfilename, _itow(suffix, fixbuff, 10));
		wcscat(_dbfilename, L".");
		wcscat(_dbfilename, _T(DB_FILE_EXT));
		suffix++;
		
		while( (hfind= ::FindFirstFile(_dbfilename, &finddata))!=INVALID_HANDLE_VALUE) { // found
			::FindClose(hfind);
			_itow(suffix, fixbuff, 10);
			_dbfilename[wcslen(_dbfilename)-5]=fixbuff[0];
			suffix++;
		}
		::FindClose(hfind);
	}
	else {
		::FindClose(hfind);
	}
	
	// create new file
	HANDLE hf=::CreateFile(_dbfilename, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_NEW, 0, NULL);
	if(INVALID_HANDLE_VALUE ==hf) {
		char fname[MAX_PATH];
		size_t dwCount = MAX_PATH;
		wcstombs(fname, _dbfilename, dwCount);
		
		throw ZQ::common::Exception(std::string("can not create file ")+fname);
		return STVPLDBFAIL;
	}
	::CloseHandle(hf);
	
	return STVSUCCESS;
}


//##ModelId=4147B84B0399
bool STVPlaylist::isMyChannel(std::string deviceid, std::string ipaddr, std::string ipport)
{
	char buff[MAX_PATH];
	int	matchnum=0;

	ZQ::common::IPreference* dbIpref = _dbdoc->root();
	ZQ::common::IPreference* pChnl = dbIpref->firstChild(XML_PORT);
	if(!pChnl)
		return false;

	// compare the ipaddr and ipport with address and port in XML
	if(pChnl->has(XML_PORT_IPADRESS)) {
		pChnl->get(XML_PORT_IPADRESS, buff);
		if(ipaddr == buff) 
			matchnum++;
	}
	if(pChnl->has(XML_PORT_IPPORT)) {
		pChnl->get(XML_PORT_IPPORT, buff);
		if(ipport == buff) 
			matchnum++;
	}
	if(pChnl->has(XML_PORT_DEVICEID)) {
		pChnl->get(XML_PORT_DEVICEID, buff);
		if(deviceid == buff) 
			matchnum++;
	}

	if(pChnl) pChnl->free();
	if(dbIpref) dbIpref->free();
	return (matchnum==3);
}

//##ModelId=4147B84B03A9
SYSTEMTIME STVPlaylist::getNextTime()
{
	return _BeginTime;
}

bool STVPlaylist::isInEffective(SYSTEMTIME currtime)
{
	if(_PLtype==LISTTYPE_PLAYLIST)	// has no effective time
		return TRUE;

	int comp1,comp2;
	comp1 = STVPlaylistManager::timeComp(currtime,_BeginTime);
	comp2 = STVPlaylistManager::timeComp(_EndTime, currtime);
	if((comp1>=0)&&(comp2>=0))
		return TRUE;
		
	return FALSE;
}

bool STVPlaylist::updatePlayingElem(const SSAENotification feedback, char* backstream, DWORD* backcount)
{
	char xmlbuff[32];
	
	if(feedback.dwStatus == SAENS_FAIL)
		return FALSE;

	ZQ::common::IPreference* dbIpref = _dbdoc->root();
	ZQ::common::IPreference* pList;
	
	if(!dbIpref)
		return FALSE;
	
	if(_PLtype != LISTTYPE_FILLER)
		pList = dbIpref->firstChild(XML_PLAYLIST);
	if(_PLtype == LISTTYPE_FILLER)
		pList = dbIpref->firstChild(XML_FILLERLIST);
	if(!pList) {
		if(dbIpref) dbIpref->free();
		return FALSE;
	}

	std::map<DWORD, ChannelInfo>::iterator iter = _mgm->_mapChnl.find(feedback.dwPurchaseID);
	if(iter==_mgm->_mapChnl.end()) {
		if(dbIpref) dbIpref->free();
		if(pList) pList->free();
		return FALSE;	
	}

	char backbuff[64];
	ZQ::common::IPreference* pBack = _dbdoc->newElement(XML_STATUSINFO);
	ZQ::common::IPreference* pBackPort = _dbdoc->newElement(XML_PORT);
	ZQ::common::IPreference* pBackEvent = _dbdoc->newElement(XML_STATUSINFO_EVENT);
	ZQ::common::IPreference* pBackAsset = _dbdoc->newElement(XML_PLAYLIST_ASSET);
	ZQ::common::IPreference* pBackAE = _dbdoc->newElement(XML_PLAYLIST_ASSET_AE);
	ChannelInfo tmpCh= iter->second;
	
	// set event info
	SYSTEMTIME currTime;
	::GetLocalTime(&currTime);
	sprintf(xmlbuff, "%.4d%.2d%.2d %.2d:%.2d:%.2d", 
		currTime.wYear, currTime.wMonth, currTime.wDay, 
		currTime.wHour, currTime.wMinute, currTime.wSecond);
	pBackEvent->set(XML_STATUSINFO_EVENT_STATUS, "1");
	pBackEvent->set(XML_STATUSINFO_EVENT_TIME, xmlbuff);
	
	// make up port info
	ultoa(feedback.dwPurchaseID/MAX_SUBCHNL, backbuff, 10);
	pBackPort->set(XML_PORT_CHANNELID, backbuff);	// channel id
	if(feedback.dwPurchaseID%MAX_SUBCHNL) {
		ultoa(feedback.dwPurchaseID%MAX_SUBCHNL, backbuff, 10);
		pBackPort->set(XML_PORT_SUBCHANNELID, backbuff);	// sub-channel id
	}
	ultoa(tmpCh.nodegroup, backbuff, 10);
	pBackPort->set(XML_PORT_DEVICEID, backbuff);	// nodegroup id
	pBackPort->set(XML_PORT_IPADRESS, tmpCh.ipaddr);	// ip address
	ultoa(tmpCh.ipport, backbuff, 10);
	pBackPort->set(XML_PORT_IPPORT, backbuff);		// ip port

	pBack->addNextChild(pBackPort);
	if(pBackPort) pBackPort->free();
	
	// search for current playing asset
	ZQ::common::IPreference* pCurrAsset;
	for(pCurrAsset = pList->firstChild(XML_FILLERLIST_ASSET); pCurrAsset; pCurrAsset = pList->nextChild() ) 
	{		
		ZQ::common::IPreference* pCurrAE;
		for(pCurrAE= pCurrAsset->firstChild(XML_FILLERLIST_ASSET_AE); pCurrAE; pCurrAE = pCurrAsset->nextChild() ) {
			pCurrAE->get(XML_FILLERLIST_ASSET_AE_ID, xmlbuff);
			DWORD tmpAEID;
			tmpAEID = atol(xmlbuff);
			if( tmpAEID==feedback.dwAeUID ) 
			{	// is the current playing asset element
				
				// make up back asset
				pCurrAsset->get(XML_PLAYLIST_ASSET_ID, backbuff);	
				pBackAsset->set(XML_PLAYLIST_ASSET_ASSETID, backbuff);	// asset id
				pCurrAsset->get(XML_PLAYLIST_ASSET_NO, backbuff);
				pBackAsset->set(XML_PLAYLIST_ASSET_NO, backbuff);		// asset NO
				if(_PLtype == LISTTYPE_FILLER)		// asset isFiller
					pBackAsset->set(XML_PLAYLIST_ASSET_ISFILLER, "1");
				else
					pBackAsset->set(XML_PLAYLIST_ASSET_ISFILLER, "2");
				
				pCurrAE->get(XML_PLAYLIST_ASSET_AE_ID, backbuff);
				pBackAE->set(XML_PLAYLIST_ASSET_AE_ID, backbuff);	// ae id
				pCurrAE->get(XML_PLAYLIST_ASSET_AE_NO, backbuff);
				pBackAE->set(XML_PLAYLIST_ASSET_AE_NO, backbuff);	// ae NO
														
				switch(feedback.wOperation) {
				case SAENO_PLAY:
					pBackEvent->set(XML_STATUSINFO_EVENT_ID, "1");
					break;
				case SAENO_STOP:
					pBackEvent->set(XML_STATUSINFO_EVENT_ID, "2");
					break;
				case SAENO_ABORT:
					pBackEvent->set(XML_STATUSINFO_EVENT_ID, "3");
					break;
				}
				pBackAsset->addNextChild(pBackAE);
				if(pBackAE) pBackAE->free();
				pBack->addNextChild(pBackEvent);
				if(pBackEvent) pBackEvent->free();
				pBack->addNextChild(pBackAsset);
				if(pBackAsset) pBackAsset->free();
				pBack->toStream(backstream, backcount);
				if(pBack) pBack->free();

				if(pCurrAE) pCurrAE->free();
				if(pCurrAsset) pCurrAsset->free();
				if(pList) pList->free();
				if(dbIpref) dbIpref->free();
				return TRUE;
			}
			if(pCurrAE) pCurrAE->free();
		}
		if(pCurrAsset) pCurrAsset->free();
	}
//	}

	if(pBackAE) pBackAE->free();
	if(pBackAsset) pBackAsset->free();
	if(pBackEvent) pBackEvent->free();
	if(pBack) pBack->free();	
	
	if(pList) pList->free();
	if(dbIpref) dbIpref->free();
	return FALSE;
}
bool STVPlaylist::updatePlayingElemEx(SSAENotification feedback, char* backstream, DWORD* backcount)
{
	char xmlbuff[32];
	
	if(feedback.dwStatus == SAENS_FAIL)
		return FALSE;

	ZQ::common::IPreference* dbIpref = _dbdoc->root();
	ZQ::common::IPreference* pList;
	
	if(!dbIpref)
		return FALSE;
	
	if(_PLtype != LISTTYPE_FILLER)
		pList = dbIpref->firstChild(XML_PLAYLIST);
	if(_PLtype == LISTTYPE_FILLER)
		pList = dbIpref->firstChild(XML_FILLERLIST);
	if(!pList) {
		if(dbIpref) dbIpref->free();
		return FALSE;
	}

	std::map<DWORD, ChannelInfo>::iterator iter = _mgm->_mapChnl.find(feedback.dwPurchaseID);
	if(iter==_mgm->_mapChnl.end()) {
		if(dbIpref) dbIpref->free();
		if(pList) pList->free();
		return FALSE;	
	}

	char backbuff[64];
	ZQ::common::IPreference* pBack = _dbdoc->newElement(XML_STATUSINFO);
	ZQ::common::IPreference* pBackPort = _dbdoc->newElement(XML_PORT);
	ZQ::common::IPreference* pBackEvent = _dbdoc->newElement(XML_STATUSINFO_EVENT);
	ZQ::common::IPreference* pBackAsset = _dbdoc->newElement(XML_PLAYLIST_ASSET);
	ZQ::common::IPreference* pBackAE = _dbdoc->newElement(XML_PLAYLIST_ASSET_AE);
	ChannelInfo tmpCh= iter->second;
	
	// set event info
	SYSTEMTIME currTime;
	::GetLocalTime(&currTime);
	sprintf(xmlbuff, "%.4d%.2d%.2d %.2d:%.2d:%.2d", 
		currTime.wYear, currTime.wMonth, currTime.wDay, 
		currTime.wHour, currTime.wMinute, currTime.wSecond);
	pBackEvent->set(XML_STATUSINFO_EVENT_STATUS, "1");
	pBackEvent->set(XML_STATUSINFO_EVENT_TIME, xmlbuff);
	
	// make up port info
	ultoa(feedback.dwPurchaseID/MAX_SUBCHNL, backbuff, 10);
	pBackPort->set(XML_PORT_CHANNELID, backbuff);	// channel id
	if(feedback.dwPurchaseID%MAX_SUBCHNL) {
		ultoa(feedback.dwPurchaseID%MAX_SUBCHNL, backbuff, 10);
		pBackPort->set(XML_PORT_SUBCHANNELID, backbuff);	// sub-channel id
	}
	ultoa(tmpCh.nodegroup, backbuff, 10);
	pBackPort->set(XML_PORT_DEVICEID, backbuff);	// nodegroup id
	pBackPort->set(XML_PORT_IPADRESS, tmpCh.ipaddr);	// ip address
	ultoa(tmpCh.ipport, backbuff, 10);
	pBackPort->set(XML_PORT_IPPORT, backbuff);		// ip port

	pBack->addNextChild(pBackPort);
	if(pBackPort) pBackPort->free();
	
	// search for current playing asset
	ZQ::common::IPreference* pCurrAsset;
	bool elementWrong = FALSE;
	for(pCurrAsset = pList->firstChild(XML_FILLERLIST_ASSET); pCurrAsset; pCurrAsset = pList->nextChild() ) 
	{		
		ZQ::common::IPreference* pCurrAE;
		for(pCurrAE= pCurrAsset->firstChild(XML_FILLERLIST_ASSET_AE); pCurrAE; pCurrAE = pCurrAsset->nextChild() ) {
			pCurrAE->get(XML_FILLERLIST_ASSET_AE_NO, xmlbuff);
			DWORD tmpNO = atol(xmlbuff);
			if( (tmpNO==_PlayingAsset||_PlayingAsset==0)&& !elementWrong ) {	// is the current playing asset element
				if(_PlayingAsset==0) {	// if no ae before was playing, set to first ae
//					printf("playing ae NO reset to %x\n",tmpNO);
					setPlayingAsset(tmpNO);
				}
				pCurrAE->get(XML_FILLERLIST_ASSET_AE_ID, xmlbuff);
				DWORD tmpAEID;
				tmpAEID = atol(xmlbuff);
				if(tmpAEID!=feedback.dwAeUID) {
					//printf("ae %x not match\n",tmpAEID);
					// element wrong ! some status must have been skipped, so search for proper asset
					elementWrong = TRUE;
				}
				else {
//					printf("ae %x match\n",tmpAEID);
					// make up back asset
					pCurrAsset->get(XML_PLAYLIST_ASSET_ID, backbuff);	
					pBackAsset->set(XML_PLAYLIST_ASSET_ASSETID, backbuff);	// asset id
					pCurrAsset->get(XML_PLAYLIST_ASSET_NO, backbuff);
					pBackAsset->set(XML_PLAYLIST_ASSET_NO, backbuff);		// asset NO
					if(_PLtype == LISTTYPE_FILLER)		// asset isFiller
						pBackAsset->set(XML_PLAYLIST_ASSET_ISFILLER, "1");
					else
						pBackAsset->set(XML_PLAYLIST_ASSET_ISFILLER, "2");
					
					pCurrAE->get(XML_PLAYLIST_ASSET_AE_ID, backbuff);
					pBackAE->set(XML_PLAYLIST_ASSET_AE_ID, backbuff);	// ae id
					pCurrAE->get(XML_PLAYLIST_ASSET_AE_NO, backbuff);
					pBackAE->set(XML_PLAYLIST_ASSET_AE_NO, backbuff);	// ae NO
															
					// element right, everything is perfect. finish work and return
					switch(feedback.wOperation) {
					case SAENO_PLAY:
						pBackEvent->set(XML_STATUSINFO_EVENT_ID, "1");
						// ok, no operation
						break;
					case SAENO_STOP:
						pBackEvent->set(XML_STATUSINFO_EVENT_ID, "2");
						// search next asset element, mark it as current playing
						if(pCurrAsset->hasNextChild()) {	// another AE in same asset
							if(pCurrAE) pCurrAE->free();
							pCurrAE = pCurrAsset->nextChild();
							pCurrAE->get(XML_FILLERLIST_ASSET_AE_NO, xmlbuff);
							setPlayingAsset(atol(xmlbuff));
						}
						else {	// no more AE in this asset
							if(pList->hasNextChild()) {	// another asset
								if(pCurrAsset) pCurrAsset->free();
								pCurrAsset = pList->nextChild();
								if(pCurrAE) pCurrAE->free();
								pCurrAE = pCurrAsset->firstChild(XML_FILLERLIST_ASSET_AE);
								pCurrAE->get(XML_FILLERLIST_ASSET_AE_NO, xmlbuff);
								setPlayingAsset(atol(xmlbuff));
							}
							else {	// no more asset in playlist
								if(_PLtype==LISTTYPE_PLAYLIST)
									setPlayingAsset(0);
								else
									setPlayingAsset(0);
							}
						}
						break;
					case SAENO_ABORT:
						pBackEvent->set(XML_STATUSINFO_EVENT_ID, "3");
						setPlayingAsset(0);
						break;
					}
					pBackAsset->addNextChild(pBackAE);
					if(pBackAE) pBackAE->free();
					pBack->addNextChild(pBackEvent);
					if(pBackEvent) pBackEvent->free();
					pBack->addNextChild(pBackAsset);
					if(pBackAsset) pBackAsset->free();
					pBack->toStream(backstream, backcount);
					if(pBack) pBack->free();

					if(pCurrAE) pCurrAE->free();
					if(pCurrAsset) pCurrAsset->free();
					if(pList) pList->free();
					if(dbIpref) dbIpref->free();
					return TRUE;
				}
			}
			else if(elementWrong) {
				pCurrAE->get(XML_FILLERLIST_ASSET_AE_ID, xmlbuff);
				DWORD tmpAEID;
				tmpAEID = atol(xmlbuff);
				if(tmpAEID==feedback.dwAeUID) {	// found correct element after wrong element
					// make up back asset
					pCurrAsset->get(XML_PLAYLIST_ASSET_ID, backbuff);	
					pBackAsset->set(XML_PLAYLIST_ASSET_ASSETID, backbuff);	// asset id
					pCurrAsset->get(XML_PLAYLIST_ASSET_NO, backbuff);
					pBackAsset->set(XML_PLAYLIST_ASSET_NO, backbuff);	// asset NO
					if(_PLtype == LISTTYPE_FILLER)		// asset isFiller
						pBackAsset->set(XML_PLAYLIST_ASSET_ISFILLER, "1");
					else
						pBackAsset->set(XML_PLAYLIST_ASSET_ISFILLER, "2");
					
					pCurrAE->get(XML_PLAYLIST_ASSET_AE_ID, backbuff);
					pBackAE->set(XML_PLAYLIST_ASSET_AE_ID, backbuff);	// ae id
					pCurrAE->get(XML_PLAYLIST_ASSET_AE_NO, backbuff);
					pBackAE->set(XML_PLAYLIST_ASSET_AE_NO, backbuff);	// ae NO
										
					switch(feedback.wOperation) {
					case SAENO_PLAY:
						pBackEvent->set(XML_STATUSINFO_EVENT_ID, "1");
						// ok
						pCurrAE->get(XML_FILLERLIST_ASSET_AE_NO, xmlbuff);
						setPlayingAsset(atol(xmlbuff));
						break;
					case SAENO_STOP:
						pBackEvent->set(XML_STATUSINFO_EVENT_ID, "2");
						// search next asset element, mark it as current playing
						if(pCurrAsset->hasNextChild()) {	// another AE in same asset
							if(pCurrAE) pCurrAE->free();
							pCurrAE = pCurrAsset->nextChild();
							pCurrAE->get(XML_FILLERLIST_ASSET_AE_NO, xmlbuff);
							setPlayingAsset(atol(xmlbuff));
						}
						else {	// no more AE in this asset
							if(pList->hasNextChild()) {	// another asset
								if(pCurrAsset) pCurrAsset->free();
								pCurrAsset = pList->nextChild();
								if(pCurrAE) pCurrAE->free();
								pCurrAE = pCurrAsset->firstChild(XML_FILLERLIST_ASSET_AE);
								pCurrAE->get(XML_FILLERLIST_ASSET_AE_NO, xmlbuff);
								setPlayingAsset(atol(xmlbuff));
							}
							else {	// no more asset in playlist
								if(_PLtype==LISTTYPE_PLAYLIST)
									setPlayingAsset(_TopAsset+1);
								else
									setPlayingAsset(0);
							}
						}
						break;
					case SAENO_ABORT:
						pBackEvent->set(XML_STATUSINFO_EVENT_ID, "3");
						setPlayingAsset(0);
						break;
					}
					pBackAsset->addNextChild(pBackAE);
					if(pBackAE) pBackAE->free();
					pBack->addNextChild(pBackEvent);
					if(pBackEvent) pBackEvent->free();
					pBack->addNextChild(pBackAsset);
					if(pBackAsset) pBackAsset->free();
					pBack->toStream(backstream, backcount);
					if(pBack) pBack->free();
					
					if(pCurrAE) pCurrAE->free();
					if(pCurrAsset) pCurrAsset->free();
					if(pList) pList->free();
					if(dbIpref) dbIpref->free();
					return TRUE;
				}
			}
			if(pCurrAE) pCurrAE->free();
		}
		if(pCurrAsset) pCurrAsset->free();
	}
//	}

	if(pBackAE) pBackAE->free();
	if(pBackAsset) pBackAsset->free();
	if(pBackEvent) pBackEvent->free();
	if(pBack) pBack->free();	
	
	if(pList) pList->free();
	if(dbIpref) dbIpref->free();
	return FALSE;
}

void STVPlaylist::setStatus(std::string name, std::string key, std::string value)
{
	ZQ::common::IPreference* dbIpref = _dbdoc->root();
	ZQ::common::IPreference* dbStatusPref = dbIpref->firstChild(XML_STATUS);
	ZQ::common::IPreference* pCurr=dbStatusPref->firstChild(name.c_str());
	if(!pCurr) {	// node not exist, create a new node
		pCurr =_dbdoc->newElement(name.c_str());
		dbStatusPref->addNextChild(pCurr);
	}
	
	pCurr->set(key.c_str(),value.c_str());
	
	updateToDB();

	if(pCurr) pCurr->free();
	if(dbStatusPref) dbStatusPref->free();
	if(dbIpref) dbIpref->free();
}

bool STVPlaylist::getStatus(std::string name, std::string key, std::string& value)
{
	ZQ::common::IPreference* dbIpref = _dbdoc->root();
	ZQ::common::IPreference* dbStatusPref = dbIpref->firstChild(XML_STATUS);
	ZQ::common::IPreference* pCurr=dbStatusPref->firstChild(name.c_str());
	if(!pCurr) {	// node not exist, create a new node
		return FALSE;
	}
	char buff[MAX_PATH];
	pCurr->get(key.c_str(),buff);
	value = buff;
		
	if(pCurr) pCurr->free();
	if(dbStatusPref) dbStatusPref->free();
	if(dbIpref) dbIpref->free();
	if(value.empty())
		return FALSE;
	return TRUE;
}

/*
bool STVPlaylist::getChnlInfo(std::string& deviceid, std::string& ipaddr, std::string& port)
{
	// do not use channel map to store channel info
	char buff[MAX_PATH];
	int	matchnum=0;

	ZQ::common::IPreference* pChnl = dbIpref->firstChild(XML_PORT);
	if(!pChnl)
		return false;

	if(pChnl->has(XML_PORT_IPADRESS)) {
		pChnl->get(XML_PORT_IPADRESS, buff);
		ipaddr = buff;
		matchnum++;
	}
	if(pChnl->has(XML_PORT_IPPORT)) {
		pChnl->get(XML_PORT_IPPORT, buff);
		port = buff;
		matchnum++;
	}
	if(pChnl->has(XML_PORT_DEVICEID)) {
		pChnl->get(XML_PORT_DEVICEID, buff);
		deviceid = buff;
		matchnum++;
	}

	pChnl->free();
	return (matchnum==3);
}
*/

int STVPlaylist::OnPLIdsSession(PASSETS pAssetList)
{
	ASSET Asset;
	//char buff[1024];

	glog(ZQ::common::Log::L_DEBUG, L"PENDING  STVPlaylist::OnPLIdsSession()  Begin construct AE for playlist %s", _dbfilename);
	// fetch Asset info or AE info(if already has AE)
	if(!fetchAE(pAssetList)) {
		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylist::OnPLIdsSession()  Can not fetch AE");
		return STVINVALIDPL;
	}
	glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPlaylist::OnPLIdsSession()  AE fetched:");
	for( size_t i=0; i< pAssetList->size(); i++) {
		glog(ZQ::common::Log::L_DEBUG, "\tAsset: 0x%x", pAssetList->at(i).dwAssetUID);
		for( size_t j=0; j< pAssetList->at(i).AssetElements.size(); j++) {
			glog(ZQ::common::Log::L_DEBUG, "\t\tElement: 0x%x", pAssetList->at(i).AssetElements[j].dwAEUID);
		}
	}
/*
	if(!_SetIDS) {	// playlist had not IDS queried, query it and save
		// query IDS
		HRESULT retae= _mgm->getBuilder()->getElementList(pAssetList);
		if(retae!= S_OK)
			return STVIDSERROR;
		
		// if this is a filler set, set _SetIDS flag to TRUE to avoid duplicate IDS query
		if(_PLtype==LISTTYPE_FILLER) {
			if(!attachAE(*pAssetList))
				return STVINVALIDPL;
		}
	}
*/
	return STVSUCCESS;
}

/*
bool STVPlaylist::attachAE(ASSETS listasset)
{
	ZQ::common::IPreference* dbIpref = _dbdoc->root();
	ZQ::common::IPreference* pList;
	if(_PLtype==LISTTYPE_FILLER)
		pList = dbIpref->firstChild(XML_FILLERLIST);
	else
		pList = dbIpref->firstChild(XML_PLAYLIST);

	if(!pList)
		return FALSE;
	
	ZQ::common::IPreference* pAsset = pList->firstChild(XML_FILLERLIST_ASSET);
	for(int i=0; i<listasset.size(); i++) {
		char buff[64];
		// set Asset duration
		sprintf(buff, "%x", listasset[i].dwPlayTime);
		pAsset->set(XML_FILLERLIST_ASSET_DURATION, buff);
		// set ae count
		int aecount = (int)listasset[i].AssetElements.size();
		itoa( aecount, buff, 10);
		pAsset->set(XML_FILLERLIST_ASSET_AENUMBER, buff);
		// set ae
		for( int j=0; j<aecount; j++ ) {
			ZQ::common::IPreference* pAe = _dbdoc->newElement(XML_FILLERLIST_ASSET_AE);
			// NO
			itoa( j+1, buff, 10);
			pAe->set(XML_FILLERLIST_ASSET_AE_NO, buff);
			// ID
			sprintf(buff, "%x", listasset[i].AssetElements[j].dwAEUID);
			pAe->set(XML_FILLERLIST_ASSET_AE_ID, buff);
			// Duration
			sprintf(buff, "%x", listasset[i].AssetElements[j].dwPlayTime);
			pAe->set(XML_FILLERLIST_ASSET_AE_DURATION, buff);
			// BitRate
			sprintf(buff, "%x", listasset[i].AssetElements[j].dwBitRate);
			pAe->set(XML_FILLERLIST_ASSET_AE_BITRATE, buff);
			
			// append
			pAsset->addNextChild(pAe);
			pAe->free();
		}

		if(i!=(listasset.size()-1))
			pAsset = pList->nextChild();
		
	}
	pAsset->free();
	pList->free();
	dbIpref->free();

	_SetIDS = TRUE;
	setStatus(XML_STATUS_CURRENT, XML_STATUS_CURRENT_IDS, "1");
	updateToDB();
	return TRUE;
}
*/

bool STVPlaylist::fetchAE(PASSETS plistasset)
{
	try
	{
		ZQ::common::IPreference* dbIpref = _dbdoc->root();
		ZQ::common::IPreference* pList;
		ZQ::common::IPreference* pAsset;
		if(_PLtype==LISTTYPE_FILLER)
			pList = dbIpref->firstChild(XML_FILLERLIST);
		else
			pList = dbIpref->firstChild(XML_PLAYLIST);

		if(!pList) {
			glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylist::fetchAE()  Playlist format error");
			return FALSE;
		}
		glog(ZQ::common::Log::L_DEBUG, L"NOTIFY   STVPlaylist::fetchAE()  Playlist %s begin fetch", _dbfilename);
		for(pAsset=pList->firstChild(XML_FILLERLIST_ASSET);pAsset; pAsset=pList->nextChild() ) {
			char buff[64];
			ASSET assnode;
			bool	validAsset = FALSE;
			
			// get Asset info
			if(pAsset->has(XML_FILLERLIST_ASSET_ID)) {
				pAsset->get(XML_FILLERLIST_ASSET_ID, buff);
				sscanf(buff, "%d", &(assnode.dwAssetUID));
			}
			if(pAsset->has(XML_FILLERLIST_ASSET_DURATION)) {
				pAsset->get(XML_FILLERLIST_ASSET_DURATION, buff);
				sscanf(buff, "%d", &(assnode.dwPlayTime));
			}
			if( (_PLtype==LISTTYPE_BARKER)&&(pAsset->has(XML_FILLERLIST_ASSET_PRIORITY)) ){
				pAsset->get(XML_FILLERLIST_ASSET_PRIORITY, buff);
				sscanf(buff, "%d", &(assnode.dwWeight));
				if(assnode.dwWeight<1)
					assnode.dwWeight = 1;
			}
			else {
				assnode.dwWeight = 1;
			}
			glog(ZQ::common::Log::L_DEBUG, "NOTIFY   STVPlaylist::fetchAE()  Processing asset %d", assnode.dwAssetUID);
			// get AE info
			for(ZQ::common::IPreference* pAe = pAsset->firstChild(XML_FILLERLIST_ASSET_AE); pAe; pAe = pAsset->nextChild()) {
				
				ASSETELEMENT aenode;
				DWORD hh, mm, ss, frame;
				int aeNO;
				if(pAe->has(XML_FILLERLIST_ASSET_AE_NO)) {
					pAe->get(XML_FILLERLIST_ASSET_AE_NO, buff);
					sscanf(buff, "%d", &aeNO);
				}
				
				// ID
				if(pAe->has(XML_FILLERLIST_ASSET_AE_ID)) {
					pAe->get(XML_FILLERLIST_ASSET_AE_ID, buff);
					sscanf(buff, "%d", &(aenode.dwAEUID));
				}
				// Duration
//				pAe->get(XML_FILLERLIST_ASSET_AE_DURATION, buff);
//				sscanf(buff, "%d", &(aenode.dwPlayTime));
				// BitRate
				if(pAe->has(XML_FILLERLIST_ASSET_AE_BITRATE)) {
					pAe->get(XML_FILLERLIST_ASSET_AE_BITRATE, buff);
					sscanf(buff, "%d", &(aenode.dwBitRate));
				}
				// CueIn
				if(pAe->has(XML_FILLERLIST_ASSET_AE_CUEIN)) {
					pAe->get(XML_FILLERLIST_ASSET_AE_CUEIN, buff);
					sscanf(buff, "%d:%d:%d:%d", &hh,&mm,&ss,&frame);
					aenode.dwCueIn = hh*3600+mm*60+ss;
				}
				else {
					aenode.dwCueIn = 0;
				}
				// CueOut
				if(pAe->has(XML_FILLERLIST_ASSET_AE_CUEOUT)) {
					pAe->get(XML_FILLERLIST_ASSET_AE_CUEOUT, buff);
					sscanf(buff, "%d:%d:%d:%d", &hh,&mm,&ss,&frame);
					aenode.dwCueOut = hh*3600+mm*60+ss;
				}
				else {
					aenode.dwCueOut = 0;
				}
						
				if(aeNO>=_PlayingAsset) {
					validAsset = TRUE;
					assnode.AssetElements.push_back(aenode);
				}
				if(pAe) pAe->free();
			}
			
			if(pAsset) pAsset->free();

			if(validAsset) {
				plistasset->push_back(assnode);
				assnode.AssetElements.clear();
			}
		}
		//pAsset->free();
		if(pList) pList->free();
		if(dbIpref) dbIpref->free();

	}
	catch (ZQ::common::Exception excp) {
#ifdef _DEBUG
		printf("fetch AE exception: %s", excp.getString());
#endif
		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVPlaylist::fetchAE()  Fetch AE Error, with error string: %s", excp.getString());
		return FALSE;
	}

	glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPlaylist::fetchAE()  Fetch AE ok, with %d asset(s)", plistasset->size());
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// convert Cue time format into seconds, frame was ignored
// eg:  1:20:30:115 (1 hour 20 min 30 sec 115 frame) ->  4830 seconds
DWORD STVPlaylist::Cue2Sec(std::string CueTime)
{
	DWORD dwRet=0;
	DWORD factor=3600;		// time factor, 3600 for hour, 60 for min, 1 for sec
	size_t pos;		// position for colon ':'
	std::string substr=CueTime;
	std::string timestr;

	if(substr.empty())
		return 0;
	
	// calculate hour, min and sec
	for(int i=3; i>0; i--)
	{	
		pos = substr.find_first_of(":");
		if(pos==std::string::npos)
			return 0;
		timestr = substr.substr(0, pos);
		dwRet += atol(timestr.c_str())*factor;
		substr = substr.substr(pos+1, substr.length()-pos-1);
		factor/=60;
	}
	
	return dwRet;
}