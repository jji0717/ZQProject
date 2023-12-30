#ifndef __VodMonitor_H__
#define __VodMonitor_H__

#define	_USE_NEW_SESSION_MON 1

#include <ZQ_common_conf.h>
#include "VstrmClass.h"
#include <log.h>
#include <vector>

namespace ZQ{
namespace StreamSmith {
		


class VstrmSessSink 
{
friend class VstrmSessMon;
public:
	VstrmSessSink(VstrmSessMon& monitor);
	~VstrmSessSink();
public:

	void	UnSubScribleSink();
	void	SubScribleSink();
	
protected:
#if _USE_NEW_SESSION_MON
	virtual void OnVstrmSessDetected(const PVOD_SESSION_INFORMATION sessinfo, ZQ::common::Log& log , const int64& timeStamp );
	virtual void OnVstrmSessStateChanged(const PVOD_SESSION_INFORMATION sessinfo, const ULONG PreviousState,ZQ::common::Log& log ,  const int64& timeStamp );
	virtual void OnVstrmSessSpeedChanged(const PVOD_SESSION_INFORMATION sessinfo, const VVX_FILE_SPEED PreviousSpeed,ZQ::common::Log& log ,  const int64& timeStamp );
	virtual void OnVstrmSessProgress(const PVOD_SESSION_INFORMATION sessinfo, const TIME_OFFSET PreviousTimeOffset,ZQ::common::Log& log ,  const int64& timeStamp );
	virtual	void OnVstrmSessExpired(const ULONG sessionID,ZQ::common::Log& log ,  const int64& timeStamp );

	void		NotifyVstrmSessDetected(const PVOD_SESSION_INFORMATION sessinfo,ZQ::common::Log& log ,  const int64& timeStamp );
	void		NotifyVstrmSessStateChanged(const PVOD_SESSION_INFORMATION sessinfo, const ULONG PreviousState,ZQ::common::Log& log ,  const int64& timeStamp );
	void		NotifyVstrmSessSpeedChanged(const PVOD_SESSION_INFORMATION sessinfo, const VVX_FILE_SPEED PreviousSpeed,ZQ::common::Log& log ,  const int64& timeStamp );
	void		NotifyVstrmSessProgress(const PVOD_SESSION_INFORMATION sessinfo, const TIME_OFFSET PreviousTimeOffset,ZQ::common::Log& log ,  const int64& timeStamp );
	void		NotifyVstrmSessExpired(const ULONG sessionID,ZQ::common::Log& log ,  const int64& timeStamp );

	//for notify when service startup
#ifdef _ICE_INTERFACE_SUPPORT
	virtual void		VstrmSessNotifyWhenStartup(const PVOD_SESSION_INFORMATION sessinfo);
	virtual	void		vstrmSessNotifyOverWhenStartup();
#endif
#else
	virtual void OnVstrmSessDetected(const PVOD_SESSION_INFORMATION sessinfo);
	virtual void OnVstrmSessStateChanged(const PVOD_SESSION_INFORMATION sessinfo, const ULONG PreviousState);
	virtual void OnVstrmSessSpeedChanged(const PVOD_SESSION_INFORMATION sessinfo, const VVX_FILE_SPEED PreviousSpeed);
	virtual void OnVstrmSessProgress(const PVOD_SESSION_INFORMATION sessinfo, const TIME_OFFSET PreviousTimeOffset);
#endif
	
	
	
	VstrmSessMon& _monitor;
public:
	///register session id,so subscriber can monitor it
	void		RegisterSessID(ULONG sessID);
	void		UnRegisterSessID(ULONG sessID);
protected:
	std::vector<ULONG>		_SessIdStack;
	ZQ::common::Mutex		_listOpLocker;
	//ZQ::common::Mutex		_StackMutex;
	ULONG					_ProgressGranularity;

