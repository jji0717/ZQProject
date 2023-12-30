#include "SentryHttpImpl.h"
#include "SentryEnv.h"

//////////////////////////////////////////////////////////////////////////
HttpResponse::HttpResponse(ZQHttp::IResponse& resp)
    :_resp(resp), _statusCode(HSC_OK)
{
}
HttpResponse::~HttpResponse()
{
    flush();
    send();
}
void HttpResponse::flush()
{
    _buf_stable << _buf.str();
    _buf.str("");//clear buffer
}
void HttpResponse::send()
{
    if(HSC_OK == _statusCode)
    {
        std::string msg;
        _buf_stable.str().swap(msg);
        _resp.setStatus(HSC_OK);
        _resp.headerPrepared();
        //send msg
        _resp.addContent(msg.data(), msg.size());
        _resp.complete();
     }
    else
    {
        _resp.sendDefaultErrorPage(_statusCode);
    }

    // clear buffer
    _buf_stable.str("");
}

HttpRequestCtx::HttpRequestCtx(const ZQTianShan::Sentry::SentryEnv& env, const ZQHttp::IRequest& req, ZQHttp::IResponse& resp)
    :_response(resp)
{
    _rootDir = env._strWebRoot;
    _rootURL = env._selfInfo.adminRootUrl;
    _uri = req.uri();
    if(!_uri.empty() && _uri[0] == '/')
    { // remove the leading slash to fit the old interface
        _uri = _uri.substr(1);
    }
    switch(req.method())
    {
    case ZQHttp::GET:
        _method = M_GET;
        break;
    case ZQHttp::POST:
        _method = M_POST;
        break;
    default:
        _method = M_UNKNOWN;
        break;
    }
    _vars = req.queryArguments();
}

