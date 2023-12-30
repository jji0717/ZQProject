#include "EdgeRMEnv.h"
#include "S6Handler.h"
#include "strHelper.h"
#include "TianShanIceHelper.h"
#include <algorithm>
#include <functional>
#include "Guid.h"
namespace ZQTianShan{
namespace EdgeRM{

#define S6StateLOGFMT(_C, _X) CLOGFMT(_C, "CONN["FMT64U"]SessionId[%s]OnDemandSess[%s] CSeq[%s]Method[%s] " _X), request->getCommunicator()->getCommunicatorId(), sessId.c_str(), onDemandSessionId.c_str(), strCSeq.c_str(), strMethod.c_str()

S6Handler::S6Handler(EdgeRMEnv& env, ::TianShanIce::EdgeResource::EdgeRMPrx &edgeRMPrx)
:_env(env),_edgeRMPrx(edgeRMPrx),_b1stSetupMsg(true)
{
#ifdef TestForNanJing
	srand((unsigned)time(NULL));
#endif
}

S6Handler::~S6Handler()
{

}

bool S6Handler::HandleMsg(ZQRtspCommon::IRtspReceiveMsg* receiveMsg, ZQRtspCommon::IRtspSendMsg* sendMsg)
{
	// validate the header Require
	::std::string require = receiveMsg->getHeader(NGOD_HEADER_REQUIRE);
	transform(require.begin(), require.end(), require.begin(),tolower);
	if (::std::string::npos == require.find("ngod.s6"))
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(_C, "CONN["FMT64U"]CSeq[%s] failed due to mismatched header Require[%s]"), receiveMsg->getCommunicator()->getCommunicatorId(), receiveMsg->getHeader(NGOD_HEADER_SEQ).c_str(), require.c_str());
		sendMsg->setStartline(ResponseBadRequest);
		sendMsg->post();
		return true;
	}

	// initialize the out-going Response Header Require
	sendMsg->setHeader(NGOD_HEADER_REQUIRE, "com.comcast.ngod.s6");

	switch(receiveMsg->getVerb())
	{
	case RTSP_MTHD_SETUP:
		{
			//first receive set up msg, change peer erm to standby work mode
		
			return doSetup(receiveMsg, sendMsg);
		}
		break;
	case RTSP_MTHD_TEARDOWN:
		{
			return doTeardown(receiveMsg, sendMsg);
		}
		break;
	case RTSP_MTHD_GET_PARAMETER:
		{
			return doGetParameter(receiveMsg, sendMsg);
		}
		break;
	case RTSP_MTHD_SET_PARAMETER:
		{
			return doSetParameter(receiveMsg, sendMsg);
		}
		break;
	case RTSP_MTHD_RESPONSE:
		{
			return procResponse(receiveMsg, sendMsg);
		}
		break;
	default:
		sendMsg->setStartline(ResponseMethodNotAllowed);
		sendMsg->post();
		return true;
	}	
}

