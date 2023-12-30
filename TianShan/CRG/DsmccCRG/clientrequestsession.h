#ifndef __zq_dsmcc_gateway_client_session_implement_header_file_h__
#define __zq_dsmcc_gateway_client_session_implement_header_file_h__

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <TsSRM.h>
#include <TsStreamer.h>
#include <TsApplication.h>
#include "clientrequest.h"

namespace ZQ{	namespace CLIENTREQUEST	{

class Environment;
class GatewayCenter;
class ClientRequestSession : public TianShanIce::ClientRequest::Session, public IceUtil::AbstractMutexI<IceUtil::RecMutex> 
{
public:
	ClientRequestSession( Environment& env ,GatewayCenter& center);
	ClientRequestSession( Environment& env ,GatewayCenter& center, const std::string& sessId, const std::string& clientId );
	virtual ~ClientRequestSession(void);
	
	virtual void destroy(const ::Ice::Current& = ::Ice::Current());

	virtual void attachWeiwooSession(const ::TianShanIce::SRM::SessionPrx&, const ::Ice::Current& = ::Ice::Current()) ;

	virtual void attachStreamSession(const ::TianShanIce::Streamer::StreamPrx&, const std::string&,const ::Ice::Current& = ::Ice::Current()) ;

	virtual void attachPurchaseSession(const ::TianShanIce::Application::PurchasePrx&, const ::Ice::Current& = ::Ice::Current()) ;

	virtual void setProperty(const ::std::string&, const ::std::string&, const ::Ice::Current& = ::Ice::Current());

	virtual void setProperties(const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current());

	virtual void removeProperty(const ::std::string&, const ::Ice::Current& = ::Ice::Current());

	virtual void updateTimer(Ice::Long, const ::Ice::Current& = ::Ice::Current() );

	virtual ::TianShanIce::SRM::SessionPrx getWeiwooSession(const ::Ice::Current& = ::Ice::Current()) const;

	virtual ::TianShanIce::Streamer::StreamPrx getStreamSession(const ::Ice::Current& = ::Ice::Current()) const;

	virtual ::TianShanIce::Application::PurchasePrx getPurchaseSession(const ::Ice::Current& = ::Ice::Current()) const;

	virtual ::std::string getProperty(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;

	virtual ::TianShanIce::Properties getProperties(const ::Ice::Current& = ::Ice::Current()) const;

	virtual ::TianShanIce::State getState(const ::Ice::Current& = ::Ice::Current()) const ;

	virtual std::string getSessId(const ::Ice::Current& = ::Ice::Current()) const ;

	virtual std::string getClientId(const ::Ice::Current& = ::Ice::Current()) const ;

	virtual void onRestore(const ::Ice::Current& = ::Ice::Current());

private:
	Environment&		mEnv;
	GatewayCenter&		mGatewayCenter;
};

}}//namespace ZQ::CLIENTREQUEST

#endif//__zq_dsmcc_gateway_client_session_implement_header_file_h__
