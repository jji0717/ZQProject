
#include <VrepSpeaker.h>
#include "D5Update.h"
#include "NgodConfig.h"
#include "SOPConfig.h"
#include <TianShanIceHelper.h>

#ifdef ZQ_OS_LINUX
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

using namespace ZQ::Vrep;
namespace NGOD
{

bytes stringToBytes(const std::string& str) 
{
	return bytes(str.begin(), str.end());
}

D5StateSinker::D5StateSinker( NgodEnv& environment , D5Speaker& speaker )
:mEnv(environment),
mD5Speaker(speaker)
{
}

D5StateSinker::~D5StateSinker()
{
}

void D5StateSinker::onStateChanged( ZQ::Vrep::StateDescriptor from, ZQ::Vrep::StateDescriptor to) 
{
	if( ZQ::Vrep::st_Active == to )
	{
		mD5Speaker.onConnected();
	}
}

void D5StateSinker::onEvent( ZQ::Vrep::Event e)
{

}


//////////////////////////////////////////////////////////////////////////
//D5Speaker
#ifdef ZQ_OS_MSWIN
#define FORALLOTHERSPEAKER( func ) {	std::vector<ZQ::Vrep::Speaker*>::iterator it = mOtherD5Speakers.begin();\
	for( ; it != mOtherD5Speakers.end() ; it ++ )\
{\
	(*it)->##func ;\
}}
#else
#define FORALLOTHERSPEAKER( func ) {	std::vector<ZQ::Vrep::Speaker*>::iterator it = mOtherD5Speakers.begin();\
	for( ; it != mOtherD5Speakers.end() ; it ++ )\
{\
	(*it)->func ;\
}}
#endif

D5Speaker::D5Speaker( NgodEnv& environment , NgodResourceManager&selManager , ZQ::common::NativeThreadPool& pool)
:ZQ::Vrep::Speaker( *environment.mMainLogger ,pool ),
mEnv(environment),
mSelManager(selManager),
mbQuit(true)
{
}
D5Speaker::~D5Speaker()
{
	std::vector<ZQ::Vrep::Speaker*>::iterator it = mOtherD5Speakers.begin();
	for( ; it != mOtherD5Speakers.end() ; it ++ )
	{
		if(*it)
		{			
			delete *it;
		}
	}
	mOtherD5Speakers.clear();
}

#define ABSOLUTEX(x) ((x)>0?(x):-(x))
bool D5Speaker::isChanged( int64 last , int64 current)
{
	static int32 diffPercent = ngodConfig.d5messsage.diffPercent;

	if( ( last <= 0 && current > 0 ) || ( last >0 && current <= 0  ) )
		return true;
	if( last > 0 )
	{
		int64 rate = ABSOLUTEX( (( last - current ) * 100 / last) );
		return rate >= diffPercent;
	}
	return false;
}
bool D5Speaker::isBWChanged( const BWUsage& last , const BWUsage& current )
{	
	return isChanged( last.totalBW , current.totalBW) || isChanged( last.availBW , current.availBW );
}

