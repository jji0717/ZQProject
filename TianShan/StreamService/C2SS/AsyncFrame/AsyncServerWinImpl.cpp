#include <WinSock2.h>

#include "AsyncServerWinImpl.hpp"

#define EXIT_CODE  (-1)

AsyncServerWinImpl::AsyncServerWinImpl(IMgr& mgr, uint32 maxWorkers)
: _maxWorkers(maxWorkers),_mgr(mgr)
{
	memset(&_overLapped, 0, sizeof(_overLapped));
	assert( _maxWorkers >= 1 );
	_ioCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if(NULL == _ioCompletionPort)
	{
		//log error
	}

	for (uint32 worker = 0 ; worker < _maxWorkers ; ++worker)
	{
		AsyncServerWinWorker* workerThr = new AsyncServerWinWorker( *this);
		assert(NULL != workerThr);
		_winWorkers.push_back(workerThr);
		workerThr->start();
	}
}

AsyncServerWinImpl::~AsyncServerWinImpl()
{
#pragma messages("release resource for AsyncServerWinImpl")
}

void    AsyncServerWinImpl::start(void)
{

}

void    AsyncServerWinImpl::stop(void)
{
	std::vector<AsyncServerWinWorker*>::iterator it;
	for (it = _winWorkers.begin(); it < _winWorkers.end(); ++it)
	{// post EXIT_CODE
		PostQueuedCompletionStatus(_ioCompletionPort, 0, (DWORD)EXIT_CODE, NULL);
	}

	for (it = _winWorkers.begin(); it < _winWorkers.end(); ++it)
	{
		(*it)->waitHandle(-1);
		delete (*it);
	}
	_winWorkers.clear();
}

int32	AsyncServerWinImpl::doRecvSync(int sock, char* buffer, unsigned int bufSize, int32 timeout/* = -1*/)
{
    int timeoutInterval = timeout;
	if( timeoutInterval <= 0 )
	{
		int iRet = ::recv(sock, buffer, bufSize, 0);
		if( iRet == SOCKET_ERROR  )
		{
			//MLOG(ZQ::common::Log::L_ERROR, COMMFMT(read,"failed to invoke recv and errorCode[%u]"), WSAGetLastError());
			return ERROR_CODE_OPERATION_FAIL;
		}
		return static_cast<int32>(iRet);
	}
	else
	{
		struct  timeval selectTime;	
		selectTime.tv_sec	=	timeoutInterval / 1000;
		selectTime.tv_usec	=	(timeoutInterval % 1000 ) * 1000;
		fd_set	readSet;
		FD_ZERO(&readSet);
		FD_SET(sock, &readSet);

		int iRet = 0;

		iRet = ::select( 1 , &readSet , NULL , NULL,&selectTime );
		if( SOCKET_ERROR == iRet )
		{
			//MLOG(ZQ::common::Log::L_ERROR, COMMFMT(read,"failed to invoke select and errorCode[%u]"), WSAGetLastError() );
			return ERROR_CODE_OPERATION_FAIL;
		}
		else
		{			
			if( iRet != 1 )
			{
				//MLOG(ZQ::common::Log::L_ERROR, COMMFMT(read,"failed to invoke select and errorCode[%u] and return value [%d]"),	WSAGetLastError() ,iRet );
				return ERROR_CODE_OPERATION_FAIL;
			}
			else
			{
				if( FD_ISSET(sock, &readSet))
				{
					iRet = ::recv(sock, buffer, bufSize, 0);
					if( iRet == SOCKET_ERROR  )
					{
					//	MLOG(ZQ::common::Log::L_ERROR,	COMMFMT(read,"failed to invoke recv and errorCode[%u]"), WSAGetLastError() );
						return ERROR_CODE_OPERATION_FAIL;
					}
					else
					{
						return iRet;
					}
				}
				else
				{
					//MLOG(ZQ::common::Log::L_ERROR,COMMFMT(read,"should not be here !"));
					return ERROR_CODE_OPERATION_FAIL;
				}
			}
		}
	}
	return true;
}

