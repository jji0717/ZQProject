// implement the SentryPages::proxyPage

#include <algorithm>
#include <boost/regex.hpp>
#include "SentryPages.h"
#include "SentryEnv.h"

#include "HttpClient.h"
#include "SentryHttpImpl.h"
#include "urlstr.h"

namespace ZQTianShan {
namespace Sentry {
using namespace ZQ::common;

#define WEBPRX_VAR_URI    "prx#uri"
namespace HTTPUtil{
    typedef std::map<std::string, std::string> Headers;
    static const char* getHeader(const Headers& headers, const char* name)
    {
        if(NULL == name)
            return NULL;

        for (Headers::const_iterator it = headers.begin(); it != headers.end(); ++it)
        {
            if(0 == stricmp(it->first.c_str(), name))
                return it->second.c_str();
        }
        return NULL;
    }

    struct ProxyContext
    {
        std::string entry;
        std::string basePath; // with '/' tailed
    };

    typedef std::string (*ContentHandler)(const ProxyContext&, const std::string&);

    // make the url absolute
    static std::string absoluteURL(const std::string& basePath, const std::string& url)
    {
        // check the url
        std::string absURL;
        if(std::string::npos != url.find(':'))
        { // treat as absolute url
            absURL = url;
        }
        else
        { // relative url
            absURL = basePath;
            absURL.append(url, ('/' == url[0] ? 1 : 0), url.size());
        }
        return absURL;
    }

