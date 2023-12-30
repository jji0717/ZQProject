#ifndef __ZQ_CDNSS_SNMP_EXP_H__
#define __ZQ_CDNSS_SNMP_EXP_H__
#include "snmp/SubAgent.hpp"
#include "snmp/ZQSnmpMgmt.hpp"
#include "CacheStoreImpl.h"

template<typename Type>
class TableMediatorVar: public ZQ::Snmp::IVariable
{
public:
	TableMediatorVar(){}
	TableMediatorVar(Type var):val_(var){}

	~TableMediatorVar(){}

	virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType)
	{
		return smivalFrom(val, val_, desiredType);
	}

	virtual bool set(const ZQ::Snmp::SmiValue& val)
	{
		return smivalTo(val, val_);
	}

	virtual bool validate(const ZQ::Snmp::SmiValue& val) const
	{
		return true;
	}

private:
	Type val_;
};

typedef  TableMediatorVar<std::string>  strVariable;

template<typename TableClass, typename TableData>
class TableMediator: public ZQ::Snmp::IManaged
{
public:
	TableMediator(ZQ::common::FileLog * reporter, const ZQ::Snmp::Oid subid, ZQ::Snmp::Subagent* snmpTableAgent, TableData & tableEnv)
		:_subid(subid), _triggerSubid("1.1"), _snmpTableAgent(snmpTableAgent), _createTable(tableEnv), _tableEnv(tableEnv), _reporter(reporter)
	{
		_inStoreTable = _createTable(_reporter);
	};

	virtual ~TableMediator(){};

	virtual ZQ::Snmp::Status get(const ZQ::Snmp::Oid& subid, ZQ::Snmp::SmiValue& val)
	{
		if (0 == _triggerSubid.compare(0, subid.length(), subid))
			_inStoreTable = _createTable(_reporter);//refresh table

		(*_reporter)(ZQ::common::Log::L_DEBUG, CLOGFMT(TableMediator, "TableMediator  get method"));
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

		(*_reporter)(ZQ::common::Log::L_DEBUG, CLOGFMT(TableMediator, "TableMediator  get next method"));
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
	ZQ::common::FileLog *     _reporter;
	TableClass      _createTable;	
	TableData &     _tableEnv;
};


class MissedContents
{
public:
	MissedContents(ZQTianShan::ContentStore::CacheStoreImpl & tableEnv)
		:_store(tableEnv){}

	~MissedContents(){}

	ZQ::Snmp::TablePtr  operator()(ZQ::common::FileLog * reporter)
	{
		int  sortNum;
		char stampSinceBuf[80];
		char stampLatestBuf[80];
		TianShanIce::Storage::ContentCounterList sortedList;
		ZQ::Snmp::TablePtr tbMissedContentsUsage(new ZQ::Snmp::Table());
		TianShanIce::Storage::AccessRegistrarPtr& acList = _store.getContentMissed();

		enum HotContentsTableColunm
		{
			HC_CONTENT_NAME = 1,
			HC_ACCESS_COUNT,
			HC_STAMP_SINCE ,
			HC_STAMP_LATEST,
			TABLE_COLUNM_COUNT
		};

		tbMissedContentsUsage->addColumn(HC_CONTENT_NAME,   ZQ::Snmp::AsnType_Octets,     ZQ::Snmp::aReadOnly);            
		tbMissedContentsUsage->addColumn(HC_ACCESS_COUNT,   ZQ::Snmp::AsnType_Integer,	  ZQ::Snmp::aReadOnly); 
		tbMissedContentsUsage->addColumn(HC_STAMP_SINCE ,	ZQ::Snmp::AsnType_Octets,     ZQ::Snmp::aReadOnly);    
		tbMissedContentsUsage->addColumn(HC_STAMP_LATEST,	ZQ::Snmp::AsnType_Octets,     ZQ::Snmp::aReadOnly); 

		if (!acList)
		{
			(*reporter)(ZQ::common::Log::L_INFO, CLOGFMT(MissedContents, "cdnss snmp create MissedContents return, acList[empty], row[%d], sortedList size[%d]"), sortNum, sortedList.size());
			return tbMissedContentsUsage;
		}

		try	
		{
			acList->sort(_store._timeWinOfPopular, false, 0, sortedList);
			for (sortNum = 0; sortNum < (int) sortedList.size(); ++sortNum)
			{
				int rowIndex           = 1 + sortNum;
				ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex(rowIndex);
				TianShanIce::Storage::ContentAccess & result = sortedList[sortNum].base;

				memset(stampSinceBuf, 0, sizeof(stampSinceBuf));
				memset(stampLatestBuf, 0, sizeof(stampLatestBuf));
				
				int  accessCount = result.accessCount;
				std::string contentName(result.contentName);
				std::string stampSince(ZQTianShan::TimeToUTC(result.stampSince, stampLatestBuf, sizeof(stampLatestBuf) - 2));
				std::string stampLatest(ZQTianShan::TimeToUTC(result.stampLatest, stampLatestBuf, sizeof(stampLatestBuf) - 2));

				tbMissedContentsUsage->addRowData(HC_CONTENT_NAME,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<std::string>(contentName) ));              
				tbMissedContentsUsage->addRowData(HC_ACCESS_COUNT,	indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(accessCount) ));	            
				tbMissedContentsUsage->addRowData(HC_STAMP_SINCE ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<std::string>(stampSince) ));
				tbMissedContentsUsage->addRowData(HC_STAMP_LATEST,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<std::string>(stampLatest) ));
			}

			(*reporter)(ZQ::common::Log::L_INFO, CLOGFMT(MissedContents, "cdnss snmp create MissedContents end, row[%d], sortedList size[%d]"), sortNum, sortedList.size());
		}
		catch(const ::Ice::Exception& ex)
		{
			(*reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(MissedContents, "cdnss snmp create MissedContents, row[%d], sortedList size[%d], caught exception[%s]"), sortNum, sortedList.size(), ex.ice_name().c_str());
		}
		catch(...)
		{
            (*reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(MissedContents, "cdnss snmp create MissedContents, row[%d], sortedList size[%d], caught exception[unknown]"), sortNum, sortedList.size());
		}

		return tbMissedContentsUsage;
	}

