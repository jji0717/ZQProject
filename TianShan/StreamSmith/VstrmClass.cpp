
#include "VstrmClass.h"
#include <Log.h>
#include <time.h>
#include "HelperClass.h"

#include <TianShanDefines.h>

#include <streamsmithconfig.h>


#ifdef _DEBUG
	#include "adebugmem.h"
#endif


namespace ZQ{
namespace StreamSmith {

static int VstrmGetSessionOnPort(HANDLE vstrmClassHandle, const ULONG port_number);

const char* nodeStateStr( UCHAR state)
{
	switch(state)
	{
	case INIT_NODE:
		return "INIT_NODE";
	case POLLING:
		return "POLLING";
	case RED:
		return "RED";
	case ORANGE:
		return "ORANGE";
	case YELLOW:
		return "YELLOW";
	case GREEN:
		return "GREEN";
	case CFG_NODE:
		return "CFG_NODE";
	case NODE_OFF:
		return "NODE_OFF";
	default:
		return "UNKNOWN";
	};
}
bool VstrmClass::isNodeReady( )
{
	if( VstrmClassOpenEx(&_hVstrmClass) != VSTRM_SUCCESS )
	{
		_hVstrmClass = NULL;
		return false;
	}
	CLUSTER_INFO clusterInfo;
	ULONG ret;
	if( VstrmClassGetClusterDataEx(_hVstrmClass ,&clusterInfo,sizeof(clusterInfo) , &ret) == VSTRM_SUCCESS )
	{
		VstrmClassCloseEx( _hVstrmClass );
		_hVstrmClass = NULL;

		//printf("node state = %d \n" , clusterInfo.localNodeState );
		if( clusterInfo.localNodeState < ORANGE )
		{
			glog(ZQ::common::Log::L_WARNING,CLOGFMT(VstrmClass,"node state[%s]"), nodeStateStr(clusterInfo.localNodeState) );
			return false;
		}
		else
		{
			return true;
		}
	}
	else	
	{
		VstrmClassCloseEx( _hVstrmClass );
		_hVstrmClass = NULL;
		return true;
	}
}

VstrmClass::VstrmClass(ZQ::common::NativeThreadPool& thpool)
:_threadPool(thpool),
_edgeEventListener(*this),
_bwUsageScanner(*this),
_SpigotStatusScanner(*this),
_bwEventScaner(_bwUsageScanner)
{
	_pSpigotCallbackUserData = NULL;
	_bSpigotCallBackIsCalled = false;

}

bool VstrmClass::initVstrmClass( )
{
	if( _hVstrmClass != NULL )
	{
		VstrmClassCloseEx(_hVstrmClass );
		_hVstrmClass = NULL;
	}
	if (VstrmClassOpenEx (&_hVstrmClass) != VSTRM_SUCCESS)
	{
		_hVstrmClass = NULL;
		return false;
	}

	// get the mediaClusterId
	CLUSTER_INFO clusterInfo;
	ULONG ret;
	VstrmClassGetClusterDataEx(_hVstrmClass ,&clusterInfo,sizeof(clusterInfo) , &ret);
	GAPPLICATIONCONFIGURATION.mediaClusterId = (int32)clusterInfo.clusterId;

	glog(ZQ::common::Log::L_INFO , CLOGFMT(VstrmClass,"Get cluster Id [%u]"),GAPPLICATIONCONFIGURATION.mediaClusterId);

	//refreshPortInfo();

	createNodeStatusListener();
	_edgeEventListener.start( );
	_bwUsageScanner.start( );
	_SpigotStatusScanner.start( );
	_bwEventScaner.start();
	return true;
}

VstrmClass::~VstrmClass()
{
	//do not stop edge event listener
	close();
	::Sleep(1);
}

void VstrmClass::close()
{
	if (isValid())
	{	
		_bwUsageScanner.stop();
		_bwEventScaner.stop();
		_SpigotStatusScanner.stop();
		VstrmClassCloseEx(_hVstrmClass);
		_hVstrmClass = NULL;
	}	
}

bool VstrmClass::isValid()
{
	bool bValid = ( _bSpigotCallBackIsCalled == true  && 
					 _hVstrmClass != NULL  && 
					 m_vstrmPorts.size() > 0 &&
					m_spigotMap.size() > 0  );
	return bValid;
}

const VHANDLE VstrmClass::handle() const
{
	return _hVstrmClass;
}

const char* VstrmClass::getErrorText(const VSTATUS status, char* textbuf, const int maxlen)
{
	if (textbuf == NULL)
		return NULL;
	
	if (VSTRM_SUCCESS == VstrmClassGetErrorText(_hVstrmClass, status, textbuf, maxlen))
		return textbuf;
	else return NULL;
}

int VstrmClass::refreshPortInfo()
{
	if ( _hVstrmClass== NULL )
	{
		glog(ZQ::common::Log::L_CRIT , CLOGFMT(VstrmClass,"Vstrm Class Initialize failed, please check vstrm settings"));
		return -1;
	}

	ZQ::common::MutexGuard	guard(m_portMapMutex);
	glog(ZQ::common::Log::L_INFO , CLOGFMT(VstrmClass,"Begin refresh vstrm port information, clear current vstrm port map"));	
	m_vstrmPorts.clear( );

	try
	{
		VSTATUS status;
		status=VstrmClassForEachPort(_hVstrmClass, NULL, &cbPortInfo, this);
		if (VSTRM_SUCCESS != status)
		{
			//		VstrmClassGetErrorText(pxP->VstrmClassHandle, status, szErrorBuf, sizeof(szErrorBuf));
			//		glog(ZQ::common::Log::L_DEBUG, LOGFMT("VstrmClassForEachPort failed with Vstrm Error = 0x%08x %s"), status, szErrorBuf);
			char szBuf[1024];
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(VstrmClass,"VstrmClassForEachPort() failed with error description is %s"),getErrorText(status,szBuf,sizeof(szBuf)-1));
			return -1;
		}
		HANDLE hSpigotIDHAndle = SpigotGetDllClassHandle();
		status = SpigotClassForEachSpigot(hSpigotIDHAndle,this,NULL,&SpigotCallbackForEachPort);
		
		if (VSTRM_SUCCESS != status)
		{			
			char szBuf[1024];
			glog(ZQ::common::Log::L_ERROR, 
				CLOGFMT(VstrmClass,"VstrmClassForEachPort() failed with error description is %s"),
				getErrorText(status,szBuf,sizeof(szBuf)-1));
			return -1;
		}
		
	}
	catch( ... )
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(VstrmClass , "unkown exception when refresh vstrm port  , clear vstrm port map"));
		m_vstrmPorts.clear();
		return -1;
	}
	int c = getPortCount();
	glog((c<=0 ? ZQ::common::Log::L_ERROR:ZQ::common::Log::L_INFO), CLOGFMT(VstrmClass,"refreshPortInfo() ends, port count:[%d]"), c);

	if (GAPPLICATIONCONFIGURATION.lSessScanCoef != -1) 
	{
		glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmClass,"refreshPortInfo() scan coef has been changed to [%d] ,do not calculate it"),GAPPLICATIONCONFIGURATION.lSessScanCoef);
	}
	else
	{
		//calculate the scan coef using vstrm port number
		GAPPLICATIONCONFIGURATION.lSessScanCoef = c  / 500 +1;
		glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmClass,"refreshPortInfo() calculate the scan coef =[%d]"),GAPPLICATIONCONFIGURATION.lSessScanCoef);
	}
	bool bOK = true;
	{
		ZQ::common::MutexGuard gd(m_portMapMutex);
		VSTRMPORTS::const_iterator it = m_vstrmPorts.begin();
		for( ; it != m_vstrmPorts.end() ; it ++ )
		{
			if( it->availOnSpigot != 1 )
			{
				glog(ZQ::common::Log::L_ERROR , 
					CLOGFMT(VstrmClass,"refreshPortInfo() port [%d] do not inclulded in an valid board"),
					it->portId );
				bOK = false;
			}
		}
	}
	if( !bOK)
		return -1;
	else
		return c;
}

