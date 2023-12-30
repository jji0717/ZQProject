// ssm_tianshan.cpp : Defines the entry point for the DLL application.
//

#include "public.h"
#include <Log.h>

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>

#if ICE_INT_VERSION/100 < 303
	#include <ice\IdentityUtil.h>
#endif


#include <StreamSmithModuleEx.h>
#include <StreamSmithAdminI.h>
#include <SSEventSinkI.h>
#include <tianshandefines.h>

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

void UnInitialize(ZQADAPTER_DECLTYPE adapter);
using namespace ZQ::StreamSmith;
EventSinkEventChannel*	_eventSinkChannel=NULL;
TianShanIce::Streamer::StreamSmithAdminPtr		_streamSmithAdmin=NULL;
ZQ::common::Log*	_SuperLog=NULL;
Ice::Identity	idStreamSmith;

void Initialize(		IPlaylistManager* pMan,
						ZQADAPTER_DECLTYPE adapter,
						std::string strTopicManagerEndPoint,
						char* svcNetID,
						ZQ::common::Log* lg,
						const std::vector<int>& SpigotsID )
{
	_SuperLog=lg;
//	UnInitialize(adapter);
	_eventSinkChannel=new EventSinkEventChannel(pMan,adapter);
	_streamSmithAdmin=new StreamSmithI(pMan,adapter,svcNetID,SpigotsID);
	_eventSinkChannel->setTopicProxyString(strTopicManagerEndPoint);
	_eventSinkChannel->registerEventSink();
	idStreamSmith = adapter->getCommunicator()->stringToIdentity("StreamSmith");
		
	Ice::CommunicatorPtr ic = adapter->getCommunicator ();


	adapter->ZQADAPTER_ADD(ic,_streamSmithAdmin,"StreamSmith");
	//adapter->ZQADAPTER_ADD(ic,_streamSmithAdmin,idStreamSmith);

	
	(*lg)(ZQ::common::Log::L_DEBUG,"create instance StreamSmith on adpater");
}
void UnInitialize(ZQADAPTER_DECLTYPE adapter)
{
	//deactivate adapter first
	if(_eventSinkChannel)
	{
		try
		{
			delete _eventSinkChannel;
			_eventSinkChannel=NULL;			
		}
		catch(...){}
	}
	Ice::ObjectPtr obj= NULL;
	try
	{ 
		obj = adapter->remove(idStreamSmith);
	}
	catch (...) 
	{
	}

	if(_streamSmithAdmin)
	{	
		_streamSmithAdmin=NULL;
	}
}