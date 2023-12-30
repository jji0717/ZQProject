//file to define the thread
//inherit NativeThread
//chop the RTSP message from NGOD server
//after chop, stored the message into a message list
#pragma once

#include "ngod_common_structure.h"

class ngod_chop_thread : public ZQ::common::NativeThread
{
public:
	ngod_chop_thread(ZQ::common::FileLog *logfile);
	ngod_chop_thread(ZQ::common::FileLog *logfile, NSSSessionGroupList &_NSSSessionGroupList);
	~ngod_chop_thread();

	void setNSSSessionGroupList(NSSSessionGroupList &_NSSSessionGroupList);

	int		run(void);

private:
	NSSSessionGroupList	*m_NSSSessionGroupList;
	ZQ::common::FileLog *m_pFileLog;
};