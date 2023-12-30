#pragma once

//#include "interfaceDefination.h"
//#include "BroadcastGuid.h"
//#include ".\dodportctrl.h"
//#include ".\dodsubchannel.h"

#define MAXDODCTRLNUMBER 5
#define DODMAXCHANNELNUMBER 5

class CDODPortCtrl;
class DW_ParameterSet;

typedef struct PORTChannelInfo 
{ 
	int wPID;			// Channel PID.
	int wRate;			// Kilo bits 
	int bEnable;		// Channel anble or disable.
	char cDescriptor[3];
	int wRepeatTime;	// for subchinna
	int bBeDetect;		// 	Automotion
	int wBeDetectInterval; //manual time .
	int wBeEncrypted;   
	int wChannelType;
	WORD nStreamType;
	int nIndex;					//ID for catalog,set by sw itself	
	char 		szPath[MAX_PATH];		//Catalog path 
} PPChannelInfo;


typedef struct ZQSBF_IP_PORT_Info
{
	WORD wSendType;		//TCPIP, UDP, MULTICAST, BROADCAST
	char cSourceIp[16]; 
	WORD wSourcePort;
	char cDestIp[16];
	WORD wDestPort;
}ZQSBFIPPORTINF, *PZQSBFIPPORTINF;

typedef struct PORTINFO 
{
	PPChannelInfo m_Chanel[DODMAXCHANNELNUMBER];
	ZQSBFIPPORTINF m_castPort[DODMAXCHANNELNUMBER];
	int		wChannelCount;						//Files Count		    
	int nCastCount;						//output Count		    
	char cSourceIp[16]; 
	int wSourcePort;
	char cDestIp[16];
	int wDestPort;
	int wPmtPID;
	WORD wSendType;		//TCPIP, UDP, MULTICAST, BROADCAST
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
	DWORD 	SetCatalogName(DWORD dwPortID, WORD wPID,TCHAR *pszCatalog);
	DWORD 	GetCatalogName(DWORD dwPortID, WORD wPID,TCHAR *pszCatalog);
	DWORD	UpdateCatalog(DWORD dwPortID, WORD wPID);

	DWORD	SetDetectedFlag(DWORD dwPortID, WORD wPID,BOOL bDetected);
	DWORD	GetDetectedFlag (DWORD dwPortID, WORD wPID,BOOL* bDetected);

	DWORD   SetChannelEvent(DWORD dwPortID, WORD wPID,HANDLE hStateEvent);
	// ICatalogState methods 
	DWORD SetCatalogStateEvent(DWORD dwPortID, WORD wPID,HANDLE hStateEvent);	 
	DWORD GetLastErrorCode(DWORD dwPortID, WORD wPID,int* nError);
	DWORD GetCurrentStatus(DWORD dwPortID, WORD wPID,int* pnStatus);

	//Open, Close, Pause, Run, Stop port function.
	//Return valuse is 0 indicates that it is successful.
	DWORD OpenPort(DWORD dwPortID,PPortInfo PortInfo);
	DWORD GetPort(DWORD dwPortID, PPortInfo* pPortInfo);
	DWORD ClosePort(DWORD dwPortID);
	DWORD RunPort(DWORD dwPortID); 
	DWORD PausePort(DWORD dwPortID);
	DWORD StopPort(DWORD dwPortID);

	//Functions to set and get specified port information.
	//0 indicates it is successful.
	DWORD GetPortState(DWORD dwPortID, DWORD* pdwPortState);
	DWORD GetTotalBitRate(DWORD dwPortID, DWORD *pdwBitRate);
	DWORD GetSessionID(DWORD dwPortID, DWORD *psessionid);
//
	DWORD SetChannelDetectInterval(DWORD dwPortID, WORD wPID, long dwInterval);
	DWORD GetChannelDetectInterval(DWORD dwPortID, WORD wPID, long* pdwInterval);

	DWORD SetWrapParaSet( DWORD dwPortID, WORD wPID,DW_ParameterSet inPara );
	DWORD GetWrapParaSet( DWORD dwPortID, WORD wPID,DW_ParameterSet * outPara );

	DWORD AddChannelVersion(DWORD dwPortID, WORD wPID);
	DWORD ResetChannelVersion(DWORD dwPortID, WORD wPID);
	DWORD SetChannelVersion(DWORD dwPortID, WORD wPID,const char * inObjectKey, BYTE inObjectKeyLen, BYTE inVersion);
	DWORD GetChannelVersion(DWORD dwPortID, WORD wPID,const char * inObjectKey, BYTE inObjectKeyLen, BYTE * outVersion);
	DWORD SetObjectKeyLength(DWORD dwPortID, WORD wPID, BYTE dwObjKeyLen);
	DWORD GetObjectKeyLength(DWORD dwPortID, WORD wPID, BYTE* pdwObjKeyLen);
	DWORD SetChannelIdxDesLen(DWORD dwPortID, WORD wPID, BYTE dwIdxDesLen);
	DWORD GetChannelIdxDesLen(DWORD dwPortID, WORD wPID, BYTE* pdwIdxDesLen);

	DWORD EnableChannel(DWORD dwPortID, WORD wPID, BOOL bAnable);
// ------------------------------------------------------ Modified by zhenan_ji at 2005Äê3ÔÂ29ÈÕ 11:09:53

	CDODPortCtrl *m_pDODPortCtrl[MAXDODCTRLNUMBER];
	PPortInfo	*m_PortInfo[MAXDODCTRLNUMBER];
	DWORD m_nDODPortCtrlNumber;
private:
	DWORD ReleaseAll();
	DWORD Parse( CSCMemoryBlock block );
	PPortInfo m_DefaultPortInfo;
};
