#include "stdafx.h"
#include "DataAppImpl.h"
#include "DataStreamExImpl.h"
#include "DataPointPublisherImpl.h"
#include "util.h"
#include "global.h"
#include "DataAppThread.h"
#include "..\common\Reskey.h"
using namespace ZQ::common;
namespace TianShanIce {
namespace Application {
namespace DataOnDemand {
DataStreamExImpl::DataStreamExImpl()
{
/*	try
	{	
		if(!weiwooSession)
		{			
			_stream = DataOnDemand::DataStreamPrx::checkedCast(
											weiwooSession->getStream());
		}
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_WARNING,  CLOGFMT(DataStreamExImpl,"[%s]fail to  get stream porxy (%d)"), myInfo.name.c_str(), GetLastError());
		 weiwooSession = NULL;
	}*/
}

DataStreamExImpl::~DataStreamExImpl()
{
	_thisPrx = NULL;
}
bool 
DataOnDemand::DataStreamExImpl::init()
{
	return true;
}

/// impl interface ::TianShanIce::Application::PublishPoint
::std::string
DataStreamExImpl::getDesc(const Ice::Current& current) const
{
	Lock sync(*this);
	return  desc;
}

void
DataStreamExImpl::setDesc(const ::std::string& description,
						  const Ice::Current& current)
{
	Lock sync(*this);
	desc = description;
}

::Ice::Int
DataStreamExImpl::getMaxBitrate(const Ice::Current& current) const
{
	Lock sync(*this);
	return maxBitrate;
}

void
DataStreamExImpl::setMaxBitrate(::Ice::Int maxBit,const Ice::Current& current)
{
	Lock sync(*this);
	maxBitrate  = maxBit;
}
void
DataStreamExImpl::restrictReplica(const ::TianShanIce::StrValues& contentStoreNetIds,
											const Ice::Current& current)
{
	Lock sync(*this);
	replicas = contentStoreNetIds;
}

::TianShanIce::StrValues
DataStreamExImpl::listReplica(const Ice::Current& current) const
{
	Lock sync(*this);
	return replicas;
}

::std::string
DataStreamExImpl::getName(const Ice::Current& current) const
{
	Lock sync(*this);
	return myInfo.name;
}

::std::string 
DataStreamExImpl::getType(const Ice::Current& current) const
{
	Lock sync(*this);
	return type;
}

void
DataStreamExImpl::setProperties(const ::TianShanIce::Properties& prop,
											 const Ice::Current& current)
{
	Lock sync(*this);
	properties.clear();
	properties = prop;
}

::TianShanIce::Properties
DataStreamExImpl::getProperties(const Ice::Current& current) const
{
	Lock sync(*this);
	return properties;
}

void
DataOnDemand::DataStreamExImpl::destroy(const Ice::Current& current)
{
	Lock sync(*this);
	TianShanIce::SRM::SessionPrx sessionprx ;
	try
	{	
		glog(ZQ::common::Log::L_INFO, CLOGFMT(DataStreamExImpl, "[%s]destroy stream"), myInfo.name.c_str());

		if(weiwooSession)
		{
			weiwooSession->destroy();
		}
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataStreamExImpl,"[%s]fail to destroy stream (%d)"), myInfo.name.c_str(), GetLastError());	
	}

	SessionRenewThread *pThread = 
		TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::getSessionTrd(myInfo.name);
	if(pThread)
	{
		pThread->stop();
		TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_SessionTrdMap.erase(myInfo.name);
	}

	myParent->removeDataStream(myInfo.name);

	TianShanIce::Application::DataOnDemand::AttachedInfoDict::iterator iter;
	for(iter =myDataPublishPoints.begin(); iter != myDataPublishPoints.end(); iter++)
	{	
		TianShanIce::Application::DataOnDemand::DataPublishPointPrx datappPrx = iter->second.datapp;

		std::string chType = datappPrx->getType();

		if (chType == dataLocalFolder || chType == dataSharedFolder) {

			TianShanIce::Application::DataOnDemand::FolderExPrx folderExPrx = 
				TianShanIce::Application::DataOnDemand::FolderExPrx ::checkedCast(datappPrx);

			folderExPrx->unlinkDataStream(myInfo.name);

		} else if (chType == dataMessage) {

			TianShanIce::Application::DataOnDemand::MessageQueueExPrx msgQueueExPrx = 
				TianShanIce::Application::DataOnDemand::MessageQueueExPrx ::checkedCast(datappPrx);

			msgQueueExPrx->unlinkDataStream(myInfo.name);
		}		
	}

