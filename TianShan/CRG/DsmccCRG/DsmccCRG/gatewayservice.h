#ifndef __zq_dsmcc_gateway_service_implement_header_file_h__
#define __zq_dsmcc_gateway_service_implement_header_file_h__

#include <BaseZQServiceApplication.h>
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
};

}}//namespace ZQ::CLIENTREQUEST

#endif//__zq_dsmcc_gateway_service_implement_header_file_h__

