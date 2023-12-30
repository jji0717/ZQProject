
#include "SiteAdminSvcImpl.h"
#include "SiteAdminEnv.h"
#include "SaIceImpl.h"
#include "SiteAdminDatabase.h"
#include <TianShanIceHelper.h>
#include <urlstr.h>
#include <TimeUtil.h>


//////////////////////////////////////////////////////////////////////////
///SiteAdminImpl
SiteAdminImpl::SiteAdminImpl( SiteAdminEnv& env, SiteAdminDb& db, TxnWatchDog& watchDog, TxnTransfer& transfer)
:mEnv(env)
,mDb(db),
mWatchDog(watchDog),
mTxnTransfer(transfer)
{	
}

void SiteAdminImpl::init()
{	
	TianShanIce::Site::VirtualSites sites = mDb.listSites();

	ZQ::common::MutexGuard gd(mUsedResLocker);
	TianShanIce::Site::VirtualSites::const_iterator itSite = sites.begin();
	for( ; itSite != sites.end() ; itSite ++ )
	{
		const TianShanIce::Site::VirtualSite& info = *itSite;
		USEDRESMAP::iterator it = mUsedResMap.find(info.name);
		if( it == mUsedResMap.end() )
		{
			UsedSiteResource res;
			res._maxBandwidth = info.maxDownstreamBwKbps;
			res._maxSessions = info.maxSessions;
			res._usedBandwidth = 0;
			res._usedSessions = 0;
			mUsedResMap.insert(USEDRESMAP::value_type(info.name,res));
		}
		else
		{
			UsedSiteResource& res = it->second;
			res._maxBandwidth = info.maxDownstreamBwKbps;
			res._maxSessions = info.maxSessions;
		}
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminImpl, "init() restore site[%s] resource limitation, maxBW[%lld] maxSess[%d]"),
			info.name.c_str(), info.maxDownstreamBwKbps,info.maxSessions);
	}
	//restore txn from databse
	std::vector<TianShanIce::Site::LiveTxnPrx> txns = mDb.listAllTxn();
	std::vector<TianShanIce::Site::LiveTxnPrx>::iterator it = txns.begin();
	size_t restoredSession = 0 ;
	for( ; it != txns.end() ; it ++ )
	{
		try
		{
			TianShanIce::Site::LiveTxnPrx txn = *it;
			std::string siteName  = txn->getSitename();
			std::string sessid = txn->getSessId();
			TianShanIce::Properties props = txn->getProperties();
			Ice::Long requestBW = -1;
			ZQTianShan::Util::getPropertyDataWithDefault( props, SYS_PROP(bandwidth), -1,requestBW );
			if( requestBW > 0 )
			{
				addSiteResourceUsage( sessid, siteName, requestBW );
				restoredSession ++;
			}
			txn->updateTimer( rand() % (20 * 1000) );
		}
		catch( const Ice::ObjectNotExistException& )
		{			
		}
		catch( const Ice::Exception& )
		{

		}
	}
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminImpl, "init() restore %u txns from db"),restoredSession);
}

SiteAdminImpl::~SiteAdminImpl(void)
{
}

::std::string SiteAdminImpl::getAdminUri(const ::Ice::Current& c)
{
	::ZQTianShan::_IceThrow<TianShanIce::NotImplemented> (MLOG, EXPFMT(SiteAdmin, 101, __MSGLOC__ "TODO: impl here"));
	return ""; // dummy statement to avoid compiler error
}

::TianShanIce::State SiteAdminImpl::getState(const ::Ice::Current& c)
{
	::ZQTianShan::_IceThrow<TianShanIce::NotImplemented> (MLOG, EXPFMT(SiteAdmin, 201, __MSGLOC__ "TODO: impl here"));
	return ::TianShanIce::stInService; // dummy statement to avoid compiler error
}

::TianShanIce::Site::VirtualSites SiteAdminImpl::listSites(const ::Ice::Current& c) const
{
	return mDb.listSites();
}

void SiteAdminImpl::checkUrl( const std::string& sessId, const std::string& uri, std::string& sitename, std::string& pathname )
{
	ZQ::common::URLStr urlstr( uri.c_str() );	
	std::string urlHost = urlstr.getHost();
	std::string urlPath = urlstr.getPath();

	// verify the url base on scheme: DNS mode / path mode
	ZQ::common::stringHelper::STRINGVECTOR stdpath;
	ZQ::common::stringHelper::SplitString(urlPath, stdpath, "/", "/");
		
#define UrlMode mEnv.getConfig().urlMode.c_str() 
#define IsPathMode ( 0 == stricmp( UrlMode, "Path" ) )
#define IsDNSMode (!IsPathMode)
	if( IsDNSMode )
	{
		if(stdpath.size() == 1)
		{ // use the url path as the app path
			sitename = urlHost;
			pathname = urlPath;
		}
		else
		{ // invalid url
			::ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (MLOG, EXPFMT(SiteAdmin, 301, "resolvePurchase() sess[%s] invalid URI [%s] in DNS mode."), sessId.c_str(), uri.c_str());
		}
	}
	else if (IsPathMode)
	{ // extract the site name and the app path from the url path
		switch(stdpath.size())
		{
		case 2:
			sitename = stdpath[0];
			pathname = stdpath[1];
			break;
		case 1: // only app path, use the default site
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "resolvePurchase() use default site in Path mode"));
			sitename = DEFAULT_SITENAME;
			pathname = urlPath;
			break;
		default: // invalid url
			::ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (MLOG, EXPFMT(SiteAdmin, 301, "resolvePurchase() sess[%s] invalid URI [%s] in Path mode"), sessId.c_str(), uri.c_str());
		}
	}
	else
	{ // shouldn't happen
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SiteAdmin, "resolvePurchase() Unknown url mode [%s]"), UrlMode);
	}
	
	if( !mDb.openCVirtualSite(sitename) )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(SiteAdmin, "resolvePurchase() site[%s] not found, try default site"),sitename.c_str() );
		sitename = DEFAULT_SITENAME;
	}
}

#define PD_KEY_SiteName			"site.SiteName"
#define PD_KEY_Path				"site.Path"
#define PD_KEY_URL				"site.Url"
#define KEY_Pref_App_Endpoint	"sys.PrefAppEndpoint"


