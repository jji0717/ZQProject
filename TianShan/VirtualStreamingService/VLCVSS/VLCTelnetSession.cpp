//include winsock to avoid multi-definition error
#include <WinSock2.h>

//include class header
#include "VLCTelnetSession.h"

VLCTelnetSession::VLCTelnetSession(::ZQ::common::FileLog &fileLog, std::string &key, ::ZQ::common::Telnet &telnetClient, std::string strDestIp, int32 iDestPort)
:_fileLog(&fileLog)
,_strKey(key)
,_pTelnetClient(&telnetClient)
,_telnetParser(&fileLog)
,_strDestIp(strDestIp)
,_iDestPort(iDestPort)
{
	//set url
	_bDestroy = false;
}

VLCTelnetSession::VLCTelnetSession(::ZQ::common::FileLog &fileLog, std::string &key, std::string strDestIp, int32 iDestPort)
:_fileLog(&fileLog)
,_strKey(key)
,_pTelnetClient(NULL)
,_telnetParser(&fileLog)
,_strDestIp(strDestIp)
,_iDestPort(iDestPort)
{
	//set url
	_bDestroy = false;
}


VLCTelnetSession::~VLCTelnetSession()
{

}

bool VLCTelnetSession::newPL(CommonVLCPlaylist &pl, std::string &strRetMsg, int timeOut)
{
	CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "new playlist(%s)"), pl._name.c_str());
	
	VLCProps vlcProps;
	VLCProp vlcProp;
	vlcProp.prop = SetProps(strVLCInput, pl);
	vlcProps.push_back(vlcProp);

	vlcProp.prop = SetProps(strVLCOutput, pl);
	vlcProps.push_back(vlcProp);

	std::string strCmd = NewPlayList(pl._name, pl._type, vlcProps);

	if (strCmd.empty())
	{
		CLIENTLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCTelnetSession, "new playlist(%s) failed"), pl._name.c_str());
		return false;
	}
	else
	{
		CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "new playlist command[%s]"), strCmd.c_str());
		bool b = _pTelnetClient->sendCMD(strCmd.c_str(), strRetMsg, timeOut);
		if (!b)
		{
			CLIENTLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCTelnetSession, "new playlist(%s) failed: telnet error"), pl._name.c_str());
		}
		else
		{
			CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "new playlist(%s) success"), pl._name.c_str());
			b = _telnetParser.checkError(strRetMsg);
		}
		return b;
	}
}

bool VLCTelnetSession::setupPL(CommonVLCPlaylist &pl, std::string &strRetMsg, int timeOut)
{
	CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "setup playlist(%s)"), pl._name.c_str());

	VLCProps vlcProps;
	VLCProp vlcProp;
	vlcProp.prop = SetProps(strVLCInput, pl);
	vlcProps.push_back(vlcProp);

	vlcProp.prop = SetProps(strVLCOutput, pl);
	vlcProps.push_back(vlcProp);

	std::string strCmd = SetupPlayList(pl._name, vlcProps);
	if (strCmd.empty())
	{
		CLIENTLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCTelnetSession, "setup playlist(%s) failed"), pl._name.c_str());
		return false;
	}
	else
	{
		CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "setup playlist command[%s]"), strCmd.c_str());
		bool b = _pTelnetClient->sendCMD(strCmd.c_str(), strRetMsg, timeOut);
		if (!b)
		{
			CLIENTLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCTelnetSession, "setup playlist(%s) failed: telnet error"), pl._name.c_str());
		}
		else
		{
			CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "setup playlist(%s) success"), pl._name.c_str());
			b = _telnetParser.checkError(strRetMsg);
		}
		return b;
	}
}

