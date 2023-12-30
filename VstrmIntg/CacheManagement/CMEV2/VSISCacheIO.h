#ifndef _VSISCACHEIO_H_
#define _VSISCACHEIO_H_

#include "NativeThread.h"
#include "SystemUtils.h"
#include "TCPSocket.h"
#include "Pointer.h"

#include "CacheStorage.h"
#include "CMECacheAlgorithm.h"
#include "CMEUtil.h"


// CME - VSIS interface definition use USHORT & ULONG
typedef unsigned short USHORT;	
typedef unsigned long  ULONG;

////////// TO HELP PROCESS CME-VSIS MESSAGES ///////////////////
typedef struct _cv_tlv {
	USHORT	tag;
	USHORT	length;
	char    value[1];
} CV_TLV, *PCV_TLV;				// CME/VSIS TLV

#define PAID_SIZE  38    // Bytes allocated per PAID
#define PID_SIZE   32    // Bytes per PID
#define CLUID_SIZE 16    // Bytes per cluster ID
#define VIEWER_BYTES 4	 // Count of viewer bytes
#define DISKID_SIZE 32    // Bytes per disk ID (drive serial number)

#define HEADER_BYTES (offsetof(CV_TLV, value))

#define advance_message(pointer)                               \
	pointer = (PCV_TLV)((char *)pointer + pointer->length)

#define advance_byte(pointer)            	                   \
	pointer = (PCV_TLV)((char *)pointer + 1)

////////// TO HELP PROCESS CME-VSIS MESSAGES ///////////////////
#define MAX_SOCKET_BUFF_SIZE		64 * 1024

#define MAX_BW_CHANGE_THRESHOLD     10

namespace CacheManagement {
class VSISCacheIO;

class VSISEntity : public ZQ::common::SharedObject, public ZQ::common::TCPClient
{
	friend class VSISCacheIO;
public:
	VSISEntity(VSISCacheIO& vsisIO, std::string name, std::string clusterID, std::string ip, uint32 reservedImpBW, uint32 maxReportInterval, const ZQ::common::InetAddress &bind, ZQ::common::tpport_t port = 0);

	virtual ~VSISEntity();

public:
	std::string _clusterID;
	std::string _name;		// composed by clutserid and ip
	std::string _ip;
	std::string _nodeName;	// node name from VSIS. we don't use it actually. 

	// flags
	bool		_isCMEPremetersSent;			// if it is false, need to re-process
	bool		_isCMERegistered;				// if has not been registered, re-process
	bool		_isVSISRegistered;				// if VSIS registered
	uint64		_cmeRegisteredTime;				// time of CME sent register msg
	
protected:
	VSISCacheIO& _vsisIO;

	bool        _connected;						// is socket connected with vsis
	bool		_status;						// avaiable or unavaiable (CDN is up or down)
	uint64      _lastUpdate;					// last update of _status

	uint64		_totalOutputBW;					// total output bandwidth; not used
	uint64		_usedImportBW;
	uint64		_totalImportBW;
	uint64		_usedStreamBW;
	uint64		_totalStreamBW;

	uint32      _reservedImpBW;
	uint32      _maxReportInterval;

	uint32      _bwNotChangeTimes;

	ZQ::common::InetAddress _serverAddress;
	int						_serverPort;
public:
	void setConnStatus(bool connected);
	void setVSISStatus(bool avaible);
	void setTotalOutputBW(uint64 bw);
	void setBandwidth(uint64 usedImportBW, uint64 totalImportBW, uint64 usedStreamBW, uint64 totalStreamBW);

	bool isConnected() { return _connected; };
	bool isAvailable() { return _status; };
	bool isIOReady()   { return (_connected && _status && _isVSISRegistered); };
	bool isActivity()  { return _bwNotChangeTimes < MAX_BW_CHANGE_THRESHOLD; };
	bool isRecentReported();

