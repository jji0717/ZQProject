#include "EdgeRMEnv.h"
#include "EdgeRMEnvSnmp.h"
#include "EdgeRMService.h" 
#include "../common/TianShanDefines.h"

extern  ZQ::common::BaseZQServiceApplication *Application;

namespace ZQTianShan {
	namespace EdgeRM {

template<typename TableClass, typename TableData>
class TableMediator: public ZQ::Snmp::IManaged
{
public:
	TableMediator(ZQ::common::Log& reporter, const ZQ::Snmp::Oid subid, ZQ::Snmp::Subagent* snmpTableAgent, TableData& tableEnv)
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
	ZQ::common::Log&          _reporter;
	TableClass                _createTable;	
};

class ErmDevicesTable
{
private:
	typedef DECLARE_SNMP_RO_TYPE(std::string, std::string, std::string)   RoEdString;
	typedef TianShanIce::EdgeResource::EdgeDeviceInfos::iterator          DeviceInfos_iter;

	enum ErmDevicesColumn
	{
		ED_NULL		      = 0,
		ED_ZONE           ,
		ED_DEVICE_NAME    ,
		ED_VERNDOR        ,
		ED_MODEL          ,
		ED_DESCRIPTION    ,
		ED_TFTP           ,
		ED_ADMIN_URL      ,
		ED_COLUNM_COUNT
	};

public:
	ErmDevicesTable(ZQTianShan::EdgeRM::EdgeRMEnv& edgeRMEnv)
		:_edgeRmEnv(edgeRMEnv)
	{}

	ZQ::Snmp::TablePtr  operator()(ZQ::common::Log& reporter)
	{
		int  rowIndex = 0;
		ZQ::Snmp::TablePtr tbEd(new ZQ::Snmp::Table());

		tbEd->addColumn(ED_ZONE        , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
		tbEd->addColumn(ED_DEVICE_NAME , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly);
		tbEd->addColumn(ED_VERNDOR     , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
		tbEd->addColumn(ED_MODEL       , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
		tbEd->addColumn(ED_DESCRIPTION , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly);            
		tbEd->addColumn(ED_TFTP        , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
		tbEd->addColumn(ED_ADMIN_URL   , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly);

		ZQTianShan::EdgeRM::EdgeRMImpl::Ptr edgeRmPtr = _edgeRmEnv.getEdgeRmPtr();
		TianShanIce::EdgeResource::EdgeRM*  edgeRmP = edgeRmPtr.get();

		if (!edgeRmPtr)
		{
			(reporter)(ZQ::common::Log::L_INFO, CLOGFMT(ErmDevicesTable, "EdgeRM create ErmDevicesTable Table end, edgeRmPtr[empty] row[%d]"), rowIndex);
			return tbEd;
		}

		TianShanIce::EdgeResource::EdgeDeviceInfos deviceInfos;
		TianShanIce::StrValues expectedMetaData;

		expectedMetaData.push_back(SYS_PROP(Zone));
		expectedMetaData.push_back(SYS_PROP(Vendor));
		expectedMetaData.push_back(SYS_PROP(Model));
		expectedMetaData.push_back(SYS_PROP(Desc));
		expectedMetaData.push_back(SYS_PROP(Tftp));
		expectedMetaData.push_back(SYS_PROP(AdminUrl));
		deviceInfos = edgeRmP->listDevices(expectedMetaData, Ice::Current());

		for (DeviceInfos_iter it = deviceInfos.begin(); it != deviceInfos.end(); ++it)
		{
			TianShanIce::ObjectInfo& deviceInfo = *it;
			TianShanIce::Properties& props = deviceInfo.props;
			TianShanIce::Properties::iterator itTemp;

#define GET_STR_FROM_MAP_BY_INDEX(itTemp,mapSrc,index) \
	(itTemp = mapSrc.find(index), (itTemp != mapSrc.end()) ? itTemp->second : "" )

			std::string zone        = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(Zone));
			std::string deviceName  = deviceInfo.ident.name;
			std::string vendor      = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(Vendor));
			std::string model       = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(Model));
			std::string description = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(Desc));
			std::string tftp        = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(Tftp));
			std::string adminUrl    = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(AdminUrl));

			ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex(rowIndex);           
			tbEd->addRowData(ED_ZONE        , indexOid, ZQ::Snmp::VariablePtr( new RoEdString(zone) ));	            
			tbEd->addRowData(ED_DEVICE_NAME , indexOid, ZQ::Snmp::VariablePtr( new RoEdString(deviceName) ));
			tbEd->addRowData(ED_VERNDOR     , indexOid, ZQ::Snmp::VariablePtr( new RoEdString(vendor) ));
			tbEd->addRowData(ED_MODEL       , indexOid, ZQ::Snmp::VariablePtr( new RoEdString(model) ));
			tbEd->addRowData(ED_DESCRIPTION , indexOid, ZQ::Snmp::VariablePtr( new RoEdString(description) ));              
			tbEd->addRowData(ED_TFTP        , indexOid, ZQ::Snmp::VariablePtr( new RoEdString(tftp) ));	            
			tbEd->addRowData(ED_ADMIN_URL   , indexOid, ZQ::Snmp::VariablePtr( new RoEdString(adminUrl) ));	 
		}

