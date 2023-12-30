#include "WebServer.h"
#include "SentryHttpImpl.h"
#include "DynSharedObj.h"
//#include "SystemUtils.h"


/// impl
static std::string getMIMETypeOfFile(const std::string& filePath)
{ // get the MIME type of the target file base on the file ext
	std::string ext = filePath.substr(filePath.find_last_of('.'));

	static const char* typeTbl[] = {
		".htm", "text/html",
		".html", "text/html",
		".js", "text/javascript",
		".css", "text/css",
		".txt", "text/plain",
		".xml", "text/xml",
		".gif", "image/gif",
		".jpg", "image/jpeg",
		".exe", "application/octet-stream"
	} ;

	for(size_t i = 0; i < sizeof(typeTbl) / sizeof(typeTbl[0]); i += 2)
	{
		if(0 == stricmp(typeTbl[i], ext.c_str()))
			return typeTbl[i + 1];
	}
	return "application/octet-stream";
}

//////////////////////////////////////////
// implemention of file reader
class ROFileHandler: public ZQ::eloop::HttpHandler
{
public:
	explicit ROFileHandler(ZQ::eloop::HttpPassiveConn& conn,ZQ::common::Log& logger,const ZQ::eloop::HttpHandler::Properties& dirProps, const ZQ::eloop::HttpHandler::Properties& appProps,const std::string& root, const std::string indexFile)
		:ZQ::eloop::HttpHandler(conn,logger,dirProps, appProps)
	{
		_rootPath = root;
		if(!_rootPath.empty())
		{
			char tail = *_rootPath.rbegin();
			if(tail == '\\' || tail == '/')
				_rootPath.erase(_rootPath.size() - 1);
		}
		_idxFile = indexFile;
		if(!_idxFile.empty() && (_idxFile[0] == '\\' || _idxFile[0] == '/'))
		{
			_idxFile = _idxFile.substr(1);
		}

		_idxFile = _idxFile.empty() ? "/index.html" : (std::string("/") + _idxFile);
	}

	/// on*() return true for continue, false for break.
	virtual bool onHeadersEnd( const ZQ::eloop::HttpMessage::Ptr msg)
	{
		std::string strFile = _rootPath + ((0 == strcmp("/", msg->url().c_str())) ? _idxFile : msg->url().c_str());
		//step 1 . open the file
		_infile = fopen(strFile.c_str(), "rb");
		if(!_infile)
		{ // 404
//			resp.sendDefaultErrorPage(404);
			_conn.errorResponse(404);
			return false;
		}
		//step 2 .get file size
		//        BY_HANDLE_FILE_INFORMATION fileInfo;
		//        GetFileInformationByHandle(hFile,&fileInfo);
		//        m_CreationTime = fileInfo.ftCreationTime;
		ZQ::eloop::HttpMessage::Ptr resp = new ZQ::eloop::HttpMessage(ZQ::eloop::HttpMessage::MSG_RESPONSE);

		resp->header("Content-Type", getMIMETypeOfFile(strFile).c_str());

		std::ifstream in(strFile.c_str());   
		in.seekg(0,std::ios::end);   
		long size = in.tellg();   
		in.close();
		printf("filename:%s,content length = %ld\n",strFile.c_str(),size);

		resp->contentLength(size);

//		resp->chunked(true);
		std::string head = resp->toRaw();
		_conn.write(head.c_str(),head.size());
		_conn.onRespHeader();
/*
		char	szBuf[4096];
		size_t	dwRead = 0;
		std::string len;
		do 
		{
			dwRead = fread(szBuf, 1, 4096, infile);
			if(dwRead > 0)
			{
				len = resp->uint2hex(dwRead, 4, '0') + CRLF;

				_conn.write(len.c_str(),len.size());
				_conn.write(szBuf,dwRead);
				_conn.write(CRLF,2);
			}
		} while(dwRead == 4096);
		fclose(infile);
*/
		return true;
	}
	virtual bool onBodyData( const char* data, size_t size)
	{
		if(data)
		{
			//char ip[17] = {0};
			//int port = 0;
			//_conn.getpeerIpPort(ip,port);
			//_Logger(ZQ::common::Log::L_INFO, CLOGFMT(ClientRequestHandler,"onBodyData [%s:%d]"),ip,port);
			_postedData.append(data,size);
		}
		return true;
	}

	virtual void onMessageCompleted()
	{
	}