private:
	ZQTianShan::ContentStore::CacheStoreImpl & _store;
};



class HotContents
{
public:
	HotContents(ZQTianShan::ContentStore::CacheStoreImpl & tableEnv)
		:_store(tableEnv){}

	~HotContents(){}

	ZQ::Snmp::TablePtr  operator()(ZQ::common::FileLog * reporter)
	{
		int  sortNum;
		char stampSinceBuf[80];
		char stampLatestBuf[80];
		TianShanIce::Storage::ContentCounterList sortedList;
		ZQ::Snmp::TablePtr tbHotContentsUsage(new ZQ::Snmp::Table());
		TianShanIce::Storage::AccessRegistrarPtr& acList = _store.getContentHotLocals();

		enum HotContentsTableColunm
		{
			HC_CONTENT_NAME = 1,
			HC_ACCESS_COUNT,
			HC_STAMP_SINCE ,
			HC_STAMP_LATEST,
			TABLE_COLUNM_COUNT
		};

		tbHotContentsUsage->addColumn(HC_CONTENT_NAME,  ZQ::Snmp::AsnType_Octets,     ZQ::Snmp::aReadOnly);            
		tbHotContentsUsage->addColumn(HC_ACCESS_COUNT,  ZQ::Snmp::AsnType_Integer,	  ZQ::Snmp::aReadOnly); 
		tbHotContentsUsage->addColumn(HC_STAMP_SINCE ,	ZQ::Snmp::AsnType_Octets,     ZQ::Snmp::aReadOnly);    
		tbHotContentsUsage->addColumn(HC_STAMP_LATEST,	ZQ::Snmp::AsnType_Octets,     ZQ::Snmp::aReadOnly); 

		if (!acList)
		{
			(*reporter)(ZQ::common::Log::L_INFO, CLOGFMT(HotContents, "cdnss snmp create HotContents return, acList[empty], row[%d], sortedList size[%d]"), sortNum, sortedList.size());
			return tbHotContentsUsage;
		}

		try	
		{
			acList->sort(_store._timeWinOfPopular, false, 0, sortedList);
			for (sortNum = 0; sortNum < (int) sortedList.size(); ++sortNum)
			{
				int rowIndex           = 1 + sortNum;
				ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex(rowIndex);
				TianShanIce::Storage::ContentAccess & result = sortedList[sortNum].base;

				memset(stampSinceBuf, 0, sizeof(stampSinceBuf));
				memset(stampLatestBuf, 0, sizeof(stampLatestBuf));

				int  accessCount = result.accessCount;
				std::string contentName(result.contentName);
				std::string stampSince(ZQTianShan::TimeToUTC(result.stampSince, stampLatestBuf, sizeof(stampLatestBuf) - 2));
				std::string stampLatest(ZQTianShan::TimeToUTC(result.stampLatest, stampLatestBuf, sizeof(stampLatestBuf) - 2));

				tbHotContentsUsage->addRowData(HC_CONTENT_NAME,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<std::string>(contentName) ));              
				tbHotContentsUsage->addRowData(HC_ACCESS_COUNT,	 indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(accessCount) ));	            
				tbHotContentsUsage->addRowData(HC_STAMP_SINCE ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<std::string>(stampSince) ));
				tbHotContentsUsage->addRowData(HC_STAMP_LATEST,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<std::string>(stampLatest) ));
			}

			(*reporter)(ZQ::common::Log::L_INFO, CLOGFMT(HotContents, "cdnss snmp create HotContents end, row[%d], sortedList size[%d]"), sortNum, sortedList.size());
		}
		catch(const ::Ice::Exception& ex)
		{
			(*reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(HotContents, "cdnss snmp create HotContents, row[%d], sortedList size[%d], caught exception[%s]"), sortNum, sortedList.size(), ex.ice_name().c_str());
		}
		catch(...)
		{
			(*reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(HotContents, "cdnss snmp create HotContents, row[%d], sortedList size[%d], caught exception[unknown]"), sortNum, sortedList.size());
		}

		return tbHotContentsUsage;
	}

