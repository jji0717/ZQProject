#ifndef __RTSP_ACTION_H__
#define __RTSP_ACTION_H__

#include "ZQ_common_conf.h"
#include "FileLog.h"
#include "common_structure.h"

namespace rtsp_action
{
	bool RTSPAction(RTSPClientState state,
					RtspCSeqSignal &rtspCSeqSignal,
					CVSSRtspSession *pCVSSSession,
					::ZQ::common::NativeThreadPool &pool,
					::ZQ::common::FileLog &fileLog);

	bool waitSignal(RtspCSeqSignal &rtspCSeqSignal, uint16 index);
};

#endif __RTSP_ACTION_H__