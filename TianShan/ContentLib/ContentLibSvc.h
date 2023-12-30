#ifndef __CONTENTLIB_SERVICE_H__
#define __CONTENTLIB_SERVICE_H__

#include "ZQ_common_conf.h"
#ifdef ZQ_OS_MSWIN
#include "BaseZQServiceApplication.h"
#else
#include "ZQDaemon.h"
#endif
#include "EventChannel.h"
#include "IceLog.h"
#include "ContentLibEnv.h"

class ContentLibSvc : public ZQ::common::BaseZQServiceApplication
{
public:
	ContentLibSvc();
	virtual ~ContentLibSvc();
public:

	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);

private:
	Ice::CommunicatorPtr							_communicator;
	ContentLibEnv*									_pContentLibEnv;	
	::TianShanIce::common::IceLogIPtr				_icelog;
	Ice::PropertiesPtr								_properties;
	ZQ::common::FileLog*							_iceFileLog;
};
#endif