	uint64 getLastUpdateTime() { return _lastUpdate; }
	uint64 getFreeImportBW() { return (_totalImportBW-_reservedImpBW) > _usedImportBW ? (_totalImportBW-_reservedImpBW-_usedImportBW) : 0 ; };
	void getImportBW(uint64& used, uint64& total, uint32& reserved) { used=_usedImportBW; total=_totalImportBW; reserved=_reservedImpBW; };

	void consumeImportBW(uint32 usedBW) { _usedImportBW += usedBW; };

public:
	bool onConnectLost();

public:
	//overried TCPSocket
	virtual void OnConnected();
	virtual void OnError();
	virtual void OnDataArrived();
	virtual void OnTimeout() {};
public:
	typedef struct _RECV_VSIS_DATA
	{
		uint32  length;
		char    data[MAX_SOCKET_BUFF_SIZE];
		uint64	No;		// for test 
	}REC_VSIS_DATA;

	std::queue<REC_VSIS_DATA*>		_bufferQueue;
	ZQ::common::Mutex				_buffLocker;

	uint32							_addupLength;
	std::vector<REC_VSIS_DATA*>     _tempVSISDatas;
};

typedef ZQ::common::Pointer<VSISEntity> VSISEntityPtr;
typedef std::map<std::string, VSISEntity*>	VSISENTITIES;

class VSISCacheIO : public CacheIO, protected ZQ::common::NativeThread
{
	friend class VSISEntity;

public:
	typedef enum _IO_EVENT_TYPE{ IO_CONN_LOST = 0,			// +
								IO_DATA_ARRIVE,				// +

								IO_OP_IMPORT,				// +
								IO_OP_DELETE,				// +
								IO_OP_SYNCONE,				// + 
								IO_OP_SYNCALL				// +
							  }IO_EVENT_TYPE;
	
	class IOEvent
	{
	public:
		IOEvent(IO_EVENT_TYPE type) { evtType=type; };
		IOEvent(IO_EVENT_TYPE type, VSISEntity& vsis) { evtType=type; pVsisEntity = &vsis;};
		IOEvent(IO_EVENT_TYPE type, std::string& pid, std::string& paid) 
					{ evtType=type; this->pid=pid; this->paid=paid; };
		virtual ~IOEvent() {};

		// for connection lost event & data arrive evt
		VSISEntity*					pVsisEntity;

		std::string					pid;
		std::string					paid;

		IO_EVENT_TYPE				evtType;
	};
	std::queue<IOEvent*>			_eventQueue;
	ZQ::common::Mutex				_eventLock;

	class SIMPLE_CONTENT_INFO
	{
	public:
		SIMPLE_CONTENT_INFO(std::string& p, std::string& pa) 
			{ pid=p; paid=pa; isPWE=false; playCount=0; reportedVsis=0; totalVsis=0; timestamp=ZQ::common::now();} ;
		virtual ~SIMPLE_CONTENT_INFO() {};

		std::string		pid;
		std::string		paid;
		bool		    isPWE;
		int				playCount;
		int             reportedVsis;	// how many vsis had reported the UserCount
		int             totalVsis;		// how many vsis the usercount request were sent

		uint64			timestamp;
	};
	typedef std::map<std::string, SIMPLE_CONTENT_INFO*> SIMPLE_CONTENTS;

	SIMPLE_CONTENTS _tempContents;


/////////////////////////////////////////////////////////////////////////////////////////////////////////

public:
	VSISCacheIO(std::string clusterID, CMECacheAlgorithm& cmeAlg);
	virtual ~VSISCacheIO();

protected:
	CMECacheAlgorithm&						_cmeAlg;		// used to evaluat the vsis to do operation

	std::string _clusterID;
	std::string _localIP;

	uint32		_connTimeout;
	uint32		_sendTimeout;
	uint32		_readBuffSize;

	uint32		_paidLength;

	uint64		_freeSpace;
	uint64		_totalSpace;

