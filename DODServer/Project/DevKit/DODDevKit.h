
#include"SingleConnect.h"
#include "DeviceInfoParser.h"


#define DODDEVKITERROR	0X30001
#define DODCONNECTSRMERROR				DODDEVKITERROR+0	// connect SRM error
#define DODGETSESSIONIDERROR			DODDEVKITERROR+1	// Getsessionid 's return value is negative
#define DODDELETESESSIONMERROR			DODDEVKITERROR+2	// delete session, the operation is overtime!
#define DODCOPENPORTOVERTIMEERROR		DODDEVKITERROR+3	// openport, the operation maybe overtime! 
#define DODCOCLOSEPORTOVERTIMERMERROR	DODDEVKITERROR+4	// closeport, the operation maybe overtime! 
#define DODSTOPPORTERROR				DODDEVKITERROR+5	// stopport, the operation maybe overtime! 
#define DODSETCATALOGERROR				DODDEVKITERROR+6	// setcatalogname, the operation is overtime! 
#define DODUPDATECATALOGERROR			DODDEVKITERROR+7	// updatecatalog, the operation is overtime! 
#define DODENABLECHANNEOVERTIMELERROR	DODDEVKITERROR+8	// enablechannelflag, the operation maybe overtime! 
#define DODRUNOVERTIMELERROR			DODDEVKITERROR+9	// run, the operation maybe  overtime! 

typedef struct PORTChannelInfo 
{
	int wPID;			// Channel PID.
	int wRate;			// Kilo bits 
	int bEnable;		// Channel anble or disable.
	char cDescriptor[4];
	int wRepeatTime;	// for subchannel
	int bBeDetect;		// 	Automotion
	int wBeDetectInterval; //manual time .
	int wBeEncrypted;   
	int wChannelType;
	WORD nStreamType;
	int nIndex;					//ID for catalog,set by sw itself	
	char 		szPath[MAX_PATH];		//Catalog path 

	int wMultiplestream;
	int wStreamCount;
	int wPackagingMode; //create index table or not by destionation
} PPChannelInfo;

typedef std::list< PPChannelInfo > DODCHANNELINFOLIST;

typedef struct ZQSBF_IP_PORT_Info
{
	WORD wSendType;		//TCPIP, UDP, MULTICAST, BROADCAST
	char cSourceIp[16]; 
	WORD wSourcePort;
	char cDestIp[16];
	WORD wDestPort;
}ZQSBFIPPORTINF, *PZQSBFIPPORTINF;

typedef std::list< ZQSBFIPPORTINF > DODIPPORTINFLIST;
typedef struct PORETINFO 
{
	DODCHANNELINFOLIST lstChannelInfo;
	DODIPPORTINFLIST   lstIPPortInfo;
	int		wChannelCount;						//Files Count		    
	int nCastCount;
	int wPmtPID;
	int wTotalRate;
	char szTempPath[MAX_PATH];		//Temp path for broadcastfilter
	int m_nSessionID;
} PPortInfo;
//
class AFX_EXT_CLASS CDODDevKit
{
public:
	CDODDevKit(char * logPath);
	~CDODDevKit();

	//external  module called it ,It begin to connect SRM service.
	//if their connecter is disconnectted suddenly,their can connect automatism.
	int setSRMAdd(char *RemoteIP,int Port,BOOL IsSettingIPPort,int nTimeOut=1000);

	//get sessionid for each port.
	int GetSessionID(int &nSessionID);

	//update a channel of a port
	int UpdateCatalog(int nSessionID,int PortID,int channelID);

	//set monitor path for the watchsourcefilter
	//the second parameter is a pmtpid of the channel.
	DWORD SetCatalogName(int nSessionID,DWORD dwPortID, WORD wPID,TCHAR *pszCatalog);

	//create a port ,parse struct,build graphmanager
	//if a error occurs suddenly, the error was be written to log file.
	DWORD OpenPort(int nSessionID,DWORD dwPortID,PPortInfo *PtInfo);

	//get portinfo by sessionid and portID
	DWORD GetPort(int nSessionID,DWORD dwPortID, PPortInfo* pPortInfo);

	//delete this port ,include graphmanager ,etc.
	//at last,delete this element from list.
	DWORD ClosePort(int nSessionID,DWORD dwPortID);

	//star send data ,call graphmanager run.
	DWORD RunPort(int nSessionID,DWORD dwPortID); 

	//stop send data, call graphmanager stop
	DWORD StopPort(int nSessionID,DWORD dwPortID); 

	//permit or forbid send data for a channel of a port
	DWORD EnableChannel(int nSessionID,DWORD dwPortID, WORD wPID, BOOL bAnable);

	//get current state from graphmanager,
	int GetState(int nSessionID,DWORD dwPortID);

	//send this commange to SRM ,this note is delete a port of srm_resource.
	DWORD DeleteSession(int nSessionID);

	//Connecter will be stop,set reConnectFlag=false;.
	DWORD DisConnectSRM();

private:

	//in this article ,include connect SRM 's major founction,
	CSingleConnect* m_pSingleConnect;

	//to parse all received message ,
	//if the messsage is succeed ,it will be parse ,then an element is delete for m_commandList.
	CDeviceInfoParser *m_Parser;
private:

	//get current time for wrapping XML message.
	CString GetCurrDateTime();

	//message memory or buffer for waitting at received message. 
	CPtrList *m_CcommandList;
};