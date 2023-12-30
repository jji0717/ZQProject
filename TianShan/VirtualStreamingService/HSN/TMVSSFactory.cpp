#include "TMVSSFactory.h"
#include "TMVStreamImpl.h"
//#include "TMVSSEnv.h"
#include "FileLog.h"

namespace ZQTianShan{
namespace VSS{
namespace TM{

TMVSSFactory::TMVSSFactory(TMVSSEnv& env)
:_env(env)
{
	if (_env._adapter)
	{
		Ice::CommunicatorPtr ic = _env._adapter->getCommunicator();
		
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSFactory, "add factory onto communicator"));
		
		ic->addObjectFactory(this, ::TianShanIce::Streamer::TMVStreamServer::TMVStream::ice_staticId());
	}
}

Ice::ObjectPtr TMVSSFactory::create(const std::string& type)
{
	if (::TianShanIce::Streamer::TMVStreamServer::TMVStream::ice_staticId() == type)
		return new TMVStreamImpl(_env);

    return NULL;
}

void TMVSSFactory::destroy()
{
}

}//namespace TM
}//namespace NSS
}//namespace ZQTianShan