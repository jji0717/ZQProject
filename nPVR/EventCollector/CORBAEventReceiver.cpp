// ORBAEventReceiver.cpp: implementation of the CORBAEventReceiver class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4786)

#include "TaoRoutine.h"
#include "KeyDefine.h"
#include "CORBAEventReceiver.h"
#include "ChannelMessageQueue.h"
#include "Log.h"

#define _OUTPUT_MSA_DETAIL_

using namespace ZQ::common;

#define	 DEFAULT_NAMING_SERVICE		"NameService"
#define  DEFAULT_NOTIFY_SERVICE		"NotifyEventChannelFactory"


//EVENT_STR_FF_*** defined for Fault and Flow Event Header
#define ORBEHF_TIMEOF			"timeOf"
#define ORBEHF_EVENTCODE		"eventCode"
#define ORBEHF_SEV			"SEV"
#define ORBEHF_COMP			"COMP"
#define ORBEHF_ADDR			"ADDR"
#define ORBEHF_TASK			"TASK"
#define ORBEHF_MP				"MP"
#define ORBEHF_TRANS			"TRANS"

//EVENT_STR_PERF_*** defined for Performance Event Elements
#define ORBEHF_ENDTIME		"endTime"
#define ORBEHF_FOR			"FOR"
#define ORBEHF_DUR			"DUR"
#define ORBEHF_END			"END"
#define ORBEHF_STRT			"STRT"


//EVENT_STR_BE_*** defined for Event Body Elements
#define ORBEBF_OP				"OP"
#define ORBEBF_DD				"DD"
#define ORBEBF_SDESC			"SDESC"
#define ORBEBF_DUR				"DUR"
#define ORBEBF_XR				"XR"


CORBAEventReceiver::ORB_FIELD_DATA CORBAEventReceiver::_header_fields[] ={
	{KD_KN_ORBEHF_TIMEOF, ORBEHF_TIMEOF, CORBAEventReceiver::TYPE_STRING},
	{KD_KN_ORBEHF_EVENTCODE, ORBEHF_EVENTCODE, CORBAEventReceiver::TYPE_STRING},
	{KD_KN_ORBEHF_SEV, ORBEHF_SEV, CORBAEventReceiver::TYPE_INT},
	{KD_KN_ORBEHF_COMP, ORBEHF_COMP, CORBAEventReceiver::TYPE_STRING},
	{KD_KN_ORBEHF_ADDR, ORBEHF_ADDR, CORBAEventReceiver::TYPE_STRING},
	{KD_KN_ORBEHF_TASK, ORBEHF_TASK, CORBAEventReceiver::TYPE_STRING},
	{KD_KN_ORBEHF_MP, ORBEHF_MP, CORBAEventReceiver::TYPE_STRING},
	{KD_KN_ORBEHF_TRANS, ORBEHF_TRANS, CORBAEventReceiver::TYPE_STRING},
	{KD_KN_ORBEHF_ENDTIME, ORBEHF_ENDTIME, CORBAEventReceiver::TYPE_STRING},
	{KD_KN_ORBEHF_FOR, ORBEHF_FOR, CORBAEventReceiver::TYPE_STRING},
	{KD_KN_ORBEHF_DUR, ORBEHF_DUR, CORBAEventReceiver::TYPE_INT},
	{KD_KN_ORBEHF_END, ORBEHF_END, CORBAEventReceiver::TYPE_STRING},
	{KD_KN_ORBEHF_STRT, ORBEHF_STRT, CORBAEventReceiver::TYPE_STRING},
};
int	CORBAEventReceiver::_header_field_count = sizeof(_header_fields)/sizeof(CORBAEventReceiver::ORB_FIELD_DATA);

CORBAEventReceiver::ORB_FIELD_DATA CORBAEventReceiver::_body_fields[] = {
	{KD_KN_ORBEBF_OP, ORBEBF_OP, CORBAEventReceiver::TYPE_STRING},
	{KD_KN_ORBEBF_DD, ORBEBF_DD, CORBAEventReceiver::TYPE_STRING},
	{KD_KN_ORBEBF_SDESC, ORBEBF_SDESC, CORBAEventReceiver::TYPE_STRING},
	{KD_KN_ORBEBF_DUR, ORBEBF_DUR, CORBAEventReceiver::TYPE_INT},
	{KD_KN_ORBEBF_XR, ORBEBF_XR, CORBAEventReceiver::TYPE_STRING},
	
};
int	CORBAEventReceiver::_body_field_count = sizeof(_body_fields)/sizeof(CORBAEventReceiver::ORB_FIELD_DATA);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define DEF_MAX_EVENT_QUEUE_DEEP	100