void VstrmClass::updateSpigotStatus( ULONG spigotId , int status )
{
	ZQ::common::MutexGuard gd(m_portMapMutex);
	bool bFound = false;
	//find the spigot
	VSTRMSPIGOTS::iterator it = m_spigotMap.begin();
	for( ; it != m_spigotMap.end() ; it ++ )
	{
		if ( it->second.spigotId == spigotId )
		{
			it->second.status		= status;
			it->second.lastUpdate	= ZQTianShan::now();
			glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmClass,"update spigot [%02d][%d] status[%s]"),
				it->first ,
				spigotId,
				( status == STATUS_READY ) ? "up":"down" );
			bFound = true;

			//and we should report the issue to out side listener 
			if( _spigotStatusCallback )
			{
				try
				{
					_spigotStatusCallback( it->second ,it->first , _pSpigotCallbackUserData );
				}
				catch( ... )
				{
				}
			}
		}
	}
	if ( !bFound )
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(VstrmClass,"spigot id[%d] is not found in spigots list"), spigotId);
}

void VstrmClass::SpigotCallbackForEachPort(void* pCallback,
										  PSPG_SPIGOT_CHARACTERISTICS	spigotChars,
										  ULONG						spigotCharsLength,
										  ULONG						spigotIndex,
										  ULONG						spigotCount)
{	
	glog(ZQ::common::Log::L_INFO,
		"Enter SpigotCallbackForEachPort with spigotIndex[%u] spigotCount[%u]  boardNumber[%d] firstPortIndex[%u] portCount[%u]",
		spigotIndex , spigotCount , spigotChars->boardNum , spigotChars->firstPortIndx , spigotChars->portCount	);

	VstrmClass* pThis = reinterpret_cast<VstrmClass*>( pCallback );
	assert( pThis );
	pThis->_bSpigotCallBackIsCalled = true;

	static  int staticSpigotNumber	= 0;

	static int lastBoardNum = -1;
	static int staticSpigotIndex = 0;
	if( lastBoardNum != spigotChars->boardNum )
	{
		staticSpigotIndex	= 0;
	}
	else
	{
		staticSpigotIndex	++;
	}

	ULONG SpigotEnd					= spigotChars->firstPortIndx + spigotChars->portCount ;

	//get spgiot IP & base port here
	DWORD	basePort				= 0;
	std::string sourceIp			= "";

	{
		char szPath[1024];
		char szKey[256];

		static const char* pKeyPath = "SYSTEM\\CurrentControlSet\\Services\\EdgeDrv\\Ip";
		
		sprintf(szPath,"%s%d\\Adapter%d",pKeyPath, spigotChars->boardNum,staticSpigotIndex );
		
		
		sprintf(szKey,"baseUdpPort");

		if( !HelperClass::GetRegistryValue( szPath , szKey , basePort))
		{
			glog(ZQ::common::Log::L_ERROR , "SpigotCallbackForEachPort() can't get [%s][%s]'s value",szPath , szKey );
		}
		else
		{
			glog(ZQ::common::Log::L_INFO , "SpigotCallbackForEachPort() get [%s][%s] value [%u]",szPath , szKey, basePort );
		}

		sprintf(szKey,"SrcIpAddr");
		if( !HelperClass::GetRegistryValue( szPath , szKey , sourceIp))
		{
			DWORD dwSourceIp = 0;
			if(!HelperClass::GetRegistryValue( szPath , szKey , dwSourceIp))
			{
				glog(ZQ::common::Log::L_ERROR , "SpigotCallbackForEachPort() can't get [%s][%s]'s value",szPath , szKey );
			}
			else
			{
				char szSourceIpBuf[256];
				unsigned char a , b , c, d;
				d =(unsigned char)( dwSourceIp >> 24 );
				c =(unsigned char)( ( dwSourceIp & 0x00ff0000) >> 16 );
				b =(unsigned char)( (dwSourceIp & 0x0000ff00) >> 8 ) ;
				a =(unsigned char)( dwSourceIp & 0x000000ff);
				sprintf(szSourceIpBuf,"%d.%d.%d.%d",a,b,c,d) ;
				sourceIp = szSourceIpBuf;
				glog(ZQ::common::Log::L_INFO , "SpigotCallbackForEachPort() get [%s][%s] value [%s]",szPath , szKey, sourceIp.c_str() );
			}
		}
		else
		{
			glog(ZQ::common::Log::L_INFO , "SpigotCallbackForEachPort() get [%s][%s] value [%s]",szPath , szKey, sourceIp.c_str() );
		}
	}
	
	VstrmSpigotChar vsc;
	vsc.portHighPart	=	spigotChars->firstPortIndx + spigotChars->portCount - 1;
	vsc.portLowPart		=	spigotChars->firstPortIndx;
	vsc.sourceIp		=	sourceIp;
	vsc.sourceBasePort	=	basePort;
	vsc.spigotId		=	spigotIndex ; //spigotChars->spigotId;
	vsc.spigotHandle	=	spigotChars->spigotId;
	vsc.status			=	(pThis->getSpigotLinkState(vsc.portLowPart) >0 ? STATUS_READY :STATUS_OUTOFSERVICE);
	vsc.firstUp			=	ZQTianShan::now();
	vsc.lastUpdate		=	vsc.firstUp;

	pThis->m_spigotMap.insert( VSTRMSPIGOTS::value_type( staticSpigotNumber , vsc ) );

	VSTRMPORTS::iterator itPort		= pThis->m_vstrmPorts.begin();
	VSTRMPORTS::iterator itPortEnd	= pThis->m_vstrmPorts.end();
	if(  vsc.portHighPart + 1 > (int)pThis->m_vstrmPorts.size() )
	{
		glog(ZQ::common::Log::L_ERROR,"Invalid port char , only [%d] ports from ports scan , but spigot scan show [%d] ports",
				(int)pThis->m_vstrmPorts.size() , vsc.portHighPart);
	}
	else
	{
		itPort	= itPort + vsc.portLowPart;
		itPortEnd	= itPort + spigotChars->portCount;
		for( ; itPort != itPortEnd ; itPort ++ )
		{
			itPort->availOnSpigot = 1;
		}
	}

	glog(ZQ::common::Log::L_INFO,"Spigot enum:SpigotId(%02d) physicalSpigotIdentity[%d] PortCount(%d) firstPortIndex(%d) spigot(%d) link status[%s]",
							spigotIndex,
							spigotChars->spigotId,
							spigotChars->portCount,
							spigotChars->firstPortIndx,
							spigotChars->boardNum,
							vsc.status == STATUS_READY ? "Up":"Down");
	
	staticSpigotNumber++;
}

int VstrmClass::getPortCount()
{
	return static_cast<int>( m_vstrmPorts.size() );
}

