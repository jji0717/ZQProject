// ActiveFolderData.cpp: implementation of the ActiveFolderData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ActiveFolderData.h"
#include "DataPointPublisherImpl.h"
#include <DataContentStore.h>
using namespace TianShanIce::Application::DataOnDemand;
using namespace ZQ::common;
//#define ACTIVELOCALFOLDERDATA "ActiveLocalFolderData"
//#define ACTIVESHAREFOLDERDATA "ActiveSharedFolderData"
//////////////////////////////////////////////////////////////////////
//classs ActiveLocalFolderData Impl
//////////////////////////////////////////////////////////////////////

ActiveLocalFolderData::ActiveLocalFolderData(
	TianShanIce::Application::DataOnDemand::FolderExPrx& channel, char* dir,long monitorTime):
_dataPPPrx(channel),DirConsumer(dir,monitorTime)
{
	_contentpath = "";
}
bool 
ActiveLocalFolderData::initchannel(void)
{
	if(!WrapData())
	{
      	glog(Log::L_WARNING, CLOGFMT(ActiveLocalFolderData, "[%s]failed to initialize local folder datapublishpoint (warp data error)"),
			_dataPPPrx->getInfo().name.c_str());
	}
	return start();
}
void ActiveLocalFolderData::uninit()
{
    DirConsumer::uninit();
}
ActiveLocalFolderData::~ActiveLocalFolderData()
{
	glog(Log::L_DEBUG, CLOGFMT(ActiveLocalFolderData, 	"[%s]destruction ActiveLocalFolderData"),_dataPPPrx->getInfo().name.c_str());
}
bool ActiveLocalFolderData::NotifyMuxItem()
{
	ZQ::common::MutexGuard guard(_mutex);
	TianShanIce::Properties myProps = _dataPPPrx->getProperties();

	TianShanIce::Application::DataOnDemand::DataStreamLinks myDatastreamLinks = _dataPPPrx->getDataStreamLinks();
	TianShanIce::Application::DataOnDemand::DataPublishPointInfo myInfo = _dataPPPrx->getInfo();

	TianShanIce::Application::DataOnDemand::DataStreamLinks::iterator iter;	
	TianShanIce::Streamer::DataOnDemand::MuxItemPrx muxitemprx;

	if(_contentpath.size() < 1)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveLocalFolderData, "[%s]invailed content file path"), myInfo.name.c_str());
		return false;
	}

	try
	{
		for(iter = myDatastreamLinks.begin(); iter != myDatastreamLinks.end(); iter++)
		{		
			glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveLocalFolderData,"[%s][%s]update localfolder data,content file path [%s]"), myInfo.name.c_str(), (*iter).first.c_str(), _contentpath.c_str());
			
			TianShanIce::SRM::SessionPrx sessionprx = iter->second.dest->getSession();
			
			TianShanIce::Streamer::DataOnDemand::DataStreamPrx streamprx = 
				TianShanIce::Streamer::DataOnDemand::DataStreamPrx::checkedCast(sessionprx->getStream());
			muxitemprx = streamprx->getMuxItem(myInfo.name);
					
			muxitemprx->notifyFullUpdate(_contentpath);
			
			glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveLocalFolderData,"[%s][%s]finished update localfolder data"),myInfo.name.c_str(), (*iter).first.c_str());
		}
	}
	catch(const Ice::ObjectNotExistException&) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveLocalFolderData, "[%s]failed to update localfolder data caught exception [%s]"), myInfo.name.c_str());
		return false;
	}
	catch (const Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveLocalFolderData, "[%s]failed to update localfolder data caught exception [%s]"), myInfo.name.c_str(), ex.ice_name().c_str());
		return false;
    } 

	return true;
}
bool ActiveLocalFolderData::WrapData()
{
	ZQ::common::MutexGuard guard(_mutex);
	TianShanIce::Properties myProps = _dataPPPrx->getProperties();
	TianShanIce::Application::DataOnDemand::DataPublishPointInfo myInfo = _dataPPPrx->getInfo();

	std::string srcFilefold;
	std::string myContentName, contentname;

	srcFilefold = "file:" + myProps["Path"];
	contentname = configSpaceName + "_" + myInfo.name;

   	try
	{
		myContentName = _dataPPPrx->getContentName(0);
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
			contentprx=  TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_contentStroe->
				openContent(myContentName,::TianShanIce::Storage::ctDODTS ,false);
			
			if(contentprx)
			{
				glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveLocalFolderData, "[%s]destroy content file [%s]"), myInfo.name.c_str() ,  myContentName.c_str());	
				contentprx->destroy();
				_dataPPPrx->setContentName(0, "");
				_contentpath= "";
			}
		}
		std::string sTime;
		GetCurrentDatatime(sTime);
        myContentName = contentname + sTime + ".dod";

		contentprx =  TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_contentStroe->
			openContent(myContentName,::TianShanIce::Storage::ctDODTS ,true);
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
		string startTimeUTC;
		string stopTimeUTC;
		dodcontent->provision(srcFilefold,::TianShanIce::Storage::ctDODTS,
											true,startTimeUTC,stopTimeUTC,0);

		_dataPPPrx->setContentName(0, myContentName);
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
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveLocalFolderData,"[%s]failed to wrapped localfolder data caught exception [%s]"),myInfo.name.c_str(), ex.message.c_str());
		return false;
	}
	catch (::TianShanIce::InvalidStateOfArt&ex) {
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveLocalFolderData,"[%s]failed to wrapped localfolder data caught exception [%s]"),myInfo.name.c_str(), ex.message.c_str());
		return false;
	}
	catch(::TianShanIce::NotImplemented&ex){
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveLocalFolderData,"[%s]failed to wrapped localfolder data caught exception [%s]"),myInfo.name.c_str(), ex.message.c_str());
		return false;
	}
	catch (::TianShanIce::Storage::NoResourceException &ex){
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveLocalFolderData,"[%s]failed to wrapped localfolder data caught exception [%s]"),myInfo.name.c_str(), ex.message.c_str());
		return false;
	}
	catch (::Ice::Exception&ex) {
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveLocalFolderData,"[%s]failed to wrapped localfolder data caught exception [%s]"), myInfo.name.c_str(), ex.ice_name().c_str());
		return false;
	}
	return true;
}
bool ActiveLocalFolderData::notifyFoldChange() 
{
	ZQ::common::MutexGuard guard(_mutex);
	TianShanIce::Application::DataOnDemand::DataPublishPointInfo myInfo = _dataPPPrx->getInfo();

	if(!WrapData())
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveLocalFolderData,"[%s]failed to wrap data"), myInfo.name.c_str());
		return false;
	}
    NotifyMuxItem();
	return true; 
}


