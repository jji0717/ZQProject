#include <WinSock2.h>
#include "VLCTelnetSessionPool.h"

VLCTelnetSessionPool::VLCTelnetSessionPool(::ZQ::common::FileLog &fileLog, std::string &strServerPath, uint16 &uServerPort, std::string &strPwd, uint16 &iTelnetPoolSize)
:_fileLog(fileLog),_telnetPool(strServerPath, uServerPort, strPwd, iTelnetPoolSize)
{
}

VLCTelnetSessionPool::~VLCTelnetSessionPool()
{
	
}

bool VLCTelnetSessionPool::addSession(const std::string &key, VLCTelnetSession *sess)
{
	return _sessMap.addClient(const_cast<std::string &>(key), sess);
}

VLCTelnetSession* VLCTelnetSessionPool::findSession(const std::string &key)
{
	//check session map
	VLCTelnetSession *localSess = _sessMap.getClient(const_cast<std::string &>(key));
	if (localSess != NULL)
	{
		_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSessionPool,"findSession() session(key:%s) exist"), key.c_str());
		localSess->_pTelnetClient = getActiveTelnet();
		if (localSess->_pTelnetClient == NULL)
		{
			_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(VLCTelnetSessionPool,"findSession() session(key:%s) exist but get telnet client session error"), key.c_str());
			return NULL;
		}
		return localSess;
	}
	else
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(VLCTelnetSessionPool,"findSession() session(key:%s) fail to find"), key.c_str());
		return NULL;
	}
}

bool VLCTelnetSessionPool::delSession(const std::string &key)
{
	VLCTelnetSession *localSess = _sessMap.removeClient(const_cast<std::string &>(key));
	if (localSess != NULL)
	{
		_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSessionPool,"delSession() session(key:%s) successfully removed"), key.c_str());
		delete localSess;
		return true;
	}
	else
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(VLCTelnetSessionPool,"delSession() session(key:%s) not exist and fail to removed"), key.c_str());
		return false;
	}
}

bool VLCTelnetSessionPool::ControlVLC(VLCCommandType type, const std::string &key, CommonVLCPlaylist &pl, std::string &strCmd, std::string &strPos, std::string& strPLIdx, std::string &strRetMsg, int32 timeOut)
{
	//get session first
	VLCTelnetSession *pSess = findSession(key);
	if (pSess == NULL)
		return false;

	bool b = false;
	switch(type)
	{
		case VLCNEW:
			b = pSess->newPL(pl, strRetMsg, timeOut);
			break;
		case VLCSETUP:
			b = pSess->setupPL(pl, strRetMsg, timeOut);
			break;
		case VLCSETUPDel:
			b = pSess->setupDelPL(pl, strRetMsg, timeOut);
			break;
		case VLCTEARDOWN:
			b = pSess->teardownPL(pl, strRetMsg, timeOut);
			break;
		case VLCSHOW:
			b = pSess->showPL(pl, strRetMsg, timeOut);
			break;
		case VLCCONTROL:
			b = pSess->controlPL(pl, strCmd, strPos, strPLIdx, strRetMsg, timeOut);
			break;
		default:
			b = false;
			break;
	}
	deActiveTelnet(pSess->_pTelnetClient);
	_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSessionPool,"ControlVLC() : retrun message is"));
	_fileLog.hexDump(::ZQ::common::Log::L_INFO, strRetMsg.c_str(), strRetMsg.length());
	return b;
}

bool VLCTelnetSessionPool::ControlVLC(const char* strCmd, std::string &strRetMsg, int32 timeout /* = -1 */)
{
	_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSessionPool,"ControlVLC() : command [%s]"), strCmd);
	ZQ::common::Telnet* telnetClient = getActiveTelnet();
	bool b = telnetClient->sendCMD(strCmd, strRetMsg, timeout);
	deActiveTelnet(telnetClient);

	_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSessionPool,"ControlVLC() : retrun message is"));
	_fileLog.hexDump(::ZQ::common::Log::L_INFO, strRetMsg.c_str(), strRetMsg.length());
	return b;
}

::ZQ::common::Telnet *VLCTelnetSessionPool::getActiveTelnet()
{
	return _telnetPool.getActiveTelnet();
}

void VLCTelnetSessionPool::deActiveTelnet(::ZQ::common::Telnet *pTelnet)
{
	 _telnetPool.deActiveTelnet(pTelnet);
}