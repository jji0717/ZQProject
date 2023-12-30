
#include <math.h>
#include <ZQ_common_conf.h>
#include <TianShanIceHelper.h>
#include <ContentStore.h>
#include <ContentSysMD.h>
#include "HelperClass.h"
#include "StreamSmithConfig.h"
#include <IndexFileParser.h>

#ifdef ZQ_OS_MSWIN
	#include <WS2tcpip.h>
#elif defined ZQ_OS_LINUX

#endif

extern "C"
{
	#include <VstrmVer.h>
	#include <vstrmuser.h>
}
#include "VstrmSsCommand.h"

#include <memoryDebug.h>

#ifdef ENVLOG
	#undef ENVLOG
#endif

#define ENVLOG ( *(mEnv->getMainLogger()) )


const std::string KEY_ITEM_FULL_PATH_NAME		= "ItemFullPathName";
const std::string KEY_PLAYLIST_SESSION_CB_ID	= "PlaylistSessionCallBackId";
const std::string KEY_CONTENT_STATE_RECORD		= "ContentStateRecord";
const std::string KEY_CONTENT_VSTRM_FLAG		= "VstrmContentFlag";

#if defined ZQ_OS_MSWIN
	#define	PLFMT(x,y) 	"%s/%08X/VstrmCommand[%s]\t"##y,mContextKey.c_str(),GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
	#define	PLFMT(x,y) 	%s/%08X/VstrmCommand[%s]\t"##y,mContextKey.c_str(),pthread_self(),#x
#endif	

namespace ZQ
{
namespace StreamService
{

int	ConvertStringIntoBinary( const std::string& strContent , char* buf )
{
	std::string str  = strContent;
	if( str.size() % 2 == 0 ) 
	{		
	}
	else
	{
		str = "0" + str;
	}

	std::transform(str.begin(), str.end(), str.begin(), toupper); 
	int iLen = static_cast<int>( str.length());
	char* pStr = (char*)str.c_str();
	unsigned char cData = 0;
	int i = 0;
	for( i=0; i<iLen; i++ )
	{
		if (isdigit(pStr[i])) 
		{
			cData=pStr[i] - '0';
		}
		else
		{
			cData=(pStr[i] - 'A') + 10;
		}
		if(i%2==0)
		{
			buf[i/2] = cData << 4;
		}
		else
		{
			buf[i/2]|= cData;
		}
	}
	return i/2;
}

VstrmCommand::VstrmCommand(  SsEnvironment* environment , VstrmStreamerManager& streamerManager , VstrmSessionScaner& sessScaner ,
						   SsServiceImpl&		service , SsContext& ctx )
:mStreamerManager(streamerManager),
mSessScaner(sessScaner),
mContextKey( ctx.id() ),
ss(service),
mSsCtx(ctx)
{
	mUrlIndex = -1;
	mStreamPort = ctx.getStreamingPort();
	if( !mStreamPort.empty() )
	{
		miStreamPort = atoi(mStreamPort.c_str());
	}
	else
	{
		ENVLOG(ZQ::common::Log::L_WARNING,PLFMT(VstrmCommand,"no stream port is available"));
		miStreamPort = -1;
	}
	mEnv  = dynamic_cast<StreamSmithEnv *>(environment);
	if (NULL == mEnv)
	{
		assert(false);
	}
	memset(&vstrmTickets,0,sizeof(vstrmTickets));
	mbEdgeServer = ( gStreamSmithConfig.serverMode == SERVER_MODE_EDGE );
}

VstrmCommand::~VstrmCommand()
{

}

ULONG VstrmCommand::getVstrmPort( )
{
	return miStreamPort;
}

int32 VstrmCommand::setDvbAttr(  DVB_SESSION_ATTR& dvbAttr , Ice::Int& muxRate )
{
	std::string		targetIp;
	Ice::Int		targetPort;	
	std::string		targetMac;
	ZQTianShan::Util::getPropertyDataWithDefault( mSsCtx.getContextProperty() , STREAMINGRESOURCE_DESTINATION_IPADDRESS, "" , targetIp );
	ZQTianShan::Util::getPropertyDataWithDefault( mSsCtx.getContextProperty() , STREAMINGRESOURCE_DESTINATION_UDPPORT , 0, targetPort );
	ZQTianShan::Util::getPropertyDataWithDefault( mSsCtx.getContextProperty() , STREAMINGRESOURCE_DESTINATION_MACADDRESS ,"" , targetMac );

	if( targetIp.empty())
	{
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(setDvbAttr,"no destination ip/port is found"));
		return ERR_RETURN_INVALID_PARAMATER;
	}
	
	ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(setDvbAttr,"set DestIp[%s] DestPort[%d] destMac[%s]"), targetIp.c_str() , targetPort , targetMac.c_str() );

	unsigned short targetUdpPort = static_cast<unsigned short>( targetPort );
	
	addrinfo* resAddrinfo;
	addrinfo hint;
	ZeroMemory(&hint,sizeof(hint));
	hint.ai_family = AF_INET6;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_protocol = IPPROTO_TCP;
	hint.ai_flags = AI_PASSIVE ;
	//test if it's IPV6
	char szPort[128];
	sprintf(szPort,"%d",targetUdpPort );
	int rc = getaddrinfo( targetIp.c_str() , szPort , &hint , &resAddrinfo );
	if (rc !=0 )
	{
		//glog(ZQ::common::Log::L_INFO,PLSESSID(setDestination ,"Can't get ip info from ip string,treat it as IPV4 address"));
		dvbAttr.ipVersion		= IP_VERSION4; //TODO: support IPv6 later
		dvbAttr.ipAddr			= inet_addr( targetIp.c_str() );
		dvbAttr.udpPort			= static_cast<USHORT>( targetUdpPort );
	}
	else
	{
		dvbAttr.ipVersion		= IP_VERSION6;
		sockaddr_in6* in6		= ( sockaddr_in6* ) resAddrinfo->ai_addr;
		memcpy(&dvbAttr.ip6Addr.ipVer , &in6->sin6_addr , sizeof(dvbAttr.ip6Addr.ipVer));
		dvbAttr.ip6Addr.flow	=	in6->sin6_flowinfo;
		dvbAttr.ip6Addr.hopLimit= IPV6_HOPLIMIT_DFLT;
		dvbAttr.ip6Addr.ip_class= 0;
		dvbAttr.udpPort			= static_cast<USHORT>( targetUdpPort );
		if( resAddrinfo )
		{
			freeaddrinfo(resAddrinfo);
			resAddrinfo = NULL;
		}
	}

	std::string& destMac = targetMac;
	if(destMac.size()>0)
	{
		int		iTemp[6];
		UCHAR	uMac[6];
		if(sscanf(destMac.c_str(),"%d-%d-%d-%d-%d-%d",&iTemp[0],&iTemp[1],&iTemp[2],&iTemp[3],
			&iTemp[4],&iTemp[5])==6)
		{
			for(int i=0;i<6;i++)
			{
				uMac[i]=(UCHAR)iTemp[i]+'0';
			}
			memcpy(dvbAttr.macAddr,uMac,sizeof(dvbAttr.macAddr));
			//memcpy(_dvbAttrs.macAddr,m_strDestinationMac.c_str(),sizeof(_dvbAttrs.macAddr));
		}
		else if(sscanf(destMac.c_str(),"%x:%x:%x:%x:%x:%x",&iTemp[0],&iTemp[1],&iTemp[2],&iTemp[3],
			&iTemp[4],&iTemp[5])==6)
		{
			for(int i=0;i<6;i++)
			{
				uMac[i]=(UCHAR)iTemp[i];
			}
			memcpy(dvbAttr.macAddr,uMac,sizeof(dvbAttr.macAddr));
			//memcpy(_dvbAttrs.macAddr,m_strDestinationMac.c_str(),sizeof(_dvbAttrs.macAddr));
		}
		else if(sscanf(destMac.c_str(),"%2X%2X%2X%2X%2X%2X",&iTemp[0],&iTemp[1],&iTemp[2],&iTemp[3],
			&iTemp[4],&iTemp[5])==6)
		{
			for(int i=0;i<6;i++)
			{
				uMac[i]=(UCHAR)iTemp[i];
			}
			memcpy(dvbAttr.macAddr,uMac,sizeof(dvbAttr.macAddr));
		}
		else
		{			
			ENVLOG(ZQ::common::Log::L_ERROR,
				PLFMT(setDvbAttr,"Invalid mac format [%s],should be [d-d-d-d-d-d] or [x:x:x:x:x:x] or [2X2X2X2X2X2X]"),
				destMac.c_str()	);
			return ERR_RETURN_INVALID_PARAMATER;
		}
	}

	ZQTianShan::Util::getPropertyDataWithDefault( mSsCtx.getContextProperty() ,STREAMINGRESOURCE_MUXRATE_MAX , 0, muxRate );
	dvbAttr.MaxMuxRate	= muxRate;
	dvbAttr.MinMuxRate	= 0;
	dvbAttr.NowMuxRate	= muxRate;

	
	return ERR_RETURN_SUCCESS;
}

SPEED_IND	VstrmCommand::convertSpeed( const float& speed )
{
	if( fabs(speed - 0.001f ) < 0.001f  )
	{
		SPEED_IND ret={0,0};
		return ret;
	}
	SPEED_IND ret;
	
	ret.denominator		= 1000;
	ret.numerator		= static_cast<int>(1000*speed);
	if( speed < 0.0001f )
	{		
		ret.numerator		= -ret.numerator;
	}
	int gcd = ZQTianShan::Util::calculateGCD<int>( ret.numerator , ret.denominator );
	if( gcd != 0)
	{
		ret.numerator /= gcd;
		ret.denominator /= gcd;
	}
	if( speed < 0.0001f )
	{		
		ret.numerator		= -ret.numerator;
	}
	return ret;

}

bool VstrmCommand::hasEcmData( const TianShanIce::Streamer::PlaylistItemSetupInfo& info )
{
	Ice::Int	enable = 0;
	ZQTianShan::Util::getValueMapDataWithDefault( info.privateData , ITEM_PARA_ECM_ENCRYPTION_ENABLE,0,enable );
	return enable > 0 ;
}

USHORT VstrmCommand::getEcmPid( const TianShanIce::Streamer::PlaylistItemSetupInfo& info )
{
	assert( hasEcmData( info ) );
	Ice::Int programNumber = 0;
	try
	{
		ZQTianShan::Util::getValueMapData( info.privateData , ITEM_PARA_ECM_PROGRAMNUMBER , programNumber );
	}
	catch(const TianShanIce::InvalidParameter&)
	{
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(getEcmPid,"bad ecm data, no programNumber is found"));
	}
	return programNumber;
}