//////////////////////////////////////////////////////////////////////
//classs ActiveSharedFolderData Impl
//////////////////////////////////////////////////////////////////////

ActiveSharedFolderData::ActiveSharedFolderData(
	TianShanIce::Application::DataOnDemand::FolderExPrx& channel): 
	_dataPPPrx(channel)
{

}

ActiveSharedFolderData::~ActiveSharedFolderData()
{
	glog(Log::L_DEBUG, CLOGFMT(ActiveSharedFolderData, "[%s]destruction ActiveSharedFolderData"),_dataPPPrx->getInfo().name.c_str());
}
bool ActiveSharedFolderData::NotifyMuxItem(int groupId, int verNumber)
{
	ZQ::common::MutexGuard guard(_mutex);
	TianShanIce::Properties myProps = _dataPPPrx->getProperties();

	TianShanIce::Application::DataOnDemand::DataStreamLinks myDataStreamLinks = _dataPPPrx->getDataStreamLinks();
	TianShanIce::Application::DataOnDemand::DataPublishPointInfo myInfo = _dataPPPrx->getInfo();

	TianShanIce::Application::DataOnDemand::DataStreamLinks::iterator iter;
	TianShanIce::Streamer::DataOnDemand::MuxItemPrx muxitemprx;
	std::string _contentpath;
	std::string myContentName; 

	if(!WrapData(groupId,_contentpath,verNumber))
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveSharedFolderData,"[%s] failed to wrapped data"),  myInfo.name.c_str());
		return false;
	}

	try
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
					glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveSharedFolderData,"[%s][%s] servicegroup ID(%d)"), myInfo.name.c_str(),(*iter).first.c_str(), groupid);
				}
				else
				{
					glog(ZQ::common::Log::L_WARNING, CLOGFMT(ActiveSharedFolderData,"[%s][%s]failed to get servicegroup ID"), myInfo.name.c_str(), (*iter).first.c_str());
					continue;
				}
			}
			else
			{
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(ActiveSharedFolderData,"[%s][%s]failed to get servicegroup ID"), myInfo.name.c_str(),(*iter).first.c_str());
				continue;
			}

			if(groupid == groupId)
			{				
				TianShanIce::SRM::SessionPrx sessionprx = iter->second.dest->getSession();
				
				TianShanIce::Streamer::DataOnDemand::DataStreamPrx streamprx = 
					TianShanIce::Streamer::DataOnDemand::DataStreamPrx::checkedCast(
					sessionprx->getStream());
				
				muxitemprx = streamprx->getMuxItem(myInfo.name);

				glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveSharedFolderData,"[%s][%s]notify full data update, content file path[%s]"), myInfo.name.c_str(),(*iter).first.c_str(), _contentpath.c_str());
				muxitemprx->notifyFullUpdate(_contentpath);				
				glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveSharedFolderData, "[%s][%s]finished to notify full data update"), myInfo.name.c_str(),(*iter).first.c_str());
			}
		}
	}
	catch(const Ice::ObjectNotExistException& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveSharedFolderData, "[%s]failed to notify full data update caught exception [%s]"), myInfo.name.c_str(), ex.ice_name().c_str());
		return false;
	}
	catch (const Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveSharedFolderData, "[%s]failed to notify full data update caught exception [%s]"), myInfo.name.c_str(), ex.ice_name().c_str());
		return false;
	} 

	return true;
}
bool ActiveSharedFolderData::WrapData(int groupId, std::string& contentpath, int verNumber)
{
	ZQ::common::MutexGuard guard(_mutex);
	TianShanIce::Properties myProps = _dataPPPrx->getProperties();
	TianShanIce::Application::DataOnDemand::DataPublishPointInfo myInfo = _dataPPPrx->getInfo();
	std::string myContentName;
	try
	{
		myContentName = _dataPPPrx->getContentName(groupId);
	}
	catch (TianShanIce::InvalidParameter&)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(ActiveSharedFolderData,"[%s]fail to get content file name"), myInfo.name.c_str());
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
			contentprx = TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_contentStroe->
				openContent(myContentName,::TianShanIce::Storage::ctDODTS ,false);
			
			if(contentprx)
			{
				glog(ZQ::common::Log::L_INFO, CLOGFMT(ActiveSharedFolderData, "[%s]destroy content file [%s]"), myInfo.name.c_str() ,  myContentName.c_str());	
				contentprx->destroy();
				_dataPPPrx->setContentName(groupId, "");
			}
		}
        
		sprintf(strtemp, "file:%s\\%d",myProps["Path"].c_str(),groupId);
		//sprintf(strtemp, "file:%s//%d",myProps["Path"],groupId);
		srcFilefold = strtemp;
		contentname = configSpaceName + "_" + myInfo.name;
		
		std::string sTime;
		GetCurrentDatatime(sTime);
		
		myContentName = contentname + "_" + sTime +".dod";
		
		contentprx =  TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_contentStroe->
			openContent(myContentName,::TianShanIce::Storage::ctDODTS ,true);

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
		datawrap.versionNumber = verNumber;
		
		dodcontent->setDataWrappingParam(datawrap);	
		string startTimeUTC;
		string stopTimeUTC;
		dodcontent->provision(srcFilefold,::TianShanIce::Storage::ctDODTS,
			true,startTimeUTC,stopTimeUTC,0);
		_dataPPPrx->setContentName(groupId,myContentName);
		::std::string targetCSType="";
		int transferProtocol = 0;
		int ttl = 0;
		::TianShanIce::Properties exppro;
		contentpath = contentprx->getExportURL(targetCSType, transferProtocol, ttl, exppro);
		long size = contentpath.find("file:");
		if(!size)
		{ 
			contentpath = contentpath.substr(5,contentpath.size() - 5);
		}
	}
	catch (::TianShanIce::InvalidParameter&ex){	
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveSharedFolderData,"[%s]failed to wrapped sharefolder data caught exception [%s]"),myInfo.name.c_str(), ex.message.c_str());
		return false;
	}
	catch (::TianShanIce::InvalidStateOfArt&ex) {
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveSharedFolderData,"[%s]failed to wrapped sharefolder data caught exception [%s]"),myInfo.name.c_str(), ex.message.c_str());
		return false;
	}
	catch(::TianShanIce::NotImplemented&ex){
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveSharedFolderData,"[%s]failed to wrapped sharefolder data caught exception [%s]"),myInfo.name.c_str(), ex.message.c_str());
		return false;
	}
	catch (::TianShanIce::Storage::NoResourceException &ex){
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveSharedFolderData,"[%s]failed to wrapped sharefolder data caught exception [%s]"),myInfo.name.c_str(), ex.message.c_str());
		return false;
	}
	catch (::Ice::Exception&ex) {
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ActiveSharedFolderData,"[%s]failed to wrapped sharefolder data caught exception [%s]"), myInfo.name.c_str(), ex.ice_name().c_str());
		return false;
	}
	return true;
}