		(reporter)(ZQ::common::Log::L_INFO, CLOGFMT(ErmDevicesTable, "EdgeRM create ErmDevicesTable Table end, row[%d]"), rowIndex);
		return tbEd;
	}

private:
	ZQTianShan::EdgeRM::EdgeRMEnv& _edgeRmEnv;
};


class ErmPortsTable
{
private:
	typedef DECLARE_SNMP_RO_TYPE(int, int, int)                           RoEpInt;
	typedef DECLARE_SNMP_RO_TYPE(std::string, std::string, std::string)   RoEpString;
	typedef TianShanIce::EdgeResource::EdgeDeviceInfos::iterator          DeviceInfos_iter;
	typedef TianShanIce::EdgeResource::EdgePortInfos::iterator            PortInfos_iter;

	enum ErmPortsColumn
	{
		EP_NULL		         = 0,
		EP_PORT_ID           ,
		EP_POWER_LEVEL       ,
		EP_MODULATION_FORMAT ,
		EP_INTER_LEAVER_MODE ,
		EP_FEC               ,
		EP_DEVICE_IP         ,
		EP_DEVICE_GROUP      ,
		EP_COLUNM_COUNT
	};

public:
	ErmPortsTable(ZQTianShan::EdgeRM::EdgeRMEnv& edgeRMEnv)
		:_edgeRmEnv(edgeRMEnv)
	{}

