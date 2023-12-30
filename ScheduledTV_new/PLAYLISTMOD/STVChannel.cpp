// STVChannel.cpp: implementation of the STVChannel class.
//
//////////////////////////////////////////////////////////////////////

#include "STVChannel.h"
#include "STVPlaylistManager.h"
#include "../MainCtrl/ScheduleTV.h"
#include <algorithm>

extern ScheduleTV gSTV;

std::string	STVChannel::_basepath = DB_DEFAULT_PATH;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

STVChannel::STVChannel(DWORD chid, const char* ip, const char* port, const char* nodegroup, const char* portid)
:_chnlID(chid), _ipStr(ip), _ipportStr(port), _nodeStr(nodegroup), _portID(portid)
{
	_status = STAT_IDLE;
	_enabled = true;

	char	idbuff[32];
	ultoa(_chnlID, idbuff, 10);
	_dbpath	= STVChannel::_basepath + idbuff;
	
	// stubs
	_stubIsAlone	= false;
	_stubIe.clear();
	_stubAE.AECount = 0;
	_stubAE.AELlist = NULL;
	_stubSchedNO	= "";
	_stubType		= LISTTYPE_UNKNOWN;
	_stubListIeIdx	= -1;
	_stubIeAssetIdx = -1;
	_stubIeAeIdx	= -1;
	_stubIeAeActive	= false;
	_stubFeedback.dwPurchaseID	= -1;
	_stubNPT		= 0;

	// create file directory
	if(_dbpath[_dbpath.length()-1] != '\\')	// add '\' if necessary
		_dbpath += "\\";

	::CreateDirectoryA(_dbpath.c_str(), NULL);
}

STVChannel::~STVChannel()
{
	_status = STAT_IDLE;
	_enabled = true;

	ZQ::common::MutexGuard	tmpGd(_listMutex);
	
	std::vector<STVList*>::iterator iter =0;
	for(iter=_normalPool.begin(); iter!=_normalPool.end(); iter++)
	{
		STVList* pExList = *iter;
		if(pExList)
		{
			delete pExList;
			*iter = NULL;
		}
	}
	_normalPool.clear();
	
	for(iter=_barkerPool.begin(); iter!=_barkerPool.end(); iter++)
	{
		STVList* pExList = *iter;
		if(pExList)
		{
			delete pExList;
			*iter = NULL;
		}
	}
	_barkerPool.clear();

	for(iter=_fillerPool.begin(); iter!=_fillerPool.end(); iter++)
	{
		STVList* pExList = *iter;
		if(pExList)
		{
			delete pExList;
			*iter = NULL;
		}
	}
	_fillerPool.clear();
}

bool STVChannel::regList(STVList* newlist)
{
	ZQ::common::MutexGuard	tmpGd(_listMutex);

	std::vector<STVList*>::const_iterator iter=0;

	switch(BASICTYPE(newlist->getType())) 
	{
	case LISTTYPE_NORMAL:
		
		iter = std::find(_normalPool.begin(), _normalPool.end(), newlist);
		if(iter != _normalPool.end())
			return false;
		
		_normalPool.push_back(newlist);

		break;
	case LISTTYPE_BARKER:

		iter = std::find(_barkerPool.begin(), _barkerPool.end(), newlist);
		if(iter != _barkerPool.end())
			return false;
		
		_barkerPool.push_back(newlist);

		break;
	case LISTTYPE_FILLER:

		iter = std::find(_fillerPool.begin(), _fillerPool.end(), newlist);
		if(iter != _fillerPool.end())
			return false;
		
		_fillerPool.push_back(newlist);

		break;
	default:
		return false;
		break;
	}

	return true;
}

bool STVChannel::unregList(STVList* exlist)
{
	ZQ::common::MutexGuard	tmpGd(_listMutex);

	std::vector<STVList*>::iterator iter=0;

	switch(BASICTYPE(exlist->getType())) 
	{
	case LISTTYPE_NORMAL:
		
		iter = std::find(_normalPool.begin(), _normalPool.end(), exlist);
		if(iter == _normalPool.end())
			return false;
		
		_normalPool.erase(iter);

		break;
	case LISTTYPE_BARKER:

		iter = std::find(_barkerPool.begin(), _barkerPool.end(), exlist);
		if(iter == _barkerPool.end())
			return false;
		
		_barkerPool.erase(iter);

		break;
	case LISTTYPE_FILLER:

		iter = std::find(_fillerPool.begin(), _fillerPool.end(), exlist);
		if(iter == _fillerPool.end())
			return false;
		
		_fillerPool.erase(iter);

		break;
	default:
		return false;
		break;
	}

	return true;
}

