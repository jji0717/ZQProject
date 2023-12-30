#include "MethodCostImpl.h"

namespace ZQTianShan {
namespace ContentProvision{

    unsigned int MethodCostI::evaluateCost(unsigned int bandwidthKbps, unsigned int sessions)
    {
        if (sessions >_maxsessionNum)
            return MAX_LOAD_VALUE + 1;

        if(bandwidthKbps > _maxBandwidthKBps)	
            return MAX_LOAD_VALUE + 1;

        int nCost1 = (int)(((float)bandwidthKbps)/_maxBandwidthKBps)*MAX_LOAD_VALUE; 
        int nCost2 = (int)(((float)sessions)/_maxsessionNum)*MAX_LOAD_VALUE; 

        return max(nCost1, nCost2);
    }

    MethodCostI::MethodCostI(unsigned int maxBandwidthKBps, unsigned int maxsessionNum)
    {
        _maxsessionNum = maxsessionNum;
        _maxBandwidthKBps = maxBandwidthKBps;
    }

    std::string MethodCostI::getCategory()
    {
        return CPH_AquaLib;
    }

}} // namespace ZQTianShan::ContentProvision