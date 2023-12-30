#include "TransferSessionImpl.h"
#include "TianShanIceHelper.h"
#include "CDNDefines.h"

namespace TianShanIce{
namespace SCS{

TransferSessionImpl::TransferSessionImpl(::ZQTianShan::CDN::C2Env &env)
    :_env(env)
{
    state = TianShanIce::stNotProvisioned;

    stampAllocated = 0;
    stampNarrowed = 0;
    stampComitted = 0;
    expiration = 0;
    costWhenAllocated = 0;
    initProperties(props);
}
void TransferSessionImpl::initProperties(TransferSessionProp& p) {
    p.transferId.clear();         ///< transfer id for this session(index)
    p.clientTransfer.clear();     ///< which client interface this session from
    p.transferPort.clear();       ///< which transfer port to stream this session, the transferport must be identified via "[<CDNCS-NetId>/]<CDNSS-NetId>/<DeviceId>"
    p.transferDelay = 0;      ///< transfer delay required by session
    p.transferRate = 0;	      ///< bandwidth of this session transfer
    p.allocatedBW = 0;	      ///< bandwidth of this session transfer
    p.transferTimeout = 0;    ///< timeout that the stream service reserve the transferId for the client
    p.others.clear(); ///< the other properties
}
TransferSessionImpl::~TransferSessionImpl()
{

}

//impl of TransferSession
::TianShanIce::Streamer::StreamPrx TransferSessionImpl::getStream(const ::Ice::Current& c)
{
    RLock rLock(*this);
    return stream;
}

TransferSessionProp TransferSessionImpl::getProps(const ::Ice::Current& c)
{
    RLock rLock(*this);
    return props;
}

void TransferSessionImpl::setStream(const ::TianShanIce::Streamer::StreamPrx& s, const ::Ice::Current&) {
    WLock wLock(*this);
    stream = s;
}

void TransferSessionImpl::setProps2(const TransferSessionProp& p, const ::Ice::Current&) {
    WLock wLock(*this);
    props = p;
}

void TransferSessionImpl::setProps3( const TianShanIce::Properties& p, const Ice::Current& )
{
	WLock wLock(*this);
	
	Ice::Long		timeout = 0;
	std::string		openForWrite;
	std::string		availRange;	
	std::string		sessId;
	std::string		c2port;

	try {
	ZQTianShan::Util::getPropertyData( p , CDN_TRANSFERID , sessId );
	ZQTianShan::Util::getPropertyData( p , CDN_TRANSFERTIMEOUT , timeout );
	ZQTianShan::Util::getPropertyData( p , CDN_AVAILRANGE , availRange );
	ZQTianShan::Util::getPropertyData( p , CDN_OPENFORWRITE , openForWrite );
	ZQTianShan::Util::getPropertyData( p , CDN_TRANSFERPORTNUM, c2port);


	ZQTianShan::Util::updateValueMapData( props.others , CDN_TRANSFERID, sessId );
	ZQTianShan::Util::updateValueMapData( props.others , CDN_TRANSFERTIMEOUT , timeout );
	ZQTianShan::Util::updateValueMapData( props.others , CDN_AVAILRANGE , availRange);
	ZQTianShan::Util::updateValueMapData( props.others , CDN_OPENFORWRITE , openForWrite );
	ZQTianShan::Util::updateValueMapData( props.others , CDN_TRANSFERPORTNUM, c2port );
	
	props.transferId = sessId;
	props.transferTimeout = timeout;

	}
	catch( const TianShanIce::InvalidParameter&)
	{
		envExtLog(ZQ::common::Log::L_ERROR,CLOGFMT(TransferSessionImpl,"setProps3 caught InvalidParameter"));
	}
}

void TransferSessionImpl::setProps(const ::TianShanIce::ValueMap &value, const ::Ice::Current& c)
{
    WLock wLock(*this);
    for(TianShanIce::ValueMap::const_iterator it = value.begin(); it != value.end(); ++it) {
        const std::string& key = it->first;
        try
        {
            if (key.compare(CDN_TRANSFERID) == 0)//set TransferId
            {
                ::ZQTianShan::Util::getValueMapData(value, key, props.transferId);
            }
            else if (key.compare(CDN_CLIENTTRANSFER) == 0)//set ClientTransfer
            {
                ::ZQTianShan::Util::getValueMapData(value, key, props.clientTransfer);
            }
            else if (key.compare(CDN_TRANSFERPORT) == 0)//set TransferPort
            {
                ::ZQTianShan::Util::getValueMapData(value, key, props.transferPort);
            }
            else if (key.compare(CDN_DELAY) == 0)//set TransferDelay
            {
                ::ZQTianShan::Util::getValueMapData(value, key, props.transferDelay);
            }
            else if (key.compare(CDN_TRANSFERRATE) == 0)//set TransferRate
            {
                ::ZQTianShan::Util::getValueMapData(value, key, props.transferRate);
            }
            else if (key.compare(CDN_ALLOCATEDCAPACITY) == 0)//set AllocateBW
            {
                ::ZQTianShan::Util::getValueMapData(value, key, props.allocatedBW);
            }
            else if (key.compare(CDN_TRANSFERTIMEOUT) == 0)//set TransferTimeout
            {
                ::ZQTianShan::Util::getValueMapData(value, key, props.transferTimeout);
            }
            else // push the other properties into the 'others'
            {
                TianShanIce::ValueMap::const_iterator it = value.find(key);
                if(it != value.end())
                {
                    props.others[key] = it->second;
                }
                else
                {
                    envExtLog(::ZQ::common::Log::L_WARNING, CLOGFMT(TransferSessionImpl, "setProp() no value with key [%s] found."), key.c_str());
                }
            }
        }
        catch(::TianShanIce::BaseException& ex)
        {
			envExtLog(::ZQ::common::Log::L_DEBUG,CLOGFMT(TransferSessionImpl, "setProp() Session(%s) caught %s:%s"), ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
        }
        catch(...)
        {
            envExtLog(::ZQ::common::Log::L_DEBUG,CLOGFMT(TransferSessionImpl, "setProp() Session(%s) catch Unknow Exception("), ident.name.c_str());
        }
    }
}

//impl of PathTicket
::Ice::Identity TransferSessionImpl::getIdent(const ::Ice::Current& c) const
{
    // no need the lock for this call
    //RLock rLock(*this);
    return ident;
}

::TianShanIce::State TransferSessionImpl::getState(const ::Ice::Current& c) const
{
    RLock rLock(*this);
    return state;
}

void TransferSessionImpl::setState(::TianShanIce::State st, const ::Ice::Current&)
{
    WLock wLock(*this);
    state = st;
}

::TianShanIce::SRM::ResourceMap TransferSessionImpl::getResources(const ::Ice::Current& c) const
{
    RLock rLock(*this);
    return resources;
}

void TransferSessionImpl::setResources(const TianShanIce::SRM::ResourceMap& rs, const ::Ice::Current&) {
    WLock wLock(*this);
    resources = rs;
}
::TianShanIce::ValueMap TransferSessionImpl::getPrivateData(const ::Ice::Current& c) const
{
    RLock rLock(*this);
    return privateData;
}


void TransferSessionImpl::renew(::Ice::Int time, const ::Ice::Current& c)
{
    WLock sync(*this);

    if (time < 0 || time > MAX_TICKET_LEASETERM)
    {
        ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envExtLog,"TransferSessionImpl",1041,"renew() out of range of 0-6hour");
    }

	if( time <= 0 )
	{
		state = ::TianShanIce::stOutOfService;		
	}
	else
	{
		state = ::TianShanIce::stInService;
	}

	envExtLog(::ZQ::common::Log::L_DEBUG, CLOGFMT(TransferSessionImpl, "TransferSession(%s) renew set next time(%d), ThreadPool(%d/%d) PendingRequests(%d)"), ident.name.c_str(), time, _env._pool.activeCount(), _env._pool.size(), _env._pool.pendingRequestSize());
	expiration = ::ZQTianShan::now() + time;

	// update watch dog timer
	_env._watch.watch(ident, time);
}

::TianShanIce::Transport::StorageLinkPrx TransferSessionImpl::getStorageLink(const ::Ice::Current& c) const
{
    return NULL;
    //RLock rLock(*this);
    //return ::TianShanIce::Transport::StorageLinkPrx::uncheckedCast(_env._adapter->createProxy(storageLinkIden));
}

::TianShanIce::Transport::StreamLinkPrx TransferSessionImpl::getStreamLink(const ::Ice::Current& c) const
{
    return NULL;
    //RLock rLock(*this);
    //return ::TianShanIce::Transport::StreamLinkPrx::uncheckedCast(_env._adapter->createProxy(streamLinkIden));
}

::Ice::Int TransferSessionImpl::getCost(const ::Ice::Current& c) const
{
    RLock rLock(*this);
    return 1;
}

::Ice::Int TransferSessionImpl::getLeaseLeft(const ::Ice::Current& c) const
{
    RLock rLock(*this);
    ::Ice::Long nowTime = ::ZQTianShan::now();
    if (nowTime > expiration)//already timeout
        return 0;
    else
        return (::Ice::Int)(expiration - nowTime);
}
void TransferSessionImpl::setExpiration(Ice::Long timeoutMsec, const ::Ice::Current&) {
    WLock sync(*this);
    expiration = ::ZQTianShan::now() + timeoutMsec;
}
void TransferSessionImpl::narrow_async(const ::TianShanIce::Transport::AMD_PathTicket_narrowPtr& amd, const ::TianShanIce::SRM::SessionPrx& sessPrx, const ::Ice::Current& c)
{
    amd->ice_response();
}

void TransferSessionImpl::commit_async(const ::TianShanIce::Transport::AMD_PathTicket_commitPtr& amd, const ::TianShanIce::SRM::SessionPrx& sessPrx, const ::Ice::Current& c)
{
    amd->ice_response();
}

void TransferSessionImpl::destroy(const ::Ice::Current& c)
{
    WLock wLock(*this);
    state = ::TianShanIce::stOutOfService;
}

void TransferSessionImpl::setPenalty(::Ice::Byte, const ::Ice::Current& c)
{

}

//impl of TimeoutObj
void TransferSessionImpl::OnTimer(const ::Ice::Current& c)
{
	bool bExpired = false;
    WLock lock(*this);
    Ice::Long leaseLeft = expiration - ZQTianShan::now();
	if( state == ::TianShanIce::stOutOfService)
	{
		envExtLog(::ZQ::common::Log::L_INFO,CLOGFMT(TransferSessionImpl, "TransferSession(%s) is in OutOfService , trying to destroy it"), ident.name.c_str());
		bExpired = true;
	}
    else if(leaseLeft <= 0)
    { // expired		
		if(stream)
		{
			//check if associated stream session is alive or not
			try
			{
				stream->ice_ping();
			}
			catch( const IceUtil::NullHandleException&)
			{
				bExpired = true;
			}
			catch( const Ice::ObjectNotExistException&)
			{
				bExpired = true;
			}
		}
		else
		{
			bExpired = true;
		}		
    }
   
	if( bExpired )
	{
		char buf[64];
		state = ::TianShanIce::stOutOfService;
		envExtLog(::ZQ::common::Log::L_INFO,CLOGFMT(TransferSessionImpl, "TransferSession(%s) OnTimer set session state to OutOfService per expiration(%s)"), ident.name.c_str(), ZQTianShan::TimeToUTC(expiration, buf, 64));
		_env.reportSessionExpired(ident);
	}
	else
    { // update the timer
		leaseLeft = rand()%30000 + 60000;
        _env._watch.watch(ident, (long)leaseLeft);
        envExtLog(::ZQ::common::Log::L_DEBUG, CLOGFMT(TransferSessionImpl, "TransferSession(%s) OnTimer set next watch time(%lld)"), ident.name.c_str(), leaseLeft);
    }
}
}//namespace SCS
}// namespace TianShanIce
