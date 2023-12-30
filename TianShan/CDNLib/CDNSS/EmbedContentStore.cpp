#include <boost/thread.hpp>
#include "FileSystemOp.h"
#include <fstream>
#include <SystemUtils.h>
#include <FileLog.h>
#include <ContentImpl.h>
#include <CacheStoreImpl.h>
#include <NativeThreadPool.h>
#include <TianShanDefines.h>
#include "CdnSSConfig.h"
#include <TianShanIceHelper.h>
#include <TsStreamer.h>
#include <strHelper.h>
//#include "CdnssSnmpExp.h"
#include "CdnSvc.h"
//#include <iostream>
ZQTianShan::ContentStore::ContentStoreImpl::Ptr embedCS = 0;
ZQTianShan::ContentStore::CacheStoreImpl::Ptr cacheStore = 0;
ZQ::common::NativeThreadPool* embedCSThreadPool = 0;

ZQ::common::FileLog     ncsMainLog;
ZQ::common::FileLog     ncsEventLog;

ZQ::common::FileLog		cacheMainLog;

extern ZQ::StreamService::CdnSSSerice  gCdnSSServiceInstance;

class MountpointTypeChecker
{
public:
	MountpointTypeChecker(){}
	virtual ~MountpointTypeChecker(){}
public:
	bool init()
	{
		mMountTypeMap.clear();
		std::ifstream mounts;
		mounts.open("/proc/mounts");
		if(!mounts.is_open())
		{
			ncsMainLog(ZQ::common::Log::L_ERROR,"CAN'T OPEN /proc/mounts");
			return false;
		}
		char szline[2048];
		while(!mounts.eof())
		{
			szline[0] = 0;
			szline[sizeof(szline)-1]= 0;
			mounts.getline(szline,sizeof(szline)-1);

			if(strlen(szline)<=0 || szline[0] =='#')
				continue;// skip comment
	
			std::string line = 	szline;
			ZQ::common::stringHelper::TrimExtra(line," \t\r\n");
			
			std::vector<std::string> tmp;
			ZQ::common::stringHelper::SplitString(line,tmp," \t"," \t\r\n");
			if( tmp.size() < 3 )
				continue;
			const std::string& mountpath = tmp[1];
			const std::string& type = tmp[2];
			addnewdeviceinfo(mountpath,type);
		}
		mounts.close();
		return true;
	}	
	
	void addnewdeviceinfo( const std::string& path , const std::string& type )
	{
		struct stat st;
		memset(&st,sizeof(st),sizeof(st));
		if( stat(path.c_str() , &st) != 0 )
		{
			ncsMainLog(ZQ::common::Log::L_WARNING,CLOGFMT(MountpointTypeChecker,"failed to stat [%s]"),path.c_str());
			return;
		}
		mMountTypeMap[st.st_dev] = 	type;
	}
	bool isMountPathLegal( const std::string& path, const std::string& type)
	{
		if(path.empty() )
			return false;
		if( type.empty() )
			return true;
		
		struct stat st;
		memset(&st,sizeof(st),sizeof(st));
		if( stat(path.c_str() , &st) != 0 )
		{
			ncsMainLog(ZQ::common::Log::L_WARNING,CLOGFMT(MountpointTypeChecker,"failed to stat [%s]"),path.c_str());
			return false;
		}
		std::map<dev_t,std::string>::const_iterator it = mMountTypeMap.find(st.st_dev);
		if( it == mMountTypeMap.end())
			return false;
		return it->second == type;
	}
private:
	std::map<dev_t,std::string> mMountTypeMap;
};

bool isVolumeTypeLegal( const std::string& mountpoint, const std::string& type)
{
	if( mountpoint.empty() || type.empty() )
		return false;
	std::string mp = mountpoint;
	while(mp.length() > 1 )
	{
	}
	return false;
}

ZQ::common::NativeThreadPool cacheStoreThreadPool;

void setCacheStoreConfig( ZQTianShan::ContentStore::CacheStoreImpl* cache ) 
{
	if(!cache) 
		return;

	const ZQ::StreamService::CacheStoreConfig&	config = gCdnSSConfig.cacheStoreConfig;

	cache->_groupBind = ZQ::common::InetHostAddress( gCdnSSConfig.csEventMulticastLocalIp.c_str() );// use FileEvent multicast local bind ip as well
	
	cache->_timeWinOfPopular = config.popTimeWindow;
	cache->_countOfPopular = config.countOfPopular;
	cache->_hotTimeWin = config.hotWindow;
	
	cache->_heatbeatInterval = config.heartbeatInterval;
	cache->_maxCandidates = config.maxCandidates;
	cache->_pwrRanking = config.pwrRanking;
	cache->_prevLoadThreshold = config.prevLoadThreshold;
	cache->_successorLoadThreshold = config.successorLoadThreshold;
	cache->_maxUnpopular = config.maxUnpopular;
	cache->_penaltyOfFwdFail = config.forwardFailPenalty;
	cache->_penaltyMax = config.penaltyMax;
	cache->_defaultProvSessionBitrateKbps = config.defaultProvSessBitrate;
	cache->_minBitratePercent = config.minBitratePercent;
	cache->_defaultTransferServerPort = config.defaultTransferServerPort;
	cache->_totalProvisionKbps = config.totalProvBitrate;
	
	cache->_paidLength = config.paidLength;
	cache->_downStreamBindIP = config.downStreamBindIp;

	cache->_upStreamBindIP = config.upStreamBindip;
	cache->_flags = config.flags;
	cache->_cacheDir = config.downloadIndexTmpFilePath;
	cache->_thisDescriptor.desc.sessionInterface = config.thisSessInterface;
	cache->_bProxySessionBook = config.proxySessionBook >= 1;


	ZQTianShan::ContentStore::SourceStores::StoreInfo info;
	info.sessionInterface = config.defaultSessInterface;
	cache->_extSourceStores.setDefault( info );

	const std::vector<ZQ::StreamService::CacheStoreConfig::CacheProviderHolder>& providers = config.cacheProviders;
	std::vector< ZQ::StreamService::CacheStoreConfig::CacheProviderHolder >::const_iterator itProvider = providers.begin();
	for( ; itProvider != providers.end() ; itProvider ++ )
	{
		info.sessionInterface = itProvider->sessInterface;
		cache->_extSourceStores.set( itProvider->name , info );
	}
}

