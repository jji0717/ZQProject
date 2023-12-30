

#include <TianShanDefines.h>
#include <algorithm>
#include <log.h>
#include "VstrmSessionMonitor.h"
#include "StreamSmithConfig.h"


char* VodProviderStateCodeText[VOD_PROVIDER_STATE_COUNT];
#if _USE_NEW_SESSION_MON
#ifdef glog
	#undef glog
	#define glog (*m_pLog)
#endif//glog
#endif

/*
CLOSED 						= 0,	// Closed
LOADED 						= 1,	// Loaded and waiting
READY_TO_PLAY 				= 2,	// Loaded, selected and ready to go
PLAYING 					= 3,	// Playing at normal speed
ABOUT_TO_FAST_REVERSE		= 4,	// Preparing to play fast reverse
FAST_REVERSE				= 5,	// Playing fast reverse
ABOUT_TO_FAST_FORWARD		= 6,	// Preparing to play fast forward
FAST_FORWARD				= 7,	// Playing fast forward
ABOUT_TO_PAUSE 				= 8,	// Preparing to pause
PAUSED 						= 9,	// Paused
RESUMING 					= 10,	// Resuming
*/

char* vstrmSessionStateText[]=
{
	"Closed",
	"Loaded",
	"ReadToPlay",
	"Playing",
	"AboutToFastReverse",
	"FastReverse",
	"AboutToFastForward",
	"FastForward",
	"AboutToPause",
	"Paused",
	"Resuming",
	"Inactive",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	""
};

