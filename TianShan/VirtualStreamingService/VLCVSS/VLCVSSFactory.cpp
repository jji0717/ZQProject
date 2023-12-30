#include "VLCVSSFactory.h"
#include "VLCStreamImpl.h"
#include "FileLog.h"

namespace ZQTianShan{
namespace VSS{
namespace VLC{

VLCVSSFactory::VLCVSSFactory(VLCVSSEnv& env)
:_env(env)
{
	if (_env._adapter)
	{
		Ice::CommunicatorPtr ic = _env._adapter->getCommunicator();
		
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSFactory, "add factory into communicator"));
		
		ic->addObjectFactory(this, ::TianShanIce::Streamer::VLCStreamServer::VLCStream::ice_staticId());
	}
}

Ice::ObjectPtr VLCVSSFactory::create(const std::string& type)
{
	if (::TianShanIce::Streamer::VLCStreamServer::VLCStream::ice_staticId() == type)
		return new VLCStreamImpl(_env);

    return NULL;
}

void VLCVSSFactory::destroy()
{
}

}//namespace VLC
}//namespace VSS
}//namespace ZQTianShan