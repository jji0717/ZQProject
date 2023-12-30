//file to define the thread
//inherit NativeThread
//chop the RTSP message from NGOD server
//after chop, stored the message into a message list
#pragma once

#include "ngod_common_structure.h"
#include "NSSImpl.h"

class NSSCommitThrd : public ZQ::common::NativeThread
{
public:
	NSSCommitThrd(::ZQTianShan::NSS::NGODStreamImplPtr NGODStreamObj,
				  const ::TianShanIce::Streamer::AMD_Stream_commitPtr amdStream);

	~NSSCommitThrd();

	bool	initialize(void);

	int		run(void);
	//used for third party to stop this thread
	int		terminate(int code /* = 0 */);

	void final(void)
	{
		delete this;
	}

private:
	::ZQTianShan::NSS::NGODStreamImplPtr _NGODStreamObj;
	const ::TianShanIce::Streamer::AMD_Stream_commitPtr _amdStream;

};