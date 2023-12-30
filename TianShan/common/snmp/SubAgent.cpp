#include <ZQ_common_conf.h>
#include "SubAgent.hpp"

#ifdef ZQ_OS_MSWIN
#include "ZQSnmpUtil.h"
#else
#include "SnmpUtil.h"
#include "ZQSnmp.h"
#include "Socket.h"
#include "XMLPreferenceEx.h"
#include <fcntl.h>
#include <sstream>
#include <cerrno>

extern ZQ::common::XMLPreferenceEx* getPreferenceNode(const std::string& path, ZQ::common::XMLPreferenceDocumentEx& config);
#endif

#include "UDPSocket.h"
#include "SystemUtils.h"
#include "SnmpInteractive.h"

namespace ZQ{
namespace Snmp{
using namespace ZQ::common;
#define SALOG if(pLog_) (*pLog_)
static void non_deleter(void*){}

Subagent::Subagent(uint32 serviceId, uint32 moduleId, uint32 serviceInstanceId)
    :pLog_(NULL), root_("1.3.6.1.4.1.22839.4.1"), quit_(false),  snmpUdpBasePort_(10000),
	serviceId_(serviceId), moduleId_(moduleId), serviceInstanceId_(serviceInstanceId),selectTimeout_(100)
{
	uint32  serviceStoreId = serviceId + (10 * serviceInstanceId_);
    root_.data().push_back(serviceStoreId);
    root_.data().push_back(moduleId);

    // link the module object to the root object
    boost::shared_ptr<Module> pMod(&mod_, non_deleter);
    rootObj_.add(root_, pMod);
}

Subagent::~Subagent()
{
    stop();
}

Module& Subagent::module()
{
    return mod_;
}

bool Subagent::addObject(const Oid& subid, ManagedPtr obj)
{
    return mod_.add(subid, obj);
}

void Subagent::setLogger(ZQ::common::Log* pLog)
{
    pLog_ = pLog;
}

void Subagent::stop()
{
    if(isRunning())
    {
		quit_ = true;
        uint32 thrdId = id();
        SALOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "stop the message processing thread %d..."), thrdId);
		waitHandle(selectTimeout_);

		SALOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "message processing thread %d is stopped"), thrdId);
    }
}

/////////////////////////////////////////

int Subagent::refreshBasePort(void)
{
    SALOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "refreshBasePort() thread[%d], serviceId[%d], moduleId[%d]"), id(), serviceId_, moduleId_);
	int nRev = false;

#if defined(ZQ_OS_MSWIN)

	HKEY        hRoot;
	DWORD       type;
	const int   BUFSIZE        = 256;
	char        value[BUFSIZE] = {0};
	DWORD       nValue         = BUFSIZE - 1;
	const char* queryString    = "SnmpUdpBasePort";
	const char* root           = "SOFTWARE\\ZQ Interactive\\SNMPOID\\CurrentVersion\\Services";

	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, root, 0, KEY_READ, &hRoot))
	{
		SALOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "refreshBasePort() thread[%d], serviceId[%d], moduleId[%d]  RegOpenKeyEx failed"), id(), serviceId_, moduleId_);
		return nRev;
	}

	if(ERROR_SUCCESS == RegQueryValueEx(hRoot, queryString, NULL, &type, (LPBYTE)value, &nValue) 
		&& REG_DWORD == type)
	{
        nRev = true;
		const unsigned int portMaxLimit = 65535;
		snmpUdpBasePort_ = *(uint32 *)value;
		snmpUdpBasePort_ = (snmpUdpBasePort_ < portMaxLimit) ? snmpUdpBasePort_ : 10000;
        SALOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "refreshBasePort() thread[%d], serviceId[%d], moduleId[%d],snmpUdpBasePort[%d] RegQueryValueEx succeed"), id(), serviceId_, moduleId_, snmpUdpBasePort_);
	}else{
        SALOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "refreshBasePort() thread[%d], serviceId[%d], moduleId[%d],snmpUdpBasePort[%d] RegQueryValueEx failed"), id(), serviceId_, moduleId_, snmpUdpBasePort_);
	}

	RegCloseKey(hRoot);