void SiteAdminImpl::applySiteRestrcitionOnSession( const std::string& sessId, const std::string& siteName, const std::string& path, const ::TianShanIce::SRM::SessionPrx& sess)
{
	TianShanIce::ValueMap sessPd;
	ZQTianShan::Util::updateValueMapData( sessPd, PD_KEY_SiteName, siteName);
	ZQTianShan::Util::updateValueMapData( sessPd, PD_KEY_Path, path );
	try
	{
		sess->setPrivateData2(sessPd);
	}
	catch(...)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminImpl, "resolvePurchase() Failed to set site name (%s) and path (%s) into session[%s] private data."),
			siteName.c_str(), path.c_str(), sessId.c_str() );
	}

	Ice::Current dummyC;
	::TianShanIce::SRM::ResourceMap siteResRestrictions =  getSiteResourceRestricutions( siteName, dummyC );

	if (siteResRestrictions.end() != siteResRestrictions.find(TianShanIce::SRM::rtStorage))
	{
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SiteAdmin, "resolvePurchase() sess[%s] site[%s] has storage restriction, applying onto session"),
			sessId.c_str(), siteName.c_str());

		const ::TianShanIce::SRM::Resource& resOfSite = siteResRestrictions[TianShanIce::SRM::rtStorage];

		if (siteResRestrictions.end() == siteResRestrictions.find(TianShanIce::SRM::rtStorage))
		{
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "no resource[rtStorage] in session[%s], apply site[%s] restriction directly"), 
				sessId.c_str(), siteName.c_str());
			sess->addResourceEx(TianShanIce::SRM::rtStorage, resOfSite);
		}
		else
		{
			const ::TianShanIce::SRM::Resource& resOfSess = siteResRestrictions[TianShanIce::SRM::rtStorage];
			::TianShanIce::SRM::Resource resIntersection;
			if (! ZQTianShan::InterRestrictResource(resOfSess, resOfSite, resIntersection))
				::ZQTianShan::_IceThrow <TianShanIce::ServerError> (MLOG, EXPFMT(SiteAdmin, 312, "resolvePurchase() failed to intersection the resource[rtStorage] between session[%s] and site[%s] restriction"), 
						sessId.c_str(), siteName.c_str());
			resIntersection.attr	= resOfSess.attr;
			resIntersection.status	= resOfSess.status;
			sess->addResourceEx(TianShanIce::SRM::rtStorage, resIntersection);
		}
	}

	if (siteResRestrictions.end() != siteResRestrictions.find(TianShanIce::SRM::rtStreamer))
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "resolvePurchase() sess[%s] site[%s] has streamer restriction, applying onto session"),
			sessId.c_str(), siteName.c_str());

		const ::TianShanIce::SRM::Resource& resOfSite = siteResRestrictions[TianShanIce::SRM::rtStreamer];

		if (siteResRestrictions.end() == siteResRestrictions.find(TianShanIce::SRM::rtStreamer))
		{
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "no resource[rtStreamer] in session[%s], apply site[%s] restriction directly"), 
				sessId.c_str(), siteName.c_str());
			sess->addResourceEx(TianShanIce::SRM::rtStreamer, resOfSite);
		}
		else
		{
			const ::TianShanIce::SRM::Resource& resOfSess = siteResRestrictions[TianShanIce::SRM::rtStreamer];
			::TianShanIce::SRM::Resource resIntersection;
			if ( !ZQTianShan::InterRestrictResource(resOfSess, resOfSite, resIntersection))
				::ZQTianShan::_IceThrow <TianShanIce::ServerError> (MLOG, EXPFMT(SiteAdmin, 313, "resolvePurchase() failed to intersection the resource[rtStreamer] between session[%s] and site[%s] restriction"), sessId.c_str(), siteName.c_str());

			resIntersection.attr = resOfSess.attr;
			resIntersection.status = resOfSess.status;
			sess->addResourceEx(TianShanIce::SRM::rtStreamer, resIntersection);
		}
	}
}

TianShanIce::Application::AppServicePrx SiteAdminImpl::getApplication( const std::string& sessId, const std::string& siteName, const std::string& pathName, const ::TianShanIce::SRM::SessionPrx& sess, TianShanIce::Site::AppInfo& info)
{
	//get prefre application endpoint
	std::string preferredEndpoint;
	TianShanIce::ValueMap sessPd ;
	try
	{
		sessPd = sess->getPrivateData();		
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(SiteAdminImpl, "resolvePurchase() failed to get prefer app endpoint due to [%s]"),ex.ice_name().c_str() );
	}

	ZQTianShan::Util::getValueMapDataWithDefault( sessPd, KEY_Pref_App_Endpoint, "", preferredEndpoint );
	if( !preferredEndpoint.empty() )
	{
		info.endpoint = preferredEndpoint;
		std::string::size_type d = info.endpoint.find(':');
		if(d != std::string::npos)
		{ // extract the app name
			info.name = info.endpoint.substr(0, d);
		}
		else
		{
			info.name.clear();
		}
		info.desc = "Preferred application";
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(SiteAdmin, "resolvePurchase() detect preferred application [%s] at [%s] for sess[%s]"), 
			info.name.c_str(), info.endpoint.c_str(), sessId.c_str() );
	}
	else
	{
		Ice::Current dummyC;
		info = findApplication( siteName, pathName, dummyC );
	}

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(SiteAdmin, "resolvePurchase() session[%s] connect to application[%s] at [%s]"),
		sessId.c_str(), info.name.c_str(), info.endpoint.c_str());

	::TianShanIce::Application::AppServicePrx app = NULL;
	try 
	{
		// 3.1. trust the endpoint as the full proxy string
		app = ::TianShanIce::Application::AppServicePrx::checkedCast( mEnv.getIc()->stringToProxy( info.endpoint ) );
	}
	catch(const ::Ice::Exception& e)
	{
		app = NULL;
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "session[%s], exception when connect to application: %s "), sessId.c_str(), e.ice_name().c_str() );
	}
	
	std::string proxystr;
	if (!app)
	{
		try 
		{			
			proxystr = std::string(SERVICE_NAME_AppService ":") + info.endpoint;
			app = ::TianShanIce::Application::AppServicePrx::checkedCast( mEnv.getIc()->stringToProxy(proxystr) );
		}
		catch(const ::Ice::Exception& e)
		{
			app = NULL;
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "session[%s] caught exception when connect to APP[%s]: %s"),sessId.c_str(), proxystr.c_str(), e.ice_name().c_str());
		}
	}
	else
	{
		proxystr = mEnv.getIc()->proxyToString( app );
	}

	if (!app)
		::ZQTianShan::_IceThrow <TianShanIce::ServerError> (MLOG, EXPFMT(SiteAdmin, 302, "resolvePurchase() failed to connect to application[%s] at %s, session[%s]"), info.name.c_str(), info.endpoint.c_str(), sessId.c_str() );

	return app;
}

