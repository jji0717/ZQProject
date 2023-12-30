// ===========================================================================
// Copyright (c) 2006 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: D6Update.cpp $
// Branch: $Name:  $
// Author: li.huang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EdgeRM/service/D6Update.cpp $
// 
// 21    6/06/16 5:20p Li.huang
// ETV-TV-NOW-CR038
// 
// 20    1/11/16 5:54p Dejian.fei
// 
// 19    12/05/13 4:23p Bin.ren
// 
// 18    11/08/13 5:44p Bin.ren
// 
// 17    11/08/13 2:12p Bin.ren
// 
// 16    9/11/13 4:16p Li.huang
// 
// 15    7/09/13 6:06p Bin.ren
// 
// 14    6/25/13 3:30p Li.huang
// 
// 13    6/25/13 2:38p Li.huang
// fix bug 18205
// 
// 12    6/21/13 3:58p Bin.ren
// 
// 11    6/21/13 3:56p Bin.ren
// change the channel updates
// 
// 10    6/18/13 11:37a Li.huang
// 
// 9     6/17/13 5:43p Bin.ren
// 
// 8     6/08/13 3:00p Li.huang
// add limit max session of channel
// 
// 7     6/05/13 7:22p Bin.ren
// 
// 6     6/04/13 10:10p Bin.ren
// 
// 5     6/04/13 9:47p Bin.ren
// 
// 4     6/03/13 4:50p Li.huang
// 
// 3     6/03/13 4:44p Li.huang
// 
// 2     5/23/13 4:00p Li.huang
// 
// 1     1/24/11 2:18p Li.huang

// initially created
// ===========================================================================

#include "D6Update.h"
#include "EdgeRMEnv.h"
//#include "./EdgeRMClient/DataTypes.h"

//define QAM Device parameter
const std::string  _qamModel = "2700";
const std::string  _qamDesc = "qam on rack 11-3";
const std::string  _qamTftpUrl= "tfp://192.168.81.108:6060";
const std::string  _qamAdminUrl = "192.168.81.108:9090";
const std::string  _deviceMac  = "01:02:03:04:05:06";

//define QAM Device  Port parameter
const int _symbolRate = 6875000;
const int _nitpid = 6;
const int _intervalPAT = 40;
const int _intervalPMT = 400;

