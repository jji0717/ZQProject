// ActiveMsgQueueData.cpp: implementation of the ActiveMsgQueueData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ActiveMsgQueueData.h"
#include "DataAppImpl.h"
#include "Util.h"
#include "global.h"
#include "DataPointPublisherImpl.h"
#include <DataContentStore.h>
using namespace TianShanIce::Application::DataOnDemand;
using namespace ZQ::common;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ActiveMsgQueueData::ActiveMsgQueueData(
	TianShanIce::Application::DataOnDemand::MessageQueueExPrx& channelPrx):
	_dataPPPrx(channelPrx), m_pmessagemanage(TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_messagemanage)
{
	m_bStop = false;
}

ActiveMsgQueueData::~ActiveMsgQueueData()
{
	glog(Log::L_DEBUG, CLOGFMT(ActiveMsgQueueData, "[%s]destruction ActiveMsgQueueData"),_dataPPPrx->getInfo().name.c_str());
}

bool ActiveMsgQueueData::ActiveDataInit()
{
	return start();
}

void ActiveMsgQueueData::uninit()
{
	stop();
}

int ActiveMsgQueueData::run()
{
	TianShanIce::Application::DataOnDemand::DataPublishPointInfo myInfo = _dataPPPrx->getInfo();

	glog(Log::L_INFO, CLOGFMT(ActiveMsgQueueData, "[%s]enter  process message queue thread"),myInfo.name.c_str());

	TianShanIce::Application::DataOnDemand::MessageQueueExPrx ch = _dataPPPrx;

	long  second = 0;
	DWORD timeout = INFINITE; 
	time_t currenttime;
	struct tm *deltime;
	char strdeltime[40] = "0"; 
	try
	{
		while(!m_bStop)
		{  
			int msgcount = m_pmessagemanage->getMessageCount(myInfo.name);
			TianShanIce::Application::DataOnDemand::messageinfo msgInfo;
			bool bret;
			if(msgcount > 0)
			{
				bret = m_pmessagemanage->getMessage(myInfo.name, msgInfo);
				if(bret == false)
				{
					continue;
				}
				if(msgInfo.bForever)
				{
					timeout = INFINITE;
				}
				else
				{
					time(&currenttime);			
					second = msgInfo.deleteTime - currenttime;

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
       		glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveMsgQueueData,"[%s]message count(%d)"), myInfo.name.c_str(), msgcount);

			if( WaitForSingleObject(m_hWaitEvent, timeout) == WAIT_OBJECT_0)
			{
				ResetEvent(m_hWaitEvent);
				continue;
			}		

			time_t DeleteTime = msgInfo.deleteTime;
			deltime = localtime(&DeleteTime);
			sprintf(strdeltime,"%04d-%02d-%02d %02d:%02d:%02d",
				deltime->tm_year + 1900, deltime->tm_mon + 1,
				deltime->tm_mday, deltime->tm_hour,
				deltime->tm_min,deltime->tm_sec);
			glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveMsgQueueData,"[%s]delete Message[%s] successfully, contentfilename[%s]deletetime[%s]"), myInfo.name.c_str(), msgInfo.messageID.c_str(),msgInfo.contentfilename.c_str(),strdeltime);

			NotityMsgMuxItemDel(msgInfo.GroupId, msgInfo.contentfilename);

			m_pmessagemanage->deleteMessage(myInfo.name, msgInfo.messageID);
		}
    }
	catch(...)
	{
      	glog(ZQ::common::Log::L_ERROR,CLOGFMT(ActiveMsgQueueData,"[%s]caught exception(%d)"), myInfo.name.c_str(), GetLastError());
	}
	glog(ZQ::common::Log::L_INFO,CLOGFMT(ActiveMsgQueueData,"[%s]leave process message queue thread"), myInfo.name.c_str() );
	return 1;
}

bool ActiveMsgQueueData::threadInit()
{
	time_t lastupdate;

	TianShanIce::Application::DataOnDemand::DataPublishPointInfo myInfo = _dataPPPrx->getInfo();

    time(&lastupdate);
	m_hWaitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if(m_pmessagemanage->getMessageCount(myInfo.name))
		SetEvent(m_hWaitEvent);

	return true;
}

