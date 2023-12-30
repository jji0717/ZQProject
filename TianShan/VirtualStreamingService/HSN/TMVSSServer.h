#ifndef __TMVSSSERVER_H__
#define __TMVSSSERVER_H__

#include "soapTMVSSService.h"
#include "NativeThread.h"
#include "urlstr.h"
#include "FileLog.h"
#include "TMVSoapClientSession.h"

class TMVSSServer : public TMVSSService,
					public ::ZQ::common::NativeThread
{
public:
	TMVSSServer(const char *localEndPoint, ::ZQ::common::FileLog &fileLog):_strLocalEndPoint(localEndPoint),_fileLog(fileLog){};
	~TMVSSServer(){};

	//function derive from ::ZQ::common::NativeThread
	int run(void);

	/// Web service operation 'setup' (returns error code or SOAP_OK)
	int setup(ZQ2__setupInfo *setupInfo, struct ZQ2__setupResponse &_param_1);

	/// Web service operation 'teardown' (returns error code or SOAP_OK)
	int teardown(std::string sessionId, int &ret);

	/// Web service operation 'getStatus' (returns error code or SOAP_OK)
	int getStatus(std::string sessionId, struct ZQ2__getStatusResponse &_param_2);

	/// Web service operation 'notifyStatus' (returns error code or SOAP_OK)
	int notifyStatus(ZQ2__notifyStatusInfo *notifyStatusInfo, int &ret);

	//soap client map, use ident name as key
	SoapClientMap _soapClientMap;

private:
	::std::string			_strLocalEndPoint;		//local soap service access endpoint
	::ZQ::common::URLStr	_urlStr;				//class to parse the _strEndPoint
	bool _bRun;
	::ZQ::common::FileLog	&_fileLog;
};

#endif __TMVSSSERVER_H__