
#include "ZQ_common_conf.h"
#include "TianShanDefines.h"
#include "SiteAdminDatabase.h"
#include "SiteAdminEnv.h"

#include <TianShanIceHelper.h>


#if ICE_INT_VERSION / 100 >= 303
#define CREATEEVICTORAPI Freeze::createBackgroundSaveEvictor
#else
#define CREATEEVICTORAPI Freeze::createEvictor
#endif//ICE_INT_VERSION / 100 >= 303

#define MOUNT_DB_PATH	"Sites"
#define MOUNT_LOCATOR	"AppMount"
#define APPINFO_LOCATOR	"AppInfo"
#define VSITE_LOCATOR	"VirtualSite"

#define TXN_DB_PATH		"TxnLive"
#define TXN_LOCATOR		"Txn"
#define TXN_DB_CACHESIZE_KB (160*1024) //160MB

#define EVENT_DB_PATH	"Event"
#define EVENT_LOCATOR	"Event"


#define DEFUALT_SITENAME "."

std::string stringtolower( const std::string& str )
{
	std::string strout = str;
	std::transform( strout.begin() , strout.end() , strout.begin(), tolower);
	return strout;
}

//////////////////////////////////////////////////////////////////////////
///SiteAdminDb
SiteAdminDb::SiteAdminDb( SiteAdminEnv& env )
:mEnv(env)
{
	mbEnableTxn		= true;
	mbEnableEvent	= false;
	mbEnableTxnMdbData	= false;
}
SiteAdminDb::~SiteAdminDb( )
{
}
void SiteAdminDb::closeDb()
{
	try
	{
		mObjAdapter->deactivate();
	}
	catch (...)
	{		
	}
	
	mAppToMount		= NULL;
	mSiteToMount	= NULL;
	mAppMount		= NULL;
	mAppInfo		= NULL;
	mSiteInfo		= NULL;

	mSiteToTxn		= NULL;
	mMountToTxn		= NULL;
	mLiveTxn		= NULL;

	mTxnToEvent		= NULL;
	mTxnEvent		= NULL;

	mDbConnection	= NULL;

	mObjAdapter     = NULL;
#ifdef ZQ_OS_MSWIN
	if(mMdbLog)
	{		
		delete mMdbLog;
		mMdbLog = NULL;
	}
#endif
}

void SiteAdminDb::updateDbProperty( ZQADAPTER_DECLTYPE obj, const std::string& key , const std::string& value )
{
	Ice::CommunicatorPtr ic = obj->getCommunicator();
	Ice::PropertiesPtr prop = ic->getProperties();
	prop->setProperty( key , value );
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminDb,"update DB property: [%s] ==> [%s]"), key.c_str() , value.c_str() );
}

bool SiteAdminDb::openDb(const std::string &dbPath, const std::string& runtimeDbPath ,ZQADAPTER_DECLTYPE objAdpater)
{
	mObjAdapter = objAdpater;
	if(dbPath.empty() )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminDb,"openDb() invalid dbpath,reject"));
		return false;
	}	
	if( !openMountDB( dbPath , objAdpater ) )
	{
		return false;
	}
	if( mbEnableTxn && !openTxnDB( runtimeDbPath , objAdpater ) )
	{
		return false;		
	}
	if( mbEnableEvent && !openEventDB( runtimeDbPath , objAdpater ) )
	{
		return false;
	}
	if( mbEnableTxnMdbData && !initTxnMDB() )
	{
		return false;
	}
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminDb,"openDb() db openned at [%s]"), dbPath.c_str() );
	return true;
}

