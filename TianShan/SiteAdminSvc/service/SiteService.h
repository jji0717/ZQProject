#ifndef __SITE_ADMIN_SERVICE_H__
#define __SITE_ADMIN_SERVICE_H__

#include <BaseZQServiceApplication.h>

#include <ice/ice.h>
#include <IceUtil/IceUtil.h>

#include "SiteAdminImpl.h"
#include "FileLog.h"
#include <IceLog.h>

#ifdef _WITH_EVENTSENDER_
#include "../eventSenderManager.h"
#endif//_WITH_EVENTSENDER_


class SiteAdminService : public ZQ::common::BaseZQServiceApplication
{
public:
	SiteAdminService();
	~SiteAdminService();
public:	
	virtual HRESULT OnInit(void);
 	virtual HRESULT OnStop(void);
	virtual HRESULT OnStart(void);
	virtual HRESULT OnUnInit(void);
    virtual void OnSnmpSet(const char*);
protected:
private:
	Ice::CommunicatorPtr					m_ic;
	::ZQTianShan::Site::SiteAdminSvcEnv*	m_pSvcEnv;
	TianShanIce::Site::SiteAdminPtr			m_SiteSrvPtr;
	ZQ::common::FileLog*					m_pSvcLog;
	ZQ::common::FileLog*					m_pLogForIce;
	Ice::LoggerPtr							m_pIceLog;
	
};




#endif//__SITE_ADMIN_SERVICE_H__
