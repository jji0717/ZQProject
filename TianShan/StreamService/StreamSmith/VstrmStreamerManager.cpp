
#include <ZQ_common_conf.h>
#include <TianShanDefines.h>
#include <TianShanIceHelper.h>
#include <iostream>
#include <assert.h>
#include "VstrmStreamerManager.h"
#include "HelperClass.h"
#include "StreamSmithConfig.h"

#include <memoryDebug.h>

namespace ZQ
{
namespace StreamService
{

VstrmStreamerManager::VstrmStreamerManager( SsEnvironment* environment )
:env(environment),
mRSMonitor(environment)
{
	mVstrmHandle = 0;
}

VstrmStreamerManager::~VstrmStreamerManager( )
{
	uninitialize( );
}

void VstrmStreamerManager::attachServiceInstance( SsServiceImpl* s )
{
	assert( s != NULL );
	ss = s;
}

bool VstrmStreamerManager::isVstrmNodeReady( )
{
	VHANDLE		hVstrmClass = 0;
	if( VstrmClassOpenEx(&hVstrmClass) != VSTRM_SUCCESS )
	{
		hVstrmClass = NULL;
		return false;
	}
	CLUSTER_INFO clusterInfo;
	ULONG ret;
	if( VstrmClassGetClusterDataEx(hVstrmClass ,&clusterInfo,sizeof(clusterInfo) , &ret) == VSTRM_SUCCESS )
	{
		VstrmClassCloseEx( hVstrmClass );
		hVstrmClass = NULL;
		//printf("node state = %d \n" , clusterInfo.localNodeState );
		if( clusterInfo.localNodeState < ORANGE )
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else	
	{
		VstrmClassCloseEx( hVstrmClass );
		hVstrmClass = NULL;
		return true;
	}
}

void VstrmStreamerManager::uninitialize( )
{
	mbThreadQuit = true;

	if( mVstrmHandle )
	{
		VstrmClassCloseEx( mVstrmHandle );
		mVstrmHandle = NULL;
	}
}

bool VstrmStreamerManager::initialize( )
{
	if( !isVstrmNodeReady() )
	{
		ENVLOG(ZQ::common::Log::L_WARNING,CLOGFMT(VstrmStreamerManager,"initialize() node is not ready"));
		return false;
	}
	if(mVstrmHandle)
		VstrmClassCloseEx( mVstrmHandle );
	if( VstrmClassOpenEx( &mVstrmHandle ) != VSTRM_SUCCESS )
	{
		mVstrmHandle = NULL;
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VstrmStreamerManager,"initialize() node is ready but can't open vstrm class"));
		return false;
	}
	CLUSTER_INFO clusterInfo;
	ULONG ret;
	if( VstrmClassGetClusterDataEx( mVstrmHandle ,&clusterInfo,sizeof(clusterInfo) , &ret) != VSTRM_SUCCESS )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VstrmStreamerManager,"initialize() node is ready but can't get cluster id"));
#pragma message(__MSGLOC__"TODO: return false or just go on ")
		return false;
	}
	else
	{
		mClusterId = static_cast<int32>(clusterInfo.clusterId);
		ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(VstrmStreamerManager,"initialize() get cluster id [%d]"),mClusterId);
	}

	if( !enumStreamer( ) )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VstrmStreamerManager,"initialize() node is ready but failed to enum streamers "));
		return false;
	}

	return start( );
}

void VstrmStreamerManager::vstrmSpigotCallback( void*							pCallback,
						 PSPG_SPIGOT_CHARACTERISTICS	spigotChars,
						 ULONG							spigotCharsLength,
						 ULONG							spigotIndex,
						 ULONG							spigotCount)
{
	assert( pCallback != NULL );
	//innervstrmSpigotCallback
	VstrmStreamerManager* pThis = reinterpret_cast<VstrmStreamerManager*>( pCallback );
	pThis->innervstrmSpigotCallback( spigotChars, spigotCharsLength , spigotIndex , spigotCount );
}

