
#include "StreamFactory.h"
#include "SsStreamImpl.h"

#ifdef ZQ_OS_MSWIN
#include "memoryDebug.h"
#endif

namespace ZQ
{
namespace StreamService
{

StreamFactory::StreamFactory(SsEnvironment* environment , SsServiceImpl& svc )
:env(environment),
svcImpl(svc)
{
}
StreamFactory::~StreamFactory( )
{

}

Ice::ObjectPtr StreamFactory::create(const std::string& id )
{
	if( id == TianShanIce::Streamer::SsPlaylist::ice_staticId() )
		return new SsStreamImpl(svcImpl,env);
	else
	{
		assert(false);
		return NULL;
	}
}

void StreamFactory::destroy()
{
	//do nothing here
}

}}
