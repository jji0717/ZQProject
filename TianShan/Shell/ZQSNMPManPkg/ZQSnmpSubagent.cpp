// ZQSnmpSubagent.cpp: implementation of the ZQSnmpSubagent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZQSnmpUtil.h"
#include "ZQSnmpSubagent.h"
#include "UDPSocket.h"
#include "SnmpInteractive.h"

#define LOG_MODULE_NAME         "Subagent" 


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ZQSnmpSubagent::ZQSnmpSubagent(UINT serviceID, UINT svcProcess, UINT svcProcessInstanceId)
:m_svcMib(serviceID, svcProcess), m_serviceID(serviceID), m_svcProcess(svcProcess), m_serviceInstanceId(svcProcessInstanceId)
{
	m_snmpUdpBasePort = 10000;
    m_bRunning = false;
	m_selectTimeout = 100;
    m_hPipe = INVALID_HANDLE_VALUE;
    m_thread = NULL;
}

ZQSnmpSubagent::~ZQSnmpSubagent()
{
    if(m_bRunning)
    {
		m_bRunning = false;
        WaitForSingleObject(m_thread, m_selectTimeout);
    }
    if(m_thread != NULL)
    {
        CloseHandle(m_thread);
        m_thread = NULL;
    }
    unInit();
}

bool ZQSnmpSubagent::ManageVariable(const char *name, void *address, DWORD type, DWORD access, UINT instId)
{
    if(NULL == name || NULL == address)
        return false;


    switch(type)
    {
    case ZQSNMP_VARTYPE_INT32:
    case ZQSNMP_VARTYPE_UINT32:
    case ZQSNMP_VARTYPE_STRING:
        break;
    default:
        SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "ManageVariable failed. Bad variable type."));
        return false;
    }
    switch(access)
    {
    case ZQSNMP_ACCESS_READONLY:
    case ZQSNMP_ACCESS_READWRITE:
        break;
    default:
        SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "ManageVariable failed. Bad access type."));
        return false;
    }

    {
        ZQ::common::MutexGuard gd(m_lockMib);
        m_svcMib.Add(Variable(name, address, type, access), instId);
    }

    return true;
}
bool ZQSnmpSubagent::Run()
{
    SMLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "Run..."));
    //create a named pipe server
    //connect the client for request
    //loop ...
    if(!init())
    {
        SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "init Failed."));
        return false;
    }

    DWORD thrdid = 0;
    m_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, this, 0, &thrdid);
    if(NULL == m_thread)
    {
        DWORD error = GetLastError();
        SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "CreateThread Failed. [error code = %u]"), error);
        unInit();
        return false;
    }

    SMLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "CreateThread successful. [thread id = %u]"), thrdid);
    SMLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "Run Successful."));
    return true;
}

ZQSNMP_STATUS ZQSnmpSubagent::processMessage(const void *pRequestMsg, int len, std::string *pResponseMsg)
{
    if(NULL == pRequestMsg || len <= 0 || NULL == pResponseMsg)
        return ZQSNMP_E_FAILURE;

    BYTE pdutype = SNMP_PDU_RESPONSE;
    AsnInteger32 errstat = SNMP_ERRORSTATUS_NOERROR;
    SnmpVarBind vb = {0};
    bool bSuccess = ZQSnmpUtil::DecodeMsg(pRequestMsg, len, &pdutype, &errstat, &vb);
    if(!bSuccess)
    {
        SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "ZQSnmpUtil::DecodeMsg Failed."));

        return ZQSNMP_E_FAILURE;
    }

    ZQSNMP_STATUS status = processRequest(pdutype, &vb);
    switch(status)
    {
    case ZQSNMP_E_NOERROR:
        errstat = SNMP_ERRORSTATUS_NOERROR;
        break;
    case ZQSNMP_E_NOSUCHNAME:
        errstat = SNMP_ERRORSTATUS_NOSUCHNAME;
        break;
    case ZQSNMP_E_BADVALUE: //for set-pdu only
        errstat = SNMP_ERRORSTATUS_BADVALUE;
        break;
    case ZQSNMP_E_READONLY: //for set-pdu only
        errstat = SNMP_ERRORSTATUS_READONLY;
        break;
    default:
        errstat = SNMP_ERRORSTATUS_GENERR;
        break;
    }
    bSuccess = ZQSnmpUtil::EncodeMsg(pdutype, errstat, &vb, pResponseMsg);

    //cleanup
    SnmpUtilVarBindFree(&vb);
    if(!bSuccess)
    {
        SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "ZQSnmpUtil::EncodeMsg Failed."));
        return ZQSNMP_E_FAILURE;
    }
    return ZQSNMP_E_NOERROR;
}
ZQSNMP_STATUS ZQSnmpSubagent::processRequest(BYTE pduType, SnmpVarBind *pVb)
{
    if(NULL == pVb)
        return ZQSNMP_E_FAILURE;

    SMLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "processRequest. [pdutype = %s, oid = %s]"), ZQSnmpUtil::GetPduType(pduType).c_str(), SnmpUtilOidToA(&(pVb->name)));
    switch(pduType)
    {
    case SNMP_PDU_GET:
        return processGetRequest(pVb);
    case SNMP_PDU_GETNEXT:
        return processGetNextRequest(pVb);
    case SNMP_PDU_SET:
        return processSetRequest(pVb);
    default:
        SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "processRequest Failed. Unsupported PDU type."));
        return ZQSNMP_E_FAILURE;//unsupported pdu
    }
}
ZQSNMP_STATUS ZQSnmpSubagent::processGetRequest(SnmpVarBind *pVb)
{
    if(NULL == pVb)
        return ZQSNMP_E_FAILURE;

    ZQSNMP_STATUS status = ZQSNMP_E_FAILURE;
    {
        ZQ::common::MutexGuard gd(m_lockMib);
        status = m_svcMib.Get(&(pVb->name), &(pVb->value));
    }

    return status;
}