bool VstrmStreamerManager::getStreamerAddress( ULONG boardNum , ULONG spigotIdx , std::string& ipv4 , std::string& ipv6 , int32& basePort )
{
	static const char* pKeyPath = "SYSTEM\\CurrentControlSet\\Services\\EdgeDrv\\Ip";
	char	szKey[1024];
	char	szPath[1024];
	sprintf(szPath,"%s%d\\Adapter%d",pKeyPath, boardNum ,spigotIdx );
	sprintf(szKey,"baseUdpPort");

	DWORD baseUdpPort = 0;
	
	if(!HelperClass::GetRegistryValue(szPath,szKey,baseUdpPort))
	{
		ENVLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(getStreamerAddress,"can't get streamer's base udp port with boardNum[%d] spigotIdx[%d]"),
			boardNum , spigotIdx );
	}
	else
	{
		basePort = baseUdpPort;
	}
	sprintf(szKey,"SrcIpAddr");
	if(!HelperClass::GetRegistryValue(szPath,szKey,ipv4))
	{
		DWORD dwIpv4 = 0;
		if(!HelperClass::GetRegistryValue(szPath,szKey,dwIpv4))
		{
			ENVLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(getStreamerAddress,"can't get streamer's ipv4 address with boardNum[%d] spigotIdx[%d]"),
				boardNum , spigotIdx );
		}
		else
		{
			char szSourceIpBuf[256];
			unsigned char a , b , c, d;
			d =(unsigned char)( dwIpv4 >> 24 );
			c =(unsigned char)( ( dwIpv4 & 0x00ff0000) >> 16 );
			b =(unsigned char)( (dwIpv4 & 0x0000ff00) >> 8 ) ;
			a =(unsigned char)( dwIpv4 & 0x000000ff);
			sprintf(szSourceIpBuf,"%d.%d.%d.%d",a,b,c,d) ;
			ipv4 = szSourceIpBuf;
		}
	}

	sprintf(szKey,"SrcIp6Addr");
	if(!HelperClass::GetRegistryValue(szPath,szKey,ipv6))
	{
		ENVLOG(ZQ::common::Log::L_WARNING,
			CLOGFMT(getStreamerAddress,"can't get streamer's ipv6 address with boardNum[%d] spigotIdx[%d]"),
			boardNum , spigotIdx );
	}

	return true;
}

std::string VstrmStreamerManager::getVstrmError( ULONG errCode ) const
{
	char szErrorBuf[1024]={0};
	VstrmClassGetErrorText( mVstrmHandle , errCode , szErrorBuf, sizeof(szErrorBuf )-1 );
	return std::string(szErrorBuf);
}

int32 VstrmStreamerManager::getSpigotLinkState( ULONG portId )
{
	VSTATUS					vStatus				= VSTRM_NOT_SUPPORTED; 
	ATTRIBUTE_ARRAY			getBuf				= {0}; 
	UCHAR					getLinkState		= 0; 
	ULONG					localPortCount		= 0; 

	getBuf.attributeArray[0].attributeCode = VSTRM_ATTR_GEN_LINK_STATE; 
	getBuf.attributeArray[0].attributeValueLength = sizeof(getLinkState); 
	getBuf.attributeArray[0].attributeValueP = (PVOID) &getLinkState; 
	getBuf.attributeCount = (USHORT) 1; 

	vStatus = VstrmClassGetPortAttributesEx( mVstrmHandle, 
											(portId),        // use a portId specific to board, they are 
											&getBuf );        // allocated in increments specific to card type 

	if ( VSTRM_SUCCESS != vStatus ) 
	{ 
		glog(ZQ::common::Log::L_ERROR,
			CLOGFMT(getSpigotLinkState,"failed to get the spigot linkage state for port[%d] and error [%s]"),
			portId,
			getVstrmError( VstrmGetLastError()).c_str() );
		return SPIGOT_LINK_DOWN;
	} 

	if (getLinkState == 0) 
	{ 
		return SPIGOT_LINK_DOWN;
	} 
	else if (getLinkState == 1) 
	{ 
		return SPIGOT_LINK_UP;
	} 
	glog(ZQ::common::Log::L_ERROR,CLOGFMT(getSpigotLinkState,"Can't get spigot linkage status for port[%d]"), 	portId  );
	return SPIGOT_LINK_UP;
}

