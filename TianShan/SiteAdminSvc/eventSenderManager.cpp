
#include <log.h>
#include "eventSenderManager.h"

#include "StreamEventReceiver.h"

namespace ZQTianShan {
namespace Site {

EventSenderManager ::EventSenderManager(SiteAdminSvcEnv& env):_env(env)
{
	_bQuit = false;
	_hNewEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
}

EventSenderManager::~EventSenderManager()
{
	_bQuit =true;
	SetEvent(_hNewEvent);
	waitHandle(INFINITE);
	CloseHandle(_hNewEvent);
}

bool EventSenderManager::regist(const OnNewMessage& pMsg ,const char* type)
{
	//find if there is a duplicated routine
	VecMessageReceiver::iterator it = _vecReceiver.begin();
	for( ; it != _vecReceiver.end() ; it ++)
	{
		if ( pMsg == *it  ) 
		{
			envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(EventSenderManager,"There is a OnNewMessage routine in the receiver vector,return with ok"));
			return true;
		}
	}
	_vecReceiver.push_back(pMsg);
	envlog(ZQ::common::Log::L_INFO,CLOGFMT(EventSenderManager,"Insert a new routine [%x] into receiver vector"),pMsg);
	return true;
}

void EventSenderManager::unregist( const OnNewMessage& pMsg ,const char* type )
{
	VecMessageReceiver::iterator it = _vecReceiver.begin();
	for( ; it != _vecReceiver.end() ; it ++)
	{
		if ( pMsg == *it  ) 
		{
			envlog(ZQ::common::Log::L_INFO,CLOGFMT(EventSenderManager,"find the OnNewMessage routine [%x] ,erase it"),pMsg);
			_vecReceiver.erase(it);
			return ;
		}
	}
	envlog(ZQ::common::Log::L_INFO,CLOGFMT(EventSenderManager,"no OnNewMessage routine [%x]  is found"),pMsg);
}

int EventSenderManager::run()
{
	while (!_bQuit) 
	{
		WaitForSingleObject( _hNewEvent , 1000 );
		if (_bQuit) 
		{
			//envlog(ZQ::common::Log::L_INFO,CLOGFMT(EventSenderManager,"Received quit command,send out all reserved message"));
			break;
		}
		{
			while ( _lstEvent.size()>0 ) 
			{
				{
					ZQ::common::MutexGuard gd(_lstLocker);
					if ( _lstEvent.size() > 0 ) 
					{
						MSGSTRUCT& msg = _lstEvent.front();
						VecMessageReceiver::const_iterator it= _vecReceiver.begin();
						for( ; it != _vecReceiver.end() ; it++ )
						{
							try
							{
								(*it)(msg, MessageIdentity(), NULL);
							}
							catch (...) 
							{
								envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EventSenderManager,"Unexpect error when invoke OnNewMessgae [%x]"),*it);
							}
						}
						_lstEvent.pop_front();
					}
				}
			}
		}
	}
	return 1;
}
void EventSenderManager::PostEvent(const MSGSTRUCT& eventMsg )
{
	{
		ZQ::common::MutexGuard gd(_lstLocker);
		_lstEvent.push_back(eventMsg);
	}
	SetEvent(_hNewEvent);
}

bool EventSenderManager::SetupEventSenderEnvironment()
{
	//load the plugin
// 	ZQ::common::ConfigLoader::VECKVMAP& conf =gSiteAdminConfig.getEnumValue("EventSinkModule");
// 	ZQ::common::ConfigLoader::VECKVMAP::iterator it = conf.begin();
    SAConfig::EventSinkModules::iterator it = gSiteAdminConfig.eventSinkModules.begin();
	HMODULE	hDll = NULL;
	ModuleInfo	mInfo;
	for(; it != gSiteAdminConfig.eventSinkModules.end() ; it ++ )
	{
		std::string	strModule		= it->file; //(*it)["file"];
		std::string	strConfig		= it->config; //(*it)["config"];
        std::string strSenderType	= it->type; //(*it)["type"];
		envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(EventSenderManager,"Load eventSinkModule [%s] with Config [%s]"),strModule.c_str(),strConfig.c_str());
		hDll = LoadLibrary(strModule.c_str());
		if(hDll)
		{
			envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(EventSenderManager,"Load Module [%s] successfully"),strModule.c_str());
			InitModule init = (InitModule)GetProcAddress(hDll,"InitModuleEntry");
			UninitModule uninit = ( UninitModule ) GetProcAddress(hDll,"UninitModuleEntry");
			if(init&&uninit)
			{
				try
				{
					if (!init(this,strSenderType.c_str(),strConfig.c_str())) 
					{
						envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EventSenderManager,"initialize EventSinkModule [%s] failed"),strModule.c_str());
						FreeLibrary(hDll);
						hDll = NULL;
						continue;
					}
					envlog(ZQ::common::Log::L_INFO,CLOGFMT(EventSenderManager,"initialize EvenSinkModule [%s] successfully"),strModule.c_str());
					mInfo._hDll = hDll;
					mInfo._strPathDll = strModule;
					_vecDll.push_back(mInfo);
				}
				catch (...) 
				{
					envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EventSenderManager,"Unexpect error when invoke [InitModule] for module [%s]"),strModule.c_str());
					FreeLibrary(hDll);
					hDll=NULL;
					continue;
				}
			}
			else
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EventSenderManager,"Invalid plugin,no [InitModule] and [UninitModule]"));
				FreeLibrary(hDll);
				hDll = NULL;
				continue;
			}
		}
		else
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EventSenderManager,"Load EventSinkModule [%s] failed and errorCode is [%d]"),strModule.c_str(),GetLastError());
		}
	}
	envlog(ZQ::common::Log::L_INFO,CLOGFMT(EventSenderManager,"load %d EventSink plugin(s)"),(int)_vecDll.size());
