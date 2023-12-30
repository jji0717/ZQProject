#include "SentryHttpImpl.h"
#include "SentryEnv.h"

//////////////////////////////////////////////////////////////////////////
HttpResponse::HttpResponse(ZQ::eloop::HttpMessage::Ptr req,ZQ::eloop::HttpPassiveConn& conn)
    :_statusCode(HSC_OK),_conn(conn)
{
		_resp = new ZQ::eloop::HttpMessage(ZQ::eloop::HttpMessage::MSG_RESPONSE);
		if (req->keepAlive()&&_conn.keepAlive())
		{
			_resp->keepAlive(true);
		}
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
/*        _resp.setStatus(HSC_OK);
        _resp.headerPrepared();
        //send msg
        _resp.addContent(msg.data(), msg.size());
        _resp.complete();
*/

		_resp->header("Server","LibAsYnc HtTp SeRVer");
		_resp->header("Date",ZQ::eloop::HttpMessage::httpdate());
		_resp->contentLength(msg.size());
		_resp->code(HSC_OK);

		std::string head = _resp->toRaw();
		msg +=head;
		//_conn.write(head.c_str(),head.size());
		_conn.write(msg.data(),msg.size());
     }
    else
    {
//        _resp.sendDefaultErrorPage(_statusCode);
		_conn.errorResponse(_statusCode);
    }

    // clear buffer
    _buf_stable.str("");
}

HttpRequestCtx::HttpRequestCtx(const ZQTianShan::Sentry::SentryEnv& env,ZQ::eloop::HttpPassiveConn& conn,const ZQ::eloop::HttpMessage::Ptr req)
:_response(req,conn)
{
    _rootDir = env._strWebRoot;
    _rootURL = env._selfInfo.adminRootUrl;
//    _uri = req.uri();
	_uri = req->url();
    if(!_uri.empty() && _uri[0] == '/')
    { // remove the leading slash to fit the old interface
        _uri = _uri.substr(1);
    }
    switch(req->method())
    {
	case ZQ::eloop::HttpMessage::GET:
        _method = M_GET;
        break;
    case ZQ::eloop::HttpMessage::POST:
        _method = M_POST;
        break;
    default:
        _method = M_UNKNOWN;
        break;
    }
    _vars = req->queryArguments();
}

