#include "SimpleHttpd.h"

#define SIMSLOG if(_pLog != NULL) (*_pLog)

#define SIMSER_LISTEN_NUM	15
#define DEFAULT_TIMEOUT		20//socket time out unit second
#define MAX_TIMEOUT			600//ten minute
#define SIMSERTH_BUFLEN		1024
#define SSTHC	"HttpHandlerProc"

#include "SystemUtils.h"

// -----------------------------
// class HttpHandlerProc
// -----------------------------
// implements the procedure on a received HTTP message
class HttpHandlerProc :	public ZQ::common::ThreadRequest
{
public:
	//@timeOut: socket time out ,unit second
	HttpHandlerProc(ZQ::common::NativeThreadPool& pool,SOCKET_T sock, int timeOut, ZQ::common::Log* pLog, SimpleHttpd* pSimSer);
	virtual ~HttpHandlerProc(void);

protected:
	bool init(void);
	int run(void);
	void final(int retcode =0, bool bCancelled =false);

private:
	bool recvData(void);
	bool sendData(std::string& strRes);
	char getChar(void);
	bool beginRecv(void);
	bool continueRecv(void);
	int  getData(void);
	bool isEOF(void);
	bool parseData(void);
	bool getLine(char* buf, size_t bufLen);

private:
	SimpleHttpd*	_pSimSer;
	SOCKET_T		_sock;
	ZQ::common::Log* _pLog;
	char			_buf[SIMSERTH_BUFLEN];
	std::string		_strContent;
	std::string			_strUri;

	int				_nTimeOut;//unit is second
	int				_bufIdx;
	int				_bufLen;
	int				_needRecv;
	bool			_bContinue;
	bool			_bAlive;
};

// -----------------------------------
// class HttpHandlerProc implement
// -----------------------------------
HttpHandlerProc::HttpHandlerProc(ZQ::common::NativeThreadPool& pool,SOCKET_T sock, int timeOut, ZQ::common::Log* pLog, SimpleHttpd* pSimSer)
:ZQ::common::ThreadRequest(pool)
{
	_sock = sock;
	_pLog = pLog;
	_pSimSer = pSimSer;	
	_nTimeOut = timeOut;
}

HttpHandlerProc::~HttpHandlerProc(void)
{
	
}

bool HttpHandlerProc::init(void)
{
	if(_sock == (SOCKET_T)-1)
	{
		SIMSLOG(ZQ::common::Log::L_ERROR,"HttpHandlerProc init erro: socket is invalidation");
		return false;
	}

	memset(_buf,0,sizeof(_buf));
	_bufIdx = 0;
	_bufLen = 0;
	_needRecv = 0;
	_bContinue = false;
	_bAlive = false;
	return true;
}

#ifdef ZQ_OS_MSWIN
typedef int socklen_t;
#endif
int HttpHandlerProc::run(void)
{
	struct sockaddr addr;
	socklen_t addrlen = sizeof(struct sockaddr);
	memset(&addr, 0, addrlen);		
	if(getpeername(_sock, &addr, &addrlen) != 0)
	{
		SIMSLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SSTHC,"run() the socket [%d] not get peer infomation error code[%d]"),_sock,SYS::getLastErr(SYS::SOCK));
		_closesocket(_sock);
		return -1;
	}
	while(!_pSimSer->bExit())
	{		
		SimpleHttpd::MsgInfo msgInfo;
		msgInfo.port = ((struct sockaddr_in*)&addr)->sin_port;
		msgInfo.ip = inet_ntoa(((struct sockaddr_in*)&addr)->sin_addr);

		if(!recvData())
		{
			SIMSLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SSTHC,"run() not receive data"));
			break;
		}
		
		msgInfo.uri = _strUri;
		std::string strRes;
		int code = _pSimSer->OnHttpMessage(msgInfo, _strContent);
		//catch return code
		switch(code)
		{
			case MSG_OK:
				strRes = "200 OK";
				break;
			case MSG_UNKNOWN_PAID:
				strRes = "404 Unknown PAID";
				break;
			default:
				strRes = "404 Not Found";
				break;
		}

		if(!sendData(strRes))
		{
			SIMSLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SSTHC, "run() send response error"));
			break;
		}

		if(!_bAlive)//is keep alive
		{
			SYS::sleep(10);
			break;
		}
	}

	_closesocket(_sock);
	return 0;
}

