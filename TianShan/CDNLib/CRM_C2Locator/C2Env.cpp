#include <TimeUtil.h>
#include <Text.h>
#include <TianShanIceHelper.h>
#include "FileSystemOp.h"
#include "C2Env.h"
#include "TransferPortManager.h"
#include "ClientManager.h"

#ifdef ZQ_OS_LINUX
extern "C"
{
#include <sys/stat.h>
}
#ifndef stricmp
#define stricmp strcasecmp
#endif
#else
#include <io.h>
#endif

namespace ZQTianShan{
namespace CDN{

InMemoryObjects::InMemoryObjects(const std::string& category)
    :category_(category) {
}

const std::string& InMemoryObjects::category() const {
    return category_;
}

Ice::ObjectPtr InMemoryObjects::locate(const Ice::Current& c,
                                       Ice::LocalObjectPtr& cookie) {
    ZQ::common::MutexGuard guard(lock_);
    Objects::iterator it = objs_.find(c.id.name);
    if(it != objs_.end()) {
        return it->second;
    } else {
        return NULL;
    }
}
void InMemoryObjects::finished(const Ice::Current& c,
                               const Ice::ObjectPtr& servant,
                               const Ice::LocalObjectPtr& cookie) {
}
void InMemoryObjects::deactivate(const std::string& category) {
}

Ice::Identity InMemoryObjects::add(Ice::ObjectPtr obj) {
    ZQ::common::MutexGuard guard(lock_);
    Ice::Identity ident;
    ident.category = category_;
    ident.name = IceUtil::generateUUID();
    objs_[ident.name] = obj;
    return ident;
}

Ice::ObjectPtr InMemoryObjects::remove(const Ice::Identity& ident) {
    ZQ::common::MutexGuard guard(lock_);
    if(ident.category != category_)
        return NULL;

    Objects::iterator it = objs_.find(ident.name);
    if(it != objs_.end()) {
        Ice::ObjectPtr obj = it->second;
        objs_.erase(it);
        return obj;
    } else {
        return NULL;
    }
}

void InMemoryObjects::clear() {
    ZQ::common::MutexGuard guard(lock_);
    objs_.clear();
}

C2Env::C2Env(::ZQ::common::FileLog& filelog, ::ZQ::common::FileLog& icelog, ::ZQ::common::NativeThreadPool& threadPool, ::Ice::CommunicatorPtr& communicator, C2LocatorConf& conf)
    :_conf(conf),
	 iTransferSessionTimeout(conf.transferSessionTimeOut),
     _communicator(communicator),
     _adapter(NULL),
     _pool(threadPool), 
     _pLog(&filelog),
     _pIceLog(&icelog),
     _watch(filelog, threadPool, _adapter, DBFILENAME_C2TransferSession),
	 _auth(*this),
     _eventChannelEndpoint(conf.icestormEP),
     _clientMgr(NULL),
     _portMgr(NULL),
	 _mmib(filelog, 2200, 3),
	 _snmpSA(filelog, _mmib, 5000)
{
    //_clientMgr = new ZQTianShan::CDN::ClientManager(*this);
    //_portMgr   = new ZQTianShan::CDN::TransferPortManager(*this);

    _endpoint = (!conf.endpoint.empty()) ? conf.endpoint : DEFAULT_ENDPOINT_C2Locator;

	if (iTransferSessionTimeout <= 0)
		iTransferSessionTimeout = DEFAULT_TICKET_LEASETERM;
	if (iTransferSessionTimeout > MAX_TICKET_LEASETERM)
		iTransferSessionTimeout = MAX_TICKET_LEASETERM;
	if (iTransferSessionTimeout < MIN_TICKET_LEASETERM)
		iTransferSessionTimeout = MIN_TICKET_LEASETERM;

    envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(C2Env, "open adapter %s at %s"), ADAPTER_NAME_C2Locator, _endpoint.c_str());
    try
    {
        //initialize adapter
        _adapter = ZQADAPTER_CREATE(_communicator, ADAPTER_NAME_C2Locator, _endpoint.c_str(), *_pLog);

        // servent locator for timers
        timers_ = new InMemoryObjects("Timer");
        _adapter->addServantLocator(timers_, timers_->category());

        _factory = new C2Factory(*this);

        openDB(conf.dbPath.c_str(), conf.dbRuntimePath.c_str());

		initialize();
    }
    catch(Ice::Exception& ex)
    {
        envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C2Env,"Create adapter and open db failed with endpoint=%s and exception is %s"), _endpoint.c_str(), ex.ice_name().c_str());
       // throw ex;
    }
    catch (...)
    {
        envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C2Env,"Create adapter failed and open db with endpoint=%s andcaught unknow exception"), _endpoint.c_str());
    }

    //init eventchannel
    try
    {
        envlog(ZQ::common::Log::L_NOTICE, CLOGFMT(C2Env, "do connect to EventChannel: [%s]"), _eventChannelEndpoint.c_str());

        _pEventChannel = new TianShanIce::Events::EventChannelImpl(_adapter, _eventChannelEndpoint.c_str(), true);

        envlog(ZQ::common::Log::L_NOTICE, CLOGFMT(C2Env, "connect to EventChannel: [%s] successfully"), _eventChannelEndpoint.c_str());
    }
    catch(const TianShanIce::BaseException& ex)
    {
        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C2Env, "connect to EventChannel: [%s] caught %s:%s"), _eventChannelEndpoint.c_str(), ex.ice_name().c_str(), ex.message.c_str());
    }
    catch(const Ice::Exception& ex)
    {
        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C2Env, "connect to EventChannel: [%s] caught an %s"), _eventChannelEndpoint.c_str(), ex.ice_name().c_str());
    }

    // init other variables
    replicaReportIntervalSec = conf.replicaReportIntervalSec;
    if(conf.testContent.name.empty())
    {
        ignoreLamWithTestContent = false;
        testContent.name.clear();
        testContent.bandwidth = 0;
        testContent.volumeList.clear();
    }
    else
    {
        ignoreLamWithTestContent = true;
        testContent = conf.testContent;
        envlog(ZQ::common::Log::L_INFO, CLOGFMT(C2Env, "Ignore LAM with Content: name=[%s], bandwidth=[%d]"), testContent.name.c_str(), testContent.bandwidth);
    }

	_watch.start();  
}

