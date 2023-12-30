#ifndef _MCCS_SERVICE_H_
#define _MCCS_SERVICE_H_


#include "ConfigLoader.h"
#include "ZQDaemon.h"

namespace ZQTianShan {
	namespace ContentStore {


class CDNCSEngineSvc: public ZQ::common::ZQDaemon
{
public:
	CDNCSEngineSvc();
	virtual ~CDNCSEngineSvc();

public:
	bool OnInit(void);
	bool OnStart(void);
	void OnStop(void);
	void OnUnInit(void);

	ZQ::common::FileLog*					_iceLogFile;
	ZQ::common::FileLog*					_EventLogFile;

	std::string								m_strProgramRootPath;
	std::string								_strLogFolder;
};

}}


#endif