int	 VstrmCommand::setEcmData(  LOAD_PARAM* loadPara,  const TianShanIce::Streamer::PlaylistItemSetupInfo& info , int paraCount )
{	
	if( hasEcmData(info) )
	{
		try
		{
		mEcmData.vendor		=	VSTRM_ENCRYPTION_VENDOR_MOTOROLA;
		const TianShanIce::ValueMap& privateData = info.privateData;
		//get ecm pid
		Ice::Int	ecmPid	= 0;
		ZQTianShan::Util::getValueMapData( privateData , ITEM_PARA_ECM_PROGRAMNUMBER , ecmPid );
		ecmPid=(ecmPid<<4)+15;
		mEcmData.ecmPid		=	static_cast<USHORT>(ecmPid);

		Ice::Int	cycle1	= 0;
		ZQTianShan::Util::getValueMapData(privateData,ITEM_PARA_ECM_CYCLE1,cycle1);
		mEcmData.Cycle1		=	static_cast<ULONG>(cycle1);

		Ice::Int	cycle2	=	0;
		ZQTianShan::Util::getValueMapData(privateData,ITEM_PARA_ECM_CYCLE2,cycle2);
		mEcmData.Cycle2		=	cycle2;

		Ice::Int	freq1	=	0;
		ZQTianShan::Util::getValueMapData(privateData,ITEM_PARA_ECM_FREQ1,freq1);
		mEcmData.Freq1		=	freq1;
		Ice::Int	freq2	=	0;
		ZQTianShan::Util::getValueMapData(privateData,ITEM_PARA_ECM_FREQ2,freq2);
		mEcmData.Freq2		=	freq2;

		std::vector<Ice::Int>	pnOffsets;
		bool bRange = false;
		ZQTianShan::Util::getValueMapData( privateData , ITEM_PARA_ECM_KEYOFFSETS , pnOffsets , bRange );
		
		Ice::Int mEcmDataCount = static_cast<Ice::Int>(pnOffsets.size());
		
		for( Ice::Int i = 0 ; i < mEcmDataCount ; i++ )
		{
			mEcmData.motoData[i].ProgramNumberOffset	= pnOffsets[i];
		}

		std::vector<std::string> ecmContents;
		ZQTianShan::Util::getValueMapData(privateData, ITEM_PARA_ECM_KEYS ,ecmContents ,bRange );
		mEcmDataCount = static_cast<Ice::Int>(ecmContents.size());
		for( Ice::Int i = 0 ; i < mEcmDataCount ; i++ )
		{
			mEcmData.motoData[i].byteCount = 4 + ConvertStringIntoBinary( ecmContents[i], mEcmData.motoData[i].Message );
		}

		}
		catch( const TianShanIce::InvalidParameter&)
		{
			return -1;
		}
		loadPara[paraCount].loadCode		=	(USHORT)LOAD_CODE_ENCRYPTION_VENDOR;
		loadPara[paraCount].loadValueP	=	&(mEcmData.vendor);
		loadPara[paraCount].loadValueLength=	sizeof(mEcmData.vendor);
		paraCount++;

		loadPara[paraCount].loadCode		=	(USHORT)LOAD_CODE_ENCRYPTION_PID;
		loadPara[paraCount].loadValueP	=	&(mEcmData.ecmPid);
		loadPara[paraCount].loadValueLength=	sizeof(mEcmData.ecmPid);
		paraCount++;

		loadPara[paraCount].loadCode		=	(USHORT)LOAD_CODE_ENCRYPTION_CYCLE_1;
		loadPara[paraCount].loadValueP	=	&(mEcmData.Cycle1);
		loadPara[paraCount].loadValueLength=	sizeof(mEcmData.Cycle1);
		paraCount++;

		loadPara[paraCount].loadCode		=	(USHORT)LOAD_CODE_ENCRYPTION_CYCLE_2;
		loadPara[paraCount].loadValueP	=	&(mEcmData.Cycle2);
		loadPara[paraCount].loadValueLength=	sizeof(mEcmData.Cycle2);
		paraCount++;

		loadPara[paraCount].loadCode		=	(USHORT)LOAD_CODE_ENCRYPTION_FREQUENCY_1;
		loadPara[paraCount].loadValueP	=	&(mEcmData.Freq1);
		loadPara[paraCount].loadValueLength=	sizeof(mEcmData.Freq1);
		paraCount++;

		loadPara[paraCount].loadCode		=	(USHORT)LOAD_CODE_ENCRYPTION_FREQUENCY_2;
		loadPara[paraCount].loadValueP	=	&(mEcmData.Freq2);
		loadPara[paraCount].loadValueLength=	sizeof(mEcmData.Freq2);
		paraCount++;

		loadPara[paraCount].loadCode		=	(USHORT)LOAD_CODE_ENCRYPTION_DATA;
		loadPara[paraCount].loadValueP	=	&(mEcmData.motoData);
		loadPara[paraCount].loadValueLength=	 static_cast<USHORT>((mEcmDataCount+1)*sizeof(mEcmData.motoData[0]));
		paraCount++;
	}
	return paraCount;
}

std::string	VstrmCommand::getNextUrl( const TianShanIce::Streamer::PlaylistItemSetupInfo& info )
{
	++mUrlIndex;
	std::vector<std::string> urls;
	bool bRange = false;
	try
	{
		ZQTianShan::Util::getValueMapData( info.privateData , ITEM_LIBRARY_URL , urls , bRange );
		if( urls.size() <= 0 )
			return "";
		if ( mUrlIndex >= (int)urls.size() )
		{
			mUrlIndex = 0;
		}		
		return urls[mUrlIndex];
	}
	catch( const TianShanIce::InvalidParameter& )
	{
		urls.clear();
		return "";
	}
}

std::string VstrmCommand::setUrlData(  LOAD_PARAM* loadPara,  const TianShanIce::Streamer::PlaylistItemSetupInfo& info , int& paraCount  )
{	
	std::string	strUrl;
	if( itemHasUrl( info )  )
	{
#if VER_PRODUCTVERSION_MAJOR >= 6
		strUrl = getNextUrl( info );
		if( !strUrl.empty() )
		{
			ENVLOG(ZQ::common::Log::L_INFO,PLFMT(setUrlData,"set url to [%s]"),strUrl.c_str());
			strncpy(mCurrentUrl, strUrl.c_str() ,strUrl.length() );
			loadPara[paraCount].loadCode		=	(USHORT)LOAD_CODE_OBJECT_NAME_URL;
			loadPara[paraCount].loadValueP		=	mCurrentUrl;
			loadPara[paraCount].loadValueLength	=	static_cast<USHORT>( strUrl.length() );
			paraCount ++;
		}
#endif //only version >= 6  can support NAS streaming
	}
	return strUrl;
}



int32 VstrmCommand::prepareLoadParam(	const TianShanIce::Streamer::PlaylistItemSetupInfo& info ,
										Ice::Long timeOffset,
										IN IOCTL_LOAD_PARAMS&	loadParams,
										OUT IOCTL_CONTROL_PARMS_LONG& parmsV2 )
{
	
	//prepare parameter to load item
	

	memset(&loadParams, 0x00, sizeof(loadParams));
	memset(&dvbAttrs,0x00, sizeof(dvbAttrs));

	loadParams.FileLocal 			= TRUE;	//Should I reset this flag when streaming a NAS object ?
	loadParams.Debug 				= TRUE;

	loadParams.Mask =	LOAD_IOCTL_FILE_LOCAL
						| LOAD_IOCTL_PAUSE_ON_PLAY
						| LOAD_IOCTL_FRAME_COUNT
						| LOAD_IOCTL_MASTER_SESSION_ID
						| LOAD_IOCTL_PRE_BLACK_FRAMES
						| LOAD_IOCTL_POST_BLACK_FRAMES
						| LOAD_IOCTL_TIME_SKIP
						| LOAD_IOCTL_BYTE_SKIP
						| LOAD_IOCTL_SKIP_HEADER
						| LOAD_IOCTL_DEST_PORT_HANDLE
						| LOAD_IOCTL_TERMINATE_ON_EXIT
						| LOAD_IOCTL_PLAYLIST_FLAG
						| LOAD_IOCTL_OBJECT_NAME
						| LOAD_IOCTL_DEBUG
						| LOAD_IOCTL_SPECIFIED_DIRECTORY;

	loadParams.TimeSkip				= static_cast<ULONG>(timeOffset);

#ifdef FILE_FLAG_NPVR
	Ice::Int	fileFlags = 0;
	ZQTianShan::Util::getValueMapDataWithDefault( info.privateData, KEY_CONTENT_VSTRM_FLAG , 0 , fileFlags );
	loadParams.FlagsAndAttributes	= ( fileFlags & STREAMSMITH_FILE_FLAG_NPVR) ?  FILE_FLAG_NPVR : 0 ;
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(prepareLoadParam,"use npvr file flag [%s]") , ( ( fileFlags & STREAMSMITH_FILE_FLAG_NPVR) ? "TRUE" : "FALSE" ));
#endif

	// Init the parameter table
	LOAD_PARAM paramTableV2[] =
	{
		LOAD_CODE_OBJECT_NAME,		 		&loadParams.ObjectName,			0,
		LOAD_CODE_TERMINATE_ON_EXIT,		&loadParams.TerminateOnExit,	sizeof(loadParams.TerminateOnExit),						
		LOAD_CODE_TIME_SKIP,				&loadParams.TimeSkip,			sizeof(loadParams.TimeSkip),					
		LOAD_CODE_DEST_HANDLE,				&loadParams.DestPortHandle,		sizeof(loadParams.DestPortHandle),				
		LOAD_CODE_DEBUG,					&loadParams.Debug,				sizeof(loadParams.Debug),

#ifdef FILE_FLAG_NPVR
		LOAD_CODE_FLAGS_AND_ATTRIBUTES,		&loadParams.FlagsAndAttributes,	sizeof(loadParams.FlagsAndAttributes),
#endif//FILE_FLAG_NPVR

		LOAD_CODE_DVB_SESSION_ATTRIBUTES, 	&dvbAttrs,						sizeof(dvbAttrs)	
	};

	std::string fullPathName			= getItemFullPathName( info );
	strcpy ( (PCHAR)&loadParams.ObjectName, fullPathName.c_str() );
	
	ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(prepareLoadParam,"use full name [%s]"),fullPathName.c_str() );

	paramTableV2[0].loadValueLength		= static_cast<USHORT>( fullPathName.length() );
	// Set the output destination
	loadParams.DestPortHandle			= getVstrmPort( );
	// Abort video on exit
	loadParams.TerminateOnExit			= FALSE;
	// splice-in or cue-in
	dvbAttrs.spliceIn					= info.spliceIn;
	dvbAttrs.inTimeOffset				= static_cast<ULONG>(info.inTimeOffset);
	// splice-out or cue-out
	dvbAttrs.spliceOut					= info.spliceOut;
	dvbAttrs.outTimeOffset				= static_cast<ULONG>(info.outTimeOffset);
	// force normal speed
	dvbAttrs.forceNormalSpeed			= info.forceNormal;
	//set destination mac-address	
	//STREAMINGRESOURCE_MUXRATE_MAX
	
	Ice::Int	maxMuxrate = 0;	

	if( setDvbAttr( dvbAttrs , maxMuxrate ) != ERR_RETURN_SUCCESS ) 
		return false;

	dvbAttrs.ProgramNumber				= static_cast<USHORT>( getVstrmPort( ) + 2 );