bool SiteAdminDb::initTxnMDB( )
{
#ifdef ZQ_OS_MSWIN
	if( !mMdbLog )
	{
		mMdbLog = new ZQ::common::MdbLog();
	}
	try
	{
		std::vector<std::string> dbScheme;
		dbScheme.push_back("create table Sessions("
			"Session char(50) primary key NOT NULL"
			", Site char(50)"
			", Path char(50)"
			", Storage char(50)"
			", Streamer char(50)"
			", ServiceGroup char(50)"
			", Bandwidth char(30)"
			", Stream char(50)"
			", ProvisionedAt char(30)"
			", InServiceAt char(30)"
			", OutOfServiceAt char(30)"
			", TeardownReason TEXT"
			", TerminateReason TEXT"
			", ClientSessionId char(50)"
			", ClientAddress char(50)"
			", OrginalUrl TEXT"
			")");
		dbScheme.push_back("create table Events(Session char(50) NOT NULL, stampUTC char(30), category char(30), eventCode char(20), eventMsg char(200))");
		mMdbLog->initialize( mEnv.getConfig().szTxnDataDest,
			mEnv.getConfig().szTxnDataTemplate,
			dbScheme, 
			(unsigned int)mEnv.getConfig().lTxnDataSize,
			(unsigned int)mEnv.getConfig().lTxnDataNumber );
	}
	catch (const ZQ::common::MdbLogError& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SiteAdminDb, "failed to initialize MdbLog dest[%s] templ[%s]: %s"),
			mEnv.getConfig().szTxnDataDest, mEnv.getConfig().szTxnDataTemplate, ex.getString());
		return false;
	}
#endif
	return true;	
}


std::string makeFreezeEnvPropKey( const std::string& envDir , const std::string& propKey )
{
	static std::string prefix = "Freeze.DbEnv.";
	return prefix + envDir + "." + propKey;
}

std::string makeFreezeEvictorPropKey( const std::string& envDir , const std::string& dbName, const std::string& propKey )
{
	static std::string prefix= "Freeze.Evictor.";
	return prefix + envDir + "." + dbName + "." + propKey;
}

bool SiteAdminDb::openMountDB( const std::string &topPath, ZQADAPTER_DECLTYPE objAdpater )
{
	std::string dbPath = ZQTianShan::Util::fsConcatPath( topPath , MOUNT_DB_PATH ); 
	if( !ZQTianShan::Util::fsCreatePath(dbPath) )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminDb,"openMountDB() failed to open db at [%s]"), dbPath.c_str() );
		return false;
	}
	try
	{	
		//create freeze connection	
		mDbConnection	= Freeze::createConnection( objAdpater->getCommunicator(), dbPath );
		//create app dictionary 
		// 	mAppDict		= new TianShanIce::Site::AppDict( mDbConnection , "Apps");
		// 	mSiteDict		= new TianShanIce::Site::SiteDict( mDbConnection , "Sites");

		mAppInfo		= CREATEEVICTORAPI( objAdpater , dbPath , APPINFO_LOCATOR , 0 );
		objAdpater->addServantLocator( mAppInfo , APPINFO_LOCATOR );
		mAppInfo->setSize( 100 );

		mSiteInfo		= CREATEEVICTORAPI( objAdpater , dbPath , VSITE_LOCATOR , 0 );
		objAdpater->addServantLocator( mSiteInfo , VSITE_LOCATOR );
		mSiteInfo->setSize( 100 );

		///create evictor with 
		mAppToMount		= new TianShanIce::Site::AppToMount("AppToMount");
		mSiteToMount	= new TianShanIce::Site::SiteToMount("SiteToMount");

		std::vector<Freeze::IndexPtr> indices;
		indices.push_back( mAppToMount );
		indices.push_back( mSiteToMount );

		mAppMount = CREATEEVICTORAPI( objAdpater, dbPath , MOUNT_LOCATOR, 0, indices);

		objAdpater->addServantLocator( mAppMount , MOUNT_LOCATOR);

		mAppMount->setSize( (int)mEnv.getConfig().performanceTune.lAppMountEvictorSize );
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminDb,"openMountDB(), Mount DB openned at[%s], set evictor size to[%d]"),
			dbPath.c_str() , mEnv.getConfig().performanceTune.lAppMountEvictorSize );
	}
	catch( const Ice::Exception& ex )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminDb,"openMountDB() failed due to ;%s"),ex.ice_name().c_str());
		return false;
	}
	return true;
}

