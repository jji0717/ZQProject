#ifndef __ZQTianShan_Plugin_SsmRichURL_Environment_H__
#define __ZQTianShan_Plugin_SsmRichURL_Environment_H__

// $(ZQProjsPath)/tianshan/streamsmith
#include "StreamSmithModule.h"
#include "RtspRelevant.h"

// $(ZQProjsPath)/Common
#include "FileLog.h"
#include "urlstr.h"
#include "XMLPreferenceEx.h"

#include "./RequestHandler.h"
#include "RichurlConfig.h"

namespace ZQTianShan
{
namespace Plugin
{
namespace SsmRichURL
{

#define DebugLevel			ZQ::common::Log::L_DEBUG
#define InfoLevel			ZQ::common::Log::L_INFO
#define NoticeLevel			ZQ::common::Log::L_NOTICE
#define WarningLevel		ZQ::common::Log::L_WARNING
#define ErrorLevel			ZQ::common::Log::L_ERROR

struct Config
{
	std::string logName;
	int logSize;
	int logLevel;
	int logNums;
	std::string userAgent;
};

class Environment
{
public: 
	Environment();
	virtual ~Environment();
	bool    doInit(IStreamSmithSite* pSite, const char* pDir);
	void    doUninit(IStreamSmithSite* pSite);
	RequestProcessResult doFixupRequest(IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	bool    loadConfig(const char* pDir);

	ZQ::common::FileLog& getLog(void){return _fileLog;};

private: 
	ZQ::common::SysLog                    _sysLog;
	ZQ::common::FileLog                   _fileLog;
	ZQ::common::XMLPreferenceDocumentEx   _xmlDoc;

public: 
	ZQ::common::Config::Loader< RichurlCfg >  _config;
};

class SmartPreference
{
public: 
	SmartPreference(ZQ::common::XMLPreferenceEx*& p);
	virtual ~SmartPreference();
	// return the name property's int value
	// name[in], the property name
	// value[out], stores the property's int value
	// if name property not found, return false, etherwise return true;
	bool getIntProp(const char* name, int& value);

	// return the name property's string value
	// name[in], the property name
	// buff[out], the buffer to receive the property value
	// buffSize[in], the buffer's size
	// if name property not found, return false, etherwise return true;
	bool getStrProp(const char* name, char* buff, const int buffSize);

protected: 
	ZQ::common::XMLPreferenceEx*& _p;
};

}
}
}
#endif // __ZQTianShan_Plugin_SsmRichURL_Environment_H__