#pragma message(__MSGLOC__"TODO:pid is not set")
// 	dvbAttrs.VideoPid.Old				=
// 	dvbAttrs.VideoPid.New				=  
	if ( hasEcmData( info ) ){ dvbAttrs.EcmPid.New = dvbAttrs.EcmPid.Old = getEcmPid(info); }

	memset(&parmsV2 , 0 ,sizeof(parmsV2));
	parmsV2.u.load.loadParamCount = sizeof (paramTableV2) / sizeof (LOAD_PARAM);
	int iParameterCount = 0;
	for ( iParameterCount = 0; iParameterCount < parmsV2.u.load.loadParamCount; iParameterCount++)
	{
		parmsV2.u.load.loadParamArray[iParameterCount].loadCode 		= paramTableV2[iParameterCount].loadCode;	
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueLength 	= paramTableV2[iParameterCount].loadValueLength;
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueP		= paramTableV2[iParameterCount].loadValueP; 	
	}
	if( hasEcmData( info ) )
	{
		iParameterCount = setEcmData( parmsV2.u.load.loadParamArray , info , iParameterCount );
		if( iParameterCount < 0 ) return false;
	}

	std::string strUrl;
	if( !mbEdgeServer && itemHasUrl( info ))//in EdgeServer mode , do not push url into vstrm
	{
		strUrl = setUrlData( parmsV2.u.load.loadParamArray, info , iParameterCount );
		if( strUrl.empty() ) return false;
	}
	ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(prepareLoadParam,"use vstrmPort[%u]"),dvbAttrs.ProgramNumber );
	
	getReservedTicket( );

	bRealeaseBWTicketOnTerm = FALSE;

	// Ensure that Vstrm will cleanup bandwidth tickets if we don't. 
	parmsV2.u.load.loadParamArray[iParameterCount].loadCode					= LOAD_CODE_BANDWIDTH_TICKET; 
	parmsV2.u.load.loadParamArray[iParameterCount].loadValueLength			= sizeof( vstrmTickets.FileTicket); 
	parmsV2.u.load.loadParamArray[iParameterCount].loadValueP				= &vstrmTickets.FileTicket; 
	iParameterCount++; 

	parmsV2.u.load.loadParamArray[iParameterCount].loadCode					= LOAD_CODE_REL_BANDWIDTH_TICKET_ONTERM; 
	parmsV2.u.load.loadParamArray[iParameterCount].loadValueLength			= sizeof(bRealeaseBWTicketOnTerm); 
	parmsV2.u.load.loadParamArray[iParameterCount].loadValueP				= &bRealeaseBWTicketOnTerm; 
	iParameterCount++; 


	parmsV2.u.load.loadParamCount = iParameterCount;

	return ERR_RETURN_SUCCESS;

}

//Ice::Long
#define STREAMINGBANDWITHTICKETS_FILE	STREAMINGRESOURCE(vstrmBandwidth.ticket.file)
#define	STREAMINGBANDWITHTICKETS_EDGE	STREAMINGRESOURCE(vstrmBandwidth.ticket.edge)
#define STREAMINGBANDWITHTICKETS_CDN	STREAMINGRESOURCE(vstrmBandwidth.ticket.cdn)

int32 VstrmCommand::getReservedTicket( )
{	
	memset( &vstrmTickets , 0, sizeof(vstrmTickets) );
	int64 tickets[3];
	try
	{		
		ZQTianShan::Util::getPropertyDataWithDefault( mSsCtx.getContextProperty() , STREAMINGBANDWITHTICKETS_FILE, 0 , tickets[0] );
		vstrmTickets.FileTicket = tickets[0];

		ZQTianShan::Util::getPropertyDataWithDefault( mSsCtx.getContextProperty() , STREAMINGBANDWITHTICKETS_EDGE, 0 , tickets[1] );
		vstrmTickets.EdgeTicket = tickets[1];

		ZQTianShan::Util::getPropertyDataWithDefault( mSsCtx.getContextProperty() , STREAMINGBANDWITHTICKETS_CDN , 0 , tickets[2] );
		vstrmTickets.CdnTicket = tickets[2];
	}
	catch( const TianShanIce::InvalidParameter&)
	{
	}
	return ERR_RETURN_SUCCESS;
}

void VstrmCommand::collectRunningBandWidthTickets( std::vector<ULONG64>& runningTickets ) 
{
	runningTickets.clear();
	VSTRM_BANDWIDTH_BCB_WRAPPER		bcb;	
	VSTATUS							vret = VSTRM_SUCCESS;
	ULONG							curIndex = 0;
	ULONG64							ticket = 0;
	while( vret == VSTRM_SUCCESS )
	{
		memset(&bcb,0,sizeof(bcb));
		vret =  VstrmClassGetNextBandwidthTicketHolder( vstrmHandle() , &bcb  , sizeof(bcb) ,&ticket, &curIndex );
		if( ticket == (ULONG64)-1 )
			break;
		if( VSTRM_SUCCESS != vret )
		{				
			break;
		}
		else if ( bcb.Bcb.ClientId == kVSTRM_BANDWIDTH_CLIENTID_ZQ_STREAMSMITH )
		{
			runningTickets.push_back( bcb.Bcb.Ticket );				
		}
	}
}

VSTATUS sessionControlCallback(HANDLE classHandle, PVOID cbParam , PVOID bufP, ULONG bufLen)
{
	extern ZQ::StreamService::SsServiceImpl* pServiceInstance;
	PIOCTL_STATUS_BUFFER pStatusBlk = (PIOCTL_STATUS_BUFFER) bufP;	
	VsrtmSessCallbackReqeust* pRequest = new VsrtmSessCallbackReqeust(	getSsEnv()->getMainThreadPool(), 
																		getSsEnv(),
																		*pServiceInstance,
																		reinterpret_cast<uint32>(cbParam),
																		pStatusBlk->sessionId,
																		"");
	assert( pRequest != NULL );
	pRequest->start();
	return VSTRM_SUCCESS;
}

#ifndef kVSTRM_BANDWIDTH_CLIENTID_ZQ_STREAMSMITH
	#define kVSTRM_BANDWIDTH_CLIENTID_ZQ_STREAMSMITH	0x04000000
#endif

int32 VstrmCommand::reserveBandwidthTickets( const PlaylistItemSetupInfos& infos )
{
	memset( &vstrmTickets , 0, sizeof(vstrmTickets) );

	std::string assetName = "Dummy";

	Ice::Int muxRate = 0;
	ZQTianShan::Util::getPropertyDataWithDefault( mSsCtx.getContextProperty() ,STREAMINGRESOURCE_MUXRATE_MAX , 0, muxRate );	

	ULONG spigotId = getSpigotHandle( getVstrmPort() );
	VSTRM_BANDWIDTH_RESERVE_BLOCK	brb; 
	VSTATUS bwStatus	= VSTRM_INVALID_PARAMETER; 
	ULONG clientId		= kVSTRM_BANDWIDTH_CLIENTID_ZQ_STREAMSMITH ;//+ GetCurrentProcessId(); 

	memset(&brb, 0, sizeof(brb)); 

	if ( spigotId != (ULONG) - 1 ) 
	{ 
		// First, allocate bandwidth on the decoder/output board 
		memset(&brb, 0, sizeof(brb)); 
		brb.Type			= kVSTRM_BANDWIDTH_TYPE_WRITE; 
		brb.TargetType		= kVSTRM_BANDWIDTH_TARGETTYPE_SLICE; 
		brb.ClientId		= clientId; 
		brb.BwTarget		= (PVOID64)spigotId; 
		brb.MaxBandwidth	= muxRate; // Need to already know the bitrate 
		brb.MinBandwidth	= 0; 
		bwStatus = VstrmClassReserveBandwidth( vstrmHandle(), &brb, &vstrmTickets.EdgeTicket); 
		if (bwStatus!=VSTRM_SUCCESS) 
		{
			ENVLOG(ZQ::common::Log::L_ERROR, PLFMT(reserveBandwidthTickets,"failed to reserve bandwidth with bitrate[%u] assetName[%s] because[%s] API{VstrmClassReserveBandwidth}"),
				muxRate ,assetName.c_str() , mStreamerManager.getVstrmError( VstrmGetLastError()).c_str());
			return ERR_RETURN_INVALID_PARAMATER;
		} 
	} 
	else 
	{
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT( reserveBandwidthTickets, "Invalid spigot handle") );
		return ERR_RETURN_INVALID_PARAMATER;
	}

	// Now allocate the file bandwidth, passing in the edge card's ticket... 
	if ( bwStatus == VSTRM_SUCCESS )
	{ 
		memset(&brb, 0, sizeof(brb)); 
		brb.Type				= kVSTRM_BANDWIDTH_TYPE_WRITE; 
		brb.TargetType			= kVSTRM_BANDWIDTH_TARGETTYPE_FILE; 
		brb.Flags				= kVSTRM_BRB_FLAG_REL_AFFIL_UPONREL; 

		brb.ClientId			= clientId; 
		brb.TicketAffiliate		= vstrmTickets.EdgeTicket;
		brb.BwTarget			= reinterpret_cast<void*>((char*)"dummy"); 
		brb.MaxBandwidth		= muxRate; 
		brb.MinBandwidth		= 0; 
		bwStatus = VstrmClassReserveBandwidth( vstrmHandle(), &brb, &vstrmTickets.FileTicket); 
		if (bwStatus!=VSTRM_SUCCESS) 
		{
			ENVLOG(ZQ::common::Log::L_ERROR, PLFMT(reserveBandwidthTickets,"failed to reserve file bandwidth with bitrate[%u] asset[%s] because[%s] API{VstrmClassReserveBandwidth}"),
				muxRate , assetName.c_str(), mStreamerManager.getVstrmError( VstrmGetLastError()).c_str()	);
			VstrmClassReleaseBandwidth( vstrmHandle(), vstrmTickets.EdgeTicket);
			return ERR_RETURN_INVALID_PARAMATER;
		} 
	}
	
	mSsCtx.updateContextProperty( STREAMINGBANDWITHTICKETS_FILE , (int64)vstrmTickets.FileTicket );
	mSsCtx.updateContextProperty( STREAMINGBANDWITHTICKETS_EDGE , (int64)vstrmTickets.EdgeTicket );
	mSsCtx.updateContextProperty( STREAMINGBANDWITHTICKETS_CDN , (int64)vstrmTickets.CdnTicket );	

	return ERR_RETURN_SUCCESS;
}

int32 VstrmCommand::releaseBandwidthTickets( )
{
	getReservedTicket();
	
	VSTATUS vret = 0;
	if( vstrmTickets.FileTicket != 0 )
	{
		vret = VstrmClassReleaseBandwidth( vstrmHandle() , vstrmTickets.FileTicket );
#if defined _DEBUG || defined DEBUG
		if( vret != VSTRM_SUCCESS )
		{
			char szErr[1024];
			VstrmClassGetErrorText(vstrmHandle(),vret,szErr,sizeof(szErr));
			printf("failed to release bw with FileTicket [%llx] because [%s]\n",vstrmTickets.FileTicket , szErr);

		}
#endif

	}
	if( vstrmTickets.EdgeTicket != 0 )
	{
		vret = VstrmClassReleaseBandwidth( vstrmHandle() , vstrmTickets.EdgeTicket );
#if defined _DEBUG || defined DEBUG
		if( vret != VSTRM_SUCCESS )
		{
			char szErr[1024];
			VstrmClassGetErrorText(vstrmHandle(),vret,szErr,sizeof(szErr));
			printf("failed to release bw with EdgeTicket [%llx] because [%s]\n",vstrmTickets.EdgeTicket , szErr);

		}
#endif
	}
	if( vstrmTickets.CdnTicket != 0 )
	{
		vret = VstrmClassReleaseBandwidth( vstrmHandle() , vstrmTickets.CdnTicket );
#if defined _DEBUG || defined DEBUG
		if( vret != VSTRM_SUCCESS )
		{
			char szErr[1024];
			VstrmClassGetErrorText(vstrmHandle(),vret,szErr,sizeof(szErr));
			printf("failed to release bw with CdnTicket [%llx] because [%s]\n",vstrmTickets.CdnTicket , szErr);

		}
#endif
	}
	
	memset( &vstrmTickets , 0, sizeof(vstrmTickets) );
	return ERR_RETURN_SUCCESS;
}

