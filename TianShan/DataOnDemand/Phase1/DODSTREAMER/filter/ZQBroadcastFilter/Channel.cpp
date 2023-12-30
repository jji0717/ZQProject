#include "stdafx.h"
#include "channel.h"
#include "scqueue.h"
#include "ChannelManager.h"
#include <assert.h>

#define BLOCKSIZE (188*10)

extern unsigned int channel_queue_size;

// #define QUEUESZIE 32
#define QUEUESZIE	channel_queue_size;

typedef CSCMemoryBlockQueue* PBROADMEMORYQUEUE;

// modified by Cary
#ifndef _NO_FIX_DOD

CChannel::CChannel()
{
	m_nPID=0;	
	m_nRate=0;
	m_bEnable=TRUE;
	m_bFileChangeFlag=FALSE;
	m_CurrFileName[0]='\0';
	m_BackFileName[0]='\0';	
	m_bIsFileMode=TRUE;
	m_hSendThread=NULL;
	//	m_bIsStop =FALSE;
	//	m_hAccessEvent = NULL;
	m_pManager=NULL;
	m_Queue=NULL;
	m_hFile=NULL;
	m_pcCurrData=NULL;
	m_pcBackData=NULL;    	
	// Modified by zhenan_ji at 2005年3月21日 14:58:03
	m_npCurrDataLen=0;
	m_npBackDataLen=0;    
	m_nRepeatTime=0;
	m_nblockSize=BLOCKSIZE;
	m_nQueueSize=QUEUESZIE;
	m_bFirstIsUsing=FALSE;
	m_nStreamType=0x00;
	m_DescTag=0x0f;
	m_pBuffer=NULL;
	m_nbufferLength=0;
	m_nStreamCount=0;
	m_bOccurError=FALSE;
	InitializeCriticalSection(&m_channelCriticalSection);

#ifdef _USE_OLD_QUEUE
	// Added by Cary
	m_multiWaitHandle = NULL;
#else
	m_queueGroup = NULL;
#endif
}

CChannel::~CChannel()
{
	DeleteCriticalSection(&m_channelCriticalSection);

#if !defined(_NO_FIX_DOD)
	for(int i = 0; i < m_pManager->m_nPortNumber; i ++) {
		if (m_Queue[i] != NULL) {
			delete m_Queue[i];
			m_Queue[i] = NULL;
		}
	}
#else // #if !defined(_NO_FIX_DOD)
	for(int i = 0; i < m_pManager->m_nPortNumber; i ++)
	{
		if (m_Queue[i] != NULL)
		{
			INT32 Size = m_Queue[i]->Size();
			while(Size)
			{
				// CSCMemoryBlock::FreeBlock(m_Queue[i]->Front()->GetBlock());
				FredPtr blkptr;
				m_Queue[i]->Pop(blkptr);
				Size--;
			}
			delete m_Queue[i];
			m_Queue[i] = NULL;
		}
	}

#endif // #if !defined(_NO_FIX_DOD)

	if (m_Queue)
	{
		delete  m_Queue;
		m_Queue=NULL;
	}

	if (m_hFile)  
	{
		CloseHandle(m_hFile); 
		m_hFile=NULL;
	}

	if(m_pcCurrData)
	{
		delete[] m_pcCurrData;
		m_pcCurrData = NULL; 
	}	

	if(m_pcBackData)
	{
		delete[] m_pcBackData;
		m_pcBackData = NULL;
	}

	// Modified by zhenan_ji at 2005年4月15日 11:11:47
	DeleteFile(m_CurrFileName);
	DeleteFile(m_BackFileName);

#ifdef _USE_OLD_QUEUE
	// Added by Cary
	if (m_multiWaitHandle) {
		CSCMemoryBlockQueue::CloseMultiQueues(m_multiWaitHandle);
	}
#else

	if (m_queueGroup)
		CSCMemoryBlockQueue::closeQueueGroup(m_queueGroup);
#endif
}

int CChannel::Init()
{
	if(m_pManager->m_nPortNumber <0)
		return 1;
	int nCount=m_pManager->m_nPortNumber;

	m_Queue = new CSCMemoryBlockQueue*[nCount];

	for (int i=0;i<nCount;i++)
	{
		m_Queue[i]=new CSCMemoryBlockQueue(m_nQueueSize);
		glog(ISvcLog::L_DEBUG,"Queue: %8x,ch_pid: %d",(DWORD)m_Queue[i], m_nPID);
	}

	if(m_bIsFileMode)
	{

		m_hFile = CreateFile(m_CurrFileName, GENERIC_READ, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);

		m_bFirstIsUsing=TRUE;

		// Modified by zhenan_ji at 2005年12月14日 11:25:33
		// m_bFirstIsUsing = FALSE;
		if (m_hFile == INVALID_HANDLE_VALUE) 
		{			
			DWORD dwErr = GetLastError();
			glog(ISvcLog::L_ERROR, "CreateFile(%s) error err=%d", 
				m_CurrFileName, dwErr);

			return HRESULT_FROM_WIN32(dwErr);
		}
	} 
	else
	{		
		m_pcCurrData=new BYTE[BUFTYPEBUFFERLEN];
		m_pcBackData=new BYTE[BUFTYPEBUFFERLEN];

		m_pBuffer=m_pcBackData;
		m_nbufferLength=0;
	}	
	return 0;
}

