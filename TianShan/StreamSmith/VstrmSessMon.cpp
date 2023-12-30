
#include "VstrmSessMon.h"
#include "winutil.h"
#include "log.h"
#include "streamsmithconfig.h"
#include <tianshandefines.h>
#ifdef _DEBUG
	#include "adebugmem.h"
#endif


#if _USE_NEW_SESSION_MON
#ifdef glog
	#undef glog
	#define glog (*m_pLog)
#endif//glog
	
#endif

#ifdef LOGFMT
	#undef LOGFMT
	#define LOGFMT 
#endif

#define DELAY_QUERY_COUNT (30*3600*6) // 6hr
//#define DELAY_QUERY_COUNT (1*1000) // 6hr

char* VodProviderStateCodeText[VOD_PROVIDER_STATE_COUNT];


#pragma message(__MSGLOC__"TODO:define a permanent macro for IDLE session use")

#ifndef VOD_STATE_ALL_EXCEPT_INIT
#define VOD_STATE_ALL_EXCEPT_INIT	VOD_STATE_ALL_EXCEPT_IDLE
#endif

using namespace ZQ::common;
namespace ZQ{
	namespace StreamSmith {
// -----------------------------
// class VstrmSessSink
// -----------------------------
VstrmSessSink::VstrmSessSink(VstrmSessMon& monitor) : _monitor(monitor)
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
		//ZQ::common::MutexGuard gd(_StackMutex,__MSGLOC__);
		_monitor.unsubscribe(*this);
	}
	catch(...)
	{
	//	glog(Log::L_ERROR,"unexpect error when call _monitor.unsubscribe(*this)");
	}
}
#if _USE_NEW_SESSION_MON
#ifdef _ICE_INTERFACE_SUPPORT
void		VstrmSessSink::VstrmSessNotifyWhenStartup(const PVOD_SESSION_INFORMATION sessinfo)
{
	
}
void		VstrmSessSink::vstrmSessNotifyOverWhenStartup()
{
	
}
#endif


void VstrmSessSink::OnVstrmSessDetected(const PVOD_SESSION_INFORMATION sessinfo,ZQ::common::Log& log , const int64& timeStamp  )
{
	log(ZQ::common::Log::L_DEBUG, LOGFMT("OnVstrmSessDetected(%u)"), sessinfo->sessionId);
}
void VstrmSessSink::OnVstrmSessStateChanged(const PVOD_SESSION_INFORMATION sessinfo, const ULONG PreviousState,ZQ::common::Log& log , const int64& timeStamp )
{
	log(ZQ::common::Log::L_DEBUG, LOGFMT("OnVstrmSessStateChanged(%u): %s(%d) => %s(%d)"), sessinfo->sessionId, VodProviderStateCodeText[PreviousState], PreviousState, VodProviderStateCodeText[sessinfo->currentState], sessinfo->currentState);	
}
void VstrmSessSink::OnVstrmSessSpeedChanged(const PVOD_SESSION_INFORMATION sessinfo, const VVX_FILE_SPEED PreviousSpeed,ZQ::common::Log& log ,  const int64& timeStamp )
{
	log(ZQ::common::Log::L_DEBUG, LOGFMT("OnVstrmSessSpeedChanged(%u): {%u, %u} => {%u, %u}"), sessinfo->sessionId, (LONGLONG) PreviousSpeed.numerator, PreviousSpeed.denominator, sessinfo->currentSpeed.numerator, sessinfo->currentSpeed.denominator);
}
void VstrmSessSink::OnVstrmSessProgress(const PVOD_SESSION_INFORMATION sessinfo, const TIME_OFFSET PreviousTimeOffset,ZQ::common::Log& log, const int64& timeStamp )
{
	//log(ZQ::common::Log::L_DEBUG, LOGFMT("OnVstrmSessProgress(%u): %u => %u"), sessinfo->sessionId, PreviousTimeOffset, sessinfo->currentTimeOffset);
}
void VstrmSessSink::OnVstrmSessExpired(const ULONG sessionID,ZQ::common::Log& log, const int64& timeStamp )
{	
}

void VstrmSessSink::RegisterSessID(ULONG sessID)
{
	try
	{
		ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
		_SessIdStack.push_back(sessID);		
		//glog(ZQ::common::Log::L_DEBUG,"[Playlist] SESSID(%s)[%16s]\tregister sessID [%u]",plsessID.c_str(),"RegisterSessID",sessID);
	}
	catch(...)
	{
	//	glog(Log::L_ERROR,"unexpect error when RegisterSessID ");
	}
}
void VstrmSessSink::UnRegisterSessID(ULONG sessID)
{
	try
	{
		ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
		for(std::vector<ULONG>::iterator it=_SessIdStack.begin();it!=_SessIdStack.end();it++)
		{
			if(*it == sessID)
			{
				_SessIdStack.erase(it);				
				return;
			}		
		}
	}
	catch(...)
	{
	//	glog(Log::L_ERROR,"unexpect error when UnRegisterSessID");
	}
}
void	VstrmSessSink::NotifyVstrmSessDetected(const PVOD_SESSION_INFORMATION sessinfo,ZQ::common::Log& log, const int64& timeStamp )
{
//	try
//	{
		ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
		for(std::vector<ULONG>::iterator it=_SessIdStack.begin();it!=_SessIdStack.end();it++)
		{
			if(sessinfo->sessionId== (*it) )
			{
				OnVstrmSessDetected(sessinfo, log, timeStamp);
				return;
			}
		}
//	}
//	catch(...)
//	{
//		glog(Log::L_ERROR,"unexpect error when call OnVstrmSessDetected");
//	}
}
void	VstrmSessSink::NotifyVstrmSessStateChanged(const PVOD_SESSION_INFORMATION sessinfo, const ULONG PreviousState,ZQ::common::Log& log, const int64& timeStamp )
{
//	try
//	{
		ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
		for(std::vector<ULONG>::iterator it=_SessIdStack.begin();it!=_SessIdStack.end();it++)
		{
			if(sessinfo->sessionId== (*it) )
			{
				OnVstrmSessStateChanged(sessinfo,PreviousState,log, timeStamp);
				return;
			}
		}
//	}
//	catch(...)
//	{
//		glog(Log::L_ERROR,"unexpect error when call OnVstrmSessStateChanged");
//	}
}
void	VstrmSessSink::NotifyVstrmSessSpeedChanged(const PVOD_SESSION_INFORMATION sessinfo, const VVX_FILE_SPEED PreviousSpeed,ZQ::common::Log& log, const int64& timeStamp )
{
//	try
//	{
		ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
		for(std::vector<ULONG>::iterator it=_SessIdStack.begin();it!=_SessIdStack.end();it++)
		{
			if(sessinfo->sessionId== (*it) )
			{
				OnVstrmSessSpeedChanged(sessinfo,PreviousSpeed,log, timeStamp);
				return;
			}
		}
//	}
//	catch(...)
//	{
//		glog(Log::L_ERROR,"unexpect error when call OnVstrmSessSpeedChanged");
//	}
}
void	VstrmSessSink::NotifyVstrmSessProgress(const PVOD_SESSION_INFORMATION sessinfo, const TIME_OFFSET PreviousTimeOffset,ZQ::common::Log& log, const int64& timeStamp )
{
//	try
//	{
		ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
		for(std::vector<ULONG>::iterator it=_SessIdStack.begin();it!=_SessIdStack.end();it++)
		{
			if(sessinfo->sessionId== (*it) )
			{
				OnVstrmSessProgress(sessinfo,PreviousTimeOffset,log, timeStamp);
				return;
			}
		}
//	}
//	catch(...)
//	{
//		glog(Log::L_ERROR,"unexpect error when call OnVstrmSessSpeedChanged");
//	}
}
void VstrmSessSink::NotifyVstrmSessExpired(const ULONG sessionID,ZQ::common::Log& log, const int64& timeStamp )
{
//	try
//	{
		ZQ::common::MutexGuard gd(_listOpLocker,__MSGLOC__);
		for(std::vector<ULONG>::iterator it=_SessIdStack.begin();it!=_SessIdStack.end();it++)
		{
			if(sessionID== (*it) )
			{
				OnVstrmSessExpired(sessionID,log, timeStamp);
				return;
			}
		}
//	}
//	catch(...)
//	{
//		glog(Log::L_ERROR,"unexpect error when call OnVstrmSessExpired");
//	}
}