bool D5Speaker::checkBwUsage()
{
	bool bRet = false;
	SOPS sops; std::string temp;
	mSelManager.getSopData( sops , temp );
	
	SOPS::const_iterator itSop = sops.begin();

	for( ; itSop != sops.end() ; itSop++ )
	{
		const ResourceStreamerAttrMap& sop = itSop->second;

		bool bChanged =  false;

		int64 totalBW = 0;
		int64 availBW = 0;

		std::string lastNetId;
		std::string lastSourceIp;
		std::string lastVolumeName;

		ResourceStreamerAttrMap::const_iterator itStreamer = sop.begin();
		for( ; itStreamer != sop.end() ; itStreamer++ )
		{	
			const ResourceStreamerAttr& streamer = itStreamer->second;
			bool bStreamerAvail = streamer.bReplicaStatus && streamer.penalty <=0  && streamer.bMaintainEnable;

			if( bStreamerAvail )
			{
				totalBW += streamer.maxBw;
				int64 tmpAvailBW = streamer.maxBw - streamer.usedBw;
				tmpAvailBW = max(tmpAvailBW,0);
				availBW += tmpAvailBW;
			}			
		}
		int32 cost = 0;
		cost = totalBW > 0 ? (int32)(( totalBW-availBW)*256/totalBW) : 255 ;
		
		bool bUpdateBW = false;
		
		BWUsage lastUsage;
		BWUsage currentUsage;
		{
			ZQ::common::MutexGuard gd(mMutex);
			lastUsage = mSopBwUsage[itSop->first];
			
			currentUsage.totalBW	= totalBW;
			currentUsage.availBW	= availBW;
			
			bUpdateBW = isBWChanged( lastUsage , currentUsage );

			mSopBwUsage[ itSop->first ] = currentUsage;
		}
		if( bUpdateBW)
		{
			bRet = true;
			MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(D5Speaker,"BW info of SOP[%s]: totalBW[%lld/%lld] availBW[%lld/%lld]"),
				itSop->first.c_str() , lastUsage.totalBW , currentUsage.totalBW , lastUsage.availBW , currentUsage.availBW );
			
			std::string sopName = getGroupNameOfSop( itSop->first );

			sendBWChangedInfo( itSop->first , sopName, getSopPortId( itSop->first ) , lastSourceIp, 
				(int32)(totalBW/1000) , (int32)(availBW/1000) , 0 );
		}

	}
	return bRet;
}
int D5Speaker::run( )
{
	int32 interval = ngodConfig.d5messsage.msgUpdateInterval;

	interval = interval < 1000 ? 1000 : interval;
	
	int64 lastWakeupTime = ZQTianShan::now();

	while ( !mbQuit)
	{
		{
			ZQ::common::MutexGuard gd(mMutex);
			mCond.wait(mMutex,200);//check status every 200ms
		}
		
		int64 currentTime = ZQTianShan::now();
		if( (currentTime -lastWakeupTime) >= (int64)interval )
		{
			MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(D5Speaker,"update BW info"));
			onSpigotStateChange( );
			lastWakeupTime = ZQTianShan::now();
		}
		else if( checkBwUsage() )
		{
			lastWakeupTime = ZQTianShan::now();
		}
	}
	return 0;
}

void D5Speaker::onStateChanged( ZQ::Vrep::StateDescriptor from, ZQ::Vrep::StateDescriptor to)
{
	ZQ::Vrep::Speaker::onStateChanged( from , to );
	if( ZQ::Vrep::st_Established == to )
	{
		onConnected();
	}
}

void D5Speaker::onConnected( )
{
	sendServiceState( true );
	
	onSpigotStateChange();
}

void D5Speaker::onDisconnected()
{
	// ??
}

bool D5Speaker::initSpeakerConf( )
{
	ZQ::common::Config::Holder<NGOD::D5MessageConf>& d5Conf = ngodConfig.d5messsage;
	
	const std::vector<NGOD::SubD5Listener::SubListenerHolder>& subListener = ngodConfig.d5messsage.subListener;	
	
	if( d5Conf.d5serverIp.empty() )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(D5Speaker,"no d5server configured , quit ... "));
		return false;
	}

	ZQ::Vrep::Configuration conf;

	struct hostent * hostId = gethostbyname( d5Conf.nextHopServer.c_str() )	;
	if( hostId )
	{	
		conf.identifier = *((unsigned  long   *)hostId->h_addr_list[0]);
	}

	conf.streamingZone			= d5Conf.streamZone;
	conf.componentName			= getComponentName();
	conf.vendorString			= "SeaChange Intl";
	conf.defaultHoldTimeSec		= d5Conf.holdTimer/1000;
	conf.connectRetryTimeSec	= 60;
	conf.connectTimeoutMsec		= 2000;
	conf.keepAliveTimeSec		= d5Conf.keepAliveInterval/1000;
	conf.sendReceiveMode		= VREP_SendOnlyMode;

	enableAutoRestart( 10 );
	

	std::vector<NGOD::SubD5Listener::SubListenerHolder>::const_iterator itSubListener = subListener.begin(); 
	for( ; itSubListener != subListener.end() ; itSubListener ++ )
	{
		ZQ::Vrep::Speaker* pSpeaker = new ZQ::Vrep::Speaker(MLOG,mEnv.getThreadPool());
		if( itSubListener->d5serverIp.empty())
			continue;
		pSpeaker->setPeer(itSubListener->d5serverIp.c_str(),itSubListener->d5serverPort);
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(D5Speaker,"set d5 sub listener IP[%s] PORT[%d]"),
			itSubListener->d5serverIp.c_str(),itSubListener->d5serverPort);

		pSpeaker->enableAutoRestart(10);
		pSpeaker->start(conf);
	}

	setPeer( d5Conf.d5serverIp.c_str() , d5Conf.d5serverPort );
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(D5Speaker,"set d5 listener IP[%s] PORT[%d]"),
		d5Conf.d5serverIp.c_str() , d5Conf.d5serverPort);
	return ZQ::Vrep::Speaker::start(conf);
}