bool S6Handler::doSetup(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6Handler, "CONN["FMT64U"]enter doSetup()"), request->getCommunicator()->getCommunicatorId());

	Ice::Long lStart = ZQTianShan::now();

	std::string strMethod = METHOD_SETUP;

	// step 1. read the incomming NGOD S6 SETUP request
	std::string strCSeq = request->getHeader(NGOD_HEADER_SEQ);
	std::string onDemandSessionId = request->getHeader(NGOD_HEADER_ONDEMANDSESSIONID);
	std::string sessId ="";

	response->setHeader(NGOD_HEADER_SEQ,               strCSeq.c_str());
	response->setHeader(NGOD_HEADER_ONDEMANDSESSIONID, onDemandSessionId.c_str());
	response->setHeader(NGOD_HEADER_MTHDCODE, METHOD_SETUP);

	// 1.1 parse and validate request parameters
	// session id,  odsessId
	::std::string strSessionGroup = request->getHeader(NGOD_HEADER_SESSIONGROUP);
	if (onDemandSessionId.empty() || strSessionGroup.empty())
	{
		envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "failed due to header onDemandSessionId[%s] or SessionGroup[%s] is empty, took %dms"),
			onDemandSessionId.c_str(), strSessionGroup.c_str(), (int)(ZQTianShan::now() - lStart));
		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s SETUP request processed, took %dms"), ResponseBadRequest, ZQTianShan::now() - lStart);
		response->setStartline(ResponseBadRequest);
		response->post();
		return true;
	}

	// about header Transport
	std::string strTransport = request->getHeader(NGOD_HEADER_TRANSPORT);
	TianShanIce::StrValues  transportList;
	ZQ::common::stringHelper::SplitString(strTransport, transportList, ",\r\n");
	if (transportList.empty())
	{
		envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "failed due to header Transport is empty, took %dms"), (int)(ZQTianShan::now() - lStart));
		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s SETUP request processed, took %dms"), ResponseBadRequest, ZQTianShan::now() - lStart);
		response->setStartline(ResponseBadRequest);
		response->post();
		return true;
	}

	TianShanIce::LValues bandwidth;
	TianShanIce::BValues modulationFormat;
	TianShanIce::Properties qam_name2Client;
	std::string qam_group="";
	::std::string strQamName = "";
	for (::TianShanIce::StrValues::iterator iter = transportList.begin(); iter != transportList.end(); iter++)
	{
		::TianShanIce::StrValues transportContent;
		::ZQ::common::stringHelper::SplitString(*iter, transportContent, ";");
		//::std::string strQamName = "";
		::std::string strClient = "";

		for (::TianShanIce::StrValues::iterator contentIter = transportContent.begin(); contentIter != transportContent.end(); contentIter++)
		{
			::TianShanIce::StrValues content;
			content = ::ZQ::common::stringHelper::split(*contentIter, '=');

			//not xx=xx
			if (content.size() < 2)
				continue;

			if (content[0] == "bandwidth")
				bandwidth.push_back(_atoi64(content[1].c_str()));
			else if (content[0] == "qam_group")
			{
				if(qam_group.size() > 0)
				{
					envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "content[qam_group] mutil value"));
					envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s SETUP request processed, took %dms"), ResponseBadRequest, ZQTianShan::now() - lStart);
					response->setStartline(ResponseBadRequest);
					response->post();
					return true;
				}
				qam_group = content[1];
			}
			else if (content[0] == "qam_name")
				strQamName = content[1];
			else if (content[0] == "client")
				strClient = content[1];
			else if (content[0] == "modulation")
				modulationFormat.push_back(modulationStr2Int(content[1]));
		}

		if (!strQamName.empty() && !strClient.empty())
		{
			qam_name2Client[strQamName] = strClient;
			//strQamName.clear();
			strClient.clear();
		}
	}

	try
	{
		if(_env._pErmInstanceSyncer)
		{
			int mode = _env._pErmInstanceSyncer->getInstanceMode();
			std::string netid = "";
			/*
			ZQTianShan::EdgeRM::ErmInstanceSyncer::PeerinfoMap peerInfoMap = _env._pErmInstanceSyncer->getPeerInfoMap();
			ZQTianShan::EdgeRM::ErmInstanceSyncer::PeerinfoMap::iterator peerInfoIter = peerInfoMap.find(netid);
			if(peerInfoIter == peerInfoMap.end())
			{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(S6Handler, "cant find netID[%s] by RouteName[%s]"), netid.c_str(), strQamName.c_str());
			}
			*/
			if(mode == ZQTianShan::EdgeRM::ErmInstanceSyncer::esm_Active)
			{
				if(_b1stSetupMsg)
				{
					_b1stSetupMsg = false;
					netid = pConfig.netId;
					Ice::ObjectPrx objPrx = NULL;
					ZQTianShan::EdgeRM::ErmInstanceSyncer::PeerinfoMap peerInfoMap = _env._pErmInstanceSyncer->getPeerInfoMap();
					ZQTianShan::EdgeRM::ErmInstanceSyncer::PeerinfoMap::iterator peerInfoIter = peerInfoMap.begin();
					if(peerInfoIter == peerInfoMap.end())
					{
						envlog(ZQ::common::Log::L_ERROR, CLOGFMT(doSetup, "cant find GERM in config file"));
						envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s SETUP request processed, took %dms"), ResponseInternalError, ZQTianShan::now() - lStart);
						response->setStartline(ResponseInternalError);
						response->post();
						return false;
					}
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(doSetup, "call GERM[%s] to change this EERM[%s] status to active"), peerInfoIter->second.endpoint.c_str(), pConfig.edgeRMEndpoint.c_str());
					TianShanIce::EdgeResource::EdgeRMPrx GERMPrx  = TianShanIce::EdgeResource::EdgeRMPrx::checkedCast(_env._adapter->getCommunicator()->stringToProxy(peerInfoIter->second.endpoint.c_str()));
					
					#if ICE_INT_VERSION / 100 >= 306
						objPrx = GERMPrx->ice_collocationOptimized(false);
					#else
						objPrx = GERMPrx->ice_collocationOptimization(false);
					#endif
					GERMPrx = TianShanIce::EdgeResource::EdgeRMPrx::uncheckedCast(objPrx);
					GERMPrx->forceBackupMode(netid, ZQTianShan::EdgeRM::ErmInstanceSyncer::esm_Active);
				}
			}
			else if(mode == ZQTianShan::EdgeRM::ErmInstanceSyncer::esm_Standby)
			{
				netid = _env._pErmInstanceSyncer->getNetId(strQamName);
				ZQTianShan::EdgeRM::ErmInstanceSyncer::PeerinfoMap& peerInfoMap = _env._pErmInstanceSyncer->getPeerInfoMap();
				ZQTianShan::EdgeRM::ErmInstanceSyncer::PeerinfoMap::iterator peerInfoIter = peerInfoMap.find(netid);
				if(peerInfoIter == peerInfoMap.end())
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(doSetup, "cant find netID[%s] by RouteName[%s]"), netid.c_str(), strQamName.c_str());
					envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s SETUP request processed, took %dms"), ResponseInternalError, ZQTianShan::now() - lStart);
					response->setStartline(ResponseInternalError);
					response->post();
					return false;
				}
				else
				{
					if(peerInfoIter->second.b1stS6Msg)
					{
						peerInfoIter->second.b1stS6Msg = false;
						_env._pErmInstanceSyncer->changeWorkForMode(netid, ZQTianShan::EdgeRM::ErmInstanceSyncer::esm_Standby);
					}
				}
			}
		}
	}
	catch (const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6Handler, "change remote erm work mode caught exception[%s]"), ex.ice_name().c_str());
		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s SETUP request processed, took %dms"), ResponseInternalError, ZQTianShan::now() - lStart);
		response->setStartline(ResponseInternalError);
		response->post();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(S6Handler, "change remote erm work mode caught unknown exception"));
		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s SETUP request processed, took %dms"), ResponseInternalError, ZQTianShan::now() - lStart);
		response->setStartline(ResponseInternalError);
		response->post();
	}
	