	myDataPublishPoints.clear();
}

///impl interface ::TianShanIce::Application::BroadcastPublishPoint
::Ice::Long
DataStreamExImpl::requireResource(::TianShanIce::SRM::ResourceType type,
								  const ::TianShanIce::SRM::Resource& res,
								  const Ice::Current& current)
{
	Lock sync(*this);
/*	::TianShanIce::SRM::ResourceMap::iterator itor;
	itor = resources.find(type);
	if(itor != resources.end())
		res = itor->second;
	else
		throw TianShanIce::InvalidParameter(DATASTREAMEXIMPL, 1,"can't find this resource");*/
	return 0;
}

::TianShanIce::SRM::ResourceMap
DataStreamExImpl::getResourceRequirement(const Ice::Current& current) const
{
	Lock sync(*this);
	return resources;
}

void
DataStreamExImpl::withdrawResourceRequirement(::TianShanIce::SRM::ResourceType type,
											  const Ice::Current& current)
{
	Lock sync(*this);	
	::TianShanIce::SRM::ResourceMap::iterator itor;
	itor = resources.find(type);
	if(itor != resources.end())
		resources.erase(itor);
}

::TianShanIce::SRM::SessionPrx
DataStreamExImpl::getSession(const Ice::Current& current)
{
	Lock sync(*this);
	return weiwooSession;
} 

void
DataStreamExImpl::setup(const Ice::Current& current)
{
}

::Ice::Long
DataStreamExImpl::getUpTime(const Ice::Current& current)
{
	Lock sync(*this);
	return 0;
}
DataStreamInfo
DataStreamExImpl::getInfo(const Ice::Current& current)
{
	Lock sync(*this);
	return myInfo;
}
void
DataOnDemand::DataStreamExImpl::start(const Ice::Current& current)
{
	TianShanIce::Streamer::StreamState state;

	TianShanIce::Streamer::DataOnDemand::DataStreamPrx mystream = getStreamPrx();
	if(mystream == NULL)
	{
		glog( ZQ::common::Log::L_WARNING,CLOGFMT(DataStreamExImpl, "[%s]fail to get stream proxy ,retry create this stream"), myInfo.name.c_str());
		return;
	}

	glog( ZQ::common::Log::L_INFO, CLOGFMT(DataStreamExImpl, "[%s]get current stream status"), myInfo.name.c_str());
	try {
		state = mystream->getCurrentState();	 
	} catch (Ice::Exception& ) {

		//throw TianShanIce::Application::DataOnDemand::StreamerException("DataTunnelApp", 1 , "fail to get stream state");				
		throw TianShanIce::Application::DataOnDemand::StreamerException();
	}

	if (state == TianShanIce::Streamer::stsStreaming) {
		glog( ZQ::common::Log::L_INFO, CLOGFMT(DataStreamExImpl, "[%s]stream already play"), myInfo.name.c_str());
		return; // already streamming

	} else if (state == TianShanIce::Streamer::stsStop) {

		//throw TianShanIce::Application::DataOnDemand::StreamInvalidState("DataTunnelApp", 1 , "invalid stream state");
		throw TianShanIce::Application::DataOnDemand::StreamerException();
	}

	bool bplay = mystream->play();

	if(bplay)
	{			
		myState = TianShanIce::Streamer::stsStreaming;
		glog(ZQ::common::Log::L_INFO, CLOGFMT(DataStreamExImpl,"[%s]creat session renew thread"),
			myInfo.name.c_str());
		try
		{	
			SessionRenewThread * pThread =  
				new SessionRenewThread(weiwooSession, myInfo.name);

			if(pThread)
			{
				pThread->start();
				TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_SessionTrdMap[myInfo.name] = pThread;
				pThread = NULL;
			}
		}
		catch (...) {
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataStreamExImpl,"[%s]start stream caught unknown excetpoion (%d)"),myInfo.name.c_str(),GetLastError());

			//throw TianShanIce::Application::DataOnDemand::StreamerException("DataTunnelApp", 1 , "start play caught unknown exception");				
			throw TianShanIce::Application::DataOnDemand::StreamerException();
		}
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(DataStreamExImpl, "[%s]fail to play stream, may be sum of channelRate over than port totalrate or stream status error"), myInfo.name.c_str());

		stop(Ice::Current());
		destroy(Ice::Current());
		Beep(700, 1000);	
//		throw TianShanIce::Application::DataOnDemand::StreamerException("DataTunnelApp",1, "fail to play stream, may be sum of channelRate over than port totalrate or stream status error");	
		throw TianShanIce::Application::DataOnDemand::StreamerException();
	}
}

///impl interface ::TianShanIce::Application::DataStream
void
DataOnDemand::DataStreamExImpl::stop(const Ice::Current& current)
{
	Lock sync(*this);
	TianShanIce::Streamer::StreamState state;

	TianShanIce::Streamer::DataOnDemand::DataStreamPrx mystream = getStreamPrx();
	if(mystream != NULL)
	{
		try {
			state = mystream->getCurrentState();

		} catch (Ice::Exception& ) {
			throw StreamerException();
		}

		if (state == TianShanIce::Streamer::stsStop) {

			throw StreamInvalidState();
		}

		try {

			mystream->destroy();	
			myState = TianShanIce::Streamer::stsStop;

		} catch (Ice::Exception& ) {

			throw StreamerException();
		}
	}
	else
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(DataStreamExImpl,"[%s]fail to get stream proxy"), myInfo.name.c_str());
	}

}



::TianShanIce::State
DataStreamExImpl::getState(const Ice::Current& current)
{
	Lock sync(*this);
	TianShanIce::State state;
	try
	{
		if(weiwooSession)
			state = weiwooSession->getState();
		else 
			state = TianShanIce::State::stOutOfService;
	}
	catch(Ice::Exception&ex)
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DataStreamExImpl, "[%s]getState caught ice exception (%s)"), myInfo.name.c_str(), ex.ice_name().c_str());
		state = TianShanIce::State::stOutOfService;
 	}

	return state;
}