void STVChannel::clearExpiredLists()
{
	ZQ::common::MutexGuard		tmpGd(_listMutex);
	std::vector<STVList*>::iterator iter=0;
	
	SYSTEMTIME currtime;
	::GetLocalTime(&currtime);
	
	// scan normal list
	for(iter=_normalPool.begin(); iter!=_normalPool.end();)
	{
		int		ieidx=-1;
		DWORD	sleep;
		int result = (*iter)->timeStat(currtime, ieidx, sleep);
		
		if(result == LISTSTAT_EXPIRED)
		{		
			STVList* pEx = (*iter);
			_normalPool.erase(iter);
			if(pEx)
			{
				pEx->setDeleteFile(true);
				delete pEx;
			}
		}
		else
		{
			iter++;
		}
		
	}

	
	// scan barker list
	for(iter=_barkerPool.begin(); iter!=_barkerPool.end();)
	{
		int		ieidx=-1;
		DWORD	sleep;
		int result = (*iter)->timeStat(currtime, ieidx, sleep);
		
		if(result == LISTSTAT_EXPIRED)
		{	
			STVList* pEx = (*iter);
			_barkerPool.erase(iter);
			if(pEx)
			{
				pEx->setDeleteFile(true);
				delete pEx;
			}
		}
		else
		{
			iter++;
		}
		
	}


	// scan filler list
	for(iter=_fillerPool.begin(); iter!=_fillerPool.end();)
	{
		int		ieidx=-1;
		DWORD	sleep;
		int result = (*iter)->timeStat(currtime, ieidx, sleep);
		
		if(result == LISTSTAT_EXPIRED)
		{	
			STVList* pEx = (*iter);
			_fillerPool.erase(iter);
			if(pEx)
			{
				pEx->setDeleteFile(true);
				delete pEx;
			}
		}
		else
		{
			iter++;
		}
	}
}

bool STVChannel::clearSerialLists(const char* schNO, int listtype)
{
	ZQ::common::MutexGuard		tmpGd(_listMutex);
	std::vector<STVList*>::iterator iter=0;
	
	std::string schStr = schNO;
	size_t	pkey = schStr.find_last_of('#');	// after '#' sign, is the sequence number
	if(pkey == std::string::npos)
	{
		GTRACEERR;
		return false;
	}

	std::string seqStr = schStr.substr(pkey+1);	// if sequence number is not 0, do not clear duplicate lists
	if(seqStr != "0")
	{
		GTRACEERR;
		glog(ZQ::common::Log::L_ERROR, "STVChannel::clearSerialLists()  duplicated list with non-zero schedule sequence: %s", schNO);
		return false;
	}

	schStr = schStr.substr(0, pkey);			// sequence is 0, so clear all list with this schedule NO

	if(listtype == LISTTYPE_NORMAL)
	{
		// scan normal list
		for(iter=_normalPool.begin(); iter!=_normalPool.end();)
		{
			std::string tmpSch = (*iter)->getScheduleNO();
			if(strncmp(tmpSch.c_str(), schStr.c_str(), schStr.length())==0)
			{		
				if(_stubSchedNO == tmpSch)	// this list is playing, stop it
				{
//#pragma message(__INFO__ "move comment")
					gSTV.OnStopStream(_chnlID);
				}
				
				STVList* pEx = (*iter);
				_normalPool.erase(iter);
				if(pEx)
				{
					pEx->setDeleteFile(true);
					glog(ZQ::common::Log::L_DEBUG, "STVChannel::clearSerialLists()  previous list '%s' deleted", pEx->getDBFile().c_str());
					delete pEx;
				}
				
			}
			else
			{
				iter++;
			}
			
		}
	}
	else if(listtype == LISTTYPE_BARKER)
	{
		// scan barker list
		for(iter=_barkerPool.begin(); iter!=_barkerPool.end();)
		{
			std::string tmpSch = (*iter)->getScheduleNO();
			if(strncmp(tmpSch.c_str(), schStr.c_str(), schStr.length())==0)
			{		
				if(_stubSchedNO == tmpSch)	// this list is playing, stop it
				{
// #pragma message(__INFO__ "move comment")
					gSTV.OnStopStream(_chnlID);
				}
				STVList* pEx = (*iter);
				_barkerPool.erase(iter);
				if(pEx)
				{
					pEx->setDeleteFile(true);
					glog(ZQ::common::Log::L_DEBUG, "STVChannel::clearSerialLists()  previous list '%s' deleted", pEx->getDBFile().c_str());
					delete pEx;
				}
				
			}
			else
			{
				iter++;
			}
			
		}
	}

	else if(listtype = LISTTYPE_FILLER)
	{
		// scan filler list
		for(iter=_fillerPool.begin(); iter!=_fillerPool.end();)
		{
			std::string tmpSch = (*iter)->getScheduleNO();
			if(strncmp(tmpSch.c_str(), schStr.c_str(), schStr.length())==0)
			{		
				if(_stubSchedNO == tmpSch)	// this list is playing, stop it
				{
// #pragma message(__INFO__ "move comment")
					gSTV.OnStopStream(_chnlID);
				}
				STVList* pEx = (*iter);
				_fillerPool.erase(iter);
				if(pEx)
				{
					pEx->setDeleteFile(true);
					glog(ZQ::common::Log::L_DEBUG, "STVChannel::clearSerialLists()  previous list '%s' deleted", pEx->getDBFile().c_str());
					delete pEx;
				}
			}
			else
			{
				iter++;
			}
		}
	}
	
	return true;
}

