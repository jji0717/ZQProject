#include "ClientManager.h"

namespace ZQTianShan{
namespace CDN{

class ClientIdleTimer: public TianShanUtils::TimeoutObj
{
public:
    ClientIdleTimer(C2Env& env, ClientManager& clientMgr)
        :_env(env), _clientMgr(clientMgr)
    {
        _defaultCheckTime = 2 * 60 * 60 * 1000; // 2h
        _idleTimeout = 24 * 60 * 60 * 1000; // 24h
    }

    virtual void OnTimer(const Ice::Current& = Ice::Current())
    {
        Ice::Long longestIdleTime = _clientMgr.removeIdleClients(_idleTimeout);
        if(longestIdleTime > 0)
        { // with some idle clients
            _env._watch.watch(_ident, (_idleTimeout - longestIdleTime));
        }
        else // no idle client
        {
            _env._watch.watch(_ident, _defaultCheckTime);
        }
    }

    Ice::Identity& ident()
    {
        return _ident;
    }
    void enable()
    {
        _env._watch.watch(_ident, 0);
    }
private:
    C2Env& _env;
    ClientManager& _clientMgr;
    Ice::Identity _ident;
    Ice::Long _defaultCheckTime;
    Ice::Long _idleTimeout;
};

#define C2IdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::SCS::_CLASS##Prx::uncheckedCast((_ENV)._adapter->createProxy(_ID))

ClientManager::ClientManager(C2Env& env)
    :_log(*env._pLog), _env(env)
{
}

bool ClientManager::restore()
{
    ZQ::common::MutexGuard guard(_lock);
    _clients.clear();
    Freeze::EvictorIteratorPtr evicIter = _env._eC2TransferSession->getIterator("", _env._eC2TransferSession->getSize());
    const Ice::Context context;
    while (evicIter && evicIter->hasNext())
    {
        ::Ice::Identity ident = evicIter->next();
        try
        {
            TianShanIce::SCS::TransferSessionPrx transferSessionPrx = C2IdentityToObjEnv(_env, TransferSession, ident);
            if (!transferSessionPrx)
                continue;
            // update the status of the client
            ::TianShanIce::SCS::TransferSessionProp prop = transferSessionPrx->getProps();
            Clients::iterator it = _clients.find(prop.clientTransfer);
            if(it != _clients.end())
            {
                it->second.consumedBandwidth += prop.allocatedBW;
                it->second.ingressCapacity += prop.allocatedBW;
                it->second.activeTransferCount += 1;
            }
            else
            {
                if(prop.clientTransfer.empty()) {
                    _log(::ZQ::common::Log::L_WARNING,CLOGFMT(ClientManager, "restore() got empty client transfer, identity=(%s), bandwidth=(%lld)"), ident.name.c_str(), prop.allocatedBW);
                    continue;
                }
                ClientInfo client;
                client.address = prop.clientTransfer;
                client.consumedBandwidth = prop.allocatedBW;
                client.ingressCapacity = prop.allocatedBW;
                client.activeTransferCount = 1;
                _clients[prop.clientTransfer] = client;
            }
        }
        catch(::Ice::UnmarshalOutOfBoundsException &)
        {
            _log(::ZQ::common::Log::L_WARNING,CLOGFMT(ClientManager, "catch UnmarshalOutOfBoundsException when restore TransferSession(%s) from db"), ident.name.c_str());
            _env._eC2TransferSession->remove(ident);
        }
        catch(::Ice::Exception &ex)
        {
            _log(::ZQ::common::Log::L_WARNING,CLOGFMT(ClientManager, "catch %s when restore TransferSession(%s) from db"), ex.ice_name().c_str(), ident.name.c_str());
            _env._eC2TransferSession->remove(ident);
        }
        catch(...)
        {
            _log(::ZQ::common::Log::L_WARNING,CLOGFMT(ClientManager, "restore TransferSession(%s) from db catch unknown error"), ident.name.c_str());
            _env._eC2TransferSession->remove(ident);
        }
    }

    // report statistics data
    _env.clientPerf = gatherStatisticsData();
    return true;
}
bool ClientManager::reserveTransfer(const std::string& reqId, const std::string& client, Ice::Long ingressCapacity, Ice::Long allocatedBw)
{
    ZQ::common::MutexGuard guard(_lock);
    Clients::iterator it = _clients.find(client);
    if(it == _clients.end())
    { // new client detected
        _log(ZQ::common::Log::L_INFO, CLOGFMT(ClientManager, "[%s] New client [%s] detected with ingress capacity [%lld]."), reqId.c_str(), client.c_str(), ingressCapacity);
        ClientInfo c;
        c.address = client;
        c.ingressCapacity = ingressCapacity;
        c.consumedBandwidth = 0;
        c.activeTransferCount = 0;
        it = _clients.insert(std::make_pair(client, c)).first;
    }
    it->second.stampIdle = 0; // active client

    // update the capacity first
    if(it->second.ingressCapacity != ingressCapacity)
    {
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(ClientManager, "[%s] Client ingress capacity updated: [%lld] -> [%lld]. client:[%s]"), reqId.c_str(), it->second.ingressCapacity, ingressCapacity, client.c_str());
        it->second.ingressCapacity = ingressCapacity;
    }