namespace ZQTianShan {
	namespace EdgeRM {

int convertModuationFormat(int modulationFormat)
{
	switch(modulationFormat)
	{
	case 1:
		return 0x08;
	case 2:
		return 0x0c;
	case 3:
		return 0x10;
	default:
		return 0x00;
	}
}
D6Update::D6Update(EdgeRMEnv& env, ::TianShanIce::EdgeResource::EdgeRMPrx& edgeRMPrx)
                  : _env(env), _edgeRMPrx(edgeRMPrx)
{
}

D6Update::~D6Update(void)
{

}
void D6Update::onStateChanged(ZQ::Vrep::StateDescriptor from, ZQ::Vrep::StateDescriptor to) 
{
	if(to == ZQ::Vrep::st_Established) //代表协议握手结束
	{
	}
	else if(to == ZQ::Vrep::st_Idle ) //代表状态机终止
	{
	}
	else if(to == ZQ::Vrep::st_Active)
	{

	}
}
void D6Update::onEvent(ZQ::Vrep::Event e) 
{

}
void D6Update::onOpenMessage(const ZQ::Vrep::OpenMessage& msg)
{
	/*	Message type - OPEN
	version: 2
	reserved: 0
	holdTime: 30
	reserved2: 0
	identifier: 172.21.224.252
	par_Capability: 
	Capability 1
	Capability code: 1
	Capability length: 4
	Route Supported -
	Address Family: 32769
	Application Protocol: 32769
	Capability 2
	Capability code: 2
	Capability length: 4
	Send Received Supported: 2
	par_StreamingZone: NanJing.local_name
	par_ComponentName: NanJing.local_name.RFGW-1
	par_Vendor: RFGW-1 Cisco Systems
	*/
    
	//get DeviceName and DeviceZone parameter
	_edgeDeviceName = "";
	_edgeDeviceZone = "";
	_edgeDeviceVendor = "";

	ZQ::Vrep::OpenParameters openPara = msg.parameters();
	if(!openPara.getComponentName(_edgeDeviceName))
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "onOpenMessage() missing Component Name parameters"));
		return;
	}
	if(!openPara.getStreamingZone(_edgeDeviceZone))
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "onOpenMessage() missing Streaming Zone parameters"));
		return;
	}
	if(!openPara.getVendorString(_edgeDeviceVendor))
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "onOpenMessage() missing device vendor parameters"));
		return;
	}
    
	glog(::ZQ::common::Log::L_INFO, CLOGFMT(D6Update, "onOpenMessage()ComponentName[%s]StreamingZone[%s]Vendor[%s]"),
		_edgeDeviceName.c_str(), _edgeDeviceZone.c_str(), _edgeDeviceVendor.c_str());

}
void D6Update::onUpdateMessage(const ZQ::Vrep::UpdateMessage& msg)
{
/*		Message length - 144
		Message type - UPDATE
		ReachableRoutes: 
		ReachableRoutes 1
		Address Family: 32769
		Application Protocol: 32769
		Address: 1
		NextHotServer: 
		reserved - 0
		componentAddr - 172.21.224.252
		componentPort - 0
		streamingZone - NanJing.local_name
		QamNames: 
		Qam - NanJing.local_name.2
		TotalBandwidth: 38800
		AvailableBandwidth: 38800
		Cost: 0
		QamParameters: 
		Frequency :952500
		Mode :QAM128
		Interleaver :6
		TSID :2
		Annex :1
		Channel Width :8
		Reserved :0xffff
		UdpMap: 
		Static Port Num: 0
		Dynamic Port Range: 1
		UDP port - 49253	 MPGE Program - 1	 Count - 99
		ServiceStatus: null
		MaxMpegFlows: 0
		OutputPort: 1
     ReachableRoutes中的Address中的标记即是portID，根据Address就可以确定input port和channel的对应关系
       EdgeInput: 
	    Host - 192.168.0.1
		SubnetMask - 255.255.255.0
		Port ID - 1
		Group Name - GbEX1
		Bandwidth - 1000000
		Host - 192.168.0.2
		SubnetMask - 255.255.255.0
		Port ID - 2
		Group Name - GbEX3
		Bandwidth - 1000000
		这里的host为input port的推流地址。PortID唯一确定一个input port
*/

	//open message中获得DeviceName,DeviceZone和DeviceVendor, UpdateMessage中, Qam - NanJing.local_name.2, OutputPort参数作为RfPortId, 2作为ChannelId

	//1. setp1. UpdateMessage中, 第一个消息应该是EdgeInputs. 包含Input口信息
	if(!msg.getEdgeInputs(_edgeInputs) && _edgeInputs.empty())
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "missing edge inputs parameters"));
		return;
	}

	bool  bWithDraw = false;
	ZQ::Vrep::Routes routes;
	if(msg.getWithdrawnRoutes(routes))
	{
		bWithDraw = true;
	}
	else if(!msg.getReachableRoutes(routes))
	{
		return;
	}

	//step2. get channelId from updateMessage
	ZQ::Vrep::QAMNames qamnames;
	if(!msg.getQAMNames(qamnames) && qamnames.size() < 1)
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "missing QamNames parameters"));
		return;
	}
	std::string strChId = qamnames[0];
	int npos = strChId.rfind('.');
	if(npos > 0)
		strChId = strChId.substr(npos+1);

    int chId = atoi(strChId.c_str());
	//step3. get RFPortId from updateMessage
	ZQ::Vrep::dword RFPortId;
	if(!msg.getOutputPort(RFPortId))
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "missing outputPort parameters"));
		return;
     }
	glog(::ZQ::common::Log::L_INFO, CLOGFMT(D6Update, "QamNames[%s]DeviceName[%s]RFportId[%d]ChannelId[%d]"), qamnames[0].c_str(), _edgeDeviceName.c_str(), RFPortId, chId);
	//step4. add channel to Device or Updata channel
    bool bExistDevice = false;
	TianShanIce::EdgeResource::EdgeDeviceExPrx  edgeDeviceExPrx = NULL;
	try
	{
		Ice::Identity identDevice;
		identDevice.name = _edgeDeviceName;
		identDevice.category = DBFILENAME_EdgeDevice;
		bExistDevice = _env._eEdgeDevice->hasObject(identDevice);

		if(bExistDevice)
		{
			Ice::ObjectPrx objPrx = NULL;
			TianShanIce::EdgeResource::EdgeDevicePrx  edgeDeviceOptPrx = NULL;
			edgeDeviceOptPrx = _edgeRMPrx->openDevice(_edgeDeviceName);
			#if  ICE_INT_VERSION / 100 >= 306
				objPrx = edgeDeviceOptPrx->ice_collocationOptimized(false);	
			#else
				objPrx = edgeDeviceOptPrx->ice_collocationOptimization(false);
			#endif
			edgeDeviceExPrx = TianShanIce::EdgeResource::EdgeDeviceExPrx::uncheckedCast(objPrx);
		}
	}
	catch (TianShanIce::ServerError&ex)
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "[%s]TianShanIce ServerError errorcode[%d], errmsg[%s]"),
			_edgeDeviceName.c_str(), ex.errorCode, ex.message.c_str());
	}
	catch(Ice::Exception&ex)
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "[%s]Ice Exception[%s]"),
			_edgeDeviceName.c_str(), ex.ice_name().c_str());
	}
	//step5. if edgeDevice not exist, add it to DB
	if(!bExistDevice) 
	{
		if(!bWithDraw)
		{
			glog(::ZQ::common::Log::L_INFO, CLOGFMT(D6Update, "add Device[%s] to DB"), _edgeDeviceName.c_str());
			addDevice(chId, RFPortId, msg);
		}
		else
		{
			glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "invaild Device status[%s], device not exist and update withdraw mode"), _edgeDeviceName.c_str());
		}
		return;
	}

	TianShanIce::EdgeResource::EdgeChannelExPrx chExPrx = NULL;
	//判断Channel是否存在，如果存在就更新, 不存在就Add
	bool bExistCh = false;

	try
	{ 
		Ice::Identity identCh;
		identCh.name = EdgeRMImpl::formatHiberarchy(_edgeDeviceName, RFPortId, chId);
		identCh.category = DBFILENAME_EdgeChannel;
		bExistCh = _env._eEdgeChannel->hasObject(identCh);
	}
	catch (const Ice::ObjectNotExistException &ex)
	{
	}
	catch(const Ice::Exception&ex)
	{
		return ;
	}
	glog(::ZQ::common::Log::L_INFO, CLOGFMT(D6Update, "QAM name[%s] update"), _edgeDeviceName.c_str());

	//Channel不存在, 添加到Device中
	if(!bExistCh)
	{
		addChannelToDevice(edgeDeviceExPrx, RFPortId, chId, msg);
		if(pConfig.autoLink)
		{
			ZQ::Vrep::QAMParameters qamParameters;
			msg.getQAMParameters(qamParameters);
			TianShanIce::Variant freqs;
			freqs.type = TianShanIce::vtLongs;
			freqs.bRange = false;
			std::string::size_type pos = qamnames[0].find_last_of('.');
			std::string routeName = qamnames[0].substr(0,pos);
			freqs.lints.push_back(qamParameters.frequencyKHz);
			edgeDeviceExPrx->linkRoutes(RFPortId, routeName,freqs);
		}
	}
	else
	{
		Ice::Identity identCh;
		identCh.name = EdgeRMImpl::formatHiberarchy(_edgeDeviceName, RFPortId, chId);
		chExPrx = _env._openChannel(identCh.name);
		if(NULL == chExPrx)
		{
			glog(::ZQ::common::Log::L_ERROR,CLOGFMT(D6Update,"[%s]open channel error"),_edgeDeviceName.c_str());
			return;
		}

		if(bWithDraw)//channel 存在 并且是withDraw Mod
		{
			glog(::ZQ::common::Log::L_INFO, CLOGFMT(D6Update, "QAM name[%s] withdraw routes"), _edgeDeviceName.c_str());
			WithdrawnRoutes(chExPrx, RFPortId, chId, routes);
		}
		else //channe 存在 并且不是withDraw Mod, 更新它
		{
			try
			{
				std::string chName = chExPrx->getId();
				//chExPrx->enable(true);
				//::TianShanIce::EdgeResource::EdgeChannelPrx chPrx = ::TianShanIce::EdgeResource::EdgeChannelPrx::uncheckedCast(chExPrx);
				if(_env._pQamTimerObjects)
					_env._pQamTimerObjects->addTimer(chName, chExPrx);
				UpdateChannel(chExPrx,msg);
			}
			catch (TianShanIce::ServerError&ex)
			{
				glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "[%s]enable channel caught TianShanIce ServerError errorcode[%d], errmsg[%s]"),
					_edgeDeviceName.c_str(), ex.errorCode, ex.message.c_str());
			}
			catch(Ice::Exception&ex)
			{
				glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "[%s]enable channel cauthe Ice Exception[%s]"),
					_edgeDeviceName.c_str(), ex.ice_name().c_str());
			}
		}
	}
}

