#ifndef __SERVICE_GROUP_PUMP_H__
#define __SERVICE_GROUP_PUMP_H__

#include <Locks.h>
#include <vector>
#include <string>
#include "SystemUtils.h"
#include "NativeThread.h"
#include "UDPSocket.h"

namespace ZQMODApplication
{

class ServiceGroupPump:public ZQ::common::NativeThread
{
public:
	ServiceGroupPump(const std::string& ip,int port, const std::string& sgpumpformat,int64 timeout = 5000,int64 interval = 2000);
	~ServiceGroupPump();
	void add(const std::string& sg, const std::string&  smartCardID);
	virtual int run();

private:
	std::string fixup(const std::string& sg, const std::string&  smartCardID);
	bool _Quit;
	int64 _timeout;
	int64 _interval;
	std::string _sgpumpFormat;
	ZQ::common::UDPSocket* m_udpSocket;
	std::vector<std::string> m_vAsset;
	ZQ::common::Mutex _lockAsset;
	struct sockaddr_in peer_addr;
	SYS::SingleObject _WakeUp;
};
}
#endif
