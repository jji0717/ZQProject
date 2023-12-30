#include "WebServer.h"
#include "SentryHttpImpl.h"
#include "DynSharedObj.h"
#include "SystemUtils.h"

//////////////////////////////////////////
// implemention of file reader
class ROFileHandler: public ZQHttp::IRequestHandler
{
public:
    explicit ROFileHandler(const std::string& root, const std::string indexFile)
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
    virtual bool onRequest(const ZQHttp::IRequest& req, ZQHttp::IResponse& resp);
    virtual bool onPostData(const ZQHttp::PostDataFrag& frag)
    {
        return true;
    }
    virtual bool onPostDataEnd()
    {
        return true;
    }
    virtual void onRequestEnd()
    {
    }

    // break the current request processing
    virtual void onBreak()
    {
    }
private:
    std::string _rootPath;
    std::string _idxFile;
};

class FileReaderFactory: public ZQHttp::IRequestHandlerFactory
{
public:
    FileReaderFactory(const std::string& root, const std::string& idxFile = "index.html")
        :_roFileHandler(root, idxFile)
    {
    }
    virtual ~FileReaderFactory(){}
public:
    virtual ZQHttp::IRequestHandler* create(const char* uri)
    {
        return &_roFileHandler;
    }
    virtual void destroy(ZQHttp::IRequestHandler*)
    {
    }
private:
    ROFileHandler _roFileHandler;
};

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

// implement the RO file reader
bool ROFileHandler::onRequest(const ZQHttp::IRequest& req, ZQHttp::IResponse& resp)
{
    std::string strFile = _rootPath + ((0 == strcmp("/", req.uri())) ? _idxFile : req.uri());
    //step 1 . open the file
    FILE* infile = fopen(strFile.c_str(), "rb");
    if(!infile)
    { // 404
        resp.sendDefaultErrorPage(404);
        return false;
    }
    //step 2 .get file size
    //        BY_HANDLE_FILE_INFORMATION fileInfo;
    //        GetFileInformationByHandle(hFile,&fileInfo);
    //        m_CreationTime = fileInfo.ftCreationTime;
    resp.setHeader("Content-Type", getMIMETypeOfFile(strFile).c_str());
    resp.headerPrepared();
    char	szBuf[4096];
    size_t	dwRead = 0;
    do 
    {
        dwRead = fread(szBuf, 1, 4096, infile);
        if(dwRead > 0)
        {
            resp.addContent(szBuf,dwRead);
        }

    } while(dwRead == 4096);
    fclose(infile);
    resp.complete();
    return true;
}

//////////////////////////////////////
// implement the sysinvoke
class SystemFunctionHandler: public ZQHttp::IRequestHandler
{
public:
    SystemFunctionHandler(ZQTianShan::Sentry::SentryEnv& env)
        :_env(env)
    {
        _ctx = NULL;
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
    virtual bool onRequest(const ZQHttp::IRequest& req, ZQHttp::IResponse& resp)
    {
        _ctx = new HttpRequestCtx(_env, req, resp);
        return true;
    }

    virtual bool onPostData(const ZQHttp::PostDataFrag& frag)
    {
        if (frag.name && ('\0' != *frag.name) && frag.data)
            _ctx->setRequestVar(frag.name, std::string(frag.data, frag.len), frag.partial);
        else
            httplog(ZQ::common::Log::L_WARNING, CLOGFMT(SystemFunctionHandler, "NULL post data fragment"));

		return true;
    }

    virtual bool onPostDataEnd()
    {
        return true;
    }
    virtual void onRequestEnd()
    {
        _env._pages.SystemPage(_ctx);
        if(_ctx)
        {
            _ctx->GetResponseObject().flush();
            delete _ctx;
            _ctx = NULL;
        }
    }

    // break the current request processing
    virtual void onBreak()
    {
        clear();
    }

private:
    void clear()
    {
        if(_ctx)
        {
            _ctx->GetResponseObject().clear();
            delete _ctx;
            _ctx = NULL;
        }
    }
private:
    ZQTianShan::Sentry::SentryEnv& _env;
    HttpRequestCtx* _ctx;
};

class SysinvokeHandlerFactory: public ZQHttp::IRequestHandlerFactory
{
public:
    explicit SysinvokeHandlerFactory(ZQTianShan::Sentry::SentryEnv& env)
        :_env(env)
    {
    }
public:

    virtual ZQHttp::IRequestHandler* create(const char* uri)
    {
        return new SystemFunctionHandler(_env);
    }

    virtual void destroy(ZQHttp::IRequestHandler* h)
    {
        if (h)
            delete h;
    }

private:
    ZQTianShan::Sentry::SentryEnv& _env;
};

///////////////////////////////////////
// implement the tswl dll extension
class TswlExtensionHandler: public ZQHttp::IRequestHandler
{
public:
    TswlExtensionHandler(ZQTianShan::Sentry::SentryEnv& env)
        :_env(env)
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

    void openTswlResource();
public:
    /// on*() return true for continue, false for break.
    virtual bool onRequest(const ZQHttp::IRequest& req, ZQHttp::IResponse& resp)
    {
        _ctx = new HttpRequestCtx(_env, req, resp);
        return true;
    }

    virtual bool onPostData(const ZQHttp::PostDataFrag& frag)
    {
        if (frag.name && ('\0' != *frag.name) && frag.data)
            _ctx->setRequestVar(frag.name, std::string(frag.data, frag.len), frag.partial);
        else
            httplog(ZQ::common::Log::L_WARNING, CLOGFMT(TswlExtensionHandler, "NULL post data fragment"));

        return true;
    }

    virtual bool onPostDataEnd()
    {
        return true;
    }

    virtual void onRequestEnd()
    {
        openTswlResource();
        if(_ctx)
        {
            _ctx->GetResponseObject().flush();
            delete _ctx;
            _ctx = NULL;
        }
    }

    // break the current request processing
    virtual void onBreak()
    {
        if(_ctx)
        {
            _ctx->GetResponseObject().clear();
            delete _ctx;
            _ctx = NULL;
        }
    }
private:
    ZQTianShan::Sentry::SentryEnv& _env;
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
    return;
}

class TswlHandlerFactory: public ZQHttp::IRequestHandlerFactory
{
public:
    explicit TswlHandlerFactory(ZQTianShan::Sentry::SentryEnv& env)
        :_env(env)
    {
    }
public:
    virtual ZQHttp::IRequestHandler* create(const char* uri)
    {
        return new TswlExtensionHandler(_env);
    }
    virtual void destroy(ZQHttp::IRequestHandler* h)
    {
        if(h)
        {
            delete h;
        }
    }
private:
    ZQTianShan::Sentry::SentryEnv& _env;
};

// implementation of the WebServer

WebServer::WebServer(ZQTianShan::Sentry::SentryEnv& env)
    :_env(env), _engine(*env._pHttpSvcLog)
{
    char buf[12] = {0};
    _engine.setEndpoint(_env._webBind.c_str(), itoa(_env._webPort, buf, 10));
    _engine.setCapacity(20);
    _engine.setIdleTimeout(_env._httpConnIdleTimeout); // 5min

    _fileReaderFac = new FileReaderFactory(_env._strWebRoot);
    _sysFuncFac = new SysinvokeHandlerFactory(_env);
    _tswlExtFac = new TswlHandlerFactory(_env);
}
WebServer::~WebServer()
{
    if(_fileReaderFac)
    {
        delete _fileReaderFac;
        _fileReaderFac = NULL;
    }
    if(_sysFuncFac)
    {
        delete _sysFuncFac;
        _sysFuncFac = NULL;
    }
    if(_tswlExtFac)
    {
        delete _tswlExtFac;
        _tswlExtFac = NULL;
    }
}
void WebServer::start()
{
    _engine.registerHandler(".*\\.sysinvoke", _sysFuncFac);
    _engine.registerHandler(".*\\.tswl", _tswlExtFac);
    _engine.registerHandler(".*", _fileReaderFac);
    _engine.start();
}
void WebServer::stop()
{
    _engine.stop();
}
