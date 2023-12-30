#ifndef __ZQTianShan_TMVSoapClientSession_H__
#define __ZQTianShan_TMVSoapClientSession_H__

//include zqcommonstlp header
#include "urlstr.h"
#include "FileLog.h"
#include "NativeThread.h"
#include "Locks.h"

//include gsoap header
#include "soapTMVSSProxy.h"

class TMVSoapClientSession
{
public:
	TMVSoapClientSession(::ZQ::common::FileLog &fileLog, const char* soapServerAddr, const int& soapServerPort);
	virtual ~TMVSoapClientSession();

	bool soapSetup(ZQ2__setupInfo *setupInfo, struct ZQ2__setupResponse &setupResInfo);
	bool soapTeardown(std::string sessionId, int &ret);
	bool soapGetStatus(std::string sessionId, struct ZQ2__getStatusResponse &getStatusResInfo);

	std::string strStreamName;
	std::string strOndemandSessionId;
	bool _bDestroy;

private:
	TMVSSProxy _tmvssSoapClient;
	::ZQ::common::URLStr	_urlStr;				//class to parse the _strEndPoint
	//::ZQ::common::URLStr	_urlStrNotify;			//class to parse the _strEndPoint
	::ZQ::common::FileLog	*_fileLog;
};

typedef ::std::map<::std::string, TMVSoapClientSession *> TMVSoapClientSessionMap;
//typedef ::std::map<::std::string, TMVSSProxy *> TMVSoapClientSessionMap;
class SoapClientMap
{
public:
	TMVSoapClientSession *getSoapClient(std::string &key)
	//TMVSSProxy *getSoapClient(std::string &key)
	{
		ZQ::common::MutexGuard guard(_mutex);
		TMVSoapClientSessionMap::iterator iter = _soapClientMap.find(key);
		if (iter == _soapClientMap.end())
			return NULL;
		else
			return iter->second;
	}
	bool addSoapClient(std::string &key, TMVSoapClientSession *soapClient)
	//bool addSoapClient(std::string &key, TMVSSProxy *soapClient)
	{
		if (soapClient == NULL)
			return false;
		if (key.empty())
			return false;
		ZQ::common::MutexGuard guard(_mutex);
		TMVSoapClientSessionMap::iterator iter = _soapClientMap.find(key);
		if (iter == _soapClientMap.end())
		{
			_soapClientMap[key] = soapClient;
			return true;
		}
		else
			return false;
	}
	TMVSoapClientSession* removeSoapClient(std::string &key)
	{
		if (key.empty())
			return NULL;
		ZQ::common::MutexGuard guard(_mutex);
		TMVSoapClientSessionMap::iterator iter = _soapClientMap.find(key);
		if (iter == _soapClientMap.end())
		{
			return NULL;
		}
		else
		{
			TMVSoapClientSession *tmpSess = iter->second;
			_soapClientMap.erase(iter);
			return tmpSess;
		}
	}
	TMVSoapClientSessionMap::iterator find(std::string &key)
	{
		ZQ::common::MutexGuard guard(_mutex);
		return _soapClientMap.find(key);
	}
private:
	TMVSoapClientSessionMap _soapClientMap;
	ZQ::common::Mutex _mutex;
};

#define SOAPLOG if (_fileLog) (*_fileLog)
#endif __ZQTianShan_TMVSoapClientSession_H__