#ifndef TestForNanJing // faked performance
	//creat allocation and add resource 
	TianShanIce::EdgeResource::AllocationPrx allocPrx = NULL;
	std::string allocId;	
	Ice::Long lStartPrcess =  ZQTianShan::now();

	try{
		generateSessionID(sessId);

		TianShanIce::SRM::ResourceMap resRequirement;

		//add resource
		//add edge device name list
		TianShanIce::SRM::Resource allocResource;
		allocResource.status = TianShanIce::SRM::rsRequested;
		allocResource.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
		allocResource.resourceData.clear();
		/*
		// take the qam_group as the service group if it can be converted to integer
		if (!qam_group.empty())
		{
		int serviceGrp = ::atoi(qam_group.c_str());
		if (serviceGrp >0)
		{
		//TODO: insert the resource of ServiceGroup
		::TianShanIce::Variant var_sg;
		var_sg.type = ::TianShanIce::vtInts;
		var_sg.bRange = false;
		var_sg.ints.push_back(serviceGrp);
		MAPSET(::TianShanIce::ValueMap, allocResource.resourceData, "id",  var_sg);
		MAPSET(::TianShanIce::SRM::ResourceMap, resRequirement, TianShanIce::SRM::rtServiceGroup, allocResource);
		allocResource.resourceData.clear();
		}
		}

		//add RouteName
		if(!strQamName.empty())
		{
		::TianShanIce::Variant var_rn;
		var_rn.type = ::TianShanIce::vtStrings;
		var_rn.bRange = false;
		var_rn.strs.push_back(strQamName);
		MAPSET(::TianShanIce::ValueMap,allocResource.resourceData,"routeName",var_rn);
		MAPSET(::TianShanIce::SRM::ResourceMap, resRequirement, TianShanIce::SRM::rtServiceGroup, allocResource);
		allocResource.resourceData.clear();
		}
		*/

		if(!qam_name2Client.empty())
		{
			::TianShanIce::Variant var_edgeDeviceName;
			var_edgeDeviceName.type = ::TianShanIce::vtStrings;
			var_edgeDeviceName.bRange = false;
			var_edgeDeviceName.strs.clear();
			for (::TianShanIce::Properties::iterator iter = qam_name2Client.begin(); iter != qam_name2Client.end(); iter++)
				var_edgeDeviceName.strs.push_back(iter->first);
			MAPSET(::TianShanIce::ValueMap,allocResource.resourceData,"routeName",var_edgeDeviceName);
			MAPSET(::TianShanIce::SRM::ResourceMap, resRequirement, TianShanIce::SRM::rtServiceGroup, allocResource);
			allocResource.resourceData.clear();
		}

		// modulationFormat
		allocResource.resourceData.clear();
		::TianShanIce::Variant var_modulationFormat;
		var_modulationFormat.bRange = false;
		var_modulationFormat.type = ::TianShanIce::vtBin;
		var_modulationFormat.bin.clear();
		for (size_t i = 0; i < modulationFormat.size(); i++)
			var_modulationFormat.bin.push_back(modulationFormat[i]);
		MAPSET(::TianShanIce::ValueMap, allocResource.resourceData, "modulationFormat",  var_modulationFormat);
		MAPSET(::TianShanIce::SRM::ResourceMap, resRequirement, TianShanIce::SRM::rtAtscModulationMode, allocResource);

		// bandwidth
		allocResource.resourceData.clear();
		TianShanIce::Variant var_bandwidth;
		var_bandwidth.type = ::TianShanIce::vtLongs;
		var_bandwidth.bRange = false;
		var_bandwidth.lints.clear();
		::std::sort(bandwidth.begin(), bandwidth.end(), ::std::greater<Ice::Long>( ));
		for (::TianShanIce::LValues::iterator iter = bandwidth.begin(); iter != bandwidth.end(); iter++)
			var_bandwidth.lints.push_back(*iter);

		MAPSET(::TianShanIce::ValueMap, allocResource.resourceData, "bandwidth",  var_bandwidth);
		MAPSET(::TianShanIce::SRM::ResourceMap, resRequirement, TianShanIce::SRM::rtTsDownstreamBandwidth, allocResource);

		// step 2. calling ERM to create an allocation
		allocPrx = _edgeRMPrx->createAllocation(resRequirement, _env._s6AllocationLeaseMs, _env._allocOwnerPrx, sessId);
		allocId = allocPrx->getId();
		TianShanIce::EdgeResource::AllocationExPrx allocExPrx = TianShanIce::EdgeResource::AllocationExPrx::uncheckedCast(allocPrx);

		// step 2.1 apply the user's fields
		allocExPrx->setSessionGroup(strSessionGroup);
		allocExPrx->setOnDemandSessionId(onDemandSessionId);
		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "created allocation[%s] took %dms"), allocId.c_str(), ZQTianShan::now() - lStartPrcess);
	}
	catch(::TianShanIce::BaseException &ex)
	{
		envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "failed to create allocation object caught exception [category:%s][errorcode:%d][errorMsg:%s], took %dms"),
			ex.category.c_str(), ex.errorCode, ex.message.c_str(), (int)(ZQTianShan::now() - lStart));
		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s SETUP request processed, took %dms"), ResponseInternalError, ZQTianShan::now() - lStart);
		response->setStartline(ResponseInternalError);
		response->post();
		return true;
	}
	catch(::Ice::Exception &ex)
	{
		envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "failed to create allocation object caught exception [%s], took %dms"),ex.ice_name().c_str(), (int)(ZQTianShan::now() - lStart));
		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s SETUP request processed, took %dms"), ResponseInternalError, ZQTianShan::now() - lStart);
		response->setStartline(ResponseInternalError);
		response->post();
		return true;
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "failed to create allocation object caught exception err[%d], took %dms"), SYS::getLastErr(), (int)(ZQTianShan::now() - lStart));
		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s SETUP request processed, took %dms"), ResponseInternalError, ZQTianShan::now() - lStart);
		response->setStartline(ResponseInternalError);
		response->post();
		return true;
	}

	std::string strTransportResponse ="";
	try
	{
		//step 3. provision the allocation 
		lStartPrcess =  ZQTianShan::now();
		allocPrx->provision(0, true);
		envlog(ZQ::common::Log::L_DEBUG, S6StateLOGFMT(S6Handler, "allocation[%s] provision took %dms"), allocId.c_str(), ZQTianShan::now() - lStartPrcess);

		//step 4. provision the allocation 
		lStartPrcess =  ZQTianShan::now();
		allocPrx->serve();
		envlog(ZQ::common::Log::L_DEBUG, S6StateLOGFMT(S6Handler, "allocation[%s] serve took %dms"), allocId.c_str(), ZQTianShan::now() - lStartPrcess);

		//step 5. read resource parameters and compose the S6 response
		TianShanIce::StatedObjInfo allocInfo = allocPrx->getInfo();
		TianShanIce::SRM::ResourceMap allocResourceMap = allocPrx->getResources();

		std::string qam_name, qam_zone, qam_destination, destination, client, modulation, qam_mac;
		Ice::Int port, symbolRate;
		Ice::Long channelId, pn;
		Ice::Byte modulationFormat, FEC;

		//ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel,  "edgeDeviceName", qam_name);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtServiceGroup,       "routeName",        qam_name);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel,    "edgeDeviceZone",   qam_zone);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel,    "edgeDeviceIP",     destination);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel,    "edgeDeviceMac",    qam_mac);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel,    "channelId",        channelId);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtPhysicalChannel,    "destPort",         port);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "modulationFormat", modulationFormat);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "FEC",              FEC);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtAtscModulationMode, "symbolRate",       symbolRate);
		ZQTianShan::Util::getResourceData(allocResourceMap, TianShanIce::SRM::rtMpegProgram,        "Id",               pn);

		modulation = modulationInt2Str(modulationFormat);
		client = qam_name2Client[qam_name];

		char tempRes[2048]="";
		memset(tempRes, 0, 2048);
		sprintf(tempRes, "MP2T/DVBC/UDP;unicast;client=%s;qam_destination=%lld.%lld;destination=%s;client_port=%d;qam_name=%s;qam_group=%s;modulation=%s;symbolRate=%d;qam_mac=%s;qam_zone=%s",
			client.c_str(), channelId * 1000, pn, destination.c_str(), port, qam_name.c_str(),qam_zone.c_str(), modulation.c_str(), symbolRate, qam_mac.c_str(), qam_zone.c_str());

		strTransportResponse = tempRes;
	}
	catch(::TianShanIce::BaseException& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "allocation provision caught exception[category:%s][errorcode:%d][errorMsg:%s], took %dms"),
			ex.category.c_str(), ex.errorCode, ex.message.c_str(), (int)(ZQTianShan::now() - lStart));
		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s SETUP request processed, took %dms"), ResponseInternalError, ZQTianShan::now() - lStart);
		response->setStartline(ResponseInternalError);
		response->post();
		return true;
	}
	catch(::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "allocation provision resource caught exception[%s], took %dms"), ex.ice_name().c_str(), (int)(ZQTianShan::now() - lStart));
		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s SETUP request processed, took %dms"), ResponseInternalError, ZQTianShan::now() - lStart);
		response->setStartline(ResponseInternalError);
		response->post();
		return true;
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "allocation provision resource caught unknow exception[%d], took %dms"), SYS::getLastErr(), (int)(ZQTianShan::now() - lStart));
		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s SETUP request processed, took %dms"), ResponseInternalError, ZQTianShan::now() - lStart);
		response->setStartline(ResponseInternalError);
		response->post();
		return true;
	}
