
#include "SsEnvironment.h"
#include "StreamFactory.h"

#ifdef ZQ_OS_MSWIN
#include "memoryDebug.h"
#endif

namespace ZQ 
{
namespace StreamService
{

SsEnvironment::SsEnvironment(ZQ::common::Log& mainLog, ZQ::common::Log& sessLog , ZQ::common::NativeThreadPool& pool)
:mainLogger(mainLog),sessLogger(sessLog),mainThreadPool(pool),mainScheduler(this)
{
}

SsEnvironment::~SsEnvironment()
{
}

bool SsEnvironment::init()
{
	return true;	
}

void SsEnvironment::uninit()
{
}


}}//namespace ZQ::StreamSmith