C2Env::~C2Env()
{
}

void C2Env::initialize()
{
    for(size_t i = 0; i < _conf.pubLogs.size(); ++i) {
        const PublishedLog& l = _conf.pubLogs[i];
        _adapter->publishLogger(l.path.c_str(), l.syntax.c_str(), l.key.c_str(), l.type.c_str(), l.properties);
    }
}


// generate the DB_CONFIG file
static bool generateSimpleFile(const char* name, const char* content) {
    FILE* f = fopen(name, "w+");
    if(f) {
        int count = strlen(content);
        if(count == fwrite(content, 1, count, f)) {
            fclose(f);
            return true;
        } else {
            fclose(f);
            return false;
        }
    } else {
        return false;
    }
}
static void generateDbConfigFile(const std::string& dir) {
    std::string fileName = dir + "DB_CONFIG";
    if(0 != access(fileName.c_str(), 0)) {
#define Default_DB_CONFIG "set_lk_max_locks 100000\nset_lk_max_objects 100000\nset_lk_max_lockers 100000\nset_cachesize 0 16777216 0\n"
        generateSimpleFile(fileName.c_str(), Default_DB_CONFIG);
    }
}

void C2Env::uninitial()
{
	try
	{
		closeDB();
		_factory = NULL;
		_pEventChannel = NULL;
	}
	catch (::TianShanIce::BaseException& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C2Env, "uninitial catch TianShanIce exception[%s]"), ex.ice_name().c_str());
	}
	catch (::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C2Env, "uninitial catch iCE exception[%s]"), ex.ice_name().c_str());
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C2Env, "uninitial catch unkown exception"));
	}
}