VSTATUS VstrmClass::cbPortInfo(	HANDLE					VstrmClassHandle,
								 PVOID				    pCtx,
								 PEPORT_CHARACTERISTICS	portChars,
								 ULONG					portCharSize,
								 ULONG					currentPort,
								 ULONG					portCount)
{	
	VstrmClass* pThis = reinterpret_cast<VstrmClass*>( pCtx );
	assert( pThis != NULL );

	PORT_CHARACTERISTICS& pc = portChars->PortCharacteristics;
	if ( ( pc.DeviceType == NULL_DEVICE_C) ||
		 (	IS_VIDEO_PORT( pc.DeviceType ) &&
			!( IS_INPUT_PORT(pc)) 
#ifdef IS_FSI_PORT
			&& !( IS_FSI_PORT(pc.DeviceType) )	
#endif
			))		
	{
		
		VstrmPortChar		vpc;

		vpc.portId			=	currentPort;
		vpc.portInUse		=	PORT_NORMAL;
		vpc.status			=	0;
		vpc.availOnSpigot	=	0;

		pThis->m_vstrmPorts.push_back( vpc );

		glog(ZQ::common::Log::L_DEBUG, LOGFMT("found VstrmPort [%d] of [%d]"), currentPort, portCount);

	}

	glog(ZQ::common::Log::L_DEBUG,"PortType:(%d) PortID:(%d) SubHandle:(%d) DeviceUnit:(%d) DeviceClass:(%d)"
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

	return (VSTRM_SUCCESS);}

bool	VstrmClass::GetPortProperty( ULONG port ,PORTCHARRACTER& prop )
{
	ZQ::common::MutexGuard	guard(m_portMapMutex);
	glog(ZQ::common::Log::L_INFO,"Enter GetPortProperty with port[%u]" , port );

	VSTRMSPIGOTS::const_iterator itSpigot = m_spigotMap.begin( );
	for( ; itSpigot != m_spigotMap.end() ; itSpigot++ )
	{
		if( (itSpigot->second.portLowPart<= (int)port) && ((int)port <= itSpigot->second.portHighPart) )
		{			
			prop.sourceIp	=	itSpigot->second.sourceIp;
			prop.sourcePort	=	itSpigot->second.sourceBasePort + port - itSpigot->second.portLowPart;
			return true;
		}
	}
	glog(ZQ::common::Log::L_INFO,"GetPortProperty , do not find port[%u]" , port );
	return false;
}

void VstrmClass::getSpigotInfo( VSTRMSPIGOTS& spigots )
{
	ZQ::common::MutexGuard gd(m_portMapMutex);
	spigots = m_spigotMap;
}
int VstrmClass::getSpigotLinkState( ULONG portId )
{
	VSTATUS					vStatus				= VSTRM_NOT_SUPPORTED; 
	ATTRIBUTE_ARRAY			getBuf				= {0}; 
	UCHAR					getLinkState		= 0; 
	ULONG					localPortCount		= 0; 

	getBuf.attributeArray[0].attributeCode = VSTRM_ATTR_GEN_LINK_STATE; 
	getBuf.attributeArray[0].attributeValueLength = sizeof(getLinkState); 
	getBuf.attributeArray[0].attributeValueP = (PVOID) &getLinkState; 
	getBuf.attributeCount = (USHORT) 1; 

	vStatus = VstrmClassGetPortAttributesEx( _hVstrmClass, 
											(portId),        // use a portId specific to board, they are 
											&getBuf );        // allocated in increments specific to card type 

	if ( VSTRM_SUCCESS != vStatus ) 
	{ 
		char szError[1024];
		szError[sizeof(szError)-1] = 0;
		glog(ZQ::common::Log::L_ERROR,
			CLOGFMT(VstrmClass,"failed to get the spigot linkage state for port[%d] and error [%s]"),
			portId,
			getErrorText( VstrmGetLastError(),szError , sizeof(szError)-1 ) );
		return -1;
	} 

	if (getLinkState == 0) 
	{ 
		return 0;
	} 
	else if (getLinkState == 1) 
	{ 
		return 1;
	} 
	glog(ZQ::common::Log::L_ERROR,CLOGFMT(VstrmClass,"Can't get spigot linkage status for port[%d]"),
			portId );
	return -1;
}
int	VstrmClass::getPortFromVstrmPortsPool( int firstPort , int lastPort )
{
	int32 cooldownTime = gStreamSmithConfig.lPortCooldownTime; //need configuration
	cooldownTime = max( cooldownTime , 100 );

	ZQ::common::MutexGuard	guard(m_portMapMutex);

	int iPortCount = getPortCount( );
	if( firstPort <  0 || firstPort > iPortCount ||
		lastPort < 0 || lastPort > iPortCount	)
	{
		glog(ZQ::common::Log::L_ERROR,
			"getPortFromVstrmPortsPool() Internal Error portCount[%d] but passed in firstPort[%d] LastPot[%d] ",
			iPortCount , firstPort , lastPort	);
		return -1;
	}
	if ( firstPort > lastPort )
	{
		int temPort	= firstPort;
		firstPort	= lastPort;
		lastPort	= temPort;
	}

	VSTRMPORTS::iterator itPortFirst	= m_vstrmPorts.begin() + firstPort;	
	VSTRMPORTS::iterator itPortEnd		= m_vstrmPorts.begin() + lastPort + 1; //pass the last port index

	int seed  = 0;
	int maxPortOnSpigot = lastPort - firstPort;
	if( maxPortOnSpigot > 2 )
	{
		seed = rand() % ( maxPortOnSpigot - 1 ) ;
	}
	
	/*
	walk whole spigot port vector 
	if the port is free and it's cool down time > configured cool down time . use it
	if can not select a port because no one's cool down time > config cool down time, select the max one
	*/

	VSTRMPORTS::iterator itPort = itPortFirst + seed;
	VSTRMPORTS::iterator itPortStopMark = itPort;
	
	VSTRMPORTS::iterator itCooldownMax = itPortEnd;

	int64 curTime = ZQTianShan::now();
	int64 lastMaxDiffTime = 0;

	for( ; itPort < itPortEnd ; itPort++ )
	{
		if ( (itPort->availOnSpigot >= 1) && (itPort->portInUse == PORT_NORMAL) )
		{
			int64 timeDiff = curTime - itPort->lastReleasedTime;

			if (timeDiff > cooldownTime)
			{
				if (FALSE != VstrmGetSessionOnPort(_hVstrmClass, itPort->portId))
					continue; // has session on the VstrmPort, eliminate it from the selection
				glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmClass,"getPortFromVstrmPortsPool() vstrm port[%d] is free"), itPort->portId );
				itPort->portInUse = PORT_IN_USE;
				return itPort->portId;
			}

			if( timeDiff > lastMaxDiffTime)
			{
				if (FALSE != VstrmGetSessionOnPort(_hVstrmClass, itPort->portId))
					continue; // has session on the VstrmPort, eliminate it from the selection

				itCooldownMax = itPort;
				lastMaxDiffTime = timeDiff;
			}
		}
	}
	
	itPort = itPortFirst;
	for( ; itPort < itPortStopMark ; itPort ++ )
	{
		if( (itPort->availOnSpigot >= 1) && (itPort->portInUse == PORT_NORMAL) )
		{
			int64 timeDiff = curTime - itPort->lastReleasedTime;

			//TODO: HongQuan, these two loops can be merged into one, -andy:
			// for (int i=0; i< (itPortEnd- itPortFirst); i++)
			// {
			//		itPort = itPortFirst + (seed +i) % (itPortEnd- itPortFirst);
			//		...
			// }
					

			if ( timeDiff > cooldownTime )
			{
				if (FALSE != VstrmGetSessionOnPort(_hVstrmClass, itPort->portId))
					continue; // has session on the VstrmPort, eliminate it from the selection

				itPort->portInUse = PORT_IN_USE;
				glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmClass,"getPortFromVstrmPortsPool() vstrm port[%d] is free"), itPort->portId );
				return itPort->portId;
			}

			if ( timeDiff > lastMaxDiffTime)
			{
				if (FALSE != VstrmGetSessionOnPort(_hVstrmClass, itPort->portId))
					continue; // has session on the VstrmPort, eliminate it from the selection

				itCooldownMax = itPort;
				lastMaxDiffTime = timeDiff;
			}
		}
	}

	if( itCooldownMax != itPortEnd )
	{
		itCooldownMax->portInUse = PORT_IN_USE;
		glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmClass,"getPortFromVstrmPortsPool() vstrm port[%d] is free"), itCooldownMax->portId );
		return itCooldownMax->portId;
	}

	return -1;
}

