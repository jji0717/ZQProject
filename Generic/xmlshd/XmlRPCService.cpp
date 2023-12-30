#include <ZQ_common_conf.h>
#include "XmlRPCService.h"
#include "XmlRPCConf.h"
#include "SimpleXMLParser.h"
#include "ConsoleCommand.h"
#include <SystemUtils.h>
#include "selector.h"

#ifdef ZQ_OS_MSWIN
#include <minidump.h>
#else
extern "C"
{
#include <sys/stat.h>
#include <sys/types.h>
}
#endif

static std::string int2str(int i)
{
    char buf[12] = {0};
    return itoa(i, buf, 10);
}

struct MethodCall {
    std::string methodName;
    std::string param;
    bool parse(const std::string& inXml, std::string& err) {
        typedef SimpleXMLParser::Node Node;
        SimpleXMLParser parser;
        try {
            parser.parse(inXml.data(), inXml.size(), 1);
        } catch(const ZQ::common::ExpatException& e) { // bad xml format
            err = "ExpatException: [";
            err += e.getString() ? e.getString() : "NULL";
            err += "] during parsing request body";
            return false;
        }
        /*
        <?xml version="1.0"?>
            <methodCall>
            <methodName>calc</methodName>
            <params>
            <param>
            <value><string>ls -l</string></value>
            </param>
            </params>
            </methodCall>
        */
        const Node* nameNode = findNode(&parser.document(), "methodCall/methodName");
        if(nameNode) {
            methodName = nameNode->content;
        } else {
            err = "No <methodName> provided";
            return false;
        }

        const Node* paramNode = findNode(&parser.document(), "methodCall/params/param/value/string");
        if (NULL == paramNode)
		{
			// http://www.xmlrpc.com/spec says: if no type is indicated, the default type is string
			paramNode = findNode(&parser.document(), "methodCall/params/param/value");
        } 

		if (NULL == paramNode)
		{
            err = "Invalid <param>";
            return false;
        }

		param = paramNode->content;
		return true;
    }
};

struct MethodResponse {
    MethodResponse():faultCode(0){}
    std::string data;
    int faultCode;
    std::string faultString;
    std::string encode(const std::string& val) const {
        /*
        std::string buf;
        buf.resize((val.size() / 3 + 2) * 4);
        if (base64encode(val.data(), val.size(), &buf[0], buf.size())) {
            buf.resize(strlen(buf.c_str());
            return buf;
        } else {
            return "";
        }
        */
        std::ostringstream buf;
        for(size_t i = 0; i < val.size(); ++i) {
            switch(val[i]) {
                case '<':
                    buf << "&lt;";
                    break;
                case '&':
                    buf << "&amp;";
                default:
                    buf << val[i];
                    break;
            }
        }
        return buf.str();
    }
    void build(std::string& outXml) {
        outXml.clear();
        std::ostringstream buf;
        buf << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
            << "<methodResponse>\n";
        if(faultCode == 0) {
        /*
            <methodResponse>
            <params>
            <param>
            <value><string>South Dakota</string></value>
            </param>
            </params>
            </methodResponse>
         */
            buf << "    <params><param>\n"
                << "            <value><string>"
                << encode(data)
                << "            </string></value>\n"
                << "    </param></params>\n";
        } else {
            /*
            <methodResponse>
                <fault>
                <value>
                <struct>
                <member>
                <name>faultCode</name>
                <value><int>4</int></value>
                </member>
                <member>
                <name>faultString</name>
                <value><string>Too many parameters.</string></value>
                </member>
                </struct>
                </value>
                </fault>
                </methodResponse>
            */
            buf << "    <fault><value><struct>\n"
                << "        <member>\n"
                << "            <name>faultCode</name>\n"
                << "            <value><int>" << faultCode << "</int></value>\n"
                << "        </member>\n"
                << "        <member>\n"
                << "            <name>faultString</name>\n"
                << "            <value><string>" << encode(faultString) << "</string></value>\n"
                << "        </member>\n"
                << "    </struct></value></fault>\n";
        }
        buf << "</methodResponse>";
        buf.str().swap(outXml);
    }
};