	volatile 	long				_nRef;

public:
	void addRef(){InterlockedIncrement(&_nRef);}
	void decRef()
	{
		if (_nRef>0)
			InterlockedDecrement(&_nRef);
	}
	long ref(){return _nRef;}
};

class VstrmSessMon : public ZQ::common::ThreadRequest
{
friend class VstrmSessSink;
public:
	VstrmSessMon(VstrmClass& cls);
	~VstrmSessMon() ;

	void stop() { _bQuit = true; }

	bool removeSessionInfo(ULONG sessId);
	const PVOD_SESSION_INFORMATION findSessionInfo(ULONG sessId);

	HANDLE			GetQuittedHandle(){return _handleQuitted;}
public:

	bool subscribe(VstrmSessSink& subscr);
	bool unsubscribe(VstrmSessSink& subscr);

	virtual bool init();
	virtual int  run();
//////////////////////////////////////////////////////////////////////////
	void		SetProgressGranularity(long Granularity);
	void		SetLogInstance(ZQ::common::Log* pLog)
	{
		m_pLog=pLog; 
	}

//////////////////////////////////////////////////////////////////////////
	
	typedef std::map<ULONG, VOD_SESSION_INFORMATION > SessionInfoMap;
	typedef std::map<ULONG, VOD_INDEX_INFORMATION > SessionIdxMap;
	typedef std::vector<VstrmSessSink* > Subscriber_v;

	SessionInfoMap						_sessInfos;
	SessionIdxMap						_sessIdx;
	
	Subscriber_v						_subscribers;
	ZQ::common::Mutex					_subscribers_locker;
	bool								_bQuit;
	HANDLE								_handleQuitted;
private:
	ULONG									_lastOutputPortCount;
	ULONG									_checkDelayCount;
	PVOD_DRIVER_QUERY_NEXT_SESSION_PARMS	*_pSessionInfoParam;
	PVOD_SESSION_INFORMATION				*_pSessState;
	long									_bufferCount;
	long									_maxSessionCount;
	
	//////////////////////////////////////////////////////////////////////////
	typedef struct _tagSessProperty 
	{
		VOD_SESSION_INFORMATION*			pSessInfo;
	}SessProperty;
	//////////////////////////////////////////////////////////////////////////

	long									m_NewSessionCount;	
	long									m_OldSessionCount;
	VOD_SESSION_INFORMATION					*m_pOldSessionArray;	
	VOD_SESSION_INFORMATION					*m_pNewsessInfoArray;
	ULONG									*m_pExpiredIDArray;
	ULONG									*m_pExpiredVStrmPort[2];//0 for Port 1 for sessid
	//////////////////////////////////////////////////////////////////////////
#ifdef _ICE_INTERFACE_SUPPORT
	ULONG									*m_pVstrmPortArrWhenStartup;
#endif	
public:
#ifdef _ICE_INTERFACE_SUPPORT
	void									ScanSessWhenStartup();	
#endif

private:
	void									DumpSessionArray(VOD_SESSION_INFORMATION* pSess,int iCount,const char* tips);
	long									ReadOutputPortCount();
	long									FindSessIDInSession(VOD_SESSION_INFORMATION* pSess,long sessCount,ULONG SessID,long oldCurIter);
	bool									m_bFoundNewSessionIDInArray;
private:
	VstrmClass&									_cls;
	ZQ::common::Log*							m_pLog;
	ULONG										m_ProgressGranularity;
public:

	typedef std::vector< VOD_SESSION_INFORMATION > SessionInfo_v;
	typedef std::vector< VOD_INDEX_INFORMATION > SessionIdx_v;
	
#if _USE_NEW_SESSION_MON
	VSTATUS querySessionInfo(VOD_SESSION_INFORMATION* sessionInfoArray, bool ExceptInitSessions=false);
#else//_USE_NEW_SESSION_MON
	/*static*/ VSTATUS querySessionInfo(SessionInfo_v& SessionInfos, bool ExceptInitSessions=false);
#endif//_USE_NEW_SESSION_MON
	
	static VSTATUS querySessionIndex(SessionIdx_v& SessionIdxs);
};
}
}
#endif // __VodMonitor_H__