void D6Update::onNotificationMessage(const ZQ::Vrep::NotificationMessage& msg)
{

}
bool D6Update::addChannelToDevice(TianShanIce::EdgeResource::EdgeDeviceExPrx& edgeDevicePrx, int RFPortId, int chId, const ZQ::Vrep::UpdateMessage& msg)
{
	Ice::Int modulationFormat;            
	Ice::Int interleaverMode;             
	Ice::Int interleaverLevel = 1;  
	Ice::Long frequency;
	Ice::Int tsId;
	Ice::Long totalBandwidth;

	ZQ::Vrep::QAMParameters qamParameters;
	if(!msg.getQAMParameters(qamParameters))
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "missing QAM parameters"));
		return false;
	}
	frequency = qamParameters.frequencyKHz;
	tsId = qamParameters.tsid;
	interleaverMode = qamParameters.interleaver;
	modulationFormat = convertModuationFormat(qamParameters.modulationMode);

	ZQ::Vrep::dword bw, aw;
	if(!msg.getTotalBandwidth(bw))
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "missing QAM Total Bandwidth parameters"));
		return false;
	}
	totalBandwidth = bw * 1000;

	ZQ::Vrep::NextHopServer srv;
	if(!msg.getNextHopServer(srv))
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "missing QAM ZONE parameters"));
		return false;
	}

	std::string deviceZone = std::string(srv.streamingZone.begin(), srv.streamingZone.end());

	ZQ::Vrep::Routes routes;
	std::string deviceIp;
	::TianShanIce::StrValues _deviceIpList;
	::TianShanIce::StrValues _deviceMacList;

	if(!msg.getReachableRoutes(routes) && routes.size() > 0)
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "missing QAM Reachable Routes parameters"));
		return false;
	}

	ZQ::Vrep::EdgeInputs::iterator itorEdgeInputs;
	ZQ::Vrep::EdgeInput edgeInput;
