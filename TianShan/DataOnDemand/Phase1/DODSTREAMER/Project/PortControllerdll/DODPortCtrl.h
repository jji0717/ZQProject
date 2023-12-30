#pragma once
#include "dodport.h"

#include <dshow.h>
#include <stdio.h>
#include <tchar.h>
#include "msxmldom.h"
#include "clog.h"
#include "dodchannel.h"
#include "BroadcastGuid.h"
#include "common.h"


class CDODChannel;


#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }
typedef enum {
	STATEPortinitialized = 0, STATEPortopened, STATEPortRunning, STATEPortStop, STATEPortClose,STATEPortError
} PLAYER_STATE2 ;

typedef enum {
	STATEUnknown = 0, STATEStopped, STATEPaused, STATEPlaying, STATEScanning
} PLAYER_STATE ;
class CDODPortCtrl
{
public:
	CDODPortCtrl(void);
	~CDODPortCtrl(void);

	DWORD Initialize();

	//Functions to set and get specified port information.
	//0 indicates it is successful.
	DWORD GetPortState(DWORD* pdwPortState);
	DWORD GetPort(CDODPort* pDODPort);
	DWORD BuildGraphManager();
	DWORD ApplyChannelParameter();
// ------------------------------------------------------ Modified by zhenan_ji at 2005年3月31日 10:34:55
// for CBroadcastFilter;

//	DWORD SetChannelCount( DWORD dwChannelCount ); 
	DWORD ConfigChannel( int nPinIndex, ZQSBFCHANNELINFO* pChannelInfo); 
	DWORD SetTotalRate(DWORD wTotalRate); 
	DWORD SetTmpFilePath(char *cfilepath); 
	DWORD SetPmtPID( WORD wPID); 
	DWORD EnablePin( WORD wPID, BOOL bAnable);
	DWORD GetChannelInfo( WORD wPID, ZQSBFCHANNELINFO* pChannelInfo);
	DWORD GetTotalRate(DWORD* pwTotalRate); 
	DWORD GetTmpFilePath(char *cfilepath); 
	DWORD GetPmtPID( WORD* pwPID); 

	DWORD SetIPPortConfig( ZQSBFIPPORTINFO* pInfoList[], int nCount); 
	DWORD GetIPPortConfig( int nIndex, ZQSBFIPPORTINFO* pInfo);

// ------------------------------------------------------ Modified by zhenan_ji at 2005年6月6日 16:50:30

	//  ICatalogConfigure methods

	DWORD 	SetCatalogName(int index,TCHAR *pszCatalog);
	DWORD	GetCatalogName(int index,TCHAR *pszCatalog);
	DWORD	UpdateCatalog(int index);
	DWORD	SetDetectedFlag(int index,BOOL bDetected);
	DWORD	GetDetectedFlag (int index,BOOL* bDetected);
	DWORD	SetDetectedInterval(int index,long lInterval) ;
	DWORD	GetDetectedInterval (int index,long* lInterval);
	DWORD	SetSubChannelRate(vector<DWS_SubChannelList *> pSubChannellists, long nSubChannel);
	DWORD   SetChannelEvent(int index,HANDLE hStateEvent);

	//  ICatalogState methods 
	DWORD SetCatalogStateEvent(int index,HANDLE hStateEvent);	 
	DWORD GetLastErrorCode(int index,int* nError);
	DWORD GetCurrentStatus(int index,int* pnStatus);

	// ------------------------------------------------------ Modified by zhenan_ji at 2005年3月31日 9:57:56
	//for CDataWrapperFilter interface, extern
	DWORD SetWrapParaSet(int index, DW_ParameterSet inPara );
	DWORD GetWrapParaSet(int index, DW_ParameterSet * outPara );


	DWORD AddVersionNumber(int index);
	DWORD ResetVersionNumber(int index);
	DWORD SetVersionNumber(int index,const char * inObjectKey, BYTE inObjectKeyLen, BYTE inVersion);
	DWORD GetVersionNumber(int index,const char * inObjectKey, BYTE inObjectKeyLen, BYTE * outVersion);
	DWORD SetObjectKeyLength(int index,BYTE inLength);
	DWORD GetObjectKeyLength(int index,BYTE *outLength);
	DWORD SetIndexDescLength(int index,BYTE inLength);
	DWORD GetIndexDescLength(int index,BYTE *outLength);	
	DWORD ReleaseAll();    

	PLAYER_STATE2 m_nState;

	//channel parameter
	DWORD m_dwChannelCount;
	CDODChannel			**m_pChannel;

	//portcast info
	ZQSBFIPPORTINFO		**m_pCaster;
	DWORD m_dwCasterCount;

	//save channelinfo for config zqBroadcastfilter()
	ZQSBFCHANNELINFO	**m_cchannelinfo;

	// GraphController
	IGraphBuilder *m_pGraph;
	IMediaControl *m_pMC;
	IMediaEventEx *m_pME;
	IBaseFilter *  m_pBroadcastfilter	;

	//ZIPPORTLIST m_UserIPPORT;
	
	//It is portID
	DWORD m_dwID;

	//The port's totalBitRate	and pmtPID
	DWORD m_dwTotalBitRate;
	WORD m_dwPmtPID;
	char m_szBCFTempFilePath[256];
	char m_sArrangePath[256];

	//sessionID,come from SRM
	int m_nSessionID;


	//save portinfo for SRM call getportinfo
	//PPortInfo	m_PortInfo;
private:

	DWORD ConnectFilters();    
	DWORD AddFilters();
	DWORD CreateInterfaceFromFilters();

private:

	//for setting ZQBroadcastfilter interface.
	IRateControl	*m_IRatecontrol;
	IIPSetting		*m_IIpsetting;
	BOOL m_bCallinitialition;
};