TianShanIce::Application::PurchasePrx SiteAdminImpl::createPurchase( const std::string& sessId, const std::string& siteName, const TianShanIce::SRM::SessionPrx& sess, TianShanIce::Application::AppServicePrx appService )
{
	Ice::Current dummyC;
	assert( appService );
	TianShanIce::Properties siteProps = getSiteProperties( siteName, dummyC );
	ZQTianShan::Util::TimeSpan span;
	TianShanIce::Application::PurchasePrx purchasePrx = NULL;
	try
	{
		std::string proxystr = mEnv.getIc()->proxyToString(appService);
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SiteAdmin, "resolvePurchase() sess[%s] createPurchase through [%s]"),sessId.c_str(),proxystr.c_str());

		span.start();
		purchasePrx = appService->createPurchase(sess, siteProps);		
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SiteAdmin, "resolvePurchase() sess[%s] createPurchase through [%s] ok and use time [%lld]"),
			sessId.c_str(), proxystr.c_str(), span.stop() );

		span.start();
		purchasePrx->provision();		

		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "resolvePurchase() sess[%s] purchase was provisioned, took %lldms"), 
			sessId.c_str(), span.stop() );
	}
	catch (const ::TianShanIce::BaseException& ex) 
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdmin, "TianShan exception caught when create and provison purchase:%s, sess[%s]"),
			ex.ice_name().c_str(), sessId.c_str() );
		_IceReThrow(TianShanIce::ServerError, ex);
	}
	catch (Ice::Exception& ex) 
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdmin, "ice exception [%s]  caught when CreatePurchase for Session %s"),
			ex.ice_name().c_str(),sessId.c_str());
		ex.ice_throw();
	}

	return purchasePrx;
}

int64 SiteAdminImpl::applyBWRestrcition( TianShanIce::Application::PurchasePrx purchase, const TianShanIce::SRM::SessionPrx& sess, const std::string sessId, const std::string& siteName, UsedSiteResource& resUsage)
{
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SiteAdmin, "resolvePurchase() sess[%s] get resource from SRM session"), sessId.c_str() );
	TianShanIce::SRM::ResourceMap resMap =  sess->getReources();
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdmin, "resolvePurchase() sess[%s] get resource from SRM session ok and find bandwidth resource"), sessId.c_str() );

	int64 requestBW = -1;
	ZQTianShan::Util::getResourceDataWithDefault( resMap, TianShanIce::SRM::rtTsDownstreamBandwidth, "bandwidth", -1, requestBW );
	if( requestBW < 0 )
	{
		TianShanIce::Properties prop;
		prop[SYS_PROP(terminateReason)]= "213001 no bandwidth information is given after purchase is resolved";
		TianShanIce::Variant var ;
		var.type = TianShanIce::vtStrings;
		var.bRange = false;	var.strs.clear ();
		var.strs.push_back ( prop[SYS_PROP(terminateReason)] );			
		sess->setPrivateData ( SYS_PROP(terminateReason), var);
		purchase->detach( sessId, prop );		
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdmin, "No bandwidth is got after provision purchase, sess[%s]"),sessId.c_str() );
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, EXPFMT(SiteAdmin, 303, "resolvePurchase() sess[%s] no bandwidth information is given after purchase is resolved"), sessId.c_str());
	}

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdmin, "resolvePurchase() sess[%s] get bandwidth [%lld] through SRM session"),sessId.c_str(),requestBW);

	int64	curBW = 0;
	int64	maxBW = 0;
	int		curSessions = 0;
	int		maxSessions = 0;
	int     requestedKbps = (requestBW +999)/1000;

	bool bOk = false;
	do {
		ZQ::common::MutexGuard gd(mUsedResLocker);		

		USEDRESMAP::iterator itResource = mUsedResMap.find(siteName);
		if( itResource == mUsedResMap.end() )
			break;

		UsedSiteResource& res = itResource->second;
		curBW = res._usedBandwidth;
		curSessions = res._usedSessions;
		maxBW = res._maxBandwidth;
		maxSessions = res._maxSessions;

		if (res._maxBandwidth >0 && (res._usedBandwidth + requestedKbps) >res._maxBandwidth)
			break;

		if (res._maxSessions >0 && res._usedSessions >= res._maxSessions)
			break;

		res._usedBandwidth += requestedKbps;
		res._usedSessions ++;
		resUsage = res;
		bOk = true;

		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdmin, "resolvePurchase() adjusted usage of site[%s] for session[%s]: BW[%lld/%lld]Kbps sessions[%d/%d]"),
			siteName.c_str(), sessId.c_str(), res._usedBandwidth, res._maxBandwidth, res._usedSessions, res._maxSessions );

	} while(0);

	if (!bOk)
	{
		TianShanIce::Properties prop;
		char	szBuf[1024];
		sprintf(szBuf, "213002 no enough resource for sess[%s] site[%s]: requested=%dbps"
			"currentBW="FMT64",currentSession=%d,totalBW="FMT64",totalSess=%d",
			sessId.c_str(), siteName.c_str(), requestBW, curBW, curSessions, maxBW, maxSessions);

		std::string	strTerminateReason = szBuf;
		prop[SYS_PROP(terminateReason)]= strTerminateReason;
		TianShanIce::Variant var ;
		var.bRange = false;
		var.type = TianShanIce::vtStrings;
		var.strs.clear ();
		var.strs.push_back ( strTerminateReason );		
		sess->setPrivateData ( SYS_PROP(terminateReason), var);

		purchase->detach(sessId,prop);		

		::ZQTianShan::_IceThrow<TianShanIce::Site::OutOfQuota>(MLOG,EXPFMT(SiteAdmin, 305, "resolvePurchase() session[%s] "
			"no enough resource on site[%s] requested[%lld]bps "
			"bw[%lld/%lld]Kbps sessions[%d/%d]"),
			sessId.c_str(), siteName.c_str(), requestBW,
			curBW, maxBW, curSessions, maxSessions);
	}

	return requestBW;
}