/*
int  RegisterCacheStoreSnmp( )
{
	int nRev = true;
	int registerCount = 0;
	//ZQ::Snmp::Subagent*  cdnssSnmp = gCdnSSServiceInstance.getCdnssSnmp();
	ZQ::common::ServiceMIB::Ptr _pServiceMib = gCdnSSServiceInstance.getCdnssMIB();

// 	if (NULL == cdnssSnmp)
// 	{
// 		nRev = false;
// 		cacheMainLog(ZQ::common::Log::L_WARNING, CLOGFMT(RegisterCacheStoreSnmp, "cdnss snmp RegisterCacheStoreSnmp, registerCount[%d], cdnssSnmp[NULL]"), registerCount);
// 		return nRev;
// 	}

	try
	{
// 		{".3", "CdnSSExt" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3)
// 		{".3.1", "CdnSSStat" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1)
// 		{".3.1.1", "hotContentsTbl" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).hotContentsTbl(1)
// 		{".3.1.1.1", "HotContentsEntry" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).hotContentsTbl(1).HotContentsEntry(1)
// 		{".3.1.1.1.1", "hcContentName" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).hotContentsTbl(1).HotContentsEntry(1).hcContentName(1)
// 		{".3.1.1.1.2", "hcAccessCount" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).hotContentsTbl(1).HotContentsEntry(1).hcAccessCount(2)
// 		{".3.1.1.1.3", "hcStampSince" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).hotContentsTbl(1).HotContentsEntry(1).hcStampSince(3)
// 		{".3.1.1.1.4", "hcStampLatest" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).hotContentsTbl(1).HotContentsEntry(1).hcStampLatest(4)
// 		{".3.1.11", "CdnSSStat-cache-Measure-Reset" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).CdnSSStat-cache-Measure-Reset(11)
// 		{".3.1.12", "CdnSSStat-cache-Measure-Since" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).CdnSSStat-cache-Measure-Since(12)
// 		{".3.1.13", "CdnSSStat-cache-missedSize" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).CdnSSStat-cache-missedSize(13)
// 		{".3.1.14", "CdnSSStat-cache-hotLocalsSize" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).CdnSSStat-cache-hotLocalsSize(14)
// 		{".3.1.15", "CdnSSStat-cache-requestsInTimeWin" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).CdnSSStat-cache-requestsInTimeWin(15)
// 		{".3.1.16", "CdnSSStat-cache-hitInTimeWin" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).CdnSSStat-cache-hitInTimeWin(16)
// 		{".3.1.2", "missedContentTable" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).missedContentTable(2)
// 		{".3.1.2.1", "MissedContentsEntry" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).missedContentTable(2).MissedContentsEntry(1)
// 		{".3.1.2.1.1", "mcContentName" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).missedContentTable(2).MissedContentsEntry(1).mcContentName(1)
// 		{".3.1.2.1.2", "mcAccessCount" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).missedContentTable(2).MissedContentsEntry(1).mcAccessCount(2)
// 		{".3.1.2.1.3", "mcStampSince" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).missedContentTable(2).MissedContentsEntry(1).mcStampSince(3)
// 		{".3.1.2.1.4", "mcStampLatest" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).missedContentTable(2).MissedContentsEntry(1).mcStampLatest(4)
// 		{".3.1.3", "cacheStoreNeighborTbl" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreNeighborTbl(3)
// 		{".3.1.3.1", "CacheStoreNeighbourEntry" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreNeighborTbl(3).CacheStoreNeighbourEntry(1)
// 		{".3.1.3.1.1", "csnNetId" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreNeighborTbl(3).CacheStoreNeighbourEntry(1).csnNetId(1)
// 		{".3.1.3.1.2", "csnState" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreNeighborTbl(3).CacheStoreNeighbourEntry(1).csnState(2)
// 		{".3.1.3.1.3", "csnLoad" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreNeighborTbl(3).CacheStoreNeighbourEntry(1).csnLoad(3)
// 		{".3.1.3.1.4", "csnLoadImport" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreNeighborTbl(3).CacheStoreNeighbourEntry(1).csnLoadImport(4)
// 		{".3.1.3.1.5", "csnLoadCacheWrite" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreNeighborTbl(3).CacheStoreNeighbourEntry(1).csnLoadCacheWrite(5)
// 		{".3.1.3.1.6", "csnStampAsOf" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreNeighborTbl(3).CacheStoreNeighbourEntry(1).csnStampAsOf(6)
// 		{".3.1.3.1.7", "csnEndpoint" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreNeighborTbl(3).CacheStoreNeighbourEntry(1).csnEndpoint(7)
// 		{".3.1.3.1.8", "csnSessionInterface" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreNeighborTbl(3).CacheStoreNeighbourEntry(1).csnSessionInterface(8)
// 		{".3.1.4", "cacheStoreStreamCounters" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreStreamCounters(4)
// 		{".3.1.4.1", "cacheStoreStreamCountersEntry" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreStreamCounters(4).cacheStoreStreamCountersEntry(1)
// 		{".3.1.4.1.1", "scrName" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreStreamCounters(4).cacheStoreStreamCountersEntry(1).scrName(1)
// 		{".3.1.4.1.2", "scrCount" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreStreamCounters(4).cacheStoreStreamCountersEntry(1).scrCount(2)
// 		{".3.1.4.1.3", "scrFailCount" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreStreamCounters(4).cacheStoreStreamCountersEntry(1).scrFailCount(3)
// 		{".3.1.4.1.4", "scrLatencyAvg" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreStreamCounters(4).cacheStoreStreamCountersEntry(1).scrLatencyAvg(4)
// 		{".3.1.4.1.5", "scrLatencyMax" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnSS(2100).CdnSSExt(3).CdnSSStat(1).cacheStoreStreamCounters(4).cacheStoreStreamCountersEntry(1).scrLatencyMax(5)

		//using namespace ZQ::Snmp;

		//ZQ::Snmp::ManagedPtr hotContentsTbl(new TableMediator<HotContents, ZQTianShan::ContentStore::CacheStoreImpl>(&cacheMainLog, ZQ::Snmp::Oid("1.1.1"), cdnssSnmp, *(cacheStore.get()) ));
		//cdnssSnmp->addObject(ZQ::Snmp::Oid("1.1.1"), ZQ::Snmp::ManagedPtr(hotContentsTbl));	  ++registerCount;

		ZQ::SNMP::Oid hotContentsTbl;
		_pServiceMib->reserveTable("hotContentsTbl", 4, hotContentsTbl);

		TianShanIce::Storage::AccessRegistrarPtr& acList = cacheStore->getContentHotLocals();
		TianShanIce::Storage::ContentCounterList sortedList;
		acList->sort(cacheStore->_timeWinOfPopular, false, 0, sortedList);
		for ( int sortNum = 0; sortNum < (int) sortedList.size(); ++sortNum)
		{
			int rowIndex           = 1 + sortNum;
			
			TianShanIce::Storage::ContentAccess & result = sortedList[sortNum].base;

			char stampSinceBuf[256];
			char stampLatestBuf[256];
			memset(stampSinceBuf, 0, sizeof(stampSinceBuf));
			memset(stampLatestBuf, 0, sizeof(stampLatestBuf));

			int  accessCount = result.accessCount;
			std::string contentName(result.contentName);
			std::string stampSince(ZQTianShan::TimeToUTC(result.stampSince, stampLatestBuf, sizeof(stampLatestBuf) - 2));
			std::string stampLatest(ZQTianShan::TimeToUTC(result.stampLatest, stampLatestBuf, sizeof(stampLatestBuf) - 2));

			_pServiceMib->addTableCell(hotContentsTbl,  1 , rowIndex,new ZQ::SNMP::SNMPObjectDupValue("hcContentName", contentName));
			_pServiceMib->addTableCell(hotContentsTbl,  2 , rowIndex,new ZQ::SNMP::SNMPObjectDupValue("hcAccessCount", accessCount));
			_pServiceMib->addTableCell(hotContentsTbl,  3 , rowIndex,new ZQ::SNMP::SNMPObjectDupValue("hcStampSince", stampSince));
			_pServiceMib->addTableCell(hotContentsTbl,  4 , rowIndex,new ZQ::SNMP::SNMPObjectDupValue("hcStampLatest", stampLatest));			
		}

		//ZQ::Snmp::ManagedPtr missedContentTable(new TableMediator<MissedContents, ZQTianShan::ContentStore::CacheStoreImpl>(&cacheMainLog, ZQ::Snmp::Oid("1.2.1"), cdnssSnmp, *(cacheStore.get()) ));
		//cdnssSnmp->addObject(ZQ::Snmp::Oid("1.2.1"), ZQ::Snmp::ManagedPtr(missedContentTable));	 ++registerCount;

		ZQ::SNMP::Oid missedContentTable;
		_pServiceMib->reserveTable("missedContentTable", 4, missedContentTable);

		TianShanIce::Storage::AccessRegistrarPtr& missedList = cacheStore->getContentMissed();
		sortedList.clear();
		missedList->sort(cacheStore->_timeWinOfPopular, false, 0, sortedList);
		for ( int sortNum = 0; sortNum < (int) sortedList.size(); ++sortNum)
		{
			int rowIndex           = 1 + sortNum;

			TianShanIce::Storage::ContentAccess & result = sortedList[sortNum].base;

			char stampSinceBuf[256];
			char stampLatestBuf[256];
			memset(stampSinceBuf, 0, sizeof(stampSinceBuf));
			memset(stampLatestBuf, 0, sizeof(stampLatestBuf));

			int  accessCount = result.accessCount;
			std::string contentName(result.contentName);
			std::string stampSince(ZQTianShan::TimeToUTC(result.stampSince, stampLatestBuf, sizeof(stampLatestBuf) - 2));
			std::string stampLatest(ZQTianShan::TimeToUTC(result.stampLatest, stampLatestBuf, sizeof(stampLatestBuf) - 2));

			_pServiceMib->addTableCell(missedContentTable,  1 , rowIndex,new ZQ::SNMP::SNMPObjectDupValue("mcContentName", contentName));
			_pServiceMib->addTableCell(missedContentTable,  2 , rowIndex,new ZQ::SNMP::SNMPObjectDupValue("mcAccessCount", accessCount));
			_pServiceMib->addTableCell(missedContentTable,  3 , rowIndex,new ZQ::SNMP::SNMPObjectDupValue("mcStampSince", stampSince));
			_pServiceMib->addTableCell(missedContentTable,  4 , rowIndex,new ZQ::SNMP::SNMPObjectDupValue("mcStampLatest", stampLatest));			
		}


		//ZQ::Snmp::ManagedPtr cacheStoreNeighborTbl(new TableMediator<CacheStoreNeighbour, ZQTianShan::ContentStore::CacheStoreImpl>(&cacheMainLog, ZQ::Snmp::Oid("1.3.1"), cdnssSnmp, *(cacheStore.get()) ));
		//cdnssSnmp->addObject(ZQ::Snmp::Oid("1.3.1"), ZQ::Snmp::ManagedPtr(cacheStoreNeighborTbl));   ++registerCount;
		ZQ::SNMP::Oid cacheStoreNeighborTbl;
		_pServiceMib->reserveTable("cacheStoreNeighborTbl", 8, cacheStoreNeighborTbl);
		ZQTianShan::ContentStore::CacheStoreImpl::CacheStoreListInt nbList;
		cacheStore->_listNeighorsEx(nbList);

		ZQTianShan::ContentStore::CacheStoreImpl::CacheStoreListInt::iterator nbListIter;
		int rowIndex = 0;
		for(nbListIter = nbList.begin(); nbListIter != nbList.end(); ++nbListIter)
		{
			++rowIndex;
			char stampAsOfBuf[256];
			memset(stampAsOfBuf, 0, sizeof(stampAsOfBuf));
			ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex(rowIndex);
			TianShanIce::Storage::CacheStoreDescriptor & csdRef = nbListIter->desc;

			int state = csdRef.state;
			int load  = csdRef.loadStream;
			int loadImport  = csdRef.loadImport;
			int loadCacheWrite = csdRef.loadCacheWrite;

			std::string netId(csdRef.netId);
			std::string endpoint("exception");
			std::string sessionInterface(csdRef.sessionInterface);
			std::string stampAsOf(ZQTianShan::TimeToUTC(csdRef.stampAsOf, stampAsOfBuf, sizeof(stampAsOfBuf) - 2));

			TianShanIce::Storage::ContentStorePrx cacheStorePrx = csdRef.theStore->theContentStore();
			if(NULL != cacheStorePrx.get())
				endpoint = cacheStorePrx->getNetId();
			else
				cacheMainLog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreNeighbour, "cdnss snmp create CacheStoreNeighbour, row[%d], acList size[%d], cacheStorePrx[NULL], endpoint[exception]"), rowIndex, acList->size());

			_pServiceMib->addTableCell(cacheStoreNeighborTbl,  1, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("csnNetId", netId));			
			_pServiceMib->addTableCell(cacheStoreNeighborTbl,  2, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("csnState", state));			
			_pServiceMib->addTableCell(cacheStoreNeighborTbl,  3, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("csnLoad", load));			
			_pServiceMib->addTableCell(cacheStoreNeighborTbl,  4, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("csnLoadImport", loadImport));			
			_pServiceMib->addTableCell(cacheStoreNeighborTbl,  5, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("csnLoadCacheWrite", loadCacheWrite));			
			_pServiceMib->addTableCell(cacheStoreNeighborTbl,  6, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("csnStampAsOf", stampAsOf));			
			_pServiceMib->addTableCell(cacheStoreNeighborTbl,  7, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("csnEndpoint", endpoint));			
			_pServiceMib->addTableCell(cacheStoreNeighborTbl,  8, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("csnSessionInterface", sessionInterface));
		}

		//ZQ::Snmp::ManagedPtr cacheStoreStreamCountersTable(new TableMediator<cacheStoreStreamCounters, ZQTianShan::ContentStore::CacheStoreImpl>(&cacheMainLog, ZQ::Snmp::Oid("1.3.1"), cdnssSnmp, *(cacheStore.get()) ));
		//cdnssSnmp->addObject(ZQ::Snmp::Oid("1.4.1"), ZQ::Snmp::ManagedPtr(cacheStoreStreamCountersTable));	++registerCount;

		ZQ::SNMP::Oid cacheStoreStreamCountersTable;
		_pServiceMib->reserveTable("cacheStoreStreamCounters", 5, cacheStoreStreamCountersTable);
		rowIndex = 0;
		for(int indexCount = 0; indexCount < (int)(ZQTianShan::ContentStore::CacheStoreImpl::ec_max); ++indexCount)
		{
			++rowIndex;
			const ZQTianShan::ContentStore::CacheStoreImpl::ExportCount & exportCounter = cacheStore->_exportCounters[indexCount];			

			std::string ecName(exportCounter.name);
			int ecCount(exportCounter.count);
			int ecFailCount(exportCounter.failCount);
			int ecLatencyAvg(0);
			int ecLatencyMax(exportCounter.latencyMax);

			if (0 < ecCount)
				ecLatencyAvg = exportCounter.latencyTotal / ecCount;

			_pServiceMib->addTableCell(cacheStoreStreamCountersTable,  1, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("scrName", ecName));
			_pServiceMib->addTableCell(cacheStoreStreamCountersTable,  2, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("scrCount", ecCount));
			_pServiceMib->addTableCell(cacheStoreStreamCountersTable,  3, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("scrFailCount", ecFailCount));
			_pServiceMib->addTableCell(cacheStoreStreamCountersTable,  4, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("scrLatencyAvg", ecLatencyAvg));
			_pServiceMib->addTableCell(cacheStoreStreamCountersTable,  5, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("scrLatencyMax", ecLatencyMax));

			cacheMainLog(ZQ::common::Log::L_DEBUG, CLOGFMT(cacheStoreStreamCounters, "cdnss snmp create cacheStoreStreamCounters, row[%d], name[%s] count[%d] failCount[%d] latencyTotal[%d] latencyMax[%d]"),
				rowIndex, exportCounter.name, exportCounter.count, exportCounter.failCount, exportCounter.latencyTotal,  exportCounter.latencyMax);
		}


		//ZQTianShan::ContentStore::CacheStoreImpl * cacheStoreTmp = cacheStore.get();
		//cdnssSnmp->addObject( Oid("1.11"), ManagedPtr(new SimpleObject(VariablePtr(new CdnssCacheMeasureReset( *cacheStoreTmp)),  AsnType_Integer, aReadWrite)));  ++registerCount;
		//cdnssSnmp->addObject( Oid("1.12"), ManagedPtr(new SimpleObject(VariablePtr(new CdnssCacheMeasureSince( *cacheStoreTmp)),  AsnType_Octets, aReadOnly)));   ++registerCount;

		//SvcMIB_ExportReadOnlyVar("CdnSSStat-cache-Measure-Reset", &cacheStore->)
		//ResetWrapper
		ZQ::SNMP::SNMPObject::Ptr obj = new ZQ::SNMP::SNMPObjectByAPI<ZQTianShan::ContentStore::CacheStoreImpl, uint32>("CdnSSStat-cache-Measure-Reset", *cacheStore.get(), ZQ::SNMP::AsnType_Int32,
			&ZQTianShan::ContentStore::CacheStoreImpl::snmpDummyGet, &ZQTianShan::ContentStore::CacheStoreImpl::snmpResetCounters);
		char snmpMeasuredSince[80] = {0};
		std::string measureSince(ZQTianShan::TimeToUTC(cacheStore->_stampMesureSince, snmpMeasuredSince, sizeof(snmpMeasuredSince)-2));
		static std::string strMeasuredSince = snmpMeasuredSince;
		SvcMIB_ExportReadOnlyVar("CdnSSStat-cache-Measure-Since",&strMeasuredSince,AsnType_String, "");
		SvcMIB_ExportReadOnlyVar("CdnSSStat-cache-missedSize",&cacheStore->_sizeMissed,AsnType_Int32, "");
		SvcMIB_ExportReadOnlyVar("CdnSSStat-cache-hotLocalsSize",&cacheStore->_sizeHotLocals,AsnType_Int32, "");
		SvcMIB_ExportReadOnlyVar("CdnSSStat-cache-requestsInTimeWin",&cacheStore->_reqsMissedInWin,AsnType_Int32, "");
		SvcMIB_ExportReadOnlyVar("CdnSSStat-cache-hitInTimeWin",&cacheStore->_reqsHotLocalsInWin,AsnType_Int32, "");

		cacheMainLog(ZQ::common::Log::L_WARNING, CLOGFMT(RegisterCacheStoreSnmp, "cdnss snmp RegisterCacheStoreSnmp, registerCount[%d]"), registerCount);
	}
	catch (const ::Ice::Exception& ex)
	{
		nRev = false;
		cacheMainLog(ZQ::common::Log::L_WARNING, CLOGFMT(RegisterCacheStoreSnmp, "cdnss snmp create cacheStoreStreamCounters, registerCount[%d], caught exception[%s]"), registerCount, ex.ice_name().c_str());
	}
	catch(...)
	{
		nRev = false;
		cacheMainLog(ZQ::common::Log::L_ERROR,CLOGFMT(RegisterCacheStoreSnmp,"failed to regester CacheStore snmp, registerCount[%d], caught exception[unknown]"), registerCount);
	}

   return nRev;
}
*/