    // check the restriction
    if(CDN_IngressCapacity_NoLimit == ingressCapacity || (allocatedBw + it->second.consumedBandwidth) <= ingressCapacity)
    {
        it->second.consumedBandwidth += allocatedBw;
        it->second.activeTransferCount += 1;
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(ClientManager, "[%s] Reserved transfer with [%lld] bitrate. status:[%lld(%lld)/%lld]. client:[%s]"), reqId.c_str(), allocatedBw, it->second.consumedBandwidth, it->second.activeTransferCount, it->second.ingressCapacity, client.c_str());

        // report statistics data
        _env.clientPerf = gatherStatisticsData();

        return true;
    }
    else
    { // exceed the restriction
        _log(ZQ::common::Log::L_WARNING, CLOGFMT(ClientManager, "[%s] Client bandwidth exceed. Required bitrate:[%lld]. status:[%lld(%lld)/%lld]. client:[%s]"), reqId.c_str(), allocatedBw, it->second.consumedBandwidth, it->second.activeTransferCount, it->second.ingressCapacity, client.c_str());
        return false;
    }
}

bool ClientManager::removeReservation(const std::string& client, const std::string& reqId, Ice::Long allocatedBw, ClientInfo* status)
{
	ZQ::common::MutexGuard guard(_lock);
	Clients::iterator it = _clients.find(client);
	if(it == _clients.end())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(ClientManager, "removeReservation() reqId[%s] no client[%s] found to free allocatedBw[%lld]bps"), reqId.c_str(), client.c_str(), allocatedBw);
		return false;
	}

	if (--it->second.activeTransferCount <0)
		it->second.activeTransferCount =0;

	it->second.consumedBandwidth -= allocatedBw;
	if (it->second.consumedBandwidth <0)
		it->second.consumedBandwidth =0;

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ClientManager, "removeReservation() reqId[%s] client[%s] free-ed [%lld]bps: [%lld(%lld)/%lld]"),
		reqId.c_str(), client.c_str(), allocatedBw, it->second.consumedBandwidth, it->second.activeTransferCount, it->second.ingressCapacity);

	if (it->second.activeTransferCount <= 0)
		it->second.stampIdle = ZQTianShan::now();

	// report statistics data
	_env.clientPerf = gatherStatisticsData();

	if (status)
		*status = it->second;

	return true;
}
/*
bool ClientManager::getCapacityStatus(const std::string& client, TianShanIce::SCS::ClientTransfer& status)
{
    ZQ::common::MutexGuard guard(_lock);
    Clients::iterator it = _clients.find(client);
    if(it != _clients.end())
    {
        status = it->second;
        return true;
    }
    else
    {
        return false;
    }
}
*/
/// list all the clients with current state
TianShanIce::SCS::ClientTransfers ClientManager::snapshot() const
{
    TianShanIce::SCS::ClientTransfers ret;
    ZQ::common::MutexGuard guard(_lock);
    ret.reserve(_clients.size());
    for(Clients::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        ret.push_back(it->second);
    }

	_log(ZQ::common::Log::L_INFO, CLOGFMT(ClientManager,"snapshot() size[%d], _clients size[%d]"), ret.size(), _clients.size());
	return ret;
}

