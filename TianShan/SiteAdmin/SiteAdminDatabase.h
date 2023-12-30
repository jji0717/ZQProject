#ifndef __tianshan_siteadmin_database_header_file_h__
#define __tianshan_siteadmin_database_header_file_h__

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

#include "SiteToMount.h"
#include "AppToMount.h"

#include "MountToTxn.h"
#include "SiteToTxn.h"
#include "TxnToEvent.h"
#include "SiteAdminSvc.h"
#include "SiteAdminDatabase.h"
#ifdef ZQ_OS_MSWIN
#include "MdbLog.h"
#endif

class SiteAdminEnv;

class SiteAdminDb
{
public:
	SiteAdminDb( SiteAdminEnv& env );
	virtual ~SiteAdminDb();

	bool	openDb( const std::string& dbPath, const std::string& runtimeDbPath , ZQADAPTER_DECLTYPE objAdpater );
	void	closeDb( );

	///virtual site
	TianShanIce::Site::CVirtualSitePrx openCVirtualSite( const std::string& id ) const;
	TianShanIce::Site::VirtualSites	listSites( ) const;
	bool	addVirtualSite( TianShanIce::Site::CVirtualSitePtr p );
	void	removeVirtualSite( const std::string& name );

	///app info
	TianShanIce::Site::CAppInfoPrx openCAppInfo( const std::string& id ) const;
	TianShanIce::Site::AppInfos listAppInfos( ) const;
	bool	addAppInfo( TianShanIce::Site::CAppInfoPtr p );
	void	removeAppInfo( const std::string& name );

	///app mount
	TianShanIce::Site::AppMountPrx openAppMount( const std::string& name ) const;
	TianShanIce::Site::AppMounts listAppMountsbySite( const std::string& siteName ) const;
	TianShanIce::Site::AppMounts listAppMountsByApp( const std::string& appName ) const;
	
	bool	addAppMount( TianShanIce::Site::AppMountPtr p );
	void	removeAppMount( const std::string& idname );

	///Txn
	TianShanIce::Site::LiveTxnPrx openTxn( const std::string& sessId ) const;	
	std::vector<TianShanIce::Site::LiveTxnPrx> listAllTxn( ) const;
	std::vector<TianShanIce::Site::LiveTxnPrx> openTxnBySite( const std::string& siteName ) const;
	std::vector<TianShanIce::Site::LiveTxnPrx> openTxnByMount( const std::string& mountId ) const;
	std::vector<TianShanIce::Site::LiveTxnPrx> openTxnBySiteAndMount( const std::string& siteName , const std::string& appMount , const std::string& startId ) const;
	bool	addTxn( TianShanIce::Site::LiveTxnPtr p);
	TianShanIce::Site::LiveTxnPtr	removeTxn( const std::string& sessId );
	bool	updateTxnDataLog( const char* sql );

	///TxnEvent
	TianShanIce::Site::TxnEventPrx openTxnEvent( const std::string& id ) const;
	bool	addTxnEvent( TianShanIce::Site::TxnEventPtr p );
	std::vector<TianShanIce::Site::TxnEventPrx> openEventByTxnId( const std::string& txnId ) const;
	TianShanIce::Site::TxnEventPtr removeTxnEvent( const std::string& id );
	bool	updateTxnEventLog( const char* sql );

protected:

	bool	openMountDB(  const std::string& dbPath, ZQADAPTER_DECLTYPE objAdpater );

	bool	openTxnDB(  const std::string& runtimeDbPath, ZQADAPTER_DECLTYPE objAdpater );

	bool	openEventDB(  const std::string& runtimeDbPath, ZQADAPTER_DECLTYPE objAdpater );	

	void	updateDbProperty( ZQADAPTER_DECLTYPE objAdapter , const std::string& key , const std::string& value );

	bool	initTxnMDB( );

	typedef std::vector<TianShanIce::Site::CVirtualSitePrx > VirtualSitePrxs;
	VirtualSitePrxs	getVirtualSitePrxs( ) const;

	typedef std::vector<TianShanIce::Site::CAppInfoPrx> CAppInfoPrxs;
	CAppInfoPrxs	getAppInfoPrxs( ) const;

private:

	void	setProperty( const std::string& key , const std::string& value);

private:

	SiteAdminEnv&						mEnv;

	ZQADAPTER_DECLTYPE					mObjAdapter;

	Freeze::ConnectionPtr				mDbConnection;

	TianShanIce::Site::AppToMountPtr	mAppToMount;
	TianShanIce::Site::SiteToMountPtr	mSiteToMount;
	Freeze::EvictorPtr					mAppMount;
	Freeze::EvictorPtr					mAppInfo;
	Freeze::EvictorPtr					mSiteInfo;

	TianShanIce::Site::SiteToTxnPtr		mSiteToTxn;
	TianShanIce::Site::MountToTxnPtr	mMountToTxn;
	Freeze::EvictorPtr					mLiveTxn;
	bool								mbEnableTxn;

	TianShanIce::Site::TxnToEventPtr	mTxnToEvent;
	Freeze::EvictorPtr					mTxnEvent;

	bool								mbEnableEvent;
	bool								mbEnableTxnMdbData;

#ifdef ZQ_OS_MSWIN
	ZQ::common::MdbLog*					mMdbLog;
#endif
};


#endif//__tianshan_siteadmin_database_header_file_h__
