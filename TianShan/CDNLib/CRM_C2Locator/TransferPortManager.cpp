#include "TransferPortManager.h"
#include "TianShanIceHelper.h"
#include <Text.h>
#undef max
#undef min
#include <boost/rational.hpp>

#ifdef ZQ_OS_LINUX
#include <arpa/inet.h>
#endif

namespace ZQTianShan{
namespace CDN{

class PortRenewTimer: public TianShanUtils::TimeoutObj
{
public:
    PortRenewTimer(C2Env& env, TransferPortManager& tpMgr)
        :_env(env), _tpMgr(tpMgr)
    {
        _defaultCheckTime = _env.replicaReportIntervalSec * 1000 / 2;
        _renewTimeout = 2 * _env.replicaReportIntervalSec * 1000;
    }

    virtual void OnTimer(const Ice::Current& = Ice::Current())
    {
        Ice::Long longestRenewTime = _tpMgr.removeUnavailablePorts(_renewTimeout);
        if(longestRenewTime > 0)
        { // with ports
            _env._watch.watch(_ident, (long)(_renewTimeout - longestRenewTime));
        }
        else
        { // no port
            _env._watch.watch(_ident, (long)_defaultCheckTime);
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
    TransferPortManager& _tpMgr;
    Ice::Identity _ident;
    Ice::Long _defaultCheckTime;
    Ice::Long _renewTimeout;
};
class PortPenaltyReducingTimer: public TianShanUtils::TimeoutObj
{
public:
    PortPenaltyReducingTimer(C2Env& env, TransferPortManager& tpMgr)
        :_env(env), _tpMgr(tpMgr)
    {
    }

    virtual void OnTimer(const Ice::Current& = Ice::Current())
    {
        _tpMgr.reducePenalty(1);
        _env._watch.watch(_ident, (long) _tpMgr.nReducingIntervalMsec_);
    }

    Ice::Identity& ident()
    {
        return _ident;
    }
    void enable()
    {
        _env._watch.watch(_ident, (long) _tpMgr.nReducingIntervalMsec_);
    }
private:
    C2Env& _env;
    TransferPortManager& _tpMgr;
    Ice::Identity _ident;
};

#define C2IdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::SCS::_CLASS##Prx::uncheckedCast((_ENV)._adapter->createProxy(_ID))

TransferPortManager::TransferPortManager(C2Env& env)
    :_log(*env._pLog), _env(env)
{
    // the penalty configuration
    nPunishmentUnit_ = 10;
    nReducingIntervalMsec_ = 1000;
    nRetryLimit_ = 5;
    nPenaltyMax_ = 30;
}

/// @param nPunishmentUnit the number of penalty points per failure
/// @param nReducingIntervalMsec  how long to reduce 1 penalty point
/// @param nRetryLimit     the minamal penalty points needed to retry the port
/// @param nMax            maximum number of the penalty point
void TransferPortManager::configurePenalty(size_t nPunishmentUnit, size_t nReducingIntervalMsec, size_t nRetryLimit, size_t nMax) {
    if(nPunishmentUnit < 0)
        nPunishmentUnit = 0;

    if(nReducingIntervalMsec <= 0)
        nReducingIntervalMsec = 1000;

    if(nRetryLimit < 0)
        nRetryLimit = 0;

    if(nMax <= nPunishmentUnit)
        nMax = 3 * nPunishmentUnit;

    nPunishmentUnit_ = nPunishmentUnit;
    nReducingIntervalMsec_ = nReducingIntervalMsec;
    nRetryLimit_ = nRetryLimit;
    nPenaltyMax_ = nMax;
    _log(::ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager, "configurePenalty() set penalty conf: PunishmentUnit(%d), ReducingIntervalMsec(%d), RetryLimit(%d), MaxPenalty(%d)"), nPunishmentUnit_, nReducingIntervalMsec_, nRetryLimit_, nPenaltyMax_);
}
	
void TransferPortManager::addPenalty(const std::string& name)
{
    ZQ::common::MutexGuard guard(_lock);
    TransferPortMap::iterator it = _tpMap.find(name);
    if(it == _tpMap.end())
		return;

	Ice::Long penalty = it->second.penalty;
	penalty = penalty >= 0 ? (penalty + nPunishmentUnit_) : nPunishmentUnit_;
	if(penalty > nPenaltyMax_)
		penalty = nPenaltyMax_;

	it->second.penalty = penalty;
	_log(::ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager, "addPenalty() the penalty of %s is set to %d"), it->first.c_str(), penalty);