void SiteAdminImpl::createLiveTxn( const std::string& sessId, const std::string& siteName, const std::string& pathName,
								  int64 requestBW, const TianShanIce::Site::AppInfo& appinfo, const UsedSiteResource& resUsage,
								  const TianShanIce::SRM::SessionPrx& sess )
{
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(SiteAdmin, "resolvePurchase() sess[%s] prepare a new live txn with ID [%s]"),sessId.c_str(),sessId.c_str());
	TianShanIce::Site::LiveTxnPtr p = new LiveTxnImpl(mEnv,mDb,mWatchDog,mTxnTransfer,*this, sessId );
	p->ident.name		= p->sessId = sessId;
	p->lastState		= TianShanIce::stInService;
	p->siteName			= siteName;
	p->mountedPath		= pathName;
	ZQTianShan::Util::updatePropertyData( p->properties, SYS_PROP(appName), appinfo.name );
	ZQTianShan::Util::updatePropertyData( p->properties, SYS_PROP(appEndpoint), appinfo.endpoint );
	ZQTianShan::Util::updatePropertyData( p->properties, SYS_PROP(stampCommitted), "" );
	ZQTianShan::Util::updatePropertyData( p->properties, SYS_PROP(stampStopped), "");
	ZQTianShan::Util::updatePropertyData( p->properties, SYS_PROP(allocateCost), "");
	char timebuf[32]; ZQ::common::TimeUtil::TimeToUTC( ZQTianShan::now(), timebuf, sizeof(timebuf));
	ZQTianShan::Util::updatePropertyData( p->properties, SYS_PROP(stampResolved), timebuf );
	ZQTianShan::Util::updatePropertyData( p->properties, SYS_PROP(bandwidth), requestBW );

	TianShanIce::ValueMap sessPd = sess->getPrivateData();
	std::string tmp;
	ZQTianShan::Util::getValueMapDataWithDefault( sessPd, "ClientRequest#clientSessionId", "", tmp );
	ZQTianShan::Util::updatePropertyData( p->properties, SYS_PROP(clientSessionId), tmp );

	ZQTianShan::Util::getValueMapDataWithDefault( sessPd, "ClientRequest#orginalUrl", "", tmp);
	ZQTianShan::Util::updatePropertyData( p->properties, SYS_PROP(orginalUrl), tmp );

	ZQTianShan::Util::getValueMapDataWithDefault( sessPd, "ClientRequest#clientAddress", "", tmp );
	ZQTianShan::Util::updatePropertyData( p->properties, SYS_PROP(clientAddress), tmp );

	p->SRMSess = sess;

	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SiteAdmin, "resolvePurchase() add the new live txn with Id [%s] into DB"),	sessId.c_str());

	mDb.addTxn( p );

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(SiteAdmin, "site[%s] created a LiveTxn for sess[%s]: app[%s] %lldbps, "
		"usage: BW[%lld/%lld]Kbps sessions[%d/%d]"),
		siteName.c_str(), sessId.c_str(), appinfo.endpoint.c_str(), requestBW, 
		resUsage._usedBandwidth, resUsage._maxBandwidth, resUsage._usedSessions, resUsage._maxSessions);
}

::TianShanIce::Application::PurchasePrx SiteAdminImpl::resolvePurchase(const ::TianShanIce::SRM::SessionPrx& sess, const ::Ice::Current& c)
{
	std::string sessId = sess->getId();	
	::TianShanIce::SRM::ResourceMap resMap = sess->getReources();
	std::string requestUrl;
	ZQTianShan::Util::getResourceDataWithDefault( resMap, ::TianShanIce::SRM::rtURI, "uri", "", requestUrl );
	if( requestUrl.empty() )
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(MLOG,EXPFMT(SiteAdmin,301, "resolvePurchase() sess[%s] no valid rtURI has been specified"),sessId.c_str() );
	
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "resolvePurchase() sess[%s] resolve purchase for %s"), sessId.c_str(), requestUrl.c_str());

	//check url for site and path
	std::string siteName,pathName;
	checkUrl( sessId, requestUrl, siteName, pathName );
	
	//apply site restriction
	applySiteRestrcitionOnSession( sessId, siteName, pathName, sess );

	TianShanIce::Site::AppInfo appinfo;
	TianShanIce::Application::AppServicePrx appService = getApplication( sessId, siteName, pathName,  sess, appinfo );
	assert( appService );

	TianShanIce::Application::PurchasePrx purchase = createPurchase(sessId, siteName, sess, appService );

	UsedSiteResource siteResUsage;
	int64 requestBW = applyBWRestrcition( purchase, sess, sessId, siteName, siteResUsage);
	
	createLiveTxn(sessId, siteName, pathName, requestBW, appinfo, siteResUsage, sess );

	return purchase;
}

bool SiteAdminImpl::updateSite(const ::std::string& name, const ::std::string& desc, const ::Ice::Current& c)
{
	TianShanIce::Site::VirtualSite info;
	info.name	= name;
	info.desc	= desc;

	Lock sync(*this);//to avoid multi site added concurrently
	try
	{
		TianShanIce::Site::CVirtualSitePrx prx = mDb.openCVirtualSite( name );
		if( !prx )
		{
			CVirtualSiteImpl* pNewSite = new CVirtualSiteImpl(mEnv,mDb);
			pNewSite->updateVsInfo(info);
			mDb.addVirtualSite( pNewSite );
		}
		else
		{			
			prx->updateVsInfo( info );
		}

		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminImpl, "updateSite() update site with name[%s] desc[%s]"), name.c_str(), desc.c_str() );
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminImpl, "updateSite() failed to update site[%s] information due to:%s"),
			name.c_str(), ex.ice_name().c_str() );
		return false;
	}
	return true;
}

bool SiteAdminImpl::removeSite(const ::std::string& name, const ::Ice::Current& c)
{
	///remove site
	mDb.removeVirtualSite( name );
	
	///remove app mount relative to this site
	TianShanIce::Site::AppMounts mounts = mDb.listAppMountsbySite(name);
	TianShanIce::Site::AppMounts::iterator it = mounts.begin();
	for( ; it != mounts.end() ; it ++ )
	{
		try
		{
			(*it)->destroy();
		}
		catch( const Ice::Exception& ){}
	}
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminImpl, "removeSite site[%s] removed"),name.c_str());
	return true;
}

bool SiteAdminImpl::setSiteProperties(const ::std::string& siteName, const ::TianShanIce::Properties& props, const ::Ice::Current& c)
{	
	TianShanIce::Site::CVirtualSitePrx sitePrx = mDb.openCVirtualSite( siteName );
	if( !sitePrx)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SiteAdminImpl, "setSiteProperties() no site is found according to name[%s]"),siteName.c_str() );
		return false;
	}
	sitePrx->updateProps( props );
	return true;
}

::TianShanIce::Properties SiteAdminImpl::getSiteProperties(const ::std::string& siteName, const ::Ice::Current& c) const
{
	TianShanIce::Site::CVirtualSitePrx sitePrx = mDb.openCVirtualSite( siteName );
	if( !sitePrx)
	{		
		::ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (MLOG, EXPFMT(SiteAdmin, 401, "getSiteProperties(%s) site not found"), siteName.c_str());
	}
	return sitePrx->getProps();
}

::TianShanIce::Site::AppInfos SiteAdminImpl::listApplications(const ::Ice::Current& c) const
{
	return mDb.listAppInfos();
}

bool SiteAdminImpl::updateApplication(const ::std::string& name, const ::std::string& endpoint, const ::std::string& desc, const ::Ice::Current& c)
{
	TianShanIce::Site::AppInfo info;
	info.name		= name;
	info.endpoint	= endpoint;
	info.desc		= desc;

	Lock sync(*this);
	TianShanIce::Site::CAppInfoPrx appPrx = mDb.openCAppInfo( name );
	if( !appPrx)
	{
		TianShanIce::Site::CAppInfoPtr p = new CAppInfoImpl(mEnv,mDb);
		p->updateAppInfo(info);
		mDb.addAppInfo(p);
	}
	else
	{
		appPrx->updateAppInfo( info );
	}
	return true;
}