void cache_resetSnmpStat(ZQ::common::ServiceMIB::Ptr pMib)
{
	if (!pMib || !cacheStore)
		return;

	cacheStore->resetCounters();
	char snmpMeasuredSince[80] = {0};
	static std::string strMeasuredSince;
	strMeasuredSince = ZQTianShan::TimeToUTC(cacheStore->_stampMesureSince, snmpMeasuredSince, sizeof(snmpMeasuredSince)-2);
	pMib->addObject(new ZQ::SNMP::SNMPObject("cdnssStat-cache-Measure-Since", strMeasuredSince));
}

void cache_refreshSnmpStat(ZQ::common::ServiceMIB::Ptr pMib, int iStep)
{
	if (!pMib || !cacheStore)
		return;

	int rowIndex = 0;
	try
	{
		TianShanIce::Storage::ContentCounterList sortedList;

		switch(iStep)
		{
		case 0:
			{
				static bool bRegistered = false;
				if (!bRegistered)
				{
					//{".2.1.20", "cdnssStat-cache-missedSize" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cdnssStat-cache-missedSize(20)
					//{".2.1.21", "cdnssStat-cache-hotLocalsSize" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cdnssStat-cache-hotLocalsSize(21)
					//{".2.1.22", "cdnssStat-cache-requestsInTimeWin" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cdnssStat-cache-requestsInTimeWin(22)
					//{".2.1.23", "cdnssStat-cache-hitInTimeWin" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cdnssStat-cache-hitInTimeWin(23)
					pMib->addObject(new ZQ::SNMP::SNMPObject("cdnssStat-cache-missedSize",       cacheStore->_sizeMissed));
					pMib->addObject(new ZQ::SNMP::SNMPObject("cdnssStat-cache-hotLocalsSize",    cacheStore->_sizeHotLocals));
					pMib->addObject(new ZQ::SNMP::SNMPObject("cdnssStat-cache-requestsInTimeWin",cacheStore->_reqsMissedInWin));
					pMib->addObject(new ZQ::SNMP::SNMPObject("cdnssStat-cache-hitInTimeWin",     cacheStore->_reqsHotLocalsInWin));
					bRegistered = true;
				}
			}
			break;

		case 1:
			//{".2.1.100", "hotContentsTbl" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).hotContentsTbl(100)
			//{".2.1.100.1", "hotContent" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).hotContentsTbl(100).hotContent(1)
			//{".2.1.100.1.1", "hcContentName" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).hotContentsTbl(100).hotContent(1).hcContentName(1)
			//{".2.1.100.1.2", "hcAccessCount" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).hotContentsTbl(100).hotContent(1).hcAccessCount(2)
			//{".2.1.100.1.3", "hcStampSince" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).hotContentsTbl(100).hotContent(1).hcStampSince(3)
			//{".2.1.100.1.4", "hcStampLatest" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).hotContentsTbl(100).hotContent(1).hcStampLatest(4)
			{
				ZQ::SNMP::Oid hotContentsTbl;
				pMib->reserveTable("hotContentsTbl", 4, hotContentsTbl);

				TianShanIce::Storage::AccessRegistrarPtr& acList = cacheStore->getContentHotLocals();
				acList->sort(cacheStore->_timeWinOfPopular, false, 0, sortedList);
				for (int sortNum = 0; sortNum < (int) sortedList.size(); ++sortNum)
				{
					rowIndex           = 1 + sortNum;

					TianShanIce::Storage::ContentAccess & result = sortedList[sortNum].base;

					char stampSinceBuf[256];
					char stampLatestBuf[256];
					memset(stampSinceBuf, 0, sizeof(stampSinceBuf));
					memset(stampLatestBuf, 0, sizeof(stampLatestBuf));

					int  accessCount = result.accessCount;
					std::string contentName(result.contentName);
					std::string stampSince(ZQTianShan::TimeToUTC(result.stampSince, stampLatestBuf, sizeof(stampLatestBuf) - 2));
					std::string stampLatest(ZQTianShan::TimeToUTC(result.stampLatest, stampLatestBuf, sizeof(stampLatestBuf) - 2));

					pMib->addTableCell(hotContentsTbl,  1 , rowIndex,new ZQ::SNMP::SNMPObjectDupValue("hcContentName", contentName));
					pMib->addTableCell(hotContentsTbl,  2 , rowIndex,new ZQ::SNMP::SNMPObjectDupValue("hcAccessCount", accessCount));
					pMib->addTableCell(hotContentsTbl,  3 , rowIndex,new ZQ::SNMP::SNMPObjectDupValue("hcStampSince",  stampSince));
					pMib->addTableCell(hotContentsTbl,  4 , rowIndex,new ZQ::SNMP::SNMPObjectDupValue("hcStampLatest", stampLatest));			
				}
			}
			break;

		case 2:
			//{".2.1.101", "missedContentTable" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).missedContentTable(101)
			//{".2.1.101.1", "missedContent" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).missedContentTable(101).missedContent(1)
			//{".2.1.101.1.1", "mcContentName" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).missedContentTable(101).missedContent(1).mcContentName(1)
			//{".2.1.101.1.2", "mcAccessCount" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).missedContentTable(101).missedContent(1).mcAccessCount(2)
			//{".2.1.101.1.3", "mcStampSince" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).missedContentTable(101).missedContent(1).mcStampSince(3)
			//{".2.1.101.1.4", "mcStampLatest" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).missedContentTable(101).missedContent(1).mcStampLatest(4)
			{
				ZQ::SNMP::Oid missedContentTable;
				pMib->reserveTable("missedContentTable", 4, missedContentTable);

				TianShanIce::Storage::AccessRegistrarPtr& missedList = cacheStore->getContentMissed();
				sortedList.clear();
				missedList->sort(cacheStore->_timeWinOfPopular, false, 0, sortedList);
				for (int sortNum = 0; sortNum < (int) sortedList.size(); ++sortNum)
				{
					rowIndex           = 1 + sortNum;

					TianShanIce::Storage::ContentAccess & result = sortedList[sortNum].base;

					char stampSinceBuf[256];
					char stampLatestBuf[256];
					memset(stampSinceBuf, 0, sizeof(stampSinceBuf));
					memset(stampLatestBuf, 0, sizeof(stampLatestBuf));

					int  accessCount = result.accessCount;
					std::string contentName(result.contentName);
					std::string stampSince(ZQTianShan::TimeToUTC(result.stampSince, stampLatestBuf, sizeof(stampLatestBuf) - 2));
					std::string stampLatest(ZQTianShan::TimeToUTC(result.stampLatest, stampLatestBuf, sizeof(stampLatestBuf) - 2));

					pMib->addTableCell(missedContentTable,  1 , rowIndex,new ZQ::SNMP::SNMPObjectDupValue("mcContentName", contentName));
					pMib->addTableCell(missedContentTable,  2 , rowIndex,new ZQ::SNMP::SNMPObjectDupValue("mcAccessCount", accessCount));
					pMib->addTableCell(missedContentTable,  3 , rowIndex,new ZQ::SNMP::SNMPObjectDupValue("mcStampSince",  stampSince));
					pMib->addTableCell(missedContentTable,  4 , rowIndex,new ZQ::SNMP::SNMPObjectDupValue("mcStampLatest", stampLatest));			
				}
			}
			break;

		case 3:
			//{".2.1.102", "cacheStoreNeighborTbl" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreNeighborTbl(102)
			//{".2.1.102.1", "cacheStoreNeighbor" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreNeighborTbl(102).cacheStoreNeighbor(1)
			//{".2.1.102.1.1", "csnNetId" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreNeighborTbl(102).cacheStoreNeighbor(1).csnNetId(1)
			//{".2.1.102.1.2", "csnState" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreNeighborTbl(102).cacheStoreNeighbor(1).csnState(2)
			//{".2.1.102.1.3", "csnLoad" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreNeighborTbl(102).cacheStoreNeighbor(1).csnLoad(3)
			//{".2.1.102.1.4", "csnLoadImport" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreNeighborTbl(102).cacheStoreNeighbor(1).csnLoadImport(4)
			//{".2.1.102.1.5", "csnLoadCacheWrite" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreNeighborTbl(102).cacheStoreNeighbor(1).csnLoadCacheWrite(5)
			//{".2.1.102.1.6", "csnStampAsOf" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreNeighborTbl(102).cacheStoreNeighbor(1).csnStampAsOf(6)
			//{".2.1.102.1.7", "csnEndpoint" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreNeighborTbl(102).cacheStoreNeighbor(1).csnEndpoint(7)
			//{".2.1.102.1.8", "csnSessionInterface" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreNeighborTbl(102).cacheStoreNeighbor(1).csnSessionInterface(8)
			{
				ZQ::SNMP::Oid cacheStoreNeighborTbl;
				pMib->reserveTable("cacheStoreNeighborTbl", 8, cacheStoreNeighborTbl);
				ZQTianShan::ContentStore::CacheStoreImpl::CacheStoreListInt nbList;
				cacheStore->_listNeighorsEx(nbList);

				ZQTianShan::ContentStore::CacheStoreImpl::CacheStoreListInt::iterator nbListIter;
				for(nbListIter = nbList.begin(); nbListIter != nbList.end(); ++nbListIter)
				{
					++rowIndex;
					char stampAsOfBuf[256];
					memset(stampAsOfBuf, 0, sizeof(stampAsOfBuf));
					// ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex(rowIndex);
					TianShanIce::Storage::CacheStoreDescriptor & csdRef = nbListIter->desc;

					int state = csdRef.state;
					int load  = csdRef.loadStream;
					int loadImport  = csdRef.loadImport;
					int loadCacheWrite = csdRef.loadCacheWrite;

					std::string netId(csdRef.netId);
					std::string endpoint("exception");
					std::string sessionInterface(csdRef.sessionInterface);
					std::string stampAsOf(ZQTianShan::TimeToUTC(csdRef.stampAsOf, stampAsOfBuf, sizeof(stampAsOfBuf) - 2));

					TianShanIce::Storage::ContentStorePrx cacheStorePrx = csdRef.theStore->theContentStore();
					if(NULL != cacheStorePrx.get())
						endpoint = cacheStorePrx->getNetId();
					else
						cacheMainLog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreNeighbour, "cache_refreshSnmpStat() CacheStoreNeighbour, row[%d]"), rowIndex);

					pMib->addTableCell(cacheStoreNeighborTbl,  1, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("csnNetId", netId));			
					pMib->addTableCell(cacheStoreNeighborTbl,  2, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("csnState", state));			
					pMib->addTableCell(cacheStoreNeighborTbl,  3, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("csnLoad", load));			
					pMib->addTableCell(cacheStoreNeighborTbl,  4, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("csnLoadImport", loadImport));			
					pMib->addTableCell(cacheStoreNeighborTbl,  5, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("csnLoadCacheWrite", loadCacheWrite));			
					pMib->addTableCell(cacheStoreNeighborTbl,  6, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("csnStampAsOf", stampAsOf));			
					pMib->addTableCell(cacheStoreNeighborTbl,  7, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("csnEndpoint", endpoint));			
					pMib->addTableCell(cacheStoreNeighborTbl,  8, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("csnSessionInterface", sessionInterface));
				}
			}
			break;

		case 4:
			//{".2.1.103", "cacheStoreStreamCounters" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreStreamCounters(103)
			//{".2.1.103.1", "cacheStoreStreamCtr" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreStreamCounters(103).cacheStoreStreamCtr(1)
			//{".2.1.103.1.1", "scrName" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreStreamCounters(103).cacheStoreStreamCtr(1).scrName(1)
			//{".2.1.103.1.2", "scrCount" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreStreamCounters(103).cacheStoreStreamCtr(1).scrCount(2)
			//{".2.1.103.1.3", "scrFailCount" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreStreamCounters(103).cacheStoreStreamCtr(1).scrFailCount(3)
			//{".2.1.103.1.4", "scrLatencyAvg" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreStreamCounters(103).cacheStoreStreamCtr(1).scrLatencyAvg(4)
			//{".2.1.103.1.5", "scrLatencyMax" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).cdnss(2100).cdnssSvcApp(2).cdnssAttr(1).cacheStoreStreamCounters(103).cacheStoreStreamCtr(1).scrLatencyMax(5)
			{
				ZQ::SNMP::Oid cacheStoreStreamCountersTable;
				pMib->reserveTable("cacheStoreStreamCounters", 5, cacheStoreStreamCountersTable);
				rowIndex = 0;
				for(int indexCount = 0; indexCount < (int)(ZQTianShan::ContentStore::CacheStoreImpl::ec_max); ++indexCount)
				{
					++rowIndex;
					const ZQTianShan::ContentStore::CacheStoreImpl::ExportCount & exportCounter = cacheStore->_exportCounters[indexCount];			

					std::string ecName(exportCounter.name);
					int ecCount(exportCounter.count);
					int ecFailCount(exportCounter.failCount);
					int ecLatencyAvg(0);
					int ecLatencyMax(exportCounter.latencyMax);

					if (0 < ecCount)
						ecLatencyAvg = exportCounter.latencyTotal / ecCount;

					pMib->addTableCell(cacheStoreStreamCountersTable,  1, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("scrName", ecName));
					pMib->addTableCell(cacheStoreStreamCountersTable,  2, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("scrCount", ecCount));
					pMib->addTableCell(cacheStoreStreamCountersTable,  3, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("scrFailCount", ecFailCount));
					pMib->addTableCell(cacheStoreStreamCountersTable,  4, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("scrLatencyAvg", ecLatencyAvg));
					pMib->addTableCell(cacheStoreStreamCountersTable,  5, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("scrLatencyMax", ecLatencyMax));

					cacheMainLog(ZQ::common::Log::L_DEBUG, CLOGFMT(cacheStoreStreamCounters, "cache_refreshSnmpStat() create cacheStoreStreamCounters, row[%d], name[%s] count[%d] failCount[%d] latencyTotal[%d] latencyMax[%d]"),
						rowIndex, exportCounter.name, exportCounter.count, exportCounter.failCount, exportCounter.latencyTotal,  exportCounter.latencyMax);
				}
			}
			break;
		}// switch

		cacheMainLog(ZQ::common::Log::L_INFO, CLOGFMT(RegisterCacheStoreSnmp, "cache_refreshSnmpStat() iStep[%d] rows[%d]"), iStep, rowIndex);
	}
	catch (const ::Ice::Exception& ex)
	{
		cacheMainLog(ZQ::common::Log::L_WARNING, CLOGFMT(RegisterCacheStoreSnmp, "cache_refreshSnmpStat() iStep[%d] failed: rows[%d], caught exception[%s]"), iStep, rowIndex, ex.ice_name().c_str());
	}
	catch(...)
	{
		cacheMainLog(ZQ::common::Log::L_ERROR,CLOGFMT(RegisterCacheStoreSnmp,"cache_refreshSnmpStat() iStep[%d] failed: rows[%d], caught exception"), iStep, rowIndex);
	}
}

