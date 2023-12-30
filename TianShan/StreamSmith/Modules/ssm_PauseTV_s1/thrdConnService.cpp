// thrdConnService.cpp: implementation of the thrdConnService class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "thrdConnService.h"
#include "ssm_PauseTV_s1.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//ZQ::common::ScLog *		thrdConnService::pLog = NULL;
ZQ::common::FileLog *		thrdConnService::pLog = NULL;
#define	myGlog			(*thrdConnService::pLog)
#define PSLOG(_X) "	" _X
#define PSLOG2(_X) "		" _X

thrdConnService::thrdConnService()
{
}

//thrdConnService::thrdConnService(ZQ::common::ScLog * log)
thrdConnService::thrdConnService(ZQ::common::FileLog * log)
{
	_event = NULL;
	pLog = log;
}

thrdConnService::~thrdConnService()
{
	stop();
}

void thrdConnService::stop()
{
	waitHandle(2000);

	if (_event)
	{
		CloseHandle(_event);
		_event = NULL;
	}
	
	CSsm_PauseTV_s1::pConnServiceThrd = NULL;
}

bool thrdConnService::init(void)
{
	myGlog(ZQ::common::Log.L_DEBUG,PSLOG2("Connect service thread starts"));
	_event =  ::CreateEvent(NULL,TRUE,FALSE,NULL);
	return true;
}

void thrdConnService::final(void)
{
	myGlog(ZQ::common::Log.L_DEBUG,PSLOG2("Connect service thread exits"));
	myGlog(ZQ::common::Log::L_DEBUG,PSLOG("****************************************	BEGIN	************************************"));
	delete(this);
}

int thrdConnService::run()
{
	timeout_t nTime = 0;
	::CSsm_PauseTV_s1::CONFIGURATION_MAP::const_iterator itConfig = 
		::CSsm_PauseTV_s1::configMap.find("RECONNECT_INTERNAL");
	if(itConfig != ::CSsm_PauseTV_s1::configMap.end())
		nTime = atol(itConfig->second.c_str());
	else
		nTime = atol(DEFAULT_RECONNECT_INTERNAL);
	while(true)
	{
		if(!CSsm_PauseTV_s1::iceInitialized)
		{
			CSsm_PauseTV_s1::InitIce();
		}
		if(!CSsm_PauseTV_s1::bConnChodsvcOK)
		{
			CSsm_PauseTV_s1::ConnChodsvc();
		}
		if(!CSsm_PauseTV_s1::bConnStreamSmithOK)
		{
			CSsm_PauseTV_s1::ConnStreamSmith();
		}
		if(!CSsm_PauseTV_s1::bConnIceStormOK)
		{
			CSsm_PauseTV_s1::ConnIceStorm();
		}
		if(CSsm_PauseTV_s1::iceInitialized && 
			CSsm_PauseTV_s1::bConnChodsvcOK && 
			CSsm_PauseTV_s1::bConnStreamSmithOK && 
			CSsm_PauseTV_s1::bConnIceStormOK)
		{
			break;
		}

		WaitForSingleObject(_event,nTime);
	}

	return 0;
}
