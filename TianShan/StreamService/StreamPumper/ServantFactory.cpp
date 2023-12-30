
#include "ServantFactory.h"
#include "SsStreamImpl.h"

#ifdef ZQ_OS_MSWIN
#include "memoryDebug.h"
#endif

namespace ZQ
{
namespace StreamService
{

ServantFactory::ServantFactory(SsEnvironment* environment , SsServiceImpl& svc )
:env(environment),
svcImpl(svc)
{
}
ServantFactory::~ServantFactory( )
{

}

Ice::ObjectPtr ServantFactory::create(const std::string& id )
{
	if( id == TianShanIce::Streamer::SsPlaylist::ice_staticId() )
		return new SsStreamImpl(svcImpl,env);
	else
	{
		assert(false);
		return NULL;
	}
}

void ServantFactory::destroy()
{
	//do nothing here
}

}}