	// return penalty;
}

/// @param name EMPTY string for every port
void TransferPortManager::reducePenalty(size_t point, const std::string& name) {
    if(point <= 0)
        return;

    ZQ::common::MutexGuard guard(_lock);
    if(!name.empty()) {
        TransferPortMap::iterator it = _tpMap.find(name);
        if(it != _tpMap.end()) {
            Ice::Long penalty = it->second.penalty - point;
            if(penalty < 0)
                penalty = 0;

            if((it->second.penalty > nRetryLimit_ && penalty <= nRetryLimit_) ||
               (it->second.penalty > 0 && penalty == 0)) {
                _log(::ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager, "reducePenalty() the penalty of %s is recovered to %d"), it->first.c_str(), penalty);
            }
            it->second.penalty = penalty;
        }
    } else { // reduce all ports' penaly
        TransferPortMap::iterator it = _tpMap.begin();
        for(; it != _tpMap.end(); ++it) {
            Ice::Long penalty = it->second.penalty - point;
            if(penalty < 0)
                penalty = 0;

            if((it->second.penalty > nRetryLimit_ && penalty <= nRetryLimit_) ||
               (it->second.penalty > 0 && penalty == 0)) {
                _log(::ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager, "reducePenalty() the penalty of %s is recovered to %d"), it->first.c_str(), penalty);
            }
            it->second.penalty = penalty;
        }
    }
}

bool TransferPortManager::restore()
{
    ZQ::common::MutexGuard guard(_lock);
    _tpMap.clear();
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
            // update the status of the transfer port
            ::TianShanIce::SCS::TransferSessionProp prop = transferSessionPrx->getProps();
            TransferPortMap::iterator it = _tpMap.find(prop.transferPort);
            if(it != _tpMap.end())
            {
                it->second.activeBandwidth += prop.allocatedBW;
                it->second.capacity += prop.allocatedBW;
                it->second.activeTransferCount += 1;
            }
            else
            {
                PortInfo port;
                port.name = prop.transferPort;
                port.isUp = false;
                port.activeBandwidth = prop.allocatedBW;
                port.capacity = prop.allocatedBW;
                port.activeTransferCount = 1;
                port.enabled = true; // default: enable

                _tpMap[prop.transferPort] = port;
            }
        }
        catch(const ::Ice::UnmarshalOutOfBoundsException& ex)
        {
            _log(::ZQ::common::Log::L_WARNING,CLOGFMT(TransferPortManager, "caught %s when restore TransferSession(%s) from db"), ex.ice_name().c_str(), ident.name.c_str());
            _env._eC2TransferSession->remove(ident);
        }
        catch(const ::Ice::Exception& ex)
        {
            _log(::ZQ::common::Log::L_WARNING ,CLOGFMT(TransferPortManager, "caught %s when restore TransferSession(%s) from db"), ex.ice_name().c_str(), ident.name.c_str());
            _env._eC2TransferSession->remove(ident);
        }
        catch(...)
        {
            _log(::ZQ::common::Log::L_WARNING,CLOGFMT(TransferPortManager, "restore TransferSession(%s) from db caught exception"), ident.name.c_str());
            _env._eC2TransferSession->remove(ident);
        }
    }
    // report the statistics data
    _env.portPerf = gatherStatisticsData();

    return true;
}

