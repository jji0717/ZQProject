
#ifndef _ssm_ngod2_snmp_query_servant_implementation_header_file_h__
#define _ssm_ngod2_snmp_query_servant_implementation_header_file_h__

#include <map>
#include "ZQSnmpMgmt.hpp"
#include "Subagent.hpp"
#include "SOPConfig.h"
#include "ContextImpl.h"

enum SopUsageAttrType 
{
	STREAMER_SNMPTYPE_SOPNAME			=	0,		//sopName
	STREAMER_SNMPTYPE_STREAMER			=	1,		//sopStreamer
	STREAMER_SNMPTYPE_ENDPOINT			=	2,		//sopStreamService
	STREANER_SNMPTYPE_STATUS			=	3,		//sopStatus
	STREAMER_SNMPTYPE_PENALTY			=	4,		//sopPenalty
	STREAMER_SNMPTYPE_SESSIONUSED		=	5,		//sopSessionUsed
	STREAMER_SNMPTYPE_SESSIONFAILED		=	6,		//sopSessionFailed
	STREAMER_SNMPTYPE_ERRORRATE			=	7,		//sopErrorRate
	STREAMER_SNMPTYPE_BWUSED			=	8,		//sopUsedBandwidth
	STREAMER_SNMPTYPE_BWMAX				=	9,		//sopMaxBandwidth
	STREAMER_SNMPTYPE_ACTIVESESSION		=	10,		//sopActiveSession
	STREAMER_SNMPTYPE_MAXSESSION		=	11,		//sopMaxSession
	STREAMER_SNMPTYPE_LOCALSESSION		=	12,		//sopLocalSession
	
};

enum ImportChannelAttrType
{
	IMPORTCAHNNEL_SNMPTYPE_NAME			=	0,
	IMPORTCHANNEL_SNMPTYPE_TOTALBW		=	1,
	IMPORTCHANNEL_SNMPTYPE_USEDBW		=	2,
	IMPORTCHANNEL_SNMPTYPE_SESSCOUNT	=	3
};

class VariableSopUsageImpl : public ZQ::Snmp::IVariable
{
public:
	VariableSopUsageImpl( SopUsageAttrType type , const std::string& sopName , const std::string& streamerNetId , const std::string& endpoint);
	virtual ~VariableSopUsageImpl( );
	
	virtual bool get( ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType ) const;
	virtual bool set( const ZQ::Snmp::SmiValue& val );
	virtual bool validate( const ZQ::Snmp::SmiValue& val ) const;

protected:

	bool		getVariableWithType( const NGODr2c1::StreamerUsage& streamerUsage, ZQ::Snmp::SmiValue& val , ZQ::Snmp::AsnType desiredType  ) const;

private:
	std::string			mSopName;
	std::string			mStreamerNetId;
	std::string			mEndpoint;
	SopUsageAttrType	mAttrType;
};

class VariableImportChannelUsageImpl : public ZQ::Snmp::IVariable
{
public:
	VariableImportChannelUsageImpl( ImportChannelAttrType type , const std::string& importChannelName , const std::string& endpoint);
	virtual ~VariableImportChannelUsageImpl( );
	
	virtual bool get( ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType ) const;
	virtual bool set( const ZQ::Snmp::SmiValue& val );
	virtual bool validate( const ZQ::Snmp::SmiValue& val ) const;
protected:
	
	bool		getVariableWithType( const NGODr2c1::ImportChannelUsage& importChannelUsage, ZQ::Snmp::SmiValue& val , ZQ::Snmp::AsnType desiredType  ) const;

private:
	std::string					mImportChannelName;
	std::string					mEndPoint;
	ImportChannelAttrType		mAttrType;
};

class SnmpQueryServantImpl
{
public:
	SnmpQueryServantImpl(  );
	virtual ~SnmpQueryServantImpl(void);

public:

	void					setEndpoint( const std::string& endpoint );

	ZQ::Snmp::TablePtr 		initNgodUsageTable(int& count);
	ZQ::Snmp::TablePtr 		initImportChannelUsageTable(int& count);

	bool					registerSnmpTable( );

	void					unregisterSnmpTable( );

private:
	std::string					mEndPoint;
	ZQ::Snmp::Subagent*			mAgent;

};

#endif//_ssm_ngod2_snmp_query_servant_implementation_header_file_h__

