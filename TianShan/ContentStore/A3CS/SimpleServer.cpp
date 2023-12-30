#include "SimpleServer.h"
#include "SimpleSerThread.h"

#define SIMSLOG if(_pLog != NULL) (*_pLog)
#define SIMSER_LISTEN_NUM 15
#define DEFAULT_TIMEOUT 20

using namespace ZQ::common;

SimpleServer::SimpleServer(const char* hostIP, const int& port, ZQ::common::Log* pLog):
_strIP(hostIP), _port(port), _pLog(pLog), _sock((SOCKET_T)-1), _bExit(false), _nTimeOut(DEFAULT_TIMEOUT)
{
}

SimpleServer::~SimpleServer(void)
{
	close();
}

bool SimpleServer::init(void)
{
	SIMSLOG(Log::L_DEBUG,"SimpleServer::start() listen port: %d",_port);
	if(_sock != (SOCKET_T)-1)
	{
		closesocket(_sock);
		_sock = (SOCKET_T)-1;
	}
	_sock = (SOCKET_T)socket(AF_INET, SOCK_STREAM, 0);
	if(_sock == (SOCKET_T)-1)
	{
		SIMSLOG(Log::L_ERROR,"SimpleServer::start() can not create a socket");
		return false;
	}
	struct sockaddr_in  myaddr;
	memset(&myaddr,0,sizeof(struct sockaddr_in));
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(_port);
	if(_strIP.length() > 0)
	{
		myaddr.sin_addr.s_addr = inet_addr(_strIP.c_str());
	}
	else
	{
		myaddr.sin_addr.s_addr = INADDR_ANY;
	}

	if(bind(_sock,  (struct sockaddr *)&myaddr, sizeof(struct sockaddr)) != 0)//has error
	{
		SIMSLOG(Log::L_ERROR,"SimpleServer::start() bind socket %d has error code = %d",int(_sock),WSAGetLastError());
		return false;
	}
	else
	{
		SIMSLOG(Log::L_DEBUG,"SimpleServer::start() bind OK");
	}

	if(listen(_sock,SIMSER_LISTEN_NUM) != 0)
	{
		SIMSLOG(Log::L_ERROR,"SimpleServer::start() listen socket %d error code = %d",(int)_sock,WSAGetLastError());
		return false;
	}
	else
	{
		SIMSLOG(Log::L_DEBUG,"SimpleServer::start() listen OK");
	}

	return true;
}

int SimpleServer::run(void)
{
	SOCKET_T newSock;
	struct sockaddr_in newAddr;
	int len = sizeof(struct sockaddr_in);
	while(!_bExit)
	{
		memset(&newAddr,0,sizeof(struct sockaddr_in));
		newSock = accept(_sock, (struct sockaddr*)&newAddr,&len);
		if(newSock == (SOCKET_T)-1)
		{
			SIMSLOG(Log::L_ERROR,"simpleServer::start() accept a invalidation socket errorcode: %d",WSAGetLastError());
			continue;
		}
		else//add to the thread pool
		{
			SIMSLOG(Log::L_DEBUG,"simpleServer::start() accept a socket %d",(int)newSock);
			SimpleSerThread* pSimpleSerTh = new SimpleSerThread(_pool, newSock, _nTimeOut, _pLog, this);
			pSimpleSerTh->start();		
		}
	}

	return 0;
}

int SimpleServer::handleMsg(const std::string& strType, const std::string& strMsg) {

#ifdef _DEBUG
	printf("%s\n",strMsg.c_str());
#endif
	return MSG_OK;
}

void SimpleServer::close()
{ 
	_bExit = true;
	closesocket(_sock);
	_pool.stop();
	SIMSLOG(Log::L_DEBUG,"SimpleServer close");
}

#ifdef ZQ_OS_MSWIN
SimpleSerWSA::SimpleSerWSA()
{
	//-initialize OS socket resources!
	WSADATA w;
	if (WSAStartup(MAKEWORD(2, 2), &w))
	{
		abort();
	}
};

SimpleSerWSA::~SimpleSerWSA() 
{ 
	WSACleanup(); 
} 
#endif