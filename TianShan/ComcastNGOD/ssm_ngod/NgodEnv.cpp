#include "NgodEnv.h"
#include "NgodConfig.h"

#include "TimeUtil.h"

namespace NGOD
{

NgodEnv::NgodEnv(ZQ::common::Log& mainLogger, ZQ::common::Log& eventLogger)
:mMainLogger(&mainLogger), mEventLogger(&eventLogger),
mSelManager(mSelEnv),
mLastEventRecvTime(0),
_mmib(mainLogger, 1000, 3), _snmpSA(mainLogger, _mmib, 5000)
{
	mLastEventRecvTime = ZQ::common::now();

	// register the SNMP exports
	_mmib.addObject(new ZQ::SNMP::SNMPObjectByAPI<NgodEnv, uint32>("sopReset", *this, ZQ::SNMP::AsnType_Int32, &NGOD::NgodEnv::snmp_dummyGet, &NGOD::NgodEnv::snmp_refreshSOPUsage));

	// writeable log levels
	_mmib.addObject(new ZQ::SNMP::SNMPObjectByAPI<NgodEnv, uint32>("rtspLogNgod2",      *this, ZQ::SNMP::AsnType_Int32, &NGOD::NgodEnv::snmp_getLogLevel_Main, &NGOD::NgodEnv::snmp_setLogLevel_Main));
	// _mmib.addObject(new ZQ::SNMP::SNMPObjectByAPI<NgodService, uint32>("rtspLogNgod2Ice",   *this, ZQ::SNMP::AsnType_Int32, &NGOD::NgodEnv::snmp_getLogLevel_Ice, &NGOD::NgodEnv::snmp_setLogLevel_Ice));
	_mmib.addObject(new ZQ::SNMP::SNMPObjectByAPI<NgodEnv, uint32>("rtspLogNgod2Event", *this, ZQ::SNMP::AsnType_Int32, &NGOD::NgodEnv::snmp_getLogLevel_Event, &NGOD::NgodEnv::snmp_setLogLevel_Event));

	//{".3.6", "ngod2-LogFile-size" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspExtNGOD(3).ngod2-LogFile-size(6)
	//{".3.7", "ngod2-LogFile-level" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspExtNGOD(3).ngod2-LogFile-level(7)
	//{".3.8", "ngod2-eventChannel-endpoint" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspExtNGOD(3).ngod2-eventChannel-endpoint(8)
	//{".3.9", "ngod2-Bind-endpoint" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspExtNGOD(3).ngod2-Bind-endpoint(9)
	//{".3.10", "ngod2-RTSPSession-timeout" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspExtNGOD(3).ngod2-RTSPSession-timeout(10)
	//{".3.11", "ngod2-RTSPSession-cacheSize" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspExtNGOD(3).ngod2-RTSPSession-cacheSize(11)
	//{".3.12", "ngod2-RTSPSession-defaultServiceGroup" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspExtNGOD(3).ngod2-RTSPSession-defaultServiceGroup(12)
	//{".3.13", "ngod2-Database-path" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspExtNGOD(3).ngod2-Database-path(13)
	//{".3.14", "ngod2-Announce-useGlobalCSeq" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspExtNGOD(3).ngod2-Announce-useGlobalCSeq(14)
	//{".3.15", "ngod2-Response-setupFailureWithSessId" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspExtNGOD(3).ngod2-Response-setupFailureWithSessId(15)
	//{".3.16", "ngod2-Response-streamCtrlPort" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspExtNGOD(3).ngod2-Response-streamCtrlProt(16)
	//{".3.17", "ngod2-LAM-TestMode-enabled" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspExtNGOD(3).ngod2-LAM-TestMode-enabled(17)
	//{".3.18", "ngod2-playlistControl-enableEOT" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspExtNGOD(3).ngod2-playlistControl-enableEOT(18)
	_mmib.addObject(new ZQ::SNMP::SNMPObject("ngod2-LogFile-size",           ngodConfig.pluginLog.size));
	_mmib.addObject(new ZQ::SNMP::SNMPObject("ngod2-LogFile-level",          ngodConfig.pluginLog.level));
	_mmib.addObject(new ZQ::SNMP::SNMPObject("ngod2-eventChannel-endpoint",  ngodConfig.iceStorm.endpoint));
	_mmib.addObject(new ZQ::SNMP::SNMPObject("ngod2-Bind-endpoint",          ngodConfig.bind.endpoint));
	_mmib.addObject(new ZQ::SNMP::SNMPObject("ngod2-RTSPSession-timeout",    ngodConfig.rtspSession.timeout));
	_mmib.addObject(new ZQ::SNMP::SNMPObject("ngod2-RTSPSession-cacheSize",  ngodConfig.rtspSession.cacheSize));
	_mmib.addObject(new ZQ::SNMP::SNMPObject("ngod2-RTSPSession-defaultServiceGroup",    ngodConfig.rtspSession.defaultServiceGroup));
	_mmib.addObject(new ZQ::SNMP::SNMPObject("ngod2-Database-path",                      ngodConfig.database.path));
	_mmib.addObject(new ZQ::SNMP::SNMPObject("ngod2-Announce-useGlobalCSeq",             ngodConfig.announce.useGlobalCSeq));

	_mmib.addObject(new ZQ::SNMP::SNMPObject("ngod2-Response-setupFailureWithSessId",    ngodConfig.response.setupFailureWithSessId));
	_mmib.addObject(new ZQ::SNMP::SNMPObject("ngod2-Response-streamCtrlPort",            ngodConfig.response.streamCtrlProt));

	_mmib.addObject(new ZQ::SNMP::SNMPObject("ngod2-playlistControl-enableEOT",          ngodConfig.playlistControl.enableEOT));
	_mmib.addObject(new ZQ::SNMP::SNMPObject("ngod2-playlistControl-nptByPrimary",       ngodConfig.playlistControl.nptByPrimary));

	_mmib.addObject(new ZQ::SNMP::SNMPObject("ngod2-LAM-TestMode-enabled",             ngodConfig.lam.lamTestMode.enabled, true)); // TODO: false)); to be writeable

	// snmp_refreshSOPUsage(0);
}

NgodEnv::~NgodEnv(void)
{
}
static ZQ::common::Mutex mSeqLocker;
uint16 NgodEnv::incAndGetSeqNumber( )
{
	ZQ::common::MutexGuard gd(mSeqLocker);
	return mGlobaSeq++;
}

ZQ::common::Mutex gEventRecvTimeLocker;

void NgodEnv::updateLastEventRecvTime( int64 t)
{		
	ZQ::common::MutexGuard gd(gEventRecvTimeLocker);
	mLastEventRecvTime = t;
}

int64 NgodEnv::getLastEventRecvTime( ) const
{
	ZQ::common::MutexGuard gd(gEventRecvTimeLocker);
	return mLastEventRecvTime;
}

void NgodEnv::snmp_refreshSOPUsage(const uint32& iDummy)
{
	static ZQ::SNMP::Oid subOidTbl;
	if (subOidTbl.isNil())
		_mmib.reserveTable("sopTable", 15, subOidTbl);
	if (subOidTbl.isNil())
	{
		if (mMainLogger)
			(*mMainLogger)(ZQ::common::Log::L_WARNING, CLOGFMT(NgodService,"snmp_refreshSOPUsage() failed to locate sopTable in MIB"));
		return;
	}

	// step 1. clean up the table content
	ZQ::SNMP::Oid tmpOid(subOidTbl);
	tmpOid.append(1);
	_mmib.removeSubtree(tmpOid);

	SOPS sopRes;
	std::string strSince;
	mSelManager.getSopData(sopRes, strSince);

	uint32 idxRow =1;
	for (SOPS::iterator it=sopRes.begin(); it != sopRes.end(); it++)
	{
		for (ResourceStreamerAttrMap::iterator j = it->second.begin(); j != it->second.end(); j++, idxRow++)
		{
			ResourceStreamerAttr& stmrdata = j->second;
			_mmib.addTableCell(subOidTbl,  1, idxRow, new ZQ::SNMP::SNMPObjectDupValue("sopIndex", (int32)idxRow));
			_mmib.addTableCell(subOidTbl,  2, idxRow, new ZQ::SNMP::SNMPObjectDupValue("sopName",  it->first));
			_mmib.addTableCell(subOidTbl,  3, idxRow, new ZQ::SNMP::SNMPObjectDupValue("sopStreamer", stmrdata.netId));
			_mmib.addTableCell(subOidTbl,  4, idxRow, new ZQ::SNMP::SNMPObjectDupValue("sopStreamService", stmrdata.endpoint));
			_mmib.addTableCell(subOidTbl,  5, idxRow, new ZQ::SNMP::SNMPObjectDupValue("sopStatus", (const char*)(stmrdata.bReplicaStatus?"avail":"unavail")));
			_mmib.addTableCell(subOidTbl,  6, idxRow, new ZQ::SNMP::SNMPObjectDupValue("sopPenalty", (int64)stmrdata.penalty));
			_mmib.addTableCell(subOidTbl,  7, idxRow, new ZQ::SNMP::SNMPObjectDupValue("sopSessionUsed", stmrdata.statisticsUsedSessCount));
			_mmib.addTableCell(subOidTbl,  8, idxRow, new ZQ::SNMP::SNMPObjectDupValue("sopSessionFailed", stmrdata.statisticsFailedSessCount));
			_mmib.addTableCell(subOidTbl,  9, idxRow, new ZQ::SNMP::SNMPObjectDupValue("sopErrorRate", (int32)(stmrdata.statisticsTotalSessCount? (stmrdata.statisticsFailedSessCount*100 / stmrdata.statisticsTotalSessCount) :0)));
			_mmib.addTableCell(subOidTbl, 10, idxRow, new ZQ::SNMP::SNMPObjectDupValue("sopUsedBandwidth", (int64) stmrdata.usedBw));
			_mmib.addTableCell(subOidTbl, 11, idxRow, new ZQ::SNMP::SNMPObjectDupValue("sopMaxBandwidth", (int64) stmrdata.maxBw));
			_mmib.addTableCell(subOidTbl, 12, idxRow, new ZQ::SNMP::SNMPObjectDupValue("sopActiveSession", (int64)stmrdata.usedSessCount));
			_mmib.addTableCell(subOidTbl, 13, idxRow, new ZQ::SNMP::SNMPObjectDupValue("sopMaxSession", (int64)stmrdata.maxSessCount));
			_mmib.addTableCell(subOidTbl, 14, idxRow, new ZQ::SNMP::SNMPObjectDupValue("sopLocalSession", (int64)(stmrdata.statisticsTotalSessCount -stmrdata.statisticsRemoteSessCount)));
			_mmib.addTableCell(subOidTbl, 15, idxRow, new ZQ::SNMP::SNMPObjectDupValue("sopVolume", stmrdata.volume));
		}
	}
	_mmib.addObject(new ZQ::SNMP::SNMPObjectDupValue("sopCount", (int32)idxRow -1));
	if (mMainLogger)
		(*mMainLogger)(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodService,"snmp_refreshSOPUsage() table refreshed: %d rows"), (int32)idxRow -1);