	virtual void	onError( int error,const char* errorDescription ){}
	virtual void	onHttpDataSent()
	{
		char	szBuf[4096];
		size_t	dwRead = 0;
//		std::string len;

		dwRead = fread(szBuf, 1, 4096, _infile);
		if(dwRead > 0)
		{
//			len = resp->uint2hex(dwRead, 4, '0') + CRLF;

//			_conn.write(len.c_str(),len.size());
			_conn.write(szBuf,dwRead);
//			_conn.write(CRLF,2);
		}
		else
		{
			fclose(_infile);
			_conn.onRespComplete();
		}
	}
	virtual void	onHttpDataReceived( size_t size ){}
private:
	std::string _rootPath;
	std::string _idxFile;
	std::string _postedData;
	FILE* _infile;
};

class FileReaderFactory: public ZQ::eloop::HttpBaseApplication
{
public:
	FileReaderFactory(const std::string& root, const std::string& idxFile = "index.html")
		:_root(root),
		_idxFile(idxFile)
	{
	}
	virtual ~FileReaderFactory(){}
public:

	virtual ZQ::eloop::HttpHandler::Ptr create( ZQ::eloop::HttpPassiveConn& conn,ZQ::common::Log& logger,const ZQ::eloop::HttpHandler::Properties& dirProps)
	{
		return new ROFileHandler(conn,logger,dirProps,_appProps,_root,_idxFile);
	}

private:
	const std::string&	_root;
	const std::string	_idxFile;
};

// --------------------------------------------
// class SystemFunctionHandler
// --------------------------------------------
// implement the sysinvoke
class SystemFunctionHandler: public ZQ::eloop::HttpHandler
{
public:
	SystemFunctionHandler(ZQ::eloop::HttpPassiveConn& conn,ZQ::common::Log& logger,const ZQ::eloop::HttpHandler::Properties& dirProps, const ZQ::eloop::HttpHandler::Properties& appProps,ZQTianShan::Sentry::SentryEnv& env)
		:ZQ::eloop::HttpHandler(conn,logger,dirProps, appProps),
		_env(env),
		_ctx(NULL)
	{
	}
	virtual ~SystemFunctionHandler()
	{
		if(_ctx)
		{
			delete _ctx;
			_ctx = NULL;
		}
	}
public:
	/// on*() return true for continue, false for break.
	virtual bool onHeadersEnd( const ZQ::eloop::HttpMessage::Ptr msg)
	{
		_ctx = new HttpRequestCtx(_env,_conn,msg);
		return true;
	}
	virtual bool onBodyData( const char* data, size_t size)
	{
		/*if (frag.name && ('\0' != *frag.name) && frag.data)
			_ctx->setRequestVar(frag.name, std::string(frag.data, frag.len), frag.partial);
		else
			httplog(ZQ::common::Log::L_WARNING, CLOGFMT(SystemFunctionHandler, "NULL post data fragment"));*/
		if(data)
		{
			//char ip[17] = {0};
			//int port = 0;
			//_conn.getpeerIpPort(ip,port);
			//_Logger(ZQ::common::Log::L_INFO, CLOGFMT(ClientRequestHandler,"onBodyData [%s:%d]"),ip,port);
			_postedData.append(data,size);
		}
		return true;
	}

	virtual void onMessageCompleted()
	{
		_env._pages.SystemPage(_ctx);
		if(_ctx)
		{
			_ctx->GetResponseObject().flush();
			delete _ctx;
			_ctx = NULL;
		}
	}

	virtual void	onError( int error,const char* errorDescription ){}
	virtual void	onHttpDataSent(){}
	virtual void	onHttpDataReceived( size_t size ){}

private:
	ZQTianShan::Sentry::SentryEnv& _env;
	std::string _postedData;
	HttpRequestCtx* _ctx;
};

// --------------------------------------------
// class SysinvokeHandlerFactory
// --------------------------------------------
class SysinvokeHandlerFactory: public ZQ::eloop::HttpBaseApplication
{
public:
	explicit SysinvokeHandlerFactory(ZQTianShan::Sentry::SentryEnv& env)
		:_env(env)
	{
	}
public:

	virtual ZQ::eloop::HttpHandler::Ptr create( ZQ::eloop::HttpPassiveConn& conn,ZQ::common::Log& logger,const ZQ::eloop::HttpHandler::Properties& dirProps)
	{
		return new SystemFunctionHandler(conn,logger,dirProps,_appProps,_env);
	}

private:
	ZQTianShan::Sentry::SentryEnv& _env;
};

