#include "AckWindow.h"
#include "TimeConv.h"

AckWindow::AckWindow(ZQ::common::Log& log,const std::string& filepath)
		:_log(log)
		,_filepath(filepath)
{

}

AckWindow::~AckWindow()
{
	_ackWindow.clear();
}

void AckWindow::add(int64 pos,msgSendStatus msgStatus)
{
	ZQ::common::MutexGuard sync(_lockAckWindow);
	_ackWindow[pos] = msgStatus;
}

size_t AckWindow::getWindwoSize()
{
	ZQ::common::MutexGuard sync(_lockAckWindow);
	return _ackWindow.size();
}

void AckWindow::detectExpiredTime()
{
	ZQ::common::MutexGuard guard(_lockAckWindow);
	std::map<int64,msgSendStatus>::iterator it = _ackWindow.begin();
	while(it != _ackWindow.end())
	{
		if (it->second.expiredTime < ZQ::common::now())
		{
			_log(ZQ::common::Log::L_INFO, CLOGFMT(AckWindow, "detectExpiredTime() Messages more than expiration time.remove ack msg filepath[%s],pos[%llu]"),_filepath.c_str(),it->first);
		//	it = _ackWindow.erase(it);
			_ackWindow.erase(it++);
		}
		else
			it++;
	}
}


void AckWindow::updateStatus(int64 pos,const std::string& type,EventStatus status)
{
	ZQ::common::MutexGuard guard(_lockAckWindow);
	std::map<int64,msgSendStatus>::iterator it;
	it = _ackWindow.find(pos);
	if (it != _ackWindow.end())
	{
		msgSendStatus& sendStatus =  it->second;
		std::map<std::string,EventStatus>::iterator StatusIt = sendStatus.typeTostatus.find(type);
		if (StatusIt != sendStatus.typeTostatus.end())
		{
			sendStatus.typeTostatus[type] = status;
		}
		bool isAck = false;

		for (StatusIt = sendStatus.typeTostatus.begin();StatusIt != sendStatus.typeTostatus.end();StatusIt++)
		{
			if (StatusIt->second == event_pending)
			{
				isAck = true;
				_log(ZQ::common::Log::L_INFO, CLOGFMT(AckWindow, "send pending filepath[%s],pos[%llu],type[%s]"),_filepath.c_str(),pos,StatusIt->first.c_str());
			}
			else
			{
				_log(ZQ::common::Log::L_INFO, CLOGFMT(AckWindow, "recv ack filepath[%s],pos[%llu],type[%s]"),_filepath.c_str(),pos,StatusIt->first.c_str());
			}
		}
		if (!isAck)
		{
			_ackWindow.erase(it);
			_log(ZQ::common::Log::L_INFO, CLOGFMT(AckWindow, "updateStatus() remove ack msg filepath[%s],pos[%llu]"),_filepath.c_str(),pos);
		}
	}
}

void AckWindow::remove(int64 pos)
{
	ZQ::common::MutexGuard guard(_lockAckWindow);
	std::map<int64,msgSendStatus>::iterator it;
	it = _ackWindow.find(pos);
	if (it != _ackWindow.end())
	{
		_ackWindow.erase(it);
	}
}

AckWindowManager::AckWindowManager()
		:maxWindow(5)
		,_idleTime(7000)
		,_expiredTime(10000)
{
}

AckWindowManager::~AckWindowManager()
{
	std::map<std::string,AckWindow*>::iterator it = _windowMgr.begin();
	for(;it != _windowMgr.end();it++)
	{
		delete it->second;
	}
	_windowMgr.clear();
}

void AckWindowManager::addWindow(const std::string& filepath,AckWindow* window)
{
	ZQ::common::MutexGuard sync(_lockWindowMgr);
	_windowMgr[filepath] = window;

}
void AckWindowManager::removeWindow(const std::string& filepath)
{
	ZQ::common::MutexGuard sync(_lockWindowMgr);
	std::map<std::string,AckWindow*>::iterator it;
	it = _windowMgr.find(filepath);
	if (it != _windowMgr.end())
	{
		delete it->second;
		_windowMgr.erase(it);
	}
}

AckWindow* AckWindowManager::getAckWindow(const std::string& filepath)
{
	ZQ::common::MutexGuard sync(_lockWindowMgr);
	std::map<std::string,AckWindow*>::const_iterator it;
	it = _windowMgr.find(filepath);
	if (it != _windowMgr.end())
	{
		return it->second;
	}
	return NULL;
}

int64 AckWindowManager::getExpiredTime()
{
	return (ZQ::common::now() + _expiredTime);
}

bool AckWindowManager::detectWindow(const std::string& filepath)
{
	AckWindow* windowPtr = getAckWindow(filepath);
	windowPtr->detectExpiredTime();
	if (windowPtr->getWindwoSize() >= maxWindow)
	{
		return false;
	}
	else
		return true;
}

int64 AckWindowManager::resetIdleTime()
{
	return _idleTime;
}