bool SiteAdminDb::openTxnDB(  const std::string& runtimeDbPath, ZQADAPTER_DECLTYPE objAdpater )
{
	std::string dbPath = ZQTianShan::Util::fsConcatPath( runtimeDbPath , TXN_DB_PATH ); 
	if( !ZQTianShan::createDBFolderEx(MLOG, dbPath, TXN_DB_CACHESIZE_KB))
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminDb,"openTxnDB() failed to open db at [%s]"),dbPath.c_str() );
		return false;
	}
	setProperty( makeFreezeEnvPropKey(dbPath,"CheckpointPeriod") , mEnv.getConfig().performanceTune.strCheckpointPeriod);
	setProperty( makeFreezeEnvPropKey(dbPath,"DbRecoverFatal") , mEnv.getConfig().performanceTune.strDbRecoverFatal);
	setProperty( makeFreezeEvictorPropKey(dbPath,TXN_LOCATOR,"SavePeriod") , mEnv.getConfig().performanceTune.strLivTxnSavePeriod);
	setProperty( makeFreezeEvictorPropKey(dbPath,TXN_LOCATOR,"SaveSizeTrigger") , mEnv.getConfig().performanceTune.strLivTxnSaveSizeTrigger);
	
	try
	{
		mSiteToTxn	= new TianShanIce::Site::SiteToTxn("SiteToTxn");
		mMountToTxn	= new TianShanIce::Site::MountToTxn("MountToTxn");
		std::vector<Freeze::IndexPtr> indices;
		indices.push_back( mSiteToTxn );
		indices.push_back( mMountToTxn );
		mLiveTxn = CREATEEVICTORAPI( objAdpater , dbPath , TXN_LOCATOR , 0 , indices );
		objAdpater->addServantLocator( mLiveTxn , TXN_LOCATOR);
		
		mLiveTxn->setSize( (int)mEnv.getConfig().performanceTune.lTxnEvictorSize );

		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminDb,"openTxnDB() Tnx DB openned at [%s], set evictor size to[%u]"),
			dbPath.c_str(), mEnv.getConfig().performanceTune.lTxnEvictorSize );
	}
	catch( const Ice::Exception& ex )
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SiteAdminDb,"openTxnDB() failed due to : %s"), ex.ice_name().c_str() );
		return false;
	}
	return true;
}
bool SiteAdminDb::openEventDB(const std::string &runtimeDbPath, ZQADAPTER_DECLTYPE objAdpater)
{
	std::string dbPath = ZQTianShan::Util::fsConcatPath( runtimeDbPath , EVENT_DB_PATH ); 
	if( !ZQTianShan::Util::fsCreatePath(dbPath) )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminDb,"openTxnDB() failed to open db at [%s]"),dbPath.c_str() );
		return false;
	}
	try
	{
		mTxnToEvent = new TianShanIce::Site::TxnToEvent("TxnToEvent");
		std::vector<Freeze::IndexPtr> indices;
		indices.push_back( mTxnToEvent );
		mTxnEvent = CREATEEVICTORAPI(objAdpater , dbPath , EVENT_LOCATOR , 0 , indices );
		objAdpater->addServantLocator( mTxnEvent ,EVENT_LOCATOR);

		mTxnEvent->setSize( (int)mEnv.getConfig().performanceTune.lTxnEventEvictorSize );
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminDb,"openEventDB() EVent DB openned at[%s], set evicor size[%u]"),
			dbPath.c_str() , mEnv.getConfig().performanceTune.lTxnEventEvictorSize );
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminDb,"openEventDB() failed due to : %s"),ex.ice_name().c_str() );
		return false;
	}
	return true;
}


