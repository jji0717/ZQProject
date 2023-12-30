#include <syslog.h>
#include <sstream>

#include "XMLPreferenceEx.h"
#include "strHelper.h"
#include "FileLog.h"
#include "SnmpUtil.h"
#include "ZQSnmp.h"
#include "Locks.h"
#include "UDPSocket.h"
#include "SnmpInteractive.h"

#include <net-snmp/agent/net-snmp-agent-includes.h>

#define MYLOG (logger)


using namespace ZQ::common;

namespace {
	// .iso.org.dod.internet.private.enterprises.ZQ.SE1.TianShan
	const char* OID_PREFIX = ".1.3.6.1.4.1.22839.4.1";
	// .serviceID.Service/Shell.name/value/perm.variable_number
	// .2.2.1.11

	const char* MODULE_NAME = "SnmpAgent";
	const char* CONFIG  = "/etc/TianShan.xml";

	FileLog logger;
	
	uint32        g_snmpUdpBasePort      = 10000;
	timeout_t     g_snmpAgentPendTimeOut = 5000;
	unsigned long g_agentRecvCount       = 0;

	Mutex reqlock;
}

extern XMLPreferenceEx* getPreferenceNode(const std::string&, XMLPreferenceDocumentEx&); 
extern bool loadConfig();
extern std::string print_requests(netsnmp_request_info*);

#ifdef __cplusplus
extern "C" {

	extern int agentHandler(
		netsnmp_mib_handler* handler,
		netsnmp_handler_registration* reginfo,
		netsnmp_agent_request_info* reqinfo,
		netsnmp_request_info* requests); 

	extern void init_SNMPAgent(); 

}
#endif

void init_SNMPAgent()
{
	std::string logDir =".";
	uint32 loggingMask = 0;

	ZQ::common::XMLPreferenceDocumentEx doc; 
	if (!doc.open(CONFIG)) 
	{
		syslog(LOG_ERR, "failed to load configuration file[%s]", CONFIG);
		return ;
	}

	ZQ::common::XMLPreferenceEx* node = getPreferenceNode("SNMP", doc);
	if (!node) 
	{
		syslog(LOG_ERR, "failed to find element[SNMP]");
		return ;
	}

	char value[256] ="\0";
	// loggingMask 
	memset(value, '\0', 256);
	if (node->getAttributeValue("loggingMask", value))
		loggingMask = atol(value);

	// logPath 
	memset(value, '\0', 256);
	if (node->getAttributeValue("logPath", value))
		logDir = value;

	// fixup the configurations
	if (logDir.empty())
		logDir = "." FNSEPS;

	if (FNSEPC != logDir[logDir.length()-1])
		logDir += FNSEPC;

	// open the log file
	std::string fnLog = logDir + MODULE_NAME +".log";
	ZQ::common::FileLog filelog;
	try{
		filelog.open(fnLog.c_str(), loggingMask & 0x0F);
	}
	catch(...)
	{	
		syslog(LOG_ERR, "failed to create logfile");
		return;
	}

	//prepare root oid	
	static oid myOid[MAX_OID_LEN];
	size_t len = MAX_OID_LEN;

	filelog(Log::L_DEBUG, "creating rootOid");
	if (!read_objid(OID_PREFIX, myOid, &len))
	{
		filelog(Log::L_ERROR, "failed to read root oid");
		return;
	}
	
	//register request handler	
	filelog(Log::L_DEBUG, "registering handler");
	netsnmp_handler_registration* myHandler = netsnmp_create_handler_registration(
		"agentHandler",
		agentHandler,
		myOid,
		len,
		HANDLER_CAN_RWRITE);

	if(!myHandler) 
	{
		filelog(Log::L_ERROR, "failed to create agent handler");
		return;
	}

	myHandler->my_reg_void = NULL;
	netsnmp_register_handler(myHandler);

    filelog(Log::L_INFO, "SNMPAgent initialized: oid[%s], handler[%p]", OID_PREFIX, myHandler);									 
	
	filelog.flush();
}