int32	AsyncServerWinImpl::doRecvAsync(Message* recvMsg)
{
	DWORD dwReadByte = 0;
	DWORD dwFlag = 0;

	if (NULL == recvMsg)
		return false;
	
	recvMsg->_wsaBuffer.buf = (char*)recvMsg->_buf + recvMsg->_ioSize;
	recvMsg->_wsaBuffer.len = recvMsg->_bufSize - recvMsg->_ioSize;
	memset(&recvMsg->_overlapped, 0,sizeof(recvMsg->_overlapped) );

	int iRet = WSARecv(recvMsg->_sock, &recvMsg->_wsaBuffer, 1, &dwReadByte, &dwFlag, &(recvMsg->_overlapped), NULL);

	assert( iRet == 0 || iRet == SOCKET_ERROR );	

	if( iRet == SOCKET_ERROR  )
	{//it's OK
		int lastErr = WSAGetLastError();
		if(WSA_IO_PENDING == lastErr)
		{
			return ERROR_CODE_OPERATION_PENDING;
		}
		else// if (SOCKET_ERROR == WSARecv(recvMsg->_sock, &recvMsg->_wsaBuffer, 1, &dwReadByte, &dwFlag, &(recvMsg->_overlapped), NULL) )
		{			
			//MLOG(ZQ::common::Log::L_ERROR,COMMFMT(readAsync,"failed to invoke WSARecv, errorCode[%u]"),WSAGetLastError() );
			return ERROR_CODE_OPERATION_FAIL;
		}
	}

	return true;
}

int32	AsyncServerWinImpl::doSendSync(int sock, const char* buffer, unsigned int bufSize, int32 timeout/* = -1*/)
{
	int timeoutInterval = timeout;
	int mSock           = sock;

	if ( timeoutInterval <= 0 )
	{
		int iRet = ::send( mSock, buffer, (int)bufSize, 0);
		if(SOCKET_ERROR == iRet)
		{
			//MLOG(ZQ::common::Log::L_ERROR,COMMFMT(write,"failed to invoke send, errorCode[%u/%s]"),
			//	WSAGetLastError() , getSocketErrorstring( WSAGetLastError() ).c_str()  );
			return ERROR_CODE_OPERATION_FAIL;
		}
		else
		{
			return iRet ;
		}
	}
	else
	{
		struct  timeval selectTime;	
		selectTime.tv_sec	=	timeoutInterval / 1000;
		selectTime.tv_usec	=	(timeoutInterval % 1000 ) * 1000;

		fd_set	writeSet;
		FD_ZERO( &writeSet );
		FD_SET(mSock,&writeSet);
		int iRet = ::select( 1, NULL, &writeSet, NULL, &selectTime);
		if( iRet != 1)
		{
			//MLOG(ZQ::common::Log::L_ERROR,COMMFMT(write,"failed to invoke send, timed out") );
			return ERROR_CODE_OPERATION_TIMEOUT;
		}
		else
		{
			if( FD_ISSET(mSock,&writeSet))
			{
				iRet = ::send( mSock, buffer, (int)bufSize, 0);
				if( iRet == SOCKET_ERROR )
				{
					//MLOG(ZQ::common::Log::L_ERROR,COMMFMT(write,"failed to invoke send, errorCode[%u/%s]"),
					//	WSAGetLastError() , getSocketErrorstring( WSAGetLastError() ).c_str()  );
					return ERROR_CODE_OPERATION_FAIL;
				}
				else
				{
					return iRet ;
				}
			}
			else
			{
				return ERROR_CODE_OPERATION_TIMEOUT;
			}
		}
	}
	return true;
}