class XmlRpcCmd: public ZQHttp::IRequestHandler {
public:
    XmlRpcCmd(selector<void>& allowedCommands)
        :allowed_(allowedCommands) {
        xmlescapecharacters.push_back("<") ;
		xmlescapecharacters.push_back(">") ;
		xmlescapecharacters.push_back("&") ;
		xmlescapecharacters.push_back("'") ;
		xmlescapecharacters.push_back("\"") ;
    }
    virtual ~XmlRpcCmd() { };
    /// on*() return true for continue, false for break.
    virtual bool onConnected(ZQHttp::IConnection&){ return true; }
    virtual bool onRequest(const ZQHttp::IRequest& req, ZQHttp::IResponse& resp) {
        reqContent_.clear();
        resp_ = &resp;
        return true;
    }
    virtual bool onPostData(const ZQHttp::PostDataFrag& frag) {
        if(frag.data) {
            reqContent_.append(frag.data, frag.len);
        }
        return true;
    }
    virtual bool onPostDataEnd() {
        return true;
    }
    virtual void onRequestEnd() {
        MethodCall reqCall;
        MethodResponse resp;
        std::string err;
        if(reqCall.parse(reqContent_, err)) {
            if(reqCall.methodName == "exec") {
                //TODO: check the allowed command
                if(allowed_.get(reqCall.param)) {
                    std::ostringstream buf;
                    if(Console::execute(buf, NULL, reqCall.param.c_str())) {
						resp.data = buf.str();
						for(pitescapechs=xmlescapecharacters.begin();pitescapechs != xmlescapecharacters.end();pitescapechs++)
						{
							if((*pitescapechs).compare("<")==0)
								rEscapeCharacterXML(resp.data,*pitescapechs,"&lt;");
							if((*pitescapechs).compare(">")==0)
								rEscapeCharacterXML(resp.data,*pitescapechs,"&gt;");
							if((*pitescapechs).compare("&")==0)
								rEscapeCharacterXML(resp.data,*pitescapechs,"&amp;");		
							if((*pitescapechs).compare("'")==0)
								rEscapeCharacterXML(resp.data,*pitescapechs,"&apos;");
							if((*pitescapechs).compare("\"")==0)
								rEscapeCharacterXML(resp.data,*pitescapechs,"&quot;");
						}	

                        resp.faultCode = 0;
                    } else {
                        resp.faultCode = 4;
                        resp.faultString = "Command failed: " + reqCall.param;
                    }
                } else {
                    resp.faultCode = 3;
                    resp.faultString = "Command not allowed: " + reqCall.param;
                }
            } else {
                resp.faultCode = 2;
                resp.faultString = "Method not allowed: " + reqCall.methodName;
            }
        } else {
            resp.faultCode = 1;
            resp.faultString = "Bad format: " + err;
        }
        std::string respData;
        resp.build(respData);
        resp_->setStatus(200, "OK");
        resp_->setHeader("Content-Length", int2str(respData.size()).c_str());
        resp_->setHeader("Content-Type", "text/xml");
        resp_->headerPrepared();
        resp_->addContent(respData.data(), respData.size());
        resp_->complete();
    }

    // break the current request processing
    virtual void onBreak() {
    }
private:
	std::vector<std::string> xmlescapecharacters;
	std::vector<std::string>::iterator pitescapechs ;
    std::string reqContent_;
    ZQHttp::IResponse* resp_;
    selector<void>& allowed_;
    std::string& rEscapeCharacterXML(std::string& orignStr,const std::string& oldStr,const std::string& newStr)
	{
		size_t pos = 0;
	    std::string::size_type newStrLen = newStr.length();
	    std::string::size_type oldStrLen = oldStr.length();
	    while(true)
	    {
	        pos = orignStr.find(oldStr, pos);
	        if (pos == std::string::npos) 
	        	break;
	        orignStr.replace(pos, oldStrLen, newStr);        
	        pos += newStrLen;
	    }
	    return orignStr; 	
	}	
};