bool VLCTelnetSession::setupDelPL(CommonVLCPlaylist &pl, std::string &strRetMsg, int timeOut)
{
	CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "setupdel playlist(%s)"), pl._name.c_str());

	VLCProps vlcProps;
	VLCProp vlcProp;
	vlcProp.prop = SetProps(strVLCInputdel, pl);
	vlcProps.push_back(vlcProp);

	std::string strCmd = SetupPlayList(pl._name, vlcProps);
	if (strCmd.empty())
	{
		CLIENTLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCTelnetSession, "setupdel playlist(%s) failed"), pl._name.c_str());
		return false;
	}
	else
	{
		CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "setupDel playlist command[%s]"), strCmd.c_str());
		bool b = _pTelnetClient->sendCMD(strCmd.c_str(), strRetMsg, timeOut);
		if (!b)
		{
			CLIENTLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCTelnetSession, "setupdel playlist(%s) failed: telnet error"), pl._name.c_str());
		}
		else
		{
			CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "setupdel playlist(%s) success"), pl._name.c_str());
			b = _telnetParser.checkError(strRetMsg);
		}
		return b;
	}
}

bool VLCTelnetSession::teardownPL(CommonVLCPlaylist &pl, std::string &strRetMsg, int timeOut)
{
	CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "teardown playlist(%s)"), pl._name.c_str());
	std::string strCmd = DelPlayList(pl._name);
	if (strCmd.empty())
	{
		CLIENTLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCTelnetSession, "teardown playlist(%s) failed"), pl._name.c_str());
		return false;
	}
	else
	{
		CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "teardown playlist command[%s]"), strCmd.c_str());
		bool b = _pTelnetClient->sendCMD(strCmd.c_str(), strRetMsg, timeOut);
		if (!b)
		{
			CLIENTLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCTelnetSession, "teardown playlist(%s) failed: telnet error(%s)"), pl._name.c_str(), _pTelnetClient->getLastErr().c_str());
		}
		else
		{
			CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "teardown playlist(%s) success"), pl._name.c_str());
			b = _telnetParser.checkError(strRetMsg);
		}
		return b;
	}
}

bool VLCTelnetSession::showPL(CommonVLCPlaylist &pl, std::string &strRetMsg, int timeOut)
{
	CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "show playlist(%s)"), pl._name.c_str());
	std::string strCmd = ShowPlayList(pl._name);
	if (strCmd.empty())
	{
		CLIENTLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCTelnetSession, "show playlist(%s) failed"), pl._name.c_str());
		return false;
	}
	else
	{
		CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "show playlist command[%s]"), strCmd.c_str());
		bool b = _pTelnetClient->sendCMD(strCmd.c_str(), strRetMsg, timeOut);
		if (!b)
		{
			CLIENTLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCTelnetSession, "show playlist(%s) failed: telnet error(%s)"), pl._name.c_str(), _pTelnetClient->getLastErr().c_str());
		}
		else
		{
			CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "show playlist(%s) success"), pl._name.c_str());
			b = _telnetParser.checkError(strRetMsg);
		}
		return b;
	}
}

bool VLCTelnetSession::controlPL(CommonVLCPlaylist &pl, std::string &strCmd, std::string &strPos, std::string& strPLIdx, std::string &strRetMsg, int timeOut)
{
	CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "control(%s) playlist(%s)"), strCmd.c_str(), pl._name.c_str());

	std::string strCtrlCmd = strCmd;
	if (!strPLIdx.empty())
		strCtrlCmd += strBlank + strPLIdx;
	if (!strPos.empty())
		strCtrlCmd += strBlank + strPos;

	std::string strSendCMD = ControlPlayList(pl._name, strCtrlCmd);
	if (strSendCMD.empty())
	{
		CLIENTLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCTelnetSession, "control(%s) playlist(%s) failed"), strCmd.c_str(), pl._name.c_str());
		return false;
	}
	else
	{
		CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "control playlist command[%s]"), strSendCMD.c_str());
		bool b = _pTelnetClient->sendCMD(strSendCMD.c_str(), strRetMsg, timeOut);
		if (!b)
		{
			CLIENTLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCTelnetSession, "control(%s) playlist(%s) failed: telnet error"), strCmd.c_str(), pl._name.c_str());
		}
		else
		{
			CLIENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(VLCTelnetSession, "control(%s) playlist(%s) success"), strCmd.c_str(), pl._name.c_str());
			b = _telnetParser.checkError(strRetMsg);
		}
		return b;
	}
}