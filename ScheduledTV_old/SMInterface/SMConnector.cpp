  // SMConnector.cpp: implementation of the CSMConnector class.
//
//////////////////////////////////////////////////////////////////////


#include "SMConnector.h"
#include "xmlwritestream.h"

#include "..\MainCtrl\ScheduleTV.h"
#include "Log.h"
#include "ScLog.h"

using namespace ZQ::common;


const int HEART_BEAT_TIME = 6000;




//////////////////////////////////////////////////////////////////////////
/// sleep specific time, and can break by change a value
///@param dwMill		time to sleep,  in milliseconds
///@param dwElem		time to check, sleep dwElem then check if break
void SleepWithBreak(DWORD dwMill, DWORD dwElem, bool* bSleep)
{
	while(*bSleep && dwMill)
	{
		if (dwElem<dwMill)
		{
			Sleep(dwElem);
			dwMill -= dwElem;
		}
		else
		{
			Sleep(dwMill);
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CSMConnector::CSMConnector():Socket(AF_INET, SOCK_STREAM, 0)
{
	_sBindIP[0] = '\0';
	_sSMIP[0] = '\0';
	_nPort = 0;
		
	_bWorking = false;
	
}


CSMConnector::~CSMConnector()
{
	close();
}


int CSMConnector::ConnectSM(const char* sBindIP, const char* sSMIP, int nSMPort)
{
	resetSocket();
 
	//////////////////////////////////////////////////////////////////////////
	// bind local ip
	struct sockaddr_in bindaddr;
	memset(&bindaddr, 0, sizeof(bindaddr));
	bindaddr.sin_port = 0;
	bindaddr.sin_family = AF_INET;
	bindaddr.sin_addr.S_un.S_addr = inet_addr(sBindIP);
	
	
	// bind local ip 
	bind(_so, (const struct sockaddr*)&bindaddr, sizeof(const struct sockaddr));


	// set property
	setNoDelay(true);


	// connect
	struct sockaddr_in inaddr;
	memset(&inaddr, 0, sizeof(inaddr));
	inaddr.sin_port = htons(nSMPort);
	inaddr.sin_family = AF_INET;
	inaddr.sin_addr.S_un.S_addr = inet_addr(sSMIP);

	

	DWORD dwRet;

#ifdef TEST_BREAK_TIME
	DWORD dw1 = GetTickCount();
#endif

	dwRet = connect(_so, (struct sockaddr*)&inaddr, sizeof(inaddr));
	if (dwRet == SOCKET_ERROR)
	{
#ifdef TEST_BREAK_TIME
		printf("connect_error: %d\n", GetTickCount()-dw1);
#endif
		
		return connectError();
	}

	_state = CONNECTED;



	// get the local socket port
	int len = sizeof(bindaddr);
	getsockname(_so, (struct sockaddr *)&bindaddr, &len);
	_nLocalPort = ntohs(bindaddr.sin_port);



#ifdef TEST_BREAK_TIME
	printf("Connected: %d\n", GetTickCount()-dw1);
#endif
	

	return errSuccess;
}


int CSMConnector::run()
{	
	_bWorking = true;
	
	int nRet;		// use for return value

	int nRead;		// use to save already received data count

	char* pBegin;	// begin of a message 
	char* pPtr;		// temp pointer
	char* pEnd;		// end of a message

	
	SEND_DATA_MSG* pMsg;
	bool  bTranslateOk;

	DWORD	dwLastHeartBeat;


	glog(Log::L_DEBUG, "SMConnector::run()  Enter Connect Thread!");
	
	//do the com initialize
	xmlProc.CoInit();


	

	while(_bWorking)
	{
		glog(Log::L_INFO, "SMConnector:run()  bind ip: %s, Connect to ip: %s  port:%d", _sBindIP, _sSMIP, _nPort);
		
		nRet = ConnectSM(_sBindIP, _sSMIP, _nPort);

		if (nRet != errSuccess)
		{
			switch(nRet) 
			{
				case errResourceFailure:
					glog(Log::L_WARNING, "Connect Fail:Connection Resource Failure!");					
					SleepWithBreak(TIME_RF_WAIT, TIME_MINI, &_bWorking);
					break;
				case errConnectBusy:
					glog(Log::L_WARNING, "Connect Fail:Connection Busy!");
					SleepWithBreak(TIME_CB_WAIT, TIME_MINI, &_bWorking);
					break;
				case errConnectRefused:
					glog(Log::L_WARNING, "Connect Fail:Connection Refused!");
					SleepWithBreak(TIME_CR_WAIT, TIME_MINI, &_bWorking);
					break;
				default:
					glog(Log::L_WARNING, "Connect Fail!");
					SleepWithBreak(TIME_DEFAULT_WAIT, TIME_MINI, &_bWorking);
			}

			continue;		
		}


		glog(Log::L_INFO, "SMConnector::run()  Connected");

		// do some initialize
		nRead = 0;		// data already read

		bTranslateOk = true;	//if data translate ok



		/// connect ok, so send the handshake message
		if (SendHandShakeMsg() != errSuccess)
			continue;

		dwLastHeartBeat = GetTickCount();

		// invoke main control event	
		_pSTV->OnConnected();



		/// receive message and process

		while(_bWorking)
		{
			while(_bWorking)
			{
				if(isPending(pendingInput, TIME_MINI))
					break;
				else
				{
					// check && process send data
					DWORD dwCur = GetTickCount();
					if (dwCur - dwLastHeartBeat > HEART_BEAT_TIME)
					{
						if (SendHandShakeMsg() != errSuccess)
						{
							bTranslateOk = false;
							break;
						}
						
						dwLastHeartBeat = dwCur;
					}
		
					
					// see if have data to send
					if (!_qSendData.empty())
					{
						// get out message
						_mutexSendQueue.enter();


						pMsg = _qSendData.front();
						_qSendData.pop();


						_mutexSendQueue.leave();



						// process message
						
						if (SendData(pMsg->pData, pMsg->nLen) != errSuccess)
						{
							delete[]  pMsg->pData;
							delete	pMsg;

							bTranslateOk = false;

							break;
						}

						delete[]  pMsg->pData;
						delete	pMsg;
					}
					

				}
			}


			if (!_bWorking || !bTranslateOk)
			{
				break;
			}

			// receive
			nRet = recv(_so, _sXmlBuf + nRead, MAX_XML_BUF - nRead, 0);
			if (nRet > 0)
			{
				nRead += nRet;	// adjust read data length

				
				pBegin = _sXmlBuf;
				pPtr=_sXmlBuf;
				pEnd = _sXmlBuf + nRead;


				while(pPtr<pEnd)
				{
					while(pPtr<pEnd && *pPtr!=MSG_END_FLAG)
					{
						pPtr++;
					}

					if (pPtr<pEnd)
					{
						*pPtr++ = '\0';		// ternite this string

						if (pBegin[0])
						{
							try
							{
								xmlProc.XmlProc(pBegin);
							}
							catch(ZQ::common::Exception excp)
							{
								printf("there is a error in xmlproc : %s\n", excp.getString());
								glog(Log::L_ERROR, "SMConnector::run()  there is a error in xmlproc!");
							}
							catch (...) 
							{
								printf("there is a error in xmlproc!\n");
								glog(Log::L_ERROR, "SMConnector::run()  there is a error in xmlproc!");
							}
						}
							

						pBegin = pPtr;
					}				
				}


				if (pBegin < pEnd)
				{
					nRead = pEnd - pBegin;

					if (pBegin != _sXmlBuf)
						MoveMemory(_sXmlBuf, pBegin, nRead);					
				}
				else
				{
					nRead = 0;
				}
			}
			else
			{
				// means network error, break current layer to reconnect 
				break;
			}
		}


		if (_bWorking)
		{
			// sleep sometime to reconnect
			SleepWithBreak(TIME_DEFAULT_WAIT, TIME_MINI, &_bWorking);
		}
		

		if (_bWorking)
			glog(Log::L_INFO, "SMConnector::run()  Reconnect");
		else
			glog(Log::L_DEBUG, "SMConnector::run()  Exit Run While");
	}


	// do com uninit
	xmlProc.CoUnInit();


	glog(Log::L_DEBUG, "SMConnector::run()  Leave Connect Thread!");


	return 0;
}


void CSMConnector::resetSocket()
{
	endSocket();
	
	setSocket();

	_so = socket(AF_INET, SOCK_STREAM, 0);
	if(_so == INVALID_SOCKET)
	{
		error(errCreateFailed);
		return;
	}
	_state = AVAILABLE;
}


int CSMConnector::RecvData(char* sBuf, int nLen, bool* _bWorking)
{
	int nRead = 0;
	int nRet;

	while(*_bWorking && nRead<nLen)
	{
		while(*_bWorking)		
		{
			if(isPending(pendingInput, TIME_MINI))
				break;
		}

		if (!*_bWorking)
			break;

		nRet = recv(_so, sBuf + nRead, nLen - nRead, 0);

		//////////////////////////////////////////////////////////////////////////
		// sometimes nRet is 0, when connect lost
		if (nRet <= 0)
		{
			return errInput;
		}

		nRead += nRet;
	}

	return errSuccess;
}


int CSMConnector::SendData(const char* sBuf, int nLen)
{
	int nSend = 0;
	int nRet;


	while(nSend<nLen)
	{
		nRet = send(_so, sBuf + nSend, nLen - nSend, 0);
		if (nRet == SOCKET_ERROR)
		{
			return errInput;
		}
		nSend += nRet;
	}

	return errSuccess;
}


void CSMConnector::SetParam(ScheduleTV* pSITV, int nClientID, const char* sBindIP, const char* sSMIP, int nSMPort)
{
	strcpy(_sBindIP, sBindIP);
	strcpy(_sSMIP, sSMIP);
	_nPort = nSMPort;
	_nClientID = nClientID;
	_pSTV = pSITV;

	xmlProc.SetMainControl(pSITV);
	xmlProc.SetConnectionObject(this);
}


void CSMConnector::close()
{
	if (_bWorking)
	{
		_bWorking = false;

		if(!wait(10000))
			terminate(1);

		endSocket();
		
		glog(Log::L_DEBUG, "SMConnector::run()  after close socket");		
	}
}

void CSMConnector::SendStatusFeedback(const char* sContent)
{
	char sBuf[MAX_XML_BUF];
	int nXmlLen;

	// fill the xml content in the buf, and return the content length
	xmlProc.XmlStatusFeedback(
		sContent,
		sBuf,
		&nXmlLen);

	sBuf[nXmlLen] = MSG_END_FLAG;

	nXmlLen++;

	SEND_DATA_MSG* pMsg = new SEND_DATA_MSG;
	pMsg->pData = new char[nXmlLen];
	pMsg->nLen = nXmlLen;

	memcpy(pMsg->pData, sBuf, nXmlLen);


	_mutexSendQueue.enter();
	_qSendData.push(pMsg);
	_mutexSendQueue.leave();
}


void CSMConnector::SendEnquireSchedule(const char* sContent)
{
	char sBuf[MAX_SEND_XML_BUF];

	int nXmlLen;
	
	xmlProc.XmlEnquireSchedule(
		sContent,
		sBuf,
		&nXmlLen);

	sBuf[nXmlLen] = MSG_END_FLAG;

	nXmlLen++;


	SEND_DATA_MSG* pMsg = new SEND_DATA_MSG;
	pMsg->pData = new char[nXmlLen];
	pMsg->nLen = nXmlLen;

	memcpy(pMsg->pData, sBuf, nXmlLen);


	_mutexSendQueue.enter();
	_qSendData.push(pMsg);
	_mutexSendQueue.leave();	
}



void CSMConnector::SendResponse(int nMsgID, bool bSuccess, int nErrorCode /* = 0 */, const char* sErrorString /* = NULL */)
{
	char sBuf[MAX_SEND_XML_BUF];

	int nXmlLen;
	
	nXmlLen = xmlProc.XmlResponse(
		sBuf,
		nMsgID,
		bSuccess,
		nErrorCode,
		sErrorString);

	sBuf[nXmlLen] = MSG_END_FLAG;

	SendData(sBuf, nXmlLen + 1);	
}



int CSMConnector::SendHandShakeMsg()
{
	char sBuf[MAX_SEND_XML_BUF];
	int nXmlLen;
	
	nXmlLen = xmlProc.XmlHandShakeMsg(sBuf, _nClientID);

	sBuf[nXmlLen] = MSG_END_FLAG;

	return SendData(sBuf, nXmlLen + 1);	
}


void CSMConnector::SendEnquireConfig()
{
	char sBuf[MAX_SEND_XML_BUF];

	int nXmlLen;
	
	nXmlLen = xmlProc.XmlEnquireConfig(sBuf, _nClientID);

	sBuf[nXmlLen] = MSG_END_FLAG;

	nXmlLen++;


	SEND_DATA_MSG* pMsg = new SEND_DATA_MSG;
	pMsg->pData = new char[nXmlLen];
	pMsg->nLen = nXmlLen;

	memcpy(pMsg->pData, sBuf, nXmlLen);


	_mutexSendQueue.enter();
	_qSendData.push(pMsg);
	_mutexSendQueue.leave();	
}


void CSMConnector::SendQueryFillerList(const char* sContent)
{
	char sBuf[MAX_SEND_XML_BUF];

	int nXmlLen;
	
	xmlProc.XmlQueryFillerList(
		sContent,
		sBuf,
		&nXmlLen);

	sBuf[nXmlLen] = MSG_END_FLAG;

	nXmlLen++;


	SEND_DATA_MSG* pMsg = new SEND_DATA_MSG;
	pMsg->pData = new char[nXmlLen];
	pMsg->nLen = nXmlLen;

	memcpy(pMsg->pData, sBuf, nXmlLen);


	_mutexSendQueue.enter();
	_qSendData.push(pMsg);
	_mutexSendQueue.leave();	
}

