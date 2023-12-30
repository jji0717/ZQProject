#ifndef __zq_dsmcc_gateway_service_implement_header_file_h__
#define __zq_dsmcc_gateway_service_implement_header_file_h__


#ifdef ZQ_OS_MSWIN
#include "BaseZQServiceApplication.h"
#else
#include "ZQDaemon.h"
#endif

#include <FileLog.h>
#include "gatewaycenter.h"

namespace ZQ{ namespace CLIENTREQUEST {

class GatewayService : public ZQ::common::BaseZQServiceApplication 
{
public:
	GatewayService(void);
	virtual ~GatewayService(void);

	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);

    virtual bool isHealth(void);
	virtual void doEnumSnmpExports();

    void    refreshCrStatTable();

	GatewayCenter*	getGatewayCenter()
	{
		return mGwCenter;
	}

protected:

	bool		initIceRuntime( );

private:
	Environment*			mGatewayEnv;
	GatewayCenter*			mGwCenter;
	ZQ::common::FileLog		mIceTraceLogger;
	Ice::CommunicatorPtr	mIc;
	ZQADAPTER_DECLTYPE		mAdapter;

    int64       _lastRefreshTime;
};

}}//namespace ZQ::CLIENTREQUEST

#endif//__zq_dsmcc_gateway_service_implement_header_file_h__