void ActiveMsgQueueData::final()
{
	if (m_hWaitEvent)
		CloseHandle(m_hWaitEvent);
}


void
ActiveMsgQueueData::onMessageDeleted(::Ice::Int groupId ,const ::std::string& messageId)
{
    TianShanIce::Application::DataOnDemand::DataPublishPointInfo myInfo = _dataPPPrx->getInfo();
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ActiveMsgQueueData,"[%s]enter onMessageDeleted()"), myInfo.name.c_str());

	if( m_pmessagemanage->modityMessage(myInfo.name, messageId))
	{
		SetEvent(m_hWaitEvent);
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveMsgQueueData,"[%s]leave onMessageDeleted()"), myInfo.name.c_str());
}

bool
ActiveMsgQueueData::onMessageAdded(
					::Ice::Int groupId,
					const ::std::string& messageId,
					const TianShanIce::Application::DataOnDemand::Message& message)
{	
	TianShanIce::Application::DataOnDemand::DataPublishPointInfo myInfo = _dataPPPrx->getInfo();

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ActiveMsgQueueData,"[%s]enter onMessageAdded()"), myInfo.name.c_str());

	if (message.msgBody.size()< 1)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData, "[%s]fail to create message file: message's length is zero"),myInfo.name.c_str());
		return false;
	}

	std::string messageBody;
	std::string destination = message.msgDest.substr(message.msgDest.size() - 6, 6) +  "/" ;
    messageBody = destination + message.msgBody;

	time_t deletime;
	std::string sTime;
	char strDel[30] = "";

	std::string  sContentfileName;
	GetCurrentDatatime(sTime);
	sContentfileName = sTime + message.msgDest + ".dod";;
	
	TianShanIce::Application::DataOnDemand::messageinfo msginfo;

	msginfo.messageID = messageId;
	msginfo.GroupId = groupId;
	msginfo.messageBody = messageBody;
	msginfo.contentfilename = sContentfileName;

	if (message.expiration == 0)
	{
		msginfo.bForever = TRUE;
		msginfo.deleteTime = -1;
	}	
	else
	{
		time(&deletime);
		msginfo.bForever = FALSE;
		msginfo.deleteTime = deletime + message.expiration;
	}	

    int nret = m_pmessagemanage->addMessage(myInfo.name, msginfo);
	if(nret == MessageManage::InsertPos::unknown)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData, "[%s]failed to add message to messagelist"), myInfo.name.c_str());
		return false;
	}

	if(nret == MessageManage::InsertPos::first)
	{
      SetEvent(m_hWaitEvent);
	}

	try
	{
		std::string _contentpath;
		if(!WrapData(messageBody,sContentfileName,_contentpath))
		{
          	glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData, "[%s] failed to wrap message data file"), myInfo.name.c_str());
			return false;
		}
	   NotityMsgMuxItemAdd(groupId,_contentpath);
	}
	catch (...) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData, "[%s]onMessageAdded() caught exception(%d)"),myInfo.name.c_str(),GetLastError());
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveMsgQueueData, "[%s]Leave onMessageAdded()"), myInfo.name.c_str());
	return true;
}