void D5Speaker::initOutputPort()
{
	SOPS sops; std::string temp;
	mSelManager.getSopData( sops , temp );
	
	SOPS::const_iterator itSop = sops.begin();

	PORTIDMAP volPortIds;
	for( ; itSop != sops.end() ; itSop++ )
	{
		bool bAllStreamerHasIC = true;
		VOLUMECAPACITYMAP volInfo;
		
		const ResourceStreamerAttrMap& sop = itSop->second;
		ResourceStreamerAttrMap::const_iterator itStreamer = sop.begin();

		for( ; itStreamer != sop.end() ; itStreamer++ )
		{
			const ResourceStreamerAttr& streamer = itStreamer->second;

			if( streamer.importChannelName.empty() )
				bAllStreamerHasIC = false;

			VOLUMECAPACITYMAP::iterator itVol = volInfo.find( streamer.volumeNetId );
			if( itVol != volInfo.end() )
			{
				itVol->second.capacity += streamer.maxBw;
			}
			else
			{
				VolumeInformation vinfo;
				vinfo.capacity	= streamer.maxBw;
				vinfo.portId	= 0;
				volInfo.insert( VOLUMECAPACITYMAP::value_type( streamer.volumeNetId, vinfo ) );
			}
			volPortIds.insert( PORTIDMAP::value_type( streamer.volumeNetId , 0 ) );
		}
		if( bAllStreamerHasIC )
		{
			VolumeInformation vinfo;
			vinfo.capacity	= 1000 * 1000 * 1000;
			vinfo.portId	= 0;
			std::string dumyVolName = "library";
			volInfo.insert( VOLUMECAPACITYMAP::value_type( dumyVolName , vinfo ) );
			volPortIds.insert( PORTIDMAP::value_type( dumyVolName , 0 ) );
		}

		mSopVolInfos.insert( SOPVOLCAPACITYMAP::value_type( itSop->first, volInfo ) );
		const std::string& sopName = itSop->first;
		std::string::size_type dotPos = sopName.find_last_of('.');
		int portId = 0;
		if( dotPos !=std::string::npos )
		{
			std::string portIdStr = sopName.substr( dotPos+1);
			if(!portIdStr.empty())
				portId = atoi(portIdStr.c_str());
		}
		mSopOutputPorts.insert( PORTIDMAP::value_type( itSop->first , portId ) );
	}

	int tmpPortId = 1;
	PORTIDMAP::iterator itPort = volPortIds.begin();
	for( ; itPort != volPortIds.end() ; itPort++ )
	{
		itPort->second = tmpPortId ++;
	}
	
	tmpPortId = 1;
	SOPVOLCAPACITYMAP::iterator itSopForVol = mSopVolInfos.begin();	
	for( ; itSopForVol != mSopVolInfos.end() ; itSopForVol ++ )
	{
		VOLUMECAPACITYMAP& volcapacity = itSopForVol->second;
		VOLUMECAPACITYMAP::iterator itCap = volcapacity.begin();
		for( ; itCap != volcapacity.end() ; itCap++ )
		{
			PORTIDMAP::iterator itTmp = volPortIds.find(itCap->first);
			if(itTmp != volPortIds.end())
			{
				itCap->second.portId = itTmp->second;
			}
		}
	}
}

std::string	D5Speaker::getASopName( ) const
{
	if ( mSopOutputPorts.empty())
	{
		return "unkown";
	}
	else
	{
		return mSopOutputPorts.begin()->first;
	}
}

