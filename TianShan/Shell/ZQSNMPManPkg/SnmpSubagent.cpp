#include "ZQ_common_conf.h"
#include "SnmpUtil.h"
#include "SnmpSubagent.h"
#include "SnmpManPkg.h"
#include "SnmpInteractive.h"
#include "XMLPreferenceEx.h"
#include "UDPSocket.h"
#include "FileLog.h"
#include <sstream>
#include <cerrno>
#include <fcntl.h>

extern ZQ::common::FileLog* _snmp_logger;

using namespace ZQ::common;

extern XMLPreferenceEx* getPreferenceNode(const std::string&, XMLPreferenceDocumentEx&); 

pthread_mutex_t SnmpSubagent::_mutex = PTHREAD_MUTEX_INITIALIZER;
std::string     SnmpSubagent::_pipeName;

SnmpSubagent::SnmpSubagent(uint32_t serviceID, uint32_t svcProcess, uint32_t svcInstanceId /* = 0*/)
:_running(false),_id(0),_mib(serviceID, svcProcess), _serviceID(serviceID), 
_svcProcess(svcProcess), _snmpUdpBasePort(10000), _svcInstanceId(svcInstanceId), _selectTimeout(100)
{				

}

SnmpSubagent::~SnmpSubagent() 
{
    if(_running) 
	{
        _running = false;
        //open(_pipeName.c_str(), O_WRONLY);
        
		pthread_join(_id, 0);
    }
    unInit();
}

bool SnmpSubagent::ManageVariable(const char *name, void *address, uint32_t type, uint32_t access, int subOid) 
{
    if(!name || !address) 
        return false;
  
	switch(type) 
	{
    case ZQSNMP_VARTYPE_INT32:
    case ZQSNMP_VARTYPE_CSTRING:
    case ZQSNMP_VARTYPE_STDSTRING:
        break;
    default:
        SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "ManageVariable failed. Bad variable type."));
        return false;
    }

    switch(access) {

    case ZQSNMP_ACCESS_READONLY:
    case ZQSNMP_ACCESS_READWRITE:
        break;
    default:
        SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "ManageVariable failed. Bad access type."));
        return false;
    }

    pthread_mutex_lock(&_mutex);
    _mib.Add(Variable(name, address, type, access), subOid);
    pthread_mutex_unlock(&_mutex);

    return true;
}

bool SnmpSubagent::Run() 
{
    SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "SnmpSubagent run enter ..."));

    if(!init()) {
        SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "init Failed."));
        return false;
    }
    
    if(pthread_create(&_id, 0, ThreadProc, this)) {
        SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "failed to create thread: (%s)"), strerror(errno));
        unInit();
        return false;
    }

    SNMPLOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "CreateThread successful. [thread id = %u]"), _id);
    return true;
}

ZQSNMP_STATUS SnmpSubagent::processMessage(const u_char* request, int len, std::string& response)
{
    if(!request || len <= 0) 
        return ZQSNMP_E_FAILURE;
    
    u_char pdutype = ZQSNMP_PDU_UNKNOWN;
    int errstat = SNMP_ERR_NOERROR;

    netsnmp_variable_list vb = {0};
    bool bSuccess = ZQSNMP::Util::decodeMsg(request, len, &pdutype, &errstat, &vb);
    if(!bSuccess) {
        SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "ZQSnmpUtil::DecodeMsg Failed."));

        return ZQSNMP_E_FAILURE;
    }

    ZQSNMP_STATUS status = processRequest(pdutype, &vb);
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
    if(!bSuccess) {
        SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "ZQSnmpUtil::EncodeMsg failed"));

        return ZQSNMP_E_FAILURE;
    }

    response.assign((const char*)res, reslen);

    return ZQSNMP_E_NOERROR;
}

ZQSNMP_STATUS SnmpSubagent::processRequest(u_char mode, netsnmp_variable_list *vars) 
{
    if(!vars) 
        return ZQSNMP_E_FAILURE;

    switch(mode) 
	{
    case ZQSNMP_PDU_GET:
        return processGetRequest(vars);
    case ZQSNMP_PDU_GETNEXT:
        return processGetNextRequest(vars);
    case ZQSNMP_PDU_SET:
        return processSetRequest(vars);
    default:
        SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "processRequest Failed. Unsupported PDU type."));
        return ZQSNMP_E_FAILURE;
    }

    return ZQSNMP_E_FAILURE;
}