bool StartCachestore( ZQADAPTER_DECLTYPE& adapter, const std::string& serviceName)
{
	if(gCdnSSConfig.cacheStoreConfig.enable <= 0 )
	{
		cacheMainLog(ZQ::common::Log::L_INFO,CLOGFMT(cacheStore,"cachestore is disbaled"));
		return true;
	}
	
	cacheMainLog(ZQ::common::Log::L_INFO, "======================== start embeded CacheStore =====================" );
	std::string endpoint = gCdnSSConfig.serviceEndpoint;
	if(endpoint.find(':') == std::string::npos)
	{
		endpoint = serviceName + ":" + endpoint;
	}
	
	TianShanIce::Streamer::StreamServicePrx strmService = TianShanIce::Streamer::StreamServicePrx::uncheckedCast(
			adapter->getCommunicator()->stringToProxy( endpoint ));
			
	cacheStore = new ZQTianShan::ContentStore::CacheStoreImpl( cacheMainLog, ncsEventLog, serviceName.c_str(), *embedCS.get(), strmService,  cacheStoreThreadPool );
	embedCS->_edgeMode = true;
	setCacheStoreConfig( cacheStore.get() );

	if(!cacheStore->doInit())
	{
		cacheStore = 0;
		cacheMainLog(ZQ::common::Log::L_ERROR,CLOGFMT(CacheStore,"failed to initialize CacheStore"));
		return false;
	}
	
	//if(!RegisterCacheStoreSnmp())
		//cacheMainLog(ZQ::common::Log::L_ERROR,CLOGFMT(CacheStore,"failed to Register CacheStore Snmp"));
	
	return true;
}