void HttpHandlerProc::final(int retcode, bool bCancelled)
{
	delete this;
}

bool HttpHandlerProc::recvData()
{
	_bContinue = false;
	_strContent = "";
	_needRecv = 0;

	if(!beginRecv())
	{
		SIMSLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SSTHC,"begin receive failed,maybe socket has closed"));
		return false;
	}

	while (!isEOF())
	{
		if(!continueRecv())
		{
			SIMSLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SSTHC,"continue receive failed"));
			return false;
		}
	}
	
	return true;
}

bool HttpHandlerProc::beginRecv()
{
	memset(_buf, 0, sizeof(_buf));
	_bufIdx = 0;
	_bufLen = 0;

	char chR = getChar();
	if(chR == '\0')
	{
		SIMSLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SSTHC,"not receive data"));
		return false;
	}

	if(!_bContinue)
	{
		if(!parseData())
		{
			SIMSLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SSTHC,"parse data error"));
			return false;
		}
		else
		{
			if(_needRecv == 0)
				return true;
			
			_strContent = _buf + _bufIdx;
			_needRecv -= _bufLen - _bufIdx;
			if(_needRecv < 0)
			{
				size_t len = _strContent.length();
				len += _needRecv;
				_strContent[len] = '\0';				
			}
		}
	}
	else
	{
		_needRecv -= _bufLen;
		_strContent += _buf;
		if(_needRecv < 0)
		{
			size_t len = _strContent.length();
			len += _needRecv;
			_strContent[len] = '\0';
		}
	}

	return true;
}

char HttpHandlerProc::getChar()
{
	if (_bufIdx >= _bufLen && getData() == 0)
		return '\0';

	return _buf[_bufIdx++];
}

int HttpHandlerProc::getData()
{
	_bufIdx = 0;
	if(_nTimeOut > 0)//hase time out
	{
		if(_nTimeOut > MAX_TIMEOUT)
			_nTimeOut = MAX_TIMEOUT;

		struct timeval st;
		fd_set rfd,exfd;
		st.tv_sec = _nTimeOut;
		st.tv_usec = 0;

		FD_ZERO(&rfd);
		FD_ZERO(&exfd);
		FD_SET(_sock, &rfd);
		FD_SET(_sock, &exfd);
		int res = select((int)_sock + 1, &rfd, NULL, &exfd, &st);
		if(res < 0 || FD_ISSET(_sock, &exfd))//has error or timeout
		{
			SIMSLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SSTHC,"select() receive date error code: %d"),SYS::getLastErr(SYS::SOCK));
			_bufLen = 0;
			return 0;
		}
		else if(res == 0)
		{
			SIMSLOG(ZQ::common::Log::L_WARNING,CLOGFMT(SSTHC,"select() receive data time out"));
			_bufLen = 0;
			return 0;
		}
		
	}
	
	_bufLen = recv(_sock, _buf, sizeof(_buf)-1, 0);

	if(_bufLen < 0)//have error
	{
		SIMSLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SSTHC,"received zero number data errorcode = %d"),SYS::getLastErr(SYS::SOCK));
		_bufLen = 0;
		return 0;
	}
	else if(_bufLen == 0)//socket has closed
	{
		SIMSLOG(ZQ::common::Log::L_INFO, CLOGFMT(SSTHC,"the connection has been closed"));
		return 0;
	}

	return _bufLen;
}

bool HttpHandlerProc::isEOF()
{
	return (_needRecv <= 0);
}

bool HttpHandlerProc::continueRecv()
{
	_bContinue = true;
	return beginRecv();
}