void VstrmStreamerManager::innervstrmSpigotCallback(	 PSPG_SPIGOT_CHARACTERISTICS		spigotChars,
														 ULONG							spigotCharsLength,
														 ULONG							spigotIndex,
														 ULONG							spigotCount )
{
	ENVLOG(ZQ::common::Log::L_INFO,
		CLOGFMT(SpigotCallback,"spigot callback with spigotIndex[%d] spigotCount[%d] "
		"boardNumber[%d] firstPortIndex[%d] portCount[%d]"),
		spigotIndex , spigotCount,
		spigotChars->boardNum , 
		spigotChars->firstPortIndx ,
		spigotChars->portCount );
	
	std::string		ipv4;
	std::string		ipv6;
	int32			basePort;
	getStreamerAddress( spigotChars->boardNum , spigotIndex , ipv4 , ipv6, basePort );
	
	SpigotAttr attr;
	attr.spigotId		=	spigotChars->spigotId;
	attr.baseUdpPort	=	basePort;
	attr.spigotIpv4		=	ipv4;
	attr.spigotIpv6		=	ipv6;
	attr.portLowBound	=	spigotChars->firstPortIndx;
	attr.portHighBound	=	spigotChars->firstPortIndx + spigotChars->portCount - 1;
	attr.status			=	getSpigotLinkState( attr.portLowBound );
	attr.lastUpdate		=	ZQTianShan::now();
	attr.firstUp		=	(attr.status == SPIGOT_LINK_UP) ? ZQTianShan::now() : 0 ;
	//log it
	ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(SpigotCallBack,
		"spigot[%d] idx[%d] boardNum[%d] count[%d] id[%d] baseUdoPort[%d] ipv4[%s] ipv6[%s] portLow[%d] portHigh[%d] status[%s]"),
		mSpigotIndex,spigotIndex,spigotChars->boardNum,
		spigotCount,attr.spigotId ,
		attr.baseUdpPort,
		attr.spigotIpv4.c_str(),
		attr.spigotIpv6.c_str(),
		attr.portLowBound,
		attr.portHighBound,
		(attr.status == SPIGOT_LINK_UP) ? "Link Up":"Link Down");

	


	{//check all port belong to this spigot
		ZQ::common::MutexGuard gd(mSpigotsMutex);
		if( attr.portHighBound >= static_cast<int32>( mTmpPorts.size() ) )
		{
			ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SpigotCallBack,"bad vstrm port attribute, this only [%d] ports ,"
				" but spigot comes with firstIdx[%d] count[%d]"),
				mTmpPorts.size() ,
				spigotChars->firstPortIndx,
				spigotChars->portCount);
		}
		else
		{			
			for(int32 i = attr.portLowBound ; i <= attr.portHighBound ; i ++ )
			{
				mTmpPorts[i].portAvailableOnSpigot = true;
				attr.ports.push_back(mTmpPorts[i]);
			}
			mSpigots.insert( SPIGOTS::value_type(mSpigotIndex,attr));
		}
	}	
	mSpigotIndex++;	
}

VSTATUS VstrmStreamerManager::vstrmPortCallback(	HANDLE					VstrmClassHandle,
												  PVOID				    pCtx,
												  PEPORT_CHARACTERISTICS	portChars,
												  ULONG					portCharSize,
												  ULONG					currentPort,
												  ULONG					portCount)
{
	assert( pCtx !=NULL );
	VstrmStreamerManager* pThis = reinterpret_cast<VstrmStreamerManager*>(pCtx);
	return pThis->innervstrmPortCallback( VstrmClassHandle , portChars ,portCharSize,currentPort , portCount );

}



