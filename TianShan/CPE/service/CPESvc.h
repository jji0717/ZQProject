#ifndef _CPE_SERVICE_H_
#define _CPE_SERVICE_H_

//#include "ConfigLoader.h"
#include <FileLog.h>

#ifdef ZQ_OS_MSWIN
#include "BaseZQServiceApplication.h"
#else
#include <ZQDaemon.h>
#endif

#include <Ice/Ice.h>

namespace ZQTianShan {
namespace CPE {

class CPEEnv;


class ContentProvisionEngineSvc : public ZQ::common::BaseZQServiceApplication 
{
public:
	
	ContentProvisionEngineSvc ();
	virtual ~ContentProvisionEngineSvc ();
	
public:
	
	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);
	virtual void OnSnmpSet(const char*);
    virtual void doEnumSnmpExports();
	void snmp_MothodTable(const uint32& iDummy);

	IceUtil::Handle<Ice::Logger> _iceLogger;
	ZQ::common::FileLog* _iceLogFile;
	CPEEnv* _pCPEEnv;
		
#ifdef ZQ_OS_MSWIN
	std::string m_strProgramRootPath;
#else
	ZQ::common::FileLog* _svcLog;
#endif
	std::string _strLogFolder;
	std::string _strPluginFolder;

private:
	bool InitIce(void);
};

}}

#endif