bool HttpHandlerProc::parseData()
{
	_bufIdx = 0;
	char chLine[1024];

	//get start line
	if(!getLine(chLine, sizeof(chLine)))
	{
		SIMSLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SSTHC,"can not get start line"));
		return false;
	}
	//it is a post packet
	if(strncmp(chLine,"POST",4) != 0)
	{
		SIMSLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SSTHC,"it is not a post packet"));
		return false;
	}

	char* s = NULL;
	if ((s = strchr(chLine, ' ')))
	{
		s++;
		char* end = NULL;
		end = strrchr(chLine,' ');
		if(end != NULL)
			*end = '\0';
		else
			return false;

		_strUri = s;
	}
	else
	{
		SIMSLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SSTHC,"parse start line"));
		return false;
	}

	do
	{
		if(!getLine(chLine, sizeof(chLine)))
		{
			return false;
		}

		s = strchr(chLine, ':');
		if (s)
		{ 
			*s = '\0';
			do s++;
			while (*s && *s <= 32);
			if (*s == '"')
				s++;
		
			if(stricmp(chLine, "Content-Length") == 0)
			{
				_needRecv = atoi(s);
				if(_needRecv == 0)
				{
					_bAlive = false;					
					return true;
				}
			}
			else if(stricmp(chLine, "Connection") == 0)
			{
				if(stricmp(s,"keep-alive") == 0)
					_bAlive = true;
				else
					_bAlive = false;
			}
		}
		else
			break;
		
	} while (1);
	
	return true;
}

bool HttpHandlerProc::getLine(char* buf, size_t bufLen)
{
	size_t i = bufLen;
	char c = 0;
	for (;;)
	{ 
		while (--i > 0)
		{ 
			c = getChar();
			if (c == '\r' || c == '\n')
				break;
			if (c == '\0')
				return false;
			*buf++ = c;
		}
		if (c != '\n')
			c = getChar(); // got \r or something else, now get \n 
		if (c == '\n')
		{ 
			*buf = '\0';
			if (i+1 == (int)bufLen) // empty line: end of header 
				break;
			c = getChar();
			--_bufIdx;
			if (c != ' ' && c != '\t') // HTTP line continuation? 
				break;
		}
		else if (c == '\0')
			return false;
	}
	if (i < 0)
		return false;
	
	return true;
}

bool HttpHandlerProc::sendData(std::string& strRes)
{
	std::string strSend;
	//start line
	if(strRes.length() > 0)
		strSend = "HTTP/1.1 " + strRes + "\r\n";
	else
		strSend = "HTTP/1.1 200 OK\r\n";

	strSend += "Content-Length: 0\r\n";
	
	char format[] = {"%a, %d %b %Y %H:%M:%S GMT"};
	char buf[50] = {0};
	time_t tt;
	struct tm* stm;
	time(&tt);
	stm = gmtime(&tt);
	strftime(buf,sizeof(buf),format,stm);
	strSend += "Data: ";
	strSend += buf;
	strSend += "\r\n\r\n";

	size_t nCount = strSend.length();
	int nTry = 5;
	
	do
	{
		if(_nTimeOut > 0)//hase time out
		{
			if(_nTimeOut > MAX_TIMEOUT)
				_nTimeOut = MAX_TIMEOUT;

			struct timeval st;
			fd_set wfd,exfd;
			st.tv_sec = _nTimeOut;
			st.tv_usec = 0;

			FD_ZERO(&wfd);
			FD_ZERO(&exfd);
			FD_SET(_sock, &wfd);
			FD_SET(_sock, &exfd);
			int res = select((int)_sock + 1, NULL, &wfd, &exfd, &st);
			if(res < 0 || FD_ISSET(_sock, &exfd))//has error or timeout
			{
				SIMSLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SSTHC,"select() send date error code: %d"),SYS::getLastErr(SYS::SOCK));
				return false;
			}
			else if(res == 0)
			{
				SIMSLOG(ZQ::common::Log::L_WARNING,CLOGFMT(SSTHC,"select() send data time out"));
				return false;
			}
		}

		int nHasS = send(_sock, strSend.c_str(), static_cast<int>(strSend.length()), 0);
		if(nHasS < 0)//have error
		{
			SIMSLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SSTHC,"send data has error code: %d"),SYS::getLastErr(SYS::SOCK));	
			return false;
		}
		else if(nHasS == 0)//block maybe has a timeout
		{
			nTry--;
			SYS::sleep(1);
		}
		else
			nCount -= nHasS;
		
	}while(nCount && nTry);

	if(nCount > 0)//not send all data
	{
		SIMSLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SSTHC,"send data is blocking"));	
		return false;
	}
	
	return true;
}


