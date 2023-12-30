
#include <ZQ_common_conf.h>
#include "clientrequestsession.h"
#include <TianShanIceHelper.h>
#include "gatewaycenter.h"
#include "gatewayconfig.h"
#include "environment.h"
#include <TimeUtil.h>

namespace ZQ{	namespace CLIENTREQUEST	{

extern	Config::GateWayConfig gwConfig;

static void fillListenerInfo( TianShanIce::Properties& sessProps )
{
	static bool bInited = false;
	static std::string lscpTcpListener;
	static std::string lscpUdpListener;
	if(!bInited)
	{
		static ZQ::common::Mutex listenerFillMutex;
		ZQ::common::MutexGuard gd(listenerFillMutex);
		if(!bInited)
		{
			bInited = true;
			const std::vector<Config::SocketServer::ServerListenerHolder>& listeners = gwConfig.sockserver.listeners;
			std::vector<Config::SocketServer::ServerListenerHolder>::const_iterator it = listeners.begin();
			std::ostringstream ossTcp;
			std::ostringstream ossUdp;
			for( ; it != listeners.end() ; it ++ )
			{
				if( stricmp(it->protocol.c_str(),"lscp") != 0 )
					continue;				
				if( stricmp(it->type.c_str(),"tcp") == 0 )
				{
					if( it->exportAddress.empty() )
					{
						ossTcp<<it->ip<<":"<<it->port<<";";
					}
					else
					{
						ossTcp << it->exportAddress<<";";
					}
				}
				else if( stricmp(it->type.c_str(),"udp") == 0 )
				{
					if( it->exportAddress.empty())
					{
						ossUdp<<it->ip<<":"<<it->port<<";";
					}
					else
					{
						ossUdp << it->exportAddress<<";";
					}
				}
			}
			lscpTcpListener = ossTcp.str();
			lscpUdpListener = ossUdp.str();
		}		
	}
	ZQTianShan::Util::updatePropertyData(sessProps,SESS_PROP_LSC_UDP,lscpUdpListener);
	ZQTianShan::Util::updatePropertyData(sessProps,SESS_PROP_LSC_TCP,lscpTcpListener);
}

ClientRequestSession::ClientRequestSession( Environment& env ,GatewayCenter& center)
	:mEnv(env),
	mGatewayCenter(center)
{
}
ClientRequestSession::ClientRequestSession( Environment& env , GatewayCenter& center ,const std::string& sessId, const std::string& clientId )
	:mEnv(env),
	mGatewayCenter(center)
{
	this->sessionId	= sessId;
	this->clientId	= clientId;

	sessState = TianShanIce::stInService;
	uint32 streamHandle = mGatewayCenter.getStrmHandleMap().createStreamHandle();
	ZQTianShan::Util::updatePropertyData(sessProps,SESS_PROP_STREAMHANDLE,streamHandle);
	fillListenerInfo(sessProps);
	mGatewayCenter.getStrmHandleMap().updateStreamHandleInfo(sessId, streamHandle);
	timeoutTarget = 0;
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(ClientRequestSession,"create() session[%s] with clientId[%s] created: strmHandle[%u] "),
		sessId.c_str(), clientId.c_str() , streamHandle );
}

ClientRequestSession::~ClientRequestSession()
{
}

void ClientRequestSession::destroy(const ::Ice::Current& )
{	
	{
		Lock sync(*this);
		sessState = TianShanIce::stOutOfService;
		mGatewayCenter.getDatabase().removeSession(sessionId);
		mGatewayCenter.getStrmHandleMap().removeStreamHandleInfo(sessionId);
		mGatewayCenter.getStrmHandleMap().removeStreamSessionInfo(sessionId);
	}
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(ClientRequestSession,"destroy() session[%s] destroyed"),sessionId.c_str() );
}

void ClientRequestSession::onRestore(const ::Ice::Current& ic)
{
	Lock sync(*this);
	
	Ice::Long streamHandle = 0;
	ZQTianShan::Util::getPropertyDataWithDefault( sessProps,SESS_PROP_STREAMHANDLE,0,streamHandle);

	std::string streamSessionId;
	ZQTianShan::Util::getPropertyDataWithDefault( sessProps,SESS_PROP_STREAMSESSIONID,"",streamSessionId);

	mGatewayCenter.getStrmHandleMap().updateStreamHandleInfo( sessionId,(uint32)streamHandle );
	mGatewayCenter.getStrmHandleMap().updateStreamSessionInfo( sessionId, streamSessionId );


	if( timeoutTarget > 0 )
	{
		Ice::Long interval = timeoutTarget - ZQ::common::now();
		if( interval < 1000 )
		{
			interval = 1000;
			interval += rand()%10000;
		}
		updateTimer(interval,ic);
	}
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(ClientRequestSession,"onRestore() session[%s] restored"), sessionId.c_str() );
}

