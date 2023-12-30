// ActiveMsgChannel.cpp: implementation of the ActiveMsgChannel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ActiveMsgChannel.h"
#include "DODAppImpl.h"
#include "Util.h"
#include "global.h"
#include "DataPublisherImpl.h"
#include <DODContentStore.h>
using namespace DataOnDemand;
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
		"~ActiveMsgChannel(), delete Object success",
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

	glog(Log::L_INFO, "channelName = %s, ThreadId = %d",
		myInfo.name.c_str(), GetCurrentThreadId());

	DataOnDemand::MessageChannelPrx ch = _channelPrx;

	zqMessageList::iterator itrMessage;
	long  second = 0;
	DWORD timeout = INFINITE; 
	time_t currenttime;
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
       		glog(ZQ::common::Log::L_INFO,"[channelname: %s]Msg count (%d)",
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
				   "[channelname: %s]delete MessageID(%s) filename(%s error(%s)",
				   myInfo.name.c_str(), itrMessage->sMessageID.c_str(), 
				   itrMessage->fileName.c_str(), GetLastError());
			}
			else
			{ 
				time_t DeleteTime = itrMessage->deleteTime;
				deltime = localtime(&DeleteTime);
				sprintf(strdeltime,"%04d-%02d-%02d %02d:%02d:%02d",
					deltime->tm_year + 1900, deltime->tm_mon + 1,
					deltime->tm_mday, deltime->tm_hour,
					deltime->tm_min,deltime->tm_sec);
			   glog(ZQ::common::Log::L_INFO,
				   "[channelname: %s]delete MessageID(%s) \t\t   "
				   "filename(%s) OK,\t\t   \tDeletime(%s)!",myInfo.name.c_str(), 
				   itrMessage->sMessageID.c_str(),itrMessage->fileName.c_str(),
				   strdeltime);
			}
			
			NotityMsgMuxItemDel(itrMessage->GroupId, itrMessage->fileName);
			m_MessageList.erase(itrMessage);
  
			LeaveCriticalSection(&m_channelCriticalSection);
		}
    }
	catch(...)
	{
      	glog(ZQ::common::Log::L_ERROR,
		"[channelname: %s] caught DODFileListhread exception(%d)",
		myInfo.name.c_str() , GetLastError());
	}
	glog(ZQ::common::Log::L_INFO,
		"[channelname: %s] Exit DODFileListhread",
		myInfo.name.c_str() );
	return 1;
}

