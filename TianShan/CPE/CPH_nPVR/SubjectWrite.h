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

#ifndef ZQTS_CPE_SUBJECTWRITE_H
#define ZQTS_CPE_SUBJECTWRITE_H



#include <string>
#include <map>
#include "locks.h"
#include "ObserverWriteI.h"
#include "SubjectWriteI.h"


namespace ZQTianShan 
{
	namespace ContentProvision
	{


class SubjectWrite : public SubjectWriteI
{
public:
	SubjectWrite();

	//make a reservation, then it would think it's not idle
	virtual void makeReservation(ObserverWriteI* pObserver);

	virtual void removeReservation(ObserverWriteI* pObserver);

	virtual unsigned int getReservationTimeout();

	virtual void setReservationTimeout(unsigned int nTimeoutInMs);

	virtual void registerObserver(ObserverWriteI* pObserver);
	virtual void removeObserver(ObserverWriteI* pObserver);
	
	virtual bool isIdle();

	// in mili-second
	virtual unsigned int getIdleTime();

	virtual int getObserverCount();

	virtual void notifyObserverWrite(const std::string& file, void* pBuf, int nLen);
	
	virtual void notifyObserverDestroy();
		
	virtual void getLastError(std::string& strErr, int& errCode)
	{
		strErr = _strErr;
		errCode = _errCode;
	}

	virtual void setLastError(const std::string& strErr, int errCode)
	{
		_strErr = strErr;
		_errCode = errCode;
	}

private:	


	//without lock
	bool _isIdle();
	void _removeExpiredReservation();

	void _removeReservation(ObserverWriteI* pObserver);


	std::string							_strErr;
	int									_errCode;

	typedef std::map<ObserverWriteI*, unsigned int>	ObserverListType;
	ObserverListType			_observerList;
	ObserverListType			_observerReservation;
	ZQ::common::Mutex			_lock;

	unsigned int				_lastIdleStamp;

	unsigned int				_reservationTimeout;
};


}
}

#endif