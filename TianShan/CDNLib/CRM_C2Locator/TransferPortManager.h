#ifndef __C2Locator_TransferPortManager_H__
#define __C2Locator_TransferPortManager_H__

#include <ZQ_common_conf.h>
#include "C2Env.h"
#include "C2Locator.h"
#include <TianShanIceHelper.h>

typedef std::vector<std::string> Strings;
namespace ZQTianShan{
namespace CDN{
struct SubnetFilter {
    std::string mask4;
    std::string target4;

    std::string mask6;
    std::string target6;
};

class PortPenaltyReducingTimer;

class TransferPortManager
{
public:
    struct PortInfo: public TianShanIce::SCS::TransferPort
    {
        PortInfo()
        {
            isUp = false;
            capacity = 0;
            activeBandwidth = 0;
            activeTransferCount = 0;
            enabled = false;
            penalty = 0;

            stampRenew = 0;
            sessionCountTotal = 0;
            sessionCountFailed = 0;
        }

		std::string toString() const
		{
			char buffer[1024];
			buffer[ sizeof(buffer)-1 ] = 0;
			snprintf(buffer,sizeof(buffer)-2, "NAME[%s] UP[%s] ENABLE[%s] capacity[%lld] bw[%lld] sesscount[%lld] penalty[%lld] ipv4[%s] ipv6[%s]",
				name.c_str() , isUp?"TRUE":"FALSE", enabled ?"TRUE":"FALSE",
				capacity, activeBandwidth, activeTransferCount,penalty,
				ZQTianShan::Util::dumpTianShanIceStrValues(addressListIPv4).c_str(),
				ZQTianShan::Util::dumpTianShanIceStrValues(addressListIPv6).c_str());
			return std::string(buffer);
		}
		
        Ice::Long stampRenew;
        Ice::Long sessionCountTotal;
        Ice::Long sessionCountFailed;
    };

	typedef ::std::vector< struct PortInfo> PortInfos;
    friend class PortPenaltyReducingTimer;
    explicit TransferPortManager(C2Env& env);
    bool restore();

    // update the status fields of the port
    // return the total bandwidth delta
    Ice::Long updatePortStatus(const TianShanIce::SCS::TransferPort& port);

    // update the availability fields of the port
    void updatePortAvailability(const TianShanIce::SCS::TransferPort& port);

    /// @param port the selected port info
    /// @param allocatedBw the bandwidth to be allocated
    /// @param portGroupList the limitation of the port group. all group will
    ///                      be included in the selection if this list is empty
    /// @param exclusionList the port address that will be exclude in the selection
    /// @param lowPrioritySSList the stream service that in low priority in the selection
    /// @param ipFilter the subnet filter
    /// @return true for success, false for failure.
    bool selectAndReserve(const std::string& reqId, ::TianShanIce::SCS::TransferPort& port, Ice::Long allocatedBw, const Strings& portGroupList, const Strings& exclusionList, const Strings& lowPrioritySSList, const SubnetFilter& ipFilter, bool bAssetStack = false);
	bool removeReservation(const std::string& name, const std::string& reqId, Ice::Long allocatedBw);

    /// list all the transfer ports with the current state
    TianShanIce::SCS::TransferPorts snapshot() const;
	size_t  snapshotPortInfos(PortInfos & ret) const;

    /// query the port with the specified name
    bool queryPort(const std::string& name, PortInfo& port) const;

    void startRenewTimer();
    void stopRenewTimer();
    Ice::Long removeUnavailablePorts(Ice::Long timeout);

    void recordStreamCreated(const std::string& name, bool successfully);
    void resetStreamCreationCounter();

    /// @param nPunishmentUnit the number of penalty points per failure
    /// @param nReducingIntervalMsec  how long to reduce 1 penalty point
    /// @param nRetryLimit     the minimal penalty points needed to retry the port
    /// @param nMax            maximum number of the penalty point
    void configurePenalty(size_t nPunishmentUnit, size_t nReducingIntervalMsec, size_t nRetryLimit, size_t nMax);
    void addPenalty(const std::string& name);

private:
    /// @param name EMPTY string for every port
    void reducePenalty(size_t point, const std::string& name = "");

    // no thread protection in this helper function
    PortPerfData gatherStatisticsData() const;
private:
    ZQ::common::Log& _log;
    ZQ::common::Mutex _lock;
    C2Env& _env;

    typedef std::map<std::string, PortInfo> TransferPortMap;
    TransferPortMap _tpMap;

    /// penalty configuration
    Ice::Long nPunishmentUnit_;
    Ice::Long nReducingIntervalMsec_;
    Ice::Long nRetryLimit_;
    Ice::Long nPenaltyMax_;

    // timers
    Ice::Identity renewTimerIdent_;
    Ice::Identity penaltyReducingTimerIdent_;
};

}} // namespace ZQTianShan::CDN

#endif
