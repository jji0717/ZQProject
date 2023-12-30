#include "NativeService.h"
#include "NativeServiceConfig.h"
#include "ContentImpl.h"
#include "FileSystemOp.h"
#include "IceLog.h"

ZQTianShan::ContentStore::NativeService server;
ZQ::common::ZQDaemon* Application = &server;

ZQ::common::Config::ILoader	*configLoader=&configGroup;

ZQ::common::NativeThreadPool* g_pMCCSThreadPool = 0;
ZQTianShan::ContentStore::ContentStoreImpl::Ptr g_pStore;

ZQADAPTER_DECLTYPE g_adapter;
Ice::CommunicatorPtr g_ic;


namespace ZQTianShan{
namespace ContentStore{

NativeService::NativeService() {
}

NativeService::~NativeService() {
}

bool NativeService::OnInit() {

    configGroup.setContentStoreNetId(configGroup.mccsConfig.netId);
    g_pMCCSThreadPool = new ZQ::common::NativeThreadPool();
    if ( !g_pMCCSThreadPool) {
        (*_logger)(Log::L_ERROR, CLOGFMT(NativeService, "Can't create threadpool instance"));
        return false;
    }

    int i=0;
    Ice::InitializationData iceInitData;
    iceInitData.properties =Ice::createProperties(i, 0);
				
    std::map<std::string, std::string>::const_iterator it = configGroup.mccsConfig.iceProp.begin();
    for( ; it != configGroup.mccsConfig.iceProp.end(); it++ ) {		
        iceInitData.properties->setProperty( it->first , it->second);
    }

    std::ostringstream oss;
    oss << _logDir << FNSEPS << getServiceName() << ".IceTrace.log";
    if( configGroup.iceTraceEnabled) {
        _iceLogFile = new ZQ::common::FileLog(oss.str().c_str(),
                                   configGroup.iceTraceLevel,
                                   ZQLOG_DEFAULT_FILENUM,
                                   configGroup.iceTraceSize);

        iceInitData.logger = new TianShanIce::common::IceLogI(_iceLogFile);
    }

    g_ic=Ice::initialize(i,0,iceInitData);

    g_pMCCSThreadPool->resize(configGroup.mccsConfig.workerThreadSize);

    try {
        g_adapter = ZQADAPTER_CREATE(g_ic, ADAPTER_NAME_ContentStore, configGroup.mccsConfig.clusterEndpoint.c_str(), (*_logger));
    }
    catch(Ice::Exception& ex) {
        (*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(NativeService, "Create g_adapter failed with endpoint=%s and exception is %s"),
            configGroup.mccsConfig.clusterEndpoint.c_str(), ex.ice_name().c_str());
        return false;
    }


    try {
        std::ostringstream oss;
        oss << configGroup.dbPath << '/' << getServiceName();
        if (!FS::createDirectory(oss.str())) {
            (*_logger)(Log::L_ERROR, CLOGFMT(NativeService, "Data db path %s error"), oss.str().c_str());
            return false;
        }

//        printf("NativeServer: %p\n", _logger);
        g_pStore = new ZQTianShan::ContentStore::ContentStoreImpl(
                    (*_logger), glog, *g_pMCCSThreadPool, g_adapter, oss.str().c_str());

        if (!g_pStore) {
            (*_logger)(Log::L_ERROR, CLOGFMT(NativeService, "create content store object failed."));
                return false;
        }

        g_pStore->_netId = configGroup.mccsConfig.netId;
        g_pStore->_contentEvictorSize = configGroup.mccsConfig.contentEvictorSize;
        g_pStore->_volumeEvictorSize = configGroup.mccsConfig.volumeEvictorSize;
        g_pStore->_cacheable = configGroup.mccsConfig.isCacheMode;
        g_pStore->_cacheLevel = configGroup.mccsConfig.cacheLevel;
        g_pStore->_exposeStreamService = 0;

        if (!g_pStore->initializeContentStore()) {
            (*_logger)(Log::L_ERROR, CLOGFMT(NativeService, "initializeContentStore() failed"));
            return false;
        }

		NativeServiceConfig::Volumes::const_iterator iter = configGroup.mccsConfig.volumes.begin();
		for(; iter != configGroup.mccsConfig.volumes.end(); ++iter) {		
			std::string path = iter->path;
			if(iter->path.at(iter->path.length()-1) != FNSEPC) {
				path += FNSEPC;		
			}
			g_pStore->mountStoreVolume(iter->name, path, iter->isDefault);
		}
    }
    catch(Ice::Exception& ex) {
        (*_logger)(Log::L_ERROR, CLOGFMT(NativeService, "create content store object failed and exception is %s"), ex.ice_name().c_str());
        return false;
    }
			
    return ZQDaemon::OnInit();
}

bool NativeService::OnStart() {
    try {
        g_adapter->activate();
    }
    catch(const Ice::Exception& ex) {
        (*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(NativeService, "activate g_adapter caught exception: %s"), ex.ice_name().c_str());
    }

    return ZQDaemon::OnStart();
}

void NativeService::OnUnInit() {
    try {
        if (g_pStore) {
            g_pStore->unInitializeContentStore();
            g_pStore= NULL;
        }
    }
    catch (...) {
    }


    try {
        g_ic->shutdown();
        g_ic->destroy();
    }
    catch (...) {
    }	

     try {
        if (_iceLogFile) {
            delete _iceLogFile;
            _iceLogFile= NULL;
        }
    }
    catch (...) {
        _iceLogFile= NULL;
    }

    if(g_pMCCSThreadPool) {
        try {
            delete g_pMCCSThreadPool;
            g_pMCCSThreadPool=NULL;
        }
        catch(...){}
    }

    return ZQDaemon::OnUnInit();	
}

void NativeService::OnStop() {
    return ZQDaemon::OnStop();	
}

}}