// --------------------------------------------
// class TswlExtensionHandler
// --------------------------------------------
// implement the tswl dll extension
class TswlExtensionHandler:public ZQ::eloop::HttpHandler
{
public:
	TswlExtensionHandler(ZQ::eloop::HttpPassiveConn& conn,ZQ::common::Log& logger,const ZQ::eloop::HttpHandler::Properties& dirProps, const ZQ::eloop::HttpHandler::Properties& appProps,ZQTianShan::Sentry::SentryEnv& env)
		:ZQ::eloop::HttpHandler(conn,logger,dirProps, appProps),
		_env(env)
	{
		_ctx = NULL;
	}
	virtual ~TswlExtensionHandler()
	{
		if(_ctx)
		{
			delete _ctx;
			_ctx = NULL;
		}
	}
	virtual bool onHeadersEnd( const ZQ::eloop::HttpMessage::Ptr msg)
	{
		_ctx = new HttpRequestCtx(_env,_conn,msg);
		return true;
	}
	virtual bool onBodyData( const char* data, size_t size)
	{
		if(data)
		{
			//char ip[17] = {0};
			//int port = 0;
			//_conn.getpeerIpPort(ip,port);
			//_Logger(ZQ::common::Log::L_INFO, CLOGFMT(ClientRequestHandler,"onBodyData [%s:%d]"),ip,port);
			_postedData.append(data,size);
		}
		return true;
	}

	virtual void onMessageCompleted()
	{
		for(std::map< std::string, std::string >::const_iterator it = _ctx->_vars.begin(); it != _ctx->_vars.end(); ++it)
		{
			printf("key:%s	value:%s\n",it->first.c_str(),it->second.c_str());
		}
		openTswlResource();
		if(_ctx)
		{
			_ctx->GetResponseObject().flush();
			delete _ctx;
			_ctx = NULL;
		}
	}

	void openTswlResource();
	virtual void onError( int error,const char* errorDescription ){}
	virtual void	onHttpDataSent(){}
	virtual void	onHttpDataReceived( size_t size ){}

private:
	ZQTianShan::Sentry::SentryEnv& _env;
	std::string _postedData;
	HttpRequestCtx* _ctx;
};

