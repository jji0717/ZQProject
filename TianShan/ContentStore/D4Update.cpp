
//#include "NGODEnv.h"
//#include "SOPConfig.h"

//#include <Ws2tcpip.h>

#include <VrepSpeaker.h>
#include <VrepUtils.h>
#include "D4Update.h"
#include <TianShanIceHelper.h>
#include "MCCSCfg.h"
#include "CPHInc.h"
#ifdef ZQ_OS_MSWIN
#include <Ws2tcpip.h>
#else
#include <netdb.h>
#endif

#define MLOG (glog)
ZQ::Vrep::bytes stringToBytes(const std::string& str) 
{
	return ZQ::Vrep::bytes(str.begin(), str.end());
}

D4StateSinker::D4StateSinker( D4Speaker& speaker )
:mD4Speaker(speaker)
{
}

D4StateSinker::~D4StateSinker()
{
}

void D4StateSinker::onStateChanged( ZQ::Vrep::StateDescriptor from, ZQ::Vrep::StateDescriptor to) 
{
	if( ZQ::Vrep::st_Active == to )
	{
		mD4Speaker.onConnected();
	}
}

void D4StateSinker::onEvent( ZQ::Vrep::Event e)
{

}


//////////////////////////////////////////////////////////////////////////
//D5Speaker
D4Speaker::D4Speaker(ZQ::common::Log& log,ZQ::common::NativeThreadPool& pool, ::TianShanIce::Storage::ContentStoreExPrx& csPrx, D4MessageConfig& d4MsgCfg)
:mbQuit(true),
mSpeaker(log,pool),
_csPrx(csPrx),
_d4MsgCfg(d4MsgCfg),
_hisAllocBW(0),
_hisTotalBW(0),
_bReportVolYet(false)
{
}
D4Speaker::~D4Speaker()
{

}
int D4Speaker::run( )
{
	//do nothing
	return 0;
}

void D4Speaker::onConnected( )
{
	sendServiceState( true );

}

void D4Speaker::onDisconnected()
{
	// ??
}

bool D4Speaker::initSpeakerConf( )
{
	if( _d4MsgCfg.listener.empty() )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(D4Speaker,"invalid D4 Server listener configed"));
		return false;
	}

	std::vector<std::string> res = ZQ::common::stringHelper::split(_d4MsgCfg.listener, ':');
	if(res.size() == 2) 
	{
		_strListenerIp = res.at(0);
		_ListenerPort = atoi(res.at(1).c_str());
	}
	else
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(D4Speaker,"invalid D4 Server listener configed"));
		return false;
	}
	mSpeaker.setPeer(_strListenerIp.c_str(),_ListenerPort);

	std::string strA3Interface = _d4MsgCfg.strA3Interface;
	size_t ipos = strA3Interface.find_first_of(':');
	if (ipos == std::string::npos)
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(D4Speaker,"invalid A3 Interface configed"));
		return false;
	}
	size_t ipos1 = strA3Interface.find_last_of(':');
	if (ipos1 == std::string::npos || ipos1 == ipos || strA3Interface.size() < 4)
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(D4Speaker,"invalid A3 Interface configed"));
		return false;
	}
	_strNextHop = strA3Interface.substr(ipos+3, ipos1 - ipos -3);

// 	if (_d4MsgCfg.AdMethod.size())
// 	{
// 		_bHasSpecifiedMethod = true;
// 		for (std::vector<AdvertiseMethod>::iterator iter = _d4MsgCfg.AdMethod.begin(); iter != _d4MsgCfg.AdMethod.end();iter++)
// 			_speMethodVec.push_back(iter->method);	
// 	}

	ZQ::Vrep::Configuration conf;

	struct hostent * hostId = gethostbyname(_strNextHop.c_str())	;
	if( hostId )
	{	
		conf.identifier = *((unsigned  long   *)hostId->h_addr_list[0]);
	}

	conf.streamingZone			= _d4MsgCfg.strStreamZone;
	conf.componentName			= "Vrep";
	conf.vendorString			= "Speaker";
	conf.defaultHoldTimeSec		= _d4MsgCfg.advInterval/1000;
	conf.connectRetryTimeSec	= 60;
	conf.connectTimeoutMsec		= 2000;
	conf.keepAliveTimeSec		= _d4MsgCfg.advInterval/1000;
	conf.sendReceiveMode		= VREP_SendOnlyMode;

	mSpeaker.enableAutoRestart(_d4MsgCfg.advInterval/1000);

	return mSpeaker.start(conf);
}

