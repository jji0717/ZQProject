#include "StreamSmithModule.h"
#include "SnmpQueryServantImpl.h"
#include "NgodService.h"
#include "NgodConfig.h"
#include "snmp/ZQSnmp.hpp"

//////////////////////////////////////////////////////////////////////////
///
extern NGOD::NgodService ngodService;
#define NGODENVLOG	(ngodService.getFileLog())
VariableSopUsageImpl::VariableSopUsageImpl( SopUsageAttrType type , const std::string& sopName , const std::string& streamerNetId , const std::string& endpoint)
:mSopName(sopName),
mStreamerNetId(streamerNetId),
mAttrType(type),
mEndpoint(endpoint)
{
	if( endpoint.find(':') == std::string::npos )
	{
		mEndpoint = std::string("Ngod2View:") + endpoint;
	}
}

VariableSopUsageImpl::~VariableSopUsageImpl()
{
}

bool VariableSopUsageImpl::getVariableWithType( const NGOD::StreamerUsage& streamerUsage, ZQ::Snmp::SmiValue& val , ZQ::Snmp::AsnType desiredType  ) const
{
	switch(mAttrType)
	{
	case STREAMER_SNMPTYPE_SOPNAME:
		{
			return ZQ::Snmp::smivalFrom<std::string>( val, mSopName, desiredType );
		}
		break;
	case STREAMER_SNMPTYPE_STREAMER:
		{
			return ZQ::Snmp::smivalFrom<std::string>( val, streamerUsage.streamerNetId, desiredType );
		}
		break;
	case STREAMER_SNMPTYPE_ENDPOINT:
		{
			return ZQ::Snmp::smivalFrom<std::string>( val, streamerUsage.streamerEndpoint, desiredType );
		}
		break;
	case STREANER_SNMPTYPE_STATUS:
		{
			std::string		status;
			if( streamerUsage.maintenanceEnable > 0)
			{
				status = streamerUsage.available > 0 ? "avail" : "unavail";
			}
			else
			{
				status = "unavail";
			}
			return ZQ::Snmp::smivalFrom<std::string>( val, status, desiredType );
		}
		break;
	case STREAMER_SNMPTYPE_PENALTY:
		{
			return ZQ::Snmp::smivalFrom<Ice::Long>( val, streamerUsage.penaltyValue, desiredType );
		}
		break;
	case STREAMER_SNMPTYPE_SESSIONUSED:
		{
			return ZQ::Snmp::smivalFrom<Ice::Long>( val, streamerUsage.usedSession, desiredType );
		}
		break;
	case STREAMER_SNMPTYPE_SESSIONFAILED:
		{
			return ZQ::Snmp::smivalFrom<Ice::Long>( val, streamerUsage.failedSession, desiredType );
		}
		break;
	case STREAMER_SNMPTYPE_ERRORRATE:
		{
			int errorRate = 0;
			uint64 nTotoalSession = streamerUsage.usedSession + streamerUsage.failedSession;
			if ( nTotoalSession != 0 )
			{
				errorRate = (int)((streamerUsage.failedSession * 100 ) / nTotoalSession);	
			}

			return ZQ::Snmp::smivalFrom<int>( val, errorRate, desiredType );
		}
		break;
	case STREAMER_SNMPTYPE_BWUSED:
		{
			return ZQ::Snmp::smivalFrom<Ice::Long>( val, streamerUsage.usedBandwidth, desiredType );
		}
		break;
	case STREAMER_SNMPTYPE_BWMAX:
		{
			return ZQ::Snmp::smivalFrom<Ice::Long>( val, streamerUsage.totalBandwidth, desiredType );
		}
		break;
	case STREAMER_SNMPTYPE_ACTIVESESSION:
		{
			return ZQ::Snmp::smivalFrom<Ice::Long>( val, streamerUsage.usedStreamCount, desiredType );
		}
		break;
	case STREAMER_SNMPTYPE_MAXSESSION:
		{
			return ZQ::Snmp::smivalFrom<Ice::Long>( val, streamerUsage.maxStreamCount, desiredType );
		}
		break;
	case STREAMER_SNMPTYPE_LOCALSESSION:
		{
			return ZQ::Snmp::smivalFrom<Ice::Long>( val, streamerUsage.histCountTotalSess - streamerUsage.histCountRemoteSess, desiredType );
		}
        break;
    case STREAMER_SNMPTYPE_VOLUME:
        {
            return ZQ::Snmp::smivalFrom( val, streamerUsage.attachedVolumeName, desiredType );
        }
        break;
	default:
		{
			return false;
		}
	}
	return true;
}

