
#ifndef __CONTENTLIB_SERVICE_H__
#define __CONTENTLIB_SERVICE_H__

#include "ZQDaemon.h"
#include "ContentLibEnv.h"
#include "IceLog.h"

class ContentLibService : public ZQ::common::ZQDaemon
{
public:
	ContentLibService();
	virtual ~ContentLibService();

public:
    virtual bool OnInit(void);
    virtual bool OnStart(void);
    virtual void OnStop(void);
    virtual void OnUnInit(void);

private:
	Ice::CommunicatorPtr							_communicator;
	ContentLibEnv*									_pContentLibEnv;	
	::TianShanIce::common::IceLogIPtr				_icelog;
	Ice::PropertiesPtr								_properties;
	ZQ::common::FileLog*							_iceFileLog;

};

#endif // __CONTENTLIB_SERVICE_H__