ULONG	VstrmClass::getSpigotIdFromPortId( ULONG portNum ) 
{
	ULONG spigotId = (ULONG)-1;
	{
		ZQ::common::MutexGuard	guard(m_portMapMutex);
		VSTRMSPIGOTS::const_iterator itSpigot = m_spigotMap.begin();
		for( ; itSpigot != m_spigotMap.end() ; itSpigot++ )
		{
			if( ((int)portNum >= itSpigot->second.portLowPart) && ((int)portNum <= itSpigot->second.portHighPart) )
			{
				spigotId = itSpigot->second.spigotHandle;
				break;
			}
		}
	}
	return spigotId;
}

ULONG	VstrmClass::GetUnUsePort( ULONG port , int SpigotID )
{
	ZQ::common::MutexGuard	guard(m_portMapMutex);
	glog(ZQ::common::Log::L_INFO , 
		CLOGFMT(VstrmClass,"Enter GetUnUsePort() with port[%d] spigotId[%d] and current vstrmPortCount[%d]") ,
		port , SpigotID , getPortCount( ) );

	int	portId = (ULONG)-1;

	if( port != ((ULONG)-1) )
	{
		if( (int)port < getPortCount() )
		{
			for (VSTRMSPIGOTS::const_iterator itSpigot = m_spigotMap.begin( ); itSpigot != m_spigotMap.end() ; itSpigot++ )
			{
				if( (int)port < itSpigot->second.portLowPart || (int)port > itSpigot->second.portHighPart )
					continue;

				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VstrmClass,"find port[%u] belong to spigot[%d]"), port, itSpigot->first);

				if( itSpigot->second.status != STATUS_READY )
				{
					glog(ZQ::common::Log::L_WARNING, CLOGFMT(VstrmClass, "spigot[%d] is not available now"), port, itSpigot->first);
					return (ULONG)-1;
				}

				if( m_vstrmPorts[port].portInUse == PORT_IN_USE )
				{
					glog(ZQ::common::Log::L_ERROR , CLOGFMT(VstrmClass,"port [%u] is in use"), port);
					return (ULONG)-1;
				}

				glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmClass,"getPortFromVstrmPortsPool() vstrm port[%d] is free"), port );
				m_vstrmPorts[port].portInUse = PORT_IN_USE;
				return port;
			}
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR,"GetUnUsePort() Invalid specified port[%d]", port );
		}
	}
	else if( SpigotID < 0 )
	{//system will allocate a spigot for you
		VSTRMSPIGOTS::const_iterator itSpigot = m_spigotMap.begin();
		for( ; itSpigot != m_spigotMap.end() ; itSpigot ++ )
		{
			if( itSpigot->second.status == STATUS_READY )
			{
				if( ( portId = getPortFromVstrmPortsPool( itSpigot->second.portLowPart , itSpigot->second.portHighPart ) ) >= 0 )
				{					
					return portId;
				}
			}
		}		
		return (ULONG)-1;
	}
	else
	{
		//get the target spigot id first
		VSTRMSPIGOTS::const_iterator itSpigot = m_spigotMap.find( SpigotID );
		if( itSpigot != m_spigotMap.end() && itSpigot->second.status == STATUS_READY )
		{
			if( ( portId = getPortFromVstrmPortsPool( itSpigot->second.portLowPart , itSpigot->second.portHighPart ) ) >= 0 )
			{
				return (ULONG)portId;
			}
			else
			{
				return (ULONG)-1;
			}
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR,"GetUnUsePort() can't find spigotId[%d] , maybe it is not available now" , SpigotID );
			return (ULONG)-1;
		}
	}
	return (ULONG)-1;

	/*
	portmap_t::iterator	it = _portmap.begin( );
	

	if( port != (ULONG)-1 )
	{//get the specified vstrm port
		
		for( it = _portmap.begin() ; it != _portmap.end() ; it++ )
		{//it->first.Handle==port
			if( it->spigot == -1 )
				continue;	//no usable spigot
			if( it->Port==port )
			{
				it->IsInUse=PORT_IN_USE;
				glog(ZQ::common::Log::L_INFO , 
					CLOGFMT(VstrmClass,"Get Un-Use Port [%d] ") , it->Port );
				return it->Port;
			}
		}
		return -1;
	}
	else
	{
		if( GAPPLICATIONCONFIGURATION.lUseRandomVstrmPortAssignStrategy >= 1 )
		{
			static bool bInit=false;
			if(!bInit)
			{
				srand(time(NULL));
				bInit=true;
			}
			int iOffset = 0;
			if( (int)_portmap.size() > 2) 
			{
				iOffset= rand()%( _portmap.size() - 1 );
			}

			portmap_t::iterator itSeperator = _portmap.begin();
			portmap_t::iterator itBegin = _portmap.begin();
			portmap_t::iterator itEnd = _portmap.end();

			if ( GAPPLICATIONCONFIGURATION.lEnablePartialSelectVstrmPort > 0 ) 
			{
				long lSelectHigh =GAPPLICATIONCONFIGURATION.lPartialSelectHigh;
				lSelectHigh = max(0,lSelectHigh);
				lSelectHigh = min(lSelectHigh,_portmap.size());
				
				long lSelectLow = GAPPLICATIONCONFIGURATION.lPartialSelectLow;
				lSelectLow = max(0,lSelectLow);
				lSelectLow = min(lSelectLow,_portmap.size());

				if (lSelectLow > lSelectHigh) 
				{
					lSelectHigh = lSelectLow;
				}

				itEnd =	lSelectHigh+itBegin;
				itBegin += lSelectLow;
				itSeperator = itBegin;
				if ( (itEnd-itBegin) >= 2 ) 
				{
					iOffset %= (itEnd-itBegin);
				}
				else
				{
					iOffset = 0;
				}
			}

			itSeperator += iOffset;
			for( it = itSeperator ; it < itEnd; it++ )
			{
				if( it->spigot==-1 )
					continue;	//no useable spigot
				if( SpigotID == -1 )
				{
					if( it->IsInUse==PORT_NORMAL )
					{
						it->IsInUse = PORT_IN_USE;	
						//it->second.IsFreedByPlayList=false;
						glog(ZQ::common::Log::L_INFO , 
							CLOGFMT(VstrmClass,"Get Un-Use Port [%d] ") , it->Port );
						return it->Port;
					}
				}
				else
				{
					if( it->spigot==SpigotID && it->IsInUse==PORT_NORMAL )
					{
						it->IsInUse = PORT_IN_USE;
						glog(ZQ::common::Log::L_INFO , 
							CLOGFMT(VstrmClass,"Get Un-Use Port [%d] ") , it->Port );
						return it->Port;
					}
				}
			}
			for( it=itBegin ; it < itSeperator ; it++ )
			{
				if( it->spigot == -1 )
					continue;	//no useable spigot
				if( SpigotID == -1 )
				{
					if( it->IsInUse == PORT_NORMAL )
					{
						it->IsInUse = PORT_IN_USE;	
						//it->second.IsFreedByPlayList=false;
						glog(ZQ::common::Log::L_INFO , 
							CLOGFMT(VstrmClass,"Get Un-Use Port [%d] ") , it->Port );
						return it->Port;
					}
				}
				else
				{
					if(it->spigot==SpigotID 
						&& it->IsInUse==PORT_NORMAL)
					{
						it->IsInUse=PORT_IN_USE;
						glog(ZQ::common::Log::L_INFO , 
							CLOGFMT(VstrmClass,"Get Un-Use Port [%d] ") , it->Port );
						return it->Port;
					}
				}
			}
		}
		else
		{
			portmap_t::iterator itBegin = _portmap.begin();
			portmap_t::iterator itEnd = _portmap.end();

			if ( GAPPLICATIONCONFIGURATION.lEnablePartialSelectVstrmPort > 0 ) 
			{
				long lSelectHigh =GAPPLICATIONCONFIGURATION.lPartialSelectHigh;
				lSelectHigh =max(0,lSelectHigh);
				lSelectHigh =min(lSelectHigh,_portmap.size());
				
				long lSelectLow = GAPPLICATIONCONFIGURATION.lPartialSelectLow;
				lSelectLow = max(0,lSelectLow);
				lSelectLow = min(lSelectLow,_portmap.size());

				if (lSelectLow > lSelectHigh) 
				{
					lSelectHigh = lSelectLow;
				}

				itEnd =	lSelectHigh+itBegin;
				itBegin += lSelectLow;				
			}

			for(it=itBegin;it!=itEnd;it++)
			{
				if( it->spigot == -1 )
					continue;	//no useable spigot
				if( SpigotID == -1 )
				{
					if( it->IsInUse == PORT_NORMAL )
					{
						it->IsInUse = PORT_IN_USE;	
						//it->second.IsFreedByPlayList=false;
						glog(ZQ::common::Log::L_INFO , 
							CLOGFMT(VstrmClass,"Get Un-Use Port [%d] ") , it->Port );
						return it->Port;
					}
				}
				else
				{
					if(it->spigot==SpigotID 
						&& it->IsInUse==PORT_NORMAL)
					{
						it->IsInUse=PORT_IN_USE;
						glog(ZQ::common::Log::L_INFO , 
							CLOGFMT(VstrmClass,"Get Un-Use Port [%d] ") , it->Port );
						return it->Port;
					}
				}
			}
		}
	}
	return -1;
	*/

}
void VstrmClass::listStreamer(std::vector<int>& IDs)
{
	IDs.clear();
	
	ZQ::common::MutexGuard	guard(m_portMapMutex);
	
	VSTRMSPIGOTS::const_iterator itSpigot = m_spigotMap.begin();
	
	for( ; itSpigot != m_spigotMap.end() ; itSpigot ++ )
	{
		IDs.push_back( itSpigot->first );
	}	

// 	portmap_t::iterator it;
// 	std::vector<int>::iterator idIt;
// 	bool	bHasAlready=false;
// 	for(it=_portmap.begin();it!=_portmap.end();it++)
// 	{
// 		for(idIt=IDs.begin();idIt!=IDs.end();idIt++)
// 		{
// 			if(it->spigot==*idIt)
// 			{
// 				bHasAlready=true;
// 				break;
// 			}
// 		}
// 		if(!bHasAlready)
// 		{
// 			IDs.push_back(it->spigot);			
// 		}
// 		bHasAlready=false;
// 	}
}

