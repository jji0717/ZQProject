//#include "ERMICmds.h"
#include "EdgeRMEnv.h"
#include "ERMIClient.h"
#include "TianShanIceHelper.h"
#include "memory.h"
#include <ostream>
#define MOLOG		(_env._log)
#define MOEVENTLOG	(_env._eventlog)
const std::string XML_HEADER = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";

namespace ZQTianShan {
	namespace EdgeRM {

// -----------------------------
// class ClientCmds
// -----------------------------
ClientCmds::ClientCmds(EdgeRMEnv& env)
: ThreadRequest(env._thpool), _env(env)
{
}

ClientCmds::~ClientCmds()
{
}

// -----------------------------
// class ERMISessSetupCmd
// -----------------------------
///
ERMISessSetupCmd::ERMISessSetupCmd(EdgeRMEnv& env, std::string& clabClientSessId)
: ClientCmds(env), _clabClientSessId(clabClientSessId)
{

}

int ERMISessSetupCmd::run(void)
{

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ERMISessSetupCmd, "[SETUP]alloc[%s] send session setup message"), _clabClientSessId.c_str());

	::std::string strTransportResponse ="";
	TransportInfo _transPort;
	TianShanIce::EdgeResource::ProvisionPort provisionPort;
	//allocation provision 
	try
	{
		Ice::Identity ident;
		ident.name = _clabClientSessId;
		ident.category = DBFILENAME_Allocation;
		TianShanIce::EdgeResource::AllocationExPrx allocExPrx = IdentityToObjEnv2(_env, AllocationEx, ident);
		//get resource and post back
		TianShanIce::StatedObjInfo allocInfo = allocExPrx->getInfo();
		TianShanIce::SRM::ResourceMap allocResourceMap = allocExPrx->getResources();

		std::string qam_name, qam_zone, qam_destination, destination, client, modulation, qam_mac;
		Ice::Int port, symbolRate;
		Ice::Long channelId, pn, bandwidth;
		Ice::Byte modulationFormat, FEC;
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceName", qam_name);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceZone", qam_zone);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceIP", destination);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceMac", qam_mac);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "channelId", channelId);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "destPort", port);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "modulationFormat", modulationFormat);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "FEC", FEC);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "symbolRate", symbolRate);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtMpegProgram, "Id", pn);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtTsDownstreamBandwidth, "bandwidth", bandwidth);

		Message message;
		message.sessionType       = "unicast";   
		message.bit_rate          = bandwidth;
		message.source_address    = "";
		message.destination       = destination;
		message.destination_port  = port;
		message.multicast_address = "";
		message.rank = 1;

		_transPort.messages.push_back(message);

		char strQamDestination[128] = "";
		snprintf(strQamDestination, sizeof(strQamDestination) -1, "%lld.%lld", channelId, pn);
		_transPort.qamName = qam_name;
		_transPort.qam_Destination = strQamDestination;
		_transPort.clabClientSessId = _clabClientSessId;
		_transPort.encryptSess = false;
    }
	catch(::TianShanIce::BaseException& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ERMISessSetupCmd, "[SETUP]alloc[%s]failed to get resource info caught exception[category:%s][errorcode:%d][errorMsg:%s]"),
			_clabClientSessId.c_str(),ex.category.c_str(), ex.errorCode, ex.message.c_str());

		return 0;
	}
	catch(::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ERMISessSetupCmd, "[SETUP]alloc[%s]failed to get resource info caught exception[%s]"), 
			_clabClientSessId.c_str(), ex.ice_name().c_str());

		return 0;
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ERMISessSetupCmd, "[SETUP]alloc[%s]failed to get resource info caught unknow exception[%d]"), 
			_clabClientSessId.c_str(),SYS::getLastErr());

		return 0;
	}

	int64 lStart = ZQ::common::now();
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionSetupCmd, "[SETUP]alloc[%s] QamName[%s] setup"), _transPort.clabClientSessId.c_str(), _transPort.qamName.c_str());
	ERMISession::Ptr ermiSession = ERMISessionGroup::createSession(_transPort.clabClientSessId, _transPort.qamName.c_str());
	if(!ermiSession)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SessionSetupCmd, "[SETUP]alloc[%s] QamName[%s] failed to create ermisession"),  _transPort.clabClientSessId.c_str(), _transPort.qamName.c_str());
		return 0;
	}

	ERMIClient* ermiClient = ermiSession->getERMIClient();

	if(!ermiClient)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SessionSetupCmd, "[SETUP]alloc[%s] QamName[%s] failed to get eqam server connection info"),  _transPort.clabClientSessId.c_str(), _transPort.qamName.c_str());
		return 0;
	}

	std::string transport;
	char strbuf[2048];
	memset(strbuf, sizeof(strbuf), 0);
	snprintf(strbuf, sizeof(strbuf) -1, "clab-MP2T/DBVC/QAM;qam_name=%s;qam_destination=%s,", _transPort.qamName.c_str(), _transPort.qam_Destination.c_str());
	transport+= strbuf;

	for(int i = 0; i < _transPort.messages.size(); i++)
	{
		if(i != 0)
			transport+= ",";

		memset(strbuf, sizeof(strbuf), 0);
		if(_transPort.messages[i].sessionType == "unicast")
		{
		  snprintf(strbuf, sizeof(strbuf) -1, "clab-MP2T/DBVC/UDP;unicast;bit_rate=%lld;destination=%s;destination_port=%d", 
		  _transPort.messages[i].bit_rate,  _transPort.messages[i].destination.c_str(), _transPort.messages[i].destination_port);
		}
		else 
		{
			snprintf(strbuf, sizeof(strbuf) -1, "clab-MP2T/DBVC/UDP;multicast;bit_rate=%lld;source_adderss=%s;destination=%s;destination_port=%d;multicst_address=%s;rank=%d", 
				_transPort.messages[i].bit_rate, _transPort.messages[i].source_address.c_str(), _transPort.messages[i].destination.c_str(), _transPort.messages[i].destination_port,
				_transPort.messages[i].multicast_address.c_str(),_transPort.messages[i].rank);
		}

		transport+= strbuf;
	}

	RTSPMessage::AttrMap headers;
	MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_CONTENTTYPE,			"text/xml");
	MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_REQUIRE,				ERMI_HEADER_REQUIRE_VAL);
	MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_CLABCLIENTSESSIONID,	_transPort.clabClientSessId.c_str());
	MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_TRANSPORT,	         transport.c_str());
	MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_SESSIONGROUP,			_transPort.qamName.c_str());

	std::string SDP = "";
	if(_transPort.encryptSess)
	{
		std::ostringstream buf;
		buf << XML_HEADER ;
		buf << "<ermi:clab-ermi xmlns:ermi=\"urn:cablelabs:namespaces:DOCSIS:xsd:EQAM:ermi\"xmlns:xsi=\"http://wwww.w3.org/2001/XMLschema-instance\">\n";
		buf << "<ermi:EncryptionData>\n";
		buf << "<ermi:encryptSess>";buf<<(_transPort.encryptSess == true ? "true" : "false");buf <<"</ermi:encryptSess>\n";
		buf << "<ermi:casId>" ; buf<<_transPort.casId ; buf <<"</ermi:casId>\n";
		buf << "<ermi:clientMac>" ; buf<<_transPort.clientMac ; buf <<"</ermi:clientMac>\n";
		buf << "<ermi:cciLevel>" ; buf<<_transPort.cciLevel ; buf <<"</ermi:cciLevel>\n";
		buf << "<ermi:apsLevel>" ; buf<<_transPort.apsLevel ; buf <<"</ermi:apsLevel>\n";
		buf << "<ermi:CIT>" ; buf<<_transPort.CIT ; buf <<"</ermi:CIT>\n";
		buf << "<ermi:encryptESK>" ; buf<<_transPort.encryptESK ; buf <<"</ermi:encryptESK>\n";
		buf << "</ermi:EncryptionData>\n";
		buf << "</ermi:clab-ermi>\n";	
		SDP = buf.str();
	}

	int currentCSeq = ermiClient->sendSETUP(*ermiSession, SDP.c_str(), NULL, headers);

	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(SessionSetupCmd, "[SETUP]alloc[%s] QamName[%s] send setup message took %dms"),
		_transPort.clabClientSessId.c_str(), _transPort.qamName.c_str(), (int)(ZQ::common::now() - lStart));

	return 1;
}

