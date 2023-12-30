#include "ZQ_common_conf.h"
#include "TianShanDefines.h"
#include "SaIceImpl.h"
#include "SiteAdminEnv.h"
#include "SiteAdminDatabase.h"
#include "SiteAdminSvcImpl.h"
#include "TianShanIceHelper.h"

//////////////////////////////////////////////////////////////////////////
///CVirtualSiteImpl
CVirtualSiteImpl::CVirtualSiteImpl( SiteAdminEnv& env ,SiteAdminDb& db )
:mEnv(env),
mDb(db)
{
}
CVirtualSiteImpl::~CVirtualSiteImpl()
{
}

TianShanIce::Site::VirtualSite CVirtualSiteImpl::getVsInfo(const ::Ice::Current& ) const
{
	Lock sync(*this);
	return vsinfo;
}

void CVirtualSiteImpl::updateVsInfo(const ::TianShanIce::Site::VirtualSite& info, const ::Ice::Current& )
{
	Lock sync(*this);
	vsinfo = info;
}

void CVirtualSiteImpl::destroy( const ::Ice::Current& )
{
	Lock sync(*this);
	mDb.removeVirtualSite( vsinfo.name );
}

void CVirtualSiteImpl::updateProp(const ::std::string& key, const ::std::string& value, const ::Ice::Current& )
{
	Lock sync(*this);
	vsinfo.properties[key] = value;
}

void CVirtualSiteImpl::updateProps(const ::TianShanIce::Properties& props, const ::Ice::Current& )
{
	Lock sync(*this);
	ZQTianShan::Util::mergeProperty( vsinfo.properties, props);
}

::TianShanIce::Properties CVirtualSiteImpl::getProps(const ::Ice::Current& )
{
	Lock sync(*this);
	return vsinfo.properties;
}

void CVirtualSiteImpl::updateLimitation(::Ice::Long maxBW, ::Ice::Int maxSess, const ::Ice::Current& )
{
	Lock sync(*this);
	vsinfo.maxDownstreamBwKbps	= maxBW;
	vsinfo.maxSessions			= maxSess;
}

//////////////////////////////////////////////////////////////////////////
///CAppInfoImpl
CAppInfoImpl::CAppInfoImpl( SiteAdminEnv& env ,SiteAdminDb& db)
:mEnv(env),
mDb(db)
{
}
CAppInfoImpl::~CAppInfoImpl()
{
}

::TianShanIce::Site::AppInfo CAppInfoImpl::getAppInfo(const ::Ice::Current& ) const 
{
	Lock sync(*this);
	return apinfo;
}

void CAppInfoImpl::updateAppInfo(const ::TianShanIce::Site::AppInfo& info, const ::Ice::Current& )
{
	Lock sync(*this);
	apinfo = info;
}
void CAppInfoImpl::destroy(const Ice::Current & )
{
	Lock sync(*this);
	mDb.removeAppInfo( apinfo.name );
}

//////////////////////////////////////////////////////////////////////////
///AppMountImpl
AppMountImpl::AppMountImpl(SiteAdminEnv& env,SiteAdminDb& db)
:mEnv(env),
mDb(db)
{
}

AppMountImpl::~AppMountImpl()
{
}

::std::string AppMountImpl::getMountedPath(const ::Ice::Current& ) const
{
	Lock sync(*this);
	return mountedPath;
}

::std::string AppMountImpl::getAppName(const ::Ice::Current& ) const
{
	Lock sync(*this);
	return appName;
}

void AppMountImpl::destroy(const ::Ice::Current& )
{
	Lock sync(*this);
	mDb.removeAppMount( ident.name );
}


//////////////////////////////////////////////////////////////////////////
///LiveTxnImpl
LiveTxnImpl::LiveTxnImpl(SiteAdminEnv& env ,SiteAdminDb& db ,  TxnWatchDog& watchDog , TxnTransfer& transfer, SiteAdminImpl& admin )
:mEnv(env),
mDb(db),
mTxnTransfer(transfer),
mWatchDog(watchDog),
mAdmin(admin)
{
}
LiveTxnImpl::LiveTxnImpl( SiteAdminEnv& env ,SiteAdminDb& db ,TxnWatchDog& watchDog ,TxnTransfer& transfer, SiteAdminImpl& admin , const std::string& sessId )
:mEnv(env),
mDb(db),
mTxnTransfer(transfer),
mWatchDog(watchDog),
mAdmin(admin)
{
	this->sessId = sessId;
	updateTimer( 3600 * 1000 );
}