bool SiteAdminImpl::removeApplication(const ::std::string& name, const ::Ice::Current& c)
{
	mDb.removeAppInfo(name);
	TianShanIce::Site::AppMounts mounts = mDb.listAppMountsByApp(name);
	TianShanIce::Site::AppMounts::iterator it = mounts.begin();
	for( ; it != mounts.end() ; it ++ )
	{
		TianShanIce::Site::AppMountPrx mountPrx = *it;
		try
		{
			mountPrx->destroy();
		}
		catch( const Ice::Exception& )
		{
		}
	}
	return true;
}

::TianShanIce::Site::AppMounts SiteAdminImpl::listMounts(const ::std::string& siteName, const ::Ice::Current& c) const
{
	return mDb.listAppMountsbySite( siteName );
}

::TianShanIce::Site::AppMountPrx SiteAdminImpl::mountApplication(const ::std::string& siteName, 
																 const ::std::string& mountPath, 
																 const ::std::string& appName, const ::Ice::Current& c)
{
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SiteAdminImpl, "mountApplication() entering: site[%s] path[%s] app[%s]"),siteName.c_str(), mountPath.c_str(), appName.c_str() );
	if( !mDb.openCAppInfo(appName) || !mDb.openCVirtualSite(siteName))
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminImpl, "mountApplication() failed to find app or site according to: site[%s], app[%s]"),
			siteName.c_str(), appName.c_str() );
		return NULL;
	}

	Lock sync(*this);
	//make sure no same mount exist
	TianShanIce::Site::AppMounts mounts = mDb.listAppMountsbySite( siteName );
	TianShanIce::Site::AppMounts::const_iterator itMount = mounts.begin();
	for ( ; itMount != mounts.end() ; itMount++ )
	{
		try
		{
			if( (*itMount)->getMountedPath() == mountPath )
			{
				if( (*itMount)->getAppName() != appName) // remove the old mountage
				{
					MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminImpl, "mountApplication() find mount between site[%s] app[%s] with path[%s], destoy it"),
						siteName.c_str(), appName.c_str(), mountPath.c_str());
					(*itMount)->destroy();
				}
				else
					break;
				
			}
		}
		catch( const Ice::Exception& )
		{
		}
	}
	if ( itMount == mounts.end())
	{
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(SiteAdminImpl, "mountApplication() add a new mount site[%s] app[%s] path[%s]"),
			siteName.c_str(), appName.c_str(), mountPath.c_str() );
		TianShanIce::Site::AppMountPtr pMount = new AppMountImpl(mEnv,mDb);
		pMount->ident.name	= IceUtil::generateUUID();//ignore ident.category, let database manager class to do it
		pMount->appName		= appName;
		pMount->siteName	= siteName;
		pMount->mountedPath	= mountPath;
		mDb.addAppMount(pMount);
		return mDb.openAppMount( pMount->ident.name );
	}
	else
	{
		return *itMount;
	}	
}

bool SiteAdminImpl::unmountApplication(const ::std::string& site, const ::std::string& mountPath, const ::Ice::Current& c)
{
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SiteAdminImpl, "unmountApplication() site[%s] path[%s]"),site.c_str(), mountPath.c_str() );
	TianShanIce::Site::AppMounts mounts = mDb.listAppMountsbySite( site );
	TianShanIce::Site::AppMounts::iterator itMount = mounts.begin();
	bool bOK = false;
	for( ; itMount != mounts.end() ; itMount++ )
	{
		TianShanIce::Site::AppMountPrx mountPrx = *itMount;
		if( mountPrx && mountPrx->getMountedPath( ) == mountPath )
		{
			try
			{
				mountPrx->destroy();
				bOK = true;
			}
			catch( const Ice::Exception& ){}
		}
	}
	return true;
}