// -----------------------------
// class ERMISessTearDownCmd
// -----------------------------
ERMISessTearDownCmd::ERMISessTearDownCmd(EdgeRMEnv& env, const std::string& clabClientSessId, const std::string& qamName, const std::string& sessionId)
: ClientCmds(env), _clabClientSessId(clabClientSessId), _qamName(qamName), _sessionId(sessionId)
{

}

int ERMISessTearDownCmd::run(void)
{
	int64 lStart = ZQ::common::now();
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionTearDownCmd, "[TEAR_DWON]alloc[%s]session[%s] QamName[%s] session teardown"),
		_clabClientSessId.c_str(),  _sessionId.c_str(), _qamName.c_str());
	ERMISession::Ptr ermiSession = ERMISessionGroup::openSession(_clabClientSessId, _qamName, true);
	if(!ermiSession)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SessionTearDownCmd, "[TEAR_DWON]alloc[%s]session[%s] QamName[%s] failed to find ermisession info"), 
			_clabClientSessId.c_str(),  _sessionId.c_str(), _qamName.c_str());
		return 0;
	}

	ermiSession->setSessionId(_sessionId);
	std::string baseURL = ermiSession->getBaseURL();
	ERMIClient* ermiClient = ermiSession->getERMIClient();

	if(!ermiClient)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SessionTearDownCmd, "[TEAR_DWON]alloc[%s]session[%s] QamName[%s] failed to get eqam server connection info"),
			_clabClientSessId.c_str(),  _sessionId.c_str(),_qamName.c_str());
		return 0;
	}
    
	RTSPMessage::AttrMap headers;
	MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_REQUIRE,				ERMI_HEADER_REQUIRE_VAL);
	MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_SESSION,				_sessionId.c_str());
	MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_CLABCLIENTSESSIONID,	_clabClientSessId.c_str());
	MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_CLABREASON,			"200 \"User stop\"");
	MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_SESSIONGROUP,			_qamName.c_str());

	int currentCSeq = ermiClient->sendTEARDOWN(*ermiSession, NULL, headers);

	MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SessionTearDownCmd, "[TEAR_DWON]alloc[%s]session[%s] QamName[%s] send session teardown message to EQAM [%s] took %dms"),
		_clabClientSessId.c_str(), _sessionId.c_str(), _qamName.c_str(), baseURL.c_str(),(int)(ZQ::common::now() - lStart));
	return 1;
}