bool VariableSopUsageImpl::get( ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType )
{
	//step 1 
	//connect to ngod usage servant
	NGOD::SessionViewPrx usagePrx = NULL;
	try
	{		
		usagePrx = NGOD::SessionViewPrx::checkedCast( ngodService.getCommunicator()->stringToProxy(mEndpoint) );
	}
	catch (const Ice::Exception& ex)
	{
		NGODENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VariableSopUsageImpl,"caught exception:[%s] when connecting to ngod usage[%s]"),
			ex.ice_name().c_str() , mEndpoint.c_str() );
		return false;
	}

	NGOD::NgodUsage ngodUsage;

	try
	{
		std::string		measuredSince;
		usagePrx->getNgodUsage( ngodUsage , measuredSince );
	}
	catch( const Ice::Exception& ex)
	{
		NGODENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VariableSopUsageImpl,"caught exception:[%s] when fetching usage list"), ex.ice_name().c_str() );
		return false;
	}

	//step 2
	//find the streamerUsage according to sopName and streamerNetId
	NGOD::NgodUsage::const_iterator itMap = ngodUsage.find(mSopName);
	if( itMap == ngodUsage.end() )
	{
		NGODENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VariableSopUsageImpl, "failed to find sop with sopName[%s]"), mSopName.c_str() );
		return false;		
	}
	const ::std::vector< ::NGOD::StreamerUsage>& streamerUsages = itMap->second.streamerUsageInfo;

	std::vector< ::NGOD::StreamerUsage>::const_iterator itStreamer = streamerUsages.begin();

	while( itStreamer != streamerUsages.end() )
	{
		if( itStreamer->streamerNetId == mStreamerNetId )
		{
			return getVariableWithType( *itStreamer , val, desiredType );
		}
		itStreamer++;
	}

	NGODENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VariableSopUsageImpl, "failed to find streamer with sopName[%s] streamerNetId[%s]"),
		mSopName.c_str() , mStreamerNetId.c_str() );

	return false;
}

bool VariableSopUsageImpl::set( const ZQ::Snmp::SmiValue& val )
{
	NGODENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VariableSopUsageImpl,"method set() is not supported"));
	return false;
}

bool VariableSopUsageImpl::validate( const ZQ::Snmp::SmiValue& val ) const
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
///

VariableImportChannelUsageImpl::VariableImportChannelUsageImpl(ImportChannelAttrType type ,
															   const std::string& importChannelName ,
															   const std::string& endpoint)
															   :mImportChannelName(importChannelName),
															   mEndPoint(endpoint),
															   mAttrType(type)
{
	if( endpoint.find(':') == std::string::npos )
	{
		mEndPoint = std::string("Ngod2View:") + endpoint;
	}
}

VariableImportChannelUsageImpl::~VariableImportChannelUsageImpl()
{

}
bool VariableImportChannelUsageImpl::set( const ZQ::Snmp::SmiValue& val )
{
	return false;
}

bool VariableImportChannelUsageImpl::validate( const ZQ::Snmp::SmiValue& val ) const
{
	return false;
}

bool VariableImportChannelUsageImpl::getVariableWithType( const NGOD::ImportChannelUsage& importChannelUsage,
														 ZQ::Snmp::SmiValue& val , 
														 ZQ::Snmp::AsnType desiredType  ) const
{
	switch(mAttrType)
	{
	case IMPORTCAHNNEL_SNMPTYPE_NAME:
		{
			return ZQ::Snmp::smivalFrom<std::string>( val, importChannelUsage.channelName , desiredType );
		}
		break;
	case IMPORTCHANNEL_SNMPTYPE_TOTALBW:
		{
			return ZQ::Snmp::smivalFrom<Ice::Long>( val, importChannelUsage.totalImportBandwidth, desiredType );
		}
		break;
	case IMPORTCHANNEL_SNMPTYPE_USEDBW:
		{
			return ZQ::Snmp::smivalFrom<Ice::Long>( val, importChannelUsage.usedImportBandwidth, desiredType );
		}
		break;
	case IMPORTCHANNEL_SNMPTYPE_SESSCOUNT:
		{
			return ZQ::Snmp::smivalFrom<Ice::Int>( val, importChannelUsage.runningSessCount, desiredType );
		}
        break;
    case IMPORTCHANNEL_SNMPTYPE_STATUS:
        {
            std::string status = importChannelUsage.totalImportBandwidth > importChannelUsage.usedImportBandwidth ? "avail" : "unavail";
            return ZQ::Snmp::smivalFrom<std::string>( val, status, desiredType );
        }
        break;
	default:
		return false;
	}
	return false;
}

