#ifndef _METHOD_COST_IMPL_H
#define _METHOD_COST_IMPL_H

#include "IMethodCost.h"
#include "TianShanDefines.h"
#include "CPH_AquaLib.h"


//class BaseGraph;
//class PushSource;
//class BaseTarget;
//class FTPMSClientFactory;
//class HTTPClientFactory;

namespace ZQTianShan {
namespace ContentProvision{

    class MethodCostI : public MethodCost
    {
    public:
        MethodCostI(unsigned int maxBandwidthKBps, unsigned int maxsessionNum);

        /// evaluate the cost per given session count and total allocated bandwidth
        ///@param[in] bandwidthKbps to specify the allocated bandwidth in Kbps
        ///@param[in] sessions to specify the allocated session instances
        ///@return a cost in the range of [0, MAX_LOAD_VALUE] at the given load level:
        ///		0 - fully available
        ///		MAX_LOAD_VALUE - completely unavailable
        virtual unsigned int evaluateCost(unsigned int bandwidthKbps, unsigned int sessions);   

        virtual std::string getCategory();

    protected:
        unsigned int _maxsessionNum;
        unsigned int _maxBandwidthKBps;
    };

}} // namespace ZQTianShan::ContentProvision

#endif