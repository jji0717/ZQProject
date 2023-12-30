
#ifndef _ZQ_STREAMSMITH_VSTRM_SESSION_MONITOR_HEADER_FILE_H__
#define _ZQ_STREAMSMITH_VSTRM_SESSION_MONITOR_HEADER_FILE_H__

#include <ZQ_common_conf.h>
#include "VstrmClass.h"
#include <log.h>
#include <vector>


namespace ZQ{
namespace StreamSmith {


typedef struct _VstrmSessInfo 
{
	ULONG				sessionId;
	ULONG				MuxRate;
	ULONG				State;//Session State
	SPEED_IND			Speed;
	TIME_OFFSET			PlayoutTimeOffset;
	int64				PlayoutByteOffset;
	int64				ByteOffset;
	int64				ObjectSize;
	ULONG				DestPortHandle;
	bool				noOldInfomation;
	_VstrmSessInfo()
	{
		memset( this, 0, sizeof(_VstrmSessInfo) );
	}
}VstrmSessInfo;

#define _USE_NEW_SESSION_MON 1

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
	virtual void OnVstrmSessDetected( const VstrmSessInfo& sessionInfo,
										ZQ::common::Log& log , 
										const int64& timeStamp );
	virtual void OnVstrmSessStateChanged( const VstrmSessInfo& sessionInfo, 
										ULONG curState,
										ULONG PreviousState,
										ZQ::common::Log& log ,  
										const int64& timeStamp );
	virtual void OnVstrmSessSpeedChanged( const VstrmSessInfo& sessionInfo, 
										SPEED_IND curSpeed,
										SPEED_IND PreviousSpeed,
										ZQ::common::Log& log ,
										const int64& timeStamp );
	virtual void OnVstrmSessProgress( const VstrmSessInfo& sessionInfo, 
										const TIME_OFFSET curTimeOffset,
										const TIME_OFFSET PreviousTimeOffset,
										ZQ::common::Log& log ,
										const int64& timeStamp );
	virtual	void OnVstrmSessExpired( const ULONG sessionID,
										ZQ::common::Log& log ,  
										const int64& timeStamp ,
										const std::string& reason = "",
										int errorCode = 0 );

	void		NotifyVstrmSessDetected( const VstrmSessInfo& sessionInfo,
										ZQ::common::Log& log ,  
										const int64& timeStamp );
	void		NotifyVstrmSessStateChanged( const VstrmSessInfo& sessionInfo, 
										const ULONG curState,
										const ULONG PreviousState,
										ZQ::common::Log& log ,  
										const int64& timeStamp );
	void		NotifyVstrmSessSpeedChanged( const VstrmSessInfo& sessionInfo,
										const SPEED_IND curSpeed,
										const SPEED_IND PreviousSpeed,
										ZQ::common::Log& log ,  
										const int64& timeStamp );
	void		NotifyVstrmSessProgress( const VstrmSessInfo& sessionInfo, 
										const TIME_OFFSET curTimeOffset,
										const TIME_OFFSET PreviousTimeOffset,
										ZQ::common::Log& log ,  
										const int64& timeStamp );
	void		NotifyVstrmSessExpired( const ULONG sessionID,
										ZQ::common::Log& log ,
										const int64& timeStamp );

	//for notify when service startup
#ifdef _ICE_INTERFACE_SUPPORT
	
	virtual void		VstrmSessNotifyWhenStartup(const VstrmSessInfo& sessinfo);

	virtual	void		vstrmSessNotifyOverWhenStartup();

#endif


	VstrmSessMon&		_monitor;
public:
	///register session id,so subscriber can monitor it
	void				RegisterSessID( ULONG sessID );
	void				UnRegisterSessID( ULONG sessID );

protected:

