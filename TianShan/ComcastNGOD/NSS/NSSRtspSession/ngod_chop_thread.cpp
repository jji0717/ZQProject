#include "ngod_chop_thread.h"
#include "ngod_rtsp_parser/RTSPMessage/RTSPMessageParser.h"

#define MYLOG (*m_pFileLog)

ngod_chop_thread::ngod_chop_thread(ZQ::common::FileLog *logfile)
:m_NSSSessionGroupList(NULL),
m_pFileLog(logfile)
{
}

ngod_chop_thread::ngod_chop_thread(ZQ::common::FileLog *logfile, 
								   NSSSessionGroupList &_NSSSessionGroupList)
:m_NSSSessionGroupList(&_NSSSessionGroupList),
m_pFileLog(logfile)
{
}

ngod_chop_thread::~ngod_chop_thread()
{
	m_NSSSessionGroupList = NULL;
	m_pFileLog = NULL;
}

void ngod_chop_thread::setNSSSessionGroupList(NSSSessionGroupList &_NSSSessionGroupList)
{
	m_NSSSessionGroupList = &_NSSSessionGroupList;
}

int ngod_chop_thread::run(void)
{
	bool bSessionStatus = false;
	char strTmpBuffer[1024*50];
	//memset(strTmpBuffer, 0, 1024*5);

	while (1)
	{
		while (m_NSSSessionGroupList == NULL)
			Sleep(1000);

		bSessionStatus = false;
		//check the socket in this list
		if (m_NSSSessionGroupList->size() > 0)
		{
			for (NSSSessionGroupList::iterator iter = m_NSSSessionGroupList->begin();
			iter != m_NSSSessionGroupList->end(); iter++)
			{
				//define a pointer for convenient use
				NSSRTSPMessageBuffer	*pSmartBuffer = &((*iter)->m_NSSRTSPMessageBuffer);
				NSSRTSPMessageList		*pMessList = &((*iter)->m_NSSRTSPMessageList);
				int32 &iReadPos	= pSmartBuffer->iReadPos;
				int32 &iWritePos= pSmartBuffer->iWritePos;

				//check the read and write pointer position
				//if buffer could not read, then check next buffer
				if (iReadPos + 1 == iWritePos)
					continue;
				else if (iWritePos > iReadPos)
				{
					if (iReadPos + 4 + 1 > iWritePos)
						continue;
				}
				else
				{
					if (NSSRTSPMessageBufferSize - (iReadPos + 1) + iWritePos <= 4)
						continue;
				}

				bSessionStatus = true;

				//use iBufferLen to decide the chop len this loop
				//the rest of this buffer will be chopped next time
				int32 iBufferLen = 0;
				int32 iTotalChopLen = 0;
				//chop current message buffer
				while(1)
				{
					//decide the buffer len
					if (iWritePos > iReadPos)
						iBufferLen = iWritePos - iReadPos - 1 + iTotalChopLen;
					else
						iBufferLen = NSSRTSPMessageBufferSize - iReadPos - 1 + iWritePos + iTotalChopLen;		
					if (iTotalChopLen > iBufferLen / 2)
						break;

					//if (iReadPos + 1 == iWritePos)
					//	break;
					
					int iTmpRTSPBufSize = 0;
					int iMessageLen = 0;
					int iCurrentPos = (iReadPos + 1) % NSSRTSPMessageBufferSize;
				
					while(pSmartBuffer->strBuffer[(iReadPos + 1) % NSSRTSPMessageBufferSize] != '\r'
						|| pSmartBuffer->strBuffer[(iReadPos + 2) % NSSRTSPMessageBufferSize] != '\n'
						|| pSmartBuffer->strBuffer[(iReadPos + 3) % NSSRTSPMessageBufferSize] != '\r'
						|| pSmartBuffer->strBuffer[(iReadPos + 4) % NSSRTSPMessageBufferSize] != '\n')
					{
						if (iReadPos + 1 == NSSRTSPMessageBufferSize)
						{
							memcpy(strTmpBuffer + iTmpRTSPBufSize, pSmartBuffer->strBuffer + iCurrentPos, iMessageLen);
							iTmpRTSPBufSize += iMessageLen;
							iReadPos = -1;
							iMessageLen = 0;
							iCurrentPos = (iReadPos + 1) % NSSRTSPMessageBufferSize;
						}
						else
						{
							iMessageLen++;
							iReadPos++;
							while (iReadPos + 1 == iWritePos)
								Sleep(1);

						}
					}//try to find "\r\n\r\n"
					memcpy(strTmpBuffer + iTmpRTSPBufSize, pSmartBuffer->strBuffer + iCurrentPos, iMessageLen);
					iTmpRTSPBufSize += iMessageLen;
					iTotalChopLen += iTmpRTSPBufSize;

					//copy "\r\n\r\n"
					strTmpBuffer[iTmpRTSPBufSize++] = pSmartBuffer->strBuffer[(iReadPos + 1) % NSSRTSPMessageBufferSize];
					strTmpBuffer[iTmpRTSPBufSize++] = pSmartBuffer->strBuffer[(iReadPos + 2) % NSSRTSPMessageBufferSize];
					strTmpBuffer[iTmpRTSPBufSize++] = pSmartBuffer->strBuffer[(iReadPos + 3) % NSSRTSPMessageBufferSize];
					strTmpBuffer[iTmpRTSPBufSize++] = pSmartBuffer->strBuffer[(iReadPos + 4) % NSSRTSPMessageBufferSize];
					iReadPos = (iReadPos + 4) % NSSRTSPMessageBufferSize;

					//set end of current string
					strTmpBuffer[iTmpRTSPBufSize] = 0;
		
					//try to find if have content
					int iContentLen = RTSPMessageParser::GetContentLength(strTmpBuffer, iTmpRTSPBufSize);
					if (iContentLen > 0)
					{
						if (iReadPos + 1 + iContentLen > NSSRTSPMessageBufferSize)
						{
							//check the buffer state
							while (1)
							{
								if (iWritePos <= iReadPos)
								{
									if ((iReadPos + 1 + iContentLen) % NSSRTSPMessageBufferSize < iWritePos)
										break;
									else
										Sleep(1);
								}
								else
									Sleep(1);
							}
							int i = NSSRTSPMessageBufferSize - iReadPos - 1;
							memcpy(strTmpBuffer + iTmpRTSPBufSize, pSmartBuffer->strBuffer + iReadPos + 1, i);
							memcpy(strTmpBuffer + iTmpRTSPBufSize + i, pSmartBuffer->strBuffer, iContentLen - i);
							iReadPos = iContentLen - i - 1;
						}
						else
						{
							//check the buffer state
							while (1)
							{
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
							memcpy(strTmpBuffer + iTmpRTSPBufSize, pSmartBuffer->strBuffer + iReadPos + 1, iContentLen);
							iReadPos += iContentLen;
						}
					
						iReadPos = iReadPos % NSSRTSPMessageBufferSize;
						iTmpRTSPBufSize += iContentLen;
						iTotalChopLen += iTmpRTSPBufSize;
					}//if(iContentLen > 0)

					strTmpBuffer[iTmpRTSPBufSize] = 0;
					
					//cout << "chop one message\r\n" << strTmpBuffer << endl;
					//write log
					//MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ngod_chop_thread,"chop one message\r\n%s"), strTmpBuffer);
					
					//put one message into message deque
					pMessList->PushBack(string(strTmpBuffer));
					//memset(strTmpBuffer, 0, iTmpRTSPBufSize);
				}//while(1)
			}//for(...)
			if (!bSessionStatus)
				Sleep(10);
		}//if (m_NSSSessionGroupList->size() > 0)
		else
			Sleep(1000);
	}
	return 1;
}