#pragma once

//#include "interfaceDefination.h"
//#include "BroadcastGuid.h"
//#include "dodportctrl.h"
//#include "dodsubchannel.h"

class CDODPortCtrl;
class DW_ParameterSet;

typedef std::list<CDODPortCtrl*>    PORTCTRLLIST;        


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
    int wDataExchangeType; // 0, Share Folder; 1, message; 2, Local Folder
	int wStreamCount; //if wChannelType==1 .
	int wPackagingMode;
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
typedef struct PORTINFO 
{
	DODCHANNELINFOLIST lstChannelInfo;
	DODIPPORTINFLIST   lstIPPortInfo;
	int		wChannelCount;						//Files Count		    
	int nCastCount;						//output Count		    
	int wPmtPID;
//	WORD wSendType;		//TCPIP, UDP, MULTICAST, BROADCAST
	int wTotalRate;
	char szTempPath[MAX_PATH];		//Temp path for broadcastfilter
	int m_nSessionID;
} PPortInfo;

class AFX_EXT_CLASS CDODDeviceController
{
public:
	CDODDeviceController();
	~CDODDeviceController();

	DWORD Initialize(char* szConfigFile);

// really extern function 
	DWORD 	SetCatalogName(DWORD nSessionID, WORD wPID,TCHAR *pszCatalog);
	DWORD 	GetCatalogName(DWORD nSessionID, WORD wPID,TCHAR *pszCatalog);
	DWORD	UpdateCatalog(DWORD nSessionID, WORD wPID);

	DWORD	SetDetectedFlag(DWORD nSessionID, WORD wPID,BOOL bDetected);
	DWORD	GetDetectedFlag (DWORD nSessionID, WORD wPID,BOOL* bDetected);

	DWORD   SetChannelEvent(DWORD nSessionID, WORD wPID,HANDLE hStateEvent);
	// ICatalogState methods 
	DWORD SetCatalogStateEvent(DWORD nSessionID, WORD wPID,HANDLE hStateEvent);	 
	DWORD GetLastErrorCode(DWORD nSessionID, WORD wPID,int* nError);
	DWORD GetCurrentStatus(DWORD nSessionID, WORD wPID,int* pnStatus);

	//Open, Close, Pause, Run, Stop port function.
	//Return valuse is 0 indicates that it is successful.
	DWORD OpenPort(DWORD nSessionID,PPortInfo *PtInfo);
	DWORD GetPort(DWORD nSessionID, PPortInfo* pPortInfo);
	DWORD ClosePort(DWORD nSessionID);
	DWORD RunPort(DWORD nSessionID); 
	DWORD PausePort(DWORD nSessionID);
	DWORD StopPort(DWORD nSessionID);
// ------------------------------------------------------ Modified by zhenan_ji at 2005年6月6日 16:42:54
	
	//Functions to set and get specified port information.
	//0 indicates it is successful.
	DWORD GetPortState(DWORD nSessionID, DWORD* pdwPortState);
	DWORD GetTotalBitRate(DWORD nSessionID, DWORD *pdwBitRate);
	DWORD GetSessionID(DWORD nSessionID, DWORD *psessionid);
//
	DWORD SetChannelDetectInterval(DWORD nSessionID, WORD wPID, long dwInterval);
	DWORD GetChannelDetectInterval(DWORD nSessionID, WORD wPID, long* pdwInterval);

	DWORD SetWrapParaSet( DWORD nSessionID, WORD wPID,DW_ParameterSet inPara );
	DWORD GetWrapParaSet( DWORD nSessionID, WORD wPID,DW_ParameterSet * outPara );

	DWORD AddChannelVersion(DWORD nSessionID, WORD wPID);
	DWORD ResetChannelVersion(DWORD nSessionID, WORD wPID);
	DWORD SetChannelVersion(DWORD nSessionID, WORD wPID,const char * inObjectKey, BYTE inObjectKeyLen, BYTE inVersion);
	DWORD GetChannelVersion(DWORD nSessionID, WORD wPID,const char * inObjectKey, BYTE inObjectKeyLen, BYTE * outVersion);
	DWORD SetObjectKeyLength(DWORD nSessionID, WORD wPID, BYTE dwObjKeyLen);
	DWORD GetObjectKeyLength(DWORD nSessionID, WORD wPID, BYTE* pdwObjKeyLen);
	DWORD SetChannelIdxDesLen(DWORD nSessionID, WORD wPID, BYTE dwIdxDesLen);
	DWORD GetChannelIdxDesLen(DWORD nSessionID, WORD wPID, BYTE* pdwIdxDesLen);

	DWORD EnableChannel(DWORD nSessionID, WORD wPID, BOOL bAnable);
// ------------------------------------------------------ Modified by zhenan_ji at 2005年3月29日 11:09:53
	DWORD SetLogFile( LPCTSTR szLogFile );
private:

	//now ,it's no use.
	PORTCTRLLIST	m_pCtrlList;

	//all log file will be put on one place.
	CString			m_sLogFileName;

	//Save sendFile for broadcastfilter.ax
	CString			m_sPathForArrange;
	//Save sendFile for broadcastfilter.ax
	CString			m_sDirName;
private:

	//close all graphmanager
	DWORD ReleaseAll();

	//In early, its use config default setting
	DWORD Parse( CSCMemoryBlock block );
	
	//find a portctrl from portctrllilst by portID
	//if findout a destination ,It retuen true,or return false;
	BOOL FoundnSessionIDFromList(DWORD nID,CDODPortCtrl **pCtrl);
};
