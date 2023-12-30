#include "jms.h"
#include "queuemanagement.h"
#include "receivejmsmsg.h"
#include <Ice/Ice.h>
#include <DODapp.h>
typedef struct 
{
	string  ChanneName;
	string		Tag;
	int     nRepeatetime;
	::DataOnDemand::ChannelType     nDataExchangeType;	
	int		nstreamId;			/// PID
	int     nChannelRate;
	::DataOnDemand::EncryptMode    nEncrypted;
	string  strSendMsgDataType;
	int     nSendMsgExpiredTime;
	int	    nChannelType;	/// default 1
	int     nStreamCount;
	string  strdataType;
	int		streamType;
	int     nSendWithDestination;
	int     nMessageCode;
	string  strQueueName;
	string  strCacheDir;
}CHANNELINFO;

typedef vector<CHANNELINFO*> CHANNELVECTOR;

typedef struct 
{
	string IpPortName;
	int nSendType;
	string strIp;
	int nPort;
}IPPORT;
typedef vector<IPPORT*>IPPORTVECTOR;
typedef struct
{
	string  portdir;
	string  portname;
	int		pmtPid;			/// 打包进 TS 中的 PMT pid
	int		totalBandWidth;	/// destination 的 bandwidth in bps
	string	destAddress;	/// 目标地址, in the format of "<IP>:<port>[; <IP>:<port>]"
	int		groupId;	/// 组id
	CHANNELVECTOR channelvector;
	IPPORTVECTOR  ipportvector;
}DODPORT;
typedef std::vector<DODPORT *> CPORTVECTOR;

typedef struct ZQMsgParser
{
   CString QueueName;
   CReceiveJmsMsg *MsgReceive;
}ZQMSGPARSER, *PZQMSGPARSER;
class  CJMSPortManager {
public:
	
	CJMSPortManager();
	~CJMSPortManager();

	BOOL Initialize();
	BOOL UnInitialize();
  	BOOL ConnectionJBoss(void);
	BOOL Create(string strJBossIPPort, string ConfigQueueName,
		int ConfigMsgTimeOut,string strCacheDir,int nUsingJBoss);
    void stop();
	
	CJMS   *m_jms;
	CString m_sDataTypeInitial;
	CString m_ProviderValue;
	string  m_strConfigQueueName;
	int m_nConfigTimeOut;
	//ReSet MsgListener flag 
	BOOL m_bReSetMsgListener;
	// TODO: add your methods here.

    CPORTVECTOR	m_portManager;
	vector<ZQMSGPARSER> m_VecParser;
   
private:
    string m_strCacheDir;
	CQueueManageMent m_QueueManagement;
	CReceiveJmsMsg * m_ParsePortConfig;
	CJmsProcThread *m_Pjmsprocthread;
	BOOL SetMessage();
    BOOL ApplyParameter();
	BOOL AddChannelQueue();
	BOOL CheckSyn();
	BOOL CreateChannel(DODPORT * pPort,DataOnDemand::DestinationPrx destinationprx);
    BOOL ParserIPandPort(DODPORT* pPort);
	BOOL DirectoryExist(LPCTSTR lpszpathame);
	BOOL ParserChannelType(::TianShanIce::IValues *dataTypes, string strDataType);
	int ConvertTag(string strTag); 
	int m_nUsingJBoss;	
};