Ice::Long TransferPortManager::updatePortStatus(const TianShanIce::SCS::TransferPort& port)
{	
	Ice::Long penlaty = 0 ;
    Ice::Long bwDelta = 0;
    ZQ::common::MutexGuard guard(_lock);
    TransferPortMap::iterator it = _tpMap.find(port.name);
    // only update the port's status fields.
    if(it != _tpMap.end())
    {
        it->second.addressListIPv4 = port.addressListIPv4;
        it->second.addressListIPv6 = port.addressListIPv6;
        bwDelta = port.capacity - it->second.capacity;
        it->second.capacity = port.capacity;
        it->second.isUp = port.isUp;
        it->second.streamService = port.streamService;

        it->second.stampRenew = ZQTianShan::now();

		penlaty = it->second.penalty;		

        if(bwDelta != 0){
            _log(ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager, "port(%s) status updated: %s (%lld(%lld)/%lld), Address(%s)"), it->second.name.c_str(), (it->second.isUp ? "UP" : "DOWN"), it->second.activeBandwidth, it->second.activeTransferCount, it->second.capacity, (ZQ::common::Text::join(it->second.addressListIPv4) + ";" + ZQ::common::Text::join(it->second.addressListIPv6)).c_str());
        }
    }
    else
    {
        PortInfo tp;
        tp.name = port.name;
        // status fields
        tp.addressListIPv4 = port.addressListIPv4;
        tp.addressListIPv6 = port.addressListIPv6;
        bwDelta = port.capacity;
        tp.capacity = port.capacity;
        tp.isUp = port.isUp;
        tp.streamService = port.streamService;

        // fill the usage fields and availability fields
        tp.activeTransferCount = 0;
        tp.activeBandwidth = 0;
        tp.enabled = true;

        tp.stampRenew = ZQTianShan::now();
		
        _tpMap.insert(std::make_pair(tp.name, tp));
        _log(ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager, "new port(%s) discovered: %s (0(0)/%lld), Address(%s)"), tp.name.c_str(), (tp.isUp ? "UP" : "DOWN"), tp.capacity, (ZQ::common::Text::join(tp.addressListIPv4) + ";" + ZQ::common::Text::join(tp.addressListIPv6)).c_str());
    }
	_log(ZQ::common::Log::L_DEBUG,CLOGFMT(TransferPortManager,"updateReplica() port status update:name[%s] up[%s] capacity[%lld]"
		" enable[%s] penalty[%lld] activeBW[%lld] activeSession[%lld]"),
		port.name.c_str(), port.isUp ?"TRUE":"FALSE", port.capacity, port.enabled ?"TRUE":"FALSE",
		port.penalty, port.activeBandwidth, port.activeTransferCount);
    // report the statistics data
    _env.portPerf = gatherStatisticsData();

    return bwDelta;
}
// update the availability fields of the port
void TransferPortManager::updatePortAvailability(const TianShanIce::SCS::TransferPort& port)
{
    ZQ::common::MutexGuard guard(_lock);
    TransferPortMap::iterator it = _tpMap.find(port.name);
    if(it != _tpMap.end())
    {
        it->second.enabled = port.enabled;
        _log(ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager, "%s port[%s]: %s [%lld(%lld)/%lld]"), 
			(port.enabled ? "enable" : "disable"),	port.name.c_str(), (it->second.isUp ? "UP" : "DOWN"), it->second.activeBandwidth, it->second.activeTransferCount, it->second.capacity);
    }
    else
    {
        _log(ZQ::common::Log::L_WARNING, CLOGFMT(TransferPortManager, "updatePortAvailability() No port(%s) found when tring %s it."), port.name.c_str(), (port.enabled ? "enable" : "disable"));
    }
}

/// check if the first string is start with the second
static bool startsWith(const std::string& s1, const std::string& s2)
{
    return (0 == s1.compare(0, s2.size(), s2));
}
/// check if the string is start with any of string in the list
template< class Coll >
static bool startsWithAnyOf(const std::string& s, const Coll& ss)
{
    typename Coll::const_iterator it;
    for(it = ss.begin(); it != ss.end(); ++it)
    {
        if(startsWith(s, *it))
            return true;
    }

    return false;
}

namespace {
    struct TPStatus{
        ::TianShanIce::SCS::TransferPort* port;
        boost::rational<Ice::Long> rank; // store the load info
        bool operator< (const TPStatus& o){ return (rank < o.rank); }
        TPStatus():port(0){}
    };
}

// helper function of subnet comparation
static bool subnetEq4(const char* addr1, const char* addr2, const char* mask);
static bool subnetEq6(const char* addr1, const char* addr2, const char* mask);