void C2Env::updateIceProperty(Ice::PropertiesPtr iceProperty , 
									  const std::string& key ,
									  const std::string& value )
{
	assert(iceProperty);
	iceProperty->setProperty( key , value );
	envlog(ZQ::common::Log::L_INFO,CLOGFMT(C2Env,"updateIceProperty() [%s] = [%s]"),
		key.c_str() , value.c_str() );	
}

#define TransferSess TransferSessionCategory

#define FREEZEPROPENV(x,y)	std::string("Freeze.DbEnv.")+x+y
#define FREEZEPROPEVTSESSION(x,y) std::string("Freeze.Evictor.")+x+"."+TransferSessionCategory+y


bool C2Env::openDB(const char* databasePath, const char* dbRuntimePath)
{
    closeDB();

    if (NULL == databasePath || strlen(databasePath) <1)
        _dbPath = _programRootPath + "data" FNSEPS;
    else 
        _dbPath = databasePath;

    if (FNSEPC != _dbPath[_dbPath.length()-1])
        _dbPath += FNSEPS;

    if ( NULL == dbRuntimePath || strlen(dbRuntimePath)<1 ) 
    {
        _dbRuntimePath = _dbPath;
    }
    else
    {
        _dbRuntimePath = dbRuntimePath;
    }
    if (FNSEPC != _dbRuntimePath[_dbRuntimePath.length()-1])
        _dbRuntimePath += FNSEPS;
    try 
    {	

        // open the Indexes
#define INSTANCE_INDEX(_IDX) envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(C2Env, "create index: " #_IDX)); \
        _idx##_IDX = new ::TianShanIce::SCS::_IDX(INDEXFILENAME(_IDX))

        envlog(ZQ::common::Log::L_INFO, CLOGFMT(C2Env, "opening runtime database at path: %s"), _dbRuntimePath.c_str());

		_dbPath += DBFILENAME_C2TransferSession FNSEPS;
        FS::createDirectory(_dbPath.c_str());
        generateDbConfigFile(_dbPath);
		_dbRuntimePath += DBFILENAME_C2TransferSession FNSEPS;
        FS::createDirectory(_dbRuntimePath.c_str());
        generateDbConfigFile(_dbRuntimePath);

		Ice::PropertiesPtr iceProperty = _adapter->getCommunicator()->getProperties();
		
		updateIceProperty( iceProperty, FREEZEPROPENV(_dbRuntimePath,".DbPrivate") , "0");
		updateIceProperty( iceProperty, FREEZEPROPENV(_dbRuntimePath,".DbRecoverFatal" ) , "1" );
		updateIceProperty( iceProperty, FREEZEPROPENV(_dbRuntimePath,".CheckpointPeriod") , _conf.freezeCheckPointPeriod );		
		updateIceProperty( iceProperty, FREEZEPROPEVTSESSION(_dbRuntimePath,".SavePeriod"), _conf.freezeSavePeriod );		
		updateIceProperty( iceProperty, FREEZEPROPEVTSESSION(_dbRuntimePath,".SaveSizeTrigger"),_conf.freezeSaveSizeTrigger );

        INSTANCE_INDEX(SessionIdx);
        {
            std::vector<Freeze::IndexPtr> indices;
            indices.push_back(_idxSessionIdx);

            envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(C2Env, "create evictor %s with index %s"), DBFILENAME_C2TransferSession, "SessionIdx");

#if ICE_INT_VERSION / 100 >= 303
            _eC2TransferSession = ::Freeze::createBackgroundSaveEvictor(_adapter, _dbRuntimePath, TransferSess, 0, indices);
#else
            _eC2TransferSession = Freeze::createEvictor(_adapter, _dbRuntimePath, TransferSess, 0, indices);
#endif
            int32 evictorSize = _conf.transferSessionEvictorSize;
            if(evictorSize < 100) {
                evictorSize = 100;
            }
            _eC2TransferSession->setSize(evictorSize);
            _adapter->addServantLocator(_eC2TransferSession, TransferSess);	
        }

        return true;
    }
    catch(const Ice::Exception& ex)
    {
        ZQTianShan::_IceThrow<TianShanIce::ServerError> (*_pLog,"C2Env",1001,CLOGFMT(C2Env, "openDB() caught exception: %s"), ex.ice_name().c_str());
    }
    catch(...)
    {
        ZQTianShan::_IceThrow<TianShanIce::ServerError> (*_pLog,"C2Env",1002, CLOGFMT(C2Env, "openDB() caught unkown exception"));
    }

    return true;
}