std::string D5Speaker::getComponentName( ) const
{
	std::string componentName;
	if( mSopOutputPorts.empty() )
	{
		return "";
	}
	else
	{
		componentName = mSopOutputPorts.begin()->first;
		std::string::size_type pos = componentName.find_last_of('.');
		if( pos != std::string::npos )
		{
			componentName = componentName.substr(0,pos);
		}
		return componentName;
	}
}

bool D5Speaker::start()
{	
	if( ngodConfig.d5messsage.enable <= 0 )
	{
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(D5Speaker,"d5 interface disabled"));
		return false;
	}
	mbQuit = false;
	initOutputPort();
	if( !initSpeakerConf() )
	{
		mbQuit = true;
		return false;
	}
	MLOG( ZQ::common::Log::L_INFO, CLOGFMT(D5Speaker,"start D5Speaker"));
	return ZQ::common::NativeThread::start();
}

void D5Speaker::stop()
{
	if( mbQuit == false )
	{
		sendServiceState( false );
		mbQuit = true;
		mCond.signal();
		waitHandle(10000);
		//TODO: close speaker ?
	}

	FORALLOTHERSPEAKER(stop());
	ZQ::Vrep::Speaker::stop();
	MLOG( ZQ::common::Log::L_INFO, CLOGFMT(D5Speaker,"D5Speaker stopped"));
}

void D5Speaker::sendServiceState( bool bUp )
{
	if(mbQuit)
		return;
	ZQ::Vrep::UpdateMessage msg;
	msg.setServiceStatus( bUp ? VREP_ServiceStatus_Operational : VREP_ServiceStatus_ShuttingDown );
	sendUpdate(msg, 0);	
	FORALLOTHERSPEAKER(sendUpdate(msg, 0));
	MLOG( ZQ::common::Log::L_INFO, CLOGFMT(D5Speaker,"service state changed message sent out"));
}

void D5Speaker::onSpigotStateChange( const std::string& netId , bool bUp )
{
	if(mbQuit)
		return;
	onSpigotStateChange();
}

int32 D5Speaker::getSopPortId( const std::string& netId) const
{
	PORTIDMAP::const_iterator it = mSopOutputPorts.find(netId);
	if( it != mSopOutputPorts.end() )
		return it->second;
	return 0;
}


void D5Speaker::onSpigotStateChange( )
{
	if(mbQuit)
		return;
	SOPS sops; std::string temp;
	mSelManager.getSopData( sops , temp );
	
	SOPS::const_iterator itSop = sops.begin();
	
	for( ; itSop != sops.end() ; itSop++ )
	{
		const ResourceStreamerAttrMap& sop = itSop->second;

		bool bChanged =  false;

		int64 totalBW = 0;
		int64 availBW = 0;

		std::string lastNetId;
		std::string lastSourceIp;
		std::string lastVolumeName;

		ResourceStreamerAttrMap::const_iterator itStreamer = sop.begin();
		for( ; itStreamer != sop.end() ; itStreamer++ )
		{	
			const ResourceStreamerAttr& streamer = itStreamer->second;
			bool bStreamerAvail = streamer.bReplicaStatus && streamer.penalty <=0  && streamer.bMaintainEnable;
			if( bStreamerAvail )
			{
				totalBW += streamer.maxBw;
				int64 tmpAvailBW = streamer.maxBw - streamer.usedBw;
				tmpAvailBW = max(tmpAvailBW,0);
				availBW += tmpAvailBW;
			}			
		}
		int32 cost = 0;
		cost = totalBW > 0 ? (int32)(( totalBW-availBW)*256/totalBW) : 255 ;
		
		{
			{
				ZQ::common::MutexGuard gd(mMutex);
				BWUsage usage;
				usage.totalBW	= totalBW;
				usage.availBW	= availBW;
				mSopBwUsage[ itSop->first ] = usage;
			}
			std::string sopName = getGroupNameOfSop( itSop->first );
			sendBWChangedInfo( itSop->first , sopName, getSopPortId(itSop->first) , lastSourceIp, (int32)(totalBW/1000) , (int32)(availBW/1000) , 0 );
		}
	}
}

