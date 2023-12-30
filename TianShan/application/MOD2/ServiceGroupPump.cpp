#include "ServiceGroupPump.h"
#include "InetAddr.h"

namespace ZQMODApplication
{

ServiceGroupPump::ServiceGroupPump(const std::string& ip,int port,const std::string& sgpumpformat,int64 timeout,int64 interval)
		:m_udpSocket(NULL)
		,_sgpumpFormat(sgpumpformat)
		,_timeout(timeout)
		,_interval(interval)
		,_Quit(false)
{
	m_udpSocket = new ZQ::common::UDPSocket();
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ServiceGroupPump, "ip = %s ,port = %d."),ip.c_str(),port);
	m_udpSocket->setPeer((ZQ::common::InetAddress)ip.c_str(),port);

	if (timeout <= 0)
		_timeout = 5000;
	if(interval <=0 )
		_interval = 2000;
}

ServiceGroupPump::~ServiceGroupPump()
{
	_Quit = true;
	_WakeUp.signal();
	if (m_udpSocket != NULL)
	{
		delete m_udpSocket;
		m_udpSocket = NULL;
	}
}

std::string ServiceGroupPump::fixup(const std::string& sg, const std::string&  smartCardID)
{
	std::string senddata = _sgpumpFormat;

	int npos = senddata.find("${smartcard-id}");
	if(npos >= 0)
	{
		senddata.replace(npos, strlen("${smartcard-id}"), smartCardID);
	}
	npos = senddata.find("${node-group-id}");
	if(npos >= 0)
	{
		senddata.replace(npos, strlen("${node-group-id}"), sg);
	}
	return senddata;
}


void ServiceGroupPump::add(const std::string& sg, const std::string&  smartCardID)
{
	std::string assetInfo;
	assetInfo = fixup(sg,smartCardID);
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ServiceGroupPump, "add assetInfo = %s "),assetInfo.c_str());
	ZQ::common::MutexGuard guard(_lockAsset);
	m_vAsset.push_back(assetInfo);
	_WakeUp.signal();
}

int ServiceGroupPump::run()
{
	while(!_Quit)
	{
		SYS::SingleObject::STATE state = _WakeUp.wait(_interval);
		if (_Quit)
			break;
		if(SYS::SingleObject::SIGNALED == state || SYS::SingleObject::TIMEDOUT == state)
		{ 
			ZQ::common::MutexGuard guard(_lockAsset);
			for(std::vector<std::string>::iterator it = m_vAsset.begin(); it != m_vAsset.end();)
			{
				std::string pdata = *it;
					glog(ZQ::common::Log::L_INFO, CLOGFMT(ServiceGroupPump, "run() send assetInfo = %s "),pdata.c_str());
				int ret = m_udpSocket->send(pdata.c_str(),pdata.size());
				if (ret > 0)
					it = m_vAsset.erase(it);
				else
					it++;
			}
		}
	}
	return 0;
}
}