#else
	std::string strTransportResponse ="";
	try
	{	
		if(qam_name2Client.empty())
		{
			envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "missing qam_name, took %dms"), (int)(ZQTianShan::now() - lStart));
			response->setStartline(ResponseInternalError);
			response->post();
			return true;
		}

		TianShanIce::Properties::iterator iter = qam_name2Client.begin();
		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "qam_name[%s]"), iter->first.c_str());
		generateSessionID(sessId);

		Ice::Identity chIdent;
		chIdent.name = iter->first;
		chIdent.category = DBFILENAME_EdgeChannel;

		TianShanIce::EdgeResource::EdgeChannelExPrx chPrx = IdentityToObjEnv2(_env, EdgeChannelEx, chIdent);
		TianShanIce::EdgeResource::EdgePort  chEdgePort = chPrx->getEdgePort();

		TianShanIce::SRM::Resource resAtscModulationMode = chEdgePort.resAtscModulationMode;
		TianShanIce::SRM::Resource resPhysicalChannel = chEdgePort.resPhysicalChannel;

		std::string qam_name, qam_zone, qam_destination, destination, client, modulation, qam_mac;
		Ice::Int port, symbolRate = 6875000;
		Ice::Long channelId =400000, pn;
		Ice::Byte modulationFormat, FEC;

		if(!getPort(iter->first, port, pn, channelId))
		{
			TianShanIce::StrValues expectedMetaData;
			expectedMetaData.push_back(SYS_PROP(FreqRF));
			expectedMetaData.push_back(SYS_PROP(StartUDPPort));
			expectedMetaData.push_back(SYS_PROP(StartProgramNumber));
			expectedMetaData.push_back(SYS_PROP(UdpPortStepByPn));
			expectedMetaData.push_back(SYS_PROP(MaxSessions));

			TianShanIce::StatedObjInfo chStateOjb =  chPrx->getInfo(expectedMetaData);
			ChannelInfo chInfo;
			port = chInfo.startPort = atoi(chStateOjb.props[SYS_PROP(StartUDPPort)].c_str());
			pn = chInfo.startPN = atoi(chStateOjb.props[SYS_PROP(StartProgramNumber)].c_str());
			chInfo.maxSession = atoi(chStateOjb.props[SYS_PROP(MaxSessions)].c_str());
			chInfo.stepPortByPn = atoi(chStateOjb.props[SYS_PROP(UdpPortStepByPn)].c_str());
			chInfo.totalSession = 1;
			sscanf(chStateOjb.props[SYS_PROP(FreqRF)].c_str(), "%lld", &chInfo.frenquence);
			channelId = chInfo.frenquence;

			ZQ::common::MutexGuard gd(_lockChPort);
			MAPSET(ChToPort, _chToPort, iter->first, chInfo);
		}

		qam_name = iter->first.c_str();
		if(resPhysicalChannel.resourceData.find("edgeDeviceZone") != resPhysicalChannel.resourceData.end())
			qam_zone = resPhysicalChannel.resourceData["edgeDeviceZone"].strs[0];

		if(resPhysicalChannel.resourceData.find("edgeDeviceIP") != resPhysicalChannel.resourceData.end())
			destination = resPhysicalChannel.resourceData["edgeDeviceIP"].strs[0];

		if(resPhysicalChannel.resourceData.find("edgeDeviceMac") != resPhysicalChannel.resourceData.end())
			qam_mac = resPhysicalChannel.resourceData["edgeDeviceMac"].strs[0];

		if(resAtscModulationMode.resourceData.find("modulationFormat") != resAtscModulationMode.resourceData.end())
			modulationFormat = resAtscModulationMode.resourceData["modulationFormat"].bin[0];

		if(resAtscModulationMode.resourceData.find("FEC") != resAtscModulationMode.resourceData.end())
			FEC = resAtscModulationMode.resourceData["FEC"].bin[0];

		if(resAtscModulationMode.resourceData.find("symbolRate") != resAtscModulationMode.resourceData.end())
			symbolRate = resAtscModulationMode.resourceData["symbolRate"].ints[0];

		modulation = modulationInt2Str(modulationFormat);
		client = qam_name2Client[qam_name];
		char tempRes[2048]="";
		memset(tempRes, 0, 2048);
		sprintf(tempRes, "MP2T/DVBC/UDP;unicast;client=%s;qam_destination=%lld.%lld;destination=%s;client_port=%d;qam_name=%s;qam_group=%s;modulation=%s;symbolRate=%d;qam_mac=%s;qam_zone=%s",
			client.c_str(), channelId * 1000, pn, destination.c_str(), port, qam_name.c_str(),qam_zone.c_str(), modulation.c_str(), symbolRate, qam_mac.c_str(), qam_zone.c_str());
		strTransportResponse = tempRes;
		/*
		#define RANGE_MIN (10)
		#define RANGE_AVG (35)

		int r=0;
		for (int i =0; i<12; i++)
		r += rand() % ((RANGE_AVG -10) *(RANGE_AVG -10) *4);
		r/=12; r =sqrt((double)r);

		Sleep((10 + r - int(ZQ::common::now() - lStart)));
		*/

		if(ZQ::common::now() - lStart < 10)
			Sleep((10 + rand() % 50 - int(ZQ::common::now() - lStart)));
	}
	catch(::TianShanIce::BaseException& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "allocation provision caught exception[category:%s][errorcode:%d][errorMsg:%s], took %dms"),
			ex.category.c_str(), ex.errorCode, ex.message.c_str(), (int)(ZQTianShan::now() - lStart));
		response->setStartline(ResponseInternalError);
		response->post();
		return true;
	}
	catch(::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "allocation provision resource caught exception[%s], took %dms"), ex.ice_name().c_str(), (int)(ZQTianShan::now() - lStart));
		response->setStartline(ResponseInternalError);
		response->post();
		return true;
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "allocation provision resource caught unknow exception[%d], took %dms"), SYS::getLastErr(), (int)(ZQTianShan::now() - lStart));
		response->setStartline(ResponseInternalError);
		response->post();
		return true;
	}
