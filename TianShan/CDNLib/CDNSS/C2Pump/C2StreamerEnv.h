
#ifndef _cdnss_c2streamer_environment_header_file_h__
#define _cdnss_c2streamer_environment_header_file_h__

#include <assert.h>
#include <map>
#include <Log.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <NativeThreadPool.h>

//#include "PortManager.h"
//#include "C2SessionManager.h"

#define gettid() syscall(SYS_gettid)

namespace C2Streamer
{

struct NicAttr
{
	int64			speed;
	uint16			natPortBase;
	uint16			natPortCount;
	std::string		natIp;
	NicAttr()
	{
		speed = 0 ;
		natPortBase = 0;
		natPortCount = 0;
	}
};
typedef std::map<std::string ,  NicAttr> NicsConf;

struct DiskCacheCon
{
	std::string      homePath;
	int32            totalSize; //size of this dir can cached, MB
	int32            LRUSize;
	int32            readThreadCount; // count of read thread
	int32            writeThreadCount; // cout of write thread
	int32            pendingsYield;  // pendings
	int32			 maxWriteMBps;
	DiskCacheCon()
	{
		homePath = "";
		totalSize = 0;
		readThreadCount = 2;
		writeThreadCount = 2;
		pendingsYield    = 2;
		LRUSize     = 20;
		maxWriteMBps = 20;
	}

};
typedef std::vector<DiskCacheCon> DISKCACHECON;

struct C2EnvConfig_Old
{
	C2EnvConfig_Old()
	{
		minIdleInterval			= 200; // C2SessionDataRunner min timer interval, ms
		ioBlockSize				= 4 * 1024; // C2SessionDataRunner, used to calculate bytesToSend
		readBufferCount			= 1; // C2SessionDataRunner, used to calculate bytesToSend
		serverNameTag			= "C2 Streamer"; // C2HttpHandler

		mSocketSendMonitorThreashold = 50; // C2SessionDataRunner, monitor threshold
		mTimerThreadHold		= 50; // C2SessionDataRunner, monitor threshold
		mFsReadThreshold		= 50; // C2SessionDataRunner, monitor threshold
		mPendingSizeThreshold	= 20; // C2Service, normal timer thread request

		mCheckClientWhileTransfer = true; // C2TransferSession, check client address or not
		mMaxIdleCountAtFileEnd	= 0; // C2SessionDataRunner, break if we can' read more data
		mMaxSparseFileSearchRange = 4 * 1024 * 1024; // NOT USED
		mbQuitPerConnectionLost	= true;  // NOT USED
		mSendPacketSize			= 1500; // C2SessionDataRunner, max-byte-per-packet
		mPacketsPerSend			= 10; // C2SessionDataRunner, max-packet-per-send
		maxRoundPerRun			= 3; // C2SessionDataRunner, max-packets-per-run
		minYieldInterval		= 5; // C2SessionDataRunner, yield interval
		mSocketKernelSndBuf		= 1024 * 1024; // C2SessionDataRunner, socket send buffer size
		mSocketKernelSndTimeo	= 20 * 1000; // C2SessionDataRunner, socket send timeout
		mbUseTcpCork			= true; // NOT USED
		mFirstChunkWait			= 150; // session stats
		mFsIoCommitThreshold	= 3; // 10ms
		mFsDataWaitCount		= 300; // C2SessionDataRunner, max data wait count
		mBitrateInflationPercent = 5; // C2SessionDataRunner, session bitrate inflation

		mKeyFile5I				= ""; // DummyHls
		//mAquaRootUrl			= "http://demo5:demo5@10.50.16.80:8080/aqua/rest/cdmi/";
		mAquaRootUrl			= "";// DummyHls
		mHomeContainer			= ""; // DummyHls
		mLogFlag				= 0; // DummyHls
		mDefaultBitrate			= 2048000; // DummyHls

		mUseBufferIo			= false; // AioFile use buffer I/O or not
		mCacheDefaultBufferCount = 100; // CacheCenter, buffer count
		mCacheDeltaBufferCount  = 50; // CacheCenter, buffer count per increasment
		mCacheBufferSize        = 2 * 1024 * 1024; // CacheCenter, buffer size in byte
		mCacheReadaheadCount	= 1; // CacheCenter, pre-read count

		mIndexRecordCacheSize	= 4096; // CacheCenter
		mDefaultTransferBitrate	= 3750 *1000; // Default bitrate

		//for cache buffer remove
		defaultTimeOut = 24 * 3600 * 1000;
		fioErrTimeOut = 10 * 60 *1000;
		httpErrTimeOut = 1 * 60 * 1000;
		socketErrTimeOut = 2 * 1000;
		otherErrTimeOut = 5 * 1000;


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
		C2ClientMaxBufferCountPerRequest = 0;

		//for httpclient
		httpProxyURL = "";
		segmentURL = "";
		httpTimeOut = 10000;
		httpRetry = 0;

		attributesTimeoutInPwe = 3600 * 1000; // AioFile, CacheCenter


		aquaReaderFlags = 0x0f;
    	aquaReaderIoThreadPoolSize = 36;
	    aquaReaderCdmiOpsReadThreadPoolSize = 10;

		aquaReaderCacheBlockSize = 2 * 1024 * 1024;
		aquaReaderCacheBlockCount = 1000;
		aquaReaderReadaheadCount = 2;
		aquaReaderReadAheadThreshold = 0;
		aquaReaderHlsEnableMDataQuery = 1;

		natEnabled = 0;

		diskCacheEnabled = false;
		cacheLoaderTimeout = 3000;
		cacheIgnoreFiles = "";
	}
	uint32			mFsDataWaitCount;
	int32			maxRoundPerRun;//
	int32			minYieldInterval;
	int32			minIdleInterval;
	uint32			ioBlockSize;
	uint32			readBufferCount;