namespace ZQ{
namespace StreamSmith {


#define DELAY_QUERY_COUNT (30*3600*6) // 6hr

#ifndef VOD_STATE_ALL_EXCEPT_INIT
	#define VOD_STATE_ALL_EXCEPT_INIT	VOD_STATE_ALL_EXCEPT_IDLE
#endif



using namespace ZQ::common;

VstrmSessSink::VstrmSessSink(VstrmSessMon& monitor) :
_monitor(monitor)
{
	_nRef = 0;
}

VstrmSessSink::~VstrmSessSink()
{

}
void VstrmSessSink::SubScribleSink()
{
	_monitor.subscribe(*this);	
}
void VstrmSessSink::UnSubScribleSink()
{
	try
	{		
		_monitor.unsubscribe(*this);
	}
	catch(...)
	{	
	}
}

#ifdef _ICE_INTERFACE_SUPPORT
void		VstrmSessSink::VstrmSessNotifyWhenStartup(const VstrmSessInfo& sessinfo)
{
}
void		VstrmSessSink::vstrmSessNotifyOverWhenStartup()
{
}
#endif

void VstrmSessSink::OnVstrmSessDetected( const VstrmSessInfo& sessionInfo,ZQ::common::Log& log , const int64& timeStamp  )
{
	log(ZQ::common::Log::L_DEBUG, LOGFMT("OnVstrmSessDetected(%u)"), sessionInfo.sessionId );
}
void VstrmSessSink::OnVstrmSessStateChanged( const VstrmSessInfo& sessionInfo, ULONG curState ,ULONG PreviousState,ZQ::common::Log& log , const int64& timeStamp )
{
	log(ZQ::common::Log::L_DEBUG, LOGFMT("OnVstrmSessStateChanged(%u): %s(%d) => %s(%d)"), sessionInfo.sessionId, VodProviderStateCodeText[PreviousState], PreviousState, VodProviderStateCodeText[curState], curState);
}
void VstrmSessSink::OnVstrmSessSpeedChanged( const VstrmSessInfo& sessionInfo, SPEED_IND curSpeed , SPEED_IND PreviousSpeed,ZQ::common::Log& log ,  const int64& timeStamp )
{
	log(ZQ::common::Log::L_DEBUG, LOGFMT("OnVstrmSessSpeedChanged(%u): {%d, %d} => {%d, %d}"), 
		sessionInfo.sessionId, (LONG) PreviousSpeed.numerator, PreviousSpeed.denominator, curSpeed.numerator, curSpeed.denominator);
}
void VstrmSessSink::OnVstrmSessProgress( const VstrmSessInfo& sessionInfo, const TIME_OFFSET curTimeOffset , const TIME_OFFSET PreviousTimeOffset,ZQ::common::Log& log, const int64& timeStamp )
{
	//log(ZQ::common::Log::L_DEBUG, LOGFMT("OnVstrmSessProgress(%u): %u => %u"), sessinfo->sessionId, PreviousTimeOffset, sessinfo->currentTimeOffset);
}
void VstrmSessSink::OnVstrmSessExpired(const ULONG sessionID,ZQ::common::Log& log, const int64& timeStamp ,const std::string& reason , int errorCode )
{	
}

void VstrmSessSink::RegisterSessID(ULONG sessID)
{
	try
	{
		ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
		_SessIdStack.push_back(sessID);
	}
	catch(...)
	{		
	}
}

void VstrmSessSink::UnRegisterSessID(ULONG sessID)
{
	try
	{
		ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
		for(std::vector<ULONG>::iterator it = _SessIdStack.begin() ; it != _SessIdStack.end(); it++ )
		{
			if(*it == sessID)
			{
				_SessIdStack.erase( it );				
				return;
			}		
		}
	}
	catch(...)
	{		
	}
}


void	VstrmSessSink::NotifyVstrmSessDetected( const VstrmSessInfo& sessionInfo,
												ZQ::common::Log& log, 
												const int64& timeStamp )
{
	ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
	for(std::vector<ULONG>::iterator it=_SessIdStack.begin();it!=_SessIdStack.end();it++)
	{
		if( sessionInfo.sessionId == (*it) )
		{
			OnVstrmSessDetected( sessionInfo , log , timeStamp );
			return;
		}
	}
}
void	VstrmSessSink::NotifyVstrmSessStateChanged( const VstrmSessInfo& sessionInfo, 
													const ULONG curState,
													const ULONG PreviousState,
													ZQ::common::Log& log, 
													const int64& timeStamp )
{
	ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
	for(std::vector<ULONG>::iterator it=_SessIdStack.begin();it!=_SessIdStack.end();it++)
	{
		if( sessionInfo.sessionId == (*it) )
		{
			OnVstrmSessStateChanged( sessionInfo, curState,PreviousState,log, timeStamp);
			return;
		}
	}
}
void	VstrmSessSink::NotifyVstrmSessSpeedChanged( const VstrmSessInfo& sessionInfo,
													const SPEED_IND curSpeed,
													const SPEED_IND PreviousSpeed,
													ZQ::common::Log& log, 
													const int64& timeStamp )
{
	ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
	for(std::vector<ULONG>::iterator it=_SessIdStack.begin();it!=_SessIdStack.end();it++)
	{
		if( sessionInfo.sessionId == (*it) )
		{
			OnVstrmSessSpeedChanged( sessionInfo,curSpeed,PreviousSpeed,log, timeStamp);
			return;
		}
	}
}
void	VstrmSessSink::NotifyVstrmSessProgress( const VstrmSessInfo& sessionInfo, 
											    const TIME_OFFSET curTimeOffset,
												const TIME_OFFSET PreviousTimeOffset,
												ZQ::common::Log& log, 
												const int64& timeStamp )
{	
	ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
	for(std::vector<ULONG>::iterator it=_SessIdStack.begin();it!=_SessIdStack.end();it++)
	{
		if(  sessionInfo.sessionId == (*it) )
		{
			OnVstrmSessProgress( sessionInfo, curTimeOffset ,PreviousTimeOffset,log, timeStamp);
			return;
		}
	}	
}
void VstrmSessSink::NotifyVstrmSessExpired( const ULONG sessionID,
											ZQ::common::Log& log, 
											const int64& timeStamp )
{	
	ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
	for(std::vector<ULONG>::iterator it=_SessIdStack.begin();it!=_SessIdStack.end();it++)
	{
		if(sessionID== (*it) )
		{
			OnVstrmSessExpired(sessionID,log, timeStamp);
			return;
		}
	}	
}

//////////////////////////////////////////////////////////////////////////

VstrmSessMon::VstrmSessMon(VstrmClass& cls) 
: _cls(cls) , 
_bQuit(false)
{
	_lastOutputPortCount = 0;
	_pSessionInfoParam = NULL;
	_pSessState = NULL;
	_handleQuitted = NULL;
#ifdef _ICE_INTERFACE_SUPPORT

#endif	
}
VstrmSessMon::~VstrmSessMon()
{

}

bool VstrmSessMon::subscribe(VstrmSessSink& subscr)
{
	ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);