#endif
	//compose the response
	response->setStartline(ResponseOK);
	response->setHeader(NGOD_HEADER_SESSION, sessId.c_str());
	response->setHeader(NGOD_HEADER_EMBEDENCRYPT, "NO");
	response->setHeader(NGOD_HEADER_TRANSPORT, strTransportResponse.c_str());
	// step 6. send the response
	response->post();
	envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s SETUP request processed, took %dms"), ResponseOK, ZQTianShan::now() - lStart);

	// step 7. keep the connection id into session group
	S6Connection::Ptr s6Connection = S6Connection::OpenS6Connection(strSessionGroup, _env);
	if(s6Connection)
		s6Connection->add(sessId, request->getCommunicator());

	return true;
}

bool S6Handler::doTeardown(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6Handler, "CONN["FMT64U"] enter doTeardown()"), request->getCommunicator()->getCommunicatorId());

	Ice::Long lStart = ZQTianShan::now();
	::std::string strMethod = METHOD_TEARDOWN;

	// step 1. read the incomming NGOD S6 TEARDOWN request
	::std::string strCSeq = request->getHeader(NGOD_HEADER_SEQ);
	::std::string onDemandSessionId = request->getHeader(NGOD_HEADER_ONDEMANDSESSIONID);
	::std::string sessId = request->getHeader(NGOD_HEADER_SESSION);

	//set session id header
	response->setHeader(NGOD_HEADER_SESSION,           sessId.c_str());
	response->setHeader(NGOD_HEADER_ONDEMANDSESSIONID, onDemandSessionId.c_str());
	response->setHeader(NGOD_HEADER_SEQ,               strCSeq.c_str());
	response->setHeader(NGOD_HEADER_MTHDCODE,          METHOD_TEARDOWN);

	if (sessId.size() <1)
	{
		envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "failed due to missed header Session, took %dms"), (int)(ZQTianShan::now() - lStart));
		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s TEARDOWN request processed, took %dms"), ResponseBadRequest, ZQTianShan::now() - lStart);
		response->setStartline(ResponseBadRequest);
		response->post();
		return true;
	}

	// destroy alloction object
	try
	{	
		// for S6, Allocation OwnerKey takes the RTSP sessionID
		std::vector<Ice::Identity> idents = _env._idxOwnerOfAllocationEx->find(sessId);
		if(idents.empty())
		{
			envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "failed to find allocation object, took %dms"), (int)(ZQTianShan::now() - lStart));
			envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s TEARDOWN request processed, took %dms"), ResponseSessionNotFound, ZQTianShan::now() - lStart);
			response->setStartline(ResponseSessionNotFound);
			response->post();
			return true;
		}

		TianShanIce::EdgeResource::AllocationExPrx allocPrx = IdentityToObjEnv2(_env, AllocationEx, idents[0]);

		std::string strSessionGroup = allocPrx->getSessionGroup();

		S6Connection::Ptr s6Connection = S6Connection::OpenS6Connection(strSessionGroup, _env);
		if(s6Connection)
			s6Connection->remove(sessId);

		envlog(ZQ::common::Log::L_DEBUG, S6StateLOGFMT(S6Handler, "removing Allocation"));

		try
		{	
			allocPrx->destroy();
		}
		catch(const Ice::Exception&)
		{
		}
		catch(...)
		{
		}

		//post back
		response->setStartline(ResponseOK);
		response->post();

		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "%s TEARDOWN request processed, took %dms"), ResponseOK, ZQTianShan::now() - lStart);
	}
	catch(const TianShanIce::BaseException& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "destroy allocation caught exception[%s:%d:%s], took %dms"),
			ex.category.c_str(), ex.errorCode, ex.message.c_str(), (int)(ZQTianShan::now() - lStart));
		response->setStartline(ResponseInternalError);
		response->post();
		return true;
	}
	catch(const Ice::ObjectNotExistException& ex)
	{
		envlog(ZQ::common::Log::L_DEBUG, S6StateLOGFMT(S6Handler, "failed to get S6Allocation object caught exception[%s], took %dms"), ex.ice_name().c_str(), (int)(ZQTianShan::now() - lStart));
		response->setStartline(ResponseSessionNotFound);
		response->post();
		return true;
	}
	catch(const Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_WARNING, S6StateLOGFMT(S6Handler,"destroy allocation object caught exception[%s], took %dms"), 
			ex.ice_name().c_str(), (int)(ZQTianShan::now() - lStart));
		response->setStartline(ResponseInternalError);
		response->post();
		return true;
	}

	return true;
}