ZQSNMP_STATUS ZQSnmpSubagent::processGetNextRequest(SnmpVarBind *pVb)
{
    if(NULL == pVb)
        return ZQSNMP_E_FAILURE;

    AsnObjectIdentifier nextOid = {0};

    ZQSNMP_STATUS status = ZQSNMP_E_FAILURE;
    {
        ZQ::common::MutexGuard gd(m_lockMib);
        status = m_svcMib.GetNext(&(pVb->name), &nextOid, &(pVb->value));
    }

    if(ZQSNMP_E_NOERROR == status)
    {
        SnmpUtilOidFree(&(pVb->name));
        pVb->name = nextOid;
    }

    return status;
}
ZQSNMP_STATUS ZQSnmpSubagent::processSetRequest(SnmpVarBind *pVb)
{
    if(NULL == pVb)
        return ZQSNMP_E_FAILURE;

    ZQSNMP_STATUS status = ZQSNMP_E_FAILURE;
    {
        ZQ::common::MutexGuard gd(m_lockMib);
        status = m_svcMib.Set(&(pVb->name), &(pVb->value));
    }

    return status;
}
bool ZQSnmpSubagent::init()
{
	HKEY        hRoot;
	DWORD       type;
	DWORD       pendType;
	const int   BUFSIZE        = 256;
	char        value[BUFSIZE] = {0};
	DWORD       nValue         = BUFSIZE - 1;
	const char* queryString    = "SnmpUdpBasePort";
	const char* queryPendTime  = "SnmpAgentPendTimeOut";
	const char* root           = "SOFTWARE\\ZQ Interactive\\SNMPOID\\CurrentVersion\\Services";

	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, root, 0, KEY_READ, &hRoot))
	{
		SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "init() serviceId[%d], moduleId[%d], snmpUdpBasePort[%d]  RegOpenKeyEx failed"), m_serviceID, m_svcProcess, m_snmpUdpBasePort);
	}

	if(ERROR_SUCCESS == RegQueryValueEx(hRoot, queryString, NULL, &type, (LPBYTE)value, &nValue) 
		&& REG_DWORD == type)
	{
		const unsigned int portMaxLimit = 65535;
		m_snmpUdpBasePort = *(uint32 *)value;
		m_snmpUdpBasePort = (m_snmpUdpBasePort < portMaxLimit) ? m_snmpUdpBasePort : 10000;
		SMLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "init() serviceId[%d], moduleId[%d]  RegQueryValueEx succeed"), m_serviceID, m_svcProcess, m_snmpUdpBasePort);
	}else{
		SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "init() serviceId[%d], moduleId[%d], snmpUdpBasePort[%d] RegQueryValueEx failed"), m_serviceID, m_svcProcess, m_snmpUdpBasePort);
	}

	RegCloseKey(hRoot);
    
    return true;
}
void ZQSnmpSubagent::unInit()
{

}
//////////////////////////////////////////////////////////////////////////