#else
	std::istringstream       is;
	XMLPreferenceDocumentEx  doc;
	char                     value[256]  =  {0};
	const char*              CONFIG      = "/etc/TianShan.xml";

	if(!doc.open(CONFIG)) 
	{
		SALOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "refreshBasePort() thread[%d], serviceId[%d], moduleId[%d], failed to load configuration[%s]"), id(), serviceId_, moduleId_, CONFIG);
		return nRev;
	}

	ZQ::common::XMLPreferenceEx * node = NULL;
	node = getPreferenceNode("SNMP", doc);
	if(!node) 
	{
		SALOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "refreshBasePort() thread[%d], serviceId[%d], moduleId[%d], failed to get configuration[%s] SNMP node"), id(), serviceId_, moduleId_, CONFIG);
		return nRev;
	}

	nRev = node->getAttributeValue("SnmpUdpBasePort", value);
	if(nRev) 
	{
		const unsigned int portMaxLimit = 65535;
		is.str(value);
		is >> snmpUdpBasePort_;
		snmpUdpBasePort_ = (snmpUdpBasePort_ < portMaxLimit) ? snmpUdpBasePort_ : 10000;
	}

	SALOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "refreshBasePort() thread[%d], serviceId[%d], moduleId[%d], configuration[%s], snmpUdpBasePort[%d], returnCode[%d]"), id(), serviceId_, moduleId_, CONFIG, snmpUdpBasePort_, nRev);
#endif//ZQ_OS_MSWIN

	return nRev;
}