int STVChannel::createList(ZQ::common::IPreference* listnode, const char* schNO, int listtype)
{
	// first see if this list is already existed
	STVList* pList = NULL;
	pList = queryList(schNO);

	if(pList)	// clear duplication lists
	{
		if(!clearSerialLists(schNO, listtype))
		{
			glog(ZQ::common::Log::L_ERROR, "STVChannel::createList()  Could not create list with schedule NO %s, clearSerialLists() failed", schNO);
			return STVINVALIDPL;
		}
		pList = NULL;
	}
	
	pList = new STVList(*this);	// generate list
		
	int result = pList->create(listnode, schNO, listtype);
	if(result!=STVSUCCESS)
	{	// failed
		glog(ZQ::common::Log::L_ERROR, "STVChannel::createList()  Could not create list, error %s", GetSTVErrorDesc(result));
		pList->setDeleteFile(true);
		delete pList;
	}
	else
	{	// succeeded, register into pool
		if(regList(pList))
		{
			glog(ZQ::common::Log::L_DEBUG, "STVChannel::createList()  New list '%s' created", pList->getDBFile().c_str());
		}
		else
		{
			glog(ZQ::common::Log::L_WARNING, "STVChannel::createList()  Could not register list '%s'", pList->getDBFile().c_str());
			unregList(pList);
			pList->setDeleteFile(true);
			delete pList;
		}
	}

	return result;
}

STVList* STVChannel::queryList(const char* schNO)
{
	STVList*	foundList = NULL;
	std::vector<STVList*>::iterator iter=0;

	if(NULL == schNO)
		return NULL;

	ZQ::common::MutexGuard	tmpGd(_listMutex);
	for(iter = _normalPool.begin(); iter !=_normalPool.end(); iter++)
	{
		if((*iter)->getScheduleNO() == schNO)
		{	foundList = *iter;		return foundList;	}
	}

	for(iter = _barkerPool.begin(); iter !=_barkerPool.end(); iter++)
	{
		if((*iter)->getScheduleNO() == schNO)
		{	foundList = *iter;		return foundList;;	}
	}

	for(iter = _fillerPool.begin(); iter !=_fillerPool.end(); iter++)
	{
		if((*iter)->getScheduleNO() == schNO)
		{	foundList = *iter;		return foundList;	}
	}
		
	return NULL;
}

int STVChannel::loadLists()
{
	int nRet = 0;

	std::string findstr = _dbpath + "*." + DB_FILE_EXT;
	WIN32_FIND_DATAA finddata;

	// search db for lists
	HANDLE hfind = ::FindFirstFileA(findstr.c_str(), &finddata);
	if(hfind == INVALID_HANDLE_VALUE ) {	// not found
		return 0;
	}

	for(BOOL result=true; result; result=::FindNextFileA(hfind, &finddata))
	{
		std::string namestr = _dbpath + finddata.cFileName;
		if(!STVList::isValidFile(namestr.c_str()))
			continue;
		
		STVList* pList = new STVList(*this);
		int result = pList->create(namestr.c_str());
		if(result!=STVSUCCESS)
		{	// failed
			pList->setDeleteFile(true);
			delete pList;
		}
		else
		{	// succeeded, register into pool
			if(regList(pList))
			{
				glog(ZQ::common::Log::L_DEBUG, "STVChannel::loadLists()  Existing list %s loaded", pList->getDBFile().c_str());
				nRet++;
			}
			else
			{
				glog(ZQ::common::Log::L_WARNING, "STVChannel::loadLists()  Existing list %s load failed, delete it", pList->getDBFile().c_str());
				unregList(pList);
				pList->setDeleteFile(true);
				delete pList;
			}
		}
	}
	
	::FindClose(hfind);
	
	return nRet;
}