	for (Subscriber_v::iterator it =_subscribers.begin(); it != _subscribers.end(); it++)
	{
		if (*it == &subscr)
			return true;
	}
	_subscribers.push_back(&subscr);
	return true;
}

bool VstrmSessMon::unsubscribe(VstrmSessSink& subscr)
{	
	ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
	for (Subscriber_v::iterator it =_subscribers.begin(); it != _subscribers.end(); it++)
	{
		if (*it == &subscr)
		{
			_subscribers.erase(it);
			return true;
		}
	}
	return false;
}
void VstrmSessMon::stop( )
{
	if( !_bQuit)
	{
		_bQuit = true;
		waitHandle(INFINITE);
	}
}
bool lessCompare ( const VstrmSessInfo& elem1, const VstrmSessInfo& elem2 )
{
	return static_cast<bool>( elem1.sessionId < elem2.sessionId );
}

void VstrmSessMon::ScanSessWhenStartup()
{	
	querySession( true );	
	{
		_subCopy.clear( );
		ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
		for (Subscriber_v::iterator it =_subscribers.begin();
									it < _subscribers.end();
									it++)
		{
			if (NULL != (*it))
			{
				(*it)->addRef();
				_subCopy.push_back((*it));
			}
		}
	}
	std::sort( newSessionInfo.begin() , newSessionInfo.end() , lessCompare );
	oldSessionInfo = newSessionInfo;
	for (Subscriber_v::iterator it =_subCopy.begin();
								it < _subCopy.end();
								it++)
	{
		if (NULL != (*it))
		{
			SessionInfoSet::const_iterator itInfo = newSessionInfo.begin();
			for( ; itInfo != newSessionInfo.end( ) ; itInfo++ )
			{
				try
				{
					(*it)->VstrmSessNotifyWhenStartup( *itInfo );
				}
				catch (...)
				{
					glog(Log::L_ERROR,"error in VstrmSessNotifyWhenStartup");
				}
			}
			try
			{
				(*it)->vstrmSessNotifyOverWhenStartup();
			}
			catch (...) 
			{
				glog(Log::L_ERROR,"Error in vstrmSessNotifyOverWhenStartup");
			}

			(*it)->decRef();
		}
	}	
}
bool VstrmSessMon::init()
{
	INITIALIZE_VOD_STATE_CODE_TEXT	
	expiredSessionInfo.clear( );
	
	checkVstrmPortCount( );
	glog(Log::L_INFO,"Start vstrm session monitor");
	return true;
}
void VstrmSessMon::final( int retcode /* =0 */, bool bCancelled /* =false */ )
{
	glog(Log::L_INFO , "stop vstrm session monitor" );
}
bool ReadRegDWORDData(char* path,char* key,DWORD* output)
{
	if(!path || !key)
	{
		return false;
	}
	//open the registry first
	HKEY hResultKey = NULL;
	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,path,0,KEY_QUERY_VALUE,&hResultKey))
	{
		return false;
	}
	DWORD dwType=0;
	BYTE data[32];
	DWORD dataSize=32;
	if(ERROR_SUCCESS != RegQueryValueEx(hResultKey,key,0,&dwType,data,&dataSize))
	{
		return false;
	}
	if(dwType != REG_DWORD)
	{
		return false;
	}
	DWORD dwOutput;
	DWORD dwTemp;
	dwTemp=data[3];
	dwOutput=dwTemp<<24;
	dwTemp=data[2];
	dwTemp=dwTemp<<16;
	dwOutput|=dwTemp;
	dwTemp=data[1];
	dwTemp=dwTemp<<8;
	dwOutput|=dwTemp;
	dwOutput|=data[0];

	*output =dwOutput;

	return true;
}
static DWORD GetSystemErrorText(LPTSTR szSystemErrorText, DWORD dwBufSize)
{
	DWORD	dwLastError = GetLastError();

	FormatMessage (	FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dwLastError,
		MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
		szSystemErrorText,
		dwBufSize,
		NULL);

	return dwLastError;
}

bool VstrmSessMon::checkVstrmPortCount()
{
	static ULONG _checkDelayCount = 0;
	ULONG outputCount = 0;
	if (0 == _checkDelayCount++ )
	{
		if(!ReadRegDWORDData("SYSTEM\\CurrentControlSet\\Services\\VodDrv", "OutputPortCount", &outputCount) || outputCount <= 0 )
		{
			TCHAR szBuffer[1024]	;
			DWORD dwLastError;
			dwLastError = GetSystemErrorText(szBuffer, sizeof(szBuffer));			
			glog(ZQ::common::Log::L_ERROR,
				"SCP-E-REGVALUE, Error accessing OutputPortCount registry value - %s (0x%08x)", 
				szBuffer, 
				dwLastError );
			return false;
		}
		else
		{
			if( outputCount < 100 )
				outputCount = 100;
			oldSessionInfo.reserve( outputCount * 2 );
			newSessionInfo.reserve( outputCount * 2 );
			expiredSessionInfo.reserve( outputCount * 2 );
			if(outputCount!=_lastOutputPortCount)
			{//re-allocate memory
				//Clear last allocation first
				if(_pSessionInfoParam)
				{
					for(long i=0;i< _bufferCount;i++)
					{
						free(_pSessionInfoParam[i]);
					}
					free(_pSessionInfoParam);
					_pSessionInfoParam=NULL;
				}
				if(_pSessState)
				{
					free(_pSessState);
					_pSessState=NULL;
				}

				_maxSessionCount=outputCount*3;
				_bufferCount	= (_maxSessionCount / VOD_IOCTL_SESS_PER_INFO_BUF) + 1;

				_pSessionInfoParam=(PVOD_DRIVER_QUERY_NEXT_SESSION_PARMS*)malloc(sizeof(PVOD_DRIVER_QUERY_NEXT_SESSION_PARMS)*_bufferCount);
				if(!_pSessionInfoParam)
				{
					glog(Log::L_CRIT,"VstrmSessMon::ReadOutputPortCount() Not enough memory");
					return false;
				}
				for(int i=0;i<_bufferCount;i++)
				{
					_pSessionInfoParam[i]=(PVOD_DRIVER_QUERY_NEXT_SESSION_PARMS)malloc(VOD_IOCTL_SESS_INFO_BUF_SIZE);
					if(!_pSessionInfoParam[i])
					{
						glog(Log::L_CRIT,"VstrmSessMon::ReadOutputPortCount()##Not enough memory");
						return false;
					}
				}
				_pSessState=(PVOD_SESSION_INFORMATION*)malloc(_maxSessionCount*sizeof(PVOD_SESSION_INFORMATION));

				_lastOutputPortCount = outputCount;
			}
			_lastOutputCount = outputCount;
		}
	}
	return true;
}