int Subagent::run()
{
	using namespace ZQ::common;

	refreshBasePort();

	uint32         thrdId = id();
	uint32         msgid = 0;
	timeout_t      timeout = selectTimeout_;
	uint32         serviceInstanceId = 10 * serviceInstanceId_;
    uint32         snmpServerPort = (serviceId_ / 100 ) * 100 + moduleId_ + snmpUdpBasePort_ + serviceInstanceId;
	unsigned long  lastLossCount   = 0;
	unsigned long  requestLinePos  = (unsigned long) &((((struct InterActive*) 0)->_content).request);
	InetAddress    udpSvcAddress("127.0.0.1");
	UDPSocket      udpServer(udpSvcAddress, snmpServerPort);

	Socket::Error  svcErrorNum = udpServer.getErrorNumber();
	if(Socket::errSuccess != svcErrorNum)
	{
		SALOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "thread[%d] quit, snmp v3 udp svc port[%d+%d+%d+%d], snmpServerPort[%d],errorNumber[%d]"), 
			thrdId, serviceId_,moduleId_, snmpUdpBasePort_, serviceInstanceId, snmpServerPort, svcErrorNum);

		return false;
	}

	udpServer.setCompletion(false);
	SALOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "thread[%d] running, snmp v3 udp svc port[%d+%d+%d+%d], snmpServerPort[%d],timeout[%d]"), 
		thrdId, serviceId_,moduleId_, snmpUdpBasePort_, serviceInstanceId, snmpServerPort, timeout);

	while(!quit_)
	{
		timeout = selectTimeout_;
		for(int nLoop = 1; !quit_; ++nLoop)
		{
			if (udpServer.isPending(Socket::pendingInput, timeout))
			{
				SALOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "processing thread[%d], snmp udp server port[%d], input timeout[%d], loop[%d]"), thrdId, snmpServerPort, timeout, nLoop);
				break;
			}
		}

		if (quit_)
			break;
		
		InetHostAddress  udpClientAddress;
		int              peerPort = 0;
		std::string      response;
		InterActive      requestFromAgent = {0};
		int              bytes = udpServer.receiveFrom((void *)&requestFromAgent, sizeof(InterActive), udpClientAddress, peerPort);
		if (0 >= bytes)
		{
			SALOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "thread[%d], snmp udp server port[%d], client[%s], peerPort[%d], msgid[%d] lastLossCount[%d], recv erro"), thrdId, snmpServerPort, udpClientAddress.getHostAddress(), peerPort, msgid, lastLossCount);
			continue;
		}		

		SALOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "processing thread[%d], snmp udp server port[%d], client[%s], peerPort[%d], response[%d], lastLossCount[%d], agentRecvCount[%d]"), thrdId, snmpServerPort, udpClientAddress.getHostAddress(), peerPort, msgid, lastLossCount, (requestFromAgent._head._agentRecvSeq));
		bool status = processMessage((requestFromAgent._content.request), bytes - requestLinePos, response);

		++msgid;
		if (!status)
		{
			SALOG(Log::L_WARNING, CLOGFMT(SnmpSubagent, "processing thread[%d], snmp udp server port[%d], client[%s], peerPort[%d], msgid[%d] process erro"), thrdId, snmpServerPort, udpClientAddress.getHostAddress(), peerPort, msgid);
			continue;
		}

		if (ZQSNMP_MSG_LEN_MAX < response.size()) 
		{
			SALOG(Log::L_WARNING, CLOGFMT(SnmpSubagent, "response[%u] size[%u] exceeded max len, abort"), msgid, response.size()); 
			continue;
		}

		if (!udpServer.isPending(Socket::pendingOutput, timeout))
		{
			SALOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "processing response[%d] thread[%d], snmp udp server port[%d], Output timeout[%d]"), msgid, thrdId, snmpServerPort, timeout);
			continue;
		}

		SALOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "request[%u] processMessage successfully, sending response %uB  client[%s], peerPort[%d]"), msgid, response.size(), udpClientAddress.getHostAddress(), peerPort);
		InterActive sendToAgnet = {0};
		sendToAgnet._head._agentRecvSeq     = requestFromAgent._head._agentRecvSeq;
		lastLossCount                       = msgid - requestFromAgent._head._agentRecvSeq;
		sendToAgnet._head._serviceSendSeq   = msgid;
		sendToAgnet._head._lastLossCount    = lastLossCount;
		memcpy((sendToAgnet._content.request), response.data(), response.size());
		int sentState = udpServer.sendto((void *)&sendToAgnet, response.size() + requestLinePos, udpClientAddress, peerPort);
		if (0 >= sentState)
		{
			SALOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "failed to sendto client[%s], peerPort[%d] response[%u] size[%u] %s "), udpClientAddress.getHostAddress(), peerPort, msgid, response.size(), strerror(errno));
			continue;
		}

		// SALOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "client[%s], peerPort[%d]  response[%u] %u bytes send, lastLossCount[%d], agentRecvCount[%d]"), udpClientAddress.getHostAddress(), peerPort, msgid, response.size(), lastLossCount, (sendToAgnet._head._agentRecvSeq));
	}

	SALOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "message processing thread %d stops: snmp udp server port[%d], msgid[%d], lastLossCount[%d]"), thrdId, snmpServerPort, msgid, lastLossCount);
	return 0;
}