DWORD STVChannel::extract(std::string& schStr, int& ieIndex, int& type)
{
	SYSTEMTIME	currtime, listtime; 
	STVList*	pList		= NULL;
	int			index		= -1;
	DWORD		waittime	= DEFAULT_SLEEPTIME;
	
	ZeroMemory(&currtime, sizeof(SYSTEMTIME));
	ZeroMemory(&listtime, sizeof(SYSTEMTIME));
	
	::GetLocalTime(&currtime);
	listtime.wYear = 1970;
	
	if(currtime.wHour==2 && currtime.wMinute==0 && currtime.wSecond<5)
		clearExpiredLists();	// it is mid-night, should clear all out-of-date lists
	
	ZQ::common::MutexGuard		tmpGd(_listMutex);
	ZQ::common::MutexGuard		stubGd(_stubMutex);
	std::vector<STVList*>::iterator	iter = 0;

	// query normal list
	for(iter=_normalPool.begin(); iter!=_normalPool.end(); iter++)
	{
		int		ieidx=-1;
		DWORD	sleep;
		int result = (*iter)->timeStat(currtime, ieidx, sleep);
		waittime = sleep;

		if(result == LISTSTAT_PREMATURE || result == LISTSTAT_LEISURE || result == LISTSTAT_EXPIRED)
		{	// not match time point
			continue;
		}
		else
		{
			// this ie may be the right one, anyway, choose the one latest begin time
			SYSTEMTIME	ietime;
			DWORD		ielength;
			if(!(*iter)->getIeTime(ieidx, ietime, ielength))
				continue;

			// if the list is not what is streaming now and
			// if remaining time is less than gSTV.m_dwLastChanceNPT(default 10 sec), do not choose it
			if( (*iter)->getScheduleNO() != _stubSchedNO || ieidx != _stubListIeIdx)
			{
				if(STVPlaylistManager::timeSub(currtime, ietime) > ielength-gSTV.m_dwLastChanceNPT)
					continue;
			}
			
			if(STVPlaylistManager::timeComp(ietime, listtime))
			{	// this one is later
				listtime = ietime;
				pList = (*iter);
				index = ieidx;
			}
			
		}
	}

	if(pList!=NULL)	// if found a normal list should start, no need to search for barker/filler
	{
		if(pList->getScheduleNO() == _stubSchedNO && index == _stubListIeIdx)	// it is just what is playing
		{
			schStr = pList->getScheduleNO();
			ieIndex = -1;
			type = pList->getType();
			return waittime;
		}
		else	// found a new normal list to start
		{
			schStr = pList->getScheduleNO();
			ieIndex	= index;
			type = pList->getType();
			return waittime;
		}
	}
	
	// query barker list
	for(iter=_barkerPool.begin(); iter!=_barkerPool.end(); iter++)
	{
		int		ieidx=-1;
		DWORD	sleep;
		int result = (*iter)->timeStat(currtime, ieidx, sleep);
		waittime = sleep;

		if(result == LISTSTAT_PREMATURE || result == LISTSTAT_LEISURE || result == LISTSTAT_EXPIRED)
		{	// not match timepoint
			continue;
		}
		else
		{
			// this ie may be the right one, anyway, choose the one latest begin time
			SYSTEMTIME	ietime;
			DWORD		ielength;
			if(!(*iter)->getIeTime(ieidx, ietime, ielength))
				continue;

			// if the list is not what is streaming now and
			// if remaining time is less than gSTV.m_dwLastChanceNPT(default 10 sec), do not choose it
			if( (*iter)->getScheduleNO() != _stubSchedNO || ieidx != _stubListIeIdx)
			{
				if(STVPlaylistManager::timeSub(currtime, ietime) > ielength-gSTV.m_dwLastChanceNPT)
					continue;
			}
			
			if(STVPlaylistManager::timeComp(ietime, listtime))
			{	// this one is later
				listtime = ietime;
				pList = (*iter);
				index = ieidx;
			}
		}
	}

	if(pList!=NULL)	// if found a barker list should start, no need to search for filler
	{
		if(pList->getScheduleNO() == _stubSchedNO && index == _stubListIeIdx)	// it is just what is playing
		{
			schStr = pList->getScheduleNO();
			ieIndex = -1;
			type = pList->getType();
			return waittime;
		}
		else	// found a new barker list to start
		{
			schStr = pList->getScheduleNO();
			ieIndex	= index;
			type = pList->getType();
			return waittime;
		}
	}

	// query filler list
	for(iter=_fillerPool.begin(); iter!=_fillerPool.end(); iter++)
	{
		int		ieidx=-1;
		DWORD	sleep;
		int result = (*iter)->timeStat(currtime, ieidx, sleep);
		waittime = sleep;

		if(result == LISTSTAT_PREMATURE || result == LISTSTAT_LEISURE || result == LISTSTAT_EXPIRED)
		{	// not match timepoint
			continue;
		}
		else
		{
			// this ie may be the right one, anyway, choose the one latest begin time
			SYSTEMTIME	ietime;
			DWORD		ielength;
			if(!(*iter)->getIeTime(ieidx, ietime, ielength))
				continue;

			// if the list is not what is streaming now and
			// if remaining time is less than gSTV.m_dwLastChanceNPT(default 10 sec), do not choose it
			if( (*iter)->getScheduleNO() != _stubSchedNO || ieidx != _stubListIeIdx)
			{
				if(STVPlaylistManager::timeSub(currtime, ietime) > ielength-gSTV.m_dwLastChanceNPT)
					continue;
			}
			
			if(STVPlaylistManager::timeComp(ietime, listtime))
			{	// this one is later
				listtime = ietime;
				pList = (*iter);
				index = ieidx;
			}
		}
	}

	if(pList!=NULL)
	{
		if(pList->getScheduleNO() == _stubSchedNO && index == _stubListIeIdx)	// it is just what is playing
		{
			schStr = pList->getScheduleNO();
			ieIndex = -1;
			type = pList->getType();
			return waittime;
		}
		else	// found a new filler list to start
		{
			schStr = pList->getScheduleNO();
			ieIndex	= index;
			type = pList->getType();
			return waittime;
		}
	}

	schStr = "";
	ieIndex = index;
	type = LISTTYPE_UNKNOWN;
	return waittime;
}

