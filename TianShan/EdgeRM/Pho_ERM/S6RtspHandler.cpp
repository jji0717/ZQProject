#ifdef ZQ_OS_MSWIN
#include "StdAfx.h"
#endif
#include "S6RtspHandler.h"
namespace ZQTianShan {
	namespace EdgeRM {
S6RtspHandler::S6RtspHandler(ZQTianShan::EdgeRM::PhoEdgeRMEnv& env)
:_env(env)
{

}

S6RtspHandler::~S6RtspHandler(void)
{

}

bool S6RtspHandler::HandleMsg(ZQRtspCommon::IRtspReceiveMsg* receiveMsg, ZQRtspCommon::IRtspSendMsg* sendMsg)
{
	switch(receiveMsg->getVerb())
	{
	case RTSP_MTHD_RESPONSE:
		{
			return Response(receiveMsg, sendMsg);
		}
		break;
	case RTSP_MTHD_ANNOUNCE:
		{
            return Announce(receiveMsg, sendMsg);
		}
		break;
	default:
		sendMsg->setStartline(ResponseMethodNotAllowed);
		sendMsg->post();
		return true;
	}	
}

bool S6RtspHandler::Response(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response)
{
	::std::string strMethod = "RESPONSE";
	::std::string strCSeq = request->getHeader(NGOD_HEADER_SEQ);
	::Ice::Long lSeq = _atoi64(strCSeq.c_str());

	{
		ZQ::common::MutexGuard gd(_env._s6responsesMutex);
		S6RESPONSES::iterator itor = _env.s6responses.find(lSeq);
		if(itor == _env.s6responses.end())
		{
//			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6RtspHandler, "ingore this response %s"), strCSeq.c_str());
			return false;
		}
		(itor->second)->setResponse(request);
		(itor->second)->setProcessHandle();
		_env.s6responses.erase(itor);
	}
	return true;
}
bool S6RtspHandler::Announce(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response)
{
    try
    {
		(new S6Announce(_env, request))->start();
    }
    catch (...)
    {
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(S6RtspHandler, "Fail to create S6Announce thread(%d)"),SYS::getLastErr());
		return false;
    }
	return true;
}

void S6RtspHandler::onCommunicatorError(ZQ::DataPostHouse::IDataCommunicatorPtr communicator)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(S6RtspHandler, "enter onCommunicatorError() (0x%08x) ConnectId(%lld)"), 
		 (unsigned)communicator, communicator->getCommunicatorId());

	try
	{
		ZQ::common::MutexGuard gd(_env._rtspclentsMutex);
		ZQTianShan::EdgeRM::RTSPCLIENTS::iterator itor = _env._rtspclients.begin();
		while(itor != _env._rtspclients.end())
		{
			if(itor->second.communicator == communicator)
			{
				glog(ZQ::common::Log::L_INFO, CLOGFMT(S6RtspHandler, "onCommunicatorError() remove rtsp client [%s]"), 
					(itor->first).c_str());
				ZQTianShan::EdgeRM::RTSPCLIENTS::iterator iter2 = itor;
				_env._rtspclients.erase(iter2);
			}
			++itor;
		}

	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(S6RtspHandler, "onCommunicatorError()(0x%08x) ConnectId(%lld) caught unknown exception (%d)"), 
			(unsigned)communicator, communicator->getCommunicatorId(), SYS::getLastErr());
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(S6RtspHandler, "leave onCommunicatorError() (0x%08x) ConnectId(%lld)"), 
		 (unsigned)communicator, communicator->getCommunicatorId());
}
} ///end namespace EdgeRM
} ///end namespace ZQTianShan