bytes getIpaddressThroughName( const std::string& hostname )
{
	bytes retip;
	if(hostname.empty())
		return retip;
	if( !isalpha( hostname.at(0)) ) 
	{
		retip = stringToBytes( hostname );
	}
	else
	{
		struct hostent* host = gethostbyname( hostname.c_str() );
		if( host )
		{
			struct in_addr addr;
			memset( &addr , 0 , sizeof(addr) );
			addr.s_addr = *(u_long *)( host->h_addr_list[0] ) ;
			retip = stringToBytes( inet_ntoa( addr ) );
		}
	}
	return retip;
}

void D5Speaker::setRouteAndHopServerInfo( const std::string& sopGroupName , ZQ::Vrep::Route& r ,ZQ::Vrep::NextHopServer& n)
{
	bytes serverIpBytes;
	ZQ::common::Config::Holder<NGOD::D5MessageConf>& d5Conf = ngodConfig.d5messsage;	

	r.family		= VREP_AddressFamily_NGOD;
	r.protocol		= VREP_AppProtocol_R2;	
	r.address		= getIpaddressThroughName(d5Conf.nextHopServer);
	r.name			= stringToBytes( sopGroupName );		 

	n.componentAddress	= getIpaddressThroughName(d5Conf.nextHopServer);
	n.streamingZone		= stringToBytes( d5Conf.streamZone );
}

void D5Speaker::getVolumeInfo( const std::string& sopName , ZQ::Vrep::Volumes& vols ) const
{
	SOPVOLCAPACITYMAP::const_iterator itSop = mSopVolInfos.find(sopName);
	if( itSop == mSopVolInfos.end() )
		return;
	const VOLUMECAPACITYMAP& volInfo = itSop->second;
	
	VOLUMECAPACITYMAP::const_iterator itVol = volInfo.begin();
	
	for( ; itVol != volInfo.end() ; itVol++ )
	{
		ZQ::Vrep::Volume v;
		v.name		= stringToBytes(itVol->first);
		v.portId	= itVol->second.portId;
		v.readBw	= (dword)(itVol->second.capacity/1000);
		v.writeBw	= 0;
		vols.push_back(v);
	}
}

void D5Speaker::sendBWChangedInfo( const std::string& sopName , const std::string& sopGroupName , int32 portId ,  const std::string& sourceIp , int32 totalBW , int32 availBW , int32 cost)
{
	if(mbQuit)
		return;

	ZQ::common::Config::Holder<NGOD::D5MessageConf>& d5Conf = ngodConfig.d5messsage;

	ZQ::Vrep::UpdateMessage msg;
	// reachable route
	ZQ::Vrep::Route r;
	ZQ::Vrep::NextHopServer nhs;	
	setRouteAndHopServerInfo( sopGroupName, r , nhs );
	ZQ::Vrep::Routes rs;
	rs.push_back(r);

	msg.setReachableRoutes(rs);
	msg.setNextHopServer( nhs ); // next hop server
	msg.setTotalBandwidth( totalBW ); // total bw
	msg.setAvailableBandwidth( availBW ); // available bw
	msg.setOutputPort( portId );	
	msg.setOutputAddress( getIpaddressThroughName(sourceIp) );
	
	ZQ::Vrep::Volumes vols;
	getVolumeInfo( sopName , vols );
	msg.setVolumes(vols);

	sendUpdate(msg, 0);
	FORALLOTHERSPEAKER(sendUpdate(msg, 0));
	MLOG( ZQ::common::Log::L_INFO, CLOGFMT(D5Speaker,"BWChange message sent out sop[%s] totalBW[%d] availBW[%d] portId[%d] ") , 
		sopName.c_str() , totalBW , availBW ,portId );
}

void D5Speaker::sendVolumesInfo( )
{
	if(mbQuit)
		return;
}

void D5Speaker::sendMaxStreamsInfo( )
{
	if(mbQuit)
		return;
}

std::string D5Speaker::getGroupNameOfSop( const std::string& sopName ) const
{
	const std::map< std::string ,  NGOD::SOPRestriction::SopHolder >& sops =  sopConfig.sopRestrict.sopDatas;
	std::map< std::string ,  NGOD::SOPRestriction::SopHolder >::const_iterator it = sops.find(sopName);
	if( it == sops.end() )
		return std::string("");
	return it->second.sopGroupName;
}

}