DataAttachInfos
DataStreamExImpl::listAttachments(const ::std::string& dataPPName,const Ice::Current& current)
{
	Lock sync(*this);
	TianShanIce::Application::DataOnDemand::AttachedInfoDict::iterator dataPPItor;
	TianShanIce::Application::DataOnDemand::DataAttachInfos dataAttachinfos;
	if(dataPPName != "")
	{
		dataPPItor = myDataPublishPoints.find(dataPPName);
		if(dataPPItor != myDataPublishPoints.end())
		{
			dataAttachinfos.push_back(dataPPItor->second);
		}
	}
	else
	{
		for(dataPPItor = myDataPublishPoints.begin(); dataPPItor != myDataPublishPoints.end(); dataPPItor++)
		{
			dataAttachinfos.push_back(dataPPItor->second);
		}
	}		
	return dataAttachinfos;
}

void
DataStreamExImpl::attachDataPublishPoint(const ::std::string& dataPPName,
									     const ::TianShanIce::Application::DataOnDemand::DataAttachInfo& attachInfo, 
									     const Ice::Current& current)
{	
	Lock sync(*this);
	glog( ZQ::common::Log::L_INFO, CLOGFMT(DataStreamExImpl, "[%s]attach datapublishpoint (%s)"), myInfo.name.c_str(), dataPPName.c_str());

	DataPublishPointPrx datappPrx;
	DataPublishPointInfo datappInfo;
	TianShanIce::Streamer::DataOnDemand::MuxItemPrx muxItem;

	try {
		datappPrx = myParent->openDataPublishPoint(dataPPName);
		datappInfo = datappPrx->getInfo();

	} catch (Ice::ObjectNotExistException&) {
		glog( ZQ::common::Log::L_INFO, CLOGFMT(DataStreamExImpl,"[%s]can't get dataPublishPoint [%s] info"), myInfo.name.c_str(),dataPPName.c_str());
		throw ::TianShanIce::InvalidParameter();
	}


	if (myDataPublishPoints.find(dataPPName) != myDataPublishPoints.end()) {

		glog( ZQ::common::Log::L_INFO,CLOGFMT(DataStreamExImpl,"[%s]datapublishpoiont [%s] object already exist"), myInfo.name.c_str(),dataPPName.c_str());
		throw ObjectExistException();
	}

	FolderExPrx folderExPrx;
	MessageQueueExPrx msgQueuePrx;

	TianShanIce::Streamer::DataOnDemand::CacheType ctype;
	std::string addr;

	std::string chType = datappPrx->getType();

	if (chType == dataLocalFolder || chType == dataSharedFolder) {
		
		folderExPrx = FolderExPrx::checkedCast(datappPrx);
		folderExPrx->getCacheInfo(ctype, addr);

	} else {
		msgQueuePrx = MessageQueueExPrx::checkedCast(datappPrx);
		msgQueuePrx->getCacheInfo(ctype, addr);
	}

	TianShanIce::Streamer::DataOnDemand::DataStreamPrx mystream = getStreamPrx();
	if(mystream == NULL)
	{
//		throw StreamerException();
		glog( ZQ::common::Log::L_WARNING,CLOGFMT(DataStreamExImpl,"[%s]datastream porxy is invalid,retry create this stream"), myInfo.name.c_str());
		return;
	}

	try {
		TianShanIce::Streamer::DataOnDemand::MuxItemInfo muxItemInfo;
		muxItemInfo.name = datappInfo.name;
		muxItemInfo.streamId = datappInfo.streamId;
		muxItemInfo.streamType = datappInfo.streamType;
		muxItemInfo.bandWidth = attachInfo.minBitRate;
		muxItemInfo.tag = datappInfo.tag;
		muxItemInfo.expiration = 0;
		muxItemInfo.repeatTime = attachInfo.repeatTime;
		muxItemInfo.ctype = TianShanIce::Streamer::DataOnDemand::dodCacheTypeSmb;
		muxItemInfo.cacheAddr = addr;
		muxItemInfo.encryptMode = datappInfo.encrypt;
		muxItemInfo.subchannelCount = datappInfo.subchannelCount;
		muxItem = mystream->createMuxItem(muxItemInfo);
		if(	muxItem == NULL)
		{
			glog( ZQ::common::Log::L_ERROR, CLOGFMT(DataStreamExImpl,"[%s]fail to create MuxItem ,please check configration"), myInfo.name.c_str());
			throw Ice::ObjectNotExistException(__FILE__,__LINE__);
		}

	} catch (Ice::ObjectNotExistException&) {
		 throw StreamerException();
	} catch (TianShanIce::Streamer::DataOnDemand::NameDupException&) {

		throw ObjectExistException();
	}
	catch(Ice::Exception &ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataStreamExImpl,"[%s]create MuxItem caught ice exception (%s)"),myInfo.name.c_str(), ex.ice_name().c_str());
		throw TianShanIce::Application::DataOnDemand::StreamerException();
	}
	catch(...)
	{		
		glog( ZQ::common::Log::L_ERROR,CLOGFMT(DataStreamExImpl, "[%s]create MuxItem caught unknown exception(%d)"),myInfo.name.c_str(),GetLastError());
		throw TianShanIce::Application::DataOnDemand::StreamerException();
	}

	std::pair<AttachedInfoDict::iterator, bool> ir;
	DataAttachInfo ai;
	ai.datapp = datappPrx;
	ai.minBitRate = attachInfo.minBitRate;
	ai.repeatTime = attachInfo.repeatTime;
	ai.dataPublishPointName = dataPPName;

	ir = myDataPublishPoints.insert(
		AttachedInfoDict::value_type(dataPPName, ai));

	if (chType == dataLocalFolder || chType == dataSharedFolder)
	{	
		std::string  myContentName = "";
		std::string  _contentpath = "";
		if(chType == dataSharedFolder)
		{
			try
			{	
				myContentName = folderExPrx->
										getContentName(myInfo.groupId);
			}
			catch (TianShanIce::InvalidParameter&)
			{
				folderExPrx->setContentName(myInfo.groupId,myContentName);	
			}
		}
		else
		{
			try
			{	
				myContentName = folderExPrx->getContentName(0);

				::TianShanIce::Storage::ContentPrx contentprx;
				
				contentprx=  DataOnDemand::DataPointPublisherImpl::_contentStroe->
					openContent(myContentName,::TianShanIce::Storage::ctDODTS ,false);

				::std::string targetCSType="";
				int transferProtocol = 0;
				int ttl = 0;
				::TianShanIce::Properties exppro;
                 _contentpath = contentprx->getExportURL(targetCSType,
														transferProtocol,ttl, exppro);
				 long size = _contentpath.find("file:");
				 if(!size)
				 { 
					 _contentpath = _contentpath.substr(5,_contentpath.size() - 5);
				 }
				 
				 glog(ZQ::common::Log::L_INFO, CLOGFMT(DataStreamExImpl, "[%s][%s]notify Local folder data update"), myInfo.name.c_str(), dataPPName.c_str());
				 
				 muxItem->notifyFullUpdate(_contentpath);

				 glog(ZQ::common::Log::L_INFO, CLOGFMT(DataStreamExImpl, "[%s][%s]finished to notify Local folder data update"),
					 myInfo.name.c_str(), dataPPName.c_str());				 
			}
			catch (TianShanIce::InvalidParameter&)
			{
				folderExPrx->setContentName(0, myContentName);	
			}
			catch (TianShanIce::InvalidStateOfArt&ex) {
				glog(ZQ::common::Log::L_ERROR,CLOGFMT(DataStreamExImpl,"[%s]caught exception (%s)"),myInfo.name.c_str(),ex.message.c_str());
			}
			catch(::TianShanIce::NotImplemented&ex){
				glog(ZQ::common::Log::L_ERROR,CLOGFMT(DataStreamExImpl,"[%s]caught exception (%s)"),myInfo.name.c_str(),ex.message.c_str());
		}
			catch(const Ice::ObjectNotExistException&) 
			{
				glog(ZQ::common::Log::L_CRIT,CLOGFMT(DataStreamExImpl,"[%s]fail to connect to stream service"),myInfo.name.c_str());
			}
			catch (const ::Ice::Exception) 
			{
				glog(ZQ::common::Log::L_CRIT,CLOGFMT(DataStreamExImpl,"[%s]fail to connect to stream service"),myInfo.name.c_str());
			} 
		}
		try
		{
			folderExPrx->linkDataStream(myInfo.name, getThisProxy(current.adapter), 0);
		} 
		catch(ObjectExistException& )
		{
			throw ObjectExistException();
		}
	} 
	else 
		if (chType == dataMessage)
		{			
			try 
			{
				msgQueuePrx->linkDataStream(myInfo.name, 
					getThisProxy(current.adapter), 0);
			}
			catch(ObjectExistException& ) 
			{
				
				throw ObjectExistException();
			}
		} 
		else
		{
			assert(false);
			throw Ice::UnknownException(__FILE__, __LINE__);
		}

		glog( ZQ::common::Log::L_INFO,CLOGFMT(DataStreamExImpl,"[%s]Leave attach datapublishpoint (%s)"),myInfo.name.c_str(), dataPPName.c_str());
}

