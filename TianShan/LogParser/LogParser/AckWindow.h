#ifndef __ACK_WINDOW_H__
#define __ACK_WINDOW_H__

#include "Pointer.h"
#include <Locks.h>
#include <map>

enum EventStatus
{
	event_pending,
	event_ack,
};

class AckWindow
{
public:

	AckWindow(ZQ::common::Log& log,const std::string& filepath);
	~AckWindow();
	typedef struct sendStatus
	{
		int64 expiredTime;
		std::map<std::string,EventStatus> typeTostatus;
	}msgSendStatus;
	void add(int64 pos,msgSendStatus msgStatus);
	void updateStatus(int64 pos,const std::string& type,EventStatus status);
	void remove(int64 pos);
	size_t getWindwoSize();
	void detectExpiredTime();
private:
	std::map<int64,msgSendStatus> _ackWindow;//map position to sendStatus
	ZQ::common::Mutex _lockAckWindow;
	ZQ::common::Log& _log;
	std::string _filepath;
};

class AckWindowManager
{
public:
	AckWindowManager();
	~AckWindowManager();
	void addWindow(const std::string& filepath,AckWindow* window);
	void removeWindow(const std::string& filepath);
	AckWindow* getAckWindow(const std::string& filepath);
	bool detectWindow(const std::string& filepath);
	int64 getExpiredTime();
	int64 resetIdleTime();

private:
	std::map<std::string,AckWindow*> _windowMgr;//map filepath to AckWindow
	ZQ::common::Mutex _lockWindowMgr;
	int64 _expiredTime;
	size_t maxWindow;
	int64 _idleTime;
};

#endif

