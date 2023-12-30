#include "environment.h"
#include "NativeThreadPool.h"
// #include "snmp/ZQSnmpMgmt.hpp"
// #include "snmp/ZQSnmp.hpp"
#include "DsmccDefine.h"
#include "crm_dsmcc/crm_dsmcc.h"

namespace ZQ { namespace CLIENTREQUEST{

/*
class DsmccMeasureSince: public ZQ::Snmp::IVariable
{
public:
	DsmccMeasureSince(ZQ::CLIENTREQUEST::GatewayStatisticsCollector& statsCollector)
		:_statsCollector(statsCollector){};

	virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType)
	{
		char tempbuf[80] = {0};
		std::string measureSince(ZQTianShan::TimeToUTC(_statsCollector.measuredSince(), tempbuf, sizeof(tempbuf)-2));

		return smivalFrom(val, measureSince, desiredType);
	}

	virtual bool set(const ZQ::Snmp::SmiValue& val)
	{
		return true;// read only, not set
	}

	virtual bool validate(const ZQ::Snmp::SmiValue& val) const
	{
		return true;
	}

private:
	ZQ::CLIENTREQUEST::GatewayStatisticsCollector&  _statsCollector;
};

int  ZQ::CLIENTREQUEST::Environment::registerSnmp(GatewayCenter* gwCenter)
{
	using namespace ZQ::Snmp;
	int nRev = false;
	int registerCount = 0;
	if (NULL == gwCenter)
	{
        mLogger(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "registerSnmp() gwCenter[NULL] failed"));
		return nRev;
	}

    ZQ::Snmp::Subagent* dsmccSnmpAgent = getDsmccSnmpAgent();
	if(NULL == dsmccSnmpAgent)
	{
		mLogger(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "registerSnmp() dsmccSnmpAgent[NULL] failed"));
		return nRev;
	}

	try
	{
		ZQ::common::NativeThreadPool&  threadPool = gwCenter->getThreadPool();

		typedef DECLARE_SNMP_RO_TYPE(GatewayCenter&, size_t (GatewayCenter::*)(void) const, int)                                   DsmccSessionCount;
		typedef DECLARE_SNMP_RO_TYPE(ZQ::common::NativeThreadPool&, const int (ZQ::common::NativeThreadPool::*)(void), int)        DsmccPendingSize;
		typedef DECLARE_SNMP_RO_TYPE(ZQ::common::NativeThreadPool&, int (ZQ::common::NativeThreadPool::*)(void) const, int)        DsmccBusyThreads;
		typedef DECLARE_SNMP_RO_TYPE(ZQ::common::NativeThreadPool&, int (ZQ::common::NativeThreadPool::*)(void) const, int)        DsmccThreadPoolSize;
		typedef  struct TypeStruct<TYPELIST_3(int, int, int)>                                                                     thrIntList;
		typedef  struct TypeStruct<TYPELIST_3(GatewayStatisticsCollector&, void (GatewayStatisticsCollector::*)(void), int) >      refCollectorList;
		typedef  VarCommon<aReadWrite, TYPELIST_2(thrIntList, refCollectorList) >                                                  DsmccMeasureReset;
//		typedef DECLARE_SNMP_RW_TYPE(int, int, int, GatewayStatisticsCollector&, void (GatewayStatisticsCollector::*)(void), int)  DsmccMeasureReset;

//		dsmccSnmpAgent->addObject( Oid("1.2"), ManagedPtr(new SimpleObject(VariablePtr(new DsmccSessionCount(*gwCenter, &GatewayCenter::sessionCount)),                      AsnType_Integer, aReadOnly)));  ++registerCount;
		dsmccSnmpAgent->addObject( Oid("1.3"), ManagedPtr(new SimpleObject(VariablePtr(new DsmccPendingSize(threadPool, &ZQ::common::NativeThreadPool::pendingRequestSize)), AsnType_Integer, aReadOnly)));  ++registerCount;
		dsmccSnmpAgent->addObject( Oid("1.4"), ManagedPtr(new SimpleObject(VariablePtr(new DsmccBusyThreads(threadPool, &ZQ::common::NativeThreadPool::activeCount)),        AsnType_Integer, aReadOnly)));  ++registerCount;
		dsmccSnmpAgent->addObject( Oid("1.5"), ManagedPtr(new SimpleObject(VariablePtr(new DsmccThreadPoolSize(threadPool, &ZQ::common::NativeThreadPool::size)),            AsnType_Integer, aReadOnly)));  ++registerCount;
	//	dsmccSnmpAgent->addObject( Oid("1.298"), ManagedPtr(new SimpleObject(VariablePtr(new DsmccMeasureReset((int)1, (int)1, mStatsCollector, &GatewayStatisticsCollector::reset)),  AsnType_Integer, aReadWrite))); ++registerCount;
		dsmccSnmpAgent->addObject( Oid("1.299"), ManagedPtr(new SimpleObject(VariablePtr(new DsmccMeasureSince(mStatsCollector)),  AsnType_Octets,  aReadOnly)));   ++registerCount;

		nRev = true;
		mLogger(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "Environment registerSnmp() registerCount[%d] succeed"), registerCount);
	}
	catch (...)//not  allowed  to failed
	{
		mLogger(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "Environment registerSnmp() failed registerCount[%d]"), registerCount);
	}
    
	if(!registerSnmpTable())
	{
		 nRev = false;
		 mLogger(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "Environment registerSnmp() failed to register snmp table"));
	}

    return nRev;
}

template<typename Type>
class TableMediatorVar: public ZQ::Snmp::IVariable
{
public:
	TableMediatorVar(){}
	TableMediatorVar(Type var):_val(var){}

	~TableMediatorVar(){}

	virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType)
	{
		return smivalFrom(val, _val, desiredType);
	}

	virtual bool set(const ZQ::Snmp::SmiValue& val)
	{
		return smivalTo(val, _val);
	}

	virtual bool validate(const ZQ::Snmp::SmiValue& val) const
	{
		return true;
	}

private:
	Type _val;
};

typedef  TableMediatorVar<int>  intVariable;

template<typename TableClass, typename TableData>
class TableMediator: public ZQ::Snmp::IManaged
{
public:
	TableMediator(ZQ::common::Log * reporter, const ZQ::Snmp::Oid subid, ZQ::Snmp::Subagent* snmpTableAgent, TableData & tableEnv)
		:_subid(subid), _triggerSubid("1.1"), _snmpTableAgent(snmpTableAgent), _createTable(tableEnv), _reporter(reporter)
	{
		_inStoreTable = _createTable(_reporter);
	};

	virtual ~TableMediator(){};

	virtual ZQ::Snmp::Status get(const ZQ::Snmp::Oid& subid, ZQ::Snmp::SmiValue& val)
	{
		if (0 == _triggerSubid.compare(0, subid.length(), subid))
			_inStoreTable = _createTable(_reporter);//refresh table

		return _inStoreTable->get(subid, val); 
	};

	virtual ZQ::Snmp::Status set(const ZQ::Snmp::Oid& subid, const ZQ::Snmp::SmiValue& val)
	{
		return _inStoreTable->set(subid, val);
	};

	virtual ZQ::Snmp::Status next(const ZQ::Snmp::Oid& subid, ZQ::Snmp::Oid& nextId) const
	{
		if (0 == _triggerSubid.compare(0, subid.length(), subid))
		{
			TableMediator* tempThis = const_cast<TableMediator*>(this);
			tempThis->_inStoreTable = tempThis->_createTable(_reporter);//refresh table
		}

		return  _inStoreTable->next(subid, nextId);
	};

	virtual ZQ::Snmp::Status first(ZQ::Snmp::Oid& firstId) const
	{
		return _inStoreTable->first(firstId);
	};

	bool addColumn(uint32 colId, ZQ::Snmp::AsnType type, ZQ::Snmp::Access access)
	{
		return _inStoreTable->addColumn(colId, type, access);
	};

	bool addRowData(uint32 colId, ZQ::Snmp::Oid rowIndex, ZQ::Snmp::VariablePtr var)
	{
		return _inStoreTable->addRowData(colId, rowIndex, var);
	};

	ZQ::Snmp::Oid buildIndex(const std::string& idx)
	{
		return _inStoreTable->buildIndex(idx);
	};

	ZQ::Snmp::Oid buildIndex(uint32 idx)
	{
		return _inStoreTable->buildIndex(idx);
	};

private:
	ZQ::Snmp::Oid             _subid;	
	ZQ::Snmp::Oid             _triggerSubid;
	ZQ::Snmp::Subagent *      _snmpTableAgent;
	ZQ::Snmp::TablePtr        _inStoreTable;
	ZQ::common::Log     *     _reporter;

	TableClass      _createTable;	
};

class StatisticStateTable
{
public:
	enum ColumnName{
		CN_NULL		          = 0,
		CN_ROW_NAME           = 1,
		CN_REQUEST_COUNT       ,
		CN_TIME_COST_MAX       ,
//		CN_TIME_COST_MIX       ,
		CN_TIME_COST_AVG       ,
		CN_COUNT_OK            ,
		CN_COUNT_NO_SESS       ,
		CN_COUNT_BAD_REQUEST   ,
		CN_COUNT_INVALID_METHOD,
		CN_COUNT_SERVER_ERROR  ,
		TABLE_COLUNM_COUNT
	};

public:
		StatisticStateTable(GatewayStatisticsCollector & tableEnv)
			:_statsCollector(tableEnv){}
	
		~StatisticStateTable(){}
	
		ZQ::Snmp::TablePtr  operator()(ZQ::common::Log * reporter)
		{
			using namespace ZQ::DSMCC;
			ZQ::Snmp::TablePtr tbLocateRequestUsage(new ZQ::Snmp::Table());
			const char* rowName[] = {
				"null"  ,
				"setup"	,
				"GetParameter"	,
				"play",
				"TearDown",
				"Colunm end"
			};

			tbLocateRequestUsage->addColumn( CN_ROW_NAME     ,	ZQ::Snmp::AsnType_Octets,     ZQ::Snmp::aReadOnly);            
			tbLocateRequestUsage->addColumn( CN_REQUEST_COUNT,	ZQ::Snmp::AsnType_Integer,	   ZQ::Snmp::aReadOnly); 
			tbLocateRequestUsage->addColumn( CN_TIME_COST_MAX,	ZQ::Snmp::AsnType_Integer,     ZQ::Snmp::aReadOnly);    
//			tbLocateRequestUsage->addColumn( CN_TIME_COST_MIX,	ZQ::Snmp::AsnType_Integer,     ZQ::Snmp::aReadOnly); 
			tbLocateRequestUsage->addColumn( CN_TIME_COST_AVG,	ZQ::Snmp::AsnType_Integer,     ZQ::Snmp::aReadOnly); 
			tbLocateRequestUsage->addColumn( CN_COUNT_OK            ,	ZQ::Snmp::AsnType_Integer,     ZQ::Snmp::aReadOnly); 
			tbLocateRequestUsage->addColumn( CN_COUNT_NO_SESS       ,	ZQ::Snmp::AsnType_Integer,     ZQ::Snmp::aReadOnly);
			tbLocateRequestUsage->addColumn( CN_COUNT_BAD_REQUEST   ,	ZQ::Snmp::AsnType_Integer,     ZQ::Snmp::aReadOnly); 
			tbLocateRequestUsage->addColumn( CN_COUNT_INVALID_METHOD,	ZQ::Snmp::AsnType_Integer,     ZQ::Snmp::aReadOnly);
			tbLocateRequestUsage->addColumn( CN_COUNT_SERVER_ERROR  ,	ZQ::Snmp::AsnType_Integer,     ZQ::Snmp::aReadOnly); 			
	
			int rowIndex = 0;
			GatewayStatisics statisticsState[STATISTICS_COUNT];
			const int ROW_END = sizeof(statisticsState) / sizeof(GatewayStatisics) + 1;
			_statsCollector.getStatistics(statisticsState);

			for (int nStep = 1; nStep < ROW_END; ++nStep)
			{
				try
				{
					++rowIndex;
					ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex(rowIndex);
					struct GatewayStatisics * rowState = (struct GatewayStatisics *)statisticsState + (nStep - 1);
					ErrorToCountMap& erroeToCountMapRef = rowState->errorToCountMap;
					std::string rowInstance(rowName[nStep]);
					int totalCount(rowState->totalCount) ;	 
					int timeCostMax(rowState->timeCostMax); 
					int timeCostMin(rowState->timeCostMin);
					int timeCostAvg(rowState->timeCostAvg);
					ErrorToCountMap::iterator it;

#define GET_VALUE_FROM_MAP_BY_INDEX(index)   (it = erroeToCountMapRef.find(index), (it != erroeToCountMapRef.end()) ? it->second : 0 )

					int countOK            = GET_VALUE_FROM_MAP_BY_INDEX( RsnOK );
					int countNoSess        = GET_VALUE_FROM_MAP_BY_INDEX( RspNeNoSession );
					int countBadReqest     = GET_VALUE_FROM_MAP_BY_INDEX( lscErr_BadRequest );
					int countInvalidMethod = GET_VALUE_FROM_MAP_BY_INDEX( lscErr_InvalidMethod );
					int countServerError   = GET_VALUE_FROM_MAP_BY_INDEX( lscErr_ServerErr );

					tbLocateRequestUsage->addRowData(CN_ROW_NAME     ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<std::string>(rowInstance) ));              
					tbLocateRequestUsage->addRowData(CN_REQUEST_COUNT,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(totalCount ) ));	            
					tbLocateRequestUsage->addRowData(CN_TIME_COST_MAX,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(timeCostMax) ));
//					tbLocateRequestUsage->addRowData(CN_TIME_COST_MIX,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(timeCostMin) ));
					tbLocateRequestUsage->addRowData(CN_TIME_COST_AVG,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(timeCostAvg) ));
					tbLocateRequestUsage->addRowData(CN_COUNT_OK            ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(countOK           ) ));
					tbLocateRequestUsage->addRowData(CN_COUNT_NO_SESS       ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(countNoSess       ) ));
					tbLocateRequestUsage->addRowData(CN_COUNT_BAD_REQUEST   ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(countBadReqest    ) ));
					tbLocateRequestUsage->addRowData(CN_COUNT_INVALID_METHOD,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(countInvalidMethod) ));
					tbLocateRequestUsage->addRowData(CN_COUNT_SERVER_ERROR  ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(countServerError  ) ));
					(*reporter)(ZQ::common::Log::L_DEBUG, CLOGFMT(StatisticStateTable, "Dsmcc snmp state row[%d], rowName[%s] count[%d] Max[%d] Min[%d] Avg[%d]"),
						rowIndex, rowInstance.c_str(), totalCount, timeCostMax, timeCostMin, timeCostAvg);
				}
				catch (...) 
				{
					(*reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(StatisticStateTable, "Dsmcc snmp state table add data error, row[%d]"), rowIndex);
				}
			}
	
			(*reporter)(ZQ::common::Log::L_INFO, CLOGFMT(StatisticStateTable, "Dsmcc snmp create state Table end, row[%d]"), rowIndex);
			return tbLocateRequestUsage;
		}
	
	private:
		GatewayStatisticsCollector & _statsCollector;
};


int  ZQ::CLIENTREQUEST::Environment::registerSnmpTable(void)
{
	using namespace ZQ::Snmp;
	int nRev = true;

	ManagedPtr statisticStateTable(new TableMediator<StatisticStateTable, GatewayStatisticsCollector >(&mLogger, ZQ::Snmp::Oid("1.300.1"), mdsmccCRGSnmpAgnet, mStatsCollector));
	nRev = mdsmccCRGSnmpAgnet->addObject(ZQ::Snmp::Oid("1.300.1"), ManagedPtr(statisticStateTable));

	mLogger(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "Environment registerSnmpTable() %s"), (nRev ? "Succeed" : "Failed"));

	return nRev;
}
*/

//////////////////////////////////////////////////////////////////////////
GatewayStatisticsCollector::GatewayStatisticsCollector()
{
	reset(0);
}
GatewayStatisticsCollector::~GatewayStatisticsCollector()
{
}

void GatewayStatisticsCollector::reset(const uint32& dummy)
{
	ZQ::common::MutexGuard gd(mLocker);
	for(size_t i = 0 ; i < STATISTICS_COUNT ; i ++ )
	{
		mStats[i].reset();
	}
	mMeasuredSince = ZQ::common::now();
}



void GatewayStatisticsCollector::_leverage(uint32 winSizeMsec)
{
	int64 stampNow = ZQ::common::now();
	int64 stampSince = stampNow -winSizeMsec;

	mMeasuredSince = stampSince;

	for (size_t i = 0 ; i < STATISTICS_COUNT ; i ++ )
	{
		GatewayStatisics& row = mStats[i];

		if (_stampTill < stampSince)
		{
			reset(0);
			continue;
		}

		int64 newSince = stampSince;
		if (newSince < row.winSince)
			newSince =  row.winSince;

		int64 oldWin = stampNow - row.winSince;
		if (oldWin <=0)
			continue;

		int64 deltaW = newSince - row.winSince;
		if (deltaW <=0)
			continue;

		int deltaT = (int) ((row.totalCount * deltaW) / oldWin);
		if (deltaT<=0)
			continue;

		row.totalCount -= deltaT;
		row.winSince   = newSince;
		if (newSince < mMeasuredSince)
			mMeasuredSince = newSince; 

		if (row.totalCount <=0)
		{
			row.reset();
			continue;
		}

		// adjust the latency values
		row.timeCostTotal -= deltaT * row.timeCostAvg;
		if (row.winSince > row.stampMax)
		{
			row.timeCostMax = (row.timeCostMax + row.timeCostAvg*5) /6;
			row.stampMax = row.winSince;
		}

		// correct the per errcode count
		ErrorToCountMap::iterator it;
		int delta=0;
		for (ErrorToCountMap::iterator it = row.errorToCountMap.begin(); deltaT>0 && it != row.errorToCountMap.end(); it++)
		{
			delta = (int) ((it->second * deltaW) / oldWin);
			if (delta<=0)
				continue;

			it->second -= delta;
			deltaT -= delta;
		}

		for (it = row.errorToCountMap.end(), it--; deltaT>0 && it != row.errorToCountMap.begin(); it--)
		{
			delta = (it->second <(uint32)deltaT) ? it->second :deltaT;
			if (delta<=0)
			{
				it->second =0;
				continue;
			}

			it->second -= delta;
			deltaT -= delta;
		}
	}

	stampSince-=winSizeMsec/4;

	if (mMeasuredSince < stampSince)
		mMeasuredSince = stampSince;
}

GatewayStatisics* GatewayStatisticsCollector::getStatistics()
{
    ZQ::common::MutexGuard gd(mLocker);
    _leverage();

    return mStats;
}

void GatewayStatisticsCollector::getStatistics( GatewayStatisics stats[] )
{
	ZQ::common::MutexGuard gd(mLocker);
	_leverage();

	for(size_t i = 0 ; i < STATISTICS_COUNT ; i ++ )
	{
		stats[i] = mStats[i];
	}
}

uint64 GatewayStatisticsCollector::measuredSince()
{
	ZQ::common::MutexGuard gd(mLocker);
	return (uint64)mMeasuredSince;
}

void GatewayStatisticsCollector::collect( GATEWAYCOMMAND cmd, uint32 resultCode, uint32 timecost)
{
	_stampTill = ZQ::common::now(); 
	ZQ::common::MutexGuard gd(mLocker);
	GatewayStatisics* stats = 0;
	switch (cmd)
	{
	case COMMAND_SETUP_RESPONSE:
		stats = &mStats[STAT_SETUP];
		break;
	case COMMAND_PLAY_RESPONSE:
		stats = &mStats[STAT_PLAY];
		break;
	case COMMAND_STATUS_RESPONSE:
		stats = &mStats[STAT_GETPARAMETER];
		break;
	case COMMAND_DESTROY_RESPONSE:
		stats = &mStats[STAT_TEARDOWN];
		break;
	default:
		return;
	}

	if(!stats)	return;
	if (timecost > stats->timeCostMax)
	{
		stats->timeCostMax = timecost;
		stats->stampMax = _stampTill;
	}

	stats->timeCostMin = ( stats->totalCount <= 0 || stats->timeCostMin > timecost ) ?  timecost : stats->timeCostMin;
	stats->timeCostTotal += timecost;
	stats->totalCount ++;
	stats->timeCostAvg = stats->totalCount > 0 ? (stats->timeCostTotal / stats->totalCount):0;
	uint32 countPerError = 0;
	if (stats->errorToCountMap.end() != stats->errorToCountMap.find(resultCode))
		countPerError = stats->errorToCountMap[resultCode];
	MAPSET(ErrorToCountMap, stats->errorToCountMap, resultCode, ++countPerError);
}

}}//namespace ZQ::CLIENTREQUEST