private:
	ZQTianShan::ContentStore::CacheStoreImpl & _store;
};


class CacheStoreNeighbour
{
public:
	CacheStoreNeighbour(ZQTianShan::ContentStore::CacheStoreImpl & tableEnv)
		:_store(tableEnv){}

	~CacheStoreNeighbour(){}

	ZQ::Snmp::TablePtr  operator()(ZQ::common::FileLog * reporter)
	{
		int  rowIndex = 0;
		char stampAsOfBuf[80];
		TianShanIce::Storage::ContentCounterList sortedList;
		ZQ::Snmp::TablePtr tbCacheStoreNeighbourUsage(new ZQ::Snmp::Table());
		ZQTianShan::ContentStore::CacheStoreImpl::CacheStoreListInt acList;

		enum CacheStoreNeighbourColunm
		{
			CSN_NET_ID           = 1,
			CSN_STATE            ,
			CSN_LOAD             ,
			CSN_LOAD_IMPORT      ,
			CSN_LOAD_CACHE_WRITE ,
			CSN_STAMP_AS_OF      ,
			CSN_ENDPOINT         ,
			CSN_SESSION_INTERFACE,
			TABLE_COLUNM_COUNT
		};

		tbCacheStoreNeighbourUsage->addColumn(CSN_NET_ID           ,  ZQ::Snmp::AsnType_Octets,     ZQ::Snmp::aReadOnly);            
		tbCacheStoreNeighbourUsage->addColumn(CSN_STATE            ,  ZQ::Snmp::AsnType_Integer,    ZQ::Snmp::aReadOnly); 
		tbCacheStoreNeighbourUsage->addColumn(CSN_LOAD             ,  ZQ::Snmp::AsnType_Integer,    ZQ::Snmp::aReadOnly);    
		tbCacheStoreNeighbourUsage->addColumn(CSN_LOAD_IMPORT      ,  ZQ::Snmp::AsnType_Integer,    ZQ::Snmp::aReadOnly); 
		tbCacheStoreNeighbourUsage->addColumn(CSN_LOAD_CACHE_WRITE ,  ZQ::Snmp::AsnType_Integer,    ZQ::Snmp::aReadOnly);            
		tbCacheStoreNeighbourUsage->addColumn(CSN_STAMP_AS_OF      ,  ZQ::Snmp::AsnType_Octets,	    ZQ::Snmp::aReadOnly); 
		tbCacheStoreNeighbourUsage->addColumn(CSN_ENDPOINT         ,  ZQ::Snmp::AsnType_Octets,     ZQ::Snmp::aReadOnly);    
		tbCacheStoreNeighbourUsage->addColumn(CSN_SESSION_INTERFACE,  ZQ::Snmp::AsnType_Octets,     ZQ::Snmp::aReadOnly); 

		_store._listNeighorsEx(acList);
		if (acList.empty())
		{
			(*reporter)(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreNeighbour, "cdnss snmp create CacheStoreNeighbour return, row[%d], acList size[%d]"), rowIndex, acList.size());
			return tbCacheStoreNeighbourUsage;
		}