bool TransferPortManager::selectAndReserve(const std::string& reqId, ::TianShanIce::SCS::TransferPort& port, Ice::Long allocatedBw, const Strings& portGroupList, const Strings& exclusionList, const Strings& lowPrioritySSList, const SubnetFilter& ipFilter, bool bAssetStack)
{
    // step 1: find out the ports that can supply the bandwidth
    // step 2: remove the ports that are in the exclusion list
    // step 3: split out the ports that are belong to the low priority ss list
    typedef std::list<TPStatus> TPList;
	typedef std::map<std::string, TPList> TPlistMap;
    TPList normalPorts, lowpPorts;
	TPlistMap normalPortsMap, lowPortsMap;
    // we merge the step 1,2,3 into one iteration on the ports
    // and compute the cost rank at the same time

    // construct the port list with the '/' append
    Strings portGroups = portGroupList;
    {
        for(Strings::iterator itGroup = portGroups.begin(); itGroup != portGroups.end(); ++itGroup)
        {
            (*itGroup) += '/';
        }
        Strings::iterator newEnd = std::unique(portGroups.begin(), portGroups.end());
        if(newEnd != portGroups.end())
        {
            portGroups.erase(newEnd, portGroups.end());
        }
    }
	_log(ZQ::common::Log::L_INFO,CLOGFMT(TransferPortManager,"[%s] allocating port with requiredBW[%lld] ports[%s] excludeList[%s]"),
		reqId.c_str(), allocatedBw, ZQTianShan::Util::dumpTianShanIceStrValues(portGroups).c_str(), ZQTianShan::Util::dumpTianShanIceStrValues(exclusionList).c_str());

    ZQ::common::MutexGuard guard(_lock);
	long long timenow = ZQTianShan::now();

    for (TransferPortMap::iterator it = _tpMap.begin(); it != _tpMap.end(); ++it)
    {
		const PortInfo& info = it->second;
        // step 0: ban the down port
        if (!info.isUp || !info.enabled || ((_tpMap.size() >1) && (info.penalty > nRetryLimit_*2)) || (timenow - info.stampRenew > _env.replicaReportIntervalSec * 1000 ) )
        {
			_log(ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager,"[%s] eliminated port[%s] due to: up[%s] enable[%s]"
				"penalty[%lld] retryLimit[%lld] lastRenew[%lld] now[%lld] diff[%lld] reportInterval[%lld]"),
				reqId.c_str(),it->first.c_str(),
				info.isUp?"TRUE":"FALSE", info.enabled ? "TRUE":"FALSE",
				info.penalty, nRetryLimit_,
				info.stampRenew, timenow, (timenow - info.stampRenew), _env.replicaReportIntervalSec * 1000 );
            continue;
        }
//         // step 0.1: exclude the disabled port
//         if(!it->second.enabled)
//         {
//             continue;
//         }
// 
//         // step 0.2: exclude the high penalty port
//         if(it->second.penalty > nRetryLimit_) {
//             continue;
//         }
// 
//         // step 0.3: exclude the timeout port
//         if(ZQTianShan::now() - it->second.stampRenew > _env.replicaReportIntervalSec * 1000)
//         {
//             continue;
//         }

        // step 0.5: check if the port in the desired port group list
        if(!portGroups.empty() && !startsWithAnyOf(it->first, portGroups))
        {
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(TransferPortManager,"[%s] eliminated port[%s]: portGroup[%s] mismatched requested[%s] "),
				reqId.c_str(), it->first.c_str(), ZQTianShan::Util::dumpTianShanIceStrValues(portGroups).c_str() , it->first.c_str() );
            continue;
        }

        // step 1:
        if(it->second.activeBandwidth + allocatedBw > it->second.capacity)
        {
			_log(ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager,"[%s] eliminated port[%s]: port allocatedBw[%lld] requiredBW[%lld] port capacity[%lld]"),
				reqId.c_str(), it->first.c_str(), info.activeBandwidth, allocatedBw, info.capacity );
            continue;
        }

        // step 2:
        if(!exclusionList.empty())
        {
#define AUX_HaveCommonElement(coll1, coll2) (coll1.end() != std::find_first_of(coll1.begin(), coll1.end(), coll2.begin(), coll2.end()))
            if(AUX_HaveCommonElement(it->second.addressListIPv4, exclusionList) || AUX_HaveCommonElement(it->second.addressListIPv6, exclusionList))
            { // exclude it
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(TransferPortManager,"[%s] eliminated port[%s] due to ip[%s] is in exclude-list"),
					reqId.c_str() , it->first.c_str() , ZQTianShan::Util::dumpTianShanIceStrValues(exclusionList).c_str() );
                continue;
            }