////////////////////////////
//class R6SessPreSetupCmd
////////////////////////////
/*
R6SessPreSetupCmd::R6SessPreSetupCmd(EdgeRMEnv& env, std::string& clabClientSessId)
:ClientCmds(env),_clabClientSessId(clabClientSessId)
{
	
}
int R6SessPreSetupCmd::run(void)
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(R6SessPreSetupCmd, "[SETUP]alloc[%s] send session setup message"), _clabClientSessId.c_str());

	::std::string strTransportResponse ="";
	TransportInfo _transPort;
	TianShanIce::EdgeResource::ProvisionPort provisionPort;
	std::string qam_name, qam_zone, qam_destination, destination, client, modulation, qam_mac;
	Ice::Int port, symbolRate;
	Ice::Long channelId, pn, bandwidth;
	Ice::Byte modulationFormat, FEC;
	int32 startPort = 0,step = 0,startPN = 0,maxSessions = 0;
	//allocation provision 
	try
	{
		Ice::Identity ident;
		ident.name = _clabClientSessId;
		ident.category = DBFILENAME_Allocation;
		TianShanIce::EdgeResource::AllocationExPrx allocExPrx = IdentityToObjEnv2(_env, AllocationEx, ident);
		//get provision port info
		TianShanIce::EdgeResource::EdgeChannelExPrx edgeChannelExPrx = TianShanIce::EdgeResource::EdgeChannelExPrx::uncheckedCast(allocExPrx->getChannel());
		if(!edgeChannelExPrx)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(R6SessPreSetupCmd, "[SETUP]alloc[%s] get edge channel error"), _clabClientSessId.c_str());
			return 0;
		}
		provisionPort = edgeChannelExPrx->getProvisionPort();
		TianShanIce::StrValues  metaData;
		metaData.push_back(SYS_PROP(StartUDPPort));
		metaData.push_back(SYS_PROP(UdpPortStepByPn));
		metaData.push_back(SYS_PROP(StartProgramNumber));
		metaData.push_back(SYS_PROP(MaxSessions));
		TianShanIce::StatedObjInfo infos = edgeChannelExPrx->getInfo(metaData);
		::TianShanIce::Properties::iterator iter = infos.props.find(SYS_PROP(StartUDPPort));
		if(iter != infos.props.end())
			startPort = atoi(iter->second.c_str());
		iter = infos.props.find(SYS_PROP(UdpPortStepByPn));
		if(iter != infos.props.end())
			step = atoi(iter->second.c_str());
		iter = infos.props.find(SYS_PROP(StartProgramNumber));
		if(iter != infos.props.end())
			startPN = atoi(iter->second.c_str());
		iter = infos.props.find(SYS_PROP(MaxSessions));
		if(iter != infos.props.end())
			maxSessions = atoi(iter->second.c_str());

		MOLOG(ZQ::common::Log::L_INFO,CLOGFMT(R6SessPreSetupCmd,"[SETUP]alloc[%s]:startPort:%d,step:%d,startPN:%d,maxSessions:%d")
						, _clabClientSessId.c_str(),startPort,step,startPN,maxSessions);
		
		//get resource and post back
		TianShanIce::SRM::ResourceMap allocResourceMap = allocExPrx->getResources();
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceName", qam_name);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceZone", qam_zone);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceIP", destination);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceMac", qam_mac);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "channelId", channelId);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "destPort", port);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "modulationFormat", modulationFormat);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "FEC", FEC);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "symbolRate", symbolRate);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtMpegProgram, "Id", pn);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtTsDownstreamBandwidth, "bandwidth", bandwidth);

		Message message;
		message.qamDestination	= channelId + "." + pn;
		message.bandwidth		= bandwidth;
		message.qamName			= qam_name;
		message.qamClient		= "FFFFFFFFFFFF";
		message.udpDestination	= destination;
		message.clientPort		= port;
		message.source			= "0.0.0.0";
		message.serverPort		= 0;
		message.udpClient		= "FFFFFFFFFFFF";
		message.qamSessType		= "unicast";
		message.udpSessType		= "unicast";
		_transPort.message		= message;

	}
	catch(::TianShanIce::BaseException& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(R6SessPreSetupCmd, "[SETUP]alloc[%s]failed to get resource info caught exception[category:%s][errorcode:%d][errorMsg:%s]"),
			_clabClientSessId.c_str(),ex.category.c_str(), ex.errorCode, ex.message.c_str());

		return 0;
	}
	catch(::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(R6SessPreSetupCmd, "[SETUP]alloc[%s]failed to get resource info caught exception[%s]"), 
			_clabClientSessId.c_str(), ex.ice_name().c_str());

		return 0;
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(R6SessPreSetupCmd, "[SETUP]alloc[%s]failed to get resource info caught unknow exception[%d]"), 
			_clabClientSessId.c_str(),SYS::getLastErr());

		return 0;
	}

	_transPort.provisionPortMsg = provisionPort;
	_transPort.provisionPort	= 1;
	_transPort.onDemandSessionId = _clabClientSessId;

	int64 lStart = ZQ::common::now();
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(R6SessPreSetupCmd, "[SETUP]alloc[%s] QamName[%s] setup"), _transPort.onDemandSessionId.c_str(), _transPort.message.qamName.c_str());
	R6Session::Ptr r6Session = R6SessionGroup::createSession(_transPort.onDemandSessionId,qam_name.c_str());
	if(!r6Session)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(R6SessPreSetupCmd, "[SETUP]alloc[%s] QamName[%s] failed to create r6session"),  _transPort.onDemandSessionId.c_str(), _transPort.message.qamName.c_str());
		return 0;
	}
	R6Client*	r6Client = r6Session->getR6Client();
	if(!r6Client)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(R6SessPreSetupCmd, "[SETUP]alloc[%s] QamName[%s] failed to get eqam server connection info"),  _transPort.onDemandSessionId.c_str(), _transPort.message.qamName.c_str());
		return 0;
	}

	char strBuf[32];
	for(int i = 0;i < maxSessions;i++)
	{
		memset(strBuf,0,sizeof(strBuf));
		itoa(startPN,strBuf,10);
		std::string temp(strBuf);
		_transPort.message.qamDestination = channelId + "." + temp;
		_transPort.message.clientPort = startPort;
		startPN += 1;
		startPort += step;

		std::string transport;
		char qamStrbuf[2048];
		memset(qamStrbuf, sizeof(qamStrbuf), 0);
		snprintf(qamStrbuf, sizeof(qamStrbuf) -1, "MP2T/DBVC/QAM;%s;qam_destination=%s;bandwidth=%lld;qam_name=%s;client=%s,"
			,_transPort.message.qamSessType.c_str(),_transPort.message.qamDestination.c_str(),_transPort.message.bandwidth
			,_transPort.message.qamName.c_str(),_transPort.message.qamClient.c_str());
		char udpStrbuf[2048];
		memset(udpStrbuf,sizeof(udpStrbuf),0);
		snprintf(udpStrbuf,sizeof(udpStrbuf)-1,"MP2T/DVBC/UDP;%s;destination=%s;client_port=%d;source=%s;server_port=%d;client=%s,"
			,_transPort.message.udpSessType.c_str(),_transPort.message.udpDestination.c_str(),_transPort.message.clientPort
			,_transPort.message.source.c_str(),_transPort.message.serverPort,_transPort.message.udpClient.c_str());
		char reportTrafficMismatchStr[10];
		memset(reportTrafficMismatchStr,0,sizeof(reportTrafficMismatchStr));
		itoa(_transPort.provisionPortMsg.reportTrafficMismatch,reportTrafficMismatchStr,10);
		char jitterBufferStr[10];
		memset(jitterBufferStr,0,sizeof(jitterBufferStr));
		itoa(_transPort.provisionPortMsg.jitterBuffer,jitterBufferStr,10);
		RTSPMessage::AttrMap headers;
		MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_PROVISIONPORT,		R6_HEADER_PROVPORT_VAL);
		MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_REQUIRE,			R6_HEADER_REQUIRE_VAL);
		MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_TRANSPORT,			qamStrbuf);
		MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_TRANSPORT,			udpStrbuf);
		MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_REPTRAFFICMISMATCH,	reportTrafficMismatchStr);
		MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_INBANDMARKER,		_transPort.provisionPortMsg.inbandMarker);
		MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_JITTERBUFFER,		jitterBufferStr);
		MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_ONDEMANDSESSIONID,	_transPort.onDemandSessionId);

		int currentCSeq = r6Client->sendSETUP(*r6Session, NULL, NULL, headers);

		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(R6SessPreSetupCmd, "[SETUP]alloc[%s] QamName[%s] send setup message took %dms"),
			_transPort.onDemandSessionId.c_str(), _transPort.message.qamName.c_str(), (int)(ZQ::common::now() - lStart));
	}
	

	return 1;
}
*/