int32 VstrmCommand::doLoad(	const TianShanIce::Streamer::PlaylistItemSetupInfo& info ,							
						  float& newSpeed , Ice::Long timeOffset , ULONG& newSessId )
{
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(doLoad,"load item[%s] with speed[%f] timeoffset[%lld]"),info.contentName.c_str() , newSpeed , timeOffset );
	{
		{
			std::string strPokeHoleSessionId = mSsCtx.getContextProperty(VSTRMPROP_POKE_SESSION);			
			if( !strPokeHoleSessionId.empty() )
			{
				ENVLOG(ZQ::common::Log::L_DEBUG, PLFMT(doLoad,"poke hole session [%s] , check remote session signal"), strPokeHoleSessionId.c_str());
#pragma message(__MSGLOC__"TODO: must be a new configuration here")
				if( !mStreamerManager.getRSMonitor().checkRemoteSession( strPokeHoleSessionId , 5000 ) )
				{
					ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doLoad,"can't get remote session callback , reject"));
					return ERR_RETURN_SERVER_ERROR;
				}
				ENVLOG(ZQ::common::Log::L_INFO, PLFMT(doLoad,"check poke hole session[%s] ok"), strPokeHoleSessionId.c_str());
			}

		}
	}
	IOCTL_CONTROL_PARMS_LONG	loadParam;
	IOCTL_LOAD_PARAMS	prepareParam;
	memset(&prepareParam , 0 ,sizeof(prepareParam));
	int32 retValue = prepareLoadParam( info, timeOffset, prepareParam, loadParam   );
	if( retValue != ERR_RETURN_SUCCESS )
		return retValue;
	uint32 callbackId = getCallBackId( mContextKey );

	VSTATUS ret = VstrmClassControlSessionEx1(  mStreamerManager.getVstrmHandle(),
														0,
														VSTRM_GEN_CONTROL_LOAD,
														&loadParam, 
														sizeof(loadParam), 
														sessionControlCallback, 														
#if VER_PRODUCTVERSION_MAJOR < 6
														callbackId );
#else
														reinterpret_cast<PVOID>( callbackId ) );
#endif
	
	//dumpLoadParameter( info );
	if( ret != VSTRM_SUCCESS )
	{//失败咯
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doLoad,"failed to load item[%s] because[%s] API{VstrmClassControlSessionEx1}"),
			info.contentName.c_str() ,
			mStreamerManager.getVstrmError( VstrmGetLastError()).c_str() );
		return ERR_RETURN_INVALID_PARAMATER;
	}
	else
	{
		newSessId = loadParam.u.load.sessionId ;

		//mSessScaner.registerSessionId( mContextKey , newSessId );		
		if ( ERR_RETURN_SUCCESS != doChangeSpeed( newSpeed , false ) )
		{
			doUnload( newSessId );
			return ERR_RETURN_SERVER_ERROR;
		}
		//prime the stream
		ret = VstrmClassPrimeEx( mStreamerManager.getVstrmHandle() , newSessId );
		if( ret != VSTRM_SUCCESS )
		{
			ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doLoad,"failed to prime session [%d] because [%s] API{VstrmClassPrimeEx}"),
					newSessId ,
					mStreamerManager.getVstrmError( VstrmGetLastError() ).c_str() );
			
			doUnload( newSessId );

			return ERR_RETURN_SERVER_ERROR;
		}
		ret = VstrmClassPlayEx ( mStreamerManager.getVstrmHandle() , newSessId );
		if( ret != VSTRM_SUCCESS )
		{
			ENVLOG(ZQ::common::Log::L_ERROR,
				PLFMT(doLoad,"failed to call VstrmClassPlayEx(0 with sessionId[%s] because [%s] API{VstrmClassPlayEx}"),
				newSessId , 
				mStreamerManager.getVstrmError( VstrmGetLastError() ).c_str()	);
			
			doUnload( newSessId );

			return ERR_RETURN_SERVER_ERROR;
		}
		//get current loaded session's character

		ESESSION_CHARACTERISTICS sessInfo;
		if( doGetInfo( newSessId ,sessInfo ) != ERR_RETURN_SUCCESS )
		{			
			ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doLoad,"can't get the session[%u]'s character") , newSessId );
			doUnload( newSessId );
			return ERR_RETURN_SERVER_ERROR;
		}
		if( sessInfo.SessionCharacteristics.MuxRate ==  0 )
		{			
			ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doLoad,"invalid Mux bitrate[%d] "),
				sessInfo.SessionCharacteristics.MuxRate );
			doUnload( newSessId );
			return ERR_RETURN_SERVER_ERROR;
		}

		if( sessInfo.SessionCharacteristics.Speed.denominator == 0 )
		{
			ENVLOG(ZQ::common::Log::L_WARNING,PLFMT(doLoad,"invalid speed denominator [0], set scale as 0.0f"));
			mStreamInfo.scale = 0.0f;
		}
		else
		{
			mStreamInfo.scale			= (float)sessInfo.SessionCharacteristics.Speed.numerator  / (float)sessInfo.SessionCharacteristics.Speed.denominator;
		}

		mStreamInfo.timeoffset		= sessInfo.SessionCharacteristics.PlayoutTimeOffset;
		mStreamInfo.streamState		= TianShanIce::Streamer::stsStreaming;		
		mStreamInfo.mask			= MASK_TIMEOFFSET | MASK_SCALE | MASK_STATE;

		//updateSessionState( it ,  TianShanIce::Streamer::stsStreaming );
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(doLoad,"item[%s] is loaded and sessionId[%u] timeOffset[%lld] scale[%f]"),
			info.contentName.c_str() ,			
			newSessId ,
			mStreamInfo.timeoffset,
			mStreamInfo.scale );
		return ERR_RETURN_SUCCESS;
	}
	
}

int32 VstrmCommand::doUnload( ULONG sessId )
{
	bool bUnloadOK = false;
	if( gStreamSmithConfig.lUseRepositionWhenUnload >= 1 )
	{
		int delayUnloadInterval = gStreamSmithConfig.lRetryIntervalAtUnload;
		if( delayUnloadInterval != 0 )
		{
			delayUnloadInterval = delayUnloadInterval < 100 ? delayUnloadInterval : 100;
			delayUnloadInterval = delayUnloadInterval > 10000 ? 10000 : delayUnloadInterval;
		}

		for ( int i = 0 ; !bUnloadOK && i < 2 ; i ++ )
		{
			IOCTL_CONTROL_PARMS controlParam;
			memset(&controlParam, 0 ,sizeof(controlParam));
			controlParam.u.reposition.timeOffset			=	VSTRM_REPOSITION_STOP_NPT;
			controlParam.controlCode						=	VSTRM_GEN_CONTROL_REPOSITION;			
			controlParam.u.reposition.timeOffsetType		=	ABSOLUTE_TIME_OFFSET;
			controlParam.u.reposition.setSpeed				=	FALSE;

			VSTATUS status = VstrmClassControlSessionEx(	vstrmHandle(), sessId, VSTRM_GEN_CONTROL_REPOSITION,
															&controlParam, sizeof(IOCTL_REPOSITION_PARAMS),
															NULL, NULL);
			if( VSTRM_SUCCESS != status)
			{				
				ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doUnload,"VstrmClassControlSessionEx(VSTRM_GEN_CONTROL_REPOSITION) failed for sess[%d],error [%s]" ),
					sessId,
					mStreamerManager.getVstrmError( VstrmGetLastError() ).c_str());

				if( delayUnloadInterval == 0 )
					break;
				if( VSTRM_INVALID_SESSION != status  )
				{
					Sleep( (i>= 1) ? 200 : delayUnloadInterval );
				}
				else
				{
					bUnloadOK = true;
				}
			}
			else
			{
				bUnloadOK = true;				
			}
		}
	}
	if( !bUnloadOK )
		VstrmClassUnloadEx ( vstrmHandle(), sessId );
	
	return ERR_RETURN_SUCCESS;	
}

int32 VstrmCommand::doReposition( ULONG sessId ,const float& newSpeed ,Ice::Long timeOffset )
{	
	IOCTL_CONTROL_PARMS controlParam;
	memset(&controlParam, 0 ,sizeof(controlParam));
	controlParam.controlCode						=	VSTRM_GEN_CONTROL_REPOSITION;		
	controlParam.u.reposition.timeOffset			=	static_cast<ULONG>(timeOffset);
	controlParam.u.reposition.timeOffsetType		=	ABSOLUTE_TIME_OFFSET;

	controlParam.u.reposition.setSpeed				=	fabs( newSpeed ) > 0.01f ;
	controlParam.u.reposition.speedIndicator		=	convertSpeed( newSpeed );
	
	controlParam.u.playSpeed.direction				=  newSpeed > 0.0001f ? VSTRM_FORWARD :	VSTRM_REVERSE;
	

	VSTATUS retStatus = VstrmClassControlSessionEx(	vstrmHandle(),
													sessId,
													VSTRM_GEN_CONTROL_REPOSITION,
													&controlParam,
													sizeof(IOCTL_REPOSITION_PARAMS),
													NULL,
													NULL);
	if( retStatus != VSTRM_SUCCESS )
	{//失败咯		
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doReposition,"failed to reposition session[%u] to offset[%lld] scale[%f] because[%s] API{VstrmClassControlSessionEx}"),
			sessId ,timeOffset ,newSpeed ,
			mStreamerManager.getVstrmError( VstrmGetLastError() ).c_str());
		return ERR_RETURN_SERVER_ERROR;
	}
	else
	{//我靠，就这么成功了
		SPEED_IND speedRet = controlParam.u.reposition.speedIndicator;
		if(speedRet.denominator == 0 )
		{
			mStreamInfo.scale = 0.0f;
		}
		else
		{
			mStreamInfo.scale			= (float)controlParam.u.reposition.speedIndicator.numerator / 
										(float)controlParam.u.reposition.speedIndicator.denominator;
		}

		mStreamInfo.streamState			= TianShanIce::Streamer::stsStreaming;
		mStreamInfo.timeoffset			= controlParam.u.reposition.timeOffset;
		mStreamInfo.mask				= MASK_TIMEOFFSET | MASK_SCALE | MASK_STATE;
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(doReposition,"reposition ok for [%u] and current timeoffset[%lld] scale[%f]"),
				sessId , mStreamInfo.timeoffset , mStreamInfo.scale);
		return ERR_RETURN_SUCCESS;
	}
}

