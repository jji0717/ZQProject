
#include <ZQ_common_conf.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include "Playlist.h"
#include "StreamSmithSite.h"
#include <stdarg.h>
#include <timeutil.h>
#include <Ws2tcpip.h>
#include <guid.h>

#include "VVXParser\VstrmProc.h"
#include "vvxParser\VvxParser.h"
#include <math.h>
#include "HelperClass.h"
#include <TianShanIceHelper.h>

#ifndef _RTSP_PROXY
	#include <StreamSmithConfig.h>
#endif

extern "C"
{
#include <vstrmuser.h>
};


#ifdef _DEBUG
	#include "adebugmem.h"
#endif

#define VSTRMAPICALLSTART(x) ZQTianShan::Util::TimeSpan __localSpan##x;__localSpan##x.start();
#define VSTRMAPICALLEND(x) if(__localSpan##x.stop() > 200 ){ glog(ZQ::common::Log::L_INFO,PLSESSID(x,"vstrm api "#x" time cost [%lld]"),__localSpan##x.span() ); }


#define NEW_SETSPEEDEX_LOGIC	1

#define _CHECK_ITEM_TYPE_USE_NEWLOGIC

extern "C"
{
#include <time.h>
}
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

//#define		PLSESSID(x,y) 	"[Playlist] SESSID(%s) ["#x"]\t"##y,_strGuid.c_str()
#define		PLSESSID(x,y) 	"[Playlist] Stream[%s]UserSess[%s]Thread[%10lu][%16s]\t"##y,_strGuid.c_str(),m_strClientSessionID.c_str(),GetCurrentThreadId(),#x

namespace ZQ{
namespace StreamSmith {



// 5 sec for minimal preload, don't change, it was from test result
#define MIN_PRELOAD_TIME             (5000)
#define	CHANGE_DIRECTION_LIMITED		2
#define	REUPDATE_TIMER_TIME				200
#define	FAKE_TIMER_TIMECOUNT		((timeout64_t)-1)
// -----------------------------
// class Playlist
// -----------------------------


#define ABSOLUTEX(x) ( x > 0 ? x : -x )

const SPEED_IND	Playlist::SPEED_NORMAL = {  1, 1 };
const SPEED_IND	Playlist::SPEED_FF = {  2, 1 };
const SPEED_IND	Playlist::SPEED_REW = { -2, 1 };

inline bool isFFScale( float scale )
{
	return scale > 1.0001f;
}
inline bool isREWScale( float scale )
{
	return scale < -0.0001f;
}

inline bool isFFScale( SPEED_IND scale )
{
	return (scale.numerator > scale.denominator) && (scale.numerator > 0.01);

}
inline bool isREWScale( SPEED_IND scale )
{
	return scale.numerator < -0.0001;
}
//inline bool isDeletable( int64 flag )
//{
//	return flag & PLISFlagOnce;
//}

inline bool IsItemPlaytimesCountZero( Playlist::iterator& item )
{
	return ( item->_itemPlayTimesCount == 0 );
}


void addExtraPropertiesToVariant( StreamControlResultInfo * info , ZQ::common::Variant& var , const std::string& prefix ="echo")
{
	if(!info)	return;

	const std::map<std::string,std::string>& extraProps= info->extraProperties;
	std::map< std::string , std::string >::const_iterator it = extraProps.begin();
	for( ; it != extraProps.end() ; it ++ )
	{
		if( it->first.find(prefix) == 0 )
			var.set( it->first.c_str() , it->second );
	}
}
unsigned long	Playlist::_cfgIdleToExpire = 1000000*1000;
void Playlist::ERRLOG(int level,char* fmt,...)
{
	char msg[1024];
	va_list args;	
	ZeroMemory(msg,sizeof(msg));
	va_start(args, fmt);	
	::_vsnprintf(msg,sizeof(msg),fmt, args);
	va_end(args);
	glog(level,msg);
	_lastError=msg;
}
void convertSpeed( int& numerator , int& denominator )
{
	int a = numerator > 0 ? numerator : -numerator;
	int b = denominator;

	if ( a < b ){int c = a;	a = b ;	b = c;}
	int temp = 0;
	while( b != 0 )
	{
		temp = a % b;
		a = b;
		b = temp;
	}
	if ( a != 0 )
	{
		numerator /= a;
		denominator /= a;
	}
}



std::string printfUTCTime(time_t t)
{
	if( t == 0 )
		return std::string("");

	char UTCTimeBuffer[256];
	ZeroMemory(UTCTimeBuffer,sizeof(UTCTimeBuffer));
	tm* tLocal = localtime (&t) ;
	strftime(UTCTimeBuffer,sizeof(UTCTimeBuffer)-1,"%Y:%m:%d-%H:%M:%S",tLocal);
	return std::string(UTCTimeBuffer);
}
inline float convertSPEEDtoFloat( const SPEED_IND& speed )
{
	if( speed.denominator == 0 )
	{
		return 0.0f;
	}
	else
	{
		return (float)speed.numerator / (float)speed.denominator;

	}
}
class vstrmCallBackRequest : public ZQ::common::ThreadRequest
{
public :
	vstrmCallBackRequest( ZQ::common::NativeThreadPool& pool ,
							long		plId,
							SESSION_ID sessId,							
							VSTATUS status)
		:ZQ::common::ThreadRequest(pool)
	{	
		m_strClientSessionID = "";
		_plId = plId;
		_sessId = sessId;
		_strGuid = "";		
		_status = status;
	}
	~vstrmCallBackRequest( )
	{

	}
public:
	int run( )
	{
		Playlist* _pl = NULL;
		try
		{
			_pl = ZQ::StreamSmith::StreamSmithSite::m_pPlayListManager->plExist( _plId );
		}
		catch(...)
		{
			return -1;
		}
		if( !_pl )
			return -1;
		try
		{			
			if(! ( IS_VSTRM_SUCCESS( _status ) ) )
			{//term abnormal
				char szBuf[1024]={0};
				ZeroMemory(szBuf,sizeof(szBuf));
				//#pragma message(__MSGLOC__"Fire Session abnormal expired event")
// 				glog(Log::L_ERROR,PLSESSID(OnItemCompleted,"session %u was terminate by system and error description is %s"),
// 						_sessId , 
// 						ZQ::StreamSmith::StreamSmithSite::m_pPlayListManager->getErrorText( _status,szBuf,sizeof(szBuf)-1) );	
				
				char szLogMessage[2048] = {0};
				snprintf(szLogMessage,sizeof(szLogMessage)-1,"VstrmSess[%lu] terminated with error[%s/%ld]",
																_sessId,
																ZQ::StreamSmith::StreamSmithSite::m_pPlayListManager->getErrorText( _status,szBuf,sizeof(szBuf)-1) ,
																_status );				
				glog(Log::L_ERROR,"[Playlist] SESSID[%s]CLIENTID[%s]Thread[%10lu][%16s]\t%s",
								_pl->_strGuid.c_str() , _pl->getUserCtxIdx(),GetCurrentThreadId(),"OnItemCompleted",								
								szLogMessage);	
				try
				{
					_pl->OnItemAbnormal( _sessId  );
				}
				catch (...)
				{
				}
				try
				{				
					glog( ZQ::common::Log::L_INFO , PLSESSID( OnItemCompleted , "session[%lu] done, calling VstrmSessExpired") , _sessId );
					_pl->OnVstrmSessExpired( _sessId , glog , ZQTianShan::now() , szLogMessage , _status );
				}
				catch (...) 
				{
				}	
			}
			else
			{
				char szBuf[512]={0};
				char szLogMessage[2048] = {0};
				snprintf(szLogMessage,sizeof(szLogMessage)-1,"VstrmSess[%lu] terminated by vstrm with[%s/%ld]",
					_sessId,
					ZQ::StreamSmith::StreamSmithSite::m_pPlayListManager->getErrorText( _status,szBuf,sizeof(szBuf)-1),
					_status );

				glog( Log::L_INFO, "[Playlist] SESSID[%s]CLIENTID[%s]Thread[%10lu][%16s]\t%s",
						_pl->_strGuid.c_str() , _pl->getUserCtxIdx(),GetCurrentThreadId(),"OnItemCompleted",
						szLogMessage);
				try
				{
					_pl->OnVstrmSessExpired( _sessId , glog , ZQTianShan::now() );	
				}
				catch(...){}
			}					
		}
		catch(...)
		{
		}
		return 1;
	}

protected:

	virtual void final(int retcode/* = 0*/ , bool bCancelled /* = false*/ )
	{
		delete this;
	}

private:
	long			_plId;
	SESSION_ID		_sessId;
	std::string		_strGuid;
	std::string		m_strClientSessionID;
	VSTATUS			_status;

};
VSTATUS Playlist::cbItemCompleted(HANDLE classHandle, PVOID cbParam, PVOID bufP, ULONG bufLen)
{

	//glog(ZQ::common::Log::L_DEBUG, LOGFMT("Playlist::cbItemCompleted"));
	long plId = reinterpret_cast<long>(cbParam);

	PIOCTL_STATUS_BUFFER pStatusBlk = (PIOCTL_STATUS_BUFFER) bufP;	
		
	
	if(pStatusBlk->status==ERROR_SUCCESS /*&& pStatusBLK->status!=536870912 */)
	{
		pStatusBlk->status=VSTRM_SUCCESS;				
	}
	try
	{
		vstrmCallBackRequest* request = new vstrmCallBackRequest(	ZQ::StreamSmith::StreamSmithSite::m_pPlayListManager->_pool
																	, plId, pStatusBlk->sessionId , pStatusBlk->status );
		request->start();
	}
	catch(...)
	{
		glog(Log::L_ERROR,"unexpected error in vstrm callback handling");
	}

	return VSTRM_SUCCESS;
}

VSTATUS Playlist::OnItemCompleted(PIOCTL_STATUS_BUFFER pStatusBlk, ULONG BlkLen)
{	
	ZQ::common::MutexGuard gd(_listOpLocker);
	return VSTRM_SUCCESS;
}

#ifndef kVSTRM_BANDWIDTH_CLIENTID_ZQ_STREAMSMITH 
	#define kVSTRM_BANDWIDTH_CLIENTID_ZQ_STREAMSMITH 0x04000000
#endif

bool	Playlist::commit( )
{
	_lastExtErrCode = EXT_ERRCODE_BANDWIDTH_EXCEEDED;
	if( !_bCommitted )
	{//mVstrmBwTcikets
		_bCommitted	= true;
		//reserve vstrm bandwidth
		//step 1 , get mux bitrate
		ULONG muxRate = _dvbAttrs.NowMuxRate;
		if( muxRate == 0 )
		{
			glog(ZQ::common::Log::L_ERROR,PLSESSID(commit,"no muxRate has been specified, skip reserving bandwidth ticket"));
		}
		else
		{
			glog(ZQ::common::Log::L_INFO, PLSESSID(commit, "reserving bandwidth ticket pair with bitrate[%lu]"), muxRate);

			ULONG spigotId = _mgr._cls.getSpigotIdFromPortId(_vstrmPortNum);
			glog(ZQ::common::Log::L_INFO,PLSESSID(commit,"get spigot Id[%lu] from port[%lu]"),
				spigotId , _vstrmPortNum );
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
				brb.BwTarget		= (PVOID64)(SC_UINT_PA)spigotId; 
				brb.MaxBandwidth	= muxRate; // Need to already know the bitrate 
				brb.MinBandwidth	= 0; 
				
				VSTRMAPICALLSTART(VstrmClassReserveBandwidth);
				bwStatus = VstrmClassReserveBandwidth( _mgr.classHandle(), &brb, &mVstrmBwTcikets.EdgeTicket); 
				VSTRMAPICALLEND(VstrmClassReserveBandwidth);

				if (bwStatus!=VSTRM_SUCCESS) 
				{
					char szErrbuf[1024];
					ERRLOG(ZQ::common::Log::L_ERROR, PLSESSID(commit,"VstrmClassReserveBandwidth(TARGETTYPE_SLICE) failed with bitrate[%lu] error[%s]"),
						muxRate ,
						_mgr.getErrorText(VstrmGetLastError(),szErrbuf,1023) );
					_lastErrCode = ERR_PLAYLIST_INVALID_PARA;
					return false;
				}
				else
				{
					glog(ZQ::common::Log::L_INFO, PLSESSID(commit,"VstrmClassReserveBandwidth(TARGETTYPE_SLICE), got slice ticket [%llx]"),mVstrmBwTcikets.EdgeTicket);
				}
			} 
			else 
			{
				_lastErrCode = ERR_PLAYLIST_SERVER_ERROR;
				ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(commit, "invalid spigot handle"));
				return false;
			}

			bool bEgdeServer = gStreamSmithConfig.serverMode == 2;
			
			if( bEgdeServer )
			{
				// Now allocate the file bandwidth, passing in the edge card's ticket... 
				if ( bwStatus == VSTRM_SUCCESS )
				{
					memset(&brb, 0, sizeof(brb)); 
					brb.Type				= kVSTRM_BANDWIDTH_TYPE_WRITE; 
					brb.TargetType			= kVSTRM_BANDWIDTH_TARGETTYPE_FILE; 
					brb.Flags				= kVSTRM_BRB_FLAG_REL_AFFIL_UPONREL; 

					brb.ClientId			= clientId; 
					brb.TicketAffiliate		= mVstrmBwTcikets.EdgeTicket;
					brb.BwTarget			= reinterpret_cast<void*>((char*)"dummy"); 
					brb.MaxBandwidth		= muxRate; 
					brb.MinBandwidth		= 0; 
					VSTRMAPICALLSTART(VstrmClassReserveBandwidth);
					bwStatus = VstrmClassReserveBandwidth( _mgr.classHandle(), &brb, &mVstrmBwTcikets.FileTicket); 
					VSTRMAPICALLEND(VstrmClassReserveBandwidth);
					if (bwStatus!=VSTRM_SUCCESS) 
					{
						char szErrbuf[1024];
						ERRLOG(ZQ::common::Log::L_ERROR, PLSESSID(commit,"VstrmClassReserveBandwidth(TARGETTYPE_FILE) failed with bitrate[%lu] error[%s]"),
							muxRate , 
							_mgr.getErrorText(VstrmGetLastError(),szErrbuf,1023) );
						VstrmClassReleaseBandwidth( _mgr.classHandle(), mVstrmBwTcikets.EdgeTicket);
						_lastErrCode = ERR_PLAYLIST_INVALID_PARA;
						return false;
					} 
				}
			}
			glog(ZQ::common::Log::L_INFO, PLSESSID(commit,"reserved bandwidth[%lu], get ticket edge[%llx] file[%llx]"),
				muxRate,
				mVstrmBwTcikets.EdgeTicket,
				mVstrmBwTcikets.FileTicket);

		}
	}
	_mgr.UpdatePlaylistFailStore(this,true,false);
	return true;
}

Playlist::Playlist( StreamSmithSite* pSite,PlaylistManager& mgr, const ZQ::common::Guid& guid,
				   bool bFailOver,const std::vector<int>& boardIDs , const std::string& userSessId  )
: _mgr(mgr), _itCurrent(listStart()), _itNext(listStart()), // _itNextCriticalStart(begin()),
_b1stPlayStarted(false), _bLongFilename(false),
_vstrmPortNum(0), _mastSessId(0), VstrmSessSink(mgr._sessmon),
_stampDone(0), _guid(guid),_stampSpeedChangeReq(0),_stampStateChangeReq(0),
_lastExtErrCode(0),
_lastItemStepErrorCode(0),
_lastplaylistNpt(0),
_lastplaylistNptPrimary(0),
_lastItemNpt(0),
m_strClientSessionID(userSessId)
{
	_lastItemStepErrorCode = 0;
	memset(&mVstrmBwTcikets,0,sizeof(mVstrmBwTcikets));
	_bCommitted	=	false;
	//initialize the event sequence
	_eventCSeqBegginingOfStream = -1 ;
	_eventCSeqEndofStream = -1 ;
	_eventCSeqSpeedChanged = -1 ;
	_eventCSeqStateChanged = -1 ;
	_eventCSeqItemStepped = -1 ;
	_eventCSeqPlaylistExit = -1;
	_eventCseqNewSessDetected = -1;
	_eventCSeqProgress = -1;
	_eventCSeqSessExpired = -1;

	setStreamPID(-1);

	_pokeHoleSessID = "";
	_bEnableEOT = true;
	_isGoodItemInfoData = true;
	_iExitCode=0;
	_bDontPrimeNext=false;
	m_iErrInfoCount=0;
	_isReverseStreaming=false;
#ifdef _ICE_INTERFACE_SUPPORT
	m_bCurrentSessionIsONLine=false;
	
#endif
	_bCleared=false;
	m_pStreamSite=pSite;
	endTimer64();

	memset(&_dvbAttrs, 0x00, sizeof(_dvbAttrs));
	memset(&_ioctrlparms, 0x00, sizeof(_ioctrlparms));
	memset(&_iostatus_u, 0x00, sizeof(_iostatus_u));

	_crntSpeed = SPEED_NORMAL;
	
	char	szBuf[128];
	_plExistId = _mgr.reg(*this);
	ZeroMemory(szBuf,sizeof(szBuf));
	_guid.toString(szBuf,sizeof(szBuf)-1);
	_strGuid=szBuf;
	
	_vstrmSessioIDForFailOver=0;
	if(!bFailOver)
	{
		if(boardIDs.size()<=0)
			_vstrmPortNum = _mgr.GetUnUsePort(-1);
		else
		{
			for(int i=0;i<(int)boardIDs.size();i++)
			{
				_vstrmPortNum=_mgr.GetUnUsePort(boardIDs[i]);
				if(_vstrmPortNum!=(ULONG)-1)
					break;
			}
		}
		//_vstrmPortNum = 100;
		if(_vstrmPortNum==(ULONG)-1)			
		{
			ERRLOG(Log::L_ERROR,PLSESSID(Playlist,"can't find idle VstrmPort"));
		}
		else
		{
			glog(Log::L_DEBUG,PLSESSID(Playlist, "got VstrmPort[%lu]"), _vstrmPortNum);
			//reset port speed
			VSTATUS status;
			IOCTL_CONTROL_PARMS_LONG controlParam;
			memset(&controlParam , 0 , sizeof(controlParam));

			controlParam.u.portSpeed.speedIndicator.denominator = 1;
			controlParam.u.portSpeed.speedIndicator.numerator = 1;

			VSTRMAPICALLSTART(VstrmClassControlPortEx1);
			status = VstrmClassControlPortEx1(	_mgr.classHandle(),
												_vstrmPortNum,
												VSTRM_GEN_CONTROL_PORT_SPEED,
												&controlParam,
												sizeof(IOCTL_CONTROL_PARMS_LONG),
												NULL,
												0);
			VSTRMAPICALLEND(VstrmClassControlPortEx1);

			glog(ZQ::common::Log::L_DEBUG, PLSESSID(Playlist,"reset VstrmPort[%lu] to [1/1]"), _vstrmPortNum);

		}
		
	}


	// Set the destination IP address
	setDestination("127.0.0.1", _vstrmPortNum+1000);

	// Set stream's current bit rate and maximum bitrate.
	setMuxRate(1000000, 1000000);

	setProgramNumber( (USHORT)_vstrmPortNum+2 );

	// add self to the play list manager
	
	_stampCreated = GetTickCount( );
	_itNext=_itCurrent=_list.end( );
	//////////////////////////////////////////////////////////////////////////	
	//////////////////////////////////////////////////////////////////////////		
	//updateTimer();
	glog(Log::L_DEBUG, PLSESSID(Playlist,"initialize playlist with state[PLAYLIST_SETUP]"));
	
	FireStateChanged(PLAYLIST_SETUP,false);
	//_currentStatus=PLAYLIST_SETUP;
#ifdef _ICE_INTERFACE_SUPPORT
	PlaylistAttr	attr;
	attr.playlistState=IPlaylist::PLAYLIST_SETUP;
	ZeroMemory(szBuf,sizeof(szBuf));
	_guid.toString(szBuf,sizeof(szBuf));
	attr.Guid=szBuf;
	attr.vstrmPort=_vstrmPortNum;
	attr.currentCtrlNum=0;
//	DWORD	storeTimeTest=GetTickCount();
//	_mgr.addNewPlaylistIntoFailOver(szBuf,attr);
//	_mgr.UpdatePlaylistFailStore(this,true,false);	
//	glog(Log::L_DEBUG,PLSESSID("addNewPlaylistIntoFailOver run time=%d"),GetTickCount()-storeTimeTest);
#endif

	///向list里面添加一个dummy这样就可以把_list.begin()当作一个标志，就像_list.end()一样
	Item	itemDummy;
	itemDummy.userCtrlNum=INVALID_CTRLNUM;
	_list.push_back(itemDummy);
	_itCurrent=_itNext=listBegin();
	
	//mgr._sessmon.subscribe(*this);
	SubScribleSink();
	
	_lastErrCode=ERR_PLAYLIST_SUCCESS;
	 
	if (_vstrmPortNum>=0) 
	{
		glog(ZQ::common::Log::L_DEBUG, PLSESSID(Playlist,"constructed playlist ok with VstrmPort[%lu]"), _vstrmPortNum );
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR, PLSESSID(Playlist,"constructed playlist with invalid VstrmPort"));
	}
	SetTimerEx(gStreamSmithConfig.lPlaylistTimeout);
//	_ProgressGranularity=10*1000;

}
void Playlist::DumpListInfo(bool bOnlyAvailSessId)
{
	iterator it=listBegin();
	glog(Log::L_DEBUG, PLSESSID(DumpListInfo, "start dumping list information"));
	for(;it!=listEnd();it++)
	{
		if( bOnlyAvailSessId && it->sessionId == 0 )
			continue;
		glog(Log::L_DEBUG,PLSESSID(dumpListInfo,"filename[%s] ctrlNum[%lu] criticalStart[%s] stampLoad[%u] stampLaunched[%u] stampUnload[%u] VstrmSess[%lu]"),
			it->objectName.string,
			it->userCtrlNum,
			it->criticalStart!=0 ? printfUTCTime(it->criticalStart).c_str():"",
			it->stampLoad, 
			it->stampLaunched,
			it->stampUnload, 
			it->sessionId);			
	}
	glog(Log::L_DEBUG,PLSESSID(DumpListInfo,"end of list dumping"));
}

void Playlist::convertVstrmErrToTianShanError( VSTATUS t)
{
}

void Playlist::ClearAllResource()
{
	glog(ZQ::common::Log::L_INFO,PLSESSID(ClearAllResource,"starting release resource allocated for this playlist"));

	std::string		strProviderId	= "";
	std::string		strProviderAssetId = "" ;
	std::string		strStreamingSource = "";
	bool			bExitOnPlaying = false;
	{
		if (_bCleared)
			return;

		ZQ::common::MutexGuard gd(_listOpLocker);
		if (_bCleared)
			return;

		_bCleared = true;

		endTimer64();
		
		UnSubScribleSink();		
		
		//record providerId and providerAssetId if available
		if ( !isCompleted() )
		{
			if(iterValid(_itCurrent))
			{
				strProviderId 		= _itCurrent->_strProviderId;
				strProviderAssetId 	= _itCurrent->_strProviderAssetId;
				strStreamingSource 	= _itCurrent->_itemLibraryUrl;
				if (gStreamSmithConfig.serverMode != 2)
					strStreamingSource = "";//reset to empty if current mode is not EdgeMode
				bExitOnPlaying		= _itCurrent->sessionId != 0;
			}
		}
		else
		{

		}

		glog(Log::L_DEBUG,PLSESSID(ClearAllResource,"set playlist state to PLAYLIST_STOP"));
		if( !isCompleted() && iterValid(_itCurrent) )
		{
			glog(Log::L_DEBUG,PLSESSID(ClearAllResource,"current playlist item[%s][%lu]"),
											_itCurrent->objectName.string, _itCurrent->userCtrlNum);
		}
		DumpListInfo();
		FireStateChanged(PLAYLIST_STOP);
		//_currentStatus=PLAYLIST_STOP;

		
		clear_pending(true);
		DumpListInfo();
		glog(Log::L_DEBUG,PLSESSID(ClearAllResource,"unloading next item"));
		unloadEx( _itNext , false );		
		

		//flush_expired();		
		DumpListInfo();		
		glog(Log::L_DEBUG,PLSESSID(ClearAllResource,"unloading current item"));
		unloadEx( _itCurrent , false );
		//_list.clear();
		if(isReverseStreaming())
			_itCurrent=listStart();
		else
			_itCurrent=listEnd();		
	}
	glog(Log::L_DEBUG,PLSESSID(ClearAllResource,"cleaning this playlist from sessmon"));
	

	{
		VSTATUS vret = 0;
		
		//if( mVstrmBwTcikets.EdgeTicket != 0 )
		{
			VSTRMAPICALLSTART(VstrmClassReleaseBandwidth);
			vret = VstrmClassReleaseBandwidth( _mgr.classHandle() , mVstrmBwTcikets.EdgeTicket );
			VSTRMAPICALLEND(VstrmClassReleaseBandwidth);

			if( vret != VSTRM_SUCCESS )
			{
				char szErrBuf[1024];
				glog(ZQ::common::Log::L_WARNING, PLSESSID(ClearAllResource,"VstrmClassReleaseBandwidth() failed for Slice Ticket[%llx], error[%s]"),
					mVstrmBwTcikets.EdgeTicket,_mgr.getErrorText(VstrmGetLastError(),szErrBuf,1023));
			}
			else
			{
				glog(ZQ::common::Log::L_INFO, PLSESSID(ClearAllResource,"VstrmClassReleaseBandwidth() Slice Ticket[%llx] released"), mVstrmBwTcikets.EdgeTicket);
			}
		}

		//if( mVstrmBwTcikets.FileTicket!= 0 )
		{
			VSTRMAPICALLSTART(VstrmClassReleaseBandwidth);
			vret = VstrmClassReleaseBandwidth( _mgr.classHandle() , mVstrmBwTcikets.FileTicket );
			VSTRMAPICALLEND(VstrmClassReleaseBandwidth);

			if( vret != VSTRM_SUCCESS )
			{
				char szErrBuf[1024];
				glog(ZQ::common::Log::L_WARNING, PLSESSID(ClearAllResource,"VstrmClassReleaseBandwidth() failed for File Ticket[%llx], error[%s] "),
					mVstrmBwTcikets.FileTicket ,_mgr.getErrorText(VstrmGetLastError(),szErrBuf,1023));
			}
			else
			{
				glog(ZQ::common::Log::L_INFO, PLSESSID(ClearAllResource,"VstrmClassReleaseBandwidth() File Ticket[%llx] released"), mVstrmBwTcikets.FileTicket);
			}
		}
	}
	
#ifdef _ICE_INTERFACE_SUPPORT
	_mgr.ClearFailOverInfo(_guid);
#endif	
	if(!_ResourceGuid.isNil())
	{	
		_mgr.FreeResource(_ResourceGuid);
		glog(Log::L_DEBUG,PLSESSID(ClearAllResource,"playlist resource cleaned"));	
	}

	{
		ZQ::common::MutexGuard gd(_listOpLocker);
		_list.erase(listBegin(),listEnd());
	}


	glog(Log::L_DEBUG,PLSESSID(ClearAllResource,"free VstrmPort[%lu]"),_vstrmPortNum);	
	_mgr.FreePortUsage(_vstrmPortNum);
	_lastErrCode=ERR_PLAYLIST_SUCCESS;

	//destroy ticket
	//set a fake time count to destroy ticket
	SetTimerEx(FAKE_TIMER_TIMECOUNT);

	{//fire event play list destroyed
		if(!m_pStreamSite)
		{
			m_pStreamSite=(StreamSmithSite*)StreamSmithSite::getDefaultSite();
		}		
		if(m_pStreamSite)
		{
			long lSeq = InterlockedIncrement(&_eventCSeqPlaylistExit);
			try
			{
#ifdef NEED_EVENTSINK_RAWDATA
				
#else
				ZQ::common::Variant	var;
				var.set( EventField_EventCSEQ ,				lSeq );
				var.set( EventField_PlaylistGuid,			_strGuid);
				var.set( EventField_ExitReason,				_strClearReason);
				var.set( EventField_ExitCode,				_iExitCode);
				var.set( EventField_ClientSessId,			m_strClientSessionID);
			
				var.set( EventField_prevProviderId,			strProviderId);
				var.set( EventField_prevProviderAssetId,	strProviderAssetId);
				
				var.set( EventField_PrevStreamingSource,	strStreamingSource);

				var.set( EventField_clusterId,				(long)GAPPLICATIONCONFIGURATION.mediaClusterId);
				var.set( EventField_playlistExitStatus,		(bool)bExitOnPlaying );
				{
					SYSTEMTIME st_utc;
					GetSystemTime(&st_utc);

					char	szNowUTC[256];
					ZeroMemory(szNowUTC,sizeof(szNowUTC));
					snprintf(szNowUTC, 255,  "%04d%02d%02dT%02d%02d%02d.%03dZ", 
										st_utc.wYear, st_utc.wMonth, st_utc.wDay,
										st_utc.wHour, st_utc.wMinute, st_utc.wSecond, 
										st_utc.wMilliseconds);
					var.set(EventField_StampUTC,std::string(szNowUTC));
				}
#endif			
				
				glog(Log::L_DEBUG, PLSESSID(ClearAllResource,"queuing E_PLAYLIST_DESTROYED, seq: [%ld], reason: %s"),
					lSeq, _strClearReason.c_str());
				m_pStreamSite->PostEventSink(E_PLAYLIST_DESTROYED,var,_strGuid);
			}
			catch (...)
			{
				ERRLOG(Log::L_ERROR,PLSESSID(ClearAllResource,"unexpected exception caught when calling PostEventSink(EVENTSINK_SESS_SPEEDCHANGED)"));
			}
		}		
	}
	if( !_pokeHoleSessID.empty() )
		_mgr._cls.unregisterPokeSession(_pokeHoleSessID);
	_mgr.unreg(*this);
	glog(Log::L_DEBUG,PLSESSID(ClearAllResource," session resource cleared"));
}

Playlist::~Playlist()
{	
	//glog(Log::L_DEBUG,PLSESSID(~Playlist,"I am dead now!"));
}
const CtrlNum Playlist::insert(IPlaylist::Item& newitem)
{
	return insertEx(newitem._whereInsert,newitem._fileName,newitem._currentUserCtrlNum,
					newitem._inTimeOffset,newitem._outTimeOffset,newitem._var,
					newitem._criticalStart,newitem._spliceIn,newitem._spliceOut,
					newitem._forceNormal,newitem._flags);
}
const CtrlNum Playlist::insert(IN const CtrlNum inserCtrlnum, IN const char* fileName, IN const CtrlNum userCtrlNum,
								 IN const uint32 inTimeOffset, IN const uint32 outTimeOffset,
							IN const time_t criticalStart, const bool spliceIn, IN const bool spliceOut,
							IN const bool forceNormal, IN const uint32 flags)
{
	ZQ::common::Variant var;
	return insertEx(inserCtrlnum,fileName,userCtrlNum,
					inTimeOffset,outTimeOffset,var,
					criticalStart,spliceIn,spliceOut,
					forceNormal,flags);
}
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
			buf[i/2]=cData<<4;
		}
		else
		{
			buf[i/2]|=cData;
		}
	}
	return i/2;
}

std::string logVectorString( const std::vector<std::string>& stringVec )
{
	
	static char szBuf[512];
	memset( szBuf, 0 ,sizeof(szBuf) );
	std::vector<std::string>::const_iterator it = stringVec.begin( );
	int pos = 0;
	for ( ; it != stringVec.end() ; it ++ , pos < sizeof(szBuf) )
	{
		pos += ::_snprintf( szBuf+pos , sizeof(szBuf) - pos , "%s " , (*it).c_str()  );
	}
	return szBuf;
	
	return std::string(szBuf);

}
std::string			Playlist::convertItemFlagToStr( Playlist::const_iterator it )
{
	if( !iterValid(it))
		return std::string("");

	std::string strFlag  = convertItemFlagToStr(it->itemFlag );
	std::ostringstream oss;
	oss<<it->objectName.string<<"/"<<it->userCtrlNum<<"/";
	return oss.str() + strFlag;
}
std::string			Playlist::convertItemFlagToStr( ULONG uFlag)
{
	std::string strRet = " ";
	//check the flags	
	if ( uFlag & PLISFlagNoPause ) 
	{
		strRet += "NoPause ";
	}
	if ( uFlag & PLISFlagNoFF ) 
	{
		strRet += "NoFF ";
	}
	if ( uFlag & PLISFlagNoRew ) 
	{
		strRet += "NoRew ";
	}
	if ( uFlag & PLISFlagNoSeek ) 
	{
		strRet += "NoSeek ";
	}
	/*if( uFlag & PLISFlagOnce )
	{
		strRet += "Deletable ";
	}*/
	if ( uFlag & PLISFlagPlayTimes )
	{
		strRet += "PlayTimes ";
	}
	if ( uFlag & PLISFlagSkipAtFF )
	{
		strRet += "SkipAtFF ";
	}
	if ( uFlag & PLISFlagSkipAtRew)
	{
		strRet += "SkipAtRew ";
	}
	return strRet;
}
// list operations
const CtrlNum Playlist::insertEx(IN const CtrlNum inserCtrlnum, IN const char* fileName, IN const CtrlNum userCtrlNum,
							IN const uint32 inTimeOffset, IN const uint32 outTimeOffset, 
							IN ZQ::common::Variant& varData,
							IN const time_t criticalStart, const bool spliceIn, IN const bool spliceOut,
							IN const bool forceNormal, IN const uint32 flags)
//throw(PlaylistException)
{
	if (fileName==NULL || strlen(fileName) <=0)
	{
		_lastErrCode=ERR_PLAYLIST_INVALID_PARA;
		ERRLOG(Log::L_ERROR,PLSESSID(insert,"invalid filename is given, reject operation"));
		return INVALID_CTRLNUM;
		//throw PlaylistException("insert an element with illegal filename");
	}
	
	
	glog(ZQ::common::Log::L_INFO, 	PLSESSID(insert, "inserting item with filename[%s] userCtrlNum[%u] "
						"insertCtrlNum[%u] inTimeOffset[%u] outTimeoffset[%u] spliceIn[%d] spliceout[%d] "
						"forceNormal[%d] flag[%s] criticalStart[%s]"),
						fileName,userCtrlNum,
						inserCtrlnum,inTimeOffset,outTimeOffset,
						spliceIn,spliceOut,forceNormal,
						convertItemFlagToStr(flags).c_str(),
						printfUTCTime(criticalStart).c_str());

	ZQ::common::Guard < ZQ::common::Mutex > guard(_listOpLocker,__MSGLOC__);

	// step 1. prepare the element node
	Item newElem; 
	std::string	strFullName;
	


	

	newElem.userCtrlNum = userCtrlNum;
	newElem.reservedFlags = flags;
	newElem.itemFlag = flags;		//use a whole 32bit flag
	newElem.inTimeOffset = inTimeOffset;
	newElem.outTimeOffset = outTimeOffset;
	newElem.spliceIn = (spliceIn?1:0);
	newElem.spliceOut = (spliceOut?1:0);
	newElem.forceNormalSpeed = (forceNormal?1:0);
	newElem.criticalStart = (criticalStart>0 ? criticalStart:0);	
	
	strncpy(newElem._rawItemName,fileName,sizeof(newElem._rawItemName)-1);
	
	//analyze encryption data and set it into item data
	{
		//get library url if it exist
		if ( varData.has(STORAGE_LIBRARY_URL) )
		{			
			ZQ::common::Variant& urls = varData[STORAGE_LIBRARY_URL];
			int iUrlCount = urls.size();
			bool bHasLibraryUrl = false;
			for(int i = 0 ;i < iUrlCount ; i++ )
			{
				//strncpy(newElem._itemLibraryUrl, strLibraryUrl.c_str(), sizeof(newElem._itemLibraryUrl)-1 );
				std::string& strTemp  = (std::string)urls[i];
				if ( !strTemp.empty() )
				{
					newElem._itemLibraryUrls.push_back( strTemp  );
					bHasLibraryUrl = true;
				}
			}
			newElem._curUsedLibraryUrlIndex = 0;
			
			if ( iUrlCount > 0 ) 
			{
				newElem._bEnableItemLibrary = true;
			}
			
			glog(ZQ::common::Log::L_INFO, PLSESSID(insertEx , "got storage library url [%s]" ),
				logVectorString( newElem._itemLibraryUrls).c_str() );
		}
		else
		{
			newElem._bEnableItemLibrary = false;
			glog(ZQ::common::Log::L_INFO , PLSESSID(insertEx , "no storage library is found, take it as from local storage" ));
		}

#ifndef _CHECK_ITEM_TYPE_USE_NEWLOGIC

		strFullName = "\\vod\\";	
		strFullName += fileName;

#else
		CheckContent tmpContentChecker(_mgr.m_ic , 
										_mgr.mContentStoreProxy ,
										_mgr._cls.handle() ,
										&_mgr.mIdxParserEnv );
		static bool bEdgeServer = gStreamSmithConfig.serverMode == 2;
		if( 0 /*newElem._bEnableItemLibrary && !bEdgeServer*/ ) 
		{
			glog(ZQ::common::Log::L_DEBUG , PLSESSID(insertEx,"item has library url, take voddrv to stream off NAS"));
			strFullName = "\\vod\\";			
			strFullName += fileName;
		}
		else if( !tmpContentChecker.GetItemType(std::string(fileName),strFullName,_strGuid,&newElem.fileFlag ) )
		//else if( !_mgr.m_contentChecker.GetItemType(std::string(fileName),strFullName,_strGuid,&newElem.fileFlag ) )
		{
			ERRLOG(ZQ::common::Log::L_ERROR, PLSESSID(insert,"can not get item type with name[%s], error[%s] "),
				fileName ,tmpContentChecker.getLastError().c_str() ) ;
			_lastErrCode = ERR_PLAYLIST_INVALID_PARA;
			return INVALID_CTRLNUM;
		}
#endif

		strncpy(newElem.objectName.string, strFullName.c_str(),sizeof(newElem.objectName.string));

		/*
#define ITEMDATA_PROVIDERID				"providerId"
#define ITEMDATA_PROVIDERASSETID		"providerAssetId"
		*/
		if( varData.has(ITEMDATA_PROVIDERID) && varData.has(ITEMDATA_PROVIDERASSETID) )
		{
			newElem._strProviderId			= (std::string)varData[ITEMDATA_PROVIDERID];
			newElem._strProviderAssetId		= (std::string)varData[ITEMDATA_PROVIDERASSETID];
			glog(ZQ::common::Log::L_INFO , PLSESSID(insertEx,"got providerId[%s] providerAssetId[%s] for item[%s][%u]"),
				newElem._strProviderId.c_str(),
				newElem._strProviderAssetId.c_str() ,
				fileName,
				userCtrlNum);
		}
		else
		{
			if( gStreamSmithConfig.serverMode == 2)//edge server mode
			{
				glog(ZQ::common::Log::L_WARNING , PLSESSID(insertEx, "no providerId and providerAssetId specified" ));
			}
			else
			{
				glog(ZQ::common::Log::L_DEBUG , PLSESSID(insertEx, "no providerId and providerAssetId specified" ));
			}
		}
		
		//just for PID test
		if (varData.has(VSTRM_ITEM_PID)) 
		{
			newElem._bhasItemPID =  true;
			newElem._itemPID = (USHORT)varData[VSTRM_ITEM_PID];
		}
		else
		{
			newElem._bhasItemPID =  false;
		}

		//get ecm vendor
		int iEnableEcm = (int ) varData[ENCRYPTION_ENABLE];
		if(iEnableEcm > 0)
		{
			glog(ZQ::common::Log::L_INFO,PLSESSID(insert,"ECM encryption is specified"));			
			newElem._bEnableEcm = true;
			int iecmVendor = (int)varData[ENCRYPTION_VENDOR];
			newElem._encryptionData.vendor = (VSTRM_ENCRYPTION_VENDOR)iecmVendor ; 
			glog(ZQ::common::Log::L_INFO,PLSESSID(insert,"  ECM:Vendor [%d] < MOTOROLA [%d] >"),iecmVendor,VSTRM_ENCRYPTION_VENDOR_MOTOROLA);

			int iecmPID = (int)varData[ENCRYPTION_ECM_PID];
			newElem._encryptionData.ecmPid = iecmPID;
			glog(ZQ::common::Log::L_INFO,PLSESSID(insert,"  ECM:PID [%d]"),iecmPID);

			int iecmCycle1 = (int)varData[ENCRYPTION_CYCLE1];
			newElem._encryptionData.Cycle1 = iecmCycle1;
			glog(ZQ::common::Log::L_INFO,PLSESSID(insert,"  ECM:cycle 1 [%d]"),iecmCycle1);

			int iecmCycle2 = (int) varData[ENCRYPTION_CYCLE2];
			newElem._encryptionData.Cycle2 = iecmCycle2;
			glog(ZQ::common::Log::L_INFO,PLSESSID(insert,"  ECM:cycle 2 [%d]"),iecmCycle2);

			int iecmFreq1 = (int) varData[ENCRYPTION_FREQ1];
			newElem._encryptionData.Freq1 = iecmFreq1;
			glog(ZQ::common::Log::L_INFO,PLSESSID(insert,"  ECM:freq 1 [%d]"),iecmFreq1);

			int iecmFreq2 = (int) varData[ENCRYPTION_FREQ2];
			newElem._encryptionData.Freq2 = iecmFreq2;
			glog(ZQ::common::Log::L_INFO,PLSESSID(insert,"  ECM:freq 2 [%d]"),iecmFreq2);

			int iecmDataCount = (int) varData[ENCRYPTION_DATACOUNT];
			glog(ZQ::common::Log::L_INFO,PLSESSID(insert,"  ECM:data count [%d]"),iecmDataCount);

			newElem._iEcmDataCount = iecmDataCount;
			{
				ZQ::common::Variant& varEcmData = (ZQ::common::Variant&)varData[ENCRYPTION_DATAPREFIX];
				char	szEcmDataName[256];
				for(int iCount =0 ; iCount <iecmDataCount ; iCount ++)
				{
					sprintf(szEcmDataName,"%s%d",ENCRYPTION_DATAPREFIX,iCount);
					std::string	strData = (std::string)varEcmData[szEcmDataName];
					glog(ZQ::common::Log::L_INFO,PLSESSID(insert,"ECM:data(%d): %s"), strData.length(), strData.c_str() );
					newElem._encryptionData.motoData[iCount].byteCount = 4+ConvertStringIntoBinary(strData,newElem._encryptionData.motoData[iCount].Message);
				}
			}
			
			{
				ZQ::common::Variant varPnOffsetData = (ZQ::common::Variant&)varData[ENCRYPTION_PNOFFSETPREFIX];
				char szPnOffsetName[256];
				for(int iCount=0; iCount<iecmDataCount ;iCount++)
				{
					sprintf(szPnOffsetName,"%s%d",ENCRYPTION_PNOFFSETPREFIX,iCount);
					newElem._encryptionData.motoData[iCount].ProgramNumberOffset = (int)varPnOffsetData[szPnOffsetName];
				}
			}			
		}
		else
		{
			newElem._bEnableEcm = false;
		}

	}

#ifndef _CHECK_ITEM_TYPE_USE_NEWLOGIC
	
	newElem._itemPlaytime = -1;
	newElem._itemBitrate  = -1;

#else//_CHECK_ITEM_TYPE_USE_NEWLOGIC
	static bool bEdgeServer = gStreamSmithConfig.serverMode == 2;
	if( 0 /*newElem._bEnableItemLibrary  && !bEdgeServer*/ )
	{
		glog(ZQ::common::Log::L_INFO, PLSESSID(insertEx, "item has url, take playtime = -1 as stream from NAS"));
		newElem._itemPlaytime = -1;
		newElem._itemBitrate  = -1;	
	}
	else
	{
		if(!_mgr.m_contentChecker.GetItemAttribute(std::string(fileName),
												newElem._itemPlaytime,
												newElem._itemBitrate,
												newElem._itemRealTotalTime,
												newElem._bPWE,
												_strGuid,
												newElem.spliceIn,
												newElem.spliceOut,
												newElem.inTimeOffset,
												newElem.outTimeOffset))
		{
			_lastErrCode=ERR_PLAYLIST_INVALID_PARA;
			ERRLOG(Log::L_ERROR,PLSESSID(insert,"can't get item attribute of content [%s]"),fileName);
			return INVALID_CTRLNUM;
		}
	}

#endif//_CHECK_ITEM_TYPE_USE_NEWLOGIC

	int64 playtimes = (newElem.itemFlag & PLISFlagPlayTimes) >> 4;
	if ( playtimes >= 1 && playtimes <= 13)
	{
		newElem._itemPlayTimesCount = playtimes;
		glog(Log::L_INFO,PLSESSID( insert, "Advertisement playtimes: [%d], flag: [%s]"), playtimes, convertItemFlagToStr(newElem.itemFlag).c_str());
	}
	glog(ZQ::common::Log::L_INFO,PLSESSID(insert,"got content attribute by name[%s]: playTime[%ld]; bitRate[%ld]"),
											fileName,newElem._itemPlaytime,newElem._itemBitrate);
		
	iterator where = findUserCtrlNum(inserCtrlnum);
	// step 1. test if the location about to insert is at a allowed place
	if (where <listBegin() || where > listEnd())
	{
		_lastErrCode=ERR_PLAYLIST_INVALID_PARA;
		ERRLOG(Log::L_ERROR,PLSESSID(insert,"insert an element out of the range of list"));
		return INVALID_CTRLNUM;
		//throw PlaylistException("insert an element out of the range of list");
	}

	if(! ( _currentStatus == PLAYLIST_SETUP || _currentStatus == PLAYLIST_STOP ) )
	{		
		if(!isReverseStreaming())
		{//正常Streaming
			if(_itNext < listEnd() && where==_itNext)
			{
				_lastErrCode=ERR_PLAYLIST_INVALIDSTATE;
				ERRLOG(Log::L_ERROR,PLSESSID(insert,"could not insert element before the playing or preloaded element"));
				return INVALID_CTRLNUM;
			}
		}
		else
		{//反向Streaming
			if( _itNext > listStart() && where ==_itCurrent )
			{
				_lastErrCode=ERR_PLAYLIST_INVALIDSTATE;
				ERRLOG(Log::L_ERROR,PLSESSID(insert,"could not insert element before the playing or preloaded element"));
				return INVALID_CTRLNUM;
			}
		}
	}

	// step 3, do the inserting
	

	static ULONG sCtrlId_ =0;  // global internal control id seed
	//do not use internal user ctrl num 
	//because if Service restart but session is still there , internal user ctrl num may conflict
	newElem.intlCtrlNum = newElem.userCtrlNum;//sCtrlId_++;

	// step 3.1 back up the old location of _itCurrent, _itNext, _itNextCriticalStart
	//这里需要区别对待streaming的方向
	ULONG crntSeqNum = INVALID_CTRLNUM, nextSeqNum =INVALID_CTRLNUM, nextStartSeqNum=INVALID_CTRLNUM;
	if ( iterValid(_itCurrent) )
		crntSeqNum = _itCurrent->intlCtrlNum;

	if ( iterValid(_itNext) /*_itNext > listStart() && _itNext < listEnd()*/ )
		nextSeqNum = _itNext->intlCtrlNum;
	
	
	bool isLastItem= (bool)( _itNext == listEnd() || _itNext == listStart() );

	// step 3.2 do the insert
	const_iterator ret = _list.insert( where , newElem );
	
	///Check if current item is the last item 

	// step 3.3 restore where the iterators were
	if ( INVALID_CTRLNUM != crntSeqNum )
	{
		_itCurrent = findInternalCtrlNum( crntSeqNum );
	}
	else
	{
		_itCurrent = listBegin( );
	}

	if ( INVALID_CTRLNUM != nextSeqNum )
	{
		_itNext = findInternalCtrlNum( nextSeqNum );
	}
	else
	{
		if( isReverseStreaming( ) )
		{
			_itNext= _itCurrent > listStart() ? _itCurrent -1 : listStart();
		}
		else
		{
			_itNext= _itCurrent < listEnd() ? _itCurrent + 1 : listEnd();
		}
		//_itNext = listBegin();
	}

	//glog(Log::L_DEBUG,"Now PlayListSize=%d",size());;
	glog(Log::L_INFO,PLSESSID( insert , "inserted a item with ctrlNum[%u] wheretoInsert[%u] filename[%s], and now playlist size[%d]"),
								userCtrlNum , inserCtrlnum , strFullName.c_str() , size() );
	
	if( _bCommitted )
		printList("Playlist::insert");
	

	endTimer64();
	_isGoodItemInfoData = true;
	SetTimerEx( gStreamSmithConfig.lPlaylistTimeout );
	if(_mgr._timeout ==TIMEOUT_INF)
		_mgr.wakeup();	
	if(isLastItem && PLAYLIST_PLAY==_currentStatus)
	{
		//glog(ZQ::common::Log::L_INFO,PLSESSID(insert,"last item ,call primeNext"));
		//primeNext();
		//如果在保护区域内insert会导致不能及时播放新的Item
		updateTimer();
	}
	else if( _currentStatus == PLAYLIST_PLAY )
	{
		//glog(ZQ::common::Log::L_INFO,PLSESSID(insert,"Update Timer"));
		updateTimer();
	}
#ifdef _ICE_INTERFACE_SUPPORT
	_mgr.UpdatePlaylistFailStore(this,true,true);
#endif	
	_lastErrCode=ERR_PLAYLIST_SUCCESS;

	glog(ZQ::common::Log::L_INFO,PLSESSID(insert,"succesfully inserted file[%s][%u]"),fileName,userCtrlNum);

	return ret->userCtrlNum;
}

const CtrlNum	Playlist::push_back(IPlaylist::Item& item)
{
//	return push_back(item._fileName,item._currentUserCtrlNum,item._inTimeOffset,
//					item._outTimeOffset,item._var item._criticalStart,
//					item._spliceIn,item._spliceOut,
//					item._forceNormal,item._flags);
	CtrlNum	 ctrl=INVALID_CTRLNUM;
	return insertEx(ctrl, item._fileName, item._currentUserCtrlNum, 
					item._inTimeOffset, item._outTimeOffset,
					item._var,
					item._criticalStart, item._spliceIn, item._spliceOut,
					item._forceNormal, item._flags);
}
const CtrlNum Playlist::push_back(const char* fileName, 
									const uint32 userCtrlNum, const uint32 inTimeOffset, 
									const uint32 outTimeOffset, const time_t criticalStart, 
									const bool spliceIn, const bool spliceOut, 
									const bool forceNormal, const uint32 flags)
{
	CtrlNum	 ctrl=INVALID_CTRLNUM;
	return insert(ctrl, fileName, userCtrlNum, inTimeOffset, outTimeOffset, criticalStart, spliceIn, spliceOut, forceNormal, flags);
}


void  Playlist::printList(const char* hints)
{
#if 1
	glog(ZQ::common::Log::L_DEBUG, PLSESSID(printList,"%s Playlist:"), (hints?hints:""));
	for (iterator it = listBegin(); it <listEnd(); it++)
		glog(ZQ::common::Log::L_DEBUG, PLSESSID(printList,"%c%c%c{intlCtrlNum:%lu; file:%s; userCtrlNum:%lu; sessId:%lu criticalStart:%s}"), 
//		       ((!isCompleted() && it >=_itCurrent && it == _itNextCriticalStart) ? '-' : ' '), 
			   ((isCompleted() || it !=_itCurrent) ? ' ' : '>'), (0 !=it->sessionId ? '*' : ' '),
			   (0 !=it->criticalStart ? 'i' : ' '),
			   it->intlCtrlNum, 
			   it->objectName.string,
			   it->userCtrlNum,
			   it->sessionId,
			   (0 !=it->criticalStart ? printfUTCTime(it->criticalStart).c_str() : "") );
#endif // _DEBUG
}

//Playlist::const_iterator Playlist::flush_expired()
const CtrlNum		Playlist::flush_expired()
{
	ZQ::common::Guard < ZQ::common::Mutex > guard(_listOpLocker,__MSGLOC__);
	if (empty())
	{
		//return end();
		glog(Log::L_DEBUG,PLSESSID(flush_expired,"playlist is empty, skipping"));
		return INVALID_CTRLNUM;
	}
	glog(ZQ::common::Log::L_DEBUG,PLSESSID(flush_expired,"enter flush_expired"));
	if( !iterValid(_itCurrent) )
	{
		glog(ZQ::common::Log::L_DEBUG,PLSESSID(flush_expired,"invalid current item, leave flush_expired"));
		return INVALID_CTRLNUM;
	}
	ULONG crntSeqNum = _itCurrent->intlCtrlNum;
	
	if(!isReverseStreaming())
	{
		if ( _itCurrent >= listBegin() )
			_list.erase(listBegin(), _itCurrent);
	}
	else
	{
		//_list.erase(listEnd(),)
		iterator	itTemp=_itCurrent;
		if( itTemp < listEnd() )
		{
			_list.erase(itTemp+1,listEnd());
		}
	}
	// flush the elements before current	
	_itCurrent = findInternalCtrlNum(crntSeqNum);

	if(!isReverseStreaming())
	{
		_itNext = _itCurrent <listEnd() ?_itCurrent+1:listEnd();
	}
	else
	{
		_itNext = _itCurrent>listStart() ?_itCurrent-1:listStart();
	}

	glog(ZQ::common::Log::L_DEBUG,PLSESSID(flush_expired,"done"));
	if( iterValid(_itNext) )
	{
		_lastErrCode = ERR_PLAYLIST_SUCCESS;
		return _itNext->userCtrlNum;
	}
	else
	{		
		return INVALID_CTRLNUM;
	}
}

const bool Playlist::clear_pending(const bool includeInitedNext)
{
	if (isCompleted())
	{
		_lastErrCode=ERR_PLAYLIST_SUCCESS;
		return true;
	}

	ZQ::common::Guard < ZQ::common::Mutex > guard(_listOpLocker,__MSGLOC__);
	glog(ZQ::common::Log::L_DEBUG,PLSESSID(clear_pending,"entering"));
	iterator it = listCurrent();
	if( !iterValid(it) )
	{
		glog(ZQ::common::Log::L_DEBUG, PLSESSID(clear_pending,"quitting cleaning per empty playlist"));
		return true;
	}
	CtrlNum curCtrlNum=it->intlCtrlNum;
	
	printList("Playlist::clear_pending");

	// move to the skip element
	if(!isReverseStreaming())
	{
		if ( (++it) == listEnd())
			return true;
		iterator itTemp=it+1;
		for(;itTemp<listEnd();itTemp++)
		{
			if(itTemp->sessionId!=0)
			{				
				unloadEx(itTemp);
			}
		}		
		_list.erase(it+1, listEnd());
	}
	else
	{
		if(--it==listStart())
			return true;
		iterator itTemp=listBegin();
		for(;itTemp<it;itTemp++)
		{
			if(itTemp->sessionId!=0)
				unloadEx(itTemp);
		}
		_list.erase(listBegin(),it);
	}

	// clear up those after the skip element
	 
	printList("Playlist::clear_pending");
	

	_itCurrent = findInternalCtrlNum(curCtrlNum);
	if( isReverseStreaming() )
	{//rewind
		_itNext = _itCurrent>listStart() ?_itCurrent-1 :listStart();
	}
	else
	{//normal
		_itNext = (_itCurrent < listEnd()) ? (_itCurrent+1) : listEnd();
	}

	_lastErrCode = ERR_PLAYLIST_SUCCESS;
	glog(ZQ::common::Log::L_DEBUG, PLSESSID(clear_pending,"done"));
	return true;
}


//Playlist::const_iterator Playlist::erase(const_iterator where)
const CtrlNum	Playlist::erase(const CtrlNum ctrlNum)
{
	ZQ::common::Guard < ZQ::common::Mutex > guard(_listOpLocker,__MSGLOC__);
	
	glog(Log::L_DEBUG,PLSESSID(erase,"entering with CtrlNum=%u"),ctrlNum);
	iterator where = findUserCtrlNum(ctrlNum);

	if ( !iterValid(where) /*where <= listStart() || where >=listEnd()*/ )
	{
		_lastErrCode=ERR_PLAYLIST_INVALID_PARA;
		ERRLOG(Log::L_ERROR,PLSESSID(erase,"can't find item with ctrlNum[%u]"),ctrlNum);
		return INVALID_CTRLNUM;
	}
	
	if (0 != where->sessionId)
	{
		_lastErrCode=ERR_PLAYLIST_INVALIDSTATE;
		ERRLOG(Log::L_ERROR, PLSESSID(erase,"can not erase an ongiong item item[%s][%lu]"), where->objectName.string, where->userCtrlNum);
		return INVALID_CTRLNUM;
	}

	// flush the elements before current	
	ULONG crntSeqNum;
	bool bComplete = false;
	if (isCompleted())
	{
		bComplete =true;
	}
	else
	{
		if( iterValid(_itCurrent) )
		{
			crntSeqNum = _itCurrent->intlCtrlNum;
			if( where == _itCurrent )
			{
			if(  iterValid( _itNext ) )
				{
					crntSeqNum = _itNext->intlCtrlNum;
				}	
				else
				{
					if (isReverseStreaming()) 
					{
						_itCurrent = listStart();
					}
					else
					{
						_itCurrent = listEnd();
					}
				}
			}
		}
		else
		{
			if (isReverseStreaming()) 
			{
				_itCurrent = listStart();
			}
			else
			{
				_itCurrent = listEnd();
			}
		}
	}
	const_iterator ret = _list.erase(where);
	glog(Log::L_INFO,PLSESSID(erase,"erased with ctrlNum[%u]"),ctrlNum);
	if( bComplete )
	{
		if (isReverseStreaming()) 
		{
			_itCurrent = listStart();
		}
		else
		{
			_itCurrent = listEnd();
		}
	}
	else
	{
		_itCurrent = findInternalCtrlNum(crntSeqNum);
	}

	if(isReverseStreaming())
	{
		_itNext=_itCurrent>listStart() ?_itCurrent-1:listStart();
	}
	else
	{
		_itNext=_itCurrent<listEnd() ?_itCurrent+1:listEnd();
	}
	
	glog(Log::L_DEBUG,PLSESSID(erase,"done"));
	return 1;
}



/*
    @ 2013-12-01
	Change doLoad() behavir based on discussion on XORCE-814
	The reason to adjust the logic is because the implementation for VVX-1951. It is the load won't block while the index is loading. 
	
	Old logic before this change
	- load (with TimeSkip)
	- Reposition (NPT, no speed)
	- SetSpeed
	- Prime
	- Play

	New logic
	- load (no TimeSkip)
	- Reposition (NPT, no speed) (need retry if it returns VSTRM_DEVICE_BUSY)
	- SetSpeed                   (need retry if it returns VSTRM_DEVICE_BUSY)    
	- Prime
	- Play
	
	Addtional comment for Reposition:
	  The reason why it needs retry logic becaue:
	  a)In case of 1x and FF, reposition may return earlier before index loading finished in some special situation.
	  b)In case of FR, reposition is not a sync API, need to retry until index finished. (Ex: FR from B to A, A is pre-load and need retry)
*/
bool Playlist::doLoad( IN iterator it , IN float newSpeed , IN ULONG timeOffset  ,OUT StreamControlResultInfo& info , bool bSeek )
{
	ZQ::common::MutexGuard gd( _listOpLocker );
	if( isCompleted() || !iterValid(it) )
	{
		_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
		ERRLOG(Log::L_ERROR,PLSESSID(doLoad , "the item specified to load is out of range") );
		FireStateChanged( PLAYLIST_STOP );		
		return false;
	}
	if( it->sessionId != 0 )
	{				
		glog( ZQ::common::Log::L_INFO, PLSESSID( doLoad , "current item[%s][%lu] is being played as VstrmSess[%lu]"),
			it->objectName.string, it->userCtrlNum, it->sessionId	);
		return true;
	}
	if (IsItemPlaytimesCountZero(it))
	{
		glog(ZQ::common::Log::L_INFO, PLSESSID(doLoad,"ads playtimes: [%d], flag: [%s]"), it->_itemPlayTimesCount, convertItemFlagToStr(it->itemFlag).c_str());
		return true;
	}
	bool bChangeParamter = false;
	if( isFFScale( newSpeed ) && !checkResitriction( it , PLISFlagNoFF ) )
	{
		bChangeParamter = true;
		newSpeed = 1.0;
	}
	else if( isREWScale( newSpeed ) && !checkResitriction( it , PLISFlagNoRew ))
	{
		bChangeParamter = true;
		newSpeed = 1.0;
	}
	
	if( timeOffset != 0 && !checkResitriction( it , PLISFlagNoSeek ) && _b1stPlayStarted ) 
	{//allow seeking if this is the first play of this session
		// bugid 16514
		bChangeParamter = true;
		timeOffset	= 0;
		newSpeed	= 1.0;
	}
	if( bChangeParamter )
	{
		glog(ZQ::common::Log::L_WARNING,PLSESSID(doLoad,"adjust timeoffset[%ld] newSpeed[%f] due to restriction[%s]"),
			timeOffset , newSpeed , convertItemFlagToStr( it ).c_str() );
	}

	if( bSeek )
	{
		timeOffset += it->inTimeOffset;
		long	absolutePlayTime = it->_itemRealTotalTime;
		if( it->outTimeOffset != 0 )
		{
			absolutePlayTime = absolutePlayTime > (long)it->outTimeOffset ? absolutePlayTime : (long)it->outTimeOffset;
		}
		glog(ZQ::common::Log::L_WARNING, PLSESSID(doLoad,"adjusted timeoffset to [%lu] because of inTimeOffset[%lu]"), timeOffset , it->inTimeOffset);

		{
			//adjust the time offset
			if( isLastItem(it) && it->_bPWE )
			{
				//do nothing, do not adjust the time offset if item is in PWE mode
			}			
			else if( _bEnableEOT && isLastItem(it) )
			{
				if( gStreamSmithConfig.lEOTProtectionTime >  0 )
				{
					if(  newSpeed > 0.01f || ( fabs(newSpeed-0.00f)<0.01f  && _crntSpeed.numerator > 0 ) )
					{
						long	uBitRate = it->_itemBitrate ;
						long	uTotalPlayTime = it->_itemPlaytime;
						long	uTotalDuration = it->_itemRealTotalTime ;
						long	targetTimeOffset = static_cast<long>( timeOffset );
						long	absoluteOutTimeOffset = 0;

						absoluteOutTimeOffset = it->_itemRealTotalTime;
						if( it->outTimeOffset != 0 )
						{
							absoluteOutTimeOffset = (ULONG)uTotalDuration > it->outTimeOffset ? it->outTimeOffset : (ULONG)uTotalDuration;
						}
						if( targetTimeOffset > absoluteOutTimeOffset - gStreamSmithConfig.lForceNormalTimeBeforeEnd )
						{
							targetTimeOffset = absoluteOutTimeOffset - gStreamSmithConfig.lForceNormalTimeBeforeEnd;

							glog(ZQ::common::Log::L_WARNING, PLSESSID(doLoad,"adjusted to new timeOffset[%ld] for this last item[%lu][%s] with inTimeOffset[%lu], outTimeOffset[%lu], EOTSize[%d]"),
								targetTimeOffset,
								it->userCtrlNum, it->objectName.string,								
								it->inTimeOffset , it->outTimeOffset,
								gStreamSmithConfig.lForceNormalTimeBeforeEnd);
						}
						timeOffset = targetTimeOffset;
					}
				}
			}		
		}


		if( !(isLastItem(it) && it->_bPWE ) && gStreamSmithConfig.lPreloadTime > 0 )
		{
			if( fabs(newSpeed - 0.00f) < 0.001f )
			{	
				if( _crntSpeed.numerator > 0 )
				{
					if( (int32)timeOffset > (absolutePlayTime - gStreamSmithConfig.lPreloadTime) )
					{
						timeOffset = absolutePlayTime - gStreamSmithConfig.lPreloadTime;
					}
				}
				if( _crntSpeed.numerator < 0 )
				{
					if( (int32)timeOffset < (gStreamSmithConfig.lPreloadTime ) )
					{
						timeOffset = gStreamSmithConfig.lPreloadTime;
					}
				}
			}
			else
			{
				if( newSpeed > 0.01f  )
				{
					if( (int32)timeOffset > (absolutePlayTime - gStreamSmithConfig.lPreloadTime) )
					{
						timeOffset = absolutePlayTime - gStreamSmithConfig.lPreloadTime;
					}
				}
				if( newSpeed < -0.01f )
				{
					if( (int32)timeOffset < gStreamSmithConfig.lPreloadTime )
					{
						timeOffset = gStreamSmithConfig.lPreloadTime;
					}
				}
			}
			glog(ZQ::common::Log::L_INFO, PLSESSID(doLoad,"adjusted timeoffset to [%lu] due to preloadTime[%d] totalDuration[%ld]"), timeOffset , gStreamSmithConfig.lPreloadTime , it->_itemRealTotalTime);
		}
	}


	glog(Log::L_INFO, PLSESSID(doLoad,"loading item[%s][%lu] with speed=[%f] and Timeoffset[%lu]") , it->objectName.string,it->userCtrlNum, newSpeed , timeOffset );

	ZQTianShan::Util::TimeSpan spanLoad;spanLoad.start();

	//prepare parameter to load item
	IOCTL_LOAD_PARAMS			loadParams;
	DVB_SESSION_ATTR			dvbAttrs;

	memset(&loadParams, 0x00, sizeof(loadParams));
	memset(&dvbAttrs,0x00, sizeof(dvbAttrs));

	loadParams.FileLocal 			= TRUE;
	loadParams.Debug 				= TRUE;

	loadParams.Mask =
		LOAD_IOCTL_FILE_LOCAL
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



	loadParams.Debug		= TRUE;
	if( gStreamSmithConfig.lLoadItemWithoutNpt > 0 )
		loadParams.TimeSkip = 0 ;
	else
		loadParams.TimeSkip		= timeOffset;


#ifdef FILE_FLAG_NPVR	
	loadParams.FlagsAndAttributes = (it->fileFlag & STREAMSMITH_FILE_FLAG_NPVR) ?  FILE_FLAG_NPVR : 0 ;
#endif

	// make a copy of common DVB attributes of the playlist
	memcpy(&dvbAttrs, &_dvbAttrs, sizeof(dvbAttrs));

	//////////////////////////////////////////////////////////////////////////

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

#ifdef VSTRM_ONDEMANDE_SESSID_ON
		LOAD_CODE_ONDEMAND_SESSION_ID,		&_userSessGuid,						sizeof(_userSessGuid),
#endif//FILE_FLAG_NPVR

		LOAD_CODE_DVB_SESSION_ATTRIBUTES, 	&dvbAttrs,						sizeof(_dvbAttrs)	
	};
	// Set vod file spec
	strcpy ((PCHAR)&loadParams.ObjectName, it->objectName.string);
	paramTableV2[0].loadValueLength = static_cast<USHORT>( strlen(it->objectName.string));

	// Set the output destination
	loadParams.DestPortHandle = _vstrmPortNum; 

	// Abort video on exit
	loadParams.TerminateOnExit = FALSE;

	///Restamp ts , so that two clips can be treat as one clip when jumping from one to the other
	//dvbAttrs.FlagRestamp		= TRUE;

	// splice-in or cue-in
	dvbAttrs.spliceIn			= it->spliceIn;
	dvbAttrs.inTimeOffset		= it->inTimeOffset;

	// splice-out or cue-out
	dvbAttrs.spliceOut			= it->spliceOut;
	dvbAttrs.outTimeOffset		= it->outTimeOffset;

	// force normal speed
	dvbAttrs.forceNormalSpeed	= false;//it->forceNormalSpeed;

	dvbAttrs.VideoPid.Old =	dvbAttrs.VideoPid.New =  (USHORT)_perStreamPID;	

	if (it->_bEnableEcm) 
	{
		dvbAttrs.EcmPid.New = dvbAttrs.EcmPid.Old = it->_encryptionData.ecmPid;
	}

	// Copy the parameters from the temporary location into the IOCTL structure for the call
	IOCTL_CONTROL_PARMS_LONG	parmsV2;
	memset(&parmsV2 , 0 ,sizeof(parmsV2));
	parmsV2.u.load.loadParamCount = sizeof (paramTableV2) / sizeof (LOAD_PARAM);
	int iParameterCount = 0;
	for ( iParameterCount = 0; iParameterCount < parmsV2.u.load.loadParamCount; iParameterCount++)
	{
		parmsV2.u.load.loadParamArray[iParameterCount].loadCode 		= paramTableV2[iParameterCount].loadCode;	
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueLength 	= paramTableV2[iParameterCount].loadValueLength;
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueP		= paramTableV2[iParameterCount].loadValueP; 	
	}

	//add pre-encryption-data if needed
	if(it->_bEnableEcm)
	{
		parmsV2.u.load.loadParamArray[iParameterCount].loadCode		=	(USHORT)LOAD_CODE_ENCRYPTION_VENDOR;
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueP	=	&(it->_encryptionData.vendor);
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueLength=	sizeof(it->_encryptionData.vendor);
		iParameterCount++;

		parmsV2.u.load.loadParamArray[iParameterCount].loadCode		=	(USHORT)LOAD_CODE_ENCRYPTION_PID;
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueP	=	&(it->_encryptionData.ecmPid);
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueLength=	sizeof(it->_encryptionData.ecmPid);
		iParameterCount++;

		parmsV2.u.load.loadParamArray[iParameterCount].loadCode		=	(USHORT)LOAD_CODE_ENCRYPTION_CYCLE_1;
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueP	=	&(it->_encryptionData.Cycle1);
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueLength=	sizeof(it->_encryptionData.Cycle1);
		iParameterCount++;

		parmsV2.u.load.loadParamArray[iParameterCount].loadCode		=	(USHORT)LOAD_CODE_ENCRYPTION_CYCLE_2;
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueP	=	&(it->_encryptionData.Cycle2);
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueLength=	sizeof(it->_encryptionData.Cycle2);
		iParameterCount++;

		parmsV2.u.load.loadParamArray[iParameterCount].loadCode		=	(USHORT)LOAD_CODE_ENCRYPTION_FREQUENCY_1;
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueP	=	&(it->_encryptionData.Freq1);
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueLength=	sizeof(it->_encryptionData.Freq1);
		iParameterCount++;

		parmsV2.u.load.loadParamArray[iParameterCount].loadCode		=	(USHORT)LOAD_CODE_ENCRYPTION_FREQUENCY_2;
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueP	=	&(it->_encryptionData.Freq2);
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueLength=	sizeof(it->_encryptionData.Freq2);
		iParameterCount++;

		parmsV2.u.load.loadParamArray[iParameterCount].loadCode		=	(USHORT)LOAD_CODE_ENCRYPTION_DATA;
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueP	=	&(it->_encryptionData.motoData);
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueLength=	 static_cast<USHORT>((it->_iEcmDataCount+1)*sizeof(it->_encryptionData.motoData[0]));
		iParameterCount++;
	}

	if ((it->_bhasItemPID)) 
	{
		parmsV2.u.load.loadParamArray[iParameterCount].loadCode		=	(USHORT)LOAD_CODE_VIDEO_PID;
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueP	=	&(it->_itemPID);
		parmsV2.u.load.loadParamArray[iParameterCount].loadValueLength=	sizeof(it->_itemPID);
		iParameterCount++;
	}

	if ( it->_bEnableItemLibrary  && gStreamSmithConfig.serverMode != 2 ) //not EdgeServer
	{//if is not running at EdgeServer mode and has URL
#if VER_PRODUCTVERSION_MAJOR >= 6
		if( it->_itemLibraryUrls.size() <= 0 )
		{
			glog(ZQ::common::Log::L_ERROR, PLSESSID(loadEx,"no library url specified" ));
		}
		else if( it->_curUsedLibraryUrlIndex >=  (long)it->_itemLibraryUrls.size())
		{
			glog(ZQ::common::Log::L_ERROR, PLSESSID(loadEx,"tried all urls provided, reached the end" ));
		}
		else
		{
			strncpy( it->_itemLibraryUrl ,it->_itemLibraryUrls[ it->_curUsedLibraryUrlIndex ].c_str() , sizeof(it->_itemLibraryUrl)-1 );
			parmsV2.u.load.loadParamArray[iParameterCount].loadCode		=	(USHORT)LOAD_CODE_OBJECT_NAME_URL;
			parmsV2.u.load.loadParamArray[iParameterCount].loadValueP	=	&(it->_itemLibraryUrl);
			parmsV2.u.load.loadParamArray[iParameterCount].loadValueLength=	 static_cast<USHORT>(strlen( it->_itemLibraryUrl ));
			iParameterCount++;
			glog(ZQ::common::Log::L_INFO , PLSESSID(loadEx , "loading item with URL[%s] index[%ld] for ctrlNum[%lu]" ),
				it->_itemLibraryUrl,
				it->_curUsedLibraryUrlIndex,
				it->userCtrlNum	);
		}

#endif //only version >= 6  can support NAS streaming
	}
	else
	{		
		if( ( it->_curUsedLibraryUrlIndex >= 0 ) && ( it->_curUsedLibraryUrlIndex  < (int)it->_itemLibraryUrls.size() ) )
			strncpy( it->_itemLibraryUrl ,it->_itemLibraryUrls[ it->_curUsedLibraryUrlIndex ].c_str() , sizeof(it->_itemLibraryUrl)-1 );		
	}

	parmsV2.u.load.loadParamArray[iParameterCount].loadCode					= LOAD_CODE_BANDWIDTH_TICKET; 
	parmsV2.u.load.loadParamArray[iParameterCount].loadValueLength			= sizeof( mVstrmBwTcikets.FileTicket); 
	parmsV2.u.load.loadParamArray[iParameterCount].loadValueP				= &mVstrmBwTcikets.FileTicket; 
	iParameterCount++; 
	

	//////////////////////////////////////////////////////////////////////////
	//LOAD_CODE_CDN_PWE
// 	int needCdnPwe = 1;//it->_bPWE ? 1 : 0;
// 	parmsV2.u.load.loadParamArray[iParameterCount].loadCode					= LOAD_CODE_CDN_PWE; 
// 	parmsV2.u.load.loadParamArray[iParameterCount].loadValueLength			= sizeof( needCdnPwe); 
// 	parmsV2.u.load.loadParamArray[iParameterCount].loadValueP				= &needCdnPwe; 
// 	iParameterCount++; 
//
//	glog(ZQ::common::Log::L_INFO,PLSESSID(doLoad,"load item with bwticket[%llx] PWE[%d]"), mVstrmBwTcikets.FileTicket, needCdnPwe);


// 	BOOLEAN			bRealeaseBWTicketOnTerm = TRUE;
// 	parmsV2.u.load.loadParamArray[iParameterCount].loadCode					= LOAD_CODE_REL_BANDWIDTH_TICKET_ONTERM; 
// 	parmsV2.u.load.loadParamArray[iParameterCount].loadValueLength			= sizeof(bRealeaseBWTicketOnTerm); 
// 	parmsV2.u.load.loadParamArray[iParameterCount].loadValueP				= &bRealeaseBWTicketOnTerm; 
//  iParameterCount++; 	

	//Add this for new parameter entry
	parmsV2.u.load.loadParamCount = iParameterCount;

	VSTRMAPICALLSTART(VstrmClassControlSessionEx1);
	VSTATUS status = VstrmClassControlSessionEx1(  _mgr.classHandle(), 
													0,
													VSTRM_GEN_CONTROL_LOAD,
													&parmsV2, 
													sizeof(parmsV2), 
													cbItemCompleted, 
#if VER_PRODUCTVERSION_MAJOR >= 6
													(PVOID)_plExistId);
#else
													(ULONG)_plExistId);
#endif 

	VSTRMAPICALLEND(VstrmClassControlSessionEx1);
	if ( status != VSTRM_SUCCESS )
	{
		it->sessionId 	=	0;					// just to be sure
		_ntstatus 		= GetLastError ();		// capture secondary error code (if any)
		
		char	szErrbuf[1024];
		ZeroMemory(szErrbuf,1024);	
		
		switch( VstrmGetLastError() )
		{//VSTRM_BANDWIDTH_EXCEEDED
		case VSTRM_BANDWIDTH_EXCEEDED:
			{
				_lastExtErrCode			= EXT_ERRCODE_BANDWIDTH_EXCEEDED;
				
				_lastItemStepErrorCode	= VSTRM_BANDWIDTH_EXCEEDED;
				_lastItemStepItemName	= it->_rawItemName;
			}
			break;
		default:
			{
				//do not reset it to 0 here, do this in OnItemStep() function
				//_lastItemStepErrorCode	= 0;
				_lastExtErrCode = 0;
			}
			break;
		}
		_lastErrCode = ERR_PLAYLIST_SERVER_ERROR;
		
		ERRLOG(Log::L_EMERG, PLSESSID(doLoad,"VstrmClassControlSessionEx1(VSTRM_GEN_CONTROL_LOAD) failed with content[%s] for item[%s][%lu], error[%s] , api time cost[%lld]"),
			it->objectName.string ,	it->objectName.string, 	it->userCtrlNum ,_mgr.getErrorText(VstrmGetLastError(),szErrbuf,1023) , spanLoad.stop() );
		
		_lastItemStepErrDesc = _lastError;

		DumpItemInfo(it);		
	}
	else
	{
		_lastExtErrCode			= 0;

		//do not reset it to 0 here, do this in OnItemStep() function
		//_lastItemStepErrorCode	= 0;

		updateOpTime();

		glog(Log::L_INFO, PLSESSID(doLoad,"loaded item[%s][%lu], url[%s], got VstrmSess[%lu]"),
			it->objectName.string,
			it->userCtrlNum,			
			it->_itemLibraryUrl,
			parmsV2.u.load.sessionId);
#if _USE_NEW_SESSION_MON
		RegisterSessID(parmsV2.u.load.sessionId);
#endif	
		DumpItemInfo(it);

		it->sessionId = parmsV2.u.load.sessionId;

		DisplaySessionAttr(it);

		// stamp the execution
		it->stampLoad		= GetTickCount();
		it->stampLaunched	= it->stampUnload = it->stampLastUpdate =0;		
	

		//////////////////////////////////////////////////////////////////////////
		_lastSessionID = it->sessionId;		
		
#ifdef _ICE_INTERFACE_SUPPORT
		//为什么不使用it-listbegin()是因为有可能正好在Load itemnext
		_currentItemDist=(int)(_itCurrent-listBegin());		
		_mgr.UpdatePlaylistFailStore(this,true,true);
#endif//_ICE_INTERFACE_SUPPORT
		
		/**
		Ticket 18032 suggested:

		To avoid the VSTRM_DEVICE_BUSY errors I would suggest you set the port speed first The available speeds are defined in the index header. Then... set the starting position. The port speed could even be set before the session is loaded. Positioning will set up the session defining the in-point to the current trick file. Changing speed after positioning requires the session setup to be done a second time. 
		Speed is a characteristic of the port. 
		Position is a characteristic of a session. 

		To avoid the VSTRM_DEVICE_BUSY errors, the operation sequence can be: 
			load 
			set speed (port number) 
			set position (session) 
			prime 
			play 
		or 
			set speed (port number) 
			load 
			set position (session) 
			prime 
			play 

		我们以前的调用顺序是:
		load
		reposition
		setspeed
		prime
		play

		现在将使用Ticket 18032的调用顺序

		*/

		if( (!_b1stPlayStarted) && (fabs(newSpeed)<0.01f) )
		{
			glog(ZQ::common::Log::L_DEBUG, PLSESSID(doLoad, "adjust speed to normal for the first play"));
			for( int i = 0 ; i < gStreamSmithConfig.lRetryCountAtBusyChangeSpeed ; i ++ ) {
				glog(ZQ::common::Log::L_DEBUG,PLSESSID(doLoad,"trying to change speed: %d"),i+1);
				if( doChangeSpeed( 1.0f ,true , info ,it->userCtrlNum ,false ) )
					break;
				if( VstrmGetLastError() != VSTRM_DEVICE_BUSY )
					break;
				if( (i + 1) < gStreamSmithConfig.lRetryCountAtBusyChangeSpeed )
					ZQ::common::delay(200);//sleep 200ms and have another try				
			}

			if( gStreamSmithConfig.lLoadItemWithoutNpt > 0 ) {
				for( int i = 0 ; i < gStreamSmithConfig.lRetryCountAtBusyChangeSpeed ; i ++ ) {
					glog(ZQ::common::Log::L_DEBUG,PLSESSID(doLoad,"trying to reposition: %d"),i+1);
					if( repositionAfterLoad(it,timeOffset,info) )
						break;
					if( VstrmGetLastError() != VSTRM_DEVICE_BUSY )
						break;
					if( (i + 1) < gStreamSmithConfig.lRetryCountAtBusyChangeSpeed )
						ZQ::common::delay(200);//sleep 200ms and have another try				
				}
			}
		
			_b1stPlayStarted = true;
		}
		else 
		{		
			for( int i = 0 ; i < gStreamSmithConfig.lRetryCountAtBusyChangeSpeed ; i ++ ) {
				glog(ZQ::common::Log::L_DEBUG,PLSESSID(doLoad,"trying to change speed: %d"),i+1);
				if( doChangeSpeed( newSpeed ,true , info , it->userCtrlNum , false) )
					break;
				if( VstrmGetLastError() != VSTRM_DEVICE_BUSY )
					break;
				if( (i + 1) < gStreamSmithConfig.lRetryCountAtBusyChangeSpeed )
					ZQ::common::delay(200);//sleep 200ms and have another try				
			}
			
			if( gStreamSmithConfig.lLoadItemWithoutNpt > 0 ) {
				for( int i = 0 ; i < gStreamSmithConfig.lRetryCountAtBusyChangeSpeed ; i ++ ) {
					glog(ZQ::common::Log::L_DEBUG,PLSESSID(doLoad,"trying to reposition: %d"),i+1);
					if( repositionAfterLoad(it,timeOffset,info) )
						break;
					if( VstrmGetLastError() != VSTRM_DEVICE_BUSY )
						break;
					if( (i + 1) < gStreamSmithConfig.lRetryCountAtBusyChangeSpeed )
						ZQ::common::delay(200);//sleep 200ms and have another try				
				}
			}

			if (!_b1stPlayStarted )
			{
				 _b1stPlayStarted = true;
			}
		}	

		FireStateChanged( IPlaylist::PLAYLIST_PLAY , true );
		/*if (it->_itemAdsPlayTimes >0)
		{
			it->_itemAdsPlayTimes--;
			glog(ZQ::common::Log::L_INFO,PLSESSID(doLoad,"ad PlayTimes: [%d], flag: [%s]"), it->_itemAdsPlayTimes, convertItemFlagToStr(it->itemFlag).c_str());
		}*/

// 		{//vvx-1150
// 			glog(ZQ::common::Log::L_INFO,PLSESSID(doLoad,"repositio to Encoding point ****************************************"));
// 			//TODO: test code
// 			IOCTL_CONTROL_PARMS controlParam;
// 			memset(&controlParam, 0 ,sizeof(controlParam));
// 
// // 			controlParam.u.reposition.timeOffset			=	VSTRM_REPOSITION_TO_ENCODE_POINT_NPT;
// // 			controlParam.controlCode						=	VSTRM_GEN_CONTROL_REPOSITION;			
// // 			controlParam.u.reposition.timeOffsetType		=	ENCODE_POINT_TIME_OFFSET;
// // 			controlParam.u.reposition.setSpeed				=	FALSE;
// 			controlParam.u.reposition.timeOffset			=	10 * 60 * 60 * 1000;
// 			controlParam.controlCode						=	VSTRM_GEN_CONTROL_REPOSITION;			
// 			controlParam.u.reposition.timeOffsetType		=	ABSOLUTE_TIME_OFFSET;
// 			controlParam.u.reposition.setSpeed				=	FALSE;
// 
// 			VSTATUS repStatus = VstrmClassControlSessionEx(	_mgr.classHandle(),
// 															it->sessionId,
// 															VSTRM_GEN_CONTROL_REPOSITION,
// 															&controlParam,
// 															sizeof(IOCTL_REPOSITION_PARAMS),
// 															NULL,
// 															NULL);
// 			if( repStatus == VSTRM_SUCCESS)
// 			{
// 				ULONG	lDuration = CalculateCurrentTotalTime( );
// 				//get item playtime
// 				glog(ZQ::common::Log::L_INFO,PLSESSID(doload,"seekTOEncodePoint succeeded: item[%s][%lu], timeoffset[%lu], speed[%d/%d] ,totalPlayTime[%lu] "),
// 					it->objectName.string,
// 					it->userCtrlNum,
// 					controlParam.u.reposition.timeOffset ,
// 					controlParam.u.reposition.speedIndicator.numerator,
// 					controlParam.u.reposition.speedIndicator.denominator,
// 					lDuration);
// 			}
// 			else
// 			{
// 				char szErrbuf[1024];
// 				glog(ZQ::common::Log::L_INFO,PLSESSID(doload,"seekTOEncodePoint failed:%s "),_mgr.getErrorText(repStatus,szErrbuf,1023));
// 			}
// 		}

		VSTRMAPICALLSTART(VstrmClassPrimeEx);
		status				=	VstrmClassPrimeEx( _mgr.classHandle(), it->sessionId );
		VSTRMAPICALLEND(VstrmClassPrimeEx);

		if( status != VSTRM_SUCCESS )
		{
			_lastErrCode	=	ERR_PLAYLIST_SERVER_ERROR;
			char szErrbuf[1024];
			ERRLOG(Log::L_ERROR, PLSESSID(doLoad,"VstrmClassPrimeEx() failed for item[%s][%lu] VstrmSess[%lu], error [%s]"),
				it->objectName.string,
				it->userCtrlNum,
				it->sessionId,
				_mgr.getErrorText(VstrmGetLastError(),szErrbuf,1023));
			return false;
		}

		VSTRMAPICALLSTART(VstrmClassPlayEx);
		status = VstrmClassPlayEx ( _mgr.classHandle() , it->sessionId );
		VSTRMAPICALLEND(VstrmClassPlayEx);

		it->stampLaunched = GetTickCount();

		printList("doLoad::");

		if( info.flag & ( GET_NPT_CURRENTPOS | GET_NPT_TOTALPOS | GET_ITEM_CURRENTPOS | GET_ITEM_TOTALPOS | GET_SPEED  ) )
			doGetStreamResultInfo( it , info );

		info.plState		=	IPlaylist::PLAYLIST_PLAY;
		info.userCtrlNum	=	it->userCtrlNum;

		_lastErrCode = ERR_PLAYLIST_SUCCESS;		

		glog(Log::L_INFO, PLSESSID(doLoad,"succeeded on loading item[%s][%lu], api time cost[%lld]"), 
			it->objectName.string, it->userCtrlNum, spanLoad.stop() );
	}
	return ( status == VSTRM_SUCCESS );
}

bool Playlist::repositionAfterLoad( iterator it, ULONG timeoffet , StreamControlResultInfo& info) 
{
	IOCTL_CONTROL_PARMS controlParam;
	memset(&controlParam, 0 ,sizeof(controlParam));

	controlParam.controlCode						=	VSTRM_GEN_CONTROL_REPOSITION;		
	controlParam.u.reposition.timeOffset			=	timeoffet;
	controlParam.u.reposition.timeOffsetType		=	ABSOLUTE_TIME_OFFSET;
	controlParam.u.reposition.setSpeed				=	FALSE;		

	

	VSTRMAPICALLSTART(VstrmClassControlSessionEx);
	VSTATUS repStatus = VstrmClassControlSessionEx(	_mgr.classHandle(),
													it->sessionId,
													VSTRM_GEN_CONTROL_REPOSITION,
													&controlParam,
													sizeof(IOCTL_REPOSITION_PARAMS),
													NULL,
													NULL);
	VSTRMAPICALLEND(VstrmClassControlSessionEx);
	if( repStatus == VSTRM_SUCCESS)
	{
		ULONG	lDuration = CalculateCurrentTotalTime( );
		//get item playtime
		glog(ZQ::common::Log::L_INFO,PLSESSID(doload,"repositionAfterLoad succeeded:sess[%u] item[%s][%lu], timeoffset[%lu], speed[%d/%d] ,totalPlayTime[%lu] "),
			it->sessionId,
			it->objectName.string,
			it->userCtrlNum,
			controlParam.u.reposition.timeOffset ,
			controlParam.u.reposition.speedIndicator.numerator,
			controlParam.u.reposition.speedIndicator.denominator,
			lDuration);
		info.timeOffset = controlParam.u.reposition.timeOffset;
		return true;
	}
	else
	{
		char szErrbuf[1024]={0};
		glog(ZQ::common::Log::L_ERROR,PLSESSID(doload,"repositionAfterLoad failed:sess[%u] item[%s][%lu], err %s "),
			it->sessionId,
			it->objectName.string,
			it->userCtrlNum,
			_mgr.getErrorText(repStatus,szErrbuf,1023));
		return false;
	}
}

bool Playlist::doSeekStreamCommand(  iterator it , IN LONG timeOffset ,  IN float newSpeed , OUT StreamControlResultInfo& info )
{
	if( !iterValid(it) )
	{
		_lastErrCode = ERR_PLAYLIST_INVALID_PARA;
		ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(SeekStream,"invalid item specified to seek to"));
		return false;
	}
	_lastExtErrCode = 0;

	//ok find the Item,calculate the offset inside the item
	long itemOffset = (long)timeOffset;
	glog(ZQ::common::Log::L_INFO, PLSESSID(SeekStream,"located the item[%s][%lu] and Offset[%ld] startpos[SEEK_POS_BEGIN]"),
		it->objectName.string, it->userCtrlNum, itemOffset );

	if( isFFScale( newSpeed ) && !checkResitriction( it , PLISFlagNoFF ) )
	{		
		glog(ZQ::common::Log::L_WARNING,PLSESSID(SeekStream,"adjust newspeed[%f] due to restriction[%s]"),
			newSpeed,convertItemFlagToStr(_itCurrent).c_str() );
		newSpeed = 1.0;
	}
	else if( isREWScale(newSpeed) && !checkResitriction( it , PLISFlagNoRew ))
	{
		_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
		ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(SeekStream,"reject seek with speed[%f] due to restriction[%s]"),
			newSpeed, convertItemFlagToStr( it).c_str() );
		return false;
	}

	if(  iterValid(_itCurrent) && it == _itCurrent )
	{
		if( _itCurrent->sessionId != 0 )
			return doReposition( 1 , itemOffset , newSpeed , info );
		else
		{
			iterator itDummy = listStart();
			bool bFirstLoadOK =true;
			for( ; it != listEnd() ; it ++ )
			{
				if(!bFirstLoadOK)
				{
					itemOffset = 0;
				}
				if (IsItemPlaytimesCountZero(it))
				{	
					glog(ZQ::common::Log::L_INFO,PLSESSID(SeekStream, "doSeekStreamCommand1 skip item for playtimes = 0"));
					continue;
				}
				if( doLoad( it , newSpeed , itemOffset , info ))
				{
					_itCurrent = it;
					if( isReverseStreaming() )
					{
						_itNext=_itCurrent>listStart() ? _itCurrent-1 : listStart();
					}
					else
					{
						_itNext=_itCurrent<listEnd() ? _itCurrent+1 : listEnd();
					}
					OnItemStepped(itDummy , it ,itemOffset, "UserRequestedSeek",0,&info);
					return true;
				}
				else
				{
					bFirstLoadOK = false;
				}
			}
			glog(ZQ::common::Log::L_ERROR,PLSESSID(SeekStream, "failed to load items, reached the playlist end"));
			_itCurrent = listEnd();
			OnPlaylistDone();						
			return false;						
		}
	}
	else
	{
		iterator itTempCur = listStart();
		iterator itTempNext = listStart();
		if( iterValid(_itCurrent) )
		{
			itTempCur = _itCurrent->sessionId == 0 ? listStart() : _itCurrent ;
		}
		if( iterValid(_itNext) )
		{
			itTempNext = _itNext->sessionId == 0 ? listStart() : _itNext;
		}
		bool bFirstOK = true;

		unloadEx( itTempNext  );
		unloadEx( itTempCur  );

		for( ; it != listEnd() ; it ++)
		{
			if(!bFirstOK)
			{
				itemOffset = 0;
			}	
			if (IsItemPlaytimesCountZero(it))
			{
				glog(ZQ::common::Log::L_INFO,PLSESSID(SeekStream, "doSeekStreamCommand2 skip item for playtimes = 0"));
				continue;
			}
			if( doLoad( it , newSpeed , itemOffset , info ) )
			{	

				OnItemStepped(itTempCur , it , itemOffset, "UserRequestedSeek",0,&info);

				_itCurrent = it;
				if( isReverseStreaming() )
				{
					_itNext=_itCurrent>listStart() ? _itCurrent-1 : listStart();
				}
				else
				{
					_itNext=_itCurrent<listEnd() ? _itCurrent+1 : listEnd();
				}

				return true;
			}
			else
			{							
				bFirstOK = false;
			}						
		}
		glog(ZQ::common::Log::L_ERROR,PLSESSID(doSeekStream,"failed to load items, reached the playlist end"));
		_itCurrent = listEnd();
		_itNext = listEnd();
		OnPlaylistDone();					
		return false;					
	}
}

bool Playlist::jumpValidation( iterator itFrom , iterator itTo, ULONG fromNpt, ULONG toNpt)
{
	if( !( iterValid(itFrom) && iterValid(itTo) ) )
		return true;//allow to jump

	if( !_b1stPlayStarted )
		return true;// BUGID 16514

	if( ( !checkResitriction(itFrom , PLISFlagNoFF ) && ( toNpt > fromNpt ) ) 
		||( !checkResitriction(itFrom ,PLISFlagNoRew) && (toNpt < fromNpt) ) )
	{
		_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
		ERRLOG(ZQ::common::Log::L_WARNING,PLSESSID(jumpValidation,"reject jump command due to:fromNpt[%lu] toNpt[%lu] currentFlag[%s]"),
			fromNpt, toNpt, convertItemFlagToStr(itFrom).c_str() );
		return false;
	}

	iterator itA = min(itFrom, itTo), itB =max(itFrom, itTo); //ticket#16138
	for( iterator it = itA ; it <= itB ; it ++ ) {
		if(!checkResitriction(it,PLISFlagNoSeek))
		{
			_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
			ERRLOG(ZQ::common::Log::L_WARNING,PLSESSID(jumpValidation,"reject jump command due to:fromNpt[%lu] toNpt[%lu] currentFlag[%s]"),
					fromNpt, toNpt, convertItemFlagToStr(it).c_str() );
			return false;
		}
	}		 

	glog(ZQ::common::Log::L_INFO,PLSESSID(jumpValidation,"accept jump command due to:fromNpt[%lu] toNpt[%lu] currentFlag[%s]"),
			fromNpt, toNpt, convertItemFlagToStr(itFrom).c_str() );
	return true;
}

bool Playlist::doSeekStream( IN LONG timeOffset , IN short from , IN float newSpeed , OUT StreamControlResultInfo& info )
{
	long offset = static_cast<long>( timeOffset );
	ZQ::common::MutexGuard gd(_listOpLocker);
	glog(ZQ::common::Log::L_INFO,PLSESSID(SeekStream,"seeking in playlist with offset[%ld] and from[%d]") , offset , from );
	CtrlNum	targetCtrlNum = -2; //initialize to invalid control number	

	//When the current item is restricted FF, if a request tries to seek FROM this item to some NPT AFTER the current offset, please also reject via 403
	//So is FR, if tries to seek FROM this item to an npt BEFORE the current offset, please reject	

	LONG	backupOffset = timeOffset ;

	switch (from)
	{
	case IPlaylist::SEEK_POS_BEGIN:
	{
		List::iterator it;
		for( it =listBegin() ; it!= listEnd(); it++)
		{	
				// there is unnecessary to check attributes if timeOffset ==0
			if ( 0 != timeOffset && ( isLastItem(it) || it->_itemPlaytime <= 0 ) )
			{
				if(!checkContentAttribute(it))
				{
					ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID( SeekStream , "failed to query item[%s][%lu] for attributes"),
						it->_rawItemName, it->userCtrlNum);
					return false;
				}				
			}
			else
			{
				glog(ZQ::common::Log::L_INFO,PLSESSID(SeekStream,"take cached attributes of item[%s][%lu]: playtime[%ld]ms, bitrate[%ld]"),
					it->_rawItemName,it->userCtrlNum,it->_itemPlaytime,it->_itemBitrate);
			}
			if (IsItemPlaytimesCountZero(it))
			{
				offset -= it->_itemPlaytime;
				offset = (offset < 0 ? 0 : offset);
				glog(ZQ::common::Log::L_INFO,PLSESSID(SeekStream,"SEEK_POS_BEGIN skip item for playtimes = 0"));
				continue;
			}
			if( offset < static_cast<long>( it->_itemPlaytime ))
			{
				if(!jumpValidation(_itCurrent,it,_lastplaylistNpt,backupOffset))
					return false;
				return doSeekStreamCommand( it , offset , newSpeed , info );
			}
			offset -= it->_itemPlaytime;			
		}
		iterator itLast = lastItem();
		if( iterValid(itLast) && itLast->_bPWE )
		{
			if(!jumpValidation(_itCurrent,it,_lastplaylistNpt,backupOffset))
				return false;
			glog(ZQ::common::Log::L_INFO, PLSESSID(SeekStream,"lastItem[%s][%lu] is in PWE mode, seek to [%ld]ms"),
				itLast->_rawItemName, itLast->userCtrlNum , offset );
			return doSeekStreamCommand( itLast , offset , newSpeed , info );
		}
		else if( offset <= gStreamSmithConfig.lRepositioInaccuracy )
		{
			if(!jumpValidation(_itCurrent,it,_lastplaylistNpt,backupOffset))
				return false;

			glog(ZQ::common::Log::L_INFO, PLSESSID(SeekStream,"[%ld]ms exceeded the playlist, with inaccuracy windows [%d]ms, adjusting to the last item"),
				offset,gStreamSmithConfig.lRepositioInaccuracy);
			it = listEnd() - 1;
			offset =it->_itemPlaytime - 1;			
			return doSeekStreamCommand( it , offset , newSpeed , info );
		}

		ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(SeekStream,"out of range, invalid offset [%ld] from[%d]"), backupOffset , from );
		_lastErrCode	= ERR_PLAYLIST_INVALID_PARA;
		_lastExtErrCode = EXT_ERRCODE_INVALID_RANGE;
		return false;
	}

		break;

	case IPlaylist::SEEK_POS_END:
		{
		if(offset > 0)
		{
			ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(SeekStream,"invalid offset [%ld] specified: must be negative with SEEK_POS_END"), offset );
			_lastErrCode= ERR_PLAYLIST_INVALID_PARA;
		}
		List::iterator it =listEnd();
		it--;

		offset = -offset;
		
		for( ; it != listStart() ; it-- )
		{
			if (IsItemPlaytimesCountZero(it))
			{
				glog(ZQ::common::Log::L_INFO,PLSESSID(SeekStream,"SEEK_POS_END1 skip item for playtimes = 0"));
				continue;
			}
			if( it + 1 == listEnd() || it->_itemPlaytime <= 0 )
			{
				if( !checkContentAttribute(it) )
				{
					ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID( SeekStream , "failed to query item [%lu][%s] for attributes"),
						it->userCtrlNum , it->_rawItemName );
					return false;
				}				
			}
			else
			{
				glog(ZQ::common::Log::L_INFO,PLSESSID(SeekStream,"take cached attributes of item[%s][%lu]: playtime[%ld]ms, bitrate[%ld]"),
					it->_rawItemName,it->userCtrlNum,it->_itemPlaytime,it->_itemBitrate);
			}

			if( offset < it->_itemPlaytime )
			{
				long itemOffset = (long)offset;
				glog(ZQ::common::Log::L_INFO, PLSESSID(SeekStream,"found the item[%s][%lu] with offset[%ld] startPos[SEEK_POS_END]"),
					it->objectName.string, it->userCtrlNum, itemOffset );
				
				if( iterValid(_itCurrent) && ( it == _itCurrent) )
				{
					if( _itCurrent->sessionId != 0 )
					{
						if( isREWScale(newSpeed) && !checkResitriction(_itCurrent,PLISFlagNoRew))
						{
							_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
							ERRLOG( Log::L_ERROR, PLSESSID(doSeekStream, "reject seek due to[%s]"),
								convertItemFlagToStr(_itCurrent).c_str() );
							return false;
						}
						return doReposition( from , -itemOffset , newSpeed , info );
					}
					else
					{
						if( it->outTimeOffset != 0 )
						{
							itemOffset = it->outTimeOffset - itemOffset;
						}
						else
						{
							itemOffset = it->_itemPlaytime - itemOffset;
						}
						itemOffset = (long)(itemOffset > (long)it->inTimeOffset ? itemOffset : it->inTimeOffset);
						iterator itDummy = listStart();
						bool bFirstOk = true;
						for( ; it != listEnd() ; it ++ )
						{
							if(!bFirstOk)
							{
								itemOffset = 0;
							}
							if( isREWScale(newSpeed) && !checkResitriction(it,PLISFlagNoRew))
							{
								_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
								ERRLOG( Log::L_ERROR, PLSESSID(doSeekStream, "reject seek due to[%s]"),
									convertItemFlagToStr(it).c_str() );
								return false;
							}
							if (IsItemPlaytimesCountZero(it))
							{
								glog(ZQ::common::Log::L_INFO,PLSESSID(SeekStream,"SEEK_POS_END2 skip item for playtimes = 0"));
								continue;
							}
							if( doLoad( it , newSpeed ,itemOffset , info))
							{
								OnItemStepped( itDummy , it , itemOffset, "UserRequestedSeek");

								_itCurrent = it;
								if( isReverseStreaming() )
								{
									_itNext=_itCurrent>listStart() ? _itCurrent-1 : listStart();
								}
								else
								{
									_itNext=_itCurrent<listEnd() ? _itCurrent+1 : listEnd();
								}
								return true;
							}
							else
							{
								bFirstOk = false;
							}
						}
						glog(ZQ::common::Log::L_ERROR,PLSESSID(doSeekStream,"failed to load items, reached the playlist end" ) );
						_itCurrent = listEnd();
						OnPlaylistDone();						
						
						return false;						
					}
				}
				else
				{
					iterator itTempCur = _itCurrent;
					iterator itTempNext = _itNext;
					if( it->outTimeOffset != 0 )
					{
						itemOffset = it->outTimeOffset - itemOffset;
					}
					else
					{
						itemOffset = it->_itemPlaytime - itemOffset;
					}
					
					if( isREWScale(newSpeed) && !checkResitriction(it,PLISFlagNoRew))
					{
						_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
						ERRLOG( Log::L_ERROR, PLSESSID(doSeekStream, "reject seek due to[%s]"),
							convertItemFlagToStr(it).c_str() );
						return false;
					}

					unloadEx( itTempNext );
					unloadEx( itTempCur   );

					itemOffset = itemOffset > (long)it->inTimeOffset ? itemOffset : it->inTimeOffset;
					bool bFirstOK = true;
					for( ; it != listEnd() ; it ++ )
					{
						if(!bFirstOK)
						{
							itemOffset = 0;
						}
						if (IsItemPlaytimesCountZero(it))
						{
							glog(ZQ::common::Log::L_INFO,PLSESSID(SeekStream,"SEEK_POS_END3 skip item for playtimes = 0"));
							continue;
						}
						if( doLoad( it , newSpeed , itemOffset , info ))
						{	
							
							OnItemStepped( itTempCur , it , itemOffset, "UserRequestedSeek");

							_itCurrent = it;
							if( isReverseStreaming() )
							{
								_itNext=_itCurrent>listStart() ? _itCurrent-1 : listStart();
							}
							else
							{
								_itNext=_itCurrent<listEnd() ? _itCurrent+1 : listEnd();
							}

							return true;
						}
						else
						{							
							bFirstOK = false;
						}
					}
					glog(ZQ::common::Log::L_ERROR,PLSESSID(doSeekStream,"failed to load items, reached the playlist end" ));
					_itCurrent = listEnd();
					_itNext = listEnd();
					OnPlaylistDone();	
					return false;					
				}		
			
			}
			offset -= it->_itemPlaytime;
		}
		ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(SeekStream,"out of range, invalid offset [%ld] from[%d]"),backupOffset,from);
		_lastErrCode = ERR_PLAYLIST_INVALID_PARA;
		_lastExtErrCode = EXT_ERRCODE_INVALID_RANGE;
		return false;
	}
		break;

	case IPlaylist::SEEK_POS_CUR:
		if (0 == timeOffset)
			return doPlay(0, newSpeed, info);

		// currently seek from SEEK_POS_CUR is not well supported, leave any non-zero timeOffset as unsupported
		// so, no "break;" here

	default:
		ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(SeekStream,"invalid parameter, unsupported parameter: startPos[%d], offset[%ld]") , from, timeOffset);
		_lastErrCode	= ERR_PLAYLIST_INVALID_PARA;
		_lastExtErrCode = EXT_ERRCODE_INVALID_RANGE;

		return false;
	}
}

bool Playlist::exPlayItem( IN CtrlNum to , IN float newSpeed ,
							IN ULONG timeOffset , IN short pos , 
							OUT StreamControlResultInfo& info  )
{
	ZQ::common::MutexGuard gd(_listOpLocker);
	if( _bCleared )
	{
		_lastErrCode = ERR_PLAYLIST_INVALID_PARA;
		ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(PlayItem,"playlist is destroyed, reject play command"));
		return false;
	}
	glog(ZQ::common::Log::L_INFO, PLSESSID(PlayItem,"entering with: ctrlNum[%u] timeOffset[%lu] beginPos[%d] speed[%f]"),
		to , timeOffset , pos ,newSpeed );
	iterator it = findUserCtrlNum(to);
	if( iterValid(it) )
	{
		_lastplaylistNpt	= getCurrentPlaylistNpt();
		_lastItemNpt		= getCurrentItemNpt();

		if( iterValid(_itCurrent) && ( it == _itCurrent ) &&  _itCurrent->sessionId != 0 )
		{		
			if(  checkContentAttribute(it) && doReposition( pos, timeOffset , newSpeed ,info ) )
			{
				primeNext();				
			}
			else
			{
				return false;
			}
		}
		else
		{
			if( it->_itemPlaytime <= 0 || it->_bPWE || isLastItem(it) )
			{
				if( !checkContentAttribute(it) )
				{
					_lastErrCode= ERR_PLAYLIST_INVALID_PARA;
					ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(PlayItem,"failed to query item[%s][%lu] for attributes"),
						it->_rawItemName,it->userCtrlNum);
					return false;
				}				
			}

			if ( pos == IPlaylist::SEEK_POS_BEGIN)
			{
				//timeOffset += it->inTimeOffset;
			}
			else if ( pos == IPlaylist::SEEK_POS_END )
			{
				if( it->outTimeOffset != 0 )
				{
					timeOffset = it->outTimeOffset + timeOffset;
				}
				else
				{
					timeOffset = it->_itemPlaytime + timeOffset;
				}
			}			
			iterator itPrev = listStart();
			if( iterValid(_itNext) )
			{
				if( _itNext->sessionId != 0 )
				{
					unloadEx( _itNext ,false );
				}
			}
			if( iterValid(_itCurrent) )
			{
				if( _itCurrent->sessionId != 0 )
				{
					itPrev = _itCurrent;
					unloadEx(_itCurrent,false);
				}
			}
			bool bFirstOk	= true;
			bool bLoadOk	= false;
			for( ; it != listEnd() && it != listStart() && !bLoadOk;  isReverseStreaming() ? it-- : it ++ )
			{
				bool bSeek = true;
				if(!bFirstOk)
				{
					bSeek = false;
					timeOffset = 0;
				}
				if( it->_itemPlaytime <= 0 || it->_bPWE || isLastItem(it) )
				{
					if( !checkContentAttribute(it) )
					{
						_lastErrCode= ERR_PLAYLIST_INVALID_PARA;
						ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(PlayItem,"failed to query item[%s][%lu] for attributes"),
							it->_rawItemName,it->userCtrlNum);
						return false;
					}				
				}
				if( doLoad( it , newSpeed , timeOffset , info , bSeek ) )
				{
					_itCurrent = it;

					if( isReverseStreaming() )
					{
						_itNext=_itCurrent>listStart() ? _itCurrent-1 : listStart();
					}
					else
					{
						_itNext=_itCurrent<listEnd() ? _itCurrent+1 : listEnd();
					}
					OnItemStepped( itPrev , it , timeOffset, "UserRequestedSeek");
					
					primeNext( );
					glog(ZQ::common::Log::L_INFO, PLSESSID(SeekTo,"relocated to item[%s][%u] timeOffset[%lu] beginPos[%d]"),
						it->objectName.string, to, timeOffset , pos );
					bLoadOk = true;
				}
				else
				{
					bFirstOk = false;
				}
			}
			if( !bLoadOk )
			{
				_itCurrent = listEnd();
				OnPlaylistDone();
				glog(ZQ::common::Log::L_ERROR,PLSESSID(PlayItem,"reached the playlist end"));
				return false;			
			}
		}

		if( info.flag & GET_NPT_CURRENTPOS )
		{
			info.timeOffset = info.timeOffset + CalculatePastTime( );
		}
		if( info.flag & GET_NPT_TOTALPOS )
		{
			info.totalOffset = CalculatePastTime() + CalculateCurrentTotalTime() + CalculateFutureTime();
		}
		if( info.flag & GET_ITEM_TOTALPOS )
		{
			info.itemTotalOffset = CalculateCurrentTotalTime();
		}
		return true;
	}
	else
	{
		_lastErrCode	=	ERR_PLAYLIST_INVALID_PARA;
		ERRLOG(ZQ::common::Log::L_ERROR , PLSESSID(PlayItem , "failed to find item with ctrlNum[%u]"), to );
		return false;
	}	
}

bool Playlist::SeekTo( CtrlNum to , long timeOffset , IPlaylist::SeekStartPos pos )
{
	ZQ::common::MutexGuard gd(_listOpLocker);
	glog(ZQ::common::Log::L_INFO, PLSESSID(SeekTo,"entering with ctrlNum[%u] timeOffset[%ld] beginPos[%d]"),
		to , timeOffset , pos );
	iterator it = findUserCtrlNum(to);
	if( iterValid(it) )
	{
		/*SEEK_POS_CUR,
		SEEK_POS_BEGIN,
		SEEK_POS_END*/
		if( iterValid(_itCurrent) && ( it == _itCurrent ) && _itCurrent->sessionId != 0 )
		{			
			StreamControlResultInfo info;
			if( checkContentAttribute(it) && doReposition( pos, timeOffset , 0.0f ,info ) )
			{
				primeNext();
				return true;
			}
			else
			{
				false;
			}
		}
		else
		{
			if( it->_itemPlaytime <= 0 || isLastItem(it) )
			{
				if( !checkContentAttribute(it) )
				{
					_lastErrCode= ERR_PLAYLIST_INVALID_PARA;
					ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(SeekStream,"query item[%s][%lu] ,get playtime and bitrate failed"),
						it->_rawItemName,it->userCtrlNum);
					return false;
				}				
			}

			if ( pos == IPlaylist::SEEK_POS_BEGIN)
			{
				//timeOffset += it->inTimeOffset;
			}
			else if ( pos == IPlaylist::SEEK_POS_END )
			{
				if( it->outTimeOffset != 0 )
				{
					timeOffset = it->outTimeOffset + timeOffset;
				}
				else
				{
					timeOffset = it->_itemPlaytime + timeOffset;
				}
			}
			StreamControlResultInfo info;
			iterator itPrev = listStart();
			if( iterValid(_itNext) )
			{
				if( _itNext->sessionId != 0 )
				{
					unloadEx( _itNext ,false );
				}
			}
			if( iterValid(_itCurrent) )
			{
				if( _itCurrent->sessionId != 0 )
				{
					itPrev = _itCurrent;
					unloadEx(_itCurrent,false);
				}
			}
			bool bFirstOk = true;
			for( ; it != listEnd() ; it ++ )
			{
				bool bSeek = true;
				if(!bFirstOk)
				{
					bSeek = false;
					timeOffset = 0;
				}
				if( doLoad( it , 0.0f ,timeOffset , info , bSeek ) )
				{
					_itCurrent = it;

					if( isReverseStreaming() )
					{
						_itNext=_itCurrent>listStart() ? _itCurrent-1 : listStart();
					}
					else
					{
						_itNext=_itCurrent<listEnd() ? _itCurrent+1 : listEnd();
					}
					OnItemStepped( itPrev , it , timeOffset, "UserRequestedSeek");
					primeNext();
					glog(ZQ::common::Log::L_INFO, PLSESSID(SeekTo,"relocated to item[%s][%u] timeOffset[%ld] beginPos[%d]"),
						it->objectName.string, to, timeOffset , pos );
					return true;
				}
				else
				{
					bFirstOk = false;
				}
			}
			_itCurrent = listEnd();
			OnPlaylistDone();
			glog(ZQ::common::Log::L_ERROR,PLSESSID(SeekTo,"reached the playlist end"));
			return false;			
		}
	}
	else
	{
		_lastErrCode	=	ERR_PLAYLIST_INVALID_PARA;
		ERRLOG(ZQ::common::Log::L_ERROR , PLSESSID(SeekTo , "failed to locate item with ctrlNum[%u]"), to );
		return false;
	}
	return false;
}

bool Playlist::doReposition( IN short from , IN LONG timeOffset , IN float newSpeed ,  INOUT StreamControlResultInfo& info )
{
	ZQ::common::MutexGuard gd(_listOpLocker);
	glog(ZQ::common::Log::L_INFO ,PLSESSID( doReposition,"entering with timeOffset[%u] speed[%f] from[%d]") , timeOffset, newSpeed, from );
	if( isCompleted() || (!iterValid(_itCurrent)) )
	{
		_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
		ERRLOG( Log::L_ERROR, PLSESSID(doReposition, "no VstrmSess is running") );
		DumpListInfo( );
		return false;
	}
	endTimer64( );
	if ( _itCurrent->itemFlag & PLISFlagNoSeek ) 
	{
		_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
		ERRLOG(ZQ::common::Log::L_ERROR, PLSESSID(doReposition , "item[%s][%lu] with restriction flag[%s], reject seek command" ),
			_itCurrent->objectName.string ,
			_itCurrent->userCtrlNum ,
			convertItemFlagToStr(_itCurrent).c_str() );
		return false;
	}
	
	iterator it = _itCurrent;

	long	uBitRate = 0 ;
	long	uTotalPlayTime = 0 ;
	long	uTotalDuration = 0 ;
	bool	bPWE = false;

// 	if(!checkContentAttribute(it))
// 	{
// 		ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID( doReposition , "failed to query item[%s][%lu] for attributes"),it->_rawItemName, it->userCtrlNum);
// 		return false;
// 	}
	uBitRate = it->bitrate;
	uTotalPlayTime = it->_itemPlaytime;
	uTotalDuration = it->_itemRealTotalTime;
	bPWE = it->_bPWE;

	glog(ZQ::common::Log::L_INFO , PLSESSID(doReposition,"got item[%s][%lu] attribute: duration[%ld] playtime[%ld] bitrate[%ld]"),
		it->objectName.string,it->userCtrlNum ,
		uTotalDuration , uTotalPlayTime , uBitRate );	

	LONG	targetTimeOffset		= 0;
	LONG	absoluteOutTimeOffset	= uTotalDuration;

	//calculate the real offset
	switch( from )
	{
	case 0://from current
		{
			_lastErrCode	=	ERR_PLAYLIST_SERVER_ERROR;
			ERRLOG( ZQ::common::Log::L_ERROR , PLSESSID( doReposition , "operation forbidden with from[0]" ) );
			return false;
		}
		break;
	case 1://from begin
		{
			targetTimeOffset = timeOffset + it->inTimeOffset;
			if( it->outTimeOffset != 0 )
			{
				absoluteOutTimeOffset = (ULONG)uTotalDuration > it->outTimeOffset ? it->outTimeOffset : (ULONG)uTotalDuration;
			}
			targetTimeOffset = targetTimeOffset > (LONG)it->inTimeOffset ? targetTimeOffset : it->inTimeOffset;
			glog(ZQ::common::Log::L_INFO, PLSESSID(doReposition,"repositioning from beginning, adjusting to target timeOffset[%lu] in the item[%s][%lu], with"
				" totalDuration[%ld] inTimeOffset[%lu] outTimeOffset[%lu]"),
				targetTimeOffset, 
				it->objectName.string,
				it->userCtrlNum,				
				uTotalDuration,
				it->inTimeOffset,
				it->outTimeOffset);
		}
		break;
	case 2://from end
		{			
			if( it->outTimeOffset != 0 )
			{
				absoluteOutTimeOffset = (ULONG)uTotalDuration > it->outTimeOffset ? it->outTimeOffset : (ULONG)uTotalDuration;
			}
			targetTimeOffset = absoluteOutTimeOffset + timeOffset;
			targetTimeOffset = targetTimeOffset > (LONG)it->inTimeOffset ? targetTimeOffset : it->inTimeOffset;
			glog(ZQ::common::Log::L_INFO, PLSESSID(doReposition,"repositioning from end, adjusting to target timeOffset[%lu] in the item[%s][%lu], with"
				" totalDuration[%ld] inTimeOffset[%lu] outTimeOffset[%lu]"),
				targetTimeOffset, 
				it->objectName.string,
				it->userCtrlNum,				
				uTotalDuration,
				it->inTimeOffset,
				it->outTimeOffset);
		}
		break;
	default:
		{
			_lastErrCode = ERR_PLAYLIST_INVALID_PARA;
			ERRLOG(ZQ::common::Log::L_ERROR, PLSESSID(doReposition,"unkown paramter: from[%d]"), from);
			return false;
		}
		break;
	}

	if(	isLastItem(it) )		
	{//If current item is the last item in 
		if( it->_bPWE )
		{//do nothing
		}
		else if( _bEnableEOT )
		{
			if(gStreamSmithConfig.lForceNormalTimeBeforeEnd > 0 &&			
				memcmp( &_crntSpeed , &SPEED_NORMAL , sizeof(SPEED_NORMAL) ) != 0 &&
				( targetTimeOffset > ( absoluteOutTimeOffset - gStreamSmithConfig.lForceNormalTimeBeforeEnd ) ) )
			{//current speed is not NORMAL SPEED and EOTSize > 0 and of course 
				targetTimeOffset = absoluteOutTimeOffset - gStreamSmithConfig.lForceNormalTimeBeforeEnd ;
				targetTimeOffset = targetTimeOffset > (LONG)it->inTimeOffset ? targetTimeOffset : it->inTimeOffset;
				glog(ZQ::common::Log::L_INFO, PLSESSID(doReposition,"hit the EOT protection: item[%s][%lu], EOTSize[%d],inTimeOffset[%lu], outTimeOffset[%lu]; adjusting to new timeOffset[%ld]"),
					it->objectName.string,it->userCtrlNum,
					gStreamSmithConfig.lForceNormalTimeBeforeEnd,
					it->inTimeOffset , it->outTimeOffset ,targetTimeOffset );
			}
			if( gStreamSmithConfig.lForceNormalTimeBeforeEnd > 0  )
			{
				if( (newSpeed > 0.01f) || ( fabs(newSpeed-0.00f)<0.01f  && _crntSpeed.numerator > 0 ) )
				{
					if( targetTimeOffset > absoluteOutTimeOffset - gStreamSmithConfig.lForceNormalTimeBeforeEnd )
					{

						targetTimeOffset = absoluteOutTimeOffset - gStreamSmithConfig.lForceNormalTimeBeforeEnd;
						glog(ZQ::common::Log::L_INFO, PLSESSID(doReposition,"hit the EOT protection: item[%s][%lu], EOTSize[%d],inTimeOffset[%lu], outTimeOffset[%lu]; adjusting to new timeOffset[%ld]"),
							it->objectName.string,it->userCtrlNum,
							gStreamSmithConfig.lForceNormalTimeBeforeEnd,
							it->inTimeOffset , it->outTimeOffset ,targetTimeOffset );
					}
				}
			}
		}
	}	
	if( !(isLastItem(it) && it->_bPWE) && gStreamSmithConfig.lPreloadTime > 0 )
	{
		if( fabs(newSpeed - 0.00f) < 0.001f )
		{	
			if( _crntSpeed.numerator > 0 )
			{
				if( targetTimeOffset > (absoluteOutTimeOffset - gStreamSmithConfig.lPreloadTime) )
				{
					targetTimeOffset = absoluteOutTimeOffset - gStreamSmithConfig.lPreloadTime;
				}
			}
			if( _crntSpeed.numerator < 0 )
			{
				if( targetTimeOffset < gStreamSmithConfig.lPreloadTime )
				{
					targetTimeOffset = gStreamSmithConfig.lPreloadTime + it->inTimeOffset;
				}
			}
		}
		else
		{
			if( newSpeed > 0.01f )
			{
				if( targetTimeOffset > (absoluteOutTimeOffset - gStreamSmithConfig.lPreloadTime) )
				{
					targetTimeOffset = absoluteOutTimeOffset - gStreamSmithConfig.lPreloadTime;
				}
			}
			if( newSpeed < -0.01f )
			{
				if( targetTimeOffset < gStreamSmithConfig.lPreloadTime )
				{
					targetTimeOffset = gStreamSmithConfig.lPreloadTime + it->inTimeOffset;
				}
			}
		}				
	}
	if( !(isLastItem(it) && it->_bPWE) )
	{
		targetTimeOffset = targetTimeOffset < (LONG)it->inTimeOffset ? it->inTimeOffset : targetTimeOffset;	
		targetTimeOffset = targetTimeOffset > (LONG)absoluteOutTimeOffset ? absoluteOutTimeOffset : targetTimeOffset;
	}

	glog(ZQ::common::Log::L_INFO, PLSESSID(doReposition,"hit EOI protection: item[%s][%lu], preloadTime[%d], totalDuration[%ld]; adjusting timeoffset to[%ld]"),
		it->objectName.string,it->userCtrlNum,
		 gStreamSmithConfig.lPreloadTime , uTotalDuration ,targetTimeOffset	);

	iterator itNext = _itNext;
	if ( iterValid(itNext) /*( itNext != listEnd() ) && ( itNext != listStart() )*/ )
	{
		if ( itNext->sessionId != 0 ) 
		{							
			glog(Log::L_INFO,PLSESSID(doReposition,"unloading the preloaded next item[%s][%lu]"),
				itNext->objectName.string, itNext->userCtrlNum);
			if( !unloadEx( itNext ) )
			{
				_lastErrCode = ERR_PLAYLIST_SERVER_ERROR;
				ERRLOG(Log::L_ERROR , PLSESSID( doReposition , "failed to unload item[%s][%lu]" ),
					itNext->objectName.string, itNext->userCtrlNum );
			}
		}
	}

	bool bChangeParameter = false;
	if( targetTimeOffset != 0 && !checkResitriction( _itCurrent , PLISFlagNoSeek ))
	{
		bChangeParameter = true;
		targetTimeOffset = 0;
	}
	if( isFFScale( newSpeed ) && !checkResitriction( _itCurrent, PLISFlagNoFF ) ||
		isREWScale( newSpeed ) && !checkResitriction( _itCurrent , PLISFlagNoRew ))
	{
		bChangeParameter = true;
		newSpeed = 1.0;
	}
	if( bChangeParameter)
	{
		glog(ZQ::common::Log::L_WARNING,PLSESSID(doReposition,"adjust timeoffset[%ld] speed[%f] due to restriction[%s]"),
			targetTimeOffset , newSpeed , convertItemFlagToStr(_itCurrent).c_str() );
	}

	int lastSegmentNpt = getCurrentItemNpt();

	IOCTL_CONTROL_PARMS controlParam;
	memset(&controlParam, 0 ,sizeof(controlParam));
	controlParam.controlCode						=	VSTRM_GEN_CONTROL_REPOSITION;		
	controlParam.u.reposition.timeOffset			=	targetTimeOffset;
	controlParam.u.reposition.timeOffsetType		=	ABSOLUTE_TIME_OFFSET;

	controlParam.u.reposition.setSpeed				=	fabs( newSpeed ) > 0.01f ;
	
	
	int tmpNumerator = (int)(newSpeed * 1000.0f);
	int tmpDenominator = 1000;
	convertSpeed( tmpNumerator , tmpDenominator );
	SPEED_IND	speed;
	speed.numerator		= tmpNumerator;
	speed.denominator	= tmpDenominator;

	controlParam.u.reposition.speedIndicator	=	speed;
	
	//forward
	controlParam.u.playSpeed.direction =  newSpeed > 0.0f ? VSTRM_FORWARD :	VSTRM_REVERSE;

	glog(ZQ::common::Log::L_INFO, PLSESSID(doReposition,"repositioning to item[%s][%lu] at timeoffset[%ld] newSpeed[%f]"),
		it->objectName.string, it->userCtrlNum, targetTimeOffset, newSpeed);
	if ( (tmpNumerator != _crntSpeed.numerator) || (tmpDenominator != _crntSpeed.denominator) )
	{
		_stampSpeedChangeReq = GetTickCount();
		glog(ZQ::common::Log::L_INFO,PLSESSID(doReposition,"_stampSpeedChangeReq[%u]"),
			_stampSpeedChangeReq);
	}

	VSTRMAPICALLSTART(VstrmClassControlSessionEx);
	VSTATUS status = VstrmClassControlSessionEx(	_mgr.classHandle(),
												_itCurrent->sessionId,
												VSTRM_GEN_CONTROL_REPOSITION,
												&controlParam,
												sizeof(IOCTL_REPOSITION_PARAMS),
												NULL,
												NULL);
	VSTRMAPICALLEND(VstrmClassControlSessionEx);

	if(	status != VSTRM_SUCCESS )
	{		
		char szErrbuf[1024];
		ZeroMemory(szErrbuf,1024);
		_lastErrCode=ERR_PLAYLIST_SERVER_ERROR;
		ERRLOG(Log::L_ERROR,PLSESSID(doReposition,"VstrmClassControlSessionEx(VSTRM_GEN_CONTROL_REPOSITION) failed, error [%s]"),
			_mgr.getErrorText(VstrmGetLastError(),szErrbuf,1023));
		return false;
	}
	else
	{
		updateOpTime();

		_isReverseStreaming = 	controlParam.u.reposition.speedIndicator.numerator < 0 ;

		if( isReverseStreaming() )
		{
			_itNext=iterValid(_itCurrent) ? _itCurrent-1 : listStart();
		}
		else
		{
			_itNext=iterValid(_itCurrent) ? _itCurrent+1 : listEnd();
		}
		glog(ZQ::common::Log::L_INFO,PLSESSID(doReposition,"repostion succeeded: item[%s][%lu], timeoffset[%lu], speed[%d/%d] "),
				it->objectName.string,
				it->userCtrlNum,
				controlParam.u.reposition.timeOffset ,
				controlParam.u.reposition.speedIndicator.numerator,
				controlParam.u.reposition.speedIndicator.denominator);

		info.plState		=	IPlaylist::PLAYLIST_PLAY;
		info.userCtrlNum	=	_itCurrent->userCtrlNum;		
		info.itemTimeOffset = info.timeOffset		=	controlParam.u.reposition.timeOffset - _itCurrent->inTimeOffset;
		
		if( info.timeOffset < 0 )
		{
			info.timeOffset			= 0;
			info.itemTimeOffset		= 0;
		}
		info.speed			=	convertSPEEDtoFloat( controlParam.u.reposition.speedIndicator );

		onItemReposition( it , lastSegmentNpt ,info.itemTimeOffset , &info );

		FireStateChanged( IPlaylist::PLAYLIST_PLAY );

		primeNext();
		updateTimer();

		DisplaySessionAttr( _itCurrent );
		_lastErrCode=ERR_PLAYLIST_SUCCESS;
		DumpListInfo(true);
		glog(ZQ::common::Log::L_INFO,PLSESSID(doReposition,"done"));
		return true;
	}
}


bool Playlist::unloadEx( iterator it , bool bReset )
{
	
	ZQ::common::Guard < ZQ::common::Mutex > guard(_listOpLocker,__MSGLOC__);

	if ( !iterValid(it) /*it <= listStart() || it >= listEnd()*/)
	{
		glog(Log::L_DEBUG,PLSESSID(unloadEx,"playlist item iterator is out of range, skip"));
		return false;
	}
	glog(Log::L_INFO,PLSESSID(unloadEx,"entering with item[%s][%lu], VstrmSess[%lu]"),
				it->objectName.string, it->userCtrlNum, it->sessionId);
	if (it->sessionId ==0)
	{
		glog(Log::L_DEBUG,PLSESSID(unloadEx,"item[%s][%lu] is not loaded yet,skip"),
					it->objectName.string, it->userCtrlNum);
		return false; // haven't been loaded yet
	}



	StreamControlResultInfo info;
	if( GAPPLICATIONCONFIGURATION.lPauseWhenUnloadItem >= 1 )
	{
		doPause(false,info);
	}
	try
	{
// 		glog(Log::L_DEBUG,PLSESSID(unloadEx,"Unload item with CtrlNum=%u and fileName=%s and Vstrm SessionId=%u"),
// 									it->userCtrlNum,it->objectName.string,it->sessionId);

		bool bUnloadSuccessfully = false;
#ifdef VSTRM_REPOSITION_STOP_NPT
		
		int delayUnloadInterval = gStreamSmithConfig.lRetryIntervalAtUnload;
		if( delayUnloadInterval != 0 )
		{
			delayUnloadInterval = delayUnloadInterval < 100 ? delayUnloadInterval : 100;
			delayUnloadInterval = delayUnloadInterval > 10000 ? 10000 : delayUnloadInterval;
		}

		if( gStreamSmithConfig.lUseRepositionWhenUnload >= 1 )
		{
			for( int i = 0 ; !bUnloadSuccessfully && i < 2 ; i++ )
			{
			IOCTL_CONTROL_PARMS controlParam;
			memset(&controlParam, 0 ,sizeof(controlParam));


			controlParam.u.reposition.timeOffset			=	VSTRM_REPOSITION_STOP_NPT;

			controlParam.controlCode						=	VSTRM_GEN_CONTROL_REPOSITION;			
			controlParam.u.reposition.timeOffsetType		=	ABSOLUTE_TIME_OFFSET;
			controlParam.u.reposition.setSpeed				=	FALSE;

			VSTRMAPICALLSTART(VstrmClassControlSessionEx);
			VSTATUS status = VstrmClassControlSessionEx(	_mgr.classHandle(),
														it->sessionId,
														VSTRM_GEN_CONTROL_REPOSITION,
														&controlParam,
														sizeof(IOCTL_REPOSITION_PARAMS),
														NULL,
														NULL);
			VSTRMAPICALLEND(VstrmClassControlSessionEx);

			if( VSTRM_SUCCESS != status)
			{
				_ntstatus 	= GetLastError ();		// capture secondary error code (if any)
				char	szErrbuf[1024];
				ZeroMemory(szErrbuf,1024);
				_lastErrCode = ERR_PLAYLIST_SERVER_ERROR;
				ERRLOG(Log::L_ERROR,PLSESSID(unloadEx,"VstrmClassControlSessionEx(VSTRM_GEN_CONTROL_REPOSITION) failed ,error [%s], item[%s][%u]" ),
						_mgr.getErrorText( status,szErrbuf,1023), it->objectName.string, it->userCtrlNum );
				if( delayUnloadInterval == 0 )
					break;
				if( VSTRM_INVALID_SESSION != status  )
				{
					Sleep( (i>= 1) ? 200 : delayUnloadInterval );
				}
				else
				{
					bUnloadSuccessfully = true;
				}
			}
			else
			{
				bUnloadSuccessfully = true;
				glog(Log::L_INFO, PLSESSID(unloadEx,"VstrmClassControlSessionEx(VSTRM_GEN_CONTROL_REPOSITION) successfully unload item[%s][%u], VstrmSess[%u]"),
					it->objectName.string, it->userCtrlNum, it->sessionId);					
			}
			}
		}		
#endif
		{
			if( !bUnloadSuccessfully )
			{	
			VSTATUS status = VSTRM_SUCCESS;

			VSTRMAPICALLSTART(VstrmClassUnloadEx);
			if( ( status = VstrmClassUnloadEx (_mgr.classHandle(), it->sessionId)) != VSTRM_SUCCESS )
			{
				VSTRMAPICALLEND(VstrmClassUnloadEx);
				_ntstatus 	= GetLastError ();		// capture secondary error code (if any)
				char	szErrbuf[1024];
				ZeroMemory(szErrbuf,1024);
				_lastErrCode = ERR_PLAYLIST_SERVER_ERROR;
				ERRLOG(Log::L_ERROR,PLSESSID(unloadEx,"VstrmClassUnloadEx() failed ,error [%s], item[%s][%lu]" ),
					_mgr.getErrorText( status, szErrbuf, 1023), it->objectName.string, it->userCtrlNum );
			}
			else
			{
				VSTRMAPICALLEND(VstrmClassUnloadEx);
				glog(Log::L_INFO,PLSESSID(unloadEx,"VstrmClassUnloadEx() successfully unload item[%s][%lu], VstrmSess[%lu]"),
					it->objectName.string, it->userCtrlNum, it->sessionId);
				try
				{
					//send the item done message
					//				OnItemDone(it);
				}
				catch (...)
				{
				}			
			}
			}
		}
	}
	catch(...) 
	{
		glog(Log::L_ERROR,PLSESSID(unloadEx,"caught unknown exception"));
	}

	it->sessionId = 0;

	// stamp the execution
	it->stampUnload = GetTickCount();

	//set it to 0 for future 
	it->stampLaunched =0;
	// make the un-played element loadable in the future
	if (it->stampLaunched ==0)
	{
		it->sessionId = 0;
		it->stampLoad = 0; 
	}
	_lastErrCode=ERR_PLAYLIST_SUCCESS;
	
	glog(Log::L_INFO, PLSESSID(unloadEx,"item[%s][%lu] unloaded"),it->objectName.string, it->userCtrlNum);

	if( bReset && ( GAPPLICATIONCONFIGURATION.lPauseWhenUnloadItem >= 1 ) )
	{
		doResume(info , true );
	}
	return true;
}


bool Playlist::isLaunched(iterator it)
{
	if (it->sessionId ==0)
	{
		it->stampLaunched =0;
		return false;
	}
	
	return (it->stampLaunched >0);
}

Playlist::iterator Playlist::findItem(const ULONG userCtrlNum, const_iterator from)
{
	ZQ::common::Guard < ZQ::common::Mutex > guard(_listOpLocker,__MSGLOC__);
	
	iterator it ;
	for ( it = (from < listBegin() ? listBegin() : from); it < listEnd(); it++)
	{
		if (it->userCtrlNum == userCtrlNum)
			break;
	}
	
	return it;
}

const bool Playlist::getItemInfo(const_iterator it, Item& elem)
{
	if (it < listBegin() || it >= listEnd())
		return false;

	ZQ::common::Guard < ZQ::common::Mutex > guard(_listOpLocker,__MSGLOC__);
	memcpy(&elem, &(*it), sizeof(Item));
	return true;
}

Playlist::iterator Playlist::findInternalCtrlNum(const ULONG intlCtrlNum)
{
	ZQ::common::Guard < ZQ::common::Mutex > guard(_listOpLocker,__MSGLOC__);

	iterator it = listEnd();
	for (  it = listBegin(); it != listEnd(); it++)
	{
		if (it->intlCtrlNum == intlCtrlNum)
			break;
	}
	
	return it;
}

Playlist::iterator Playlist::findUserCtrlNum(const CtrlNum userCtrlNum)
{
	ZQ::common::Guard < ZQ::common::Mutex > guard(_listOpLocker,__MSGLOC__);
	iterator it = listEnd();
	for ( it = listBegin(); it < listEnd(); it++)
	{
		if (it->userCtrlNum == userCtrlNum)
			break;
	}
	
	return it;
}

#define FABS(x)	(x>0 ? x :-x )

//Who call this function must lock the playlist item
bool Playlist::checkVstrmRestrictionArea( const float& newScale , TIME_OFFSET& timeLeft )
{
	if( fabs(newScale - 0.00f) <= 0.001f || _crntSpeed.denominator == 0 )
		return true;

	float curScale = (float)_crntSpeed.numerator /(float)_crntSpeed.denominator;

	if(  (curScale * newScale > 0.001f) && (FABS(newScale) < FABS(curScale)) )
		return true;

	LONGLONG			tmpPendingDataSize = 0;
	ULONG				tmpBitRate = 0;
	LONGLONG			tmpObjectSize = 0;
	LONGLONG			tmpPlayOffset = 0;
	TIME_OFFSET			tmpPlayoutTimeoffset = 0;
	TIME_OFFSET			tmpEOSTimeoffset = 0;		
	TIME_OFFSET			tmpRestTime = 0;
	timeLeft = 0;
	ESESSION_CHARACTERISTICS esessInfo;
	if(  ! getVstrmSessInfo( _itCurrent , esessInfo ) )
	{
		tmpPlayoutTimeoffset	= _itCurrent->timeOffset;			
		tmpPlayOffset			= _itCurrent->byteOffset;
		tmpObjectSize			= _itCurrent->byteOffsetEOS;
		tmpBitRate				= _itCurrent->bitrate;
		tmpPendingDataSize		= _itCurrent->pendingDataSize;
		if ( tmpObjectSize != 0 && tmpBitRate != 0  )
		{
			//只有在当前是FR的时候tmpRestTime才会被用上
			if ( _itCurrent->speed.numerator < 0  )
			{
				tmpRestTime			=  tmpPlayoutTimeoffset;
			}
			else
			{
				tmpRestTime			=  (TIME_OFFSET)(( tmpObjectSize - tmpPlayOffset )* 8000 / tmpBitRate);
			}
		}			
	}			
	else
	{
		SESSION_CHARACTERISTICS* pInfo = &( esessInfo.SessionCharacteristics );
		tmpPlayoutTimeoffset	= pInfo->PlayoutTimeOffset;
		tmpPlayOffset			= pInfo->ByteOffset.QuadPart;
		tmpObjectSize			= pInfo->ObjectSize.QuadPart;
		tmpBitRate				= pInfo->MuxRate;
		tmpRestTime				= (pInfo->EndTimeOffset - pInfo->PlayoutTimeOffset);
		tmpPendingDataSize		= pInfo->PendingDataSize;
	}
	if( tmpBitRate <= 0 || _crntSpeed.numerator == 0  )
	{
		timeLeft = 0;
		glog(ZQ::common::Log::L_ERROR, PLSESSID(checkVstrmRestrictionArea,"invalid biterate[%lu] or current speed[%d/%d]"), 
			tmpBitRate , _crntSpeed.numerator , _crntSpeed.denominator );
		return true;
	}

	//calculate the real time left
	timeLeft = (TIME_OFFSET)(FABS(( tmpObjectSize - tmpPlayOffset ) * 8000 * _crntSpeed.denominator / tmpBitRate / _crntSpeed.numerator));
	TIME_OFFSET pendingLeft = (TIME_OFFSET)(tmpPendingDataSize * 8000 / tmpBitRate );

	timeLeft += pendingLeft;
	
	if( timeLeft <= (TIME_OFFSET)gStreamSmithConfig.lProtectionWindow )
	{
		glog(ZQ::common::Log::L_INFO, PLSESSID(checkVstrmRestrictionArea,"enter vstrm restriction area: item[%s][%u] byteOffset[%llu] objectSize[%llu] pendingDataSize[%lu] time[%lu] "),
			_itCurrent->objectName.string, _itCurrent->userCtrlNum,
			tmpPlayOffset, tmpObjectSize, tmpPendingDataSize , timeLeft	);
		return false;
	}
	else
	{
		glog(ZQ::common::Log::L_INFO, PLSESSID(checkVstrmRestrictionArea," vstrm session information: item[%s][%u] byteOffset[%llu] objectSize[%llu] pendingDataSize[%lu] time[%lu] "),
			_itCurrent->objectName.string, _itCurrent->userCtrlNum,
			tmpPlayOffset, tmpObjectSize, tmpPendingDataSize , timeLeft	);
	}
	return true;
}

bool Playlist::CanSetSpeed ( const SPEED_IND newspeed )
{
	if( _currentStatus == PLAYLIST_STOP || _currentStatus == PLAYLIST_SETUP )
	{
		_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
		ERRLOG( Log::L_ERROR , PLSESSID( CanSetSpeed , "invalid state[%d] of playlist to change speed" ), _currentStatus);
		return false;
	}
	if ( newspeed.numerator == 0 ) 
	{
		_lastErrCode = ERR_PLAYLIST_INVALID_PARA;
		ERRLOG( Log::L_ERROR , PLSESSID( CanSetSpeed , "illegal newspeed with numerator[%d]" ) , newspeed.numerator  );
		return false;
	}
	
	
	if( _currentStatus != PLAYLIST_PAUSE )
	{
		if( !iterValid(_itCurrent) )
		{
			_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
			ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(CanSetSpeed,"invalid item state of playlist to change speed"));
			return false;
		}
		if( gStreamSmithConfig.lProtectionWindow <= 0  )
		{//If PreLoad time is 0 or less 
			glog(Log::L_DEBUG, PLSESSID(CanSetSpeed ,"skip validating as configuration pretection area[%d] <= 0" ), gStreamSmithConfig.lProtectionWindow );
			return true;
		}

		ULONG				tmpBitRate = 0;
		LONGLONG			tmpObjectSize = 0;
		LONGLONG			tmpPlayOffset = 0;
		TIME_OFFSET			tmpPlayoutTimeoffset = 0;
		TIME_OFFSET			tmpEOSTimeoffset = 0;		
		TIME_OFFSET			tmpRestTime = 0;

		ESESSION_CHARACTERISTICS esessInfo;
		if(  ! getVstrmSessInfo( _itCurrent , esessInfo ) )
		{
			tmpPlayoutTimeoffset	= _itCurrent->timeOffset;			
			tmpPlayOffset			= _itCurrent->byteOffset;
			tmpObjectSize			= _itCurrent->byteOffsetEOS;
			tmpBitRate				= _itCurrent->bitrate;
			if ( tmpObjectSize != 0 && tmpBitRate != 0  )
			{
				//只有在当前是FR的时候tmpRestTime才会被用上
				if ( _itCurrent->speed.numerator < 0  )
				{
					tmpRestTime			=  tmpPlayoutTimeoffset;
				}
				else
				{
					tmpRestTime			=  (TIME_OFFSET)(( tmpObjectSize - tmpPlayOffset )* 8000 / tmpBitRate);
				}
			}			
		}			
		else
		{
			SESSION_CHARACTERISTICS* pInfo = &( esessInfo.SessionCharacteristics );
			tmpPlayoutTimeoffset	= pInfo->PlayoutTimeOffset;
			tmpPlayOffset			= pInfo->ByteOffset.QuadPart;
			tmpObjectSize			= pInfo->ObjectSize.QuadPart;
			tmpBitRate				= pInfo->MuxRate;
			
			tmpRestTime			= pInfo->EndTimeOffset - pInfo->PlayoutTimeOffset;
			
		}
		char playlist_status_str[256];
		snprintf(playlist_status_str, sizeof(playlist_status_str) -2, "item[%s][%lu], timeOffset[%lu], objsize[%lld], byteOffset[%lld], bitrate[%lu], inTimeOffset[%lu], speed[%d/%d]",
			_itCurrent->objectName.string,
			_itCurrent->userCtrlNum,
			tmpPlayoutTimeoffset ,
			tmpObjectSize , tmpPlayOffset , tmpBitRate,
			_itCurrent->inTimeOffset ,
			_crntSpeed.numerator , _crntSpeed.denominator );

		int		GuardTimeMulti = 1;
		int		timeOff = 0;
		int		timeOff1 = 0 , timeOff2 = 0;		
	
		if( tmpBitRate != 0 && tmpObjectSize != 0 )
		{			
			if ( !isReverseStreaming () ) 
			{//forward
				if (newspeed.numerator > 0 ) 
				{					
					//gStreamSmithConfig.lProtectionWindow * GuardTimeMulti
					if( ( newspeed.numerator /newspeed.denominator) > 
						(_crntSpeed.numerator / _crntSpeed.denominator ))
					{
						timeOff  = (int) ( ((tmpObjectSize - tmpPlayOffset) *8*1000) / (tmpBitRate));
					}
					else
					{
						timeOff = gStreamSmithConfig.lProtectionWindow * GuardTimeMulti + 1;
						glog(ZQ::common::Log::L_INFO,PLSESSID(CanSetSpeed,"valid speed change to[%d/%d] at %s"),
							newspeed.numerator,newspeed.denominator,playlist_status_str);
					}
					
					if( (_itCurrent + 1 == listEnd()) && (timeOff <= (gStreamSmithConfig.lProtectionWindow * GuardTimeMulti)) )
					{
						glog(ZQ::common::Log::L_INFO,PLSESSID(CanSetSpeed,"accept new speed[%d/%d] for the last item at %s"),
							newspeed.numerator,newspeed.denominator,playlist_status_str);
						timeOff = gStreamSmithConfig.lProtectionWindow * GuardTimeMulti + 1;						
					}
				
				}
				else
				{

					timeOff = (int)( (LONGLONG)( tmpPlayoutTimeoffset - _itCurrent->inTimeOffset ) * _crntSpeed.denominator /_crntSpeed.numerator );
					
					if( (_itCurrent - 1 == listStart()) && (timeOff <= (gStreamSmithConfig.lProtectionWindow * GuardTimeMulti)) )
					{
						glog(ZQ::common::Log::L_INFO,PLSESSID(CanSetSpeed,"accept new speed[%d/%d] for the first item at %s"),
							newspeed.numerator,newspeed.denominator,playlist_status_str);
						timeOff = gStreamSmithConfig.lProtectionWindow * GuardTimeMulti + 1;
					}
				}
			}
			else
			{//backward
				//get current item total playtime so that I can calculate the rest play time
				if ( _itCurrent->_itemPlaytime <  0  ||  ( _itCurrent + 1 == listEnd () ) ) 
				{
					glog(ZQ::common::Log::L_DEBUG, PLSESSID(CanSetSpeed , "query playtime for item[%s][%lu]" ),
						_itCurrent->_rawItemName , _itCurrent->userCtrlNum );

					if(!checkContentAttribute(_itCurrent))
					{
						_lastErrCode =  ERR_PLAYLIST_SERVER_ERROR;
						ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(CanSetSpeed,"failed to get attribute of item[%s][%lu] "
								"playtime and bitrate failed"),
								_itCurrent->_rawItemName, _itCurrent->userCtrlNum);
						return false;
					}
				}
				long tmpRestTime1 = _itCurrent->_itemPlaytime - tmpPlayoutTimeoffset + _itCurrent->inTimeOffset;				

				glog(ZQ::common::Log::L_INFO, PLSESSID(CanSetSpeed,"got item[%s][%lu] total playtime[%ld] at %s"),
					_itCurrent->objectName.string , _itCurrent->userCtrlNum , _itCurrent-> _itemPlaytime, playlist_status_str);
								
				if ( newspeed.numerator > 0  ) 
				{	
					timeOff = (int) ( (LONGLONG)tmpRestTime * ( _crntSpeed.denominator ) / ( -_crntSpeed.numerator ) );

					if( (_itCurrent + 1 == listEnd()) && (timeOff <= (gStreamSmithConfig.lProtectionWindow * GuardTimeMulti)) 	)
					{
						glog(ZQ::common::Log::L_INFO,PLSESSID(CanSetSpeed,"accept new speed[%d/%d] for the last item at %s"),
							newspeed.numerator,newspeed.denominator,playlist_status_str);
						timeOff = gStreamSmithConfig.lProtectionWindow * GuardTimeMulti + 1;
					}
				}
				else
				{
					
					timeOff = (int) ( (LONGLONG)( tmpPlayoutTimeoffset - _itCurrent->inTimeOffset ) * ( _crntSpeed.denominator ) / ( -_crntSpeed.numerator ) );

					if( (_itCurrent - 1 == listStart()) && (timeOff <= (gStreamSmithConfig.lProtectionWindow * GuardTimeMulti)) )
					{
						glog(ZQ::common::Log::L_INFO, PLSESSID(CanSetSpeed,"accept new speed[%d/%d] for the first item at %s"),
							newspeed.numerator,newspeed.denominator,playlist_status_str);
						timeOff = gStreamSmithConfig.lProtectionWindow * GuardTimeMulti + 1;
					}
				}

			}
		}
		else
		{					
			if( size() <= 1 )
			{
				glog( ZQ::common::Log::L_INFO, PLSESSID(CanSetSpeed,"allow speed change in single-item-Playlist by ingoring bitrate[%lu] objectSize[%lld]"),
					tmpBitRate , tmpObjectSize);
				timeOff = gStreamSmithConfig.lProtectionWindow * GuardTimeMulti + 1;
			}
			else
			{
				glog( ZQ::common::Log::L_WARNING, PLSESSID(CanSetSpeed,"reject speed change in multi-item-Playlist for illegal bitrate[%lu] objectSize[%lld]"),
						tmpBitRate , tmpObjectSize);
				timeOff = gStreamSmithConfig.lProtectionWindow * GuardTimeMulti - 1;
			}
		}
		
		
		{			
			//check if current item is the last item and EOT is enabled
			if( !isCompleted() && //not complete
				_bEnableEOT && //enable EOT
				_itCurrent+1 == listEnd() )//正好处在最后一个Itemn并且是在PauseTV状态,并且是正向播放,并且EnableEOT为TRUE
			{
				if (	newspeed.numerator > 0 &&
						newspeed.numerator > newspeed.denominator  ) 
				{//fast forward ?
					//how to deal with it
					if ( _crntSpeed.denominator == 0 || newspeed.numerator == 0 ) 
					{
						_lastErrCode = ERR_PLAYLIST_INVALID_PARA;
						ERRLOG( Log::L_ERROR , PLSESSID(CanSetSpeed , "invalid target speed[%d/%d]" ),
							/*_crntSpeed.numerator , _crntSpeed.denominator ,*/ newspeed.numerator , newspeed.denominator );
						return false;							
					}
					
					int  targetTime = (int)( ((LONGLONG)timeOff) * FABS(_crntSpeed.numerator) / _crntSpeed.denominator * newspeed.denominator / FABS(newspeed.numerator));
					if (  targetTime < gStreamSmithConfig.lProtectionWindow ) 
					{
						_lastErrCode = ERR_PLAYLIST_INVALIDSTATE ; 
						ERRLOG( Log::L_ERROR, PLSESSID( CanSetSpeed , "hit EOT protection area, reject change speed to [%d/%d]: %s"),							
							newspeed.numerator , newspeed.denominator ,
							playlist_status_str );

						return false;
					}
				}
			}			
		}

		//glog( Log::L_DEBUG , PLSESSID( CanSetSpeed ," now timeoff = [%d] ") , timeOff );
		if( timeOff < ( gStreamSmithConfig.lProtectionWindow * GuardTimeMulti ) || _itCurrent->sessionId==0 || tmpBitRate ==0 )
		{
			_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
			ERRLOG( Log::L_ERROR, PLSESSID(CanSetSpeed, "reject speed change in EOI: [%d] , %s"), timeOff, playlist_status_str );
			return false;
		}
		else
		{
			glog(Log::L_INFO, PLSESSID(CanSetSpeed, "accept changing speed to [%d/%d]"),
				newspeed.numerator , newspeed.denominator );
		}
	}
	else
	{		
		glog(Log::L_INFO , PLSESSID(CanSetSpeed,"current state is PAUSE, accept changing speed" ) );
	}

	return true;
}


class TempUnlocker
{
public:
	TempUnlocker(ZQ::common::Mutex& m)
		:mMutex(m)
	{
		mMutex.leave();
	}
	TempUnlocker::~TempUnlocker()
	{
		mMutex.enter();
	}
private:
	ZQ::common::Mutex& mMutex;
};

bool	Playlist::doChangeSpeed( IN float newScale ,  IN bool bForeceChangeSpeed , OUT StreamControlResultInfo& info , CtrlNum curItem ,bool bUpdateTimer )
{
	ZQ::common::MutexGuard gd(_listOpLocker);
	glog(ZQ::common::Log::L_INFO , PLSESSID(doChangeSpeed,"entering with speed[%f]" ),newScale );
	if( isCompleted() || (!iterValid(_itCurrent)) )
	{
		_lastErrCode=ERR_PLAYLIST_INVALIDSTATE;
		ERRLOG(Log::L_ERROR,PLSESSID(setSpeedEx,"invalid state of playlist, reject to change speed"));
		return false;
	}
	iterator itOp = findUserCtrlNum(curItem);
	if( !iterValid(itOp))
		itOp = _itCurrent;

	if( fabs(newScale) < 0.01 )
	{
		glog(ZQ::common::Log::L_NOTICE , PLSESSID(doChangeSpeed,"ignore new speed[%f]" ), newScale );
		if( bForeceChangeSpeed )
			return true;//do not get stream information if bForeceChangeSpeed == true
		else
			return doGetStreamResultInfo( itOp , info);		
	}
	
	int tmpNumerator	=  (int)(newScale*1000.0);
	int tmpDenominator	= 1000;

	convertSpeed( tmpNumerator ,  tmpDenominator);

	SPEED_IND newspeed;
	newspeed.numerator		= tmpNumerator;
	newspeed.denominator	= tmpDenominator;

	if ( ( !bForeceChangeSpeed ) )
	{
		if( ( _currentStatus == IPlaylist::PLAYLIST_PLAY && ( newspeed.denominator==_crntSpeed.denominator && newspeed.numerator==_crntSpeed.numerator ) ) )
		{
			glog(Log::L_DEBUG,PLSESSID(setSpeedEx,"new speed is the same as current speed, skip"));		
			return doGetStreamResultInfo( itOp , info);		
		}
		else if( !(( _currentStatus == IPlaylist::PLAYLIST_PAUSE ) || ( _currentStatus == IPlaylist::PLAYLIST_PLAY )) )
		{
			_lastErrCode=ERR_PLAYLIST_INVALIDSTATE;
			ERRLOG(Log::L_ERROR,PLSESSID(setSpeedEx,"invalid state[%d] of playlist, reject to change speed"), _currentStatus );
			return false;
		}
	}
	
	if( (isFFScale(newScale) && !checkResitriction(itOp ,PLISFlagNoFF )) ||
		 isREWScale(newScale) && !checkResitriction( itOp , PLISFlagNoRew ))
	{
		_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
		ERRLOG(ZQ::common::Log::L_ERROR, PLSESSID(setSpeedEx ,"item[%s][%lu] , newSpeed[%d/%d] with restriction flag[%s], reject speed change" ),
			itOp->objectName.string , itOp->userCtrlNum , 
			newspeed.numerator ,newspeed.denominator,
			convertItemFlagToStr( itOp ).c_str() );
		return false;
	}

	_stampSpeedChangeReq = GetTickCount();
	glog(ZQ::common::Log::L_INFO , PLSESSID(doChangeSpeed,"_stampSpeedChangeReq[%u]" ),_stampSpeedChangeReq );
	bool	bEndTimer=false;
	if(!bForeceChangeSpeed)
	{
		//if ( !CanSetSpeed(newspeed)) 
		TIME_OFFSET timeLeft = 0;
		if( !checkVstrmRestrictionArea( newScale , timeLeft) )
		{			
			{	
				TempUnlocker unlocker(_listOpLocker);	
				glog(Log::L_INFO,PLSESSID(setSpeedEx,"enter vstrm protection area, sleep [%d]"), timeLeft + gStreamSmithConfig.vstrmRelativeConf.sessionCallbackTimeout );
				Sleep( timeLeft + gStreamSmithConfig.vstrmRelativeConf.sessionCallbackTimeout );
			}
			if( isCompleted() || (!iterValid(_itCurrent)) || PLAYLIST_STOP == _currentStatus )
			{
				_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
				ERRLOG(Log::L_ERROR,PLSESSID(setSpeedEx,"invalid state of playlist, reject to change speed"));
				return false;
			}
			itOp = _itCurrent;						
		}
		endTimer64();
		bEndTimer=true;		
	}

#if NEW_SETSPEEDEX_LOGIC
	iterator itNeedUnload = listEnd();
#endif

	if( ( _crntSpeed.numerator * 	newspeed.numerator ) <  0 )
	{
		iterator itCurLoadedItem = findUserCtrlNum(curItem);
		if( _itNext != listStart() && 
			_itNext != listEnd() && 
			itCurLoadedItem != _itNext &&
			_itNext->sessionId != 0  &&
			gStreamSmithConfig.lPreloadTime > 0 )
		{

#if NEW_SETSPEEDEX_LOGIC			
			itNeedUnload = _itNext;
#else
			unloadEx(_itNext,false);		
#endif 
		}
		else
		{
#if NEW_SETSPEEDEX_LOGIC
			itNeedUnload = listEnd();
#endif
		}
	}
	else
	{
#if NEW_SETSPEEDEX_LOGIC
		itNeedUnload = listEnd();
#endif
	}
#if VER_PRODUCTVERSION_MAJOR >= 6 &&  (( VER_PRODUCTVERSION_MINOR == 0 && VER_PRODUCTBUILD >= 9207 ) || VER_PRODUCTVERSION_MINOR > 0 )
	static size_t	tsoveripStrLen = strlen("\\vv2\\");
	//if( strncmp( itOp->objectName.string , "\\vv2\\",tsoveripStrLen ) == 0 )
	if(0)
	{
#else
	static size_t	tsoveripStrLen = strlen("\\tsoverip\\");
	if( strncmp( itOp->objectName.string , "\\tsoverip\\",tsoveripStrLen ) == 0 )
	{
#endif

#if 1
		StreamControlResultInfo tempInfo;
		if( _currentStatus != IPlaylist::PLAYLIST_PAUSE )
			if( !doPause( false, tempInfo ) && !bForeceChangeSpeed  )
				return false;
#endif
	}

	VSTATUS					status = VSTRM_SUCCESS;

	IOCTL_CONTROL_PARMS_LONG controlParam;
	memset(&controlParam , 0 , sizeof(controlParam));

	controlParam.u.portSpeed.speedIndicator = newspeed;

	VSTRMAPICALLSTART(VstrmClassControlPortEx1);
	status = VstrmClassControlPortEx1(	_mgr.classHandle(),
										_vstrmPortNum,
										VSTRM_GEN_CONTROL_PORT_SPEED,
										&controlParam,
										sizeof(IOCTL_CONTROL_PARMS_LONG),
										NULL,
										0);
	VSTRMAPICALLEND(VstrmClassControlPortEx1);

	if (status == VSTRM_SUCCESS)
	{	
		updateOpTime();
		
#ifdef VSTRM_PORT_TRANSITION_HAS_VALID_SESSION_ID
		if( controlParam.u.portSpeed.magicNumber == VSTRM_PORT_TRANSITION_HAS_VALID_SESSION_ID )
			checkEffectiveSession( controlParam.u.portSpeed.sessionId );
		if( iterValid(_itCurrent))
			itOp = _itCurrent;
#endif

		_isReverseStreaming = newspeed.numerator < 0;
		// Retrieve the returned actual speed from the driver

		status							= controlParam.NtStatus;		

		SPEED_IND oldSpeed = _crntSpeed;
		_crntSpeed						= controlParam.u.portSpeed.speedIndicator;
		itOp->speed.numerator			= controlParam.u.portSpeed.speedIndicator.numerator;
		itOp->speed.denominator			= controlParam.u.portSpeed.speedIndicator.denominator;		
		itOp->timeOffset				= controlParam.u.portSpeed.timeOffset;

		glog(Log::L_INFO, PLSESSID(setSpeedEx,"successfully changed speed at VstrmPort[%lu]: newSpeed[%d/%d] timeoffset[%lu] vstrmSess[%lu]"),
			_vstrmPortNum , _crntSpeed.numerator , _crntSpeed.denominator , itOp->timeOffset ,itOp->sessionId);



		info.plState	=	IPlaylist::PLAYLIST_PLAY;
		info.userCtrlNum=	itOp->userCtrlNum;
		if( info.flag & ( GET_NPT_CURRENTPOS | GET_ITEM_CURRENTPOS ) )
		{
			info.itemTimeOffset  = info.timeOffset	=	controlParam.u.portSpeed.timeOffset - itOp->inTimeOffset;
			if( info.timeOffset< 0 )
				info.timeOffset = 0;
			if( info.itemTimeOffset < 0 )
				info.itemTimeOffset  = 0;		
		}		

		if( info.flag & GET_SPEED )
			info.speed		=	convertSPEEDtoFloat(controlParam.u.portSpeed.speedIndicator);


#if NEW_SETSPEEDEX_LOGIC
		//setspeed successfully , unload the itNeedUnload item
		unloadEx(itNeedUnload );
#else//NEW_SETSPEEDEX_LOGIC

#endif //NEW_SETSPEEDEX_LOGIC

		//change the playlist state because setspeed ok
		FireStateChanged( IPlaylist::PLAYLIST_PLAY );
		FireSpeedChanged( controlParam.u.portSpeed.sessionId, _crntSpeed, oldSpeed );

		if( !bForeceChangeSpeed )
		{			
			primeNext();
			updateTimer();
		}

		if( bUpdateTimer )		
		{
			updateTimer();
		}

		printList("SetspeedEx");
		_lastErrCode = ERR_PLAYLIST_SUCCESS;
	}
	else
	{
		_ntstatus 		=	GetLastError ();		// capture secondary error code (if any)
		
		_lastErrCode	=	(VstrmGetLastError() == VSTRM_API_DISABLED) ? ERR_PLAYLIST_INVALIDSTATE : ERR_PLAYLIST_SERVER_ERROR;
		
		char	szErrbuf[1024];
		ZeroMemory( szErrbuf , 1024 );
		ERRLOG(Log::L_ERROR, PLSESSID(setSpeedEx,"VstrmClassControlPortEx1(VSTRM_GEN_CONTROL_PORT_SPEED) failed, VstrmSess[%lu] VstrmPort[%lu] error[%s]" ),
			itOp->sessionId, _vstrmPortNum, _mgr.getErrorText(VstrmGetLastError(),szErrbuf,1023) );
	}
	
	glog(Log::L_DEBUG, PLSESSID(setSpeedEx,"Leave SetSpeedEx"));
	return ( status == VSTRM_SUCCESS );
}

bool	Playlist::play( )
{
	StreamControlResultInfo info;
	return doPlay( 0 , 1.0f , info );
}

bool Playlist::doPlay( IN LONG timeOffset , IN float newSpeed , OUT StreamControlResultInfo& info )
{
	ZQ::common::MutexGuard gd( _listOpLocker );

	glog(Log::L_DEBUG,PLSESSID(play,"Enter play"));

	if( _currentStatus == PLAYLIST_PAUSE )
	{//Current playlist is in pause state
		glog( Log::L_DEBUG , PLSESSID(doPlay,"playlist is in PAUSE state,redirect to resume() ") );
		return doResume(info);
	}
	if( _currentStatus == PLAYLIST_PLAY )
	{
		glog(Log::L_DEBUG,PLSESSID( doPlay , "playlist is in PLAY state, skip") );
		
		doGetStreamResultInfo( _itCurrent , info );
		
		return true;
	}
	if( !iterValid(_itCurrent) || size() <= 0 )
	{
		_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
		ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(doPlay,"invalid state of playlist to play"));
		return false;
	}

	_stampStateChangeReq = GetTickCount();

	if( gStreamSmithConfig.lEnablemaxWaitTimeToStartFirstItem >= 1 )
	{
		//glog( ZQ::common::Log::L_DEBUG , PLSESSID( doPlay , "playlist has critical-start item") );

		long lMaxWait = gStreamSmithConfig.lmaxWaitTimeToStartFirstItem;

		//lMaxWait = min(lMaxWait,3000);//Adjust max wait under 3000
		if ( lMaxWait > 3000 ) 
		{
			glog(ZQ::common::Log::L_WARNING, PLSESSID(doPlay,"adjust maxWaitTimeToStartFirstItem [%ld]ms to [3000]ms"), lMaxWait );
			lMaxWait = 3000;
		}
		//只有enable了这个配置项才有用
		if ( lMaxWait > 0 &&  _itCurrent->criticalStart > 0 ) 
		{
			ZQ::common::MutexGuard gd(_listOpLocker);

			//convert UTC time to system time
			SYSTEMTIME	sysCR;
			SYSTEMTIME	sysNow;
			ZQ::common::TimeUtil::Time2SystemTime(_itCurrent->criticalStart,sysCR);
			GetSystemTime(&sysNow);

			FILETIME	ftCR ;
			FILETIME	ftNow;
			SystemTimeToFileTime(&sysCR,&ftCR);
			SystemTimeToFileTime(&sysNow,&ftNow);
			time_t UTCnow = time(NULL);


			ULARGE_INTEGER criticalStart ;
			ULARGE_INTEGER currentTime;
			memcpy(&criticalStart,&ftNow,sizeof(ftNow));
			memcpy(&currentTime,&ftCR,sizeof(ftCR));
			long wait = (long)((currentTime.QuadPart-criticalStart.QuadPart)/10000);

			//如果时间差小于10毫秒就直接执行这个命令
			if (wait>10) 
			{			
				if ( wait > gStreamSmithConfig.lmaxWaitTimeToStartFirstItem/*config*/ ) 
				{
					_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
					ERRLOG(Log::L_ERROR, PLSESSID(doPlay,"reject per the next scheduled criticalStartTime[%s], allowed error time window[%ld]"),
						printfUTCTime(_itCurrent->criticalStart).c_str(), lMaxWait);
					return false;
				}
				glog(ZQ::common::Log::L_INFO, PLSESSID(doPlay,"force to pause [%ld]ms per criticalStartTime [%s] of item [%s][%lu]"),
					wait,printfUTCTime(_itCurrent->criticalStart).c_str(),
					_itCurrent->objectName.string,_itCurrent->userCtrlNum);
				Sleep(wait);
			}
		}
	}
	else
	{
		//glog(ZQ::common::Log::L_DEBUG,PLSESSID( doPlay  , "disable CriticalStartPlayWait" ) );
	}

	//glog(Log::L_DEBUG,PLSESSID(doPlay,"Loadex Item (%d)%s in Play"), _itCurrent->userCtrlNum , _itCurrent->objectName.string);	

	{
		ZQ::common::MutexGuard gd(_listOpLocker);
		LockPrimeNext lock(_bDontPrimeNext);
		
		for( ; _itCurrent<listEnd() && _itCurrent>listStart() ; _itCurrent += (isReverseStreaming() ? -1 : 1) )
		{
			if (IsItemPlaytimesCountZero(_itCurrent))
			{
				glog(Log::L_DEBUG, PLSESSID(doPlay,"ads playtimes= 0, skip, flag: [%s]"), convertItemFlagToStr(_itCurrent->itemFlag).c_str());
				continue;
			}
			if(!checkContentAttribute(_itCurrent))
			{
				glog(ZQ::common::Log::L_WARNING, PLSESSID(doPlay,"failed to get attribute for [%s][%lu]"),
					_itCurrent->_rawItemName, _itCurrent->userCtrlNum );
				timeOffset = 0;
				continue;
			}
			if ( doLoad( _itCurrent , newSpeed , timeOffset, info , false ) )
			{				
				_itNext = isReverseStreaming() ? _itCurrent-1 : _itCurrent+1;
				timeOffset = 0;
				break;
			}
		}		
		if(_itCurrent>=listEnd() || _itCurrent <=listStart())
		{	
			try
			{
				_stampDone = GetTickCount();
				OnPlaylistDone();
				return false;		
			}
			catch(...)
			{
				glog( Log::L_ERROR , PLSESSID( doPlay , "caught unknown exception when invoke OnPlaylistDone ") );
			}
			_lastErrCode = ERR_PLAYLIST_SERVER_ERROR;
			return false;
		}

		
		FireStateChanged(PLAYLIST_PLAY);
		/*if (_itCurrent->_itemAdsPlayTimes > 0)
		{
			_itCurrent->_itemAdsPlayTimes --;
			glog(Log::L_DEBUG, PLSESSID(doPlay, "ads playtimes: [%d], flag: [%s]"), _itCurrent->_itemAdsPlayTimes, convertItemFlagToStr(_itCurrent->itemFlag).c_str());
		}*/
		
		OnItemStepped( listStart() , _itCurrent , timeOffset );
		
		_b1stPlayStarted = true;		

	}

	Sleep(1); // yield for current playing

	updateTimer();
	
	_lastErrCode = ERR_PLAYLIST_SUCCESS;
	
	glog(Log::L_DEBUG,PLSESSID(play,"Leave play _stampStateChangeReq[%u]"), _stampStateChangeReq);

	return true;
}

bool Playlist::pause()
{
	StreamControlResultInfo info;
	return doPause( true , info );
}

bool Playlist::doGetStreamResultInfo( IN iterator it ,   OUT StreamControlResultInfo& info )
{
	if( !iterValid(it))
	{
		glog(ZQ::common::Log::L_DEBUG , PLSESSID(doGetStreamResultInfo,"invalid item iterator, reject") );
		return false;
	}
	//glog(ZQ::common::Log::L_DEBUG , PLSESSID(doGetStreamResultInfo,"get session information with sessionId[%u]") , sessId );

	ESESSION_CHARACTERISTICS sessChar;
	if( !getVstrmSessInfo( it , sessChar ) )
	{
		return false;
	}
	else
	{
		if( info.flag & GET_STATE)
		{
			info.plState	= _currentStatus;
		}
		//if( info.flag & GET_SPEED)
		{
			info.speed		= convertSPEEDtoFloat(sessChar.SessionCharacteristics.Speed);
		}
		//if( info.flag & ( GET_NPT_CURRENTPOS | GET_ITEM_CURRENTPOS ) )
		{
			info.itemTimeOffset =  info.timeOffset	= sessChar.SessionCharacteristics.PlayoutTimeOffset - it->inTimeOffset;
			if( info.timeOffset < 0 )
			{
				info.timeOffset = 0;
				info.itemTimeOffset = 0;
			}
			
		}		
		//if( info.flag & GET_USERCTRLNUM )
		{
			info.userCtrlNum= it->userCtrlNum;
		}
		glog(ZQ::common::Log::L_INFO,PLSESSID(doGetStreamResultInfo,"got session info:ctrlNum[%lu] timeoffset[%lu] , scale[%f]"),
			info.userCtrlNum , info.timeOffset , info.speed );
			
		return true;
	}

	return true;
}

bool	Playlist::doPause( bool bChangeState ,  OUT StreamControlResultInfo& info )
{
	ZQ::common::MutexGuard sync( _listOpLocker );

	glog(Log::L_DEBUG,PLSESSID(doPause,"entering"));

	if( isCompleted() || !iterValid(_itCurrent) )
	{
		_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
		ERRLOG(ZQ::common::Log::L_ERROR, PLSESSID(doPause,"playlist empty, reject pause command"));
		return false;
	}
	//do not perform pause action if Forbid pause is set

	if( !checkResitriction(_itCurrent, PLISFlagNoPause ))
	{
		_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
		ERRLOG(Log::L_ERROR, PLSESSID(doPause,"reject pause command with restriction[%s]"),
			convertItemFlagToStr(_itCurrent).c_str());
		return false;
	}

	if (_currentStatus != IPlaylist::PLAYLIST_PLAY) 
	{
		_lastErrCode=ERR_PLAYLIST_INVALIDSTATE;
		ERRLOG(Log::L_ERROR,PLSESSID(doPause,"playlist is not in PLAY state, reject pause command"));
		return false;
	}

	_stampStateChangeReq = GetTickCount();

	if (_currentStatus == IPlaylist::PLAYLIST_PAUSE) 
	{
		glog(ZQ::common::Log::L_INFO,PLSESSID(doPause,"playlist is in PAUSE state, skip"));
		
		return doGetStreamResultInfo( _itCurrent ,  info );
	}
	glog(ZQ::common::Log::L_INFO, PLSESSID(pause,"pause at VstrmPort[%lu] with item[%s][%lu]"),
		_vstrmPortNum ,_itCurrent->objectName.string, _itCurrent->userCtrlNum);
	
	IOCTL_CONTROL_PARMS_LONG controlParam;
	memset(&controlParam, 0 ,sizeof(controlParam));

	ULONG portNum					= _vstrmPortNum;  // _vstrmPortNum.getPortNum();	
	controlParam.u.pause.timeCode	= TIME_CODE_NOW ;


	VSTRMAPICALLSTART(VstrmClassControlPortEx1);
	VSTATUS status = VstrmClassControlPortEx1(	_mgr.classHandle(),
												portNum,
												VSTRM_GEN_CONTROL_PAUSE,
												&controlParam,
												sizeof(IOCTL_CONTROL_PARMS_LONG),
												NULL,
												0);
	VSTRMAPICALLEND(VstrmClassControlPortEx1);

	if (status == VSTRM_SUCCESS && !isCompleted())
	{
		updateOpTime();
#ifdef VSTRM_PORT_TRANSITION_HAS_VALID_SESSION_ID
		if( controlParam.u.pause.magicNumber == VSTRM_PORT_TRANSITION_HAS_VALID_SESSION_ID )
			checkEffectiveSession( controlParam.u.pause.sessionId );
#endif
		// Retrieve the returned actual speed from the driver
		status					= controlParam.NtStatus;
		_itCurrent->timeOffset	= controlParam.u.pause.timeOffset;
		_crntSpeed				= controlParam.u.pause.speedIndicator;
		
		
		info.plState			= IPlaylist::PLAYLIST_PAUSE;		
		info.speed				= convertSPEEDtoFloat( _crntSpeed );
		info.itemTimeOffset =  info.timeOffset			= controlParam.u.pause.timeOffset - _itCurrent->inTimeOffset;
		if (info.timeOffset <0 )
		{
			info.timeOffset = 0;
			info.itemTimeOffset = 0;
		}
		info.userCtrlNum		= _itCurrent->userCtrlNum;

		
		if ( bChangeState )
		{
			FireStateChanged(PLAYLIST_PAUSE);
		}

		//_currentStatus=PLAYLIST_PAUSE;
		glog(Log::L_INFO, PLSESSID(pause,"VstrmSess[%lu] at VstrmPort[%lu] with item[%s][%lu] "
			"was paused: timeOffset[%lu] speed[%d/%d]"),
			_itCurrent->sessionId,_vstrmPortNum,
			_itCurrent->objectName.string, _itCurrent->userCtrlNum, _itCurrent->timeOffset,
			_crntSpeed.numerator,_crntSpeed.denominator );

		//DisplaySessionAttr(_itCurrent);

		endTimer64();
		_isGoodItemInfoData=true;
		SetTimerEx( gStreamSmithConfig.lPlaylistTimeout );
		if(_mgr._timeout ==(timeout_t)TIMEOUT_INF || (timeout_t)gStreamSmithConfig.lPlaylistTimeout<_mgr._timeout)
			_mgr.wakeup();	
#ifdef _ICE_INTERFACE_SUPPORT
		_currentItemDist=(int)(_itCurrent-listBegin());
		_mgr.UpdatePlaylistFailStore(this,true,true);
#endif//_ICE_INTERFACE_SUPPORT
	}
	else
	{		
		_ntstatus 	= GetLastError ();		// capture secondary error code (if any)
		char	szErrbuf[1024];
		ZeroMemory(szErrbuf,1024);
		_lastErrCode=ERR_PLAYLIST_SERVER_ERROR;
		ERRLOG(Log::L_ERROR, PLSESSID( doPause , "VstrmClassControlPortEx1(VSTRM_GEN_CONTROL_PAUSE) failed at VstrmPort[%lu], item[%s][%lu] error[%s]"),
			_vstrmPortNum,_itCurrent->objectName.string,_itCurrent->userCtrlNum,_mgr.getErrorText(VstrmGetLastError(),szErrbuf,1023));		
	}
	_lastErrCode=ERR_PLAYLIST_SUCCESS;	
	glog(ZQ::common::Log::L_INFO,PLSESSID(doPause,"done, _stampStateChangeReq[%u]"), _stampStateChangeReq);
	return ( status == VSTRM_SUCCESS );
}

bool Playlist::resume( )
{
	StreamControlResultInfo info;
	return doResume( info );
}
bool	Playlist::doResume( OUT StreamControlResultInfo& info , bool bForce )
{
	ZQ::common::MutexGuard gd(_listOpLocker);
	glog( Log::L_DEBUG , PLSESSID(resume , "entering with VstrmPort[%lu]") , _vstrmPortNum );
	if (isCompleted() || !iterValid(_itCurrent) )
	{
		_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
		ERRLOG(Log::L_ERROR , PLSESSID( resume , "playlist empty, reject") );
		return false;
	}
	if ( 0 == _itCurrent->sessionId)
	{
		glog(ZQ::common::Log::L_INFO , PLSESSID(resume , "item[%lu][%s] is not being played, play it"),
			_itCurrent->userCtrlNum ,
			_itCurrent->objectName.string );
		iterator itPrev = listEnd();
		if( doLoad( _itCurrent ,0.0f, 0 , info ,false ))
		{
			OnItemStepped( itPrev , _itCurrent );
			return true;
		}
		else
		{
			return false;
		}
	}
	IOCTL_CONTROL_PARMS_LONG controlParam;
	memset(&controlParam , 0 , sizeof(controlParam));

	ULONG portNum					= _vstrmPortNum ; 
	controlParam.u.resume.timeCode	= TIME_CODE_NOW ;

	VSTRMAPICALLSTART(VstrmClassControlPortEx1);
	VSTATUS status = VstrmClassControlPortEx1(_mgr.classHandle(),
												portNum,
												VSTRM_GEN_CONTROL_RESUME,
												&controlParam,
												sizeof(IOCTL_CONTROL_PARMS_LONG),
												NULL,
												0);
	VSTRMAPICALLEND(VstrmClassControlPortEx1);
	if(status!=VSTRM_SUCCESS)
	{
		_ntstatus 	= GetLastError ();		// capture secondary error code (if any)
		char	szErrbuf[1024];
		ZeroMemory(szErrbuf,1024);
		_lastErrCode = ERR_PLAYLIST_SERVER_ERROR;
		ERRLOG(Log::L_ERROR, PLSESSID(resume,"VstrmClassControlPortEx1(VSTRM_GEN_CONTROL_RESUME) failed at VstrmPort[%lu]: item[%s][%lu] error[%s]"),
			_vstrmPortNum,_itCurrent->objectName.string,_itCurrent->userCtrlNum,
			_mgr.getErrorText(VstrmGetLastError(),szErrbuf,1023));

		return false;

	}
	else
	{
		updateOpTime();
#ifdef VSTRM_PORT_TRANSITION_HAS_VALID_SESSION_ID
		if( controlParam.u.resume.magicNumber == VSTRM_PORT_TRANSITION_HAS_VALID_SESSION_ID )
			checkEffectiveSession( controlParam.u.resume.sessionId );
#endif

		updateTimer();
		DisplaySessionAttr( _itCurrent );
		//glog(Log::L_DEBUG,PLSESSID(resume,"set playlist state to PLAYLIST_PLAY because resume ok"));
		
		FireStateChanged(PLAYLIST_PLAY);
		
		glog(Log::L_INFO, PLSESSID(resume,"successfully resume at VstrmPort[%lu], item[%s][%lu] VstrmSess[%lu]"),
			_vstrmPortNum,_itCurrent->objectName.string,_itCurrent->userCtrlNum,_itCurrent->sessionId );

		info.plState	=	IPlaylist::PLAYLIST_PLAY;
		info.speed		=	convertSPEEDtoFloat( controlParam.u.resume.speedIndicator );
		info.itemTimeOffset = info.timeOffset	=	controlParam.u.resume.timeOffset - _itCurrent->inTimeOffset;
		if( info.timeOffset < 0 )
		{
			info.timeOffset = 0 ;
			info.itemTimeOffset = 0;
		}
		info.userCtrlNum=	_itCurrent->userCtrlNum;
		
		_lastErrCode	=	ERR_PLAYLIST_SUCCESS;
		glog(Log::L_DEBUG,PLSESSID(resume,"done"));
		return true;
	}
}


void Playlist::OnVstrmSessDetected(const VstrmSessInfo& sessinfo
#if _USE_NEW_SESSION_MON
								   ,ZQ::common::Log& log
#endif
								   ,const int64& timeStamp  )
{
	{
		ZQ::common::MutexGuard gd(_listOpLocker);
		
		if(size()<=0 || _bCleared)
			return;
		
		const_iterator it = updateItem(sessinfo);
		if ( !iterValid(it) )
		{		
			return;
		}	
	}

	if(!m_pStreamSite)
	{
		m_pStreamSite=(StreamSmithSite*)StreamSmithSite::getDefaultSite();
	}

	
	if(m_pStreamSite)
	{
		long lSeq = InterlockedIncrement( &_eventCseqNewSessDetected );
#ifdef NEED_EVENTSINK_RAWDATA
		SESSDETECTED	sd;
		sd.pSessInfo=sessinfo;
		ZQ::common::Variant var(&sd,sizeof(SESSDETECTED));		
#else
		ZQ::common::Variant var;
		const ZQ::common::Variant varValue=(int)sessinfo.sessionId;
		var.set(EventField_EventCSEQ , lSeq );
		var.set(EventField_SessionId,varValue);
		var.set(EventField_PlaylistGuid,_strGuid);
		var.set(EventField_ClientSessId,m_strClientSessionID);
#endif
		glog(Log::L_INFO,PLSESSID(NewSessDetect,"event E_PLAYLIST_STARTED is fired with VstrmSess[%lu] seq[%ld]"),
								sessinfo.sessionId , lSeq );
		m_pStreamSite->PostEventSink( E_PLAYLIST_STARTED , var , _strGuid );
	}

	else
	{
		//ERRLOG(Log::L_ERROR,PLSESSID("void Playlist::OnVstrmSessDetected()##m_pStreamSite==NULL,So can't send sink event"));
	}
	//VstrmSessSink::OnVstrmSessDetected(sessinfo);
}

void Playlist::OnVstrmSessStateChanged(const VstrmSessInfo& sessinfo,const ULONG curState, const ULONG PreviousState
#if _USE_NEW_SESSION_MON
									   ,ZQ::common::Log& log
#endif
									   , const int64& timeStamp)
{
	ZQ::common::MutexGuard gd(_listOpLocker);

	switch( sessinfo.State ) 
	{
	case 3://streaming
		{
			glog(ZQ::common::Log::L_DEBUG, PLSESSID( OnVstrmSessStateChanged , "VstrmSess[%lu] is beging played" ),
				sessinfo.sessionId);
			if( _lastSessOpTime > timeStamp )
			{
// 				glog(ZQ::common::Log::L_DEBUG, PLSESSID( OnVstrmSessStateChanged , "VstrmSess[%u]'s status is expired , just ignore this StateChange event" ),
// 					sessinfo.sessionId);
				break;
			}
			FireStateChanged (IPlaylist::PLAYLIST_PLAY);
		}
		break;
	case 9://paused
		{
			glog(ZQ::common::Log::L_DEBUG, PLSESSID( OnVstrmSessStateChanged , "VstrmSess[%lu] is paused" ),
				sessinfo.sessionId);		
			if( _lastSessOpTime > timeStamp )
			{
// 				glog(ZQ::common::Log::L_DEBUG, PLSESSID( OnVstrmSessStateChanged , "session[%u]'s status is expired , just ignore this StateChange event" ),
// 					sessinfo.sessionId);
				break;
			}
			FireStateChanged (IPlaylist::PLAYLIST_PAUSE);
		}
		break;
	default:
		{
		}
		break;
	}
	//updateTimer();

	return;
	//return here

//	ZQ::common::MutexGuard gd(_listOpLocker);

	if(size()<=0)
		return;
	
	const_iterator it= updateItem(sessinfo);
	
	if ( !iterValid(it) /*listEnd() <= it ||it<=listStart()*/)
		return;
	if(!m_pStreamSite)
	{
		m_pStreamSite=(StreamSmithSite*)StreamSmithSite::getDefaultSite();
	}

	//VstrmSessSink::OnVstrmSessStateChanged(sessinfo, PreviousState);
	if(m_pStreamSite)
	{
		
#ifdef NEED_EVENTSINK_RAWDATA
		SESSSTATECHANGED	ss;
		ss.pSessInfo = sessinfo;
		ss.PrevState = PreviousState;
		ZQ::common::Variant var(&ss,sizeof(SESSSTATECHANGED));		
#else
		ZQ::common::Variant	var;
		var.set(EventField_SessionId,ZQ::common::Variant((int)sessinfo.sessionId));
		var.set(EventField_PrevState,ZQ::common::Variant((int)PreviousState));
		var.set(EventField_CurrentState,ZQ::common::Variant((int)sessinfo.State));
		var.set(EventField_PlaylistGuid,_strGuid);
		var.set(EventField_ClientSessId,m_strClientSessionID);
#endif 
		glog(Log::L_INFO, PLSESSID(SessStateChanged,"event E_PLAYLIST_STATECHANGED is fired with previousState[%s] currentState[%s]"),
			VodProviderStateCodeText[(int)PreviousState],VodProviderStateCodeText[(int)curState]);
		m_pStreamSite->PostEventSink(E_PLAYLIST_STATECHANGED,var,_strGuid);
	}
	else
	{
		//ERRLOG(Log::L_ERROR,PLSESSID("void Playlist::OnVstrmSessStateChanged()## m_pStreamSite==NULL,So can't send event sink"));
	}

//	if (_itCurrent == it && VOD_STATE(DONE) == sessinfo->currentState)
//	{
//		//glog(Log::L_DEBUG,PLSESSID("session state==DONE(%u)"),sessinfo->sessionId);
//		//stepList(it->sessionId);
//	}
}
void Playlist::OnVstrmSessExpired( const ULONG sessionID,ZQ::common::Log& log , const int64& timeStamp ,const std::string& reason , int errorCode )
{
	if( _bCleared )
		return;
	CtrlNum num=INVALID_CTRLNUM;
	ZQ::common::MutexGuard guard(_listOpLocker,__MSGLOC__);
	
	if( size()<=0 || _bCleared )
		return;

	iterator it;
	for( it = listBegin(); it != listEnd(); it++ )
	{
		if(it->sessionId==sessionID)
		{
			num=it->userCtrlNum;
			break;
		}
	}
#if _USE_NEW_SESSION_MON
	UnRegisterSessID(sessionID);
#endif
	{
		//if( it != listEnd() &&  it!=_itCurrent &&(_itCurrent!=listEnd() ||_itCurrent!=listStart()))
		if( iterValid(it) && (iterValid(_itCurrent)) && (it != _itCurrent) )
		{
			//不正常的vstrm session 结束
			glog(Log::L_WARNING,PLSESSID(SessExpired,"ablnormal session[%lu] expired, userctrlNum[%u]"), sessionID , num );
			it->sessionId = 0;
		}
	}
	
	
	glog(Log::L_DEBUG,PLSESSID(SessExpired,"session[%lu] expired, userctrlNum(%u)"),sessionID, num);
	stepList(sessionID,false,reason,errorCode );
}

void Playlist::stepList( ULONG doneSessionId,bool unloadCurrent /*=false*/,const std::string& reason ,int errorCode )
{
	ZQ::common::MutexGuard guard(_listOpLocker,__MSGLOC__);
	
	if (isCompleted())
		return;
	if(_bCleared)
	{
		glog(Log::L_DEBUG,PLSESSID(stepList,"playlist resource has already be cleared, skip"));
		return;
	}
	
	if( iterValid(_itCurrent))
	{
		if( _itCurrent->sessionId != doneSessionId || ( _itCurrent->_sessionDone == true ) )
		{
			glog(Log::L_DEBUG,PLSESSID(stepList,"expired VstrmSess[%lu] did not match current item's VstrmSess[%lu], skip"),
				doneSessionId,_itCurrent->sessionId);
			//should I call updateTimer() here ???
			return;
		}
	}
	else
	{
		glog(ZQ::common::Log::L_WARNING,PLSESSID(stepList,"current item is not running, skip"));
		return ;
	}

	//TODO: record last error code and description
	
	if(  !IS_VSTRM_SUCCESS( errorCode ) )
	{
		_lastItemStepErrorCode	= errorCode;
		//char szTempBuf[1024] = {0};
		//_mgr._cls.getErrorText( _lastItemStepErrorCode , szTempBuf, sizeof(szTempBuf)-1 );
		
		_lastItemStepErrDesc	= reason; //szTempBuf;		

		_lastItemStepItemName	=  _itCurrent->_rawItemName;
	}
	
	_itCurrent->_sessionDone=true;	
	iterator	itBackup=_itCurrent;
	
	glog(ZQ::common::Log::L_INFO, PLSESSID(stepList,"entering steplist item[%s][%lu]"),
									_itCurrent->objectName.string, _itCurrent->userCtrlNum);

	// current item has been done, move the cursor to next
	iterator itTemp = _itCurrent;
	
	//set session id to 0
	itTemp->sessionId		= 0;
	itTemp->stampLaunched	= 0;
	itTemp->stampUnload		= 0;
	
	if(isReverseStreaming())
	{
		if( iterValid(_itCurrent) )
		{
			_lastplaylistNpt	= getNptFromItem( _itCurrent );
			_lastplaylistNptPrimary = getNptPrimaryFromItem( _itCurrent );
		}
		_lastItemNpt		= 0;

		_itCurrent = ( _itNext <_itCurrent  && _itNext >listStart() ) ? _itNext :(_itCurrent-1);
		_itNext = _itCurrent>listStart() ?_itCurrent-1 :listStart();
	}
	else
	{
		if(iterValid(_itCurrent))
		{
			_lastItemNpt = CalculateCurrentTotalTime();
		}
		else
		{
			_lastItemNpt		= -1;
		}
		_itCurrent = (_itNext > _itCurrent && _itNext < listEnd()) ? _itNext : (_itCurrent+1);
		_itNext = (_itCurrent < listEnd()) ? (_itCurrent+1) : listEnd();
			
		//get npt before current item
		_lastplaylistNpt	= getNptFromItem( _itCurrent );	
		//get nptpriamry before current item
		_lastplaylistNptPrimary = getNptPrimaryFromItem( _itCurrent );
	}
	if( _itCurrent > listStart() && _itCurrent < listEnd() )
		glog(Log::L_INFO,PLSESSID(stepList,"change current iterator to item[%s][%lu] VstrmSess[%lu] "),
			_itCurrent->objectName.string, _itCurrent->userCtrlNum, _itCurrent->sessionId);
	
	
	/////
	itBackup->_sessionDone=false;

	
//#pragma message(__MSGLOC__"TODO:如果steplist的时候_itNext没有被load近来怎么办？")
	if( !isCompleted() && iterValid(_itCurrent) )
	{
		if (unloadCurrent)
				unloadEx( itTemp );

		if( _itCurrent->sessionId ==  0 )
		{//current item is not loaded yet
			glog(ZQ::common::Log::L_INFO,PLSESSID(stepList,"current item [%s][%lu] is not loaded yet , try to load it"),
							_itCurrent->objectName.string, _itCurrent->userCtrlNum);
			if(!_bCleared)
			{
				iterator itTmpCurrent = _itCurrent;
				StreamControlResultInfo info;
				LockPrimeNext lock(_bDontPrimeNext);
				//			play();//play current item
				bool bOK=false;
				float speed = 0.0f;			
				

				while( iterValid(_itCurrent) && ( _itCurrent->sessionId ==0 ) )
				{
					
					if( isFFScale(_crntSpeed) && !checkResitriction( _itCurrent , PLISFlagNoFF ) )
					{
						speed = 1.0f;
						glog(ZQ::common::Log::L_WARNING,PLSESSID(stepList,"adjust speed[1.0] due to restriction[%s]"),
							convertItemFlagToStr(_itCurrent).c_str());
					}
					bool bRewRestriction = false;
					while( iterValid(_itCurrent) && ( isREWScale(_crntSpeed) && (!checkResitriction( _itCurrent , PLISFlagNoRew ) || !checkResitriction( _itCurrent, PLISFlagSkipAtRew) ) ) )
					{
						glog(ZQ::common::Log::L_WARNING,PLSESSID(stepList,"skip to next item due to REW restriction[%s]"),
							convertItemFlagToStr(_itCurrent).c_str() );
						_itCurrent =( _itNext <_itCurrent  && _itNext >listStart() )?_itNext :(_itCurrent-1);
						_itNext = _itCurrent>listStart() ?_itCurrent-1 :listStart();
						bRewRestriction = true;
					}
					while ( iterValid(_itCurrent) && ( isFFScale(_crntSpeed) && !checkResitriction(_itCurrent, PLISFlagSkipAtFF)) )
					{
						glog(ZQ::common::Log::L_WARNING,PLSESSID(stepList,"skip to next item, ads cant' play in FF mode, flag: [%s]"),
							convertItemFlagToStr(_itCurrent).c_str() );
						_itCurrent =( _itNext >_itCurrent  && _itNext <listEnd() )?_itNext :(_itCurrent+1);
						_itNext = _itCurrent < listEnd() ?_itCurrent+1 :listEnd();
						bRewRestriction = true;
					}

					if( bRewRestriction )
					{
						unloadEx(itTmpCurrent);						
					}
					
					if( !iterValid(_itCurrent) )
					{
						break;
					}

					bOK = doLoad( _itCurrent, 0.0f , 0, info , false );

					if( bOK)
						break;

					if(isReverseStreaming())
					{
						_itCurrent =( _itNext <_itCurrent  && _itNext >listStart() )?_itNext :(_itCurrent-1);
						_itNext = _itCurrent>listStart() ?_itCurrent-1 :listStart();
					}
					else
					{		
						_itCurrent = (_itNext > _itCurrent && _itNext < listEnd()) ? _itNext : (_itCurrent+1);
						_itNext = (_itCurrent < listEnd()) ? (_itCurrent+1) : listEnd();
					}
					if (isCompleted())
					{
						break;
					}
				}				

				try
				{
					int curTimeoffset = 0 ;
					if( iterValid(_itCurrent) )
					{
						curTimeoffset = isReverseStreaming() ? _itCurrent->_itemPlaytime : 0;
					}
					OnItemStepped( itTemp , _itCurrent , curTimeoffset , reason );
				}
				catch(...) 
				{
					glog(Log::L_ERROR,PLSESSID(stepList,"caught unknown exception when invoke OnItemStepped"));
				}

				if( isCompleted() )
				{
					try
					{
						_stampDone = GetTickCount();
						OnPlaylistDone();
					}
					catch (...)
					{
					}
					return;
				}		
				
				{
					glog(ZQ::common::Log::L_INFO, PLSESSID(stepList,"step list to item[%s][%lu] VstrmSess[%lu] "),
						_itCurrent->objectName.string, _itCurrent->userCtrlNum, _itCurrent->sessionId);
				}
			}
			//updateTimer();//??			
		}
		else//if( _itCurrent->sessionId ==  0 )
		{
			iterator itTmpCurrent = _itCurrent;

			if( isFFScale(_crntSpeed) && !checkResitriction(_itCurrent, PLISFlagNoFF ))
			{
				glog(ZQ::common::Log::L_WARNING,PLSESSID(stepList,"change scale to[1.0] due to restriction[%s]"),
					convertItemFlagToStr(_itCurrent).c_str() );
				StreamControlResultInfo info;
				doChangeSpeed( 1.0, true , info );
			}

			bool bRewRestriction = false;
			while( iterValid(_itCurrent) && ( isREWScale(_crntSpeed) && !checkResitriction( _itCurrent , PLISFlagNoRew )) )
			{
				glog(ZQ::common::Log::L_WARNING,PLSESSID(stepList,"skip to next item due to REW restriction[%s]"),
					convertItemFlagToStr(_itCurrent).c_str() );
				_itCurrent =( _itNext <_itCurrent  && _itNext >listStart() ) ? _itNext :(_itCurrent-1);
				_itNext = _itCurrent>listStart() ?_itCurrent-1 :listStart();
				bRewRestriction = true;
			}

			if( bRewRestriction )
			{
				unloadEx(itTmpCurrent);
				if(iterValid(_itCurrent))
					SeekTo( _itCurrent->userCtrlNum , 0 , SEEK_POS_END );				
			}
			//do not care rewind because a rewind-disabled item should not to be loaded
			try
			{
				int curTimeoffset = 0 ;
				if( iterValid(_itCurrent) )
				{
					curTimeoffset = isReverseStreaming() ? itTmpCurrent->_itemPlaytime : 0;
				}
				OnItemStepped( itTemp , _itCurrent , curTimeoffset , reason );
			}
			catch(...) 
			{
				glog(Log::L_ERROR,PLSESSID(stepList,"caught unknown exception when invoke OnItemStepped"));
			}			
			if( isCompleted() )
			{
				try
				{
					_stampDone = GetTickCount();
					OnPlaylistDone();
				}
				catch (...)
				{
				}
				return;
			}
		}
		primeNext();
		updateTimer();
	}
	else
	{
		try
		{
			OnItemStepped(itTemp,_itCurrent,0,reason);
		}
		catch(...) 
		{
			glog(Log::L_ERROR,PLSESSID(stepList,"caught unknown exception when invoke OnItemStepped"));
		}

		unloadEx(itTemp );
		try
		{
			_stampDone = GetTickCount();
			OnPlaylistDone(reason);
		}
		catch(...) 
		{
			glog(Log::L_ERROR,PLSESSID(stepList,"caught unknown exception when invoke OnPlaylistDone"));
		}
		//		endTimer();
		//		return;		
	}

	//delete the previous item if it is deletable
	if(iterValid(itTemp))
	{
		CtrlNum curCtrlNum = itTemp->userCtrlNum;		
		/*if( isDeletable(itTemp->itemFlag) )
		{
			glog(ZQ::common::Log::L_INFO,PLSESSID(stepList,"item[%s][%lu] flag[%s] is deletable, erase it"),
				itTemp->objectName.string,itTemp->userCtrlNum,convertItemFlagToStr(itTemp).c_str());
			erase(curCtrlNum);
		}*/
	}


#ifdef _ICE_INTERFACE_SUPPORT
	_currentItemDist=(int)( _itCurrent - listBegin() );
	_mgr.UpdatePlaylistFailStore(this,true,false);
#endif//_ICE_INTERFACE_SUPPORT	

	printList("Steplist");
	DumpListInfo(true);
}

Playlist::iterator Playlist::findNextCriticalStartItem(timeout_t& timeout)
{
	timeout = TIMEOUT_INF;
	Playlist::iterator it = listEnd();

	// return the regular timer if _itNextCriticalStart is invalid
	{
		ZQ::common::Guard < ZQ::common::Mutex > guard(_listOpLocker,__MSGLOC__);
		if ( isCompleted() || isReverseStreaming() )
		{
			return it;
		}
		
		for (it = _itCurrent+1; it < listEnd() && 0 ==it->criticalStart; it++);
		
		if (it != listEnd())
		{
			time_t now;
			time(&now);
			timeout = static_cast<timeout_t>( (it->criticalStart - now) *1000 );
			glog(ZQ::common::Log::L_INFO, PLSESSID(findNextCriticalStartItem,"find criticalStart Item[%s][%lu] at [%s] ,[%u] from now"),
				it->objectName.string,it->userCtrlNum,printfUTCTime(it->criticalStart).c_str(),timeout);
		}
	}

	if ( timeout <0 || timeout>=0x8fffffff )//unsigned data
		timeout = 0;

	return it;
}

void Playlist::FireSpeedChanged( ULONG sessId, SPEED_IND newSpeed, SPEED_IND PreviousSpeed )
{
	if ( newSpeed.denominator == PreviousSpeed.denominator &&  newSpeed.numerator == PreviousSpeed.numerator )
	{
		glog(ZQ::common::Log::L_DEBUG, PLSESSID(FireSpeedChanged,"newSpeed[%d/%d] currentSpeed[%d/%d] is the same, reject emiting new speed change event"),
			newSpeed.numerator, newSpeed.denominator, PreviousSpeed.numerator,PreviousSpeed.denominator);
		return;
	}	

	if(!m_pStreamSite)
	{
		m_pStreamSite=(StreamSmithSite*)StreamSmithSite::getDefaultSite();
	}

	if(m_pStreamSite)
	{
		long lSeq = InterlockedIncrement( &_eventCSeqSpeedChanged );
		try
		{
#ifdef NEED_EVENTSINK_RAWDATA
			SINKSPEEDCHANGED	ss;
			ss.pSessionInfo = sessinfo;
			ss.prevFileSpeed=PreviousSpeed;
			ZQ::common::Variant	var(&ss,sizeof(SINKSPEEDCHANGED));
#else
			ZQ::common::Variant	var;
			ZQ::common::Variant	 varSpeed;
			DWORD now = GetTickCount();
			std::string perRequested = (now < (_stampSpeedChangeReq + gStreamSmithConfig.lperRequestInterval) ) ? "1" : "0";
			glog(ZQ::common::Log::L_DEBUG, PLSESSID(OnVstrmSessSpeedChanged,"event(E_PLAYLIST_SPEEDCHANGED) is fired to EventChannel, now[%u], _stampSpeedChangeReq[%u], interval[%u], cfg[%d],perRequested[%s]"),now, _stampSpeedChangeReq, now - _stampSpeedChangeReq,gStreamSmithConfig.lperRequestInterval,perRequested.c_str());

			var.set(EventField_perRequested, perRequested);

			var.set(EventField_SessionId,ZQ::common::Variant((int)sessId));

			varSpeed.clear();
			varSpeed.set(EventField_SpeedNumer,ZQ::common::Variant((int)PreviousSpeed.numerator));
			varSpeed.set(EventField_SpeedDenom,ZQ::common::Variant((int)PreviousSpeed.denominator));
			var.set(EventField_PrevSpeed,varSpeed);

			varSpeed.clear();
			varSpeed.set(EventField_SpeedDenom,ZQ::common::Variant((int)newSpeed.denominator));
			varSpeed.set(EventField_SpeedNumer,ZQ::common::Variant((int)newSpeed.numerator));
			var.set(EventField_CurrentSpeed,varSpeed);

			var.set(EventField_PlaylistGuid,_strGuid);
			var.set(EventField_ClientSessId,m_strClientSessionID);
			var.set(EventField_EventCSEQ ,  lSeq );

			var.set(EventField_CurrentPlayPos, getCurrentPlaylistNpt() );
			var.set(EventField_CurrentItemTimeOffset, getCurrentItemNpt() );
			var.set( EventField_PreviousePlayPos , _lastplaylistNpt );
			var.set( EventField_PreviousePlayPosPrimary, _lastplaylistNptPrimary );// npt
			var.set( EventField_TotalDuration, getTotalStreamDuration());
			var.set ( EventField_TotalVideoDuration, getTotalVideoStreamDuration());

			if (!isCompleted()) 
			{	
				if (iterValid(_itCurrent) )
				{
					var.set(EventField_UserCtrlNum,(int)_itCurrent->userCtrlNum );
					var.set(EventField_ItemFileName,_itCurrent->_rawItemName );
				}
				else
				{
					var.set(EventField_ItemFileName,std::string("") );					
					var.set(EventField_UserCtrlNum,(int)INVALID_CTRLNUM);
				}

			}
			else
			{
				var.set(EventField_UserCtrlNum,(int)INVALID_CTRLNUM);
				var.set(EventField_ItemFileName,"");
			}
#endif
			glog(Log::L_INFO, PLSESSID(OnVstrmSessSpeedChanged,"event E_PLAYLIST_SPEEDCHANGED is fired , speed changed from [%d/%d] to [%d/%d] and seq[%ld]"),
				(int)PreviousSpeed.numerator, (int)PreviousSpeed.denominator,														
				(int)newSpeed.numerator, (int)newSpeed.denominator,
				lSeq );

			m_pStreamSite->PostEventSink(E_PLAYLIST_SPEEDCHANGED,var,_strGuid);
		}
		catch (...)
		{
			ERRLOG(Log::L_ERROR,PLSESSID(SessSpeedChanged,"caught unknown exceptionwhen call PostEventSink with EVENTSINK_SESS_SPEEDCHANGED"));
		}
	}
	else
	{
		//ERRLOG(Log::L_ERROR,PLSESSID("void Playlist::OnVstrmSessSpeedChanged()## m_pStreamSite==NULL,can't send sink event"));
	}
}

void Playlist::OnVstrmSessSpeedChanged(const VstrmSessInfo& sessinfo, SPEED_IND curSpeed , SPEED_IND PreviousSpeed
#if _USE_NEW_SESSION_MON
									   ,ZQ::common::Log& log
#endif
									   , const int64& timeStamp)
{	
	int iCurCtrlNum = INVALID_CTRLNUM;
	std::string curRawName= "";
	{
		ZQ::common::MutexGuard gd(_listOpLocker);
		
		if( size()<=0 || _bCleared )
			return;
		
		iterator it=updateItem( sessinfo, true );
		if ( !iterValid(it) )
			return;
		//因为SpeedChaged事件被检测到的时机可能会滞后，所以需要primenext！
		glog(Log::L_INFO,PLSESSID(SessSpeedChanged,"speedChangeded detected stream session ([%d/%d]===>>[%d/%d]) and timeoffset[%lu]"),
			PreviousSpeed.numerator,PreviousSpeed.denominator,
			sessinfo.Speed.numerator,sessinfo.Speed.denominator,
			sessinfo.PlayoutTimeOffset );
		
		primeNext();
		if (iterValid(_itCurrent) )
		{
			iCurCtrlNum = _itCurrent->userCtrlNum;
			curRawName = _itCurrent->_rawItemName;	
		}

		if( curSpeed.denominator == _crntSpeed.denominator && curSpeed.numerator == _crntSpeed.numerator )
		{
			glog(ZQ::common::Log::L_DEBUG,PLSESSID(OnVstrmSessSpeedChanged,"newSpeed[%d/%d] is the same as old [%d/%d], skip"),
				curSpeed.numerator,curSpeed.denominator, _crntSpeed.numerator, _crntSpeed.denominator );
			return;
		}

		if( sessinfo.noOldInfomation )
		{
			PreviousSpeed = _crntSpeed;
			_crntSpeed = curSpeed;
		}
		else if ( PreviousSpeed.numerator == 0 ) 
		{
			glog(Log::L_DEBUG,PLSESSID(OnVstrmSessSpeedChanged,"previous speed is 0 ,skip sending event"));
			return;
		}
		else
		{
			_crntSpeed.denominator = sessinfo.Speed.denominator;
			_crntSpeed.numerator = sessinfo.Speed.numerator;		
			glog(ZQ::common::Log::L_INFO , PLSESSID(OnVstrmSessSpeedChanged,"speed is changed from[%d/%d] to [%d/%d]"),
				PreviousSpeed.numerator , PreviousSpeed.denominator , _crntSpeed.numerator ,_crntSpeed.denominator );
		}
		FireSpeedChanged( sessinfo.sessionId, curSpeed, PreviousSpeed );
	}
}



void Playlist::OnVstrmSessProgress(const VstrmSessInfo& sessinfo,const TIME_OFFSET curTimeOffset , const TIME_OFFSET PreviousTimeOffset
#if _USE_NEW_SESSION_MON
								   ,ZQ::common::Log& log
#endif
								  , const int64& timeStamp )
{
	int totalSize = 0;
	int step =0;
	int iCurCtrlNum = 0;
	std::string strRawName ="";
	{
		ZQ::common::MutexGuard gd(_listOpLocker);
		
		if( size()<=0 || _bCleared )
			return;
		
		//glog(Log::L_DEBUG,PLSESSID("Current TimeOffset=(%ld/%ld) byeoffset(%lld/%lld)"),PreviousTimeOffset,sessinfo->currentTimeOffset,sessinfo->runningByteOffset,sessinfo->endOfStreamByteOffset);
		if(sessinfo.MuxRate == 0 || sessinfo.ObjectSize == 0 )
		{
			glog(Log::L_DEBUG,PLSESSID(SessProgress,"invalid session information in progress message"));
		}
		if( isCompleted() || (!iterValid(_itCurrent)) )
		{
			return;
		}
		
		bool bSendMessage=false;
		if( _itCurrent->sessionId == sessinfo.sessionId )
		{
			if((_itCurrent->lastProgressEventTimeoffset - sessinfo.PlayoutTimeOffset!=0) &&
				abs( long( _itCurrent->lastProgressEventTimeoffset - sessinfo.PlayoutTimeOffset )) <= 
				gStreamSmithConfig.lProgressEventSendoutInterval )
			{
				bSendMessage=false;
			}
			else
			{
				_itCurrent->lastProgressEventTimeoffset  = sessinfo.PlayoutTimeOffset;
				bSendMessage=true;
			}
		}
		else
		{
			bSendMessage=false;
		}
		const_iterator it= updateItem(sessinfo);
		
		if(!bSendMessage)
			return;
		
		if ( !iterValid(it) /*listEnd() <= it || it <= listStart()*/)
			return;
		
		totalSize = size();
		step = (int(listCurrent()-listBegin())) +1;
		iCurCtrlNum = it->userCtrlNum;
		strRawName = it->_rawItemName;
			
	}
	if(!m_pStreamSite)
	{
		m_pStreamSite=(StreamSmithSite*)StreamSmithSite::getDefaultSite();
	}
	if(m_pStreamSite)
	{
		long lSeq = InterlockedIncrement(&_eventCSeqProgress);
		try
		{
#ifdef NEED_EVENTSINK_RAWDATA
			SINKPROGRESS	sp;
			sp.pSessionInfo=sessinfo;
			sp.timeOffset=PreviousTimeOffset;
			ZQ::common::Variant var(&sp,sizeof(SINKPROGRESS));		
#else
			ZQ::common::Variant var;
			var.set(EventField_EventCSEQ , lSeq );
			var.set(EventField_SessionId,ZQ::common::Variant((int)sessinfo.sessionId));
			var.set(EventField_PrevTimeOffset,ZQ::common::Variant((int)PreviousTimeOffset));
			var.set(EventField_CurrentTimeOffset,ZQ::common::Variant((int)sessinfo.PlayoutTimeOffset));
			char szBuf[128];
			ZeroMemory(szBuf,sizeof(szBuf));
			_guid.toString(szBuf,127);

			var.set(EventField_PlaylistGuid,ZQ::common::Variant(szBuf));
			var.set(EventField_totalStep,ZQ::common::Variant(totalSize));
			var.set(EventField_currentStep,ZQ::common::Variant( step ));
			var.set(EventField_UserCtrlNum,ZQ::common::Variant(iCurCtrlNum));
			var.set(EventField_ItemFileName,ZQ::common::Variant(strRawName ));
			var.set(EventField_ClientSessId,m_strClientSessionID);
			///how to set a longlong var data????
			var.setQuadword(var,EventField_runningByteOffset,sessinfo.PlayoutByteOffset);
			var.setQuadword(var,EventField_TotalbyteOffset,sessinfo.ObjectSize);
#endif
			m_pStreamSite->PostEventSink(E_PLAYLIST_INPROGRESS,var,_strGuid);
		}
		catch (...)
		{
			ERRLOG(Log::L_ERROR,PLSESSID(SessProgress,"caught unknown exception when call PostEventSink with EVENTSINK_SESS_PROGRESS"));
		}
	}
	else
	{
		//ERRLOG(Log::L_ERROR,PLSESSID("void Playlist::OnVstrmSessProgress()## m_pStreamSite==NULL,can't send sink event"));
	}
//	VstrmSessSink::OnVstrmSessProgress(sessinfo, PreviousTimeOffset);
}

Playlist::const_iterator Playlist::updateItem( const VstrmSessInfo& sessinfo, bool forceUpdateTimer)
{
	if (  size()<=0 || isCompleted() )
	{
		glog(Log::L_DEBUG,PLSESSID(updateItem,"null session info or playlist complete"));
		return listEnd(); // invalid iterator or input
	}
	if ( sessinfo.MuxRate == 0 || sessinfo.ObjectSize == 0 ) 
	{
		return listEnd();
	}	

	Playlist::iterator it = listEnd();

	if(_itCurrent<listEnd() &&_itCurrent >listStart())
		forceUpdateTimer |= (_itCurrent->bitrate ==0);
	else
		forceUpdateTimer=true;


	if ( iterValid(_itCurrent) && ( _itCurrent->sessionId == sessinfo.sessionId)  )
	{
		it = _itCurrent;
		if( _itCurrent >= listEnd() || _itCurrent <= listStart()  )
			return it;
		// about the current item
		it->bitrate				= sessinfo.MuxRate;
		it->state				= sessinfo.State;
		it->speed.numerator		= sessinfo.Speed.numerator;
		it->speed.denominator	= sessinfo.Speed.denominator;
		it->timeOffset			= sessinfo.PlayoutTimeOffset;				//  current time offset	(miliseconds)
		it->byteOffset			= sessinfo.ByteOffset;
		it->byteOffsetEOS		= sessinfo.ObjectSize;			// end of stream offset
		it->stampLoad = it->stampLastUpdate = GetTickCount();
	}
	else if ( iterValid(_itNext) && (_itNext->sessionId == sessinfo.sessionId ))
	{
		// about the next item
		it = _itNext;
		if( _itNext >= listEnd() || _itNext <= listStart() )
			return it;

		it->bitrate				= sessinfo.MuxRate;
		it->state				= sessinfo.State;
		it->speed.numerator		= sessinfo.Speed.numerator;
		it->speed.denominator	= sessinfo.Speed.denominator;
		it->timeOffset			= sessinfo.PlayoutTimeOffset;		//  current time offset	(miliseconds)
		it->byteOffset			= sessinfo.ByteOffset;
		it->byteOffsetEOS		= sessinfo.ObjectSize;		// end of stream offset
		it->stampLoad			= it->stampLastUpdate = GetTickCount();
	}		
	if (forceUpdateTimer)
	{
//		glog(Log::L_DEBUG,PLSESSID("force update timer when update item"));
//		updateTimer();
	}

	return it;
}

void Playlist::updateTimer()
{
	glog(Log::L_DEBUG,PLSESSID(updateTimer,"Enter update timer"));
	long to = getSessDurLeft();
	if (TIMEOUT_INF == to)
	{		
		glog(ZQ::common::Log::L_DEBUG, PLSESSID(updateTimer,"time==TIMEOUT_INF return updateTimer(%ums)"), gStreamSmithConfig.lPlaylistTimeout);		
		_isGoodItemInfoData=true;
		SetTimerEx(gStreamSmithConfig.lPlaylistTimeout);
		if(_mgr._timeout==_UI64_MAX)
			_mgr.wakeup();
		return;		
	}

	if(_isGoodItemInfoData)
	{
		if(!isCompleted()&&//没有结束
			_bEnableEOT &&
			iterValid(_itCurrent) &&
			((_itCurrent+1)==listEnd())&&
			_isGoodItemInfoData&&
			!isReverseStreaming() &&
			(_crntSpeed.numerator > _crntSpeed.denominator) )//正好处在最后一个Itemn并且是在PauseTV状态,并且是正向播放,并且EnableEOT为TRUE
		{
			int forceNormalTime = gStreamSmithConfig.lForceNormalTimeBeforeEnd;
// 			glog(Log::L_DEBUG, PLSESSID(updateTimer,"this is the last item and in pauseTV mode with configuration time=[%d]ms"),
// 																	forceNormalTime);
			if( forceNormalTime < gStreamSmithConfig.lPreloadTime )
			{
// 				glog(Log::L_DEBUG,PLSESSID(updateTimer,"configuration time=[%d]ms is too small,set it to [%d]ms"),
// 															forceNormalTime,gStreamSmithConfig.lPreloadTime);
				forceNormalTime = gStreamSmithConfig.lPreloadTime;
			}
			to -= forceNormalTime;
			glog(Log::L_INFO,PLSESSID(updateTimer,"update timer to [%ld]ms, EOTSize[%d] preLoadTime[%d] "),
				to,gStreamSmithConfig.lForceNormalTimeBeforeEnd,
				gStreamSmithConfig.lPreloadTime);
		}
		else if( to > gStreamSmithConfig.lPreloadTime )
		{//为什么会gStreamSmithConfig.lPreloadTime*2，是因为希望在接近preload的区域前重新校准时间
			to -= gStreamSmithConfig.lPreloadTime;
		}
//		else if(to>gStreamSmithConfig.lPreloadTime)
//		{
//			to-=gStreamSmithConfig.lPreloadTime;
//		}
	}
	//adjust to 0 if the target is < 0
	if( to < 0 )
		to = 0;

	
	// adjust regular timer if _itNextCriticalStart is valid
	timeout_t t2;
	iterator it = findNextCriticalStartItem( t2 );
	if (it < listEnd() && it> listStart() && TIMEOUT_INF != t2 && static_cast<time_t>(to) > t2)
	{
		glog(ZQ::common::Log::L_DEBUG, PLSESSID(updateTimer, "adjust timer to [%u],due to item[%s][%lu] has criticalStartTime[%s] "), 
			t2,it->objectName.string , it->userCtrlNum, printfUTCTime(it->criticalStart).c_str() );
		to = t2;
	}
//#pragma message(__MSGLOC__"TODO:到底是 >=0 还是 >0")
	if (to >= 0)
	{
		glog(ZQ::common::Log::L_INFO, PLSESSID(updateTimer, "update current Timer to [%ld]ms"), to+200 );
		SetTimerEx( to+200, _isGoodItemInfoData );
	}

	if ( 0 == to || to <= _mgr._timeout ||  ( to > 0 && _mgr._timeout == _UI64_MAX ))
	{
		glog(Log::L_DEBUG, PLSESSID(updateTimer, "wakeup timer execution and to = [%ld]ms mgr._timeout=[%lld]ms"), to, _mgr._timeout );
		_mgr.wakeup();
	}
	//glog(Log::L_DEBUG,PLSESSID(updateTimer,"Leave UpdateTimer with to = [%d]ms"),to+10);
}

void Playlist::OnTimer()
{
	try
	{
		ZQ::common::MutexGuard gd( _listOpLocker );
		//判断一下Timer的误差
		timeout64_t tNow = _GetTickCount64();
		glog(ZQ::common::Log::L_INFO, PLSESSID(OnTimer, "OnTimer():enter with time different :%lld (now[%lld]/target[%lld])"), 
			tNow-_lastUpdateTimer64,tNow,_lastUpdateTimer64);
		endTimer64(); // stop the time first	
	
		if( (isCompleted()  || _currentStatus!=PLAYLIST_PLAY)&& _isGoodItemInfoData )
		{
			if(_currentStatus==PLAYLIST_PAUSE)
			{
				try
				{
					onPauseTimeout();

					if (_mgr.keepOnPause())
					{
						glog(Log::L_INFO, PLSESSID(OnTimer, "state[PLAYLIST_PAUSE] w/ keepOnPause[true], simply renewing the timer"));
						SetTimerEx(gStreamSmithConfig.lPlaylistTimeout);
					}	
					else
					{
						glog(Log::L_INFO, PLSESSID(OnTimer, "state[LAYLIST_PAUSE] w/ keepOnPause[false], killing the playlist"));
						ClearAllResource();
					}
				}
				catch(...) {}

				return;
			}

			glog(Log::L_INFO, PLSESSID(OnTimer, "timed out at idle: state[%d], isComplete[%d]"), _currentStatus, isCompleted());
			try
			{				
				//delete this;
#ifdef _ICE_INTERFACE_SUPPORT
				ClearAllResource();
#else//_ICE_INTERFACE_SUPPORT
				ClearAllResource();
#endif//_ICE_INTERFACE_SUPPORT
			}
			catch (...)
			{
				glog(Log::L_ERROR,PLSESSID(OnTimer, "caught exception when invoke ClearAllResource()"));
			}
		}
		else 
		{
			if(_bCleared)
			{
				glog(Log::L_DEBUG, PLSESSID(OnTimer, "playlist had been cleared, skip clearing resource"));
				return;
			}

			if(m_iErrInfoCount>=100)
			{
				ERRLOG(Log::L_ERROR, PLSESSID(OnTimer, "failed to read attributes from VstrmSess after 100 retries, terminating playlist"));
				_strClearReason=_lastError;
				try
				{
					ClearAllResource();
				}
				catch (...){}
				return;
			}

			if(_currentStatus!=PLAYLIST_PLAY)
				return;

			//glog(Log::L_DEBUG,PLSESSID(OnTimer,"may be should load next item"));
			try
			{
				if( iterValid(_itCurrent) && ( (_itCurrent+1) == listEnd()) && _isGoodItemInfoData )
				{		
					if( _bEnableEOT && ( gStreamSmithConfig.lForceNormalTimeBeforeEnd>0 ) )//需要在最后一个Item前的某个时刻变成normalPlay
					{
						if(!isReverseStreaming())
						{
							glog(Log::L_INFO,PLSESSID(OnTimer,"current item[%s][%lu] is the last of the playlist, checking EOT protection with EOTSize[%d]"),
									_itCurrent->objectName.string,_itCurrent->userCtrlNum, gStreamSmithConfig.lForceNormalTimeBeforeEnd);

							if (memcmp(&_crntSpeed, &SPEED_NORMAL, sizeof(SPEED_NORMAL) ) != 0 )
							{
								long lLeft = getSessDurLeft();
								if( lLeft <= gStreamSmithConfig.lForceNormalTimeBeforeEnd )
								{
									glog(Log::L_INFO, PLSESSID(OnTimer, "force item[%s][%lu] to normal speed per EOT protection  with EOTSize[%d]"),
										_itCurrent->objectName.string,_itCurrent->userCtrlNum,
										gStreamSmithConfig.lForceNormalTimeBeforeEnd);
									StreamControlResultInfo info;
									doChangeSpeed( 1.0f , true, info);
								}

								updateTimer();
								//setSpeedEx( SPEED_NORMAL , true );
								//如果这个时候发送一个speed为normal的ANNOUNCE如何?
							}

							return;
						}						
					}
				}

				timeout_t t;
				iterator it = findNextCriticalStartItem(t);	
				//增加为critical start的计时
				if ( iterValid(it) && t ==0)
				{
					glog(Log::L_INFO, PLSESSID(OnTimer,"skipping to item[%s][%lu] due to its criticalStartTime"),
							it->objectName.string, it->userCtrlNum);
					skipToItem(it, true);
				}
				//glog(Log::L_DEBUG,PLSESSID(OnTimer,"Prime Next in OnTimer"));
				primeNext();
				
			}
			catch(...)
			{
				
			}
		}
	}
	catch (...) 
	{
	}
}

bool Playlist::skipToItem(const_iterator it, bool bPlay)
{	
	{
		ZQ::common::Guard < ZQ::common::Mutex > guard(_listOpLocker,__MSGLOC__);
		glog(Log::L_DEBUG,PLSESSID(skipToItem,"entering"));

		if (isCompleted() || /*it <= _itCurrent ||*/ it >=listEnd() ||it<=listStart())
		{
			_lastErrCode = ERR_PLAYLIST_INVALID_PARA;
			
			FireStateChanged(PLAYLIST_STOP);
			//_currentStatus=PLAYLIST_STOP;
			ERRLOG(Log::L_ERROR,PLSESSID(skipToItem,"the SKIP-TO-ITEM specified is out of range, reject"));
			DumpListInfo();
			return false;
		}
	
		// time to start, if there is any still playing, stop it
		if ( _itNext <listEnd() && _itNext>listStart() && 0 != _itNext->sessionId)
			unloadEx( _itNext  ); // kill the preloaded of _itCurrent
		
//		CtrlNum lastCtrlNum = INVALID_CTRLNUM;
//		std::string	lastFileName = "";
		iterator prevIterator = listStart();

		if ( _itCurrent->sessionId != 0 )
		{
			prevIterator = _itCurrent;
//			lastCtrlNum = _itCurrent->userCtrlNum;
//			lastFileName = _itCurrent->objectName.string;
			unloadEx(_itCurrent); // kill the session of _itCurrent
		}
		
		if(bPlay)
			glog(Log::L_DEBUG,PLSESSID(skipToItem,"loading item[%s][%lu]"), it->objectName.string, it->userCtrlNum );

		{
			iterator itCur = it;
			bool bLoadOK = true;
			bool bComplete = false;
			StreamControlResultInfo info;
			while ( !bComplete  && ! doLoad(itCur , 0.0f ,0 ,info ,false ) )
			{		
				if(isReverseStreaming())
				{
					itCur--;
				}
				else
				{
					itCur++;
				}
				_itCurrent=itCur;
				bComplete= isCompleted();
				bLoadOK=false;
			}
			_itCurrent = itCur;
			//调整_itNext
			if(isReverseStreaming())
			{
				_itNext=_itCurrent>listStart() ?_itCurrent-1:listStart();
			}
			else
			{
				_itNext=_itCurrent<listEnd() ?_itCurrent+1:listEnd();
			}	
			if(!bLoadOK)
			{
				_lastErrCode = ERR_PLAYLIST_SERVER_ERROR;
				ERRLOG(Log::L_ERROR,PLSESSID(skipToItem,"reached the playlist end"));
				FireStateChanged(PLAYLIST_STOP);
				//OnItemDoneDummy(lastCtrlNum,lastFileName, listEnd());
				OnItemStepped(prevIterator, listEnd());
				OnPlaylistDone();
				updateTimer();
				return false;					 
			}
			
			if(!_b1stPlayStarted)
			{				
				_b1stPlayStarted=true;				
			}
			//fire a Item stepped event
			//OnItemDoneDummy(lastCtrlNum,lastFileName,_itCurrent);
			OnItemStepped(prevIterator, _itCurrent);
			
			//fire a state changed
			FireStateChanged(PLAYLIST_PLAY);
			//_currentStatus=PLAYLIST_PLAY;
		}
	}
	primeNext();
	glog(Log::L_DEBUG, PLSESSID(skipToItem,"done"));
	return true;
}

void Playlist::primeNext()
{
	ZQ::common::MutexGuard gd(_listOpLocker);

	glog(Log::L_DEBUG,PLSESSID(primeNext,"entering"));
	if (isCompleted() || _bCleared )
	{
		glog(Log::L_DEBUG,PLSESSID(primeNext,"playlist is not running"));

		if(isReverseStreaming())
		{
			_itNext = listStart();
		}
		else
		{
			_itNext = listEnd();
		}		
		return;
	}
	if(_bDontPrimeNext)
	{
		glog(Log::L_DEBUG,PLSESSID(primeNext,"do not prime next item due to guard _bDontPrimeNext[true]"));
		return;			 
	}
	//////////////////////////////////////////////////////////////////////////	
	{	
		ZQ::common::Guard< ZQ::common::Mutex > guard(_listOpLocker,__MSGLOC__);	

		if(isReverseStreaming())
		{
			if(_itNext >=_itCurrent || _itNext <=listStart())
				_itNext=_itCurrent-1;
		}
		else
		{
			if (_itNext <=_itCurrent || _itNext >=listEnd())
				_itNext = _itCurrent +1;
		}
		
		//////////////////////////////////////////////////////////////////////////		
	
		if(  !iterValid(_itNext)/*_itNext==listEnd() || _itNext==listStart()*/)
		{
			glog(Log::L_DEBUG,PLSESSID(primeNext,"no more item for prime next"));			
			//TODO: should I disable this timer update ?
			//updateTimer();
			return;
		}

		if( _itNext->sessionId!=0)
		{
			//也就是说如果下一个item已经被Load起来了以后，就不需要再updateTimer了！
			//到steplist的时候会调用一次UpdateTimer			
			glog(Log::L_INFO,PLSESSID(primeNext,"next item[%s][%lu], VstrmSess[%lu] is already loaded, skip"),
					_itNext->objectName.string, _itNext->userCtrlNum, _itNext->sessionId );
			return;
		}
		
		ULONG to = getSessDurLeft();
		if(_isGoodItemInfoData)
		{
			m_iErrInfoCount=0;
			glog(Log::L_DEBUG,PLSESSID(primeNext,"[%lu]ms left of current item[%s][%lu]  "),
				to, _itCurrent->objectName.string, _itCurrent->userCtrlNum);
			if ( ( 0 >= gStreamSmithConfig.lPreloadTime  ||	to <= static_cast<ULONG>( gStreamSmithConfig.lPreloadTime ) ) && 
					0 ==_itNext->sessionId) // round up to 1 sec
			{
				glog(ZQ::common::Log::L_INFO, PLSESSID(primeNext,"pre-loading next item at [%lu]ms before VstrmSess[%lu] ends... and current item[%s][%lu]"), 
					to, _itCurrent->sessionId,_itCurrent->objectName.string, _itCurrent->userCtrlNum);
				// scan the following items in the list and load the first loadable
				bool bLoadOk = false ;
				if(isReverseStreaming())
				{//reverse direction
					glog(Log::L_INFO, PLSESSID(primeNext,"isReverseStreaming"));
					for( ;  _itNext >listStart() && _itNext->sessionId == 0 ; _itNext--)
					{						
						if( isREWScale(_crntSpeed) && !checkResitriction(_itNext,PLISFlagNoRew))
						{							
							glog(ZQ::common::Log::L_WARNING,PLSESSID(primeNext,"skip next due to rew restriction[%s]"),
								convertItemFlagToStr(_itNext).c_str() );
							continue;							
						}
						if ( (isREWScale(_crntSpeed) && !checkResitriction(_itNext, PLISFlagSkipAtRew)) )
						{
							glog(Log::L_DEBUG, PLSESSID(doPlay, "ads can't play in RE mode, skip, flag: [%s]"),  
								convertItemFlagToStr(_itNext->itemFlag).c_str());
							continue;
						}

						StreamControlResultInfo info;
						if (IsItemPlaytimesCountZero(_itNext))
						{
							glog(Log::L_INFO, PLSESSID(primeNext,"skip item for playtimes = 0"));
							continue;
						}
						if( doLoad( _itNext , 0.0f , 0 , info ,false ) )
						{
							glog(Log::L_DEBUG, PLSESSID(primeNext,"loaded successfully for item[%s][%lu]"),
									_itNext->objectName.string, _itNext->userCtrlNum);
							bLoadOk = true;
							break;
						}
					}					
				}
				else
				{//normal direction		
					glog(Log::L_INFO, PLSESSID(primeNext,"normal direction"));
					for (; _itNext < listEnd() && 0 == _itNext->sessionId; _itNext++)
					{
						float speed = 0.0f;//means no speed change
						
						if( gStreamSmithConfig.lApplyRestrictionWhenPrimenext >= 1 )
						{
							if( isFFScale(_crntSpeed) && !checkResitriction(_itNext,PLISFlagNoFF) )
							{
								glog(ZQ::common::Log::L_WARNING,PLSESSID(primeNext,"adjust speed[1.0] due to restriction[%s]"),
									convertItemFlagToStr(_itNext).c_str());
								speed = 1.0;
							}	
						}
						if ( isFFScale(_crntSpeed) && !checkResitriction(_itNext, PLISFlagSkipAtFF) )
						{
							glog(Log::L_DEBUG, PLSESSID(primeNext,"ads can't play in FF mode, skip, flag: [%s]"), 
								convertItemFlagToStr(_itNext->itemFlag).c_str());
							continue;
						}

						StreamControlResultInfo info;
						if (IsItemPlaytimesCountZero(_itNext))
						{
							glog(Log::L_INFO, PLSESSID(primeNext,"skip item for playtimes = 0"));
							continue;
						}
						if( doLoad( _itNext , speed , 0 , info ,false ) )
						{
							glog(Log::L_DEBUG, PLSESSID(primeNext,"loaded successfully for item[%s][%lu]"),
								_itNext->objectName.string, _itNext->userCtrlNum) ;
							bLoadOk =true;
							break;
						}
					}			
					
				}
				//如果没有一个Load成功的
				if(!bLoadOk)
				{
					glog(Log::L_ERROR,PLSESSID(primeNext,"failed to load items, reached the playlist end"));
				}				
				updateTimer();				
			}	
			else if( _itNext->sessionId == 0 )
			{
				//glog(Log::L_DEBUG,PLSESSID(primeNext,"do not need to load next item,but need to update timer"));
				updateTimer();
				printList("PrimeNext");
			}
			else
			{
				//glog(Log::L_DEBUG,PLSESSID(primeNext,"do not need to load next item,and do not need to update timer"));
			}
			//如果下一个item已经被Load起来就不要updateTimer了
		}
		else
		{
			m_iErrInfoCount++;
			updateTimer();
		}
	}	
}

long Playlist::getSessDurLeft()
{
	if (isCompleted())// || _itCurrent->sessionId ==0 || _itCurrent->bitrate ==0 )//|| _itCurrent->byteOffsetEOS==0 ||_itCurrent->byteOffset==0)
	{
		_isGoodItemInfoData=true;
		return TIMEOUT_INF; 
	}

	if( _currentStatus != PLAYLIST_PLAY )
	{
		_isGoodItemInfoData=true;
		return TIMEOUT_INF;
	}
	{		
		ESESSION_CHARACTERISTICS esessInfo;
		if(!getVstrmSessInfo( _itCurrent, esessInfo ))
		{
// 			glog(Log::L_DEBUG,PLSESSID(getSessDurLeft,"getVstrmSessInfo fail with session id =%u and current Item CtrlNum=%u and fileName=%s"),
// 									_itCurrent->sessionId ,_itCurrent->userCtrlNum,_itCurrent->objectName.string);
			_isGoodItemInfoData=false;
			return REUPDATE_TIMER_TIME;
		}			
		//计算offset
		SESSION_CHARACTERISTICS* pInfo=&(esessInfo.SessionCharacteristics);

		glog(Log::L_DEBUG, PLSESSID( getSessDurLeft, "got attributes of item[%s][%lu] VstrmSess[%lu]: objectSize[%lld] byteOffset[%lld] bitrate[%lu]"),
			_itCurrent->objectName.string , _itCurrent->userCtrlNum , _itCurrent->sessionId,
			pInfo->ObjectSize , pInfo->ByteOffset , pInfo->MuxRate );
		
		int intv=0;

//#if VER_PRODUCTVERSION_MAJOR >= 6
//		intv = (int)((pInfo->EndTimeOffset - pInfo->PlayoutTimeOffset));
//		intv = ( intv < 0 )?( -intv ) : intv;
//#else //VER_PRODUCTVERSION_MAJOR >= 6
		if( pInfo->MuxRate <= 0 )
		{
			if( size() <= 1)
			{
				//glog(Log::L_DEBUG,PLSESSID(getSessDurLeft,"invalid muxRate info , but only 1 item in playlist"));
				_isGoodItemInfoData=true;
				return TIMEOUT_INF;
			}
			else
			{
				glog(Log::L_WARNING, PLSESSID(getSessDurLeft,"VstrmClassGetSessionChars() failed, return invalid muxRate[%lu] for VstrmSess[%lu]"), pInfo->MuxRate, _itCurrent->sessionId);
				_isGoodItemInfoData=false;
				return REUPDATE_TIMER_TIME;
			}
		}
		intv =(int)( ((pInfo->ObjectSize.QuadPart-pInfo->ByteOffset.QuadPart)*8000 )/pInfo->MuxRate);
//#endif//VER_PRODUCTVERSION_MAJOR >= 6		
		
		//intv= (int)  ((LONGLONG)(( pInfo->ObjectSize.QuadPart - pInfo->ByteOffset.QuadPart) *8*1000)) / ((LONGLONG) pInfo->MuxRate );

		_isGoodItemInfoData=true;

		glog(Log::L_DEBUG,PLSESSID(getSessDurLeft,"[%d]ms left for item[%s][%lu] VstrmSess[%lu]"),
							(intv >0) ? intv : 1, _itCurrent->objectName.string, _itCurrent->userCtrlNum , _itCurrent->sessionId );
		
		return (intv >0) ? intv : 1;
	}

// 	if( (_itCurrent->sessionId==0 || _itCurrent->bitrate ==0 ) && _currentStatus==PLAYLIST_PLAY)
// 	{
// 		glog(Log::L_DEBUG,PLSESSID(getSessDurLeft,"(%d)session id=%d bitrate=%d in PLAYLIST_PLAY"),
// 			_itCurrent->userCtrlNum,_itCurrent->sessionId,_itCurrent->bitrate);
// 		_isGoodItemInfoData=false;
// 		return REUPDATE_TIMER_TIME;
// 	}
// 
// 	glog(Log::L_DEBUG,PLSESSID(getSessDurLeft,"ULONG Playlist::getSessDurLeft()##(%d) bitrate=%d byteOffsetEOS=%lld byteOffset=%lld,timeOffset=%u"),
// 							_itCurrent->userCtrlNum, _itCurrent->bitrate,
// 							_itCurrent->byteOffsetEOS,_itCurrent->byteOffset,_itCurrent->timeOffset);
// 	
// 	if(isReverseStreaming())
// 	{
// 		if(_itCurrent->timeOffset != 0 && _itCurrent->byteOffsetEOS==_itCurrent->byteOffset && _itCurrent->byteOffset!=0)
// 		{
// 			glog(Log::L_DEBUG,PLSESSID(getSessDurLeft,"(%d)timeoffset=%u byteEOS=%lld byteoffset=%lld in reverse streaming return REUPDATE_TIMER_TIME=%d"),
// 										_itCurrent->userCtrlNum,_itCurrent->timeOffset ,
// 										_itCurrent->byteOffsetEOS,_itCurrent->byteOffset,
// 										REUPDATE_TIMER_TIME);
// 			_isGoodItemInfoData=false;
// 			return 	REUPDATE_TIMER_TIME;//TEST
// 		}
// 	}
// 	else
// 	{		
// 		if ( (_itCurrent->byteOffsetEOS< _itCurrent->byteOffset))
// 		{
// //			glog(Log::L_DEBUG,PLSESSID("return with gStreamSmithConfig.lPreloadTime to=%d"),gStreamSmithConfig.lPreloadTime+1);
// //			return gStreamSmithConfig.lPreloadTime+1;
// 			glog(Log::L_DEBUG,PLSESSID(getSessDurLeft,"(%d) EOS=%lld byteOffset=%lld  in normal streaming"),
// 				_itCurrent->userCtrlNum, _itCurrent->byteOffsetEOS,_itCurrent->byteOffset);
// 			_isGoodItemInfoData=false;
// 			return REUPDATE_TIMER_TIME;
// 		}
// 		if(_itCurrent->byteOffsetEOS==_itCurrent->byteOffset && _itCurrent->timeOffset==0 )
// 		{
// 			glog(Log::L_DEBUG,PLSESSID(getSessDurLeft,"(%d) EOS=%lld offset=%lld timeoffset=%u"),
// 										_itCurrent->userCtrlNum, _itCurrent->byteOffsetEOS,
// 										_itCurrent->byteOffset,_itCurrent->timeOffset);
// 			_isGoodItemInfoData=false;
// 			return REUPDATE_TIMER_TIME;
// 		}
// 	}
// 	if(/*_itCurrent->byteOffset==0 ||*/ _itCurrent->byteOffsetEOS==0)
// 	{
// 		_isGoodItemInfoData=false;
// 		glog(Log::L_DEBUG,PLSESSID(getSessDurLeft,"no byteoffset or bytreoffsetEOS value return to=%d"),gStreamSmithConfig.lPreloadTime+1000);
// 		return REUPDATE_TIMER_TIME;
// 	}
// 
// 	int intv = (int) ( (( _itCurrent->byteOffsetEOS - _itCurrent->byteOffset) *8*1000) / ( _itCurrent->bitrate ));	
// //	glog(ZQ::common::Log::L_DEBUG, PLSESSID("session(%d) will be termed in %dms\n", _itCurrent->sessionId, intv);
// 	glog(Log::L_DEBUG,PLSESSID(getSessDurLeft,"(%d)return with to=%d"),_itCurrent->userCtrlNum,(intv >0) ? intv : 1);
// 	_isGoodItemInfoData=true;
// 	return (intv >0) ? intv : 1;
}

bool Playlist::setDestMac(IN char* macAddr)
{
	if(!macAddr||strlen(macAddr)<0)
	{
		ERRLOG(Log::L_ERROR,PLSESSID(setDestMac,"empty mac address passed in , reject"));
		return false;
	}
	glog(Log::L_INFO,PLSESSID(setDestMac,"enteringwith mac[%s]"),macAddr);
	m_strDestinationMac=macAddr;
	if(m_strDestinationMac.size()>0)
	{	
		glog(Log::L_DEBUG,PLSESSID(setDestMac,"try to set mac to [%s]"),m_strDestinationMac.c_str());
		int		iTemp[6];
		UCHAR	uMac[6];
		if(sscanf(m_strDestinationMac.c_str(),"%d-%d-%d-%d-%d-%d",&iTemp[0],&iTemp[1],&iTemp[2],&iTemp[3],
			&iTemp[4],&iTemp[5])==6)
		{
			for(int i=0;i<6;i++)
			{
				uMac[i]=(UCHAR)iTemp[i]+'0';
			}
			memcpy(_dvbAttrs.macAddr,uMac,sizeof(_dvbAttrs.macAddr));
			//memcpy(_dvbAttrs.macAddr,m_strDestinationMac.c_str(),sizeof(_dvbAttrs.macAddr));
		}
		else if(sscanf(m_strDestinationMac.c_str(),"%x:%x:%x:%x:%x:%x",&iTemp[0],&iTemp[1],&iTemp[2],&iTemp[3],
			&iTemp[4],&iTemp[5])==6)
		{
			for(int i=0;i<6;i++)
			{
				uMac[i]=(UCHAR)iTemp[i];
			}
			memcpy(_dvbAttrs.macAddr,uMac,sizeof(_dvbAttrs.macAddr));
			//memcpy(_dvbAttrs.macAddr,m_strDestinationMac.c_str(),sizeof(_dvbAttrs.macAddr));
		}
		else if(sscanf(m_strDestinationMac.c_str(),"%2X%2X%2X%2X%2X%2X",&iTemp[0],&iTemp[1],&iTemp[2],&iTemp[3],
			&iTemp[4],&iTemp[5])==6)
		{
			for(int i=0;i<6;i++)
			{
				uMac[i]=(UCHAR)iTemp[i];
			}
			memcpy(_dvbAttrs.macAddr,uMac,sizeof(_dvbAttrs.macAddr));
		}
		else
		{
			ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(setDestMac,"invalid mac format [%s], should be [d-d-d-d-d-d] or [x:x:x:x:x:x] or [2X2X2X2X2X2X]"),macAddr);
			return false;
		}
	}
	glog(Log::L_DEBUG,PLSESSID(setDestMac,"done"));
	return true;
	
}

// Playlist attributes
bool Playlist::setDestination(IN char* ipAddr, IN int udpPort)
{
	char	*pIPAddr;
	if (NULL == ipAddr)
	{
		ERRLOG(Log::L_ERROR,PLSESSID(setDestination,"NULL ipAddr passed in"));
		return false;
	}
	
	pIPAddr = ipAddr;
	int64 stampStart = ZQ::common::now();

	glog(Log::L_DEBUG, PLSESSID(setDestination, "entering with ip[%s] port[%d]"), ipAddr, udpPort);

	_destIP =ipAddr;
	char	szIPStr[256];
	szIPStr[255]='\0';
	strncpy(szIPStr,ipAddr,sizeof(szIPStr)-1);

	//version 6 or version 4
	//int rc = getaddrinfo(nodeName,servName,&hint,&resAddrinfo);
	char szPortStr[32];
	ZeroMemory(szPortStr, sizeof(szPortStr));
	_snprintf(szPortStr, 31, "%d", udpPort);
	addrinfo* resAddrinfo;
	addrinfo hint;
	ZeroMemory(&hint, sizeof(hint));
	hint.ai_family    = AF_INET6;
	hint.ai_socktype  = SOCK_STREAM;
	hint.ai_protocol  = IPPROTO_TCP;
	hint.ai_flags     = AI_PASSIVE;

	// test if it's IPV6
	// NOTE: Windows has a local daemon that does DNS caching. The call to getaddrinfo() is getting routed to
	// that daemon, which presumably is checking its cache before submitting the query to the DNS server.
	// If the invocation encountered long latency, pls disable such a cache by executing: net stop dnscache
	// https://support.microsoft.com/zh-cn/kb/318803
	int rc = getaddrinfo(szIPStr,szPortStr,&hint,&resAddrinfo);
	if (rc !=0 )
	{
		glog(ZQ::common::Log::L_DEBUG, PLSESSID(setDestination ,"no IPv6 address associated for[%s], try IPv4"), szIPStr);
		_dvbAttrs.ipVersion		= IP_VERSION4; //TODO: support IPv6 later
		_dvbAttrs.ipAddr		= inet_addr(pIPAddr);
		_dvbAttrs.udpPort		= udpPort;
		glog(ZQ::common::Log::L_INFO, PLSESSID(setDestination,"set destination[0x%x] port[%d]"), _dvbAttrs.ipAddr, _dvbAttrs.udpPort);
	}
	else
	{
		glog(ZQ::common::Log::L_DEBUG, PLSESSID(setDestination ,"converted [%s] to Ipv6 address"), szIPStr);
		_dvbAttrs.ipVersion		= IP_VERSION6;
		sockaddr_in6* in6 = ( sockaddr_in6* ) resAddrinfo->ai_addr;
		memcpy(&_dvbAttrs.ip6Addr.ipVer , &in6->sin6_addr , sizeof(_dvbAttrs.ip6Addr.ipVer));
		_dvbAttrs.ip6Addr.flow	=	in6->sin6_flowinfo;

		_dvbAttrs.ip6Addr.hopLimit = IPV6_HOPLIMIT_DFLT;

		_dvbAttrs.ip6Addr.ip_class = 0;
		_dvbAttrs.udpPort		=  udpPort; //in6->sin6_port;
		freeaddrinfo(resAddrinfo);
	}

	glog(Log::L_INFO, PLSESSID(setDestination, "ip[%s] port[%d], took %dmsec"), ipAddr, udpPort, (int)(ZQ::common::now() - stampStart));
	return (0 !=_dvbAttrs.ipAddr);
}

bool Playlist::setMuxRate(IN ULONG NowRate, IN ULONG MaxRate, IN ULONG MinRate)
{
	//glog(Log::L_DEBUG,PLSESSID(setMuxRate,"entering with maxRate=%u(kbps) nowRate=(%u(kbps)"),MaxRate/1000,NowRate/1000);
	// Set stream's current bit rate and maximum bitrate.
	_dvbAttrs.MaxMuxRate = MaxRate; 
	_dvbAttrs.NowMuxRate = NowRate; 
	_dvbAttrs.MinMuxRate = MinRate;
	glog(Log::L_INFO, PLSESSID(setMuxRate,"taking maxRate=%lu(kbps) nowRate=(%lu(kbps)"),MaxRate/1000,NowRate/1000);
	return true;
}

bool Playlist::setProgramNumber(IN USHORT ProgramNumber)
{
	// Set stream's current bit rate and maximum bitrate.
	//glog(Log::L_DEBUG,PLSESSID(setProgramNumber,"enteringwith programNumber[%u]"),ProgramNumber);
	_dvbAttrs.ProgramNumber = ProgramNumber;
	glog(Log::L_INFO, PLSESSID(setProgramNumber,"taking programNumber[%d]"), ProgramNumber);
	return true;
}

bool Playlist::setAuth(IN const char* endpoint)
{
	if (NULL == endpoint)
		return false;

	_authEndpoint = endpoint;
 	return !_authEndpoint.empty();
}

const char* getStateString(IPlaylist::State plState,char* szBuf,int size)
{
	memset(szBuf,0,size);
	switch(plState) 
	{
	case IPlaylist::PLAYLIST_SETUP:
		{
			return "PLAYLIST_SETUP";
		}
		break;
	case IPlaylist::PLAYLIST_PAUSE:
		{
			return	"PLAYLIST_PAUSE";
		}
		break;
	case IPlaylist::PLAYLIST_PLAY:
		{
			return "PLAYLIST_PLAY";
		}
		break;
	case IPlaylist::PLAYLIST_STOP:
		{
			return "PLAYLIST_STOP";
		}
		break;
	default:
		return "";
	}
}

void Playlist::destroy()
{
	glog(Log::L_INFO,PLSESSID(destroy,"entering destroy"));
	try
	{
		ZQ::common::Variant	var;
		_stampStateChangeReq = GetTickCount();
		m_pStreamSite->PostEventSink(E_PLAYLIST_DESTROYED,var,_strGuid);
		//delete this;
		ClearAllResource();
	}
	catch (...)
	{
	}
	glog(Log::L_INFO, PLSESSID(destroy,"leave destroy, _stampStateChangeReq[%u]"),_stampStateChangeReq);
}

bool Playlist::setSpeed(const float newSpeed)
{	
	StreamControlResultInfo info;
	return doChangeSpeed(newSpeed,false,info);
}

#ifndef  STATUS_FILE_CLOSED
	#define STATUS_FILE_CLOSED               ((NTSTATUS)0xC0000128L)
#endif
bool isSuccessErrorCode( long err )
{
	switch(err)
	{
	case STATUS_FILE_CLOSED:
		return true;
		break;			
	default:
		return false;
	}
	return false;
}

//make a table to translate vstrm error to TianShan error
int tranlasteVstrmErrorCodeToTianShanErrorCode( int vstrmErrCode )
{
	if( IS_VSTRM_SUCCESS( vstrmErrCode ) || isSuccessErrorCode( vstrmErrCode ) )
		return TianShanIce::Streamer::seSuccess;

	switch (vstrmErrCode)
	{
	case VSTRM_NO_VSTRM_OBJECT_PATH : 
	case VSTRM_INVALID_OBJECT_PATH : 
	case VSTRM_CRC_ERROR : 
	case VSTRM_NO_SUCH_FILE : 
	case VSTRM_PROVIDER_TIMEOUT : 
	case VSTRM_DATA_UNDERFLOW : 
	case VSTRM_INTERNAL_FILE_ERROR : 
	case VSTRM_FILE_CORRUPT : 
	case VSTRM_SCSI_REQUEST_ABORTED : 
	case VSTRM_SCSI_BUS_RESET : 
	case VSTRM_SCSI_PARITY_ERROR : 
	case VSTRM_SCSI_DATA_OVERRUN : 
	case VSTRM_SCSI_ERROR_RECOVERY : 
	case VSTRM_SCSI_END_OF_DATA : 
	case VSTRM_DATA_ERROR : 
	case VSTRM_ACCESS_DENIED : 
	case VSTRM_BANDWIDTH_EXCEEDED :
		//4400	Error Reading Content Data	The ODRM should tear down the stream, and pass the Announce to the SM. 
		return TianShanIce::Streamer::seReadingContentData;
		break; 
	
	case VSTRM_INSUFICIENT_RESOURCES : 
	case VSTRM_NO_SUCH_DEVICE : 
	case VSTRM_DEVICE_NOT_READY : 
	case VSTRM_LINK_DOWN : 
		//5200	Server Resources Unavailable	The ODRM should tear down the stream, and pass the Announce to the SM. 
		return TianShanIce::Streamer::seDownstreamFailure;
		break;

	case VSTRM_UNKNOWN_CONSUMER_ERROR : 
	case VSTRM_CONSUMER_NOT_READY : 
	case VSTRM_NETWORK_NOT_READY : 
	case VSTRM_NETWORK_LINK_FAILED : 
		//5401	Downstream Failure N/A 
		return TianShanIce::Streamer::seDownstreamFailure;
		break;

// 	case VSTRM_BANDWIDTH_EXCEEDED : 
// 		//5602	Bandwidth Exceeded Limit	The ODRM should tear down the stream, and pass the Announce to the SM. 
// 		return TianShanIce::Streamer::seBandwidthExceeded;
// 		break;
		
	case VSTRM_ARP_FAILED : 
	case VSTRM_NETWORK_UNREACHABLE : 
		//6004	Downstream Destination Unreachable	The ODRM should tear down the stream, and pass the Announce to the SM. 
		return TianShanIce::Streamer::seDownstreamDestUnreachable;
		break; 

	default:
		return TianShanIce::Streamer::seReadingContentData;
	}
}

int Playlist::getNptFromItem( iterator it )
{
	ZQ::common::MutexGuard gd(_listOpLocker);
	iterator itTemp = _itCurrent;
	_itCurrent = it;
	ULONG lret = CalculatePastTime();
	_itCurrent = itTemp;
	return (int)lret;
}

int Playlist::getNptPrimaryFromItem( iterator it)
{
	ZQ::common::MutexGuard gd(_listOpLocker);
	iterator itTemp = _itCurrent;
	_itCurrent = it;
	ULONG lret =   CalculatePastTimeNPTPriamry();
	_itCurrent = itTemp;
	return (int)lret;
}

void Playlist::OnItemStepped( const iterator prevItem ,const iterator nextItem,int curTimeOffset /*= 0 */, const std::string& reason , int errCode , StreamControlResultInfo* controlInfo )
{
	if(!m_pStreamSite)
	{
		m_pStreamSite=(StreamSmithSite*)StreamSmithSite::getDefaultSite();
	}
	if(m_pStreamSite)
	{
		long lSeq = InterlockedIncrement( &_eventCSeqItemStepped );
		try
		{
			std::string	strPrevItem,strNextItem;
			int ctrlPrev=INVALID_CTRLNUM,ctrlNext=INVALID_CTRLNUM;
#ifdef NEED_EVENTSINK_RAWDATA
			
#else
			//get current time
			//snprintf
			SYSTEMTIME st_utc;
			GetSystemTime(&st_utc);

			char	szNowUTC[256];
			ZeroMemory(szNowUTC,sizeof(szNowUTC));
			snprintf(szNowUTC, 255,  "%04d%02d%02dT%02d%02d%02d.%03dZ", 
						st_utc.wYear, st_utc.wMonth, st_utc.wDay,
						st_utc.wHour, st_utc.wMinute, st_utc.wSecond, 
						st_utc.wMilliseconds);

			ZQ::common::Variant var;

				ZQ::common::Variant varExtraProps;
				addExtraPropertiesToVariant( controlInfo , varExtraProps );
			var.set( EventField_ExtraProperties , varExtraProps );

			var.set(EventField_EventCSEQ ,	lSeq );
			var.set(EventField_StampUTC,	std::string(szNowUTC));
			var.set(EventField_PlaylistGuid, _strGuid);
			var.set(EventField_ClientSessId,	m_strClientSessionID);
			var.set(EventField_CurrentItemTimeOffset,	curTimeOffset);	
			var.set(EventField_PrevItemTimeOffset, _lastItemNpt );

			var.set(EventField_clusterId,	(long)GAPPLICATIONCONFIGURATION.mediaClusterId);
			var.set(EventField_ItemExitReason, reason );
			
			if( _lastItemStepErrorCode < 0 )
			{				
				_lastItemStepErrorCode = tranlasteVstrmErrorCodeToTianShanErrorCode(_lastItemStepErrorCode);//translate error code
				var.set( EventField_AnnounceLastErrorCode , _lastItemStepErrorCode );
				var.set( EventField_AnnounceLastItemName , _lastItemStepItemName );
				var.set( EventField_AnnounceLastErrDesc , _lastItemStepErrDesc );
			}
			else
			{
				var.set( EventField_AnnounceLastErrorCode , 0 );
				var.set( EventField_AnnounceLastItemName , std::string("") );
				var.set( EventField_AnnounceLastErrDesc , std::string("") );
			}			

			_lastItemStepErrorCode = 0;//reset it to 0 because we have sent a message out
			int prevCtrlNum = 0;
			int curCtrlNum = 0 ;

			if ( prevItem >= listEnd() || prevItem<= listStart() ) 
			{
				if(prevItem <= listStart())
				{
					var.set( EventField_UserCtrlNum ,	(int)TianShanIce::Streamer::PlaylistHeadCtrlNum	);
					prevCtrlNum = (int)TianShanIce::Streamer::PlaylistHeadCtrlNum;
				}
				else
				{
					var.set( EventField_UserCtrlNum ,	(int)TianShanIce::Streamer::PlaylistTailCtrlNum	);
					prevCtrlNum = (int)TianShanIce::Streamer::PlaylistTailCtrlNum;
				}
				var.set( EventField_ItemFileName ,	std::string(""));
				
				var.set( EventField_prevProviderId ,std::string("") );
				var.set( EventField_prevProviderAssetId , std::string("") );
				
				var.set( EventField_PrevStreamingSource , std::string("") );
				var.set( EventField_PreviousePlayPos , (int)0 );
				var.set( EventField_PreviousePlayPosPrimary, (int)0 ); 
				if (iterValid(_itCurrent))
				{
					var.set( EventField_CurrPlayTime, (long)0);
					var.set( EventField_PrevPlayTime, (long)0);
					var.set( EventField_CurrFlag, (ULONG)0);
					var.set( EventField_PrevFlag, (ULONG)0);			
				}		
			}
			else
			{
				var.set( EventField_UserCtrlNum,	ZQ::common::Variant((int)prevItem->userCtrlNum ));
				var.set( EventField_ItemFileName,	ZQ::common::Variant(prevItem->_rawItemName) );	
				strPrevItem = prevItem->_rawItemName;
				ctrlPrev	= prevItem->userCtrlNum;
				
				prevCtrlNum = ctrlPrev;
				
				var.set( EventField_prevProviderId ,		prevItem->_strProviderId );
				var.set( EventField_prevProviderAssetId ,	prevItem->_strProviderAssetId);
				
				var.set( EventField_PreviousePlayPos , _lastplaylistNpt );
				var.set( EventField_PreviousePlayPosPrimary, _lastplaylistNptPrimary );// npt
				
				
				if (iterValid(_itCurrent))
				{
					if (IsItemPlaytimesCountZero(_itCurrent)) //current item doesn't exists
					{
						var.set( EventField_CurrPlayTime, (long)0);
						var.set( EventField_PrevPlayTime, (long)0);
						var.set( EventField_PrevFlag, (ULONG)0);
						var.set( EventField_CurrFlag, (ULONG)0);
					}
					else
					{
						var.set( EventField_CurrPlayTime, _itCurrent->_itemPlaytime);
						var.set( EventField_PrevPlayTime, prevItem->_itemPlaytime);
						var.set( EventField_PrevFlag, prevItem->itemFlag);
						var.set( EventField_CurrFlag, _itCurrent->itemFlag);
					}
				}
		
				if( ( strlen(prevItem->_itemLibraryUrl) != 0 ) && ( gStreamSmithConfig.serverMode == 2 ) )
				{					
					var.set( EventField_PrevStreamingSource ,	prevItem->_itemLibraryUrl );
				}
				else
				{
					var.set( EventField_PrevStreamingSource , std::string("") );
				}
			}
			
			if ( nextItem>=listEnd() || nextItem<=listStart() ) 
			{
				if(nextItem <= listStart())
				{					
					var.set( EventField_NextUserCtrlNum, (int)TianShanIce::Streamer::PlaylistHeadCtrlNum);
					curCtrlNum = (int)TianShanIce::Streamer::PlaylistHeadCtrlNum;
				}
				else
				{
					var.set( EventField_NextUserCtrlNum, (int)TianShanIce::Streamer::PlaylistTailCtrlNum);
					curCtrlNum = (int)TianShanIce::Streamer::PlaylistTailCtrlNum;
				}
				
				var.set( EventField_ItemOtherFileName,"");
				var.set( EventField_currentPoviderId , std::string("") );
				var.set( EventField_currentProviderAssetId ,  std::string("")  );
				var.set( EventField_CurStreamingSource , std::string("") );
				if( nextItem <= listStart() )
					var.set( EventField_CurrentPlayPos , (int)0 );
				else
					var.set( EventField_CurrentPlayPos , getNptFromItem( listEnd() ) );
				

			}
			else
			{
				var.set( EventField_NextUserCtrlNum,ZQ::common::Variant((int)nextItem->userCtrlNum));
				var.set( EventField_ItemOtherFileName,std::string(nextItem->_rawItemName));
				strNextItem = nextItem->_rawItemName;
				ctrlNext = nextItem->userCtrlNum;
				curCtrlNum = ctrlNext;

				var.set( EventField_currentPoviderId , nextItem->_strProviderId);
				var.set( EventField_currentProviderAssetId , nextItem->_strProviderAssetId );
				var.set( EventField_CurrentPlayPos , ( getNptFromItem( nextItem ) + curTimeOffset ) );

				if ( ( strlen(nextItem->_itemLibraryUrl) != 0 ) && ( gStreamSmithConfig.serverMode == 2 )  )
				{				
					var.set( EventField_CurStreamingSource , nextItem->_itemLibraryUrl );				
				}
				else
				{					
					var.set( EventField_CurStreamingSource , std::string("") );
				}
			}

			Variant varCurSpeed;
			varCurSpeed.clear();
			varCurSpeed.set(EventField_SpeedNumer,ZQ::common::Variant((int)_crntSpeed.numerator));
			varCurSpeed.set(EventField_SpeedDenom,ZQ::common::Variant((int)_crntSpeed.denominator));
			var.set(EventField_CurrentSpeed,varCurSpeed);
			var.set( EventField_TotalDuration, getTotalStreamDuration());
			var.set ( EventField_TotalVideoDuration, getTotalVideoStreamDuration());
			if( iterValid(nextItem) )
			{
				var.set(EventField_CurrentState, ((long)_currentStatus) );
				if (nextItem->_itemPlayTimesCount >0)
				{
					nextItem->_itemPlayTimesCount --;
					glog(ZQ::common::Log::L_INFO,PLSESSID(OnItemStepped,"ad PlayTimes: [%d], flag: [%s]"), nextItem->_itemPlayTimesCount, convertItemFlagToStr(nextItem->itemFlag).c_str());
				}
			}
			else
			{
				var.set(EventField_CurrentState,((long)PLAYLIST_STOP));
			}
			
#endif

			glog(Log::L_INFO,PLSESSID(OnItemStepped, "event(E_PLAYLIST_ITEMDONE) is fired . item stepped from [%s][%u] to [%s][%u] and seq[%ld]"),
						strPrevItem.c_str(),ctrlPrev,
						strNextItem.c_str(),ctrlNext,
						lSeq);
			glog(ZQ::common::Log::L_DEBUG,PLSESSID(OnItemStepped,"transition [%d] to [%d]"),prevCtrlNum , curCtrlNum);

			m_pStreamSite->PostEventSink( E_PLAYLIST_ITEMDONE , var , _strGuid);
		}
		catch (...) 
		{			
		}
	}
}
void Playlist::OnItemDoneDummy(int PrevCtrlNum ,const std::string& strFileName , iterator nextItemIter)
{
	if(!m_pStreamSite)
	{
		m_pStreamSite=(StreamSmithSite*)StreamSmithSite::getDefaultSite();
	}
	if(m_pStreamSite)
	{
		long lSeq = InterlockedIncrement(&_eventCSeqItemStepped);
		try
		{
#ifdef NEED_EVENTSINK_RAWDATA
			
#else
			ZQ::common::Variant var;
			var.set(EventField_EventCSEQ , lSeq );
			var.set(EventField_PlaylistGuid,_strGuid);
			//completedItem->userCtrlNum
			var.set(EventField_UserCtrlNum,ZQ::common::Variant((int)PrevCtrlNum ));
			var.set(EventField_ItemFileName,ZQ::common::Variant(strFileName) );
			var.set(EventField_ClientSessId,m_strClientSessionID);

			if ( nextItemIter>=listEnd() || nextItemIter<=listStart() ) 
			{
				var.set(EventField_NextUserCtrlNum,ZQ::common::Variant((int)INVALID_CTRLNUM));
				var.set(EventField_ItemOtherFileName,"");
			}
			else
			{
				var.set(EventField_NextUserCtrlNum,ZQ::common::Variant((int)nextItemIter->userCtrlNum));
				var.set(EventField_ItemOtherFileName,std::string(nextItemIter->_rawItemName));
			}
			
#endif
			glog(Log::L_INFO,PLSESSID(OnItemDone,"event E_PLAYLIST_ITEMDONE is fired with item[%s][%u] and seq=%ld"),
				"", PrevCtrlNum, lSeq );
			m_pStreamSite->PostEventSink(E_PLAYLIST_ITEMDONE,var,_strGuid);
		}
		catch (...)
		{			
		}
	}
	else
	{
		//ERRLOG(Log::L_ERROR,PLSESSID("void Playlist::OnVstrmSessProgress()## m_pStreamSite==NULL,can't send sink event"));
	}
}
void Playlist::OnItemDone(iterator completedItem,iterator itNext)
{
	
#if _USE_NEW_SESSION_MON
	//UnRegisterSessID(_lastSessionID);
#endif
//#pragma message(__MSGLOC__"是否有必要在这里检查当前的vtrsm session是否真的结束?????")
	if(!iterValid( completedItem) )
	{
		glog(ZQ::common::Log::L_ERROR,PLSESSID(OnItemDone,"invalid done item"));
		return ;
	}
	
	completedItem->sessionId=0;

	if(!m_pStreamSite)
	{
		m_pStreamSite=(StreamSmithSite*)StreamSmithSite::getDefaultSite();
	}
	if(m_pStreamSite)
	{
		long lSeq = InterlockedIncrement( & _eventCSeqItemStepped );
		try
		{
#ifdef NEED_EVENTSINK_RAWDATA
			
#else
			ZQ::common::Variant var;
			var.set(EventField_EventCSEQ , lSeq );
			var.set(EventField_PlaylistGuid,_strGuid);
			var.set(EventField_UserCtrlNum,ZQ::common::Variant((int)completedItem->userCtrlNum));
			var.set(EventField_ItemFileName,ZQ::common::Variant(completedItem->_rawItemName));
			var.set(EventField_ClientSessId,m_strClientSessionID);
			
			if (itNext>=listEnd()||itNext<=listStart()) 
			{
				var.set(EventField_NextUserCtrlNum,ZQ::common::Variant((int)INVALID_CTRLNUM));
				var.set(EventField_ItemOtherFileName,std::string(""));
			}
			else
			{
				var.set(EventField_NextUserCtrlNum,ZQ::common::Variant((int)itNext->userCtrlNum));
				var.set(EventField_ItemOtherFileName,std::string(itNext->_rawItemName));
			}
			
#endif
			glog(Log::L_INFO,PLSESSID(OnItemDone,"event(E_PLAYLIST_ITEMDONE_ is fired with item[%s][%lu] and seq[%ld]"),				
				completedItem->objectName.string ,
				completedItem->userCtrlNum,
				lSeq );
			m_pStreamSite->PostEventSink(E_PLAYLIST_ITEMDONE,var,_strGuid);
		}
		catch (...)
		{			
		}
	}
	else
	{
		//ERRLOG(Log::L_ERROR,PLSESSID("void Playlist::OnVstrmSessProgress()## m_pStreamSite==NULL,can't send sink event"));
	}
}	


bool Playlist::PlayListInitOK()
{
	return _vstrmPortNum<(ULONG)_mgr._cls.getPortCount();
}
void Playlist::OnPlaylistDone( const std::string& reason/*=""*/ )
{
	//glog(Log::L_DEBUG,PLSESSID(OnPlaylistDone,"set playlist state to  PLAYLIST_STOP because playlist done"));
	endTimer64();
	ZQ::common::MutexGuard gd(_listOpLocker);
	FireStateChanged(PLAYLIST_STOP);
	//_currentStatus=PLAYLIST_STOP;
	//should send a event sink to plug in to announce that play list is done!	
	if(!m_pStreamSite)
	{
		m_pStreamSite=(StreamSmithSite*)StreamSmithSite::getDefaultSite();
	}
	if(m_pStreamSite)
	{
		
		try
		{//Post PlayList end event 
			ZQ::common::Variant var;
			var.set(EventField_ClientSessId,m_strClientSessionID);

			//set current to listEnd so we can get the total NPT
			iterator itTemp = _itCurrent;
			_itCurrent = listEnd();
			ULONG lTotal = CalculatePastTime();
			var.set(EventField_TotalTimeOffset,(long)lTotal );
			_itCurrent = itTemp;

			var.set(EventField_PlaylistGuid,_strGuid/*ZQ::common::Variant(szBuf)*/);
			var.set(EventField_ItemExitReason,reason);
				
				ZQ::common::Variant varSpeed;
				varSpeed.clear();
				varSpeed.set(EventField_SpeedDenom,ZQ::common::Variant((int)_crntSpeed.denominator));
				varSpeed.set(EventField_SpeedNumer,ZQ::common::Variant((int)_crntSpeed.numerator));
			var.set( EventField_CurrentSpeed , varSpeed );

			int iCtrlNum = -2;

			if( _itCurrent>=listEnd())
			{
				long lSeq = InterlockedIncrement(&_eventCSeqEndofStream);
				var.set(EventField_EventCSEQ , lSeq );
				glog(Log::L_INFO,PLSESSID(OnPlaylistDone,"event(E_PLAYLIST_END), reach the end of stream and seq[%ld]"),lSeq);
				
				int streamPos = getNptFromItem( listEnd() );
				var.set( EventField_CurrentPlayPos , streamPos );

				//use last item npt because end-of-stream
				var.set( EventField_CurrentItemTimeOffset , _lastItemNpt );
				
				iterator it = listEnd() - 1;
				if( iterValid(it))
				{
					iCtrlNum = it->userCtrlNum;
				}

				var.set( EventField_UserCtrlNum ,  iCtrlNum );

				m_pStreamSite->PostEventSink(E_PLAYLIST_END,var,_strGuid);
			}
			else
			{
				long lSeq = InterlockedIncrement(&_eventCSeqBegginingOfStream);
				var.set(EventField_EventCSEQ , lSeq );
				glog(Log::L_INFO,PLSESSID(OnPlaylistDone,"event(E_PLAYLIST_BEGIN) is fired, reach the beginning of the stream and seq[%ld]"),lSeq);

				var.set( EventField_CurrentPlayPos , 0 );
				//use last item npt because begin-of-stream
				var.set( EventField_CurrentItemTimeOffset , _lastItemNpt );

				iterator it = listStart() + 1;
				if( iterValid(it))
				{
					iCtrlNum = it->userCtrlNum;
				}
				var.set( EventField_UserCtrlNum ,  iCtrlNum );

				m_pStreamSite->PostEventSink(E_PLAYLIST_BEGIN,var,_strGuid);
			}
		}
		catch (...) 
		{
			//ERRLOG(Log::L_ERROR,PLSESSID(OnPlaylistDone,"void Playlist::OnPlaylistDone()##unexpect exception was threw out when call "));
		}
	}

	_isGoodItemInfoData	=true;
	SetTimerEx( gStreamSmithConfig.lPlaylistTimeout );
	//当放完以后直接复位
	_crntSpeed			= Playlist::SPEED_NORMAL;
	_b1stPlayStarted	= false;
	_itCurrent			= listBegin();
	_isReverseStreaming = false;
	_itNext				= _itCurrent+1;
	_bCleared 			= false;
	_lastplaylistNpt	= 0;
	_lastplaylistNptPrimary = 0;
	
	glog(ZQ::common::Log::L_DEBUG, PLSESSID(OnPlaylistDone,"done"));
}

void Playlist::setUserCtxIdx(const char* SessID)
{
	if (NULL==SessID || 0x00 == *SessID)
	{
		m_strClientSessionID ="";
#ifdef VSTRM_ONDEMANDE_SESSID_ON
		// eventIS doesn't provide an OnDemandSessionId, so take the guid of playlist by default
		ZQ::common::Guid::UUID guid = (ZQ::common::Guid::UUID) ZQ::common::Guid(_strGuid.c_str());
		memcpy(&_userSessGuid, &guid, sizeof(_userSessGuid));
#endif // VSTRM_GUID
	}
	else
	{
		m_strClientSessionID=SessID;
#ifdef VSTRM_ONDEMANDE_SESSID_ON
		ZQ::common::Guid::UUID guid= (ZQ::common::Guid::UUID) ZQ::common::Guid(SessID);
		if (0 == guid.Data1 && 32 == strlen(SessID) && NULL == strchr(SessID, '-'))
		{
			// sample guid 01234567-8901-2345-6789-012345678901
			//             012345678901234567890123456789012345
			char buf[40];
			strncpy(&buf[0],  SessID,    8);   buf[8] ='-';
			strncpy(&buf[9],  SessID+8,  4);   buf[13]='-';
			strncpy(&buf[14], SessID+12, 4);   buf[18]='-';
			strncpy(&buf[19], SessID+16, 4);   buf[23]='-';
			strncpy(&buf[24], SessID+20, 12);  buf[36]='\0';
			guid= (ZQ::common::Guid::UUID) ZQ::common::Guid(buf);
		}
		memcpy(&_userSessGuid, &guid, sizeof(_userSessGuid));
#endif // VSTRM_GUID
	}
	glog(Log::L_DEBUG,PLSESSID(setUserCtxIdx,"take UserIdx[%s]"), m_strClientSessionID.c_str());
}

int Playlist::getCurrentItemNpt()
{
	int lRet = 0;
	ZQ::common::Variant var;

	if( getInfo( IPlaylist::infoPLAYPOSITION, var ) )
	{
		if( var.has(EventField_CurrentTimeOffset) )
		{
			lRet = (int)var[EventField_CurrentTimeOffset];
		}
	}
	return lRet;
}
int Playlist::getCurrentPlaylistNpt()
{
	int lRet = 0;
	ZQ::common::Variant var;
	
	if( getInfo( IPlaylist::infoPLAYNPTPOS, var ) )
	{
		if( var.has(EventField_CurrentTimeOffset) )
		{
			lRet = (int)var[EventField_CurrentTimeOffset];
		}
	}
	return lRet;
}

bool Playlist::getInfo(IN unsigned long mask, ZQ::common::Variant& var  )
{
	bool bLocalCall = (mask & 0xF0000000) == 0;
	mask = 0x0FFFFFFF & mask;

	if(!bLocalCall)
		glog(Log::L_DEBUG,PLSESSID(getInfo,"entering with mask =[%lu]"), mask );

	switch(mask)
	{
	case IPlaylist::infoSTREAMSOURCE:
		{
			VstrmClass::PORTCHARRACTER portChar;
			if(!_mgr._cls.GetPortProperty(_vstrmPortNum , portChar))
			{
				glog(ZQ::common::Log::L_ERROR,PLSESSID(getInfo ,"can't get port character with port[%lu]"),_vstrmPortNum );
			}
			var.set(EventField_StreamSourceIp,portChar.sourceIp);
			var.set(EventField_StreamSourcePort,(int)portChar.sourcePort);
			glog(ZQ::common::Log::L_INFO,PLSESSID(getInfo,"get source ip[%s] port[%d] for VstrmPort[%lu]"),
				portChar.sourceIp.c_str() , portChar.sourcePort,_vstrmPortNum);
		}
		break;
	case IPlaylist::infoDVBCRESOURCE:
		{
			glog(Log::L_DEBUG, PLSESSID(getInfo,"got dvbc resource information"));
			var=_varQamResource;
		}
		break;
	case IPlaylist::infoPLAYNPTPOS:
		{
			int iItemIndex = 0;
			int iItemOffset = 0;
			
			ZQ::common::MutexGuard gd(_listOpLocker);

			if(size()<=0)
			{
				ERRLOG(Log::L_ERROR, PLSESSID(getInfo,"playlist is empty, reject"));
				_lastErrCode=ERR_PLAYLIST_INVALIDSTATE;
				return false;
			}
			
			if(_itCurrent<=listStart() || _itCurrent>=listEnd())
			{
				if(!bLocalCall)
					ERRLOG(Log::L_WARNING, PLSESSID(getInfo, "current item is not running, reject"));
				_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
				return false;
			}

			iItemIndex = static_cast<int>( _itCurrent - listBegin() ) + 1;

			if(!bLocalCall)
				glog(Log::L_DEBUG, PLSESSID(getInfo,"get playposition info for item[%s][%lu] VstrmSess[%lu] "),
						_itCurrent->objectName.string, _itCurrent->userCtrlNum, _itCurrent->sessionId);

			/*
			Get current NPT and stream duration
			-------------------^^^^^^^^^+++++++++++++++++++
			previous            current  future
			*/
			int		curOffset	= 0;
			int		totalOffset = 0;
			float	scale = 0.0f;
			iterator itItem = listBegin( );

			for( ; itItem != listCurrent() ; itItem ++ )
			{
				if (itItem->_itemPlaytime < 0 ) 
				{//the playtime is not available,query from Content Store
					if( !checkContentAttribute(itItem) )
					{
						_lastErrCode =  ERR_PLAYLIST_SERVER_ERROR;
						ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(getInfo,"failed to get attributs of item[%s][%lu]"),
							itItem->_rawItemName,itItem->userCtrlNum);
						return false;
					}
					else
					{
						if(!bLocalCall)
							glog(ZQ::common::Log::L_DEBUG,PLSESSID(getInfo,"got attributes of item[%s][%lu]: playtime[%ld], bitrate[%ld]"),
								itItem->_rawItemName,itItem->userCtrlNum, itItem->_itemPlaytime,itItem->_itemBitrate);
					}
				}
				
				curOffset	+= itItem->_itemPlaytime;
				totalOffset += itItem->_itemPlaytime;;
			}

			if ( itItem == listCurrent() && (_itCurrent->sessionId != 0) )
			{
				int tempItemOffset = 0;
				int tempItemDuration = 0;
				if( !getStrmSessAttribute( _itCurrent , tempItemOffset , scale , tempItemDuration ,false , bLocalCall ) )
				{
					ERRLOG(ZQ::common::Log::L_ERROR, PLSESSID(getInfo,"can't get stream attribute"));
					_lastErrCode = ERR_PLAYLIST_SERVER_ERROR;
					return false;
				}
				if( _itCurrent->inTimeOffset != 0 )
				{
					if(!bLocalCall)
						glog(ZQ::common::Log::L_DEBUG, PLSESSID(getInfo,"item[%s] session[%lu] ctrlNum[%lu]'s inTimeOffset [%lu] itemDuration[%d] scale[%f],adjust itemOffset from [%d] to [%d]"),
							_itCurrent->objectName.string ,
							_itCurrent->sessionId,
							_itCurrent->userCtrlNum,
							_itCurrent->inTimeOffset,
							tempItemDuration,
							scale,
							tempItemOffset,
							tempItemOffset - _itCurrent->inTimeOffset);
					
					tempItemOffset		= tempItemOffset - _itCurrent->inTimeOffset;

					if(tempItemOffset <0 )
						tempItemOffset = 0;
				}
				else
				{
					if(!bLocalCall)
						glog(ZQ::common::Log::L_DEBUG, PLSESSID(getInfo,"item[%s] session[%lu] ctrlNum[%lu]'s inTimeOffset [%lu] itemDuration[%d] scale[%f]"),
							_itCurrent->objectName.string ,
							_itCurrent->sessionId,
							_itCurrent->userCtrlNum,
							_itCurrent->inTimeOffset,
							tempItemDuration,
							scale );
				}
				//if we can't get the item duration , tempItemDuration will be set to 0
				_itCurrent->_itemPlaytime	= tempItemDuration;	

				iItemOffset					= tempItemOffset;

				curOffset					+= tempItemOffset;
				totalOffset					+= tempItemDuration;				
			}
			
			if( itItem != listEnd())
				itItem ++;

			for( ; itItem != listEnd() ; itItem ++ )
			{
				if (itItem->_itemPlaytime < 0) 
				{//the playtime is not available,query from Content Store
					if( !checkContentAttribute(itItem) )
					{
						_lastErrCode =  ERR_PLAYLIST_SERVER_ERROR;
						ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(getInfo,"query item %s userCtrlNum[%lu] "
							"playtime and bitrate failed"),
							itItem->_rawItemName,itItem->userCtrlNum);
						return false;
					}
					else
					{
						if(!bLocalCall)
							glog(ZQ::common::Log::L_DEBUG, PLSESSID(getInfo,"query item [%s] userCtrlNum[%lu] playtime [%ld] and bitrate [%ld]"),
								itItem->_rawItemName,itItem->userCtrlNum, itItem->_itemPlaytime,itItem->_itemBitrate);
					}
				}

				//curOffset	+= itItem->_itemPlaytime;
				totalOffset += itItem->_itemPlaytime;
			}		
			char szBuf[24];
			sprintf(szBuf,"%f",scale);
			var.set( EventField_PlaylistGuid,_strGuid);
			var.set( EventField_CurrentTimeOffset, (int)curOffset );			
			var.set( EventField_PlayScale, std::string(szBuf));
			var.set( EventField_TotalTimeOffset, int(totalOffset));
			var.set( EventField_ItemIndex, iItemIndex );
			var.set( EventField_ItemOffset,iItemOffset );
			glog(ZQ::common::Log::L_INFO, PLSESSID( getInfo , "get playlist information with curNPT[%d] , totalNPT[%d] ,scale[%s]" ), curOffset , totalOffset, szBuf );
		}
		break;
	case IPlaylist::infoPLAYPOSITION:
		{
			int iItemIndex = 0;
			
			if( size() <= 0 )
			{
				if(!bLocalCall)
					ERRLOG( Log::L_ERROR, PLSESSID(getInfo, "No items in playlist" ) );
				_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
				return false;
			}
			ZQ::common::MutexGuard gd( _listOpLocker );
			if(_itCurrent <= listStart() || _itCurrent >= listEnd())
			{
				if(!bLocalCall)
					ERRLOG(Log::L_ERROR , PLSESSID(getInfo , "current item's stream is down") );
				_lastErrCode = ERR_PLAYLIST_INVALIDSTATE;
				return false;
			}

			iItemIndex = static_cast<int>( _itCurrent - listBegin() ) + 1;

			if(!bLocalCall)
				glog(Log::L_DEBUG,PLSESSID( getInfo , "get playposition info with ctrlNum[%lu] and filename[%s] sessionid[%lu] "),
												_itCurrent->userCtrlNum , _itCurrent->objectName.string , _itCurrent->sessionId );

			int	cOffset = 0 , totalOffset = 0 ;
			float scale = 0.0f ;			
			if(!getStrmSessAttribute( _itCurrent, cOffset, scale, totalOffset,false, bLocalCall ))
			{
				ERRLOG(ZQ::common::Log::L_ERROR, PLSESSID(getInfo, "Can't get stream attribute") );
				_lastErrCode = ERR_PLAYLIST_SERVER_ERROR;
				return false;
			}
			cOffset -= _itCurrent->inTimeOffset ; 
			if ( cOffset < 0 ) 
			{
				if(!bLocalCall)
					glog(ZQ::common::Log::L_INFO, PLSESSID(getInfo , "offset[%d] < 0 , item[%s] ctrlnum[%lu]'s playtime[%d] is an invalid value with intimeoffset[%lu],just adjust it to 0"),
									cOffset,_itCurrent->objectName.string,_itCurrent->userCtrlNum,
									cOffset , _itCurrent->inTimeOffset);

				cOffset = 0;

			}
			else
			{
				if(!bLocalCall)
					glog(ZQ::common::Log::L_INFO, PLSESSID(getInfo, "adjust item[%s] ctrlNum[%lu]'s NPT to [%d] because intimeoffset[%lu]" ),
									_itCurrent->objectName.string , _itCurrent->userCtrlNum , 
									cOffset , _itCurrent->inTimeOffset);
			}

			char szBuf[24];
			sprintf(szBuf,"%f",scale);
			var.set(EventField_UserCtrlNum, (int)_itCurrent->userCtrlNum);
			var.set(EventField_CurrentTimeOffset, (int)cOffset);			
			var.set(EventField_PlayScale, std::string(szBuf));
			var.set(EventField_TotalTimeOffset, int(totalOffset));
			var.set(EventField_ItemIndex, iItemIndex );
			var.set(EventField_ItemOffset,(int)cOffset );

			glog(ZQ::common::Log::L_INFO , PLSESSID( getInfo , "get item information with curOffset[%d] , totalPlaytime[%d] ,scale[%s]" ),
				cOffset , totalOffset, szBuf);

		}
		break;
	default:
		return false;
		break;
	}
	//glog(Log::L_DEBUG,PLSESSID(getInfo,"done"));
	return true;
}
bool Playlist::getStrmSessAttribute(iterator it,int& cTimeOffset,float& scale,int& tTimeOffset , bool canIgnoreTotalOffset , bool bLocalCall )
{
	TIME_OFFSET	tOffset=0;
	LONGLONG	totalOffset=0;
	LONGLONG	curOffset=0;
	ULONG		bitRate=0;
	ESESSION_CHARACTERISTICS esessInfo;
	SPEED_IND	tempSpeed ;
	tempSpeed.denominator = 0 ;
	tempSpeed.numerator = 0;
	if( !iterValid( it ) )
	{
		glog(ZQ::common::Log::L_ERROR,PLSESSID(getStrmSessAttribute,"invalid item iterator passed in"));
		return false;
	}

	if(it->sessionId==0)
	{
		glog(Log::L_WARNING,PLSESSID(getStrmSessAttribute,"item[%s][%lu] has not been loaded"), it->objectName.string, it->userCtrlNum);
	}
	
	//#define _SCAN_RESULT_FOR_OFFSET
#ifdef _SCAN_RESULT_FOR_OFFSET
	else
	{
		glog(Log::L_DEBUG,PLSESSID(getSessAttr,"get vstrm session information from session scan first"));
		if(it->bitrate==0 || it->byteOffsetEOS==0 )
		{
			glog(Log::L_DEBUG,PLSESSID(getSessAttr,"invalid session scan result,turn to getVstrmSessInfo"));			
			if(!getVstrmSessInfo(_itCurrent->sessionId,esessInfo))
			{
				ERRLOG(Log::L_ERROR,PLSESSID(getSessAttr,"Can't get vstrm SessInfo from getVstrmSessInfo with sessionId=%u"),
					it->sessionId);
				tOffset=it->timeOffset;
				totalOffset=it->byteOffsetEOS;
				bitRate=it->bitrate;
				curOffset=it->byteOffset;
				tempSpeed.numerator = it->speed.numerator;
				tempSpeed.denominator = it->speed.denominator;

				glog(Log::L_DEBUG, PLSESSID(getSessAttr,"get Iteminfo with CtrlNum=%u fileName=%s vstrmSessId=%u and timeOffset=%u  byteOffset=%lld byteOffset=%lld byteOffsetEOS=%lld bitRate=%u speed[%d/%d]from session scan result"),
					it->userCtrlNum,it->objectName.string,it->sessionId,
					tOffset,it->byteOffset,curOffset,totalOffset,bitRate,
					tempSpeed.numerator,tempSpeed.denominator);
			}
			else
			{
				SESSION_CHARACTERISTICS* pInfo=&(esessInfo.SessionCharacteristics);				
				tOffset=pInfo->PlayoutTimeOffset;
				totalOffset=pInfo->ObjectSize.QuadPart;
				bitRate=pInfo->MuxRate;
				curOffset=pInfo->ByteOffset.QuadPart;
				tempSpeed.numerator = pInfo->Speed.numerator;
				tempSpeed.denominator = pInfo->Speed.denominator;
				glog(Log::L_DEBUG, PLSESSID(getSessAttr,"get Iteminfo with CtrlNum=%u fileName=%s vstrmSessId=%u"
					" and timeOffset[%u]  byteOffset[%lld]BYTE byteOffsetEOS[%lld]BYTE bitRate[%u]bps  speed[%d/%d] from getVstrmSessInfo"),
					it->userCtrlNum,it->objectName.string,it->sessionId,
					tOffset,curOffset,totalOffset,bitRate,
					tempSpeed.numerator , tempSpeed.denominator );
				
			}
		}
		else
		{					
			tOffset=it->timeOffset;
			totalOffset=it->byteOffsetEOS;
			bitRate=it->bitrate;
			curOffset=it->byteOffset;
			tempSpeed.numerator = it->speed.numerator;
			tempSpeed.denominator = it->speed.denominator;
			glog(Log::L_DEBUG, PLSESSID(getSessAttr,"get Iteminfo with CtrlNum=%u fileName=%s vstrmSessId=%u "
				"and timeOffset[%u]  byteOffset[%lld]BYTE byteOffsetEOS[%lld]BYTE bitRate[%u]bps speed[%d/%d] from session scan result"),
				it->userCtrlNum,it->objectName.string,it->sessionId,
				tOffset,curOffset,totalOffset,bitRate,
				tempSpeed.numerator , tempSpeed.denominator );
			
		}
	}
#else//_SCAN_RESULT_FOR_OFFSET
	
	//glog(Log::L_DEBUG,PLSESSID(getSessAttr,"querying for vstrm session information"));

	if(!getVstrmSessInfo( it, esessInfo ))
	{//call getVstrmSessInfo fail
		tOffset = it->timeOffset;
		totalOffset = it->byteOffsetEOS;
		bitRate = it->bitrate;
		curOffset = it->byteOffset;
		tempSpeed.numerator = (short)it->speed.numerator;
		tempSpeed.denominator = (unsigned short) it->speed.denominator;
		glog(Log::L_DEBUG, PLSESSID(getSessAttr,"failed to get VstrmSess[%lu] attributes from API query, take those of item[%s][%lu] from session scan: "
			"timeOffset[%lu], byteOffset[%lld],objectSize[%lld], bitrate[%lu]bps, speed[%d/%d]"),
			it->sessionId, it->objectName.string, it->userCtrlNum,
			tOffset,curOffset,totalOffset,bitRate,
			tempSpeed.numerator , tempSpeed.denominator );
	}		
	else
	{//call getVstrmSessInfo Success
		SESSION_CHARACTERISTICS* pInfo = &(esessInfo.SessionCharacteristics);
		tOffset = pInfo->PlayoutTimeOffset;
		totalOffset = pInfo->ObjectSize.QuadPart;
		bitRate = pInfo->MuxRate;
		curOffset = pInfo->ByteOffset.QuadPart;
		tempSpeed.numerator = pInfo->Speed.numerator;
		tempSpeed.denominator = pInfo->Speed.denominator;
		glog(Log::L_INFO,PLSESSID(getSessAttr,"got attributes of item[%s][%lu] from api query: VstrmSess[%lu], "
			"timeOffset[%lu], byteOffset[%lld], objectSize[%lld], bitRate[%lu]bps, speed [%d/%d]"),
			it->objectName.string, it->userCtrlNum, it->sessionId,
			tOffset, curOffset, totalOffset, bitRate,
			tempSpeed.numerator , tempSpeed.denominator );
	}
	
#endif//_SCAN_RESULT_FOR_OFFSET
	
	//var.set(EventField_UserCtrlNum,(int)_itCurrent->userCtrlNum);
	//var.set(EventField_CurrentTimeOffset,(int)tOffset);
	cTimeOffset = tOffset;
	/*double scale=0.0f;*/
	
	//should I update the playlist's speed when get session's information successfully ??
	//if( _crntSpeed.denominator == 0 )
	if( tempSpeed.denominator == 0 )
	{//do not update playlist speed using result from getInfo, but send this result back to client
		scale = 0.0f;
	}
	else
	{
		//scale = (float) ( (double) _crntSpeed.numerator / (double) _crntSpeed.denominator );
		scale = (float)( (double)tempSpeed.numerator / (double)tempSpeed.denominator );
	}
	
	long totalTime = 0;
// 	if( bitRate == 0 )
// 	{
// //		glog ( Log::L_ERROR , PLSESSID(getSessAttr, "invalid bitrate got " ) );
// 		//var.set(E,int(1));
// 		tTimeOffset = 0;
// 	}
// 	else
	{	
		if( it->_bEnableItemLibrary && canIgnoreTotalOffset )
		{
			tTimeOffset = 0;
			glog(ZQ::common::Log::L_DEBUG, PLSESSID(getSessAttr, "skip to query item duration due to item[%s][%lu] has library url"),
				it->objectName.string,
				it->userCtrlNum);
			return true;
		}		
		else if( !bLocalCall && !checkContentAttribute(it) )	
		{//get total playtime from content store instead of using vsm_GetFileSize
			if( _crntSpeed.denominator == 0 || bitRate == 0 )
			{
				tTimeOffset = 0;
			}
			else
			{
				tTimeOffset = (long)(( totalOffset*1000*8*ABSOLUTEX(_crntSpeed.numerator))/(bitRate*ABSOLUTEX(_crntSpeed.denominator)));
			}
			_lastErrCode =  ERR_PLAYLIST_SERVER_ERROR;
			ERRLOG(ZQ::common::Log::L_ERROR, PLSESSID(getSessAttr,"failed to query attributes of content[%s]"),
				it->_rawItemName);
			return false;
		}
		else
		{
			tTimeOffset = it->_itemPlaytime;
// 			glog(ZQ::common::Log::L_INFO, PLSESSID(getSessAttr,"got attributes of content[%s]: playtime[%ld],bitrate[%ld]"),
// 				it->_rawItemName, it->_itemPlaytime,it->_itemBitrate);
		}
	}
	return true;
}

bool	Playlist::allocateResource(int serviceGroupID, ZQ::common::Variant& varOut,int bandwidth)
{
	glog(Log::L_DEBUG,PLSESSID(allocateResource,"entering with serviceGroupId[%d], needBandwidth[%d](kbps)"),
										serviceGroupID,	bandwidth);
	//walk play list item and detect the max bitrate of the specify file
	//iterator	it;
	VvxParser	parser;
	//vsm_ParseVvxFile
	ULONG		maxRate=0;
	

	if( bandwidth < 0 )
	{
		maxRate=_dvbAttrs.MaxMuxRate/1000;
		glog(Log::L_WARNING, PLSESSID(allocateResource,"negative bandwidth is specified, adjust to MaxMuxRate[%lu]kbps"), maxRate);		
	}
	else
	{
		
		maxRate=bandwidth;//kbps
	}
	
	glog(Log::L_DEBUG,PLSESSID(allocateResource,"allocating resource with bandwidth[%lu]kbps"), maxRate);
	if(!_ResourceGuid.isNil())
		releaseResource(_ResourceGuid);
	//maxRate/=1024*1024;
	
	std::string		TargetMac;
	std::string		TargetIP;
	int				TargetPort=0;
	int				TargetFrequency=0;
	int				TargetProNum=0;
	int				TargetChannelID=0;
	int				QamMode=0;
	if(!_mgr.GetResource(serviceGroupID,maxRate,TargetIP,TargetPort,TargetMac,
		TargetProNum,TargetFrequency,TargetChannelID,_ResourceGuid,QamMode) || _ResourceGuid.isNil())
	{
		ERRLOG(Log::L_ERROR,PLSESSID(allocateResource,"failed to get resource for service group[%d], bandWidth[%lu]kbps"),
								serviceGroupID,maxRate);
		return false;
	}
	else
	{
		glog(Log::L_DEBUG,PLSESSID(allocateResource,"got resource successfully: grpId[%d] maxRate[%lu] return ip[%s] port[%d] TargetMac[%s] proNum[%d] frequency[%d] chanlId[%d]"),
									serviceGroupID,maxRate,TargetIP.c_str(),TargetPort,TargetMac.c_str(),
									TargetProNum,TargetFrequency,TargetChannelID);
	}
	char	szBuf[128];
	ZeroMemory(szBuf,128);
	_ResourceGuid.toString(szBuf,127);
	varOut.set(Playlist::RES_GUID,szBuf);
	varOut.set(Playlist::RES_FRENQENCY,(int)TargetFrequency);
	varOut.set(Playlist::RES_PROGRAMNUMBER,(int)TargetProNum);
	varOut.set(Playlist::RES_DESTIP,TargetIP);
	varOut.set(Playlist::RES_DESTPORT,TargetPort);
	varOut.set(Playlist::RES_DESTMAC,TargetMac);
	varOut.set(Playlist::RES_QAMMODE,QamMode);
	
	varOut.set(Playlist::RES_CHANNEL,TargetChannelID);

	_varQamResource = varOut;

	//OK I get a program number
	//set it to vstrm
	//setProgramNumber(TargetProNum);
	setDestination((char*)TargetIP.c_str(),TargetPort);
	if(!TargetMac.empty())
	{
		setDestMac((char*)TargetMac.c_str());
	}
	glog(Log::L_DEBUG,PLSESSID(allocateResource,"leave allocResource"));

	return true;
}
bool	Playlist::releaseResource(ZQ::common::Guid& uid)
{
	 _mgr.FreeResource(uid);
	 uid.create(true);
	 return true;
}

//////////////////////////////////////////////////////////////////////////
#ifdef _ICE_INTERFACE_SUPPORT
void Playlist::copyList(List& l)
{
	ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
	for (iterator it = listBegin () ; it != listEnd () ; it++) 
	{
		l.push_back (*it);
	}	
}
void	Playlist::VstrmSessNotifyWhenStartup(const VstrmSessInfo& sessinfo)
{
	if( sessinfo.sessionId == _vstrmSessioIDForFailOver )
	{
		_itCurrent=findUserCtrlNum(_currentItemDist);
		if( iterValid(_itCurrent) )
		{
			glog(Log::L_DEBUG,PLSESSID(restore,"found item[%s][%lu]"),
				_itCurrent->objectName.string, _itCurrent->userCtrlNum);
			_itNext=_itCurrent+1;
		}
		else
		{
			_itNext=listEnd();
		}
		m_bCurrentSessionIsONLine = true;
		RegisterSessID( sessinfo.sessionId );
		
		glog( ZQ::common::Log::L_INFO, PLSESSID(restore,"re-establish call back logic for session [%u]"),sessinfo.sessionId );
		VstrmClassEnableSessionCallbackEx( _mgr.classHandle(), sessinfo.sessionId, cbItemCompleted,this);

		FireStateChanged( PLAYLIST_PLAY );
		updateItem( sessinfo , true );

		{
			ZQ::common::MutexGuard gd(_listOpLocker);
			ESESSION_CHARACTERISTICS esessInfo;
			if( iterValid(_itCurrent) && getVstrmSessInfo( _itCurrent , esessInfo) )
			{
				_crntSpeed = esessInfo.SessionCharacteristics.Speed;
			}
			else
			{
				_crntSpeed = Playlist::SPEED_NORMAL;
			}
			
			if (isReverseStreaming () ) 
			{
				if ( _itCurrent > listStart () )
				{
					_itNext = _itCurrent - 1;
				}
				else
				{
					_itNext = listStart ();
				}
			}
			else
			{
				if ( _itCurrent< listEnd ()) 
				{
					_itNext = _itCurrent +1;
				}
				else
				{
					_itNext = listEnd ();
				}
			}
		}		
	}
}
void	Playlist::vstrmSessNotifyOverWhenStartup()
{
	if( !m_bCurrentSessionIsONLine )
	{
		glog(Log::L_DEBUG, PLSESSID(restore,"loading next item as the latest VstrmSess[%lu] has been gone"),	_vstrmSessioIDForFailOver);
		
		
		_itCurrent = findUserCtrlNum(_currentItemDist);

		if( iterValid(_itCurrent) && _itCurrent != listEnd() )
		{
// 			glog(Log::L_DEBUG, PLSESSID(restore, "find the item with  userCtrlNum[%u] and itemName[%s]"),
// 				_itCurrent->userCtrlNum,_itCurrent->objectName.string);
			_itNext = _itCurrent + 1;
		}
		else
		{
			_itNext = listEnd();
		}

		if( iterValid(_itNext) )
		{
			//glog(ZQ::common::Log::L_INFO,PLSESSID(restore,"last vstrm session is gone ,skip to next item"));
			skipToItem( _itNext, true );
		}
		else
		{
			glog(Log::L_DEBUG,PLSESSID(restore,"reached the playlist end"));			
			FireStateChanged(PLAYLIST_STOP);
			//_currentStatus=PLAYLIST_STOP;
			updateTimer();
		}

	}
	else
	{
		//primeNext();
	}
	
	{//Get current vstrm speed and update 
		ZQ::common::MutexGuard gd(_listOpLocker);
		ESESSION_CHARACTERISTICS esessInfo;
		if( (_itCurrent != listStart() && _itCurrent != listEnd() ) && getVstrmSessInfo( _itCurrent , esessInfo) )
		{
			_crntSpeed = esessInfo.SessionCharacteristics.Speed;
		}
		else
		{
			_crntSpeed = Playlist::SPEED_NORMAL;
		}
		if (isReverseStreaming () ) 
		{
			if ( _itCurrent > listStart () )
			{
				_itNext = _itCurrent - 1;
			}
			else
			{
				_itNext = listStart ();
			}
		}
		else
		{
			if ( _itCurrent< listEnd ()) 
			{
				_itNext = _itCurrent +1;
			}
			else
			{
				_itNext = listEnd ();
			}
		}
		//check all running vstrsm session on this port
		iterator it = listBegin( );
		for( ; it != listEnd( ) ; it ++ )
		{
			ESESSION_CHARACTERISTICS esessInfo;
			if( it->sessionId != 0 && getVstrmSessInfo( it , esessInfo) )
			{
				if( it != _itCurrent && it != _itNext )
				{
					unloadEx( it , false );
				}
			}
		}
	}
	updateTimer ();
	primeNext();
	glog(Log::L_DEBUG,PLSESSID(restore,"playlist is restored successfully, current state[%d]"), _currentStatus );
}
#endif//_ICE_INTERFACE_SUPPORT

void Playlist::DisplaySessionAttr(iterator it)
{
	if(it<=listStart() || it>=listEnd())
	{
		//glog(Log::L_ERROR,PLSESSID(DisplaySessionAttr,"invalid session"));
		return;
	}
	if(it->sessionId==0)
	{
		glog(Log::L_ERROR,PLSESSID(displaySessionAttr,"current cursor item[%s][%lu] has not been loaded"), it->objectName.string, it->userCtrlNum );
		return;			 
	}
	ESESSION_CHARACTERISTICS	buffer;

	//VSTATUS status=VstrmClassGetSessionChars(_mgr.classHandle(),it->sessionId,&buffer,sizeof(buffer),&returnSize);
	if ( getVstrmSessInfo( it , buffer ) )
	{		
		glog(Log::L_DEBUG,PLSESSID( displaySessAttr, "got VstrmSess[%lu]'s attribute: ObjectSize[%lld] byteOffset[%lld] timePlayStarted[%lld]  PlayoutByteOffset[%lld] PlayoutTimeOffset[%lu] MuxRate=%lu "),
			it->sessionId ,
			buffer.SessionCharacteristics.ObjectSize,
			buffer.SessionCharacteristics.ByteOffset,
			buffer.SessionCharacteristics.TimePlayStarted,
			buffer.SessionCharacteristics.PlayoutByteOffset,
			buffer.SessionCharacteristics.PlayoutTimeOffset,
			buffer.SessionCharacteristics.MuxRate);		
	}
	else
	{
		glog(Log::L_ERROR,PLSESSID(displaySessAttr,"failed to get attributes of VstrmSess[%lu]"),it->sessionId);
	}
}

bool Playlist::getVstrmSessInfo( iterator it , ESESSION_CHARACTERISTICS& valueOut)
{
	if( !iterValid(it) || it->sessionId == 0 )	
	{
		memset( &valueOut , 0 , sizeof(valueOut) );
		glog(ZQ::common::Log::L_DEBUG, PLSESSID(getVstrmSessInfo,"skip due to session[0]"));
		return true;
	}
	ULONG	returnSize ;
	
	VSTRMAPICALLSTART(VstrmClassGetSessionChars);
	VSTATUS status=VstrmClassGetSessionChars(_mgr.classHandle(),it->sessionId, &valueOut, sizeof(ESESSION_CHARACTERISTICS),&returnSize);
	VSTRMAPICALLEND(VstrmClassGetSessionChars);

	if( IS_VSTRM_SUCCESS( status ) )
	{
		if( it->byteOffsetEOS < (UQUADWORD)valueOut.SessionCharacteristics.ObjectSize.QuadPart ||
			it->byteOffset < (UQUADWORD)valueOut.SessionCharacteristics.ByteOffset.QuadPart)
		{
			it->byteOffsetEOS	= (UQUADWORD)valueOut.SessionCharacteristics.ObjectSize.QuadPart;
			it->byteOffset		= (UQUADWORD)valueOut.SessionCharacteristics.ByteOffset.QuadPart;

			it->timeOffset		= (TIME_OFFSET)valueOut.SessionCharacteristics.PlayoutTimeOffset;
			it->pendingDataSize	= valueOut.SessionCharacteristics.PendingDataSize;
		}
		return true;
	}
	else
	{
		char szBuf[1024];
		ZeroMemory(szBuf,sizeof(szBuf));
		glog(Log::L_ERROR, PLSESSID(getVstrmSessInfo,"VstrmClassGetSessionChars() failed,VstrmSess[%lu], error [%s]"),
				it->sessionId, _mgr.getErrorText(VstrmGetLastError() , szBuf , sizeof(szBuf)-1 ) );				
		return false;
	}
}
const bool Playlist::distance(OUT int* dist, IN CtrlNum to, IN CtrlNum from /*= INVALID_CTRLNUM*/)
{
	return false;	
}
const CtrlNum Playlist::findItem(const CtrlNum userCtrlNum,const CtrlNum from /* = INVALID_CTRLNUM */)
{
	return INVALID_CTRLNUM;
}
const bool Playlist::isRunning()
{
	 return (!isCompleted() && (_itCurrent->sessionId !=0));
}
const bool Playlist::isCompleted()
{
// 	if( isReverseStreaming( ) )
// 		return (bool)(_itCurrent <=listStart());
// 	else
// 		return (bool)(_itCurrent >=listEnd()); 
	return ( (_itCurrent <= listStart( )) || (_itCurrent >= listEnd( )) );
}
const CtrlNum Playlist::current()const
{
	if(_itCurrent==_list.end())
		return INVALID_CTRLNUM;
	else
		return _itCurrent->userCtrlNum;
}
void Playlist::SetTimerEx(timeout64_t t,bool bGoodData)
{	
	//renew ticket
	try
	{		
		int		iTime=0;
		//现在采用在这里启动一个线程的方式来renew ticket
		if(t==FAKE_TIMER_TIMECOUNT)
		{
			iTime=-1;
			glog(Log::L_DEBUG,PLSESSID(SetTimerEx,"delete the associated pathTicket [%s]"),m_plePxStr.c_str() );
		}
		else
		{
			//glog(Log::L_DEBUG,PLSESSID(SetTimerEx,"update timer [%lld]"),t);
			setTimer64(t);
			if (0 ==t || t <= _mgr._timeout ||  (t >0 && _mgr._timeout ==_UI64_MAX))
				_mgr.wakeup();
			_lastUpdateTimer64=t+_GetTickCount64();
			iTime=(int)t+60*1000;
			//glog(Log::L_DEBUG,PLSESSID(SetTimerEx,"renew the ticket with time =%d"),iTime);
		}
#ifdef _ICE_INTERFACE_SUPPORT

		if(!m_plePxStr.empty())
		{
			RenewTicketRequest* pRequest=new RenewTicketRequest( _gThreadPool, m_plePxStr, m_ticketPrx, _mgr.m_Adapter , iTime );		
			
			pRequest->start();
		}
#endif//_ICE_INTERFACE_SUPPORT
	}
	catch(...)
	{
		
	}
}
std::vector<ULONG>	Playlist::getItemSet()
{
	std::vector<ULONG>	vecRet;
	List::const_iterator it;
	ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
	for(it=listBegin();it!=listEnd();it++)
	{
		vecRet.push_back(it->userCtrlNum);
	}
	return vecRet;
}
void Playlist::DumpItemInfo(iterator it)
{
	if(it>=listEnd() || it <=listStart())
		//out of range
		return;	
	glog(Log::L_DEBUG,PLSESSID(dumpItemInfo,"item[%s][%lu] spliceIn[%lu] spliceOut[%lu] foreNormalSpeed[%lu] inTimeOffset[%lu] outTimeOffset[%lu] criticalStart[%s] destIp[%s] destPort[%d] VstrmPort[%lu]"),
												it->objectName.string,
												it->userCtrlNum,
												it->spliceIn,
												it->spliceOut,
												it->forceNormalSpeed,
												it->inTimeOffset,
												it->outTimeOffset,
												it->criticalStart != 0 ? printfUTCTime(it->criticalStart).c_str() : "" ,
												_destIP.c_str() ,
												_dvbAttrs.udpPort,
												_vstrmPortNum);	
}
void Playlist::OnItemAbnormal(SESSION_ID sessId)
{
	if(!m_pStreamSite)
	{
		m_pStreamSite=(StreamSmithSite*)StreamSmithSite::getDefaultSite();
	}		
	
	bool bFound=false;
	CtrlNum num = -2;
	{
		ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
		iterator it=listBegin();
		for(;it!=listEnd();it++)
		{
			if(it->sessionId==sessId)
			{
				num=it->userCtrlNum;
				bFound=true;
			}
		}
	}
	if(m_pStreamSite&&bFound)
	{
		try
		{
#ifdef NEED_EVENTSINK_RAWDATA
			
#else
			ZQ::common::Variant	var;				
			var.set(EventField_PlaylistGuid,_strGuid);
			var.set(EventField_ClientSessId,m_strClientSessionID);
			var.set(EventField_UserCtrlNum,(long)num);
#endif
			glog(Log::L_DEBUG,PLSESSID(OnItemAbnormal,"event(E_PLAYLIST_SESSEXPIRED) is fired to EventChannel, ctrlNum[%u]"),num);
			DumpListInfo();
			m_pStreamSite->PostEventSink(E_PLAYLIST_SESSEXPIRED,var,_strGuid);
		}
		catch (...)
		{
			
		}
	}
}

void Playlist::FireStateChanged(IPlaylist::State plState , bool bFire)
{
	if(!m_pStreamSite)
	{
		m_pStreamSite=(StreamSmithSite*)StreamSmithSite::getDefaultSite();
	}
	if(m_pStreamSite)
	{
		try
		{
			
			if ( bFire && (plState!=_currentStatus) ) 
			{
				ZQ::common::Variant	var;
				DWORD now = GetTickCount();
				std::string preRequested = (now < (_stampStateChangeReq + gStreamSmithConfig.lperRequestInterval) ) ? "1" : "0";
				glog(ZQ::common::Log::L_DEBUG, PLSESSID(FireStateChanged,"event(E_PLAYLIST_STATECHANGED) is fired to EventChannel, now[%u], "
					"_stampStateChangeReq[%u], interval[%u],cfg[%d], perRequested[%s]"),
					now, _stampStateChangeReq, now - _stampStateChangeReq, gStreamSmithConfig.lperRequestInterval, preRequested.c_str());
				var.set(EventField_perRequested, preRequested);
				var.set(EventField_PlaylistGuid,_strGuid);
				var.set(EventField_ClientSessId,m_strClientSessionID);
				//var.set(EventField_UserCtrlNum,(long)num);
				var.set(EventField_PrevState,(long)_currentStatus);
				var.set(EventField_CurrentState,(long)plState);
				if (!isCompleted() && iterValid(_itCurrent) ) 
				{				
					var.set(EventField_UserCtrlNum,(int)_itCurrent->userCtrlNum);
					var.set(EventField_ItemFileName,std::string(_itCurrent->_rawItemName));
				}
				else
				{
					var.set(EventField_UserCtrlNum,(int)INVALID_CTRLNUM);
					var.set(EventField_ItemFileName,std::string(""));
				}
					
					ZQ::common::Variant varSpeed;
					varSpeed.clear();
					varSpeed.set(EventField_SpeedDenom,ZQ::common::Variant((int)_crntSpeed.denominator));
					varSpeed.set(EventField_SpeedNumer,ZQ::common::Variant((int)_crntSpeed.numerator));
				var.set( EventField_CurrentSpeed , varSpeed );

				int streamPos = getCurrentPlaylistNpt();
				var.set(EventField_CurrentPlayPos,  streamPos );
				var.set(EventField_CurrentItemTimeOffset, getCurrentItemNpt() );
				var.set( EventField_PreviousePlayPos , _lastplaylistNpt );
				var.set( EventField_PreviousePlayPosPrimary, _lastplaylistNptPrimary );// npt
				var.set( EventField_TotalDuration, getTotalStreamDuration());
				var.set ( EventField_TotalVideoDuration, getTotalVideoStreamDuration());
				char szBuf1[256];
				char szBuf2[256];
				long	lSeq = InterlockedIncrement( &_eventCSeqStateChanged );
				var.set(EventField_EventCSEQ , lSeq);
				glog(ZQ::common::Log::L_DEBUG, PLSESSID(FireStateChanged,"event(E_PLAYLIST_STATECHANGED) is fired to EventChannel, last state[%s], current state[%s], seq[%ld]"),
					getStateString(_currentStatus,szBuf2,sizeof(szBuf2)),
					getStateString(plState,szBuf1,sizeof(szBuf1)) ,
					lSeq );
				m_pStreamSite->PostEventSink(E_PLAYLIST_STATECHANGED,var,_strGuid);
			}
			_currentStatus		= plState;			
		}
		catch (...) 
		{			
		}
	}

}

void Playlist::ClearItemsState()
{
	ZQ::common::MutexGuard gd(_listOpLocker);
	iterator it=listBegin();
	for(;it!=listEnd();it++)
	{
		it->bitrate=0;
		it->timeOffset=0;
		it->byteOffset=0;
		it->byteOffset=0;
		it->stampLoad=0;
		it->stampLaunched=0;
		it->stampLastUpdate=0;
		it->stampUnload=0;
		it->_sessionDone=false;
	}
	_bCleared = false;
}
const int Playlist::size()
{
	if(_list.size()<=0)
		return 0;
	else
		return(int)( _list.size()-1); 
}
Playlist::iterator Playlist::listBegin() 
{
	/*ZQ::common::MutexGuard gd(_listOpLocker);*/
	iterator it=_list.begin() ; 
	if (_list.size()<=0)
		return it;
	else 
		return (++it);
}
Playlist::iterator Playlist::listEnd() 
{
	return _list.end(); 
}
Playlist::iterator Playlist::listStart() 
{
	return _list.begin(); 
}
Playlist::iterator Playlist::listCurrent() const	
{
	return _itCurrent; 
}
const bool	Playlist::empty()	const
{
	return (_list.size()-1) <= 0; 
}

bool Playlist::skipToItem(const CtrlNum where, bool bPlay )
{
	const_iterator	it=findUserCtrlNum(where);
	return skipToItem(it,bPlay);
}
void Playlist::setPlaylistExProxy( const std::string&  playlistPrxStr , const std::string& ticketPrxStr)
{
	m_plePxStr		=	playlistPrxStr;
	m_ticketPrx		=	ticketPrxStr;
	glog(ZQ::common::Log::L_DEBUG, PLSESSID(setPlaylistExProxy,"got PathTicket[%s]"), ticketPrxStr.c_str());
}

void Playlist::setStreamServPort(unsigned short uPort)
{	
	_dvbAttrs.udpSrcPort = uPort;
	glog( ZQ::common::Log::L_DEBUG, PLSESSID( setStreamServPort , "taking stream source udp port [%u]"), uPort);
}
void Playlist::enableEoT(bool bEnable)
{	
	_bEnableEOT = bEnable;
	glog(ZQ::common::Log::L_DEBUG,PLSESSID(enableEoT,"EOT protection [%s]"), bEnable ? "enable":"disable");
}

ULONG	Playlist::CalculateCurrentTotalTime( )
{
	ZQ::common::MutexGuard gd(_listOpLocker);
	if(isCompleted() || !iterValid(_itCurrent) || _bCleared )
	{
		glog(ZQ::common::Log::L_DEBUG,PLSESSID(CalculateCurrentTotalTime,"skip calculating"));
		return 0;
	}

	iterator it = _itCurrent;
	if ( ( it->_itemPlaytime <= 0 ) || ( GAPPLICATIONCONFIGURATION.lQueryLastItemPlayTime && ( isLastItem(it) ) ) )
	{
		if( !checkContentAttribute(it) )
		{
			glog(ZQ::common::Log::L_ERROR, PLSESSID(CalculateCurrentTotalTime, "failed to get attributes of item[%s][%lu]"),
				it->_rawItemName,it->userCtrlNum);

			it->_itemPlaytime = -1;

		}
		else
		{
			glog(ZQ::common::Log::L_DEBUG, PLSESSID(CalculateCurrentTotalTime,"got playtime [%ld] of item[%s][%lu]"),
				it->_itemPlaytime,it->_rawItemName,it->userCtrlNum);
		}		
	}
	glog(ZQ::common::Log::L_DEBUG,PLSESSID( CalculateCurrentTotalTime , "Calculated current total time [%lu]"), it->_itemPlaytime);
	return it->_itemPlaytime;
}
ULONG	Playlist::CalculateFutureTime( )
{
	ZQ::common::MutexGuard gd(_listOpLocker);
	if( isCompleted() || !iterValid(_itCurrent) || _bCleared )
	{
		glog(ZQ::common::Log::L_DEBUG,PLSESSID(CalculateCurrentTotalTime,"skip calculating"));
		return 0;
	}

	iterator it  = _itCurrent;
	it ++;
	ULONG ret = 0;
	for( ; it < listEnd() ; it ++ )
	{
		if ( ( it->_itemPlaytime <= 0 )||  ( GAPPLICATIONCONFIGURATION.lQueryLastItemPlayTime && ( isLastItem(it)) ) )
		{
			if(!checkContentAttribute(it))
			{
				glog(ZQ::common::Log::L_ERROR, PLSESSID(calculateRemainingTime,"failed to get atrtibute of item[%s][%lu]"),
					it->_rawItemName,it->userCtrlNum);

				it->_itemPlaytime = -1;

			}
			else
			{
				glog(ZQ::common::Log::L_DEBUG, PLSESSID(calculateRemainingTime,"got playtime[%ld] of item[%s][%lu]"),
					it->_itemPlaytime,it->_rawItemName,it->userCtrlNum);
			}
		}
		ret += it->_itemPlaytime;
	}
	glog(ZQ::common::Log::L_DEBUG,PLSESSID( CalculateFutureTime , "Calculated future time [%u]"), ret);

	return ret;
}
ULONG Playlist::CalculatePastTime()
{
	ZQ::common::MutexGuard gd(_listOpLocker);
	ULONG ulPastTime = 0;
	if( _itCurrent <= listStart() || _bCleared )
	{
		glog(ZQ::common::Log::L_DEBUG,PLSESSID(CalculateCurrentTotalTime,"skip calculating"));
		return 0;
	}

	iterator it  = listBegin();
	for ( ; it < _itCurrent ; it ++ ) 
	{
		if (it->_itemPlaytime <= 0 || ( GAPPLICATIONCONFIGURATION.lQueryLastItemPlayTime && ( isLastItem(it) ) ) )
		{
			if( !checkContentAttribute(it) )
			{
				glog(ZQ::common::Log::L_ERROR, PLSESSID(sumPrevItemsPlaytime, "failed to get attributes of item[%s][%lu]"),
						it->_rawItemName,it->userCtrlNum);

				it->_itemPlaytime = -1;
				
			}
			else
			{
				glog(ZQ::common::Log::L_DEBUG, PLSESSID(sumPrevItemsPlaytime, "got playtime[%ld] of item[%s][%lu]"),
					it->_itemPlaytime,it->_rawItemName,it->userCtrlNum);
			}
		}
		ulPastTime += it->_itemPlaytime;
// 		glog(ZQ::common::Log::L_DEBUG, PLSESSID(CalculatePastTime,"calculate item playtime [%u] with item[%s] ctrlNum[%u]"),
// 						it->_itemPlaytime,it->_rawItemName,it->userCtrlNum);
	}
	glog(ZQ::common::Log::L_DEBUG, PLSESSID(sumPrevItemsPlaytime,"got playlist subtotal of previous items:[%lu]"), ulPastTime);
	return ulPastTime;
}


ULONG Playlist::CalculatePastTimeNPTPriamry()
{
	ZQ::common::MutexGuard gd(_listOpLocker);
	ULONG ulPastTimeNptPrimary = 0;
	if( _itCurrent <= listStart() || _bCleared )
	{
		glog(ZQ::common::Log::L_DEBUG,PLSESSID(CalculatePastTimeNPTPriamry,"skip calculating"));
		return 0;
	}

	iterator it  = listBegin();
	for ( ; it < _itCurrent ; it ++ ) 
	{
		if (it->_itemPlaytime <= 0 || ( GAPPLICATIONCONFIGURATION.lQueryLastItemPlayTime && ( isLastItem(it) ) ) )
		{
			if( !checkContentAttribute(it) )
			{
				glog(ZQ::common::Log::L_ERROR, PLSESSID(sumPrevItemsPlaytime, "failed to get attributes of item[%s][%lu]"),
					it->_rawItemName,it->userCtrlNum);

				it->_itemPlaytime = -1;

			}
			else
			{
				glog(ZQ::common::Log::L_DEBUG, PLSESSID(sumPrevItemsPlaytime, "got playtime[%ld] of item[%s][%lu]"),
					it->_itemPlaytime,it->_rawItemName,it->userCtrlNum);
			}
		}

		if (it->_itemPlayTimesCount == -1)  //don't calculate ads' npt 
		{
			ulPastTimeNptPrimary += it->_itemPlaytime;
		}
	}
	glog(ZQ::common::Log::L_DEBUG, PLSESSID(sumPrevItemsPlaytime,"got playlist NptPrimary of previous items:[%lu]"), ulPastTimeNptPrimary);
	return ulPastTimeNptPrimary;
}


//这里需要用锁吗????
bool	Playlist::exPause( OUT StreamControlResultInfo& info )
{	
	ULONG flag = info.flag;
	ZQ::common::MutexGuard gd(_listOpLocker);
	
	if( _bCleared )
	{
		_lastErrCode = ERR_PLAYLIST_INVALID_PARA;
		ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(pauseEx,"playlist is destroyed, reject pause command"));
		return false;
	}

	switch( _currentStatus )
	{
	case IPlaylist::PLAYLIST_SETUP:
	case IPlaylist::PLAYLIST_STOP:
		{
			_lastErrCode	=  IPlaylist::ERR_PLAYLIST_INVALIDSTATE;
			ERRLOG(ZQ::common::Log::L_ERROR , PLSESSID(pauseEx,"playlist is not at streaming state, reject"));
			return false;
		}
		break;
	case IPlaylist::PLAYLIST_PAUSE:
		{
			if( !doGetStreamResultInfo(_itCurrent , info ) )
				return false;
		}
		break;
	case IPlaylist::PLAYLIST_PLAY:
		{
			if(!doPause(true,info) )
				return false;
		}
		break;
	default:
		{
			_lastErrCode = IPlaylist::ERR_PLAYLIST_SERVER_ERROR;
			ERRLOG(ZQ::common::Log::L_ERROR , PLSESSID(pauseEx,"ivnalid playlist state[%d]") , _currentStatus );
			return false;
		}
		break;
	}
	updateTimer();
	if( flag & GET_NPT_CURRENTPOS )
	{
		info.timeOffset = info.timeOffset + CalculatePastTime( );
	}
	if( flag & GET_NPT_TOTALPOS )
	{
		info.totalOffset = CalculatePastTime() + CalculateCurrentTotalTime() + CalculateFutureTime();
	}
	if( flag & GET_ITEM_TOTALPOS )
	{
		info.itemTotalOffset = CalculateCurrentTotalTime();
	}
	return true;
}

bool Playlist::exPlay( IN float newSpeed ,IN ULONG offset , IN short from , OUT StreamControlResultInfo& info )
{
	{
		ZQ::common::MutexGuard gd(_listOpLocker);
		if( _bCleared )
		{
			_lastErrCode = ERR_PLAYLIST_INVALID_PARA;
			ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(playEx,"playlist is destroyed, reject play command"));
			return false;
		}
	}
	if( !_b1stPlayStarted && !_pokeHoleSessID.empty() && _hRemoteSessionNotifier )
	{
		if(GAPPLICATIONCONFIGURATION.natConfiguration.enable >= 1)
		{
			glog(ZQ::common::Log::L_INFO, PLSESSID(playEx,"first play with pokeHoleSession[%s], waiting for pokeHoleSess signal for [%d]ms"),
				_pokeHoleSessID.c_str(),
				GAPPLICATIONCONFIGURATION.natConfiguration.maxWaiting);

			if( WaitForSingleObject(_hRemoteSessionNotifier,GAPPLICATIONCONFIGURATION.natConfiguration.maxWaiting)	== WAIT_TIMEOUT )
			{
				_lastErrCode = ERR_PLAYLIST_INVALID_PARA;
				ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(playEx,"pokeHoleSession[%s] timeout, has not been activated by the client"), _pokeHoleSessID.c_str() );
				return false;
			}
			else
			{
				glog(ZQ::common::Log::L_INFO,PLSESSID(playEx,"pokeHoleSession[%s] is activated"), _pokeHoleSessID.c_str());
			}
		}
		else
		{
			// 					glog(ZQ::common::Log::L_INFO, PLSESSID(playEx,"disable waiting for remote session"));
		}
	}

	
	glog(ZQ::common::Log::L_INFO , PLSESSID(playEx,"entering with newSpeed[%f] offset[%lu] from[%d]") , newSpeed , offset , from );
	{
		ZQ::common::MutexGuard gd(_listOpLocker);

		if( size() <=0 )
		{
			_lastErrCode=ERR_PLAYLIST_INVALIDSTATE;
			ERRLOG(Log::L_ERROR,PLSESSID(playEx,"playlist is empty, rejecting operation"));
			return false;
		}
		_lastplaylistNpt	= getCurrentPlaylistNpt();
		_lastItemNpt		= getCurrentItemNpt();
	}


	ULONG flag = info.flag;
	switch ( _currentStatus )
	{
	case IPlaylist::PLAYLIST_SETUP:		
	case IPlaylist::PLAYLIST_STOP:
		{
			if(0 == offset && (IPlaylist::SEEK_POS_CUR == from || (IPlaylist::SEEK_POS_BEGIN == from && !_b1stPlayStarted) ))
			{
				if(!doPlay( 0 , newSpeed , info ) )
					return false;
			}
			else
			{
				if(!doSeekStream(offset , from , newSpeed , info ))
				{
					return false;
				}
			}
		}
		break;

	case IPlaylist::PLAYLIST_PLAY:
		{
			if( 0 == offset && IPlaylist::SEEK_POS_CUR == from)
			{
				if(!doChangeSpeed( newSpeed , false , info ))
					return false;
			}
			else
			{
				if(!doSeekStream(offset , from , newSpeed , info ))
					return false;
			}
		}
		break;
	case IPlaylist::PLAYLIST_PAUSE:
		{
			
			if( !(offset == 0 && from == IPlaylist::SEEK_POS_CUR) ) 
			{
				if(!doSeekStream(offset , from , newSpeed , info ))
					return false;
			}
			else if( fabs(newSpeed) > 0.01 )
			{
				if(!doChangeSpeed( newSpeed , false , info ))
					return false;
			}
			else if(!doResume(info))
			{
				return false;
			}
		}
		break;
	
	default:
		{
			ERRLOG( ZQ::common::Log::L_ERROR , PLSESSID( playEx , "invalid playlist state[%d]") , _currentStatus );
			return false;
		}
		break;
	}


	{
		ZQ::common::MutexGuard gd(_listOpLocker);
		if( _bCleared )
		{
			_lastErrCode = ERR_PLAYLIST_INVALID_PARA;
			ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(playEx,"playlist is destroyed, reject play command"));
			return false;
		}
	}
	primeNext( );

	{
		ZQ::common::MutexGuard gd(_listOpLocker);

		if( flag & GET_NPT_CURRENTPOS )
		{
			info.timeOffset = info.timeOffset + CalculatePastTime( );
		}
		if( flag & GET_NPT_TOTALPOS )
		{
			info.totalOffset = CalculatePastTime() + CalculateCurrentTotalTime() + CalculateFutureTime();
		}
		if( flag & GET_ITEM_TOTALPOS )
		{
			info.itemTotalOffset = CalculateCurrentTotalTime();
		}
	}
	return true;
}

void Playlist::setPlaylistAttributes( const TianShanIce::ValueMap& values ) 
{
	ZQ::common::MutexGuard gd(_listOpLocker);

	Ice::Int	PauseLastUntilNext  = 0;
	ZQTianShan::Util::getValueMapDataWithDefault( values , "PauseLastUntilNext" , 0 , PauseLastUntilNext);


	_dvbAttrs.PauseLastUntilNext	=	(PauseLastUntilNext > 0) ? 1 : 0 ;
	glog(ZQ::common::Log::L_INFO, PLSESSID(setPlaylistAttributes,"taking PauseLastUntilNext[%s] ") ,_dvbAttrs.PauseLastUntilNext == 1 ? "true" : "false" );

}

bool Playlist::setPokeHoleSessionID( const std::string& strPokeHoleSessID  ) 
{
	glog(ZQ::common::Log::L_INFO,PLSESSID(setPokeHoleSessionID,"taking PokeHoleSessId[%s]"),strPokeHoleSessID.c_str ());	
	if( strPokeHoleSessID.length () != 20 )
	{
		glog(ZQ::common::Log::L_ERROR,PLSESSID(setPokeHoleSessionID,"illegal PokeHoleSessId[%s]: length[%d] != 20"),
			strPokeHoleSessID.c_str(),
			strPokeHoleSessID.length ());
		return false;			 
	}
	
	char szPokeHoleSession[20];
	memset(  szPokeHoleSession, 0 , sizeof(szPokeHoleSession) );
	ConvertStringIntoBinary( strPokeHoleSessID,szPokeHoleSession );

	//_pokeHoleSessID = strPokeHoleSessID;
	_pokeHoleSessID.assign( szPokeHoleSession , 10 );

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

		VSTATUS	status =  VstrmClassSetPortAttributesEx(_mgr.classHandle () , _vstrmPortNum,&attrArry);
		if( status != VSTRM_SUCCESS )
		{
			_ntstatus 	= GetLastError ();		// capture secondary error code (if any)
			char	szErrbuf[1024];
			ZeroMemory(szErrbuf,1024);	
			_lastErrCode=ERR_PLAYLIST_SERVER_ERROR;
			ERRLOG (ZQ::common::Log::L_ERROR, PLSESSID(setPokeHoleSessionID,"VstrmClassSetPortAttributesEx(VSTRM_ATTR_GEN_SESSION_ID) failed, PokeHoleSessionId[%s], error[%s]"),
				HelperClass::dumpBinary(_pokeHoleSessID).c_str(),
				_mgr.getErrorText(VstrmGetLastError(),szErrbuf,1023));
			return false;
		}
		else
		{
			glog(ZQ::common::Log::L_INFO, PLSESSID(setPokeHoleSessionID,"successfully set PokeHoleSessionId[%s] to VstrmPort[%lu]"),
				HelperClass::dumpBinary(_pokeHoleSessID).c_str() ,
				_vstrmPortNum);
		}

	}
	_hRemoteSessionNotifier = _mgr._cls.registerPokeSession(_pokeHoleSessID);
	
	glog(ZQ::common::Log::L_INFO, PLSESSID(setPokeHoleSessionID,"registered pokeHoleSessionId[%s] to Vstrm"),
		HelperClass::dumpBinary(_pokeHoleSessID).c_str() );

	return true;
}
void	Playlist::setStreamPID(int pid)
{
	glog(ZQ::common::Log::L_INFO,PLSESSID(setStreamPID,"taking stream pid[%d]"), pid);
	_perStreamPID = pid;
}

void Playlist::updateOpTime( )
{
	_lastSessOpTime = ZQTianShan::now();
	glog(ZQ::common::Log::L_DEBUG, PLSESSID(updateOpTime, " updated last operation time to [%lld]"), _lastSessOpTime );	
}

bool	Playlist::iterValid( const iterator& it  ) const
{
	iterator itStart = (const_cast<Playlist*>(this))->listStart();
	iterator itEnd = ( const_cast<Playlist*>(this))->listEnd();
	return ((it > itStart) && ( it < itEnd ));
}

bool Playlist::checkResitriction( const_iterator itTarget , const long mask )
{
	if( !iterValid(itTarget) )	return true;//invalid item , so return true which means no restriction on it
	bool bNoRestriction = true;

	ULONG flag = itTarget->itemFlag;	
	if( mask & PLISFlagNoPause )
	{
		if( flag & PLISFlagNoPause )
			bNoRestriction = false;
	}
	if( mask & PLISFlagNoFF )
	{
		if( flag & PLISFlagNoFF )
			bNoRestriction = false;
	}
	if( mask & PLISFlagNoRew )
	{
		if( flag & PLISFlagNoRew )
			bNoRestriction = false;
	}
	if( mask & PLISFlagNoSeek )
	{
		if( flag & PLISFlagNoSeek )
			bNoRestriction = false;
	}	
	if ( mask & PLISFlagSkipAtFF )
	{
		if ( flag & PLISFlagSkipAtFF )
			bNoRestriction = false;
	}
	if ( mask & PLISFlagSkipAtRew )
	{
		if ( flag & PLISFlagSkipAtRew )
			bNoRestriction = false;
	}

	return bNoRestriction;
}

void Playlist::onPauseTimeout( )
{
	ZQ::common::MutexGuard gd(_listOpLocker);
	
	if(!m_pStreamSite)
	{
		m_pStreamSite=(StreamSmithSite*)StreamSmithSite::getDefaultSite();
	}
	if(m_pStreamSite)
	{
		int streamNpt	= getCurrentPlaylistNpt();
		int ctrlNum		= -2;
		//we can get segment npt directly
		if( iterValid( _itCurrent) )
		{
			ctrlNum = _itCurrent->userCtrlNum;
		}
		ZQ::common::Variant var;

		SYSTEMTIME st_utc;
		GetSystemTime(&st_utc);

		char	szNowUTC[256];
		ZeroMemory(szNowUTC,sizeof(szNowUTC));
		snprintf(szNowUTC, 255,  "%04d%02d%02dT%02d%02d%02d.%03dZ", 
			st_utc.wYear, st_utc.wMonth, st_utc.wDay,
			st_utc.wHour, st_utc.wMinute, st_utc.wSecond, 
			st_utc.wMilliseconds);
		var.set( EventField_StampUTC,	std::string(szNowUTC));
		var.set( EventField_PlaylistGuid, _strGuid);
		var.set( EventField_ClientSessId,	m_strClientSessionID);
		var.set( EventField_CurrentTimeOffset,	streamNpt);		
		var.set( EventField_UserCtrlNum , ctrlNum );
		var.set( EventField_SourceNetId , std::string(gStreamSmithConfig.szServiceID));

		m_pStreamSite->PostEventSink( E_PLAYLIST_PAUSETIMEOUT , var , _strGuid );
		glog(Log::L_INFO,PLSESSID(OnItemStepped, "event(E_PLAYLIST_PAUSETIMEOUT) is fired ."));
	}
}

void Playlist::onItemReposition( iterator it , int oldTimeOffset, int newTimeOffset , StreamControlResultInfo* controlInfo )
{
	if( !iterValid(it)) 	return;
	ZQ::common::MutexGuard gd(_listOpLocker);

	if(!m_pStreamSite)
	{
		m_pStreamSite=(StreamSmithSite*)StreamSmithSite::getDefaultSite();
	}	
	if(!m_pStreamSite)	{ return; }
	ZQ::common::Variant var;

	//snprintf
	SYSTEMTIME st_utc;
	GetSystemTime(&st_utc);
	char	szNowUTC[256];
	ZeroMemory(szNowUTC,sizeof(szNowUTC));
	snprintf(szNowUTC, 255,  "%04d%02d%02dT%02d%02d%02d.%03dZ", 
		st_utc.wYear, st_utc.wMonth, st_utc.wDay,
		st_utc.wHour, st_utc.wMinute, st_utc.wSecond, 
		st_utc.wMilliseconds);
	var.set( EventField_StampUTC,	std::string(szNowUTC));
	var.set( EventField_PlaylistGuid, _strGuid);
	var.set( EventField_ClientSessId,	m_strClientSessionID);
	var.set( EventField_CurrentItemTimeOffset,	newTimeOffset);		
	var.set( EventField_PrevItemTimeOffset , oldTimeOffset );
	var.set( EventField_UserCtrlNum , (int)it->userCtrlNum );
	var.set( EventField_SourceNetId , std::string(gStreamSmithConfig.szServiceID));

		ZQ::common::Variant varSpeed;
		varSpeed.clear();
		varSpeed.set(EventField_SpeedDenom,ZQ::common::Variant((int)_crntSpeed.denominator));
		varSpeed.set(EventField_SpeedNumer,ZQ::common::Variant((int)_crntSpeed.numerator));
	var.set(EventField_CurrentSpeed,varSpeed);
	
		ZQ::common::Variant varExtraProps;
		addExtraPropertiesToVariant( controlInfo , varExtraProps );
	var.set( EventField_ExtraProperties , varExtraProps );


	m_pStreamSite->PostEventSink( E_PLAYLIST_REPOSITION , var, _strGuid );
	glog(Log::L_INFO,PLSESSID(OnItemStepped, "event(E_PLAYLIST_REPOSITION) is fired . "));
}

void Playlist::checkEffectiveSession( SESSION_ID currentSessioId )
{
	ZQ::common::MutexGuard gd(_listOpLocker);
	glog(ZQ::common::Log::L_DEBUG , PLSESSID(checkEffectiveSession,"effective session[%u]"), currentSessioId );
	if( iterValid(_itCurrent) && _itCurrent->sessionId == currentSessioId )
		return;

	if( iterValid(_itCurrent)&& (_itCurrent->sessionId != 0) && iterValid(_itNext) && _itNext->sessionId == currentSessioId )
	{
		glog(ZQ::common::Log::L_INFO, PLSESSID(checkEffectiveSession,"session of next item is effective, step list") );
		stepList( _itCurrent->sessionId );
	}
}
bool Playlist::checkContentAttribute( iterator it )
{
	if( !iterValid(it))
	{
		glog(ZQ::common::Log::L_WARNING,PLSESSID(checkContentAttribute,"invalid iterator passed in"));
		return false;
	}
 	//can streamsmith work like this
 	if( !( it->_itemPlaytime <= 0 ||  it->_bPWE || isLastItem(it) ) )
	{
		return true;//take last item data
	}

	if(!_mgr.m_contentChecker.GetItemAttribute(it->_rawItemName,
												it->_itemPlaytime,
												it->_itemBitrate,
												it->_itemRealTotalTime,
												it->_bPWE,
												_strGuid,
												it->spliceIn,
												it->spliceOut,
												it->inTimeOffset,
												it->outTimeOffset))
	{
		ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID( checkContentAttribute , "failed to query item[%s][%lu] for attributes"),
			it->_rawItemName, it->userCtrlNum);
		return false;
	}
	else
	{
		glog(ZQ::common::Log::L_INFO,PLSESSID(checkContentAttribute,"got attributes of item[%s][%lu]: playtime[%ld]ms, bitrate[%ld] pwe[%s]"),
			it->_rawItemName,it->userCtrlNum,it->_itemPlaytime,it->_itemBitrate , it->_bPWE ? "true":"false");
		return true;
	}	
}

Playlist::iterator Playlist::lastItem( )
{
	iterator it = listEnd();
	if( size() > 0 )
		it = listEnd() - 1;
	return it;
}
bool Playlist::isLastItem( const iterator& it )
{
	if( it < listEnd() && it > listStart() )
	{
		return ( (it+1) == listEnd() );
	}
	else
		return false;
}

bool Playlist::isItemPWE( const iterator& it ) const
{
	if( !iterValid(it))
		return false;
	return it->_bPWE;
}

int64 Playlist::getTotalStreamDuration()
{
	int64 totalDur = 0;
	iterator itItem = listBegin();
	for (; itItem != listEnd(); itItem++ )
	{
		if ( itItem->_itemPlaytime <= 0 || ( GAPPLICATIONCONFIGURATION.lQueryLastItemPlayTime && ( isLastItem(itItem) ) ))
		{
			if ( !checkContentAttribute(itItem) )
			{
				_lastErrCode = ERR_PLAYLIST_SERVER_ERROR;
				ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(getTotalStreamDuration,"failed to get attributs of item[%s][%lu]"),
					itItem->_rawItemName,itItem->userCtrlNum);
				return false;
			}
			else
			{
				glog(ZQ::common::Log::L_DEBUG,PLSESSID(getTotalStreamDuration,"got attributes of item[%s][%lu]: playtime[%ld], bitrate[%ld]"),
				itItem->_rawItemName,itItem->userCtrlNum, itItem->_itemPlaytime,itItem->_itemBitrate);
			}
		}
		totalDur += itItem->_itemPlaytime;
	}
	return totalDur;
}

int64 Playlist::getTotalVideoStreamDuration()
{
	int64 totalDur = 0, playtimes = -1;
	iterator itItem = listBegin();
	for (; itItem != listEnd(); itItem++ )
	{
		playtimes = (itItem->itemFlag & PLISFlagPlayTimes) >> 4;
		if (playtimes < 1 || playtimes > 13)
		{
			if ( itItem->_itemPlaytime < 0  || ( GAPPLICATIONCONFIGURATION.lQueryLastItemPlayTime && ( isLastItem(itItem) ) ))
			{
				if ( !checkContentAttribute(itItem) )
				{
					_lastErrCode = ERR_PLAYLIST_SERVER_ERROR;
					ERRLOG(ZQ::common::Log::L_ERROR,PLSESSID(getTotalVideoStreamDuration,"failed to get attributs of item[%s][%lu]"),
						itItem->_rawItemName,itItem->userCtrlNum);
					return false;
				}
				else
				{
					glog(ZQ::common::Log::L_DEBUG,PLSESSID(getTotalVideoStreamDuration,"got attributes of item[%s][%lu]: playtime[%ld], bitrate[%ld]"),
					itItem->_rawItemName,itItem->userCtrlNum, itItem->_itemPlaytime,itItem->_itemBitrate);
				}
			}
			
			totalDur += itItem->_itemPlaytime;
		}
		
	}
	return totalDur;
}

}}//namespace

