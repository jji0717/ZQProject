
#include "Log.h"
#include "EventSenderManager.h"
#include "StreamEventReceiver.h"
#include "SiteAdminEnv.h"


EventSenderManager ::EventSenderManager(SiteAdminEnv& env)
:mEnv(env)
{
	mbQuit = false;	
}

EventSenderManager::~EventSenderManager()
{
	mbQuit =true;
	mSemaphore.post();
}

bool EventSenderManager::regist( const OnNewMessage& pMsg ,const char* type )
{
	//find if there is a duplicated routine
	VecMessageReceiver::iterator it = mVecReceiver.begin();
	for( ; it != mVecReceiver.end() ; it ++)
	{
		if ( pMsg == *it  ) 
		{
			MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(EventSenderManager,"There is a OnNewMessage routine in the receiver vector,return with ok"));
			return true;
		}
	}
	mVecReceiver.push_back(pMsg);
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(EventSenderManager,"Insert a new routine [%x] into receiver vector"),pMsg);
	return true;
}

void EventSenderManager::unregist( const OnNewMessage& pMsg ,const char* type )
{
	VecMessageReceiver::iterator it = mVecReceiver.begin();
	for( ; it != mVecReceiver.end() ; it ++)
	{
		if ( pMsg == *it  ) 
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(EventSenderManager,"find the OnNewMessage routine [%x] ,erase it"),pMsg);
			mVecReceiver.erase(it);
			return ;
		}
	}
	//MLOG(ZQ::common::Log::L_INFO,CLOGFMT(EventSenderManager,"no OnNewMessage routine [%x]  is found"),pMsg);
}

int EventSenderManager::run()
{
	while (!mbQuit) 
	{
		//WaitForSingleObject( mSemaphore , 1000 );
		mSemaphore.wait();
		if (mbQuit) { break; }		
		while ( true ) 
		{
			MSGSTRUCT* msg = NULL;
			bool	bGotMsg = false;
			{
				ZQ::common::MutexGuard gd(mLstLocker);
				if ( mLstEvent.size() <= 0 ) 
					break;
				bGotMsg = true;
				msg = mLstEvent.front();
				mLstEvent.pop_front();
			}
			if(!bGotMsg || !msg)
				continue;

			VecMessageReceiver::const_iterator it= mVecReceiver.begin();
			for( ; it != mVecReceiver.end() ; it++ )
			{
				try
				{
					(*it)(*msg,MessageIdentity(), NULL);
				}
				catch (...) 
				{
					MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(EventSenderManager,"Unexpect error when invoke OnNewMessgae [%x]"),*it);
				}
			}
			delete msg;
			msg = NULL;
		}	
	}
	return 1;
}
void EventSenderManager::PostEvent( MSGSTRUCT* eventMsg )
{
	{
		ZQ::common::MutexGuard gd(mLstLocker);
		mLstEvent.push_back( eventMsg );
	}
	mSemaphore.post();	
}

bool EventSenderManager::SetupEventSenderEnvironment()
{
	//load the plugin
    SAConfig::EventSinkModules::iterator it = mEnv.getConfig().eventSinkModules.begin();
	for( ; it != mEnv.getConfig().eventSinkModules.end() ; it ++ )
	{
		const std::string& modulePath = it->file;
		const std::string& moduleConfig = it->config;
		const std::string& moduleType = it->type;

		if( it->enable <= 0 )
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(EventSenderManager,"SetupEventSenderEnvironment() skip module[%s] config[%s] type[%s] due to DISABLED"),
				modulePath.c_str() , moduleConfig.c_str() , moduleType.c_str() );
			continue;
		}
		
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(EventSenderManager,"SetupEventSenderEnvironment() trying to load module[%s] config[%s] type[%s]"),
			modulePath.c_str() , moduleConfig.c_str() , moduleType.c_str() );
		ZQ::common::DynSharedObj* obj = new ZQ::common::DynSharedObj(modulePath.c_str());
		
		assert( obj != NULL );

		if(!obj->isLoaded() )
		{
			delete obj;
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(EventSenderManager,"SetupEventSenderEnvironment() failed to load plugin[%s]"),modulePath.c_str());
			continue;
		}
		EventPluginFacet facet(*obj);
		if(!facet.isValid())
		{
			delete obj;
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(EventSenderManager,"SetupEventSenderEnvironment() module[%s] is not a good plugin"),modulePath.c_str());
			continue;
		}
		
		try
		{
			facet.InitModuleEntry( this , moduleType.c_str() , moduleConfig.c_str() );
		}
		catch( ... )
		{
			delete obj;
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(EventSenderManager,"SetupEventSenderEnvironment() unexpect exception occurred during initialize plugin[%s]"),
				modulePath.c_str() );			
			continue;
		}
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(EventSenderManager,"SetupEventSenderEnvironment() module[%s] is loaded"),modulePath.c_str() );
		mPlugins.push_back(obj);
	}
	start();
	return true;
}