#define BytesToString(BS) std::string(BS.begin(), BS.end())
	for(itorEdgeInputs = _edgeInputs.begin(); itorEdgeInputs != _edgeInputs.end(); itorEdgeInputs++)
	{
		//这里可能有问题,不知道为何Address是个Bytes结构
		uint intAddr = (uint)atoi(BytesToString(routes[0].address).c_str());
		if(itorEdgeInputs->portId == intAddr)
		{
			_deviceIpList.push_back(BytesToString(itorEdgeInputs->host));
			break;
		}
	}

	if(_deviceIpList.empty())
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "missing edgeInput host parameters"));
		return false;
	}

	_deviceMacList.push_back(_deviceMac);

	ZQ::Vrep::UDPMap udpMap;
	Ice::Int	startUDPPort = 0;
	Ice::Int    startProgramNumber = 0;
	Ice::Int    maxSessions = 0;

	ZQ::Vrep::PortMap dynamicPorts; 
	if(msg.getUDPMap(udpMap))
	{
		dynamicPorts = udpMap.dynamicPorts;
		if(dynamicPorts.size() >0)
		{
			startUDPPort = dynamicPorts[0].port;
			startProgramNumber = dynamicPorts[0].pn;
			if(dynamicPorts[0].count > MAX_SESSION_CHANNELS)
				maxSessions = MAX_SESSION_CHANNELS;
			else
				maxSessions = dynamicPorts[0].count;
		}
	}

	TianShanIce::EdgeResource::EdgePort edgePort;
	ZQTianShan::EdgeRM::EdgeChannelImpl::Ptr edgeChannelPtr;
	IdentCollection identChannels;
    bool bExistPort = false;
	Ice::Current c;  
	try
	{
		//get EdgePort info, if exist, update edgeDeviceIP resource, else add edgePort to EdgeDevice
		edgePort = edgeDevicePrx->getEdgePort(RFPortId);
		bExistPort = true;
		//update in edgeDeviceIP    
		TianShanIce::Variant value; 
		value.type = TianShanIce::vtStrings;                                              
		value.bRange = false;                                                               
		value.strs.clear();                                                                 
		value.strs = _deviceIpList;                                                         
		edgePort.resPhysicalChannel.resourceData["edgeDeviceIP"] = value;  
	}
	catch(TianShanIce::InvalidParameter ex)
	{
		bExistPort = false;
	}
	catch (Ice::Exception)
	{
	}
	catch (...)
	{
	}

	try
	{
		if(!bExistPort)
		{
			//get EdgePort attribute from XML                               	
			edgePort.Id			= RFPortId;            	
			edgePort.powerLevel	= 50;

			edgePort.resAtscModulationMode.resourceData.clear();       
			edgePort.resPhysicalChannel.resourceData.clear();          

			TianShanIce::Variant value;                                                         
			edgePort.resAtscModulationMode.status	= ::TianShanIce::SRM::rsAssigned;           
			edgePort.resAtscModulationMode.attr		= ::TianShanIce::SRM::raMandatoryNegotiable;

			//add resAtscModulationMode resource map                                            
			//add in modulationFormat                                                           
			value.type = ::TianShanIce::vtBin;                                                  
			value.bRange = false;                                                               
			value.bin.clear();                                                                  
			value.bin.push_back(modulationFormat);                                              
			edgePort.resAtscModulationMode.resourceData["modulationFormat"] = value;           

			//add in interleaveDepth                                                             
			value.type = ::TianShanIce::vtBin;                                                  
			value.bRange = false;                                                               
			value.bin.clear();                                                                  
			value.bin.push_back(interleaverMode);                                               
			edgePort.resAtscModulationMode.resourceData["interleaveDepth"] = value;            

			//add in FEC                                                                        
			value.type = ::TianShanIce::vtBin;                                                  
			value.bRange = false;                                                               
			value.bin.clear();                                                                  
			value.bin.push_back(interleaverLevel);                                              
			edgePort.resAtscModulationMode.resourceData["FEC"] = value;                        

			//add resPhysicalChannel resource map                                               
			//add in edgeDeviceName                                                             
			value.type = ::TianShanIce::vtStrings;                                              
			value.bRange = false;                                                               
			value.strs.clear();                                                                 
			value.strs.push_back(_edgeDeviceName);                                                        
			edgePort.resPhysicalChannel.resourceData["edgeDeviceName"] = value;                

			//add in edgeDeviceIP                                                               
			value.type = ::TianShanIce::vtStrings;                                              
			value.bRange = false;                                                               
			value.strs.clear();                                                                 
			value.strs = _deviceIpList;                                                         
			edgePort.resPhysicalChannel.resourceData["edgeDeviceIP"] = value;                  

			//add in edgeDeviceMac                                                              
			value.type = ::TianShanIce::vtStrings;                                              
			value.bRange = false;                                                               
			value.strs.clear();                                                                 
			value.strs = _deviceMacList;                                                        
			edgePort.resPhysicalChannel.resourceData["edgeDeviceMac"] = value;                 

			//add in edgeDeviceZone                                                             
			value.type = ::TianShanIce::vtStrings;                                              
			value.bRange = false;                                                               
			value.strs.clear();                                                                 
			value.strs.push_back(_edgeDeviceZone);                                                  
			edgePort.resPhysicalChannel.resourceData["edgeDeviceZone"] = value;                

			//add EdgePort to device                                                            
			                                                                 
			edgeDevicePrx->addEdgePort(edgePort);    
		}

		Ice::Identity deviceIdent;
		deviceIdent.name = edgeDevicePrx->getName();
		deviceIdent.category = DBFILENAME_EdgeDevice;

		//get channelinfo  attribute from XML  
		edgeChannelPtr = NULL;//destroy last EdgeChannel pointer                                                       
		edgeChannelPtr = new ::ZQTianShan::EdgeRM::EdgeChannelImpl(_env);                                              

		edgeChannelPtr->identDevice = deviceIdent;                                                              
		edgeChannelPtr->ident.name = ::ZQTianShan::EdgeRM::EdgeRMImpl::formatHiberarchy(_edgeDeviceName, edgePort.Id, chId);
		edgeChannelPtr->ident.category = DBFILENAME_EdgeChannel;                                                       
		edgeChannelPtr->ePort = edgePort;                                                                             
		edgeChannelPtr->enabled = true;                                                                                

		//add resAtscModulationMode resource map                                                                        
		//add in symbolRate  
		TianShanIce::Variant value; 
		Ice::Int symbolRate = _symbolRate;                                                                                   
		value.type = ::TianShanIce::vtInts;                                                                             
		value.bRange = false;                                                                                           
		value.ints.clear();                                                                                             
		value.ints.push_back(symbolRate);                                                                               
		edgeChannelPtr->ePort.resAtscModulationMode.resourceData["symbolRate"] = value;                                

		edgeChannelPtr->TSID = tsId;                                                          
		edgeChannelPtr->freqRF	= frequency;                                                        
		//init with reserved value                                                                                      
		edgeChannelPtr->NITPID = _nitpid;                                                      
		edgeChannelPtr->deviceState = ::TianShanIce::stInService;
		identChannels.push_back(edgeChannelPtr->ident);

		edgeChannelPtr->startUDPPort		= startUDPPort; //modify here                                                     
		edgeChannelPtr->udpPortStepByPn	= 1; //modify here   
		edgeChannelPtr->startProgramNumber	= startProgramNumber; //modify here   
		edgeChannelPtr->maxSessions		= maxSessions; //modify here   
		edgeChannelPtr->lowBandwidthUtilization = 200*1000;
		edgeChannelPtr->highBandwidthUtilization = totalBandwidth;
		edgeChannelPtr->intervalPAT = _intervalPAT;	
		edgeChannelPtr->intervalPMT = _intervalPMT;	

		edgeChannelPtr->stampLastUpdated = ::ZQTianShan::now();
		_env._eEdgeChannel->add(edgeChannelPtr, edgeChannelPtr->ident);	

		IdentCollection identChannels;
		try
		{	
			ZQ::common::MutexGuard gd(_env._lkdevicechannels);
			DeviceChannelsMap::iterator itorDC = _env._devicechannels.find(deviceIdent.name);
			if(itorDC != _env._devicechannels.end())
				identChannels = itorDC->second;
		}
		catch (...){	
		}
        //modifty here 
		identChannels.push_back(edgeChannelPtr->ident);
	    _env.addChannelsToDevice(deviceIdent.name, identChannels);                                                

		TianShanIce::EdgeResource::EdgeChannelExPrx chExPrx = IdentityToObjEnv2(_env, EdgeChannelEx, edgeChannelPtr->ident);
		if(_env._pQamTimerObjects)
			_env._pQamTimerObjects->addTimer(edgeChannelPtr->ident.name, chExPrx);
	}
	catch (TianShanIce::ServerError&ex)
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "[%s]TianShanIce ServerError errorcode[%d], errmsg[%s]"),
			_edgeDeviceName.c_str(), ex.errorCode, ex.message.c_str());

	}
	catch(Ice::Exception&ex)
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "[%s]Ice Exception[%s]"),
			_edgeDeviceName.c_str(), ex.ice_name().c_str());
	}
	return true;
}
bool D6Update::addDevice(int RFPortId, int chId, const ZQ::Vrep::UpdateMessage& msg)
{
	ZQTianShan::EdgeRM::EdgeDeviceImpl::Ptr edgeDevice;
	try
	{
		//EdgeDevice
		edgeDevice = new ::ZQTianShan::EdgeRM::EdgeDeviceImpl(_env);
		std::string netIdSlash = pConfig.netId + "/";
		if(_edgeDeviceName.find(netIdSlash) == std::string::npos && pConfig._backup.mode == "active")
			_edgeDeviceName = pConfig.netId + "/" + _edgeDeviceName;
		edgeDevice->ident.name = _edgeDeviceName;                             
		edgeDevice->ident.category = DBFILENAME_EdgeDevice;         

		//get device info from XML                                   
		edgeDevice->deviceZone	    = _edgeDeviceZone;                       
		edgeDevice->type			= "QAM";                               
		edgeDevice->vendor			= _edgeDeviceVendor;                 
		edgeDevice->model			= _qamModel;                  
		edgeDevice->desc			= _qamDesc;                     
		edgeDevice->tftpUrl	        = _qamTftpUrl;                   
		edgeDevice->adminUrl		= _qamAdminUrl;   
		//add to evictor map                                                                                               		
		_env._eEdgeDevice->add(edgeDevice, edgeDevice->ident);                                                           
		TianShanIce::EdgeResource::EdgeDeviceExPrx edgeDevicePrx = IdentityToObjEnv2(_env, EdgeDeviceEx, edgeDevice->ident);

		addChannelToDevice(edgeDevicePrx, RFPortId, chId, msg);

		edgeDevice = NULL;//release local reference  
	}
	catch (TianShanIce::ServerError&ex)
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "[%s]TianShanIce ServerError errorcode[%d], errmsg[%s]"),
			_edgeDeviceName.c_str(), ex.errorCode, ex.message.c_str());

	}
	catch(Ice::Exception&ex)
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "[%s]Ice Exception[%s]"),
			_edgeDeviceName.c_str(), ex.ice_name().c_str());
	}
	return true;

}