////////////////////////////
//class R6SessSetupCmd
///////////////////////////
R6ProvPortCmd::R6ProvPortCmd(EdgeRMEnv& env, std::string& clabClientSessId)
: ClientCmds(env), _clabClientSessId(clabClientSessId)
{
}

int R6ProvPortCmd::run(void)
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(R6SessSetupCmd, "[SETUP]alloc[%s] send session setup message"), _clabClientSessId.c_str());

	::std::string strTransportResponse ="";
	TransportInfo _transPort;
	TianShanIce::EdgeResource::ProvisionPort provisionPort;
	std::string qam_name, qam_zone, qam_destination, destination, client, modulation, qam_mac;
	Ice::Int port, symbolRate, startPort, step, startPN, maxSessions;
	Ice::Long channelId, pn, bandwidth;
	Ice::Byte modulationFormat, FEC;
	//allocation provision 
	try
	{
		Ice::Identity ident;
		ident.name = _clabClientSessId;
		ident.category = DBFILENAME_Allocation;
		TianShanIce::EdgeResource::AllocationExPrx allocExPrx = IdentityToObjEnv2(_env, AllocationEx, ident);
		//get provision port info
		TianShanIce::EdgeResource::EdgeChannelExPrx edgeChannelExPrx = TianShanIce::EdgeResource::EdgeChannelExPrx::uncheckedCast(allocExPrx->getChannel());
		if(!edgeChannelExPrx)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(R6SessSetupCmd, "[SETUP]alloc[%s] get edge channel error"), _clabClientSessId.c_str());
			return 0;
		}

		TianShanIce::StrValues  metaData;
		metaData.push_back(SYS_PROP(StartUDPPort));
		metaData.push_back(SYS_PROP(UdpPortStepByPn));
		metaData.push_back(SYS_PROP(StartProgramNumber));
		metaData.push_back(SYS_PROP(MaxSessions));
		//get udp port info
		TianShanIce::StatedObjInfo infos = edgeChannelExPrx->getInfo(metaData);
		::TianShanIce::Properties::iterator iter = infos.props.find(SYS_PROP(StartUDPPort));
		if(iter != infos.props.end())
			startPort = atoi(iter->second.c_str());
		iter = infos.props.find(SYS_PROP(UdpPortStepByPn));
		if(iter != infos.props.end())
			step = atoi(iter->second.c_str());
		iter = infos.props.find(SYS_PROP(StartProgramNumber));
		if(iter != infos.props.end())
			startPN = atoi(iter->second.c_str());
		iter = infos.props.find(SYS_PROP(MaxSessions));
		if(iter != infos.props.end())
			maxSessions = atoi(iter->second.c_str());

		provisionPort = edgeChannelExPrx->getProvisionPort();
		TianShanIce::StrValues strValue;
		//get resource and post back
		TianShanIce::StatedObjInfo allocInfo = allocExPrx->getInfo();
		TianShanIce::SRM::ResourceMap allocResourceMap = allocExPrx->getResources();
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceName", qam_name);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceZone", qam_zone);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceIP", destination);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceMac", qam_mac);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "channelId", channelId);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "destPort", port);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "modulationFormat", modulationFormat);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "FEC", FEC);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "symbolRate", symbolRate);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtMpegProgram, "Id", pn);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtTsDownstreamBandwidth, "bandwidth", bandwidth);

		Message message;
		char temp[120];
		memset(temp,0,sizeof(temp));
		itoa(channelId,temp,10);
		std::string strChannel(temp);
		memset(temp,0,sizeof(temp));
		itoa(pn,temp,10);
		std::string strPn(temp);
		message.qamDestination	= strChannel + "." + strPn;
		message.bandwidth		= bandwidth;
		message.qamName			= qam_name;
		message.qamClient		= "FFFFFFFFFFFF";
		message.udpDestination	= destination;
		message.clientPort		= port;
		message.source			= "0.0.0.0";
		message.serverPort		= 0;
		message.udpClient		= "FFFFFFFFFFFF";
		message.qamSessType		= "unicast";
		message.udpSessType		= "unicast";
		_transPort.message		= message;

	}
	catch(::TianShanIce::BaseException& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(R6SessSetupCmd, "[SETUP]alloc[%s]failed to get resource info caught exception[category:%s][errorcode:%d][errorMsg:%s]"),
			_clabClientSessId.c_str(),ex.category.c_str(), ex.errorCode, ex.message.c_str());

		return 0;
	}
	catch(::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(R6SessSetupCmd, "[SETUP]alloc[%s]failed to get resource info caught exception[%s]"), 
			_clabClientSessId.c_str(), ex.ice_name().c_str());

		return 0;
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(R6SessSetupCmd, "[SETUP]alloc[%s]failed to get resource info caught unknow exception[%d]"), 
			_clabClientSessId.c_str(),SYS::getLastErr());

		return 0;
	}
	//send message by loop, then invoke proPortComplete()

	_transPort.provisionPortMsg = provisionPort;
	if(provisionPort.enabled == 0)
		_transPort.provisionPort = 0;
	else
		_transPort.provisionPort = 1;
	_transPort.onDemandSessionId = _clabClientSessId;

	int64 lStart = ZQ::common::now();
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(R6SessSetupCmd, "[SETUP]alloc[%s] QamName[%s] setup"), _transPort.onDemandSessionId.c_str(), _transPort.message.qamName.c_str());
	R6Session::Ptr r6Session = R6SessionGroup::createSession(_clabClientSessId,qam_name.c_str());
	if(!r6Session)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(R6SessSetupCmd, "[SETUP]alloc[%s] QamName[%s] failed to create r6session"),  _transPort.onDemandSessionId.c_str(), _transPort.message.qamName.c_str());
		return 0;
	}

	R6Client*	r6Client = r6Session->getR6Client();
	if(!r6Client)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(R6SessSetupCmd, "[SETUP]alloc[%s] QamName[%s] failed to get eqam server connection info"),  _transPort.onDemandSessionId.c_str(), _transPort.message.qamName.c_str());
		return 0;
	}

	char strBuf[32];
	for(int i = 0;i < maxSessions;i++)
	{
		memset(strBuf,0,sizeof(strBuf));
		itoa(startPN,strBuf,10);
		std::string strPn(strBuf);
		memset(strBuf,0,sizeof(strBuf));
		itoa(channelId,strBuf,10);
		std::string strChannel(strBuf);
		_transPort.message.qamDestination = strChannel + "." + strPn;
		_transPort.message.clientPort = startPort;
		startPN += 1;
		startPort += step;

		std::string transport;
		char qamStrbuf[2048];
		memset(qamStrbuf, sizeof(qamStrbuf), 0);
		snprintf(qamStrbuf, sizeof(qamStrbuf) -1, "MP2T/DBVC/QAM;%s;qam_destination=%s;bandwidth=%lld;qam_name=%s;client=%s,"
					,_transPort.message.qamSessType.c_str(),_transPort.message.qamDestination.c_str(),_transPort.message.bandwidth
					,_transPort.message.qamName.c_str(),_transPort.message.qamClient.c_str());
		char udpStrbuf[2048];
		memset(udpStrbuf,sizeof(udpStrbuf),0);
		snprintf(udpStrbuf,sizeof(udpStrbuf)-1,"MP2T/DVBC/UDP;%s;destination=%s;client_port=%d;source=%s;server_port=%d;client=%s,"
					,_transPort.message.udpSessType.c_str(),_transPort.message.udpDestination.c_str(),_transPort.message.clientPort
					,_transPort.message.source.c_str(),_transPort.message.serverPort,_transPort.message.udpClient.c_str());
		char reportTrafficMismatchStr[10];
		memset(reportTrafficMismatchStr,0,sizeof(reportTrafficMismatchStr));
		itoa(_transPort.provisionPortMsg.reportTrafficMismatch,reportTrafficMismatchStr,10);
		char jitterBufferStr[10];
		memset(jitterBufferStr,0,sizeof(jitterBufferStr));
		itoa(_transPort.provisionPortMsg.jitterBuffer,jitterBufferStr,10);
		RTSPMessage::AttrMap headers;
		MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_PROVISIONPORT,		R6_HEADER_PROVPORT_VAL);
		MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_REQUIRE,			R6_HEADER_REQUIRE_VAL);
		MAPSET(RTSPMessage::AttrMap, headers, "Transport",					qamStrbuf);
		MAPSET(RTSPMessage::AttrMap, headers, "Transport#2",				udpStrbuf);
		if(_transPort.provisionPortMsg.enabled == 1)
		{
			MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_REPTRAFFICMISMATCH,	reportTrafficMismatchStr);
			MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_INBANDMARKER,		_transPort.provisionPortMsg.inbandMarker);
			MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_JITTERBUFFER,		jitterBufferStr);
		}
		MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_ONDEMANDSESSIONID,	_transPort.onDemandSessionId);

		int currentCSeq = r6Client->sendSETUP(*r6Session, NULL, NULL, headers);

		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(R6SessSetupCmd, "[SETUP]alloc[%s] QamName[%s] send setup message took %dms"),
			_transPort.onDemandSessionId.c_str(), _transPort.message.qamName.c_str(), (int)(ZQ::common::now() - lStart));

		if(i == maxSessions - 1)
		{
			r6Session->_lastRequest = LASTREQUEST_PROPORT;
			r6Session->_bProPortDone = true;
		}

	}
	
	return 1;
}