	int32           clientType;
	std::string		serverNameTag;
	NicsConf		streamingNics;
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
	int32 			mBitrateInflationPercent;


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
	int32					 C2ClientAlignment;
	int32					 C2ClientMaxBufferCountPerRequest;

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
	int32					aquaReaderIoThreadPoolSize;
	int32					aquaReaderCdmiOpsReadThreadPoolSize;
    int32                   aquaReaderCacheBlockSize;
    int32                   aquaReaderCacheBlockCount;
    int32                   aquaReaderReadaheadCount;
    int32                   aquaReaderReadAheadThreshold ;
	int32					aquaReaderHlsEnableMDataQuery;

	int32					natEnabled;
	bool 					diskCacheEnabled;
	int32                   cacheLoaderTimeout;
	std::string 			cacheIgnoreFiles;
	DISKCACHECON            diskCacheConfig;
};

struct ConfStreamingNics {
	NicsConf	nics;// no default configuration values
};

struct ConfWriter {
	uint32 udpSessMinStreamInterval;
	uint32 mFsDataWaitCount;
	uint32 maxRoundPerRun;
	uint32 minYieldInterval;
	uint32 minIdleInterval;
	uint32 ioBlockSize;
	uint32 readBufferCount;
	uint32 mMaxIdleCountAtFileEnd;
	bool mbQuitPerConnectionLost;
	uint32 mSendPacketSize;
	uint32 mPacketsPerSend;
	uint32 mSocketKernelSndBuf;
	uint32 mSocketKernelSndTimeo;
	bool mbUseTcpCork;
	uint32 mFirstChunkWait;
	uint32 mBitrateInflationPercent;
	uint32 mDefaultTransferBitrate;
	uint32 mSocketSendMonitorThreashold;
	uint32 mTimerThreadHold;
	uint32 mFsReadThreshold;
	uint32 mFsIoCommitThreshold;