TianShanIce::Site::CVirtualSitePrx SiteAdminDb::openCVirtualSite( const std::string& id ) const
{
	Ice::Identity ident;
	ident.name		= stringtolower(id);
	ident.category	= VSITE_LOCATOR;
	TianShanIce::Site::CVirtualSitePrx prx = NULL;
	try
	{
		prx =  TianShanIce::Site::CVirtualSitePrx::checkedCast( mObjAdapter->createProxy(ident) );
	}
	catch( const Ice::Exception& )
	{
		return NULL;
	}
	return prx;
}

SiteAdminDb::VirtualSitePrxs	SiteAdminDb::getVirtualSitePrxs( ) const
{
	VirtualSitePrxs rets;
	::Freeze::EvictorIteratorPtr itPtr = mSiteInfo->getIterator("",1024);	
	while( itPtr->hasNext())
	{
		TianShanIce::Site::CVirtualSitePrx vsPrx =  openCVirtualSite(itPtr->next().name );
		if(vsPrx)
			rets.push_back( vsPrx );
	}
	return rets;
}
::TianShanIce::Site::VirtualSites SiteAdminDb::listSites( ) const
{
	::TianShanIce::Site::VirtualSites rets;
	VirtualSitePrxs prxs = getVirtualSitePrxs();
	VirtualSitePrxs::const_iterator it = prxs.begin();
	for ( ; it != prxs.end() ; it ++ )
	{
		try
		{
			TianShanIce::Site::VirtualSite vs ;
			vs = (*it)->getVsInfo();
			rets.push_back( vs );
		}
		catch(...){}
	}
	return rets;
}

bool SiteAdminDb::addVirtualSite( TianShanIce::Site::CVirtualSitePtr p )
{
	Ice::Identity id ;
	id.name			= stringtolower(p->vsinfo.name);
	p->vsinfo.name	= id.name;
	id.category		= VSITE_LOCATOR;

	try
	{
		mSiteInfo->add( p , id );
		return true;
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminDb,"addVirtualSite() failed to add site[%s] into DB due to:%s"),
			id.name.c_str() , ex.ice_name().c_str());
		return false;
	}	
}

void SiteAdminDb::removeVirtualSite( const std::string& name )
{
	Ice::Identity id;
	id.name		= stringtolower(name);
	id.category	= VSITE_LOCATOR;
	try
	{
		mSiteInfo->remove(id);
	}
	catch( const Ice::Exception& )
	{
	}
}

TianShanIce::Site::CAppInfoPrx SiteAdminDb::openCAppInfo( const std::string& id ) const
{
	Ice::Identity ident;
	ident.name		= id;
	ident.category	= APPINFO_LOCATOR;
	TianShanIce::Site::CAppInfoPrx prx = NULL;
	try
	{
		prx =  TianShanIce::Site::CAppInfoPrx::checkedCast( mObjAdapter->createProxy(ident) );
	}
	catch( const Ice::Exception& )
	{
		return NULL;
	}
	return prx;
}

SiteAdminDb::CAppInfoPrxs SiteAdminDb::getAppInfoPrxs( ) const
{
	CAppInfoPrxs rets;
	::Freeze::EvictorIteratorPtr itPtr = mAppInfo->getIterator("",1024);	
	while( itPtr->hasNext())
	{
		std::string name = itPtr->next().name;
		TianShanIce::Site::CAppInfoPrx prx = openCAppInfo( name );
		if( prx)
			rets.push_back( prx );
	}
	return rets;
}

TianShanIce::Site::AppInfos SiteAdminDb::listAppInfos( ) const
{
	::TianShanIce::Site::AppInfos rets;
	CAppInfoPrxs prxs = getAppInfoPrxs();
	CAppInfoPrxs::const_iterator it = prxs.begin();
	for ( ; it != prxs.end() ; it ++ )
	{
		try
		{
			rets.push_back( (*it)->getAppInfo() );
		}
		catch( ... )
		{
		}
	}
	return rets;
}

