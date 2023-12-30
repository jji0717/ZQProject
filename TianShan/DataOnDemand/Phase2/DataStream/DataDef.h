// DataDef.h: interface for the DataDef class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATADEF_H__D91CE7BF_5F41_46D3_A8CF_BB12726452DC__INCLUDED_)
#define AFX_DATADEF_H__D91CE7BF_5F41_46D3_A8CF_BB12726452DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////

#define ZQLIB					ZQ::common

#ifdef _DEBUG
#define LOG_FMT					"\n        %s(%d) : "
#define LOG_ARG					__FILE__, __LINE__
#else
#define LOG_FMT					
#define LOG_ARG					0
#endif

//////////////////////////////////////////////////////////////////////////

#define	TRANSFER_UDP			0
#define	TRANSFER_TCP			1
#define TRANSFER_MULTICAST      2

#define TRANSFER_UNIT_SIZE		((1500 / 188) * 188)
#define	ICE_SERVICE_NAME		"DataStreamService"
#define	ICE_ADAPTER_NAME		"DataStream"
#define _COUNTOF(x)				(sizeof(x) / sizeof((x)[0]))

#define DEFAULT_STD_PERIOD		50
#define DEFAULT_STD_MAXQUEUE	64
#define DEFAULT_STD_MINQUEUE	16

//////////////////////////////////////////////////////////////////////////

namespace ZQ { 
	namespace common {

class ZQThreadPool;

	} // namespace common {
} // namespace ZQ { 

namespace DataStream {

class BufferManager;
class DataSender;
class PsiPusher;

/*class DataConfig {
protected:
	DataConfig();

public:
	virtual ~DataConfig();

	static DataConfig& getDataConfig();

	typedef std::map<std::string, std::string> StrMap;

	int				totalRate;
	bool			higherPriority;
	unsigned long	profileFlag;
	size_t			readerThreadPoolMinSize;
	size_t			readerThreadPoolMaxSize;
	size_t			senderThreadPoolMinSize;
	size_t			senderThreadPoolMaxSize;

	// log file
	char			logFile[MAX_PATH];
	size_t			logFileSize;
	int				logLevel;

	char			catchDir[MAX_PATH];
	StrMap			iceConfig;
	char			netId[128];
	int				checkStreamTimeout;

	char			netWorkcardIP[MAX_PATH];

	unsigned int	stdPeriod;
	size_t			stdMaxQueue;
	size_t			stdMinQueue;
};

#define dataConfig		(::DataStream::DataConfig::getDataConfig())*/

extern ZQLIB::ZQThreadPool* readerThreadPool;
extern ZQLIB::ZQThreadPool* senderThreadPool;
extern BufferManager* bufferManager;
extern DataSender* dataSender;
extern PsiPusher* psiPusher;

//////////////////////////////////////////////////////////////////////////

class TransferAddress {
public:

	TransferAddress():
	  _proto(0), _ip(0), _port(0)
	{


	}

	/// now support IPv4 address only
	TransferAddress(int proto, ULONG ip, WORD port): 
	  _proto(proto), _ip(ip), _port(port)
	{

	}

	int getProto() const
	{
		return _proto;
	}

	ULONG getIP() const
	{
		return _ip;
	}

	WORD getPort() const
	{
		return _port;
	}
	
	bool operator == (const TransferAddress& addr) const
	{
		return addr._proto == _proto && addr._ip == _ip && 
			addr._port == _port;
	}

	bool operator < (const TransferAddress& addr) const
	{
		if (addr._proto < _proto) {
			return true;

		} else if (addr._proto == _proto) {

			if (addr._ip < _ip) {
				return true;

			} else if (addr._ip == _ip) {

				return addr._port < _port;
			} else {

				return false;
			}

		} else {

			return false;
		}
	}

	ULONG			_ip;
	WORD			_port;
	int				_proto;
};

//////////////////////////////////////////////////////////////////////////

#define MAX_EXTRA_LEN			256

struct ProgMapEntry {

	unsigned char	streamType;
	unsigned short	elementId;
	unsigned short	extraInfoLen;
	unsigned char	extraInfo[MAX_EXTRA_LEN];
};

struct ProgMapInfo {

	unsigned short	pmtPid;
	unsigned short	progNum;
	typedef std::vector<ProgMapEntry> PmeVec;
	PmeVec			progMapEntrys;
};

//////////////////////////////////////////////////////////////////////////
std::string getFileNamePart(const std::string& path);

} // namespace DataStream {

#endif // !defined(AFX_DATADEF_H__D91CE7BF_5F41_46D3_A8CF_BB12726452DC__INCLUDED_)