bool D6Update::UpdateChannel(TianShanIce::EdgeResource::EdgeChannelExPrx& chExPrx, const ZQ::Vrep::UpdateMessage& msg)
{
	//step 1:set expectedMetaData
	TianShanIce::StrValues expectedMetaData;
    
	//1.1  qam parameters metadata
	ZQ::Vrep::QAMParameters qamParameters;
	bool paraBExist = false;
	paraBExist=msg.getQAMParameters(qamParameters);
	if(paraBExist)
	{
		expectedMetaData.push_back(SYS_PROP(FreqRF));
		expectedMetaData.push_back(SYS_PROP(TSID));
	}

	//1.2 totalBandwidth metadata
	ZQ::Vrep::dword totalBandwidth = 0;
	bool bwBExist = false;
	bwBExist = msg.getTotalBandwidth(totalBandwidth);
	if(bwBExist)
	{
		expectedMetaData.push_back(SYS_PROP(HighBandwidthUtilization));
		totalBandwidth = totalBandwidth * 1000;
	}

	//1.3 nextHopServer metadata
	ZQ::Vrep::NextHopServer srv;
	bool srvBExist = false;
	srvBExist = msg.getNextHopServer(srv); 

	//1.4 reachable route metadata
	ZQ::Vrep::Routes routes;
	TianShanIce::StrValues deviceIpList;
	TianShanIce::StrValues deviceMacList;
	if(msg.getReachableRoutes(routes))
	{
		std::string deviceIp;

		ZQ::Vrep::EdgeInputs::iterator itorEdgeInputs;
		for(itorEdgeInputs = _edgeInputs.begin(); itorEdgeInputs != _edgeInputs.end(); itorEdgeInputs++)
		{
			uint intAddr = (uint)atoi(BytesToString(routes[0].address).c_str());
			if(itorEdgeInputs->portId == intAddr)
			{
				deviceIpList.push_back(BytesToString(itorEdgeInputs->host));
				break;
			}
		}

		if(deviceIpList.empty())
		{
			glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "missing edgeInput host parameters"));
			return false;
		}

		deviceMacList.push_back(_deviceMac);
	}

	//1.5 udpMap metadata
	ZQ::Vrep::UDPMap udpMap;
	bool udpBExist = false;
	udpBExist = msg.getUDPMap(udpMap);
	if(udpBExist)
	{	
		if(udpMap.dynamicPorts.size() > 0 )
		{
			expectedMetaData.push_back(SYS_PROP(StartUDPPort));
			expectedMetaData.push_back(SYS_PROP(StartProgramNumber));
			expectedMetaData.push_back(SYS_PROP(MaxSessions));
		}	
	}

	//step 2:get channel metaData
	TianShanIce::StatedObjInfo info;
	try
	{
		info = chExPrx->getInfo(expectedMetaData);
	}
	catch (TianShanIce::ServerError&ex)
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "[%s]TianShanIce ServerError errorcode[%d], errmsg[%s]"),
			_edgeDeviceName.c_str(), ex.errorCode, ex.message.c_str());

		return false;
	}
	catch(Ice::Exception&ex)
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "[%s]Ice Exception[%s]"),
			_edgeDeviceName.c_str(), ex.ice_name().c_str());

		return false;
	}

	//step 3:set Properties
	TianShanIce::Properties attrs;
	attrs.clear();
	//3.1 set QamParameters
	if(paraBExist)
	{
		bool enabledOnly = false;
		::Ice::Int modulationFormat;            
		::Ice::Int interleaverMode;             
		::Ice::Long frequency;
		::Ice::Int tsId;

		frequency = qamParameters.frequencyKHz;
		tsId = qamParameters.tsid;
		interleaverMode = qamParameters.interleaver;
		modulationFormat = convertModuationFormat(qamParameters.modulationMode); 

		glog(::ZQ::common::Log::L_INFO, CLOGFMT(D6Update, "name[%s],channeId[1], frequency[%s], tsID[%s]]"),
			_edgeDeviceName.c_str(),info.props[SYS_PROP(FreqRF)].c_str(), info.props[SYS_PROP(TSID)].c_str());

		Ice::Long freq =(Ice::Long) (_atoi64(info.props[SYS_PROP(FreqRF)].c_str()));
		if(freq != frequency)
		{
			//update channel frequency
			char temp[65] = "";
			sprintf(temp, "%lld", frequency);
			MAPSET(TianShanIce::Properties, attrs, SYS_PROP(FreqRF), temp);
		}
		Ice::Int tsid  =(Ice::Int) atoi(info.props[SYS_PROP(TSID)].c_str());
		if(tsid != tsId)
		{
			//update channel tsId
			char temp[65] = "";
			sprintf(temp, "%d", tsId);
			MAPSET(TianShanIce::Properties, attrs, SYS_PROP(TSID), temp);
		}
		TianShanIce::EdgeResource::EdgePort  edgePort = chExPrx->getEdgePort();
		TianShanIce::SRM::Resource resAtscModulationMode =  edgePort.resAtscModulationMode;
		TianShanIce::Variant value;
		TianShanIce::ValueMap::iterator itor;
		itor = edgePort.resAtscModulationMode.resourceData.find("modulationFormat");
		if(itor != edgePort.resAtscModulationMode.resourceData.end())
		{
			value = itor->second;
			if(value.type == ::TianShanIce::vtBin &&  value.bin.size() >0)
			{
				glog(::ZQ::common::Log::L_INFO, CLOGFMT(D6Update, "name[%s],channeId[1],modulationFormat[%d] "),
					_edgeDeviceName.c_str(), value.bin[0]);
				if(modulationFormat != value.bin[0])
				{
					//update modulationFormat	
					char temp[65] = "";
					sprintf(temp, "%d", modulationFormat);
					MAPSET(TianShanIce::Properties, attrs, SYS_PROP(ModulationFMT), temp);
				}
			}
		}

		itor = edgePort.resAtscModulationMode.resourceData.find("interleaveDepth");
		if(itor != edgePort.resAtscModulationMode.resourceData.end())
		{
			value = itor->second;
			if(value.type == ::TianShanIce::vtBin &&  value.bin.size() >0)
			{
				glog(::ZQ::common::Log::L_INFO, CLOGFMT(D6Update, "name[%s],channeId[1],interleaveDepth[%d] "),
					_edgeDeviceName.c_str(), value.bin[0]);

				if(interleaverMode != value.bin[0])
				{
					//update interleaverMode
					char temp[65] = "";
					sprintf(temp, "%d", interleaverMode);
					MAPSET(TianShanIce::Properties, attrs, SYS_PROP(InterleaverDepth), temp);
				}
			}
		}
	}

	
	//3.2 set totalBandwidth
	if(bwBExist)
	{
		int64 HighBandWidth = _atoi64(info.props[SYS_PROP(HighBandwidthUtilization)].c_str());
		if(totalBandwidth != HighBandWidth)
		{
			char temp[65] = "";
			sprintf(temp, "%lu", totalBandwidth);
			MAPSET(TianShanIce::Properties, attrs, SYS_PROP(HighBandwidthUtilization), temp);
		}
	}
	
	//3.3 set UDPMap
	if(udpBExist)
	{
		Ice::Int	startUDPPort = 0;
		Ice::Int    startProgramNumber = 0;
		Ice::Int    maxSessions = 0;

		ZQ::Vrep::PortMap dynamicPorts; 
		dynamicPorts = udpMap.dynamicPorts;
		if(dynamicPorts.size() < 1)
		{
			return false;
		}	
		startUDPPort = dynamicPorts[0].port;
		startProgramNumber = dynamicPorts[0].pn;

		if(dynamicPorts[0].count > MAX_SESSION_CHANNELS)
			maxSessions = MAX_SESSION_CHANNELS;
		else
			maxSessions = dynamicPorts[0].count;

		if(atoi( info.props[SYS_PROP(StartUDPPort)].c_str()) != startUDPPort)
		{
			char temp[65] = "";
			sprintf(temp, "%d", startUDPPort);
			MAPSET(TianShanIce::Properties, attrs, SYS_PROP(StartUDPPort), temp);
		}
		if(atoi( info.props[SYS_PROP(StartProgramNumber)].c_str()) != startProgramNumber)
		{
			char temp[65] = "";
			sprintf(temp, "%d", startProgramNumber);
			MAPSET(TianShanIce::Properties, attrs, SYS_PROP(StartProgramNumber), temp);
		}
		if(atoi( info.props[SYS_PROP(MaxSessions)].c_str()) != maxSessions)
		{
			char temp[65] = "";
			sprintf(temp, "%d", maxSessions);
			MAPSET(TianShanIce::Properties, attrs, SYS_PROP(MaxSessions), temp);
		}
	}

	//3.4 set DeviceZone
	if(srvBExist)
	{
		std::string deviceZone = std::string(srv.streamingZone.begin(), srv.streamingZone.end());
		try
		{
			TianShanIce::EdgeResource::EdgePort  edgePort = chExPrx->getEdgePort();

			TianShanIce::ValueMap::iterator itor;
			itor = edgePort.resAtscModulationMode.resourceData.find("edgeDeviceZone");
			if(itor != edgePort.resAtscModulationMode.resourceData.end())
			{
				TianShanIce::Variant value = itor->second;
				if(value.type == ::TianShanIce::vtStrings &&  value.strs.size() >0)
				{
					glog(::ZQ::common::Log::L_INFO, CLOGFMT(D6Update, "name[%s],channeId[1], deviceZone[%s] "),
						_edgeDeviceName.c_str(), value.strs[0].c_str());
					if(deviceZone != value.strs[0])
					{
						MAPSET(TianShanIce::Properties, attrs, SYS_PROP(DeviceZone), deviceZone);
					}
				}
			}
		}
		catch (TianShanIce::ServerError&ex)
		{
			glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "[%s]TianShanIce ServerError errorcode[%d], errmsg[%s]"),
				_edgeDeviceName.c_str(), ex.errorCode, ex.message.c_str());

			return false;

		}
		catch(Ice::Exception&ex)
		{
			glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "[%s]Ice Exception[%s]"),
				_edgeDeviceName.c_str(), ex.ice_name().c_str());

			return false;
		}
	}

	//3.5 set reachable route
	if(!deviceIpList.empty() && !deviceMacList.empty())
	{
		std::string strIplist;
		for(int i = 0 ; i < deviceIpList.size();i ++)
		{
			strIplist+= deviceIpList[i];
			strIplist+= ", ";
		}
		glog(::ZQ::common::Log::L_INFO, CLOGFMT(D6Update, "Update QAM ReachableRoutes name[%s], deviceIplist[%s]"),
			_edgeDeviceName.c_str(), strIplist.c_str());

		try
		{
			TianShanIce::EdgeResource::EdgePort  edgePort = chExPrx->getEdgePort();

			TianShanIce::ValueMap::iterator itor;
			itor = edgePort.resPhysicalChannel.resourceData.find("edgeDeviceIP");
			if(itor != edgePort.resPhysicalChannel.resourceData.end())
			{
				TianShanIce::Variant value = itor->second;
				if(value.type == ::TianShanIce::vtStrings &&  value.strs.size() >0)
				{
					glog(::ZQ::common::Log::L_INFO, CLOGFMT(D6Update, "name[%s],channeId[1], deviceIp[%s] "),
						_edgeDeviceName.c_str(), value.strs[0].c_str());
					if(deviceIpList[0] != value.strs[0])
					{
						MAPSET(TianShanIce::Properties, attrs, SYS_PROP(DeviceIP), deviceIpList[0]);
					}
				}
			}
		}
		catch (TianShanIce::ServerError&ex)
		{
			glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "[%s]TianShanIce ServerError errorcode[%d], errmsg[%s]"),
				_edgeDeviceName.c_str(), ex.errorCode, ex.message.c_str());

			return false;

		}
		catch(Ice::Exception&ex)
		{
			glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "[%s]Ice Exception[%s]"),
				_edgeDeviceName.c_str(), ex.ice_name().c_str());

			return false;
		}
	}

	//step 4:update channel attributes
	if(info.state != TianShanIce::stInService)
	{
		char temp[32]="";
		itoa(TianShanIce::stInService, temp, 10);
		attrs[SYS_PROP(DeviceState)] = temp;
	}

	if(!attrs.empty())
	{
		chExPrx->updateAttributes(attrs);

		::TianShanIce::Properties::iterator iterAttrs = attrs.begin();
		for(iterAttrs;iterAttrs != attrs.end();iterAttrs++)
			glog(::ZQ::common::Log::L_DEBUG,CLOGFMT(UpdateChannel,"channel attributes: [%s,%s] updated"),iterAttrs->first.c_str(),iterAttrs->second.c_str());
	}

	glog(ZQ::common::Log::L_DEBUG,CLOGFMT(UpdateChannel,"channel attributes :%d attributes has been updated"),attrs.size());

	return true;
}