bool ActiveMsgQueueData::NotityMsgMuxItemAdd(int groupId,
										        std::string _contentpath)
{
	TianShanIce::Application::DataOnDemand::DataStreamLinks::iterator iter;

	TianShanIce::Streamer::DataOnDemand::MuxItemPrx muxitemprx;

	TianShanIce::Application::DataOnDemand::DataStreamLinks myDataStreamLinks = _dataPPPrx->getDataStreamLinks();
	TianShanIce::Application::DataOnDemand::DataPublishPointInfo myInfo = _dataPPPrx->getInfo();

	try
	{
		if(groupId != 0 )
		{	
			for(iter = myDataStreamLinks.begin(); iter != myDataStreamLinks.end(); iter++)
			{		
				TianShanIce::SRM::ResourceMap resourceRequirement = iter->second.dest->getResourceRequirement();
				TianShanIce::ValueMap::iterator itorVmap;
				TianShanIce::SRM::ResourceMap::iterator itorRsMap;
				int groupid;
				itorRsMap = resourceRequirement.find(::TianShanIce::SRM::ResourceType::rtServiceGroup);
				if(itorRsMap != resourceRequirement.end())
				{
					::TianShanIce::SRM::Resource& resource = itorRsMap->second;
					itorVmap = resource.resourceData.find("id");
					if(itorVmap != resource.resourceData.end())
					{
						groupid  = itorVmap->second.ints[0];
						glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveMsgQueueData,"[%s][%s] servicegroup ID(%d)"), myInfo.name.c_str(), (*iter).first.c_str(), groupid);
					}
					else
					{
						glog(ZQ::common::Log::L_WARNING, CLOGFMT(ActiveMsgQueueData,"[%s][%s]failed to get servicegroup ID"), myInfo.name.c_str(), (*iter).first.c_str());
						continue;
					}
				}
				else
				{
					glog(ZQ::common::Log::L_WARNING, CLOGFMT(ActiveMsgQueueData,"[%s][%s]failed to get servicegroup ID"), myInfo.name.c_str(), (*iter).first.c_str());
					continue;
				}

				if(groupid == groupId ||groupid == 0)
				{								
					TianShanIce::SRM::SessionPrx sessionprx = iter->second.dest->getSession();
				
			   		TianShanIce::Streamer::DataOnDemand::DataStreamPrx streamprx = 
						TianShanIce::Streamer::DataOnDemand::DataStreamPrx::checkedCast(
														sessionprx->getStream());

		     		muxitemprx = streamprx->getMuxItem(myInfo.name);

					glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveMsgQueueData,"[%s][%s]notify message add, content file path[%s]"), myInfo.name.c_str(),(*iter).first.c_str(), _contentpath.c_str());
					muxitemprx->notifyFileAdded(_contentpath);			
					glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveMsgQueueData,"[%s][%s]finished notify message add"), myInfo.name.c_str(),(*iter).first.c_str());	
				}
			}		
		}
		else
		{
			for(iter = myDataStreamLinks.begin(); iter != myDataStreamLinks.end(); iter++)
			{		
                TianShanIce::SRM::SessionPrx sessionprx = iter->second.dest->getSession();
			
				TianShanIce::Streamer::DataOnDemand::DataStreamPrx streamprx = 
				       TianShanIce::Streamer::DataOnDemand::DataStreamPrx::checkedCast(
												sessionprx->getStream());
		
			   muxitemprx = streamprx->getMuxItem(myInfo.name);

				glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveMsgQueueData,"[%s][%s]notify file added, content file path[%s]"), myInfo.name.c_str(),(*iter).first.c_str(), _contentpath.c_str());													

				muxitemprx->notifyFileAdded(_contentpath);
				
				glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveMsgQueueData,"[%s][%s]finished notify file added"), myInfo.name.c_str(),(*iter).first.c_str());
			}		
		}
	}
	catch(const Ice::ObjectNotExistException&) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData, "[%s]failed to notify full data update caught exception [%s]"), myInfo.name.c_str());
		return false;
	}
	catch (const Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData, "[%s]failed to notify full data update caught exception [%s]"), myInfo.name.c_str(), ex.ice_name().c_str());
		return false;
	} 
	return true;
}