bool SiteAdminDb::addAppInfo( TianShanIce::Site::CAppInfoPtr p )
{
	Ice::Identity id;
	id.name		= p->apinfo.name;
	id.category	= APPINFO_LOCATOR;
	try
	{
		mAppInfo->add( p , id );
		return true;
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminDb,"addAppInfo() failed to add app[%s] into DB due to :%s"),
			id.name.c_str() , ex.ice_name().c_str() );
		return false;
	}
}

void SiteAdminDb::removeAppInfo( const std::string& name )
{
	Ice::Identity id;
	id.name		= name;
	id.category	= APPINFO_LOCATOR;
	try
	{
		mAppInfo->remove( id );
	}
	catch( const Ice::Exception& )
	{		
	}
}

TianShanIce::Site::AppMountPrx SiteAdminDb::openAppMount( const std::string& id ) const
{
	Ice::Identity ident;
	ident.name		= id;
	ident.category	= MOUNT_LOCATOR;
	TianShanIce::Site::AppMountPrx prx = NULL;
	try
	{
		prx =  TianShanIce::Site::AppMountPrx::checkedCast( mObjAdapter->createProxy(ident) );
	}
	catch( const Ice::Exception& )
	{
		return NULL;
	}
	return prx;	
}

TianShanIce::Site::AppMounts SiteAdminDb::listAppMountsbySite( const std::string& siteName ) const
{
	TianShanIce::Site::AppMounts rets;
	std::vector<Ice::Identity> ids= mSiteToMount->find( stringtolower(siteName) );
	std::vector<Ice::Identity>::const_iterator itId = ids.begin();
	for( ; itId != ids.end() ; itId++ )
	{
		try
		{
			rets.push_back( openAppMount( itId->name ) );
		}
		catch(...){}
	}
	return rets;	
}
TianShanIce::Site::AppMounts SiteAdminDb::listAppMountsByApp( const std::string& appName ) const
{
	TianShanIce::Site::AppMounts rets;
	std::vector<Ice::Identity> ids= mAppToMount->find( appName );
	std::vector<Ice::Identity>::const_iterator itId = ids.begin();
	for( ; itId != ids.end() ; itId++ )
	{
		try
		{
			TianShanIce::Site::AppMountPrx mountPrx = openAppMount( itId->name );
			if( mountPrx )
				rets.push_back( mountPrx );
		}
		catch(...){}
	}
	return rets;	
}

bool SiteAdminDb::addAppMount( TianShanIce::Site::AppMountPtr p )
{
	p->ident.category = MOUNT_LOCATOR;
	p->siteName = stringtolower(p->siteName);
	const Ice::Identity& id = p->ident;	
	try
	{
		mAppMount->add( p , id );
		return true;
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminDb,"addAppMount() failed to add AppMount[%s] into DB due to : %s"),
			id.name.c_str() , ex.ice_name().c_str() );
		return false;
	}
}

void SiteAdminDb::removeAppMount( const std::string& idname )
{
	Ice::Identity id;
	id.name		= idname;
	id.category	= MOUNT_LOCATOR;
	try
	{
		mAppMount->remove(id);
	}
	catch( const Ice::Exception& )
	{
	}
}

std::vector<TianShanIce::Site::LiveTxnPrx> SiteAdminDb::listAllTxn( ) const
{
	std::vector<TianShanIce::Site::LiveTxnPrx> rets;
	::Freeze::EvictorIteratorPtr iter = mLiveTxn->getIterator("",1024);
	if(!iter)
		return rets;
	while( iter->hasNext())
	{
		try
		{
			Ice::Identity id = iter->next();
			TianShanIce::Site::LiveTxnPrx txn = openTxn(id.name);
			if( txn)
				rets.push_back( txn );
		}
		catch( const Ice::Exception&)
		{
		}
	}
	return rets;
}