bool D6Update::WithdrawnRoutes(TianShanIce::EdgeResource::EdgeChannelExPrx& chExPrx, int RFPortId, int chId, ZQ::Vrep::Routes& routes)
{
	glog(::ZQ::common::Log::L_INFO, CLOGFMT(D6Update, "Update QamName[%s.%d.%d] withdrawn Routes"), _edgeDeviceName.c_str(), RFPortId,chId);
	try
	{
		TianShanIce::Properties attrs;
		attrs.clear();
		char temp[32]="";
		itoa(TianShanIce::stOutOfService, temp, 10);
        attrs[SYS_PROP(DeviceState)] = temp;
		chExPrx->updateAttributes(attrs);
		std::string chName = chExPrx->getId();
		if(_env._pQamTimerObjects)
			_env._pQamTimerObjects->removeTimer(chName);
	}
	catch (TianShanIce::ServerError&ex)
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "QamName[%s.%d.%d]withdrawn Route caught TianShanIce ServerError errorcode[%d], errmsg[%s]"),
			_edgeDeviceName.c_str(), RFPortId,chId, ex.errorCode, ex.message.c_str());

	}
	catch(Ice::Exception&ex)
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(D6Update, "QamName[%s.%d.%d]withdrawn Route caught Ice Exception[%s]"),
			_edgeDeviceName.c_str(), RFPortId,chId, ex.ice_name().c_str());
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
///////   class QAMTimerObjects
////////////////////////////////////////////////////////////////////////////////////////////
QAMTimerObjects::QAMTimerObjects(ZQTianShan::EdgeRM::EdgeRMEnv& env,ZQ::common::NativeThreadPool&thpool, int timeoutInterval)
:_env(env), _timeOutInterval(timeoutInterval), _pwatchDog(NULL)
{
	_pwatchDog = new ZQ::Vrep::Watchdog(glog, thpool);
	if(_pwatchDog)
		_pwatchDog->start();
}
QAMTimerObjects::~QAMTimerObjects(void)
{
  if(_pwatchDog)
  {
	  delete _pwatchDog;
  }
  _pwatchDog = NULL;
}

 bool QAMTimerObjects::addTimer(std::string& chName, ::TianShanIce::EdgeResource::EdgeChannelExPrx& chExPrx)
 {
    ZQ::common::MutexGuard guard(_lkqamtimer);
	try
	{
		QAMTimers::iterator itor = qamTimers.find(chName);
		if(itor != qamTimers.end())
			itor->second->restart();
		else
		{
			(_env._log)(::ZQ::common::Log::L_INFO, CLOGFMT(QAMTimerObjects, "add timer with channel[%s]"),chName.c_str());
			TimerPtr pTimer(new ZQ::Vrep::Timer(*_pwatchDog, ZQ::Vrep::TimeoutObjectPtr(new QAMWithDrawTimeOut(chExPrx, chName))));
			pTimer->start(_timeOutInterval);
			MAPSET(QAMTimers, qamTimers, chName, pTimer);
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
 }
 bool QAMTimerObjects::removeTimer(std::string& chName)
 {
	 ::ZQ::common::MutexGuard guard(_lkqamtimer);
	 try
	 {
		 QAMTimers::iterator itorQamtimer = qamTimers.find(chName);
		 if(itorQamtimer != qamTimers.end())
		 { 
			 (_env._log)(::ZQ::common::Log::L_INFO, CLOGFMT(QAMTimerObjects, "remove timer with channel[%s]"),chName.c_str());
			 qamTimers.erase(itorQamtimer);
		 }
	 }
	 catch (...)
	 {
		 return false;
	 }
	 
	 return true;
 }

 void QAMTimerObjects::removeAll()
 {
	 ZQ::common::MutexGuard guard(_lkqamtimer);
	 try
	 {
		 QAMTimers::iterator itorQamtimer = qamTimers.begin();
		 while(itorQamtimer != qamTimers.end())
		 {
			 (_env._log)(::ZQ::common::Log::L_INFO, CLOGFMT(QAMTimerObjects, "remove timer with channel[%s]"),(itorQamtimer->first).c_str());
			 qamTimers.erase(itorQamtimer);
			 itorQamtimer->second.reset();
			 itorQamtimer = qamTimers.begin();
		 }
	 }
	 catch (...)
	 {
	 }
 }
}} // namespace