int32 VstrmCommand::doPause( )
{
	IOCTL_CONTROL_PARMS_LONG controlParam;
	memset(&controlParam, 0 ,sizeof(controlParam));

	ULONG portNum					= getVstrmPort( );
	controlParam.u.pause.timeCode	= TIME_CODE_NOW ;

	VSTATUS status = VstrmClassControlPortEx1(	vstrmHandle(),
												portNum,
												VSTRM_GEN_CONTROL_PAUSE,
												&controlParam,
												sizeof(IOCTL_CONTROL_PARMS_LONG),
												NULL,
												0);
	if( status != VSTRM_SUCCESS )
	{//失败咯		
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doResume,"failed to pause with vstrm port [%u] because [%s] API{VstrmClassControlPortEx1}"),
			portNum,
			mStreamerManager.getVstrmError( VstrmGetLastError() ).c_str() );
		return ERR_RETURN_SERVER_ERROR;
	}
	else
	{//成功鸟		
		SPEED_IND speedRet = controlParam.u.pause.speedIndicator;
		if(speedRet.denominator == 0 )
		{
			mStreamInfo.scale = 0.0f;
		}
		else
		{
			mStreamInfo.scale			= (float)controlParam.u.pause.speedIndicator.numerator / 
											(float)controlParam.u.pause.speedIndicator.denominator;
		}

		mStreamInfo.streamState			= TianShanIce::Streamer::stsPause;
		mStreamInfo.timeoffset			= controlParam.u.pause.timeOffset;
		mStreamInfo.mask				= MASK_TIMEOFFSET | MASK_SCALE | MASK_STATE;
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(doPause,"pause ok for port [%u] and current timeoffset[%lld] scale[%f]"),
			portNum , mStreamInfo.timeoffset , mStreamInfo.scale);
		return ERR_RETURN_SUCCESS;
	}
}

int32 VstrmCommand::doResume(  )
{	
	ULONG vstrmPort		=	getVstrmPort( );

	IOCTL_CONTROL_PARMS_LONG controlParam;
	memset(&controlParam , 0 , sizeof(controlParam));

	ULONG portNum					= vstrmPort ; 
	controlParam.u.resume.timeCode	= TIME_CODE_NOW ;

	VSTATUS retStatus = VstrmClassControlPortEx1(	vstrmHandle(),
												portNum,
												VSTRM_GEN_CONTROL_RESUME,
												&controlParam,
												sizeof(IOCTL_CONTROL_PARMS_LONG),
												NULL,
												0 );
	if( retStatus != VSTRM_SUCCESS )
	{//失败咯		
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doResume,"failed to resume with vstrm port[%u] because [%s] API{VstrmClassControlPortEx1}"),
			vstrmPort ,
			mStreamerManager.getVstrmError( VstrmGetLastError() ).c_str() );
		return ERR_RETURN_SERVER_ERROR;
	}
	else
	{//成功鸟
		SPEED_IND speedRet = controlParam.u.resume.speedIndicator;
		if(speedRet.denominator == 0 )
		{
			mStreamInfo.scale		= 0.0f;
		}
		else
		{
			mStreamInfo.scale		= (float)controlParam.u.resume.speedIndicator.numerator / 
											(float)controlParam.u.resume.speedIndicator.denominator;
		}

		mStreamInfo.streamState		= TianShanIce::Streamer::stsStreaming;
		mStreamInfo.timeoffset		= controlParam.u.resume.timeOffset;
		mStreamInfo.mask			= MASK_TIMEOFFSET | MASK_SCALE | MASK_STATE;	
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(doResume,"resume ok for port [%u] and current timeoffset[%lld] scale[%f]"),
			portNum , mStreamInfo.timeoffset , mStreamInfo.scale);
		return ERR_RETURN_SUCCESS;
	}
}

int32 VstrmCommand::doChangeSpeed(  float& newSpeed , bool bGetInfo )
{
	if( fabs( newSpeed ) < 0.001f )
	{
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(doChangeSpeed,"newSpeed[%f] , return ok") , newSpeed );
		return ERR_RETURN_SUCCESS;
	}	

	IOCTL_CONTROL_PARMS_LONG controlParam;
	memset(&controlParam , 0 , sizeof(controlParam));

	controlParam.u.portSpeed.speedIndicator = convertSpeed( newSpeed );

	ULONG vstrmPort = getVstrmPort();

	VSTATUS retStatus = VstrmClassControlPortEx1(	vstrmHandle(),
													vstrmPort,
													VSTRM_GEN_CONTROL_PORT_SPEED,
													&controlParam,
													sizeof(IOCTL_CONTROL_PARMS_LONG),
													NULL,
													0);
	if( retStatus != VSTRM_SUCCESS )
	{//失败鸟		
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doChangeSpeed,"failed to change speed to[%f] on port[%d] because[%s] API{VstrmClassControlPortEx1}"),
			newSpeed , vstrmPort , mStreamerManager.getVstrmError( VstrmGetLastError() ).c_str() );
		return ERR_RETURN_SERVER_ERROR;
	}
	else
	{//成功了，假象?
		if( bGetInfo )
		{
			SPEED_IND speedRet = controlParam.u.portSpeed.speedIndicator;
			if( speedRet.denominator == 0 )
			{
				mStreamInfo.scale = 0.0f;
			}
			else
			{
				mStreamInfo.scale			= (float)controlParam.u.portSpeed.speedIndicator.numerator  / 
												(float)controlParam.u.portSpeed.speedIndicator.denominator;
			}
			mStreamInfo.streamState			= TianShanIce::Streamer::stsStreaming;
			mStreamInfo.timeoffset			= controlParam.u.portSpeed.timeOffset;			
			mStreamInfo.mask				= MASK_TIMEOFFSET | MASK_SCALE | MASK_STATE;
			ENVLOG(ZQ::common::Log::L_INFO,PLFMT(doChangeSpeed,"changeSpeed ok for port [%u] and current timeoffset[%lld] scale[%f]"),
				vstrmPort , mStreamInfo.timeoffset , mStreamInfo.scale);
		}		
		return ERR_RETURN_SUCCESS;
	}
}

ULONG VstrmCommand::getSpigotHandle( ULONG vstrmPort )
{
	return mStreamerManager.getSpigotHandle(vstrmPort);
}


// int32 VstrmCommand::checkSessions(	SessionRestoreInfos& sessInfos )
// {
// 	std::vector<ULONG64> ticketInDb;
// 
// 	SessionRestoreInfos retInfos;
// 	SessionRestoreInfos::const_iterator it = sessInfos.begin();
// 	ULONG returnSize = 0;
// 	
// 	ESESSION_CHARACTERISTICS vstrmSessInfo;
// 	for( ; it != sessInfos.end() ; it ++ )
// 	{		
// 		SessionRestoreInfo info = *it;
// 		//get bandwidth ticket from attributes
// 		std::vector<Ice::Long> tickets;
// 		bool bRange = false;  
// 		try
// 		{
// 			ZQTianShan::Util::getValueMapData( info.attributes , STREAMINGBANDWITHTICKETS , tickets, bRange );
// 		}
// 		catch( const TianShanIce::InvalidParameter&)
// 		{
// 			tickets.clear();
// 		}
// 		//ticketInDb += tickets;
// 		std::copy( tickets.begin(), tickets.end(), ticketInDb.end() );
// 
// 		if( !info.currentStreamId.empty() )
// 		{//check current session id
// 			ULONG sessId		= atol( info.currentStreamId.c_str() );
// 			VSTATUS retStatus	= VstrmClassGetSessionChars( vstrmHandle(),sessId, &vstrmSessInfo,sizeof(vstrmSessInfo),&returnSize);
// 			if( retStatus != VSTRM_SUCCESS)
// 			{
// 				info.currentStreamId = "";
// 			}
// 			else
// 			{
// 				if( vstrmSessInfo.SessionCharacteristics.Speed.denominator != 0 )
// 				{
// 					info.speed	=	((float)vstrmSessInfo.SessionCharacteristics.Speed.numerator)/((float)vstrmSessInfo.SessionCharacteristics.Speed.denominator);
// 				}
// 				else
// 				{
// 					info.speed = 0.0f;
// 				}
// 				if( vstrmSessInfo.SessionCharacteristics.State == PAUSED )
// 				{
// 					info.state = TianShanIce::Streamer::stsPause;
// 				}
// 				else
// 				{
// 					info.state = TianShanIce::Streamer::stsStreaming;
// 				}
// 
// 			}
// 		}
// 		if( !info.nextStreamId.empty() )
// 		{//check next session id
// 			ULONG sessId		= atol( info.nextStreamId.c_str() );
// 			VSTATUS retStatus	= VstrmClassGetSessionChars( vstrmHandle(),sessId, &vstrmSessInfo,sizeof(vstrmSessInfo),&returnSize);
// 			if( retStatus != VSTRM_SUCCESS)
// 			{
// 				info.nextStreamId = "";
// 			}
// 			else
// 			{
// 				if( vstrmSessInfo.SessionCharacteristics.Speed.denominator != 0 )
// 				{
// 					info.speed	=	((float)vstrmSessInfo.SessionCharacteristics.Speed.numerator)/((float)vstrmSessInfo.SessionCharacteristics.Speed.denominator);
// 				}
// 				else
// 				{
// 					info.speed = 0.0f;
// 				}
// 				if( vstrmSessInfo.SessionCharacteristics.State == PAUSED )
// 				{
// 					info.state = TianShanIce::Streamer::stsPause;
// 				}
// 				else
// 				{
// 					info.state = TianShanIce::Streamer::stsStreaming;
// 				}
// 			}
// 		}
// 		retInfos.push_back( info );
// 	}
// 
// 	sessInfos = retInfos;
// 
// 	//remove orphan bandwidth tickets
// 	std::vector<ULONG64> orphanTickets;
// 	std::vector<ULONG64> runningBwTickets;
// 	collectRunningBandWidthTickets(runningBwTickets);
// 	std::sort( runningBwTickets.begin() , runningBwTickets.end() );
// 	std::sort( ticketInDb.begin() , ticketInDb.end() );
// 
// 	std::set_difference( runningBwTickets.begin(), runningBwTickets.end(),
// 						ticketInDb.begin(), ticketInDb.end(),
// 						std::inserter< std::vector<ULONG64> >( orphanTickets , orphanTickets.begin() )	);
// 
// 	std::vector<ULONG64>::const_iterator itOrphan = orphanTickets.begin();
// 	for( ; itOrphan != orphanTickets.end(); it ++ )
// 	{
// 		VstrmClassReleaseBandwidth( vstrmHandle() , *itOrphan );
// 		ENVLOG(ZQ::common::Log::L_WARNING,PLFMT(checkSessions,"remove orphan bandwidth ticket[%llx]"), *itOrphan );
// 	}
// 
// 	return ERR_RETURN_SUCCESS;
// }

