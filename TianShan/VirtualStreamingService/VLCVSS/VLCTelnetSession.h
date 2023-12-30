#ifndef __ZQTianShan_VLCTelnetSession_H__
#define __ZQTianShan_VLCTelnetSession_H__

//include zqcommonstlp header
#include "urlstr.h"
#include "FileLog.h"
#include "NativeThread.h"
#include "Locks.h"

//include telnet header
#include "Telnet.h"
#include "VLCCmd.h"

class VLCTelnetSession
{
public:
	VLCTelnetSession(::ZQ::common::FileLog &fileLog, std::string &key, ::ZQ::common::Telnet &telnetClient, std::string strDestIp, int32 iDestPort);
	VLCTelnetSession(::ZQ::common::FileLog &fileLog, std::string &key, std::string strDestIp, int32 iDestPort);
	virtual ~VLCTelnetSession();

	bool newPL(CommonVLCPlaylist &pl, std::string &strRetMsg, int timeOut);
	bool setupPL(CommonVLCPlaylist &pl, std::string &strRetMsg, int timeOut);
	bool setupDelPL(CommonVLCPlaylist &pl, std::string &strRetMsg, int timeOut);
	bool teardownPL(CommonVLCPlaylist &pl, std::string &strRetMsg, int timeOut);
	bool showPL(CommonVLCPlaylist &pl, std::string &strRetMsg, int timeOut);
	bool controlPL(CommonVLCPlaylist &pl, std::string &strCmd, std::string &strPos, std::string& strPLIdx, std::string &strRetMsg, int timeOut);

	std::string _strStreamName;
	std::string _strKey;
	bool _bDestroy;
	std::string _strDestIp;
	int32		_iDestPort;
	::ZQ::common::Telnet *_pTelnetClient;

private:
	::ZQ::common::TelnetParser	_telnetParser;
	::ZQ::common::URLStr	_urlStr;				//class to parse the _strEndPoint
	::ZQ::common::FileLog	*_fileLog;
};

typedef ::std::map<::std::string, VLCTelnetSession *> VLCTelnetSessionMap;

#define CLIENTLOG if (_fileLog) (*_fileLog)
#endif __ZQTianShan_VLCTelnetSession_H__