int STVChannel::compose(const char* schNO, int ieIndex)
{
	PAELEMENT	pAElements = NULL;
	int aeNum = 0, indxAE=0, i, j;
	DWORD maxBitRate=0;

	// query the list and get its ie
	STVList* pList = queryList(schNO);
	if(pList == NULL)
		return STVNOSCHNO;

	ZQ::common::MutexGuard	stubGd(_stubMutex);

	// clear former ie
	for(int ieidx=0; ieidx<_stubIe.size(); ieidx++)
	{
		_stubIe[ieidx].AssetElements.clear();
	}
	_stubIe.clear();

	// fetch new ie
	if(pList->fetchAE(ieIndex, _stubIe)!=STVSUCCESS)
	{
		GTRACEERR;
		return STVINVALIDPL;
	}

	int			tmpType		= BASICTYPE(pList->getType());
	DWORD		tmpLength	= STVPlaylistManager::getFillLength();
	FILLTYPE	tmpFillType	= pList->getFillType();

	if(tmpType == LISTTYPE_NORMAL)	// normal list, just compose all element
	{
		// calculate total asset element number
		for(i=0; i<_stubIe.size(); i++) 
		{
			for(j=0; j<_stubIe[i].AssetElements.size(); j++) 
			{
				aeNum++;
				if(_stubIe[i].AssetElements[j].dwBitRate>maxBitRate)
					maxBitRate = _stubIe[i].AssetElements[j].dwBitRate;
			}
		}

		// allocate buffer for elements
		pAElements = new AELEMENT[aeNum];
		for(i=0; i<_stubIe.size(); i++) {
			for(j=0; j<_stubIe[i].AssetElements.size(); j++) {
				pAElements[indxAE].AEUID					= _stubIe[i].AssetElements[j].dwAEUID;
				pAElements[indxAE].BitRate					= _stubIe[i].AssetElements[j].dwBitRate;
				pAElements[indxAE].StartPos.seconds			= _stubIe[i].AssetElements[j].dwCueIn;
				pAElements[indxAE].StartPos.microseconds	= 0;
				pAElements[indxAE].EndPos.seconds			= _stubIe[i].AssetElements[j].dwCueOut;
				pAElements[indxAE].EndPos.microseconds		= 0;
				pAElements[indxAE].PlayoutEnabled			= true;
				pAElements[indxAE].EncryptionVendor			= ITV_ENCRYPTION_VENDOR_None;
				pAElements[indxAE].EncryptionDevice			= ITV_ENCRYPTION_DEVICE_None;
				indxAE++;
			}
		}
	}
	else		// filler/barker, should confuse it first
	{
		PASSETLIST filledlist;
		filledlist.clear();
		
		if(tmpType == LISTTYPE_BARKER)	
		{
			STVPlaylistManager::getBarkerF()->setFillers(&_stubIe);
			STVPlaylistManager::getBarkerF()->fill(&filledlist, tmpLength, tmpFillType);
		}
		else if(tmpType == LISTTYPE_FILLER)
		{
			STVPlaylistManager::getAutoF()->setFillers(&_stubIe);
			STVPlaylistManager::getAutoF()->fill(&filledlist, tmpLength, tmpFillType);
		}
		
		// calculate total asset element number
		for(i=0; i<filledlist.size(); i++) {
			for(j=0; j<filledlist[i]->AssetElements.size(); j++) {
				aeNum++;
				if(filledlist[i]->AssetElements[j].dwBitRate>maxBitRate)
					maxBitRate = filledlist[i]->AssetElements[j].dwBitRate;
			}
		}
		
		// allocate buffer for elements
		pAElements = new AELEMENT[aeNum];
		for(i=0; i<filledlist.size(); i++) 
		{
			for(j=0; j<filledlist[i]->AssetElements.size(); j++) 
			{
				pAElements[indxAE].AEUID					= filledlist[i]->AssetElements[j].dwAEUID;
				pAElements[indxAE].BitRate					= filledlist[i]->AssetElements[j].dwBitRate;
				pAElements[indxAE].StartPos.seconds			= filledlist[i]->AssetElements[j].dwCueIn;
				pAElements[indxAE].StartPos.microseconds	= 0;
				pAElements[indxAE].EndPos.seconds			= filledlist[i]->AssetElements[j].dwCueOut;
				pAElements[indxAE].EndPos.microseconds		= 0;
				pAElements[indxAE].PlayoutEnabled			= true;
				pAElements[indxAE].EncryptionVendor			= ITV_ENCRYPTION_VENDOR_None;
				pAElements[indxAE].EncryptionDevice			= ITV_ENCRYPTION_DEVICE_None;
				indxAE++;
			}
		}

		filledlist.clear();
	}
	
	// update stubs
	if(_stubAE.AELlist != NULL)
	{
		delete[] (PAELEMENT)_stubAE.AELlist;
		_stubAE.AELlist = NULL;
	}
	_stubAE.AssetUID	= _stubIe[0].dwAssetUID;
	_stubAE.BitRate		= maxBitRate;
	_stubAE.AECount		= aeNum;
	_stubAE.AELlist		= (PAEARRAY)pAElements;

	_stubSchedNO		= pList->getScheduleNO();
	_stubType			= pList->getType();
	_stubListIeIdx		= ieIndex;
	_stubIeAssetIdx		= -1;
	_stubIeAeIdx		= -1;
	_stubIeAeActive		= false;
	
	SYSTEMTIME	currTime, ieTime;
	DWORD	ieLength;
	::GetLocalTime(&currTime);
	if(pList->getType()==LISTTYPE_NORMAL && pList->getIeTime(ieIndex, ieTime, ieLength))
	{
		_stubNPT = STVPlaylistManager::timeSub(currTime, ieTime);
		_stubNPT = (_stubNPT==0xFFFFFFFF || _stubNPT<DEFAULT_FIRST_CHANCE_NPT )? 0 : _stubNPT;
	}
	else
	{
		_stubNPT = 0;
	}


	return STVSUCCESS;
}