std::string reqdesc(netsnmp_request_info* request) 
{
	//#ifdef _DEBUG
	std::string reqstr;
	switch(request->agent_req_info->mode) 
	{
	case MODE_GET:
		reqstr = "GET";
		break;

	case MODE_GETNEXT:
		reqstr = "GETNEXT";
		break;

	case MODE_SET_RESERVE1:
	case MODE_SET_RESERVE2:
	case MODE_SET_COMMIT:       
	case MODE_SET_ACTION:       
		reqstr = "SET";
		break;

	default:
		reqstr = "UNKNOWN";
		break;
	}

	reqstr += " ";
	for (variable_list* vars = request->requestvb; vars; vars= vars->next_variable)
	{
		std::ostringstream reqOid;
		for(size_t i = 0; i < vars->name_length; ++i) 
		{
			reqOid << vars->name[i];
			if(i != vars->name_length-1)
				reqOid << '.';
		}
		char buf[2056]= "";
		std::string oidstr=  reqOid.str();

		sprintf(buf,"Oid:[%s],type:[%u]varlen[%d]nextVariable[%p]", 
			oidstr.c_str(), vars->type,vars->val_len, vars->next_variable);
		reqstr +=std::string(buf);

		unsigned char bitstring = 0;
		unsigned long high = 0;
		unsigned long low = 0;
		long interger = 0;
		oid objid = 0;
		unsigned char str = 0;

		if( NULL != vars->val.bitstring)
			bitstring = *(vars->val.bitstring);
		if( NULL != vars->val.counter64)
		{
			high = vars->val.counter64->high;
			low = vars->val.counter64->low;
		}
		if(NULL != vars->val.integer)
			interger = *(vars->val.integer);
		if(NULL != vars->val.objid)
			objid = *(vars->val.objid);
		if( NULL != vars->val.string)
			str = *(vars->val.string);

		memset(buf, 0, sizeof(buf));

		sprintf(buf,"val.bitstring[%p=%u],varsTemp->val.counter64[%p=high:%lld,low:%lld],varsTemp->val.integer[%p=%lld],varsTemp->val.objid[%p=%u]varsTemp->val.string[%p=%u] ", 
			vars->val.bitstring, bitstring, vars->val.counter64, high, low, vars->val.integer, interger, vars->val.objid, objid, vars->val.string, str);

		reqstr +=std::string(buf);
	}
	return reqstr;
}

