// snmpplug.cpp : Defines the entry point for the DLL application.
//

#include "StdAfx.h"
#include "SnmpSender.h"
#include "FileLog.h"
#include <EventGwHelper.h>

#define THIS_MODULE_NAME "EGH_SnmpTrap"

using namespace std;
using namespace ZQ::common;

Config::Loader <SnmpSenderInfo> gConfig(THIS_MODULE_NAME ".xml");
ZQ::common::Log* plog = NULL;

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif

SnmpSender* pSender = NULL;

void OnSnmpMessage(const MSGSTRUCT& msgStruct)
{
	if(pSender == NULL)
		return;

 	pSender->AddMessage(msgStruct);
}

bool init(const char* pType)
{
	if(pType == NULL)
		return false;

	if((stricmp(pType,"snmp") != 0))
		return false;

    pSender = new SnmpSender();
    if(pSender) {		
        if(!pSender->GetParFromFile()) {
            glog(Log::L_ERROR,"SNMP sender get configuration error");
            delete pSender;
            pSender = NULL;

			ZQ::common::setGlogger();
			if (NULL != plog)
				delete plog;
            plog = NULL;

            return false;		
        }
        if(!pSender->init()) {
            glog(Log::L_ERROR,"SNMP sender init error");
            delete pSender;
            pSender = NULL;
				
			ZQ::common::setGlogger();
			if (NULL != plog)
				delete plog;
            plog = NULL;
				
            return false;
        }

        glog(Log::L_INFO,"SNMP sender init successful");
    }
    else {
        return false;
	}

	return true;
}

void Exit()
{
	if(pSender != NULL)
	{
//		pSender->Close();
		delete pSender;
		pSender = NULL;
		glog(Log::L_INFO,"SNMP Sender plug exit");		
		return;
		
	}

	glog(Log::L_WARNING,"SNMP sender plug has exit,can not exit again");
}
/////////////////////////////////////
// adapte to the egh interface
class SnmpTrapEventHelper: public EventGateway::IGenericEventHelper
{
public:
    virtual ~SnmpTrapEventHelper(){}

    virtual void onEvent(
                         const ::std::string& category,
                         ::Ice::Int eventId,
                         const ::std::string& eventName,
                         const ::std::string& stampUTC,
                         const ::std::string& sourceNetId,
                         const EventGateway::Properties& params
                         )
    {
        MSGSTRUCT msg;
        msg.id = eventId;
        msg.category = category;
        msg.timestamp = stampUTC;
        msg.eventName = eventName;
        msg.sourceNetId = sourceNetId;
        msg.property = params;
        OnSnmpMessage(msg);
    }
};
SnmpTrapEventHelper gTrapHelper;
#define TopicOfSnmpTrap "TianShan/Event/Generic"

EventGateway::IEventGateway* _gw;
///////////////////////////////////////////////
extern "C"
{

__EXPORT bool EventGw_Module_Entry_init(EventGateway::IEventGateway* gateway)
{
    if(!gateway) {
        return false;
    }

    _gw = gateway;

    gConfig.setLogger(&_gw->superLogger());
    if(!gConfig.loadInFolder(_gw->getConfigFolder().c_str())) {
        return false;
    }

    std::string logPath = _gw->getLogFolder() + THIS_MODULE_NAME ".log";

	try {
		plog = new ZQ::common::FileLog(logPath.c_str(),gConfig. logLevel, gConfig.logNumber, gConfig.logSize);
		ZQ::common::setGlogger(plog);
	}
	catch(FileLogException& ex) {
        _gw->superLogger()(ZQ::common::Log::L_ERROR, "Caught unknown exception during create FileLog [%s]", logPath.c_str());
		return false;			
	}
	catch(...) {
		return false;
	}	
    gConfig.snmpRegister("");
	
	if(init("snmp")) {
		if(_gw->subscribe(&gTrapHelper, TopicOfSnmpTrap))
		{
			glog(Log::L_INFO,"PlugIn register SNMP sender successful");
			return true;
		}
		glog(Log::L_ERROR,"SNMP sender regist failed");
		Exit();//release plug 
	}

	return false;
}

__EXPORT void EventGw_Module_Entry_uninit()
{
	_gw->unsubscribe(&gTrapHelper, TopicOfSnmpTrap);

	Exit();

	glog(Log::L_INFO,"SNMP sender plug unregister");
	ZQ::common::setGlogger();
	if (NULL != plog)
		delete plog;
	plog = NULL;
}

}//extern "c"