int STVChannel::updatePlayingElem(const SSAENotification& feedback, char* backstream, DWORD* backcount)
{
	int nRet = STVBADFEEDBACK;

	if(feedback.dwStatus == SAENS_FAIL)
		return STVBADFEEDBACK;

	ZQ::common::MutexGuard		tmpGd(_stubMutex);

	int startAssetIdx =0, startAeIdx =0;
	if(BASICTYPE(_stubType) == LISTTYPE_NORMAL)	// if normal list, search from current playing element
	{
		startAssetIdx	= (_stubIeAssetIdx == -1)? 0 : _stubIeAssetIdx;
		startAeIdx		= (_stubIeAeIdx == -1)? 0 : _stubIeAeIdx;
	}
	
	SYSTEMTIME	currTime;
	char		timeBuff[32]={0};
	DWORD		tmpAssetID = 0, tmpAssetNO = 0;
	DWORD		tmpElementID =0 , tmpElementNO = 0;
	DWORD		tmpIsFiller = 0;

	::GetLocalTime(&currTime);
	sprintf(timeBuff, "%.4d%.2d%.2d %.2d:%.2d:%.2d", 
		currTime.wYear, currTime.wMonth, currTime.wDay, 
		currTime.wHour, currTime.wMinute, currTime.wSecond);

	// search for current playing asset
	for(int i=startAssetIdx; i<_stubIe.size(); i++)
	{
		if(nRet == STVSUCCESS)
			break;

		for(int j=(i==startAssetIdx)?startAeIdx:0 ; j<_stubIe[i].AssetElements.size(); j++)
		{
			if(feedback.dwAeUID == _stubIe[i].AssetElements[j].dwAEUID)
			{	// match
				nRet = STVSUCCESS;
				tmpAssetID		= _stubIe[i].dwAssetUID;
				tmpAssetNO		= _stubIe[i].dwNO;
				tmpElementID	= _stubIe[i].AssetElements[j].dwAEUID;
				tmpElementNO	= _stubIe[i].AssetElements[j].dwNO;
				
				// update stubs
				if(feedback.wOperation == SAENO_PLAY)
				{
					_stubIeAssetIdx = i;
					_stubIeAeIdx	= j;
					_stubIeAeActive	= true;
				}
				else
				{
					_stubIeAssetIdx = i+1;
					_stubIeAeIdx	= 0;
					_stubIeAeActive = false;
				}

				if(BASICTYPE(_stubType)==LISTTYPE_FILLER)
					tmpIsFiller = 1;
				else
					tmpIsFiller = 2;

				break;
			}
			
		}
	}

	if(nRet != STVSUCCESS)
	{
		GTRACEERR;

		glog(ZQ::common::Log::L_DEBUG, "*----> schedule stub dump <----*");
		glog(ZQ::common::Log::L_DEBUG, "\tstubIeAssetIdx  = %d   stubIeAeIdx  = %d", _stubIeAssetIdx,_stubIeAeIdx);
		glog(ZQ::common::Log::L_DEBUG, "\tstartAssetIndex = %d   startAeIndex = %d", startAssetIdx,startAssetIdx);
		for(int ii=0; ii<_stubIe.size(); ii++)
		{
			glog(ZQ::common::Log::L_DEBUG, "\tAsset 0x%X", _stubIe[ii].dwAssetUID);
			for(int jj=0 ; jj<_stubIe[ii].AssetElements.size(); jj++)
			{
				glog(ZQ::common::Log::L_DEBUG, "\t    |- Element 0x%X", _stubIe[ii].AssetElements[jj].dwAEUID);
			}
		}
		glog(ZQ::common::Log::L_DEBUG, "*------------------------------*");
		
		return nRet;
	}

	// prepare return xml
	std::string	backStr = "";
	char portbuff[256]	={0};
	char eventbuff[128]	={0};
	char assetbuff[256]	={0};

	if(_chnlID%MAX_SUBCHNL == 0)	// no sub channel
	{
		sprintf(portbuff, "\t<%s %s=\"%ld\" %s=\"%s\" %s=\"%s\" %s=\"%s\" />\n",
			TAG_PORT, 
			KEY_PORT_CHANNELID,		_chnlID/MAX_SUBCHNL, 
			KEY_PORT_DEVICEID,		_nodeStr.c_str(),
			//KEY_PORT_PORTID,		_portID.c_str(),
			KEY_PORT_IPADRESS,		_ipStr.c_str(),
			KEY_PORT_IPPORT,		_ipportStr.c_str());
	}
	else	// has sub channel
	{
		sprintf(portbuff, "\t<%s %s=\"%ld\" %s=\"%ld\" %s=\"%s\" %s=\"%s\" %s=\"%s\" />\n",
			TAG_PORT, 
			KEY_PORT_CHANNELID,		_chnlID/MAX_SUBCHNL, 
			KEY_PORT_SUBCHANNELID,	_chnlID%MAX_SUBCHNL,
			KEY_PORT_DEVICEID,		_nodeStr.c_str(),
			//KEY_PORT_PORTID,		_portID.c_str(),
			KEY_PORT_IPADRESS,		_ipStr.c_str(),
			KEY_PORT_IPPORT,		_ipportStr.c_str());
	}
	
	sprintf(eventbuff, "\t<%s %s=\"%ld\" %s=\"%s\" %s=\"1\" %s=\"0\" />\n",
		TAG_STATUSINFO_EVENT,
		KEY_STATUSINFO_EVENT_ID,	feedback.wOperation,
		KEY_STATUSINFO_EVENT_TIME,	timeBuff,
		KEY_STATUSINFO_EVENT_STATUS,
		KEY_STATUSINFO_EVENT_ERRORCODE);

	sprintf(assetbuff, "\t<%s %s=\"%ld\" %s=\"%ld\" %s=\"%ld\">\n\t\t<%s %s=\"%ld\" %s=\"%ld\" />\n\t</%s>\n",
		TAG_STATUSINFO_ASSET,
		KEY_STATUSINFO_ASSET_NO,		tmpAssetNO,
		KEY_STATUSINFO_ASSET_ASSETID,	tmpAssetID,
		KEY_STATUSINFO_ASSET_ISFILLER,	tmpIsFiller,
		TAG_STATUSINFO_ASSET_ELE,
		KEY_STATUSINFO_ASSET_ELE_NO,	tmpElementNO,
		KEY_STATUSINFO_ASSET_ELE_ELEID,	tmpElementID,
		TAG_STATUSINFO_ASSET);

	backStr  = "<"  TAG_STATUSINFO ">\n";
	backStr += portbuff;
	backStr += eventbuff;
	backStr += assetbuff;
	backStr += "</" TAG_STATUSINFO ">";

	if(*backcount < backStr.length())
	{
		*backcount = backStr.length();
		return STVBUFFTOOSMALL;
	}

	strncpy(backstream, backStr.c_str(), *backcount);
	*backcount = backStr.length();

	_stubFeedback = feedback;
	
	return STVSUCCESS;
}