////////////////////////////
//class R6SessStartChecking
///////////////////////////
R6StartCheckCmd::R6StartCheckCmd(EdgeRMEnv& env, std::string& clabClientSessId, std::string& sessionId, std::string& sourceIP, ZQ::common::tpport_t sourcePort)
:ClientCmds(env),_clabClientSessId(clabClientSessId),_sessionId(sessionId),_sourceIP(sourceIP),_sourcePort(sourcePort)
{
}

int R6StartCheckCmd::run()
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(R6StartCheckCmd, "[SETUP]alloc[%s] send session start checking message"), _clabClientSessId.c_str());

	::std::string strTransportResponse ="";
	TransportInfo _transPort;
	TianShanIce::EdgeResource::ProvisionPort provisionPort;
	std::string qam_name, qam_zone, qam_destination, destination, client, modulation, qam_mac;
	Ice::Int port, symbolRate;
	Ice::Long channelId, pn, bandwidth;
	Ice::Byte modulationFormat, FEC;
	//allocation provision 
	try
	{
		Ice::Identity ident;
		ident.name = _clabClientSessId;
		ident.category = DBFILENAME_Allocation;
		TianShanIce::EdgeResource::AllocationExPrx allocExPrx = IdentityToObjEnv2(_env, AllocationEx, ident);
		//get provision port info
		TianShanIce::EdgeResource::EdgeChannelExPrx edgeChannelExPrx = TianShanIce::EdgeResource::EdgeChannelExPrx::uncheckedCast(allocExPrx->getChannel());
		if(!edgeChannelExPrx)
			return 0;
		provisionPort = edgeChannelExPrx->getProvisionPort();
		//get resource and post back
		TianShanIce::StatedObjInfo allocInfo = allocExPrx->getInfo();
		TianShanIce::SRM::ResourceMap allocResourceMap = allocExPrx->getResources();
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceName", qam_name);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceZone", qam_zone);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceIP", destination);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceMac", qam_mac);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "channelId", channelId);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "destPort", port);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "modulationFormat", modulationFormat);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "FEC", FEC);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "symbolRate", symbolRate);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtMpegProgram, "Id", pn);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtTsDownstreamBandwidth, "bandwidth", bandwidth);

		Message message;
		message.udpDestination	= destination;
		message.clientPort		= port;
		message.source			= _sourceIP;
		message.serverPort		= _sourcePort;
		message.udpClient		= "FFFFFFFFFFFF";
		message.udpSessType		= "unicast";
		_transPort.message		= message;

	}
	catch(::TianShanIce::BaseException& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(R6StartCheckCmd, "[SETUP]alloc[%s]failed to get resource info caught exception[category:%s][errorcode:%d][errorMsg:%s]"),
			_clabClientSessId.c_str(),ex.category.c_str(), ex.errorCode, ex.message.c_str());

		return 0;
	}
	catch(::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(R6StartCheckCmd, "[SETUP]alloc[%s]failed to get resource info caught exception[%s]"), 
			_clabClientSessId.c_str(), ex.ice_name().c_str());

		return 0;
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(R6StartCheckCmd, "[SETUP]alloc[%s]failed to get resource info caught unknow exception[%d]"), 
			_clabClientSessId.c_str(),SYS::getLastErr());

		return 0;
	}

	_transPort.provisionPortMsg = provisionPort;
	_transPort.onDemandSessionId = _clabClientSessId;

	int64 lStart = ZQ::common::now();
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(R6StartCheckCmd, "[SETUP]alloc[%s] QamName[%s] setup"), _transPort.onDemandSessionId.c_str(), qam_name.c_str());
	R6SessionGroup::Ptr r6SessGroup = R6SessionGroup::findSessionGroup(qam_name);
	if(!r6SessGroup)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(R6SessSetupCmd, "[SETUP]alloc[%s] QamName[%s] failed to find r6sessionGroup"),  _transPort.onDemandSessionId.c_str(), qam_name.c_str());
		return 0;
	}
	R6Session::Ptr		r6Session = r6SessGroup->lookupByOnDemandSessionId(_transPort.onDemandSessionId.c_str());
	if(!r6Session)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(R6StartCheckCmd, "[SETUP]alloc[%s] QamName[%s] failed to create ermisession"),  _transPort.onDemandSessionId.c_str(), qam_name.c_str());
		return 0;
	}

	R6Client*	r6Client = r6Session->getR6Client();

	if(!r6Client)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(R6StartCheckCmd, "[SETUP]alloc[%s] QamName[%s] failed to get eqam server connection info"),  _transPort.onDemandSessionId.c_str(), qam_name.c_str());
		return 0;
	}

	char transport[2048];
	memset(transport,sizeof(transport),0);
	snprintf(transport,sizeof(transport)-1,"MP2T/DVBC/UDP;%s;destination=%s;client_port=%d;source=%s;server_port=%d;client=%s,"
		,_transPort.message.udpSessType.c_str(),_transPort.message.udpDestination.c_str(),_transPort.message.clientPort
		,_transPort.message.source.c_str(),_transPort.message.serverPort,_transPort.message.udpClient.c_str());
	char reportTrafficMismatchStr[10];
	memset(reportTrafficMismatchStr,0,sizeof(reportTrafficMismatchStr));
	itoa(_transPort.provisionPortMsg.reportTrafficMismatch,reportTrafficMismatchStr,10);
	char jitterBufferStr[10];
	memset(jitterBufferStr,0,sizeof(jitterBufferStr));
	itoa(_transPort.provisionPortMsg.jitterBuffer,jitterBufferStr,10);

	RTSPMessage::AttrMap headers;
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_SESSION,			_sessionId);
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_STARTCHECKING,		"1");
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_REQUIRE,			R6_HEADER_REQUIRE_VAL);
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_TRANSPORT,			transport);
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_REPTRAFFICMISMATCH,	reportTrafficMismatchStr);
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_INBANDMARKER,		_transPort.provisionPortMsg.inbandMarker);
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_JITTERBUFFER,		jitterBufferStr);
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_ONDEMANDSESSIONID,	_transPort.onDemandSessionId);

	int currentCSeq = r6Client->sendSETUP(*r6Session, NULL, NULL, headers);

	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(R6StartCheckCmd, "[SETUP]alloc[%s] QamName[%s] send setup message took %dms"),
		_transPort.onDemandSessionId.c_str(), qam_name.c_str(), (int)(ZQ::common::now() - lStart));
	return 1;
}