VSTATUS VstrmStreamerManager::innervstrmPortCallback(	HANDLE					VstrmClassHandle,													
												   PEPORT_CHARACTERISTICS	portChars,
												   ULONG					portCharSize,
												   ULONG					currentPort,
												   ULONG					portCount)
{
	PORT_CHARACTERISTICS& pc = portChars->PortCharacteristics;
	if ( ( pc.DeviceType == NULL_DEVICE_C) ||
		(	IS_VIDEO_PORT( pc.DeviceType ) &&
		!( IS_INPUT_PORT(pc)) 
#ifdef IS_FSI_PORT
		&& !( IS_FSI_PORT(pc.DeviceType) )	
#endif
		)	)
	{
		PortAttr		attr;
		attr.portId					=	currentPort;
		attr.portStatus				=	VSTRM_PORT_FREE;
		attr.portAvailableOnSpigot	=	false; // need to be checked in spigotcallback		
		mTmpPorts.push_back(attr);
		ENVLOG(ZQ::common::Log::L_INFO,
			LOGFMT("found an available stream output vstrmPort [%d] of [%d]"), 
			currentPort, portCount );
	}
	ENVLOG(ZQ::common::Log::L_INFO,
		"PortType:(%d) PortID:(%d) SubHandle:(%d) DeviceUnit:(%d) DeviceClass:(%d)"
		" ConsumerFeatures:(%u) serialNumber:(%d) spigot:(%d) subUnit:(%d) DriverState:(%d)",
		pc.DeviceType,
		portChars->Handle,
		portChars->SubHandle,
		portChars->DeviceUnit,
		pc.DeviceClass,
		pc.ConsumerFeatures,
		pc.ExtendedVersionChars.QmpVersionChars.serialNumber,
		pc.ExtendedVersionChars.QmpVersionChars.boardNumber,
		pc.ExtendedVersionChars.QmpVersionChars.subUnit,
		portChars->DriverState);

	return (VSTRM_SUCCESS);
}



bool VstrmStreamerManager::enumStreamer( )
{
	ZQ::common::MutexGuard gd( mSpigotsMutex );
	mSpigots.clear( );
	mTmpPorts.clear();

	//enum spigots
	
	//enum ports
	if (VSTRM_SUCCESS != VstrmClassForEachPort( mVstrmHandle, NULL, &vstrmPortCallback, this ) )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(VstrmStreamerManager,"enumStreamer() failed to call  VstrmClassForEachPort with error [%s]"),
			getVstrmError( VstrmGetLastError()).c_str() );
		return false;
	}

	HANDLE	hSpigotsHandle = SpigotGetDllClassHandle( );
	if( !hSpigotsHandle )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VstrmStreamerManager,"enumStreamer() failed to get SpigotHandle"));
		return false;
	}
	
	mSpigotIndex = 0;
	
	if( SpigotClassForEachSpigot( hSpigotsHandle , this, NULL , vstrmSpigotCallback  ) != VSTRM_SUCCESS )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(VstrmStreamerManager,"enumStreamer() failed to call SpigotClassForEachSpigot with error[%s]"),getVstrmError( VstrmGetLastError() ).c_str() );
		return false;
	}
	//check all ports
	int32 iPortCount = static_cast<int32>(mTmpPorts.size());
	for( int32 i = 0; i < iPortCount ; i ++ )
	{
		if( ! mTmpPorts[i].portAvailableOnSpigot )
		{
			ENVLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(enumStreamer(),"port [%d] is not belong to any spigot"),
				mTmpPorts[i].portId );
			return false;
		}
	}
	return true;
}