void	VstrmClass::FreePortUsage(ULONG port)
{
	ZQ::common::MutexGuard	guard(m_portMapMutex);
	if( port < (ULONG)getPortCount() )
	{
		glog(ZQ::common::Log::L_INFO , 
			CLOGFMT(VstrmClass,"Free In-Use Port [%d] ") , port );

		m_vstrmPorts[port].portInUse			= PORT_NORMAL;
		m_vstrmPorts[port].lastReleasedTime		= ZQTianShan::now();

	}
	else
	{
		glog(ZQ::common::Log::L_ERROR  , 
			CLOGFMT(VstrmClass,"FreePortUsage() invalid port [%d] with total port count[%d]"),
			port , getPortCount() );
	}
}
HANDLE	VstrmClass::registerPokeSession( const std::string& strPokeSessionId )
{
	ZQ::common::MutexGuard gd(pokeInfoMapLoker);
	
	NATPOKEINFOMAP::iterator it = pokeInfoMap.find( strPokeSessionId );
	if( it != pokeInfoMap.end() )
	{
		glog(ZQ::common::Log::L_ERROR,
			"VstrmClass() pokeHoleSession[%s] handle[%x] already available" ,
			HelperClass::dumpBinary(strPokeSessionId).c_str() ,
			it->second.hNotifier);

		return it->second.hNotifier;
	}
	else
	{
		HANDLE hNotifier = CreateEvent(NULL,FALSE,FALSE,NULL);
		NatPokeInfo info;
		info.hNotifier = hNotifier;
		pokeInfoMap[strPokeSessionId]=info;
		glog(ZQ::common::Log::L_INFO,
			"VstrmClass() register pokeHoleSession[%s] handle[%x] OK " , 
			HelperClass::dumpBinary(strPokeSessionId).c_str() , hNotifier);
		return hNotifier;
	}
}

void	VstrmClass::unregisterPokeSession( const std::string& strPokeSessionId )
{
	ZQ::common::MutexGuard gd(pokeInfoMapLoker);
	NATPOKEINFOMAP::iterator it = pokeInfoMap.find( strPokeSessionId );
	if( it != pokeInfoMap.end() )
	{
		CloseHandle( it->second.hNotifier );
		pokeInfoMap.erase(it);
	}
}

void VstrmClass::remoteSessionAvailableNotify( const char* pPokeSessionId )
{
	std::string strPokeSessionId;
	strPokeSessionId.assign(pPokeSessionId,10);
	HANDLE hEvent = NULL;
	{
		ZQ::common::MutexGuard gd(pokeInfoMapLoker);
		NATPOKEINFOMAP::iterator it = pokeInfoMap.find( strPokeSessionId );
		if( it != pokeInfoMap.end() )
		{
			hEvent = it->second.hNotifier;			
		}
	}
	glog(ZQ::common::Log::L_INFO,"VstrmClass() remoteSession [%s] available", HelperClass::dumpBinary(strPokeSessionId).c_str()  );
	if( hEvent != NULL )
	{
		glog(ZQ::common::Log::L_INFO,"VstrmClass() remoteSession [%s] available ,do notify", 
			HelperClass::dumpBinary(strPokeSessionId).c_str() );
		SetEvent(hEvent);
	}
}