		try
		{
			ZQTianShan::ContentStore::CacheStoreImpl::CacheStoreListInt::iterator acListIter;
			for(acListIter = acList.begin(); acListIter != acList.end(); ++acListIter)
			{
				++rowIndex;
				memset(stampAsOfBuf, 0, sizeof(stampAsOfBuf));
				ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex(rowIndex);
				TianShanIce::Storage::CacheStoreDescriptor & csdRef = acListIter->desc;

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
					(*reporter)(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreNeighbour, "cdnss snmp create CacheStoreNeighbour, row[%d], acList size[%d], cacheStorePrx[NULL], endpoint[exception]"), rowIndex, acList.size());

				tbCacheStoreNeighbourUsage->addRowData(CSN_NET_ID           ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<std::string>(netId) ));              
				tbCacheStoreNeighbourUsage->addRowData(CSN_STATE            ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(state) ));	            
				tbCacheStoreNeighbourUsage->addRowData(CSN_LOAD             ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(load) ));
				tbCacheStoreNeighbourUsage->addRowData(CSN_LOAD_IMPORT      ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(loadImport) ));
				tbCacheStoreNeighbourUsage->addRowData(CSN_LOAD_CACHE_WRITE ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(loadCacheWrite) ));              
				tbCacheStoreNeighbourUsage->addRowData(CSN_STAMP_AS_OF      ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<std::string>(stampAsOf) ));	            
				tbCacheStoreNeighbourUsage->addRowData(CSN_ENDPOINT         ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<std::string>(endpoint) ));
				tbCacheStoreNeighbourUsage->addRowData(CSN_SESSION_INTERFACE,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<std::string>(sessionInterface) ));
			}

			(*reporter)(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreNeighbour, "cdnss snmp create CacheStoreNeighbour end, row[%d], acList size[%d]"), rowIndex, acList.size());
		}
		catch(const ::Ice::Exception& ex)
		{
			(*reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(CacheStoreNeighbour, "cdnss snmp create CacheStoreNeighbour, row[%d], acList size[%d], caught exception[%s]"), rowIndex, acList.size(), ex.ice_name().c_str());
		}
		catch(...)
		{
			(*reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(CacheStoreNeighbour, "cdnss snmp create CacheStoreNeighbour, row[%d], acList size[%d], caught exception[unknown]"), rowIndex, acList.size());
		}

		return tbCacheStoreNeighbourUsage;
	}

private:
	ZQTianShan::ContentStore::CacheStoreImpl & _store;
};


class CacheStoreStreamCounters
{
public:
	CacheStoreStreamCounters(ZQTianShan::ContentStore::CacheStoreImpl & tableEnv)
		:_store(tableEnv){}

	~CacheStoreStreamCounters(){}