CORBAEventReceiver::CORBAEventReceiver(int channelID):BaseMessageReceiver(channelID)
{
	InitializeCriticalSection(&_lock);
	_hSem = NULL;
	_hExit = NULL;
	_nMaxEventQueueDeep = DEF_MAX_EVENT_QUEUE_DEEP;
}

CORBAEventReceiver::~CORBAEventReceiver()
{
	close();

	DeleteCriticalSection(&_lock);
}

bool CORBAEventReceiver::init(InitInfo& initInfo, const char* szSessionName)
{
	initInfo.setCurrent(szSessionName);
	if (!initInfo.getValue(KD_KN_ORB_DOMAIN, _strDomain, true, true)
		|| !initInfo.getValue(KD_KN_ORB_NAMESERVICE, _strNameServer, true, true)
		|| !initInfo.getValue(KD_KN_ORB_ENDPOINT, _strEndPoint, true, true)
		|| !initInfo.getValue(KD_KN_ORB_EVENTCHANNELNAME, _strEventChannelName, true, true))
	{
		return false;
	}

	initInfo.getValue(KD_KN_ORB_NAMING_SERVICE, _strNameServiceInitRef, DEFAULT_NAMING_SERVICE);
	initInfo.getValue(KD_KN_ORB_NOTIFYCATION_SERVICE, _strNotifycationServiceInitRef, DEFAULT_NOTIFY_SERVICE);

	_headerMap.clear();
	_bodyMap.clear();

	for(int i=0;i<_header_field_count;i++)
	{
		_headerMap.insert(std::pair<std::string, ORB_FIELD_DATA*>(_header_fields[i].szOuter, &_header_fields[i]));
	}

	for(i=0;i<_body_field_count;i++)
	{
		_bodyMap.insert(std::pair<std::string, ORB_FIELD_DATA*>(_body_fields[i].szOuter, &_body_fields[i]));
	}

	_hSem = CreateEvent(NULL, FALSE, FALSE, NULL);
	_hExit = CreateEvent(NULL, FALSE, FALSE, NULL);

	start();

	return true;
}

void CORBAEventReceiver::close()
{
	if (_hExit)
	{
		SetEvent(_hExit);
		waitHandle(INFINITE);
		CloseHandle(_hExit);
		CloseHandle(_hSem);
		_hExit = NULL;
		_hSem = NULL;
	}
}

void CORBAEventReceiver::requireFields(std::vector<std::string>& fields)
{
	for(int i=0;i<_header_field_count;i++)
	{
		fields.push_back(_header_fields[i].szOuter);
	}

	for(i=0;i<_body_field_count;i++)
	{
		fields.push_back(_body_fields[i].szOuter);
	}
}

using namespace std;