ULONG convertVODStateToVstrmState( ULONG state )
{
	switch(state)
	{
	case VOD_PROVIDER_STATE_INIT:
		return 1;//loaded
	case VOD_PROVIDER_STATE_ERROR:
		return 0;//closed
	case VOD_PROVIDER_STATE_STREAMING_OPEN:
		return 2;//ready to play
	case VOD_PROVIDER_STATE_PAUSED_OPEN:
		return 8;//about to pause
	case VOD_PROVIDER_STATE_SPLICING:
		return 2;//ready to play
	case VOD_PROVIDER_STATE_ENTERING_STREAM:
		return 2;//ready to play
	case VOD_PROVIDER_STATE_PRIMING_STREAM:
		return 2;//read to play
	case VOD_PROVIDER_STATE_STREAMING:
		return 3;//playing
	case VOD_PROVIDER_STATE_LEAVING_STREAM:
		return 2;//ready to play
	case VOD_PROVIDER_STATE_PRIMING_PAUSE:
		return 8;//about to pause
	case VOD_PROVIDER_STATE_PAUSED:
		return 9;//paused
	case VOD_PROVIDER_STATE_LEAVING_PAUSE:
		return 2;//ready to play
	case VOD_PROVIDER_STATE_DONE:
		return 11;//inactive
#ifdef VOD_PROVIDER_STATE_STOPPED
	case VOD_PROVIDER_STATE_STOPPED:
		return 11;
#endif//VOD_PROVIDER_STATE_STOPPED

	}
	return 0;
}
void VstrmSessMon::convertDeviceIoCharToVstrmInfo( const VOD_SESSION_INFORMATION* sessinfo , VstrmSessInfo & info )
{
	
	info.sessionId			= sessinfo->sessionId;
	info.ByteOffset			= sessinfo->runningByteOffset;
	info.MuxRate			= sessinfo->mpegBitRate;
	info.ObjectSize			= sessinfo->endOfStreamByteOffset;
	info.PlayoutTimeOffset	= sessinfo->currentTimeOffset;
	info.Speed.numerator	= (SHORT)sessinfo->currentSpeed.numerator;
	info.Speed.denominator	= (USHORT)sessinfo->currentSpeed.denominator;

	info.State				= convertVODStateToVstrmState(sessinfo->currentState);
	info.PlayoutByteOffset	= sessinfo->runningByteOffset;
	info.DestPortHandle		= sessinfo->destPortHandle;
}
void VstrmSessMon::convertSessCharToVstrmInfo( const ESESSION_CHARACTERISTICS* sessinfo ,VstrmSessInfo& info )
{
	info.sessionId		= sessinfo->SessionId;
	info.ByteOffset		= sessinfo->SessionCharacteristics.ByteOffset.QuadPart;
	info.MuxRate		= sessinfo->SessionCharacteristics.MuxRate;
	info.ObjectSize		= sessinfo->SessionCharacteristics.ObjectSize.QuadPart;
	info.PlayoutTimeOffset = sessinfo->SessionCharacteristics.PlayoutTimeOffset;
	info.Speed			= sessinfo->SessionCharacteristics.Speed;
	info.State			= sessinfo->SessionCharacteristics.State;
	info.PlayoutByteOffset = sessinfo->SessionCharacteristics.PlayoutByteOffset.QuadPart;
	info.DestPortHandle	= sessinfo->DestPortHandle;
}