#else//_USE_NEW_SESSION_MON

void VstrmSessSink::OnVstrmSessDetected(const PVOD_SESSION_INFORMATION sessinfo)
{
	glog(ZQ::common::Log::L_DEBUG, LOGFMT("OnVstrmSessDetected(%u)"), sessinfo->sessionId);
}

void VstrmSessSink::OnVstrmSessStateChanged(const PVOD_SESSION_INFORMATION sessinfo, const ULONG PreviousState)
{
	glog(ZQ::common::Log::L_DEBUG, LOGFMT("OnVstrmSessStateChanged(%u): %s(%d) => %s(%d)"), sessinfo->sessionId, VodProviderStateCodeText[PreviousState], PreviousState, VodProviderStateCodeText[sessinfo->currentState], sessinfo->currentState);
}

void VstrmSessSink::OnVstrmSessSpeedChanged(const PVOD_SESSION_INFORMATION sessinfo, const VVX_FILE_SPEED PreviousSpeed)
{
	glog(ZQ::common::Log::L_DEBUG, LOGFMT("OnVstrmSessSpeedChanged(%u): {%u, %u} => {%u, %u}"), sessinfo->sessionId, (LONGLONG) PreviousSpeed.numerator, PreviousSpeed.denominator, sessinfo->currentSpeed.numerator, sessinfo->currentSpeed.denominator);
}

void VstrmSessSink::OnVstrmSessProgress(const PVOD_SESSION_INFORMATION sessinfo, const TIME_OFFSET PreviousTimeOffset)
{
	glog(ZQ::common::Log::L_DEBUG, LOGFMT("OnVstrmSessProgress(%u): %u => %u"), sessinfo->sessionId, PreviousTimeOffset, sessinfo->currentTimeOffset);
}
#endif//_USE_NEW_SESSION_MON
// -----------------------------
// class VstrmSessMon
// -----------------------------

VstrmSessMon::VstrmSessMon(VstrmClass& cls) : _cls(cls) , _bQuit(false), ThreadRequest(cls.getThreadPool()) 
{
	_handleQuitted=::CreateEvent(NULL,FALSE,FALSE,NULL);
	_checkDelayCount=0;
	_lastOutputPortCount=0;
	_pSessionInfoParam=NULL;
	_pSessState=NULL;
	m_pNewsessInfoArray=NULL;
	m_pOldSessionArray=NULL;
	m_pExpiredIDArray=NULL;
	m_NewSessionCount=m_OldSessionCount=0;
	m_pLog=NULL;
	m_ProgressGranularity=100000;
	m_pExpiredVStrmPort[0]=m_pExpiredVStrmPort[1]=NULL;
	_bufferCount = 0;
	_maxSessionCount = 0;
	
#ifdef _ICE_INTERFACE_SUPPORT
	m_pVstrmPortArrWhenStartup=NULL;
#endif	
}
VstrmSessMon::~VstrmSessMon()
{
	if(_pSessState)
	{
		free(_pSessState);
		_pSessState=NULL;
	}
	if(_pSessionInfoParam)
	{
		for(int i=0;i<_bufferCount;i++)
		{
			free(_pSessionInfoParam[i]);
		}
		free(_pSessionInfoParam);
		_pSessionInfoParam=NULL;
	}
	if(m_pNewsessInfoArray)
	{
		free(m_pNewsessInfoArray);
		m_pNewsessInfoArray=NULL;
	}
	if(m_pOldSessionArray)
	{
		free(m_pOldSessionArray);
		m_pOldSessionArray=NULL;
	}
	if(m_pExpiredIDArray)
	{
		free(m_pExpiredIDArray);
		m_pExpiredIDArray=NULL;
	}
	if(m_pExpiredVStrmPort[0])
	{
		free(m_pExpiredVStrmPort[0]);					
	}
	if(m_pExpiredVStrmPort[1])
	{
		free(m_pExpiredVStrmPort[1]);
	}
#ifdef _ICE_INTERFACE_SUPPORT
	if(m_pVstrmPortArrWhenStartup!=NULL)
	{
		free(m_pVstrmPortArrWhenStartup);
	}
#endif	
//	if(m_pLog)
//	{
//		delete m_pLog;
//	}
}
bool VstrmSessMon::removeSessionInfo(ULONG sessId)
{
	SessionInfoMap::iterator it = _sessInfos.find(sessId);

	if (it == _sessInfos.end())
		return false;
	
	_sessInfos.erase(it);
	return true;
}

const PVOD_SESSION_INFORMATION VstrmSessMon::findSessionInfo(ULONG sessId)
{
	SessionInfoMap::iterator it = _sessInfos.find(sessId);

	return (it != _sessInfos.end() ? &it->second : NULL);
}