int CChannel::Run()
{	
	DWORD IDThread;
	m_bOccurError = FALSE;
	m_hStopSendEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_hSendThread = CreateThread(NULL, 0, _ThreadProc, this, 0, &IDThread);
	return 0;
}

int CChannel::Stop() 
{
	glog(ISvcLog::L_INFO, "CChannel::Stop()");

	//m_bIsStop = TRUE;
	if(m_hStopSendEvent)
	{
		SetEvent(m_hStopSendEvent);		
	}

	if(m_hSendThread)
	{
		glog(ISvcLog::L_DEBUG, "CChannel::Stop(WaitForSingleObject) file:%s", 
			m_CurrFileName);

		WaitForSingleObject(m_hSendThread, INFINITE);

		if(m_hSendThread)
		{
			CloseHandle(m_hSendThread);
			m_hSendThread = NULL;
		}
		glog(ISvcLog::L_INFO,"CChannel::Stop(end) file:%s",m_CurrFileName);
	}

	if (m_hStopSendEvent)
	{
		CloseHandle(m_hStopSendEvent);
		m_hStopSendEvent=NULL;
	}
	return 0;
}

DWORD CChannel::_ThreadProc(PVOID param)
{
	CChannel* pThis = (CChannel* )param;
	if (pThis->m_bIsFileMode)
		return pThis->ProcessFile();
	else
		return pThis->ProcessMemory();
}

