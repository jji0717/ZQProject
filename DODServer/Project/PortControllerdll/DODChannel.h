#pragma once
#include ".\dodportctrl.h"
#include "interfaceDefination.h"
#include "IObjectControl.h"
#include "common.h"


class CDODChannel
{
public:
	CDODChannel(void);
	~CDODChannel(void);
	DWORD ReleaseFilter();

	DWORD CreateInterface();
		/*  ICatalogConfigure methods */
	DWORD 	SetCatalogName(TCHAR *pszCatalog);
	DWORD	GetCatalogName(TCHAR *pszCatalog);
	DWORD	UpdateCatalog();
	DWORD	SetDetectedFlag(BOOL bDetected);
	DWORD	GetDetectedFlag (BOOL* bDetected);
	DWORD	SetDetectedInterval(long lInterval) ;
	DWORD	GetDetectedInterval (long* lInterval);
	DWORD	SetSubChannelRate(vector<DWS_SubChannelList > pSubChannellists, long nSubChannel);
	DWORD   SetChannelEvent(HANDLE hStateEvent);

	/*  ICatalogState methods */
	DWORD SetCatalogStateEvent(HANDLE hStateEvent);	 
	DWORD GetLastErrorCode(int* nError);
	DWORD GetCurrentStatus(int* pnStatus);

// ------------------------------------------------------ Modified by zhenan_ji at 2005年3月31日 9:57:56
//for CDataWrapperFilter interface, extern
	DWORD SetWrapParaSet( DW_ParameterSet inPara );
	DWORD GetWrapParaSet( DW_ParameterSet * outPara );

	DWORD SetChannelPID(long inPID);
	DWORD GetChannelPID(long * outPID);
	DWORD SetChannelTag(const char * inTag, BYTE inLength);
	DWORD GetChannelTag(char * outTag, BYTE * outLength);
	DWORD AddVersionNumber();
	DWORD ResetVersionNumber();
	DWORD SetVersionNumber(const char * inObjectKey, BYTE inObjectKeyLen, BYTE inVersion);
	DWORD GetVersionNumber(const char * inObjectKey, BYTE inObjectKeyLen, BYTE * outVersion);
	DWORD SetObjectKeyLength(BYTE inLength);
	DWORD GetObjectKeyLength(BYTE *outLength);
	DWORD SetIndexDescLength(BYTE inLength);
	DWORD GetIndexDescLength(BYTE *outLength);	
	DWORD SetMode(int innMode);
	DWORD GetMode(BYTE *OutnMode);
// ------------------------------------------------------ Modified by zhenan_ji at 2005年8月8日 17:19:40

	DWORD SetEncrypt(BYTE inEncrypt);
	DWORD GetEncrypt(BYTE * outEncrypt);

	DWORD SetStreamCount(int inCount,char *pcharPath);
	DWORD GetStreamCount(int *OutCount);


/*
	DWORD SetObjectKey( const char * inObjectName, int inObjectNameLen, const char * inObjectKey, BYTE inObjectKeyLen );
	DWORD GetObjectKey( const char * inObjectName, int inObjectNameLen, char * outObjectKey, BYTE * outObjectKeyLen);
	DWORD SetMaxSizeOfSample(long inSize);
	DWORD GetMaxSizeOfSample(long *outSize);	
	DWORD SetMinSizeOfSample(long inSize);
	DWORD GetMinSizeOfSample(long *outSize);	*/

	IBaseFilter * m_pDataWatcherSource;
	IBaseFilter * m_pDataWrapper;

	ICatalogConfigure	*m_pCatalogConfigure;
	ICatalogState		*m_pCatalogState;
	IWrapperControl		*m_pWrapperControl;
};