DWORD WINAPI ZQSnmpSubagent::ThreadProc(LPVOID lpParameter)
{
	SMLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "thread start. [thread id = %u]"), GetCurrentThreadId());
	ZQSnmpSubagent *pSubagent      = (ZQSnmpSubagent *)lpParameter;
	uint32         msgid           = 0;
	unsigned long  lastLossCount   = 0;
	uint32         thrdId          = GetCurrentThreadId();
	timeout_t      timeout         = pSubagent->m_selectTimeout;
	int            instanceId      = 10 * pSubagent->m_serviceInstanceId;
	unsigned long  requestLinePos  = (unsigned long) &((((struct InterActive*) 0)->_content).request);
	uint32         snmpServerPort  = ((int)pSubagent->m_serviceID/100)*100 + pSubagent->m_svcProcess + pSubagent->m_snmpUdpBasePort + instanceId;
	InetAddress    udpSvcAddress("127.0.0.1");
	UDPSocket      udpServer(udpSvcAddress, snmpServerPort);

	Socket::Error  svcErrorNum = udpServer.getErrorNumber();
	if(Socket::errSuccess != svcErrorNum)
	{
	   SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "thread[%d] quit, snmp udp svc port[%d+%d+%d+%d], snmpServerPort[%d],errorNumber[%d]"), 
		   thrdId, pSubagent->m_serviceID, pSubagent->m_svcProcess, pSubagent->m_snmpUdpBasePort, instanceId, snmpServerPort, svcErrorNum);

	   return false;
	}

	udpServer.setCompletion(false);
	pSubagent->m_bRunning = true;

	SMLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "thread[%d] running, snmp udp svc port[%d+%d+%d+%d], snmpServerPort[%d],timeout[%d]"), 
		thrdId, pSubagent->m_serviceID, pSubagent->m_svcProcess, pSubagent->m_snmpUdpBasePort, instanceId, snmpServerPort, timeout);

	while(pSubagent->m_bRunning)
	{
		timeout = pSubagent->m_selectTimeout;
		for(int nLoop = 1; pSubagent->m_bRunning; ++nLoop)
		{
			if (udpServer.isPending(Socket::pendingInput, timeout))
			{
				SMLOG(Log::L_DEBUG, CLOGFMT(SnmpSubagent, "processing thread[%d], snmp udp server port[%d], input timeout[%d], loop[%d]"), thrdId, snmpServerPort, timeout, nLoop);
				break;
			}
		}

		if (!pSubagent->m_bRunning)
			break;

		InterActive      requestFromAgent = {0};
		InetHostAddress  udpClientAddress;
		int              peerPort = 0;
		int              bytes = udpServer.receiveFrom(&requestFromAgent, sizeof(requestFromAgent), udpClientAddress, peerPort);
		if (0 >= bytes)
		{
			SMLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "processing thread[%d], snmp udp server port[%d], client[%s], peerPort[%d] recv erro"), thrdId, snmpServerPort, udpClientAddress.getHostAddress(), peerPort);
			continue;
		}

		SMLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "processing thread[%d], snmp udp server port[%d], client[%s], peerPort[%d], serviceSendCount[%d], agentRecvCount[%d], lastLossCount[%d]"), thrdId, snmpServerPort, udpClientAddress.getHostAddress(), peerPort, msgid, (requestFromAgent._head._agentRecvSeq), lastLossCount);
		std::string response;
		ZQSNMP_STATUS status = pSubagent->processMessage( (requestFromAgent._content.request), bytes - requestLinePos, &response);
		++msgid;

		if (ZQSNMP_E_NOERROR != status)
		{
			SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "processing thread[%d], snmp udp server port[%d], client[%s], peerPort[%d], msgid[%d] process erro"), thrdId, snmpServerPort, udpClientAddress.getHostAddress(), peerPort, msgid);
			continue;
		}

		if (ZQSNMP_MSG_LEN_MAX < response.size()) 
		{
			SMLOG(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "response[%u] size[%u] exceeded max len, abort"), msgid, response.size()); 
			continue;
		}

		if (!udpServer.isPending(Socket::pendingOutput, timeout))
		{
			SMLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "processing msgid[%d] thread[%d], snmp udp server port[%d], Output timeout[%d]"), msgid, thrdId, snmpServerPort, timeout);
			continue;
		}

		SMLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "request[%u] processMessage successfully, sending response %uB  client[%s], peerPort[%d]"), msgid, response.size(), udpClientAddress.getHostAddress(), peerPort);
		InterActive sendToAgent = {0};
		lastLossCount = msgid - requestFromAgent._head._agentRecvSeq;
		sendToAgent._head._serviceSendSeq   = msgid;
		sendToAgent._head._agentRecvSeq     = requestFromAgent._head._agentRecvSeq;
		sendToAgent._head._lastLossCount    = lastLossCount;
		memcpy((sendToAgent._content.request), response.data(), response.size());
		int sentState = udpServer.sendto(&sendToAgent, response.size() + requestLinePos, udpClientAddress, peerPort);
		if (0 >= sentState)
		{
			SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "failed sendto client[%s], peerPort[%d] msgid[%u] size[%u] lastLossCount[%d] agentRecvCount[%d] %s"), udpClientAddress.getHostAddress(), peerPort, msgid, response.size(), lastLossCount, (sendToAgent._head._agentRecvSeq), strerror(errno));
			continue;
		}

		SMLOG(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "client[%s], peerPort[%d], lastLossCount[%d], agentRecvCount[%d], msgid[%u] %u bytes send"), udpClientAddress.getHostAddress(), peerPort, lastLossCount, sendToAgent._head._agentRecvSeq, msgid, response.size());
	}

	SMLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "leave the message processing thread %d  snmp udp server port[%d]"), thrdId, snmpServerPort);
	return 0;
}
//////////////////////////////////////////////////////////////////////////