bool S6Handler::doGetParameter(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6Handler, "CONN["FMT64U"]enter GetParameter()"), request->getCommunicator()->getCommunicatorId());

	Ice::Long lStart = ZQTianShan::now();

	std::string  strMethod = METHOD_GETPARAMETER;
	std::string onDemandSessionId = "";

	//handle NGOD S6 message
	std::string strCSeq = request->getHeader(NGOD_HEADER_SEQ);
	std::string sessId = request->getHeader(NGOD_HEADER_SESSION);
	std::string strContent = request->getContent();

	//set session id header
	response->setHeader(NGOD_HEADER_SEQ, strCSeq.c_str());
	response->setHeader(NGOD_HEADER_MTHDCODE, METHOD_GETPARAMETER);

	// if the request is session ping
	if (sessId.size() > 0  && (strContent.size() == 0 || strContent.size() == 2))// 2 && strContent =="\r\n")
	{
		// session-oriented GET_PARAMETER
		ZQTianShan::IdentCollection idents;

		try
		{
			idents = _env._idxOwnerOfAllocationEx->find(sessId);
		}
		catch(...)
		{}

		if (idents.empty())
		{
			envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "failed to find alloction object, took %dms"), (int)(ZQTianShan::now() - lStart));
			response->setStartline(ResponseSessionNotFound);
			response->post();
			return true;
		}

		// for (ZQTianShan::IdentCollection::iterator it = idents.begin(); it < idents.end(); it++) {
		ZQTianShan::IdentCollection::iterator it = idents.begin();

		try {
			TianShanIce::EdgeResource::AllocationExPrx allocExPrx = IdentityToObjEnv2(_env, AllocationEx, *it);

			std::string strSessionGroup = allocExPrx->getSessionGroup();
			onDemandSessionId = allocExPrx->getOnDemandSessionId();
			allocExPrx->renew(_env._s6AllocationLeaseMs);
			allocExPrx->setRetrytimes(0);
			envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "renew allocation[%s] with %lld(ms)"), it->name.c_str(), _env._s6AllocationLeaseMs);

			//update session ConnectionId
			S6Connection::Ptr s6Connection = S6Connection::OpenS6Connection(strSessionGroup, _env);
			if(s6Connection)
				s6Connection->update(sessId, request->getCommunicator());
		}
		catch(Ice::ObjectNotExistException& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "failed to update expiration caught exception[%s], took %dms"), ex.ice_name().c_str(), (int)(ZQTianShan::now() - lStart));
			response->setStartline(ResponseSessionNotFound);
			response->post();
			return true;
		}
		catch(Ice::Exception &ex)
		{
			envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "failed to update expiration caught exception[%s], took %dms"), ex.ice_name().c_str(), (int)(ZQTianShan::now() - lStart));
			response->setStartline(ResponseInternalError);
			response->post();
			return true;
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "failed to update expiration caught unknown exception(%d), took %dms"), SYS::getLastErr(), (int)(ZQTianShan::now() - lStart));
			response->setStartline(ResponseInternalError);
			response->post();
			return true;
		}

		response->setStartline(ResponseOK);
		response->setHeader(NGOD_HEADER_SESSION, sessId.c_str());
		response->post();

		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "doGetParameter() processed for sess[%s], took %dms"), sessId.c_str(), ZQTianShan::now() - lStart);
		return true;
	} // end of session-oriented GET_PARAMETER

	// Non-session-oriented GET_PARAMETER request: session_list, connection_timeout, servicegroup
	try
	{
		::TianShanIce::StrValues contents;
		::ZQ::common::stringHelper::SplitString(strContent, contents, " \r\n\t", " \r\n\t");

		if(contents.size() <1)
		{
			envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "Bad request, no query-for in content body, took %dms"), (int)(ZQTianShan::now() - lStart));
			response->setStartline(ResponseBadRequest);
			response->post();
			return true;
		}

		::std::string strContents = "";
		for (size_t i = 0; i < contents.size(); i++)
		{
			if (contents[i] == "connection_timeout")
			{
				strContents += ::std::string("connection_timeout: 60\r\n");
				continue;
			}

			if (contents[i] == "sessionGroups")
			{
				std::vector< std::string > sessGroups = S6Connection::getAllS6Connection();
				strContents += ::std::string("sessionGroups:");
				for (std::vector< std::string >::iterator itorSGs = sessGroups.begin(); itorSGs != sessGroups.end(); itorSGs++)
				{
					strContents += (::std::string(" ") + *itorSGs);
				}
				strContents += "\r\n";
				continue;
			}

			if (contents[i] == "session_list")
			{
				::TianShanIce::StrValues sessionGroups;
				::std::string strSessionGroup = request->getHeader(NGOD_HEADER_SESSIONGROUP);
				if (!strSessionGroup.empty())
				{	
					::ZQ::common::stringHelper::SplitString(strSessionGroup, sessionGroups, " \r\n"); 
				}

				envlog(ZQ::common::Log::L_DEBUG, S6StateLOGFMT(S6Handler, "get session list from group: %s"), strSessionGroup.c_str());
				strContents += ::std::string("session_list:");
				::Freeze::EvictorIteratorPtr itObjId;
				envlog(ZQ::common::Log::L_DEBUG, S6StateLOGFMT(S6Handler, "check SessionGroup from DB"));
				for (itObjId = _env._eAllocation->getIterator("", _env._allocationEvictorSize); itObjId && itObjId->hasNext(); )
				{
					Ice::Identity objId = itObjId->next();
					try {
						TianShanIce::EdgeResource::AllocationExPrx allocExPrx = IdentityToObjEnv2(_env, AllocationEx, objId);

						std::string strSessionGroup = allocExPrx->getSessionGroup();
						if(strSessionGroup.empty())
							continue;

						std::string onDemandSessionId = allocExPrx->getOnDemandSessionId();
						std::string sessionId = allocExPrx->getOwnerKey();
						if(sessionGroups.size() < 1)
						{
							strContents += (::std::string(" ") + sessionId + ":" + onDemandSessionId);
							strContents += "\r\n";
						}
						else
						{
							if (find(sessionGroups.begin(), sessionGroups.end(),strSessionGroup) != sessionGroups.end())
							{
								strContents += (::std::string(" ") + sessionId + ":" + onDemandSessionId);
								strContents += "\r\n";
							}
						}
					}
					catch (const ::Ice::Exception& ex)
					{
						envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "caught exception[%s]"), ex.ice_name().c_str());
					}
					catch(...)
					{
						envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "caught unknown exception"));
					}
				} // session_list	
				continue;
			} // end of for each contents

			// unrecognized query-for, reject via BadRequest
			response->setStartline(ResponseBadRequest);
			response->post();
			envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "BadRequest due to unknown parameter[] is queried, took %dms"), contents[i].c_str(), ZQTianShan::now() - lStart);
			return true;
		}

		// succeeded response here
		response->setStartline(ResponseOK);
		response->setContent(strContents.c_str());
		response->post();
		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "request processed(%dms, success)"), ZQTianShan::now() - lStart);
	}
	catch(::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "caught Ice exception[%s], took %dms"), ex.ice_name().c_str(), (int)(ZQTianShan::now() - lStart));
		response->setStartline(ResponseInternalError);
		response->post();
		return true;
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "caught unknown exception(%d), took %dms"), SYS::getLastErr(), (int)(ZQTianShan::now() - lStart));
		response->setStartline(ResponseInternalError);
		response->post();
		return true;
	}

	envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "Leave GetParameter() took %d ms"), ZQTianShan::now() - lStart);
	return true;
}