void CORBAEventReceiver::OnMessage(int nMessageID, MessageFields* pMessage)
{
	CosNotification::StructuredEvent *event = new CosNotification::StructuredEvent();
	event->header.fixed_header.event_type.domain_name = CORBA::string_dup(_strDomain.c_str());

	{
		std::vector<struct MESSAGE_FIELD>::iterator it;
		bool bEventTypeOK = false;
		bool bEventNameOK = false;
		int  nHeaderCount = 0;
		int  nBodyCount = 0;
		
		event->header.variable_header.length(_headerMap.size());
		event->filterable_data.length(_bodyMap.size());
		
		for(it=pMessage->begin();it!=pMessage->end();it++)
		{
			if (!bEventTypeOK && !strcmp(it->key.c_str(), KD_KN_ORBEHF_EVENTTYPE))
			{
				event->header.fixed_header.event_type.type_name = CORBA::string_dup(it->value.c_str());				
				bEventTypeOK = true;
			}
			else if (!bEventNameOK && !strcmp(it->key.c_str(), KD_KN_ORBEHF_EVENTNAME))
			{
				event->header.fixed_header.event_type.type_name = CORBA::string_dup(it->value.c_str());
				bEventNameOK = true;
			}
			else
			{
				std::map<std::string, ORB_FIELD_DATA*>::iterator itm = _headerMap.find(it->key.c_str());
				if (itm != _headerMap.end())
				{
					event->header.variable_header[nHeaderCount].name = CORBA::string_dup(itm->second->szInter);

					if (itm->second->type == TYPE_STRING)	//string
						event->header.variable_header[nHeaderCount].value <<= it->value.c_str();
					else
						event->header.variable_header[nHeaderCount].value <<= atoi(it->value.c_str());

#ifdef _OUTPUT_MSA_DETAIL_
					glog(Log::L_DEBUG, L"MessageID[%04x] Header: %S=%S", nMessageID, itm->second->szInter, it->value.c_str());
#endif

					nHeaderCount++;					

					continue;
				}

				itm = _bodyMap.find(it->key.c_str());
				if (itm != _bodyMap.end())
				{
					event->filterable_data[nBodyCount].name = CORBA::string_dup(itm->second->szInter);

					if (itm->second->type == TYPE_STRING)	//string
						event->filterable_data[nBodyCount].value <<= it->value.c_str();
					else
						event->filterable_data[nBodyCount].value <<= atoi(it->value.c_str());

#ifdef _OUTPUT_MSA_DETAIL_
					glog(Log::L_DEBUG, L"MessageID[%04x] Body: %S=%S", nMessageID, itm->second->szInter, it->value.c_str());
#endif

					nBodyCount++;									
				}
			}
		}

		event->header.variable_header.length(nHeaderCount);
		event->filterable_data.length(nBodyCount);
	}

	_EVENT_DATA ed;
	ed.nMessageID = nMessageID;
	ed.pEvent = event;

	EnterCriticalSection(&_lock);
	if (_eventQueue.size()>=_nMaxEventQueueDeep)
	{
		_EVENT_DATA tmp;					
		tmp = _eventQueue.front();
		_eventQueue.pop_front();

		if (tmp.pEvent)
		{
			delete tmp.pEvent;
		}		
		glog(Log::L_ERROR, L"MessageID[%04x] Event queue is full, skip this event", tmp.nMessageID);
	}
	_eventQueue.push_back(ed);
	LeaveCriticalSection(&_lock);
	
	SetEvent(_hSem);
}