bool ActiveMsgChannel::threadInit()
{
	time_t lastupdate;

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
		glog(ZQ::common::Log::L_INFO,
			"[ChannelName: %s]AddMsgList() This Message is a forever Msg "
			"MessageID(%s)",myInfo.name.c_str(),pMsginfo->sMessageID.c_str());

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
		glog(ZQ::common::Log::L_INFO,
			"[ChannelName: %s]AddMsgList()This Message is Push_front! MessageID(%s)",
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
			glog(ZQ::common::Log::L_INFO,
				"[ChannelName: %s]AddMsgList() This Message is Push_back! MessageID(%s)",
				myInfo.name.c_str(),pMsginfo->sMessageID.c_str());

			m_MessageList.insert(itrMessage, *pMsginfo);
		}
		else
		{
			glog(ZQ::common::Log::L_INFO,
				"[ChannelName: %s]AddMsgList() This Message is Push_middle "
				"MessageID(%s)",myInfo.name.c_str(),pMsginfo->sMessageID.c_str());
			m_MessageList.push_back(*pMsginfo);
		}
	LeaveCriticalSection(&m_channelCriticalSection);
}
void
ActiveMsgChannel::notifyMessageDeleted(const ::std::string& messageId,
	                             ::Ice::Int groupId)
{
    zqMessageList::iterator itrMessage;
	ZQCMessageInfoTINF zqMsgInfo;
    time_t deletime;

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
	    	glog(ZQ::common::Log::L_INFO,
				"[ChannelName: %s]notifyMessageDeleted() StopFileSend find "
				"MessageID(%s) (filename = %s)",
				myInfo.name.c_str(),messageId.c_str(),zqMsgInfo.fileName.c_str());
		}
	}
	catch (...) 
	{
		LeaveCriticalSection(&(m_channelCriticalSection));

		glog(ZQ::common::Log::L_ERROR,
			"[ChannelName: %s]notifyMessageDeleted() Stop File Send error(%d)",
			myInfo.name.c_str(), GetLastError());
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
    time_t deletime;
	std::string sTime;
	char strDel[30] = "";
	std::string _messageBody;

	std::string destination = dest.substr(dest.size() - 6, 6) +  "/" ;

    _messageBody = destination + messageBody;

	TianShanIce::Properties myProps = _channelPrx->getProperties();
	DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();

    ::std::string _CacheDir = myProps["Path"];
	int strsize = messageBody.size();

	if (strsize < 1)
	{
		glog(ZQ::common::Log::L_ERROR,"Create Message File: message's length error" );
		return false;
	}

	std::string sFileName = "Msg", sFileNameTemp;
	GetCurrentDatatime(sTime);
	sFileName = _CacheDir + "\\" +sTime ;
	sFileNameTemp = sTime;
	
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
			sFileNameTemp = sFileNameTemp + strDel + dest + ".dod";
		}	
		else
		{
            time(&deletime );
			info.bForever = FALSE;
			info.deleteTime = deletime + exprie;
			sprintf(strDel,"%dDEL%d",info.GroupId,info.deleteTime);
			info.fileName = sFileName + strDel + dest +".dod";
			sFileNameTemp = sFileNameTemp + strDel + dest +".dod";
		}	

		HANDLE hFile;
		hFile = CreateFileA(info.fileName.c_str(), GENERIC_WRITE, 
			    FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) 
		{
			glog(ZQ::common::Log::L_ERROR,
				"[ChannelName: %s]Create Message File error:CreateFile.\t\t\t   "
				"filename(%s).",myInfo.name.c_str(),info.fileName.c_str());
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
			"[ChannelName: %s]Create Message File :\t\t\t  "
			"filename(%s),write data to file error",
			myInfo.name.c_str(),info.fileName.c_str());
		return false;
	}
	glog(ZQ::common::Log::L_DEBUG,
		"[ChannelName: %s]Create Message File success\t\t\tfilename(%s)",
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
		std::string _contentpath;

		if(!WrapData(_messageBody,sFileNameTemp,_contentpath))
		{
          	glog(ZQ::common::Log::L_INFO, "[ChannelName: %s] Warp Data Error", 
							myInfo.name.c_str());
			return false;
		}
	   NotityMsgMuxItemAdd(groupId,sFileNameTemp,_contentpath);
	}
	catch (...) 
	{
		glog(ZQ::common::Log::L_ERROR,
			"[ChannelName: %s]NotityMsgMuxItemAdd() fail to Create Msg File,"
			"\t\t   filename(%s),",
			myInfo.name.c_str(),info.fileName.c_str());
	}

	return true;
}

bool ActiveMsgChannel::NotityMsgMuxItemAdd(int groupId,std::string fileName,
										        std::string _contentpath)
{
	DataOnDemand::DestLinks::iterator iter;
    ::DataOnDemand::DestInfo destinfo;

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
					TianShanIce::SRM::SessionPrx sessionprx = 
					DataOnDemand::DataPublisherImpl::_sessManager->
								openSession(iter->second.dest->getSessionId());
				
			   		::DataOnDemand::DataStreamPrx streamprx = 
						DataOnDemand::DataStreamPrx::checkedCast(
														sessionprx->getStream());

		     		muxitemprx = streamprx->getMuxItem(myInfo.name);

					glog(ZQ::common::Log::L_INFO, 
							"[%s,%s]notifyFileAdded() will start", 
							(*iter).first.c_str(), myInfo.name.c_str());

					glog(ZQ::common::Log::L_INFO, 
						"contentpath(%s)!", _contentpath.c_str());
					muxitemprx->notifyFileAdded(_contentpath);
						
					glog(ZQ::common::Log::L_INFO, 
							"[%s,%s]notifyFileAdded() operation end", 
							(*iter).first.c_str(), myInfo.name.c_str());	
				}
			}		
		}
		else
		{
			for(iter = myDestLinks.begin(); iter != myDestLinks.end(); iter++)
			{		
				destinfo = (*iter).second.dest->getInfo();

                TianShanIce::SRM::SessionPrx sessionprx = 
				DataOnDemand::DataPublisherImpl::_sessManager->
							openSession(iter->second.dest->getSessionId());
			
			   ::DataOnDemand::DataStreamPrx streamprx = 
				       DataOnDemand::DataStreamPrx::checkedCast(
												sessionprx->getStream());
		
			   muxitemprx = streamprx->getMuxItem(myInfo.name);

				glog(ZQ::common::Log::L_INFO, 
					"[%s,%s]notifyFileAdded() will start", 
					(*iter).first.c_str(), myInfo.name.c_str());													
				glog(ZQ::common::Log::L_INFO, 
					"contentpath(%s)!", _contentpath.c_str());

				muxitemprx->notifyFileAdded(_contentpath);
				
				glog(ZQ::common::Log::L_INFO, 
					"[%s,%s]notifyFileAdded() operation end", 
					(*iter).first.c_str(), myInfo.name.c_str());
			}		
		}
	}
	catch(const Ice::ObjectNotExistException&) 
	{
		glog(ZQ::common::Log::L_DEBUG, 
			"NotityMuxItem()\tca'nt connect to streamer");
		return false;
	}
	catch (const ::Ice::Exception & ex) 
	{
		glog(ZQ::common::Log::L_DEBUG,
			"NotityMuxItem() caught Ice::Exception(%s)",
			ex.ice_name().c_str());
		return false;
    } 
	return true;
}

