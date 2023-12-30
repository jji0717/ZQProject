
#ifndef _cdnss_c2streamer_environment_header_file_h__
#define _cdnss_c2streamer_environment_header_file_h__

#include <map>
#include <Log.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <NativeThreadPool.h>
#include "PortManager.h"
#include "C2SessionManager.h"

#define gettid() syscall(SYS_gettid)

namespace C2Streamer
{

struct SERVEPORTATTR
{
	int64			speed;
	uint16			natPortBase;
	uint16			natPortCount;
	std::string		natIp;
	SERVEPORTATTR()
	{
		speed = 0 ;
		natPortBase = 0;
		natPortCount = 0;
	}
};
typedef std::map<std::string ,  SERVEPORTATTR> PORTSPEEDCONF;
	
struct C2EnvConfig 
{
	C2EnvConfig()
	{
		minIdleInterval			= 200; // 200 ms
		ioBlockSize				= 4 * 1024;
		readBufferCount			= 1;
		serverNameTag			= "C2 Streamer";
		mSocketSendMonitorThreashold = 50;
		mTimerThreadHold		= 50;
		mFsReadThreshold		= 50;
		mPendingSizeThreshold	= 20;

		mCheckClientWhileTransfer = true;
		mMaxIdleCountAtFileEnd	= 0;
		mMaxSparseFileSearchRange = 4 * 1024 * 1024;
		mbQuitPerConnectionLost	= true;
		mSendPacketSize			= 1500;
		mPacketsPerSend			= 10;
		maxRoundPerRun			= 3;
		minYieldInterval		= 5;
		mSocketKernelSndBuf		= 1024 * 1024;
		mSocketKernelSndTimeo	= 20 * 1000;
		mbUseTcpCork			= true;
		mFirstChunkWait			= 150;
		mFsIoCommitThreshold	= 3;//10ms
		mFsDataWaitCount		= 300;

		mKeyFile5I				= "";
		//mAquaRootUrl			= "http://demo5:demo5@10.50.16.80:8080/aqua/rest/cdmi/";
		mAquaRootUrl			= "";
		mHomeContainer			= "";
		mLogFlag				= 0;
		mDefaultBitrate			= 2048000;

		mUseBufferIo			= false;
		mCacheDefaultBufferCount = 100;
		mCacheDeltaBufferCount  = 50;
		mCacheBufferSize        = 2 * 1024 * 1024;
		mCacheReadaheadCount	= 1;

		mIndexRecordCacheSize	= 4096;
		mDefaultTransferBitrate	= 3750 *1000;


		//for C2Client
		clientType = 0;
		C2ClientUpStreamIP = "";
		C2ClientURL = "";
		C2ClientTransfer = "";
		C2ClientHttpCRGAddr = "";
		C2ClientTransferRate = "3750000";
		C2ClientIngressCapacity = "16512000000";
		C2ClientExclusionList = "";
		C2ClientTransferDelay = "-2000";
		C2ClientHttpCRGPort = 10080;
		C2ClientDefaultGetPort = 12000;
		C2ClientWaitBufferTime = 10000;
		C2ClientIndexTimeout	= 5*1000;
		C2ClientIndexRetryTimes = 5;
		C2ClientMainfileTimeout = 15*1000;
		C2ClientMainfileRetryTimes = 5;
		C2ClientEnableTransferDelete = 0;
		C2ClientAlignment = 8;


		//for cache buffer remove 
		defaultTimeOut = 24 * 3600 * 1000;
		fioErrTimeOut = 10 * 60 *1000;
		httpErrTimeOut = 1 * 60 * 1000;
		socketErrTimeOut = 2 * 1000;
		otherErrTimeOut = 5 * 1000;
		
		//for httpclient
		httpProxyURL = "";
		segmentURL = "";
		httpTimeOut = 10000;
		httpRetry = 0;

		attributesTimeoutInPwe = 3600 * 1000;//100ms


		aquaReaderFlags = 0x0f;
	}
	uint32			mFsDataWaitCount;
	int32			maxRoundPerRun;//
	int32			minYieldInterval;
	int32			minIdleInterval;
	uint32			ioBlockSize;
	uint32			readBufferCount;

	int32            clientType;
	std::string		serverNameTag;
	PORTSPEEDCONF	portSpeed;
	uint32			mSocketSendMonitorThreashold;
	uint32			mTimerThreadHold;
	uint32			mFsReadThreshold;
	uint32			mFsIoCommitThreshold;
	uint32			mPendingSizeThreshold;
	bool			mCheckClientWhileTransfer;
	int32			mMaxIdleCountAtFileEnd;
	uint32			mMaxSparseFileSearchRange;
	bool			mbQuitPerConnectionLost;
	uint32			mSendPacketSize;
	uint32			mPacketsPerSend;
	int32			mSocketKernelSndBuf;
	int32			mSocketKernelSndTimeo;
	bool			mbUseTcpCork;
	int32			mFirstChunkWait;
	std::string		mLocalBindPort;	