void C2Env::closeDB(void)
{
    _eC2TransferSession = NULL;
    _idxSessionIdx = NULL;
}


void C2Env::reportSessionExpired(const Ice::Identity& ident)
{
    ZQ::common::MutexGuard guard(lockExpiredSessions);
    expiredSessions.push_back(ident);
    hExpiredSessionNotifier.signal();
}
std::vector<Ice::Identity> C2Env::getExpiredSessions()
{
    ZQ::common::MutexGuard guard(lockExpiredSessions);
    std::vector<Ice::Identity> ret;
    ret.swap(expiredSessions);
    return ret;
}

//////////////////////////////////////////////////////////////////////////
// RequestAuth
#ifdef MLOG
#	undef MLOG
#endif //MLOG

#define MLOG _log
RequestAuth::RequestAuth( C2Env& env )
:_env(env),
_log(*_env._pLog),
mAuth(_log){
}

RequestAuth::~RequestAuth( ) {
}

bool RequestAuth::loadKeyfile( const std::string& keyfile ) {
	if( _env._conf.authEnable == 0 )
		return true;
	return mAuth.loadKeyFile(keyfile.c_str());
}
bool RequestAuth::auth( const SimpleXMLParser::Node* root, const std::string& sessId ) {
	if(_env._conf.authEnable <= 0 )
		return true;
	if( !root ) {
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(RequestAuth,"sess[%s], no xml root node, auth failed"),
			sessId.c_str());
		return false;
	}
	

	std::string paid,pid,txnId, clientip, clientsess, expiration, signature;
	if(!getSessionContent(root, sessId, paid, pid, txnId, clientip, clientsess, expiration, signature)) {	
		return false;
	}
	
	if( !mAuth.authC2(paid,pid,txnId,clientsess,expiration,signature) ) {
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(RequestAuth,"Sess[%s]  Authentication Failed"),
			sessId.c_str());
		return false;
	}
	if( _env._conf.checkExpirationInAuth >= 1){
		int64 llExpiration = 0 ;
		if( llExpiration = ZQ::common::TimeUtil::ISO8601ToTime(expiration.c_str()) == 0 ) {
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(RequestAuth,"Sess[%s], bad expiration, auth failed"),
				sessId.c_str());
			return false;
		}
		if( llExpiration < ZQ::common::TimeUtil::now() ) {
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(RequestAuth,"Sess[%s], expiration [%s]reached, auth failed"),
				sessId.c_str(),expiration.c_str());
			return false;
		}
	}
	return true;
}

