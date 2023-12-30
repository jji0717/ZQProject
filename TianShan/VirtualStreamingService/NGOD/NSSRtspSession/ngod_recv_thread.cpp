#include "ngod_recv_thread.h"
#include "ngod_rtsp_parser/ClientSocket.h"

ngod_recv_thread::ngod_recv_thread(ZQ::common::FileLog *logfile)
:m_NSSSessionGroupList(NULL),
m_pLogFile(logfile)
{
}

ngod_recv_thread::ngod_recv_thread(ZQ::common::FileLog *logfile, 
								   NSSSessionGroupList &_NSSSessionGroupList)
:m_NSSSessionGroupList(&_NSSSessionGroupList),
m_pLogFile(logfile)
{
}

ngod_recv_thread::~ngod_recv_thread()
{
	m_NSSSessionGroupList = NULL;
}

void ngod_recv_thread::setNSSSessionGroupList(NSSSessionGroupList &_NSSSessionGroupList)
{
	m_NSSSessionGroupList = &_NSSSessionGroupList;
}

int ngod_recv_thread::run(void)
{
	bool bSessionStatus = false;
	while (1)
	{
		while (m_NSSSessionGroupList == NULL)
			Sleep(1000);

		bSessionStatus = false;
		//check the socket in this list
		if (m_NSSSessionGroupList->size() > 0)
		{
			SessionSocketBufferVec bufferVec;

			//modify for connection map
			for (NSSSessionGroupList::iterator iter = m_NSSSessionGroupList->begin(); iter != m_NSSSessionGroupList->end(); iter++)
			{
				{
					::ZQ::common::MutexGuard guard((*iter)->m_SessionGroupSocketMapMutex);
					for (SessionGroupSocketMap::iterator groupSockIter = (*iter)->m_SessionGroupSocketMap.begin(); groupSockIter != (*iter)->m_SessionGroupSocketMap.end(); groupSockIter++)
					{
						SessionSocketBuffer *ptmpSessionSocketBuffer	= new SessionSocketBuffer();
						ptmpSessionSocketBuffer->_pRTSPMessageBuffer	= &(groupSockIter->second->_messageBuffer);
						ptmpSessionSocketBuffer->_pRTSPMessageList		= &((*iter)->m_NSSRTSPMessageList);
						ptmpSessionSocketBuffer->_pSessionSocket		= &(groupSockIter->second->_sessSock);
						bufferVec.push_back(ptmpSessionSocketBuffer);
					}
				}
			}

			for (SessionSocketBufferVec::iterator bufferIter = bufferVec.begin(); bufferIter != bufferVec.end(); bufferIter++)
			{
				//define a pointer for convenient use
				//NSSRTSPMessageBuffer *pSmartBuffer = &((*iter)->m_NSSRTSPMessageBuffer);
				NSSRTSPMessageBuffer *pSmartBuffer = (*bufferIter)->_pRTSPMessageBuffer;
				SessionSocket *pSessSocket = (*bufferIter)->_pSessionSocket;

				//check the read and write pointer position
				//if buffer could not write, then check next socket

				int32 iReadPos = pSmartBuffer->iReadPos;
				if (pSmartBuffer->iWritePos == iReadPos)
				{
					delete (*bufferIter);
					continue;
				}
				//socket status error
				//if ((*iter)->m_SessionSocket.m_Status == false)
				if (pSessSocket->m_Status == false)
				{
					delete (*bufferIter);
					continue;
				}

				//decide the max recv len
				int iMaxRecvLen = 0;
				if (pSmartBuffer->iWritePos < iReadPos)
					iMaxRecvLen = pSmartBuffer->iReadPos - pSmartBuffer->iWritePos;
				else
				{
					if (iReadPos != -1)
						iMaxRecvLen = NSSRTSPMessageBufferSize - pSmartBuffer->iWritePos;
					else
						iMaxRecvLen = NSSRTSPMessageBufferSize - pSmartBuffer->iWritePos - 1;
				}
			
				//try to recv data and push into smart buffer
				int ret = sRecv(pSmartBuffer->strBuffer + pSmartBuffer->iWritePos,
					iMaxRecvLen,
					/*(*iter)->m_SessionSocket.m_Socket,*/
					pSessSocket->m_Socket,
					TCPSOCKET,
					NONBLOCK);

				//check the recv return value
				if (ret < 0)
				{
					int err = WSAGetLastError();
					//(*iter)->m_SessionSocket.m_Status = false;
					pSessSocket->m_Status = false;
				}
				else if (ret > 0)
				{
					//move the write position
					pSmartBuffer->iWritePos = (pSmartBuffer->iWritePos + ret) % NSSRTSPMessageBufferSize;
					bSessionStatus = true;
				}
				delete (*bufferIter);
			}//for
			bufferVec.clear();
			if (!bSessionStatus)
				Sleep(10);
		}
		else
			Sleep(1000);
	}
	return 1;
}