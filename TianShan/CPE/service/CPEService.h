#ifndef _CPESERVICE_H_
#define _CPESERVICE_H_

#include <ZQDaemon.h>
#include <FileLog.h>
#include <Ice/Ice.h>

namespace ZQTianShan {
namespace CPE {

class CPEEnv;
/*
class IceLogger:public ::Ice::Logger
{
public:
	IceLogger(ZQ::common::Log& log):_logger(log)
	{		
	}
	~IceLogger()
	{	
	}
	void print(const ::std::string& message);
	void trace(const ::std::string& category, const ::std::string& message);
	void warning(const ::std::string& message);
	void error(const ::std::string& message);
private:
	ZQ::common::Log& _logger;
};
*/

class ContentProvisionEngineSvc : public ZQ::common::ZQDaemon
{
public:
	
	ContentProvisionEngineSvc ();
	virtual ~ContentProvisionEngineSvc ();
	
public:
	
	virtual bool OnInit(void);
	virtual bool OnStart(void);
	virtual void OnStop(void);
	virtual void OnUnInit(void);

private:
	IceUtil::Handle<Ice::Logger>			_iceLogger;
	ZQ::common::FileLog*				_iceLogFile;
	ZQ::common::FileLog*				_svcLog;
	CPEEnv*						_pCPEEnv;
		
	std::string					_strLogFolder;
	
private:
	bool InitIce(void);
	void initWithConfig(Ice::PropertiesPtr proper);
	
};

}}

#endif
