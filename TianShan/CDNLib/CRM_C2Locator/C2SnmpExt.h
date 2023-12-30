#ifndef __ZQ_C2SnmpExt_H__
#define __ZQ_C2SnmpExt_H__
#include "snmp/ZQSnmpMgmt.hpp"

#include "ClientManager.h"
#include "TransferPortManager.h"
#include "Text.h"

template<typename Type>
class TableMediatorVar: public ZQ::Snmp::IVariable
{
public:
	TableMediatorVar(Type var):_val(var){}

	~TableMediatorVar(){}

	TableMediatorVar & operator=(const TableMediatorVar & other)
	{
		if (this != &other)
			_val = other._val;

		return *this;
	}

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
	ZQ::Snmp::Subagent *        _snmpTableAgent;
	mutable ZQ::Snmp::TablePtr        _inStoreTable;
	ZQ::common::FileLog *     _reporter;
	TableClass      _createTable;	
	TableData &     _tableEnv;
};

class TransferPortTable
{
public:
	TransferPortTable(ZQTianShan::CDN::TransferPortManager & tableEnv)
		:_portMgr(tableEnv){}

	~TransferPortTable(){}

	ZQ::Snmp::TablePtr  operator()(ZQ::common::FileLog * reporter)
	{
		ZQ::Snmp::TablePtr tbTransferPortUsage(new ZQ::Snmp::Table());
		enum TransforPortTableColunm
		{
			TP_NAME = 1,
			TP_STATUS,
			TP_SESSION,
			TP_BW_SUBTOTAL,
			TP_BW_MAX,
			TP_ENABLED,
			TP_PENALTY,
			TP_ADDRESS,
			TABLE_COLUNM_COUNT
		};

		tbTransferPortUsage->addColumn( (uint32)TP_NAME,	   ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);            
		tbTransferPortUsage->addColumn( (uint32)TP_STATUS,	    ZQ::Snmp::AsnType_Integer,		ZQ::Snmp::aReadOnly); 
		tbTransferPortUsage->addColumn( (uint32)TP_SESSION,	    ZQ::Snmp::AsnType_Integer,	    ZQ::Snmp::aReadOnly);    
		tbTransferPortUsage->addColumn( (uint32)TP_BW_SUBTOTAL,	ZQ::Snmp::AsnType_Integer,	    ZQ::Snmp::aReadOnly); 
		tbTransferPortUsage->addColumn( (uint32)TP_BW_MAX,	    ZQ::Snmp::AsnType_Integer,	    ZQ::Snmp::aReadOnly);        
		tbTransferPortUsage->addColumn( (uint32)TP_ENABLED,	    ZQ::Snmp::AsnType_Integer,	    ZQ::Snmp::aReadOnly);    
		tbTransferPortUsage->addColumn( (uint32)TP_PENALTY,	    ZQ::Snmp::AsnType_Integer,	    ZQ::Snmp::aReadOnly); 
		tbTransferPortUsage->addColumn( (uint32)TP_ADDRESS,	    ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly); 

		ZQTianShan::CDN::TransferPortManager::PortInfos  transferPorts;
		_portMgr.snapshotPortInfos(transferPorts);
		(*reporter)(ZQ::common::Log::L_DEBUG, CLOGFMT(TransferPortTable, "snmp TransferPort table created, column[%d], transferPorts size[%d]"), TABLE_COLUNM_COUNT -  1, transferPorts.size());
		int rowIndex = 1;
		for (std::vector< ZQTianShan::CDN::TransferPortManager::PortInfo>::iterator it = transferPorts.begin(); 
			it != transferPorts.end(); ++it)
		{
			try
			{
				ZQTianShan::CDN::TransferPortManager::PortInfo & portInfoIt = *it;
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
				ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex( rowIndex );

				tbTransferPortUsage->addRowData((uint32)TP_NAME,	      indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<std::string>(name) ));              
				tbTransferPortUsage->addRowData((uint32)TP_STATUS,	      indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(isUp) ));	            
				tbTransferPortUsage->addRowData((uint32)TP_SESSION,	      indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(activeTransferCount) ));
				tbTransferPortUsage->addRowData((uint32)TP_BW_SUBTOTAL,   indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(activeBandwidth) ));             
				tbTransferPortUsage->addRowData((uint32)TP_BW_MAX,	      indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(capacity) ));	 
				tbTransferPortUsage->addRowData((uint32)TP_ENABLED,	      indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(enabled) ));             
				tbTransferPortUsage->addRowData((uint32)TP_PENALTY,	      indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(penalty) ));              
				tbTransferPortUsage->addRowData((uint32)TP_ADDRESS,	      indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<std::string>(address) ));
     
				++rowIndex;
			}
			catch (...) 
			{
				(*reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(TransferPortTable, "snmp TransferPort table add data error, row[%d], transferPorts size[%d]"), rowIndex, transferPorts.size());
			}
		}

		(*reporter)(ZQ::common::Log::L_DEBUG, CLOGFMT(TransferPortTable, "snmp TransferPort table end, row[%d], transferPorts size[%d]"), rowIndex -1 , transferPorts.size());
		return tbTransferPortUsage;
	}

