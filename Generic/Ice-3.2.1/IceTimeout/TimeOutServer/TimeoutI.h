#ifndef _TIME_OUT_SERVER_IMPLEMENT_HEADER_FILE_H__
#define _TIME_OUT_SERVER_IMPLEMENT_HEADER_FILE_H__
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include "TimeoutCall.h"
class TimeoutI : public test::timeout
{
public:
	TimeoutI( DWORD timeout )
		:mTimeoutMs(timeout)
	{

	}
	~TimeoutI(void)
	{
	}
	typedef IceUtil::Handle<TimeoutI> Ptr;
public:
	virtual int call(Ice::Int clientId, Ice::Int t,const ::Ice::Current& = ::Ice::Current()) 
	{
		//printf("[%s] received from client[%d], pause [%d]\n",IceUtil::Time::now().toDateTime().c_str(), clientId, t);
		Sleep( t );
		return t * 2;
	}
private:
	DWORD			mTimeoutMs;
};

#endif//_TIME_OUT_SERVER_IMPLEMENT_HEADER_FILE_H__
