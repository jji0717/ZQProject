
#ifndef __tianshan_siteadmin_ice_service_implement_header_file_h__
#define __tianshan_siteadmin_ice_service_implement_header_file_h__

#include "TianShanDefines.h"
#include <TsApplication.h>
#include "SiteAdminSvc.h"
#include "TxnTransfer.h"

class SiteAdminEnv;
class TxnWatchDog;
class SiteAdminDb;


class SiteAdminImpl : public ::TianShanIce::Site::SiteAdmin, public IceUtil::AbstractMutexI<IceUtil::RecMutex>
{
public:
	SiteAdminImpl( SiteAdminEnv& env , SiteAdminDb& db , TxnWatchDog& watchDog ,TxnTransfer& transfer );
	virtual ~SiteAdminImpl(void);

	SiteAdminDb&		getDb();

public:	// impls of BaseService

	virtual ::std::string getAdminUri(const ::Ice::Current& c);
	virtual ::TianShanIce::State getState(const ::Ice::Current& c);

public:	// impls of BusinessRouter

	virtual ::TianShanIce::Site::VirtualSites listSites(const ::Ice::Current& c) const;
	virtual ::TianShanIce::Application::PurchasePrx resolvePurchase(const ::TianShanIce::SRM::SessionPrx& sess, const ::Ice::Current& c);

public:	// impls of SiteAdmin

	virtual bool updateSite(const ::std::string&, const ::std::string&, const ::Ice::Current& c);
	virtual bool removeSite(const ::std::string&, const ::Ice::Current& c);
	virtual bool setSiteProperties(const ::std::string& siteName, const ::TianShanIce::Properties& props, const ::Ice::Current& c);
	virtual ::TianShanIce::Properties getSiteProperties(const ::std::string& siteName, const ::Ice::Current& c) const;
	virtual ::TianShanIce::Site::AppInfos listApplications(const ::Ice::Current& c) const;
	virtual bool updateApplication(const ::std::string&, const ::std::string&, const ::std::string&, const ::Ice::Current& c);
	virtual bool removeApplication(const ::std::string&, const ::Ice::Current& c);
	virtual ::TianShanIce::Site::AppMounts listMounts(const ::std::string&, const ::Ice::Current& c) const;
	virtual ::TianShanIce::Site::AppMountPrx mountApplication(const ::std::string&, const ::std::string&, const ::std::string&, const ::Ice::Current& c);
	virtual bool unmountApplication(const ::std::string&, const ::std::string&, const ::Ice::Current& c);
	virtual ::TianShanIce::Site::AppInfo findApplication(const ::std::string&, const ::std::string&, const ::Ice::Current& c) const;

	virtual void commitStateChange(const ::std::string& sessId, ::TianShanIce::State state, const ::TianShanIce::SRM::SessionPrx& sess, const ::TianShanIce::Properties& props, const ::Ice::Current& c);

	virtual void listLiveTxn_async(const ::TianShanIce::Site::AMD_TxnService_listLiveTxnPtr& amdCB, const ::std::string& siteName, const ::std::string& appMount, const ::TianShanIce::StrValues& paramNames, const ::std::string &startId, int maxCount, const ::Ice::Current& c) const;
	virtual void trace_async(const ::TianShanIce::Site::AMD_TxnService_tracePtr& amdCB, const ::std::string& sessId, const ::std::string& category, const ::std::string& eventCode, const ::std::string& eventMsg, const ::Ice::Current& c);

	virtual void postYTD(const ::std::string& sessId, const ::Ice::Current& c);
	virtual void setUserProperty(const ::std::string& sessId, const ::std::string& key, const ::std::string& value, const ::Ice::Current& c);