void EventSenderManager::DestroyEventSenderEnvironment()
{
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(EventSenderManager,"Enter DestroyEventSenderEnvironment()"));
	PLUGINS::iterator it = mPlugins.begin();
	for( ; it != mPlugins.end() ; it++ )
	{
		std::string modulename = (*it)->getImageInfo()->filename;
		{
			EventPluginFacet facet( *(*it) );
			if(!facet.isValid())
				continue;
			MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(EventSenderManager,"DestroyEventSenderEnvironment() trying to uninit %s"),modulename.c_str() );
			try
			{				
				facet.UninitModuleEntry( this );			
			}
			catch(...)
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(EventSenderManager,"DestroyEventSenderEnvironment() unknown exception occurred while uninit plugin"));
			}		
		}
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(EventSenderManager,"DestroyEventSenderEnvironment() free %s"),modulename.c_str() );
		delete *it;	
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(EventSenderManager,"DestroyEventSenderEnvironment() %s freed"),modulename.c_str() );
	}
	mPlugins.clear();
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(EventSenderManager,"Leave DestroyEventSenderEnvironment()"));
}

void EventSenderManager::startEventChannel()
{

}

void EventSenderManager::stopEventChannel()
{

}

const char* FormatLocalTime(char* buf,int size)
{
#ifdef ZQ_OS_MSWIN
	SYSTEMTIME st;
	GetLocalTime(&st);
#else
	time_t now_;
	struct tm st;
	localtime_r(&now_, &st);
#endif
	memset(buf,0,size);
	snprintf(buf,size-1,"%4d-%2d-%2d %2d:%2d:%2d",
#ifdef ZQ_OS_MSWIN
	st.wYear, st.wMonth, st.wDay, 
	st.wHour, st.wMinute, st.wSecond);
#else
	st.tm_year+1900, st.tm_mon+1, st.tm_mday,
	st.tm_hour, st.tm_min, st.tm_sec);	
#endif
	return buf;	
}
const char* SystemTimeToUTC(int64 time,char* buf,int size)
{
	if (NULL == buf || size<=0)
		return "";

#ifdef ZQ_OS_MSWIN
	SYSTEMTIME systime;
	FILETIME filetime;
	time *= 10000; // //convert msec to nsec
	memcpy(&filetime, &time, sizeof(filetime));

	if (::FileTimeToSystemTime(&filetime, &systime) ==FALSE)
		return "";
#else
	struct timeval tmval;
	tmval.tv_sec = time/1000;
	tmval.tv_usec = (time%1000)*1000L;
    int nMSec = tmval.tv_usec/1000;
	struct tm systime;
    gmtime_r(&tmval.tv_sec, &systime);
#endif
	
	snprintf(buf,size,"%04d%02d%02dT%02d%02d%02d.%03dZ",
#ifdef ZQ_OS_MSWIN
		systime.wYear,systime.wMonth,systime.wDay,
		systime.wHour,systime.wMinute,systime.wSecond,
		systime.wMilliseconds);
#else
		systime.tm_year+1900, systime.tm_mon+1,systime.tm_mday,
		systime.tm_hour, systime.tm_min, systime.tm_sec, nMSec);	
#endif
	return buf;	
}