::TianShanIce::Site::AppInfo SiteAdminImpl::findApplication(const ::std::string& siteName, const ::std::string& mountedPath, const ::Ice::Current& c) const
{
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SiteAdminImpl, "findApplication() site[%s] path[%s]"),siteName.c_str(), mountedPath.c_str());
	TianShanIce::Site::AppMounts mounts = mDb.listAppMountsbySite( siteName );
	TianShanIce::Site::AppMounts::iterator itMount = mounts.begin();
	for ( ; itMount != mounts.end() ; itMount++ )
	{
		TianShanIce::Site::AppMountPrx mountPrx = *itMount;
		try
		{
			if( mountPrx && mountPrx->getMountedPath( ) == mountedPath )
			{
				break;
			}
		}
		catch( const Ice::Exception& ){}
	}
	if( itMount == mounts.end() )
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError> (MLOG, EXPFMT(SiteAdminImpl, 601, "findApplication() failed to address the mount: [%s/%s]"), siteName.c_str(), mountedPath.c_str());
	}
	TianShanIce::Site::AppMountPrx mountPrx = *itMount;
	std::string appName = mountPrx->getAppName();
	TianShanIce::Site::CAppInfoPrx appPrx = mDb.openCAppInfo( appName );
	if( !appPrx)
	{
		::ZQTianShan::_IceThrow <TianShanIce::ServerError> (MLOG, EXPFMT(SiteAdminImpl, 602, "findApplication() failed to find application info: [%s]"), appName.c_str() );
	}
	return appPrx->getAppInfo();
}
void SiteAdminImpl::sendTxnInServiceMsg( const ::std::string& sessId, ::TianShanIce::State state, const ::TianShanIce::SRM::SessionPrx& sess, const ::TianShanIce::Properties& props)
{
#ifdef _WITH_EVENTSENDER_
	//post event to eventSender
	MLOG(ZQ::common::Log::L_DEBUG,SERVFMT(SiteAdmin, "LiveTxn is in stInService state,post event to eventSender"));
	MSGSTRUCT msg;
	TianShanIce::Properties prop=props;
	msg.category = "Session";
	msg.eventName= "SessionInService";
	msg.id = 3001;
	char buf[256];
	memset(buf,0,sizeof(buf));
	msg.timestamp = SystemTimeToUTC(ZQTianShan::now(),buf,sizeof(buf)-1);
	memset(buf,0,sizeof(buf));
	msg.sourceNetId = gethostname(buf,sizeof(buf)-1)==0?buf:"";

	msg.property["sessionId"]			= sessid;
	msg.property["contentStoreNetId"]	= prop[SYS_PROP(contentStore)];
	msg.property["streamerNetId"]		= prop[SYS_PROP(Streamer)];
	msg.property["serviceGroupId"]		= prop[SYS_PROP(serviceGroupID)];
	msg.property["downstreamBandwidth"]	= prop[SYS_PROP(bandwidth)];
	msg.property["streamId"]			= prop[SYS_PROP(streamID)];
	//msg.property["stampLocal"]		= ZQTianShan::Site::FormatLocalTime(buf,sizeof(buf));
	g_pEventSinkMan->PostEvent(msg);
#endif//_WITH_EVENTSENDER_
}
void SiteAdminImpl::sendTxnOutOfServiceMsg( const ::std::string& sessId, ::TianShanIce::State state, const ::TianShanIce::SRM::SessionPrx& sess, const ::TianShanIce::Properties& props )
{
#ifdef _WITH_EVENTSENDER_
	//post event to eventSender
	envlog(ZQ::common::Log::L_DEBUG,SERVFMT(SiteAdmin, "LiveTxn is in stInService state,post event to eventSender"));
	TianShanIce::Properties prop=props;

	MSGSTRUCT msg;			
	msg.category = "Session";
	msg.eventName= "SessionOutOfService";			
	char buf[256];
	memset(buf,0,sizeof(buf));

	msg.sourceNetId = gethostname(buf,sizeof(buf)-1)==0?buf:"";

	msg.id = 4001;
	memset(buf,0,sizeof(buf));
	msg.timestamp = SystemTimeToUTC(ZQTianShan::now(),buf,sizeof(buf)-1);
	msg.property["sessionId"]			= sessid;
	msg.property["teardownReason"]		= prop[SYS_PROP(teardownReason)];
	msg.property["terminateReason"]		= prop[SYS_PROP(terminateReason)];			
	//msg.property["stampLocal"]			= FormatLocalTime(buf,sizeof(buf));

	g_pEventSinkMan->PostEvent(msg);

#endif //_WITH_EVENTSENDER_
}
void SiteAdminImpl::commitStateChange( const ::std::string& sessId, ::TianShanIce::State state, const ::TianShanIce::SRM::SessionPrx& sess, const ::TianShanIce::Properties& props, const ::Ice::Current& c)
{
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminImpl, "commitStateChange() sess[%s] state[%s]"), sessId.c_str(), ZQTianShan::ObjStateStr(state));
	TianShanIce::Site::LiveTxnPrx liveTxn = mDb.openTxn( sessId );
	if( !liveTxn )
	{
		::ZQTianShan::_IceThrow <TianShanIce::Site::NoSuchTxn> (MLOG, EXPFMT(SiteAdminImpl, 701, "no such txn %s in the live txn database"), sessId.c_str());
	}
	try
	{
		liveTxn->setState( state );
		for (::TianShanIce::Properties::const_iterator it = props.begin(); it != props.end(); it++)
		{
			std::string key = it->first;
			if (key.empty())
				continue;

			if ( 0 != key.compare(0, strlen(SYS_PROP_PREFIX), SYS_PROP_PREFIX))
				key = ::std::string(SYS_PROP_PREFIX) + key;
			liveTxn->setProperty( key, it->second );
		}
		 
		// trace the state changed time
		{
			char tmBuf[64] = {0};
			const char* nowUTC = ZQTianShan::TimeToUTC(ZQTianShan::now(), tmBuf, sizeof(tmBuf));
			if(nowUTC)
			{
				switch(state)
				{
				case TianShanIce::stProvisioned:
					liveTxn->setProperty(SYS_PROP(ProvisionedAt), nowUTC);
					break;
				case TianShanIce::stInService:
					liveTxn->setProperty(SYS_PROP(InServiceAt), nowUTC);
					break;
				case TianShanIce::stOutOfService:
					liveTxn->setProperty(SYS_PROP(OutOfServiceAt), nowUTC);
					break;
				default:
					break;
				}
			}
		}

		if ( state ==  TianShanIce::stInService ) 
		{
			sendTxnInServiceMsg(sessId, state, sess, props );
		}
		else if ( state == TianShanIce::stOutOfService ) 
		{
			// monitor the session
			MLOG(ZQ::common::Log::L_INFO, CLOGFMT(SiteAdmin, "LiveTxn[%s] is at state OutOfService. Watch it in 1 hr."), sessId.c_str());			
			//mWatchDog.WatchMe(sessId, 3600 * 1000 ); //why this ? I don't understand, we already monitor this txn  in postYTD

			sendTxnOutOfServiceMsg(sessId, state, sess, props );
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdmin, "LiveTxn[%s] is in stOutOfService State,PostYTD"), sessId.c_str());
			::Ice::Current c;
			postYTD( sessId, c );
		}

	}
	catch(const ::TianShanIce::ServerError& ex)
	{
		ex.ice_throw();
	}
	catch(const ::Ice::Exception& ex)
	{
		::ZQTianShan::_IceThrow <TianShanIce::ServerError> (MLOG, EXPFMT(SiteAdmin, 702, "commitStateChange(id=%s) caught exception:%s"), sessId.c_str(), ex.ice_name().c_str());
	}
}

void SiteAdminImpl::listLiveTxn_async(const ::TianShanIce::Site::AMD_TxnService_listLiveTxnPtr& amdCB, const ::std::string& siteName, 
									  const ::std::string& appMount, const ::TianShanIce::StrValues& paramNames,
									  const ::std::string &startId, int maxCount, const ::Ice::Current& c) const
{
	ListTxnCommand* p = new ListTxnCommand(mEnv,mDb,amdCB);
	p->prepare( siteName, appMount, paramNames, startId, maxCount );
	p->start();
}

void SiteAdminImpl::trace_async(const ::TianShanIce::Site::AMD_TxnService_tracePtr& amdCB, const ::std::string& sessId, const ::std::string& category, const ::std::string& eventCode, const ::std::string& eventMsg, const ::Ice::Current& c)
{
	SaveEventCommand * p = new SaveEventCommand( mEnv, mDb, amdCB );
	p->prepare( sessId, category, eventCode, eventMsg );
	p->start();
}

