#ifndef __SIMPLESERTHREAD_H__
#define __SIMPLESERTHREAD_H__

#pragma once

#include "NativeThreadPool.h"
#include "SimpleServer.h"
#include "Log.h"
#include <string>

#define SIMSERTH_BUFLEN 1024

class SimpleSerThread :	public ZQ::common::ThreadRequest
{
public:
	SimpleSerThread(ZQ::common::NativeThreadPool& pool,SOCKET_T sock, int timeOut, ZQ::common::Log* pLog, SimpleServer* pSimSer);
	virtual ~SimpleSerThread(void);

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
	SimpleServer*	_pSimSer;
	SOCKET_T		_sock;
	ZQ::common::Log* _pLog;
	char			_buf[SIMSERTH_BUFLEN];
	std::string		_strContent;
	char			_chContType[30];//content type like:TransferStatus

	int				_nTimeOut;//unit is second
	int				_bufIdx;
	int				_bufLen;
	int				_needRecv;
	bool			_bContinue;
	bool			_bAlive;
};

#endif //__SIMPLESERTHREAD_H__