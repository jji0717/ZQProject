#ifndef _MCCS_SERVICE_H_
#define _MCCS_SERVICE_H_


#include "ConfigLoader.h"
#ifdef ZQ_OS_MSWIN
#include "BaseZQServiceApplication.h"
#else
#include "ZQDaemon.h"
#endif

namespace ZQTianShan {
	namespace ContentStore {


class CDNCSEngineSvc: public ZQ::common::BaseZQServiceApplication
{
public:
	CDNCSEngineSvc();
	virtual ~CDNCSEngineSvc();

public:
	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);
	virtual void OnSnmpSet(const char*);

	ZQ::common::FileLog*					_iceLogFile;
	ZQ::common::FileLog*					_EventLogFile;

	std::string								m_strProgramRootPath;
	std::string								_strLogFolder;
};

}}


#endif