int32 VstrmCommand::doGetInfo(  ULONG sessId , ESESSION_CHARACTERISTICS& sessInfo )
{	
	ULONG returnSize = 0;

	VSTATUS retStatus = VstrmClassGetSessionChars( vstrmHandle(),sessId, &sessInfo,sizeof(sessInfo),&returnSize);
	if( retStatus != VSTRM_SUCCESS )
	{		
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doGetInfo,"failed to get information for session[%u] because [%s] API{VstrmClassGetSessionChars}"),
			sessId ,
			mStreamerManager.getVstrmError( VstrmGetLastError() ).c_str() );
		return ERR_RETURN_INVALID_PARAMATER;
	}
	else
	{
		return ERR_RETURN_SUCCESS;
	}
}
uint32 VstrmCommand::getCallBackId(  const std::string& contextKey )
{
	Ice::Int id = 0;	
	StreamSmithEnv* env = dynamic_cast<StreamSmithEnv*>( getSsEnv() );
	assert( env != NULL );
	return env->getCallBackManager().registerPlaylist(contextKey) ;	
}
std::string	VstrmCommand::getItemFullPathName( const TianShanIce::Streamer::PlaylistItemSetupInfo& info   )
{	
	std::string realFullPathName;
	std::string defaultName = info.contentName;
	ZQTianShan::Util::getValueMapDataWithDefault( info.privateData, KEY_ITEM_FULL_PATH_NAME , defaultName , realFullPathName );
	return realFullPathName;
}


int32 VstrmCommand::prepareItemRunTimeInfoFromVstrm( const TianShanIce::Streamer::PlaylistItemSetupInfo& info  , 
														int64& playDuration , 
														TianShanIce::Storage::ContentState& curState )
{
	std::string strItem = info.contentName;
	if( mbEdgeServer )
	{
		std::string::size_type pos = strItem.rfind('/');
		if( pos != std::string::npos )
		{
			strItem = strItem.substr( pos + 1 );
		}
	}

	using namespace ZQ::IdxParser;
	IndexData idxData;
	IndexFileParser parser( mEnv->getIdxParserEnv() );
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(prepareItemRunTimeInfoFromVstrm,"try to get asset information from Remote for [%s]"), strItem.c_str() );
	if( !parser.ParseIndexFileFromVstrm( strItem , idxData ) )
	{
		//mLastError = parser.getLastError();
		ENVLOG(ZQ::common::Log::L_ERROR, PLFMT(prepareItemRunTimeInfoFromVstrm,"can't get item information for [%s]") , strItem.c_str() );
		return ERR_RETURN_INVALID_PARAMATER;
	}
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(prepareItemRunTimeInfoFromVstrm,"now we get asset information from Remote for [%s]"), strItem.c_str() );

	playDuration	= static_cast<long>( idxData.getPlayTime() );
	curState	= TianShanIce::Storage::csProvisioning;
	return ERR_RETURN_SUCCESS;
}

int32 VstrmCommand::prepareItemRunTimeInfo( const TianShanIce::Streamer::PlaylistItemSetupInfo& info  , 
											int64& playDuration , 
											TianShanIce::Storage::ContentState& curState )
{	
	playDuration = 0;
	try
	{
		TianShanIce::Storage::ContentStorePrx csPrx = mEnv->getCsPrx();
		assert( csPrx != NULL );
		TianShanIce::Storage::UnivContentPrx ctntPrx = TianShanIce::Storage::UnivContentPrx::uncheckedCast(
			csPrx->openContentByFullname( info.contentName ) );
		if( !ctntPrx )
		{			
			return prepareItemRunTimeInfoFromVstrm( info , playDuration , curState );
		}
		TianShanIce::Storage::ContentState curState = ctntPrx->getState();
		if( mbEdgeServer && (TianShanIce::Storage::csInService != curState) )
		{
			return prepareItemRunTimeInfoFromVstrm( info ,playDuration , curState );
		}
		else
		{
			playDuration	= ctntPrx->getPlayTimeEx( );
			curState		= ctntPrx->getState();
		}
		return ERR_RETURN_SUCCESS;
	}
	catch( const Ice::ObjectNotExistException& )
	{
		return prepareItemRunTimeInfoFromVstrm( info , playDuration , curState );
	}
	catch( const TianShanIce::BaseException& ex)
	{		
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(prepareItemRunTimeInfo,"exception [%s] was caught when getting "
			"runtime information for item[%s]"),
			ex.message.c_str() ,
			info.contentName.c_str() );
		return ERR_RETURN_INVALID_PARAMATER;
	}
	catch( const Ice::Exception& ex )
	{		
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(prepareItemRunTimeInfo,"exception [%s] was caught when getting "
			"runtime information for item[%s]"),
			ex.ice_name().c_str() ,
			info.contentName.c_str() );
		return ERR_RETURN_INVALID_PARAMATER;
	}
	catch( ... )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(prepareItemRunTimeInfo,"unknown exception was caught when getting "
			"runtime information for item[%s]"),			
			info.contentName.c_str() );
		return ERR_RETURN_INVALID_PARAMATER;
	}
	return ERR_RETURN_SERVER_ERROR;	
}

int32 VstrmCommand::prepareItemStaticInfoFromVstrm( TianShanIce::Streamer::PlaylistItemSetupInfo& info )
{
	std::string strItemName = info.contentName;
	if( mbEdgeServer )
	{
		std::string::size_type pos = strItemName.rfind('/');
		if( pos != std::string::npos )
		{
			strItemName = strItemName.substr( pos + 1 );
		}
	}
	using namespace ZQ::IdxParser;
	IndexData idxData;
	IndexFileParser parser( mEnv->getIdxParserEnv() );
	


	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(prepareItemStaticInfoFromVstrm,"try to get asset information from Remote for [%s]"), strItemName.c_str() );
	if( !parser.ParseIndexFileFromVstrm( strItemName , idxData ) )
	{
		//mLastError = parser.getLastError();
		ENVLOG(ZQ::common::Log::L_ERROR, PLFMT(prepareItemStaticInfoFromVstrm,"can't get item information for [%s]") , strItemName.c_str() );
		return ERR_RETURN_INVALID_PARAMATER;
	}
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(prepareItemStaticInfoFromVstrm,"now we get asset information from Remote for [%s]"), strItemName.c_str() );
	std::string strFullName;
	if( idxData.getIndexType() == IndexData::INDEX_TYPE_VVX )
	{
		strFullName = "\\vod\\" + strItemName;
	}
	else if( idxData.getIndexType() == IndexData::INDEX_TYPE_VV2 )
	{
#if VER_PRODUCTVERSION_MAJOR >= 6 && VER_PRODUCTBUILD >= 9207
		strFullName = std::string("\\vv2\\") + strItemName;
#else
		strFullName = std::string("\\tsoverip\\") + strItemName;
#endif//
		
	}
	else if( idxData.getIndexType() == IndexData::INDEX_TYPE_VVC )
	{
		strFullName = std::string("\\vvc\\") + strItemName;
	}
	else
	{
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(prepareItemStaticInfoFromVstrm,"unkown item index type"));
		return ERR_RETURN_INVALID_PARAMATER;
	}
	ZQTianShan::Util::updateValueMapData( info.privateData , KEY_ITEM_FULL_PATH_NAME , strFullName );
	
	int32 muxBitRate = idxData.getMuxBitrate();

	ENVLOG(ZQ::common::Log::L_INFO,
		PLFMT(prepareItemStaticInfo,"get item[%s] 's static info fullPathName[%s] bitRate[%d]"),
		info.contentName.c_str() ,				
		strFullName.c_str() ,
		muxBitRate );

	return ERR_RETURN_SUCCESS;
}

int32 VstrmCommand::prepareItemStaticInfo( TianShanIce::Streamer::PlaylistItemSetupInfo& info )
{	
	try
	{
		if( !mbEdgeServer  && itemHasUrl(info) )
		{
			//if current service is not used as edge serer 
			// and has url
			//use hard code driver name
			std::string realFullPathName = info.contentName;
			std::string::size_type iPos = realFullPathName.find_last_of('/');
			if( iPos != std::string::npos )
			{
				realFullPathName = realFullPathName.substr(iPos);
			}
		
			realFullPathName = "\\vod\\" + realFullPathName;

			ENVLOG(ZQ::common::Log::L_INFO, PLFMT(prepareItemStaticInfo,"not edge server and item has url, use hard code driver ,[%s]") , realFullPathName.c_str() );			

			ZQTianShan::Util::updateValueMapData( info.privateData , KEY_ITEM_FULL_PATH_NAME , realFullPathName );

			return ERR_RETURN_SUCCESS;
		}

		///
		TianShanIce::Storage::ContentStorePrx csPrx = mEnv->getCsPrx();
		assert( csPrx != NULL );
		TianShanIce::Storage::UnivContentPrx ctntPrx = NULL;
		try
		{
			ctntPrx = TianShanIce::Storage::UnivContentPrx::uncheckedCast(
														csPrx->openContentByFullname( info.contentName ) );
			if( !ctntPrx )
			{			
// 				ENVLOG(ZQ::common::Log::L_INFO, 
// 					PLFMT(prepareItemStaticInfo,"failed to open content with name[%s] , try remote content" ),
// 					info.contentName.c_str() );
				return prepareItemStaticInfoFromVstrm( info );

			}
		}
		catch( const Ice::ObjectNotExistException& )
		{
			return prepareItemStaticInfoFromVstrm( info );
		}

		TianShanIce::Storage::ContentState curState = ctntPrx->getState();
		if( mbEdgeServer && (TianShanIce::Storage::csInService != curState) )
		{
			return prepareItemStaticInfoFromVstrm( info );
		}

		std::string realFullPathName	=	ctntPrx->getMainFilePathname();
		TianShanIce::Properties props	=	ctntPrx->getMetaData();
		//get mux bitrate
		TianShanIce::Properties::const_iterator itBitrate = props.find(METADATA_BitRate);
		if( itBitrate == props.end() )
		{			
			ENVLOG(ZQ::common::Log::L_ERROR, PLFMT(prepareItemStaticInfo,"can't get muxrate with item[%s]"),
				info.contentName.c_str() );
			return ERR_RETURN_INVALID_STATE_OF_ART;
		}

		Ice::Int	muxBitRate = atol( itBitrate->second.c_str() );


		TianShanIce::Properties::const_iterator itProp = props.find( METADATA_SubType );
		if(itProp == props.end() )
		{
			ENVLOG(ZQ::common::Log::L_ERROR,
				PLFMT(prepareItemStaticInfo()," can't get item[%s] sub type "),
				info.contentName.c_str() );
			return ERR_RETURN_INVALID_PARAMATER;
		}
		else
		{
			const std::string&	strType = itProp->second;//prx->getSubtype();

			if(strType == TianShanIce::Storage::subctVVX)
			{
				realFullPathName ="\\vod\\" + realFullPathName;
			}
			else if( strType == TianShanIce::Storage::subctVV2 )
			{
#if VER_PRODUCTVERSION_MAJOR >= 6 && VER_PRODUCTBUILD >= 9207
				realFullPathName = "\\vv2\\" + realFullPathName;
#else
				realFullPathName = "\\tsoverip\\" + realFullPathName;
#endif				
			}
			else if( stricmp( strType.c_str() , "vvc" ) == 0 )
			{
				realFullPathName =  "\\vvc\\" + realFullPathName;
			}
			else
			{
				ENVLOG(ZQ::common::Log::L_ERROR,
					PLFMT(prepareItemStaticInfo()," Not support type [%s] for item [%s]"),
					strType.c_str() , info.contentName.c_str());
				return ERR_RETURN_INVALID_PARAMATER;
			}

			itProp = props.find( METADATA_nPVRLeadCopy );
			if( itProp != props.end())
			{
				if(itProp->second.length() > 0 )
				{
					Ice::Int	fileFlag = 0;
					ZQTianShan::Util::getValueMapDataWithDefault( info.privateData ,KEY_CONTENT_STATE_RECORD,0,fileFlag );
					fileFlag |= STREAMSMITH_FILE_FLAG_NPVR;					
					ZQTianShan::Util::updateValueMapData( info.privateData ,KEY_CONTENT_VSTRM_FLAG , fileFlag );
					
#ifdef _NPVR_TEMP_NAME_SOULTION_
					std::string::size_type pos = realFullPathName.find("\\");
					if( pos != std::string::npos )
					{
						realFullPathName = realFullPathName.substr( pos + 1 );
						ENVLOG(ZQ::common::Log::L_INFO,
							PLFMT("prepareItemStaticInfo() temp solution for nPVR realName [%s]"),
							realFullPathName.c_str() );
					}
#endif _NPVR_TEMP_NAME_SOULTION_

					ENVLOG(ZQ::common::Log::L_INFO,
						PLFMT(prepareItemStaticInfo()," item[%s] is a npvr item"),
						realFullPathName.c_str() );
				}
			}
			
			ZQTianShan::Util::updateValueMapData( info.privateData , KEY_ITEM_FULL_PATH_NAME , realFullPathName );

			ENVLOG(ZQ::common::Log::L_INFO,
				PLFMT(prepareItemStaticInfo,"get item[%s] 's static info fullPathName[%s] bitRate[%d]"),
				info.contentName.c_str() ,				
				realFullPathName.c_str() ,
				muxBitRate );
		}
		return ERR_RETURN_SUCCESS;
	}
	catch( const TianShanIce::BaseException& ex)
	{		
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(prepareItemStaticInfo,"exception [%s] was caught when getting "
			"runtime information for item[%s]"),
			ex.message.c_str() ,
			info.contentName.c_str() );
		return ERR_RETURN_INVALID_PARAMATER;
	}
	catch( const Ice::Exception& ex )
	{		
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(prepareItemStaticInfo,"exception [%s] was caught when getting "
			"runtime information for item[%s]"),
			ex.ice_name().c_str() ,
			info.contentName.c_str() );
		return ERR_RETURN_INVALID_PARAMATER;
	}
	catch( ... )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(prepareItemStaticInfo,"unknown exception was caught when getting "
			"runtime information for item[%s]"),			
			info.contentName.c_str() );
		return ERR_RETURN_INVALID_PARAMATER;
	}
	return ERR_RETURN_SERVER_ERROR;
}