	ConfWriter() {
		udpSessMinStreamInterval = 50;
		mFsDataWaitCount =  300;
		maxRoundPerRun = 3;
		minYieldInterval = 5;
		minIdleInterval = 200;
		ioBlockSize = 4 * 2014;
		readBufferCount = 1;
		mMaxIdleCountAtFileEnd = 0;
		mbQuitPerConnectionLost = true;
		mSendPacketSize = 1500;
		mPacketsPerSend = 10;
		mSocketKernelSndBuf = 1024 * 1024;
		mSocketKernelSndTimeo = 20 * 1000;
		mbUseTcpCork = false;
		mFirstChunkWait = 150;
		mBitrateInflationPercent = 100;
		mDefaultTransferBitrate = 3750 * 1000;
		mSocketSendMonitorThreashold = 50;
		mTimerThreadHold = 50;
		mFsReadThreshold = 50;
		mFsIoCommitThreshold = 20;
	}
};

struct ConfDiskCache {
	bool                    diskCacheEnabled;
	int32                   cacheLoaderTimeout;
	std::string             cacheIgnoreFiles;
	DISKCACHECON            diskCacheConfig;

	ConfDiskCache() {
		diskCacheEnabled = true;
		cacheLoaderTimeout = 3000;
	}
};

struct ConfCacheCenter {
	bool mUseBufferIo;
	uint32 mCacheDefaultBufferCount;
	uint32 mCacheDeltaBufferCount;
	uint32 mCacheBufferSize;
	uint32 mCacheReadaheadCount;
	uint32 mIndexRecordCacheSize;
	int64  defaultTimeOut;
	int64  fioErrTimeOut;
	int64  httpErrTimeOut;
	int64  socketErrTimeOut;
	int64  otherErrTimeOut;
	uint32 assetAttributesTimeoutInPwe;
	bool   calcCRCForBuffer;

	ConfCacheCenter() {
		mUseBufferIo = false;
		mCacheDefaultBufferCount = 1000;
		mCacheDeltaBufferCount = 50;
		mCacheBufferSize = 2 * 1024 * 1024;
		mCacheReadaheadCount = 1;
		mIndexRecordCacheSize = 4096;
		defaultTimeOut = 24 * 3600 * 1000;
		fioErrTimeOut = 10 * 60 * 1000;
		httpErrTimeOut = 1 * 60 * 1000;
		socketErrTimeOut = 2 * 1000;
		otherErrTimeOut = 5 * 1000;
		assetAttributesTimeoutInPwe = 3600 * 1000;
		calcCRCForBuffer = false;
	}
};

struct ConfOldHls_obsolete {
	std::string mKeyFile5I;
	uint32 mKeyId;
	std::string mAquaRootUrl;
	std::string mHomeContainer;
	std::string mServerHostUrl;
	uint32 mLogFlag;
	uint32 mDefaultBitrate;
	std::map<std::string,uint32> mName2Bitrate; 

	ConfOldHls_obsolete() {
		mKeyId = 0;
		mLogFlag = 0;
		mDefaultBitrate = 3750 * 1000;
	}
};

struct ConfC2Client {
	std::string              C2ClientUpStreamIP;
	std::string              C2ClientURL;
	std::string              C2ClientTransfer;
	std::string              C2ClientHttpCRGAddr;
	std::string              C2ClientTransferRate;
	std::string              C2ClientIngressCapacity;
	std::string              C2ClientExclusionList;
	std::string              C2ClientTransferDelay;
	int32                    C2ClientHttpCRGPort;
	int32                    C2ClientDefaultGetPort;
	int32                    C2ClientWaitBufferTime;
	int32                    C2ClientIndexTimeout;
	int32                    C2ClientIndexRetryTimes;
	int32                    C2ClientMainfileTimeout;
	int32                    C2ClientMainfileRetryTimes;
	int32                    C2ClientEnableTransferDelete;
	int32                    C2ClientAlignment;
	int32                    C2ClientMaxBufferCountPerRequest;
	int64					 C2ClientMinTransferRate;
	int32 					 C2ClientBitrateInflate;
	int64 					 C2ClientMaxBitrate;

