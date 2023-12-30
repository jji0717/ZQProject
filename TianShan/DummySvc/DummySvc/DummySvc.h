#ifndef     _DUMMYSS_H
#define    _DUMMYSS_H

#ifdef ZQ_OS_MSWIN
#include "stdafx.h"
#include <conio.h>
#endif
#include "DummyStreamSmith.h"

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <fstream>

#if ICE_INT_VERSION/100 < 303
	#include <Ice/IdentityUtil.h>
#endif

#include "FileLog.h"
#include "FileSystemOp.h"

#ifndef max
	#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
	#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif
///////////////////////////////////////////////////////////////////////////////////////

#ifdef  ZQ_OS_MSWIN
	#include <BaseZQServiceApplication.h>
#else
	#include <ZQDaemon.h>
#endif

class IceLogger:public Ice::Logger
{
public:
	IceLogger(ZQ::common::Log& log);
	~IceLogger();
	void print(const ::std::string& message);
	void trace(const ::std::string& category, const ::std::string& message);
	void warning(const ::std::string& message);
	void error(const ::std::string& message);
	virtual ::std::string getPrefix() {return "";}
	virtual ::Ice::LoggerPtr cloneWithPrefix(const ::std::string&){return NULL;}

private:
	ZQ::common::Log& _logger;
	ZQ::common::Mutex _locker;
};

class DummySvc:public ZQ::common::BaseZQServiceApplication
{
public:
	DummySvc();
	~DummySvc();
public:
	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);
private:
	void initIceProps( Ice::PropertiesPtr proper);
protected:
	Ice::PropertiesPtr           _iceProperPtr;
	ZQ::common::FileLog* _dummySvcLog;
	Ice::ObjectAdapterPtr  _objAdapter;
	::Ice::CommunicatorPtr _ic;
	TianShanIce::Streamer::StreamServicePtr _service;
	IceUtil::Handle<IceLogger> _m_iceLogger;
};

#endif