int32 VstrmCommand::doGetStreamAttr(	int32 mask ,
										ULONG sessId , 
										const TianShanIce::Streamer::PlaylistItemSetupInfo& info )
{
	int32 ret = ERR_RETURN_SUCCESS;
	if( mask & MASK_CONTENT_STATE ||
		mask & MASK_CONTENT_DURATION )
	{
		int64 playDuration = 0;
		TianShanIce::Storage::ContentState newState =  TianShanIce::Storage::csOutService;		
		
		ret = prepareItemRunTimeInfo( info , playDuration , newState );
		if( ret != ERR_RETURN_SUCCESS )
		{
			return ret;
		}
		mStreamInfo.mask |= MASK_CONTENT_STATE | MASK_CONTENT_DURATION;
		mStreamInfo.duration		= playDuration;
		mStreamInfo.contentState	= newState;
	}

	if( mask & MASK_SCALE ||
		mask & MASK_TIMEOFFSET ||
		mask & MASK_STATE )
	{
		 ESESSION_CHARACTERISTICS sessInfo;
		ret = doGetInfo( sessId , sessInfo );
		if( ret != ERR_RETURN_SUCCESS )
			return ret;
		mStreamInfo.mask	|= MASK_TIMEOFFSET | MASK_TIMEOFFSET ;
		SESSION_CHARACTERISTICS& sc = sessInfo.SessionCharacteristics;
		mStreamInfo.timeoffset	= sc.PlayoutTimeOffset;
		if( sc.Speed.denominator == 0 )
			mStreamInfo.scale		= 0.0f;
		else
			mStreamInfo.scale		= (float)sc.Speed.numerator/(float)sc.Speed.denominator;
		return ERR_RETURN_SUCCESS;
	}
	return ERR_RETURN_SUCCESS;
}


bool VstrmCommand::itemHasUrl(  const TianShanIce::Streamer::PlaylistItemSetupInfo& info  )
{
	TianShanIce::ValueMap::const_iterator it =	info.privateData.find( ITEM_LIBRARY_URL );
	if( it == info.privateData.end() )
		return false;
	else
		return it->second.strs.size() > 0 ;
}

VHANDLE	VstrmCommand::vstrmHandle( )
{
	return mStreamerManager.getVstrmHandle();
}

StreamParams VstrmCommand::getStreamInfo( ) const
{
	return mStreamInfo;
}

std::string	VstrmCommand::getProperty( const TianShanIce::Properties& props , const std::string& key )
{
	TianShanIce::Properties::const_iterator it = props.find( key );
	if( it == props.end() )
	{
		return "";
	}
	else
	{
		return it->second;
	}
}
int32  VstrmCommand::applyPokeHoleSession( const std::string& pokeHoleSessId )
{
	char szPokeHoleSession[20];
	memset(  szPokeHoleSession, 0 , sizeof(szPokeHoleSession) );
	ConvertStringIntoBinary( pokeHoleSessId , szPokeHoleSession );
	
	{//set Poke Hole sessionID here to Vstrm API
		ATTRIBUTE_ARRAY attrArry;
		memset(&attrArry,0,sizeof(attrArry));
		attrArry.attributeCount = 0;
		attrArry.setAllorFailAll = true;//What is SETALL or FAILALL
		ATTRIBUTE atr;

		memset(&atr,0,sizeof(atr));
		atr.attributeCode = VSTRM_ATTR_GEN_LISTEN_FOR_ID;
		atr.attributeValueLength = 1;
		atr.attributeValueP = (UCHAR*)"1";

		attrArry.attributeArray[ attrArry.attributeCount++ ] = atr;

		memset(&atr,0,sizeof(atr));
		atr.attributeCode = VSTRM_ATTR_GEN_SESSION_ID;
		atr.attributeValueLength = 10;
		atr.attributeValueP = (UCHAR*)szPokeHoleSession;
		attrArry.attributeArray[ attrArry.attributeCount++ ] = atr;

		VSTATUS	status =  VstrmClassSetPortAttributesEx( vstrmHandle() , getVstrmPort() , &attrArry);
		if( status != VSTRM_SUCCESS )
		{			
			char	szErrbuf[1024];
			ZeroMemory(szErrbuf,1024);
			ENVLOG (ZQ::common::Log::L_ERROR,
				PLFMT(applyPokeHoleSession,"Failed to apply PokeHole SessionId[%s] and error is :[%s] API{VstrmClassSetPortAttributesEx}"),
				HelperClass::dumpBinary(pokeHoleSessId).c_str(),
				mStreamerManager.getVstrmError( VstrmGetLastError() ).c_str());
			return ERR_RETURN_SERVER_ERROR;
		}
		else
		{
			mStreamerManager.getRSMonitor().registerSession( pokeHoleSessId );
			ENVLOG(ZQ::common::Log::L_INFO,PLFMT(applyPokeHoleSession,"apply PokeHole SessionId[%s] ok"),
				HelperClass::dumpBinary(pokeHoleSessId).c_str() );
		}
	}
	return ERR_RETURN_SUCCESS;
}

int32 VstrmCommand::doCommit(	IN const PlaylistItemSetupInfos& items  ,
								IN const TianShanIce::SRM::ResourceMap& requestResource )
{
	///check if poke hole session is available or not
	std::string strPokeHoleSessionId;
	ZQTianShan::Util::getResourceDataWithDefault( requestResource , TianShanIce::SRM::rtEthernetInterface , "PokeSessionID", "", strPokeHoleSessionId );
	if( !strPokeHoleSessionId.empty() )
	{//do have poke hole session id
		int32 ret = applyPokeHoleSession( strPokeHoleSessionId );
		if( ret != ERR_RETURN_SUCCESS )
		{
			return ret;
		}
		else
		{
			strPokeHoleSessionId = mSsCtx.getContextProperty( VSTRMPROP_POKE_SESSION );//retrieve strPokeHolsession Id
		}
	}

	//reserve bandwidth tickets
	int32 ret = reserveBandwidthTickets( items );
	if( ret != ERR_RETURN_SUCCESS )
		return ret;

	///get source IP / PORT
	std::string strStreamerId = mSsCtx.getContextProperty( STREAMINGRESOURCE_STREAMERID_KEY );
	std::string strStreamPort = mSsCtx.getStreamingPort();
	
	std::string		sourceIpv4;
	std::string		sourceIpv6;
	int32			udpport = 0;
	if( !mStreamerManager.getStreamingSourceAddress( strStreamerId , strStreamPort , sourceIpv4, sourceIpv6 , udpport ) )
	{
		ENVLOG(ZQ::common::Log::L_ERROR, PLFMT(doCommit,"failed to get source IP/port with spigot[%s] streamPort[%s]"),
			strStreamerId.c_str(), strStreamPort.c_str() );
		return ERR_RETURN_INVALID_PARAMATER;
	}
	if ( sourceIpv4.empty() )
	{
		mSsCtx.updateContextProperty(STREAMINGRESOURCE_STREAMING_SOUCREIP , sourceIpv6 );
	}
	else
	{
		mSsCtx.updateContextProperty( STREAMINGRESOURCE_STREAMING_SOUCREIP , sourceIpv4 );
	}
	mSsCtx.updateContextProperty( STREAMINGRESOURCE_STREAMING_SOURCEPORT ,(Ice::Int)udpport);

	return ERR_RETURN_SUCCESS;
}