// -----------------------------------
// class SimpleHttpd implement
// -----------------------------------
SimpleHttpd::SimpleHttpd(const char* hostIP, const int& port, ZQ::common::Log* pLog)
:_pool(SIMSER_LISTEN_NUM), _strIP(hostIP), _port(port), _pLog(pLog), _sock((SOCKET_T)-1), _bExit(false), _nTimeOut(DEFAULT_TIMEOUT)
{
}

SimpleHttpd::~SimpleHttpd(void)
{

}

bool SimpleHttpd::init(void)
{
	SIMSLOG(ZQ::common::Log::L_DEBUG,"SimpleHttpd::start() listen ip[%s] port[%d]", _strIP.c_str(), _port);
	if(_sock != (SOCKET_T)-1)
	{
		_closesocket(_sock);
		_sock = (SOCKET_T)-1;
	}
	_sock = (SOCKET_T)socket(AF_INET, SOCK_STREAM, 0);
	if(_sock == (SOCKET_T)-1)
	{
		SIMSLOG(ZQ::common::Log::L_ERROR,"SimpleHttpd::start() can not create a socket");
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
		SIMSLOG(ZQ::common::Log::L_ERROR,"SimpleHttpd::start() bind socket %d has error code = %d",int(_sock),SYS::getLastErr(SYS::SOCK));
		return false;
	}
	else
	{
		SIMSLOG(ZQ::common::Log::L_DEBUG,"SimpleHttpd::start() bind OK");
	}

	if(listen(_sock,SIMSER_LISTEN_NUM) != 0)
	{
		SIMSLOG(ZQ::common::Log::L_ERROR,"SimpleHttpd::start() listen socket %d error code = %d",(int)_sock,SYS::getLastErr(SYS::SOCK));
		return false;
	}
	
	SIMSLOG(ZQ::common::Log::L_DEBUG,"SimpleHttpd::start() listen OK");


	return true;
}

void SimpleHttpd::setTimeout(int timeOut) 
{
	_nTimeOut = timeOut;
	if(_nTimeOut > MAX_TIMEOUT)
		_nTimeOut = MAX_TIMEOUT;
}

int SimpleHttpd::run(void)
{
	SOCKET_T newSock;
	struct sockaddr_in newAddr;
	socklen_t len = sizeof(struct sockaddr_in);
	while(!_bExit)
	{
		memset(&newAddr,0,sizeof(struct sockaddr_in));
		newSock = accept(_sock, (struct sockaddr*)&newAddr,&len);
		if(newSock == (SOCKET_T)-1)
		{
			SIMSLOG(ZQ::common::Log::L_WARNING,"SimpleHttpd::start() accept a invalidation socket errorcode: %d",SYS::getLastErr(SYS::SOCK));
			continue;
		}
		else//add to the thread pool
		{
			SIMSLOG(ZQ::common::Log::L_DEBUG,"SimpleHttpd::start() accept a socket %d",(int)newSock);
			HttpHandlerProc* pSimpleSerTh = new HttpHandlerProc(_pool, newSock, _nTimeOut, _pLog, this);
			pSimpleSerTh->start();		
		}
	}

	return 0;
}

int SimpleHttpd::OnHttpMessage(const MsgInfo& info, const std::string& contentBody)
{

#ifdef _DEBUG
	printf("%s\n",contentBody.c_str());
#endif
	return MSG_OK;
}

void SimpleHttpd::close()
{ 
	_bExit = true;
	_closesocket(_sock);
	_pool.stop();
	waitHandle(2000);
	SIMSLOG(ZQ::common::Log::L_DEBUG,"SimpleHttpd closed");
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