//             if(AUX_HaveCommonElement(it->second.addressListIPv6, exclusionList))
//             { // exclude it
//                 continue;
//             }
        }

        // step 2.1
        // filter the address list
        bool withUsableAddress = false;
        if(!ipFilter.target4.empty()) { // filter the v4 address
            for(size_t i4 = 0; i4 < it->second.addressListIPv4.size(); ++i4) {
                const std::string& addr4 = it->second.addressListIPv4[i4];
                if(subnetEq4(addr4.c_str(), ipFilter.target4.c_str(), ipFilter.mask4.c_str())) {
                    withUsableAddress = true;
                    break;
                }
            }
        }

        if(!withUsableAddress && !ipFilter.target6.empty()) { // filter the v6 address
            for(size_t i6 = 0; i6 < it->second.addressListIPv6.size(); ++i6) {
                const std::string& addr6 = it->second.addressListIPv6[i6];
                if(subnetEq6(addr6.c_str(), ipFilter.target6.c_str(), ipFilter.mask6.c_str())) {
                    withUsableAddress = true;
                    break;
                }
            }
        }

        if(!withUsableAddress) // all address filtered
		{
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(TransferPortManager,"[%s] eliminated port[%s] with ipFilter: [%s / %s] [%s / %s]"),
				reqId.c_str(), it->first.c_str() , ipFilter.target4.c_str(), ipFilter.mask4.c_str(), ipFilter.target6.c_str(), ipFilter.mask6.c_str()  );
			continue;
		}

        TPStatus st;
        st.port = &(it->second);
        st.rank.assign(it->second.activeBandwidth + allocatedBw, it->second.capacity);

        // step 3:
        if(lowPrioritySSList.end() == std::find(lowPrioritySSList.begin(), lowPrioritySSList.end(), it->second.streamService))
        {
            normalPorts.push_back(st);

			if(bAssetStack)
			{
				std::string portGroup;
				size_t nPos = (it->first).find('/');
				if(nPos > 0)
					portGroup = (it->first).substr(0, nPos + 1);
				else
					portGroup = (it->first);

				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(TransferPortManager,"[%s] port[%s] portGroup[%s]"),reqId.c_str(), (it->first).c_str(), portGroup.c_str());

				TPlistMap::iterator  tpItor = normalPortsMap.find(portGroup);
				if(tpItor == normalPortsMap.end())
				{
					TPList tpList;
					tpList.push_back(st);
					MAPSET(TPlistMap, normalPortsMap, portGroup, tpList);
				}
				else
				{
					tpItor->second.push_back(st);
				}
			}
        }
        else
        {
            lowpPorts.push_back(st);

			if(bAssetStack)
			{
				std::string portGroup;
				size_t nPos = (it->first).find('/');
				if(nPos > 0)
					portGroup = (it->first).substr(0, nPos + 1);
				else
					portGroup = (it->first);

				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(TransferPortManager,"[%s] port[%s] portGroup[%s]"),reqId.c_str(), (it->first).c_str(), portGroup.c_str());

				TPlistMap::iterator  tpItor = lowPortsMap.find(portGroup);
				if(tpItor == lowPortsMap.end())
				{
					TPList tpList;
					tpList.push_back(st);
					MAPSET(TPlistMap, lowPortsMap, portGroup, tpList);
				}
				else
				{
					tpItor->second.push_back(st);
				}
			}
        }
    }

    // step 4: perform load balancing on the ports base on the cost
    //      4.1: select a port in the normal ports list if the list exist
    //      4.2: or select a port in the low priority ports list
    // find the minimal load rank port
    _log(ZQ::common::Log::L_DEBUG, CLOGFMT(TransferPortManager, "[%s] %d ports available for %lld bandwidth in groups (%s) with (%s) excluded."),
		reqId.c_str(), (normalPorts.size() + lowpPorts.size()), allocatedBw, ZQ::common::Text::join(portGroups).c_str(), ZQ::common::Text::join(exclusionList).c_str());

    ::TianShanIce::SCS::TransferPort* selected = NULL;
    if(!normalPorts.empty())
    { // 4.1
		if(bAssetStack)
		{
			for(Strings::iterator portItor = portGroups.begin();  portItor != portGroups.end(); portItor++)
			{
//				_log(ZQ::common::Log::L_DEBUG,CLOGFMT(TransferPortManager,"[%s] find portGroup[%s] from selected ports"),reqId.c_str(), (*portItor).c_str());

				TPlistMap::iterator  tpItor = normalPortsMap.find(*portItor);
				if(tpItor != normalPortsMap.end())
				{
					TPList& tpList = tpItor->second;
					selected = std::min_element(tpList.begin(), tpList.end())->port;
					break;
				}
			}
		}
		if(!selected)
			selected = std::min_element(normalPorts.begin(), normalPorts.end())->port;
    }
    else if(!lowpPorts.empty())
    { // 4.2
		if(bAssetStack)
		{
			for(Strings::iterator portItor = portGroups.begin();  portItor != portGroups.end(); portItor++)
			{
//				_log(ZQ::common::Log::L_DEBUG,CLOGFMT(TransferPortManager,"[%s] find portGroup[%s] from selected ports"),reqId.c_str(), (*portItor).c_str());

				TPlistMap::iterator  tpItor = lowPortsMap.find(*portItor);
				if(tpItor != lowPortsMap.end())
				{
					TPList& tpList = tpItor->second;
					selected = std::min_element(tpList.begin(), tpList.end())->port;
					break;
				}
			}
		}
		if(!selected)
			selected = std::min_element(lowpPorts.begin(), lowpPorts.end())->port;
    }
    else
    { // no ports available
    }

    // update the selected port and return the result
    if(selected)
    {
        selected->activeTransferCount += 1;
        selected->activeBandwidth += allocatedBw;
        port = *selected;
        // filter the address list
        port.addressListIPv4.clear();
        port.addressListIPv6.clear();
        if(!ipFilter.target4.empty()) { // filter the v4 address
            for(size_t i4 = 0; i4 < selected->addressListIPv4.size(); ++i4) {
                const std::string& addr4 = selected->addressListIPv4[i4];
                if(subnetEq4(addr4.c_str(), ipFilter.target4.c_str(), ipFilter.mask4.c_str())) {
                    port.addressListIPv4.push_back(addr4);
                }
            }
        }
        if(!ipFilter.target6.empty()) { // filter the v6 address
            for(size_t i6 = 0; i6 < selected->addressListIPv6.size(); ++i6) {
                const std::string& addr6 = selected->addressListIPv6[i6];
                if(subnetEq6(addr6.c_str(), ipFilter.target6.c_str(), ipFilter.mask6.c_str())) {
                    port.addressListIPv6.push_back(addr6);
                }
            }
        }

        _log(ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager, "[%s] selected port[%s] w/ [%lld]bps reserved, status:[%lld(%lld)/%lld]"), 
        	   reqId.c_str(), selected->name.c_str(), allocatedBw, selected->activeBandwidth, selected->activeTransferCount, selected->capacity);

        // report the statistics data
        _env.portPerf = gatherStatisticsData();
        return true;
    }
    else
    {
        return false;
    }
}

