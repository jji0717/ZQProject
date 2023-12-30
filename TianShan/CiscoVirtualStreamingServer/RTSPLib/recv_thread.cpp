#include <WinSock2.h>
#include "recv_thread.h"
#include "CVSSRtspParser/ClientSocket.h"

recv_thread::recv_thread(::ZQ::common::FileLog *logfile,
						 SessionSocketSet &sessionSocketSet,
						 CVSSRtspSessionSet *rtspSessMap,
						 ::ZQ::common::NativeThreadPool &recvPool,
						 ::ZQ::common::NativeThreadPool &sendPool,
						 RtspCSeqSignal &rtspCSeqSignal,
						 ::Ice::CommunicatorPtr communicator,
						 ZQADAPTER_DECLTYPE adapter)
:BaseThread(logfile)
,_sessionSocketSet(sessionSocketSet)
,_recvPool(recvPool)
,_pCVSSRtspSessinoSet(rtspSessMap)
,_rtspCSeqSignal(rtspCSeqSignal)
,_communicator(communicator)
,_adapter(adapter)
//,_pChopThrd(logfile, sessionSocketSet, rtspSessMap, recvPool, sendPool, rtspCSeqSignal, communicator, adapter)
{
}

recv_thread::~recv_thread()
{
	_pCVSSRtspSessinoSet = NULL;
}

bool recv_thread::init(void)
{
	//_pChopThrd.start();
	return true;
}

int recv_thread::run(void)
{
	_bLoop = true;
	bool bSessionStatus = false;
	while (_bLoop)
	{
		if  (_socketSet.empty())
		{
			Sleep(10);
			continue;
		}

		bSessionStatus = false;
		//check the socket in this list
		readLock();
		SOCKETList tmpSockList(_socketSet._socketList);
		readUnlock();
			for (SOCKETList::iterator iter = tmpSockList.begin(); iter != tmpSockList.end(); iter++)
			{
				//define a pointer for convenient use
				//SessionSocket *pSessSocket = _sessionSocketSet.inSet(*iter);
				CVSSRtspSession *pSessSocket = _pCVSSRtspSessinoSet->inSet(*iter);
				if (NULL == pSessSocket)
					continue;

				////for easy to use
				//int32 iMaxBufferSize = pSessSocket->_smartBuffer._iBufferMaxSize;
				//int32 iWritePos = pSessSocket->_smartBuffer._iWritePos;
				//int32 iReadPos = pSessSocket->_smartBuffer._iReadPos;

				////check the read and write pointer position
				////if buffer could not write, then check next socket
				//if (iWritePos == iReadPos)
				//	continue;

				////decide the max recv len
				//int iMaxRecvLen = 0;
				//if (iWritePos < iReadPos)
				//	iMaxRecvLen = iReadPos - iWritePos;
				//else
				//{
				//	if (iReadPos != -1)
				//		iMaxRecvLen = iMaxBufferSize - iWritePos;
				//	else
				//		iMaxRecvLen = iMaxBufferSize - iWritePos - 1;
				//}
			
				////socket status error
				//if (pSessSocket->_status == false)
				//	continue;

				////try to recv data and push into smart buffer
				//int ret = sRecv(pSessSocket->_smartBuffer._pBuffer + iWritePos, iMaxRecvLen, pSessSocket->_socket, TCPSOCKET, NONBLOCK);

				////check the recv return value
				//if (ret < 0)
				//{
				//	MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(recv_thread, "run: socket(%d) recv status is invalid"), pSessSocket->_socket);
				//	pSessSocket->_status = false;
				//}
				//else if (ret > 0)
				//{
				//	//move the write position
				//	 pSessSocket->_smartBuffer._iWritePos = ( pSessSocket->_smartBuffer._iWritePos + ret) % iMaxBufferSize;
				//	bSessionStatus = true;
				//	MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(recv_thread, "recv socket(%d) message"), pSessSocket->_socket);
				//}
				
				if (pSessSocket->_rtspSocket.getRecvStatus())
					continue;
				else if (pSessSocket->_rtspSocket.getStatus() == false)
					continue;
				else
				{
					int16 ret = bCheckStatus(pSessSocket->_rtspSocket._socket);
					if (ret == -1)//socket status error
					{
						pSessSocket->_rtspSocket.setStatus(false);
						continue;
					}
					else if (ret == 0)
						continue;
					else
					{
						pSessSocket->_rtspSocket.setRecvStatus(true);
						RTSPResponseRecver *recvRequest = new RTSPResponseRecver((::ZQ::common::FileLog *)_pLog, _recvPool, pSessSocket, _rtspCSeqSignal, _communicator, _adapter, false);
						recvRequest->start();
					}
				}
								
			}//for (SOCKETList::iterator iter = _socketList->begin(); iter != _socketList->end(); iter++)		
		
		//if (!bSessionStatus)
		//	Sleep(10);
	}
	::SetEvent(_handle);
	cout << "exit recv thread" << endl;
	return 1;
}

