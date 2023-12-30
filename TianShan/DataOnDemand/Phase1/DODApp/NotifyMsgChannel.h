#pragma once

#include <DODAppImpl.h>
#include <deque>
#include "MessageChannelImpl.h"

/*
namespace DataOnDemand {
	class MessageChannelImpl;
}
*/

typedef std::deque<DataOnDemand::MessageChannelImpl*> CDODCHANNLEQUEUE;

class CNotifyUpdateMsgChannel
{
public:
	CNotifyUpdateMsgChannel(int InterVal);
	~CNotifyUpdateMsgChannel();

	//update channel interval
    long m_nUpdateInterVal;

	CDODCHANNLEQUEUE m_DelChannelQueue;	
	HANDLE m_hUpdateEvent;

	CRITICAL_SECTION m_UpdateCriticalSection;
    //Add MsgChannel which need to Update
    void AddUpdateChannel(DataOnDemand::MessageChannelImpl* pChannelInfo);

private:
	// for command  to write file mode 
	HANDLE m_hUpdateThread;
};