void VstrmCommand::doClearResource( )
{
	mEnv->getCallBackManager().unregisterPlaylist( mContextKey );
	releaseBandwidthTickets( );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//VsrtmSessCallbackReqeust

VsrtmSessCallbackReqeust::VsrtmSessCallbackReqeust( ZQ::common::NativeThreadPool& pool ,
												   SsEnvironment* environment,
												   SsServiceImpl&	service,
													uint32 id ,
													ULONG sessId,
													const std::string& strResult )
:ZQ::common::ThreadRequest(pool),
mCallbackId(id),
mStrResult(strResult),
ss(service)
{
	mVstrmSessionId = sessId;
	env		= environment;
	mEnv	= dynamic_cast<StreamSmithEnv*>(env);
	assert( mEnv != NULL );
	mCreateTime = ZQTianShan::now();
}


VsrtmSessCallbackReqeust::~VsrtmSessCallbackReqeust( )
{
}

bool VsrtmSessCallbackReqeust::init( )
{
	return true;
}
void VsrtmSessCallbackReqeust::final(int retcode , bool bCancelled )
{
	delete this;
}

int VsrtmSessCallbackReqeust::run( void )
{
	 VstrmSessionCallbackManager&   cbManager = mEnv->getCallBackManager();
	 std::string id = cbManager.getPlId( mCallbackId );
	 cbManager.unregisterPlaylist(mCallbackId);
	 if( id.empty() )
	 {
// 		 ENVLOG(ZQ::common::Log::L_ERROR,
// 			 CLOGFMT(VsrtmSessCallbackReqeust,"can't find callback Id [%u]"),
// 			mCallbackId	);
		 return -1;
	 }
	 try
	 {
		 ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(VsrtmSessCallbackReqeust,
			 "session call back created time[%lld] and now[%lld] with session[%u]"),
			 mCreateTime , ZQTianShan::now() , mVstrmSessionId	);		 
		 SsServiceImpl::StreamEvent e = SsServiceImpl::seGone;		 
		 StreamParams paras;
		 TianShanIce::Properties props;
		 std::ostringstream oss;
		 oss<<mVstrmSessionId;
		 ss.OnStreamEvent( e , oss.str() , paras , props );
	 }
	 catch( const TianShanIce::BaseException& ex )
	 {
		 ENVLOG(ZQ::common::Log::L_ERROR,
			 CLOGFMT(VsrtmSessCallbackReqeust,
			 "caught [%s] exception when executing vstrm session callback with id[%u]"),
			 ex.message.c_str(),
			 mCallbackId);
		 return -1;
	 }
	 catch( const Ice::Exception& ex )
	 {
		 ENVLOG(ZQ::common::Log::L_ERROR,
			 CLOGFMT(VsrtmSessCallbackReqeust,
			 "caught [%s] exception when executing vstrm session callback with id[%u]"),
			 ex.ice_name().c_str(),
			 mCallbackId );
		 return -1;
	 }
	 catch(...)
	 {
		 ENVLOG(ZQ::common::Log::L_ERROR,
			 CLOGFMT(VsrtmSessCallbackReqeust,"caught unknown exception when executing vstrm session callback with id[%u]"),
			 mCallbackId);
		 return -1;
	 }
	 return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//

bool	SsServiceImpl::listAllReplicas( SsServiceImpl& ss, OUT SsReplicaInfoS& infos )
{
	StreamSmithEnv* vstrmEnv = dynamic_cast<StreamSmithEnv*>( getSsEnv() );
	VstrmStreamerManager& streamerManager = vstrmEnv->getStreamerManager();
	return  streamerManager.listAllReplicas(infos);
}

bool SsServiceImpl::allocateStreamResource(	SsServiceImpl& ss, 
									   IN const std::string& streamerId ,
									   IN const std::string& portId ,
									   IN const TianShanIce::SRM::ResourceMap&	resource )
{
	StreamSmithEnv* vstrmEnv = dynamic_cast<StreamSmithEnv*>( getSsEnv() );
	VstrmStreamerManager& streamerManager = vstrmEnv->getStreamerManager();
	return  streamerManager.allocateStreamPort( streamerId , portId , resource );
}


bool SsServiceImpl::releaseStreamResource( SsServiceImpl& ss, SsContext& ctx, IN const std::string& streamerReplicaId  )
{
	StreamSmithEnv* vstrmEnv = dynamic_cast<StreamSmithEnv*>( getSsEnv() );
	VstrmStreamerManager& streamerManager = vstrmEnv->getStreamerManager();
	streamerManager.releaseStreamPort( streamerReplicaId , ctx.getStreamingPort() );

	VstrmCommand cmd( getSsEnv() , vstrmEnv->getStreamerManager() ,vstrmEnv->getSessionScaner() , ss , ctx );
	cmd.doClearResource( );
	return true;
}

int32	SsServiceImpl::doValidateItem(  SsServiceImpl& ss, 
										  IN SsContext& ctx,
										  INOUT TianShanIce::Streamer::PlaylistItemSetupInfo& info )
{
	StreamSmithEnv* vstrmEnv = dynamic_cast<StreamSmithEnv*>( getSsEnv() );
	VstrmCommand cmd( vstrmEnv , vstrmEnv->getStreamerManager() , vstrmEnv->getSessionScaner() , ss , ctx );
	return cmd.prepareItemStaticInfo( info );
}

int32 SsServiceImpl::doCommit(  SsServiceImpl& ss, 
							  IN SsContext& ctx,								
							  IN const PlaylistItemSetupInfos& items,
							  IN const TianShanIce::SRM::ResourceMap& requestResources )
{
	StreamSmithEnv* vstrmEnv = dynamic_cast<StreamSmithEnv*>( getSsEnv() );
	VstrmCommand cmd( getSsEnv() , vstrmEnv->getStreamerManager() ,vstrmEnv->getSessionScaner() , ss , ctx );
	
	return cmd.doCommit( items ,requestResources );

}


int32 SsServiceImpl::doLoad(	SsServiceImpl& ss,
								IN SsContext& contextKey,								
								IN const TianShanIce::Streamer::PlaylistItemSetupInfo& itemInfo, 
								IN int64 timeoffset,
								IN float scale,								
								INOUT StreamParams& ctrlParams,
								OUT std::string&	streamId )
{
	
	StreamSmithEnv* vstrmEnv = dynamic_cast<StreamSmithEnv*>( getSsEnv() );
	VstrmCommand cmd( getSsEnv() , vstrmEnv->getStreamerManager() ,vstrmEnv->getSessionScaner() , ss , contextKey );
	ULONG sessId = 0;
	int32 ret = cmd.doLoad( itemInfo , scale , timeoffset , sessId );
	if( ret == ERR_RETURN_SUCCESS )
	{
		std::ostringstream oss;
		oss<<sessId;
		streamId = oss.str();
		ctrlParams = cmd.getStreamInfo();
	}
	return ret;
}

int32 SsServiceImpl::doPlay(	SsServiceImpl& ss ,
								SsContext& contextKey,							   
							   IN const std::string& streamId,
							   IN int64 timeOffset,
							   IN float scale,							   
							   INOUT StreamParams& ctrlParams )
{	
	return ERR_RETURN_SUCCESS;
}


int32 SsServiceImpl::doPause(	SsServiceImpl& ss , SsContext& contextKey, IN const std::string& streamId , INOUT StreamParams& ctrlParams )
{
	StreamSmithEnv* vstrmEnv = dynamic_cast<StreamSmithEnv*>( getSsEnv() );
	VstrmCommand cmd( getSsEnv() , vstrmEnv->getStreamerManager() ,vstrmEnv->getSessionScaner() , ss , contextKey );
	ULONG sessId = 0;
	int32 ret = cmd.doPause();
	if( ret == ERR_RETURN_SUCCESS )
	{
		ctrlParams = cmd.getStreamInfo();
	}
	return ret;
}


int32 SsServiceImpl::doResume(  SsServiceImpl& ss , SsContext& contextKey , IN const std::string& streamId ,INOUT StreamParams& ctrlParams )
{
	StreamSmithEnv* vstrmEnv = dynamic_cast<StreamSmithEnv*>( getSsEnv() );
	VstrmCommand cmd( getSsEnv() , vstrmEnv->getStreamerManager() ,vstrmEnv->getSessionScaner() , ss , contextKey );
	ULONG sessId = 0;
	int32 ret = cmd.doResume();
	if( ret == ERR_RETURN_SUCCESS )
	{
		ctrlParams = cmd.getStreamInfo();
	}
	return ret;
}

int32 SsServiceImpl::doReposition( SsServiceImpl& ss , SsContext& contextKey,IN const std::string& streamId ,
									IN int64 timeOffset, IN const float& scale, INOUT StreamParams& ctrlParams )
{
	StreamSmithEnv* vstrmEnv = dynamic_cast<StreamSmithEnv*>( getSsEnv() );
	VstrmCommand cmd( getSsEnv() , vstrmEnv->getStreamerManager() ,vstrmEnv->getSessionScaner() , ss , contextKey );
	ULONG sessId = atol(streamId.c_str());
	
	int32 ret = cmd.doReposition( sessId , scale , timeOffset );

	if( ret == ERR_RETURN_SUCCESS )
	{
		ctrlParams = cmd.getStreamInfo();
	}
	return ret;
}

int32 SsServiceImpl::doChangeScale(	 SsServiceImpl& ss , SsContext& contextKey, IN const std::string& streamId ,								
								  IN float newScale, INOUT StreamParams& ctrlParams )
{
	StreamSmithEnv* vstrmEnv = dynamic_cast<StreamSmithEnv*>( getSsEnv() );
	VstrmCommand cmd( getSsEnv() , vstrmEnv->getStreamerManager() ,vstrmEnv->getSessionScaner() , ss , contextKey );
	ULONG sessId = atol( streamId.c_str() );
	int32 ret = cmd.doChangeSpeed( newScale , true );
	if( ret == ERR_RETURN_SUCCESS )
	{
		ctrlParams = cmd.getStreamInfo();
	}
	return ret;
}

int32 SsServiceImpl::doDestroy(	 SsServiceImpl& ss , SsContext&  contextKey ,IN const std::string& streamId )
{
	StreamSmithEnv* vstrmEnv = dynamic_cast<StreamSmithEnv*>( getSsEnv() );
	VstrmCommand cmd( getSsEnv() , vstrmEnv->getStreamerManager() ,vstrmEnv->getSessionScaner() , ss , contextKey );

	ULONG sessId  = 0 ;
	sscanf(streamId.c_str(),"%lu",&sessId);	

	int32 ret = cmd.doUnload( sessId);
	return ret;
}

int32 SsServiceImpl::doGetStreamAttr(	 SsServiceImpl& ss , SsContext& contextKey, IN	const std::string& streamId,
										IN  const TianShanIce::Streamer::PlaylistItemSetupInfo& info, OUT StreamParams& ctrlParams )
{

	StreamSmithEnv* vstrmEnv = dynamic_cast<StreamSmithEnv*>( getSsEnv() );
	VstrmCommand cmd( getSsEnv() , vstrmEnv->getStreamerManager() ,vstrmEnv->getSessionScaner() , ss , contextKey );
	
	ULONG sessId  = 0 ;
	sscanf(streamId.c_str(),"%lu",&sessId);	

	int32 ret = cmd.doGetStreamAttr( ctrlParams.mask , sessId , info );
	
	ctrlParams = cmd.getStreamInfo();
	
	return ret;
}

// SessionRestoreInfos	SsServiceImpl::checkSessions(		SsServiceImpl& ss , IN const SessionRestoreInfos& infos )
// {
// 
// 	SessionRestoreInfos retInfos = infos;
// 	StreamSmithEnv* vstrmEnv = dynamic_cast<StreamSmithEnv*>( getSsEnv() );
// 	VstrmCommand cmd( getSsEnv() , vstrmEnv->getStreamerManager() ,vstrmEnv->getSessionScaner() , ss , "", "" );
// 
// 	int32 ret = cmd.checkSessions( retInfos  );
// 	
// 	if( ret != ERR_RETURN_SUCCESS )
// 		retInfos.clear();
// 
// 	return retInfos;
// }

// void SsServiceImpl::doClearResource( SsServiceImpl& ss ,
// 									IN const std::string& contextKey,
// 									IN TianShanIce::ValueMap& playlistParams )
// {
// 	StreamSmithEnv* vstrmEnv = dynamic_cast<StreamSmithEnv*>( getSsEnv() );
// 	VstrmCommand cmd( getSsEnv() , vstrmEnv->getStreamerManager() ,vstrmEnv->getSessionScaner() , ss , "", contextKey );
// 
// 	cmd.doClearResource( playlistParams )	;
// }


}}