int CORBAEventReceiver::run()
{
	glog(Log::L_DEBUG, L"CORBAEventReceiver run thread enter, threadid [0x%04x]", GetCurrentThreadId());

	CORBA::ORB_var          _orb;

	do
	{
		//
		// init orb
		//
		{
			char * ppArgv[20]; // -ORBEndpoint iiop://host:port xxxx -ORBInitRef NameService -ORBInitRef NotificationService
			int nArgc = 0;
			
			// NameService=corbaloc::xxx.xxx.xxx.xxx:dddd/NameService
			//
			char pNameService[128];
			sprintf(pNameService, "NameService=corbaloc::%s/%s", _strNameServer.c_str(), _strNameServiceInitRef.c_str());		
			
			// Set argument value
			ppArgv[nArgc++] = "-ORBInitRef";
			ppArgv[nArgc++] = pNameService;
			
			// -ORBEndpoint iiop://host:port
			//
			char pEndpoint  [256];
			strcpy(pEndpoint, _strEndPoint.c_str());
			
			ppArgv[nArgc++] = "-ORBEndpoint";
			ppArgv[nArgc++] = pEndpoint;
			
			ppArgv[nArgc++] = "-ORBSvcConfDirective";
			ppArgv[nArgc++] = "static Server_Strategy_Factory '-ORBConcurrency thread-per-connection'";
			
			try
			{
				_orb = CORBA::ORB_init(nArgc, ppArgv);
			}
			catch (CORBA::Exception& ex) 
			{
				glog(Log::L_ERROR, L"Fail to init ORB with -ORBInitRef %S -ORBEndpoint %S, reason: %S", 
					pNameService, pEndpoint, ex._rep_id());			
				
				break;	// exit thread
			}
			
			glog(Log::L_DEBUG, L"Orb inited with -ORBInitRef %S -ORBEndpoint %S", pNameService, pEndpoint);
		}
		
		const char* szNameInitRef = _strNameServiceInitRef.c_str();
		const char* szNotifycationInitRef = _strNotifycationServiceInitRef.c_str();
		
		{
			CosNotifyChannelAdmin::StructuredProxyPushConsumer_var   pushConsumer = CosNotifyChannelAdmin::StructuredProxyPushConsumer::_nil();
			
			do{
				CosNotifyChannelAdmin::EventChannel_var EventCollectorChannel = getNotifyChannel(
					_orb,
					_strEventChannelName.c_str(),
					true, szNameInitRef, szNotifycationInitRef);		
				
				if ( CORBA::is_nil(EventCollectorChannel) )
				{
					glog(Log::L_ERROR, L"Fail to getNotifyChannel(), NameInitRef[%S] NotifycationInitRef[%S] EventChannelName[%S]",
						szNameInitRef, szNotifycationInitRef, _strEventChannelName.c_str());
					break;
				}
				
				// Obtain and set the StructuredProxyPushConsumer on the factory
				CosNotifyChannelAdmin::SupplierAdmin_var sa_var;
				try
				{
					sa_var = EventCollectorChannel->default_supplier_admin();
				}
				catch(CORBA::SystemException& ex)
				{
					glog(Log::L_ERROR, L"Fail to call default_supplier_admin(), info: %S", ex._info().c_str());
					break;
				}
				
				CosNotifyChannelAdmin::ProxyID proxyId;
				CosNotifyChannelAdmin::ProxyConsumer_var proxyConsumer;
				try
				{
					proxyConsumer = sa_var -> obtain_notification_push_consumer(
					CosNotifyChannelAdmin::STRUCTURED_EVENT, proxyId);
				}
				catch(CosNotifyChannelAdmin::AdminLimitExceeded&)
				{
					glog(Log::L_ERROR, L"Fail to call obtain_notification_push_consumer(), info: AdminLimitExceeded");
					break;
				}
				catch(CORBA::SystemException& ex)
				{
					glog(Log::L_ERROR, L"Fail to call obtain_notification_push_consumer(), info: %S", ex._info().c_str());
					break;
				}

				pushConsumer = CosNotifyChannelAdmin::StructuredProxyPushConsumer::_unchecked_narrow(proxyConsumer);
				
				try
				{
					pushConsumer -> connect_structured_push_supplier(CosNotifyComm::StructuredPushSupplier::_nil());
				}
				catch(CORBA::SystemException& ex)
				{
					glog(Log::L_ERROR, L"Fail to call connect_structured_push_supplier(), info: %S", ex._info().c_str());
					break;
				}
				
				glog(Log::L_DEBUG, L"NotifyChannel [%S] connected", _strEventChannelName.c_str());
			}while(0);
			
			HANDLE hHandles[2];
			hHandles[0] = _hSem;
			hHandles[1] = _hExit;	
			
			while(true)
			{
				//wait for a message
				DWORD dwRet = WaitForMultipleObjects(2, hHandles, FALSE, INFINITE);
				if (dwRet == WAIT_OBJECT_0)
				{
					
				}
				else if (dwRet == WAIT_OBJECT_0 + 1)
				{
					glog(Log::L_INFO, "CORBAEventReceiver loop exit");
					break;
				}
				else
				{
					// error, exit
					glog(Log::L_ERROR, L"WaitForMultipleObjects return fail with code 0x%08x", dwRet);
					break;
				}
				
				//
				// get a message
				//		
				while (_eventQueue.size())
				{
					_EVENT_DATA ed;			
					EnterCriticalSection(&_lock);
					ed = _eventQueue.front();
					_eventQueue.pop_front();
					LeaveCriticalSection(&_lock);
					
					CosNotification::StructuredEvent_var stevent(ed.pEvent);					
					
					bool bSendSucc = false;
					int nErrorCount = 0;
					while(_eventQueue.size()<_nMaxEventQueueDeep)
					{						
						if (pushStructuredEvent(_orb, stevent, pushConsumer, szNameInitRef, szNotifycationInitRef, _strEventChannelName.c_str()))
						{
							bSendSucc = true;
							glog(Log::L_DEBUG, L"MessageID[%04x] pushStructuredEvent succeed", ed.nMessageID);
							break;	// success then exit
						}
						else
						{
							if (nErrorCount<3)
							{
								glog(Log::L_ERROR, L"MessageID[%04x] Fail to push event to event channel, NameInitRef[%S] NotifycationInitRef[%S] EventChannelName[%S]",
									ed.nMessageID, szNameInitRef, szNotifycationInitRef, _strEventChannelName.c_str());
							}
							nErrorCount++;
						}
						
						//retry
						Sleep(2000);						
					}
					
					if (!bSendSucc)
					{
						glog(Log::L_ERROR, L"MessageID[%04x] Fail to push event, the event queue is full, skip this event", ed.nMessageID);
					}
				}	
			}
		}

		if(!CORBA::is_nil(_orb))    
		{
			try    
			{
				_orb -> destroy();
			} 
			catch(const CORBA::Exception& )
			{
				glog(Log::L_DEBUG, L" in CORBA::ORB::destroy()");			
			}
		}
	}while(0);

	glog(Log::L_DEBUG, L"CORBAEventReceiver run thread leave, threadid [0x%04x]", GetCurrentThreadId());
	return 0;
}