TianShanIce::Site::LiveTxnPrx SiteAdminDb::openTxn( const std::string& sessId ) const
{
	Ice::Identity id;
	id.name		= sessId;
	id.category	= TXN_LOCATOR;

	TianShanIce::Site::LiveTxnPrx txnPrx = NULL;
	try
	{
		txnPrx = TianShanIce::Site::LiveTxnPrx::checkedCast( mObjAdapter->createProxy(id) );
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(SiteAdminDb,"openTxn() txn[%s] is not found due to [%s]"),
			sessId.c_str(), ex.ice_name().c_str() );
		return NULL;
	}
	return txnPrx;
}

std::vector<TianShanIce::Site::LiveTxnPrx> SiteAdminDb::openTxnBySite( const std::string& siteName ) const
{
	std::vector<TianShanIce::Site::LiveTxnPrx> rets;
	std::vector<Ice::Identity> ids= mSiteToTxn->find( stringtolower( siteName ) );
	std::vector<Ice::Identity>::const_iterator itId = ids.begin();
	for ( ; itId != ids.end() ; itId++ )
	{
		TianShanIce::Site::LiveTxnPrx prx = openTxn( itId->name );
		if( prx)
			rets.push_back( prx );
	}
	return rets;
}

std::vector<TianShanIce::Site::LiveTxnPrx> SiteAdminDb::openTxnByMount( const std::string& mountId ) const
{
	std::vector<TianShanIce::Site::LiveTxnPrx> rets;
	std::vector<Ice::Identity> ids= mMountToTxn->find(mountId);
	std::vector<Ice::Identity>::const_iterator itId = ids.begin();
	for ( ; itId != ids.end() ; itId++ )
	{
		TianShanIce::Site::LiveTxnPrx prx = openTxn( itId->name );
		if( prx)
			rets.push_back( prx );
	}
	return rets;
}

std::vector<TianShanIce::Site::LiveTxnPrx> SiteAdminDb::openTxnBySiteAndMount( const std::string& csiteName , const std::string& appMount , const std::string& startId) const
{
	std::string siteName = stringtolower(csiteName);

	std::vector<TianShanIce::Site::LiveTxnPrx> rets;
	try
	{
		std::vector<Ice::Identity> ids;
		
		if( siteName.empty() || appMount.empty() )
		{
			if( siteName.empty() )
			{
				::Freeze::EvictorIteratorPtr itPtr = mLiveTxn->getIterator("",1024);	
				while( itPtr->hasNext())
				{
					ids.push_back( itPtr->next() );
				}		
			}
			else
			{
				ids = mSiteToTxn->find( siteName );
			}
			std::sort( ids.begin() , ids.end() );
		}
		else
		{
			std::vector<Ice::Identity> idSiteToTxn;
			std::vector<Ice::Identity> idMountToTxn;
			idSiteToTxn = mSiteToTxn->find( siteName );
			idMountToTxn = mMountToTxn->find( appMount );
			::std::sort(idMountToTxn.begin(), idMountToTxn.end());
			::std::sort(idSiteToTxn.begin(), idSiteToTxn.end());
			ids.clear();
			std::set_intersection(idMountToTxn.begin(),idMountToTxn.end(),idSiteToTxn.begin(),idSiteToTxn.end(),ids.begin());
		}
		
		std::vector<Ice::Identity>::const_iterator it = ids.begin();

		for( ; it != ids.end() ; it++ )
		{
			if( it->name.compare( startId) < 0 )
				continue;
			TianShanIce::Site::LiveTxnPrx prx = openTxn( it->name );
			if( prx )
				rets.push_back(prx);
		}
	}
	catch( const Ice::Exception& ex )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminDb,"openTxnBySiteAndMount() caught %s"),ex.ice_name().c_str() );
	}
	return rets;
}

bool SiteAdminDb::addTxn( TianShanIce::Site::LiveTxnPtr p)
{
	p->ident.category	= TXN_LOCATOR;
	const Ice::Identity& id = p->ident;
	try
	{
		mLiveTxn->add( p , id );
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminDb,"addTxn() failed to add txn[%s] intoDb:%s"),
			id.name.c_str() , ex.ice_name().c_str() );
		return false;
	}
	return true;
}