private:
	ZQTianShan::CDN::TransferPortManager&  _portMgr;
};


class ClientTransferTable
{
public:
	ClientTransferTable(ZQTianShan::CDN::ClientManager& clientMgr)
		:_clientMgr(clientMgr){}

	~ClientTransferTable(){}

	ZQ::Snmp::TablePtr  operator()(ZQ::common::FileLog * reporter)
	{
		ZQ::Snmp::TablePtr tbClientTransferUsage(new ZQ::Snmp::Table());
		enum ClientTransferColunm
		{
			CT_NAME = 1,
			CT_SESSION,
			CT_BW_SUBTOTAL,
			CT_BW_MAX,
			TABLE_COLUNM_COUNT
		};

		tbClientTransferUsage->addColumn(CT_NAME,         ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);            
		tbClientTransferUsage->addColumn(CT_SESSION,      ZQ::Snmp::AsnType_Integer,	ZQ::Snmp::aReadOnly);    
		tbClientTransferUsage->addColumn(CT_BW_SUBTOTAL,  ZQ::Snmp::AsnType_Integer,	ZQ::Snmp::aReadOnly); 
		tbClientTransferUsage->addColumn(CT_BW_MAX,       ZQ::Snmp::AsnType_Integer,	ZQ::Snmp::aReadOnly);        

		typedef ::std::vector< ::TianShanIce::SCS::ClientTransfer >::iterator itClientTransfer;
		::TianShanIce::SCS::ClientTransfers  clientTransfers;
		_clientMgr.snapshotTransfers(clientTransfers);
		(*reporter)(ZQ::common::Log::L_DEBUG, CLOGFMT(ClientTransferTable, "snmp ClientTransfer table, colunm[%d], clientTransfers size[%d]"), TABLE_COLUNM_COUNT - 1, clientTransfers.size());

		int rowIndex = 1;
		for (itClientTransfer it = clientTransfers.begin(); it != clientTransfers.end(); ++it)
		{
			try
			{
				int consumedBandwidth = it->consumedBandwidth / 1000;
                int ingressCapacity = it->ingressCapacity / 1000;

				ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex(rowIndex);
				tbClientTransferUsage->addRowData(CT_NAME,         indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<std::string>(it->address) )); 
				tbClientTransferUsage->addRowData(CT_SESSION,      indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(it->activeTransferCount) ));    
				tbClientTransferUsage->addRowData(CT_BW_SUBTOTAL,  indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(consumedBandwidth) ));
				tbClientTransferUsage->addRowData(CT_BW_MAX,       indexOid , ZQ::Snmp::VariablePtr( new TableMediatorVar<int>(ingressCapacity) )); 
				++rowIndex;
			}
			catch (...)
			{
                (*reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(ClientTransferTable, "snmp ClientTransfer table add data error, row[%d], clientTransfers size[%d]"), rowIndex, clientTransfers.size());
			}
		}

		(*reporter)(ZQ::common::Log::L_DEBUG, CLOGFMT(ClientTransferTable, "snmp ClientTransfer table end, colunm[%d], row[%d], clientTransfers size[%d]"), TABLE_COLUNM_COUNT - 1, rowIndex - 1, clientTransfers.size());
		return tbClientTransferUsage;
	}

private:
	ZQTianShan::CDN::ClientManager&  _clientMgr;
};
#endif  //__ZQ_C2SnmpExt_H__