int32	AsyncServerWinImpl::doSendAsync(Message* sendMsg)
{
	DWORD  dwSent = 0;
	DWORD  dwFlag = 0;

	memset(&sendMsg->_overlapped, 0, sizeof(sendMsg->_overlapped) );
	int iRet = WSASend(sendMsg->_sock, &sendMsg->_wsaBuffer, 1, &dwSent, dwFlag, &sendMsg->_overlapped, NULL);
	if(SOCKET_ERROR == iRet)
	{
		int err = WSAGetLastError();
		if(WSA_IO_PENDING == err)
		{
			return ERROR_CODE_OPERATION_OK;
		}
		else if (WSAEWOULDBLOCK == err)
		{
			return ERROR_CODE_OPERATION_OK;
		}else{
			return ERROR_CODE_OPERATION_FAIL;
		}
	}
	else
	{
		return ERROR_CODE_OPERATION_OK ;
	}

	return true;
}

int32	AsyncServerWinImpl::addServent(int socketForInject, ICommuncator* key, int event)
{
	assert(INVALID_HANDLE_VALUE != _ioCompletionPort);
	assert(NULL != key);
	//DataCompletionKey* key = comm->getCompletionKey();
	//MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(DataPostDak,"add communicator [%lld] with key[%x]"), comm->getCommunicatorId(),key );
	HANDLE newCompletionPort = CreateIoCompletionPort((HANDLE)socketForInject, (HANDLE)_ioCompletionPort, (ULONG_PTR)key, 0);	
	if(NULL == newCompletionPort)
	{
		//int res = WSAGetOverlappedResult(socketForInject, )
		int err = WSAGetLastError();
		err = err;
		if (WSA_INVALID_PARAMETER == err)
		{
			err = err;
		}
		//MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DataPostDak,"addnewCommunicator() failed to add communicator[%lld] into dak, err[%u]"),
		//	comm->getCommunicatorId(), WSAGetLastError() );
	}
	
	return (NULL != newCompletionPort);
}

int32   AsyncServerWinImpl::activeServent(int socketForInject, ICommuncator *key, int event)
{
	assert(INVALID_HANDLE_VALUE != _ioCompletionPort);
	assert(NULL != key);
	//DataCompletionKey* key = comm->getCompletionKey();
	//MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(DataPostDak,"add communicator [%lld] with key[%x]"), comm->getCommunicatorId(),key );
	HANDLE newCompletionPort = CreateIoCompletionPort((HANDLE)socketForInject, (HANDLE)_ioCompletionPort, (ULONG_PTR)key, 0);	
	if(NULL == newCompletionPort)
	{
		//MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DataPostDak,"addnewCommunicator() failed to add communicator[%lld] into dak, err[%u]"),
		//	comm->getCommunicatorId(), WSAGetLastError() );
	}

	return (NULL != newCompletionPort);
}

int32	AsyncServerWinImpl::removeServent(int socketForRemove, ICommuncator* key)
{
	return true;
}

void    AsyncServerWinImpl::getLocalAddress(std::string& localIP, std::string& localPort) const
{
	return ;
}

void    AsyncServerWinImpl::getRemoteAddress(std::string& remoteIP, std::string& remotePort) const
{
	return ;
}

uint32	AsyncServerWinImpl::getIdleTime()
{
	return true;
}

AsyncServerWinWorker::AsyncServerWinWorker(AsyncServerWinImpl& asyncServerWin)
:_asyncServerWin(asyncServerWin)
{
    _quit = false;
}