bool recv_thread::addSessionSocket(SOCKET &sock)
{
	writeLock();
	if (_socketSet.inSet(sock))
	{
		writeUnlock();
		MYLOG(ZQ::common::Log::L_WARNING, CLOGFMT(recv_thread, "addSessionSocket: Session with index(%d) already exist!"), sock);
		return false;
	}
	else
	{
		//add to socket set
		_socketSet.push(sock);
		writeUnlock();

		//if (false == _pChopThrd.addSessionSocket(sock))
		//{
		//	MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(recv_thread, "addSessionSocket: add Session Socket(%d) fail, socket already in chop thread map"), sock);
		//	return false;
		//}

		MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(recv_thread, "addSessionSocket: add Session Socket(%d) success"), sock);
		return true;
	}
}

bool recv_thread::removeSessionSocket(SOCKET &sock)
{
	//remove from socket set
	writeLock();
	bool b = _socketSet.remove(sock);
	writeUnlock();

	if (b)
	{
		//if (false == _pChopThrd.removeSessionSocket(sock))
		//{
		//	MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(recv_thread, "removeSessionSocket: remove Session Socket(%d) fail, socket already in chop thread map"), sock);
		//	return false;
		//}
		MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(recv_thread, "removeSessionSocket: remove socket(%d) from list success"), sock);	
	}
	else
		MYLOG(ZQ::common::Log::L_WARNING, CLOGFMT(recv_thread, "removeSessionSocket: remove socke(%d) from list fail, not in list"), sock);
	return b;
	
}

//impl of recv thread request
recvThrdRequest::recvThrdRequest(::ZQ::common::FileLog *logFile, ::ZQ::common::NativeThreadPool &pool, SessionSocket *sessSock)
:ThreadRequest(pool)
,_pLog(logFile)
,pSessSocket(sessSock)
{

}

recvThrdRequest::~recvThrdRequest()
{
	pSessSocket->setRecvStatus(false);
	_pLog = NULL;
	pSessSocket = NULL;
}

int recvThrdRequest::run()
{
	//for easy to use
	int32 iMaxBufferSize = pSessSocket->_smartBuffer._iBufferMaxSize;
	int32 iWritePos = pSessSocket->_smartBuffer._iWritePos;
	int32 iReadPos = pSessSocket->_smartBuffer._iReadPos;

	//check the read and write pointer position
	//if buffer could not write, then check next socket
	if (iWritePos == iReadPos)
		return 1;

	//decide the max recv len
	int iMaxRecvLen = 0;
	if (iWritePos < iReadPos)
		iMaxRecvLen = iReadPos - iWritePos;
	else
	{
		if (iReadPos != -1)
			iMaxRecvLen = iMaxBufferSize - iWritePos;
		else
			iMaxRecvLen = iMaxBufferSize - iWritePos - 1;
	}

	//socket status error
	if (pSessSocket->_status == false)
		return -1;

	//try to recv data and push into smart buffer
	int ret = sRecv(pSessSocket->_smartBuffer._pBuffer + iWritePos, iMaxRecvLen, pSessSocket->_socket, TCPSOCKET, NONBLOCK);

	//check the recv return value
	if (ret < 0)
	{
		MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(recv_thread, "run: socket(%d) recv status is invalid"), pSessSocket->_socket);
		pSessSocket->_status = false;
		return -1;
	}
	else if (ret > 0)
	{
		//move the write position
		pSessSocket->_smartBuffer._iWritePos = ( pSessSocket->_smartBuffer._iWritePos + ret) % iMaxBufferSize;
		MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(recv_thread, "recv socket(%d) message"), pSessSocket->_socket);
		return 1;
	}
	else
	{
		return 0;
	}
}