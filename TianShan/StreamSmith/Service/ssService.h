
#ifndef __ZQ_TIANSHAN_STREAMSMITH_H__
#define __ZQ_TIANSHAN_STREAMSMITH_H__

#include <Ice/Ice.h>
#include <BaseZQServiceApplication.h>
#include <string>
#include <log.h>
#include <global.h>
#include <RtspSessionMgr.h>
#include <RtspDialog.h>
#include <StreamSmithSite.h>
#include "DataPostHouseService.h"



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
		_logger(ZQ::common::Log::L_DEBUG,message.c_str());
	}
	void trace(const ::std::string& category, const ::std::string& message)
	{
		_logger(ZQ::common::Log::L_DEBUG,"catagory %s,message %s",category.c_str(),message.c_str());
	}
	void warning(const ::std::string& message)
	{
		_logger(ZQ::common::Log::L_WARNING,message.c_str());
	}
	void error(const ::std::string& message)
	{
		_logger(ZQ::common::Log::L_ERROR,message.c_str());
	}
private:
	ZQ::common::Log& _logger;
};

class StreamSmithService:public ZQ::common::BaseZQServiceApplication
{
	friend void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);
public:
	StreamSmithService();
	virtual ~StreamSmithService();
	
	HANDLE getStopEvent( )
	{
		return m_StopEvent;
	}
protected:
	HRESULT OnInit();
	
	HRESULT OnStart();
	
	HRESULT OnStop();
	
	HRESULT OnUnInit();	
	
	void	OnSnmpSet(const char *varName);

	virtual void doEnumSnmpExports();

protected:

	unsigned short						m_usPort;
	ZQ::common::Log*					m_pLog;
	ZQ::common::Log*					m_pSessMonLog;
	ZQ::common::Log*					m_pPluginLog;
	ZQ::common::Log*					m_pIceLogger;
	IceUtil::Handle<Ice::Logger>		m_iceLogger;
	Ice::CommunicatorPtr				m_ic;
};

#endif//__ZQ_TIANSHAN_STREAMSMITH_H__