LiveTxnImpl::~LiveTxnImpl()
{
}

::std::string LiveTxnImpl::getSessId(const ::Ice::Current& ) const
{
	Lock sync(*this);
	return ident.name;
}

::std::string LiveTxnImpl::getSitename(const ::Ice::Current& ) const 
{
	Lock sync(*this);
	return siteName;
}

::std::string LiveTxnImpl::getPath(const ::Ice::Current& ) const 
{
	Lock sync(*this);
	return mountedPath;
}

::TianShanIce::State LiveTxnImpl::getState(const ::Ice::Current& ) const 
{
	Lock sync(*this);
	return lastState;
}

void LiveTxnImpl::setState(::TianShanIce::State state, const ::Ice::Current& ) 
{
	Lock sync(*this);
	lastState = state;
}

void LiveTxnImpl::setProperty(const ::std::string& key, const ::std::string& value, const ::Ice::Current& ) 
{
	Lock sync(*this);
	properties[ key ] = value;
}

::TianShanIce::Properties LiveTxnImpl::getProperties(const ::Ice::Current& ) const 
{
	Lock sync(*this);
	return properties;
}

void LiveTxnImpl::destroy(const ::Ice::Current& ) 
{
	Lock sync(*this);
	try
	{		
		mTxnTransfer.pushSess( sessId );
		mWatchDog.UnWatchMe( sessId );
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AppMount, "destroy(id=%s) object already gone, ignore"), sessId.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		::ZQTianShan::_IceThrow <TianShanIce::ServerError> (MLOG, EXPFMT(AppMount, 1201, "destroy(id=%s) caught DB exception: %s"), sessId.c_str(), ex.ice_name().c_str());
	}
}
void LiveTxnImpl::updateTimer(Ice::Long lMilliSeconds,const ::Ice::Current& )
{
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(LiveTxnImpl,"updateTimer() txn[%s] update timer interval [%lld]"),
		sessId.c_str() , lMilliSeconds );
	mWatchDog.WatchMe( sessId , lMilliSeconds );
}
void LiveTxnImpl::onTimer(const ::Ice::Current& ) 
{
	bool bCanBeDestroyed = false;
	try
	{
		SRMSess->ice_ping(); // ping weiwoo's session to see if the txn should be destroyed or not
		updateTimer( 1*3600*1000 + rand() % ( 15*60*1000 ) );
		mLastTimeout = 0;
	}
	catch( const Ice::ObjectNotExistException& )
	{
		bCanBeDestroyed = true;
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(LiveTxnImpl,"txn[%s] SRM session not exist, post LiveTxn to YTD database"), sessId.c_str() );
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(LiveTxnImpl,"Got %s while pinging txn[%s] from SRM"),ex.ice_name().c_str() , sessId.c_str() );
		
		::Ice::Long stampNow = ZQTianShan::now();
		if (mLastTimeout <=0)
			mLastTimeout = stampNow;

		if (stampNow - mLastTimeout <6*3600*1000)
		{
			MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(LiveTxn,"txn[%s] failed to ping SRM session, will retry in 1 hr"), sessId.c_str());
			updateTimer( 3600 * 1000 );
		}
		else
		{
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(LiveTxn,"txn[%s] failed to keep touch with SRM session in 6 hr, cleanup"), sessId.c_str());
			bCanBeDestroyed = true;
		}
	}

	if (!bCanBeDestroyed)
	{
		if( TianShanIce::stOutOfService == lastState )
		{
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LiveTxn,"txn[%s] is at state OutOfService. Watch it in 1 hr."), sessId.c_str());
			updateTimer(3600 * 1000);
		}
		return;
	}
	
	try
	{
		Ice::Current dummyC;
		mAdmin.postYTD( sessId , dummyC );
	}
	catch (...) {}
}



