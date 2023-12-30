#ifndef __TS_HAMMER_RTSP_SESSION_HANDLER_H__
#define __TS_HAMMER_RTSP_SESSION_HANDLER_H__

#include "Log.h"
#include "RtspInterface.h"
#include "RtspSession.h"

namespace ZQHammer
{

class RtspSessionHandler : public ZQRtspCommon::IHandler
{
public:
	RtspSessionHandler(ZQ::common::Log* log, RtspSessionMap& sessionMap);
	~RtspSessionHandler();
public:
	virtual bool HandleMsg(ZQRtspCommon::IRtspReceiveMsg* receiveMsg, ZQRtspCommon::IRtspSendMsg* sendMsg);
	virtual void onCommunicatorError(ZQ::DataPostHouse::IDataCommunicatorPtr communicator);
private:
	ZQ::common::Log* _log;
	RtspSessionMap& _sessionMap;
};

} // end for ZQHammer

#endif
