#include "jms.h"
#include "queuemanagement.h"
#include "receivejmsmsg.h"
#include <Ice/Ice.h>
#include <TsAppDOD.h>
#include "Locks.h"
typedef struct 
{
	std::string ChannelName;
	std::string		Tag;
	std::string     DataExchangeType;	
	int		nstreamId;			/// PID
	TianShanIce::Application::DataOnDemand::EncryptMode    nEncrypted;
	std::string  strSendMsgDataType;
	int     nSendMsgExpiredTime;
	int	    nChannelType;	/// default 1
	int     nStreamCount;
	std::string  strdataType;
	int		streamType;
	int     nSendWithDestination;
	int     nMessageCode;
	std::string  strQueueName;
	std::string  strCacheDir;
}CHANNELINFO;
typedef ::std::map<std::string , CHANNELINFO*> CHANNELMAP;

typedef struct  
{
   std::string     ChannelName;
   int             nRepeatetime;
   int             nChannelRate; 
   std::string     DataExchangeType;
}CHANNELATTACHINFO;
typedef ::std::map<std::string, CHANNELATTACHINFO*>CHANNELATTACHINFOMAP;

typedef struct 
{
	std::string IpPortName;
	int nSendType;
	std::string strIp;
	int nPort;
}IPPORT;

typedef std::vector<IPPORT*>IPPORTVECTOR;
typedef struct
{
	std::string  portdir;
	std::string  portname;
	std::string  DestinationName;
	int		pmtPid;			/// 打包进 TS 中的 PMT pid
	int		totalBandWidth;	/// destination 的 bandwidth in bps
	std::string	destAddress;	/// 目标地址, in the format of "<IP>:<port>[; <IP>:<port>]"
	int		groupId;	/// 组id
	CHANNELATTACHINFOMAP channelattachMap;
	IPPORTVECTOR  ipportvector;
}DODPORT;
typedef std::vector<DODPORT *> CPORTVECTOR;

typedef struct
{
	std::string  IpPortName;
	int		pmtPid;			/// 打包进 TS 中的 PMT pid
	int		totalBandWidth;	/// destination 的 bandwidth in bps
	std::string	destAddress;	/// 目标地址, in the format of "<IP>:<port>[; <IP>:<port>]"
	int		groupId;	/// 组id
	CHANNELATTACHINFOMAP channelattachMap;
}DODFAILPORT;
typedef std::vector<DODFAILPORT> CFAILPORTVECTOR;


typedef struct ZQMsgParser
{
   CString QueueName;
   CReceiveJmsMsg *MsgReceive;
}ZQMSGPARSER, *PZQMSGPARSER;
class  CJMSPortManager {
public:
	
	CJMSPortManager(std::string datatunnelendpoint,Ice::CommunicatorPtr& ic);
	~CJMSPortManager();

	BOOL Initialize();
	BOOL UnInitialize();
  	BOOL ConnectionJBoss(void);
	BOOL Create(std::string strJBossIPPort, std::string ConfigQueueName,
		int ConfigMsgTimeOut,std::string strCacheDir,int nUsingJBoss);
    void stop();
    TianShanIce::Application::DataOnDemand::DataPointPublisherPrx GetDataPointPublisherPrx();
private:
	int m_nUsingJBoss;
	BOOL SetMessage();
	BOOL AddChannelQueue();
	BOOL CheckSyn();
	BOOL CreateChannels();
	BOOL CreatePorts();
	BOOL AttachChannel();
	BOOL DirectoryExist(LPCTSTR lpszpathame);
	BOOL ParserChannelType(::TianShanIce::IValues *dataTypes, std::string strDataType);
	int ConvertTag(std::string strTag); 
	
	BOOL ConventChannelName();
	BOOL ConvertPortName();
	::TianShanIce::StrValues  listPortChannels(DODPORT * pPort);
	BOOL RetryCreatPort();
	BOOL CreateFailPort(DODFAILPORT* pfailport);

public:	
	CJMS   *m_jms;
	CString m_sDataTypeInitial;
	CString m_ProviderValue;
	std::string  m_strConfigQueueName;
	int m_nConfigTimeOut;
	BOOL m_bReSetMsgListener;

    CHANNELMAP m_channels;
    CPORTVECTOR	m_portManager;
	CFAILPORTVECTOR m_FailPort;
	std::vector<ZQMSGPARSER> m_VecParser;
	
private:
    std::string m_strCacheDir;
	CQueueManageMent m_QueueManagement;
	CReceiveJmsMsg * m_ParsePortConfig;
	CJmsProcThread *m_Pjmsprocthread;
	ZQ::common::Mutex _mutex;
	std::string m_strDataTunnelEndpoint;
	Ice::CommunicatorPtr m_ic;
};