void ClientRequestSession::attachWeiwooSession(const ::TianShanIce::SRM::SessionPrx& sess, const ::Ice::Current& ) 
{
	Lock sync(*this);
	weiwooSession = sess;
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(ClientRequestSession,"attachWeiwooSession() session[%s] attach weiwoo srm session[%s]"),
		sessionId.c_str(), mEnv.getIc()->proxyToString(sess).c_str() );
}

void ClientRequestSession::attachStreamSession(const ::TianShanIce::Streamer::StreamPrx& sess, const std::string& streamSessId, const ::Ice::Current& ) 
{
	Lock sync(*this);
	streamSession = sess;
	ZQTianShan::Util::updatePropertyData( sessProps, SESS_PROP_STREAMSESSIONID, streamSessId );
	mGatewayCenter.getStrmHandleMap().updateStreamSessionInfo( sessionId, streamSessId );
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(ClientRequestSession,"attachStreamSession() session[%s] attach stream[%s | %s]"), 
		sessionId.c_str() , streamSessId.c_str(), mEnv.getIc()->proxyToString(sess).c_str() );
}

void ClientRequestSession::attachPurchaseSession(const ::TianShanIce::Application::PurchasePrx& sess, const ::Ice::Current& ) 
{
	Lock sync(*this);
	appSession = sess;
}

void ClientRequestSession::setProperty(const ::std::string& k, const ::std::string& v, const ::Ice::Current& )
{
	Lock sync(*this);
	sessProps[k] = v;
}

void ClientRequestSession::setProperties(const ::TianShanIce::Properties& props, const ::Ice::Current& )
{
	Lock sync(*this);
	ZQTianShan::Util::mergeProperty(sessProps,props);
}

void ClientRequestSession::removeProperty(const ::std::string& k, const ::Ice::Current& )
{
	Lock sync(*this);
	sessProps.erase(k);
}

::TianShanIce::SRM::SessionPrx ClientRequestSession::getWeiwooSession(const ::Ice::Current& ) const
{
	Lock sync(*this);
	return weiwooSession;
}

::TianShanIce::Streamer::StreamPrx ClientRequestSession::getStreamSession(const ::Ice::Current& ) const
{
	Lock sync(*this);
	return streamSession;
}

::TianShanIce::Application::PurchasePrx ClientRequestSession::getPurchaseSession(const ::Ice::Current& ) const
{
	Lock sync(*this);
	return appSession;
}

::std::string ClientRequestSession::getProperty(const ::std::string& k, const ::Ice::Current& ) const
{
	Lock sync(*this);
	TianShanIce::Properties::const_iterator it = sessProps.find(k);
	if( it == sessProps.end() )
		return std::string("");
	return it->second;
}

::TianShanIce::Properties ClientRequestSession::getProperties(const ::Ice::Current& ) const
{
	Lock sync(*this);
	return sessProps;
}

::TianShanIce::State ClientRequestSession::getState(const ::Ice::Current& ) const
{
	Lock sync(*this);
	return sessState;
}

std::string ClientRequestSession::getSessId(const ::Ice::Current&) const 
{
	return sessionId;
}

std::string ClientRequestSession::getClientId(const ::Ice::Current&) const 
{
	return clientId;
}

void ClientRequestSession::updateTimer(Ice::Long interval , const ::Ice::Current& )
{
	if( interval < 0 )
		interval = 0;
	{
		Lock sync(*this);
		timeoutTarget = ZQ::common::now() + interval;
	}
	char buffer[256];buffer[sizeof(buffer)-1] = 0;
	ZQ::common::TimeUtil::TimeToUTC(timeoutTarget,buffer,sizeof(buffer)-1,true);;
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(ClientRequestSession,"updateTimer() session[%s] update time interval[%lld], will expire at[%s]"),
		sessionId.c_str() , interval,buffer );
	mGatewayCenter.updateTimer( sessionId, interval );
	
}

}}//namespace ZQ::CLIENTREQUEST