	ZQ::Snmp::TablePtr  operator()(ZQ::common::FileLog * reporter)
	{
		using namespace ZQTianShan::ContentStore;

		int  rowIndex = 0;
		ZQ::Snmp::TablePtr tbCacheStoreStreamCountersUsage(new ZQ::Snmp::Table());

		enum CacheStoreStreamCountersColunm
		{
			CSS_NAME           = 1,
			CSS_COUNT             ,
			CSS_FAIL_COUNT        ,
			CSS_LATENCY_AVG       ,
			CSS_LATENCY_MAX       ,
			TABLE_COLUNM_COUNT
		};

		tbCacheStoreStreamCountersUsage->addColumn(CSS_NAME        ,  ZQ::Snmp::AsnType_Octets,     ZQ::Snmp::aReadOnly);            
		tbCacheStoreStreamCountersUsage->addColumn(CSS_COUNT       ,  ZQ::Snmp::AsnType_Integer,    ZQ::Snmp::aReadOnly); 
		tbCacheStoreStreamCountersUsage->addColumn(CSS_FAIL_COUNT  ,  ZQ::Snmp::AsnType_Integer,    ZQ::Snmp::aReadOnly);    
		tbCacheStoreStreamCountersUsage->addColumn(CSS_LATENCY_AVG ,  ZQ::Snmp::AsnType_Integer,    ZQ::Snmp::aReadOnly); 
		tbCacheStoreStreamCountersUsage->addColumn(CSS_LATENCY_MAX ,  ZQ::Snmp::AsnType_Integer,    ZQ::Snmp::aReadOnly);            

		try
		{
			for(int indexCount = 0; indexCount < (int)(CacheStoreImpl::ec_max); ++indexCount)
			{
				++rowIndex;
				CacheStoreImpl::ExportCount & exportCounter = _store._exportCounters[indexCount];
				ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex(rowIndex);
				
				std::string ecName(exportCounter.name);
				int ecCount(exportCounter.count);
				int ecFailCount(exportCounter.failCount);
				int ecLatencyAvg(0);
				int ecLatencyMax(exportCounter.latencyMax);

				if (0 < ecCount)
					ecLatencyAvg = exportCounter.latencyTotal / ecCount;

				tbCacheStoreStreamCountersUsage->addRowData(CSS_NAME        ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<std::string>(ecName) ));              
				tbCacheStoreStreamCountersUsage->addRowData(CSS_COUNT       ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(ecCount) ));	            
				tbCacheStoreStreamCountersUsage->addRowData(CSS_FAIL_COUNT  ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(ecFailCount) ));
				tbCacheStoreStreamCountersUsage->addRowData(CSS_LATENCY_AVG ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(ecLatencyAvg) ));
				tbCacheStoreStreamCountersUsage->addRowData(CSS_LATENCY_MAX ,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(ecLatencyMax) )); 

				(*reporter)(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStoreStreamCounters, "cdnss snmp create CacheStoreStreamCounters, row[%d], name[%s] count[%d] failCount[%d] latencyTotal[%d] latencyMax[%d]"),
					                   rowIndex, exportCounter.name, exportCounter.count, exportCounter.failCount, exportCounter.latencyTotal,  exportCounter.latencyMax);
			}
		}
		catch(const ::Ice::Exception& ex)
		{
			(*reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(CacheStoreStreamCounters, "cdnss snmp create CacheStoreStreamCounters, row[%d], name[%s], caught exception[%s]"), rowIndex, _store._exportCounters[rowIndex - 1].name, ex.ice_name().c_str());
		}
		catch(...)
		{
			(*reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(CacheStoreStreamCounters, "cdnss snmp create CacheStoreStreamCounters, row[%d], name[%s],caught exception[unknown]"), rowIndex, _store._exportCounters[rowIndex - 1].name);
		}

		return tbCacheStoreStreamCountersUsage;
	}

private:
	ZQTianShan::ContentStore::CacheStoreImpl & _store;
};


class CdnssCacheMeasureReset: public ZQ::Snmp::IVariable
{
public:
	CdnssCacheMeasureReset(ZQTianShan::ContentStore::CacheStoreImpl & store)
		:_store(store){};

	virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType)
	{
		int nRev = true;
		return smivalFrom(val, nRev, desiredType);
	}

	virtual bool set(const ZQ::Snmp::SmiValue& val)
	{
		_store.resetCounters();
		return true;
	}

	virtual bool validate(const ZQ::Snmp::SmiValue& val) const
	{
		return true;
	}

private:
	ZQTianShan::ContentStore::CacheStoreImpl & _store;
};

class CdnssCacheMeasureSince: public ZQ::Snmp::IVariable
{
public:
	CdnssCacheMeasureSince(ZQTianShan::ContentStore::CacheStoreImpl & store)
		:_store(store){};

	virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType)
	{
		char tempbuf[80] = {0};
		std::string measureSince(ZQTianShan::TimeToUTC(_store._stampMesureSince, tempbuf, sizeof(tempbuf)-2));

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
    ZQTianShan::ContentStore::CacheStoreImpl & _store;
};

class TypeInstance: public ZQ::Snmp::IVariable
{
public:
	TypeInstance(int & store)
		:_store(store){};

	virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType)
	{
		int desired = _store;
		return smivalFrom(val, desired, desiredType);
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
	int & _store;
};

typedef TypeInstance CacheMissedSize;
typedef TypeInstance CacheHotLocalsSize;
typedef TypeInstance CacheRequestsInTimeWin;
typedef TypeInstance CacheHitInTimeWin;

#endif  //__ZQ_CDNSS_SNMP_EXP_H__