bool STVChannel::getLastFeedback(SSAENotification& feedback)
{
	ZQ::common::MutexGuard		tmpGd(_stubMutex);

	if(_stubFeedback.dwPurchaseID == -1)	// no feedback
	{
		return false;
	}
	
	feedback.dwAeUID		= _stubFeedback.dwAeUID;
	feedback.dwPurchaseID	= _stubFeedback.dwPurchaseID;
	feedback.dwStatus		= _stubFeedback.dwStatus;
	feedback.wOperation		= _stubFeedback.wOperation;
	
	return true;
}

int STVChannel::enable()
{
	if(_enabled)
		return STVSUCCESS;

	_enabled = true;
	return STVSUCCESS;
}

int STVChannel::disable()
{
	if(!_enabled)
		return STVSUCCESS;

	_enabled = false;
	return STVSUCCESS;
}

int STVChannel::collectStubs(std::string& scheNO, int& type, int& ieIndex, int& assetIndex, int&aeIndex)
{
	ZQ::common::MutexGuard	stubGd(_stubMutex);

	scheNO		= _stubSchedNO;
	type		= _stubType;
	ieIndex		= _stubListIeIdx;
	assetIndex	= _stubIeAssetIdx;
	aeIndex		= _stubIeAeIdx;

	return STVSUCCESS;
}