void VstrmClass::registerSpigotStatusCallback( SpigotStatusCallback callback , void* pUserData  )
{
	_spigotStatusCallback		= callback;
	_pSpigotCallbackUserData	= pUserData;
}
void VstrmClass::registerBandwidthUsageCallback( bandwidthUsageCallBack callback ,  void* pUserData )
{
	_bandwidthUsageCallback		=	callback;
	_pBandwidthUsageUserData	=	pUserData;
}
void VstrmClass::registerNodeStatusCallback( NodeStatusCallback callback , void* pUserData )
{
	_nodeStatusCallback = callback;
	_pNodeStatusUserData = pUserData;
}


std::string replaceString( std::string& str , const std::string& pattern , int newContent )
{
	std::string::size_type pos = str.find(pattern);
	if(  pos == std::string::npos )
	{
		return str;
	}
	char szBuf[256];
	sprintf(szBuf,"%d",newContent);
	return str.replace( pos , pattern.length() , szBuf );
}
std::string formatNodeId( int clusterId , int nodeIndex)
{
	static std::string format = gStreamSmithConfig.spigotReplicaConfig.nodeidformat;
	std::string nodename =format;
	nodename = replaceString(nodename,"{CLUSTERID}",clusterId);
	nodename = replaceString(nodename,"{NODEINDEX}",nodeIndex);	
	return nodename;
}
void VstrmClass::updateNodeStatus( int clusterId , int nodeId , bool bUp )
{	
	//char nodename[256];
	//sprintf(nodename,"SEAC%d_N%d",clusterId,nodeId);
	if(_nodeStatusCallback)
	{
		try
		{
			_nodeStatusCallback( formatNodeId(clusterId,nodeId) , bUp , _pNodeStatusUserData );
		}
		catch(...){}
	}
}

BandwidthUsage VstrmClass::getBwUsage()
{
	ZQ::common::MutexGuard gd(m_portMapMutex);
	return _bwUsage;
}

static void VstrmEventCb(HANDLE hHandle, PVOID parm, ULONG EventType, ULONG EventParm)
{
	VstrmClass* pThis = (VstrmClass*)parm;
	switch(EventType)
	{
	case VSTRM_EVENT_TYPE_CLUSTER_NOTIFICATION:
		{
			glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmEventCb,"comes a VSTRM_EVENT_TYPE_CLUSTER_NOTIFICATION"));
			CLUSTER_INFO clusterInfo;
			ULONG ret;
			VSTATUS status = VstrmClassGetClusterDataEx( hHandle, &clusterInfo , sizeof(clusterInfo) , &ret);
			if( VSTRM_SUCCESS == status )
			{
// 				for( int i = 0 ; i < clusterInfo.clusterSize ; i ++ )
// 				{
// 					bool bUp = true;
// 					switch (clusterInfo.raidState[i])
// 					{
// 					case RAID_DEAD_DRIVE:
// 						bUp = false;
// 						glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmEventCb,"cluster[%d] node[%d] raidState[RAID_DEAD_DRIVE]"),clusterInfo.clusterId, i);
// 						break;
// 					case RAID_REBUILDING:
// 						bUp = false;
// 						glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmEventCb,"cluster[%d] node[%d] raidState[RAID_REBUILDING]"),clusterInfo.clusterId, i);
// 						break;
// 					case RAID_CHECKING_CONSISTENCY:
// 						bUp = false;
// 						glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmEventCb,"cluster[%d] node[%d] raidState[RAID_CHECKING_CONSISTENCY]"),clusterInfo.clusterId, i);
// 						break;
// 					default:
// 						break;
// 					}					
// 					pThis->updateNodeStatus(clusterInfo.clusterId, i , bUp );
// 				}
				for( int i = 0 ; i < clusterInfo.clusterSize ; i ++ )
				{					
					bool bUp = true;
					switch (clusterInfo.node[i].NodeState)
					{
					case NODE_OFF:
						bUp = false;
						glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmEventCb,"cluster[%d] node[%d] nodeState[NODE_OFF]"),clusterInfo.clusterId, i);
						break;
					case RED:
						bUp = false;
						glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmEventCb,"cluster[%d] node[%d] nodeState[RED]"),clusterInfo.clusterId, i);
						break;
					case POLLING:
						bUp = false;
						glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmEventCb,"cluster[%d] node[%d] nodeState[POLLING]"),clusterInfo.clusterId, i);
						break;
					case INIT_NODE:
						bUp = false;
						glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmEventCb,"cluster[%d] node[%d] nodeState[INIT_NODE]"),clusterInfo.clusterId, i);
						break;
					case CFG_NODE:
						bUp = false;
						glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmEventCb,"cluster[%d] node[%d] nodeState[CFG_NODE ]"),clusterInfo.clusterId, i);
						break;
					default:
						break;
					}					
					pThis->updateNodeStatus(clusterInfo.clusterId, i , bUp );
				}
			}
			else
			{
				glog(ZQ::common::Log::L_ERROR,CLOGFMT(VstrmEventCb,"got VSTRM_EVENT_TYPE_CLUSTER_NOTIFICATION notification but failed to query cluster information; %s"),
					getVstrmError(hHandle,status).c_str() );
			}
		}
		break;
	default:
		break;
	}
}
void VstrmClass::createNodeStatusListener()
{
	VHANDLE eventHandler = INVALID_HANDLE_VALUE;
	VSTATUS vret = VstrmClassRegisterEventCb( _hVstrmClass , VSTRM_EVENT_TYPE_CLUSTER_NOTIFICATION , VstrmEventCb, this, &eventHandler );
	if( vret != VSTRM_SUCCESS )
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(VstrmClass,"createNodeStatusListener() failed to create node status event callback listener:%s"),
			getVstrmError(_hVstrmClass, vret).c_str());
	}
}


void VstrmClass::updateCdnBwUsage( const BandwidthUsage& tmpUsage )
{
	{
		ZQ::common::MutexGuard gd(m_portMapMutex);
		if( memcmp(&tmpUsage,&_bwUsage,sizeof(tmpUsage)) !=0 )
			glog(ZQ::common::Log::L_INFO,CLOGFMT(updateCdnBwUsage,"CDN Import Channel usage:usedBW[%lld] totalBW[%lld] runningSess[%ld]"),
										tmpUsage.cdnUsedBandiwidth,
										tmpUsage.cdnTotalBandwidth,
										tmpUsage.cdnImportSessionCount );
		_bwUsage = tmpUsage;		
	}
	if( _bandwidthUsageCallback )
	{
		try
		{
			_bandwidthUsageCallback( tmpUsage , _pBandwidthUsageUserData );
		}
		catch( ... )
		{
		}
	}

}

//////////////////////////////////////////////////////////////////////////