	///HLS part
	std::string		mKeyFile5I;
	uint32			mKeyId;
	std::string		mAquaRootUrl;
	std::string		mHomeContainer;
	std::string		mServerHostUrl;
	uint32			mLogFlag;
	uint32			mDefaultBitrate;
	std::map<std::string,uint32> mName2Bitrate;

	//File Cache part
	bool			mUseBufferIo;
	uint32			mCacheDefaultBufferCount;
	uint32			mCacheDeltaBufferCount;
	uint32			mCacheBufferSize;
	uint32			mCacheReadaheadCount;

	uint32			mIndexRecordCacheSize;

	uint32			mDefaultTransferBitrate;

	//for c2client
	std::string              C2ClientUpStreamIP;
	std::string              C2ClientURL;
	std::string              C2ClientTransfer;
	std::string              C2ClientHttpCRGAddr;
	std::string 			 C2ClientTransferRate;
	std::string 			 C2ClientIngressCapacity;
	std::string  			 C2ClientExclusionList;
	std::string  			 C2ClientTransferDelay;
	int32                    C2ClientHttpCRGPort;
	int32                    C2ClientDefaultGetPort;
    int32                    C2ClientWaitBufferTime;
    int32					 C2ClientIndexTimeout;
	int32					 C2ClientIndexRetryTimes;
	int32					 C2ClientMainfileTimeout;
	int32					 C2ClientMainfileRetryTimes;
	int32					 C2ClientEnableTransferDelete;
	int32 					 C2ClientAlignment;

	//for cache buffer remove
	int64                    defaultTimeOut;
	int64                    fioErrTimeOut;
	int64                    httpErrTimeOut;
	int64                    socketErrTimeOut;
	int64                    otherErrTimeOut;
	// for httpclient
	std::string            httpProxyURL;
	std::string            segmentURL;
	int32                   httpTimeOut;
	int32                  httpRetry;

	///attributes timeout in pwe
	int32					attributesTimeoutInPwe;


	//for AquaReader
	std::string				aquaReaderRootUrl;
	std::string				aquaReaderUserDomain;
	std::string				aquaReaderHomeContainer;
	int32 					aquaReaderFlags;
};

class IAttrBridge
{
public:
	virtual ~IAttrBridge(){}
	virtual	bool	isFileBeingWritten( const std::string& filename ,const std::string& sessionId ) = 0;

	///return true if the range is available, false if not
	virtual bool	getFileDataRange( const std::string& filename , const std::string& sessionId , int64& startByte , int64& endByte ) = 0;
};


class C2StreamerEnv
{
public:
	
	C2StreamerEnv( );	
	virtual ~C2StreamerEnv();	

	inline ZQ::common::Log*					getLogger()
	{
		return mLogger;
	}

	inline ZQ::common::NativeThreadPool&	getThreadPool( )
	{
		return *mThreadPool;
	}
	
	inline const std::string& 				getDocumentRootFolder() const
	{
		return mDocRootFolder;
	}

	std::string								getRequestSequence( );

	std::string								generateSessionId( ) const;

	inline const C2EnvConfig&				getConfig() const
	{
		return mConfig;
	}

	inline IAttrBridge*						getAttrBridge()
	{
		//assert( mAttrBridge != NULL );
		return mAttrBridge;
	}
	int64									getUpTime( ) const;
	bool                    setLatencyMap(std::string& fileName, int64 startOffset, int64 time);
	int                     getLatencyMap(std::string& fileName, int64 startOffset, int64 time);
public:
	ZQ::common::Log*				mLogger;
	ZQ::common::NativeThreadPool*	mThreadPool;
	std::string						mDocRootFolder;	
	C2EnvConfig						mConfig;
	IAttrBridge*					mAttrBridge;
	int64							mServiceStartTime;
private:

	ZQ::common::Mutex				mEnvMutex;
	int64							miSeqBase;
	ZQ::common::Mutex                               mLatencyMapMutex;
	ZQ::common::LRUMap<std::string, int64>          mLatencyMap;
};

#define MLOG	(*mEnv.getLogger())

extern C2StreamerEnv*	getEnvironment( );

#define TRANSFERSESSION_PREFIX "scs/sessiontransfer/"

std::string constructResponseSessId( const std::string& sessId);

std::string getSessionIdFromCompoundString( const std::string& str );

std::string dumpStringVector( const std::vector<std::string>& value , const std::string& delimiter =" " );




std::string timeToString( uint64 timeTicket );

}//namespace C2Streamer
#endif//_cdnss_c2streamer_environment_header_file_h__