	snmp_refreshIcUsage(iDummy); // trigger IC refresh as currently take a single sopReset as the trigger to refresh both table
}

void NgodEnv::snmp_refreshIcUsage(const uint32&)
{
	static ZQ::SNMP::Oid subOidTbl;
	if (subOidTbl.isNil())
		_mmib.reserveTable("icTable", 6, subOidTbl);
	if (subOidTbl.isNil())
	{
		if (mMainLogger)
			(*mMainLogger)(ZQ::common::Log::L_WARNING, CLOGFMT(NgodService,"snmp_refreshIcUsage() failed to locate icTable in MIB"));
		return;
	}

	// step 1. clean up the table content
	ZQ::SNMP::Oid tmpOid(subOidTbl);
	tmpOid.append(1);
	_mmib.removeSubtree(tmpOid);

	ResourceImportChannelAttrMap ics;
	mSelManager.getImportChannelData(ics);

	uint32 idxRow =1;
	for (ResourceImportChannelAttrMap::iterator it=ics.begin(); it != ics.end(); it++, idxRow++)
	{
		ResourceImportChannelAttr& ic = it->second;
		_mmib.addTableCell(subOidTbl,  1, idxRow, new ZQ::SNMP::SNMPObjectDupValue("icIndex",         (int32)idxRow));
		_mmib.addTableCell(subOidTbl,  2, idxRow, new ZQ::SNMP::SNMPObjectDupValue("icChannelName",   ic.netId));
		_mmib.addTableCell(subOidTbl,  3, idxRow, new ZQ::SNMP::SNMPObjectDupValue("icUsedBandwidth", (int64)ic.reportUsedBW));
		_mmib.addTableCell(subOidTbl,  4, idxRow, new ZQ::SNMP::SNMPObjectDupValue("icTotalBandwidth", (int64)ic.reportMaxBW));
		_mmib.addTableCell(subOidTbl,  5, idxRow, new ZQ::SNMP::SNMPObjectDupValue("icRunningSessCount", (int32)ic.reportUsedSessCount));
		_mmib.addTableCell(subOidTbl,  6, idxRow, new ZQ::SNMP::SNMPObjectDupValue("icStatus",           (char*)"n/a")); // ????
	}

	_mmib.addObject(new ZQ::SNMP::SNMPObjectDupValue("icCount", (int32)idxRow -1));
	if (mMainLogger)
		(*mMainLogger)(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodService,"snmp_refreshIcUsage() table refreshed: %d rows"), (int32)idxRow -1);
}

}//namespace NGOD