void StopCacheStore()
{
	if(cacheStore)
	{
		cacheMainLog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStore,"stoping cache store"));
		cacheStore->doUninit();
		cacheStore = 0;
	}
}

Ice::ObjectPrx StartContentStore(ZQADAPTER_DECLTYPE& adapter, const std::string& serviceName, const std::string& logDir, ZQ::common::Log& log )
{
    try
	{
        std::string file = logDir + "/" + serviceName+"_CS.log";
        ncsMainLog.open(file.c_str(), gCdnSSConfig.mainLogLevel, ZQLOG_DEFAULT_FILENUM, 
                        gCdnSSConfig.mainLogFileSize, gCdnSSConfig.mainLogBufferSize, gCdnSSConfig.mainLogTimeout);
    }
    catch(const ZQ::common::FileLogException& ex) {
        log(ZQ::common::Log::L_ERROR,"can't create CsMain log :%s",ex.what());
        return 0;
    }

	try
	{
		std::string file = logDir + "/" + serviceName+"_CH.log";
		cacheMainLog.open( file.c_str(), gCdnSSConfig.mainLogLevel, ZQLOG_DEFAULT_FILENUM,
						gCdnSSConfig.mainLogFileSize, gCdnSSConfig.mainLogBufferSize, gCdnSSConfig.mainLogTimeout);
	}
	catch( const ZQ::common::FileLogException& ex)
	{
		log(ZQ::common::Log::L_ERROR,"can't create CacheMain log:%s",ex.what());
		return 0;
	}
	
    ncsMainLog(ZQ::common::Log::L_INFO, "======================== start embeded ContentStore =====================" );

    try {
        std::string file = logDir +  "/" + serviceName+"_CSEvent.log";
        ncsEventLog.open(file.c_str(), gCdnSSConfig.eventLogLevel, ZQLOG_DEFAULT_FILENUM, 
                         gCdnSSConfig.eventLogFileSize, gCdnSSConfig.eventLogBufferSize, gCdnSSConfig.eventLogTimeout);
    }
    catch(const ZQ::common::FileLogException& ex) {
        log(ZQ::common::Log::L_ERROR,"can't create CsEvent log :%s",ex.what());
        return 0;
    }

    std::ostringstream oss;
    oss << gCdnSSConfig.dbPath << "/" << serviceName;
    if (!FS::createDirectory(oss.str())) {
        ncsMainLog(ZQ::common::Log::L_ERROR, CLOGFMT(NativeService, "Data db path %s error"), oss.str().c_str());
        return false;
    }
	int32 poolSize = gCdnSSConfig.csThreadPoolSize;
	poolSize = MAX(2,poolSize);
    embedCSThreadPool = new ZQ::common::NativeThreadPool(poolSize);
    try 
	{
         embedCS = new ZQTianShan::ContentStore::ContentStoreImpl(ncsMainLog, ncsEventLog,  
                                                *embedCSThreadPool, adapter, oss.str().c_str());

        if (!embedCS) 
		{
            ncsMainLog(ZQ::common::Log::L_ERROR, CLOGFMT(NativeService, "create content store object failed."));
            return false;
        }

        embedCS->_netId       = gCdnSSConfig.netId;
        embedCS->_replicaGroupId = gCdnSSConfig.csStrReplicaGroupId;
        embedCS->_replicaId = gCdnSSConfig.netId;
        embedCS->_replicaPriority = gCdnSSConfig.csReplicaPriority;
        embedCS->_replicaTimeout = gCdnSSConfig.csReplicaTimeout;
        embedCS->_cacheLevel = gCdnSSConfig.cacheLevel;
        embedCS->_cacheable = gCdnSSConfig.isCacheMode; 
        embedCS->_contentEvictorSize = gCdnSSConfig.contentEvictorSize;
        embedCS->_volumeEvictorSize = gCdnSSConfig.volumeEvictorSize;
        embedCS->_storeFlags = STOREFLAG_populateSubfolders;
        embedCS->_timeoutNotProvisioned = gCdnSSConfig.csTimeoutNotProvisioned;
        embedCS->_timeoutIdleProvisioning = gCdnSSConfig.csTimeoutIdleProvisioning;
       
        if(!embedCS->initializeContentStore()) {
            ncsMainLog(ZQ::common::Log::L_ERROR,"failed to initialize content store");
            return false;
        }


        if(!gCdnSSConfig.csMasterEndpoint.empty()) {
            std::ostringstream oss;
            oss << SERVICE_NAME_ContentStore << ":" << gCdnSSConfig.csMasterEndpoint;
            embedCS->subscribeStoreReplica(oss.str(), true); 
        }
        
        bool bMounted = false;
		int64 startTime = ZQ::common::now();
		int64 timeoutInterval = gCdnSSConfig.csVolCheckTimeout;
		timeoutInterval = MAX( 2000, timeoutInterval );
		do
		{
			MountpointTypeChecker typechecker;
        	typechecker.init();
			ZQ::StreamService::CdnSSConfig::Volumes::const_iterator iter = gCdnSSConfig.volumes.begin();
			for(; iter != gCdnSSConfig.volumes.end(); ++iter) 
			{
				std::string path = iter->path;
				if(iter->path.at(iter->path.length()-1) != FNSEPC) {
					path += FNSEPC;		
				}
				if(!typechecker.isMountPathLegal(path,iter->fstype) )
				{
					ncsMainLog(ZQ::common::Log::L_WARNING,"path[%s] is not mounted with type[%s]",path.c_str(),iter->fstype.c_str());
					log(ZQ::common::Log::L_WARNING,"path[%s] is not mounted with type[%s]",path.c_str(),iter->fstype.c_str());
					continue;
				}
				ncsMainLog(ZQ::common::Log::L_INFO,"Mount volume [%s] with path[%s] type[%s]",
					iter->name.c_str() , path.c_str(), iter->fstype.c_str());
				bMounted = true;					
				embedCS->mountStoreVolume(iter->name, path, iter->isDefault);
				break;
			}
			//ZQ::common::delay(10*1000);
		}while( 0  );	
		if(!bMounted)
		{
			log(ZQ::common::Log::L_ERROR,"no volume is mounted, quit");
			return NULL;
		}
        embedCS->setFlags(STOREFLAG_checkFSonOpenContentByFullName);
    }
    catch( const Ice::Exception& ex) {
        log(ZQ::common::Log::L_ERROR , "Create ContenStoreImpl failed:%s", ex.ice_name().c_str() );
        return NULL;
    }

	if(!StartCachestore(adapter, serviceName) )
   {
	   return false;
   }


    Ice::Identity Id = adapter->getCommunicator()->stringToIdentity(SERVICE_NAME_ContentStore);
    return adapter->createProxy(Id);
}

bool StopContentStore (ZQ::common::Log& log) {   
    ncsMainLog(ZQ::common::Log::L_INFO, "******************************* Stop MediaServer ContentStore *******************************" );
       
   StopCacheStore();	
    if(embedCS) {
        embedCS->unInitializeContentStore();
//        embedCS = 0;
    }

    if(embedCSThreadPool) {
       embedCSThreadPool->stop();
        
        for( int i= 0 ; i< 100 ; i++) {
            if(embedCSThreadPool->activeCount() <= 0) {
                break;
            }
            SYS::sleep(100);
        }
        
        delete embedCSThreadPool;
        embedCSThreadPool = 0;
		embedCS = 0;
    }

//  if( ncsMainLog )
//  {
//      delete ncsMainLog;
//      ncsMainLog =NULL;
//  }
// 
//  if( ncsEventLog )
//  {
//      delete ncsEventLog;
//      ncsEventLog = NULL;
//  }

    return true;
}

