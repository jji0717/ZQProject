#include "SelectPort.h"
#include "Locks.h"

std::queue<int> SelectPort::_portQueue;
ZQ::common::Mutex SelectPort::_lockQueue;

SelectPort::SelectPort(void)
{	
}

SelectPort::~SelectPort(void)
{
}

void SelectPort::setPortRange( std::vector<int>& portRange)
{
	std::vector<int>::iterator iter;

	ZQ::common::MutexGuard gd(_lockQueue);
	for (iter = portRange.begin();iter != portRange.end();iter++)
	{
		_portQueue.push(*iter);
	}
}

int SelectPort::getPort()
{
	ZQ::common::MutexGuard gd(_lockQueue);
	if( _portQueue.empty() ) {
		return (0);
	}

	int port  = _portQueue.front();
	_portQueue.pop();
	_portQueue.push(port);
	return port;
}

int SelectPort::getQueueSize() const {
	ZQ::common::MutexGuard gd(_lockQueue);
	return _portQueue.size();
}
