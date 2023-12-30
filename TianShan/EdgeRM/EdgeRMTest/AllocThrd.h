#include "NativeThread.h"
#include "TianShanDefines.h"
#include "EdgeRM.h"

class AllocThrd : public ZQ::common::NativeThread
{
public:
	AllocThrd(ZQ::common::Log& log, TianShanIce::EdgeResource::EdgeResouceManagerPrx& _erm, TianShanIce::EdgeResource::AllocationOwnerPrx& _allocOwnerPrx, int _allocCount, DWORD _sleepTime);
	~AllocThrd();
	int		run(void);

private:
	ZQ::common::Log&		        log;
	TianShanIce::EdgeResource::EdgeResouceManagerPrx& erm;
	TianShanIce::EdgeResource::AllocationOwnerPrx& allocOwnerPrx;
	int allocCount;
	DWORD sleepTime;
	std::vector<TianShanIce::EdgeResource::AllocationPrx> allocationPrxs;
};
