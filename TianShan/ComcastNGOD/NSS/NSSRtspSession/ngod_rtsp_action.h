#ifndef __NGOD_RTSP_ACTION_H__
#define __NGOD_RTSP_ACTION_H__

#include "ZQ_common_conf.h"
#include "FileLog.h"
#include "ngod_common_structure.h"

namespace ngod_rtsp_action
{
	bool SetupAction(string &strOnDemandSessionId,
					 NSSSessionGroupList &pNSSSessionGroupList,
					 ZQ::common::NativeThreadPool &pool,
					 ZQ::common::FileLog &fileLog);

	bool PlayAction(string &strOnDemandSessionId,
					NSSSessionGroupList &pNSSSessionGroupList,
					ZQ::common::NativeThreadPool &pool,
					ZQ::common::FileLog &fileLog);

	bool PauseAction(string &strOnDemandSessionId,
					 NSSSessionGroupList &pNSSSessionGroupList,
					 ZQ::common::NativeThreadPool &pool,
					 ZQ::common::FileLog &fileLog);

	bool TeardownAction(string &strOnDemandSessionId,
						NSSSessionGroupList &pNSSSessionGroupList,	
						ZQ::common::NativeThreadPool &pool,
						ZQ::common::FileLog &fileLog);

	bool GetParameterAction(string &strSessionGroup,
							NSSSessionGroupList &pNSSSessionGroupList,
							ZQ::common::NativeThreadPool &pool,
							ZQ::common::FileLog &fileLog);

	bool GetParameterActionBySessionId(string &strOnDemandSessionId,
									   NSSSessionGroupList &pNSSSessionGroupList,
									   ZQ::common::NativeThreadPool &pool,
									   ZQ::common::FileLog &fileLog,
									   ::std::vector<GETPARAMETER_EXT> &headerList);


	bool SetParameterAction(string &strSessionGroup,
							NSSSessionGroupList &pNSSSessionGroupList,
							ZQ::common::NativeThreadPool &pool,
							ZQ::common::FileLog &fileLog);


	bool PingAction(string &strOnDemandSessionId,
					NSSSessionGroupList &pNSSSessionGroupList,
					ZQ::common::NativeThreadPool &pool,
					ZQ::common::FileLog &fileLog);

	RTSPClientSession* FindOnDemandSessIter(NSSSessionGroupList &pNSSSessionGroupList,
											string &strOnDemandSessionId,
											NSSSessionGroup **pNSSSessionGroup);

	RTSPClientSession* FindSessIter(NSSSessionGroupList &pNSSSessionGroupList,
									string &strSessionId,
									NSSSessionGroup **pNSSSessionGroup);

	bool waitSignal(RTSPClientSession *sess, uint16 index);

	bool checkGroupSocketStatus(string &strSessionGroup,
								NSSSessionGroupList &pNSSSessionGroupList,
								ZQ::common::NativeThreadPool &pool,
								ZQ::common::FileLog &fileLog);
};

#endif __NGOD_RTSP_ACTION_H__