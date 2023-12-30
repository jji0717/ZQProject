// ===========================================================================
// Copyright (c) 2008 by
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
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================


#include "SubjectWrite.h"
#include <vector>

using namespace ZQ::common;


namespace ZQTianShan 
{
	namespace ContentProvision
	{


SubjectWrite::SubjectWrite()
{
	_errCode = 0;
	_lastIdleStamp = GetTickCount();
	_reservationTimeout = 60*1000;
}

unsigned int SubjectWrite::getReservationTimeout()
{
	return _reservationTimeout;
}

void SubjectWrite::setReservationTimeout(unsigned int nTimeoutInMs)
{
	_reservationTimeout = nTimeoutInMs;
}

//make a reservation, then it would think it's not idle
void SubjectWrite::makeReservation(ObserverWriteI* pObserver)
{
	Guard<Mutex>  opLock(_lock);

	ObserverListType::iterator it = _observerReservation.find(pObserver);
	if (it==_observerReservation.end())
	{
		_observerList[pObserver] = GetTickCount();			
	}	
	else
	{
		it->second = GetTickCount();
	}	

	_lastIdleStamp = GetTickCount();
}

void SubjectWrite::_removeReservation(ObserverWriteI* pObserver)
{
	ObserverListType::iterator it = _observerReservation.find(pObserver);
	if (it!=_observerReservation.end())
	{
		_observerReservation.erase(it);
	}
}

void SubjectWrite::_removeExpiredReservation()
{
	std::vector<ObserverWriteI*>	toRemove;
	
	{
		ObserverListType::iterator it = _observerReservation.begin();
		for (;it!=_observerReservation.end();it++)
		{
			if (GetTickCount() - it->second > getReservationTimeout())
			{
				toRemove.push_back(it->first);			
			}
		}
	}


	{
		std::vector<ObserverWriteI*>::iterator it;
		for(it=toRemove.begin();it!=toRemove.end();it++)
		{
			_removeReservation(*it);
		}
	}
}

bool SubjectWrite::_isIdle()
{
	_removeExpiredReservation();

	return (!_observerList.size() && !_observerReservation.size());
}

bool SubjectWrite::isIdle()
{
	Guard<Mutex>  opLock(_lock);	
	return _isIdle();
}

// in mili-second
unsigned int SubjectWrite::getIdleTime()
{
	Guard<Mutex>  opLock(_lock);	
	if (!_isIdle())
		return 0;

	return GetTickCount() - _lastIdleStamp;
}

void SubjectWrite::registerObserver(ObserverWriteI* pObserver)
{
	if (!pObserver)
		return;

	Guard<Mutex>  opLock(_lock);

	_removeReservation(pObserver);

	ObserverListType::iterator it = _observerList.find(pObserver);
	if (it==_observerList.end())
	{
		_observerList[pObserver] = GetTickCount();
	}	
}

void SubjectWrite::removeObserver(ObserverWriteI* pObserver)
{
	Guard<Mutex>  opLock(_lock);

	_removeReservation(pObserver);

	ObserverListType::iterator it = _observerList.find(pObserver);
	if (it!=_observerList.end())
	{
		_observerList.erase(pObserver);
	}	

	_lastIdleStamp = GetTickCount();
}

int SubjectWrite::getObserverCount()
{
	Guard<Mutex>  opLock(_lock);
	return _observerList.size();
}

void SubjectWrite::notifyObserverWrite(const std::string& file, void* pBuf, int nLen)
{
	Guard<Mutex>  opLock(_lock);
	ObserverListType::iterator it = _observerList.begin();
	for (;it!=_observerList.end();it++)
	{
		ObserverWriteI* pObserver = it->first;
		pObserver->notifyWrite(file, pBuf, nLen);
	}
}

void SubjectWrite::notifyObserverDestroy()
{
	std::string strErr;
	int nErrorCode;
	getLastError(strErr, nErrorCode);

	ObserverListType observerCopy;

	{
		Guard<Mutex>  opLock(_lock);
		observerCopy = _observerList;
	}

	ObserverListType::iterator it = observerCopy.begin();
	for (;it!=observerCopy.end();it++)
	{
		ObserverWriteI* pObserver = it->first;
		pObserver->notifyDestroy(strErr, nErrorCode);
	}
}

void SubjectWrite::removeReservation( ObserverWriteI* pObserver )
{
	Guard<Mutex>  opLock(_lock);

	_removeReservation(pObserver);
}

}}