    static std::string proxifyURL(const ProxyContext& ctx, const std::string& url)
    {
        if(url.empty())
            return "";

        ZQ::common::URLStr resolved(ctx.entry.c_str(), true);
        resolved.setVar(WEBPRX_VAR_URI, absoluteURL(ctx.basePath, url).c_str());

        const char* sUrl = resolved.generate();
        return sUrl ? sUrl : "";
    }

#define EXP_STARTTARG   "<\\s*(\\w+)([^<>]*)>"
#define EXP_ATTR(name)  "(" #name ")\\s*(?:=\\s*(?:\"([^\"]*)\"|'([^']*)'|([^'\"][^\\s>]*)|(['\"])))?"
    static std::string proxifyHTML(const ProxyContext& ctx, const std::string& doc)
    {
        std::ostringstream out;
        boost::regex re(EXP_STARTTARG);
        boost::smatch resStartTag;
        std::string::const_iterator begPos = doc.begin();
        std::string::const_iterator endPos = doc.end();
        while(boost::regex_search(begPos, endPos, resStartTag, re))
        { // got a start tag
            // copy the skipped data
            if(resStartTag.prefix().matched)
            {
                std::copy(resStartTag.prefix().first, resStartTag.prefix().second, std::ostream_iterator<char>(out));
            }

            // proxify the start tag
            std::string expAttr; // the attribute to be altered

            std::string tagName = resStartTag[1].str();
            std::transform(tagName.begin(), tagName.end(), tagName.begin(), (int (*)(int))(tolower)); // case insensitive
            if(
                0 == tagName.compare("a") ||
                0 == tagName.compare("link") ||
                0 == tagName.compare("area")
                )
            { // proxify the href attribute
                expAttr = EXP_ATTR(href);
                out << "<" << tagName;
                boost::regex reAttr(expAttr, boost::regex::normal | boost::regex::icase);
                boost::smatch resAttr;

                std::string::const_iterator attrsBeg = resStartTag[2].first;
                std::string::const_iterator attrsEnd = resStartTag[2].second;
                if(boost::regex_search(attrsBeg, attrsEnd, resAttr, reAttr))
                {
                    if(resAttr.prefix().matched)
                    {
                        std::copy(resAttr.prefix().first, resAttr.prefix().second, std::ostream_iterator<char>(out));
                    }

                    if(resAttr[2].matched)
                    { // quoted by "
                        out << resAttr[1] << "=\"" << proxifyURL(ctx, resAttr[2].str()) << "\"";
                    }
                    else if( resAttr[3].matched)
                    { // quoted by '
                        out << resAttr[1] << "='" << proxifyURL(ctx, resAttr[3].str()) << "'";
                    }
                    else if(resAttr[4].matched)
                    { // not be quoted
                        out << resAttr[1] << "=" << proxifyURL(ctx, resAttr[4].str());
                    }
                    else if(resAttr[5].matched)
                    { // ill syntax
                    }
                    else
                    { // not found the attribute
                    }

                    if(resAttr.suffix().matched)
                    {
                        std::copy(resAttr.suffix().first, resAttr.suffix().second, std::ostream_iterator<char>(out));
                    }
                }
                else
                { // not found the attribute
                    std::copy(attrsBeg, attrsEnd, std::ostream_iterator<char>(out));
                }

                out << " >";
            }
            else if(
                0 == tagName.compare("img") ||
                0 == tagName.compare("style") ||
                0 == tagName.compare("script") ||
                0 == tagName.compare("frame") ||
                0 == tagName.compare("iframe")
                )
            { // proxify the src attribute
                expAttr = EXP_ATTR(src);
                out << "<" << tagName;
                boost::regex reAttr(expAttr, boost::regex::normal | boost::regex::icase);
                boost::smatch resAttr;

                std::string::const_iterator attrsBeg = resStartTag[2].first;
                std::string::const_iterator attrsEnd = resStartTag[2].second;
                if(boost::regex_search(attrsBeg, attrsEnd, resAttr, reAttr))
                {
                    if(resAttr.prefix().matched)
                    {
                        std::copy(resAttr.prefix().first, resAttr.prefix().second, std::ostream_iterator<char>(out));
                    }

                    if(resAttr[2].matched)
                    { // quoted by "
                        out << resAttr[1] << "=\"" << proxifyURL(ctx, resAttr[2].str()) << "\"";
                    }
                    else if( resAttr[3].matched)
                    { // quoted by '
                        out << resAttr[1] << "='" << proxifyURL(ctx, resAttr[3].str()) << "'";
                    }
                    else if(resAttr[4].matched)
                    { // not be quoted
                        out << resAttr[1] << "=" << proxifyURL(ctx, resAttr[4].str());
                    }
                    else if(resAttr[5].matched)
                    { // ill syntax
                    }
                    else
                    { // not found the attribute
                    }

                    if(resAttr.suffix().matched)
                    {
                        std::copy(resAttr.suffix().first, resAttr.suffix().second, std::ostream_iterator<char>(out));
                    }
                }
                else
                { // not found the attribute
                    std::copy(attrsBeg, attrsEnd, std::ostream_iterator<char>(out));
                }

                out << " >";
            }
            else if(0 == tagName.compare("form"))
            { // proxify the action attribute
                expAttr = EXP_ATTR(action);
                out << "<" << tagName;

                std::string varURI; // the uri of the target

                boost::regex reAttr(expAttr, boost::regex::normal | boost::regex::icase);
                boost::smatch resAttr;

                std::string::const_iterator attrsBeg = resStartTag[2].first;
                std::string::const_iterator attrsEnd = resStartTag[2].second;
                if(boost::regex_search(attrsBeg, attrsEnd, resAttr, reAttr))
                {
                    if(resAttr.prefix().matched)
                    {
                        std::copy(resAttr.prefix().first, resAttr.prefix().second, std::ostream_iterator<char>(out));
                    }

                    if(resAttr[2].matched)
                    { // quoted by "
                        varURI = absoluteURL(ctx.basePath, resAttr[2].str());
                    }
                    else if( resAttr[3].matched)
                    { // quoted by '
                        varURI = absoluteURL(ctx.basePath, resAttr[3].str());
                    }
                    else if(resAttr[4].matched)
                    { // not be quoted
                        varURI = absoluteURL(ctx.basePath, resAttr[4].str());
                    }
                    else if(resAttr[5].matched)
                    { // ill syntax
                    }
                    else
                    { // not found the attribute
                    }
                    // always alter the action to the proxy entry
                    out << " action=\"" << ctx.entry << "\" ";
                    if(resAttr.suffix().matched)
                    {
                        std::copy(resAttr.suffix().first, resAttr.suffix().second, std::ostream_iterator<char>(out));
                    }
                }
                else
                { // not found the attribute
                    std::copy(attrsBeg, attrsEnd, std::ostream_iterator<char>(out));
                    // always alter the action to the proxy entry
                    out << " action=\"" << ctx.entry << "\" ";
                }

                out << ">";
                out << "<input type='hidden' name='" WEBPRX_VAR_URI "' value=\"" << varURI << "\">";
            }
            else
            {
            }

            if(!expAttr.empty())
            {
            }
            else
            { // need not alter
                std::copy(resStartTag[0].first, resStartTag[0].second, std::ostream_iterator<char>(out));
            }

            // continue processing the rest
            begPos = resStartTag[0].second;
        }

        // copy the rest data
        std::copy(begPos, endPos, std::ostream_iterator<char>(out));

        return out.str();
    }
}
static std::string errorPageF(const char *fmt, ...)
{
    char msg[2048] = {0};
    va_list args;
    va_start(args, fmt);
    vsprintf(msg, fmt, args);
    va_end(args);

    std::ostringstream buf;
    buf << "<html><head><title>Error</title></head><body>"
        << "<div>" << msg << "</div>"
        << "</body></html>";
    return buf.str();
}


static std::string errorPage(const char *url, const char *brief, const char *detail = NULL)
{
    if(NULL == detail)
        detail = "";

    return errorPageF("<table style='text-align:left'><tr style='text-align:center'><th colspan='2'><p>%s</p></th></tr><tr><th>URL</th><td>%s</td></tr><tr><th>DETAIL</th><td>%s</td></tr></table>"
        , brief, url, detail);
}


void SentryPages::proxyPage(HttpRequestCtx * pHttpRequestCtx)
{
    httplog(ZQ::common::Log::L_DEBUG,CLOGFMT(SentryPages, "request proxy page."));

#pragma message(__MSGLOC__"may be we should report the error of URL stuff more gently?")
    const char* targetURI = pHttpRequestCtx->GetRequestVar(WEBPRX_VAR_URI);
    if(NULL == targetURI)
    { // feed back the initial page or an error?
#pragma message(__MSGLOC__"feed back the initial page or an error?")
        pHttpRequestCtx->GetResponseObject().StatusCode() = HSC_BAD_REQUEST;
        return;
    }

    httplog(ZQ::common::Log::L_DEBUG,CLOGFMT(SentryPages, "proxyPage() request [%s] thru proxy."), targetURI);


    // parse the URI
    ZQ::common::URLStr target(NULL, true);
    if(!target.parse(targetURI))
    { // not a valid url
        httplog(ZQ::common::Log::L_WARNING,CLOGFMT(SentryPages, "proxyPage() bad request URI [%s]."), targetURI);
        pHttpRequestCtx->GetResponseObject().StatusCode() = HSC_BAD_REQUEST;
        return;
    }
#pragma message(__MSGLOC__"need to check the self referenced uri?")


    // send the request
    int err = HttpClient::HTTP_OK;
    ZQ::common::HttpClient client(&httplog);
    client.init();
    // set necessary headers

#pragma message(__MSGLOC__"TODO: set necessary headers")
    switch(pHttpRequestCtx->GetMethodType())
    {
    case M_GET:
        {
            // get the other request vars
            HttpRequestCtx::RequestVars vars = pHttpRequestCtx->GetRequestVars();
            vars.erase(WEBPRX_VAR_URI);
            for(HttpRequestCtx::RequestVars::iterator it = vars.begin(); it != vars.end(); ++it)
                target.setVar(it->first.c_str(),it->second.c_str());

            err = client.httpConnect(target.generate(), HttpClient::HTTP_GET);
            if(HttpClient::HTTP_OK != err)
            { // fail to connect
                pHttpRequestCtx->Response() << errorPage(targetURI, "Failed to connect the server.", client.getErrorstr());
                return;
            }
            err = client.httpEndSend();
            if(HttpClient::HTTP_OK != err)
            { // fail to send
                pHttpRequestCtx->Response() << errorPage(targetURI, "Failed to send the request.", client.getErrorstr());
                return;
            }
        }
        break;
    case M_POST:
        {
            err = client.httpConnect(targetURI, HttpClient::HTTP_POST);
            if(HttpClient::HTTP_OK != err)
            { // fail to connect
                pHttpRequestCtx->Response() << errorPage(targetURI, "Failed to connect the server.", client.getErrorstr());
                return;
            }

            HttpRequestCtx::RequestVars vars = pHttpRequestCtx->GetRequestVars();
            vars.erase(WEBPRX_VAR_URI);
            ZQ::common::URLStr temp(NULL, true);
            for(HttpRequestCtx::RequestVars::iterator it = vars.begin(); it != vars.end(); ++it)
            {
                temp.setVar(it->first.c_str(), it->second.c_str());
            }
            const char* data = strchr(temp.generate(), '?') + 1;
            if(data && '\0' != (*data))
            {
                err = client.httpSendContent(data, strlen(data));
                if(HttpClient::HTTP_OK != err)
                { // fail to send
                    pHttpRequestCtx->Response() << errorPage(targetURI, "Failed to send the request.", client.getErrorstr());
                    return;
                }
            }

            err = client.httpEndSend();
            if(HttpClient::HTTP_OK != err)
            { // fail to send
                pHttpRequestCtx->Response() << errorPage(targetURI, "Failed to send the request.", client.getErrorstr());
                return;
            }
        }
        break;
    default:
        // other methods
        pHttpRequestCtx->GetResponseObject().StatusCode() = HSC_NOT_IMPLEMENTED;
        return;
    }

    // !!! now we start to receive the response !!!
    std::stringstream resData;
    err = client.httpBeginRecv();
    if(HttpClient::HTTP_OK == err)
    {
        std::string content;
        client.getContent(content);
        resData << content;
    }
    else
    { // fail to receive
        pHttpRequestCtx->Response() << errorPage(targetURI, "Failed to receive the response.", client.getErrorstr());
        return;
    }

    while(!client.isEOF())
    {
        err = client.httpContinueRecv();
        if(HttpClient::HTTP_OK == err)
        {
            std::string content;
            client.getContent(content);
            resData << content;
        }
        else
        { // fail to receive
            pHttpRequestCtx->Response() << errorPage(targetURI, "Failed to receive the response.", client.getErrorstr());
            return;
        }
    }
    err = client.httpEndRecv();
    if(HttpClient::HTTP_OK == err)
    {
        std::string content;
        client.getContent(content);
        resData << content;
    }
    else
    { // fail to receive
        pHttpRequestCtx->Response() << errorPage(targetURI, "Failed to receive the response.", client.getErrorstr());
        return;
    }

    // the response message has been received successfully

    int stCode = client.getStatusCode();
    if(stCode == HSC_OK)
    {
#pragma message(__MSGLOC__"TODO: process the status code")
        
        // find the content type
        std::map<std::string,std::string> headers = client.getHeader();
        const char* contentType = HTTPUtil::getHeader(headers, "Content-Type");
        // find the content handler
        if(NULL == contentType)
        {
            pHttpRequestCtx->Response() << errorPage(targetURI, "No Content-Type in the response.", client.getErrorstr());
            return;
        }
        pHttpRequestCtx->GetResponseObject().setContentType(contentType);

        // select the content handler
        HTTPUtil::ContentHandler contentHandler = NULL;
        if(strstr(contentType, "text/html"))
            contentHandler = HTTPUtil::proxifyHTML;

        // fixup the link url in the html content
        HTTPUtil::ProxyContext ctx;
        ctx.entry = pHttpRequestCtx->GetRootURL();
        ctx.entry += "proxy.sysinvoke";

        ctx.basePath = target.generate();
        ctx.basePath.erase(ctx.basePath.rfind('/') + 1);

        IHttpResponse &out = pHttpRequestCtx->Response();
        if(contentHandler)
            out << contentHandler(ctx, resData.str());
        else
            out << resData.str();

    }
    else
    {
        IHttpResponse &out = pHttpRequestCtx->Response();
        out << errorPageF("Status Code: %d", stCode);
    }

}



}}
