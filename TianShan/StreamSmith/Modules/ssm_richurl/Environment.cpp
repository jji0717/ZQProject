#include "./Environment.h"
#include "./ZQResource.h"

namespace ZQTianShan
{
namespace Plugin
{
namespace SsmRichURL
{
	
Environment::Environment()
:_config("")
{

}

Environment::~Environment()
{
}

bool Environment::loadConfig(const char* pDir)
{
	if (!pDir || !strlen(pDir))
	{
		_sysLog(ErrorLevel, "richurl loadConfig failed, config file path is empty");
		return false;
	}

	if(!_config.load(pDir))
	{
	    _sysLog(ErrorLevel, "load file[%s] failed", pDir);
		return false;
	};

	if (_config._logFile._fileName.empty())
	{
		_sysLog(ErrorLevel, "FileLog.name is empty");
		return false;
	}

	if (_config._userAgent._keyword.empty())
	{
		_sysLog(ErrorLevel, "UserAgent.keyword is empty");
		return false;
	}

	return true;
}

bool Environment::doInit(IStreamSmithSite* pSite, const char* pDir)
{
	std::string strConfigPath = "";
	if( pDir )
		strConfigPath = pDir;
	if( strConfigPath.length() > 0 && 
		( strConfigPath.at(strConfigPath.length()-1) != '\\' || strConfigPath.at(strConfigPath.length()-1) != '/' )		)
		strConfigPath = strConfigPath + FNSEPS;

	strConfigPath += "ssm_richurl.xml";

	_sysLog.open(ZQ_PRODUCT_NAME, ZQ::common::Log::L_WARNING);
	if (!loadConfig(strConfigPath.c_str()))
	{
		_sysLog(ErrorLevel, "loadConfig() failed");
		return false;
	}
	try
	{
		_fileLog.open(_config._logFile._fileName.c_str(), _config._logFile._fileLogLevel, _config._logFile._fileNumber, _config._logFile._fileSize);
	}
	catch (...)
	{
		_sysLog(ErrorLevel, "create log file failed");
		return false;
	}

	_fileLog(InfoLevel, "Environment::doInit() succeed, log file[%s]", _config._logFile._fileName.c_str());
	return true;
}

void Environment::doUninit(IStreamSmithSite* pSite)
{
}

RequestProcessResult Environment::doFixupRequest(IStreamSmithSite* pSite, 
												 IClientRequestWriter* pReq)
{
	_fileLog(DebugLevel, CLOGFMT(Environment, "doFixupRequest() entry"));
	// validata input param
	if (NULL == pSite)
	{
		_fileLog(ErrorLevel, CLOGFMT(Environment, "site is null"));
		return RequestError;
	}
	
	if (NULL == pReq)
	{
		_fileLog(ErrorLevel, CLOGFMT(Environment, "request is null"));
		return RequestError;
	}
	
	IServerResponse* pResponse = pReq->getResponse();
	if (NULL == pResponse)
	{
		_fileLog(ErrorLevel, CLOGFMT(Environment, "response is null"));
		return RequestError;
	}
	
	switch (pReq->getVerb())
	{
	case RTSP_MTHD_SETUP: 
		{
			FixupSetup fixSetup(*this, pSite, pReq, pResponse);
			try
			{
			if (!fixSetup.process())
			{
				_fileLog(ErrorLevel, "fixup setup request failed");
				return RequestError;
			}
			}
			catch (...)
			{
				_fileLog(ErrorLevel, "fixup setup request caught unexpect exception");
				return RequestError;
			}
			break;
		}
	case RTSP_MTHD_DESCRIBE: 
		{
			FixupDescribe fixDescribe(*this, pSite, pReq, pResponse);
			try
			{
			if (!fixDescribe.process())
			{
				_fileLog(ErrorLevel, "fixup Describe request failed");
				return RequestError;
			}
			}
			catch (...)
			{
				_fileLog(ErrorLevel, "fixup Describe request caught unexpect exception");
				return RequestError;
			}
			break;
		}
	default: 
		break;
	}

	_fileLog(DebugLevel, "Environment::doFixupRequest() succeed");
	return RequestProcessed;
}

SmartPreference::SmartPreference(ZQ::common::XMLPreferenceEx*& p) : _p(p)
{
}

SmartPreference::~SmartPreference()
{
	if (NULL != _p)
		_p->free();
	_p = NULL;
}

bool SmartPreference::getIntProp(const char* name, int& value)
{
	bool bRet = _p->getIntProp(name, value);
	char szBuf[1024];
	memset(szBuf, 0, sizeof(szBuf));
	_p->getPreferenceName(szBuf);
	return bRet;
}

bool SmartPreference::getStrProp(const char* name, char* buff, const int buffSize)
{
	bool bRet = _p->getStrProp(name, buff, buffSize);
	char szBuf[1024];
	memset(szBuf, 0, sizeof(szBuf));
	_p->getPreferenceName(szBuf);
	return bRet;
}

}
}
}
