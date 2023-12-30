#include "EventHandle.h"
#include "EghSmsConfig.h"

using namespace ZQ::common;

#define EHANDLOG if(_pLog != NULL) (*_pLog)
char EventHandle::nameDef[3][10] =
{
	"id",// source net id
	"name",//event name
	"time"//event time stamp UTC
};

EventHandle::EventHandle(void)
:_pLog(NULL)
{
}

EventHandle::~EventHandle(void)
{
	UnInit();
}

bool EventHandle::Init(void)
{
	//init monitor map
	if(gConfig.eventgroups.size() == 0)
	{
		EHANDLOG(Log::L_ERROR,"EventHandle::Init() not set the EventGroup condition");
		return false;
	}
	
	//add event group
	std::string strMsgId;
	EGH_SMS::eghSms::SMSEVENTGROUP::iterator it;
	for(it = gConfig.eventgroups.begin(); it != gConfig.eventgroups.end(); it++)
	{
		std::string strMsgId;
		EGH_SMS::smsEventGroup::SMSEVENTS::iterator itE;
		for(itE = it->events.begin(); itE != it->events.end(); itE++)
		{
			strMsgId = itE->strCategory + "_" + itE->strEventId;
//			char* pUC = strupr((char*)strMsgId.c_str());

			struct EventGroup eventGInfo;
			eventGInfo.strEvent = strMsgId;
			eventGInfo.strFormat = itE->strMessage;

			EGH_SMS::smsEventGroup::SMSSUBSCRIBERS::iterator itS;
			for(itS = it->subscribers.begin();itS != it->subscribers.end(); itS++)
			{
				eventGInfo.telV.push_back(itS->strTel);
			}

			_eventGroupV.push_back(eventGInfo);

		}		
	}
/*	
	//test
	GROUPV::iterator itTest;
	for(itTest = _eventGroupV.begin(); itTest != _eventGroupV.end(); itTest++)
	{
		EHANDLOG(Log::L_INFO,"%s   %s",itTest->strEvent.c_str(),itTest->strFormat.c_str());
		std::vector<std::string>::iterator itVT;
		for(itVT = itTest->telV.begin(); itVT != itTest->telV.end(); itVT++)
			EHANDLOG(Log::L_INFO,"%s",(*itVT).c_str());
	}
*/

	//set port name,short message center number...	
	_SmsCtrl.SetPort(gConfig.portName.c_str());
	_SmsCtrl.SetCenterNum(gConfig.smsCN.c_str());
	_SmsCtrl.SetMsgLiveTime(gConfig.msgLLT);
	if(gConfig.bWideChar)
		_SmsCtrl.SetMsgUnicodeType(true);
	else
		_SmsCtrl.SetMsgUnicodeType(false);

	_SmsCtrl.SetLog(_pLog);

	if(!_SmsCtrl.init())
	{
		EHANDLOG(Log::L_ERROR,"EventHandle::Init() init SmsControl failed");
		return false;
	}
	
	if(!_SmsCtrl.start())
	{
		EHANDLOG(Log::L_ERROR,"EventHandle::Init() SmsControl can not start failed");
		return false;
	}
	
	EHANDLOG(Log::L_DEBUG,"EventHandle::Init() initialize successful");
	return true;
}

void EventHandle::UnInit(void)
{
	_SmsCtrl.close();
}

void EventHandle::SetLog(ZQ::common::Log* pLog)
{
	_pLog = pLog;
	_SmsCtrl.SetLog(pLog);
}

void EventHandle::onEvent(
        const ::std::string& category,
        ::Ice::Int eventId,
        const ::std::string& eventName,
        const ::std::string& stampUTC,
        const ::std::string& sourceNetId,
        const EventGateway::Properties& params)
{

	char chEid[50] = {0};
	sprintf(chEid,"%d",eventId);
	std::string strMsgId = category + "_";
	strMsgId += chEid;
//	char* pUC = strupr((char*)strMsgId.c_str());

	std::string strMessage;
	GROUPV::iterator it;
	for(it = _eventGroupV.begin(); it != _eventGroupV.end(); it++)
	{
		if(stricmp(strMsgId.c_str(), (it->strEvent).c_str()) == 0)
		{
			strMessage = it->strFormat;
			size_t nBeg = 0;
			size_t nEnd = 0;
			//replace the "${*}" content
			while((nBeg = strMessage.find("${",nBeg+1)) != std::string::npos)
			{
				nEnd = strMessage.find("}",nBeg+2);
				if(nEnd == std::string::npos)
					continue;

				if(nEnd - nBeg <= 2)
					continue;
				
				bool bFind = false;
				
				//find in basic item, id ,name ,or time
				for(size_t nC = 0; nC < sizeof(nameDef)/sizeof(nameDef[0]); nC++)
				{
					if(stricmp(nameDef[nC],(strMessage.substr(nBeg+2,nEnd-nBeg-2)).c_str()) == 0)
					{
						bFind = true;
						if(nC == 0)//id
							strMessage.replace(nBeg,nEnd-nBeg+1,sourceNetId);
						else if(nC == 1)//name
							strMessage.replace(nBeg,nEnd-nBeg+1,eventName);
						else//time
							strMessage.replace(nBeg,nEnd-nBeg+1,stampUTC);
						break;
					}

				}

				if(bFind)
					continue;

				//find from properties
				EventGateway::Properties::const_iterator itP;
				for(itP = params.begin(); itP != params.end(); itP++)
				{
					if(stricmp(((*itP).first).c_str(),(strMessage.substr(nBeg+2,nEnd-nBeg-2)).c_str()) == 0)
					{
						bFind = true;
						strMessage.replace(nBeg,nEnd-nBeg+1,itP->second);
						break;
					}
				}
				if(!bFind)
					EHANDLOG(Log::L_WARNING,"EventHandle::onEvent() not match '%s' from event properties",(strMessage.substr(nBeg,nEnd-nBeg+1)).c_str());

			}

			std::vector<std::string>::iterator itT;
			for(itT = it->telV.begin(); itT != it->telV.end(); itT++)
			{
				EHANDLOG(Log::L_DEBUG,"EventHandle::onEvent() add a message to send '%s' content '%s'",(*itT).c_str(),strMessage.c_str());
				_SmsCtrl.AddMsg((*itT).c_str(),strMessage.c_str());
			}
		}
	}

}