bool TransferPortManager::removeReservation(const std::string& name, const std::string& reqId, Ice::Long allocatedBw)
{
	ZQ::common::MutexGuard guard(_lock);
	TransferPortMap::iterator it = _tpMap.find(name);
	if (it == _tpMap.end())
	{
		// no port info. the port may not be managed by this object
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(TransferPortManager, "removeReservation() reqId[%s] port[%s] not found to remove reservation."), reqId.c_str(), name.c_str());
		return false;
	}

	if (--it->second.activeTransferCount <0)
		it->second.activeTransferCount =0;

	it->second.activeBandwidth -= allocatedBw;
	if (it->second.activeBandwidth <0)
		it->second.activeBandwidth =0;

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(TransferPortManager, "removeReservation() reqId[%s] port[%s] free-ed[%lld]bps: [%lld(%lld)/%lld]"), reqId.c_str(), name.c_str(), allocatedBw, it->second.activeBandwidth, it->second.activeTransferCount, it->second.capacity);
	// report the statistics data
	_env.portPerf = gatherStatisticsData();
	return true;
}

/// list all the transfer ports with the current state
TianShanIce::SCS::TransferPorts TransferPortManager::snapshot() const
{
    TianShanIce::SCS::TransferPorts ret;
    ZQ::common::MutexGuard guard(_lock);
    ret.reserve(_tpMap.size());
    for(TransferPortMap::const_iterator it = _tpMap.begin(); it != _tpMap.end(); ++it)
    {
		_log(ZQ::common::Log::L_INFO,CLOGFMT(TransferPortManager,"snapshot() %s"),
			it->second.toString().c_str() );
        ret.push_back(it->second);
    }
    return ret;
}

