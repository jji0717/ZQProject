// ActiveMsgChannel.cpp: implementation of the ActiveMsgChannel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ActiveMsgChannel.h"
#include "DODAppImpl.h"
#include "Util.h"
#include "global.h"
#include "DataPublisherImpl.h"

// using namespace DataOnDemand;
using namespace ZQ::common;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ActiveMsgChannel::ActiveMsgChannel(
	DataOnDemand::MessageChannelExPrx& channelPrx):
	_channelPrx(channelPrx)
{
	m_bStop = false;
}

ActiveMsgChannel::~ActiveMsgChannel()
{
	// DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();
	m_MessageList.clear();

/*	glog(Log::L_DEBUG, "[channelName= %s, threadID = %d]"
		"~ActiveMsgChannel(), delete Object success!",
		"Channel", GetCurrentThreadId());*/
}

bool ActiveMsgChannel::activeChannelInit()
{
	return start();
}

void ActiveMsgChannel::uninit()
{
	stop();
}

int ActiveMsgChannel::run()
{
	DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();

	glog(Log::L_DEBUG, "channelName = %s, ThreadId = %d",
		myInfo.name.c_str(), GetCurrentThreadId());

	DataOnDemand::MessageChannelPrx ch = _channelPrx;

	zqMessageList::iterator itrMessage;
	long  second = 0;
	DWORD timeout = INFINITE; 
	long currenttime;
	struct tm *deltime;
	char strdeltime[40] = "0";
   
	try
	{
		while(!m_bStop)
		{   
			if(m_MessageList.size() > 0 )
			{
				itrMessage = m_MessageList.begin();
				if(itrMessage->bForever)
				{
					timeout = INFINITE;
				}
				else
				{
					time(&currenttime);			
					second = itrMessage->deleteTime - currenttime;
					
					if(second > 0)
					  timeout = second * 1000;
					else
					  timeout = 0;
				}
			}
			else
			{
				timeout = INFINITE;
			}
       		glog(ZQ::common::Log::L_DEBUG,"[channelname = %s] Vect size:%d",
				myInfo.name.c_str(),m_MessageList.size());

			if( WaitForSingleObject(m_hWaitEvent, timeout) == WAIT_OBJECT_0)
			{
				ResetEvent(m_hWaitEvent);
				continue;
			}	
			EnterCriticalSection(&m_channelCriticalSection);	

			if(DeleteFileA(itrMessage->fileName.c_str()) == FALSE)
			{
			   glog(ZQ::common::Log::L_ERROR,
				   "[channelname = %s]delete nMessageID = %s filename = %s error",
				   myInfo.name.c_str(), itrMessage->sMessageID.c_str(), 
				   itrMessage->fileName.c_str());
			}
			else
			{ 
				deltime = localtime( &itrMessage->deleteTime);
				sprintf(strdeltime,"%04d-%02d-%02d %02d:%02d:%02d",
					deltime->tm_year + 1900, deltime->tm_mon + 1,
					deltime->tm_mday, deltime->tm_hour,
					deltime->tm_min,deltime->tm_sec);
			   glog(ZQ::common::Log::L_INFO,
				   "[channelname = %s]delete nMessageID = %s \n\t\t  "
				   "filename = %s OK,\n\t\t   \tDeletime = %s!",myInfo.name.c_str(), 
				   itrMessage->sMessageID.c_str(),itrMessage->fileName.c_str(),
				   strdeltime);
			}
			
			NotityMsgMuxItemDel(itrMessage->GroupId);
			m_MessageList.erase(itrMessage);
  
			LeaveCriticalSection(&m_channelCriticalSection);
		}
    }
	catch(...)
	{
      	glog(ZQ::common::Log::L_DEBUG,
		"[channelname = %s] DODFileListhread Exception! errorcode = %d",
		myInfo.name.c_str() , GetLastError());
	}
	glog(ZQ::common::Log::L_DEBUG,
		"[channelname = %s] Exit DODFileListhread  OK",
		myInfo.name.c_str() );
	return 1;
}

