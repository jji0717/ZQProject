#ifndef __ZQTianShan_SelectPort_H__
#define __ZQTianShan_SelectPort_H__

#include <queue>
#include "Locks.h"
#include <vector>

class SelectPort
{
public:
	SelectPort();
	~SelectPort(void);

	void setPortRange(std::vector<int>& portRange);
	int  getPort();
	int  getQueueSize() const;
private:
	static std::queue<int> _portQueue;
	static ZQ::common::Mutex _lockQueue;
};

#endif