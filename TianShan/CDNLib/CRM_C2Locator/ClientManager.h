#ifndef __C2Locator_ClientManager_H__
#define __C2Locator_ClientManager_H__

#include <ZQ_common_conf.h>
#include "C2Env.h"
#include "C2Locator.h"

namespace ZQTianShan{
namespace CDN{

#define CDN_IngressCapacity_NoLimit 0

class ClientManager
{
public:
    // management structure
    struct ClientInfo: public TianShanIce::SCS::ClientTransfer
    {
        ClientInfo()
        {
            ingressCapacity = 0;
            activeTransferCount = 0;
            consumedBandwidth = 0;
            stampIdle = 0;
        }

		std::string toString() const
		{
			char buffer[1024];
			buffer[ sizeof(buffer)-1 ] = 0;
			snprintf(buffer, sizeof(buffer)-2, "address[%s] ingressCapacity[%lld] activeTransferCount[%lld] consumedBandwidth[%lld] stampIdle[%lld]",
				                             address.c_str(), ingressCapacity, activeTransferCount, consumedBandwidth, stampIdle);

			return std::string(buffer);
		}

        Ice::Long stampIdle;
    };
public:
    explicit ClientManager(C2Env& env);
    bool restore(); // restore data from db
    bool reserveTransfer(const std::string& reqId, const std::string& client, Ice::Long ingressCapacity, Ice::Long allocatedBw);
    bool removeReservation(const std::string& client, const std::string& reqId, Ice::Long allocatedBw, ClientInfo* status = NULL);
    //    bool getCapacityStatus(const std::string& client, TianShanIce::SCS::ClientTransfer& status);

    /// list all the clients with current state
    TianShanIce::SCS::ClientTransfers snapshot() const;
	size_t  snapshotTransfers(TianShanIce::SCS::ClientTransfers& ret);

    void startIdleTimer();
    void stopIdleTimer();

    // remove the clients that expired due to the timeout
    // and return the longest idle time of the rest clients
    Ice::Long removeIdleClients(Ice::Long idleTimeout);
private:
    // no thread protection in this helper function
    ClientPerfData gatherStatisticsData() const;
private:
    ZQ::common::Log& _log;
    ZQ::common::Mutex _lock;
    C2Env& _env;
    typedef std::map<std::string, ClientInfo> Clients;
    Clients _clients;

    Ice::Identity idleTimerIdent_;
};

}} // namesapce ZQTianShan::CDN

#endif