ZQSNMP_STATUS SnmpSubagent::processGetRequest(netsnmp_variable_list *vars) 
{
    if(!vars) 
        return ZQSNMP_E_FAILURE;

    pthread_mutex_lock(&_mutex);
    ZQSNMP_STATUS status = _mib.Get(vars);
    pthread_mutex_unlock(&_mutex);

    
    SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "getrequest: %d type: %d"), *(vars->val.integer), vars->type);
    return status;
}

ZQSNMP_STATUS SnmpSubagent::processGetNextRequest(netsnmp_variable_list *vars) 
{
    if(!vars) 
        return ZQSNMP_E_FAILURE;

    pthread_mutex_lock(&_mutex);
    ZQSNMP_STATUS status = _mib.GetNext(vars);
    pthread_mutex_unlock(&_mutex);

    if(ZQSNMP_E_NOERROR == status) {
        vars = vars->next_variable;
    }

    return status;
}

ZQSNMP_STATUS SnmpSubagent::processSetRequest(netsnmp_variable_list *vars) 
{
    if(!vars) {
        return ZQSNMP_E_FAILURE;
    }

    pthread_mutex_lock(&_mutex);
    ZQSNMP_STATUS status = _mib.Set(vars);
    pthread_mutex_unlock(&_mutex);

    return status;
}


bool SnmpSubagent::init() 
{
	SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "SnmpSubagent init enter ..."));
	std::istringstream       is;
	XMLPreferenceDocumentEx  doc;
	int                      nRev        = false;
	char                     value[256]  =  {0};
	const char*              CONFIG      = "/etc/TianShan.xml";

	if(!doc.open(CONFIG)) 
	{
		SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "init() serviceId[%d], moduleId[%d], failed to load configuration[%s]"), _serviceID, _svcProcess, CONFIG);
		return nRev;
	}

	ZQ::common::XMLPreferenceEx * node = getPreferenceNode("SNMP", doc);
	if(!node) 
	{
		SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "init() serviceId[%d], moduleId[%d], failed to get configuration[%s] SNMP node"), _serviceID, _svcProcess, CONFIG);
		return nRev;
	}

	nRev = node->getAttributeValue("SnmpUdpBasePort", value);
	if(nRev) 
	{
		const unsigned int portMaxLimit = 65535;
		is.str(value);
		is >> _snmpUdpBasePort;
		_snmpUdpBasePort = (_snmpUdpBasePort < portMaxLimit) ? _snmpUdpBasePort : 10000;
	}

    SNMPLOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "init() serviceId[%d], moduleId[%d], configuration[%s], snmpUdpBasePort[%d], returnCode[%d]"), _serviceID, _svcProcess, CONFIG, _snmpUdpBasePort, nRev);
    return true;
}

void SnmpSubagent::unInit()
{
    SNMPLOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "unInit(), snmpUdpBasePort[%d], svcProcess[%d], serviceID[%d]"), _snmpUdpBasePort, _svcProcess, _serviceID);
    unlink(_pipeName.c_str());
}



