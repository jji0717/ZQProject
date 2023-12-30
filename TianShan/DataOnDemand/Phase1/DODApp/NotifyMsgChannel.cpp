#include "stdafx.h"
#include <list>
#include <deque>
#include "NotifyMsgChannel.h"
#include "MessageChannelImpl.h"
#include "global.h"
UINT UpdateChannelProc(LPVOID lpParam) 
{
	CNotifyUpdateMsgChannel *pPM = (CNotifyUpdateMsgChannel *)lpParam;
	CDODCHANNLEQUEUE::iterator iter;
	long timeout = INFINITE;
	long currenttime;
	long second;
	try
	{
		while(g_bServiceStarted)//!g_bStop)
		{     
			if(pPM->m_DelChannelQueue.size() > 0 )
			{
				iter = pPM->m_DelChannelQueue.begin();
				time(&currenttime);
				
//				second = (*iter)->m_UpdateTime - currenttime ;
				
				if(second > 0 )
					timeout = second * 1000;
				else
					timeout = 0;
			}
			else
			{
				timeout = INFINITE;
			}
			
			glog(ZQ::common::Log::L_INFO,"UpdateChannelProc Vect size:%d",pPM->m_DelChannelQueue.size());
			
			if( WaitForSingleObject(pPM->m_hUpdateEvent, timeout) == WAIT_OBJECT_0)
			{
				ResetEvent(pPM->m_hUpdateEvent);
				continue;
			}
			
			EnterCriticalSection(&(pPM->m_UpdateCriticalSection));
			time(&currenttime);
			(*iter)->myLastUpdate = currenttime;
			pPM->m_DelChannelQueue.pop_front();
			LeaveCriticalSection(&(pPM->m_UpdateCriticalSection));
			
			DataOnDemand::DestLinks::iterator destLinkiter;
			for(destLinkiter = (*iter)->myDestLinks.begin(); destLinkiter != (*iter)->myDestLinks.end(); destLinkiter++)
			{	
				glog(ZQ::common::Log::L_INFO, 
					"[destname = %s, channelname = %s]::UpdateChannel will start!", 
					(*destLinkiter).first.c_str(), (*iter)->myInfo.name.c_str());

//				(*destLinkiter).dest->
			     glog(ZQ::common::Log::L_INFO, 
					 "[destname = %s, channelname = %s]::Update operation end!", 
					 (*destLinkiter).first.c_str(),(*iter)->myInfo.name.c_str());	

			}
		}
	}

	catch (...) 
	{
	   glog(ZQ::common::Log::L_ERROR,"UpdateChannel thread error!");
	   glog(ZQ::common::Log::L_ERROR,"CNotifyUpdateMsgChannel::UpdateChannel thread Exit!!");
		return 0;
	}
    glog(ZQ::common::Log::L_INFO,"CNotifyUpdateMsgChannel::UpdateChannel thread Exit!!");
	return 1;
}
CNotifyUpdateMsgChannel::CNotifyUpdateMsgChannel(int InterVal)
{	
	DWORD IDThread;
	m_DelChannelQueue.clear();

	m_nUpdateInterVal = InterVal;

	m_hUpdateThread = NULL;
	m_hUpdateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	InitializeCriticalSection(&m_UpdateCriticalSection);
	
	m_hUpdateThread = CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)UpdateChannelProc,this,0 , &IDThread); 
}
CNotifyUpdateMsgChannel::~CNotifyUpdateMsgChannel()
{
	if( m_hUpdateThread )	
	{
		WaitForSingleObject( m_hUpdateThread, INFINITE );
		CloseHandle(m_hUpdateThread);
		m_hUpdateThread = NULL;
		CloseHandle(m_hUpdateEvent);
	}
    DeleteCriticalSection(&m_UpdateCriticalSection);
}

void CNotifyUpdateMsgChannel::AddUpdateChannel(DataOnDemand::MessageChannelImpl* pChannelInfo)
{
/*	long  second = 0;
	long currenttime;
	CDODCHANNLEQUEUE::iterator iter;
     
	time(&currenttime);			

	second = currenttime - pChannelInfo->myLastUpdate;

	if(second > m_nUpdateInterVal)
	{
		second = m_nUpdateInterVal;
	}

	EnterCriticalSection(&m_UpdateCriticalSection);
	if (m_DelChannelQueue.size() >= 1)
	{
		for(iter = m_DelChannelQueue.begin();iter != m_DelChannelQueue.end();++iter)
		{		
			if(*iter == pChannelInfo)
			{
				glog(ZQ::common::Log::L_INFO,"[ChannealName = %s]This Channel is Exist in UpdateChannel!",pChannelInfo->myInfo.name.c_str() );
				LeaveCriticalSection(&m_UpdateCriticalSection);
				return ;
			}	
		}
	}

	time(&currenttime);	
	pChannelInfo->m_UpdateTime = currenttime + m_nUpdateInterVal - second;

	for(iter = m_DelChannelQueue.begin();iter != m_DelChannelQueue.end();)
	{	
		second = pChannelInfo->m_UpdateTime - (*iter)->m_UpdateTime;
		if(second > 0 )	
			iter++;
		else
			break;
	}
	if(iter == m_DelChannelQueue.begin())
	{
		glog(ZQ::common::Log::L_INFO,"[ChannealName = %s]This Channel is Push_front!",pChannelInfo->myInfo.name.c_str() );
		
		m_DelChannelQueue.push_front(pChannelInfo);
		SetEvent(m_hUpdateEvent);
	}
	else
		if(iter != m_DelChannelQueue.end())
		{
			glog(ZQ::common::Log::L_INFO,"[ChannealName = %s]This Channel is Push_back!",pChannelInfo->myInfo.name.c_str() );
			
			m_DelChannelQueue.insert(iter, pChannelInfo);
		}
		else
		{
			glog(ZQ::common::Log::L_INFO,"[ChannealName = %s]This Channel is Push_middle!",pChannelInfo->myInfo.name.c_str() );
			
			m_DelChannelQueue.push_back(pChannelInfo);
		}	
		glog(ZQ::common::Log::L_INFO,"AddUpdateChannel QueueSize is %d!",m_DelChannelQueue.size());
		
	LeaveCriticalSection(&m_UpdateCriticalSection);*/

}
