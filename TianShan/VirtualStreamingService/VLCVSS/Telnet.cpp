#include "Telnet.h"

namespace ZQ {
namespace common {

Telnet::Telnet()
{
	_bUsed = false;
	_bConnected = false;
}

Telnet::Telnet(const InetAddress &host, tpport_t port /* = 23 */)
{
	setPeer(host, port);
}

Telnet::~Telnet()
{
}

void Telnet::setPeer(const InetAddress &host, tpport_t port)
{
	_TCPSocket.setPeer(host, port);
}

void Telnet::setPWD(const char *strPWD)
{
	_strPWD.clear();
	_strPWD = strPWD;
	size_t len = strlen(strPWD);
	if (_strPWD[len-1] != '\n' || _strPWD[len-2] != '\r')
		_strPWD += "\r\n";
}

bool Telnet::connectToServer(int32 timeOut /* = -1 */)
{
	if (_TCPSocket.connect(timeOut))
	{
		if (_strPWD.empty())
			return true;
		else
		{
			//receive welcome alert first
			int ret = _TCPSocket.receive(_recvBuf, TELNETRECVLEN, NONBLOCK, timeOut);
			if (ret < 0)
				return false;

			//send password
			ret = _TCPSocket.send(_strPWD.c_str(), _strPWD.length());
			if (ret != _strPWD.length())
				return false;

			//check status
			ret = _TCPSocket.receive(_recvBuf, TELNETRECVLEN, NONBLOCK, timeOut);
			if (ret < 0)
				return false;

			//return success
			_bConnected = true;
			return true;
		}
	}
	return false;
}

bool Telnet::sendCMD(const char* strCMD, std::string &strResMsg, int32 timeOut/* = -1*/)
{
	//use mutex to guarantee only one request in the same time
	ZQ::common::MutexGuard guard(_mutex);
	//check the send buffer
	if (strCMD == NULL)
		return false;

	//try to recv first byte in order to detect tcp connect status
	int recvLen = 0;
	int ret = 0;
	ret = _TCPSocket.receive(_recvBuf + recvLen, 1, NONBLOCK, 0);
	if (ret == -1)//connection lost
	{
		//try to reconnect
		if (connectToServer(0) == false)
		{
			_bConnected = false;
			return false;
		}
		recvLen = 0;
	}
	else
		recvLen += ret;

	//send command
	std::string _strCMD = strCMD;
	size_t sendLen = _strCMD.length();
	if (strCMD[sendLen - 1] != '\n' || strCMD[sendLen - 2] != '\r')
		_strCMD += "\r\n";
	ret = _TCPSocket.send(_strCMD.c_str(), _strCMD.length());
	if (ret != _strCMD.length())
		return false;

	//recv response message
	if (timeOut == -1)
		ret = _TCPSocket.receive(_recvBuf + recvLen, TELNETRECVLEN - recvLen, BLOCK);
	else
	{
		DWORD sTime = GetTickCount();
		DWORD eTime = GetTickCount();
		while (/*(recvLen < TELNETRECVLEN) && */(eTime-sTime < timeOut))
		{
			ret = _TCPSocket.receive(_recvBuf + recvLen, TELNETRECVLEN - recvLen, NONBLOCK, 0);
/*			if (ret == 0)
				break;
			else */if (ret < 0)
				return false;
			else
				recvLen += ret;
			_recvBuf[recvLen] = 0;
			strResMsg += ::std::string(_recvBuf);
			recvLen = 0;
			eTime = GetTickCount();
		}
	}

	//recv occurs error
	if (ret < 0)
		return false;

	//return recv string
	//recvLen += ret;
	//_recvBuf[recvLen] = 0;
	//strResMsg = _recvBuf;
	return true;
}

std::string Telnet::getLastErr()
{
	ZQ::common::MutexGuard guard(_mutex);
	return _strLastErr;
}

}//namespace common

}//namespace ZQ