#ifndef __ZQTianShan_Plugin_SsmRichURL_RequestHandler_H__
#define __ZQTianShan_Plugin_SsmRichURL_RequestHandler_H__

#include "StreamSmithModule.h"

namespace ZQTianShan
{
namespace Plugin
{
namespace SsmRichURL
{

class Environment;

class FixupHandle
{
public: 
	FixupHandle(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
	virtual ~FixupHandle();
	bool needProcess();

protected: 
	std::string getUrl();
	std::string getResponseHeader(const char* pHeaderStr);
	void setResponseHeader(const char* pHeaderStr, const char* strValue);
	std::string getRequestHeader(const char* pHeaderStr);

protected: 
	Environment&								_env;
	ZQ::common::Log&                            _logFromEnv;
	char										_szBuf[2048];
	IClientRequestWriter*						_pRequestWriter;
	IStreamSmithSite*							_pSite; // site object
	IServerResponse*							_pResponse; // response object
	IClientRequest*								_pRequest; // request object

}; // class FixupHandle

class FixupSetup : public FixupHandle
{
public: 
	FixupSetup(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
	virtual ~FixupSetup();
	virtual bool process();

}; // class FixupSetup

class FixupDescribe : public FixupHandle
{
public: 
	FixupDescribe(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
	virtual ~FixupDescribe();
	virtual bool process();

}; // class FixupDescribe

}
}
}

#endif // #define __ZQTianShan_Plugin_SsmRichURL_RequestHandler_H__

