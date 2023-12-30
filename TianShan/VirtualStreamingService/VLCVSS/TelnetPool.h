#ifndef __ZQ_TelnetPool_H__
#define __ZQ_TelnetPool_H__

//include ZQ common headers
#include "Locks.h"

//include telnet header
#include "Telnet.h"

//include std header
#include <list>

class TelnetPool : public ::ZQ::common::Mutex
{
typedef std::list<::ZQ::common::Telnet *> TelnetList;

public:
	TelnetPool(std::string &strServerIp, uint16 &iServerPort, std::string &strPWD, uint16 &iSize);
	virtual ~TelnetPool();

	::ZQ::common::Telnet *getActiveTelnet();
	void deActiveTelnet(::ZQ::common::Telnet *pTelnet);

private:
	TelnetList _telnetList;
	TelnetList::iterator idelIter;
	volatile long _telnetPoolSize;
	volatile long _activeSize;
	std::string _serverIp;
	int16 _serverPort;
	std::string _pwd;
	::ZQ::common::Mutex _mutex;
};

#endif __ZQ_TelnetPool_H__