#include "NativeThreadPool.h"
#include "TianShanDefines.h"
#include "EdgeRM.h"

class AllocationRequest : public ZQ::common::ThreadRequest
{
public:
	AllocationRequest(ZQ::common::NativeThreadPool& _thrdPool, ZQ::common::Log& log, TianShanIce::EdgeResource::EdgeResouceManagerPrx& _erm, TianShanIce::EdgeResource::AllocationOwnerPrx& _allocOwnerPrx);
	~AllocationRequest();

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void);

private:
	ZQ::common::Log&		        log;
	TianShanIce::EdgeResource::EdgeResouceManagerPrx& erm;
	TianShanIce::EdgeResource::AllocationOwnerPrx& allocOwnerPrx;
	std::string ident;
	static int sgCount;
};
