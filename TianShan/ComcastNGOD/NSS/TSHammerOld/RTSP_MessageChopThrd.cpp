#include "RTSP_MessageChopThrd.h"
#include "RTSP_MessageParser.h"


RTSP_MessageChopThrd::RTSP_MessageChopThrd(::ZQ::common::FileLog &fileLog, RTSPMessageBuffer *rtspMessageBuffer/*, RTSPMessageList &rtspMessageList*/, SessionMap &sessionMap)
:_fileLog(&fileLog)
,_rtspMessageBuffer(rtspMessageBuffer)
,_sessionMap(sessionMap)
//,_rtspMessageList(rtspMessageList)
{
	_rtspParseThrd = new RTSP_MessageParseThrd(fileLog, _rtspMessageList, sessionMap);
}

RTSP_MessageChopThrd::~RTSP_MessageChopThrd()
{
	::ZQ::common::MutexGuard guard(_mutex);
	_socketList.clear();
	if (NULL != _rtspParseThrd)
		delete _rtspParseThrd;
}

void RTSP_MessageChopThrd::startThrd()
{
	start();
	_rtspParseThrd->start();
}

int RTSP_MessageChopThrd::run(void)
{
	bool bSessionStatus = false;
	char strTmpBuffer[1024*5];
	//memset(strTmpBuffer, 0, 1024*5);
	RTSPMessageList	*pMessList = &_rtspMessageList;

	while (1)
	{
		while (NULL == _rtspMessageBuffer)
			Sleep(1000);

		bSessionStatus = false;
		if (_rtspMessageBuffer->size() > 0)
		{
			//_rtspMessageBuffer->readLock();
			//::ZQ::common::MutexGuard guard(_rtspMessageBuffer->_mutex);
			//for (MessageBufferMap::iterator iter = _rtspMessageBuffer->_messageBufferMap.begin(); iter != _rtspMessageBuffer->_messageBufferMap.end(); iter++)
			::ZQ::common::MutexGuard guard(_mutex);
			for (SOCKETList::iterator iter = _socketList.begin(); iter != _socketList.end(); iter++)
			{
				//define a pointer for convenient use
				MessageBufferIterater	*pSmartBuffer = _rtspMessageBuffer->getMessageBuffer(*iter);
				if (pSmartBuffer == NULL)
					SleepContinue;

				int32 &iReadPos	= pSmartBuffer->iReadPos;
				int32 iWritePos= pSmartBuffer->iWritePos;

				//check the read and write pointer position
				//if buffer could not read, then check next buffer
				if (iReadPos + 1 == iWritePos)
					SleepContinue
					//continue;
				else if (iWritePos > iReadPos)
				{
					if (iReadPos + 4 + 1 > iWritePos)
						SleepContinue
						//continue;
				}
				else
				{
					if (RTSPMessageBufferSize - (iReadPos + 1) + iWritePos <= 4)
						SleepContinue
						//continue;
				}

				DWORD sTime = GetTickCount();
				bSessionStatus = true;

				//use iBufferLen to decide the chop len this loop
				//the rest of this buffer will be chopped next time
				int32 iBufferLen = 0;
				int32 iTotalChopLen = 0;

				//chop current message buffer
				while(1)
				{
					iWritePos= pSmartBuffer->iWritePos;
					//decide the buffer len
					if (iWritePos > iReadPos)
						iBufferLen = iWritePos - iReadPos - 1 + iTotalChopLen;
					else
						iBufferLen = RTSPMessageBufferSize - iReadPos - 1 + iWritePos + iTotalChopLen;		
					if (iTotalChopLen > iBufferLen / 2)
						break;

					//if (iReadPos + 1 == iWritePos)
					//	break;

					int iTmpRTSPBufSize = 0;
					int iMessageLen = 0;
					int iCurrentPos = (iReadPos + 1) % RTSPMessageBufferSize;

					while(pSmartBuffer->strBuffer[(iReadPos + 1) % RTSPMessageBufferSize] != '\r'
						|| pSmartBuffer->strBuffer[(iReadPos + 2) % RTSPMessageBufferSize] != '\n'
						|| pSmartBuffer->strBuffer[(iReadPos + 3) % RTSPMessageBufferSize] != '\r'
						|| pSmartBuffer->strBuffer[(iReadPos + 4) % RTSPMessageBufferSize] != '\n')
					{
						if (iReadPos + 1 == RTSPMessageBufferSize)
						{
							//memcpy(strTmpBuffer + iTmpRTSPBufSize, pSmartBuffer->strBuffer + iCurrentPos, iMessageLen);
							memcpy(strTmpBuffer + iTmpRTSPBufSize, &pSmartBuffer->strBuffer[iCurrentPos], iMessageLen);
							iTmpRTSPBufSize += iMessageLen;
							iReadPos = -1;
							iMessageLen = 0;
							iCurrentPos = (iReadPos + 1) % RTSPMessageBufferSize;
						}
						else
						{
							iMessageLen++;
							iReadPos++;
							while (iReadPos + 1 == pSmartBuffer->iWritePos)
								Sleep(1);

						}
					}//try to find "\r\n\r\n"
					//memcpy(strTmpBuffer + iTmpRTSPBufSize, pSmartBuffer->strBuffer + iCurrentPos, iMessageLen);
					memcpy(strTmpBuffer + iTmpRTSPBufSize, &pSmartBuffer->strBuffer[iCurrentPos], iMessageLen);
					iTmpRTSPBufSize += iMessageLen;
					iTotalChopLen += iTmpRTSPBufSize;

					//copy "\r\n\r\n"
					strTmpBuffer[iTmpRTSPBufSize++] = pSmartBuffer->strBuffer[(iReadPos + 1) % RTSPMessageBufferSize];
					strTmpBuffer[iTmpRTSPBufSize++] = pSmartBuffer->strBuffer[(iReadPos + 2) % RTSPMessageBufferSize];
					strTmpBuffer[iTmpRTSPBufSize++] = pSmartBuffer->strBuffer[(iReadPos + 3) % RTSPMessageBufferSize];
					strTmpBuffer[iTmpRTSPBufSize++] = pSmartBuffer->strBuffer[(iReadPos + 4) % RTSPMessageBufferSize];
					iReadPos = (iReadPos + 4) % RTSPMessageBufferSize;

					//try to find if have content
					int iContentLen = RTSPMessageParser::GetContentLength(strTmpBuffer, iTmpRTSPBufSize);
					if (iContentLen > 0)
					{
						if (iReadPos + 1 + iContentLen > RTSPMessageBufferSize)
						{
							//check the buffer state
							while (1)
							{
								iWritePos= pSmartBuffer->iWritePos;
								if (iWritePos <= iReadPos)
								{
									if ((iReadPos + 1 + iContentLen) % RTSPMessageBufferSize < iWritePos)
										break;
									else
										Sleep(1);
								}
								else
									Sleep(1);
							}
							int i = RTSPMessageBufferSize - iReadPos - 1;
							//memcpy(strTmpBuffer + iTmpRTSPBufSize, pSmartBuffer->strBuffer + iReadPos + 1, i);
							//memcpy(strTmpBuffer + iTmpRTSPBufSize + i, pSmartBuffer->strBuffer, iContentLen - i);
							memcpy(strTmpBuffer + iTmpRTSPBufSize, &pSmartBuffer->strBuffer[iReadPos + 1], i);
							memcpy(strTmpBuffer + iTmpRTSPBufSize + i, &pSmartBuffer->strBuffer[0], iContentLen - i);
							iReadPos = iContentLen - i - 1;
						}
						else
						{
							//check the buffer state
							while (1)
							{
								iWritePos= pSmartBuffer->iWritePos;
								if (iWritePos > iReadPos)
								{
									if (iReadPos + 1 + iContentLen <= iWritePos)
										break;
									else
										Sleep(1);
								}
								else
									break;
							}
							//memcpy(strTmpBuffer + iTmpRTSPBufSize, pSmartBuffer->strBuffer + iReadPos + 1, iContentLen);
							memcpy(strTmpBuffer + iTmpRTSPBufSize, &pSmartBuffer->strBuffer[iReadPos + 1], iContentLen);
							iReadPos += iContentLen;
						}

						iReadPos = iReadPos % RTSPMessageBufferSize;
						iTmpRTSPBufSize += iContentLen;
						iTotalChopLen += iTmpRTSPBufSize;
					}//if(iContentLen > 0)

					strTmpBuffer[iTmpRTSPBufSize] = 0;
					MYLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTSP_MessageChopThrd,"chop one message cost %dms"), GetTickCount() - sTime);

					//cout << "chop one message\r\n" << strTmpBuffer << endl;
					//write log
					//MYLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTSP_MessageChopThrd,"chop one message\r\n%s"), strTmpBuffer);

					//put one message into message deque
					SessionHandler *sess = _sessionMap.getSessionHandler(*iter);
					if (sess != NULL)
						MYLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTSP_MessageChopThrd,"socket(%d) request-response time interval: %dms\r\n"), *iter, GetTickCount() - sess->_sessionSocket->_postMessageTime);
					pMessList->PushBack((void *)sess, string(strTmpBuffer));
					//pMessList->PushBack(string(strTmpBuffer));
				}//while(1)
				if (!bSessionStatus)
					Sleep(10);
			}
		}//if(...)
	}//while(1)

	return 1;
}