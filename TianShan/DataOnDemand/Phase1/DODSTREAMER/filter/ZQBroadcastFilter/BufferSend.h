#pragma once

#include "scsocket.h"
//#include <vector.h>
#include "BroadcastGuid.h"


// ------------------------------------------------------ Modified by zhenan_ji at 2005Äê4ÔÂ11ÈÕ 18:49:29
//188*225=42300,period=100 ms,rate=423KB
//188*215=40420,period=80  ms,rate=505KB
//#define MAXSENDDATASIZE (1024*1024*3)
//#define MIN_SEND_LEN (188*128)

class CChannelManager;

typedef struct CCIdnfo1
{
	LONGLONG nPerfCnt;
	int nSendPeriod; 
	int nCC;
	BOOL m_bEnable;
	USHORT pid;
}CCInfo1;

typedef std::vector<CCInfo1> CCVector;

class CBufferSend :
	public CAMThread
{
protected:
	DWORD ThreadProc(void);
public:
	CBufferSend();
	~CBufferSend();
	/* CAMthread state*/
	enum Command {CMD_INIT=2, CMD_RUN, CMD_STOP};

	HRESULT Init(void);
	HRESULT Run(void);
	HRESULT Stop(void);//  { return CallWorker(0); }
	
	//create cc vector,
	HRESULT RefreshParameter(void);

	//core process thread
	virtual void Process(void);

	int m_wSendType;		//TCPIP=0, UDP=1, MULTICAST=2, BROADCAST

    char m_cSourceIp[16];	//This IP address may be used bind of a socket.
	int m_wSourcePort;
	char m_cDestIp[16];		//This IP address is destination machine
	int m_wDestPort;
	CChannelManager *m_pManager; //Its father,buffersend 's director
	int m_nToralperiod;			//Here ,a rate of sending is change a sendperiod,interval 
	int m_nIndex;

	// added by Cary
	in_addr		m_destAddr;	// destaddr

private:
	CSCUDPSocket m_Socket;
	void SendData(CHAR* pBuffer,int len);
	HANDLE m_hStopEvent;
	//BOOL m_StopFlag;
	LONGLONG g_nFreq;
	CRITICAL_SECTION m_SendCriticalSection;
	BYTE *m_pSendBuf;
	CCVector  m_ccArr;	// MAXPINNUMBER==pmtpmt index;
	CSCTCPSocket* m_TCPSocke;
	CSCMulticastSocket *m_pMulitcastSocket;  // major  socket send call.
	int m_SendSize;		//It show send buffer size once a time
	char m_outfileName[MAX_PATH]; //temp file .for check send data .lose packet etc.
	BOOL m_ISWriteFile;			// create temp file flag.

	int GetContinueCount(USHORT pid);

};
