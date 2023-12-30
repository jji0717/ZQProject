#ifndef __tianshan_siteadmin_ice_class_implementation_header_file__
#define __tianshan_siteadmin_ice_class_implementation_header_file__

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include "SiteAdminSvc.h"
#include "TxnWatchDog.h"
#include "TxnTransfer.h"
#include "NativeThreadPool.h"

class SiteAdminEnv;
class SiteAdminDb;
class SiteAdminImpl;

class CVirtualSiteImpl : public TianShanIce::Site::CVirtualSite, public IceUtil::AbstractMutexI<IceUtil::RecMutex>
{
public:
	CVirtualSiteImpl( SiteAdminEnv& env , SiteAdminDb& db );
	virtual ~CVirtualSiteImpl();

	virtual ::TianShanIce::Site::VirtualSite getVsInfo(const ::Ice::Current& = ::Ice::Current()) const;
	virtual void updateVsInfo(const ::TianShanIce::Site::VirtualSite&, const ::Ice::Current& = ::Ice::Current());
	virtual void destroy( const ::Ice::Current& = ::Ice::Current() );
	virtual void updateProp(const ::std::string&, const ::std::string&, const ::Ice::Current& = ::Ice::Current());
	virtual void updateProps(const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current());
	virtual ::TianShanIce::Properties getProps(const ::Ice::Current& = ::Ice::Current());
	virtual void updateLimitation(::Ice::Long, ::Ice::Int, const ::Ice::Current& = ::Ice::Current());
private:
	SiteAdminEnv&			mEnv;
	SiteAdminDb&			mDb;
};

class CAppInfoImpl : public TianShanIce::Site::CAppInfo , public IceUtil::AbstractMutexI<IceUtil::RecMutex>
{
public:
	CAppInfoImpl( SiteAdminEnv& env ,SiteAdminDb& db );
	virtual ~CAppInfoImpl();

	virtual ::TianShanIce::Site::AppInfo getAppInfo(const ::Ice::Current& = ::Ice::Current()) const ;
	virtual void updateAppInfo(const ::TianShanIce::Site::AppInfo&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void destroy( const ::Ice::Current& = ::Ice::Current() );

private:

	SiteAdminEnv&			mEnv;
	SiteAdminDb&			mDb;
};

class AppMountImpl : public TianShanIce::Site::AppMount, public IceUtil::AbstractMutexI<IceUtil::RecMutex>
{
public:
	AppMountImpl( SiteAdminEnv& env ,SiteAdminDb& db );
	virtual ~AppMountImpl();

	virtual ::std::string getMountedPath(const ::Ice::Current& = ::Ice::Current()) const;
	virtual ::std::string getAppName(const ::Ice::Current& = ::Ice::Current()) const; 
	virtual void destroy(const ::Ice::Current& = ::Ice::Current());

private:
	SiteAdminEnv&			mEnv;
	SiteAdminDb&			mDb;
};


class LiveTxnImpl : public TianShanIce::Site::LiveTxn, public IceUtil::AbstractMutexI<IceUtil::RecMutex>
{
public:

	LiveTxnImpl( SiteAdminEnv& env ,SiteAdminDb& db ,TxnWatchDog& watchDog , TxnTransfer& transfer , SiteAdminImpl& admin );
	LiveTxnImpl( SiteAdminEnv& env ,SiteAdminDb& db ,TxnWatchDog& watchDog , TxnTransfer& transfer, SiteAdminImpl& admin , const std::string& sessId );
	~LiveTxnImpl();

	virtual ::std::string getSessId(const ::Ice::Current& = ::Ice::Current()) const;

	virtual ::std::string getSitename(const ::Ice::Current& = ::Ice::Current()) const ;

	virtual ::std::string getPath(const ::Ice::Current& = ::Ice::Current()) const ;

	virtual ::TianShanIce::State getState(const ::Ice::Current& = ::Ice::Current()) const ;

	virtual void setState(::TianShanIce::State, const ::Ice::Current& = ::Ice::Current()) ;

	virtual void setProperty(const ::std::string&, const ::std::string&, const ::Ice::Current& = ::Ice::Current()) ;

	virtual ::TianShanIce::Properties getProperties(const ::Ice::Current& = ::Ice::Current()) const ;

	virtual void destroy(const ::Ice::Current& = ::Ice::Current()) ;

	virtual void onTimer(const ::Ice::Current& = ::Ice::Current()) ;

	virtual void updateTimer(Ice::Long lMilliSeconds,const ::Ice::Current& = ::Ice::Current());

protected:

	SiteAdminEnv&			mEnv;
	SiteAdminDb&			mDb;
	TxnWatchDog&			mWatchDog;
	TxnTransfer&			mTxnTransfer;
	SiteAdminImpl&			mAdmin;
	bool					mbIsAlive;

	::Ice::Long				mLastTimeout; // thredhold to give up getting the destroy confirmation from SRM
};


class TxnEventImpl : public TianShanIce::Site::TxnEvent, public IceUtil::AbstractMutexI<IceUtil::RecMutex>
{
public:

	TxnEventImpl( SiteAdminEnv& env , SiteAdminDb& db );
	virtual ~TxnEventImpl();

	virtual void get(::Ice::Identity& identTxn, ::std::string& stampUTC, ::std::string& category, ::std::string& eventCode, ::std::string& eventMsg, const ::Ice::Current& c) const;

	virtual ::TianShanIce::Properties getEventInfo(const ::TianShanIce::StrValues& params, const ::Ice::Current& = ::Ice::Current()) const;

protected:
	SiteAdminEnv&			mEnv;
	SiteAdminDb&			mDb;
};


class SaBaseCommand : public ZQ::common::ThreadRequest
{
public:
	SaBaseCommand( SiteAdminEnv& env );
	virtual ~SaBaseCommand();

protected:
	
	void	final(int retcode /* =0 */, bool bCancelled /* =false */);

protected:
	SiteAdminEnv&		mEnv;
};

class ListTxnCommand : public SaBaseCommand
{
public:
	ListTxnCommand( SiteAdminEnv& env , SiteAdminDb& db , TianShanIce::Site::AMD_TxnService_listLiveTxnPtr amd );
	virtual ~ListTxnCommand();

	void	prepare( const ::std::string& siteName, const ::std::string& appMount, const ::TianShanIce::StrValues& paramNames, const ::std::string &startId, int maxCount );

protected:
	
	void	listTxn( TianShanIce::Site::TxnInfos& infos );

	int		run();

protected:
	SiteAdminDb&		mDb;
	TianShanIce::Site::AMD_TxnService_listLiveTxnPtr mAmdResponse;
	std::string			mSiteName;
	std::string			mAppMount;
	std::string			mStartId;
	int					mMaxCount;
	::TianShanIce::StrValues mParamNames;
};

class SaveEventCommand : public SaBaseCommand
{
public:
	SaveEventCommand( SiteAdminEnv& env, SiteAdminDb& db , ::TianShanIce::Site::AMD_TxnService_tracePtr amd );
	virtual ~SaveEventCommand();
	void prepare( const ::std::string& sessId, const ::std::string& category, const ::std::string& eventCode, const ::std::string& eventMsg );

protected:
	int			run();
protected:
	SiteAdminDb&		mDb;
	::TianShanIce::Site::AMD_TxnService_tracePtr mAmdResponse;
	std::string			mSessId;
	std::string			mCategory;
	std::string			mEventCode;
	std::string			mEventMsg;
	TianShanIce::Site::TxnEventPtr mTxnEvent;
};


#endif//__tianshan_siteadmin_ice_class_implementation_header_file__