	ZQ::Snmp::TablePtr  operator()(ZQ::common::Log& reporter)
	{
		int  rowIndex = 0;
		ZQ::Snmp::TablePtr tbEp(new ZQ::Snmp::Table());
		ZQTianShan::EdgeRM::EdgeRMImpl::Ptr edgeRmPtr = _edgeRmEnv.getEdgeRmPtr();
		TianShanIce::EdgeResource::EdgeRM*  edgeRmP   = edgeRmPtr.get();

		tbEp->addColumn(EP_PORT_ID           , ZQ::Snmp::AsnType_Integer , ZQ::Snmp::aReadOnly); 
		tbEp->addColumn(EP_POWER_LEVEL       , ZQ::Snmp::AsnType_Integer , ZQ::Snmp::aReadOnly);
		tbEp->addColumn(EP_MODULATION_FORMAT , ZQ::Snmp::AsnType_Integer , ZQ::Snmp::aReadOnly); 
		tbEp->addColumn(EP_INTER_LEAVER_MODE , ZQ::Snmp::AsnType_Integer , ZQ::Snmp::aReadOnly); 
		tbEp->addColumn(EP_FEC               , ZQ::Snmp::AsnType_Integer , ZQ::Snmp::aReadOnly);            
		tbEp->addColumn(EP_DEVICE_IP         , ZQ::Snmp::AsnType_Octets  , ZQ::Snmp::aReadOnly); 
		tbEp->addColumn(EP_DEVICE_GROUP      , ZQ::Snmp::AsnType_Octets  , ZQ::Snmp::aReadOnly);

		if (!edgeRmPtr)
		{
			(reporter)(ZQ::common::Log::L_INFO, CLOGFMT(ErmPortsTable, "EdgeRM create ErmPortsTable Table end, edgeRmPtr[empty] row[%d]"), rowIndex);
			return tbEp;
		}

		TianShanIce::EdgeResource::EdgeDeviceInfos deviceInfos;
		TianShanIce::StrValues expectedMetaData;

		deviceInfos = edgeRmP->listDevices(expectedMetaData, Ice::Current());
		for (DeviceInfos_iter it = deviceInfos.begin(); it != deviceInfos.end(); ++it)
		{
			TianShanIce::ObjectInfo& deviceInfo = *it;
			TianShanIce::EdgeResource::EdgePortInfos  portInfos;
			TianShanIce::EdgeResource::EdgeDevicePrx  devicePrx;
			try
			{
				devicePrx = edgeRmP->openDevice(deviceInfo.ident.name);
				portInfos = devicePrx->listEdgePorts();
			}
			catch (const TianShanIce::BaseException& ex)
			{
				(reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(ErmPortsTable, "ErmRM create ErmPortsTable exception[TianShanIce::BaseExceptionn %s], device[%s], row[%d]"), 
					ex.ice_name().c_str(), rowIndex, deviceInfo.ident.name.c_str());
				continue;
			}
			catch (const Ice::Exception& ex)
			{
				(reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(ErmPortsTable, "ErmRM create ErmPortsTable exception[Ice::Exception %s], device[%s], row[%d]"), 
					ex.ice_name().c_str(), rowIndex, deviceInfo.ident.name.c_str());
				continue;
			}
			catch (...) 
			{
				(reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(ErmPortsTable, "ErmRM create ErmPortsTable exception[unknown], device[%s], row[%d]"), 
					rowIndex, deviceInfo.ident.name.c_str());
				continue;
			}

			for (PortInfos_iter itPI = portInfos.begin(); itPI != portInfos.end(); ++itPI)
			{
				++rowIndex;
				TianShanIce::ValueMap& resAtscData  = itPI->resAtscModulationMode.resourceData;
				TianShanIce::ValueMap& resPhyChData = itPI->resPhysicalChannel.resourceData;
				int portID           = itPI->Id;
				int powerLevel       = itPI->powerLevel;
				int modulationFromat = regexInt(resAtscData, std::string("modulationFormat"));
				int interLeaverMode  = regexInt(resAtscData, std::string("interleaveDepth"));
				int FEC              = regexInt(resAtscData, std::string("FEC"));
				std::string deviceIP    = regexStr(resPhyChData, std::string("edgeDeviceIP"));
				std::string deviceGroup = regexStr(resPhyChData, std::string("edgeDeviceZone"));;

				ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex(rowIndex);           
				tbEp->addRowData(EP_PORT_ID           , indexOid, ZQ::Snmp::VariablePtr( new RoEpInt(portID) ));	            
				tbEp->addRowData(EP_POWER_LEVEL       , indexOid, ZQ::Snmp::VariablePtr( new RoEpInt(powerLevel) ));
				tbEp->addRowData(EP_MODULATION_FORMAT , indexOid, ZQ::Snmp::VariablePtr( new RoEpInt(modulationFromat) ));
				tbEp->addRowData(EP_INTER_LEAVER_MODE , indexOid, ZQ::Snmp::VariablePtr( new RoEpInt(interLeaverMode) ));
				tbEp->addRowData(EP_FEC               , indexOid, ZQ::Snmp::VariablePtr( new RoEpInt(FEC) ));              
				tbEp->addRowData(EP_DEVICE_IP         , indexOid, ZQ::Snmp::VariablePtr( new RoEpString(deviceIP) ));	            
				tbEp->addRowData(EP_DEVICE_GROUP      , indexOid, ZQ::Snmp::VariablePtr( new RoEpString(deviceGroup) ));
			}						 
		}

		(reporter)(ZQ::common::Log::L_INFO, CLOGFMT(ErmPortsTable, "ErmRM create ErmPortsTable end, row[%d]"), rowIndex);
		return tbEp;
	}

private:
	int regexInt(TianShanIce::ValueMap& resAtscData, std::string target)
	{
		typedef TianShanIce::ValueMap::iterator ItType;
		int destInt = -1;
		ItType itor = resAtscData.find(target);
		if (itor != resAtscData.end())
		{
			const TianShanIce::Variant& destVar = itor->second;
			if (TianShanIce::vtBin == destVar.type && destVar.bin.size() > 0)
				destInt = destVar.bin[0];
		}

		return destInt;
	}
	std::string regexStr(TianShanIce::ValueMap& resPhyChData, std::string target)
	{
		typedef TianShanIce::ValueMap::iterator ItType;
		std::string destStr;
		ItType itor = resPhyChData.find(target);

		if (itor != resPhyChData.end())
		{
			const TianShanIce::Variant& destVar = itor->second;
			if (TianShanIce::vtStrings == destVar.type && destVar.strs.size() > 0)
				destStr = destVar.strs[0];
		}

		return destStr;
	}

private:
	ZQTianShan::EdgeRM::EdgeRMEnv& _edgeRmEnv;
};