////////////////////////////
//class R6StopCheckingCmd
////////////////////////////
R6StopCheckCmd::R6StopCheckCmd(EdgeRMEnv& env,const std::string& clabClientSessId,const std::string& sessionId)
:ClientCmds(env),_clabClientSessId(clabClientSessId),_sessionId(sessionId)
{

}

int R6StopCheckCmd::run()
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(R6StopCheckCmd, "[SETUP]alloc[%s] send session stop checking message"), _clabClientSessId.c_str());

	::std::string strTransportResponse ="";
	TransportInfo _transPort;
	TianShanIce::EdgeResource::ProvisionPort provisionPort;
	std::string qam_name, qam_zone, qam_destination, destination, client, modulation, qam_mac;
	Ice::Int port, symbolRate;
	Ice::Long channelId, pn, bandwidth;
	Ice::Byte modulationFormat, FEC;
	//allocation provision 
	try
	{
		Ice::Identity ident;
		ident.name = _clabClientSessId;
		ident.category = DBFILENAME_Allocation;
		TianShanIce::EdgeResource::AllocationExPrx allocExPrx = IdentityToObjEnv2(_env, AllocationEx, ident);
		//get provision port info
		TianShanIce::EdgeResource::EdgeChannelExPrx edgeChannelExPrx = TianShanIce::EdgeResource::EdgeChannelExPrx::uncheckedCast(allocExPrx->getChannel());
		if(!edgeChannelExPrx)
			return 0;
		provisionPort = edgeChannelExPrx->getProvisionPort();
		//get resource and post back
		TianShanIce::StatedObjInfo allocInfo = allocExPrx->getInfo();
		TianShanIce::SRM::ResourceMap allocResourceMap = allocExPrx->getResources();
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceName", qam_name);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceZone", qam_zone);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceIP", destination);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "edgeDeviceMac", qam_mac);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "channelId", channelId);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel, "destPort", port);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "modulationFormat", modulationFormat);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "FEC", FEC);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "symbolRate", symbolRate);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtMpegProgram, "Id", pn);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtTsDownstreamBandwidth, "bandwidth", bandwidth);

		Message message;
		message.udpDestination	= destination;
		message.clientPort		= port;
		message.source			= "0.0.0.0";
		message.serverPort		= 0;
		message.udpClient		= "FFFFFFFFFFFF";
		message.udpSessType		= "unicast";
		_transPort.message		= message;

	}
	catch(::TianShanIce::BaseException& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(R6StopCheckCmd, "[SETUP]alloc[%s]failed to get resource info caught exception[category:%s][errorcode:%d][errorMsg:%s]"),
			_clabClientSessId.c_str(),ex.category.c_str(), ex.errorCode, ex.message.c_str());

		return 0;
	}
	catch(::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(R6StopCheckCmd, "[SETUP]alloc[%s]failed to get resource info caught exception[%s]"), 
			_clabClientSessId.c_str(), ex.ice_name().c_str());

		return 0;
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(R6StopCheckCmd, "[SETUP]alloc[%s]failed to get resource info caught unknow exception[%d]"), 
			_clabClientSessId.c_str(),SYS::getLastErr());

		return 0;
	}

	_transPort.provisionPortMsg = provisionPort;
	_transPort.onDemandSessionId = _clabClientSessId;

	int64 lStart = ZQ::common::now();
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(R6StopCheckCmd, "[SETUP]alloc[%s] QamName[%s] setup"), _transPort.onDemandSessionId.c_str(), qam_name.c_str());
	R6SessionGroup::Ptr r6SessGroup = R6SessionGroup::findSessionGroup(qam_name);
	if(!r6SessGroup)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(R6SessSetupCmd, "[SETUP]alloc[%s] QamName[%s] failed to find r6sessionGroup"),  _transPort.onDemandSessionId.c_str(), qam_name.c_str());
		return 0;
	}
	R6Session::Ptr		r6Session = r6SessGroup->lookupByOnDemandSessionId(_transPort.onDemandSessionId.c_str());
	if(!r6Session)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(R6StopCheckCmd, "[SETUP]alloc[%s] QamName[%s] failed to create ermisession"),  _transPort.onDemandSessionId.c_str(), qam_name.c_str());
		return 0;
	}

	R6Client*	r6Client = r6Session->getR6Client();

	if(!r6Client)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(R6StopCheckCmd, "[SETUP]alloc[%s] QamName[%s] failed to get eqam server connection info"),  _transPort.onDemandSessionId.c_str(), qam_name.c_str());
		return 0;
	}

	char transport[2048];
	memset(transport,sizeof(transport),0);
	snprintf(transport,sizeof(transport)-1,"MP2T/DVBC/UDP;%s;destination=%s;client_port=%d;source=%s;server_port=%d;client=%s,"
		,_transPort.message.udpSessType.c_str(),_transPort.message.udpDestination.c_str(),_transPort.message.clientPort
		,_transPort.message.source.c_str(),_transPort.message.serverPort,_transPort.message.udpClient.c_str());
	char reportTrafficMismatchStr[10];
	memset(reportTrafficMismatchStr,0,sizeof(reportTrafficMismatchStr));
	itoa(_transPort.provisionPortMsg.reportTrafficMismatch,reportTrafficMismatchStr,10);
	char jitterBufferStr[10];
	memset(jitterBufferStr,0,sizeof(jitterBufferStr));
	itoa(_transPort.provisionPortMsg.jitterBuffer,jitterBufferStr,10);

	RTSPMessage::AttrMap headers;
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_SESSION,				_sessionId);
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_STOPCHECK,				R6_HEADER_STOPCHECKVAL);
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_REQUIRE,				R6_HEADER_REQUIRE_VAL);
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_TRANSPORT,				transport);
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_REPTRAFFICMISMATCH,		reportTrafficMismatchStr);
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_ONDEMANDSESSIONID,		_transPort.onDemandSessionId);

	int currentCSeq = r6Client->sendSETUP(*r6Session, NULL, NULL, headers);

	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(R6StopCheckCmd, "[SETUP]alloc[%s] QamName[%s] send setup message took %dms"),
		_transPort.onDemandSessionId.c_str(), qam_name.c_str(), (int)(ZQ::common::now() - lStart));

	r6Session->_lastRequest = LASTREQUEST_STOPCHECK;
	return 1;
}
////////////////////////////
//class R6SessTearDownCmd
////////////////////////////
R6SessTearDownCmd::R6SessTearDownCmd(EdgeRMEnv& env, const std::string& onDemandSessionId, const std::string& qamName, const std::string& sessionId,const std::string& reason)
: ClientCmds(env), _onDemandSessionId(onDemandSessionId), _qamName(qamName), _sessionId(sessionId),_reason(reason)
{

}