bool VstrmStreamerManager::getStreamingSourceAddress( const std::string& streamerId , const std::string& strPortId , 
													 std::string& Ipv4, std::string& Ipv6, int32& port )
{
	int spigotId	= 0;
	int portId		= 0;
	
	if( streamerId.empty() || strPortId.empty() )
		return false;

	sscanf( streamerId.c_str() ,  "Spigot%02d" ,&spigotId );
	portId	= atoi( strPortId.c_str() );

	{
		ZQ::common::MutexGuard gd(mSpigotsMutex);
		SPIGOTS::const_iterator itSpigot = mSpigots.find(spigotId);
		if( itSpigot == mSpigots.end() )
			return false;
		const SpigotAttr& attr = itSpigot->second;
		if( portId < attr.portLowBound || portId > attr.portHighBound )
			return false;
		Ipv4	= attr.spigotIpv4;
		Ipv6	= attr.spigotIpv6;
		port	= portId - attr.portLowBound  + attr.baseUdpPort;
	}
	return true;

}

bool VstrmStreamerManager::allocateStreamPort(	IN const std::string& /*streamerId*/ ,
										   IN const std::string& /*portId*/ ,
										   IN const TianShanIce::SRM::ResourceMap&	/*resource*/ )
{
	return true;
}

bool VstrmStreamerManager::releaseStreamPort(	IN const std::string& /*streamerId*/ ,
												IN const std::string& /*portId*/ )
{
	return true;
}


bool VstrmStreamerManager::listAllReplicas( OUT SsReplicaInfoS& infos )
{
	ZQ::common::MutexGuard gd(mSpigotsMutex);
	
	SsReplicaInfo info;
	
	SPIGOTS::const_iterator itSpigot = mSpigots.begin();
	for( ; itSpigot != mSpigots.end() ; itSpigot ++ )
	{
		const SpigotAttr& attr = itSpigot->second;

		info.ports.clear();
		char szBuf[32];
		sprintf(szBuf,"Spigot%02d", itSpigot->first );
		info.replicaId		=	 szBuf;
		info.streamerType	=	"IpEdge";
		info.replicaState	=	( attr.status == SPIGOT_LINK_UP ) ? TianShanIce::stInService : TianShanIce::stOutOfService;
		info.category		=	"Streamer";
		info.bHasPorts		=	true;
		info.groupId		=	gStreamSmithConfig.szServiceID;
		
		PORTS::const_iterator itPort = attr.ports.begin();
		for( ; itPort != attr.ports.end() ; itPort++ )
		{
			std::ostringstream oss;
			oss << itPort->portId;
			info.ports.push_back( oss.str() );
		}		
		infos.insert( SsReplicaInfoS::value_type( info.replicaId , info ) );
	}
#pragma message(__MSGLOC__"TODO: add import channel replica here")
	//add import channel replica

	return infos.size() > 0;
}

int32 VstrmStreamerManager::getTotalVstrmPortCount( ) 
{
	ZQ::common::MutexGuard gd(mSpigotsMutex);
	return	static_cast<int32>(mTmpPorts.size());
}

VHANDLE	 VstrmStreamerManager::getVstrmHandle( ) const
{
	return mVstrmHandle;
}

ULONG	VstrmStreamerManager::getSpigotHandle( ULONG vstrmPort ) const
{
	int32 port = static_cast<int32>(vstrmPort);
	ULONG spigotHandle = -1;
	{
		ZQ::common::MutexGuard gd(mSpigotsMutex);
		SPIGOTS::const_iterator it = mSpigots.begin();
		for( ; it != mSpigots.end() ; it ++ )
		{
			if ( (port >= it->second.portLowBound) && (port <= it->second.portHighBound) )
			{
				return it->second.spigotId;
			}
		}
	}
	return spigotHandle;
}