	ConfC2Client() {
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
		C2ClientIndexTimeout = 5 * 1000;
		C2ClientIndexRetryTimes = 5;
		C2ClientMainfileTimeout = 15 * 1000;
		C2ClientMainfileRetryTimes = 5;
		C2ClientEnableTransferDelete = 0;
		C2ClientAlignment = 8;
		C2ClientMaxBufferCountPerRequest = 0;
		C2ClientMinTransferRate = 3750000;
		C2ClientBitrateInflate = 120;
		C2ClientMaxBitrate = 0;
	}
};

struct ConfHttpClient {
	std::string            httpProxyURL;
	std::string            segmentURL;
	int32                  httpTimeOut;
	int32                  httpRetry;

	ConfHttpClient() {
		httpTimeOut = 10000;
		httpRetry = 0;
	}
};

struct ConfAquaReader {
	std::string             aquaReaderRootUrl;
	std::string             aquaReaderUserDomain;
	std::string             aquaReaderHomeContainer;
	std::string				aquaReaderMainFileExtension;
	std::string				aquaReaderContentNameFormat;
	int32                   aquaReaderFlags;
	int32                   aquaReaderIoThreadPoolSize;
	int32                   aquaReaderCdmiOpsReadThreadPoolSize;
	int32                   aquaReaderCacheBlockSize;
	int32                   aquaReaderCacheBlockCount;
	int32                   aquaReaderReadaheadCount;
	int32                   aquaReaderReadAheadThreshold ;
	bool  					aquaReaderHlsEnableMDataQuery; //disable index file query

	ConfAquaReader() {
		aquaReaderFlags = 0x0f;
		aquaReaderIoThreadPoolSize = 36;
		aquaReaderCdmiOpsReadThreadPoolSize = 10;
		aquaReaderCacheBlockSize = 2 * 1024 * 1024;
		aquaReaderCacheBlockCount = 1000;
		aquaReaderReadaheadCount = 2;
		aquaReaderReadAheadThreshold = 0;
		aquaReaderHlsEnableMDataQuery = true;
	}
};


struct ConfUrlRule {
	int32 readerType;//original clientType, C2Client, HttpClient, AquaReader
	bool mCheckClientWhileTransfer;
	std::string urlPrefix;
	std::string fsPath;

	ConfUrlRule() {
		readerType = 0;
		mCheckClientWhileTransfer = true;
	}
};


struct ConfPerSessionConfig {
	ConfWriter 		writer;
	ConfUrlRule 	urlRule;
};
typedef std::map<std::string, ConfPerSessionConfig> URL2CONFS;
struct C2EnvConfig {
	uint32				mPendingSizeThreshold;
	bool 				natEnabled;
	std::string 		serverNameTag;
	std::string 		mLocalBindPort;

	ConfCacheCenter		cacheCenter;
	ConfDiskCache		diskCache;
	ConfStreamingNics	nics;

	ConfAquaReader		aquaReader;
	ConfHttpClient		httpClient;
	ConfC2Client		c2Client;
	ConfOldHls_obsolete	oldHls;

	std::string         forwardURL;
	bool                enableC2ClientHybrid;
	int32				c2HybridMaxHitCount;
	int32 				c2HybridMaxTimeDuration;
	std::string         sesslogfilename;
	std::string         sesslogfiledirectory;
	int32               maxsesslognum;
	int32               sesslogmgrloopInterval;
	int32               sesslogmgrsessExpire;
	int32               sessStatReportInterval;
	int32               stateSwitchInterval;
	int32               LastFramePos;

	int32				cacheLocateMaxPending;

	//overrideable
	URL2CONFS			 urlRules;