int STVChannel::clearStubs()
{
	ZQ::common::MutexGuard	stubGd(_stubMutex);

	for(int ieidx=0; ieidx<_stubIe.size(); ieidx++)
	{
		_stubIe[ieidx].AssetElements.clear();
	}
	_stubIe.clear();

	_stubSchedNO	= "";
	_stubType		= LISTTYPE_UNKNOWN;
	_stubListIeIdx	= -1;
	_stubIeAssetIdx = -1;
	_stubIeAeIdx	= -1;
	_stubIeAeActive	= false;
	_stubNPT		= 0;

	return STVSUCCESS;
}

DWORD STVChannel::getNpt()
{
	ZQ::common::MutexGuard	stubGd(_stubMutex);

	return	_stubNPT;
}

int STVChannel::allocAEList(AELIST& outlist)
{
	ZQ::common::MutexGuard	stubGd(_stubMutex);

	outlist.AECount		= _stubAE.AECount;
	outlist.AELlist		= _stubAE.AELlist;
	outlist.AssetUID	= _stubAE.AssetUID;
	outlist.BitRate		= _stubAE.BitRate;

	return STVSUCCESS;
}

int STVChannel::freeAEList()
{
	ZQ::common::MutexGuard	stubGd(_stubMutex);

	if(_stubAE.AELlist)
	{
		PAELEMENT pEx = (PAELEMENT)_stubAE.AELlist;
		delete[] pEx;
		_stubAE.AELlist = NULL;
	}

	return STVSUCCESS;
}