size_t  TransferPortManager::snapshotPortInfos(TransferPortManager::PortInfos & ret) const
{
	ZQ::common::MutexGuard guard(_lock);
	//ret.reserve(_tpMap.size());
	for(TransferPortMap::const_iterator it = _tpMap.begin(); it != _tpMap.end(); ++it)
	{
		PortInfo portInfo = (it->second);
		ret.push_back(portInfo);
		_log(ZQ::common::Log::L_DEBUG,CLOGFMT(TransferPortManager,"snapshotPortInfos() %s"), portInfo.toString().c_str() );
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager,"snapshotPortInfos size[%d], tpMap size[%d]"), ret.size(), _tpMap.size());
	return ret.size();
}
/// query the port with the specified name
bool TransferPortManager::queryPort(const std::string& name, PortInfo& port) const {
    ZQ::common::MutexGuard guard(_lock);
    TransferPortMap::const_iterator it = _tpMap.find(name);
    if(it != _tpMap.end()) {
        port = it->second;
        return true;
    } else {
        return false;
    }
}

void TransferPortManager::startRenewTimer()
{
    // put the timer object to the adapter
    try
    {
        IceInternal::Handle<PortRenewTimer> renewTimer = new PortRenewTimer(_env, *this);
        renewTimer->ident() = _env.timers_->add(renewTimer);
        renewTimerIdent_ = renewTimer->ident();
        renewTimer->enable();
        _log(ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager, "Start the renew timer [%s]."), _env._communicator->identityToString(renewTimerIdent_).c_str());

        configurePenalty(_env._conf.penaltyPunishmentUnit, _env._conf.penaltyReducingIntervalMsec, _env._conf.penaltyRetryLimit, _env._conf.penaltyMax);
        IceInternal::Handle<PortPenaltyReducingTimer> penaltyReducingTimer = new PortPenaltyReducingTimer(_env, *this);
        penaltyReducingTimer->ident() = _env.timers_->add(penaltyReducingTimer);
        penaltyReducingTimerIdent_ = penaltyReducingTimer->ident();
        penaltyReducingTimer->enable();
        _log(ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager, "Start the penalty reducing timer [%s]."), _env._communicator->identityToString(penaltyReducingTimerIdent_).c_str());
    }
    catch(const Ice::Exception& e)
    {
        _log(ZQ::common::Log::L_WARNING, CLOGFMT(TransferPortManager, "Failed to start renew timer and penalty reducing timer. Exception:%s"), e.ice_name().c_str());
    }
}

void TransferPortManager::stopRenewTimer()
{
    try
    {
        _env.timers_->remove(renewTimerIdent_);
        _log(ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager, "Stop the renew timer."));
        _env.timers_->remove(penaltyReducingTimerIdent_);
        _log(ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager, "Stop the penalty reducing timer."));
    }
    catch(const Ice::Exception& e)
    {
        _log(ZQ::common::Log::L_WARNING, CLOGFMT(TransferPortManager, "Failed to stop renew timer and penalty reducing timer. Exception:%s"), e.ice_name().c_str());
    }
}

