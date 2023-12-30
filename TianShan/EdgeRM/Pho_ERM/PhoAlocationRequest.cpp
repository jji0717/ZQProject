#ifdef ZQ_OS_MSWIN
#include "StdAfx.h"
#endif
#include "PhoAlocationRequest.h"

#define RemvoePhoAllocMacro    "RemovePhoAllocationR"
namespace ZQTianShan {
	namespace EdgeRM {
		RemovePhoAllocationRequest::RemovePhoAllocationRequest(ZQTianShan::EdgeRM::PhoEdgeRMEnv& env, const ::Ice::Identity& ident)
		:ZQ::common::ThreadRequest(*(env._pThreadPool)), _env(env), _ident(ident)
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(RemovePhoAllocationRequest, "[%s]remove PhoAllocation object request"), 
				_ident.name.c_str());
		}
		RemovePhoAllocationRequest::~RemovePhoAllocationRequest()
		{
		}
		bool RemovePhoAllocationRequest::init()
		{
			return true;
		}
		int RemovePhoAllocationRequest::run(void)
		{
			::TianShanIce::EdgeResource::AllocationPrx allocPrx = NULL;
			::TianShanIce::EdgeResource::PhoAllocationPrx phoAllocPrx = NULL;
			try
			{
				phoAllocPrx = IdentityToObjEnv2(_env, PhoAllocation, _ident);
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(RemovePhoAllocationRequest, "fail to get phoAllocation object [%s] caught ice exception [%s]"),
					_ident.name.c_str(), ex.ice_name().c_str());
				return false;
			}
			try
			{
				allocPrx = phoAllocPrx->getAllocation();

				allocPrx->destroy();
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(RemovePhoAllocationRequest, "check pho allocation[%s] not InService"), _ident.name.c_str());
				_env._ePhoAllocation->remove(_ident);

				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(RemovePhoAllocationRequest, "check pho allocation[%s] InService"), _ident.name.c_str());
			}
			catch (::TianShanIce::BaseException &ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(RemovePhoAllocationRequest, "check pho allocation[%s] catch exception(%s)"), _ident.name.c_str(), ex.ice_name().c_str());
			}
			catch (::Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(RemovePhoAllocationRequest, "check pho allocation[%s] catch exception(%s)"), _ident.name.c_str(), ex.ice_name().c_str());
			}
			catch (...)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(RemovePhoAllocationRequest, "check pho allocation[%s] catch unknown exception"), _ident.name.c_str());
			}
           return -1;
		}
		void RemovePhoAllocationRequest::final(int retcode, bool bCancelled)
		{
			delete this;
		}

	} // end namespace EdgeRM
}// end namespace ZQTianShan