bool ActiveMsgChannel::threadInit()
{
	long lastupdate;

	DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();

	m_MessageList.clear();
  
    time(&lastupdate);

	InitializeCriticalSection(&m_channelCriticalSection);

	m_hWaitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	LoadMsgData();
	return true;
}

void ActiveMsgChannel::final()
{
	if (m_hWaitEvent)
		CloseHandle(m_hWaitEvent);

	DeleteCriticalSection(&m_channelCriticalSection);
}

void  ActiveMsgChannel::AddMsgList(ZQCMessageInfoTINF *pMsginfo, bool IsSet)
{	
    long second;
	DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();

	EnterCriticalSection(&m_channelCriticalSection);
	
	zqMessageList::iterator itrMessage = m_MessageList.begin();

	if(pMsginfo->bForever)
	{
		glog(ZQ::common::Log::L_DEBUG,
			"[ChannealName = %s]AddMsgList: This Message is a forever Msg! \
			MsgID = %s",myInfo.name.c_str(),pMsginfo->sMessageID.c_str());

		m_MessageList.push_back(*pMsginfo);
	    LeaveCriticalSection(&m_channelCriticalSection);
		return;
	}

	while(itrMessage != m_MessageList.end())
	{
		if(pMsginfo->bForever)
			break;
		second = pMsginfo->deleteTime - itrMessage->deleteTime;
		if(second > 0 )	
			itrMessage++;
		else
			break;
	}

	if(itrMessage == m_MessageList.begin())
	{
		glog(ZQ::common::Log::L_DEBUG,
			"[ChannealName = %s]AddMsgList: This Message is Push_front! MsgID = %s",
			myInfo.name.c_str(),pMsginfo->sMessageID.c_str());
		m_MessageList.push_front(*pMsginfo);
		if(IsSet)
		{
		   SetEvent(m_hWaitEvent);
		}

	}
	else
		if(itrMessage != m_MessageList.end())
		{
			glog(ZQ::common::Log::L_DEBUG,
				"[ChannealName = %s]AddMsgList: This Message is Push_back! MsgID = %s",
				myInfo.name.c_str(),pMsginfo->sMessageID.c_str());

			m_MessageList.insert(itrMessage, *pMsginfo);
		}
		else
		{
			glog(ZQ::common::Log::L_DEBUG,
				"[ChannealName = %s]AddMsgList: This Message is Push_middle! \
				MsgID = %s",myInfo.name.c_str(),pMsginfo->sMessageID.c_str());
			m_MessageList.push_back(*pMsginfo);
		}
	LeaveCriticalSection(&m_channelCriticalSection);
}

