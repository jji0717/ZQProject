#ifndef __ZQ_HttpCRGSnmpExt_H__
#define __ZQ_HttpCRGSnmpExt_H__
#include "snmp/ZQSnmpMgmt.hpp"
#include "HttpEngine.h"

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
	ZQ::common::FileLog *     _reporter;
	TableClass      _createTable;	
	TableData &     _tableEnv;
};

class LocateRequestTable
{
public:
	LocateRequestTable(ZQHttp::EngineStatistics & tableEnv)
		:_rpStatistics(tableEnv){}

	~LocateRequestTable(){}

	ZQ::Snmp::TablePtr  operator()(ZQ::common::FileLog * reporter)
	{
		ZQ::Snmp::TablePtr tbLocateRequestUsage(new ZQ::Snmp::Table());
		enum LocateRequestTableColunm
		{
			REQ_METHOD = 1,
			REQ_COUNT,
			REQ_LANTENCY_AVG,
			REQ_LANTENCY_MAX,
			REQ_2XX,
			REQ_400,
			REQ_404,
			REQ_500,
			REQ_503,
			TABLE_COLUNM_COUNT
		};

		tbLocateRequestUsage->addColumn( REQ_METHOD,	    ZQ::Snmp::AsnType_Octets,      ZQ::Snmp::aReadOnly);            
		tbLocateRequestUsage->addColumn( REQ_COUNT,	        ZQ::Snmp::AsnType_Integer,	   ZQ::Snmp::aReadOnly); 
		tbLocateRequestUsage->addColumn( REQ_LANTENCY_AVG,	ZQ::Snmp::AsnType_Integer,     ZQ::Snmp::aReadOnly);    
		tbLocateRequestUsage->addColumn( REQ_LANTENCY_MAX,	ZQ::Snmp::AsnType_Integer,     ZQ::Snmp::aReadOnly); 
		tbLocateRequestUsage->addColumn( REQ_2XX,	        ZQ::Snmp::AsnType_Integer,	   ZQ::Snmp::aReadOnly);        
		tbLocateRequestUsage->addColumn( REQ_400,	        ZQ::Snmp::AsnType_Integer,	   ZQ::Snmp::aReadOnly);    
		tbLocateRequestUsage->addColumn( REQ_404,	        ZQ::Snmp::AsnType_Integer,	   ZQ::Snmp::aReadOnly); 
		tbLocateRequestUsage->addColumn( REQ_500,	        ZQ::Snmp::AsnType_Integer,     ZQ::Snmp::aReadOnly); 
		tbLocateRequestUsage->addColumn( REQ_503,	        ZQ::Snmp::AsnType_Integer,     ZQ::Snmp::aReadOnly); 

		ZQHttp::EngineStatistics::RPSTATUSMAP rpStatusMap;
	    size_t revSize = _rpStatistics.getStatistics(rpStatusMap);
		(*reporter)(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateRequestTable, "snmp Request table created, column[%d], rpStatusMap size[%d], revSize[%d]"), TABLE_COLUNM_COUNT -  1, rpStatusMap.size(), revSize);
		int rowIndex = 1;
		for (ZQHttp::EngineStatistics::RPSTATUSMAP::iterator it = rpStatusMap.begin(); 
			it != rpStatusMap.end(); ++it)
		{
			try
			{
				ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex(rowIndex);
				ZQHttp::EngineStatistics::RequestProcessingStatus& rpStatus = it->second;
				std::string reqMethod(rpStatus.mtdString);
				int reqCount(rpStatus.totalCount);
				int reqLantencyAvg(rpStatus.latencyInMsHeaderAvg);
				int reqLantencyMax(rpStatus.latencyInMsHeaderMax);
				int req_2xx = rpStatus.reqCount[ZQHttp::EngineStatistics::RESP_2XX];
				int req_400 = rpStatus.reqCount[ZQHttp::EngineStatistics::RESP_400];
				int req_404 = rpStatus.reqCount[ZQHttp::EngineStatistics::RESP_404];// this is defferent from  RESP_416, need confirm
				int req_500 = rpStatus.reqCount[ZQHttp::EngineStatistics::RESP_500];
				int req_503 = rpStatus.reqCount[ZQHttp::EngineStatistics::RESP_503];
																		  
				tbLocateRequestUsage->addRowData(REQ_METHOD,	    indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<std::string>(reqMethod) ));              
				tbLocateRequestUsage->addRowData(REQ_COUNT,	        indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(reqCount) ));	            
				tbLocateRequestUsage->addRowData(REQ_LANTENCY_AVG,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(reqLantencyAvg) ));
				tbLocateRequestUsage->addRowData(REQ_LANTENCY_MAX,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(reqLantencyMax) ));
				tbLocateRequestUsage->addRowData(REQ_2XX,	        indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(req_2xx) ));
				tbLocateRequestUsage->addRowData(REQ_400,	        indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(req_400) ));
				tbLocateRequestUsage->addRowData(REQ_404,	        indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(req_404) ));
				tbLocateRequestUsage->addRowData(REQ_500,	        indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(req_500) ));
				tbLocateRequestUsage->addRowData(REQ_503,	        indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(req_503) ));

				++rowIndex;
			}
			catch (...) 
			{
				(*reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(LocateRequestTable, "httpcrg snmp Request table add data error, row[%d], rpStatusMap size[%d]"), rowIndex, rpStatusMap.size());
			}
		}

		(*reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(LocateRequestTable, "httpcrg snmp create Request Table end, row[%d], rpStatusMap size[%d]"), rowIndex, rpStatusMap.size());
		return tbLocateRequestUsage;
	}

private:
	ZQHttp::EngineStatistics &  _rpStatistics;
};


class HttpCRGMeasureReset: public ZQ::Snmp::IVariable
{
public:
	HttpCRGMeasureReset(ZQHttp::EngineStatistics & httpEngineState)
		:_httpEngineState(httpEngineState){};

	virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType)
	{
		int nRev = true;
		return smivalFrom(val, nRev, desiredType);
	}

	virtual bool set(const ZQ::Snmp::SmiValue& val)
	{
		_httpEngineState.reset();
		return true;
	}

	virtual bool validate(const ZQ::Snmp::SmiValue& val) const
	{
		return true;
	}

private:
	ZQHttp::EngineStatistics & _httpEngineState;
};

class HttpCRGMeasureSince: public ZQ::Snmp::IVariable
{
public:
	HttpCRGMeasureSince(ZQHttp::EngineStatistics & httpEngineState)
		:_httpEngineState(httpEngineState){};

	virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType)
	{
		char  tempBuf[80] = {0};
		int64 mesureSinceTime = _httpEngineState.getMesuredSince();
		std::string  measureSince(ZQTianShan::TimeToUTC(mesureSinceTime, tempBuf, sizeof(tempBuf)-2) );

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
	ZQHttp::EngineStatistics & _httpEngineState;
};

#endif  //__ZQ_HttpCRGSnmpExt_H__