bool ActiveMsgChannel::NotityMsgMuxItemDel(int groupId,std::string fileName)
{
	DataOnDemand::DestLinks::iterator iter;
    ::DataOnDemand::DestInfo destinfo;

	::DataOnDemand::MuxItemPrx muxitemprx;

	DataOnDemand::DestLinks myDestLinks = _channelPrx->getDestLinks();
	DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();
	std::string contentname, _contentpath;
		
    int nIndex = fileName.rfind('\\');
	fileName = fileName.substr(nIndex +  1,fileName.size() - nIndex);
   
    contentname = configSpaceName + "_" + fileName;

	::TianShanIce::Storage::ContentPrx contentprx;
				
	contentprx=  DataOnDemand::DataPublisherImpl::_contentStroe->
			openContent(contentname,::TianShanIce::Storage::ctDODTS ,false);

	::std::string targetCSType="";
	int transferProtocol = 0;
	int ttl = 0;
	::TianShanIce::Properties exppro;			
	_contentpath = contentprx->getExportURL(targetCSType,transferProtocol,ttl,exppro);

	long size = _contentpath.find("file:");
	if(!size)
	{ 
		_contentpath = _contentpath.substr(5,_contentpath.size() - 5);
	}
				 			
	try
	{
		if(groupId != 0 )
		{	
			for(iter = myDestLinks.begin(); iter != myDestLinks.end(); iter++)
			{		
				destinfo = iter->second.dest->getInfo();
				if(destinfo.groupId == groupId || destinfo.groupId == 0)
				{								

                TianShanIce::SRM::SessionPrx sessionprx = 
				DataOnDemand::DataPublisherImpl::_sessManager->
							openSession(iter->second.dest->getSessionId());
			
			   	::DataOnDemand::DataStreamPrx streamprx = 
					DataOnDemand::DataStreamPrx::checkedCast(
					sessionprx->getStream());

				glog(ZQ::common::Log::L_INFO, 
						"[%s,%s]NotityMsgMuxItemDel will start", 
						(*iter).first.c_str(), myInfo.name.c_str());

				glog(ZQ::common::Log::L_INFO, 
						"contentpath (%s)", _contentpath.c_str());

				muxitemprx = streamprx->getMuxItem(myInfo.name);

				muxitemprx->notifyFileDeleted(_contentpath);
					
					glog(ZQ::common::Log::L_INFO, 
						"[%s,%s]NotityMsgMuxItemDel() operation end", 
						(*iter).first.c_str(), myInfo.name.c_str());	
				}
			}		
		}
		else
		{
			for(iter = myDestLinks.begin(); iter != myDestLinks.end(); iter++)
			{		
				destinfo = (*iter).second.dest->getInfo();

                TianShanIce::SRM::SessionPrx sessionprx = 
				DataOnDemand::DataPublisherImpl::_sessManager->
							openSession(iter->second.dest->getSessionId());
			
			   ::DataOnDemand::DataStreamPrx streamprx = 
				       DataOnDemand::DataStreamPrx::checkedCast(
													   sessionprx->getStream());
			
			   muxitemprx = streamprx->getMuxItem(myInfo.name);
															
				glog(ZQ::common::Log::L_INFO, 
					"[%s,%s]NotityMsgMuxItemDel() will start", 
					(*iter).first.c_str(), myInfo.name.c_str());

				glog(ZQ::common::Log::L_INFO, 
					"contentpath(%s)", _contentpath.c_str());

				muxitemprx->notifyFileDeleted(_contentpath);
				
				glog(ZQ::common::Log::L_INFO, 
					"[%s,%s]NotityMsgMuxItemDel() operation end", 
					(*iter).first.c_str(), myInfo.name.c_str());
			}		
		}
	}
	catch(const Ice::ObjectNotExistException&) 
	{
		glog(ZQ::common::Log::L_ERROR, 
			"NotityMsgMuxItemDel()\tcannt connect to streamer");
		return false;
	}
	catch (const ::Ice::Exception & ex) 
	{
		glog(ZQ::common::Log::L_ERROR,
			"NotityMsgMuxItemDel() caught Ice::Exception (%s)",
			ex.ice_name().c_str());
		return false;
    } 
	
	try
	{
		::TianShanIce::Storage::ContentPrx contentprx = 
			DataOnDemand::DataPublisherImpl::_contentStroe->
			openContent(contentname,::TianShanIce::Storage::ctDODTS ,false);
		
		if(contentprx)
		{
			contentprx->destroy();
			glog(ZQ::common::Log::L_ERROR,
				"[ContentName=%s]NotityMsgMuxItemDel()"
				 " DODContent destroy success!",
				contentname.c_str());
		}
	}
	catch (::TianShanIce::InvalidParameter&ex)
	{	
		glog(ZQ::common::Log::L_ERROR,
			"ActiveMsgChannel::NotityMsgMuxItemDel() caught "
			"TianShanIce::InvalidParameter(%s)",
			ex.message.c_str());
		return false;
	}
	catch (::TianShanIce::InvalidStateOfArt&ex) 
	{
		glog(ZQ::common::Log::L_ERROR,
			"ActiveMsgChannel::NotityMsgMuxItemDel() caught  "
			"TianShanIce::InvalidStateOfArt(%s)",
			ex.message.c_str());
		return false;
	}
	catch(::TianShanIce::NotImplemented&ex)
	{
		glog(ZQ::common::Log::L_ERROR,
			"ActiveMsgChannel::NotityMsgMuxItemDel() caught "
			"TianShanIce::NotImplemented(%s)",
			ex.message.c_str());
		return false;
	}
	catch (::Ice::Exception&ex) 
	{
		glog(ZQ::common::Log::L_ERROR,
			"ActiveMsgChannel::NotityMsgMuxItemDel() caught "
			"Ice::Exception(%s)",
			ex.ice_name().c_str());
		return false;
	}
	return true;
}
bool ActiveMsgChannel::LoadMsgData()
{
	TianShanIce::Properties myProps = _channelPrx->getProperties();
	DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();
	
    ::std::string _CacheDir = myProps["Path"];
     ZQCMessageInfoTINF info;

	time_t currenttime;
	int npos1, npos2;
	char strdeltime[40] = "0";
    struct tm *deltime;
	::std::list<::std::string>strFileName;
	
	if(ListFile(_CacheDir.c_str(), strFileName))
	{
		glog(ZQ::common::Log::L_ERROR,
			"[ChannelName: %s]ActiveMsgChannel() Load Message error(%d)",
			 myInfo.name.c_str(), GetLastError()); 
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
					  "[channelname: %s]delete MessageID(%s) filename(%s) error,errorcode(%d)",
					  myInfo.name.c_str(), info.sMessageID.c_str(), 
					  info.fileName.c_str(), GetLastError());
			  }
			  else
			  {  
				  time_t DeleteTime = info.deleteTime;
				  deltime = localtime( &DeleteTime);
				  sprintf(strdeltime,"%04d-%02d-%02d %02d:%02d:%02d",
					  deltime->tm_year + 1900, deltime->tm_mon + 1,
					  deltime->tm_mday, deltime->tm_hour,
					  deltime->tm_min,deltime->tm_sec);
				  glog(ZQ::common::Log::L_INFO,
				  "[channelname: %s]delete MessageID(%s) \t\t   "
				  "filename(%s) OK,\t\t   \tDeletime(%s)",myInfo.name.c_str(), 
				  info.sMessageID.c_str(),info.fileName.c_str(),
				  strdeltime);
			  }
              NotityMsgMuxItemDel(info.GroupId,(*itor));
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
		glog(ZQ::common::Log::L_WARNING,
			"Cache Path (%s) current path is not exist!",
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
		glog(Log::L_INFO, 
			"ActiveMsgChannel::stop()\t the state is stoped");
		return;
	}
	m_bStop = true;
	SetEvent(m_hWaitEvent);
	waitHandle(INFINITE);
}
bool ActiveMsgChannel::WrapData(std::string content,std::string filename,
													std::string& _contentpath)
{
	DestLinks::iterator iter;
    ::DataOnDemand::DestInfo destinfo;	
	::DataOnDemand::MuxItemPrx muxitemprx;

	std::string filefoldpath, contentname;
	::TianShanIce::Storage::ContentPrx contentprx;

	TianShanIce::Properties myProps = _channelPrx->getProperties();
	DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();

	filefoldpath = "msg://" + content;
    contentname = configSpaceName + "_" + filename;
	try
	{			
		contentprx =  DataOnDemand::DataPublisherImpl::_contentStroe->
			openContent(contentname,
			::TianShanIce::Storage::ctDODTS ,true);
		DataOnDemand::DODContentPrx dodcontent = 
			::DataOnDemand::DODContentPrx::checkedCast(contentprx);
		
		DataOnDemand::DataWrappingParam datawrap;
		
		datawrap.esPID = myInfo.streamId;
		datawrap.streamType = myInfo.streamType;
		datawrap.dataType = *myInfo.dataTypes.begin();
		datawrap.subStreamCount = myInfo.subchannelCount;
		datawrap.withObjIndex = myInfo.withDestination;
		datawrap.encryptType = myInfo.encrypt;
		datawrap.objTag = myInfo.tag;
		datawrap.versionNumber = 0;
		
		dodcontent->setDataWrappingParam(datawrap);	
		std::string startTimeUTC;
		std::string stopTimeUTC;
		dodcontent->provision(filefoldpath,::TianShanIce::Storage::ctDODTS,
							true,startTimeUTC,stopTimeUTC,0);

		::std::string targetCSType="";
		int transferProtocol = 0;
		int ttl = 0;
		::TianShanIce::Properties exppro;
		_contentpath = contentprx->getExportURL(targetCSType,transferProtocol, ttl, exppro);
		long size = _contentpath.find("file:");
		if(!size)
		{ 
			_contentpath = _contentpath.substr(5,_contentpath.size() - 5);
		}
	}
	catch (::TianShanIce::InvalidParameter&ex)
	{	
		glog(ZQ::common::Log::L_DEBUG,
			"ActiveMsgChannel::WrapData() caught "
			"TianShanIce::InvalidParameter(%s)",
			ex.message.c_str());
		return false;
	}
	catch (::TianShanIce::Storage::NoResourceException &ex)
	{
		glog(ZQ::common::Log::L_ERROR,
			"ActiveMsgChannel::WrapData() caught "
			"TianShanIce::Storage::NoResourceException(%s)",
			ex.message.c_str());
		return false;
	}
	catch (::TianShanIce::InvalidStateOfArt&ex) 
	{
		glog(ZQ::common::Log::L_ERROR,
			"ActiveMsgChannel::WrapData() caught "
			"TianShanIce::InvalidStateOfArt(%s)",
			ex.message.c_str());
		return false;
	}
	catch(::TianShanIce::NotImplemented&ex)
	{
		glog(ZQ::common::Log::L_ERROR,
			"ActiveMsgChannel::WrapData() caught "
			"TianShanIce::NotImplemented(%s)",
			ex.message.c_str());
		return false;
	}
	catch (::Ice::Exception&ex) 
	{
		glog(ZQ::common::Log::L_ERROR,
			"ActiveMsgChannel::WrapData() caught "
			"Ice::Exception(%s)",
			ex.ice_name().c_str());
		return false;
	}
	return true;
}