int32 AsyncServerWinWorker::run()
{
	unsigned long iocpByte;
	LONG_PTR	  completionKey;
	LPOVERLAPPED  overLapped  ;
	HANDLE&       ioCompletetionPort = _asyncServerWin._ioCompletionPort;
	DWORD         dwFlag   = 0;

	while(!_quit)
	{	
		completionKey = NULL;
		overLapped = NULL;
		iocpByte = 0;

		int bRet =  GetQueuedCompletionStatus(ioCompletetionPort, &iocpByte,(PULONG_PTR)&completionKey, &overLapped, 1000000);		
		if ( EXIT_CODE == (DWORD)completionKey || _quit)//post exit code
			break;

		Message* msgKey     = reinterpret_cast<Message*>(overLapped);
		ICommuncator* conn  = reinterpret_cast<ICommuncator*>(completionKey);

		if (!bRet)
		{
			int errCode  = ::WSAGetLastError();
			if (WAIT_TIMEOUT == errCode)
				continue;//IOCP time out

			if (NULL == completionKey || NULL == overLapped)
				break;//error

			int result      = ::WSAGetOverlappedResult(msgKey->_sock, overLapped, &iocpByte, FALSE, &dwFlag);
			int lastErrCode = ::WSAGetLastError();
			if (!result || STATUS_WAIT_0 == overLapped->Internal)
			{
				if (WSAECONNRESET == lastErrCode) //remote socket closed by force
				{
					msgKey->_msgType = ERROR_PENDING;
					conn->onError(msgKey);
				}else if (WSAENOTSOCK == lastErrCode){//inter close in case ddos or manual shutdown
					msgKey->_msgType = CLOSE_PENDING;
					conn->onClose(msgKey);
				}

				continue;
			}

			if (ERROR_NETNAME_DELETED == lastErrCode 
				|| ERROR_OPERATION_ABORTED == lastErrCode 
				|| WSAECONNABORTED == lastErrCode)
			{//client quit exception
				msgKey->_msgType = ERROR_PENDING;			
				conn->onError(msgKey);
				continue;
			}else{
				break;//iocp error, quit
			}
		}
		if (0 == iocpByte)
		{
			int result  = ::WSAGetOverlappedResult(msgKey->_sock, overLapped, &iocpByte, FALSE, &dwFlag);
			int errCode = ::WSAGetLastError();
			if (!result || WSAENOTSOCK == errCode)
			{//current socket is not valid or is pending
				continue;
			}

			switch(msgKey->_msgType)
			{
			case ACCEPT_PENDING:
			default:
				{
					msgKey->_msgType = ERROR_PENDING;
					conn->onError(msgKey);
					continue;//next while
				}
			case SEND_PENDING:// SEND_PENDING == msgKey->_msgType would be trigger while _bufSize is zero 
			case RECV_PENDING:// RECV_PENDING == msgKey->_msgType would be trigger while _bufSize is zero 
				{             
					if(0 == msgKey->_bufSize && Message::TRIGGER == msgKey->_msgSpecies)
						break;//goto Trigger
				}
			case RECV_SUCCEED:
			case SEND_SUCCEED:
				{
					int buffer;
					int ret = ::recv(msgKey->_sock, (char* )&buffer, sizeof(buffer), MSG_PEEK);//test if sock really down
					if (0 >= ret)
					{					
						msgKey->_msgType = CLOSE_PENDING;
						conn->onClose(msgKey);
						continue;//next while
					}
				}
			}
		}//fall

		msgKey->_ioSize = iocpByte;
		switch(msgKey->_msgType)// goto here: mark Trigger
		{
		case ACCEPT_PENDING:
			{
				msgKey->_msgType = ACCEPT_SUCCEED;
				conn->onAsyncIn(msgKey);
			}
			break;
		case RECV_PENDING:
			{
				msgKey->_msgType = RECV_SUCCEED;
				conn->onAsyncIn(msgKey);
			}
			break;
		case SEND_PENDING:
			{
				msgKey->_msgType = SEND_SUCCEED;
				conn->onAsyncOut(msgKey);
			}
			break;
		case CLOSE_PENDING:
		case ERROR_PENDING:
		default:
			{
				//should not be here, there must be some thing wrong
			}
			break;
		}//end switch

	}//end while

	return true;
}

bool AsyncServerWinWorker::start(void)
{
	ZQ::common::NativeThread::start();
	return true;
}

void AsyncServerWinWorker::stop(void) 
{
	_quit = true;
	return ;
}