	std::vector<ULONG>		_SessIdStack;
	ZQ::common::Mutex		_listOpLocker;
	//ZQ::common::Mutex		_StackMutex;
	ULONG					_ProgressGranularity;
	volatile 	long		_nRef;

public:
	void addRef(){InterlockedIncrement(&_nRef);}
	void decRef()
	{
		if (_nRef>0)
			InterlockedDecrement(&_nRef);
	}
	long ref(){return _nRef;}
};

class VstrmSessMon : public ZQ::common::NativeThread
{
	friend class VstrmSessSink;
public:
	VstrmSessMon( VstrmClass& cls );
	~VstrmSessMon( ) ;

	///stop session monitor thread
	void		stop( );

	bool		removeSessionInfo(ULONG sessId);

	const		ESESSION_CHARACTERISTICS* findSessionInfo( ULONG sessId );

	HANDLE		GetQuittedHandle(){	return _handleQuitted;}

	
public:

	bool subscribe( VstrmSessSink& subscr );
	bool unsubscribe( VstrmSessSink& subscr );

	virtual bool init( );
	virtual int  run( );
	virtual void final(int retcode /* =0 */, bool bCancelled /* =false */);
	
	//////////////////////////////////////////////////////////////////////////
	void		SetProgressGranularity(long Granularity)
	{
		m_ProgressGranularity = Granularity;
	}
	void		SetLogInstance(ZQ::common::Log* pLog)
	{
		m_pLog=pLog; 
	}

	//////////////////////////////////////////////////////////////////////////
protected:

	bool		checkVstrmPortCount( );
	void		querySession( bool bStart = false );
	void		queryFromDeviceIo( );
	void		queryFromVstrmScan( );
	void		queryFromVstrmEvent( );
	void		onVodUserEvent(const VOD_USER_EVENT& et );
protected:

//	typedef std::vector<ESESSION_CHARACTERISTICS>	SessionInfoSet;
	
	typedef std::vector<VstrmSessInfo>				SessionInfoSet;
	SessionInfoSet									oldSessionInfo;
	SessionInfoSet									newSessionInfo;
	SessionInfoSet									expiredSessionInfo;


	typedef std::vector<VstrmSessSink* >			Subscriber_v;
	Subscriber_v									_subscribers;
	Subscriber_v									_subCopy;
	ZQ::common::Mutex								_subscribers_locker;
	
	bool											_bQuit;
	HANDLE											_handleQuitted;

	ULONG											_lastOutputCount;

	ULONG											_lastOutputPortCount;
	ULONG											_checkDelayCount;
	PVOD_DRIVER_QUERY_NEXT_SESSION_PARMS			*_pSessionInfoParam;
	PVOD_SESSION_INFORMATION						*_pSessState;
	long											_bufferCount;
	long											_maxSessionCount;

	friend static VSTATUS vstrmFOR_EACH_SESSION_CB( HANDLE, PVOID,ESESSION_CHARACTERISTICS*,ULONG,ULONG,ULONG);
	friend static VSTATUS vtrsmCount_CB( HANDLE, PVOID, ULONG);
	static VSTATUS vstrmFOR_EACH_SESSION_CB( HANDLE, PVOID,ESESSION_CHARACTERISTICS*,ULONG,ULONG,ULONG);
	static VSTATUS vtrsmCount_CB( HANDLE, PVOID, ULONG);
public:

#ifdef _ICE_INTERFACE_SUPPORT
	void									ScanSessWhenStartup();	
#endif

	static	void							convertSessCharToVstrmInfo( const ESESSION_CHARACTERISTICS* sessinfo ,VstrmSessInfo& info );
	static	void							convertDeviceIoCharToVstrmInfo( const VOD_SESSION_INFORMATION* sessInfo , VstrmSessInfo & info );

private:
	VstrmClass&								_cls;
	ZQ::common::Log*						m_pLog;
	ULONG									m_ProgressGranularity;

};

}}//namespace ZQ::StreamSmith


#endif //_ZQ_STREAMSMITH_VSTRM_SESSION_MONITOR_HEADER_FILE_H__