bool RequestAuth::getSessionContent( const SimpleXMLParser::Node* root, const std::string& sessId,
									std::string& paid, std::string& pid,
									std::string& txnId,
									std::string& clientIp, std::string& clientSess,
									std::string& expiration, std::string& signature ) {
	typedef SimpleXMLParser::Node Node;
	const Node* nPaid = findNode(root,"Object/Name/AssetId");
	if( !nPaid ) {
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(RequestAuth,"sess[%s], no AssetId is found in xml, auth failed"),
			sessId.c_str() );
		return false;
	}
	paid = nPaid->content;

	const Node* nPid = findNode(root,"Object/Name/ProviderID");
	if( !nPid ) {
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(RequestAuth,"sess[%s], no ProviderId is found in xml, auth failed"),
			sessId.c_str() );
		return false;
	}
	pid = nPid->content;
	
	const Node* nSession = findNode(root, "Session");
	if(!nSession) {
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(RequestAuth,"sess[%s], no Session is found in xml, auth failed"),
			sessId.c_str() );
		return false;
	}
	assert(nSession!=0);	
	try {
		ZQTianShan::Util::getPropertyData(nSession->attrs,"txnId",txnId);
		ZQTianShan::Util::getPropertyData(nSession->attrs,"clientSessionId",clientSess);
		ZQTianShan::Util::getPropertyData(nSession->attrs,"client",clientIp);
		ZQTianShan::Util::getPropertyData(nSession->attrs,"expiration",expiration);
		ZQTianShan::Util::getPropertyData(nSession->attrs,"signature",signature);
		if( paid.empty() || 
			pid.empty() ||
			txnId.empty() ||
			clientSess.empty() ||
			clientIp.empty() ||
			expiration.empty() ||
			signature.empty() ) 
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(RequestAuth,"Sess[%s], lack of paid||pid||txnId||clientSessionId||client||expiration||signature, auth failed"),
				sessId.c_str());
			return false;
		}		
	}catch( const TianShanIce::InvalidParameter& ) {
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(RequestAuth,"Sess[%s], lack of paid||pid||txnId||clientSessionId||client||expiration||signature, auth failed"),
			sessId.c_str());
		return false;
	}
	return true;
}

void C2Env::registerSnmpVariables()
{
    //for snmp
    _mmib.addObject(new ZQ::SNMP::SNMPObjectByAPI<C2Env, uint32>("c2locLogLevel", *this, ZQ::SNMP::AsnType_Int32, &C2Env::snmp_getLogLevel_Main, &C2Env::snmp_setLogLevel_Main));
    _mmib.addObject(new ZQ::SNMP::SNMPObjectByAPI<C2Env, uint32>("c2locLogIce", *this, ZQ::SNMP::AsnType_Int32, &C2Env::snmp_getLogLevel_Ice, &C2Env::snmp_setLogLevel_Ice));
    _mmib.addObject(new ZQ::SNMP::SNMPObject("c2locLogSize", _conf.logsize));
    _mmib.addObject(new ZQ::SNMP::SNMPObject("c2locLamIf", _conf.lamEndpoint));
    _mmib.addObject(new ZQ::SNMP::SNMPObject("c2locForwardLocator", _conf.forwardUrl));
    _mmib.addObject(new ZQ::SNMP::SNMPObject("c2locOptSelectionRetryMax", _conf.selectionRetryMax));
    _mmib.addObject(new ZQ::SNMP::SNMPObject("c2locOptIdxRate", _conf.indexFileTransferRate));
    _mmib.addObject(new ZQ::SNMP::SNMPObject("c2locTotalBwKbps", (int64&)portPerf.totalBw));
    _mmib.addObject(new ZQ::SNMP::SNMPObject("c2locActiveBwKbps", (int64&)portPerf.activeBw));
    _mmib.addObject(new ZQ::SNMP::SNMPObject("c2locTransferCount", portPerf.sessions));

    _mmib.addObject(new ZQ::SNMP::SNMPObjectByAPI<C2Env, uint32>("c2locHitRatePercent", *this, ZQ::SNMP::AsnType_Int32, &C2Env::snmp_getHitRate));
    _mmib.addObject(new ZQ::SNMP::SNMPObjectByAPI<C2Env, uint32>("c2locHitRateReset", *this, ZQ::SNMP::AsnType_Int32, NULL, &C2Env::snmp_setHitRate));

    _snmpSA.start();
}

