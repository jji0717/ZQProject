#include "AquaLibHelper.h"
#include "CPH_AquaLibCfg.h"
#include "MethodCostImpl.h"
#include <CPHInc.h>

namespace ZQTianShan {
namespace ContentProvision{

AquaLibHelper::AquaLibHelper(ZQ::common::NativeThreadPool& pool, ICPHManager* mgr)
:BaseCPHelper(pool, mgr) 
{
    std::vector<Method::MethodHolder>::iterator it;
    for (it = _gCPHCfg.provisionMethod.methods.begin();it != _gCPHCfg.provisionMethod.methods.end();it++)
    {
        if (!it->maxSession || !it->maxBandwidth)
            continue;

        _methodCostList[it->methodName] = new MethodCostI(it->maxBandwidth, it->maxSession);				
    }			
}

AquaLibHelper::~AquaLibHelper()
{
    MethodCostList::iterator it = _methodCostList.begin();
    for (;it!=_methodCostList.end();it++)
    {
        if (it->second)
            delete it->second;
    }

    _methodCostList.clear();
}

bool AquaLibHelper::getBandwidthLoad(const char* methodType, long& bpsAllocated, long& bpsMax, long& initCost)
{
    if (stricmp(methodType, METHODTYPE_AQUA_FTPRTF)      &&
        stricmp(methodType, METHODTYPE_AQUA_FTPRTFH264)  &&
		stricmp(methodType, METHODTYPE_AQUA_FTPRTFH265)  &&
        stricmp(methodType, METHODTYPE_AQUA_NTFSRTF)     &&
        stricmp(methodType, METHODTYPE_AQUA_NTFSRTFH264) &&
		stricmp(methodType, METHODTYPE_AQUA_NTFSRTFH265) &&
        stricmp(methodType, METHODTYPE_AQUA_RTI)         &&
        stricmp(methodType, METHODTYPE_AQUA_RTIH264)	 &&
		stricmp(methodType, METHODTYPE_AQUA_RTIH265)	 &&
		stricmp(methodType, METHODTYPE_AQUA_INDEX)       &&
		stricmp(methodType, METHODTYPE_AQUA_INDEXH264)   &&
		stricmp(methodType, METHODTYPE_AQUA_INDEXH265)   &&
		stricmp(methodType, METHODTYPE_RTIRAW))
        return false;

    bpsAllocated =0;

    std::vector<Method::MethodHolder>::iterator methodIter;
    for (methodIter = _gCPHCfg.provisionMethod.methods.begin();methodIter != _gCPHCfg.provisionMethod.methods.end();methodIter++)
    {
        if (stricmp((*methodIter).methodName.c_str(),methodType) == 0)
            break;
    }
    if (methodIter == _gCPHCfg.provisionMethod.methods.end())
        return false;

    bpsMax=(*methodIter).maxBandwidth;
    initCost =0;
    return true;
}

/// query the current load information of a method type
///@param[in] methodType to specify the method type to query
///@param[out] allocatedKbps the current allocated bandwidth in Kbps
///@param[out] maxKbps the maximal allowed bandwidth in Kbps, -1 if unlimited
///@param[out] sessions the current running session instances
///@param[out] maxSessins the maximal allowed session instances, -1 if unlimited
///@return true if the query succeeded
bool AquaLibHelper:: getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins)
{
    std::vector<Method::MethodHolder>::iterator methodIter;
    for (methodIter = _gCPHCfg.provisionMethod.methods.begin();methodIter != _gCPHCfg.provisionMethod.methods.end();methodIter++)
    {
        if (stricmp((*methodIter).methodName.c_str(),methodType) == 0)
            break;
    }
    if (methodIter == _gCPHCfg.provisionMethod.methods.end())
        return false;

    maxKbps = (*methodIter).maxBandwidth;
    maxSessins = (*methodIter).maxSession;

    getCurrentLoad(methodType,allocatedKbps,sessions);

    return true;
}

MethodCost* AquaLibHelper::getMethodCost(const std::string& methodType)
{
    MethodCostList::iterator it = _methodCostList.find(methodType);
    if (it==_methodCostList.end())
        return NULL;

    return it->second;
}

ICPHSession* AquaLibHelper::createHelperSession(const char* methodType, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
{
    if ((NULL == methodType) || 
       (stricmp(methodType, METHODTYPE_AQUA_FTPRTF)      &&
        stricmp(methodType, METHODTYPE_AQUA_FTPRTFH264)  &&
		stricmp(methodType, METHODTYPE_AQUA_FTPRTFH265)  &&
		stricmp(methodType, METHODTYPE_AQUA_NTFSRTF)     &&
		stricmp(methodType, METHODTYPE_AQUA_NTFSRTFH264) &&
		stricmp(methodType, METHODTYPE_AQUA_NTFSRTFH265) &&
		stricmp(methodType, METHODTYPE_AQUA_RTI)         &&
		stricmp(methodType, METHODTYPE_AQUA_RTIH264)	 && 
		stricmp(methodType, METHODTYPE_AQUA_RTIH265)	 &&
		stricmp(methodType, METHODTYPE_AQUA_INDEX)       &&
		stricmp(methodType, METHODTYPE_AQUA_INDEXH264)   &&
		stricmp(methodType, METHODTYPE_AQUA_INDEXH265)   &&
		stricmp(methodType, METHODTYPE_RTIRAW)           &&
		stricmp(methodType, METHODTYPE_AQUA_CSI)))
		return NULL;

	if(stricmp(methodType, METHODTYPE_AQUA_CSI) == 0 )
		return new CPHAquaLibAutoCheckSess(getLog(), *this, pSess);
	else
    return new CPHAquaLibSess(getLog(), *this, pSess);
}
}} // namespace ZQTianShan::ContentProvision