std::string ActiveMsgChannel::GetCurrentDatatime()
{
	char strtime[20];
	std::string strTime;
	SYSTEMTIME time; 
	GetLocalTime(&time);
	sprintf(strtime,"%04d%02d%02d%02d%02d%02d%03d",time.wYear, time.wMonth, 
		time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	strTime = strtime;
	return strTime;
}

void
ActiveMsgChannel::notifyMessageDeleted(const ::std::string& messageId,
	                             ::Ice::Int groupId)
{

    zqMessageList::iterator itrMessage;
	ZQCMessageInfoTINF zqMsgInfo;
    long deletime;

	DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();
	
	EnterCriticalSection(&(m_channelCriticalSection));	
	if(m_MessageList.size()<1)
	{
		LeaveCriticalSection(&(m_channelCriticalSection));	
		return ;
	}
	try
	{
		for(itrMessage= m_MessageList.begin();itrMessage != m_MessageList.end();)
		{		
			if (messageId != itrMessage->sMessageID)
			{	
				++itrMessage;	
				continue;
			}
             time(&deletime ); 
			itrMessage->deleteTime  = deletime;
			itrMessage->bForever = FALSE;
			
			zqMsgInfo = *itrMessage;
			itrMessage = m_MessageList.erase(itrMessage);
			m_MessageList.push_front(zqMsgInfo);
			SetEvent(m_hWaitEvent);
	    	glog(ZQ::common::Log::L_DEBUG,
				"[ChannealName = %s]notifyMessageDeleted:StopFileSend find"
				" nMessageID = %s (filename = %s)",
				myInfo.name.c_str(),messageId.c_str(),zqMsgInfo.fileName);
		}
	}
	catch (...) 
	{
		LeaveCriticalSection(&(m_channelCriticalSection));

		glog(ZQ::common::Log::L_ERROR,
			"[ChannealName = %s]notifyMessageDeleted: StopFileSend filename:error",
			myInfo.name.c_str());
		return;
	}
	LeaveCriticalSection(&(m_channelCriticalSection));	

}

bool
ActiveMsgChannel::notifyMessageAdded(
	const ::std::string& messageId,
	const ::std::string& dest,
	const ::std::string& messageBody,
	::Ice::Long exprie,
	::Ice::Int groupId)
{
    long deletime;
	std::string sTime;
	char strDel[30] = "";

	TianShanIce::Properties myProps = _channelPrx->getProperties();
	DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();

    ::std::string _CacheDir = myProps["Path"];
	int strsize = messageBody.size();

	if (strsize < 1)
	{
		glog(ZQ::common::Log::L_DEBUG,"CreateMsgFile: message 's length error." );
		return false;
	}

	std::string sFileName = "Msg";
	sTime = GetCurrentDatatime();
	sFileName = _CacheDir + "\\" +sTime ;
	
	ZQCMessageInfoTINF info;
	try
	{	
		info.sMessageID = messageId;
		info.GroupId = groupId;
	     
		if (exprie == 0)
		{
			info.bForever = TRUE;
			info.fileName = sFileName +"DEL0000000000" + dest ;
			sprintf(strDel,"%dDEL0000000000",info.GroupId);
			info.fileName = sFileName + strDel + dest;
		}
		else
		{
            time(&deletime );
			info.bForever = FALSE;
			info.deleteTime = deletime + exprie;
			sprintf(strDel,"%dDEL%d",info.GroupId,info.deleteTime);
			info.fileName = sFileName + strDel + dest;
		}	

		HANDLE hFile;
		hFile = CreateFileA(info.fileName.c_str(), GENERIC_WRITE, 
			    FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) 
		{
			glog(ZQ::common::Log::L_ERROR,
				"[ChannealName = %s]CreateMsgFile error:CreateFile.\n\t\t\t   \
				filename = %s.",myInfo.name.c_str(),info.fileName.c_str());
			return false;
		}
		int kk = messageBody.size();
		DWORD kks = 0;
		WriteFile(hFile,messageBody.c_str(),kk,&kks,NULL);
		CloseHandle(hFile); 
		hFile = NULL;
	}
	catch (...) 
	{
		glog(ZQ::common::Log::L_ERROR,
			"[ChannealName = %s]CreateMsgFile:\n\t\t\t   \
			filename=%s,write data to file error",\
			myInfo.name.c_str(),info.fileName.c_str());
		return false;
	}
	glog(ZQ::common::Log::L_DEBUG,
		"[ChannealName = %s]CreateMsgFile  success\n\t\t   filename= %s",
		myInfo.name.c_str(),info.fileName.c_str());

	EnterCriticalSection(&m_channelCriticalSection);
	
	if(m_MessageList.size() < 1 )
	{
		m_MessageList.push_front(info);
		SetEvent(m_hWaitEvent);
	}
	else
		AddMsgList(&info);
	LeaveCriticalSection(&m_channelCriticalSection);
	
	try
	{
		NotityMsgMuxItemAdd(groupId);
	}
	catch (...) 
	{
		glog(ZQ::common::Log::L_ERROR,
			"[ChannealName = %s]CreateMsgFile:NotityMsgMuxItemAdd error ,\n"
			"\t\t   filename = %s,",
			myInfo.name.c_str(),info.fileName.c_str());
	}
	return true;
}

bool ActiveMsgChannel::NotityMsgMuxItemAdd(int groupId)
{
	DataOnDemand::DestLinks::iterator iter;
    ::DataOnDemand::DestInfo destinfo;
	
    ::Ice::Identity ident;
	::Ice::ObjectPrx  objprx;
	::DataOnDemand::MuxItemPrx muxitemprx;

	DataOnDemand::DestLinks myDestLinks = _channelPrx->getDestLinks();
	DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();

	try
	{
		if(groupId != 0 )
		{	
			for(iter = myDestLinks.begin(); iter != myDestLinks.end(); iter++)
			{		
				destinfo = iter->second.dest->getInfo();
				if(destinfo.groupId == groupId || destinfo.groupId == 0)
				{
					ident = createMuxItemIdentity(configSpaceName,
						                       (*iter).first, myInfo.name);
					objprx = createObjectWithEndPoint(
						DataOnDemand::DataPublisherImpl::_ic,ident,
						DataOnDemand::DataPublisherImpl::_strmSvcEndPoint);
					muxitemprx = ::DataOnDemand::MuxItemPrx::checkedCast(objprx);
								
					glog(ZQ::common::Log::L_INFO, 
						"[%s,%s]::UpdateChannel will start!", 
						(*iter).first.c_str(), myInfo.name.c_str());
					std::string filename ="";
					muxitemprx->notifyFullUpdate(filename);
					
					glog(ZQ::common::Log::L_INFO, 
						"[%s,%s]::Update operation end!", 
						(*iter).first.c_str(), myInfo.name.c_str());	
					//m_pNmp->AddUpdateChannel(this);
				}
			}		
		}
		else
		{
			for(iter = myDestLinks.begin(); iter != myDestLinks.end(); iter++)
			{		
				destinfo = (*iter).second.dest->getInfo();
				ident = createMuxItemIdentity(configSpaceName,(*iter).first, 
					                  myInfo.name);
				objprx = createObjectWithEndPoint(
					DataOnDemand::DataPublisherImpl::_ic,ident,
					DataOnDemand::DataPublisherImpl::_strmSvcEndPoint);
				muxitemprx = ::DataOnDemand::MuxItemPrx::checkedCast(objprx);
															
				glog(ZQ::common::Log::L_INFO, 
					"[%s,%s]::UpdateChannel will start!", 
					(*iter).first.c_str(), myInfo.name.c_str());
				
				std::string filename ="";
				muxitemprx->notifyFullUpdate(filename);
				
				glog(ZQ::common::Log::L_INFO, 
					"[%s,%s]::Update operation end!", 
					(*iter).first.c_str(), myInfo.name.c_str());
				
				//m_pNmp->AddUpdateChannel(this);
			}		
		}
	}
	catch(const Ice::ObjectNotExistException&) 
	{
		glog(ZQ::common::Log::L_DEBUG, 
			"ActiveMsgChannel::NotityMuxItem\tcannt connect to streamer");
		return false;
	}
	catch (const ::Ice::Exception & ex) 
	{
		glog(ZQ::common::Log::L_DEBUG,
			"ActiveMsgChannel::NotityMuxItem: Ice::Exception errorcode = %s",
			ex.ice_name().c_str());
		return false;
    } 
	return true;
}

bool ActiveMsgChannel::NotityMsgMuxItemDel(int groupId)
{
    NotityMsgMuxItemAdd(groupId);
	return true;
}
bool ActiveMsgChannel::LoadMsgData()
{
	TianShanIce::Properties myProps = _channelPrx->getProperties();
	DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();
	
    ::std::string _CacheDir = myProps["Path"];
     ZQCMessageInfoTINF info;

	long currenttime;
	int npos1, npos2;
	char strdeltime[40] = "0";
    struct tm *deltime;
	::std::list<::std::string>strFileName;
	
	if(ListFile(_CacheDir.c_str(), strFileName))
	{
		glog(ZQ::common::Log::L_ERROR,
			"[ChannealName = %s]ActiveMsgChannel::Load Msgdata error!",
			 myInfo.name.c_str()); 
		return false;
	}		

	::std::list<::std::string>::iterator itor;
	
	for(itor = strFileName.begin() ; itor != strFileName.end(); itor++)
	{
		info.fileName = _CacheDir + "\\" + (*itor);
		info.GroupId = (*itor)[17] - '0';
		
		npos1 = (*itor).find('L');
		npos2 = (*itor).find('_');
		info.deleteTime =atoi( (*itor).substr(
			npos1 +1, npos2 - npos1 -1).c_str());
		
		npos1 = (*itor).find('_', npos2 + 1);
		info.sMessageID = (*itor).substr(npos2 +1, npos1 - npos2 - 1);

		if(info.deleteTime ==0)
		{
			info.bForever = true;
		}
		else
		{
		  time(&currenttime);	
		  if(info.deleteTime <= currenttime)
		  {

			  if(DeleteFileA(info.fileName.c_str()) == FALSE)
			  {
				  glog(ZQ::common::Log::L_ERROR,
					  "[channelname = %s]delete nMessageID = %s filename = %s error",
					  myInfo.name.c_str(), info.sMessageID.c_str(), 
					  info.fileName.c_str());
			  }
			  else
			  {  
				  deltime = localtime( &info.deleteTime);
				  sprintf(strdeltime,"%04d-%02d-%02d %02d:%02d:%02d",
					  deltime->tm_year + 1900, deltime->tm_mon + 1,
					  deltime->tm_mday, deltime->tm_hour,
					  deltime->tm_min,deltime->tm_sec);
				  glog(ZQ::common::Log::L_INFO,
				  "[channelname = %s]delete nMessageID = %s \n\t\t   \
				  filename = %s OK,\n\t\t   \tDeletime = %s!",myInfo.name.c_str(), 
				  info.sMessageID.c_str(),info.fileName.c_str(),
				  strdeltime);
			  }
             continue;
		  }
		  else
		  {
            info.bForever = false;
		  }			
		}
		AddMsgList(&info, false);		
	}
	
	if(m_MessageList.size() > 0)
		SetEvent(m_hWaitEvent);
		
	return true;
}

int ActiveMsgChannel::ListFile(const char *argv, ::std::list<::std::string> &File)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	char DirSpec[MAX_PATH];  // directory specification
	char FileName[MAX_PATH];
	DWORD dwError;

	if (((_access ( argv, 0 )) != 0))
	{
		glog(ZQ::common::Log::L_DEBUG,
			"[CachePath = %s]current path is not exist!",
			argv);
		return false;
	}
	
	strncpy (DirSpec, argv, strlen(argv)+1);
	strncat (DirSpec, "\\*", 3);
	
	hFind = FindFirstFileA(DirSpec, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE) 
	{		 
		return -1;
	} 
	else 
	{
		//printf ("First file name is %s\n", FindFileData.cFileName);
		if ( FindFileData.dwFileAttributes == 32 )
		{
			strcpy(FileName, FindFileData.cFileName);
			File.push_back(FileName);			
		}

		while (FindNextFileA(hFind, &FindFileData) != 0) 
		{
			//printf ("Next file name is %s\n", FindFileData.cFileName);
			if ( FindFileData.dwFileAttributes == 32 )
			{
				strcpy(FileName, FindFileData.cFileName);
				File.push_back(FileName);
			}
		}		
		dwError = GetLastError();
		FindClose(hFind);
		if (dwError != ERROR_NO_MORE_FILES) 
		{	 
			return -1;
		}
	}

	return 0;
}

void ActiveMsgChannel::stop()
{
	if (m_bStop) {
		glog(Log::L_DEBUG, 
			"ActiveMsgChannel::stop()\t the state is stoped");
		return;
	}
	m_bStop = true;
	SetEvent(m_hWaitEvent);
	waitHandle(INFINITE);
}
