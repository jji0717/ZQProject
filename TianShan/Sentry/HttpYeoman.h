// HttpYeoman.h: interface for the HttpYeoman class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTTPYEOMAN_H__CE7EA374_A2F2_46EC_BF01_36C2AA5724F1__INCLUDED_)
#define AFX_HTTPYEOMAN_H__CE7EA374_A2F2_46EC_BF01_36C2AA5724F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable:4786)
#include <zq_common_conf.h>
#include <string>
#include <map>
#include <strHelper.h>
#include <urlstr.h>

#include "HttpDlg.h"
#include "httpdInterface.h"

typedef std::map<std::string, std::string> Fields;

class HttpYeoman  : public HttpDlg
{
    friend class HttpRequestCtx;
public:
	HttpYeoman(ZQTianShan::Sentry::SentryEnv& env);
	virtual ~HttpYeoman();

public:
	///process http request	
	virtual bool		ProcessHttpReqeust(  );

	int					buildResponseHeader(char* buffer , int bufLen ,int statusCode , __int64 contentLength = 0);

    int					buildResponseHeader(char* buffer , int bufLen ,int statusCode , const Fields& fields, __int64 contentLength);

	bool				ErrorResponse( int statuscode);

	const char*			GetRequestQueryString( );

	void				SetCreationTime(FILETIME& fTime );

	void				SetContentType(const std::string& strContentType);

	void				SetStatusString(const std::string& strStatusString);

	void				SetLocationString( const std::string& strLocationString );

protected:
	///parse the start line and find out the method and request resource
	bool				ParseStartLine();

	///open resource type
	bool				OpenResource();

	///open file
	bool				OpenWebFile();

	///open dll file
	bool				OpenDllFile();
	
	///open cgi
	bool				OpenCgiFile();

    ///process procedure call
    void                InvokeProcedure();
	
	//clear resource
	void				ClearResource();
	
private:
	void				appendCgiEnv(const char* pName , const char* pValue);
private:

	HTTPMETHOD			m_MethodType;
	ResourceType		m_ResourceType;
	ContentTypes		m_ContentType;
	std::string			m_strURIQueryString;
	std::string			m_strResourceFile;
	
	FILETIME			m_CreationTime;				/*not for DLL or script*/
	std::string			m_strlocation;					/* */
	std::string			m_strcontentType;				/* */
	std::string			m_strstatus;
	ZQTianShan::Sentry::SentryEnv&			_env;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class HttpDialogCreator : public IDialogueCreator
{
public:
	HttpDialogCreator(ZQTianShan::Sentry::SentryEnv& env);
	~HttpDialogCreator();
public:	
	virtual IDialogue* createDialogue() ;
	
	virtual void releaseDialogue(IN IDialogue* dlg) ;
private:
//	typedef std::map<IDialogue*,IMainConn*>	DialogMap;
//	DialogMap					_map;
//	ZQ::common::Mutex			_mutex;
	ZQTianShan::Sentry::SentryEnv&					_env;	
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class HttpResponse: public IHttpResponse
{
public:
    HttpResponse(HttpYeoman* pHttpYeoman);
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
    virtual IHttpResponse& operator<< (DWORD content){
        _buf << content;
        return (*this);
    }
    virtual IHttpResponse& operator<< (int64 content){
        _buf << content;
        return (*this);
    }

    virtual void SetLastError(const char* errmsg){
        _lastError = (errmsg ? errmsg : "");
    }
    virtual const char* GetLastError(){
        return (_lastError.empty() ? NULL : _lastError.c_str());
    }
    virtual void setContentType(const char* ct)
    {
        if(NULL == ct || '\0' == *ct)
            return; // not alter the content type
        else
            _pHttpYeoman->SetContentType(std::string("Content-Type: ") + ct);
    }

    virtual void setHeader(const char* name, const char* value)
    {
        if(name && '\0' != *name && value)
        {
            _fields[name] = value;
        }
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
    HttpYeoman*    _pHttpYeoman;
    std::ostringstream _buf;
    std::ostringstream _buf_stable;
    std::string     _lastError;
    HttpStatusCodes _statusCode;

    Fields _fields;
};
//////////////////////////////////////////////////////////////////////////

class HttpRequestCtx: public IHttpRequestCtx
{
public:
    HttpRequestCtx(HttpYeoman* pHttpYeoman);
    virtual ~HttpRequestCtx(){
        _pHttpYeoman = NULL;
    }
public:
    virtual const char* GetRootDir(){
        return _rootDir.c_str();
    }
    virtual const char* GetRootURL(){
        return _rootURL.c_str();
    }
    virtual const char* GetUri(){
        return _uri.c_str();
    }
    virtual const char* GetRequestVar(const char* varname){
        if(NULL == varname)
            return NULL;

        RequestVars::iterator itVar = _vars.find(varname);
        if(_vars.end() == itVar)
            return NULL;
        else
            return itVar->second.c_str();
    }
    virtual RequestVars GetRequestVars(){
        return _vars;
    }
    virtual IHttpResponse& Response(){
        return _response;
    }
    virtual HTTPMETHOD GetMethodType(){
        return _method;
    }

    HttpYeoman& Yeoman(){
        return (*_pHttpYeoman);
    }
    HttpResponse& GetResponseObject(){
        return _response;
    }

    // gather the request variables
    bool gatherVars();
private:
    HttpYeoman*     _pHttpYeoman;
    HttpResponse    _response;
    RequestVars     _vars;
    std::string     _rootDir;
    std::string     _rootURL;
    std::string     _uri;
    HTTPMETHOD      _method;

};
#endif // !defined(AFX_HTTPYEOMAN_H__CE7EA374_A2F2_46EC_BF01_36C2AA5724F1__INCLUDED_)