size_t  ClientManager::snapshotTransfers(TianShanIce::SCS::ClientTransfers& ret)
{
	ZQ::common::MutexGuard guard(_lock);
	ret.reserve(_clients.size());
	for(Clients::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
        ret.push_back(it->second);
//		ClientInfo clientInfo = (it->second);
//		_log(ZQ::common::Log::L_DEBUG,CLOGFMT(ClientManager,"snapshotTransfers() %s"), clientInfo.toString().c_str() );
//		ret.push_back(clientInfo);
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(ClientManager,"snapshotTransfers() size[%d], _clients size[%d]"), ret.size(), _clients.size());
	return ret.size();
}

void ClientManager::startIdleTimer()
{
    IceInternal::Handle<ClientIdleTimer> idleTimer = new ClientIdleTimer(_env, *this);
    // put the timer object to the adapter
    try
    {
        idleTimer->ident() = _env.timers_->add(idleTimer);
        idleTimerIdent_ = idleTimer->ident();
        idleTimer->enable();
        _log(ZQ::common::Log::L_INFO, CLOGFMT(ClientManager, "Start the idle timer [%s]."), _env._communicator->identityToString(idleTimerIdent_).c_str());
    }
    catch(const Ice::Exception& e)
    {
        _log(ZQ::common::Log::L_WARNING, CLOGFMT(ClientManager, "Failed to start idle timer. Exception:%s"), e.ice_name().c_str());
    }
}

void ClientManager::stopIdleTimer()
{
    try
    {
        _env.timers_->remove(idleTimerIdent_);
        _log(ZQ::common::Log::L_INFO, CLOGFMT(ClientManager, "Stop the idle timer."));
    }
    catch(const Ice::Exception& e)
    {
        _log(ZQ::common::Log::L_WARNING, CLOGFMT(ClientManager, "Failed to stop idle timer. Exception:%s"), e.ice_name().c_str());
    }
}
Ice::Long ClientManager::removeIdleClients(Ice::Long idleTimeout)
{
    ZQ::common::MutexGuard guard(_lock);

    Ice::Long longestIdleTime = -1;

    Ice::Long stampNow = ZQTianShan::now();
    Clients::iterator it = _clients.begin();
    while(it != _clients.end())
    {
        if(it->second.stampIdle > 0) // idle
        {
            Ice::Long idleTime = stampNow - it->second.stampIdle;
            if(idleTime >= idleTimeout) // expired
            {
                _log(ZQ::common::Log::L_INFO, CLOGFMT(ClientManager, "Remove client info due to idle timeout. Idle time:[%lld] client:[%s]"), idleTime, it->first.c_str());
                _clients.erase(it++);
                continue;
            }
            else
            {
                if(idleTime > longestIdleTime)
                {
                    longestIdleTime = idleTime; 
                }
            }
        }
        // else: active
        ++it; // move to next
    }
    // report statistics data
    _env.clientPerf = gatherStatisticsData();

    return longestIdleTime;
}

// no thread protection in this helper function
ClientPerfData ClientManager::gatherStatisticsData() const {
    ClientPerfData d;
    d.count = _clients.size();
    return d;
}

}} // namespace ZQTianShan::CDN
