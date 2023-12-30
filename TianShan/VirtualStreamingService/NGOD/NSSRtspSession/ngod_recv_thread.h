//file to define the thread
//inherit NativeThread
//polling each session groups socket to receive the RTSP message from NGOD server
//after receive the message, put into a shared buffer
#pragma once

#include "ngod_common_structure.h"

class ngod_recv_thread : public ZQ::common::NativeThread
{
public:
	ngod_recv_thread(ZQ::common::FileLog *logfile);
	ngod_recv_thread(ZQ::common::FileLog *logfile, 
					NSSSessionGroupList &_NSSSessionGroupList);
	~ngod_recv_thread();

	void setNSSSessionGroupList(NSSSessionGroupList &_NSSSessionGroupList);

	int		run(void);

private:
	NSSSessionGroupList	*m_NSSSessionGroupList;
	ZQ::common::FileLog *m_pLogFile;
};