Ice::Long TransferPortManager::removeUnavailablePorts(Ice::Long timeout)
{
    ZQ::common::MutexGuard guard(_lock);
    // monitor the capacity changing
    Ice::Long currentTotalBw = _env.portPerf.totalBw;

    Ice::Long nRemovingFailed = 0;
    Ice::Long longestRenewTime = 0;
    Ice::Long stampNow = ZQTianShan::now();
    TransferPortMap::iterator it = _tpMap.begin();
    while (it != _tpMap.end())
    {
        Ice::Long renewTime = stampNow - it->second.stampRenew;
        if (renewTime <= timeout) // the good ports
        {
            if(longestRenewTime < renewTime)
                longestRenewTime = renewTime;

			++it;
			continue;
        }

		// the ports that failed to renew
		if (it->second.activeTransferCount <= 0)
		{
			_tpMap.erase(it++);
			_log(ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager, "port[%s] removed due to not renewed within %dmsec"), it->first.c_str(), (int)renewTime);
			continue;
		}

		it->second.isUp = false;
		++nRemovingFailed;
		_log(ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager, "port[%s] not renewed in %dmsec but busy with %d sessions, mark it as 'down'"), it->first.c_str(), (int)renewTime, (int)it->second.activeTransferCount);
		++it;
    }

    if(nRemovingFailed > 0)
    {
        Ice::Long virtualRenewTime = timeout * nRemovingFailed / (nRemovingFailed + 1);
        if(longestRenewTime < virtualRenewTime)
            longestRenewTime = virtualRenewTime;
    }

    // report the statistics data
    _env.portPerf = gatherStatisticsData();

    // monitor the capacity changing
    Ice::Long bwDelta = _env.portPerf.totalBw - currentTotalBw;
    if(bwDelta == 0)
        _log(::ZQ::common::Log::L_INFO, CLOGFMT(TransferPortManager, "removeUnavailablePorts() bandwidth sum of known %dports: %lld"), _tpMap.size(), _env.portPerf.totalBw);
	else
		_log(::ZQ::common::Log::L_WARNING, CLOGFMT(TransferPortManager, "removeUnavailablePorts() bandwidth sum of known %dports changed: %lld (%c%lld)"), _env.portPerf.totalBw, (bwDelta>0)?'+':'-', bwDelta);

    return longestRenewTime;
}


void TransferPortManager::recordStreamCreated(const std::string& name, bool successfully) {
    ZQ::common::MutexGuard guard(_lock);
    TransferPortMap::iterator it = _tpMap.find(name);
    if(it != _tpMap.end()) {
        it->second.sessionCountTotal += 1;
        if(!successfully) {
            it->second.sessionCountFailed += 1;
        }
    }
}

void TransferPortManager::resetStreamCreationCounter() {
    ZQ::common::MutexGuard guard(_lock);
    for(TransferPortMap::iterator it = _tpMap.begin(); it != _tpMap.end(); ++it) {
        it->second.sessionCountTotal = 0;
        it->second.sessionCountFailed = 0;
    }
}
static bool subnetEq4(const char* addr1, const char* addr2, const char* mask) {
    if(NULL == addr1 || NULL == addr2 || NULL == mask)
        return false;
    u_long nAddr1 = inet_addr(addr1);
    u_long nAddr2 = inet_addr(addr2);
    u_long nMask = inet_addr(mask);

    return ((nAddr1 & nMask) == (nAddr2 & nMask));
}

static bool subnetEq6(const char* addr1, const char* addr2, const char* mask) {
    // TODO: impl
    return false;
}

// no thread protection in this helper function
PortPerfData TransferPortManager::gatherStatisticsData() const {
    PortPerfData d;
    d.count = (int32) _tpMap.size();
    for(TransferPortMap::const_iterator it = _tpMap.begin(); it != _tpMap.end(); ++it) {
        d.totalBw += it->second.capacity;
        d.activeBw += it->second.activeBandwidth;
        d.sessions += it->second.activeTransferCount;
    }

    return d;
}
}} // namespace ZQTianShan::CDN
