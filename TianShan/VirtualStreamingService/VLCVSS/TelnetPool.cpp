#include "TelnetPool.h"

#define DEFAULT_TELNETTHRPOOL_SZ 10
#define MAX_TELNETTHRPOOL_SZ 50

TelnetPool::TelnetPool(std::string &strServerIp, uint16 &iServerPort, std::string &strPWD, uint16 &iSize)
:_telnetPoolSize(iSize),_activeSize(0),_serverIp(strServerIp),_serverPort(iServerPort),_pwd(strPWD)
{
	// allocate slave threads
	int sz = (iSize<1 ? DEFAULT_TELNETTHRPOOL_SZ : iSize);
	if (sz > MAX_TELNETTHRPOOL_SZ)
		sz = MAX_TELNETTHRPOOL_SZ;

	_telnetPoolSize = sz;
	for (int i = 0; i < sz; i++)
	{
		ZQ::common::InetAddress addr;
		addr.setAddress(strServerIp.c_str());
		::ZQ::common::Telnet *pTelnet = new ::ZQ::common::Telnet(addr, _serverPort);
		if (NULL == pTelnet)
			continue;
		else
		{
			pTelnet->setPWD(_pwd.c_str());
			pTelnet->connectToServer(0);
			pTelnet->_bUsed = false;
			_telnetList.push_back(pTelnet);
		}
	}
	idelIter = _telnetList.begin();
}

TelnetPool::~TelnetPool()
{
	for (idelIter= _telnetList.begin(); idelIter != _telnetList.end(); idelIter++)
	{
		delete *idelIter;
	}
	_telnetList.clear();
}

::ZQ::common::Telnet *TelnetPool::getActiveTelnet()
{
	//get current active telnet connection number
	while (1)
	{
		{
			::ZQ::common::MutexGuard guard(_mutex);
			if (_activeSize < _telnetPoolSize)
			{
				InterlockedIncrement(&_activeSize);
				break;
			}
			else
			{
				Sleep(10);
				continue;
			}
		}
	}

	for (TelnetList::iterator iter = _telnetList.begin(); iter != _telnetList.end(); iter++)
	{
		{
			::ZQ::common::MutexGuard guard((*iter)->_mutex);
			if (!(*iter)->_bUsed)
			{
				(*iter)->_bUsed = true;
			}
			return *iter;
		}
	}

	return NULL;
}

void TelnetPool::deActiveTelnet(::ZQ::common::Telnet *pTelnet)
{
	{
		::ZQ::common::MutexGuard guard(pTelnet->_mutex);
		pTelnet->_bUsed = false;
	}

	InterlockedDecrement(&_activeSize);
}