bool VariableImportChannelUsageImpl::get( ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType )
{
	//step 1 
	//connect to ngod usage servant
	NGOD::SessionViewPrx usagePrx = NULL;
	try
	{		
		usagePrx = NGOD::SessionViewPrx::checkedCast( ngodService.getCommunicator()->stringToProxy(mEndPoint) );
	}
	catch (const Ice::Exception& ex)
	{
		NGODENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VariableSopUsageImpl,"caught exception:[%s] when connecting to ngod usage[%s]"),
			ex.ice_name().c_str() , mEndPoint.c_str() );
		return false;
	}

	NGOD::ImportChannelUsageS icUsage;

	try
	{		
		usagePrx->getImportChannelUsage( icUsage );
	}
	catch( const Ice::Exception& ex)
	{
		NGODENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VariableSopUsageImpl,"caught exception:[%s] when fetching usage list"), ex.ice_name().c_str() );
		return false;
	}


	//step 2
	//find the streamerUsage according to sopName and streamerNetId
	::NGOD::ImportChannelUsageS::const_iterator itChannel = icUsage.begin();

	while( itChannel != icUsage.end() )
	{
		if( itChannel->channelName == mImportChannelName )
		{
			return getVariableWithType( *itChannel , val, desiredType );
		}
		itChannel++;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///SnmpQueryServantImpl
using namespace ZQ::Snmp;
class IntVar: public ZQ::Snmp::IVariable
{
public:
	IntVar(int i):val_(i){}
	~IntVar(){}
	virtual bool get(SmiValue& val, AsnType desiredType)
	{
		return smivalFrom(val, val_, desiredType);
	}
	virtual bool set(const SmiValue& val)
	{
		return smivalTo(val, val_);
	}
	virtual bool validate(const SmiValue& val) const
	{
		return true;
	}
public:
	int val_;
};

SnmpQueryServantImpl::SnmpQueryServantImpl(  )
:mAgent(NULL)
{	
}
SnmpQueryServantImpl::~SnmpQueryServantImpl( )
{
}

ZQ::Snmp::TablePtr 	SnmpQueryServantImpl::initImportChannelUsageTable(int& count)
{
	ZQ::Snmp::TablePtr tbIcUsage(new ZQ::Snmp::Table());
	tbIcUsage->addColumn( 1,	ZQ::Snmp::AsnType_Integer,		ZQ::Snmp::aReadOnly);//index
	tbIcUsage->addColumn( 2,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//importChannelName
	tbIcUsage->addColumn( 3,	ZQ::Snmp::AsnType_Counter64,	ZQ::Snmp::aReadOnly);//usedBanwidth
	tbIcUsage->addColumn( 4,	ZQ::Snmp::AsnType_Counter64,	ZQ::Snmp::aReadOnly);//totalBadwidth
	tbIcUsage->addColumn( 5,	ZQ::Snmp::AsnType_Integer,		ZQ::Snmp::aReadOnly);//running session count
    tbIcUsage->addColumn( 6,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//status

	const std::map<std::string,NGOD::PassThruStreaming::ImportChannelHolder>& ics = ngodConfig.passThruStreaming.importChannelDatas;
	std::map<std::string,NGOD::PassThruStreaming::ImportChannelHolder>::const_iterator it = ics.begin();

    count = 0;
	int rowIndex = 1;
	for( ; it != ics.end() ;it ++ )
	{
		if( it->second._bConfiged )
		{//only care about configured ImportChannel data
			ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex( rowIndex );
			const std::string& channelName = it->second.name;
			//for every streamer
			tbIcUsage->addRowData( 1,		indexOid , ZQ::Snmp::VariablePtr(new IntVar( rowIndex )));
			tbIcUsage->addRowData( 2,		indexOid , ZQ::Snmp::VariablePtr(new VariableImportChannelUsageImpl( IMPORTCAHNNEL_SNMPTYPE_NAME , channelName, mEndPoint)));
			tbIcUsage->addRowData( 3,		indexOid , ZQ::Snmp::VariablePtr(new VariableImportChannelUsageImpl( IMPORTCHANNEL_SNMPTYPE_USEDBW , channelName, mEndPoint)));
			tbIcUsage->addRowData( 4,		indexOid , ZQ::Snmp::VariablePtr(new VariableImportChannelUsageImpl( IMPORTCHANNEL_SNMPTYPE_TOTALBW , channelName, mEndPoint)));
			tbIcUsage->addRowData( 5,		indexOid , ZQ::Snmp::VariablePtr(new VariableImportChannelUsageImpl( IMPORTCHANNEL_SNMPTYPE_SESSCOUNT , channelName, mEndPoint)));
            tbIcUsage->addRowData( 6,		indexOid , ZQ::Snmp::VariablePtr(new VariableImportChannelUsageImpl( IMPORTCHANNEL_SNMPTYPE_STATUS , channelName, mEndPoint)));

            ++count;
			++rowIndex;
		}
	}
	return tbIcUsage;
}
ZQ::Snmp::TablePtr 	SnmpQueryServantImpl::initNgodUsageTable(int& count)
{
	//initialize NGOD Usage Table
	ZQ::Snmp::TablePtr tbNgodUsage(new ZQ::Snmp::Table());

	tbNgodUsage->addColumn( 1,	ZQ::Snmp::AsnType_Integer,		ZQ::Snmp::aReadOnly);//index
	tbNgodUsage->addColumn( 2,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//sopName
	tbNgodUsage->addColumn( 3,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//sopStreamer
	tbNgodUsage->addColumn( 4,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//sopStreamService
	tbNgodUsage->addColumn( 5,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//sopStatus
	tbNgodUsage->addColumn( 6,	ZQ::Snmp::AsnType_Counter64,	ZQ::Snmp::aReadOnly);//sopPenalty
	tbNgodUsage->addColumn( 7,	ZQ::Snmp::AsnType_Counter64,	ZQ::Snmp::aReadOnly);//sopSessionUsed
	tbNgodUsage->addColumn( 8,	ZQ::Snmp::AsnType_Counter64,	ZQ::Snmp::aReadOnly);//sopSessionFailed
	tbNgodUsage->addColumn( 9,	ZQ::Snmp::AsnType_Integer,		ZQ::Snmp::aReadOnly);//sopErrorRate	
	tbNgodUsage->addColumn( 10, ZQ::Snmp::AsnType_Counter64,	ZQ::Snmp::aReadOnly);//sopUsedBandwidth	
	tbNgodUsage->addColumn( 11, ZQ::Snmp::AsnType_Counter64,	ZQ::Snmp::aReadOnly);//sopMaxBandwidth	
	tbNgodUsage->addColumn( 12, ZQ::Snmp::AsnType_Counter64,	ZQ::Snmp::aReadOnly);//sopActiveSession	
	tbNgodUsage->addColumn( 13, ZQ::Snmp::AsnType_Counter64,	ZQ::Snmp::aReadOnly);//sopMaxSession	
	tbNgodUsage->addColumn( 14, ZQ::Snmp::AsnType_Counter64,	ZQ::Snmp::aReadOnly);//sopLocalSession
    tbNgodUsage->addColumn( 15,	ZQ::Snmp::AsnType_Octets,		ZQ::Snmp::aReadOnly);//sopVolume

	const std::map< std::string , NGOD::SOPRestriction::SopHolder >& sops = sopConfig.sopRestrict.sopDatas;
	std::map< std::string , NGOD::SOPRestriction::SopHolder >::const_iterator itMap = sops.begin();

    count = 0;
	int rowIndex = 1;
	for( ; itMap != sops.end() ; itMap++ )
	{
		std::string sopName = itMap->second.name;
		const std::vector<NGOD::Sop::StreamerHolder>& streamers = itMap->second.streamerDatas;
		std::vector<NGOD::Sop::StreamerHolder>::const_iterator itStreamer = streamers.begin();
		
		for( ; itStreamer != streamers.end() ; itStreamer++ )
		{
			std::string streamerNetId = itStreamer->netId;			
			ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex( rowIndex );
			//for every streamer
			tbNgodUsage->addRowData( 1,		indexOid , ZQ::Snmp::VariablePtr(new IntVar( rowIndex )));
 			tbNgodUsage->addRowData( 2,		indexOid , ZQ::Snmp::VariablePtr(new VariableSopUsageImpl( STREAMER_SNMPTYPE_SOPNAME , sopName , streamerNetId , mEndPoint)));
 			tbNgodUsage->addRowData( 3,		indexOid , ZQ::Snmp::VariablePtr(new VariableSopUsageImpl( STREAMER_SNMPTYPE_STREAMER , sopName , streamerNetId , mEndPoint)));
 			tbNgodUsage->addRowData( 4,		indexOid , ZQ::Snmp::VariablePtr(new VariableSopUsageImpl( STREAMER_SNMPTYPE_ENDPOINT , sopName , streamerNetId , mEndPoint)));
 			tbNgodUsage->addRowData( 5,		indexOid , ZQ::Snmp::VariablePtr(new VariableSopUsageImpl( STREANER_SNMPTYPE_STATUS , sopName , streamerNetId , mEndPoint)));
 			tbNgodUsage->addRowData( 6,		indexOid , ZQ::Snmp::VariablePtr(new VariableSopUsageImpl( STREAMER_SNMPTYPE_PENALTY , sopName , streamerNetId , mEndPoint)));
 			tbNgodUsage->addRowData( 7,		indexOid , ZQ::Snmp::VariablePtr(new VariableSopUsageImpl( STREAMER_SNMPTYPE_SESSIONUSED , sopName , streamerNetId , mEndPoint)));
 			tbNgodUsage->addRowData( 8,		indexOid , ZQ::Snmp::VariablePtr(new VariableSopUsageImpl( STREAMER_SNMPTYPE_SESSIONFAILED , sopName , streamerNetId , mEndPoint)));
 			tbNgodUsage->addRowData( 9,		indexOid , ZQ::Snmp::VariablePtr(new VariableSopUsageImpl( STREAMER_SNMPTYPE_ERRORRATE , sopName , streamerNetId , mEndPoint)));
 			tbNgodUsage->addRowData( 10,	indexOid , ZQ::Snmp::VariablePtr(new VariableSopUsageImpl( STREAMER_SNMPTYPE_BWUSED , sopName , streamerNetId , mEndPoint)));
 			tbNgodUsage->addRowData( 11,	indexOid , ZQ::Snmp::VariablePtr(new VariableSopUsageImpl( STREAMER_SNMPTYPE_BWMAX , sopName , streamerNetId , mEndPoint)));
 			tbNgodUsage->addRowData( 12,	indexOid , ZQ::Snmp::VariablePtr(new VariableSopUsageImpl( STREAMER_SNMPTYPE_ACTIVESESSION , sopName , streamerNetId , mEndPoint)));
			tbNgodUsage->addRowData( 13,	indexOid , ZQ::Snmp::VariablePtr(new VariableSopUsageImpl( STREAMER_SNMPTYPE_MAXSESSION , sopName , streamerNetId , mEndPoint)));
			tbNgodUsage->addRowData( 14,	indexOid , ZQ::Snmp::VariablePtr(new VariableSopUsageImpl( STREAMER_SNMPTYPE_LOCALSESSION , sopName , streamerNetId , mEndPoint)));
            tbNgodUsage->addRowData( 15,	indexOid , ZQ::Snmp::VariablePtr(new VariableSopUsageImpl( STREAMER_SNMPTYPE_VOLUME , sopName , streamerNetId , mEndPoint)));
			count++;
			rowIndex++;
		}		
	}
	return tbNgodUsage;
}

class ChangeSettingsIntVar : public IntVar
{
public:
	ChangeSettingsIntVar( int i, std::string endpoint):IntVar(i), mEndPoint(endpoint){}
	virtual ~ChangeSettingsIntVar(){}
	virtual bool set(const SmiValue& val)
	{
		//TODO: change the setting first
		//return smivalTo(val, val_);
		if( mEndPoint.find(':') == std::string::npos )
		{
			mEndPoint = "Ngod2View:" + mEndPoint;
		}
		NGOD::SessionViewPrx svPrx = NULL;
		try
		{		
			svPrx = NGOD::SessionViewPrx::checkedCast( ngodService.getCommunicator()->stringToProxy(mEndPoint) );
			svPrx->resetCounters();
			return true;
		}
		catch (const Ice::Exception& ex)
		{
			NGODENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VariableSopUsageImpl,"caught exception:[%s] when connecting to ngod usage[%s]"),
				ex.ice_name().c_str() , mEndPoint.c_str() );
		}
		catch(...)
		{
		}
		return false;
	}
private:
	std::string					mEndPoint;
};


bool SnmpQueryServantImpl::registerSnmpTable(IStreamSmithSite* pSite)
{	
	using namespace ZQ::common;
	NGODENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SnmpQueryServantImpl,"registerSnmpTable() entering"));

	using ZQ::common::Variant;
	long instanceId = 0;
	try
	{
		Variant  snmpProcess    = pSite->getInfo(IStreamSmithSite::INFORMATION_SNMP_PROCESS);
		Variant  snmpInstanceId = snmpProcess["sys.snmp.instanceId"];
		instanceId  = (long)snmpInstanceId;
	}
	catch (...)
	{
		NGODENVLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SnmpQueryServantImpl,"registerSnmpTable() get unknown exception"));
	}

	NGODENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SnmpQueryServantImpl,"registerSnmpTable() instanceId[%d]"), instanceId);
 	mAgent = new ZQ::Snmp::Subagent( 1000 , 3, instanceId);
	assert( mAgent != NULL );
	mAgent->setLogger(&ngodService.getFileLog());

    int sopCount = 0;
	mAgent->addObject( ZQ::Snmp::Oid("1.1.1"), initNgodUsageTable(sopCount) );
    mAgent->addObject( ZQ::Snmp::Oid("1.2"), ZQ::Snmp::ManagedPtr(new SimpleObject(ZQ::Snmp::VariablePtr(new IntVar(sopCount)), ZQ::Snmp::AsnType_Integer, ZQ::Snmp::aReadOnly)));
	//used for changing 
	mAgent->addObject( ZQ::Snmp::Oid("1.3"), ZQ::Snmp::ManagedPtr(new SimpleObject(ZQ::Snmp::VariablePtr(new ChangeSettingsIntVar(0, mEndPoint)), ZQ::Snmp::AsnType_Integer, ZQ::Snmp::aReadWrite)));

    int icCount = 0;
	if(  ngodConfig.passThruStreaming.muteStat <= 0 && ngodConfig.passThruStreaming.importChannelDatas.size() > 0 )
	{
		mAgent->addObject( ZQ::Snmp::Oid("2.1.1"), initImportChannelUsageTable(icCount) );
	}
	//count
    mAgent->addObject( ZQ::Snmp::Oid("2.2"), ZQ::Snmp::ManagedPtr(new SimpleObject(ZQ::Snmp::VariablePtr(new IntVar(icCount)), ZQ::Snmp::AsnType_Integer, ZQ::Snmp::aReadOnly)));

	typedef  rwSnmpLogType  ssm_ngod2Log;
	typedef  rwSnmpLogType  ssm_ngod2_icetraceLog;
	typedef  rwSnmpLogType  ssm_ngod2_eventsLog;
	mAgent->addObject(Oid("3"), ManagedPtr(new SimpleObject(VariablePtr(new ssm_ngod2Log(ngodService.getFileLog(), &Log::getVerbosity, ngodService.getFileLog(), &Log::setVerbosity) ),                  AsnType_Integer, aReadWrite)));
	mAgent->addObject(Oid("4"), ManagedPtr(new SimpleObject(VariablePtr(new ssm_ngod2_icetraceLog(ngodService.getIceFileLog(), &Log::getVerbosity, ngodService.getIceFileLog(), &Log::setVerbosity) ),   AsnType_Integer, aReadWrite)));
	mAgent->addObject(Oid("5"), ManagedPtr(new SimpleObject(VariablePtr(new ssm_ngod2_eventsLog(ngodService.getEventFileLog(), &Log::getVerbosity, ngodService.getEventFileLog(), &Log::setVerbosity) ), AsnType_Integer, aReadWrite)));

	NGODENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SnmpQueryServantImpl,"registerSnmpTable() instanceId[%d]  quit"), instanceId);
	return mAgent->start();
}

void SnmpQueryServantImpl::unregisterSnmpTable( )
{
	if( mAgent)
	{
		delete mAgent;
		mAgent = NULL;
	}

	NGODENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(SnmpQueryServantImpl,"unregisterSnmpTable() quit"));
}

void SnmpQueryServantImpl::setEndpoint( const std::string& endPoint )
{
	mEndPoint = endPoint;
	if( endPoint.find(':') == std::string::npos )
	{
		mEndPoint = "Ngod2View:" + endPoint;
	}
}