VSTATUS VstrmSessMon::vstrmFOR_EACH_SESSION_CB( HANDLE vstrmClassHandle, 
								 PVOID cbParm,
								 ESESSION_CHARACTERISTICS* sessionChars, 
								 ULONG sizeofSessionChars, 
								 ULONG currentSessionInstance, 
								 ULONG totalSessions )
{
	VstrmSessMon* pThis = reinterpret_cast<VstrmSessMon*>(cbParm);
	assert(pThis);
	if( !sessionChars->SessionCharacteristics.Hidden &&
		!sessionChars->SessionCharacteristics.DeadSession &&
		IS_VIDEO_PORT( sessionChars->DestPortType ) )
	{
		VstrmSessInfo info;
		convertSessCharToVstrmInfo(sessionChars,info);
		pThis->newSessionInfo.push_back( info );
	}
	return VSTRM_SUCCESS;
}

VSTATUS VstrmSessMon::vtrsmCount_CB ( HANDLE vstrmClassHandle, PVOID cbParm, ULONG totalPorts )
{
	VstrmSessMon* pThis = reinterpret_cast<VstrmSessMon*>(cbParm);
	assert(pThis);
	return VSTRM_SUCCESS;
}

void VstrmSessMon::querySession( bool bStart )
{
	newSessionInfo.clear();
	if( gStreamSmithConfig.lUseDeviceIoScan >= 1)
	{
		queryFromDeviceIo();
	}	
	else if( gStreamSmithConfig.lUseDeviceIoScan <= -1)
	{
		if( bStart)
		{
			queryFromVstrmScan();
		}
		else
		{
			queryFromVstrmEvent();
		}
	}
	else
	{
		queryFromVstrmScan();		
	}
}
void VstrmSessMon::queryFromVstrmScan( )
{	
	VstrmClassForEachSession( _cls.handle() , vtrsmCount_CB, vstrmFOR_EACH_SESSION_CB,this);
}

char* vstrmVodEventString[]=
{
	"VSTRM_VOD_EVENT_LOST",
	"VSTRM_VOD_FORWARD_SPEED_CHANGES_DISABLED",
	"VSTRM_VOD_FORWARD_SPEED_CHANGES_ENABLED",
	"VSTRM_VOD_SPEED_CHANGE",
	"VSTRM_VOD_AT_PAUSE",
	"VSTRM_VOD_AT_POSITION",
	"VSTRM_VOD_AT_SPEED",
	"VSTRM_VOD_AT_STOP",
	"VSTRM_VOD_NEW_SESSION_CHANGED_SPEED",
	"VSTRM_VOD_SESSION_STATISTICS",
	"VSTRM_VOD_CDN_BANDWIDTH_CHANGED_SPEED",
	"UNKNOWN",
	"UNKNOWN",
	"UNKNOWN",
};
void VstrmSessMon::onVodUserEvent( const VOD_USER_EVENT& et)
{
// 	char buffer[1024];
// 	snprintf(buffer,1023," event[%d/%s] port[%u] portSeq[%u] sess[%u] speed[%d/%d] offset[%u]",
// 		et.vodEvent,vstrmVodEventString[et.vodEvent], et.portHandle,et.portSequenceNumber,
// 		et.sessionId, et.speedIndicator.numerator, et.speedIndicator.denominator,
// 		et.timeOffset);
// 	glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmSessMon,"queryFromVstrmEvent() %s"),buffer);
	VstrmSessInfo info;

	info.PlayoutTimeOffset	= et.timeOffset;
	info.sessionId			= et.sessionId;
	info.Speed				= et.speedIndicator;
	info.DestPortHandle		= et.portHandle;
	info.noOldInfomation	= true;
	switch( et.vodEvent )
	{
	case VSTRM_VOD_SPEED_CHANGE:
		{
			glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmSessMon,"onVodEvent() port[%u] sessId[%u] newSpeed[%d/%d] timeffset[%u]"),
				et.portHandle, et.sessionId, et.speedIndicator.numerator, et.speedIndicator.denominator,et.timeOffset );
			SPEED_IND oldSpeed;oldSpeed.denominator = 0;oldSpeed.numerator = 0;
			for (Subscriber_v::iterator it = _subCopy.begin();
				it < _subCopy.end(); it++)
			{
				if (NULL != (*it))
				{
					try
					{
						(*it)->NotifyVstrmSessSpeedChanged(  info , info.Speed, oldSpeed , *m_pLog , ZQTianShan::now() );
					}
					catch (...)
					{
						glog(Log::L_ERROR,"error in NotifyVstrmSessSpeedChanged");
					}					
				}
			}		
		}
		break;
	default:
		break;
	}

}
void VstrmSessMon::queryFromVstrmEvent( )
{
	glog(ZQ::common::Log::L_INFO,CLOGFMT(VstrmSessMon,"queryFromVstrmEvent() acquiring vstrm event by using VstrmFindXXXVodNotification"));
	HANDLE vClsHandle = CreateFile (	CLASS_NAME,
						GENERIC_WRITE | GENERIC_READ,
						0,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
	if( vClsHandle == INVALID_HANDLE_VALUE )
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(VstrmSessMon,"queryFromVstrmEvent() failed to open vstrm class handle"));
		return;
	}

	VOD_USER_EVENT	vodEvent;
	HANDLE vodEventHandle = VstrmFindFirstVodNotification(vClsHandle, &vodEvent);
	if( vodEventHandle == INVALID_HANDLE_VALUE )
	{		
		VstrmClassCloseEx(vClsHandle);
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(VstrmSessMon,"queryFromVstrmEvent() failed to invoke VstrmFindFirstVodNotification, err[%s]"),
			getVstrmError(_cls.handle(),VstrmGetLastError() ) );
		return;
	}

	BOOL bOK = TRUE;
	int failureCount = 0;
	do 
	{
		if( bOK && vodEvent.sessionId )
		{
			onVodUserEvent(vodEvent);
		}
		bOK = VstrmFindNextVodNotification(vClsHandle,vodEventHandle,&vodEvent);
		if( !bOK && (failureCount++ >= 10 ))
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(VstrmSessMon,"queryFromVstrmEvent() failed to invoke VstrmFindNextVodNotification, err[%s]"),
				getVstrmError(_cls.handle(),VstrmGetLastError()));
			break;
		}
		else if( bOK )
		{
			failureCount = 0;
		}
	} while (TRUE);

	VstrmFindCloseVodNotification(vClsHandle, vodEventHandle);
	// Close handle to vstrm class driver
	VstrmClassCloseEx( vClsHandle );		
}