/* moved to EventTranslator
	//I don't want to  bind this object into the service adapter,so create a adapter here
	_eventAdapter = _env._adapter; //_env._communicator->createObjectAdapter("EventSender");
	if(!_eventAdapter)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EventSenderManager,"Can't create object adapater with name EventSender"));
#pragma message(__MSGLOC__"Should I call DestroyEventSenderEnviroment Here ???????")
		DestroyEventSenderEnvironment();
		return false;
	}	
	//connect to Ice Storm using EventChannel
	
	if (strlen(gSiteAdminConfig.szIceStormEndpoint)>0) 
	{
		{
			
			try
			{
				envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(EventSenderManager,"Connect to iceStorm [%s]"),gSiteAdminConfig.szIceStormEndpoint);
				_eventChannel 	= new TianShanIce::Events::EventChannelImpl(_eventAdapter,gSiteAdminConfig.szIceStormEndpoint,true);
				
				TianShanIce::Properties qos;
				
				TianShanIce::Streamer::StreamEventSinkPtr streamEventPtr  =  new ZQTianShan::Site::StreamEventSinkI(*this);
				TianShanIce::Streamer::PlaylistEventSinkPtr	playlistEventPtr = new ZQTianShan::Site::PlaylistEventSinkI(*this);
				//TianShanIce::Streamer::PlaylistEventSinkPtr playlistEventPtr = new ZQTianShan::Site::PlaylistEventSinkI(*this);
				
				
				_eventChannel->sink(streamEventPtr,qos);
				_eventChannel->sink(playlistEventPtr,qos);
			}
			catch (const Ice::Exception& ex) 
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EventSenderManager,"ice exception [%s] when invoke EventChannel's sink"),ex.ice_name().c_str());
				DestroyEventSenderEnvironment();
				return false;
			}
			catch (...) 
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EventSenderManager,"unexpect error when invoke EventChannel's sink"));
				DestroyEventSenderEnvironment();
				return false;
			}
			//_eventChannel->sink(playlistEventPtr,qos);
			_eventAdapter->activate();
			_eventChannel->start();
		}
	}
	else
	{
		envlog(ZQ::common::Log::L_INFO,CLOGFMT(EventSenderManager,"no ice storm endpoint,sink event canceled"));
		_eventAdapter->activate();		
	}
*/
	start();

	return true;
}