void SiteAdminImpl::saveTxnDataLog( const std::string& sessId, TianShanIce::Site::LiveTxnPrx txn )
{
	if( mEnv.getConfig().lTxnDataEnabled <= 0 )
		return;
	try
	{
		char sqlBuff[1024];
		sqlBuff[sizeof(sqlBuff) - 1] = '\0';
		// insert the base info
		snprintf(sqlBuff, sizeof(sqlBuff) - 1, "INSERT INTO Sessions(Session, Site, Path) VALUES(\'%s\', \'%s\', \'%s\')", 
			txn->getSessId().c_str(), txn->getSitename().c_str(), txn->getPath().c_str());
		if( !mDb.updateTxnDataLog( sqlBuff ) )
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminImpl, "saveTxnDataLog() failed to save base info for sesson[%s], sql[%s]"),
				sessId.c_str(), sqlBuff );
		}

		
		std::ostringstream cmdBuf;
		TianShanIce::Properties txnProps = txn->getProperties();
		cmdBuf
			<< "UPDATE Sessions SET "
			<< "Storage='" << txnProps[SYS_PROP(contentStore)]
			<< "',Streamer='" << txnProps[SYS_PROP(Streamer)]
			<< "',ServiceGroup='" << txnProps[SYS_PROP(serviceGroupID)]
			<< "',Bandwidth='" << txnProps[SYS_PROP(bandwidth)]
			<< "',Stream='" << txnProps[SYS_PROP(streamID)]
			<< "',ProvisionedAt='" << txnProps[SYS_PROP(ProvisionedAt)]
			<< "',InServiceAt='" << txnProps[SYS_PROP(InServiceAt)]
			<< "',OutOfServiceAt='" << txnProps[SYS_PROP(OutOfServiceAt)]
			<< "',TeardownReason='" << txnProps[SYS_PROP(teardownReason)]
			<< "',TerminateReason='" << txnProps[SYS_PROP(terminateReason)]
			<< "',ClientSessionId='" << txnProps[SYS_PROP(clientSessionId)]
			<< "',ClientAddress='" << txnProps[SYS_PROP(clientAddress)]
			<< "',OrginalUrl='" << txnProps[SYS_PROP(orginalUrl)]
			<< "' WHERE Session='" << txn->getSessId() << "'";
		std::string sqlCmd = cmdBuf.str();
		if( !mDb.updateTxnDataLog( sqlCmd.c_str() ) )
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminImpl, "saveTxnDataLog() failed to save properries for session[%s], sql[%s]"),
				sessId.c_str(), sqlCmd.c_str() );
		}


		std::vector<TianShanIce::Site::TxnEventPrx> eventPrxs = mDb.openEventByTxnId( sessId );

		std::vector<TianShanIce::Site::TxnEventPrx> ::const_iterator itEvent = eventPrxs.begin();		
		for ( ;itEvent != eventPrxs.end() ; itEvent++ )
		{
			::TianShanIce::Site::TxnEventPrx evt = *itEvent;
			TianShanIce::Properties props;
			TianShanIce::StrValues params;
			params.push_back("stampUTC");
			params.push_back("category");
			params.push_back("eventCode");
			params.push_back("eventMsg");
			props = evt->getEventInfo(params);
			snprintf(sqlBuff, sizeof(sqlBuff) - 1, "insert into Events(Session, stampUTC, category, eventCode, eventMsg) values(\'%s\', \'%s\', \'%s\', \'%s\', \'%s\')", 
				sessId.c_str(), props["stampUTC"].c_str(), props["category"].c_str(), props["eventCode"].c_str(), props["eventMsg"].c_str());			
			if( !mDb.updateTxnEventLog( sqlBuff ) )
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminImpl, "saveTxnDataLog() failed to update event data log for sess[%s], sql[%s]"),
					sessId.c_str(), sqlBuff );
			}
		}
	}
#ifdef ZQ_OS_MSWIN
	catch (const ZQ::common::MdbLogError& ex)
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(SiteAdmin, "Write Txn[%s] data to MdbLog caught %s"),sessId.c_str(), ex.getString());
	}
#endif
	catch (const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(SiteAdmin, "Write Txn[%s] data to MdbLog caught %s"),sessId.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(SiteAdmin, "Write Txn[%s] data to MdbLog caught unexpect error"),sessId.c_str() );
	}
}
void SiteAdminImpl::postYTD(const ::std::string& sessId, const ::Ice::Current& c)
{
	TianShanIce::Site::LiveTxnPrx txn = mDb.openTxn( sessId );
	if( !txn )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(SiteAdminImpl, "postYTD() session[%s] is gone"),sessId.c_str() );
		return;
	}

	saveTxnDataLog( sessId, txn );
	
	try
	{		
		std::string siteName = txn->getSitename();
		TianShanIce::Properties txnProps = txn->getProperties();
		txn->destroy();
		std::string txnState;
		ZQTianShan::Util::getPropertyDataWithDefault( txnProps, SYS_PROP(LiveTxnOutOfService), "", txnState);
		if( txnState == "yes")
		{
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(SiteAdminImpl, "postYTD() txn[%s] is gone"),sessId.c_str() );
			return;
		}
		
		//为何原始版本再这里要判断两次liveTxn的状态?
		int64 usedBW = -1;
		ZQTianShan::Util::getPropertyDataWithDefault( txnProps, SYS_PROP(bandwidth),-1, usedBW);
		if( usedBW > 0 )
		{
			removeSiteResourceUsage( sessId, siteName, usedBW );
		}
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminImpl, "postYTD() txn[%s] destroyed"), sessId.c_str() );
	}
	catch( const Ice::Exception& ex )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminImpl, "postYTD() caught [%s] while update site resource usage"),
			ex.ice_name().c_str() );
	}	
}

bool SiteAdminImpl::addSiteResourceUsage( const std::string& sessId, const std::string& siteName, int64 bandwidth )
{
	ZQ::common::MutexGuard gd(mUsedResLocker);
	
	USEDRESMAP::iterator it = mUsedResMap.find(siteName);
	if( it == mUsedResMap.end() )
	{
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SiteAdminImpl, "addSiteResourceUsage() failed to find site[%s], sess[%s]"),
			siteName.c_str(), sessId.c_str() );
		return false;
	}

	int requestedKbps = (bandwidth+999)/1000;

	UsedSiteResource& res = it->second;
	if ( (res._maxSessions >0 && res._usedSessions >= res._maxSessions) || (res._maxBandwidth>0 && (res._usedBandwidth +requestedKbps) > res._maxBandwidth))
	{
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SiteAdminImpl, "addSiteResourceUsage() site[%s] can't host session[%s]: requestBW[%lld]bps, BW[%lld/%lld]Kbps sessions[%d/%d]"),
			siteName.c_str(), sessId.c_str(), bandwidth, 
			res._usedBandwidth, res._maxBandwidth, res._usedSessions, res._maxSessions);
		return false;
	}

	res._usedSessions ++;
	res._usedBandwidth += requestedKbps;
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(SiteAdminImpl, "addSiteResourceUsage() site[%s] accepted session[%s]: requestBW[%lld]bps, BW[%lld/%lld]Kbps sessions[%d/%d]"),
		 siteName.c_str(), sessId.c_str(), bandwidth, 
		 res._usedBandwidth,res._maxBandwidth, res._usedSessions, res._maxSessions);
	return true;
}

