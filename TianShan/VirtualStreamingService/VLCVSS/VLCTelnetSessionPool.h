#ifndef __VLCTelnetSessionPool_H__
#define __VLCTelnetSessionPool_H__

#include "NativeThread.h"
#include "FileLog.h"
#include "VLCTelnetSession.h"
#include "TelnetPool.h"

class TelnetClientMap
{
public:
	VLCTelnetSession *getClient(std::string &key)
	{
		ZQ::common::MutexGuard guard(_mutex);
		VLCTelnetSessionMap::iterator iter = _clientMap.find(key);
		if (iter == _clientMap.end())
			return NULL;
		else
			return iter->second;
	}
	bool addClient(std::string &key, VLCTelnetSession *soapClient)
	{
		if (soapClient == NULL)
			return false;
		if (key.empty())
			return false;
		ZQ::common::MutexGuard guard(_mutex);
		VLCTelnetSessionMap::iterator iter = _clientMap.find(key);
		if (iter == _clientMap.end())
		{
			_clientMap[key] = soapClient;
			return true;
		}
		else
			return false;
	}
	VLCTelnetSession* removeClient(std::string &key)
	{
		if (key.empty())
			return NULL;
		ZQ::common::MutexGuard guard(_mutex);
		VLCTelnetSessionMap::iterator iter = _clientMap.find(key);
		if (iter == _clientMap.end())
		{
			return NULL;
		}
		else
		{
			VLCTelnetSession *tmpSess = iter->second;
			_clientMap.erase(iter);
			return tmpSess;
		}
	}
	void removeAll()
	{
		ZQ::common::MutexGuard guard(_mutex);
		for (VLCTelnetSessionMap::iterator iter = _clientMap.begin(); iter != _clientMap.end(); iter++)
		{
			VLCTelnetSession *tmpSess = iter->second;
			delete tmpSess;
		}
		_clientMap.clear();
	}

private:
	VLCTelnetSessionMap _clientMap;
	ZQ::common::Mutex _mutex;
};

typedef enum
{
	VLCNEW = 1,
	VLCSETUP = 2,
	VLCSETUPDel = 3,
	VLCTEARDOWN = 4,
	VLCSHOW = 5,
	VLCCONTROL = 6

}VLCCommandType;

class VLCTelnetSessionPool
{
public:
	VLCTelnetSessionPool(::ZQ::common::FileLog &fileLog, std::string &strServerPath, uint16 &uServerPort, std::string &strPwd, uint16 &iTelnetPoolSize);
	~VLCTelnetSessionPool();

	bool addSession(const std::string &key, VLCTelnetSession *sess);
	VLCTelnetSession *findSession(const std::string &key);
	bool delSession(const std::string &key);
	::ZQ::common::Telnet *getActiveTelnet();
	void deActiveTelnet(::ZQ::common::Telnet *pTelnet);

	bool ControlVLC(VLCCommandType type, const std::string &key, CommonVLCPlaylist &pl, std::string &strCmd, std::string &strPos, std::string& strPLIdx, std::string &strRetMsg, int32 timeOut);
	bool ControlVLC(const char* strCmd, std::string &strRetMsg, int32 timeout = -1);

private:
	::ZQ::common::FileLog	&_fileLog;
	TelnetClientMap	_sessMap;
	TelnetPool _telnetPool;
};

#endif __VLCTelnetSessionPool_H__