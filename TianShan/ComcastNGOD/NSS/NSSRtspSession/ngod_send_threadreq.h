//file to define the thread
//inherit NativeThread
//polling each session groups socket to receive the RTSP message from NGOD server
//after receive the message, put into a shared buffer
#pragma once

#include "ngod_common_structure.h"

#define SENDBUFMAXSIZE 10240

class ngod_send_threadreq : public ZQ::common::ThreadRequest
{
public:
	ngod_send_threadreq(ZQ::common::FileLog *logfile, 
						ZQ::common::NativeThreadPool& Pool, 
						RTSPClientSession *ss,
						bool IsRelease);
	~ngod_send_threadreq();
	
protected:	
	int run(void);
	void final(int retcode, bool bCancelled)
	{
		delete this;
	}

private:
	RTSPClientSession	*m_pRTSPClientSession;
	ZQ::common::FileLog *m_pLogFile;
	bool _IsRelease;
	char hint[128];
};