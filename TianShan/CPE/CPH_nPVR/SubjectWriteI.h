

#ifndef _Subject_Write_INTERFACE_HEADER_
#define _Subject_Write_INTERFACE_HEADER_



#include <string>
#include "ObserverWriteI.h"

class SubjectWriteI
{
public:

	//////////////////////////////////////////////////////////////////////////
	// why need a reservation?
	// 1. because the session preparation time is a bit earlier than start time,
	//    so it is possible that, when session initialization, the leader session
	//    is there, but when session start excution, the leader session idle timeout
	//
	//////////////////////////////////////////////////////////////////////////
	
	//make a reservation, then it would think it's not idle
	virtual void makeReservation(ObserverWriteI* pObserver) = 0;

	virtual void removeReservation(ObserverWriteI* pObserver) = 0;

	// in milliseconds
	virtual unsigned int getReservationTimeout() = 0;

	virtual void setReservationTimeout(unsigned int nTimeoutInMs) = 0;

	virtual void registerObserver(ObserverWriteI* pObserver) = 0;
	virtual void removeObserver(ObserverWriteI* pObserver) = 0;
	
	virtual bool isIdle() = 0;

	// in mili-second
	virtual unsigned int getIdleTime() = 0;

	virtual int getObserverCount() = 0;

	virtual void notifyObserverWrite(const std::string& file, void* pBuf, int nLen) = 0;
	
	virtual void notifyObserverDestroy() = 0;

	virtual void getLastError(std::string& strErr, int& errCode) = 0;
	
	virtual void setLastError(const std::string& strErr, int errCode) = 0;

};




#endif