void
DataStreamExImpl::detachDataPublishPoint(const ::std::string& dataPPName,const Ice::Current& current)
{  
	Lock sync(*this);
	glog(ZQ::common::Log::L_INFO,CLOGFMT(DataStreamExImpl,"[%s] enter detach datapublishpoint(%s)"),myInfo.name.c_str(), dataPPName.c_str());

	DataPublishPointPrx DataPPPrx = myParent->openDataPublishPoint(dataPPName);


	std::string chType = DataPPPrx->getType();

	if (chType == dataLocalFolder || chType == dataSharedFolder) {

		FolderExPrx folderPrx = 
			FolderExPrx ::checkedCast(DataPPPrx);

		folderPrx->unlinkDataStream(myInfo.name);

	} else if (chType == dataMessage) {

		MessageQueueExPrx msgQueuePrx = 
			MessageQueueExPrx ::checkedCast(DataPPPrx);

		msgQueuePrx->unlinkDataStream(myInfo.name);
	}

	myDataPublishPoints.erase(dataPPName);

	glog(ZQ::common::Log::L_INFO,CLOGFMT(DataStreamExImpl,"[%s]detach datapublishpoint(%s) successfully"),myInfo.name.c_str(), dataPPName.c_str());
}


void
DataOnDemand::DataStreamExImpl::pause(const Ice::Current& current)
{
	throw ::TianShanIce::NotImplemented();
}