	virtual void dumpLiveTxn_async(const ::TianShanIce::Site::AMD_TxnService_dumpLiveTxnPtr& amdCB, const ::std::string& sessId, const ::std::string& beginFormat, const ::std::string& traceFormat, const ::std::string& endFormat, const ::Ice::Current& c);
	//    virtual void dumpHistoryTxn_async(const ::TianShanIce::Site::AMD_TxnService_dumpHistoryTxnPtr&, const ::std::string& sessId, const ::std::string& beginFormat, const ::std::string& traceFormat, const ::std::string& endFormat, const ::Ice::Current& c);

	::TianShanIce::Site::LiveTxnPrx newTxn(const ::std::string& sessId, ::Ice::Long bandwidth, const ::std::string& siteName, const ::std::string& appName, const ::std::string& mountPath, const ::Ice::Current& c);

	virtual void updateSiteResourceLimited(const ::std::string& siteName, ::Ice::Long maxBW, ::Ice::Int maxSessions, const ::Ice::Current& c);
	virtual void restrictSiteResources(const ::std::string& siteName, const ::TianShanIce::SRM::ResourceMap& resources, const ::Ice::Current& c);
	virtual ::TianShanIce::SRM::ResourceMap getSiteResourceRestricutions(const ::std::string& siteName, const ::Ice::Current& c) const;

	void	init();
protected:
	
	typedef struct _tagUsedSiteResource
	{
		Ice::Int			_usedSessions;
		Ice::Long			_usedBandwidth;
		Ice::Int			_maxSessions;
		Ice::Long			_maxBandwidth;
	} UsedSiteResource;

	void		sendTxnInServiceMsg( const ::std::string& sessId, ::TianShanIce::State state, const ::TianShanIce::SRM::SessionPrx& sess, const ::TianShanIce::Properties& props);
	void		sendTxnOutOfServiceMsg(  const ::std::string& sessId, ::TianShanIce::State state, const ::TianShanIce::SRM::SessionPrx& sess, const ::TianShanIce::Properties& props );
	
	void		saveTxnDataLog( const std::string& sessId , TianShanIce::Site::LiveTxnPrx txn );

	void		removeSiteResourceUsage( const std::string& sessId, const std::string& siteName , int64 bandwidth );
	bool		addSiteResourceUsage( const std::string& sessId, const std::string& siteName, int64 bandwidth );

	void		checkUrl( const std::string& sessId , const std::string& uri , std::string& siteName, std::string& path );
	void		applySiteRestrcitionOnSession( const std::string& sessId , const std::string& siteName , const std::string& path , const ::TianShanIce::SRM::SessionPrx& sess);
	TianShanIce::Application::AppServicePrx getApplication( const std::string& sessId, const std::string& siteName , const std::string& pathName , const ::TianShanIce::SRM::SessionPrx& sess , TianShanIce::Site::AppInfo& info);
	TianShanIce::Application::PurchasePrx createPurchase( const std::string& sessId, const std::string& siteName ,const TianShanIce::SRM::SessionPrx& sess, TianShanIce::Application::AppServicePrx appService );
	int64		applyBWRestrcition( TianShanIce::Application::PurchasePrx purchase , const TianShanIce::SRM::SessionPrx& sess, const std::string sessId, const std::string& siteName, UsedSiteResource& resUsage );
	void		createLiveTxn( const std::string& sessId, const std::string& siteName, const std::string& pathName, int64 requestBW, const TianShanIce::Site::AppInfo& appinfo, const UsedSiteResource& resUsage, const TianShanIce::SRM::SessionPrx& sess );

private:
	
	SiteAdminEnv&			mEnv;
	SiteAdminDb&			mDb;
	TxnWatchDog&			mWatchDog;
	TxnTransfer&			mTxnTransfer;	

	typedef std::map<std::string,UsedSiteResource>		USEDRESMAP;
	USEDRESMAP				mUsedResMap;
	ZQ::common::Mutex		mUsedResLocker;	
};
typedef IceUtil::Handle<SiteAdminImpl>	SiteAdminImplPtr;
#define DEFAULT_SITENAME "."

#endif//__tianshan_siteadmin_ice_service_implement_header_file_h__

