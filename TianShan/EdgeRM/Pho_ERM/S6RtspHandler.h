#ifndef __ZQ_RTSP_S6RTSPHANDLER_H__
#define __ZQ_RTSP_S6RTSPHANDLER_H__

#include "Log.h"
#include "RtspInterface.h"
#include "definition.h"
#include "PhoEdgeRMEnv.h"
namespace ZQTianShan {
namespace EdgeRM {

	class S6RtspHandler : public ZQRtspCommon::IHandler
	{
	public:
		S6RtspHandler(ZQTianShan::EdgeRM::PhoEdgeRMEnv& env);
		virtual ~S6RtspHandler();
	public:
		virtual bool HandleMsg(ZQRtspCommon::IRtspReceiveMsg* receiveMsg, ZQRtspCommon::IRtspSendMsg* sendMsg);
		virtual void onCommunicatorError(ZQ::DataPostHouse::IDataCommunicatorPtr communicator);
	private:
		bool Response(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response);
		bool Announce(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response);
	protected:
		ZQTianShan::EdgeRM::PhoEdgeRMEnv& _env;
	};

} // end for S6RtspHandler
}

#endif // __ZQ_RTSP_S6RTSPHANDLER_H__