bool ActiveMsgQueueData::NotityMsgMuxItemDel(int groupId,std::string fileName)
{
	TianShanIce::Application::DataOnDemand::DataStreamLinks::iterator iter;
	TianShanIce::Application::DataOnDemand::DataStreamInfo destinfo;

	TianShanIce::Streamer::DataOnDemand::MuxItemPrx muxitemprx;

	TianShanIce::Application::DataOnDemand::DataStreamLinks myDataStreamLinks = _dataPPPrx->getDataStreamLinks();
	TianShanIce::Application::DataOnDemand::DataPublishPointInfo myInfo = _dataPPPrx->getInfo();
	std::string contentname, _contentpath;
   
    contentname = configSpaceName + "_" + fileName;

	::TianShanIce::Storage::ContentPrx contentprx;				
	contentprx=  TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_contentStroe->
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
			for(iter = myDataStreamLinks.begin(); iter != myDataStreamLinks.end(); iter++)
			{		
				TianShanIce::SRM::ResourceMap resourceRequirement = iter->second.dest->getResourceRequirement();
				TianShanIce::ValueMap::iterator itorVmap;
				TianShanIce::SRM::ResourceMap::iterator itorRsMap;
				int groupid;
				itorRsMap = resourceRequirement.find(::TianShanIce::SRM::ResourceType::rtServiceGroup);
				if(itorRsMap != resourceRequirement.end())
				{
					::TianShanIce::SRM::Resource& resource = itorRsMap->second;
					itorVmap = resource.resourceData.find("id");
					if(itorVmap != resource.resourceData.end())
					{
						groupid  = itorVmap->second.ints[0];
						glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveMsgQueueData,"[%s][%s] servicegroup ID(%d)"), myInfo.name.c_str(), (*iter).first.c_str(), groupid);
					}
					else
					{
						glog(ZQ::common::Log::L_WARNING, CLOGFMT(ActiveMsgQueueData,"[%s][%s]failed to get servicegroup ID"), myInfo.name.c_str(), (*iter).first.c_str());

						continue;
					}
				}
				else
				{
					glog(ZQ::common::Log::L_WARNING, CLOGFMT(ActiveMsgQueueData,"[%s][%s]failed to get servicegroup ID"), myInfo.name.c_str(), (*iter).first.c_str());
					continue;
				}

				if(groupid == groupId || groupid == 0)
				{								

					TianShanIce::SRM::SessionPrx sessionprx = iter->second.dest->getSession();

					TianShanIce::Streamer::DataOnDemand::DataStreamPrx streamprx = 
						TianShanIce::Streamer::DataOnDemand::DataStreamPrx::checkedCast(
						sessionprx->getStream());

					glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveMsgQueueData,"[%s][%s]notity message delete"), myInfo.name.c_str(),(*iter).first.c_str());

					glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveMsgQueueData,"[%s][%s]content file path[%s]"), myInfo.name.c_str(),(*iter).first.c_str(),_contentpath.c_str());

					muxitemprx = streamprx->getMuxItem(myInfo.name);

					muxitemprx->notifyFileDeleted(_contentpath);

					glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveMsgQueueData,"[%s][%s]finished notity message delete"), myInfo.name.c_str(),(*iter).first.c_str());	
				}
			}		
		}
		else
		{
			for(iter = myDataStreamLinks.begin(); iter != myDataStreamLinks.end(); iter++)
			{		

                TianShanIce::SRM::SessionPrx sessionprx = iter->second.dest->getSession();

				TianShanIce::Streamer::DataOnDemand::DataStreamPrx streamprx = 
				       TianShanIce::Streamer::DataOnDemand::DataStreamPrx::checkedCast(
													   sessionprx->getStream());
			
			   muxitemprx = streamprx->getMuxItem(myInfo.name);
															
				glog(ZQ::common::Log::L_INFO,  CLOGFMT(ActiveMsgQueueData,"[%s][%s]notity message delete,content file path[%s]"), myInfo.name.c_str(),(*iter).first.c_str(), _contentpath.c_str());
				muxitemprx->notifyFileDeleted(_contentpath);
				glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveMsgQueueData,"[%s][%s]finished notity message delete"), myInfo.name.c_str(),(*iter).first.c_str());
			}		
		}
	}
	catch(const Ice::ObjectNotExistException&) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData, "[%s]failed to notity message delete caught exception [%s]"), myInfo.name.c_str());
		return false;
	}
	catch (const Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData, "[%s]failed to notity message delete caught exception [%s]"), myInfo.name.c_str(), ex.ice_name().c_str());
		return false;
	} 
	try
	{
		TianShanIce::Storage::ContentPrx contentprx = 
			TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_contentStroe->
			openContent(contentname,::TianShanIce::Storage::ctDODTS ,false);
		
		if(contentprx)
		{
			contentprx->destroy();
			glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveMsgQueueData,"[%s]destroy content file [%s] successfully"),myInfo.name.c_str(), contentname.c_str());
		}
	}
	catch (::TianShanIce::InvalidParameter&ex){	
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData,"[%s]failed to destory content caught exception[%s]"),myInfo.name.c_str(),  ex.message.c_str());
		return false;
	}
	catch (::TianShanIce::InvalidStateOfArt&ex) {
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData,"[%s]failed to destory content caught exception[%s]"),myInfo.name.c_str(),  ex.message.c_str());

		return false;
	}
	catch(::TianShanIce::NotImplemented&ex){
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData,"[%s]failed to destory content caught exception[%s]"),myInfo.name.c_str(),  ex.message.c_str());

		return false;
	}
	catch (::TianShanIce::Storage::NoResourceException &ex){
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData,"[%s]failed to destory content caught exception[%s]"),myInfo.name.c_str(),  ex.message.c_str());

		return false;
	}
	catch (::Ice::Exception&ex) {
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData,"[%s]failed to destory content caught exception[%s]"),myInfo.name.c_str(),  ex.ice_name().c_str());
		return false;
	}
	return true;
}

