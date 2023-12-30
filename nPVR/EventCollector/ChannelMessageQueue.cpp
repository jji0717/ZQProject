// ChannelMessageQueue.cpp: implementation of the ChannelMessageQueue class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4786)

#include "ChannelMessageQueue.h"
#include "BaseMessageReceiver.h"
#include "Log.h"

using namespace ZQ::common;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ChannelMessageQueue::ChannelMessageQueue()
{
	InitializeCriticalSection(&_opLock);
	_hExit = NULL;
	_hMessageSem = NULL;
	_nMessageID = 0;
}

ChannelMessageQueue::~ChannelMessageQueue()
{
	close();

	ChannelReceiverMap::iterator it=_channelReceiverMap.begin();
	for(;it!=_channelReceiverMap.end();it++)
	{
		std::vector<BaseMessageReceiver*>* pVec = it->second;
		if (pVec)
			delete pVec;
	}
	_channelReceiverMap.clear();

	DeleteCriticalSection(&_opLock);
}

void ChannelMessageQueue::addMessageReceiver(BaseMessageReceiver* pReceiver)
{
	ChannelReceiverMap::iterator it=_channelReceiverMap.find(pReceiver->getChannelID());
	if (it==_channelReceiverMap.end())
	{
		std::vector<BaseMessageReceiver*>* pVec = new std::vector<BaseMessageReceiver*>();
		pVec->push_back(pReceiver);	
		_channelReceiverMap.insert(std::pair<int,std::vector<BaseMessageReceiver*>*>(pReceiver->getChannelID(), pVec));
	}
	else
	{		
		(it->second)->push_back(pReceiver);
	}
}

void ChannelMessageQueue::addChannelMessage(int channelID, MessageFields* pMessage)
{
	CHANNEL_MESSAGE cm;
	cm.channelID = channelID;
	cm.fileds = pMessage;
	
	EnterCriticalSection(&_opLock);
	cm.messageID = ++_nMessageID;
	_messages.push_back(cm);	
	LeaveCriticalSection(&_opLock);	

	LONG nPre;
	ReleaseSemaphore(_hMessageSem, 1, &nPre);
}

int ChannelMessageQueue::run()
{
	HANDLE hHandles[2];
	hHandles[0] = _hMessageSem;
	hHandles[1] = _hExit;

	glog(Log::L_DEBUG, "ChannelMessageQueue run thread enter, threadid [0x%04x]", GetCurrentThreadId());
	
	while(true)
	{
		//wait for a message
		DWORD dwRet = WaitForMultipleObjects(2, hHandles, FALSE, INFINITE);
		if (dwRet == WAIT_OBJECT_0)
		{
			
		}
		else if (dwRet == WAIT_OBJECT_0 + 1)
		{
			glog(Log::L_INFO, "ChannelMessageQueue loop exit");
			break;
		}
		else
		{
			// error, exit
			glog(Log::L_ERROR, "WaitForMultipleObjects return fail with code 0x%08x", dwRet);
			break;
		}

		//
		// get a message
		//		
		while (_messages.size())
		{
			CHANNEL_MESSAGE cm;
			EnterCriticalSection(&_opLock);
			cm = _messages.front();
			_messages.pop_front();
			LeaveCriticalSection(&_opLock);

			ChannelReceiverMap::iterator it=_channelReceiverMap.find(cm.channelID);
			if (it!=_channelReceiverMap.end()&&it->second)
			{
				std::vector<BaseMessageReceiver*>* pVec = it->second;
				std::vector<BaseMessageReceiver*>::iterator itv;
				for(itv=pVec->begin();itv!=pVec->end();itv++)
				{
					(*itv)->OnMessage(cm.messageID, cm.fileds);
				}
			}
			else
			{
				glog(Log::L_WARNING, "No message receiver for the message of channel %d", cm.channelID);
			}

			//release the message Fileds
			if (cm.fileds)
			{
				delete cm.fileds;
			}
		}
	}

	glog(Log::L_DEBUG, "ChannelMessageQueue run thread leave, threadid [0x%04x]", GetCurrentThreadId());
	return 0;
}

bool ChannelMessageQueue::init()
{
	_hExit = CreateEvent(NULL, FALSE, FALSE, NULL);
	_hMessageSem = CreateSemaphore(NULL, 0, 1000, NULL);

	return start();
}

void ChannelMessageQueue::close()
{
	if (_hExit && _hExit!=INVALID_HANDLE_VALUE)
	{
		SetEvent(_hExit);
		waitHandle(INFINITE);

		CloseHandle(_hExit);
		_hExit = NULL;
		CloseHandle(_hMessageSem);
		_hMessageSem = NULL;
		glog(Log::L_DEBUG, "ChannelMessageQueue closed");
	}
}

//this method should be called after all the addMessageReceiver called
void ChannelMessageQueue::getChannelRequireFields(int channelID, std::vector<std::string>& fields)
{
	fields.clear();

	ChannelReceiverMap::iterator it=_channelReceiverMap.find(channelID);
	if (it!=_channelReceiverMap.end())
	{
		std::vector<BaseMessageReceiver*>* pVec = it->second;
		std::vector<BaseMessageReceiver*>::iterator itr=pVec->begin();
		(*itr)->requireFields(fields);

		itr++;
		for(;itr!=pVec->end();itr++)
		{
			std::vector<std::string> tmp;
			(*itr)->requireFields(tmp);

			for(int j=0;j<tmp.size();j++)
			{
				bool bExist=false;
				for(int i=0;i<fields.size();i++)
				{
					if (!stricmp(tmp[j].c_str(), fields[i].c_str()))
					{
						bExist = true;
						break;
					}
				}

				if (!bExist)
					fields.push_back(tmp[j]);
			}
		}
	}
}
