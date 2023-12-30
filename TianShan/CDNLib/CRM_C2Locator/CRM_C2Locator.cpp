// CRM_C2Locator.cpp : Defines the entry point for the DLL application.
//
#include "C2LocatorConf.h"
#include "C2Env.h"
#include "C2LocatorImpl.h"
#include "LocateRequestHandler.h"
#include "TransferDeleteRequestHandler.h"
#include "CacheServerRequestHandle.h"
#include <CRMInterface.h>
#include <FileLog.h>
// #include "C2Snmp.h"
// #include "C2SnmpExt.h"
// #include "snmp/ZQSnmp.hpp"

#include <IceLog.h>

#ifdef _MANAGED
#pragma managed(push, off)
#endif

struct GlobalResource
{
    ZQ::common::Config::Loader<ZQTianShan::CDN::C2LocatorConf> conf;
    ZQ::common::FileLog* pLog;
    ZQ::common::FileLog* pIceLog;
    ZQ::common::NativeThreadPool* pThreadPool;
    Ice::CommunicatorPtr communicator;
    ZQTianShan::CDN::C2Env* pEnv;
    IceInternal::Handle<TianShanIce::SCS::C2LocatorImpl> pLocator;
//    ZQ::Snmp::Subagent* pAgent;
//    PortSnmpManager portSnmpMgr;

    GlobalResource();
    void clear();
} gResource;

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        gResource.clear();
        break;
    default:
        return FALSE;
    }
    return TRUE;
}
#endif

ZQTianShan::CDN::TransferDeleteRequestHandler* pTransferDeleteHandler;
static ZQTianShan::CDN::LocateRequestHandler* pLocateHandler;
static ZQTianShan::CDN::CacheServerRequestHandle* pCacheServerRequestHandler;