#define LAYOUT_VAR_TEMPLATE     "#template"
void TswlExtensionHandler::openTswlResource()
{
	using namespace ZQ::common;
	_ctx->GetResponseObject().setContentType("text/html");
	std::string strFile = _ctx->GetUri();
	httplog(Log::L_DEBUG,CLOGFMT(TswlExtensionHandler, "openTswlResource() enter with file:%s"), strFile.c_str());

	std::string proc = "LibProc";
	//resolve the dll file and the procedure
	{
		std::string::size_type  pos_ext = strFile.find_first_of('.');
		std::string             filetype= strFile.substr(pos_ext);
		if(filetype == ".dll")
		{
			const char *filename = _env.FindReference(strFile);
			if(NULL != filename)
				strFile = filename;
		}
		else
		{
			proc = strFile.substr(0, pos_ext);
			const char *filename = _env.FindReference(filetype);
			strFile = (NULL == filename) ? "" : filename;
		}
	}

	if(strFile.empty() || proc.empty())
	{
		httplog(ZQ::common::Log::L_ERROR, CLOGFMT(TswlExtensionHandler, "unknown resource file.[%s]"), _ctx->GetUri());
		_ctx->GetResponseObject().StatusCode() = HSC_BAD_REQUEST;
		return;
	}

	ZQ::common::DynSharedObj shared(strFile.c_str());
	if(!shared.isLoaded())
	{
		httplog(ZQ::common::Log::L_ERROR, CLOGFMT(TswlExtensionHandler, "Load dll %s failed: [%s]"),strFile.c_str(), shared.getErrMsg());

		_ctx->GetResponseObject().StatusCode() = HSC_BAD_REQUEST;
		return;
	}
	try
	{
		LibInitialize init = (LibInitialize)SYS::getProcAddr(shared.getLib(), "LibInit");
		LibUnInitialize uninit = (LibUnInitialize)SYS::getProcAddr(shared.getLib(), "LibUninit");
		LibProcess process = (LibProcess)SYS::getProcAddr(shared.getLib(), proc.c_str());
		if (!( init && uninit && process )) 
		{
			httplog(Log::L_ERROR, CLOGFMT(TswlExtensionHandler, "Invalid lib file [%s],routinue LibInit LibUnit %s must be exist"),strFile.c_str(), proc.c_str());
			shared.free();

			_ctx->GetResponseObject().StatusCode() = HSC_BAD_REQUEST;
			return;
		}

		if(!init(_env._pHttpSvcLog))
		{
			shared.free();
			httplog(Log::L_ERROR, CLOGFMT(TsweExtensionHandler, "lib [%s] initialize failed"), strFile.c_str());

			_ctx->GetResponseObject().StatusCode() = HSC_BAD_REQUEST;
			return;
		}
		// init ok now

		const char* tmpl = _ctx->GetRequestVar(LAYOUT_VAR_TEMPLATE);

		if(tmpl)
		{
			HttpResponse &out = _ctx->GetResponseObject();
			_env._pages.pushHeader(out, tmpl);
			out.flush();
			if(!process(_ctx))
			{
				// discard suspect content
				out.clear();
				out << "<h3>Error occured during the request:</h3><blockquote><p>"
					<< (out.GetLastError() ? out.GetLastError() : "(no explanation)")
					<< "</p></blockquote>";
			}
			_env._pages.pushFooter(out, tmpl);
		}
		else // no template name, all content is provided by the plug-in
		{
			httplog(Log::L_DEBUG, CLOGFMT(TswlExtensionHandler, "template name missed, all the content is provided by the dll"));
			HttpResponse &out = _ctx->GetResponseObject();
			if(!process(_ctx))
			{
				// discard suspect content
				out.clear();
				_env._pages.pushHeader(out, NULL);
				out << "<h3>Error occured during the request:</h3><blockquote><p>"
					<< (out.GetLastError() ? out.GetLastError() : "(no explanation)")
					<< "</p></blockquote>";
				_env._pages.pushFooter(out, NULL);
			}
		}

		uninit();
		httplog(Log::L_DEBUG, CLOGFMT(TswlExtensionHandler, "lib [%s] uninit ok"), strFile.c_str());
		shared.free();
	}
	catch (...) 
	{
		httplog(Log::L_ERROR, CLOGFMT(TswlExtensionHandler, "openTswlResource() catch a exception when invoke dll"));
		_ctx->GetResponseObject().StatusCode() = HSC_INTERNAL_SERVER_ERROR;
		shared.free();
	}
	httplog(Log::L_DEBUG, CLOGFMT(TswlExtensionHandler, "openTswlResource() leave"));
}


// --------------------------------------------
// class TswlHandlerFactory
// --------------------------------------------
class TswlHandlerFactory: public ZQ::eloop::HttpBaseApplication
{
public:
	explicit TswlHandlerFactory(ZQTianShan::Sentry::SentryEnv& env)
		:_env(env)
	{
	}
public:
	virtual ZQ::eloop::HttpHandler::Ptr create( ZQ::eloop::HttpPassiveConn& conn,ZQ::common::Log& logger,const ZQ::eloop::HttpHandler::Properties& dirProps)
	{
		return new TswlExtensionHandler(conn,logger,dirProps,_appProps,_env);
	}

private:
	ZQTianShan::Sentry::SentryEnv& _env;
};


// --------------------------------------------
// class WebServer
// --------------------------------------------
// implementation of the WebServer
WebServer::WebServer(ZQTianShan::Sentry::SentryEnv& env)
:_env(env),_httpSvr(env._httpConfg,*env._pHttpSvcLog)//, _engine(*env._pHttpSvcLog)
{
	char buf[12] = {0};
//	_engine.setEndpoint(_env._webBind.c_str(), itoa(_env._webPort, buf, 10));
//	_engine.setCapacity(20);
//	_engine.setIdleTimeout(_env._httpConnIdleTimeout); // 5min

	_fileReaderFac = new FileReaderFactory(_env._strWebRoot);
	_sysFuncFac = new SysinvokeHandlerFactory(_env);
	_tswlExtFac = new TswlHandlerFactory(_env);
}
WebServer::~WebServer()
{
}
void WebServer::start()
{
	_httpSvr.mount(".*\\.sysinvoke", _sysFuncFac);
	_httpSvr.mount(".*\\.tswl", _tswlExtFac);
	_httpSvr.mount(".*", _fileReaderFac);
	_httpSvr.startAt();
}
void WebServer::stop()
{
	_httpSvr.stop();
}