TianShanIce::Site::LiveTxnPtr SiteAdminDb::removeTxn( const std::string& sessId )
{
	Ice::Identity id;
	id.name		= sessId;
	id.category	= TXN_LOCATOR;
	try
	{
		return TianShanIce::Site::LiveTxnPtr::dynamicCast(mLiveTxn->remove( id ));
	}
	catch( const Ice::Exception& )
	{
		return NULL;
	}
}

bool SiteAdminDb::updateTxnDataLog( const char* sql )
{
#ifdef ZQ_OS_MSWIN
	if( !sql || sql[0] == 0 || !mMdbLog )
		return false;
	try
	{
		mMdbLog->executeSql(sql);
	}
	catch( const ZQ::common::MdbLogError& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminDb,"updateTxnDataLog() failed to execute sql[%s] due to [%s]"),
			sql,ex.what() );
		return false;
	}
#endif
	return true;
}


//////////////////////////////////////////////////////////////////////////
///TxnEvent

TianShanIce::Site::TxnEventPrx SiteAdminDb::openTxnEvent( const std::string& id ) const
{
	TianShanIce::Site::TxnEventPrx eventPrx = NULL;
	Ice::Identity ident;
	ident.name	= id;
	ident.category = EVENT_LOCATOR;
	try
	{
		eventPrx = TianShanIce::Site::TxnEventPrx::checkedCast( mObjAdapter->createProxy(ident ) );
	}
	catch( const Ice::Exception& )
	{
		return NULL;
	}
	return eventPrx;
}

bool SiteAdminDb::addTxnEvent( TianShanIce::Site::TxnEventPtr p )
{	
	Ice::Identity& id = p->identTxn;
	id.category	= EVENT_LOCATOR;

	try
	{
		mTxnEvent->add( p , id );
	}
	catch( const Ice::Exception& ex )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminDb,"addTxnEvent() failed to add txn event [%s] into db: %s"),
			id.name.c_str() , ex.ice_name().c_str() );
		return false;
	}
	return true;
}

std::vector<TianShanIce::Site::TxnEventPrx> SiteAdminDb::openEventByTxnId( const std::string& txnId ) const
{
	std::vector<TianShanIce::Site::TxnEventPrx> rets;
	Ice::Identity eventId;
	eventId.name = txnId;
	eventId.category = EVENT_LOCATOR;
	std::vector<Ice::Identity> ids = mTxnToEvent->find(eventId);
	for ( std::vector<Ice::Identity>::const_iterator it = ids.begin(); it != ids.end() ; it ++ )
	{
		TianShanIce::Site::TxnEventPrx p = openTxnEvent( it->name );
		if( p )
			rets.push_back( p );
	}
	return rets;
}

TianShanIce::Site::TxnEventPtr SiteAdminDb::removeTxnEvent( const std::string& id )
{
	TianShanIce::Site::TxnEventPtr ret = NULL;
	Ice::Identity ident;
	ident.name	= id;
	ident.category = EVENT_LOCATOR;
	try
	{
		ret = TianShanIce::Site::TxnEventPtr::dynamicCast( mLiveTxn->remove(ident) );
	}
	catch(  const Ice::Exception& )
	{
	}
	return ret;
}

bool SiteAdminDb::updateTxnEventLog( const char* sql )
{
#ifdef ZQ_OS_MSWIN
	if( !sql || sql[0] == 0 || !mMdbLog)
		return false;
	try
	{
		mMdbLog->executeSql(sql);
	}
	catch( const ZQ::common::MdbLogError& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SiteAdminDb,"updateTxnEventLog() failed to execute sql[%s] due to [%s]"),
			sql,ex.what() );
		return false;
	}
#endif
	return true;
}

void SiteAdminDb::setProperty( const std::string& key , const std::string& value)
{
	Ice::PropertiesPtr props = mEnv.getIc()->getProperties();
	props->setProperty(key,value);
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminDb,"setProperty() [%s] ==> [%s]"),
		key.c_str() , value.c_str());
}