bool D4Speaker::start()
{
	if( !initSpeakerConf() )
	{
		return false;
	}
	
	mbQuit = false;
	return ZQ::common::NativeThread::start();
}

void D4Speaker::stop()
{
	if( mbQuit == false )
	{
		sendServiceState( false );
		mbQuit = true;
		mSem.post();
		waitHandle(10000);
		//TODO: close speaker ?
	}	
}

void D4Speaker::sendServiceState( bool bUp )
{
	if(mbQuit)
		return;
	ZQ::Vrep::UpdateMessage msg;
	msg.setServiceStatus( bUp ? VREP_ServiceStatus_Operational : VREP_ServiceStatus_ShuttingDown );
	mSpeaker.sendUpdate(msg, 0);
	ZQ::Vrep::dword st;
	msg.getServiceStatus(st);
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(D4Speaker,"servicestatus[%d]"),st);
}


void D4Speaker::onSpigotStateChange( const std::vector<CPCImpl::CPEInst>& cpes , bool bUp )
{
	if(mbQuit)
		return;

	int64 totalBW = 0;
	int64 availBW = 0;
	int64 allocBW = 0;
	int64 readBW = 0;
	ZQ::Vrep::byte bcap;
	std::vector<std::string> methodColl;
	int i =  1;
	bool bAssign = false;

	_BWInfoVec.clear();
    //calc readBW from CPE instance
	for (std::vector<CPCImpl::CPEInst>::const_iterator it = cpes.begin();it != cpes.end(); it++)
	{
		::TianShanIce::ContentProvision::ExportMethods exportMethods = (*it).exportMethods;
		for (::TianShanIce::ContentProvision::ExportMethods::iterator iterE = exportMethods.begin();iterE != exportMethods.end();iterE++)
		{
            readBW += iterE->maxBwKbps; 
		}
	}
    if (_d4MsgCfg.AdMethod.size() > 0)
	{
		for (std::vector<std::string>::iterator iterS =_d4MsgCfg.AdMethod.begin();iterS != _d4MsgCfg.AdMethod.end();iterS++)
		{
			MethodBWInfo bwInfo={"",0,0,0,0,0};
			for (std::vector<CPCImpl::CPEInst>::const_iterator it = cpes.begin();it != cpes.end(); it++)
			{
				::TianShanIce::ContentProvision::MethodInfos methodInfos = (*it).methodInfos;
				for (::TianShanIce::ContentProvision::MethodInfos::iterator iterM = methodInfos.begin();iterM != methodInfos.end();iterM++)
				{
					if (!stricmp((*iterS).c_str(),iterM->methodType.c_str()))
					{	
						bwInfo.totalBw += (*iterM).maxKbps;
						bwInfo.allocBw += (*iterM).allocatedKbps;	
					}
				}
			}
			bwInfo.method = *iterS;
			bwInfo.bcap = decideCapType(*iterS);	
			_BWInfoVec.push_back(bwInfo);
		}

		int size = _BWInfoVec.size();

		for(std::vector<MethodBWInfo>::iterator iter= _BWInfoVec.begin(); iter != _BWInfoVec.end();iter++)
		{
			iter->availBw = iter->totalBw - iter->allocBw;
			if (iter->totalBw)
			{
				iter->cost = (int32)(iter->totalBw - iter->availBw)*256/(iter->totalBw);

				MLOG(ZQ::common::Log::L_INFO,CLOGFMT(D4Speaker,"[Method(%s)][totalBw(%lld)Mbps, [availBW(%lld)Mbps[, [cost(%d)], [CAP(%d)], [readBW(%lld)Mbps]"),
					iter->method.c_str(),iter->totalBw/1000, iter->availBw/1000, iter->cost, iter->bcap, readBW/1000);

				sendBWChangedInfo((int32)iter->totalBw/1000,(int32)iter->availBw/1000,iter->cost,iter->bcap, readBW/1000);
			}
		}

	}
	else
	{
		for (std::vector<CPCImpl::CPEInst>::const_iterator it = cpes.begin();it != cpes.end(); it++)
		{
			::TianShanIce::ContentProvision::MethodInfos methodInfos = (*it).methodInfos;
			for (::TianShanIce::ContentProvision::MethodInfos::iterator iterM = methodInfos.begin();iterM != methodInfos.end();iterM++)
			{
				totalBW += (*iterM).maxKbps;
				allocBW += (*iterM).allocatedKbps;
				methodColl.push_back(iterM->methodType);
			}

		}

		for (std::vector<std::string>::iterator itmethod = methodColl.begin(); itmethod != methodColl.end();itmethod++)
		{
			if (!bAssign)
			{
				bcap = decideCapType(*itmethod);
				bAssign = true;
			}
			else
				bcap |= decideCapType(*itmethod);
		}

		availBW = totalBW - allocBW;
		int32 cost = 0;
		if (totalBW)
			cost = (int32)( totalBW-availBW)*256/totalBW;

		if (_hisAllocBW != allocBW || _hisTotalBW != totalBW)
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(D4Speaker,"[histroyBW(%lld)][totalBw(%lld)Mbps, [availBW(%lld)Mbps[, [cost(%d)], [CAP(%d)], [readBW(%lld)Mbps]"),
				_hisTotalBW,totalBW/1000, availBW/1000, cost, bcap, readBW/1000);
			sendBWChangedInfo((int32)totalBW/1000,(int32)availBW/1000,cost,bcap, readBW/1000);
			_hisTotalBW = totalBW;
			_hisAllocBW = allocBW;
		}
	}
}