void	VstrmStreamerManager::updateSpigotStatus( ULONG spigotHandle , int32 status )
{
	ZQ::common::MutexGuard gd(mSpigotsMutex);
	SPIGOTS::iterator it = mSpigots.begin();
	for( ; it != mSpigots.end() ; it ++ )
	{
		if( it->second.spigotId == spigotHandle )
		{
			it->second.status	= status;
			ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VstrmStreamerManager,"update spigot [%d] with status [%s]"),
				it->first ,
				status == SPIGOT_LINK_UP  ? "LINK_UP" : "LINK_DOWN");
			//notify replica reporter to get the replica information again
			std::ostringstream oss;
			oss<<it->first;
			TianShanIce::Properties props;
			SsServiceImpl::SsReplicaEvent event;
			if( status == SPIGOT_LINK_UP )
				event = SsServiceImpl::sreInService;
			else
				event = SsServiceImpl::sreOutOfService;
			
			ss->OnReplicaEvent( event , oss.str(), props );

			return ;
		}
	}
	ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VstrmStreamerManager,"updateSpigotStatus() can't find spigot with ID[%u]"),
		spigotHandle);
}

RemoteSessionMonitor& VstrmStreamerManager::getRSMonitor( )
{
	return mRSMonitor;
}

int VstrmStreamerManager::run( )
{
	if( VstrmClassOpenEx(&mEdgeEventVstrmHandle) != VSTRM_SUCCESS )
	{
		mEdgeEventVstrmHandle = NULL;
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VstrmStreamerManager,"can't open vstrm class handle when trying to scan edge event"));
		return -1;
	}

	EDGE_EVENT_DATA         tagEventData            = {0};
	ZeroMemory( &tagEventData, sizeof(tagEventData) );

	HANDLE					hChange					= INVALID_HANDLE_VALUE;
	bool					bOk						= true;

	char					szErrorBuf[512];

	__try
	{
		hChange = VstrmFindFirstEdgeNotification(   mEdgeEventVstrmHandle , &tagEventData );
		if( INVALID_HANDLE_VALUE == hChange )
		{	
			VstrmClassGetErrorText(mEdgeEventVstrmHandle , VstrmGetLastError() , szErrorBuf, sizeof(szErrorBuf )-1 );
			ENVLOG(ZQ::common::Log::L_CRIT ,CLOGFMT(VstrmStreamerManager,"EdgeEventListener failed to call VstrmFindFirstEdgeNotification() , "
				"exit edge event scanning , [%s]"),
				szErrorBuf );			
			return	-1;
		}
		while(bOk)
		{
			switch( tagEventData.edgeEvent )
			{
			case VSTRM_EDGE_EVENT_REMOTE_SESSION_AVAILABLE:
				{
					ENVLOG(ZQ::common::Log::L_INFO ,
						"EdgeEventListener() remote session available : port[%u] spigot[%d]",
						tagEventData.portHandle,
						tagEventData.spigotHandle);
					//std::string sessionId( (char*)tagEventData.pokeSessionId);
					mRSMonitor.updateSession( (char*)tagEventData.pokeSessionId ,1 );					
				}
				break;
			case VSTRM_EDGE_EVENT_LOST:
				{
					ENVLOG(ZQ::common::Log::L_WARNING , "EdgeEventListener() vstrm edge event lost");					
				}
				break;
			case VSTRM_EDGE_EVENT_NO_LISTENER_FOUND:
				{
					ENVLOG(ZQ::common::Log::L_INFO,
						"EdgeEventListener() VSTRM_EDGE_EVENT_NO_LISTENER_FOUND is found with port[%u] spigot[%d]",
						tagEventData.portHandle,
						tagEventData.spigotHandle);					
				}
				break;
			case VSTRM_EDGE_EVENT_LINK_UP:
				{
					ENVLOG(ZQ::common::Log::L_INFO,
						"EdgeEventListener() VSTRM_EDGE_EVENT_LINK_UP is found with port[%u] spigot[%d]",
						tagEventData.portHandle,
						tagEventData.spigotHandle);
					updateSpigotStatus( tagEventData.spigotHandle, SPIGOT_LINK_UP );
				}
				break;
			case VSTRM_EDGE_EVENT_LINK_DOWN:
				{
					ENVLOG(ZQ::common::Log::L_INFO,
						"EdgeEventListener() VSTRM_EDGE_EVENT_LINK_DOWN is found with port[%u] spigot[%d]",
						tagEventData.portHandle,
						tagEventData.spigotHandle);
					updateSpigotStatus( tagEventData.spigotHandle, SPIGOT_LINK_DOWN );
				}
				break;
			default:
				{
					ENVLOG(ZQ::common::Log::L_INFO,
						"EdgeEventListener() unknown egde event is found with port[%u] spigot[%d]",
						tagEventData.portHandle,
						tagEventData.spigotHandle);
				}
			}
			ZeroMemory( &tagEventData, sizeof(tagEventData) );

			//
			// Wait for next Vod event notification
			//
			bOk = VstrmFindNextEdgeNotification(  mEdgeEventVstrmHandle, hChange, &tagEventData );
			if(!bOk)
			{
				VstrmClassGetErrorText(mEdgeEventVstrmHandle , VstrmGetLastError() , szErrorBuf, sizeof(szErrorBuf )-1 );
				ENVLOG(ZQ::common::Log::L_ERROR,
					"EdgeEventListener() failed to call VstrmFindNextEdgeNotification() [%s] ",
					szErrorBuf  );
				//cout<<"failed to call VstrmFindNextEdgeNotification"<<endl;
			}
			//Sleep(100);
		}
	}
	__finally
	{
		VstrmFindCloseEdgeNotification( mEdgeEventVstrmHandle , hChange );		
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//RemoteSessionMonitor
RemoteSessionMonitor::RemoteSessionMonitor(  SsEnvironment* environment  )
:env(environment)
{
}
RemoteSessionMonitor::~RemoteSessionMonitor()
{
	ZQ::common::MutexGuard gd(mMutex);
	RSMAP::iterator it = mRsMap.begin();
	for( ; it != mRsMap.end() ; it ++ )
	{
		if( it != mRsMap.end() )
		{
			delete (it->second);
		}
	}
	mRsMap.clear();
}
void RemoteSessionMonitor::registerSession(  const std::string& sessionId )
{
	ZQ::common::MutexGuard gd(mMutex);
	RSMAP::iterator it = mRsMap.find(sessionId);
	if( it != mRsMap.end() )
		return;
	ZQ::common::Cond* pCond = new ZQ::common::Cond();
	assert( pCond != NULL );
	mRsMap.insert(RSMAP::value_type(sessionId,pCond));
}

void RemoteSessionMonitor::unregisterSession( const std::string& sessionId )
{
	ZQ::common::MutexGuard gd(mMutex);
	RSMAP::iterator it = mRsMap.find(sessionId);
	if( it != mRsMap.end() )
	{
		delete (it->second);
		mRsMap.erase(it);
	}
}

bool RemoteSessionMonitor::hasSession( const std::string& sessionId ) 
{
	ZQ::common::MutexGuard gd(mMutex);
	RSMAP::const_iterator it = mRsMap.find(sessionId);
	return  it != mRsMap.end();
}
bool RemoteSessionMonitor::checkRemoteSession( const std::string& sessionId , uint32 timeInMilliSeconds )
{
	ZQ::common::Cond* pCond = NULL;

	ZQ::common::MutexGuard gd(mMutex);
	RSMAP::const_iterator it = mRsMap.find(sessionId);
	if( it == mRsMap.end())
	{//no associated session Id , return true
		return true;
	}

	pCond = it->second;
	if( pCond->wait(mMutex,timeInMilliSeconds) )
	{
		unregisterSession( sessionId );
		return true;
	}
	else
	{
		return false;
	}
}

void RemoteSessionMonitor::updateSession( const char* sid , int32 status )
{	
	std::string sessionId(sid);
	ZQ::common::MutexGuard gd(mMutex);
	RSMAP::const_iterator it = mRsMap.find(sessionId);
	if( it == mRsMap.end())
	{
		ENVLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(RemoteSessionMonitor,"session[%s] update , but it's not registered"),
			sessionId.c_str() );
		return ;
	}
	(*(it->second)).signal();
}

}}