class ErmChannelsTable
{
private:
	typedef  std::list< TianShanIce::EdgeResource::EdgeChannelInfos>      EdgeChannelInfosList;
	typedef  TianShanIce::EdgeResource::EdgeDeviceInfos::iterator         DeviceInfos_iter;
	typedef  TianShanIce::EdgeResource::EdgePortInfos::iterator           PortInfos_iter;
	typedef  TianShanIce::EdgeResource::EdgeChannelInfos::iterator        ChannelInfos_iter;
	typedef  DECLARE_SNMP_RO_TYPE(int, int, int)                          RoEcInt;
	typedef  DECLARE_SNMP_RO_TYPE(std::string, std::string, std::string)  RoEcString;

	enum ErmChannelsColumn
	{
		EC_NULL		                  = 0,
		EC_NAME                       ,
		EC_STAMP_LAST_UPDATED         ,
		EC_ENABLED                    ,
		EC_FREQ_RF                    ,
		EC_TSID                       ,
		EC_NITPID                     ,
		EC_START_UDP_PORT             ,
		EC_UDP_PORT_STEP_BY_PN        ,
		EC_START_PROGRAM_NUMBER       ,
		EC_LOW_BANDWIDTH_UTILIZATION  ,
		EC_HIGH_BANDWIDTH_UTILIZATION ,
		EC_MAX_SESSIONS               ,
		EC_INTERVAL_PAT               ,
		EC_INTERVAL_PMT               ,
		EC_SYMBOL_RATE                ,
		EC_ALLOCATION                 ,
		EC_COLUNM_COUNT
	};

public:
	ErmChannelsTable(ZQTianShan::EdgeRM::EdgeRMEnv& edgeRMEnv)
		:_edgeRmEnv(edgeRMEnv)
	{}

