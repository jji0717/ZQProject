#ifndef __tianshan_siteadmin_ice_object_factory_banana_h__
#define __tianshan_siteadmin_ice_object_factory_banana_h__

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <SiteAdminSvc.h>

class SiteAdminEnv;
class SiteAdminDb;
class TxnWatchDog;
class TxnTransfer;
class SiteAdminImpl;

class SiteAdminIceObjFactory : public Ice::ObjectFactory
{
public:
	SiteAdminIceObjFactory( SiteAdminEnv& env , SiteAdminDb& db , TxnWatchDog& watchdog, TxnTransfer& transfer ,SiteAdminImpl& saImpl);
	virtual ~SiteAdminIceObjFactory(void);

	virtual Ice::ObjectPtr create(const std::string&);
	virtual void destroy();

private:
	
	SiteAdminEnv&		mEnv;
	SiteAdminDb&		mDb;
	TxnWatchDog&		mWatchDog;
	TxnTransfer&		mTransfer;
	SiteAdminImpl&		mSaImpl;
};
typedef IceUtil::Handle<SiteAdminIceObjFactory> SiteAdminIceObjFactoryptr;

#endif//__tianshan_siteadmin_ice_object_factory_banana_h__
