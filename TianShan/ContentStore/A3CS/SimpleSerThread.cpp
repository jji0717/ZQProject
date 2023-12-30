#include "SimpleSerThread.h"

#define SSTHLOG if(_pLog != NULL) (*_pLog)
#define SSTHC	"SimpleSerThread"
#define MAX_TIMEOUT 600//ten minute

using namespace ZQ::common;

SimpleSerThread::SimpleSerThread(ZQ::common::NativeThreadPool& pool,SOCKET_T sock, int timeOut, ZQ::common::Log* pLog, SimpleServer* pSimSer)
:ZQ::common::ThreadRequest(pool)
{
	_sock = sock;
	_pLog = pLog;
	_pSimSer = pSimSer;	
	_nTimeOut = timeOut;
}

SimpleSerThread::~SimpleSerThread(void)
{
	
}

bool SimpleSerThread::init(void)
{
	if(_sock == (SOCKET_T)-1)
	{
		SSTHLOG(Log::L_ERROR,"SimpleSerThread init erro: socket is invalidation");
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

int SimpleSerThread::run(void)
{
	while(!_pSimSer->bExit())
	{
		if(!recvData())
		{
			SSTHLOG(Log::L_ERROR,CLOGFMT(SSTHC,"run() receive data error"));
			break;
		}
		
		std::string strRes;
		int code = _pSimSer->handleMsg(_chContType, _strContent);
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
			SSTHLOG(Log::L_ERROR,CLOGFMT(SSTHC, "run() send response error"));
			break;
		}

		if(!_bAlive)//is keep alive
		{
			Sleep(10);
			break;
		}
	}

	closesocket(_sock);
	return 0;
}

void SimpleSerThread::final(int retcode, bool bCancelled)
{
	delete this;
}

bool SimpleSerThread::recvData()
{
	_bContinue = false;
	_strContent = "";
	_needRecv = 0;
	memset(_chContType,0,sizeof(_chContType));

	if(!beginRecv())
	{
		SSTHLOG(Log::L_ERROR,CLOGFMT(SSTHC,"begin receive error,maybe socket has closed"));
		return false;
	}

	while (!isEOF())
	{
		if(!continueRecv())
		{
			SSTHLOG(Log::L_ERROR,CLOGFMT(SSTHC,"continue receive error"));
			return false;
		}
	}
	
	return true;
}

bool SimpleSerThread::beginRecv()
{
	memset(_buf, 0, sizeof(_buf));
	_bufIdx = 0;
	_bufLen = 0;

	char chR = getChar();
	if(chR == '\0')
	{
		SSTHLOG(Log::L_ERROR,CLOGFMT(SSTHC,"not receive data"));
		return false;
	}

	if(!_bContinue)
	{
		if(!parseData())
		{
			SSTHLOG(Log::L_ERROR,CLOGFMT(SSTHC,"parse data error"));
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

char SimpleSerThread::getChar()
{
	if (_bufIdx >= _bufLen && getData() == 0)
		return '\0';

	return _buf[_bufIdx++];
}

int SimpleSerThread::getData()
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
			SSTHLOG(Log::L_ERROR,CLOGFMT(SSTHC,"select() receive date error code: %d"),WSAGetLastError());
			_bufLen = 0;
			return 0;
		}
		else if(res == 0)
		{
			SSTHLOG(Log::L_WARNING,CLOGFMT(SSTHC,"select() receive data time out"));
			_bufLen = 0;
			return 0;
		}
		
	}
	
	_bufLen = recv(_sock, _buf, sizeof(_buf)-1, 0);

	if(_bufLen < 0)//have error
	{
		SSTHLOG(Log::L_ERROR,CLOGFMT(SSTHC,"received zero number data errorcode = %d"),WSAGetLastError());
		_bufLen = 0;
		return 0;
	}
	else if(_bufLen == 0)//socket has closed
	{
		SSTHLOG(Log::L_WARNING,CLOGFMT(SSTHC,"the connection has been closed"));
		return 0;
	}

	return _bufLen;
}

bool SimpleSerThread::isEOF()
{
	return (_needRecv <= 0) ? true : false;
}

bool SimpleSerThread::continueRecv()
{
	_bContinue = true;
	return beginRecv();
}

bool SimpleSerThread::parseData()
{
	_bufIdx = 0;
	char chLine[1024];

	//get start line
	if(!getLine(chLine, sizeof(chLine)))
	{
		SSTHLOG(Log::L_ERROR,CLOGFMT(SSTHC,"can not get start line"));
		return false;
	}
	//it is a post packet
	if(strncmp(chLine,"POST",4) != 0)
	{
		SSTHLOG(Log::L_ERROR,CLOGFMT(SSTHC,"it is not a post packet"));
		return false;
	}

	char* s = NULL;
	unsigned short nstatus = 0;
	if ((s = strchr(chLine, ' ')))
	{
		s++;
		char* end = NULL;
		end = strrchr(chLine,' ');
		if(end != NULL)
			*end = '\0';
		else
			return false;

		if((*s == '/') || (*s == '\\'))
			s++;
		strcpy(_chContType,s);
	}
	else
	{
		SSTHLOG(Log::L_ERROR,CLOGFMT(SSTHC,"parse start line"));
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

bool SimpleSerThread::getLine(char* buf, size_t bufLen)
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

bool SimpleSerThread::sendData(std::string& strRes)
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
				SSTHLOG(Log::L_ERROR,CLOGFMT(SSTHC,"select() send date error code: %d"),WSAGetLastError());
				return false;
			}
			else if(res == 0)
			{
				SSTHLOG(Log::L_WARNING,CLOGFMT(SSTHC,"select() send data time out"));
				return false;
			}
		}

		int nHasS = send(_sock, strSend.c_str(), static_cast<int>(strSend.length()), 0);
		if(nHasS < 0)//have error
		{
			SSTHLOG(Log::L_ERROR,CLOGFMT(SSTHC,"send data has error code: %d"),WSAGetLastError());	
			return false;
		}
		else if(nHasS == 0)//block maybe has a timeout
		{
			nTry--;
			Sleep(1);
		}
		else
			nCount -= nHasS;
		
	}while(nCount && nTry);

	if(nCount > 0)//not send all data
	{
		SSTHLOG(Log::L_ERROR,CLOGFMT(SSTHC,"send data is blocking"));	
		return false;
	}
	
	return true;
}