int R6SessTearDownCmd::run(void)
{
	int64 lStart = ZQ::common::now();
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(R6SessTearDownCmd, "[TEAR_DWON]alloc[%s]session[%s] QamName[%s] session teardown"),
		_onDemandSessionId.c_str(),  _sessionId.c_str(), _qamName.c_str());
	R6Session::Ptr r6Session = R6SessionGroup::openSession(_onDemandSessionId, _qamName, true);
	if(!r6Session)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(R6SessTearDownCmd, "[TEAR_DWON]alloc[%s]session[%s] QamName[%s] failed to find ermisession info"), 
			_onDemandSessionId.c_str(),  _sessionId.c_str(), _qamName.c_str());
		return 0;
	}

	r6Session->setSessionId(_sessionId);
	std::string baseURL = r6Session->getBaseURL();
	R6Client* r6Client = r6Session->getR6Client();

	if(!r6Client)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(R6SessTearDownCmd, "[TEAR_DWON]alloc[%s]session[%s] QamName[%s] failed to get eqam server connection info"),
			_onDemandSessionId.c_str(),  _sessionId.c_str(),_qamName.c_str());
		return 0;
	}

	RTSPMessage::AttrMap headers;
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_REQUIRE,				R6_HEADER_REQUIRE_VAL);
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_REASON,					_reason.c_str());
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_SESSION,				_sessionId.c_str());
	MAPSET(RTSPMessage::AttrMap, headers, R6_HEADER_ONDEMANDSESSIONID,		_onDemandSessionId.c_str());

	int currentCSeq = r6Client->sendTEARDOWN(*r6Session, NULL, headers);

	MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(R6SessTearDownCmd, "[TEAR_DWON]alloc[%s]session[%s] QamName[%s] send session teardown message to EQAM [%s] took %dms"),
		_onDemandSessionId.c_str(), _sessionId.c_str(), _qamName.c_str(), baseURL.c_str(),(int)(ZQ::common::now() - lStart));

	return 1;
}

}
}