int agentHandler(netsnmp_mib_handler* handler,
				 netsnmp_handler_registration* reginfo,
				 netsnmp_agent_request_info* reqinfo,
				 netsnmp_request_info* requests) 
{
	if( reginfo->my_reg_void == NULL) 
	{
		if( !loadConfig())
			return SNMP_ERR_RESOURCEUNAVAILABLE;

		reginfo->my_reg_void = &logger;

		MYLOG(Log::L_INFO, "SNMPAgent  handler[%p] snmpUdpBasePort[%d] g_snmpAgentPendTimeOut[%lld]", handler, g_snmpUdpBasePort, g_snmpAgentPendTimeOut);									 
	}

	std::string reqstrDesc = print_requests(requests);

	while(requests) 
	{
		variable_list* vars = requests->requestvb;

//		std::string reqStr = reqdesc(requests);

		MYLOG(Log::L_DEBUG, "processing request[%u]", requests->index);
		
		//parse the oid to get service ID		
		uint32_t svcId, procId, infoType, idx;
		if(!ZQSNMP::Util::parseOid(vars->name, vars->name_length, svcId, procId, infoType, idx)) 
		{
			MYLOG(Log::L_DEBUG, "faled to parse oid:name[%s],len[%d]", vars->name, vars->name_length);
			return SNMP_ERR_RESOURCEUNAVAILABLE;
		}

		MYLOG(Log::L_DEBUG, "request for service[%u] process[%u]", svcId, procId);

		// encode message
		size_t len = ZQSNMP_MSG_LEN_MAX;
		u_char buff[ZQSNMP_MSG_LEN_MAX] = {0};
		int32_t err = SNMP_ERR_NOERROR;
		u_char mode = ZQSNMP_PDU_UNKNOWN;

		switch(reqinfo->mode)
		{
		case MODE_GET:
			mode = ZQSNMP_PDU_GET;
			break;
		case MODE_GETNEXT:
			mode = ZQSNMP_PDU_GETNEXT;
			break;
		case MODE_SET_RESERVE1:
		case MODE_SET_RESERVE2:
		case MODE_SET_COMMIT:       
		case MODE_SET_ACTION:       
			mode = ZQSNMP_PDU_SET;
			break;
		default:
			break;
		}

		if (mode == ZQSNMP_PDU_UNKNOWN)
		{
			MYLOG(Log::L_ERROR, "unknown request PDU[%d]", reqinfo->mode);
			return SNMP_ERR_WRONGTYPE;
		}

		if(!ZQSNMP::Util::encodeMsg(buff, &len, mode, err, vars)) 
		{
			MYLOG(Log::L_ERROR, "failed to encode request");
			return SNMP_ERR_RESOURCEUNAVAILABLE;
		} 

		timeout_t        timeout        = g_snmpAgentPendTimeOut;
		size_t           reslen         = ZQSNMP_MSG_LEN_MAX; 
		int32            snmpServerPort = svcId + procId + g_snmpUdpBasePort;
		u_char           response[ZQSNMP_MSG_LEN_MAX] = {0};
		InetHostAddress  udpSvcAddress("127.0.0.1");
		UDPSocket        udpAgnet;

		udpAgnet.setCompletion(false);
		MYLOG(Log::L_DEBUG, "udpSvcAddress[%s] snmpServerPort[%d]: %d+%d+%d", udpSvcAddress.getHostAddress(), snmpServerPort, g_snmpUdpBasePort, svcId, procId);

		{
			MutexGuard guard(reqlock);
			if (!udpAgnet.isPending(Socket::pendingOutput, timeout))
			{
				MYLOG(Log::L_ERROR, "agent Output timeout[%d]", timeout);
				return SNMP_ERR_RESOURCEUNAVAILABLE;
			}

			MYLOG(Log::L_DEBUG, "forwarding query to %s/%d: %s", udpSvcAddress.getHostAddress(), snmpServerPort, reqstrDesc.c_str());
			
			InterActive      sendToSvc       = {0};
			unsigned long    requestLinePos  = (unsigned long) &((((struct InterActive*) 0)->_content).request);
			sendToSvc._head._agentRecvSeq  = g_agentRecvCount;
			memcpy((sendToSvc._content.request), buff, len);
			
			int sentState = udpAgnet.sendto(&sendToSvc, len + requestLinePos, udpSvcAddress, snmpServerPort);
			if (0 >= sentState)
			{
				MYLOG(Log::L_ERROR, "failed to send to udpSvcAddress[%s /%d] msgLen[%d], agentRecvCount[%d]", udpSvcAddress.getHostAddress(), snmpServerPort, len + requestLinePos, g_agentRecvCount);
				return SNMP_ERR_RESOURCEUNAVAILABLE;
			}

			if (!udpAgnet.isPending(Socket::pendingInput, timeout))
			{
				MYLOG(Log::L_ERROR, "timeout at waiting for response from snmpServerPort[%d] timeout[%d]", snmpServerPort, timeout);
				return SNMP_ERR_RESOURCEUNAVAILABLE;
			}

			InterActive    recvFromSvc     = {0};
			reslen = udpAgnet.receiveFrom(&recvFromSvc, sizeof(InterActive), udpSvcAddress, snmpServerPort);
			// MYLOG(Log::L_DEBUG, "received %d bytes from udpSvcAddress[%s / %d]", reslen, udpSvcAddress.getHostAddress(), snmpServerPort);

			if (0 >= reslen 
				|| (recvFromSvc._head._agentRecvSeq) != g_agentRecvCount
				|| 0 >= (reslen - requestLinePos))
			{
				MYLOG(Log::L_ERROR, "agent processing snmp udp server[%s], snmpServerPort[%d], agentRecvCount[%d] recv erro", udpSvcAddress.getHostAddress(), snmpServerPort, g_agentRecvCount);
				return SNMP_ERR_RESOURCEUNAVAILABLE;
			}

			++g_agentRecvCount;
			memcpy(response, (recvFromSvc._content.request), (reslen - requestLinePos));
		}

		MYLOG(Log::L_INFO, "received result from [%s/%d] agentRecvCount[%d] with len[%d], query: %s", udpSvcAddress.getHostAddress(), snmpServerPort, g_agentRecvCount, reslen, reqstrDesc.c_str());
		
		//decode message
		if(!ZQSNMP::Util::decodeMsg(response, reslen, &mode, &err, vars)) 
		{
			MYLOG(Log::L_ERROR, "failed to decode response from udpSvcAddress[%s / %d]  agentRecvCount[%d]", udpSvcAddress.getHostAddress(), snmpServerPort, g_agentRecvCount);
			return SNMP_ERR_RESOURCEUNAVAILABLE;
		}

		//std::string respStr = reqdesc(requests);
		//MYLOG(Log::L_INFO, "request[%s] response[%s]", reqStr.c_str(), respStr.c_str());

		requests = requests->next;
	}

	return SNMP_ERR_NOERROR;
}

