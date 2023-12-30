// thrdConnService.cpp: implementation of the thrdConnService class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "thrdConnService.h"
#include "ssmNGODr2c1.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define	MyGlog			(*this->_pLog)

thrdConnService::thrdConnService(): b_exit(false) , _event(NULL), _pSsmNGODr2c1(NULL), _pLog(NULL)
{
	_event =  ::CreateEvent(NULL,TRUE,FALSE,NULL);
}

thrdConnService::open(ssmNGODr2c1* pSsmNGODr2c1,ZQ::common::FileLog* pLog)
{
	_pSsmNGODr2c1 = pSsmNGODr2c1;
	_pLog = pLog;
}

thrdConnService::~thrdConnService()
{
	if (_event)
	{
		CloseHandle(_event);
		_event = NULL;
	}
	stop();
}

void thrdConnService::stop()
{
	b_exit = true;
	if (_event)
		::SetEvent(_event);
	::Sleep(1);
}

bool thrdConnService::init(void)
{
	MyGlog(ZQ::common::Log::L_NOTICE, CLOGFMT(thrdConnService, "connect service thread starts"));
	return (NULL != _event);
}

void thrdConnService::final(void)
{
	MyGlog(ZQ::common::Log::L_NOTICE, CLOGFMT(thrdConnService, "connect service thread exits"));
}

int thrdConnService::run()
{
	timeout_t nTime = 0;
	bool bConnSM = false, bConnIS = false;
	
	while (!b_exit)
	{
		nTime = 10;

		if (!bConnSM)
		{
			if (_pSsmNGODr2c1->ConnectSessionManager())
				bConnSM = true;
		}

		if (!b_exit && !bConnIS)
		{
			if (_pSsmNGODr2c1->ConnectIceStorm())
				bConnIS = true;
		}
		
		if (b_exit || (bConnSM && bConnIS))
			break;

		if (_event != NULL)
			WaitForSingleObject(_event, nTime * 1000);
	}

	return 0;
}

