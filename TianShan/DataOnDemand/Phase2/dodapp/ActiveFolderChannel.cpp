// ActiveFolderChannel.cpp: implementation of the ActiveFolderChannel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ActiveFolderChannel.h"
#include "DataPublisherImpl.h"
#include <DODContentStore.h>
using namespace DataOnDemand;
using namespace ZQ::common;
//////////////////////////////////////////////////////////////////////
//classs ActiveLocalFolderChannel Impl
//////////////////////////////////////////////////////////////////////

ActiveLocalFolderChannel::ActiveLocalFolderChannel(
DataOnDemand::FolderChannelExPrx& channel, char* dir,long monitorTime):
_channelPrx(channel),DirConsumer(dir,monitorTime)
{
	_contentpath = "";
}
bool 
ActiveLocalFolderChannel::initchannel(void)
{
	if(!WrapData())
	{
      	glog(Log::L_WARNING, 
			"ActiveLocalFolderChannel::initchannel() [localfolder (%s)]"
			" WarpData error",_channelPrx->getInfo().name.c_str());
	}
	return start();
}
void ActiveLocalFolderChannel::uninit()
{
    DirConsumer::uninit();
}
ActiveLocalFolderChannel::~ActiveLocalFolderChannel()
{
	glog(Log::L_INFO, "[localfolder (%s)]~ActiveLocalFolderChannel()"
		" DeleteObject success",_channelPrx->getInfo().name.c_str());
}
bool ActiveLocalFolderChannel::NotifyMuxItem()
{
	ZQ::common::MutexGuard guard(_mutex);
	TianShanIce::Properties myProps = _channelPrx->getProperties();

	DataOnDemand::DestLinks myDestLinks = _channelPrx->getDestLinks();
	DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();

	DestLinks::iterator iter;	
	::DataOnDemand::MuxItemPrx muxitemprx;

	if(_contentpath.size() < 1)
	{
		glog(ZQ::common::Log::L_ERROR,
			"[channelname: %s]ActiveLocalFolderChannel:NotityMuxItem(): _contentpath invalidation", myInfo.name.c_str());
		return false;
	}

	try
	{
		for(iter = myDestLinks.begin(); iter != myDestLinks.end(); iter++)
		{		
			glog(ZQ::common::Log::L_INFO, 
				"[%s, %s]::UpdateChannel will start", 
				(*iter).first.c_str(), myInfo.name.c_str());
			
			TianShanIce::SRM::SessionPrx sessionprx = 
				DataOnDemand::DataPublisherImpl::_sessManager->
				openSession(iter->second.dest->getSessionId());
			
			::DataOnDemand::DataStreamPrx streamprx = 
				DataOnDemand::DataStreamPrx::checkedCast(
				sessionprx->getStream());

			glog(ZQ::common::Log::L_INFO, 
				"contentpath (%s)", _contentpath.c_str());

			muxitemprx = streamprx->getMuxItem(myInfo.name);
					
			muxitemprx->notifyFullUpdate(_contentpath);
			
			glog(ZQ::common::Log::L_INFO, 
				"[%s, %s]::Update operation end",
				(*iter).first.c_str(), myInfo.name.c_str());
		}
	}
	catch(const Ice::ObjectNotExistException&) 
	{
		glog(ZQ::common::Log::L_ERROR, 
			"ActiveLocalFolderChannel::NotityMuxItem()\tcannt connect to streamer");
		return false;
	}
	catch (const ::Ice::Exception & ex) 
	{
		glog(ZQ::common::Log::L_ERROR,
			"ActiveLocalFolderChannel::NotityMuxItem)) caught Ice::Exception(%s)",
			ex.ice_name().c_str());
		return false;
    } 

	return true;
}
bool ActiveLocalFolderChannel::WrapData()
{
	ZQ::common::MutexGuard guard(_mutex);
	TianShanIce::Properties myProps = _channelPrx->getProperties();
	DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();

	std::string srcFilefold;
	std::string myContentName, contentname;

	srcFilefold = "file:" + myProps["Path"];
	contentname = configSpaceName + "_" + myInfo.name;

   	try
	{
		myContentName = _channelPrx->getContentName(0);
	}
	catch (TianShanIce::InvalidParameter&)
	{
		myContentName = "";
	}

	try
	{	
	  	::TianShanIce::Storage::ContentPrx contentprx;
		if(!myContentName.empty())
		{
			contentprx=  DataOnDemand::DataPublisherImpl::_contentStroe->
				openContent(myContentName,::TianShanIce::Storage::ctDODTS ,false);
			
			if(contentprx)
			{
				glog(ZQ::common::Log::L_INFO, 
					"ActiveLocalFolderChannel::WrapData() Destroy Content name(%s)",myContentName.c_str());	
				contentprx->destroy();
				_channelPrx->setContentName(0, "");
				_contentpath= "";
			}
		}
		std::string sTime;
		GetCurrentDatatime(sTime);
        myContentName = contentname + sTime + ".dod";

		contentprx =  DataOnDemand::DataPublisherImpl::_contentStroe->
			openContent(myContentName,::TianShanIce::Storage::ctDODTS ,true);
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
		string startTimeUTC;
		string stopTimeUTC;
		dodcontent->provision(srcFilefold,::TianShanIce::Storage::ctDODTS,
											true,startTimeUTC,stopTimeUTC,0);

		_channelPrx->setContentName(0, myContentName);
		::std::string targetCSType="";
		int transferProtocol = 0;
		int ttl = 0;
		::TianShanIce::Properties exppro;
	    _contentpath = dodcontent->getExportURL(targetCSType,transferProtocol,ttl,exppro);
		long size = _contentpath.find("file:");
		if(!size)
		{ 
			_contentpath = _contentpath.substr(5,_contentpath.size() - 5);
		}
	}
	catch (::TianShanIce::InvalidParameter&ex){	
		glog(ZQ::common::Log::L_ERROR,
			"ActiveLocalFolderChannel::WrapData() caught "
			"TianShanIce::InvalidParameter(%s)",
			ex.message.c_str());
		return false;
	}
	catch (::TianShanIce::InvalidStateOfArt&ex) {
		glog(ZQ::common::Log::L_ERROR,
			"ActiveLocalFolderChannel::WrapData() caught "
			"TianShanIce::InvalidStateOfArt(%s)",
			ex.message.c_str());
		return false;
	}
	catch(::TianShanIce::NotImplemented&ex){
		glog(ZQ::common::Log::L_ERROR,
			"ActiveLocalFolderChannel::WrapData() caught "
			"TianShanIce::NotImplemented(%s)",
			ex.message.c_str());
		return false;
	}
	catch (::TianShanIce::Storage::NoResourceException &ex){
		glog(ZQ::common::Log::L_ERROR,
			"ActiveLocalFolderChannel::WrapData() caught "
			"TianShanIce::Storage::NoResourceException(%s)",
			ex.message.c_str());
		return false;
	}
	catch (::Ice::Exception&ex) {
		glog(ZQ::common::Log::L_ERROR,
			"ActiveLocalFolderChannel::WrapData() caught "
			"Ice::Exception(%s)",
			ex.ice_name().c_str());
		return false;
	}
	return true;
}
bool ActiveLocalFolderChannel::notifyFoldChange() 
{
	ZQ::common::MutexGuard guard(_mutex);
	DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();

	if(!WrapData())
	{
		glog(ZQ::common::Log::L_ERROR, 
			"[channelname: %s]::ActiveLocalFolderChannel::notifyFoldChange() "
			"Warp Data Error",  myInfo.name.c_str());
		return false;
	}

    NotifyMuxItem();
	return true; 
}