EdgeEventListener::EdgeEventListener(VstrmClass& c)
:_cls(c)
{
	_hClass = INVALID_HANDLE_VALUE;
}
EdgeEventListener::~EdgeEventListener()
{
}
bool EdgeEventListener::init()
{
	close();
	if( VstrmClassOpenEx( &_hClass ) != VSTRM_SUCCESS )
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(EdgeEventListener,"Open vstrm class failed"));
		return false;
	}
	return true;
}
void EdgeEventListener::close()
{
	if( _hClass != INVALID_HANDLE_VALUE )
	{
		VstrmClassCloseEx(_hClass);
		_hClass = INVALID_HANDLE_VALUE;
	}
}
void EdgeEventListener::final()
{
	close( );
}
int EdgeEventListener::run()
{
	glog( ZQ::common::Log::L_INFO , "EdgeEventListener start scan edge event");

	EDGE_EVENT_DATA         tagEventData            = {0};
	ZeroMemory( &tagEventData, sizeof(tagEventData) );

	HANDLE					hChange					= INVALID_HANDLE_VALUE;
	bool					bOk = true;
	
	char					szErrorBuf[512];

	__try
	{
		hChange = VstrmFindFirstEdgeNotification(   _hClass , &tagEventData );
		if( INVALID_HANDLE_VALUE == hChange )
		{	
			VstrmClassGetErrorText(_hClass , VstrmGetLastError() , szErrorBuf, sizeof(szErrorBuf )-1 );
			glog(ZQ::common::Log::L_CRIT ,"EdgeEventListener failed to call VstrmFindFirstEdgeNotification() , "
				"exit edge event scanning , [%s]",
				  szErrorBuf );			
			return	-1;
		}
		while(bOk)
		{
			switch( tagEventData.edgeEvent )
			{
			case VSTRM_EDGE_EVENT_REMOTE_SESSION_AVAILABLE:
				{
					glog(ZQ::common::Log::L_INFO ,
						"EdgeEventListener() remote session available : port[%u] spigot[%d]",
						tagEventData.portHandle,
						tagEventData.spigotHandle);						
					_cls.remoteSessionAvailableNotify( (const char*)tagEventData.pokeSessionId );
				}
				break;
			case VSTRM_EDGE_EVENT_LOST:
				{
					glog(ZQ::common::Log::L_WARNING , "EdgeEventListener() vstrm edge event lost");					
				}
				break;
			case VSTRM_EDGE_EVENT_NO_LISTENER_FOUND:
				{
					glog(ZQ::common::Log::L_INFO,
						"EdgeEventListener() VSTRM_EDGE_EVENT_NO_LISTENER_FOUND is found with port[%u] spigot[%d]",
						tagEventData.portHandle,
						tagEventData.spigotHandle);					
				}
				break;
			case VSTRM_EDGE_EVENT_LINK_UP:
				{
					glog(ZQ::common::Log::L_INFO,
						"EdgeEventListener() VSTRM_EDGE_EVENT_LINK_UP is found with port[%u] spigot[%d]",
						tagEventData.portHandle,
						tagEventData.spigotHandle);
					_cls.updateSpigotStatus( tagEventData.spigotHandle, STATUS_READY );
				}
				break;
			case VSTRM_EDGE_EVENT_LINK_DOWN:
				{
					glog(ZQ::common::Log::L_INFO,
						"EdgeEventListener() VSTRM_EDGE_EVENT_LINK_DOWN is found with port[%u] spigot[%d]",
						tagEventData.portHandle,
						tagEventData.spigotHandle);
					_cls.updateSpigotStatus( tagEventData.spigotHandle, STATUS_OUTOFSERVICE );
				}
				break;
			default:
				{
					glog(ZQ::common::Log::L_INFO,
						"EdgeEventListener() unokwn egde event is found with port[%u] spigot[%d]",
						tagEventData.portHandle,
						tagEventData.spigotHandle);
				}
			}
			ZeroMemory( &tagEventData, sizeof(tagEventData) );

			//
			// Wait for next Vod event notification
			//
			bOk = VstrmFindNextEdgeNotification(  _hClass, hChange, &tagEventData );
			if(!bOk)
			{
				VstrmClassGetErrorText(_hClass , VstrmGetLastError() , szErrorBuf, sizeof(szErrorBuf )-1 );
				glog(ZQ::common::Log::L_ERROR,
					"EdgeEventListener() failed to call VstrmFindNextEdgeNotification() [%s] ",
					szErrorBuf  );
				//cout<<"failed to call VstrmFindNextEdgeNotification"<<endl;
			}
			Sleep(100);
		}
	}
	__finally
	{
		VstrmFindCloseEdgeNotification( _hClass , hChange );		
	}

	return 1;
}



BandwidthEventScanner::BandwidthEventScanner( BandwidthUsageScanner& bandwidthScanner )
:mScanner(bandwidthScanner),
mbQuit(false)
{
	mHandleClass = NULL;
	VstrmClassOpenEx( &mHandleClass );
}

BandwidthEventScanner::~BandwidthEventScanner()
{
	if( mHandleClass)
		VstrmClassCloseEx( mHandleClass );
}
void BandwidthEventScanner::stop()
{
	mbQuit = true;

	VstrmSendTestBandwidthEvent( mHandleClass , VSTRM_BANDWIDTH_EVENT_CDN_STATE_CHANGE , 0 );

	waitHandle( 2 * 1000 );
}
int	BandwidthEventScanner::run( )
{
#if VER_PRODUCTVERSION_MAJOR >= 6 &&  (( VER_PRODUCTVERSION_MINOR == 1 && VER_PRODUCTBUILD >= 9310 ) || VER_PRODUCTVERSION_MINOR > 1 )
	mbQuit = false;
	BANDWIDTH_USER_EVENT bwEventData;
	
	memset( &bwEventData , 0 , sizeof(bwEventData) );

	HANDLE hNotification = VstrmFindFirstBandwidthNotification( mHandleClass , &bwEventData);	

	if( hNotification == INVALID_HANDLE_VALUE  )
	{
		char szErroMsg[256];
		VstrmClassGetErrorText( mHandleClass, VstrmGetLastError(), szErroMsg, sizeof(szErroMsg) );
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(BandwidthEventScanner,"failed to invoke VstrmFindFirstBandwidthNotification(): %s "), szErroMsg );
		return -1;
	}

	BOOLEAN bOK = false;
	do
	{
		switch ( bwEventData.bwEvent)
		{
		case VSTRM_BANDWIDTH_EVENT_CDN_STATE_CHANGE:
			{
				glog( ZQ::common::Log::L_INFO,CLOGFMT(BandwidthEventScanner,"caught VSTRM_BANDWIDTH_EVENT_CDN_STATE_CHANGE event, query bandwidth manager for bandwidth usage") );
				mScanner.notify();
			}
		default:
			break;
		}

		memset( &bwEventData , 0 , sizeof(bwEventData) );
		bOK = VstrmFindNextBandwidthNotification(mHandleClass , hNotification,&bwEventData );
		if( !bOK )
		{
			//quit the loop ?
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(BandwidthEventScanner,"failed to invoke VstrmFindNextBandwidthNotification() "));
		}
	}while( bOK && (!mbQuit) );	
	
	VstrmFindCloseBandwidthNotification(mHandleClass , hNotification);
#else
	glog(ZQ::common::Log::L_WARNING,CLOGFMT(BandwidthEventScanner,"version < 6.1.0.9310 is not defined , quit scanning bandwidthNotification"));
#endif
	return 0;
}



#ifndef kVSTRM_BRB_FLAG_NETWORK_IMPORT
	#define kVSTRM_BRB_FLAG_NETWORK_IMPORT 0x200 
#endif
// 
BandwidthUsageScanner::BandwidthUsageScanner( VstrmClass& c )
:_cls(c),
_hClass(0)
{
	_hEvent = CreateEvent( NULL , FALSE , FALSE , NULL );
	_bQuit = false;
	VstrmClassOpenEx(&_hClass);

}
BandwidthUsageScanner::~BandwidthUsageScanner()
{
	stop( );
	VstrmClassCloseEx(_hClass);
	_hClass = NULL;
}

void BandwidthUsageScanner::stop( )
{
	if(!_bQuit && _hEvent != NULL )
	{
		_bQuit = true;
		SetEvent(_hEvent);
		waitHandle(10000);
		CloseHandle(_hEvent);
		_hEvent = NULL;
	}
}