bool VstrmSessMon::subscribe(VstrmSessSink& subscr)
{
	ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
	
	for (Subscriber_v::iterator it =_subscribers.begin(); it != _subscribers.end(); it++)
	{
		if (*it == &subscr)
			return true;
	}	
	///这一段代码到底有什么问题呢？
//	glog(Log::L_DEBUG,"before push subcriber");
	_subscribers.push_back(&subscr);
//	glog(Log::L_DEBUG,"after push subcriber");

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



bool VstrmSessMon::init()
{
	glog(ZQ::common::Log::L_DEBUG,"bool VstrmSessMon::init()##Create VstrmSessMon request %p",this);
	INITIALIZE_VOD_STATE_CODE_TEXT
	return true;
}

static bool lessSessInfo(const VOD_SESSION_INFORMATION& elem1, const VOD_SESSION_INFORMATION& elem2)
{
	return (elem1.sessionId < elem2.sessionId);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void VstrmSessMon::SetProgressGranularity(long Granularity)
{
	if(Granularity<10)
		return;
	m_ProgressGranularity=Granularity;
}

#if _USE_NEW_SESSION_MON
int compareID( void * elem1, void * elem2 )
{
	long* p1=(long*)elem1;
	long* p2=(long*)elem2;
	return memcmp(p1,p2,sizeof(long));
}

#ifdef _ICE_INTERFACE_SUPPORT
void VstrmSessMon::ScanSessWhenStartup()
{
	ReadOutputPortCount();
	querySessionInfo(m_pNewsessInfoArray);

	Subscriber_v subCopy;
	{
		ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
		for (Subscriber_v::iterator it =_subscribers.begin();
									it < _subscribers.end();
									it++)
		{
			if (NULL != (*it))
			{
				(*it)->addRef();
				subCopy.push_back((*it));
			}
		}
	}
	for (Subscriber_v::iterator it =subCopy.begin();
								it < subCopy.end();
								it++)
	{
		if (NULL != (*it))
		{
			for(long i=0;i<m_NewSessionCount;i++)
			{
				
				try
				{
					(*it)->VstrmSessNotifyWhenStartup(&m_pNewsessInfoArray[i]);
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
#endif//_ICE_INTERFACE_SUPPORT


int CompareSession(const void * elem1,const void * elem2)
{
	VOD_SESSION_INFORMATION* p1=(VOD_SESSION_INFORMATION*)elem1;
	VOD_SESSION_INFORMATION* p2=(VOD_SESSION_INFORMATION*)elem2;
	//return memcmp(&p1->sessionId,&p2->sessionId,sizeof(p1->sessionId));

	if(p1->sessionId>p2->sessionId)
		return 1;
	else if(p1->sessionId==p2->sessionId)
		return 0;
	else
		return -1;
}
long VstrmSessMon::FindSessIDInSession(VOD_SESSION_INFORMATION* pSess,long sessCount,ULONG SessID,long oldCurIter)
{
	long low=oldCurIter;
	long high=sessCount-1;
	long mid=low;
	while (low<=high) 
	{
		mid=(low+high)/2;
		if(pSess[mid].sessionId==SessID)
		{
			m_bFoundNewSessionIDInArray=true;
			return mid;
		}
		else if(pSess[mid].sessionId>SessID)
			high=mid-1;
		else
			low=mid+1;
	}
	m_bFoundNewSessionIDInArray= false;
	return -1;

}
#ifdef max 
#undef max
#endif

#ifdef min 
#undef min
#endif

#define max(a,b)	(a>b?a:b)
#define min(a,b)	(a<b?a:b)

int VstrmSessMon::run()
{	
	long lStart=0;
	long lTest=0;
	long lMeanTime=0;
	ReadOutputPortCount();
	long progressScanInterval = 2000;
	long scanCoef = 3;
	int64 lastTimeStamp = 0;
	while (!_bQuit)
	{
		progressScanInterval = gStreamSmithConfig.lprogressScanInterval > 1000 ? 
										gStreamSmithConfig.lprogressScanInterval : 1000;
		scanCoef			 = max( 1, min ( 6 , gStreamSmithConfig.lSessScanCoef ) );

		lStart=GetTickCount();
		//////////////////////////////////////////////////////////////////////////
		long	lExpiredCount=0;
		long	lTempOldIDIter=0;
		//////////////////////////////////////////////////////////////////////////
		long	lNewCount=0;		
		//////////////////////////////////////////////////////////////////////////
		
		lExpiredCount=0;
		lTempOldIDIter=0;
		lNewCount=0;
		//query session information
		querySessionInfo(m_pNewsessInfoArray);
		lastTimeStamp = ZQTianShan::now();
		qsort(m_pNewsessInfoArray,m_NewSessionCount,sizeof(VOD_SESSION_INFORMATION),CompareSession);
		
		if(m_NewSessionCount<=0 && m_OldSessionCount>0)
		{
			for (long ii=0;ii<m_OldSessionCount;ii++) 
			{
				m_pExpiredVStrmPort[0][lExpiredCount]=m_pOldSessionArray[ii].destPortHandle;
				m_pExpiredVStrmPort[1][lExpiredCount]=m_pOldSessionArray[ii].sessionId;
				
				m_pExpiredIDArray[lExpiredCount]=m_pOldSessionArray[ii].sessionId;
				lExpiredCount++;				
			}
		}
		for(long oneScan=0;oneScan<m_NewSessionCount;oneScan++)
		{
			while(m_pOldSessionArray[lTempOldIDIter].sessionId < m_pNewsessInfoArray[oneScan].sessionId &&
				lTempOldIDIter<m_OldSessionCount )
			{//old id is expired
				//lTempOldIDIter++;
				m_pExpiredVStrmPort[0][lExpiredCount]=m_pOldSessionArray[lTempOldIDIter].destPortHandle;
				m_pExpiredVStrmPort[1][lExpiredCount]=m_pOldSessionArray[lTempOldIDIter].sessionId;
				
				m_pExpiredIDArray[lExpiredCount]=m_pOldSessionArray[lTempOldIDIter].sessionId;
				lExpiredCount++;
				lTempOldIDIter++;
				
			}
			if(lTempOldIDIter<m_OldSessionCount && 
				m_pOldSessionArray[lTempOldIDIter].sessionId==m_pNewsessInfoArray[oneScan].sessionId)
				lTempOldIDIter++;
			
			long oldIter=FindSessIDInSession(m_pOldSessionArray,m_OldSessionCount, m_pNewsessInfoArray[oneScan].sessionId,0);
			if(!m_bFoundNewSessionIDInArray)
			{//new session
				glog(Log::L_DEBUG,LOGFMT("VstrmSessMon::Run()::new session found %u"),m_pNewsessInfoArray[oneScan].sessionId);
				//////////////////////////////////////////////////////////////////////////				
				lNewCount++;
				//////////////////////////////////////////////////////////////////////////
				///notify that new session found ,it must use a vstrm port
#ifdef RESOURCE_MAN_TEST
				_cls.PortUsageChanged(m_pNewsessInfoArray[oneScan].destPortHandle,m_pNewsessInfoArray[oneScan].sessionId,PORT_IN_USE);
#endif
				
				Subscriber_v subCopy;
				{
					ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
					for (Subscriber_v::iterator it =_subscribers.begin();
					it < _subscribers.end();
					it++)
					{
						if (NULL != (*it))
						{
							(*it)->addRef();
							subCopy.push_back((*it));
						}
					}
				}
				for (Subscriber_v::iterator it =subCopy.begin();
				it < subCopy.end();
				it++)
				{
					if (NULL != (*it))
					{
						try
						{
							(*it)->NotifyVstrmSessDetected(&m_pNewsessInfoArray[oneScan],*m_pLog,lastTimeStamp);
						}
						catch (...)
						{
							glog(Log::L_ERROR,"error in NotifyVstrmSessDetected");
						}
						(*it)->decRef();
					}
				}		
				
			}
			else//!found new session array id in old session array
			{			
				if( memcmp(	&(m_pNewsessInfoArray[oneScan].currentSpeed),
					&(m_pOldSessionArray[oldIter].currentSpeed),
					sizeof(m_pOldSessionArray[oldIter].currentSpeed)) !=0
					)
				{
					glog(ZQ::common::Log::L_DEBUG, LOGFMT("VstrmSessMon::run() session(%u) speed changed: %ld/%ld => %ld/%ld And Now bitrate=%d byteOffsetEOS=%lld byteOffset=%lld "), 
						m_pNewsessInfoArray[oneScan].sessionId,
						m_pOldSessionArray[oldIter].currentSpeed.numerator, m_pOldSessionArray[oldIter].currentSpeed.denominator,
						m_pNewsessInfoArray[oneScan].currentSpeed.numerator, m_pNewsessInfoArray[oneScan].currentSpeed.denominator,
						m_pNewsessInfoArray[oneScan].mpegBitRate,m_pNewsessInfoArray[oneScan].endOfStreamByteOffset,m_pNewsessInfoArray[oneScan].currentTimeOffset );
					Subscriber_v subCopy;
					{
						ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
						for (Subscriber_v::iterator it =_subscribers.begin();
													it < _subscribers.end();
													it++)
						{
							if (NULL != (*it))
							{
								(*it)->addRef();
								subCopy.push_back((*it));
							}
						}
					}
					for (Subscriber_v::iterator it =subCopy.begin();
												it < subCopy.end();
												it++)
					{
						if (NULL != (*it))
						{
							try
							{
								(*it)->NotifyVstrmSessSpeedChanged(&m_pNewsessInfoArray[oneScan],m_pOldSessionArray[oldIter].currentSpeed,*m_pLog,lastTimeStamp);
							}
							catch (...) 
							{
								glog(Log::L_ERROR,"error in NotifyVstrmSessSpeedChanged");
							}
							(*it)->decRef();
						}
					}

				}
				if( m_pNewsessInfoArray[oneScan].currentState != m_pOldSessionArray[oldIter].currentState )
				{//session state changed
					glog(ZQ::common::Log::L_DEBUG, LOGFMT("VstrmSessMon::run()::session(%u) state changed: %s(%d) => %s(%d)"), 
						m_pNewsessInfoArray[oneScan].sessionId,
						VodProviderStateCodeText[m_pOldSessionArray[oldIter].currentState], m_pOldSessionArray[oldIter].currentState,
						VodProviderStateCodeText[m_pNewsessInfoArray[oneScan].currentState], m_pNewsessInfoArray[oneScan].currentState);
					
					Subscriber_v subCopy;
					{
						ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
						for (Subscriber_v::iterator it =_subscribers.begin();
													it < _subscribers.end();
													it++)
						{
							if (NULL != (*it))
							{
								(*it)->addRef();
								subCopy.push_back((*it));
							}
						}
					}
					for (Subscriber_v::iterator it =subCopy.begin();
												it < subCopy.end();
												it++)
					{
						if(NULL!=(*it))
						{
							try
							{
								(*it)->NotifyVstrmSessStateChanged(&m_pNewsessInfoArray[oneScan],m_pOldSessionArray[oldIter].currentState,*m_pLog,lastTimeStamp);
							}
							catch (...) 
							{
								glog(Log::L_ERROR,"error in NotifyVstrmSessStateChanged");
							}
							(*it)->decRef();
						}					
					}	
					
//					if(VOD_STATE(DONE) == m_pNewsessInfoArray[oneScan].currentState)
//					{//it's expired
//						m_pExpiredVStrmPort[0][lExpiredCount]=m_pOldSessionArray[oldIter].destPortHandle;
//						m_pExpiredVStrmPort[1][lExpiredCount]=m_pOldSessionArray[oldIter].sessionId;							
//						m_pExpiredIDArray[lExpiredCount++]=m_pOldSessionArray[oldIter].sessionId;
//					}
				}
				if( (m_pNewsessInfoArray[oneScan].currentTimeOffset/progressScanInterval) != 
						(m_pOldSessionArray[oldIter].currentTimeOffset/progressScanInterval))
				{
#pragma message(__MSGLOC__"TODO:Can I hard code the sess progress scan interval ??? ")
//					glog(Log::L_DEBUG,LOGFMT("VstrmSessMon::run()::session=%ld progress changed from %ld to %ld"),
//						m_pNewsessInfoArray[oneScan].sessionId,
//						m_pOldSessionArray[oldIter].currentTimeOffset,m_pNewsessInfoArray[oneScan].currentTimeOffset );
					Subscriber_v subCopy;
					{
						ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
						for (Subscriber_v::iterator it =_subscribers.begin();
													it < _subscribers.end();
													it++)
						{
							if (NULL != (*it))
							{
								(*it)->addRef();
								subCopy.push_back((*it));
							}
						}
					}
					for (Subscriber_v::iterator it =subCopy.begin();
												it < subCopy.end();
												it++)
					{
						if (NULL != (*it))
						{
							try
							{						
								(*it)->NotifyVstrmSessProgress(&m_pNewsessInfoArray[oneScan],m_pOldSessionArray[oldIter].currentTimeOffset,*m_pLog,lastTimeStamp);
							}
							catch (...) 
							{
								glog(Log::L_ERROR,"error in NotifyVstrmSessProgress");
							}
							(*it)->decRef();
						}
					}
				}
			}
		}
		if(m_NewSessionCount>0 )
		{//m_NewSessionCount<=0的时候所有的session都已经被计入expired Session了
			while ( lTempOldIDIter<m_OldSessionCount)
			{
				m_pExpiredVStrmPort[0][lExpiredCount]=m_pOldSessionArray[lTempOldIDIter].destPortHandle;
				m_pExpiredVStrmPort[1][lExpiredCount]=m_pOldSessionArray[lTempOldIDIter].sessionId;			
				m_pExpiredIDArray[lExpiredCount]=m_pOldSessionArray[lTempOldIDIter].sessionId;
				lExpiredCount++;
				lTempOldIDIter++;
			}
		}
	

		
		//////////////////////////////////////////////////////////////////////////
		for(long i=0;i<lExpiredCount;i++)
		{
//			//////////////////////////////////////////////////////////////////////////
//			if(m_OldSessionCount>0)
//				DumpSessionArray(m_pOldSessionArray,m_OldSessionCount,"Old Session Array");
//			if(m_NewSessionCount>0)
//				DumpSessionArray(m_pNewsessInfoArray,m_NewSessionCount,"New Session Array");
//
//			FindSessIDInSession(m_pOldSessionArray,m_OldSessionCount,m_pExpiredIDArray[i],0);
//			if(!m_bFoundNewSessionIDInArray)
//			{//don't find it
//				glog(Log::L_DEBUG,"wrong expired session %u in old session array and",m_pExpiredIDArray[i]);
//				DumpSessionArray(m_pOldSessionArray,m_OldSessionCount,"Old Session Array");
//				DumpSessionArray(m_pNewsessInfoArray,m_NewSessionCount,"New Session Array");				
//			}
//			FindSessIDInSession(m_pNewsessInfoArray,m_NewSessionCount,m_pExpiredIDArray[i],0);
//			if(m_bFoundNewSessionIDInArray)
//			{//find it
//				glog(Log::L_DEBUG,"wrong expired session %u in new session array",m_pExpiredIDArray[i]);
//				DumpSessionArray(m_pOldSessionArray,m_OldSessionCount,"Old Session Array");
//				DumpSessionArray(m_pNewsessInfoArray,m_NewSessionCount,"New Session Array");				
//			}
//
//			//////////////////////////////////////////////////////////////////////////
			
			glog(Log::L_DEBUG,"VStrmSessMon::Run()::expired session (%u)",m_pExpiredIDArray[i]);
#ifdef RESOURCE_MAN_TEST
			_cls.PortUsageChanged(m_pExpiredVStrmPort[0][i],m_pExpiredVStrmPort[1][i], PORT_NORMAL);
#endif			
			//NotifyVstrmSessExpired
			Subscriber_v subCopy;
			{
				ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
				for (Subscriber_v::iterator it =_subscribers.begin();
											it < _subscribers.end();
											it++)
				{
					if (NULL != (*it))
					{
						(*it)->addRef();
						subCopy.push_back((*it));
					}
				}
			}
			for (Subscriber_v::iterator it =subCopy.begin();
										it < subCopy.end();
										it++)
			{
				if (NULL != (*it))
				{
					try
					{						
						(*it)->NotifyVstrmSessExpired(m_pExpiredIDArray[i],*m_pLog,lastTimeStamp);
					}
					catch (...) 
					{
						glog(Log::L_ERROR,"error in NotifyVstrmSessExpired");
					}
					(*it)->decRef();
				}
			}

		}
		lExpiredCount=0;
		lTempOldIDIter=0;
		

		memcpy(m_pOldSessionArray,m_pNewsessInfoArray,sizeof(VOD_SESSION_INFORMATION)*m_NewSessionCount );
		m_OldSessionCount=m_NewSessionCount;		
		
		
		lStart=GetTickCount()-lStart;
		lMeanTime+=lStart;
		lTest++;
		if(lTest>=300)
		{
			if(lMeanTime/300>3)
				glog(Log::L_INFO,"Scan Mean time=[%d]",lMeanTime/1000);
			lMeanTime=0;
			lTest=0;
		}
		if( 33*scanCoef - lStart>0 )
		{
			Sleep( 33*scanCoef - lStart ); // for ntsc 30 fps: 1000/30
		}
		else
		{
			glog(Log::L_INFO,"vstream session scan ,time = (%d) and new Session=%d other session=%d",lStart,lNewCount,m_NewSessionCount-lNewCount);
		}
		
	}
	SetEvent(_handleQuitted);
	CloseHandle(_handleQuitted);
	return 0;
}
void VstrmSessMon::DumpSessionArray(VOD_SESSION_INFORMATION* pSess,int iCount,const char* tips)
{
	glog(Log::L_DEBUG,"######Dump %s with size =%d##########",tips,iCount);
	
	for(int i=0;i<iCount;i++)
	{
		glog(Log::L_DEBUG,"Session ID is %u",pSess[i].sessionId);
	}
	glog(Log::L_DEBUG,"######################");
}
#else//_USE_NEW_SESSION_MON
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int VstrmSessMon::run()
{
	VstrmSessMon::SessionIdx_v SessionIdxs;
	VstrmSessMon::SessionInfo_v SessionInfos;

	
	long lStart=0;
	long lTest=0;
	long lMeanTime=0;
	while (!_bQuit)
	{	
		lStart=GetTickCount();

		std::vector <ULONG> oldIds, expiredIds;
#ifdef RESOURCE_MAN_TEST
		//Sess port
		std::map<ULONG,ULONG> oldPort, expiredPort;
#endif//RESOURCE_MAN_TEST
		for(SessionInfoMap::iterator sessit = _sessInfos.begin(); sessit != _sessInfos.end(); sessit++)
		{
			oldIds.push_back(sessit->first);
#ifdef RESOURCE_MAN_TEST
			oldPort.insert(std::make_pair<ULONG,ULONG>(sessit->first,sessit->second.destPortHandle));
#endif
		}

		std::sort(oldIds.begin( ), oldIds.end( ));

		querySessionInfo(SessionInfos);
		std::sort(SessionInfos.begin( ), SessionInfos.end( ), lessSessInfo);

//		querySessionIndex(SessionIdxs);

		std::vector <ULONG>::iterator itId = oldIds.begin();

		// for each found session info
		for (int i=0; i< SessionInfos.size(); i++)
		{
			ULONG SessId = SessionInfos[i].sessionId;

			// mark the unfound old session as expired
			while (itId < oldIds.end() && (*itId) < SessId)
			{
				expiredIds.push_back(*itId);
#ifdef RESOURCE_MAN_TEST
				expiredPort.insert(std::make_pair<ULONG,ULONG>(*itId,oldPort[*itId]));
#endif
				itId++;
			}

			if (itId < oldIds.end() && *itId == SessId)
				itId++;

			// update the local session information
			SessionInfoMap::iterator sit = _sessInfos.find(SessId);
			if (sit == _sessInfos.end())
			{
				glog(ZQ::common::Log::L_DEBUG, LOGFMT("VstrmSessMon::run() new session found: %d"), SessId);
				
				ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
				for (Subscriber_v::iterator it =_subscribers.begin(); it < _subscribers.end(); it++)
				{
					if (NULL != (*it))
					{
						try
						{
							(*it)->OnVstrmSessDetected(&SessionInfos[i]);
						}
						catch (...) 
						{
						}
					}
				}
#ifdef RESOURCE_MAN_TEST
				_cls.PortUsageChanged(SessionInfos[i].destPortHandle,PORT_IN_USE);
#endif
				_sessInfos.insert( std::make_pair<ULONG, VOD_SESSION_INFORMATION >(SessId, SessionInfos[i]));
			}
			else if (SessionInfos[i].currentState != sit->second.currentState)
			{
				glog(ZQ::common::Log::L_DEBUG, LOGFMT("VstrmSessMon::run() session(%d) state changed: %s(%d) => %s(%d)"), SessId,
					VodProviderStateCodeText[sit->second.currentState], sit->second.currentState,
					VodProviderStateCodeText[SessionInfos[i].currentState], SessionInfos[i].currentState);
				
				ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
				for (Subscriber_v::iterator it =_subscribers.begin(); it < _subscribers.end(); it++)
				{
					if (NULL != (*it))						
					{
						try
						{
							(*it)->OnVstrmSessStateChanged(&SessionInfos[i], sit->second.currentState);
						}
						catch (...) 
						{
						}
					}

					if (VOD_STATE(DONE) == sit->second.currentState)
						expiredIds.push_back(sit->first);
				}
			}
			else if (memcmp(&SessionInfos[i].currentSpeed, &sit->second.currentSpeed, sizeof(SessionInfos[i].currentSpeed)) !=0)
			{
				glog(ZQ::common::Log::L_DEBUG, LOGFMT("VstrmSessMon::run() session(%d) speed changed: %u/%u => %u/%u"), SessId,
					sit->second.currentSpeed.numerator, sit->second.currentSpeed.denominator,
					SessionInfos[i].currentSpeed.numerator, SessionInfos[i].currentSpeed.denominator);
				
				ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
				for (Subscriber_v::iterator it =_subscribers.begin(); it < _subscribers.end(); it++)
				{
					if (NULL != (*it))
					{
						try
						{
							(*it)->OnVstrmSessSpeedChanged(&SessionInfos[i], sit->second.currentSpeed);
						}
						catch (...) 
						{
						}
					}
				}
			}
			else if (SessionInfos[i].currentTimeOffset != sit->second.currentTimeOffset)
			{
				ZQ::common::MutexGuard gd(_subscribers_locker,__MSGLOC__);
				for (Subscriber_v::iterator it =_subscribers.begin(); it < _subscribers.end(); it++)
				{
					if (NULL != (*it))
					{
						try
						{						
							(*it)->OnVstrmSessProgress(&SessionInfos[i], sit->second.currentTimeOffset);
						}
						catch (...) 
						{
						}
					}
				}
			}

			_sessInfos[SessId] = SessionInfos[i];

		} // for loop

		// trust the remaining oldIds as expired if no session info was found
		for (; itId < oldIds.end(); itId++)
		{
			expiredIds.push_back(*itId);
#ifdef RESOURCE_MAN_TEST
			expiredPort.insert(std::make_pair<ULONG,ULONG>(*itId,oldPort[*itId]));
#endif
		}

		if (expiredIds.size()>0)
		{
			char buf[2048]="", *p= buf;
			sprintf(p, "VstrmSessMon::run() %d/%d expired: ", expiredIds.size(), _sessInfos.size()); p+=strlen(p);

			for (itId = expiredIds.begin(); itId < expiredIds.end(); itId ++)
			{
			    if ( p-buf <2000)
				{
					sprintf(p, "sess(%d), ", *itId); ; p+=strlen(p);
				}
#ifdef RESOURCE_MAN_TEST
				_cls.PortUsageChanged(expiredPort[*itId],PORT_NORMAL);
#endif
				_sessInfos.erase(*itId);
			}
		    glog(ZQ::common::Log::L_DEBUG, LOGFMT("%s"), buf);
		}
		
		lStart=GetTickCount()-lStart;
		lMeanTime+=lStart;
		lTest++;
		if(lTest>1000)
		{
			lTest=0;
			glog(Log::L_DEBUG,"Scan mean time=%d",lMeanTime/1000);
			lMeanTime=0;
		}
		if(33-lStart>0)
		{
			Sleep(33-lStart); // for ntsc 30 fps: 1000/30
		}
		else
		{
			glog(Log::L_DEBUG,"vstream sesssion scan ,time = (%d)",lStart);
		}
	}
	SetEvent(_handleQuitted);
	CloseHandle(_handleQuitted);
	return 0;
}
#endif//_USE_NEW_SESSION_MON


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

#ifdef TRACE_QUERY
VstrmClass gcls;
#endif
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
long VstrmSessMon::ReadOutputPortCount()
{
	ULONG	outputCount=0;
	if (0 == _checkDelayCount++ )
	{
		if(!ReadRegDWORDData("SYSTEM\\CurrentControlSet\\Services\\VodDrv", "OutputPortCount", &outputCount) || outputCount <= 0 )
		{
			TCHAR szBuffer[1024]	;
			DWORD dwLastError;
			dwLastError = GetSystemErrorText(szBuffer, sizeof(szBuffer));
			//printf("SCP-E-REGVALUE, Error accessing OutputPortCount registry value - %s (0x%08x)\n", szBuffer, dwLastError);
			glog(ZQ::common::Log::L_ERROR,"SCP-E-REGVALUE, Error accessing OutputPortCount registry value - %s (0x%08x)", szBuffer, dwLastError);
			return -1;
		}
		else
		{
			glog(ZQ::common::Log::L_INFO , "Get Output count from SYSTEM\\CurrentControlSet\\Services\\VodDrv [%d]" , outputCount );
			if(outputCount!=_lastOutputPortCount)
			{//re-allocate memory
				//Clear last allocation first
				if(_pSessionInfoParam)
				{
					for(ULONG i=0;i<_bufferCount;i++)
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
				if(m_pNewsessInfoArray)
				{
					free(m_pNewsessInfoArray);
				}
				if(m_pOldSessionArray)
				{
					free(m_pOldSessionArray);
				}
				if(m_pExpiredIDArray)
				{
					free(m_pExpiredIDArray);
				}
				if(m_pExpiredVStrmPort[0])
				{
					free(m_pExpiredVStrmPort[0]);					
				}
				if(m_pExpiredVStrmPort[1])
				{
					free(m_pExpiredVStrmPort[1]);
				}
#ifdef _ICE_INTERFACE_SUPPORT
				if(m_pVstrmPortArrWhenStartup)
				{
					free(m_pVstrmPortArrWhenStartup);
				}
#endif
				_maxSessionCount=outputCount*3;
				_bufferCount	= (_maxSessionCount / VOD_IOCTL_SESS_PER_INFO_BUF) + 1;
			
				_pSessionInfoParam=(PVOD_DRIVER_QUERY_NEXT_SESSION_PARMS*)malloc(sizeof(PVOD_DRIVER_QUERY_NEXT_SESSION_PARMS)*_bufferCount);
				if(!_pSessionInfoParam)
				{
					glog(Log::L_CRIT,"VstrmSessMon::ReadOutputPortCount() Not enough memory");
					return -1;
				}
				for(int i=0;i<_bufferCount;i++)
				{
					_pSessionInfoParam[i]=(PVOD_DRIVER_QUERY_NEXT_SESSION_PARMS)malloc(VOD_IOCTL_SESS_INFO_BUF_SIZE);
					if(!_pSessionInfoParam[i])
					{
						glog(Log::L_CRIT,"VstrmSessMon::ReadOutputPortCount()##Not enough memory");
						return -1;
					}
				}
				_pSessState=(PVOD_SESSION_INFORMATION*)malloc(_maxSessionCount*sizeof(PVOD_SESSION_INFORMATION));
				m_pNewsessInfoArray=(VOD_SESSION_INFORMATION*)malloc(_maxSessionCount*sizeof(VOD_SESSION_INFORMATION));
				m_pOldSessionArray=(VOD_SESSION_INFORMATION*)malloc(_maxSessionCount*sizeof(VOD_SESSION_INFORMATION));
				m_pExpiredIDArray=(ULONG*)malloc(_maxSessionCount*sizeof(ULONG));
				m_pExpiredVStrmPort[0]=(ULONG*)malloc(_maxSessionCount*sizeof(ULONG));
				m_pExpiredVStrmPort[1]=(ULONG*)malloc(_maxSessionCount*sizeof(ULONG));
#ifdef _ICE_INTERFACE_SUPPORT
				m_pVstrmPortArrWhenStartup=(ULONG*)malloc(_maxSessionCount*sizeof(ULONG));
#endif				
				_lastOutputPortCount = outputCount;
			}
		}
	}
	_checkDelayCount %=DELAY_QUERY_COUNT;
	return _lastOutputPortCount;
}

#if _USE_NEW_SESSION_MON

VSTATUS VstrmSessMon::querySessionInfo(VOD_SESSION_INFORMATION* sessionInfoArray , bool ExceptInitSessions/* =false */)
{
	PVOD_SESSION_INFORMATION			 sessionInfo;
	HANDLE		vodHandle			= INVALID_HANDLE_VALUE;
	DWORD		amountRead;
	
	
	ULONG		streamCount			= 0;
	ULONG		streamPausedCount	= 0;
	ULONG		i, j;
    DWORD		dwLastError;
	TCHAR		szBuffer[256];
	BOOLEAN		bFailure			= FALSE;

	
	if(ReadOutputPortCount()<0)
	{
		glog(Log::L_CRIT,"Can't get output port information");
		return VSTRM_SEVERITY_ERROR;
	}
	long curStreamCount = 0;
	m_NewSessionCount=0;
	//redirect sessionInfoArray to m_pNewsessInfoArray because m_pNewsessInfoArray maybe reposition
	sessionInfoArray = m_pNewsessInfoArray;
	memset(sessionInfoArray,0,sizeof(VOD_SESSION_INFORMATION)*_maxSessionCount);
	//char* driverName[]={VOD_NAME,ON2_RTP_CONSUMER_NAME };
	char* driverName[]={VOD_NAME};
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
				dwLastError = GetSystemErrorText(szBuffer, sizeof(szBuffer));
				glog(ZQ::common::Log::L_CRIT,"SCP-E-OPENVOD, Failed to access %s device driver - %s (0x%08x)",
					driverName[iDriver],szBuffer, dwLastError);
				m_NewSessionCount=0;
				break;
			}
			
			// Allocate buffers to accommodate the maximum sessions possible ("streaming" and "init")
			
			// Allocate session info pointer array for sorting		
			
			for (i=0; i<(ULONG)_bufferCount; i++)
			{
				// Specify desired session states
				_pSessionInfoParam[i]->h.sessionStates = (ExceptInitSessions ? VOD_STATE_ALL_EXCEPT_INIT:VOD_STATE_ALL);
				
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
				for (j = 0; j < _pSessionInfoParam[i]->h.sessionCount; j++)
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
				m_NewSessionCount=streamCount;
				BOOLEAN		bHeader				= TRUE;
				for (i = 0; i < streamCount; i++)
				{
					//SessionInfos.push_back(*_pSessState[i]);
					memcpy(&sessionInfoArray[curStreamCount+i],_pSessState[i],sizeof(VOD_SESSION_INFORMATION));
					//glog(Log::L_DEBUG,"QueryInfo::Session iD=%ld and StreamCount=%d",_pSessState[i]->sessionId,streamCount);
					//				if (_pSessState[i]->currentState == VOD_STATE(PAUSED))
					//				{
					//					streamPausedCount++;
					//				}
				}
				curStreamCount =streamCount;
			}
			else
			{
				//m_NewSessionCount=0;
			}
			if (vodHandle != INVALID_HANDLE_VALUE)	CloseHandle(vodHandle);
		}

#ifdef TRACE_QUERY
		glog(ZQ::common::Log::L_DEBUG, LOGFMT("\n %d streams (%d paused)\n\n"), streamCount, streamPausedCount);
#endif // TRACE_QUERY

	} while (FALSE);

	
	return VSTRM_SUCCESS;
}
#else 
VSTATUS VstrmSessMon::querySessionInfo(SessionInfo_v& SessionInfos, bool ExceptInitSessions)
{
	PVOD_SESSION_INFORMATION			 sessionInfo;
	HANDLE		vodHandle			= INVALID_HANDLE_VALUE;
	DWORD		amountRead;
	
	
	ULONG		streamCount			= 0;
	ULONG		streamPausedCount	= 0;
	ULONG		i, j;
    DWORD		dwLastError;
	TCHAR		szBuffer[256];
	BOOLEAN		bFailure			= FALSE;

	
	if(ReadOutputPortCount()<0)
	{
		glog(Log::L_ERROR,"Can't get output port information");
		return VSTRM_SEVERITY_ERROR;
	}
	/*memset(SessionInfoArray,0,sizeof(VOD_SESSION_INFORMATION)*_maxSessionCount);*/
	SessionInfos.clear();
	do
	{
		// Establish a channel to the driver
		vodHandle = CreateFileA (VOD_NAME,
								GENERIC_WRITE | GENERIC_READ,
								0,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);

		if (vodHandle == INVALID_HANDLE_VALUE)
		{
			dwLastError = GetSystemErrorText(szBuffer, sizeof(szBuffer));
			printf("SCP-E-OPENVOD, Failed to access Vod device driver - %s (0x%08x)\n", szBuffer, dwLastError);
			break;
		}

		// Allocate buffers to accommodate the maximum sessions possible ("streaming" and "init")

		// Allocate session info pointer array for sorting		

		for (i=0; i<_bufferCount; i++)
		{
			// Specify desired session states
			_pSessionInfoParam[i]->h.sessionStates = (ExceptInitSessions ? VOD_STATE_ALL_EXCEPT_INIT:VOD_STATE_ALL);

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
			
			// Add session info pointers to the session array
			for (j = 0; j < _pSessionInfoParam[i]->h.sessionCount; j++)
			{
				_pSessState[streamCount++] = sessionInfo;
				sessionInfo++;
			}
			
			// Test our limits
			if ((_pSessionInfoParam[i]->h.sessionCount < VOD_IOCTL_SESS_PER_INFO_BUF) ||	// buffer not filled
				(streamCount >= _bufferCount))											// max sessions reached
			{
				break;
			}
		}
		
		// Any sessions?
		if (streamCount > 0)
		{
			// Sort the session array
			m_NewSessionCount=streamCount;
			BOOLEAN		bHeader				= TRUE;
			for (i = 0; i < streamCount; i++)
			{
				SessionInfos.push_back(*_pSessState[i]);
				//memcpy(SessionInfoArray+i,_pSessState[i],sizeof(VOD_SESSION_INFORMATION));
				if (_pSessState[i]->currentState == VOD_STATE(PAUSED))
				{
					streamPausedCount++;
				}
			}
		}

#ifdef TRACE_QUERY
		glog(ZQ::common::Log::L_DEBUG, LOGFMT("\n %d streams (%d paused)\n\n", streamCount, streamPausedCount);
#endif // TRACE_QUERY

	} while (FALSE);

	if (vodHandle != INVALID_HANDLE_VALUE)	CloseHandle(vodHandle);

	return VSTRM_SUCCESS;
}
#endif //_USE_NEW_SESSION_MON

VSTATUS VstrmSessMon::querySessionIndex(SessionIdx_v& SessionIdxs)
{
	PVOD_DRIVER_QUERY_NEXT_INDEX_PARMS	indexInfoParms[10]	= { NULL };
	PVOD_INDEX_INFORMATION*		pIndex				= NULL;
	PVOD_INDEX_INFORMATION		indexInfo;
	HANDLE		vodHandle			= INVALID_HANDLE_VALUE;
	DWORD		amountRead;
	ULONG		outputPortCount		= 0;
	ULONG		maxIndices			= 0;
	ULONG		bufferCount			= 0;
	ULONG		indexCount			= 0;
	ULONG		i, j;
    DWORD		dwLastError;
	TCHAR		szBuffer[256];
	BOOLEAN		bFailure			= FALSE;

	SessionIdxs.clear();

	do
	{
		// Get our output port count registry value
		if (!ReadRegDWORDData("SYSTEM\\CurrentControlSet\\Services\\VodDrv", "OutputPortCount", &outputPortCount))
		{
			dwLastError = GetSystemErrorText(szBuffer, sizeof(szBuffer));
			printf("SCP-E-REGVALUE, Error accessing OutputPortCount registry value - %s (0x%08x)\n", szBuffer, dwLastError);
//			glog(ZQ::common::Log::L_ERROR,"SCP-E-REGVALUE, Error accessing OutputPortCount registry value - %s (0x%08x)", szBuffer, dwLastError);
			break;
		}

		// Establish a channel to the driver
		vodHandle = CreateFileA (VOD_NAME,
								GENERIC_WRITE | GENERIC_READ,
								0,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);

		if (vodHandle == INVALID_HANDLE_VALUE)
		{
			dwLastError = GetSystemErrorText(szBuffer, sizeof(szBuffer));
			printf("SCP-E-OPENVOD, Failed to access Vod device driver - %s (0x%08x)\n", szBuffer, dwLastError);
			break;
		}

		// Allocate buffers to accommodate the maximum indices possible (all different "streaming" and "init")
		maxIndices	= outputPortCount*2;
		bufferCount	= (maxIndices/VOD_IOCTL_INDEX_PER_INFO_BUF) + 1;

		for (i=0; i<bufferCount; i++)
		{
			indexInfoParms[i] = (PVOD_DRIVER_QUERY_NEXT_INDEX_PARMS) malloc (VOD_IOCTL_INDEX_INFO_BUF_SIZE);
			if (indexInfoParms[i] == NULL)
			{
				bFailure = TRUE;
				break;
			}
		}
		if (bFailure)
		{
			printf("SCP-E-MEMALLOC, Error allocating an index info parameter block\n");
			break;
		}

		// Allocate index info pointer array for sorting
		pIndex = (PVOD_INDEX_INFORMATION*)malloc ( maxIndices * sizeof(PVOD_INDEX_INFORMATION));
		if (pIndex == NULL)
		{
			printf("SCP-E-MEMALLOC, Error allocating an index info pointer array\n");
			break;
		}
		
		for (i=0; i<bufferCount; i++)
		{
			// Maintain context
			if (i==0)
				indexInfoParms[i]->h.indexContext = 0;	// 1st time
			else
				indexInfoParms[i]->h.indexContext = indexInfoParms[i-1]->h.indexContext +
												  indexInfoParms[i-1]->h.indexCount;

			// Query for next buffer of index information
			if (!DeviceIoControl (vodHandle,
				(DWORD) IOCTL_VOD_DRIVER_QUERY_NEXT_INDEX, 
				indexInfoParms[i],
				VOD_IOCTL_INDEX_INFO_BUF_SIZE, 
				indexInfoParms[i],
				VOD_IOCTL_INDEX_INFO_BUF_SIZE, 
				&amountRead,
				NULL))
				break;
			// Locate 1st index
			indexInfo = &indexInfoParms[i]->indexInformation;
			
			// Add index info pointers to the index array
			for (j = 0; j < indexInfoParms[i]->h.indexCount; j++)
			{
				pIndex[indexCount++] = indexInfo;
				indexInfo++;
			}
			
			// Test our limits
			if ((indexInfoParms[i]->h.indexCount < VOD_IOCTL_INDEX_PER_INFO_BUF) ||	// buffer not filled
				(indexCount >= maxIndices))											// max indices reached
			{
				break;
			}
		}
		
		// Any indices?
		if (indexCount > 0)
		{
			UQUADWORD	userTotal			= 0;
			UQUADWORD	trickCountTotal		= 0;
			UQUADWORD	tricksLoadedTotal	= 0;
			UQUADWORD	spliceCountTotal	= 0;
			UQUADWORD	headerMemoryTotal	= 0;
			UQUADWORD	indexMemoryTotal	= 0;

			// Sort the index array
			for (i = 0; i < indexCount; i++)
			{
				indexInfo = pIndex[i];

				SessionIdxs.push_back(*indexInfo);
				// Accumulate totals
				userTotal			+= indexInfo->refCount;
				headerMemoryTotal	+= indexInfo->headerLength;
				trickCountTotal		+= indexInfo->trickRecordCount;
				tricksLoadedTotal	+= indexInfo->trickRecordsLoaded;
				spliceCountTotal	+= indexInfo->spliceRecordCount;
				indexMemoryTotal	+= indexInfo->indexMemory;
			}

#ifndef TRACE_QUERY
		}
#else
			// Totals
			printf ("\n\n");
			printf (" Total Users:          %24u \n",   userTotal);
			printf (" Total Indices:        %24u \n\n", indexCount);
			printf (" Total Trick Record Count:     %16u\n", trickCountTotal);
			printf (" Total Trick Records Loaded:   %16u\n", tricksLoadedTotal);
			printf (" Total Splice Record Count:    %16u\n\n", spliceCountTotal);
			printf (" Total Header Memory:  %24u %12uBytes\n", headerMemoryTotal, headerMemoryTotal);
			printf (" Total Index Memory:   %24u %12u Bytes\n", indexMemoryTotal, indexMemoryTotal);
		}
		else
		{
			glog(ZQ::common::Log::L_DEBUG, LOGFMT("\n %d indices\n\n", indexCount);
		}
#endif // TRACE_QUERY
	} while (FALSE);

	// Release any allocated memory
	for (i=0; i<bufferCount; i++)
	{
		if (indexInfoParms[i])	free(indexInfoParms[i]);
	}
	if (pIndex)								free(pIndex);
	if (vodHandle != INVALID_HANDLE_VALUE)	CloseHandle(vodHandle);

	return VSTRM_SUCCESS;
}

}
}