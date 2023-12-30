#include "RTSP_MessageRecvThrd.h"

RTSP_MessageRecvThrd::RTSP_MessageRecvThrd(::ZQ::common::FileLog &fileLog/*, SessionSocketList &sessionSocketList, RTSPMessageBuffer &rtspMessageBuffer*/, SessionMap &sessionMap)
:_fileLog(&fileLog)
//,_socketList(sessionSocketList)
//,_rtspMessageBuffer(rtspMessageBuffer)
{
	//InitializeCriticalSection(&_socketListLock);
	_rtspChopThrd = new RTSP_MessageChopThrd(fileLog, &_rtspMessageBuffer, sessionMap);
	//_rtspParseThrd = new RTSP_MessageParseThrd(fileLog, _rtspMessageBuffer);
}

RTSP_MessageRecvThrd::~RTSP_MessageRecvThrd()
{
	::ZQ::common::MutexGuard guard(_mutex);
	terminate(0);
	delete _rtspChopThrd;
	//DeleteCriticalSection(&_socketListLock);
	//delete _rtspParseThrd;
}

void RTSP_MessageRecvThrd::startThrd()
{
	start();
	_rtspChopThrd->startThrd();
	//_rtspParseThrd->start();
}
int RTSP_MessageRecvThrd::run(void)
{
	bool bSessionStatus = false;
	while (1)
	{
		bSessionStatus = false;
		//check the socket

		//lock();
		{
			::ZQ::common::MutexGuard guard(_mutex);
			for (SessionSocketList::iterator iter = _socketList.begin(); iter != _socketList.end(); iter++)
			{
				if ((*iter) == NULL)
					continue;
				//define a pointer for convenient use
				{
					//_rtspMessageBuffer.readLock();
					MessageBufferIterater *pSmartBuffer = NULL;
					{
						//::ZQ::common::MutexGuard guard(_rtspMessageBuffer._mutex);
						pSmartBuffer = _rtspMessageBuffer.getMessageBuffer((*iter)->m_Socket);
					}
					//_rtspMessageBuffer.unlock();

					if (pSmartBuffer == NULL)//no socket buffer info
						RECVTHRDCONTINUE;

					//check the read and write pointer position
					//if buffer could not write, then check next socket
					int32 iReadPos = pSmartBuffer->iReadPos;
					if (pSmartBuffer->iWritePos == iReadPos)
						RECVTHRDCONTINUE;

					//decide the max recv len
					int iMaxRecvLen = 0;
					if (pSmartBuffer->iWritePos < iReadPos)
						iMaxRecvLen = iReadPos - pSmartBuffer->iWritePos;
					else
					{
						if (iReadPos != -1)
							iMaxRecvLen = RTSPMessageBufferSize - pSmartBuffer->iWritePos;
						else
							iMaxRecvLen = RTSPMessageBufferSize - pSmartBuffer->iWritePos - 1;
					}

					//socket status error
					if ((*iter)->m_Status == false)
						RECVTHRDCONTINUE;


					//try to recv data and push into smart buffer
					//int ret = sRecv(pSmartBuffer->strBuffer + pSmartBuffer->iWritePos,
					int ret = sRecv(&pSmartBuffer->strBuffer[pSmartBuffer->iWritePos],
						iMaxRecvLen,
						(*iter)->m_Socket,
						TCPSOCKET,
						NONBLOCK);

					//check the recv return value
					if (ret < 0)
					{
						int err = WSAGetLastError();
						MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RTSP_MessageRecvThrd, "recv socket(%d) error %d"), (*iter)->m_Socket, err);
						(*iter)->m_Status = false;
					}
					else if (ret > 0)
					{
						//move the write position
						pSmartBuffer->iWritePos = (pSmartBuffer->iWritePos + ret) % RTSPMessageBufferSize;
						bSessionStatus = true;
					}
				}//for mutex
			}
			//_rtspMessageBuffer.readUnLock();
		}
		//unlock();

		if (!bSessionStatus)
			Sleep(10);
	}
	return 1;
}

bool RTSP_MessageRecvThrd::addSocket(SessionSocket *sessionSocket)
{
	if (NULL == sessionSocket)
		return false;

	//lock();
	{
		::ZQ::common::MutexGuard guard(_mutex);
		SessionSocketList::iterator iter = find_if(_socketList.begin(), _socketList.end(), FindBySocket(sessionSocket->m_Socket));
		if (iter != _socketList.end())
		{
			MYLOG(ZQ::common::Log::L_WARNING, CLOGFMT(RTSP_MessageRecvThrd, "Socket(%d) already in list! Maybe share socket with other session."), sessionSocket->m_Socket);
			return false;
		}

		_socketList.push_back(sessionSocket);
	}
	//unlock();

	MessageBufferIterater *pSmartBuffer = _rtspMessageBuffer.getMessageBuffer(sessionSocket->m_Socket);
	if (NULL != pSmartBuffer)
	{
		MYLOG(ZQ::common::Log::L_WARNING, CLOGFMT(RTSP_MessageRecvThrd, "Message Buffer with index(%d) already exist!"), sessionSocket->m_Socket);
		return false;
	}
	else
	{
		MessageBufferIterater *pNewBuffer = new MessageBufferIterater();
		pNewBuffer->strBuffer = new char[RTSPMessageBufferSize];	
		_rtspMessageBuffer.add(sessionSocket->m_Socket, pNewBuffer);
		_rtspChopThrd->addSocket(sessionSocket->m_Socket);
		MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RTSP_MessageRecvThrd, "Message Buffer with index(%d) created"), sessionSocket->m_Socket);
		return true;
	}
}