void D4Speaker::onSpigotStateChange( std::map<std::string, CPCImpl::CPEInst>& cpemap, bool bUp )
{
	if(mbQuit)
		return;

	std::vector<CPCImpl::CPEInst> cpes;
	std::map<std::string, CPCImpl::CPEInst>::iterator iter= cpemap.begin();
	for (int i = 0; i <cpemap.size();i++,iter++)
	{
		cpes.push_back(iter->second);
	}

	onSpigotStateChange(cpes,true);
}

void D4Speaker::sendBWChangedInfo(int32 totalBW , int32 availBW , int32 cost, ZQ::Vrep::byte cap, int32 readBW)
{
	if(mbQuit)
		return;
	
	ZQ::Vrep::bytes serverIpBytes;
// 	{
// 		std::string hostName	= _strNextHop;
// 		std::string serverName	= "0";
// 		addrinfo*	pAddrInfo = NULL ;
// 		addrinfo	addrHint;
// 		memset( &addrHint , 0 ,sizeof(addrHint));
// 		addrHint.ai_family		=	AF_UNSPEC;
// 		addrHint.ai_socktype	=	SOCK_STREAM;
// 		addrHint.ai_protocol	=	IPPROTO_TCP;
// 		addrHint.ai_flags		=	AI_CANONNAME|AI_PASSIVE;
// 		if( getaddrinfo( hostName.c_str() , serverName.c_str() ,&addrHint,&pAddrInfo) == 0 )
// 		{
// 			for( int i = 0 ;i < (int)pAddrInfo->ai_addrlen ; i++ )
// 			{
// 				serverIpBytes.push_back( ((char*)pAddrInfo->ai_addr)[i] );
// 			}
// 		}
// 		freeaddrinfo(pAddrInfo);
// 	}

	ZQ::Vrep::UpdateMessage msg;
	// reachable route
	ZQ::Vrep::Route r;

	r.family	= VREP_AddressFamily_NGOD;
	r.protocol	= VREP_AppProtocol_R2;	

	//TODO: how to get this ip
	r.address	= stringToBytes(_d4MsgCfg.ServerIp);
	r.name		= stringToBytes(_d4MsgCfg.ServerName);

	ZQ::Vrep::Routes rs;
	rs.push_back(r);

	msg.setReachableRoutes(rs);

	//readbw and writebw for volume need confirm the actual value
	//suppose the availableBandwidth is the write bandwidth of volume, volumeId is not the volume that list but configured in xml file
	ZQ::Vrep::Volume vol;
	vol.name = stringToBytes(_d4MsgCfg.strVolumeId);
	vol.portId = _d4MsgCfg.portId;
	vol.readBw =  readBW * 1000;
	vol.writeBw =  availBW * 1000;

	ZQ::Vrep::Volumes vols;
	vols.push_back(vol);
	msg.setVolumes(vols);

	/*if (_bReportVolYet)
	{
		TianShanIce::Storage::VolumeInfos volInfos;
		ZQ::Vrep::Volumes vols;

		getVolumeInfo(volInfos);

		if (volInfos.size())
		{
			_bReportVolYet = true;

			for (TianShanIce::Storage::VolumeInfos::iterator itV = volInfos.begin(); itV < volInfos.end(); itV ++)
			{
				ZQ::Vrep::Volume vol;
				vol.name = stringToBytes(itV->name);
				vol.portId = 0x00ff00ff;
				vol.readBw = itV->quotaSpaceMB;
				vol.writeBw = itV->quotaSpaceMB;

				vols.push_back(vol);

			}
			msg.setVolumes(vols);
		}
	}*/

	ZQ::Vrep::NextHopServer nhs;

	nhs.componentAddress	= stringToBytes(_strNextHop);
	nhs.streamingZone		= stringToBytes( _d4MsgCfg.strStreamZone );

	msg.setNextHopServer( nhs ); // next hop server

	
    msg.setTotalBandwidth( totalBW * 1000 ); // total bw
	msg.setAvailableBandwidth( availBW * 1000 ); // available bw
	msg.setCost(cost);
	msg.setTransferProtocolCapabilities(cap);

	mSpeaker.sendUpdate(msg, 0);

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(D4Speaker,"route address[%s],totalBw[%d],availBw[%d],cost[%d],cap[%x] volumeId[%s]"),_strNextHop.c_str(),totalBW ,availBW,cost,cap,_d4MsgCfg.strVolumeId.c_str());
}

