
#include <boost/thread.hpp>
#include <ZQ_common_conf.h>
#include "CdnEnv.h"
namespace ZQ
{
namespace StreamService
{

CdnSsEnvironment::CdnSsEnvironment(ZQ::common::Log& mainLog, ZQ::common::Log& sessLog , ZQ::common::NativeThreadPool& pool)
	:SsEnvironment(mainLog,sessLog,pool)
{
	mCsPrx						= NULL;
	mHttpLog					= NULL;
	mStreamerManager			= NULL;
	mTransferServerHttpIp		= "10.15.10.50";
	mTransferServerHttpPort		= "5150";
}
CdnSsEnvironment::~CdnSsEnvironment( )
{

}

}}