//////////////////////////////////////////////////////////////////////////
///TxnEventImpl
TxnEventImpl::TxnEventImpl( SiteAdminEnv& env , SiteAdminDb& db )
:mEnv(env),
mDb(db)
{
}

TxnEventImpl::~TxnEventImpl()
{
}

void TxnEventImpl::get(::Ice::Identity& identTxn, ::std::string& stampUTC, ::std::string& category, ::std::string& eventCode, ::std::string& eventMsg, const ::Ice::Current& c) const
{
	Lock sync(*this);
	identTxn = this->identTxn;
	stampUTC = this->stampUTC;
	category = this->category;
	eventCode = this->eventCode;
	eventMsg = this->eventMsg;
}

::TianShanIce::Properties TxnEventImpl::getEventInfo(const ::TianShanIce::StrValues& params, const ::Ice::Current& ) const
{
	::TianShanIce::Properties ret;
	int i = 0 ;
	int count = (int)params.size();
	for ( ; i < count ; i ++ )
	{
		if (strcmp(params[i].c_str(), "stampUTC") == 0)
		{
			ret["stampUTC"] = stampUTC;
		}
		else if (strcmp(params[i].c_str(), "category") == 0)
		{
			ret["category"] = category;
		}
		else if (strcmp(params[i].c_str(), "eventCode") == 0)
		{
			ret["eventCode"] = eventCode;
		}
		else if (strcmp(params[i].c_str(), "eventMsg") == 0)
		{
			ret["eventMsg"] = eventMsg;
		}

	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////
///SaBaseCommand
SaBaseCommand::SaBaseCommand( SiteAdminEnv& env )
:mEnv(env),
ZQ::common::ThreadRequest(env.getMainThreadpool())
{
	ZQ::common::NativeThreadPool& pool = env.getMainThreadpool();
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SaBaseCommand,"thread pool usage:pending request[%d] active thread[%d] total thread[%d]"),
		pool.pendingRequestSize(),pool.activeCount(),pool.size());
}

SaBaseCommand::~SaBaseCommand()
{
}

void SaBaseCommand::final(int , bool )
{
	delete this;
}

//////////////////////////////////////////////////////////////////////////
///ListTxnCommand
ListTxnCommand::ListTxnCommand( SiteAdminEnv& env , SiteAdminDb& db ,TianShanIce::Site::AMD_TxnService_listLiveTxnPtr amd)
:SaBaseCommand(env),
mDb(db),
mAmdResponse(amd)
{
	mMaxCount = 0;
}
ListTxnCommand::~ListTxnCommand()
{
}
void ListTxnCommand::prepare( const ::std::string& siteName, const ::std::string& appMount, const ::TianShanIce::StrValues& paramNames, const ::std::string &startId, int maxCount )
{
	mSiteName	= siteName;
	mAppMount	= appMount;
	mParamNames	= paramNames;
	mStartId	= startId;
	mMaxCount	= maxCount;
}

void ListTxnCommand::listTxn( TianShanIce::Site::TxnInfos& infos )
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ListTxnCommand, "list live transaction by site[%s] and app[%s]"), mSiteName.c_str(), mAppMount.c_str());

	if ( "*" == mSiteName)
		mSiteName = "";
	if ( "*" == mAppMount)
		mAppMount = "";
	std::vector<TianShanIce::Site::LiveTxnPrx>  prxs = mDb.openTxnBySiteAndMount( mSiteName , mAppMount , mStartId );
	std::vector<TianShanIce::Site::LiveTxnPrx>::iterator it = prxs.begin();
	for( ; it != prxs.end() ; it ++ )
	{
		TianShanIce::Site::LiveTxnPrx& txn = *it;
		TianShanIce::Site::TxnInfo info;
		try
		{
			info.sessId = txn->getSessId();
			
			::TianShanIce::Properties props = txn->getProperties();

#define TXNPARAM_BEGIN  if (0)
#define TXNPARAM_ITEM(FIELD, SVALUE)   else if (0 == pit->compare(FIELD)) info.params.insert(::TianShanIce::Properties::value_type(FIELD, SVALUE))
#define TXNPARAM_END else if (props.end() !=props.find(*pit)) info.params.insert(::TianShanIce::Properties::value_type(*pit, props[*pit]))

			for (::TianShanIce::StrValues::iterator pit= mParamNames.begin(); pit != mParamNames.end() && infos.size() < (size_t)mMaxCount ; pit++)
			{
				TXNPARAM_BEGIN;
					TXNPARAM_ITEM(SYS_PROP(siteName), txn->getSitename());
					TXNPARAM_ITEM(SYS_PROP(path),     txn->getPath());
					TXNPARAM_ITEM(SYS_PROP(state),    ZQTianShan::ObjStateStr(txn->getState()));
				TXNPARAM_END;
			}
			infos.push_back( info );
		}
		catch( const Ice::Exception& ex)
		{
			MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ListTxnCommand,"listTxn() caught %s"),ex.ice_name().c_str() );
		}
	}
}