	C2EnvConfig() {
		mPendingSizeThreshold = 100;
		natEnabled = false;
		serverNameTag = "libasync http server";
		mLocalBindPort = "12000";
		forwardURL = "";
		enableC2ClientHybrid = false;
		c2HybridMaxHitCount = 10;
		c2HybridMaxTimeDuration = 10 * 1000;
		cacheLocateMaxPending = 100;
		stateSwitchInterval = 1000;
		LastFramePos = 1;
	}

	const ConfPerSessionConfig* getPerSessConf( const std::string& urlPrefix ) const {
		URL2CONFS::const_iterator it = urlRules.find(urlPrefix);
		if( it != urlRules.end()) {
			return &(it->second);
		}
		return NULL;
	}

	const ConfPerSessionConfig* getFirstPerSessConf() const {
		assert( urlRules.size() > 0 );
		if(urlRules.size() == 0)
			return NULL;
		return &(urlRules.begin()->second);
	}
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

	inline ZQ::common::Log*					getLogger()	{
		return mLogger;
	}

	inline ZQ::common::NativeThreadPool&	getThreadPool( ) {
		return *mThreadPool;
	}

	inline ZQ::common::NativeThreadPool&	getLocateThreadPool( ) {
		return *mLocateThreadPool;
	}

	inline const std::string& 				getDocumentRootFolder() const {
		return mDocRootFolder;
	}

	std::string								getRequestSequence( );

	std::string								generateSessionId( ) const;

	inline const C2EnvConfig&				getConfig() const {
		return mConfig;
	}

	inline IAttrBridge*						getAttrBridge() {
		//assert( mAttrBridge != NULL );
		return mAttrBridge;
	}
	int64									getUpTime( ) const;

	//first persession config should be default persession config

public:
	ZQ::common::Log*				mLogger;
	ZQ::common::NativeThreadPool*	mThreadPool;
	ZQ::common::NativeThreadPool*	mLocateThreadPool;
	std::string						mDocRootFolder;
	C2EnvConfig						mConfig; // global default configuration
	IAttrBridge*					mAttrBridge;
	int64							mServiceStartTime;

private:

	ZQ::common::Mutex				mEnvMutex;
	int64							miSeqBase;
};

#define MLOG	(*mEnv.getLogger())

extern C2StreamerEnv*	getEnvironment( );

#define TRANSFERSESSION_PREFIX "scs/sessiontransfer"
#define URLRULE_GETFILE "scs/getfile"
#define URLRULE_C2INIT "scs/transferinitiate"
#define URLRULE_C2TRANSFER TRANSFERSESSION_PREFIX
#define URLRULE_C2TERM "scs/transferterminate"
#define URLRULE_C2STATUS "scs/status"
#define URLRULE_C2RESSTAT "scs/resourcestatus"
#define URLRULE_HLS "assets"
#define URLRULE_QUERYASSETINFO "scs/queryassetinfo"
#define URLRULE_UDPSESSION "scs/udpsession"


std::string constructResponseSessId( const std::string& sessId);

std::string getSessionIdFromCompoundString( const std::string& str );

std::string dumpStringVector( const std::vector<std::string>& value , const std::string& delimiter =" " );

std::string timeToString( uint64 timeTicket );

const int CLIENT_TYPE_DISKAIO 		= 0;
const int CLIENT_TYPE_C2CLIENT 		= 1;
const int CLIENT_TYPE_HTTPFETCHER 	= 2;
const int CLIENT_TYPE_AQUAREADER 	= 3;
const int CLIENT_TYPE_HTPPFETCHER2	= 4;
const int CLIENT_TYPE_HYBRID		= 5;
const int CLIENT_TYPE_HYBRID_C2CLIENT = 6;

int readerStr2Type( const std::string& reader );
const char* readerType2Str( int type );

}//namespace C2Streamer
#endif//_cdnss_c2streamer_environment_header_file_h__