void ActiveMsgQueueData::stop()
{
	TianShanIce::Application::DataOnDemand::DataPublishPointInfo myInfo = _dataPPPrx->getInfo();
	if (m_bStop) {
		return;
	}
	m_bStop = true;
	SetEvent(m_hWaitEvent);
	waitHandle(INFINITE);
}
bool ActiveMsgQueueData::WrapData(std::string content,std::string filename,
													std::string& _contentpath)
{
	TianShanIce::Application::DataOnDemand::DataStreamLinks::iterator iter;
	TianShanIce::Application::DataOnDemand::DataStreamInfo destinfo;	
	TianShanIce::Streamer::DataOnDemand::MuxItemPrx muxitemprx;

	std::string filefoldpath, contentname;
	::TianShanIce::Storage::ContentPrx contentprx;

	TianShanIce::Properties myProps = _dataPPPrx->getProperties();
	TianShanIce::Application::DataOnDemand::DataPublishPointInfo myInfo = _dataPPPrx->getInfo();

	filefoldpath = "msg://" + content;
    contentname = configSpaceName + "_" + filename;
	try
	{			
		contentprx =  TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_contentStroe->
			openContent(contentname,
			TianShanIce::Storage::ctDODTS ,true);
		TianShanIce::Storage::DataOnDemand::DODContentPrx dodcontent = 
			TianShanIce::Storage::DataOnDemand::DODContentPrx::checkedCast(contentprx);
		
		TianShanIce::Storage::DataOnDemand::DataWrappingParam datawrap;
		
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
	catch (::TianShanIce::InvalidParameter&ex){	
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData,"[%s]failed to wrapped localfolder data caught exception [%s]"),myInfo.name.c_str(), ex.message.c_str());
		return false;
	}
	catch (::TianShanIce::InvalidStateOfArt&ex) {
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData,"[%s]failed to wrapped localfolder data caught exception [%s]"),myInfo.name.c_str(), ex.message.c_str());
		return false;
	}
	catch(::TianShanIce::NotImplemented&ex){
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData,"[%s]failed to wrapped localfolder data caught exception [%s]"),myInfo.name.c_str(), ex.message.c_str());
		return false;
	}
	catch (::TianShanIce::Storage::NoResourceException &ex){
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData,"[%s]failed to wrapped localfolder data caught exception [%s]"),myInfo.name.c_str(), ex.message.c_str());
		return false;
	}
	catch (::Ice::Exception&ex) {
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveMsgQueueData,"[%s]failed to wrapped localfolder data caught exception [%s]"), myInfo.name.c_str(), ex.ice_name().c_str());
		return false;
	}
	return true;
}