	ZQ::Snmp::TablePtr  operator()(ZQ::common::Log& reporter)
	{
		ZQ::Snmp::TablePtr tbEc(new ZQ::Snmp::Table());
		int  rowIndex = 0;

		tbEc->addColumn(EC_NAME                       , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
		tbEc->addColumn(EC_STAMP_LAST_UPDATED         , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly);
		tbEc->addColumn(EC_ENABLED                    , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
		tbEc->addColumn(EC_FREQ_RF                    , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
		tbEc->addColumn(EC_TSID                       , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly);            
		tbEc->addColumn(EC_NITPID                     , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
		tbEc->addColumn(EC_START_UDP_PORT             , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly);
		tbEc->addColumn(EC_UDP_PORT_STEP_BY_PN        , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
		tbEc->addColumn(EC_START_PROGRAM_NUMBER       , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
		tbEc->addColumn(EC_LOW_BANDWIDTH_UTILIZATION  , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
		tbEc->addColumn(EC_HIGH_BANDWIDTH_UTILIZATION , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly);            
		tbEc->addColumn(EC_MAX_SESSIONS               , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
		tbEc->addColumn(EC_INTERVAL_PAT               , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly);
		tbEc->addColumn(EC_INTERVAL_PMT               , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
		tbEc->addColumn(EC_SYMBOL_RATE                , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
		tbEc->addColumn(EC_ALLOCATION                 , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 

		EdgeChannelInfosList channelInfosList;
		getEdgeChannelInfosList(channelInfosList, reporter);
		for (EdgeChannelInfosList::iterator itList = channelInfosList.begin(); itList != channelInfosList.end(); ++itList)
		{
			TianShanIce::EdgeResource::EdgeChannelInfos& channelInfos = *itList;
			for (ChannelInfos_iter itCI = channelInfos.begin(); itCI != channelInfos.end(); ++itCI)
			{
				++rowIndex;
				TianShanIce::Properties& props = itCI->props;
				TianShanIce::Properties::iterator itTemp;
				ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex(rowIndex);  

#define GET_STR_FROM_MAP_BY_INDEX(itTemp,mapSrc,index) \
	(itTemp = mapSrc.find(index), (itTemp != mapSrc.end()) ? itTemp->second : "" )

				std::string name = itCI->ident.name;
				std::string stampLastUpdated = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(StampLastUpdated));
				std::string enabled = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(Enabled));
				std::string freqRF  = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(FreqRF));
				std::string tsid    = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(TSID));
				std::string nitPid  = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(NITPID));
				std::string startUdpPort    = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(StartUDPPort));
				std::string udpPortStepByPN = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(UdpPortStepByPn));
				std::string startProgramNumber       = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(StartProgramNumber));
				std::string lowBandwidthUtilization  = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(LowBandwidthUtilization));
				std::string highBandwidthUtilization = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(HighBandwidthUtilization));
				std::string maxSessions = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(MaxSessions));
				std::string intervalPAT = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(IntervalPAT));
				std::string intervalPMT = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(IntervalPMT));
				std::string symbolRate  = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(symbolRate));
				std::string allocation  = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(AllocationCount));
				char timeBuffer[128]    = {0};
				ZQ::common::TimeUtil::TimeToUTC(_atoi64(stampLastUpdated.c_str()),timeBuffer,sizeof(timeBuffer),true);
				std::string stampLastUpdatedConvert(timeBuffer);

				tbEc->addRowData(EC_NAME                       , indexOid, ZQ::Snmp::VariablePtr( new RoEcString(name) ));	            
				tbEc->addRowData(EC_STAMP_LAST_UPDATED         , indexOid, ZQ::Snmp::VariablePtr( new RoEcString(stampLastUpdatedConvert) ));
				tbEc->addRowData(EC_ENABLED                    , indexOid, ZQ::Snmp::VariablePtr( new RoEcString(enabled) ));
				tbEc->addRowData(EC_FREQ_RF                    , indexOid, ZQ::Snmp::VariablePtr( new RoEcString(freqRF) ));
				tbEc->addRowData(EC_TSID                       , indexOid, ZQ::Snmp::VariablePtr( new RoEcString(tsid) ));              
				tbEc->addRowData(EC_NITPID                     , indexOid, ZQ::Snmp::VariablePtr( new RoEcString(nitPid ) ));	            
				tbEc->addRowData(EC_START_UDP_PORT             , indexOid, ZQ::Snmp::VariablePtr( new RoEcString(startUdpPort) ));
				tbEc->addRowData(EC_UDP_PORT_STEP_BY_PN        , indexOid, ZQ::Snmp::VariablePtr( new RoEcString(udpPortStepByPN) ));
				tbEc->addRowData(EC_START_PROGRAM_NUMBER       , indexOid, ZQ::Snmp::VariablePtr( new RoEcString(startProgramNumber) ));
				tbEc->addRowData(EC_LOW_BANDWIDTH_UTILIZATION  , indexOid, ZQ::Snmp::VariablePtr( new RoEcString(lowBandwidthUtilization) ));
				tbEc->addRowData(EC_HIGH_BANDWIDTH_UTILIZATION , indexOid, ZQ::Snmp::VariablePtr( new RoEcString(highBandwidthUtilization) ));              
				tbEc->addRowData(EC_MAX_SESSIONS               , indexOid, ZQ::Snmp::VariablePtr( new RoEcString(maxSessions ) ));	            
				tbEc->addRowData(EC_INTERVAL_PAT               , indexOid, ZQ::Snmp::VariablePtr( new RoEcString(intervalPAT) ));
				tbEc->addRowData(EC_INTERVAL_PMT               , indexOid, ZQ::Snmp::VariablePtr( new RoEcString(intervalPMT) ));
				tbEc->addRowData(EC_SYMBOL_RATE                , indexOid, ZQ::Snmp::VariablePtr( new RoEcString(symbolRate) ));
				tbEc->addRowData(EC_ALLOCATION                 , indexOid, ZQ::Snmp::VariablePtr( new RoEcString(allocation) ));
			}
		}

		(reporter)(ZQ::common::Log::L_INFO, CLOGFMT(ErmChannelsTable, "ErmRM create ErmChannelsTable end, row[%d]"), rowIndex);
		return tbEc;
	}

