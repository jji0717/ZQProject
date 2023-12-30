#ifndef _TIANSHAN_WEIWOO_SERVICE_WRAP_H__
#define _TIANSHAN_WEIWOO_SERVICE_WRAP_H__

#include "SessionImpl.h"
#include "../../AccreditedPath/PathManagerImpl.h"
#include "Guid.h"

#include "ZQResource.h"

#include <string>
#include <Ice/Ice.h>

#ifdef ZQ_OS_MSWIN
#include "BaseZQServiceApplication.h"
#else
#include "ZQDaemon.h"
#endif

class WeiWooEnvEx:public ::ZQTianShan::Weiwoo::WeiwooSvcEnv
{
public:
	WeiWooEnvEx(ZQ::common::Log& log, 
				ZQ::common::NativeThreadPool& threadPool, 
				Ice::CommunicatorPtr& communicator, 
				const char* endpoint = DEFAULT_ENDPOINT_Weiwoo)
				:ZQTianShan::Weiwoo::WeiwooSvcEnv(log,threadPool,communicator,endpoint)
	{
	}
	~WeiWooEnvEx()
	{
	}
public:
	//virtual void initWithConfig(void);
};

class IceLogger:public Ice::Logger
{
public:
	IceLogger(ZQ::common::Log& log):_logger(log)
	{		
	}
	~IceLogger()
	{	
	}
	void print(const ::std::string& message)
	{
		ZQ::common::MutexGuard gd(_locker);
		_logger(ZQ::common::Log::L_DEBUG,message.c_str());
	}
	void trace(const ::std::string& category, const ::std::string& message)
	{
		ZQ::common::MutexGuard gd(_locker);
		_logger(ZQ::common::Log::L_DEBUG,"catagory %s,message %s",category.c_str(),message.c_str());
	}
	void warning(const ::std::string& message)
	{
		ZQ::common::MutexGuard gd(_locker);
		_logger(ZQ::common::Log::L_WARNING,message.c_str());
		_logger.flush ();
	}
	void error(const ::std::string& message)
	{
		ZQ::common::MutexGuard gd(_locker);
		_logger(ZQ::common::Log::L_ERROR,message.c_str());
		_logger.flush ();
	}
        virtual ::std::string getPrefix(){return "";}
        virtual ::Ice::LoggerPtr cloneWithPrefix(const ::std::string&){return NULL;}
private:
	ZQ::common::Log& _logger;
	ZQ::common::Mutex _locker;
};

class WeiwooService : public ZQ::common::BaseZQServiceApplication 
{
public:
	WeiwooService();
	~WeiwooService();
public:

	HRESULT OnInit(void);
	HRESULT OnStop(void);
	HRESULT OnPause(void){return S_OK;}
	HRESULT OnContinue(void){return S_OK;}
	HRESULT OnStart(void);
	HRESULT OnUnInit(void);	
	bool isHealth(void){return true;}
	// void OnSnmpSet(const char *varName);

	void doEnumSnmpExports();

private:

	Ice::CommunicatorPtr	m_icWeiwoo;					/*<!-ice communicator*/
	Ice::CommunicatorPtr	m_icPath;

	ZQ::common::Log*		m_pLogWeiwoo;
	ZQ::common::Log*		m_pLogPath;
	ZQ::common::Log*		m_pIceLog;

	Ice::LoggerPtr			m_iceLogger;
	

	WeiWooEnvEx									*wenv;

	::ZQTianShan::AccreditedPath::PathSvcEnv	*penv;
	
	
	TianShanIce::SRM::SessionManagerPtr			sessmgr ;
	TianShanIce::Transport::PathManagerPtr		ADPaths ;

	//std::string									m_strProgramRootPath;

public: // snmp access
	uint32 getSessionCount() { return wenv ? (wenv->_watchDog.getWatchSize()) : 0; }
	uint32 getPendingSize()  { return wenv ? (wenv->_thpool.pendingRequestSize()) : 0; }
	uint32 getBusyThreads()  { return wenv ? (wenv->_thpool.activeCount()) : 0; }
	uint32 getThreads()      { return wenv ? (wenv->_thpool.size()) : 0; }
	uint32 dummyGet()        { return 1; }
	void   dumpSgUsage(const uint32& newv)  { if (penv) penv->dumpUsageStat(); }
	
};
#endif//_TIANSHAN_WEIWOO_SERVICE_WRAP_H__

// vim: ts=4 sw=4 bg=dark nu