#ifdef ZQ_OS_MSWIN
bool Subagent::processMessage(const void *request, int len, std::string& responseMsg)
#else
bool Subagent::processMessage(const u_char* request, int len, std::string& response) 
#endif
{
    if(NULL == request || len <= 0)
        return false;

#if defined(ZQ_OS_MSWIN)
    BYTE pdutype = SNMP_PDU_RESPONSE;
    AsnInteger32 errstat = SNMP_ERRORSTATUS_NOERROR;
    SnmpVarBind vb = {0};
    bool bSuccess = ZQSnmpUtil::DecodeMsg(request, len, &pdutype, &errstat, &vb);
#else
    u_char pdutype = ZQSNMP_PDU_UNKNOWN;
    int errstat = SNMP_ERR_NOERROR;
    netsnmp_variable_list vb = {0};
    bool bSuccess = ZQSNMP::Util::decodeMsg(request, len, &pdutype, &errstat, &vb);
#endif
    if(!bSuccess)
    {
        SALOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "ZQSnmpUtil::DecodeMsg Failed."));
        return false;
    }

    Status status = noError;
    switch(pdutype)
    {
#ifdef ZQ_OS_MSWIN
    case SNMP_PDU_GET:
#else
    case ZQSNMP_PDU_GET:
#endif
        {
            Oid name;
#ifdef ZQ_OS_MSWIN
            oidFrom(name, vb.name);
#else
            oidFrom(name, vb);
#endif
            SmiValue value;
            status = rootObj_.get(name, value);
            if(status == noError)
#ifdef ZQ_OS_MSWIN
                smivalTo(value, vb.value);
#else
                smivalTo(value, vb);
#endif
        }
        break;

#ifdef ZQ_OS_MSWIN
    case SNMP_PDU_GETNEXT:
#else
    case ZQSNMP_PDU_GETNEXT:
#endif
        {
            // find the next object
            Oid name, nextId;
#ifdef ZQ_OS_MSWIN
            oidFrom(name, vb.name);
#else
            oidFrom(name, vb);
#endif
            status = rootObj_.next(name, nextId);
            if(status != noError)
                break;
            // get the value of next object
            SmiValue value;
            status = rootObj_.get(nextId, value);
            if(status == noError)
            {
#ifdef ZQ_OS_MSWIN
                oidTo(nextId, vb.name);
                smivalTo(value, vb.value);
#else
                oidTo(nextId, vb);
                smivalTo(value, vb);
#endif
            }
        }
        break;

#ifdef ZQ_OS_MSWIN
    case SNMP_PDU_SET:
#else
    case ZQSNMP_PDU_SET:
#endif
        {
            Oid name;
#ifdef ZQ_OS_MSWIN
            oidFrom(name, vb.name);
#else
            oidFrom(name, vb);
#endif
            SmiValue value;
#ifdef ZQ_OS_MSWIN
            smivalFrom(value, vb.value, vb.value.asnType);
#else
            smivalFrom(value, vb, vb.type);
#endif
            status = rootObj_.set(name, value);
            break;
        }
    default:
        // method not support
        status = genErr;
        SALOG(Log::L_WARNING, CLOGFMT(SnmpSubagent, "Unknown pdu type %u."), pdutype);
        break;
    }

#ifdef ZQ_OS_MSWIN
    switch(status)
    {
    case noError:
        errstat = SNMP_ERRORSTATUS_NOERROR;
        break;
    case noSuchName:
        errstat = SNMP_ERRORSTATUS_NOSUCHNAME;
        break;
    case badValue: //for set-pdu only
        errstat = SNMP_ERRORSTATUS_BADVALUE;
        break;
    case readOnly: //for set-pdu only
        errstat = SNMP_ERRORSTATUS_READONLY;
        break;
    default:
        errstat = SNMP_ERRORSTATUS_GENERR;
        break;
    }
    bSuccess = ZQSnmpUtil::EncodeMsg(pdutype, errstat, &vb, &responseMsg);

    //cleanup
    SnmpUtilVarBindFree(&vb);
#else
    switch(status) {

    case ZQSNMP_E_NOERROR:
        errstat = SNMP_ERR_NOERROR;
        break;
    case ZQSNMP_E_NOSUCHNAME:
        errstat = SNMP_ERR_NOSUCHNAME;
        break;
    case ZQSNMP_E_BADVALUE: //for set-pdu only
        errstat = SNMP_ERR_BADVALUE;
        break;
    case ZQSNMP_E_READONLY: //for set-pdu only
        errstat = SNMP_ERR_READONLY;
        break;
    default:
        errstat = SNMP_ERR_GENERR;
        break;
    }

    size_t reslen = ZQSNMP_MSG_LEN_MAX;
    u_char res[ZQSNMP_MSG_LEN_MAX];
    bSuccess = ZQSNMP::Util::encodeMsg(res, &reslen, pdutype, errstat, &vb);

    response.assign((const char*)res, reslen);
#endif

    if(bSuccess)
    {
        return true;
    }
    else
    {
        SALOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "ZQSnmpUtil::EncodeMsg Failed."));
        return false;
    }
}

}} // namespace ZQ::Snmp