private:
	bool getEdgeChannelInfosList(EdgeChannelInfosList& channelInfosList, ZQ::common::Log& reporter)//all EdgeChannelInfos store in list
	{
		int nRev = false;
		ZQTianShan::EdgeRM::EdgeRMImpl::Ptr edgeRmPtr = _edgeRmEnv.getEdgeRmPtr();
		TianShanIce::EdgeResource::EdgeRM*  edgeRmP   = edgeRmPtr.get();

		if (!edgeRmPtr)
		{
			(reporter)(ZQ::common::Log::L_INFO, CLOGFMT(ErmPortsTable, "EdgeRM getEdgeChannelInfosList end, edgeRmPtr[empty]"));
			return nRev;
		}

		TianShanIce::EdgeResource::EdgeDeviceInfos deviceInfos;
		TianShanIce::StrValues expectedMetaData;

		deviceInfos = edgeRmP->listDevices(expectedMetaData, Ice::Current());
		for (DeviceInfos_iter it = deviceInfos.begin(); it != deviceInfos.end(); ++it)
		{
			TianShanIce::ObjectInfo& deviceInfo = *it;
			Ice::ObjectPrx  objPrx = NULL;
			TianShanIce::EdgeResource::EdgeDevicePrx    devicePrx;
			TianShanIce::EdgeResource::EdgePortInfos    portInfos;

			try
			{ 
				devicePrx = edgeRmP->openDevice(deviceInfo.ident.name);
				objPrx    = devicePrx->ice_collocationOptimization(false);
				devicePrx = TianShanIce::EdgeResource::EdgeDevicePrx::uncheckedCast(objPrx);
				portInfos = devicePrx->listEdgePorts();
			}
			catch (const TianShanIce::BaseException& ex)
			{
				(reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(ErmPortsTable, "ErmRM getEdgeChannelInfosList ErmPortsTable exception[TianShanIce::BaseExceptionn %s], device[%s]"), 
					ex.ice_name().c_str(), deviceInfo.ident.name.c_str());
				continue;//deviceInfos
			}
			catch (const Ice::Exception& ex)
			{
				(reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(ErmPortsTable, "ErmRM getEdgeChannelInfosList ErmPortsTable exception[Ice::Exception %s], device[%s]"), 
					ex.ice_name().c_str(), deviceInfo.ident.name.c_str());
				continue;//deviceInfos
			}
			catch (...) 
			{
				(reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(ErmPortsTable, "ErmRM getEdgeChannelInfosList ErmPortsTable exception[unknown], device[%s]"), 
					deviceInfo.ident.name.c_str());
				continue;//deviceInfos
			}

			for (PortInfos_iter itPI = portInfos.begin(); itPI != portInfos.end(); ++itPI)
			{
				try
				{ 
					TianShanIce::StrValues expectedMetaData;
					TianShanIce::EdgeResource::EdgeChannelInfos channelInfos;

					expectedMetaData.push_back(SYS_PROP(FreqRF));
					expectedMetaData.push_back(SYS_PROP(symbolRate));
					expectedMetaData.push_back(SYS_PROP(TSID));
					expectedMetaData.push_back(SYS_PROP(IntervalPAT));
					expectedMetaData.push_back(SYS_PROP(IntervalPMT));
					expectedMetaData.push_back(SYS_PROP(StampLastUpdated));
					expectedMetaData.push_back(SYS_PROP(NITPID));
					expectedMetaData.push_back(SYS_PROP(StartUDPPort));
					expectedMetaData.push_back(SYS_PROP(UdpPortStepByPn));
					expectedMetaData.push_back(SYS_PROP(StartProgramNumber));
					expectedMetaData.push_back(SYS_PROP(MaxSessions));
					expectedMetaData.push_back(SYS_PROP(LowBandwidthUtilization));
					expectedMetaData.push_back(SYS_PROP(HighBandwidthUtilization));
					expectedMetaData.push_back(SYS_PROP(Enabled));
					expectedMetaData.push_back(SYS_PROP(AllocationCount));

					channelInfos = devicePrx->listChannels(itPI->Id, expectedMetaData, false);
					channelInfosList.push_back(channelInfos);
				}
				catch (const TianShanIce::BaseException& ex)
				{
					(reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(ErmPortsTable, "ErmRM listChannels ErmPortsTable exception[TianShanIce::BaseExceptionn %s], device[%s], portID[%d]"), 
						ex.ice_name().c_str(), deviceInfo.ident.name.c_str(), itPI->Id);
					continue;//portInfos
				}
				catch (const Ice::Exception& ex)
				{
					(reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(ErmPortsTable, "ErmRM listChannels ErmPortsTable exception[Ice::Exception %s], device[%s], portID[%d]"), 
						ex.ice_name().c_str(), deviceInfo.ident.name.c_str(),itPI->Id);
					continue;//portInfos
				}
				catch (...) 
				{
					(reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(ErmPortsTable, "ErmRM listChannels ErmPortsTable exception[unknown], device[%s], portID[%d]"), 
						deviceInfo.ident.name.c_str(), itPI->Id);
					continue;//portInfos
				}
			}// end loop portInfos			

			nRev = true;
		}//end loop deviceInfos

		return nRev;
	}