DWORD CChannel::ProcessFile() 
{
	if (strlen(m_CurrFileName) == 0) {
		glog(ISvcLog::L_ERROR, "%s:\tthe length of m_CurrFileName is 0",
			__FUNCTION__);
		return -1;
	}

	if (strlen(m_BackFileName) == 0) {
		glog(ISvcLog::L_ERROR, "%s:\tthe length of m_BackFileName is 0",
			__FUNCTION__);
		return -1;
	}

	BYTE *m_pbMem = new BYTE[m_nblockSize];

	int blocksize=m_nblockSize;

	if (blocksize ==0) {
		glog(ISvcLog::L_ERROR, "blocksize=0 :PID%dbak error ,sending rate error",
			m_nPID);
		return -1;
	}

	int nCurrent = 0;

	// 读完文件后检查是否需要切换
	BOOL IsNeedFileCheckChange = FALSE;

	// 文件已经切换成功
	BOOL bFileChangeFinished = FALSE;

	// 上一次 push 成功
	bool blockPushed = true;

	FredPtr pFredPtr = NULL;
	unsigned long read = 0;

#ifdef _USE_OLD_QUEUE
	if (m_multiWaitHandle == NULL) {

		if (m_multiWaitHandle) {
			CSCMemoryBlockQueue::CloseMultiQueues(m_multiWaitHandle);
		}


		m_multiWaitHandle = CSCMemoryBlockQueue::CreateMultiQueues(
			(WaitableQueue<FredPtr> **)m_Queue, m_pManager->m_nPortNumber);
	}

#else
	if (m_queueGroup == NULL) {

		m_queueGroup = CSCMemoryBlockQueue::createQueueGroup(
			(WaitableQueue2<FredPtr> **)m_Queue, m_pManager->m_nPortNumber);
	}

#endif

	while (!m_bOccurError)
	{
		// 检查退出
		if(WaitForSingleObject(m_hStopSendEvent,0)==WAIT_OBJECT_0) {
			delete[] m_pbMem;
			m_pbMem = NULL;
			glog(ISvcLog::L_INFO,"CChannel::StopSendEvent");
			return 0;
		}

		bFileChangeFinished = FALSE;

		// 检查文件是否改变了
		if (IsNeedFileCheckChange) {
			// glog(ISvcLog::L_DEBUG_DETAIL, "CChannel::IsNeedFileCheckChange == true");

			IsNeedFileCheckChange = FALSE;

			// EnterCriticalSection(&(m_channelCriticalSection));

			if(m_bFileChangeFlag) {

				// 需要切换文件

				if (m_hFile != INVALID_HANDLE_VALUE) {
					CloseHandle(m_hFile);
					m_hFile = INVALID_HANDLE_VALUE;
				}

				if(m_bFirstIsUsing == FALSE) {

					m_hFile = CreateFile(m_CurrFileName, GENERIC_READ, 
						FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

					if (m_hFile == INVALID_HANDLE_VALUE) 
					{			
						DWORD dwErr = GetLastError();
						m_bOccurError = TRUE;

						glog(ISvcLog::L_ERROR,"File Changed,file=PID%dcur errorcode=%d", 
							m_nPID, dwErr);

						// LeaveCriticalSection(&(m_channelCriticalSection));		
						continue;
					}

					bFileChangeFinished = TRUE;

					glog(ISvcLog::L_INFO,"File Changed :PID%dcur",m_nPID);

				} else {

					m_hFile = CreateFile(m_BackFileName, GENERIC_READ, FILE_SHARE_READ, 
						NULL, OPEN_EXISTING, NULL, NULL);

					if (m_hFile == INVALID_HANDLE_VALUE) 
					{			
						DWORD dwErr = GetLastError();
						m_bOccurError=TRUE;

						glog(ISvcLog::L_ERROR,"File Changed,file=PID%dbak errorcode=%d", 
							m_nPID, dwErr);
						// LeaveCriticalSection(&(m_channelCriticalSection));		
						continue;
					}

					bFileChangeFinished = TRUE;

					glog(ISvcLog::L_INFO, "File Changed :PID%dbak",m_nPID);
				}

				m_bFileChangeFlag = FALSE;
				m_bFirstIsUsing = !m_bFirstIsUsing;
			}

			// LeaveCriticalSection(&(m_channelCriticalSection));		
		}

		try	{
			if (m_hFile == INVALID_HANDLE_VALUE) {
				m_bOccurError = TRUE;
				delete[] m_pbMem;
				m_pbMem = NULL;	

				glog(ISvcLog::L_ERROR, "Read filedata to queue::but filehandle is null");
				//LeaveCriticalSection(&(m_channelCriticalSection));		
				return 1;		
			}			

#ifdef _USE_OLD_QUEUE

			DWORD index = CSCMemoryBlockQueue::WaitForMultiQueues(m_multiWaitHandle, 3000);
#else
			DWORD index;

			if (blockPushed)
				index = m_queueGroup->preparePush(3000);
#endif

			if (index == WAIT_FAILED) {

				glog(ISvcLog::L_ERROR, "WaitForMultiQueues() failed");
				return WAIT_FAILED;

			} else if (index == WAIT_TIMEOUT) {

				glog(ISvcLog::L_DEBUG_DETAIL, "CSCMemoryBlockQueue::WaitForMultiQueues() failed");
				continue;

			}

			assert(index >= 0 && index < WAIT_TIMEOUT);

			CSCMemoryBlockQueue* queue = m_Queue[index];

			/*
			DWORD index = 0;
			CSCMemoryBlockQueue* queue = m_Queue[0];
			*/

			if (bFileChangeFinished) {

				queue->m_WFileOffset = 0;
				blockPushed = true;
				pFredPtr = NULL;

			}

			if (blockPushed) {

				SetFilePointer (m_hFile, queue->m_WFileOffset, NULL, 
					FILE_BEGIN);

				if (m_nblockSize <=0)
				{
					glog(ISvcLog::L_ERROR, 
						"blocksize=0 :PID%dbak error ,sending rate error", 
						m_nPID);

					//LeaveCriticalSection(&(m_channelCriticalSection));		
					return 0x12;
				}
				
				if( !ReadFile(m_hFile, m_pbMem, m_nblockSize, &read, NULL)) 		
				{
					DWORD dwErrorCode = ::GetLastError();
					m_bOccurError = TRUE;
					delete[] m_pbMem;
					m_pbMem = NULL;	

					glog(ISvcLog::L_ERROR,":ReadThread:readFile error,error code is %d", 
						dwErrorCode);

					//LeaveCriticalSection(&(m_channelCriticalSection));		
					return DODERROR_FILE_DATA_PART_LOST;
				}

				// glog(ISvcLog::L_DEBUG_DETAIL, "ReadFile readResult is (%d)  ", read);

				if(read > 0)
				{
					if(m_pbMem[0]!=0x47)
					{
						delete[] m_pbMem;
						m_pbMem = NULL;
						m_bOccurError = TRUE;

						glog(ISvcLog::L_ERROR,
							"channelPID(%d):Data is error(Read file),It should euqal to ts_sync", 
							m_nPID);
						
						return DODERROR_FILE_DATA_PART_LOST;
					}

					TCHAR * pcData = CSCMemoryBlock::AllocBlock(read);
					if (pcData == NULL)
					{	
						DWORD dwErr = GetLastError();
						m_bOccurError = TRUE;
						delete[] m_pbMem;
						m_pbMem = NULL;	

						glog(ISvcLog::L_ERROR, 
							"MemoryBlock::AllocBlock error,PID(%d) errorcode=%d", 
							m_nPID,dwErr);

						return DODERROR_FILE_DATA_PART_LOST;
					}

					memcpy(pcData,m_pbMem,read);
					pFredPtr = new CSCMemoryBlock( pcData, read);
					// pFredPtr = mBlock;
					// EnterCriticalSection(&(m_channelCriticalSection));

					glog(ISvcLog::L_DEBUG_DETAIL,"Push before queue: %8x,CI: %d,QS: %d",
						(DWORD)queue, index, queue->Size());

#ifdef _USE_OLD_QUEUE
					if (queue->Push(pFredPtr, 3000)) {
#else
					if (m_queueGroup->doPush(pFredPtr, index)) {
#endif
						queue->m_WFileOffset += read;
						pFredPtr = NULL;
					} else {
						glog(ISvcLog::L_DEBUG_DETAIL,"push time out");
						blockPushed = false;
					}

					glog(ISvcLog::L_DEBUG_DETAIL,"Push after queue: %8x,CI: %d,QS: %d",
						(DWORD)queue, index, queue->Size());

					// LeaveCriticalSection(&(m_channelCriticalSection));		
				} 

#ifndef _USE_OLD_QUEUE
				else {

					m_queueGroup->cancelPush(index);
				}
#endif

				if((int)read < blocksize)
				{
					IsNeedFileCheckChange = TRUE;
					queue->m_WFileOffset = 0;

					DWORD fileSize = GetFileSize(m_hFile, NULL);
					if (fileSize != INVALID_FILE_SIZE && fileSize > 0) {
						glog(ISvcLog::L_DEBUG_DETAIL, 
							"********** PID = %d read file finish **********", 
							m_nPID);
					} else // 文件 size 为 0
						Sleep(1);
				}

			} else {

				assert(pFredPtr != NULL);
				// EnterCriticalSection(&(m_channelCriticalSection));
				glog(ISvcLog::L_DEBUG_DETAIL,"Push before queue: %8x,CI: %d,QS: %d",
					(DWORD)queue, index, queue->Size());

#ifdef _USE_OLD_QUEUE
				if (queue->Push(pFredPtr, 3000)) {
#else
				if (m_queueGroup->doPush(pFredPtr, index)) {
#endif
					queue->m_WFileOffset += read;
					pFredPtr = NULL;
					blockPushed = true;
				} else {
					glog(ISvcLog::L_DEBUG_DETAIL,"push time out");
				}

				glog(ISvcLog::L_DEBUG_DETAIL,"Push after queue: %8x,CI: %d,QS: %d",
					(DWORD)queue, index, queue->Size());

				// LeaveCriticalSection(&(m_channelCriticalSection));
			}

		}
		catch (...) 
		{
			
			DWORD dwErr = GetLastError();

			glog(ISvcLog::L_ERROR,
				"channelPID(%d); Read filedata to queue::exception! errorcode=%d", 
				m_nPID, dwErr);

			m_bOccurError = TRUE;
			delete[] m_pbMem;
			m_pbMem = NULL;	

			assert(false);

			return -1;
		}

	}

	return -1;
}

DWORD CChannel::ProcessMemory()
{
	assert(false);

	if(m_pcCurrData == NULL)
		return 1;
	if(m_pcBackData == NULL)
		return 1;
	int i, j;
	BOOL IsNeed;
	int offset = 0;
	BYTE *m_pbMem = new BYTE[m_nblockSize];

	for(;;)
	{	
		IsNeed = FALSE;
		for(;;)
		{
			int nCount = m_pManager->m_nPortNumber;
			Sleep(1);

			for(i=0; i < nCount; i ++)
			{
				CSCMemoryBlockQueue* queue=m_Queue[i];
				j=queue->Size();
				if(j<m_nQueueSize)
					IsNeed = TRUE;
				if(IsNeed)
					break;
			}
			if(IsNeed)
				break;
			else
				Sleep(1);				
		}

		int read=m_nbufferLength-offset;
		//ReadFile(m_hFile, m_pbMem, m_nblockSize, &read, NULL); 		
		memcpy(m_pbMem,m_pBuffer+offset,read);

		if(read < m_nblockSize)
		{	
			EnterCriticalSection(&(m_channelCriticalSection));
			IsNeed=m_bFileChangeFlag;
			LeaveCriticalSection(&(m_channelCriticalSection));			
			if(IsNeed)
			{						
				EnterCriticalSection(&(m_channelCriticalSection));
				m_bFileChangeFlag=FALSE;
				m_bFirstIsUsing=!(m_bFirstIsUsing);
				IsNeed=m_bFirstIsUsing;
				LeaveCriticalSection(&(m_channelCriticalSection));	
				if(read >0)
				{
					int nCount=m_pManager->m_nPortNumber;

					for(int i=0; i<nCount; i++)
					{
						CSCMemoryBlockQueue* queue=m_Queue[i];

						TCHAR * pcData = CSCMemoryBlock::AllocBlock(read);
						memcpy(pcData,m_pbMem,read);
						CSCMemoryBlock  * mBlock = new CSCMemoryBlock( pcData, read );
						//	mBlock->m_iBlockNumber = iPackageNumber;
						FredPtr pFredPtr( mBlock );
						queue->Push(pFredPtr, INFINITE);
					}
				}

				if(IsNeed)
				{
					m_pBuffer = m_pcCurrData;
					m_nbufferLength = m_npCurrDataLen;
				}
				else
				{
					m_pBuffer=m_pcBackData;
					m_nbufferLength=m_npBackDataLen;
				}
			}
			offset=0;				
		}
		else
		{				
			offset+=m_nblockSize;
			int nCount=m_pManager->m_nPortNumber;

			for(int i=0;i<nCount;i++)
			{
				CSCMemoryBlockQueue* queue=m_Queue[i];

				TCHAR * pcData = CSCMemoryBlock::AllocBlock(m_nblockSize);
				memcpy(pcData,m_pbMem,m_nblockSize);

				CSCMemoryBlock  * mBlock = new CSCMemoryBlock( pcData, m_nblockSize );
				//	mBlock->m_iBlockNumber = iPackageNumber;
				FredPtr pFredPtr( mBlock );
				queue->Push(pFredPtr, INFINITE);
			}
		}		
		if(WaitForSingleObject(m_hStopSendEvent,0)==WAIT_OBJECT_0)
		{
			delete[] m_pbMem;
			m_pbMem=NULL;
			TCHAR szMsg[MAX_PATH];wsprintf(szMsg,"CChannel::StopSendEvent");
			LogMyEvent(1,1,szMsg);
			return 1;
		}
		Sleep(1);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////

#else // #ifndef _NO_FIX_DOD

CChannel::CChannel()
{
	m_nPID=0;	
	m_nRate=0;
	m_bEnable=TRUE;
	m_bFileChangeFlag=FALSE;
	m_CurrFileName[0]='\0';
	m_BackFileName[0]='\0';	
	m_bIsFileMode=TRUE;
	m_hSendThread=NULL;
//	m_bIsStop =FALSE;
//	m_hAccessEvent = NULL;
	m_pManager=NULL;
	m_Queue=NULL;
	m_hFile=NULL;
	m_pcCurrData=NULL;
	m_pcBackData=NULL;    	
// ------------------------------------------------------ Modified by zhenan_ji at 2005年3月21日 14:58:03
	m_npCurrDataLen=0;
	m_npBackDataLen=0;    
	m_nRepeatTime=0;
	m_nblockSize=BLOCKSIZE;
	m_nQueueSize=QUEUESZIE;
	m_bFirstIsUsing=FALSE;
	m_nStreamType=0x00;
	m_DescTag=0x0f;
	m_pBuffer=NULL;
	m_nbufferLength=0;
	m_nStreamCount=0;
	m_bOccurError=FALSE;
	InitializeCriticalSection(&m_channelCriticalSection);
}

CChannel::~CChannel()
{
//	Stop();
	DeleteCriticalSection(&m_channelCriticalSection);
/*	if(m_hStopSendEvent)
	{
		CloseHandle(m_hStopSendEvent);
		m_hStopSendEvent=NULL;
	}

	if(m_hAccessEvent)
	{
		CloseHandle(m_hAccessEvent);
		m_hAccessEvent=NULL;
	}
*/

	for(int i=0;i<m_pManager->m_nPortNumber;i++)	
	{
		if (m_Queue[i]!=NULL)
		{
			INT32 Size = m_Queue[i]->Size();
			while(Size)
			{
			//	CSCMemoryBlock::FreeBlock(m_Queue[i]->Front()->GetBlock());
				m_Queue[i]->Pop();
				Size--;
			}
			delete m_Queue[i];
			m_Queue[i] = NULL;
		}
	}

	if (m_Queue)
	{
		delete  m_Queue;
		m_Queue=NULL;
	}

	if (m_hFile)  
	{
		CloseHandle(m_hFile); 
		m_hFile=NULL;
	}

	if(m_pcCurrData)
	{
		delete[] m_pcCurrData;
		m_pcCurrData = NULL; 
	}	
	
	if(m_pcBackData)
	{
		delete[] m_pcBackData;
		m_pcBackData = NULL;
	}	
// ------------------------------------------------------ Modified by zhenan_ji at 2005年4月15日 11:11:47
	DeleteFile(m_CurrFileName);
	DeleteFile(m_BackFileName);
//	TCHAR szMsg[MAX_PATH];wsprintf(szMsg,"BroadcastPin::DeleteFile m_CurrFileName");		LogToFile(2,szMsg);
}

DWORD WINAPI DODMemoryListhread(LPVOID lpParam) 
{
	CChannel *ch=(CChannel *)lpParam;

	if(ch->m_pcCurrData==NULL)
		return 1;
	if(ch->m_pcBackData==NULL)
		return 1;
	int i,j;
	BOOL IsNeed;
	int offset=0;
	BYTE *m_pbMem = new BYTE[ch->m_nblockSize];

	for(;;)
	{	
		IsNeed=FALSE;
		for(;;)
		{
			int nCount=ch->m_pManager->m_nPortNumber;
			Sleep(1);

			for(i=0;i<nCount;i++)
			{
				CSCMemoryBlockQueue* queue=ch->m_Queue[i];
				j=queue->Size();
				if(j<ch->m_nQueueSize)
					IsNeed=TRUE;
				if(IsNeed)
					break;
			}
			if(IsNeed)
				break;
			else
				Sleep(1);				
		}

		int read=ch->m_nbufferLength-offset;
		//ReadFile(ch->m_hFile, m_pbMem, ch->m_nblockSize, &read, NULL); 		
		memcpy(m_pbMem,ch->m_pBuffer+offset,read);

		if(read < ch->m_nblockSize)
		{	
			EnterCriticalSection(&(ch->m_channelCriticalSection));
			IsNeed=ch->m_bFileChangeFlag;
			LeaveCriticalSection(&(ch->m_channelCriticalSection));			
			if(IsNeed)
			{						
				EnterCriticalSection(&(ch->m_channelCriticalSection));
				ch->m_bFileChangeFlag=FALSE;
				ch->m_bFirstIsUsing=!(ch->m_bFirstIsUsing);
				IsNeed=ch->m_bFirstIsUsing;
				LeaveCriticalSection(&(ch->m_channelCriticalSection));	
				if(read >0)
				{
					int nCount=ch->m_pManager->m_nPortNumber;

					for(int i=0;i<nCount;i++)
					{
						CSCMemoryBlockQueue* queue=ch->m_Queue[i];

						TCHAR * pcData = CSCMemoryBlock::AllocBlock(read);
						memcpy(pcData,m_pbMem,read);
						CSCMemoryBlock  * mBlock = new CSCMemoryBlock( pcData, read );
						//	mBlock->m_iBlockNumber = iPackageNumber;
						FredPtr pFredPtr( mBlock );
						queue->Push(pFredPtr);
					}
				}

				if(IsNeed)
				{
					ch->m_pBuffer=ch->m_pcCurrData;
					ch->m_nbufferLength=ch->m_npCurrDataLen;
				}
				else
				{
					ch->m_pBuffer=ch->m_pcBackData;
					ch->m_nbufferLength=ch->m_npBackDataLen;
				}
			}
			offset=0;				
		}
		else
		{				
			offset+=ch->m_nblockSize;
			int nCount=ch->m_pManager->m_nPortNumber;

			for(int i=0;i<nCount;i++)
			{
				CSCMemoryBlockQueue* queue=ch->m_Queue[i];

				TCHAR * pcData = CSCMemoryBlock::AllocBlock(ch->m_nblockSize);
				memcpy(pcData,m_pbMem,ch->m_nblockSize);

				CSCMemoryBlock  * mBlock = new CSCMemoryBlock( pcData, ch->m_nblockSize );
				//	mBlock->m_iBlockNumber = iPackageNumber;
				FredPtr pFredPtr( mBlock );
				queue->Push(pFredPtr);
			}
		}		
		if(WaitForSingleObject(ch->m_hStopSendEvent,0)==WAIT_OBJECT_0)
		{
			delete[] m_pbMem;
			m_pbMem=NULL;
			TCHAR szMsg[MAX_PATH];wsprintf(szMsg,"CChannel::StopSendEvent");		LogMyEvent(1,1,szMsg);
			return 1;
		}
		Sleep(1);
	}
	return 0;
}
DWORD WINAPI DODFileListhread(LPVOID lpParam) 
{
	CChannel *ch=(CChannel *)lpParam;

	if(strlen(ch->m_CurrFileName)==0)
		return 1;
	if(strlen(ch->m_BackFileName)==0)
		return 1;
	int j;
	BOOL IsNeedFileCheckChange; 
	BYTE *m_pbMem = new BYTE[ch->m_nblockSize];
//	BYTE *pbMem = new BYTE[ch->m_nblockSize];
	TCHAR szMsg[MAX_PATH];

// ------------------------------------------------------ Modified by zhenan_ji at 2005年11月23日 19:39:44
	int blocksize=ch->m_nblockSize;
	if (blocksize ==0)
	{
		wsprintf(szMsg,"blocksize=0 :PID%dbak error ,sending rate error",ch->m_nPID);		LogMyEvent(1,1,szMsg);
		return 0x12;
	}
	int nCurrent=0;
	IsNeedFileCheckChange=FALSE;
	BOOL bFileChangeFinished=FALSE;
	for(;ch->m_bOccurError==FALSE;)
	{
		if(WaitForSingleObject(ch->m_hStopSendEvent,0)==WAIT_OBJECT_0)
		{
			delete[] m_pbMem;
			m_pbMem=NULL;	
			wsprintf(szMsg,"CChannel::StopSendEvent");		LogMyEvent(1,1,szMsg);
			return 0x12;
		}
		bFileChangeFinished=FALSE;
		if (IsNeedFileCheckChange)
		{
			//wsprintf(szMsg,"CChannel::IsNeedFileCheckChange");		LogMyEvent(1,1,szMsg);
			IsNeedFileCheckChange=FALSE;
			EnterCriticalSection(&(ch->m_channelCriticalSection));
			if(ch->m_bFileChangeFlag)
			{
				if (ch->m_hFile != INVALID_HANDLE_VALUE) 
				{
					CloseHandle(ch->m_hFile);
					ch->m_hFile = INVALID_HANDLE_VALUE;
				}

				if(ch->m_bFirstIsUsing==FALSE)
				{
					ch->m_hFile = CreateFile(ch->m_CurrFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
					if (ch->m_hFile == INVALID_HANDLE_VALUE) 
					{			
						DWORD dwErr = GetLastError();
						ch->m_bOccurError=TRUE;
						wsprintf(szMsg,"File Changed,file=PID%dcur errorcode=%d",ch->m_nPID,dwErr);					LogMyEvent(1,1,szMsg);
						LeaveCriticalSection(&(ch->m_channelCriticalSection));		
						continue;
					}
					bFileChangeFinished=TRUE;
					wsprintf(szMsg,"File Changed :PID%dcur",ch->m_nPID);		LogMyEvent(1,1,szMsg);
				}
				else
				{
					ch->m_hFile = CreateFile(ch->m_BackFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
					if (ch->m_hFile == INVALID_HANDLE_VALUE) 
					{			
						DWORD dwErr = GetLastError();
						ch->m_bOccurError=TRUE;
						wsprintf(szMsg,"File Changed,file=PID%dbak errorcode=%d",ch->m_nPID,dwErr);					LogMyEvent(1,1,szMsg);
						LeaveCriticalSection(&(ch->m_channelCriticalSection));		
						continue;
					}
					bFileChangeFinished=TRUE;
					wsprintf(szMsg,"File Changed :PID%dbak",ch->m_nPID);		LogMyEvent(1,1,szMsg);
				}
				ch->m_bFileChangeFlag=FALSE;
				ch->m_bFirstIsUsing=!(ch->m_bFirstIsUsing);
			}
			LeaveCriticalSection(&(ch->m_channelCriticalSection));		

		}

		try
		{
			if (ch->m_hFile == INVALID_HANDLE_VALUE) 
			{
				ch->m_bOccurError=TRUE;
				delete[] m_pbMem;
				m_pbMem=NULL;	
				wsprintf(szMsg,"Read filedata to queue::but filehandle is null");LogMyEvent(1,1,szMsg);
				//LeaveCriticalSection(&(ch->m_channelCriticalSection));		
				return 1;		
			}
			
			int nCount=ch->m_pManager->m_nPortNumber;
			for(int i=0;i<nCount;i++)
			{
				if(WaitForSingleObject(ch->m_hStopSendEvent,0)==WAIT_OBJECT_0)
				{
					delete[] m_pbMem;
					m_pbMem=NULL;	
					wsprintf(szMsg,"CChannel::StopSendEvent");		LogMyEvent(1,1,szMsg);
					return 1;
				}
				
				EnterCriticalSection(&(ch->m_channelCriticalSection));
				CSCMemoryBlockQueue* queue=ch->m_Queue[i];
				if (queue==NULL)
				{
					LeaveCriticalSection(&(ch->m_channelCriticalSection));
					continue;
				}
				j=queue->Size();
				wsprintf(szMsg,"BroadcastPin::Push ok queuesize=%d",queue->Size());		LogMyEvent(3,1,szMsg);
				LeaveCriticalSection(&(ch->m_channelCriticalSection));
				if(j >ch->m_nQueueSize)
				{
					continue;
				}

				if (bFileChangeFinished)
				{
					queue->m_WFileOffset=0;
				}
				SetFilePointer (ch->m_hFile, queue->m_WFileOffset, NULL, FILE_BEGIN);
				if (ch->m_nblockSize <=0)
				{
					wsprintf(szMsg,"blocksize=0 :PID%dbak error ,sending rate error",ch->m_nPID);		LogMyEvent(1,1,szMsg);
					//LeaveCriticalSection(&(ch->m_channelCriticalSection));		
					return 0x12;
				}

				unsigned long read=0;
				if( !ReadFile(ch->m_hFile, m_pbMem, ch->m_nblockSize, &read, NULL)) 		
				{
					DWORD dwErrorCode = ::GetLastError();
					ch->m_bOccurError=TRUE;
					delete[] m_pbMem;
					m_pbMem=NULL;	
					sprintf(szMsg,":ReadThread:readFile error,error code is %d",dwErrorCode);
					LogMyEvent(1,1,szMsg);
					//LeaveCriticalSection(&(ch->m_channelCriticalSection));		
					return DODERROR_FILE_DATA_PART_LOST;
				}
				wsprintf(szMsg,"ReadFile readResult is (%d)  ",read);					LogMyEvent(3,1,szMsg);
				if(read>0)
				{
					if(m_pbMem[0]!=0x47)
					{
						delete[] m_pbMem;
						m_pbMem=NULL;	
						ch->m_bOccurError=TRUE;
						sprintf(szMsg,"channelPID(%d):Data is error(Read file),It should euqal to ts_sync",ch->m_nPID);LogMyEvent(1,1,szMsg);
						return DODERROR_FILE_DATA_PART_LOST;
					}

					TCHAR * pcData = CSCMemoryBlock::AllocBlock(read);
					if (pcData == NULL)
					{	
						DWORD dwErr = GetLastError();
						ch->m_bOccurError=TRUE;
						delete[] m_pbMem;
						m_pbMem=NULL;	
						wsprintf(szMsg,"MemoryBlock::AllocBlock error,PID(%d) errorcode=%d",ch->m_nPID,dwErr);					LogMyEvent(1,1,szMsg);
						return DODERROR_FILE_DATA_PART_LOST;
					}
					memcpy(pcData,m_pbMem,read);
					CSCMemoryBlock  * mBlock = new CSCMemoryBlock( pcData, read);
					FredPtr pFredPtr( mBlock );
					EnterCriticalSection(&(ch->m_channelCriticalSection));
					wsprintf(szMsg,"Push before queue: %8x,CI: %d,QS: %d",(DWORD)queue, i,queue->Size());		LogMyEvent(3,1,szMsg);
					queue->Push(pFredPtr);
					wsprintf(szMsg,"Push after queue: %8x,CI: %d,QS: %d",(DWORD)queue, i,queue->Size());		LogMyEvent(3,1,szMsg);
					LeaveCriticalSection(&(ch->m_channelCriticalSection));		

				}

				if((int)read < blocksize)
				{
					IsNeedFileCheckChange=TRUE;
					queue->m_WFileOffset = 0;
				}
				else
				{
					queue->m_WFileOffset += read;
				}
			}
		}
		catch (...) 
		{
			DWORD dwErr = GetLastError();
			wsprintf(szMsg,"channelPID(%d); Read filedata to queue::exception! errorcode=%d",ch->m_nPID,dwErr);LogMyEvent(1,1,szMsg);
			ch->m_bOccurError=TRUE;
			delete[] m_pbMem;
			m_pbMem=NULL;	
			return 0x12;
		}
		Sleep(1);	
	}			 
	
	return 0;
}
int CChannel::Init()
{
	if(m_pManager->m_nPortNumber <0)
		return 1;
	int nCount=m_pManager->m_nPortNumber;

	m_Queue = new PBROADMEMORYQUEUE[nCount];

	for (int i=0;i<nCount;i++)
	{
		m_Queue[i]=new CSCMemoryBlockQueue;
		TCHAR szMsg[MAX_PATH];
		wsprintf(szMsg,"Queue: %8x,ch_pid: %d",(DWORD)m_Queue[i], m_nPID);		LogMyEvent(2,1,szMsg);
	}

	if(m_bIsFileMode)
	{

		m_hFile = CreateFile(m_CurrFileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);

		m_bFirstIsUsing=TRUE;
// ------------------------------------------------------ Modified by zhenan_ji at 2005年12月14日 11:25:33
//		m_bFirstIsUsing=FALSE;
		if (m_hFile == INVALID_HANDLE_VALUE) 
		{			
			DWORD dwErr = GetLastError();
			TCHAR szMsg[MAX_PATH];wsprintf(szMsg,"CreateFile(%s) error err=%d",m_CurrFileName,dwErr);		LogMyEvent(1,1,szMsg);
			return HRESULT_FROM_WIN32(dwErr);
		}
	} 
	else
	{		
		m_pcCurrData=new BYTE[BUFTYPEBUFFERLEN];
		m_pcBackData=new BYTE[BUFTYPEBUFFERLEN];

		m_pBuffer=m_pcBackData;
		m_nbufferLength=0;
	}	
	return 0;
}
int CChannel::Run()
{	
	DWORD IDThread;
	m_bOccurError=FALSE;
	m_hStopSendEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	if(m_bIsFileMode)
	{	
		m_hSendThread=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)DODFileListhread,this,0 , &IDThread); 
	}
	else
	{		
		m_hSendThread=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)DODMemoryListhread,this,0 , &IDThread); 
	}
	return 0;
}
int CChannel::Stop() 
{
	TCHAR szMsg[MAX_PATH];wsprintf(szMsg,"CChannel::Stop()");		LogMyEvent(1,1,szMsg);
	//m_bIsStop =TRUE;
	if(m_hStopSendEvent)
	{
		SetEvent(m_hStopSendEvent);		
	}

	if(m_hSendThread)
	{
		wsprintf(szMsg,"CChannel::Stop(WaitForSingleObject) file:%s",m_CurrFileName);		LogMyEvent(2,1,szMsg);
		WaitForSingleObject(m_hSendThread,INFINITE);
		if(m_hSendThread)
		{
			CloseHandle(m_hSendThread);
			m_hSendThread=NULL;
		}
		wsprintf(szMsg,"CChannel::Stop(end) file:%s",m_CurrFileName);		LogMyEvent(1,1,szMsg);

	}
	if (m_hStopSendEvent)
	{

		CloseHandle(m_hStopSendEvent);
		m_hStopSendEvent=NULL;
	}
	return 0;
}

#endif // #ifndef _NO_FIX_DOD
