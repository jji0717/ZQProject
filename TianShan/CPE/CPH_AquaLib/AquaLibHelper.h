#ifndef _CPH_AQUALIB_HELPER_
#define _CPH_AQUALIB_HELPER_

#include "CPHAquaLibSession.h"
#include "MethodCostImpl.h"

namespace ZQTianShan {
namespace ContentProvision{

class AquaLibHelper : public BaseCPHelper
{
public:
    AquaLibHelper(ZQ::common::NativeThreadPool& pool, ICPHManager* mgr);

    virtual ~AquaLibHelper();

    static BaseCPHelper* _theHelper;

public: // impl of ICPHelper

    ///validate a potential ProvisionSession about to setup
    ///@param[in] sess access to the ProvisionSession about to setup
    ///@param[out] schema the collection of schema definition
    ///@return true if succeeded
    virtual bool validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
        throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource);

    virtual bool getBandwidthLoad(const char* methodType, long& bpsAllocated, long& bpsMax, long& initCost);

    /// query the current load information of a method type
    ///@param[in] methodType to specify the method type to query
    ///@param[out] allocatedKbps the current allocated bandwidth in Kbps
    ///@param[out] maxKbps the maximal allowed bandwidth in Kbps, -1 if unlimited
    ///@param[out] sessions the current running session instances
    ///@param[out] maxSessins the maximal allowed session instances, -1 if unlimited
    ///@return true if the query succeeded
    virtual bool getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins);

    virtual MethodCost* getMethodCost(const std::string& methodType);

    virtual ICPHSession* createHelperSession(const char* methodType, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess);

protected:
    typedef std::map<std::string, MethodCostI*> MethodCostList;

    MethodCostList	_methodCostList;
};

}} // namespace ZQTianShan::ContentProvision

#endif