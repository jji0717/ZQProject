#pragma once
#include "scqueue.h"
class CChannelManager;

class CChannel
{
public:
	CChannel();
	~CChannel();

	//channel pid,It will determine CC's value.
	int m_nPID;

	//Them will used switch in operation(read and write),avoid recource conflict;
	char m_CurrFileName[MAX_PATH];
	char m_BackFileName[MAX_PATH];	

	//This flag will used forbidden send current channel's data.
	BOOL m_bEnable;

	// read / write switch flag
	BOOL m_bFileChangeFlag;

	//judge while file was being used now.m_CurrFileName or m_BackFileName
	BOOL m_bFirstIsUsing;

	// send rate;
	int m_nRate;

	// its value is TRUE,channel working is read/write file mode.else is memory mode
	BOOL m_bIsFileMode;
	int Run();
	int Stop();
	HANDLE m_hStopSendEvent;	

	int Init();
	CChannelManager *m_pManager;

	//a leaguer of the queue buffer size.
	int m_nblockSize;
	int m_nRepeatTime;
	
	// Now ,it is used create PMT;
	int m_nStreamType;
	char m_Descrbuff[4];

	// for create PMT,description field
	unsigned char m_DescTag;

	CRITICAL_SECTION m_channelCriticalSection;

	//Their function are equal to m_CurrFileName basic.but defferent working mode.
	BYTE * m_pcCurrData;
	BYTE * m_pcBackData;

	//Their function are equal to m_nblockSize basic
	int m_npCurrDataLen;
	int m_npBackDataLen; 

	//exchange buffer for write and read,temp member.
	BYTE *m_pBuffer;

	int m_nbufferLength;
	HANDLE m_hFile;
	char m_cDescriptor[4];

// ------------------------------------------------------ Modified by zhenan_ji at 2005Äê11ÔÂ3ÈÕ 11:45:08
	int m_nStreamCount;

	BOOL m_bOccurError;

	//CBufferSend will extract m_Queue's element output .
	CSCMemoryBlockQueue* *m_Queue;

	//a size of total queue 's buffer ;It forbidden queue to engross more memory without anymore limit,
	int m_nQueueSize;

private:
	//read data to queue from the curfile or backfile.
	HANDLE m_hSendThread;
	
//////////////////////////////////////////////////////////////////////////
// added by Cary
#if !defined(_NO_FIX_DOD)
	static DWORD __stdcall _ThreadProc(PVOID param);
	DWORD ProcessFile();
	DWORD ProcessMemory();

#ifdef _USE_OLD_QUEUE
	void* m_multiWaitHandle;
#else
	CSCMemoryBlockQueue::QueueGroup*	m_queueGroup;
#endif // #ifdef _USE_OLD_QUEUE

#else
	friend DWORD WINAPI DODMemoryListhread(LPVOID lpParam);
	friend DWORD WINAPI DODFileListhread(LPVOID lpParam);
#endif

protected:
	HANDLE*	m_semReadQueue;		// the queue lock

public:
	CSCMemoryBlock* popQueue(int index);
};