void EventSenderManager::DestroyEventSenderEnvironment()
{
	envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(EventSenderManager,"Enter DestroyEventSenderEnvironment()"));
    /* moved to EventTranslator
	_eventChannel = NULL;
	try
	{
		_eventAdapter->deactivate();
	}
	catch (const Ice::Exception& ex) 
	{
		envlog(ZQ::common::Log::L_ERROR,
				CLOGFMT(EventSenderManager,"ice exception [%s] when deactivate event adapter"),
				ex.ice_name().c_str());
	}
	catch (...) 
	{
		envlog(ZQ::common::Log::L_ERROR,
				CLOGFMT(EventSenderManager,"unexpect error when deactivate event adapter"));
	}
	_eventAdapter = NULL;
    */
	vecDll::iterator it = _vecDll.begin();
	for(  ; it!=_vecDll.end() ; it ++ )
	{
		UninitModule uninit = ( UninitModule ) GetProcAddress(it->_hDll,"UninitModuleEntry");
		if (uninit) 
		{
			try
			{
				uninit(this);
			}
			catch (...) 
			{
				envlog(ZQ::common::Log::L_ERROR,
						CLOGFMT(EventSenderManager,"unexpect error when uninitalize module [%s]"),
						it->_strPathDll.c_str()	);
			}
		}
		FreeLibrary(it->_hDll);
	}
	envlog(ZQ::common::Log::L_INFO,CLOGFMT(EventSenderManager,"Leave DestroyEventSenderEnvironment()"));
}
const char* FormatLocalTime(char* buf,int size)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	memset(buf,0,size);
	_snprintf(buf,size-1,"%4d-%2d-%2d %2d:%2d:%2d",
				st.wYear , st.wMonth  , st.wDay , 
				st.wHour , st.wMinute ,	st.wSecond );
	return buf;	
}
const char* SystemTimeToUTC(__int64 time,char* buf,int size)
{
	if (NULL == buf || size<=0)
		return "";

	SYSTEMTIME systime;
	FILETIME filetime;
	time *= 10000; // //convert msec to nsec
	memcpy(&filetime, &time, sizeof(filetime));

	if (::FileTimeToSystemTime(&filetime, &systime) ==FALSE)
		return "";

	
	snprintf(buf,size,"%04d%02d%02dT%02d%02d%02d.%03dZ",
				systime.wYear,systime.wMonth,systime.wDay,
				systime.wHour,systime.wMinute,systime.wSecond,
				systime.wMilliseconds);
	return buf;	
}


EventTranslator::EventTranslator(SiteAdminSvcEnv& env, EventSenderManager& ssMan)
:_env(env), _ssMan(ssMan)
{
    _hQuit = CreateEvent(NULL,FALSE,FALSE,NULL);
}

EventTranslator::~EventTranslator()
{
    envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(EventTranslator,"Destroying EventTranslator..."));
    
    SetEvent(_hQuit);
    if(WAIT_OBJECT_0 != waitHandle(5000))
    {
       terminate(-1); 
    }
    CloseHandle(_hQuit);
    envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(EventTranslator,"EventTranslator Destroyed"));
}

int EventTranslator::run()
{
    //I don't want to  bind this object into the service adapter,so create a adapter here
    ::Ice::ObjectAdapterPtr _eventAdapter = _env._adapter; //_env._communicator->createObjectAdapter("EventSender");
    if(!_eventAdapter)
    {
	    envlog(ZQ::common::Log::L_ERROR,CLOGFMT(EventTranslator,"No adapter instance available."));
	    return -1;
    }	

    if (strlen(gSiteAdminConfig.szIceStormEndpoint)>0) 
    {
        //connect to Ice Storm using EventChannel
        TianShanIce::Events::EventChannelImpl::Ptr		_eventChannel;

	    do
        {
            DWORD dwResult = ::WaitForSingleObject(_hQuit, 2000);
            if(dwResult == WAIT_OBJECT_0) // quit
            {
                break;
            } // else timeout

            if(_eventChannel)
            { // working ok
                continue;
            }
            else // no initialized
            {
		        try
		        {
			        envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(EventTranslator,"Connect to EventChannel [%s]"),gSiteAdminConfig.szIceStormEndpoint);
			        _eventChannel 	= new TianShanIce::Events::EventChannelImpl(_eventAdapter,gSiteAdminConfig.szIceStormEndpoint,true);
    				
			        TianShanIce::Properties qos;
    				
			        TianShanIce::Streamer::StreamEventSinkPtr streamEventPtr  =  new ZQTianShan::Site::StreamEventSinkI(_ssMan);
			        TianShanIce::Streamer::PlaylistEventSinkPtr	playlistEventPtr = new ZQTianShan::Site::PlaylistEventSinkI(_ssMan);

                    _eventChannel->sink(streamEventPtr,qos);
			        _eventChannel->sink(playlistEventPtr,qos);
		        }
		        catch (const Ice::Exception& ex) 
		        {
			        envlog(ZQ::common::Log::L_WARNING,CLOGFMT(EventTranslator,"EventChannel is not ok. Exception [%s] caught."),ex.ice_name().c_str());
                    _eventChannel = NULL;
			        continue;
		        }
		        catch (...) 
		        {
			        envlog(ZQ::common::Log::L_WARNING,CLOGFMT(EventTranslator,"EventChannel is not ok. Unknown exception."));
                    _eventChannel = NULL;
			        continue;
		        }
		        _eventChannel->start();
            }
	    }while(true);
        // uninit
        if(_eventChannel)
        {
            _eventChannel = NULL;
        }
    }
    else
    {
	    envlog(ZQ::common::Log::L_INFO,CLOGFMT(EventSenderManager,"no ice storm endpoint,sink event canceled"));
    }
    return 0;
}
}}//namespace ZQTianShan::Site