//////////////////////////////////////////////////////////////////////
//classs ActiveSharedFolderChannel Impl
//////////////////////////////////////////////////////////////////////

ActiveSharedFolderChannel::ActiveSharedFolderChannel(
	DataOnDemand::FolderChannelExPrx& channel): 
	_channelPrx(channel)
{

}

ActiveSharedFolderChannel::~ActiveSharedFolderChannel()
{
	glog(Log::L_DEBUG, "~ActiveSharedFolderChannel():DeleteObject success");
}
bool ActiveSharedFolderChannel::NotifyMuxItem(int groupId, int verNumber)
{
	ZQ::common::MutexGuard guard(_mutex);
	TianShanIce::Properties myProps = _channelPrx->getProperties();

	DataOnDemand::DestLinks myDestLinks = _channelPrx->getDestLinks();
	DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();

	DestLinks::iterator iter;
    ::DataOnDemand::DestInfo destinfo;	
	::DataOnDemand::MuxItemPrx muxitemprx;
	std::string _contentpath;
	std::string myContentName; 

	if(!WrapData(groupId,_contentpath,verNumber))
	{
		glog(ZQ::common::Log::L_ERROR, 
			"[channelname: %s]ActiveSharedFolderChannel::NotityMuxItem()"
			"Warp Data Error",  myInfo.name.c_str());
		return false;
	}

	try
	{
		for(iter = myDestLinks.begin(); iter != myDestLinks.end(); iter++)
		{		
			destinfo = iter->second.dest->getInfo();
			if(destinfo.groupId == groupId)
			{				
				TianShanIce::SRM::SessionPrx sessionprx = 
					DataOnDemand::DataPublisherImpl::_sessManager->
					openSession(iter->second.dest->getSessionId());
				
				::DataOnDemand::DataStreamPrx streamprx = 
					DataOnDemand::DataStreamPrx::checkedCast(
					sessionprx->getStream());
				
				muxitemprx = streamprx->getMuxItem(myInfo.name);

				glog(ZQ::common::Log::L_INFO, 
					"[%s,%s]::notifyFullUpdate will start", 
					(*iter).first.c_str(), myInfo.name.c_str());

				glog(ZQ::common::Log::L_INFO, 
					"contentpath = %s", _contentpath.c_str());

				muxitemprx->notifyFullUpdate(_contentpath);
				
				glog(ZQ::common::Log::L_INFO, 
					"[%s, %s]::notifyFullUpdate operation end",
					(*iter).first.c_str(), myInfo.name.c_str());
			}
		}
	}
	catch(const Ice::ObjectNotExistException&) 
	{
		glog(ZQ::common::Log::L_ERROR, 
			"ActiveSharedFolderChannel::NotityMuxItem()\tcan't connect to streamer");
		return false;
	}
	catch (const ::Ice::Exception & ex) 
	{
		glog(ZQ::common::Log::L_ERROR,
			"ActiveSharedFolderChannel::NotityMuxItem() caught Ice::Exception(%s)",
			ex.ice_name().c_str());
		return false;
    } 

	return true;
}
bool ActiveSharedFolderChannel::WrapData(int groupId, std::string& contentpath, int verNumber)
{
	ZQ::common::MutexGuard guard(_mutex);
	TianShanIce::Properties myProps = _channelPrx->getProperties();
	DataOnDemand::ChannelInfo myInfo = _channelPrx->getInfo();
	std::string myContentName;
	try
	{
		myContentName = _channelPrx->getContentName(groupId);
	}
	catch (TianShanIce::InvalidParameter&)
	{
		glog(ZQ::common::Log::L_ERROR,
			"[channelname : %s]ActiveSharedFolderChannel:WrapData() "
			"fail to get ContentName",myInfo.name.c_str());
    	return false;
	}
    
	char strtemp[MAX_PATH];
	std::string srcFilefold;
	std::string contentfile;
	std::string contentname;

	try
	{	
		::TianShanIce::Storage::ContentPrx contentprx;
		if(!myContentName.empty())
		{
			contentprx = DataOnDemand::DataPublisherImpl::_contentStroe->
				openContent(myContentName,::TianShanIce::Storage::ctDODTS ,false);
			
			if(contentprx)
			{
				glog(ZQ::common::Log::L_INFO, 
					"ActiveSharedFolderChannel::WrapData() Destroy Content"
					" name = %s",myContentName.c_str());
				contentprx->destroy();
				_channelPrx->setContentName(groupId, "");
			}
		}
        
		sprintf(strtemp, "file:%s\\%d",myProps["Path"].c_str(),groupId);
		//sprintf(strtemp, "file:%s//%d",myProps["Path"],groupId);
		srcFilefold = strtemp;
		contentname = configSpaceName + "_" + myInfo.name;
		
		std::string sTime;
		GetCurrentDatatime(sTime);
		
		myContentName = contentname + "_" + sTime +".dod";
		
		contentprx =  DataOnDemand::DataPublisherImpl::_contentStroe->
			openContent(myContentName,::TianShanIce::Storage::ctDODTS ,true);

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
		datawrap.versionNumber = verNumber;
		
		dodcontent->setDataWrappingParam(datawrap);	
		string startTimeUTC;
		string stopTimeUTC;
		dodcontent->provision(srcFilefold,::TianShanIce::Storage::ctDODTS,
			true,startTimeUTC,stopTimeUTC,0);
		_channelPrx->setContentName(groupId,myContentName);
		::std::string targetCSType="";
		int transferProtocol = 0;
		int ttl = 0;
		::TianShanIce::Properties exppro;
		contentpath = contentprx->getExportURL(targetCSType,transferProtocol,ttl, exppro);
		long size = contentpath.find("file:");
		if(!size)
		{ 
			contentpath = contentpath.substr(5,contentpath.size() - 5);
		}
	}
	catch (::TianShanIce::InvalidParameter&ex)
	{	
		glog(ZQ::common::Log::L_ERROR,
			"ActiveSharedFolderChannel::WrapData() caught "
			"TianShanIce::InvalidParameter(%s)",
			ex.message.c_str());
		return false;
	}
	catch (::TianShanIce::Storage::NoResourceException &ex)
	{
		glog(ZQ::common::Log::L_ERROR,
			"ActiveSharedFolderChannel::WrapData() caught "
			"TianShanIce::Storage::NoResourceException(%s)",
			ex.message.c_str());
		return false;
	}
	catch (::TianShanIce::InvalidStateOfArt&ex) 
	{
		glog(ZQ::common::Log::L_ERROR,
			"ActiveSharedFolderChannel::WrapData() caught "
			"TianShanIce::InvalidStateOfArt(%s)",
			ex.message.c_str());
		return false;
	}
	catch(::TianShanIce::NotImplemented&ex)
	{
		glog(ZQ::common::Log::L_ERROR,
			"ActiveSharedFolderChannel::WrapData() caught "
			"TianShanIce::NotImplemented(%s)",
			ex.message.c_str());
		return false;
	}
	catch (::Ice::Exception&ex) 
	{
		glog(ZQ::common::Log::L_ERROR,
			"ActiveSharedFolderChannel::WrapData() caught "
			"Ice::Exception(%s)",
			ex.ice_name().c_str());
		return false;
	}
	return true;
}