class XmlRpcCmdFac: public ZQHttp::IRequestHandlerFactory
{
public:
    virtual ~XmlRpcCmdFac() {}
    bool setAllowedCommand(const std::string& syntax) {
        return allowed_.set(syntax, (void*)1);
    }
    virtual ZQHttp::IRequestHandler* create(const char* uri)
    {
        return new XmlRpcCmd(allowed_);
    }
    virtual void destroy(ZQHttp::IRequestHandler* h) 
    {
        if(h)
            delete h;
    }
private:
    selector<void> allowed_;
};


XmlRPCService g_server;
ZQ::common::BaseZQServiceApplication	*Application	= &g_server;

ZQ::common::Config::Loader<XmlRPCConf> gConfig("xmlshd.xml");
ZQ::common::Config::ILoader *configLoader = &gConfig;

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType =0 ;

ZQ::common::MiniDump			_crashDump;

void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
    DWORD dwThreadID = GetCurrentThreadId();

    LOG( ZQ::common::Log::L_ERROR,  L"Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",
        ExceptionCode, ExceptionAddress, dwThreadID);

    LOG.flush();
}
#else
extern const char* DUMP_PATH;
#endif

XmlRPCService::XmlRPCService()
{
    _pWebServer = NULL;
    _pCmdFac = NULL;
}

XmlRPCService::~XmlRPCService()
{
}

#define LOG (*m_pReporter)
HRESULT XmlRPCService::OnInit()
{
    // enable crash dump
    if(gConfig.crashDumpEnabled)
    {
#ifdef ZQ_OS_MSWIN
        _crashDump.setDumpPath((char*)gConfig.crashDumpPath.c_str());
        _crashDump.enableFullMemoryDump(true);
        _crashDump.setExceptionCB(CrashExceptionCallBack);
#else
        DUMP_PATH = gConfig.crashDumpPath.c_str();
#endif
        LOG(ZQ::common::Log::L_INFO, CLOGFMT(XmlRPCService, "OnInit() Enable crash dump at [%s]"), gConfig.crashDumpPath.c_str());
    }

    _pWebServer = new ZQHttp::Engine(LOG);
    _pCmdFac = new XmlRpcCmdFac();
    for(size_t i = 0; i < gConfig.allowedCommands.size(); ++i) {
        const std::string& cmd = gConfig.allowedCommands[i];
        if(_pCmdFac->setAllowedCommand(cmd)) {
            LOG(ZQ::common::Log::L_INFO, CLOGFMT(XmlRPCService, "OnInit() Add allowed command pattern [%s]"), cmd.c_str());
        } else {
            LOG(ZQ::common::Log::L_WARNING, CLOGFMT(XmlRPCService, "OnInit() Failed to add command pattern [%s]"), cmd.c_str());
        }
    }
    _pWebServer->registerHandler("/RPC2", _pCmdFac);

    _pWebServer->setEndpoint(gConfig.bindAddress, int2str(gConfig.bindPort));
    _pWebServer->setCapacity(gConfig.capacity);
    _pWebServer->enableMessageDump(gConfig.hexDump, true, true);

    return BaseZQServiceApplication::OnInit();
}

HRESULT XmlRPCService::OnStart()
{
    if(_pWebServer) {
        _pWebServer->start();
    }
    return BaseZQServiceApplication::OnStart();
}
HRESULT XmlRPCService::OnStop()
{
    if(_pWebServer) {
        _pWebServer->stop();
    }
    return BaseZQServiceApplication::OnStop();
}
HRESULT XmlRPCService::OnUnInit()
{
    if(_pWebServer) {
        delete _pWebServer;
        _pWebServer = 0;
    }
    if(_pCmdFac) {
        delete _pCmdFac;
        _pCmdFac = 0;
    }
    return BaseZQServiceApplication::OnUnInit();
}

void XmlRPCService::OnSnmpSet(const char *varName)
{
}