bool S6Handler::doSetParameter(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6Handler, "CONN["FMT64U"]enter SetParameter()"), request->getCommunicator()->getCommunicatorId());

	Ice::Long lStart = ZQTianShan::now();

	::std::string strMethod = METHOD_SETPARAMETER;
	::std::string sessId = "";
	::std::string onDemandSessionId = "";

	//handle NGOD S6 message
	::std::string strCSeq = request->getHeader(NGOD_HEADER_SEQ);
	//set session id header
	response->setHeader(NGOD_HEADER_SEQ, strCSeq.c_str());
	response->setHeader(NGOD_HEADER_MTHDCODE, METHOD_SETPARAMETER);

	try
	{
		std::string strContent = request->getContent();
		TianShanIce::StrValues strSessionGroups;

		ZQ::common::stringHelper::SplitString(strContent, strSessionGroups, ": \t\r\n", " \r\n");

		if (strSessionGroups.size() > 0 && strSessionGroups[0] == "sessionGroups:")
		{
			ZQ::DataPostHouse::IDataCommunicatorPtr connectId = request->getCommunicator();
			for (size_t i = 1; i < strSessionGroups.size() && !strSessionGroups[i].empty(); i++)
			{
				S6Connection::Ptr s6Connection = S6Connection::OpenS6Connection(strSessionGroups[i], _env);
				if(s6Connection)
					s6Connection->updateS6ConnectionId(connectId);
			}

			response->setStartline(ResponseOK);
			response->post();
		}
		else
		{
			response->setStartline(ResponseBadRequest);
			response->post();
		}

		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "request processed, took %dms"), ZQTianShan::now() - lStart);
		return true;
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "caught exception err[%d], took %dms"), SYS::getLastErr(), ZQTianShan::now() - lStart);
		response->setStartline(ResponseInternalError);
		response->post();
		return true;
	}
}

