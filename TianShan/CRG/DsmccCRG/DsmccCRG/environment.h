#ifndef __zq_dsmcc_gateway_environment_header_file_h__
#define __zq_dsmcc_gateway_environment_header_file_h__

#include <ZQ_common_conf.h>
#include <string>
#include <Log.h>
#include <Ice/Ice.h>
#include <TianShanDefines.h>

#include "snmp/SubAgent.hpp"
#include "gatewaycenter.h"


namespace ZQ { namespace CLIENTREQUEST{

enum StatisticsItemTag
{
	STAT_SETUP,
	STAT_GETPARAMETER,
	STAT_PLAY,
	STAT_TEARDOWN,
	STATISTICS_COUNT
};

typedef std::map <uint32, uint32> ErrorToCountMap; // map from status-code to counter

struct GatewayStatisics
{
	GatewayStatisics()
	{
		reset();
	}

	~GatewayStatisics(){}

	GatewayStatisics & operator = (GatewayStatisics & other)
	{
		totalCount = other.totalCount;
		timeCostTotal = other.timeCostTotal;
		timeCostMax   = other.timeCostMax;
		timeCostMin   = other.timeCostMin;
		timeCostAvg   = other.timeCostAvg;
		if (!errorToCountMap.empty())
			errorToCountMap.clear();

		if (!other.errorToCountMap.empty())
			errorToCountMap.insert(other.errorToCountMap.begin(), other.errorToCountMap.end());

		return *this;
	}

	uint32		totalCount;	   // export thru SNMP
	uint32		timeCostTotal;
	uint32		timeCostMax;   // export thru SNMP
	uint32		timeCostMin;
	uint32		timeCostAvg;   // export thru SNMP

	ErrorToCountMap errorToCountMap;

	void reset()
	{
		totalCount = timeCostTotal = timeCostMax = timeCostMin = timeCostAvg =0;
		errorToCountMap.clear();
	}
};

class GatewayStatisticsCollector 
{
public:
	GatewayStatisticsCollector();
	virtual ~GatewayStatisticsCollector();

	//stats[] should >= STATISTICS_COUNT*sizeof(GatewayStatisics)
	void	getStatistics( GatewayStatisics stats[]  );

	void	collect( GATEWAYCOMMAND cmd, uint32 resultCode, uint32	timecost);

	void	reset();

	int64	measuredSince() const;
	
private:
	GatewayStatisics	mStats[STATISTICS_COUNT];
	ZQ::common::Mutex	mLocker;
	int64				mMeasuredSince;
};
class Environment
{
public:
	Environment( ZQ::common::Log& logger , ZQADAPTER_DECLTYPE& objAdapter )
		:mLogger(logger),
		mAdapter(objAdapter),
		mdsmccCRGSnmpAgnet(NULL)
	{
		mIc = objAdapter->getCommunicator();
		mdsmccCRGSnmpAgnet = new ZQ::Snmp::Subagent(2700, 5);
		if(NULL != mdsmccCRGSnmpAgnet)
			mdsmccCRGSnmpAgnet->start();

		mLogger(ZQ::common::Log::L_INFO,CLOGFMT(DsmccCRG,"Environment() init snmp %s"), (NULL != mdsmccCRGSnmpAgnet) ? "Succeed" : "failed");
	}

	~Environment()
	{
		if (NULL != mdsmccCRGSnmpAgnet)
		{
			ZQ::Snmp::Subagent *tempAgent = mdsmccCRGSnmpAgnet;
			mdsmccCRGSnmpAgnet = NULL;
			delete tempAgent;
            mLogger(ZQ::common::Log::L_INFO, CLOGFMT(DsmccCRG, "Environment() uninit snmp"));
		}
	}

	inline ZQ::common::Log& getLogger()
	{
		return mLogger;
	}
	inline Ice::CommunicatorPtr getIc( )
	{
		return mIc;
	}
	inline ZQADAPTER_DECLTYPE&	getAdapter( )
	{
		return mAdapter;
	}

	inline GatewayStatisticsCollector& getCollector()
	{
		return mStatsCollector;
	}

	ZQ::Snmp::Subagent* getDsmccSnmpAgent(void)
	{
		if (NULL == mdsmccCRGSnmpAgnet)
		{
			mdsmccCRGSnmpAgnet = new ZQ::Snmp::Subagent(2700, 5);
			if(NULL != mdsmccCRGSnmpAgnet)
				mdsmccCRGSnmpAgnet->start();

			mLogger(ZQ::common::Log::L_ERROR, CLOGFMT(DsmccCRG,"getDsmccSnmpAgent() reinit snmp %s"), (NULL != mdsmccCRGSnmpAgnet) ? "Succeed" : "failed");
		}

        return mdsmccCRGSnmpAgnet;
	}

	int  registerSnmp(GatewayCenter* gwCenter);

private:
	int  registerSnmpTable(void);

private:
	ZQ::common::Log&		mLogger;
	Ice::CommunicatorPtr	mIc;
	ZQADAPTER_DECLTYPE		mAdapter;
	ZQ::Snmp::Subagent*     mdsmccCRGSnmpAgnet;
	GatewayStatisticsCollector	mStatsCollector;
};

#define MLOG  (mEnv.getLogger())

}}//namespace ZQ::DSMCC

#endif//__zq_dsmcc_gateway_environment_header_file_h__