	uint32      _reservedBW;
	uint32      _maxRptInterval;
public:
	void setLocalIP(std::string& localIP);
	void setConnParameters(uint32 connTimeout, uint32 sendTimeout, uint32 readBuffSize);
	void setConnScanInterval(uint32 scanInterval);
	void setContentParameter(uint32 paidLen);
	void setReservedBW(uint32 reserved);
	void setMaxRptInterval(uint32 maxInterval);

	std::string getClusterID() { return _clusterID; };
	void setRecentImportBW();
	void getRecentImportBW(uint64& usedBW, uint64& totalBW);
	uint32 getRecentImportBWUsage();
	uint64 getRecentFreeBW();

protected:
	SYS::SingleObject		_waitEvent;
	bool					_threadRunning;
	uint32					_connScanInterval;

	uint64                  _usedImportBW;
	uint64				    _totalImportBW;
public:
	// thread related functions
	bool initialize(std::string& nodesIP);
	void unInitialize();

protected:
	int run(void);
	void final(void);

	void checkRequiredSentMsg();
	void checkUnimportedContent();
	void printImportBW();

protected:
	VSISENTITIES	_vsisEntities;	// vsis entities

	bool VSIS_STAT_2_CONT_STAT(int vsisStat, Content::CONTENT_STATUS& cntStatus);

	bool connectVSIS();
	bool connectVSIS(VSISEntity& vsisEntity);

	int sendBufferToVSIS(VSISEntity& vsisEntity, char* buff, int length);
	bool requestRegisterCME(VSISEntity& vsisEntity);
	bool requestUserCount(VSISEntity& vsisEntity, std::string& pid, std::string& paid);
	bool requestDeletion(VSISEntity& vsisEntity, std::string& pid, std::string& paid);
	bool requestImport(VSISEntity& vsisEntity, std::string& pid, std::string& paid, uint32 pweCount);
	bool requestListAll(VSISEntity& vsisEntity);
	bool requestAssetStatus(VSISEntity& vsisEntity, std::string& pid, std::string& paid);
	bool requestAssetExpired(VSISEntity& vsisEntity, std::string& pid, std::string& paid);
	bool reqeustDeductAsset(VSISEntity& vsisEntity, std::string& pid, std::string& paid, float fraction);
	bool requestCMEPremeters(VSISEntity& vsisEntity, uint32 cushion, uint32 playTrigger);

	void processIncomingData(VSISEntity& vsisEntity);
	bool processVSISBuff(VSISEntity& vsisEntity, char* buff, uint32 buffSize, uint32& processedBytes);
	
	bool VSISRegister(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes);
	bool VSISCDNState(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes);
	bool VSISAssetStatus(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes);
	bool VSISAssetList(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes);
	bool VSISSmartParameters(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes);
	bool VSISSessionStart(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes);
	bool VSISSessionEnd(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes);
	bool VSISSessionCount(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes);
	bool VSISDiskUsage(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes);
	bool VSISVodTlvData(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes);
	bool VSISVodPopData(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes);
	bool VSISLastVodPopData(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes);

public:
	bool isReady();
	bool isActivity();
	IO_OP_RESULT importContent(std::string& pid, std::string& paid);
	IO_OP_RESULT deleteContent(std::string& pid, std::string& paid);
	IO_OP_RESULT readContent(std::string& pid, std::string& paid, PROPERTIES& properites);
	IO_OP_RESULT listContents(CONTENTS& contents);

	void onDataReceived(VSISEntity& vsis);
	void onConnectionLost(VSISEntity& vsis);

protected:
	bool doImportContent(IOEvent& evt);
	bool doImportContent(std::string& pid, std::string& paid, bool pwe=false, uint32 userCount=0);	// do real import
	bool doDeleteAsset(IOEvent& evt);
	bool doSyncOneAsset(IOEvent& evt);
	bool doSyncAllAsset();
	bool doReqUserCount(SIMPLE_CONTENT_INFO& simpleCnt);


};

}
#endif