void VstrmSessMon::queryFromDeviceIo( )
{
	HANDLE		vodHandle			= INVALID_HANDLE_VALUE;
	ULONG		streamCount			= 0;
	ULONG		streamPausedCount	= 0;
	char*		driverName[]		={VOD_PROVIDER_NAME};
	PVOD_SESSION_INFORMATION sessionInfo = NULL;
	DWORD		amountRead;
	DWORD		dwLastError;

	if( !checkVstrmPortCount() )
	{
		glog(Log::L_CRIT,"**************Can't get output port information*********");
		return ;
	}
	ULONG bufferCount	= ( (_lastOutputCount*3) / VOD_IOCTL_SESS_PER_INFO_BUF) + 1;
	do
	{
		for( int iDriver =0 ; iDriver < 1; iDriver++ )
		{
			streamCount				= 0;
			streamPausedCount		= 0;
			// Establish a channel to the driver
			vodHandle = CreateFileA (driverName[iDriver],
				GENERIC_WRITE | GENERIC_READ,
				0,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

			if (vodHandle == INVALID_HANDLE_VALUE)
			{
				char szBuffer[1024];
				dwLastError = GetSystemErrorText(szBuffer, sizeof(szBuffer));
				glog(ZQ::common::Log::L_CRIT,"SCP-E-OPENVOD, Failed to access %s device driver - %s (0x%08x)",
					driverName[iDriver],szBuffer, dwLastError);				
				break;
			}

			// Allocate buffers to accommodate the maximum sessions possible ("streaming" and "init")

			// Allocate session info pointer array for sorting		

			for ( int i=0; i<(int)bufferCount; i++)
			{
				// Specify desired session states
				_pSessionInfoParam[i]->h.sessionStates = VOD_STATE_ALL;

				// Maintain context
				if (i==0)
					_pSessionInfoParam[i]->h.sessionContext = 0;	// 1st time
				else
					_pSessionInfoParam[i]->h.sessionContext = _pSessionInfoParam[i-1]->h.sessionContext;

				// Query for next buffer of session information
				if (!DeviceIoControl (vodHandle,
					(DWORD) IOCTL_VOD_DRIVER_QUERY_NEXT_SESSION, 
					_pSessionInfoParam[i],
					VOD_IOCTL_SESS_INFO_BUF_SIZE, 
					_pSessionInfoParam[i],
					VOD_IOCTL_SESS_INFO_BUF_SIZE, 
					&amountRead,
					NULL))
					break;

				// Locate 1st session
				sessionInfo = &_pSessionInfoParam[i]->sessionInformation;
				//			glog(ZQ::common::Log::L_ALERT,"sessionContenx=%u sessionCount=%d",
				//				_pSessionInfoParam[i]->h.sessionContext,_pSessionInfoParam[i]->h.sessionCount);
				// Add session info pointers to the session array
				for (int j = 0; j < (int)_pSessionInfoParam[i]->h.sessionCount; j++)
				{				
					_pSessState[streamCount++] = sessionInfo;
					sessionInfo++;
				}			
				// Test our limits
				if ((_pSessionInfoParam[i]->h.sessionCount < VOD_IOCTL_SESS_PER_INFO_BUF)) 	// buffer not filled

				{
					break;
				}
				if((streamCount >=(ULONG) _maxSessionCount))
				{// max sessions reached
					glog(Log::L_CRIT,"############### vstrmMon BUFFER INSUFFICIENT ######################");
					break;
				}
			}

			// Any sessions?
			if (streamCount > 0)
			{
				// Sort the session array
				
				BOOLEAN		bHeader				= TRUE;
				VstrmSessInfo info;
				for (int i = 0; i < (int)streamCount; i++)
				{
					//SessionInfos.push_back(*_pSessState[i]);
					//memcpy(&sessionInfoArray[curStreamCount+i],_pSessState[i],sizeof(VOD_SESSION_INFORMATION));
					convertDeviceIoCharToVstrmInfo( _pSessState[i] , info );
					newSessionInfo.push_back(info);
					
				}
				//curStreamCount =streamCount;
			}
			else
			{
				//m_NewSessionCount=0;
			}
			if (vodHandle != INVALID_HANDLE_VALUE)	CloseHandle(vodHandle);
		}
	} while (FALSE);
}

int VstrmSessMon::run( )
{
	
	int		scanInterval = 33;	
	scanInterval = gStreamSmithConfig.lVstrmSessionScanInterval;

	if( scanInterval > 1000 )
		scanInterval = 1000;

	if(scanInterval< 33 )
		scanInterval = 33;

	Ice::Long timeStamp = 0;
	Ice::Long queryStart = 0;
	Ice::Long queryStop = 0;

	while( !_bQuit )
	{
		queryStart = ZQTianShan::now();
		querySession( );
		timeStamp = ZQTianShan::now();
		size_t newInfoCount = newSessionInfo.size( );
		size_t oldInfoCount = oldSessionInfo.size( );
		expiredSessionInfo.clear( );
		std::sort( newSessionInfo.begin() , newSessionInfo.end() , lessCompare );
		if( newInfoCount == 0 && oldInfoCount > 0 )
		{			
			expiredSessionInfo = oldSessionInfo;			
		}
		else
		{

			SessionInfoSet::const_iterator itOld = oldSessionInfo.begin( ); 
			SessionInfoSet::const_iterator itNew = newSessionInfo.begin( );

			while( itOld != oldSessionInfo.end()  &&  itNew != newSessionInfo.end() )
			{			
				if( itOld->sessionId < itNew->sessionId )
				{				
					//expired session
					expiredSessionInfo.push_back( *itOld );
					itOld ++;
				}			
				else if( itOld->sessionId == itNew->sessionId )
				{
					if( itOld->Speed.denominator != itNew->Speed.denominator || 
						itOld->Speed.numerator != itNew->Speed.numerator )
					{//SpeedChange
						glog(ZQ::common::Log::L_DEBUG, 
							LOGFMT("session(%u) on port[%u] speed changed:[%ld/%ld] => [%ld/%ld] bitrate[%d] byteOffsetEOS[%lld] byteOffset[%lld] "), 
							itNew->sessionId,
							itNew->DestPortHandle,
							itOld->Speed.numerator, itOld->Speed.denominator,
							itNew->Speed.numerator, itNew->Speed.denominator,
							itNew->MuxRate,
							itNew->ObjectSize,
							itNew->ByteOffset );
						_subCopy.clear( );
						{
							ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
							for (Subscriber_v::iterator it =_subscribers.begin();
								it < _subscribers.end();
								it++ )
							{
								if (NULL != (*it))
								{
									(*it)->addRef();
									_subCopy.push_back((*it));
								}
							}
						}
						
						for (Subscriber_v::iterator it = _subCopy.begin();
							it < _subCopy.end();
							it++)
						{
							if (NULL != (*it))
							{
								try
								{
									(*it)->NotifyVstrmSessSpeedChanged(  *itNew ,itNew->Speed, itOld->Speed , *m_pLog , timeStamp );
								}
								catch (...)
								{
									glog(Log::L_ERROR,"error in NotifyVstrmSessSpeedChanged");
								}
								(*it)->decRef();
							}
						}
					}
					if( itOld->State != itNew->State )
					{//StateChange
						glog(ZQ::common::Log::L_DEBUG,
							LOGFMT("session(%u) on port[%u] state changed: %s(%d) => %s(%d)"), 
							itNew->sessionId,
							itNew->DestPortHandle,
							vstrmSessionStateText[itOld->State],
							itOld->State,
							vstrmSessionStateText[itNew->State],
							itNew->State);
						_subCopy.clear( );
						{
							ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
							for (Subscriber_v::iterator it =_subscribers.begin();
								it < _subscribers.end();
								it++ )
							{
								if (NULL != (*it))
								{
									(*it)->addRef();
									_subCopy.push_back((*it));
								}
							}
						}
						
						for (Subscriber_v::iterator it = _subCopy.begin();
							it < _subCopy.end();
							it++)
						{
							if (NULL != (*it))
							{
								try
								{
									(*it)->NotifyVstrmSessStateChanged( *itNew ,itNew->State, itOld->State , *m_pLog , timeStamp );
								}
								catch (...)
								{
									glog(Log::L_ERROR,"error in NotifyVstrmSessStateChanged");
								}
								(*it)->decRef();
							}
						}
					}
					itOld ++;
					itNew ++;
				}
				else 
				{
					glog(Log::L_DEBUG,LOGFMT("new session found [%u] on port[%u]"),
						itNew->sessionId , itNew->DestPortHandle );
					_subCopy.clear( );
					{
						ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
						for (Subscriber_v::iterator it =_subscribers.begin();
							it < _subscribers.end();
							it++ )
						{
							if (NULL != (*it))
							{
								(*it)->addRef();
								_subCopy.push_back((*it));
							}
						}
					}
					
					for (Subscriber_v::iterator it = _subCopy.begin();
						it < _subCopy.end();
						it++)
					{
						if (NULL != (*it))
						{
							try
							{
								(*it)->NotifyVstrmSessDetected( *itNew , *m_pLog, timeStamp );
							}
							catch (...)
							{
								glog(Log::L_ERROR,"error in NotifyVstrmSessDetected");
							}
							(*it)->decRef();
						}
					}

					itNew ++;
				}
			}
			while ( itNew != newSessionInfo.end() )
			{
				glog(Log::L_DEBUG,LOGFMT("new session found [%u] on port[%u]"),
					itNew->sessionId , itNew->DestPortHandle );
				_subCopy.clear( );
				{
					ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
					for (Subscriber_v::iterator it =_subscribers.begin();
						it < _subscribers.end();
						it++ )
					{
						if (NULL != (*it))
						{
							(*it)->addRef();
							_subCopy.push_back((*it));
						}
					}
				}
				
				for (Subscriber_v::iterator it = _subCopy.begin();
					it < _subCopy.end();
					it++)
				{
					if (NULL != (*it))
					{
						try
						{
							(*it)->NotifyVstrmSessDetected(  *itNew , *m_pLog, timeStamp );
						}
						catch (...)
						{
							glog(Log::L_ERROR,"error in NotifyVstrmSessDetected");
						}
						(*it)->decRef();
					}
				}
				itNew ++;
			}
			while ( itOld != oldSessionInfo.end() )
			{
				expiredSessionInfo.push_back(*itOld);
				itOld++;
			}
		}
		SessionInfoSet::const_iterator itExpired = expiredSessionInfo.begin();
		while ( itExpired != expiredSessionInfo.end() )
		{
			glog(Log::L_DEBUG,LOGFMT("expired session found %u on port[%u]"), 
				itExpired->sessionId ,itExpired->DestPortHandle );
			_subCopy.clear( );
			{
				ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
				for (Subscriber_v::iterator it =_subscribers.begin();
					it < _subscribers.end();
					it++ )
				{
					if (NULL != (*it))
					{
						(*it)->addRef();
						_subCopy.push_back((*it));
					}
				}
			}
			for (Subscriber_v::iterator it = _subCopy.begin();
				it < _subCopy.end();
				it++)
			{
				if (NULL != (*it))
				{
					try
					{
						(*it)->NotifyVstrmSessExpired( itExpired->sessionId , *m_pLog, timeStamp );
					}
					catch (...)
					{
						glog( Log::L_ERROR , "error in NotifyVstrmSessDetected" );
					}
					(*it)->decRef();
				}
			}
			itExpired ++;
		}	
		oldSessionInfo = newSessionInfo;
		queryStop = ZQTianShan::now();
		if( ( queryStop - queryStart ) < scanInterval )
		{
			Sleep( scanInterval - static_cast<int>(queryStop - queryStart) );
		}
	}
	
	return 1;
}


}}//namespace ZQ::StreamSmith