bool BandwidthUsageScanner::init( )
{
	return true;
}
void BandwidthUsageScanner::final(void)
{

}
void dumpBandiwtdhBcbWrapper( const VSTRM_BANDWIDTH_BCB_WRAPPER& bcb )
{
	const VSTRM_BANDWIDTH_CONTROL_BLOCK& b = bcb.Bcb;
	glog(ZQ::common::Log::L_DEBUG,CLOGFMT(BandwidthUsageScanner,"got ticket[%llx]\tassetName[%s]\tflag[%08lx]\tTicketAffiliate[%llx]\tclientId[%lx]\tState[%lu]\t"
		"Type[%lu]\tTargetType[%lu]\tMaxBw[%llu]\tMinBw[%llu]\tMaxTimeReserved[%lu]\tReservedBW[%llu]\tLocalReservedBW[%llu]\tsessId[%lu]"),
		b.Ticket,bcb.AssetName,b.Flags,
		b.TicketAffiliate,  b.ClientId, b.State, 
		b.Type, b.TargetType, b.MaxBandwidth, b.MinBandwidth, b.MaxTimeReserved, b.ReservedBandwidth, b.LocalReservedBandwidth,
		b.SessionId	);
}
void BandwidthUsageScanner::notify( )
{
	if( _hEvent) 
	{
		SetEvent(_hEvent);
	}
}

void BandwidthUsageScanner::cdnBandwidthUsage( BandwidthUsage& usage )
{
	int64		totalUsage = 0;
	VSTRM_BANDWIDTH_BCB_WRAPPER		bcb;	
	ULONG64							ticket = 0;
	ULONG							curIndex = 0;

	VSTATUS vret = VSTRM_SUCCESS;

	while( vret == VSTRM_SUCCESS )
	{
		memset(&bcb,0,sizeof(bcb));

		vret =  VstrmClassGetNextBandwidthTicketHolder( _hClass , &bcb  , sizeof(bcb) ,&ticket, &curIndex );
		if( ticket == (ULONG64)-1 )
			break;
		if( VSTRM_SUCCESS != vret )
		{
			break;
		}
		else if ( 
					( ( bcb.Bcb.Flags & kVSTRM_BRB_FLAG_NETWORK_IMPORT)  ||  (bcb.Bcb.Flags & kVSTRM_BCB_FLAG_NETWORK_STREAMING) )
						&& 
					( ( bcb.Bcb.ReservedBandwidth > 0 ) && 	(bcb.Bcb.TargetType == kVSTRM_BANDWIDTH_TARGETTYPE_HOSTNIC)	)
				) 
		{//NGOD-502
			usage.cdnUsedBandiwidth += bcb.Bcb.ReservedBandwidth;
			usage.cdnImportSessionCount++;			
		}
		//dumpBandiwtdhBcbWrapper( bcb );
	}
	
	VSTRM_BANDWIDTH_INQUIRY_INFO_EX2 info2;
	memset( &info2, 0, sizeof(info2) );
	info2.Version = 4;
	info2.NodeId = 0xFFFFFFFF;

	ULONG size = sizeof(info2);
	
	if( VstrmClassInquireBandwidthEx2( _hClass, & info2, size ) == VSTRM_SUCCESS )
	{
		//according to VVX-1297, use HostNic replace File as importChannel data source
		//VSTRM_BANDWIDTH_POOL_EX3& pool = info2.Data.v4.File.Pool[kVSTRM_BANDWIDTH_POOLTYPE_CURRENT];
		VSTRM_BANDWIDTH_POOL_EX3& pool = info2.Data.v4.HostNic[0].Pool[kVSTRM_BANDWIDTH_POOLTYPE_CURRENT];
		usage.cdnUsedBandiwidth = pool.ReadInitBw - pool.ReadAvailBw;
		usage.cdnTotalBandwidth = pool.ReadInitBw;
	}
//  	glog(ZQ::common::Log::L_DEBUG,CLOGFMT(BandwidthUsageScanner,"end scanning bandwidth ticket usage [%lld]/[%lld],[%ld]"),
//  		usage.cdnUsedBandiwidth,
//  		usage.cdnTotalBandwidth,
//  		usage.cdnImportSessionCount);
}

int BandwidthUsageScanner::run( )
{
	_bQuit = false;
	uint32 interval = GAPPLICATIONCONFIGURATION.lBandwidthUsageScanInterval;
	if( interval < 100)
		interval = 100;
	if( interval > 60*60*1000)
		interval = 60*60*1000;
	while( !_bQuit )
	{
		BandwidthUsage usage;
		cdnBandwidthUsage( usage );		
		_cls.updateCdnBwUsage( usage );
		WaitForSingleObject( _hEvent , interval );
		if( _bQuit) break;		
	}
	return 1;
}

SpigotLinkScanner::SpigotLinkScanner(VstrmClass& c)
:mVc(c),
mbQuit(false)
{
}

SpigotLinkScanner::~SpigotLinkScanner( )
{
}

int SpigotLinkScanner::run()
{
	uint32 interval = gStreamSmithConfig.lSpigotStatusScanInterval;
	interval = interval < 1000 ? 1000 : interval;
	interval = interval > 24 * 60 * 60 * 1000  ? 24 * 60 * 60 * 1000 : interval;
	//VSTRMSPIGOTS	m_spigotMap;
	VstrmClass::VSTRMSPIGOTS spigotMap = mVc.m_spigotMap;
	do
	{
		VstrmClass::VSTRMSPIGOTS::iterator it = spigotMap.begin();
		for( ; it != spigotMap.end() ; it ++ )
		{
			int status = mVc.getSpigotLinkState( it->second.portLowPart ) > 0 ? STATUS_READY :STATUS_OUTOFSERVICE;
			if( status != it->second.status )
			{				
				mVc.updateSpigotStatus( it->second.spigotId , status );
				it->second.status = status;
			}
		}
		{
			ZQ::common::MutexGuard gd(mMutex);
			mCond.wait( mMutex ,interval );
		}
	}while( !mbQuit );	

	return 1;
}
void SpigotLinkScanner::stop()
{
	mbQuit = true;
	mCond.signal();
	waitHandle( 5000 * 1000 );
}

// per VVX-1417 -andy, 02/09/2010
int VstrmGetSessionOnPort(HANDLE vstrmClassHandle, ULONG port_number) 
{ 
	int sessionActive = FALSE; 
#ifdef VstrmClassGetOnePortShortCharsByPort
	PEPORT_SHORT_CHARACTERISTICS pChars = (PEPORT_SHORT_CHARACTERISTICS) ::malloc(sizeof(EPORT_SHORT_CHARACTERISTICS)); 
	
	if (NULL != pChars) 
	{ 
		ULONG retSize; 
		VSTATUS vstatus = ::VstrmClassGetOnePortShortCharsByPort(vstrmClassHandle, pChars, sizeof(*pChars), &retSize, port_number); 

		if (IS_VSTRM_SUCCESS(vstatus)) 
		{ 
			if(pChars->PortCharacteristics.DeviceSpecificChars.EdgeChars.sessionIdCurrent) 
				sessionActive = TRUE; 
		} 

		::free(pChars); 
	} 
#endif
	return sessionActive; 
}

std::string getVstrmError( VHANDLE h,VSTATUS err )
{
	char szBuf[1024];
	szBuf[sizeof(szBuf)-1] = 0;
	VstrmClassGetErrorText(h,err,szBuf,sizeof(szBuf)-2);
	return szBuf;
}


}}//namespace ZQ::StreamSmith
		
