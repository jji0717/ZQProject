#ifndef __Sentry_HttpImpl_H__
#define __Sentry_HttpImpl_H__
#include "ZQ_common_conf.h"
#include "HttpEngine.h"
#include "httpdInterface.h"
#include <sstream>

// implement the HTTP interface using HttpEngine
class HttpResponse: public IHttpResponse
{
public:
    HttpResponse(ZQHttp::IResponse& resp);
    virtual ~HttpResponse();
public:
    virtual IHttpResponse& operator<< (const std::string& content){
        _buf << content;
        return (*this);
    }
    virtual IHttpResponse& operator<< (const char* content){
        if(NULL != content){
            _buf << content;
        }
        return (*this);
    }
    virtual IHttpResponse& operator<< (char content){
        _buf << content;
        return (*this);
    }
    virtual IHttpResponse& operator<< (int content){
        _buf << content;
        return (*this);
    }
    virtual IHttpResponse& operator<< (uint32 content){
        _buf << content;
        return (*this);
    }
    virtual IHttpResponse& operator<< (int64 content){
        _buf << content;
        return (*this);
    }
#ifdef ZQ_OS_MSWIN
	virtual IHttpResponse& operator<< (DWORD content){
		_buf << content;
		return (*this);
	}
#endif

    virtual void SetLastError(const char* errmsg){
        _lastError = (errmsg ? errmsg : "");
    }
    virtual const char* GetLastError(){
        return (_lastError.empty() ? NULL : _lastError.c_str());
    }
    virtual void setContentType(const char* ct)
    {
        _resp.setHeader("Content-Type", ct);
    }

    virtual void setHeader(const char* name, const char* value)
    {
        _resp.setHeader(name, value);
    }
    HttpStatusCodes& StatusCode(){
        return _statusCode;
    }
    void flush();
    void clear(){
        _buf.str("");
    }
private:
    void send();
private:
    ZQHttp::IResponse& _resp;
    std::ostringstream _buf;
    std::ostringstream _buf_stable;
    std::string     _lastError;
    HttpStatusCodes _statusCode;
};
//////////////////////////////////////////////////////////////////////////
namespace ZQTianShan{
    namespace Sentry{
        class SentryEnv;
    }
}

class HttpRequestCtx: public IHttpRequestCtx
{
public:
    HttpRequestCtx(const ZQTianShan::Sentry::SentryEnv& env, const ZQHttp::IRequest& req, ZQHttp::IResponse& _resp);
    virtual ~HttpRequestCtx() {  }

    void setRequestVar(const std::string& k, const std::string& v, bool partial = false)
    {
        if (partial)
            _vars[k].append(v);
        else
            _vars[k] = v;
    }

    void setRequestVars(const RequestVars& vars)
    {
        for(RequestVars::const_iterator it = vars.begin(); it != vars.end(); ++it)
            _vars[it->first] = it->second;
    }

public:

	virtual const char* GetRootDir() { return _rootDir.c_str(); }
	virtual const char* GetRootURL() { return _rootURL.c_str(); }
	virtual const char* GetUri()     { return _uri.c_str(); }

    virtual const char* GetRequestVar(const char* varname)
	{
        if(NULL == varname)
            return NULL;

        RequestVars::iterator itVar = _vars.find(varname);
        if(_vars.end() == itVar)
            return NULL;
        else
            return itVar->second.c_str();
    }

    virtual RequestVars GetRequestVars() { return _vars; }
    virtual IHttpResponse& Response()    { return _response; }
    virtual HTTPMETHOD GetMethodType()   { return _method; }
    HttpResponse& GetResponseObject()    { return _response; }

private:

    HttpResponse    _response;
    RequestVars     _vars;
    std::string     _rootDir;
    std::string     _rootURL;
    std::string     _uri;
    HTTPMETHOD      _method;
};

#endif
