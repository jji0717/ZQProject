#ifndef __ZQRemovePhoAllocationRequest_H__
#define __ZQRemovePhoAllocationRequest_H__

#include <NativeThread.h>
#include <NativeThreadPool.h>
#include <Ice/Identity.h>
#include "PhoEdgeRMEnv.h"

namespace ZQTianShan {
	namespace EdgeRM {

		class RemovePhoAllocationRequest : public ZQ::common::ThreadRequest
		{
		public: 
			RemovePhoAllocationRequest(ZQTianShan::EdgeRM::PhoEdgeRMEnv& env, const ::Ice::Identity& ident);
			virtual ~RemovePhoAllocationRequest();

		protected: // impls of ScheduleTask
			virtual bool init();
			virtual int run(void);
			virtual void final(int retcode =0, bool bCancelled =false);

		protected: 
			ZQTianShan::EdgeRM::PhoEdgeRMEnv& _env;
			::Ice::Identity _ident;
		}; // class RemovePhoAllocationRequest

} // end namespace EdgeRM
}// end namespace ZQTianShan

#endif //__ZQRemovePhoAllocationRequest_H__
