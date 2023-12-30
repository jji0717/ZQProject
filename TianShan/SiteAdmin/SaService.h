
#ifndef __tianshan_site_admin_services_header_file_h__apple__
#define __tianshan_site_admin_services_header_file_h__apple__

#include "ZQ_common_conf.h"

#ifdef ZQ_OS_MSWIN
#include <BaseZQServiceApplication.h>
#else
#include "ZQDaemon.h"
#endif

#include "FileLog.h"
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>

#include "SiteAdminEnv.h"
#include "SiteAdminDatabase.h"
#include "SiteAdminSvcImpl.h"
#include "TxnWatchDog.h"
#include "TxnTransfer.h"
#include "SiteAdminIceObjFactory.h"
#include "StreamEventSinker.h"
#include "EventSenderManager.h"


class SaService : public ZQ::common::BaseZQServiceApplication
{
public:
	SaService(void);
	virtual ~SaService(void);

protected:

	virtual HRESULT OnInit(void); 
	virtual HRESULT OnStop(void);	
	virtual HRESULT OnStart(void);	
	virtual HRESULT OnUnInit(void);

private:

	void			adjustConfig( )	;
	bool			initializeIceRuntime( );
	bool			initializeSiteAdmin( );
	void			startEventSinker();
	void			stopEventSinker();	
private:
	SiteAdminEnv				mEnv;
	SiteAdminDb					mDb;
	TxnTransfer					mTxnTransfer;
	TxnWatchDog					mTxnWatchDog;
	SiteAdminImplPtr			mSaImpl;	
	StreamEventSinker			mEventSinker;
	EventSenderManager			mEventSenderManager;

	ZQ::common::FileLog			mIceLogger;
	SiteAdminIceObjFactoryptr	mObjFactory;

};

#endif//__tianshan_site_admin_services_header_file_h__apple__