extern "C"
{
    __EXPORT bool CRM_Entry_Init(CRG::ICRMManager* mgr)
    {
        // step 1: load config
        gResource.conf.setLogger(&(mgr->superLogger()));
        if(!gResource.conf.loadInFolder(mgr->getConfigFolder().c_str()))
        {
            mgr->superLogger()(ZQ::common::Log::L_ERROR, CLOGFMT(C2Locator, "Can't load configuration file in the folder %s"), mgr->getConfigFolder().c_str());
            return false;
        }
        // step 2: create the global resource such as log, communicator, adapter .etc.
        // create file logger
        std::string logpath = mgr->getLogFolder() + "CRM_C2Locator.log";
        try
        {
            gResource.pLog = new ZQ::common::FileLog(logpath.c_str(), gResource.conf.loglevel, gResource.conf.logCount, gResource.conf.logsize, gResource.conf.logbuffersize);
        }catch(ZQ::common::FileLogException& e)
        {
            mgr->superLogger()(ZQ::common::Log::L_ERROR, CLOGFMT(C2Locator, "Failed to create logger at %s. detail:%s"), logpath.c_str(), e.getString());
            return false;
        }

        std::string iceLogPath = mgr->getLogFolder() + "CRM_C2Locator.IceTrace.log";
        try
        {
            gResource.pIceLog = new ZQ::common::FileLog(iceLogPath.c_str(), gResource.conf.loglevel, 10, gResource.conf.logsize, gResource.conf.logbuffersize);
        }catch(ZQ::common::FileLogException& e)
        {
            mgr->superLogger()(ZQ::common::Log::L_ERROR, CLOGFMT(C2Locator, "Failed to create ice trace logger at %s. detail:%s"), iceLogPath.c_str(), e.getString());
            return false;
        }

        ZQ::common::Log& log = *gResource.pLog;
        // thread pool
        int threadpoolsize = gResource.conf.watchThreadPoolSize;
        try
        {
            gResource.pThreadPool = new ZQ::common::NativeThreadPool(threadpoolsize);
        }
        catch(...)
        {
            // log & release allocated resource
            log(ZQ::common::Log::L_ERROR, CLOGFMT(C2Locator, "Can't create thread pool with size %d"), threadpoolsize);
            gResource.clear();
            return false;
        }

        // communicator
        {
            Ice::InitializationData initData;
            // init the ice properties
            Ice::PropertiesPtr proper= Ice::createProperties();
            {
                std::map<std::string, std::string>::const_iterator itProp;
                for(itProp = gResource.conf.iceProps.begin(); itProp != gResource.conf.iceProps.end(); ++itProp)
                    proper->setProperty(itProp->first, itProp->second);
            }
            initData.properties = proper;
            initData.logger = new TianShanIce::common::IceLogI(gResource.pIceLog);
            gResource.communicator = Ice::initialize(initData);
        }

        // env object
        try
        {
            gResource.pEnv = new ZQTianShan::CDN::C2Env
                (*gResource.pLog,
                 *gResource.pIceLog,
                 *gResource.pThreadPool,
                 gResource.communicator,
                 gResource.conf
                 );
        }
        catch(...)
        {
            log(ZQ::common::Log::L_ERROR, CLOGFMT(C2Locator, "Failed to create C2Env object"));
            gResource.clear();
            return false;
        }

		/// load auth keyfile if enabled
		if( gResource.conf.authEnable >= 1 ) {
			if( !gResource.pEnv->_auth.loadKeyfile(gResource.conf.authKeyfile) ) {
				log(ZQ::common::Log::L_ERROR,CLOGFMT(C2Locator,"failed to load auth keyfile[%s]"),
					gResource.conf.authKeyfile.c_str() );
				return false;
			}
		}

        // locator object
        try
        {
            gResource.pLocator = new TianShanIce::SCS::C2LocatorImpl(*gResource.pEnv); // , gResource.portSnmpMgr);
        }
        catch(...)
        {
            log(ZQ::common::Log::L_ERROR, CLOGFMT(C2Locator, "Failed to create C2Locator object"));
            gResource.clear();
            return false;
        }

        // restore data from db
        try
        {
            gResource.pLocator->OnRestore(Ice::Current());
        }
        catch(...)
        {
            log(ZQ::common::Log::L_ERROR, CLOGFMT(C2Locator, "Failed to restore data from db"));
            gResource.clear();
            return false;
        }
        gResource.pLocator->getClientManager().startIdleTimer();
        gResource.pLocator->getTransferPortManager().startRenewTimer();
        // step 3: create the request handler object.
        pLocateHandler = new ZQTianShan::CDN::LocateRequestHandler(*gResource.pEnv, *gResource.pLocator);
        pTransferDeleteHandler = new ZQTianShan::CDN::TransferDeleteRequestHandler(*gResource.pEnv, *gResource.pLocator);
		pCacheServerRequestHandler = new ZQTianShan::CDN::CacheServerRequestHandle(*gResource.pEnv, *gResource.pLocator);
		pCacheServerRequestHandler->start();

		gResource.pEnv->_adapter->ZQADAPTER_ADD(gResource.communicator, gResource.pLocator, "C2Locator");
		gResource.pLocator->start();

		// step 4: register the request handlers.
		mgr->registerContentHandler(gResource.conf.cacheserveruri, pCacheServerRequestHandler);
		//mgr->registerContentHandler(gResource.conf.cacheserveruri + "/transferterminate", pCacheServerRequestHandler);

        //mgr->registerContentHandler("/?sccdn(\\.cgi)?", pLocateHandler);
        //mgr->registerContentHandler("/?vodadi(\\.cgi)?", pLocateHandler);
		//mgr->registerContentHandler("/?ngbBOne", pLocateHandler); 

		//mgr->registerContentHandler("/cacheserver",pCacheServerRequestHandler);
		//mgr->registerContentHandler("/cacheserver/transferterminate",pCacheServerRequestHandler);

        mgr->registerContentHandler("/?\\*", pTransferDeleteHandler);


        if(!gResource.conf.alternateLocateUriExp.empty())
        {
            mgr->registerContentHandler(gResource.conf.alternateLocateUriExp, pLocateHandler);
        }
        gResource.pEnv->_adapter->activate();
        gResource.pEnv->registerSnmpVariables();
        return true;
    }

    __EXPORT void CRM_Entry_Uninit(CRG::ICRMManager* mgr)
    {
        gResource.pEnv->_adapter->deactivate();
        gResource.pEnv->_adapter->destroy();
        // stop the locator server
        // step 1: unregister the request handlers
        if(!gResource.conf.alternateLocateUriExp.empty())
        {
            mgr->unregisterContentHandler(gResource.conf.alternateLocateUriExp, pLocateHandler);
        }

		mgr->unregisterContentHandler(gResource.conf.cacheserveruri, pCacheServerRequestHandler);

        mgr->unregisterContentHandler("/?\\*", pTransferDeleteHandler);
		
        // step 2: uninit the management modules.

        delete pLocateHandler;
        pLocateHandler = NULL;
        delete pTransferDeleteHandler;
        pTransferDeleteHandler = NULL;
		
		pCacheServerRequestHandler->stop();
		delete pCacheServerRequestHandler;
        pCacheServerRequestHandler=NULL;

		gResource.pLocator->getTransferPortManager().stopRenewTimer();
        gResource.pLocator->getClientManager().stopIdleTimer();

        gResource.pEnv->uninitial();
        // step 3: clean the global resources.
        gResource.clear();
    }
}



GlobalResource::GlobalResource()
:conf("CRM_C2Locator.xml")
{
    pLog = NULL;
    pThreadPool = NULL;
    pEnv = NULL;
    pLocator = NULL;
//    pAgent = NULL;
}
void GlobalResource::clear()
{
    //if(pAgent) {
    //    delete pAgent;
    //    pAgent = NULL;
    //}
    pLocator = NULL;
    if(pEnv)
    {
        delete pEnv;
        pEnv = NULL;
    }
    communicator->destroy();
    communicator = NULL;
    if(pThreadPool)
    {
        delete pThreadPool;
        pThreadPool = NULL;
    }
    if(pIceLog)
    {
        delete pIceLog;
        pIceLog = NULL;
    }
    if(pLog)
    {
        delete pLog;
        pLog = NULL;
    }
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