ZQ::Vrep::byte D4Speaker::decideCapType( std::string methodType )
{
	ZQ::Vrep::byte capType;
	if (!stricmp(methodType.c_str(),METHODTYPE_NTFSRTFVSVSTRM)|| !stricmp(methodType.c_str(),METHODTYPE_NTFSRTFH264VSVSTRM))
	{
		capType = VREP_TransferProtocolCap_NFS | VREP_TransferProtocolCap_CIFS;
	}
	else if (!stricmp(methodType.c_str(),METHODTYPE_FTPRTFVSVSTRM)|| !stricmp(methodType.c_str(),METHODTYPE_FTPRTFH264VSVSTRM))
	{
		capType = VREP_TransferProtocolCap_FTP;
	}

	return capType;
}

bool D4Speaker::getVolumeInfo(TianShanIce::Storage::VolumeInfos& volInfos)
{
	if (!_csPrx)
	{
		try
		{
			volInfos = _csPrx->listVolumes("",true);
		}
		catch (const ::TianShanIce::BaseException& ex)
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(D4Speaker,"listvolumes() failed, caught exception[%s] %s"),ex.ice_name().c_str(), ex.message.c_str());
			return false;
		}
		catch (const ::Ice::Exception& ex)
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(D4Speaker,"listvolumes() failed, caught exception[%s]"),ex.ice_name().c_str());
			return false;
		}
		catch (...)
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(D4Speaker,"listvolumes() failed,caught unknown exception"));
			return false;
		}
		return true;
	}

}