void* SnmpSubagent::ThreadProc(void* arg)
{
	SNMPLOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "thread start. [thread id = %u]"), pthread_self());
    unsigned long  requestLinePos  = (unsigned long) &((((struct InterActive*) 0)->_content).request);
	SnmpSubagent  *pSubagent       = (SnmpSubagent*)arg;
	uint32_t       msgid           = 0;
	unsigned long  lastLossCount   = 0;
	uint32         thrdId          = pthread_self();
	timeout_t      timeout         = pSubagent->_selectTimeout;
	uint32_t       svcInstanceId   = 10 * pSubagent->getSvcInstanceId();
	uint32_t       serviceId       = pSubagent->getServiceID();
	uint32         snmpServerPort  = (serviceId / 100) * 100 + pSubagent->getSvcProcess() + pSubagent->getSnmpUdpBasePort() + svcInstanceId;
	InetAddress    udpSvcAddress("127.0.0.1");
	UDPSocket      udpServer(udpSvcAddress, snmpServerPort);

	Socket::Error  svcErrorNum = udpServer.getErrorNumber();
	if(Socket::errSuccess != svcErrorNum)
	{
		SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "thread[%d] quit, snmp udp server port[%d+%d+%d+%d], snmpServerPort[%d], errorNumber[%d]"), 
			thrdId, serviceId, pSubagent->getSvcProcess(), pSubagent->getSnmpUdpBasePort(), pSubagent->getSvcInstanceId(), snmpServerPort, svcErrorNum);

		return false;
	}

	udpServer.setCompletion(false);
	pSubagent->_running = true;

	SNMPLOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "thread[%d] running, snmp udp server port[%d+%d+%d+%d], snmpServerPort[%d], timeout[%d]"), 
		thrdId, serviceId, pSubagent->getSvcProcess(), pSubagent->getSnmpUdpBasePort(), pSubagent->getSvcInstanceId(), snmpServerPort, timeout);

	while(pSubagent->_running)
	{
		timeout = pSubagent->_selectTimeout;
		for(int nLoop = 1; pSubagent->_running; ++nLoop)
		{
			if (udpServer.isPending(Socket::pendingInput, timeout))
			{
				SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "processing thread[%d], snmp udp server port[%d], input timeout[%d], loop[%d]"), thrdId, snmpServerPort, timeout, nLoop);
				break;
			}
		}

		if (!pSubagent->_running)
			break;

		InterActive      recvFromAgent = {0};
		InetHostAddress  udpClientAddress;
		int              peerPort = 0;
		int              bytes = udpServer.receiveFrom(&recvFromAgent, sizeof(InterActive), udpClientAddress, peerPort);
		if (0 >= bytes || 0 >= (bytes - requestLinePos))
		{
			SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "processing thread[%d], snmp udp server port[%d], client[%s], peerPort[%d] recv bytes[%d] lastLossCount[%d], erro"), thrdId, snmpServerPort, udpClientAddress.getHostAddress(), peerPort, bytes, lastLossCount);
			continue;
		}

		std::string response;
		SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "processing thread[%d], snmp udp server port[%d], client[%s], peerPort[%d], msgId[%d], lastLossCount[%d]"), thrdId, snmpServerPort, udpClientAddress.getHostAddress(), peerPort, msgid, lastLossCount);
		ZQSNMP_STATUS status = pSubagent->processMessage((recvFromAgent._content.request), (bytes - requestLinePos), response);
		++msgid;

		if (ZQSNMP_E_NOERROR != status)
		{
			SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "processing thread[%d], snmp udp server port[%d], client[%s], peerPort[%d], msgid[%d] process erro"), thrdId, snmpServerPort, udpClientAddress.getHostAddress(), peerPort, msgid);
			continue;
		}

		if (ZQSNMP_MSG_LEN_MAX < response.size()) 
		{
			SNMPLOG(Log::L_WARNING, CLOGFMT(SnmpSubagent, "response[%u] size[%u] exceeded max len, abort"), msgid, response.size()); 
			continue;
		}

		if (!udpServer.isPending(Socket::pendingOutput, timeout))
		{
			SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "processing msgid[%d] thread[%d], snmp udp server port[%d], Output timeout[%d]"), msgid, thrdId, snmpServerPort, timeout);
			continue;
		}

		InterActive sendToAgent = {0};
		lastLossCount                       = msgid - recvFromAgent._head._agentRecvSeq;
		sendToAgent._head._agentRecvSeq     = recvFromAgent._head._agentRecvSeq;
		sendToAgent._head._serviceSendSeq   = msgid;
		sendToAgent._head._lastLossCount    = lastLossCount;
		memcpy((sendToAgent._content.request), response.data(), response.size());
		SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "request[%u] processMessage successfully, sending response %uB  client[%s], peerPort[%d]"), msgid, response.size(), udpClientAddress.getHostAddress(), peerPort);
		int sentState = udpServer.sendto(&sendToAgent, (response.size() + requestLinePos), udpClientAddress, peerPort);
		if (0 >= sentState)
		{
			SNMPLOG(Log::L_ERROR, CLOGFMT(SnmpSubagent, "failed sendto client[%s], peerPort[%d] response[%u] size[%u], lastLossCount[%d] %s"), udpClientAddress.getHostAddress(), peerPort, msgid, response.size(), lastLossCount, strerror(errno));
			continue;
		}

		SNMPLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "client[%s], peerPort[%d]  response[%u] %u bytes send, lastLossCount[%d]"), udpClientAddress.getHostAddress(), peerPort, msgid, response.size(), lastLossCount);
	}

	SNMPLOG(Log::L_INFO, CLOGFMT(SnmpSubagent, "leave the message processing thread %d  snmp udp server port[%d], lastLossCount[%d], msgId[%d]"), thrdId, snmpServerPort, lastLossCount, msgid);
	return 0;
}




// vim: ts=4 sw=4 nu bg=dark