private:
	ZQTianShan::EdgeRM::EdgeRMEnv& _edgeRmEnv;
};


EnvSnmpRegistor::EnvSnmpRegistor()
:_ermSnmpAgent(0)
{}

EnvSnmpRegistor::~EnvSnmpRegistor()
{
	if (_ermSnmpAgent)
	{
		delete _ermSnmpAgent;
		_ermSnmpAgent = NULL;
	}
}

bool EnvSnmpRegistor::regSnmp(ZQTianShan::EdgeRM::EdgeRMEnv* edgeRmEnv)
{	
	using namespace ZQ::common;
	int instanceId = Application->getInstanceId();
	_ermSnmpAgent = new ZQ::Snmp::Subagent(2300, 3, instanceId);
	if (!_ermSnmpAgent)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EnvSnmpRegistor, "failed to initial snmp agent port[2300-%d-3]"), instanceId);
		return false;
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(EnvSnmpRegistor,"registerSnmp() port[2300-%d-3] entry"), instanceId);
	ZQ::common::Log& ermEnvLog = edgeRmEnv->getLogger();

	_ermSnmpAgent->setLogger(&ermEnvLog);
	_ermSnmpAgent->start();	

	typedef TableMediator<ErmDevicesTable,  ZQTianShan::EdgeRM::EdgeRMEnv >  ErmDevTblType;
	typedef TableMediator<ErmPortsTable,    ZQTianShan::EdgeRM::EdgeRMEnv >  ErmPortTblType;
	typedef TableMediator<ErmChannelsTable, ZQTianShan::EdgeRM::EdgeRMEnv >  ErmChanTblType;

	ZQ::Snmp::ManagedPtr ermDevTbl (new ErmDevTblType (ermEnvLog, ZQ::Snmp::Oid("1.1.1"), _ermSnmpAgent, *edgeRmEnv ));
	ZQ::Snmp::ManagedPtr ermPortTbl(new ErmPortTblType(ermEnvLog, ZQ::Snmp::Oid("2.1.1"), _ermSnmpAgent, *edgeRmEnv ));
	ZQ::Snmp::ManagedPtr ermChanTbl(new ErmChanTblType(ermEnvLog, ZQ::Snmp::Oid("3.1.1"), _ermSnmpAgent, *edgeRmEnv ));

	int nRev1 = _ermSnmpAgent->addObject(ZQ::Snmp::Oid("1.1.1"), ZQ::Snmp::ManagedPtr(ermDevTbl));
	int nRev2 = _ermSnmpAgent->addObject(ZQ::Snmp::Oid("2.1.1"), ZQ::Snmp::ManagedPtr(ermPortTbl));
	int nRev3 = _ermSnmpAgent->addObject(ZQ::Snmp::Oid("3.1.1"), ZQ::Snmp::ManagedPtr(ermChanTbl));

	glog(ZQ::common::Log::L_INFO, CLOGFMT(EnvSnmpRegistor,"registerSnmp() port[2300-%d-3] succeed"), instanceId);
	return true;
}

bool EnvSnmpRegistor::unRegSnmp(void)
{
	if (_ermSnmpAgent)
	{
		delete _ermSnmpAgent;
		_ermSnmpAgent = NULL;
	}
	return true;
}

}} // namespace