void C2Env::snmp_c2locPortTable(TianShanIce::SCS::TransferPort& info,  uint32 index)
{
    static ZQ::SNMP::Oid c2OidTbl;
    if (c2OidTbl.isNil())
        _mmib.reserveTable("c2locPortTable", 10, c2OidTbl);
    if (c2OidTbl.isNil())
    {
        envlog(ZQ::common::Log::L_WARNING, CLOGFMT(C2Env,"snmp_c2locPortTable() failed to locate table in MIB"));
        return;
    }

    /*ZQ::SNMP::Oid tmpOid(c2OidTbl);
    tmpOid.append(1);
    _mmib.removeSubtree(tmpOid);*/

    //ZQTianShan::Util::dumpTianShanIceStrValues(info.addressListIPv6).c_str());
    std::string address=ZQTianShan::Util::dumpTianShanIceStrValues(info.addressListIPv4).c_str();
    _mmib.addTableCell(c2OidTbl,  1, index, new ZQ::SNMP::SNMPObjectDupValue("c2locPortIndex",              (int32)index));
    _mmib.addTableCell(c2OidTbl,  2, index, new ZQ::SNMP::SNMPObjectDupValue("c2locPortName",               (std::string)info.name));
    _mmib.addTableCell(c2OidTbl,  3, index, new ZQ::SNMP::SNMPObjectDupValue("c2locPortSS",                 (std::string)info.streamService));
    _mmib.addTableCell(c2OidTbl,  4, index, new ZQ::SNMP::SNMPObjectDupValue("c2locPortCapacityKbps",       (int32)(info.capacity/1000)));
    _mmib.addTableCell(c2OidTbl,  5, index, new ZQ::SNMP::SNMPObjectDupValue("c2locPortUsedBwKbps",         (int32)(info.activeBandwidth/1000)));
    _mmib.addTableCell(c2OidTbl,  6, index, new ZQ::SNMP::SNMPObjectDupValue("c2locPortTransferCount",      (int32)info.activeTransferCount));
    _mmib.addTableCell(c2OidTbl,  7, index, new ZQ::SNMP::SNMPObjectDupValue("c2locPortEnabled",            (int32)info.enabled ));
    _mmib.addTableCell(c2OidTbl,  8, index, new ZQ::SNMP::SNMPObjectDupValue("c2locPortPenalty",            (int32)info.penalty ));
    _mmib.addTableCell(c2OidTbl,  9, index, new ZQ::SNMP::SNMPObjectDupValue("c2locPortAddresses",          (std::string)address ));
    _mmib.addTableCell(c2OidTbl,  10, index, new ZQ::SNMP::SNMPObjectDupValue("c2locPortErrorRatePercent",  (int32)info.isUp ));
}

void C2Env::updatePort(TianShanIce::SCS::TransferPort port)
{
    std::string name = port.name;
    ZQ::common::MutexGuard guard(_registeresLock);
    if(_registeres.end() != std::find(_registeres.begin(), _registeres.end(), name))
		return;

	// new port
	_registeres.push_back(name);
	_portCount = (int)_registeres.size();
	snmp_c2locPortTable(port, _portCount);
}