void deinit_myAgent()
{
	unlink(AGENT_PIPE); 
}

bool loadConfig() 
{
	XMLPreferenceDocumentEx doc; 
	if(!doc.open(CONFIG)) 
	{
		syslog(LOG_ERR, "failed to load configuration from (%s)", CONFIG);
		return false;
	}

	XMLPreferenceEx* node = getPreferenceNode("SNMP", doc);
	if(!node) 
	{
		syslog(LOG_ERR, "failed to get log configuration");
		return false;
	}

	char value[256];
	bool res = false;
	std::istringstream is;
	memset(value, '\0', 256);

	res = node->getAttributeValue("SnmpUdpBasePort", value);
	if(res) 
	{
		std::istringstream snmpIs;
		const unsigned int portMaxLimit = 65535;
		snmpIs.str(value);
		snmpIs >> g_snmpUdpBasePort;
		g_snmpUdpBasePort = (g_snmpUdpBasePort < portMaxLimit) ? g_snmpUdpBasePort : 10000;
	}

	memset(value, '\0', 256);
	res = node->getAttributeValue("SnmpAgentPendTimeOut", value);
	if(res) 
	{
		std::istringstream snmpPendTime;
		snmpPendTime.str(value);
		snmpPendTime >> g_snmpAgentPendTimeOut;
		g_snmpAgentPendTimeOut = g_snmpAgentPendTimeOut > 0 ? g_snmpAgentPendTimeOut : 500;
	}

	uint32 loggingMask = 0;
	// loggingMask 
	memset(value, '\0', 256);
	if (node->getAttributeValue("loggingMask", value))
		loggingMask = atol(value);

	std::string logDir =".";

    // logPath 
	memset(value, '\0', 256);
	if (node->getAttributeValue("logPath", value))
		logDir = value;

	// fixup the configurations
	if (logDir.empty())
		logDir = "." FNSEPS;

	if (FNSEPC != logDir[logDir.length()-1])
		logDir += FNSEPC;

	// open the log file

	std::string fnLog = logDir + MODULE_NAME +".log";
	try{
		logger.open(fnLog.c_str(), loggingMask);
	}
	catch(...)
	{	
		syslog(LOG_ERR, "failed to create logfile");
		return false;
	}

	node->free();
	return true;
}

std::string print_requests(netsnmp_request_info* requests) 
{
	//#ifdef _DEBUG
	std::ostringstream req;
	req << "verb[";
	switch(requests->agent_req_info->mode) 
	{
	case MODE_GET:
		req << "GET";
		break;
	case MODE_GETNEXT:
		req << "GETNEXT";
		break;
	case MODE_SET_RESERVE1:
	case MODE_SET_RESERVE2:
	case MODE_SET_COMMIT:       
	case MODE_SET_ACTION:       
		req << "SET";
		break;
	default:
		req << "UNKNOWN";
		break;
	}
	req << "] vars: ";

	while(requests) 
	{
		variable_list* vars = requests->requestvb;
		for(size_t i = 0; i < vars->name_length; ++i) 
		{
			req << vars->name[i];
			if(i != vars->name_length-1)
				req << '.';
		}
		req << "; ";
		requests = requests->next;
	}
	
	return req.str();
}

