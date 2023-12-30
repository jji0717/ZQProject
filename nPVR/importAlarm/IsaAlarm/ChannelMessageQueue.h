// ChannelMessageQueue.h: interface for the ChannelMessageQueue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHANNELMESSAGEQUEUE_H__4C9819CC_4529_4F94_9419_C1EC85E68AFA__INCLUDED_)
#define AFX_CHANNELMESSAGEQUEUE_H__4C9819CC_4529_4F94_9419_C1EC85E68AFA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <deque>
#include <vector>
#include <map>
#include "NativeThread.h"

class BaseMessageReceiver;

struct MESSAGE_FIELD
{
	std::string	key;
	std::string value;
};

typedef std::vector<struct MESSAGE_FIELD> MessageFields;

class ChannelMessageQueue : public ZQ::common::NativeThread
{
public:
	ChannelMessageQueue();
	virtual ~ChannelMessageQueue();

	bool init();

	void close();

	void addMessageReceiver(BaseMessageReceiver* pReceiver);

	//this method should be called after all the addMessageReceiver called
	void getChannelRequireFields(int channelID, std::vector<std::string>& fields);

	//called by Message handler
	void addChannelMessage(int channelID, MessageFields* pMessage);

protected:

	int run();

	typedef std::map<int, std::vector<BaseMessageReceiver*>*> ChannelReceiverMap;
	ChannelReceiverMap				_channelReceiverMap;
	struct CHANNEL_MESSAGE
	{
		int		channelID;
		int		messageID;
		MessageFields*	fileds;
	};

	CRITICAL_SECTION	_opLock;

	int					_nMessageID;

	HANDLE				_hExit;
	HANDLE				_hMessageSem;
	std::deque<struct CHANNEL_MESSAGE>		_messages;
};

#endif // !defined(AFX_CHANNELMESSAGEQUEUE_H__4C9819CC_4529_4F94_9419_C1EC85E68AFA__INCLUDED_)