void C2Env::refreshTransferPortTable()
{
    static ZQ::SNMP::Oid c2OidTbl;
    if (c2OidTbl.isNil())
        _mmib.reserveTable("transferPortTable", 8, c2OidTbl);
    if (c2OidTbl.isNil())
    {
        envlog(ZQ::common::Log::L_WARNING, CLOGFMT(C2Env,"refreshTransferPortTable() failed to locate table in MIB"));
        return;
    }

    ZQ::SNMP::Oid tmpOid(c2OidTbl);
    tmpOid.append(1);
    _mmib.removeSubtree(tmpOid);

    // update table
    TransferPortManager::PortInfos  transferPorts;
    if (!_portMgr)
    {
        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C2Env, "port manager is null"));
        return;
    }

    _portMgr->snapshotPortInfos(transferPorts);
    envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(C2Env, "snmp TransferPort table created, column[%d], transferPorts size[%d]"), 8, transferPorts.size());
    int rowIndex = 1;
    for (std::vector< TransferPortManager::PortInfo>::iterator it = transferPorts.begin(); 
        it != transferPorts.end(); ++it)
    {
        try
        {
            TransferPortManager::PortInfo & portInfoIt = *it;
            int isUp                = portInfoIt.isUp;	           
            int sessionCountTotal   = portInfoIt.sessionCountTotal;
            int capacity            = portInfoIt.capacity / 1000;           
            int activeTransferCount = portInfoIt.activeTransferCount;
            int enabled             = portInfoIt.enabled;            
            int penalty             = portInfoIt.penalty; 
            int activeBandwidth     = portInfoIt.activeBandwidth / 1000;

            std::string name(portInfoIt.name);
            std::string address(ZQ::common::Text::join(portInfoIt.addressListIPv4));
            address += "; ";
            address += ZQ::common::Text::join(portInfoIt.addressListIPv6);

            _mmib.addTableCell(c2OidTbl,  1, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("tpName", name));
            _mmib.addTableCell(c2OidTbl,  2, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("tpStatus", (int32)isUp));
            _mmib.addTableCell(c2OidTbl,  3, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("tpSession", (int32)activeTransferCount));
            _mmib.addTableCell(c2OidTbl,  4, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("tpBwSubtotal", (int32)activeBandwidth));
            _mmib.addTableCell(c2OidTbl,  5, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("tpBwMax", (int32)capacity));
            _mmib.addTableCell(c2OidTbl,  6, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("tpEnabled", (int32)enabled));
            _mmib.addTableCell(c2OidTbl,  7, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("tpPenalty", (int32)penalty));
            _mmib.addTableCell(c2OidTbl,  8, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("tpAddress", address));

            ++rowIndex;
        }
        catch (...) 
        {
            envlog(ZQ::common::Log::L_WARNING, CLOGFMT(C2Env, "snmp TransferPort table add data error, row[%d], transferPorts size[%d]"), rowIndex, transferPorts.size());
        }
    }
}
void C2Env::refreshClientTransferTable()
{
    static ZQ::SNMP::Oid c2OidTbl;
    if (c2OidTbl.isNil())
        _mmib.reserveTable("clientTransferTable", 4, c2OidTbl);
    if (c2OidTbl.isNil())
    {
        envlog(ZQ::common::Log::L_WARNING, CLOGFMT(C2Env,"refreshClientTransferTable() failed to locate table in MIB"));
        return;
    }

    ZQ::SNMP::Oid tmpOid(c2OidTbl);
    tmpOid.append(1);
    _mmib.removeSubtree(tmpOid);

    // update table
    typedef ::std::vector< ::TianShanIce::SCS::ClientTransfer >::iterator itClientTransfer;
    ::TianShanIce::SCS::ClientTransfers  clientTransfers;
    if (!_clientMgr)
    {
        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C2Env, "client manager is null"));
        return;
    }
    _clientMgr->snapshotTransfers(clientTransfers);
    envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ClientTransferTable, "snmp ClientTransfer table, colunm[%d], clientTransfers size[%d]"), 4, clientTransfers.size());

    int rowIndex = 1;
    for (itClientTransfer it = clientTransfers.begin(); it != clientTransfers.end(); ++it)
    {
        try
        {
            int consumedBandwidth = it->consumedBandwidth / 1000;
            int ingressCapacity = it->ingressCapacity / 1000;

            _mmib.addTableCell(c2OidTbl,  1, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("ctName", it->address));
            _mmib.addTableCell(c2OidTbl,  2, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("ctSession", it->activeTransferCount));
            _mmib.addTableCell(c2OidTbl,  3, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("ctBwSubtotal", (int32)consumedBandwidth));
            _mmib.addTableCell(c2OidTbl,  4, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("ctBwMax", (int32)ingressCapacity));

            ++rowIndex;
        }
        catch (...)
        {
            envlog(ZQ::common::Log::L_WARNING, CLOGFMT(ClientTransferTable, "snmp ClientTransfer table add data error, row[%d], clientTransfers size[%d]"), rowIndex, clientTransfers.size());
        }
    }
}

}// namespace CDN
}// namespace ZQTianShan