bool S6Handler::procResponse(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6Handler, "CONN["FMT64U"]enter Response()"), request->getCommunicator()->getCommunicatorId());

	Ice::Long lStart = ZQTianShan::now();
	::std::string strMethod = "RESPONSE";

	//handle NGOD S6 message
	::std::string onDemandSessionId = request->getHeader(NGOD_HEADER_ONDEMANDSESSIONID);
	::std::string sessId = request->getHeader(NGOD_HEADER_SESSION);
	::std::string strCSeq = request->getHeader(NGOD_HEADER_SEQ);
	::std::string startline = request->getStartline();

	envlog(ZQ::common::Log::L_DEBUG, S6StateLOGFMT(S6Handler, "startline(%s)"), startline.c_str());

	if (sessId.size()< 1 || onDemandSessionId.size() <1)
	{
		envlog(ZQ::common::Log::L_WARNING, S6StateLOGFMT(S6Handler, "ignore Response due to no Session or OnDemandSessionId specified"));
		return true;
	}

	::TianShanIce::StrValues responseCodes;
	::ZQ::common::stringHelper::SplitString(startline, responseCodes, " \r\n"); 

	if (responseCodes.size() < 2)
	{
		envlog(ZQ::common::Log::L_WARNING, S6StateLOGFMT(S6Handler, "ignore Response, missing response code or reason"));
		return true;
	}

	switch (atoi(responseCodes[1].c_str()))
	{
	case 200:
		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "session in progress"));
		break;

	case 404:
	case 454:
		envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "session not exists, destroy allocation"));
		try
		{
			std::vector<Ice::Identity> idents = _env._idxOwnerOfAllocationEx->find(sessId);
			if(idents.empty())
			{
				envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "failed to get allocation object"));
				response->setStartline(ResponseInternalError);
				response->post();
				return true;
			}
			TianShanIce::EdgeResource::AllocationExPrx allocPrx = IdentityToObjEnv2(_env, AllocationEx, idents[0]);

			std::string strSessionGroup = allocPrx->getSessionGroup();

			S6Connection::Ptr s6Connection = S6Connection::OpenS6Connection(strSessionGroup, _env);
			if(s6Connection)
				s6Connection->remove(sessId);

			envlog(ZQ::common::Log::L_DEBUG, S6StateLOGFMT(S6Handler,"free Allocation"));

			try
			{	
				allocPrx->destroy();
			}
			catch(const Ice::Exception&)
			{
			}
			catch(...)
			{
			}
		}
		catch(Ice::Exception &ex)
		{
			envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "failed to get S6Allocation object caught exception[%s]"), ex.ice_name().c_str());
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_ERROR, S6StateLOGFMT(S6Handler, "failed to get S6Allocation object caught unknown exception(%d)"), SYS::getLastErr());
		}

	default:
		break; // do nothing
	}

	envlog(ZQ::common::Log::L_INFO, S6StateLOGFMT(S6Handler, "Leave Response() took %d ms"), ZQTianShan::now() - lStart);

	return true;
}

void S6Handler::onCommunicatorError(ZQ::DataPostHouse::IDataCommunicatorPtr communicator)
{
	//update sessionGroup connectID

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6Handler, "CONN["FMT64U"][0x%08x]on Communicator Error "), communicator->getCommunicatorId(), communicator.get());
	std::vector<std::string> s6connections = S6Connection::getAllS6Connection();
	for(std::vector<std::string>::iterator itorSG = s6connections.begin(); itorSG != s6connections.end(); itorSG++)
	{
		S6Connection::Ptr s6connectionPtr = S6Connection::OpenS6Connection(*itorSG, _env, false);
		if(s6connectionPtr)
		{
			s6connectionPtr->connectionError(communicator);
		}
	}

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6Handler, "CONN["FMT64U"][0x%08x]leave onCommunicatorError()"), communicator->getCommunicatorId(), communicator.get());
}

void S6Handler::generateSessionID(std::string& sessionID)
{
	char buf[256];

	time_t timet;
	struct tm* pstm;
	uint32 id = 0;
	static uint32 sLastLWord = 0;

	// TODO: partition RTSP session Id by Edge ERM netId

	ZQ::common::MutexGuard gd(_genIdCritSec);
	{
		timet = time(0);
		pstm = gmtime(&timet);
		if(pstm)
			id =(uint32)pstm->tm_hour << 27 | 
			(uint32)pstm->tm_min << 21 | 
			(uint32)pstm->tm_sec << 15;

		id = id | (pstm->tm_mday<<10 )| (sLastLWord ++);
		if(sLastLWord > ( 1 << 10 ) )
			sLastLWord = 0;
	}

#ifdef _RTSP_PROXY
	if ( GAPPLICATIONCONFIGURATION.lUseLongSessionId >= 1 )
	{
		sprintf(buf, "9%011u",  id);
	}
	else
#endif
		sprintf(buf, "%u",  id);

	sessionID = buf;
}
#ifdef  TestForNanJing
bool S6Handler::getPort(std::string chName, int& port, Ice::Long& pn, int64& frenquence)
{
	ZQ::common::MutexGuard gd(_lockChPort);
	if(_chToPort.find(chName) == _chToPort.end())
		return false;
	_chToPort[chName].totalSession+=1;
	port = _chToPort[chName].startPort + ( _chToPort[chName].totalSession-1) * _chToPort[chName].stepPortByPn;
	pn  = _chToPort[chName].startPN + ( _chToPort[chName].totalSession-1);
	frenquence = _chToPort[chName].frenquence;
	return true;
}
#endif
	}//namespace EdgeRM
}//namespace ZQTianShan