void SiteAdminImpl::removeSiteResourceUsage( const std::string& sessId, const std::string& siteName, int64 bandwidth )
{
	ZQ::common::MutexGuard gd(mUsedResLocker);

	USEDRESMAP::iterator it = mUsedResMap.find(siteName);
	if( it == mUsedResMap.end() )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(SiteAdminImpl, "removeSiteResourceUsage() failed to find site[%s]"), siteName.c_str() );
		return ;
	}
	UsedSiteResource& res = it->second;
	
	int requestedKbps = (bandwidth+999)/1000;

	res._usedSessions --;
	res._usedBandwidth -= requestedKbps;
	
	res._usedBandwidth	= MAX( res._usedBandwidth, 0 );
	res._usedSessions	= MAX( res._usedSessions, 0 );

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(SiteAdminImpl, "removeSiteResourceUsage() site[%s] removed session[%s] by requestBW[%lld]bps "
		"usage: BW[%lld/%lld]Kbps sessions[%d/%d]"),
		siteName.c_str(), sessId.c_str(), bandwidth, 
		res._usedBandwidth, res._maxBandwidth, res._usedSessions, res._maxSessions);
}

void SiteAdminImpl::setUserProperty(const ::std::string& sessId, const ::std::string& key, const ::std::string& value, const ::Ice::Current& c)
{
	if (key.empty())
		return;

	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SiteAdminImpl, "setUserProperty() sess[%s] key[%s] value[%s]"),
		sessId.c_str(), key.c_str(), value.c_str() );
	::TianShanIce::Site::LiveTxnPrx livetxn = mDb.openTxn(sessId);

	if (!livetxn)
		::ZQTianShan::_IceThrow <TianShanIce::Site::NoSuchTxn> (MLOG, EXPFMT(SiteAdmin, 901, "no such txn %s in the live txn database"), sessId.c_str());

	try 
	{
		livetxn->setProperty((0!=key.compare(0, strlen(USER_PROP_PREFIX), USER_PROP_PREFIX)) ?(std::string(USER_PROP_PREFIX) + key) : key, value);
	}
	catch(const ::TianShanIce::ServerError& ex)
	{
		ex.ice_throw();
	}
	catch(const ::Ice::Exception& ex)
	{
		::ZQTianShan::_IceThrow <TianShanIce::ServerError> (MLOG, EXPFMT(SiteAdmin, 902, "setUserProperty(id=%s) caught exception:%s"), sessId.c_str(), ex.ice_name().c_str());
	}
}


void SiteAdminImpl::dumpLiveTxn_async(const ::TianShanIce::Site::AMD_TxnService_dumpLiveTxnPtr& amdCB, const ::std::string& sessId, const ::std::string& beginFormat, const ::std::string& traceFormat, const ::std::string& endFormat, const ::Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: impl here")
}

::TianShanIce::Site::LiveTxnPrx SiteAdminImpl::newTxn(const ::std::string& sessId, ::Ice::Long bandwidth, const ::std::string& siteName, const ::std::string& appName, const ::std::string& mountPath, const ::Ice::Current& c)
{
	ZQTianShan::_IceThrow<TianShanIce::NotImplemented>(MLOG, "SiteAdmin",001, "not implement");
	return NULL;
}

void SiteAdminImpl::updateSiteResourceLimited(const ::std::string& siteName, ::Ice::Long maxBW, ::Ice::Int maxSessions, const ::Ice::Current& c)
{
	TianShanIce::Site::VirtualSite record;
	record.maxDownstreamBwKbps	= maxBW * 1000 ;//this is a kbps value
	record.maxSessions			= maxSessions;

	TianShanIce::Site::CVirtualSitePrx sitePrx = mDb.openCVirtualSite( siteName );
	if( !sitePrx )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminImpl, "updateSiteResourceLimited()  site[%s] not found"), siteName.c_str() );
		return;
	}
	sitePrx->updateLimitation(maxBW,maxSessions);
	{
		ZQ::common::MutexGuard gd(mUsedResLocker);
		USEDRESMAP::iterator it = mUsedResMap.find(siteName);
		if( it == mUsedResMap.end() )
		{
			UsedSiteResource res;
			res._maxBandwidth	= maxBW;
			res._maxSessions	= maxSessions;
			res._usedBandwidth	= 0;
			res._usedSessions	= 0;
			mUsedResMap.insert( USEDRESMAP::value_type(siteName,res));
		}
		else
		{
			it->second._maxBandwidth	= maxBW;
			it->second._maxSessions		= maxSessions;
		}
	}
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminImpl, "updateSiteResourceLimited() site[%s] resource lmitation, maxBW[%lld] maxSession[%d]"),
		siteName.c_str(), maxBW, maxSessions );

	TianShanIce::Properties siteProps;
	char szBuf[256];
	memset(szBuf,0,sizeof(szBuf));
	sprintf(szBuf,FMT64,maxBW);
	siteProps[SYS_PROP(maxDownstreamBwKbps)] = szBuf;

	memset(szBuf,0,sizeof(szBuf));
	sprintf(szBuf, "%d",maxSessions);
	siteProps[SYS_PROP(maxSessions)] =szBuf;

	setSiteProperties(siteName,siteProps,c);
}

void SiteAdminImpl::restrictSiteResources(const ::std::string& siteName, const ::TianShanIce::SRM::ResourceMap& resources, const ::Ice::Current& c)
{
	TianShanIce::Site::CVirtualSitePrx sitePrx = mDb.openCVirtualSite( siteName);
	if( !sitePrx )
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError> (MLOG, EXPFMT(SiteAdmin, 1002, "restrictSiteResources() failed to find site[%s]"), siteName.c_str() );
	}
	
	TianShanIce::Site::VirtualSite record = sitePrx->getVsInfo();

	static const ::TianShanIce::SRM::ResourceType acceptableTypes[] = { ::TianShanIce::SRM::rtStorage, ::TianShanIce::SRM::rtStreamer, ::TianShanIce::SRM::rtServiceGroup };

	for (size_t i =0; i< sizeof(acceptableTypes) / sizeof(::TianShanIce::SRM::ResourceType); i++)
	{
		const ::TianShanIce::SRM::ResourceType rt = acceptableTypes[i];
		::TianShanIce::SRM::ResourceMap::const_iterator it = resources.find(rt);
		if (resources.end() != it)
		{
			record.restrictedResources[rt] = it->second;
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "restrictSiteResources(%s) update resource[%s]"), siteName.c_str(), ZQTianShan::ResourceTypeStr(rt));
		}
	}
	sitePrx->updateVsInfo( record );
}

::TianShanIce::SRM::ResourceMap SiteAdminImpl::getSiteResourceRestricutions(const ::std::string& siteName, const ::Ice::Current& c) const
{
	TianShanIce::Site::CVirtualSitePrx sitePrx = mDb.openCVirtualSite( siteName);
	if( !sitePrx )
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError> (MLOG, EXPFMT(SiteAdmin, 1002, "getSiteResourceRestricutions() failed to find site[%s]"), siteName.c_str() );
	}
	return sitePrx->getVsInfo().restrictedResources;

}
