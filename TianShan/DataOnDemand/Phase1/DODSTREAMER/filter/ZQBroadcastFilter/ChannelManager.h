#pragma once
#include "stdafx.h"
#include <commdlg.h>
#include <initguid.h>
#include <stdio.h>
#include <atlconv.h>

#include <atlbase.h>
#include "resource.h"
#include <vector>

#include "BroadcastGuid.h"

class	CBroadcastFilter;
class	CBroadcastPin;
class	CBufferSend;
class	CChannel;

class CChannelManager
{

public:
	CBroadcastFilter   *m_pFilter;       // Methods for filter interfaces
	CCritSec *m_Lock;                // Main renderer critical section

	//HANDLE   m_hFile;               // Handle to file for dumping
	int  m_nState;
	LPOLESTR m_pFileName;    
	LPOLESTR m_pFileName2;           // The filename where we dump// The filename where we dump
	BOOL     m_fWriteError;

	int m_nPinNumber,m_nPortNumber,m_nTotalRate,m_checkTotalRate;
	int m_nPMTPID;
	char m_cDirName[MAX_PATH];	
// ------------------------------------------------------ Modified by zhenan_ji at 2005年6月9日 14:48:37
	BOOL m_bStop;

// ------------------------------------------------------ Modified by zhenan_ji at 2005年7月13日 17:38:33

	//all pin gather,dynamic create
	CBroadcastPin   **m_pPinArray;

	//It is all pins a gather,its each element is be created dynamic., Their numbers is equal to all pin's total numbers 
	CChannel		**m_pChannelArray;

	//send port,multiport Outport with some different speed..
	CBufferSend		**m_pSendArray;
	int m_nTotalperiod;
	unsigned char m_PatPmtBuffer[PATPMTBUFFERLEN];
	
	//Its value will be got from TotalRate /188.
	int m_nTotalPacketNumber;
private:
	CRITICAL_SECTION  csMyCriticalSection;

public:
	CChannelManager();
	~CChannelManager();
	
	int GetFileName(char *cFileName,int pinIndex);
	int CreatePin(int pinIndex,char ctype,int PID);

	//
	HRESULT changefileFlag(int index,DWORD dwFileSize,char *strfilename);
	HRESULT CreatePatPmt();
	PBYTE GetBufferModeBuf(int nIndex);
	HRESULT SetBufferModeLen(int nIndex,int RecvLength);
	BOOL GetPinType(int nIndex);
	// Overriden to say what interfaces we support where
	HRESULT Run();

//	WORD GetRateFromList(WORD pid);
	//HRESULT socketconnect(char* szIp,int nPort);  
	// Open and write to the file
	//HRESULT Refresh();
	//HRESULT CommitBuf(WORD nPid,WORD nType,BYTE* pBuf,WORD nBufLen,WORD nRate);
	HRESULT ReadFiletoBufferList(LPCTSTR pFileName);
	HRESULT Stop();	
};