///impl interface ::TianShanIce::Application::DataStreamEx
void DataStreamExImpl::removeDataPublishPoint(const ::std::string& name,const Ice::Current& current)
{
	Lock sync(*this);
	try
	{	
		AttachedInfoDict::iterator itor =  myDataPublishPoints.find(name);
		if(itor != myDataPublishPoints.end())
				myDataPublishPoints.erase(name);
		else
		{
			glog(ZQ::common::Log::L_INFO,CLOGFMT(DataStreamExImpl,"[%s]removeChannel()can't find datapublishpoint)%s)"),myInfo.name.c_str(), name.c_str());
		}
	}
	catch (...)
	{
		
	}
}

void 
DataStreamExImpl::activate(const ::Ice::Current& current)
{
	TianShanIce::Streamer::DataOnDemand::DataStreamPrx datastreamprx;
	TianShanIce::Streamer::StreamPrx streamprx;

	if(weiwooSession)
	{
		try
		{
			TianShanIce::Streamer::StreamPrx streamproxy;
			streamproxy = weiwooSession->getStream();
			datastreamprx = TianShanIce::Streamer::DataOnDemand::DataStreamPrx::checkedCast(
				streamproxy);
			//if(myChannels.size()  >0)
            if(datastreamprx)
			{				
				if (myState == TianShanIce::Streamer::stsStreaming)
				{
					glog( ZQ::common::Log::L_INFO,CLOGFMT(DataStreamExImpl,"[%s]Dactivate() stream already play"), myInfo.name.c_str());
					//    _stream->play();
				}
				
				SessionRenewThread * pThread =  
					new SessionRenewThread(weiwooSession, myInfo.name);
				
				if(pThread)
				{
					pThread->start();
					DataOnDemand::DataPointPublisherImpl::_SessionTrdMap[myInfo.name] = pThread;
					pThread = NULL;
				}
				return;	
			}
			else
			{
				//weiwooSession->destroy();
				glog(ZQ::common::Log::L_WARNING,CLOGFMT(DataStreamExImpl,"[%s]fail to get stream proxy, retry create stream"), myInfo.name.c_str());				
				weiwooSession = NULL;
			}
		}
		catch (TianShanIce::ServerError &e)
		{
			//weiwooSession->destroy();
			glog( ZQ::common::Log::L_WARNING,CLOGFMT(DataStreamExImpl,"[%s]caught exception(%s)"), myInfo.name.c_str(),e.ice_name().c_str());
			weiwooSession = NULL;
		}
		catch(Ice::Exception &e)
		{
			//weiwooSession->destroy();
			glog( ZQ::common::Log::L_WARNING,CLOGFMT(DataStreamExImpl,"[%s]caught ice exception (%s)"), myInfo.name.c_str(),e.ice_name().c_str());
			weiwooSession = NULL;
		}
	}
	try
	{
		glog( ZQ::common::Log::L_WARNING,CLOGFMT(DataStreamExImpl, "[%s]activate()create stream"), myInfo.name.c_str());
		TianShanIce::Streamer::StreamPrx streamprx;
		TianShanIce::SRM::Resource clientResource;
		::TianShanIce::Variant var;
		::TianShanIce::StrValues strvaluesUri,strvalues;
		::TianShanIce::ValueMap valuemap;
		::TianShanIce::IValues vtints;
		std::string strurl = gDODAppServiceConfig.szDODRtspURL;
		
		clientResource.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
		clientResource.status = TianShanIce::SRM::rsRequested;
		var.bRange = false;
	    var.type = ::TianShanIce::vtStrings;
		strvaluesUri.clear();
		strvaluesUri.push_back(strurl);
		var.strs = strvaluesUri;	
		clientResource.resourceData["uri"] = var;

        weiwooSession = DataOnDemand::DataPointPublisherImpl::_sessManager->
												createSession(clientResource);

		valuemap.clear();
       	var.type = ::TianShanIce::vtInts;
		var.bRange = false;
		vtints.clear();
		vtints.push_back(gDODAppServiceConfig.lDODAppServiceGroup);
		var.ints = vtints;
		valuemap["id"]=var;
		
		weiwooSession->addResource(::TianShanIce::SRM::rtServiceGroup,
			::TianShanIce::SRM::raMandatoryNonNegotiable,
			valuemap);

		valuemap.clear();
       	var.type = ::TianShanIce::vtLongs;
		var.bRange = false;
	    var.lints.clear();
        var.lints.push_back(myInfo.totalBandwidth);
		valuemap["bandwidth"]=var;
		
		weiwooSession->addResource(::TianShanIce::SRM::rtTsDownstreamBandwidth,
			::TianShanIce::SRM::raMandatoryNonNegotiable,
			valuemap);

		var.type = ::TianShanIce::vtStrings;
		var.bRange = false;
		strvalues.clear();
		strvalues.push_back(configSpaceName);
		var.strs = strvalues;
		weiwooSession->setPrivateData(RESKEY_SPACENAME, var);

		var.type = ::TianShanIce::vtStrings;
		var.bRange = false;
		strvalues.clear();
		strvalues.push_back(myInfo.name);
		var.strs = strvalues;
        weiwooSession->setPrivateData(RESKEY_STREAMNAME, var);

		var.type = ::TianShanIce::vtStrings;
		var.bRange = false;
		strvalues.clear();
		strvalues.push_back(myInfo.destAddress);
		var.strs = strvalues;
        weiwooSession->setPrivateData(RESKEY_DESTADDRESS, var);

		var.type = ::TianShanIce::vtInts;
		var.bRange = false;
		vtints.clear();
		vtints.push_back(myInfo.groupId);
		var.ints = vtints;
		weiwooSession->setPrivateData(RESKEY_GROUPID, var);

		var.type = ::TianShanIce::vtInts;
		var.bRange = false;
		vtints.clear();
		vtints.push_back(myInfo.pmtPid);
		var.ints = vtints;

       weiwooSession->setPrivateData(RESKEY_PMTPID, var);

		var.type = ::TianShanIce::vtInts;
		var.bRange = false;
		vtints.clear();
		vtints.push_back(myInfo.totalBandwidth);
		var.ints = vtints;

		glog(ZQ::common::Log::L_INFO,CLOGFMT(DataStreamExImpl,"[%s]activate() set privateData"),myInfo.name.c_str());

		weiwooSession->setPrivateData(RESKEY_TOTALBANDWIDTH, var);

		glog(ZQ::common::Log::L_INFO,CLOGFMT(DataStreamExImpl,"[%s]activate() provision"),myInfo.name.c_str());
		weiwooSession->provision();

		glog(ZQ::common::Log::L_INFO,CLOGFMT(DataStreamExImpl,"[%s]activate() serve"),myInfo.name.c_str() );
		weiwooSession->serve();

		glog(ZQ::common::Log::L_INFO,CLOGFMT(DataStreamExImpl,"[%s]activate() renew (%d)"), myInfo.name.c_str(), gDODAppServiceConfig.lRenewtime * 1000);
		weiwooSession->renew(gDODAppServiceConfig.lRenewtime * 1000);

		std::string mySessionId = weiwooSession->getId();
		glog(ZQ::common::Log::L_INFO,CLOGFMT(DataStreamExImpl,"[%s]activate()get stream  new sessionId (%s)"),myInfo.name.c_str(), mySessionId.c_str());

		glog(ZQ::common::Log::L_INFO,CLOGFMT(DataStreamExImpl,"activate() set privateData"));

		streamprx = weiwooSession->getStream();

		datastreamprx = TianShanIce::Streamer::DataOnDemand::DataStreamPrx::checkedCast(streamprx);

		glog( ZQ::common::Log::L_INFO,CLOGFMT(DataStreamExImpl,"[%s]activate() create stream successfully"),myInfo.name.c_str());
	   }
	catch (TianShanIce::SRM::InvalidResource&ex) 
	{
		glog(ZQ::common::Log::L_WARNING,CLOGFMT(DataStreamExImpl,"activate() caught exception (%s)"),ex.message.c_str());
		throw StreamerException();
	}
	catch(TianShanIce::NotSupported&ex)
	{
		glog(ZQ::common::Log::L_WARNING,CLOGFMT(DataStreamExImpl,"activate() caught exception (%s)"),ex.message.c_str());
		throw StreamerException();
	}
	catch(TianShanIce::InvalidParameter&ex)
	{
		glog(ZQ::common::Log::L_WARNING,CLOGFMT(DataStreamExImpl,"activate() caught exception (%s)"),ex.message.c_str());
		throw StreamerException();
	}
	catch(TianShanIce::ServerError&ex)
	{
		glog(ZQ::common::Log::L_WARNING,CLOGFMT(DataStreamExImpl,"activate() caught exception (%s)"),ex.message.c_str());
		throw StreamerException();
	}
	
	catch(Ice::Exception& ex)
	{
		
		glog(ZQ::common::Log::L_WARNING,CLOGFMT(DataStreamExImpl,"activate() caught ice exception (%s)"),ex.ice_name().c_str());
		throw StreamerException();
	}

	try
	{
		rebuildMuxItem(datastreamprx);
	}
	catch (DataOnDemand::StreamerException& )
	{	
		try
		{
			datastreamprx->destroy();
			weiwooSession->destroy();	
		}
		catch (Ice::Exception&ex) 
		{
			glog(ZQ::common::Log::L_WARNING,CLOGFMT(DataStreamExImpl,"[%s]activate()fail to rebuildMuxItem, caught unknown exception (%s)"),myInfo.name.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			glog(ZQ::common::Log::L_WARNING,CLOGFMT(DataStreamExImpl,"[%s]activate()fail to rebuildMuxItem, caught unknown exception (%d)"),myInfo.name.c_str(), GetLastError());
		}
		throw StreamerException();	
	}
	try 
	{
		if (myState == TianShanIce::Streamer::stsStreaming)
		{
			bool bplay = datastreamprx->play();
			if(bplay)
			{
				SessionRenewThread * pThread =  
					new SessionRenewThread(weiwooSession, myInfo.name);
				
				if(pThread)
				{
					pThread->start();
					DataOnDemand::DataPointPublisherImpl::_SessionTrdMap[myInfo.name] = pThread;
					pThread = NULL;
				}
			}
			else
			{	
				try
				{
					datastreamprx->destroy();
					weiwooSession->destroy();	
				}
				catch (Ice::Exception&ex) 
				{
					glog(ZQ::common::Log::L_WARNING,CLOGFMT(DataStreamExImpl,"[%s]activate() fail to play stream, caught ice exception (%s)"),myInfo.name.c_str(), ex.ice_name().c_str());
				}
				catch(...)
				{
					glog(ZQ::common::Log::L_WARNING,CLOGFMT(DataStreamExImpl, "[%s]activate()fail to play stream caught unknown excetpion(%d)"),myInfo.name.c_str(), GetLastError());
				}				
				Beep(700, 1000);				
				throw StreamerException();
			}
			notifyMuxItem();
		}				
	}
	catch (Ice::Exception& )
	{		
		throw StreamerException();				
	}
}
///impl class ::TianShanIce::Application::DataStreamExImpl
inline TianShanIce::Application::DataOnDemand::DataStreamExPrx 
DataStreamExImpl::getThisProxy(
							   const Ice::ObjectAdapterPtr& adapter)
{
	if (_thisPrx == NULL) {
		_thisPrx = NameToDataStream(adapter)(myInfo.name);
	}

	return _thisPrx;
}
void 
DataStreamExImpl::rebuildMuxItem(TianShanIce::Streamer::DataOnDemand::DataStreamPrx& strm)
{
	glog( ZQ::common::Log::L_INFO,CLOGFMT(DataStreamExImpl,"[%s]enter rebuildMuxItem()"),myInfo.name.c_str());
	TianShanIce::Application::DataOnDemand::AttachedInfoDict::iterator it;
	TianShanIce::Application::DataOnDemand::DataPublishPointInfo dataPPInfo;
	TianShanIce::Streamer::DataOnDemand::MuxItemPrx muxItem;
	std::string myContentName = "";

	for (it = myDataPublishPoints.begin(); it != myDataPublishPoints.end(); it ++) 
	{
		TianShanIce::Application::DataOnDemand::DataPublishPointPrx dataPPPrx;
		TianShanIce::Application::DataOnDemand::DataAttachInfo& ai = it->second;
		dataPPPrx = ai.datapp;

		glog( ZQ::common::Log::L_INFO,CLOGFMT(DataStreamExImpl,"[%s]rebuildMuxItem()datapublishpoint (%s)"), myInfo.name.c_str(), ai.dataPublishPointName.c_str());

		TianShanIce::Application::DataOnDemand::FolderExPrx folderExPrx;
		TianShanIce::Application::DataOnDemand::MessageQueueExPrx msgQueueExPrx;

		TianShanIce::Streamer::DataOnDemand::CacheType ctype;
		std::string addr;

		std::string chType = dataPPPrx->getType();

		if (chType == TianShanIce::Application::DataOnDemand::dataLocalFolder ||
			chType == TianShanIce::Application::DataOnDemand::dataSharedFolder) {

			folderExPrx = TianShanIce::Application::DataOnDemand::FolderExPrx::checkedCast(dataPPPrx);
			folderExPrx->getCacheInfo(ctype, addr);

		} else {

			msgQueueExPrx = TianShanIce::Application::DataOnDemand::MessageQueueExPrx::checkedCast(dataPPPrx);
			msgQueueExPrx->getCacheInfo(ctype, addr);
		}

		dataPPInfo = dataPPPrx->getInfo();

		TianShanIce::Streamer::DataOnDemand::MuxItemInfo muxItemInfo;
		muxItemInfo.name = dataPPInfo.name;
		muxItemInfo.streamId = dataPPInfo.streamId;
		muxItemInfo.streamType = dataPPInfo.streamType;
		muxItemInfo.bandWidth = ai.minBitRate;
		muxItemInfo.tag = dataPPInfo.tag;
		muxItemInfo.expiration = 0;
		muxItemInfo.repeatTime = ai.repeatTime;

		muxItemInfo.ctype = ctype;
		muxItemInfo.cacheAddr = addr;
		muxItemInfo.encryptMode = dataPPInfo.encrypt;
		muxItemInfo.subchannelCount = dataPPInfo.subchannelCount;
		try
		{
			muxItem = strm->createMuxItem(muxItemInfo);
		}
		catch (TianShanIce::Streamer::DataOnDemand::DataStreamError &ex)
		{
			glog( ZQ::common::Log::L_ERROR,CLOGFMT(DataStreamExImpl,"rebuildMuxItem() caught DataOnDemand DataStreamError '%s'"),ex.message.c_str());
			throw StreamerException();
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataStreamExImpl,"rebuildMuxItem() caught TianShanIce InvalidParameter (%s)"),ex.message.c_str());	
			throw StreamerException();
		}
		catch(Ice::Exception &e)
		{
			glog( ZQ::common::Log::L_ERROR,CLOGFMT(DataStreamExImpl,"rebuildMuxItem() caught ice exception(%s)"),e.ice_name().c_str());
			throw StreamerException();
		}
		glog( ZQ::common::Log::L_INFO,CLOGFMT(DataStreamExImpl,"[%s]rebuildMuxItem()datapublishpoint [%s], rebuild muxitem successfully"),myInfo.name.c_str(), ai.dataPublishPointName.c_str());
	}
	glog( ZQ::common::Log::L_INFO, CLOGFMT(DataStreamExImpl,"[%s]Leave rebuildMuxItem()"),myInfo.name.c_str());
}