int ListTxnCommand::run()
{
	TianShanIce::Site::TxnInfos infos;
	try
	{
		listTxn(infos);
	}
	catch( const TianShanIce::BaseException& ex )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(ListTxnCommand,"run() caught %s while listTxn for site[%s] appmount[%s]"),
			ex.message.c_str() , mSiteName.c_str() , mAppMount.c_str() );
		try
		{
			mAmdResponse->ice_exception(ex);
		}
		catch( const Ice::Exception& ex)
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(ListTxnCommand,"failed to post response to client, caught %s"),ex.ice_name().c_str());
		}
		return 0;//return if we already deliver an execption to client
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(ListTxnCommand,"run() caught %s while listTxn for site[%s] appmount[%s]"),
			ex.ice_name().c_str() , mSiteName.c_str() , mAppMount.c_str() );
		try
		{
			mAmdResponse->ice_exception(ex);
		}
		catch( const Ice::Exception& ex)
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(ListTxnCommand,"failed to post response to client, caught %s"),ex.ice_name().c_str());
		}
		return 0;//return if we already deliver an execption to client
	}
	try
	{
		mAmdResponse->ice_response(infos);
	}
	catch( const Ice::Exception& ex )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(ListTxnCommand,"failed to post response to client, caught %s"),ex.ice_name().c_str());
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
///SaveEventCommand
SaveEventCommand::SaveEventCommand( SiteAdminEnv& env, SiteAdminDb& db , ::TianShanIce::Site::AMD_TxnService_tracePtr amd )
:SaBaseCommand(mEnv),
mDb(db),
mAmdResponse(amd)
{
	mTxnEvent = new TxnEventImpl(mEnv,mDb);
	char stampbuf[64];
	mTxnEvent->stampUTC = ::ZQTianShan::TimeToUTC( ZQ::common::now(), stampbuf, sizeof(stampbuf) -2);
	mTxnEvent->identTxn.name = IceUtil::generateUUID();
}

SaveEventCommand::~SaveEventCommand( )
{

}
void SaveEventCommand::prepare(const std::string &sessId, const std::string &category, const std::string &eventCode, const std::string &eventMsg)
{
	mTxnEvent->sessId		= sessId;
	mTxnEvent->category		= category;
	mTxnEvent->eventCode	= eventCode;
	mTxnEvent->eventMsg		= eventMsg;
}
int SaveEventCommand::run()
{
	try
	{
		mDb.addTxnEvent( mTxnEvent );
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin,"SaveEventCommand() saved: %s sess[%s] %s:%s %s"),
			mTxnEvent->stampUTC.c_str(), mTxnEvent->sessId.c_str(), mTxnEvent->category.c_str(), mTxnEvent->eventCode.c_str(), mTxnEvent->eventMsg.c_str());
	}
	catch( const Ice::Exception& ex )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SaveEventCommand,"run() caught %s while save event for sess[%s]"),ex.ice_name().c_str() , mSessId.c_str() );
		try
		{
			mAmdResponse->ice_exception(ex);
		}
		catch( const Ice::Exception&)
		{
		}
	}
	return 0;
}

