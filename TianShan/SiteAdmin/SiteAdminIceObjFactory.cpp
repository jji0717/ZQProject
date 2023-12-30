
#include "SiteAdminEnv.h"
#include "SiteAdminIceObjFactory.h"
#include "SaIceImpl.h"


SiteAdminIceObjFactory::SiteAdminIceObjFactory(  SiteAdminEnv& env , SiteAdminDb& db , TxnWatchDog& watchdog, TxnTransfer& transfer ,SiteAdminImpl& saImpl )
:mEnv(env),
mDb(db),
mWatchDog(watchdog),
mTransfer(transfer),
mSaImpl(saImpl)
{
	if ((Ice::ObjectAdapterPtr)(mEnv.getAdapter()))
	{
		Ice::CommunicatorPtr ic = mEnv.getIc();

		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdminIceObjFactory, "add factory onto communicator"));

		ic->addObjectFactory(this, TianShanIce::Site::AppMount::ice_staticId() );
		ic->addObjectFactory(this, TianShanIce::Site::LiveTxn::ice_staticId() );
		ic->addObjectFactory(this, TianShanIce::Site::TxnEvent::ice_staticId() );
		ic->addObjectFactory(this, TianShanIce::Site::CAppInfo::ice_staticId() );
		ic->addObjectFactory(this, TianShanIce::Site::CVirtualSite::ice_staticId() );
	}
}

SiteAdminIceObjFactory::~SiteAdminIceObjFactory(void)
{
}

Ice::ObjectPtr SiteAdminIceObjFactory::create(const std::string& type )
{
	if ( TianShanIce::Site::AppMount::ice_staticId() == type )
		return new AppMountImpl(mEnv,mDb);

	if ( TianShanIce::Site::LiveTxn::ice_staticId() == type )
		return new LiveTxnImpl(mEnv,mDb,mWatchDog,mTransfer,mSaImpl);

	if ( TianShanIce::Site::TxnEvent::ice_staticId() == type )
		return new TxnEventImpl(mEnv,mDb);

	if( TianShanIce::Site::CAppInfo::ice_staticId() == type )
		return new CAppInfoImpl(mEnv,mDb);

	if( TianShanIce::Site::CVirtualSite::ice_staticId() == type )
		return new CVirtualSiteImpl(mEnv,mDb);

	MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SiteAdminIceObjFactory, "create(%s) type unknown"), type.c_str());
	return NULL;
}

void SiteAdminIceObjFactory::destroy()
{

}