void DataStreamExImpl::notifyMuxItem()
{
	TianShanIce::Application::DataOnDemand::AttachedInfoDict::iterator it;
	TianShanIce::Streamer::DataOnDemand::MuxItemPrx muxItem;
	std::string myContentName = "";
	std::string _contentpath;

	TianShanIce::Streamer::DataOnDemand::DataStreamPrx mystream = getStreamPrx();
	if(mystream == NULL)
	{
		throw StreamerException();
	}

	for (it = myDataPublishPoints.begin(); it != myDataPublishPoints.end(); it ++)
	{		
		TianShanIce::Application::DataOnDemand::DataPublishPointPrx dataPPPrx;
		TianShanIce::Application::DataOnDemand::DataAttachInfo& ai = it->second;
		dataPPPrx = ai.datapp;
		
		FolderExPrx folderexPrx;
		
		std::string chType = dataPPPrx->getType();

		folderexPrx = FolderExPrx::checkedCast(dataPPPrx);

		if (chType == dataSharedFolder)
		{	
			try
			{
				myContentName = folderexPrx->getContentName(myInfo.groupId);
			}
			catch (TianShanIce::InvalidParameter&ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataStreamExImpl,"[%s]notifyMuxItem()caught exception (%s)"),myInfo.name.c_str(), ex.message.c_str());
				continue;
			}
			catch (Ice::Exception & ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataStreamExImpl,"[%s]notifyMuxItem()caught ice exception (%s)"),myInfo.name.c_str(), ex.ice_name().c_str());

				continue;
			}
		}
		else
			if(chType == dataLocalFolder)
			{
				try
				{
					myContentName = folderexPrx->getContentName(0);
				}
				catch (TianShanIce::InvalidParameter&ex)
				{
					glog(ZQ::common::Log::L_ERROR,CLOGFMT(DataStreamExImpl,"[%s]notifyMuxItem() caught  excetpion (%s)"),myInfo.name.c_str(), ex.message.c_str());
					continue;
				}
			}
			else
			{
				continue;
			}
			
			if(!myContentName.empty())
			{
				::TianShanIce::Storage::ContentPrx contentprx;
				
				try
				{
					contentprx=  DataOnDemand::DataPointPublisherImpl::_contentStroe->
						openContent(myContentName,::TianShanIce::Storage::ctDODTS ,false);
					
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
					TianShanIce::Streamer::DataOnDemand::MuxItemPrx muxitemprx ;
					muxitemprx = mystream->getMuxItem((*it).first);
					
					glog(ZQ::common::Log::L_INFO, CLOGFMT(DataStreamExImpl,"[%s][%s]notifyMuxItem()]notify full data update, content file path[%s]"), myInfo.name.c_str(), (*it).first.c_str(), _contentpath.c_str());

					muxitemprx->notifyFullUpdate(_contentpath);
					
					glog(ZQ::common::Log::L_INFO, CLOGFMT(DataStreamExImpl,"[%s][%s]finished to notify full data update"),myInfo.name.c_str(),(*it).first.c_str());					
				}
				catch (::TianShanIce::InvalidParameter& ex){	
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataStreamExImpl, "[%s]failed to notify full data update caught exception [%s]"), myInfo.name.c_str(), ex.message.c_str());

				}
				catch (::TianShanIce::InvalidStateOfArt& ex) {
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataStreamExImpl, "[%s]failed to notify full data update caught exception [%s]"), myInfo.name.c_str(), ex.message.c_str());

				}
				catch(::TianShanIce::NotImplemented& ex){
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataStreamExImpl, "[%s]failed to notify full data update caught exception [%s]"), myInfo.name.c_str(), ex.message.c_str());
				}
				catch(const Ice::ObjectNotExistException&) 
				{
					glog(ZQ::common::Log::L_CRIT, CLOGFMT(DataStreamExImpl,"[%s]fail to connect to streamer services"));
				}
				catch (const ::Ice::Exception & ex) 
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataStreamExImpl, "[%s]failed to notify full data update caught ice exception [%s]"), myInfo.name.c_str(), ex.ice_name().c_str());
				} 
			}
		}			
}


TianShanIce::Streamer::DataOnDemand::DataStreamPrx
DataStreamExImpl::getStreamPrx()
{
	TianShanIce::Streamer::DataOnDemand::DataStreamPrx stream = NULL;
	try
	{	
		if(weiwooSession)
		{		
			glog(ZQ::common::Log::L_INFO, CLOGFMT(DataStreamExImpl, "[%s]get stream proxy"), myInfo.name.c_str());	
            TianShanIce::Streamer::StreamPrx streamprx;
			streamprx = weiwooSession->getStream();
			stream = TianShanIce::Streamer::DataOnDemand::DataStreamPrx::checkedCast(streamprx);
		}
		else
			stream = NULL;
	}
	catch(Ice::Exception&ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataStreamExImpl,"[%s]get stream proxy caught ice exception (%s)"), myInfo.name.c_str(), ex.ice_name().c_str());
		stream = NULL;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataStreamExImpl,"[%s] fail to  get stream proxy"), myInfo.name.c_str());
		stream = NULL;
	}
	return stream;
}

} /// end namespace DataOnDemand {
} /// end namespace Application 
} /// end namespace TianshanIce 