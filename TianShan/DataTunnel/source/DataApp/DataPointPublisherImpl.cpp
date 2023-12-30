#include "stdafx.h"
#include "DataAppImpl.h"
#include "DataPointPublisherImpl.h"
#include "FolderExImpl.h"
#include "MessageQueueExImpl.h"
#include "DataStreamExImpl.h"
#include "global.h"
#include "..\common\Reskey.h"
#include <algorithm>
#include "stdlib.h"

#define DATAPOINTPUBLISHIMPL "DataPointPublisherImpl "
//////////////////////////////////////////////////////////////////////////

Ice::ObjectAdapterPtr TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_adapter;
::Freeze::EvictorPtr TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_evictor;
TianShanIce::Application::DataOnDemand::DataTunnelServicePtr TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_dodappservice;
TianShanIce::SRM::SessionManagerPrx TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_sessManager;
TianShanIce::Storage::ContentStorePrx  TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_contentStroe;
SessionTrdMap	TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_SessionTrdMap;
MessageManage*	TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_messagemanage;

Ice::CommunicatorPtr TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_ic;
	
//////////////////////////////////////////////////////////////////////////
namespace TianShanIce {
namespace Application {
namespace DataOnDemand {
DataPointPublisherImpl::DataPointPublisherImpl()
{
	_thisPrx = NULL;
}

DataPointPublisherImpl::~DataPointPublisherImpl()
{

}

bool DataPointPublisherImpl::init()
{	
	return true;
}

FolderPrx
DataPointPublisherImpl::createLocalFolderPublishPoint(const ::std::string& name,
													  const ::TianShanIce::Application::DataOnDemand::DataPublishPointInfo& info,
													  const ::std::string& path,
													  const ::std::string& desc,
													  const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]enter create local folder datapublishpoint"), name.c_str());

	FolderExPrx localfolderDataPP = FolderExPrx::uncheckedCast(
		NameToDataPublishPoint(current.adapter)(name));
    TianShanIce::Properties props;
	FolderExImpl* folderDataPP = new FolderExImpl;
    
	folderDataPP->myInfo = info;
	folderDataPP->ident = createDataPublishPointIdentity(name);
	folderDataPP->myParent = getThisProxy(current.adapter);
	folderDataPP->type = TianShanIce::Application::DataOnDemand::dataLocalFolder;
	folderDataPP->desc =  desc;
	
	props = folderDataPP->getProperties(Ice::Current());	
	props["Path"] =  path;	
	folderDataPP->setProperties(props,Ice::Current());

	if (!folderDataPP->init()) {
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to initialize local folder datapublishpoint"),name.c_str());
		delete folderDataPP;
		return NULL;
	}
	glog(ZQ::common::Log::L_INFO,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]initialize local folder datapublishpoint successfully"),name.c_str());
	Ice::Int queueSize = _evictor->getSize();
	_evictor->setSize(0);

	_evictor->add(folderDataPP, 
		DataOnDemand::createDataPublishPointIdentity(name));
		
	_evictor->setSize(queueSize);	
	
	localFolderDatas.insert(
		TianShanIce::Application::DataOnDemand::FolderDataDict::value_type(name, localfolderDataPP));
	try
	{	
		ActiveDataManager.create(TianShanIce::Application::DataOnDemand::DataPublishPointPrx::checkedCast(localfolderDataPP));
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to create local folder datapublishpoint caught excetption[%s]"),name.c_str(), ex.ice_name().c_str());
		delete folderDataPP;
		return NULL;
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]create local folder datapublishpoint successfully"),name.c_str());
    return localfolderDataPP;
}

FolderPrx
DataPointPublisherImpl::createShareFolderPublishPoint(
	                const ::std::string& name,
					const ::TianShanIce::Application::DataOnDemand::DataPublishPointInfo& info,
					 const ::std::string& desc,
					const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]enter create share folder datapublishpoint"), name.c_str());

	FolderExPrx sharefolderdataPP = FolderExPrx::uncheckedCast(
		NameToDataPublishPoint(current.adapter)(name));

	FolderExImpl* folderDataPP = new FolderExImpl;

	folderDataPP->myInfo = info;
	folderDataPP->ident = createDataPublishPointIdentity(name);
	folderDataPP->myParent = getThisProxy(current.adapter);
	folderDataPP->type = TianShanIce::Application::DataOnDemand::dataSharedFolder;
	folderDataPP->desc =  desc;

	if (!folderDataPP->init()) {
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to initialize share folder datapublishpoint"), name.c_str());
		delete folderDataPP;
		return NULL;
	}
	glog(ZQ::common::Log::L_INFO,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]initialize share folder datapublishpoint successfully"),name.c_str());

	Ice::Int queueSize = _evictor->getSize();
	_evictor->setSize(0);

	_evictor->add(folderDataPP, DataOnDemand::createDataPublishPointIdentity(name));

	_evictor->setSize(queueSize);
	
	shareFolderDatas.insert(
		TianShanIce::Application::DataOnDemand::FolderDataDict::value_type(name, sharefolderdataPP));

	try
	{	
		ActiveDataManager.create(
			TianShanIce::Application::DataOnDemand::DataPublishPointPrx::checkedCast(sharefolderdataPP));
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to create share folder datapublishpoint caught excetption[%s]"),name.c_str(), ex.ice_name().c_str());
		delete folderDataPP;
		return NULL;
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]create share folder datapublishpoint successfully"),name.c_str());

    return sharefolderdataPP;
}

MessageQueuePrx
DataPointPublisherImpl::createMessageQueue(const ::std::string& name,
					 const ::TianShanIce::Application::DataOnDemand::DataPublishPointInfo& info,
					  const ::std::string& desc,
					 const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]enter create message queue datapublishpoint"), name.c_str());

	MessageQueueExPrx msgdatapp = MessageQueueExPrx::uncheckedCast(
		                         NameToDataPublishPoint(current.adapter)(name));

	MessageQueueExImpl* msgQueue = new MessageQueueExImpl;

	msgQueue->myInfo = info;
	msgQueue->ident = createDataPublishPointIdentity(name);
	msgQueue->myParent = getThisProxy(current.adapter);
	msgQueue->type = TianShanIce::Application::DataOnDemand::dataMessage;
	msgQueue->desc =  desc;

	if (!msgQueue->init()) {
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to initialize message queue datapublishpoint"), name.c_str());
		delete msgQueue;
		return NULL;
	}
	glog(ZQ::common::Log::L_INFO,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]initialize message queue datapublishpoint successfully"),name.c_str());

	Ice::Int queueSize = _evictor->getSize();
	_evictor->setSize(0);

	_evictor->add(msgQueue, DataOnDemand::createDataPublishPointIdentity(name));

	_evictor->setSize(queueSize);
	
	msgQueues.insert(TianShanIce::Application::DataOnDemand::MsgQueueDict::value_type(name, msgdatapp));

	try
	{	
		ActiveDataManager.create( TianShanIce::Application::DataOnDemand::DataPublishPointPrx::checkedCast(msgdatapp));
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to message queue folder datapublishpoint caught excetption[%s]"),name.c_str(), ex.ice_name().c_str());
		return NULL;  
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]create message queue datapublishpoint successfully"),name.c_str());

	return msgdatapp;
}

DataPublishPointPrx
DataPointPublisherImpl::openDataPublishPoint(const ::std::string& name,const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"[%s] open datapublishpoint"),name.c_str());

	{
		TianShanIce::Application::DataOnDemand::MsgQueueDict::iterator it = msgQueues.find(name);
		if (it != msgQueues.end())
		{	
			return DataPublishPointPrx::checkedCast(it->second);
		}
	}
	{
		TianShanIce::Application::DataOnDemand::FolderDataDict::iterator it = shareFolderDatas.find(name);
		if (it != shareFolderDatas.end())
		{	
			return DataPublishPointPrx::checkedCast(it->second);
		}
	}

	{
		FolderDataDict::iterator it = localFolderDatas.find(name);
		if (it != localFolderDatas.end())
		{	
			return DataPublishPointPrx::checkedCast(it->second);
		}
	}

	Ice::Identity ident = createDataPublishPointIdentity(name);
	if(_evictor->hasObject(ident))
	{
		_evictor->remove(ident);
	}
	glog(ZQ::common::Log::L_WARNING, CLOGFMT(DATAPOINTPUBLISHIMPL,"[%s]failed to open datapublishpoint, may be it not exists"),name.c_str());

	throw Ice::ObjectNotExistException(__FILE__, __LINE__);

	return NULL;
}



DataStreamPrx
DataPointPublisherImpl::broadcast(const ::std::string& destName,
				   const ::TianShanIce::SRM::ResourceMap& resourceRequirement,
				   const ::TianShanIce::Properties& props,
				   const ::std::string& desc,
				   const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO,  CLOGFMT(DATAPOINTPUBLISHIMPL,"[%s]enter create broadcast datastream"),destName.c_str());

	TianShanIce::Application::DataOnDemand::DataStreamInfo info;  
	info.name = destName;
	
	TianShanIce::ValueMap::const_iterator itorVmap;
	TianShanIce::SRM::ResourceMap::const_iterator itorRsMap;
	itorRsMap = resourceRequirement.find(TianShanIce::SRM::ResourceType::rtIP);
	if(itorRsMap != resourceRequirement.end())
	{
		int protocol;
		int port;
		std::string address;
		char addresstemp[32];
		const TianShanIce::SRM::Resource& resource = itorRsMap->second;
		itorVmap = resource.resourceData.find("protocol");
		if(itorVmap != resource.resourceData.end())
		{
	       protocol =  itorVmap->second.ints[0];
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s] failed to create datastream with error missing parameter 'protocol'"),destName.c_str());
			throw TianShanIce::InvalidParameter("DataTunnelApp",1,"failed to create datastream with error missing parameter 'protocol'");
		}

		itorVmap = resource.resourceData.find("destAddr");
		if(itorVmap != resource.resourceData.end())
		{
			address =  itorVmap->second.strs[0];
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to create datastream with error missing parameter 'destAddr'"),destName.c_str());
			throw TianShanIce::InvalidParameter("DataTunnelApp",1,"failed to create datastream with error missing parameter 'destAddr'");
		}


		itorVmap = resource.resourceData.find("destPort");
		if(itorVmap != resource.resourceData.end())
		{
			port =  itorVmap->second.ints[0];
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to create datastream with error missing parameter 'port'"),destName.c_str());
			throw TianShanIce::InvalidParameter("DataTunnelApp",1,"failed to create datastream with error missing parameter'port'");
		}

		if(protocol == 0)
		{
			sprintf(addresstemp, "TCP:%s:%d", address.c_str(), port);
			info.destAddress = addresstemp;
		}
		else if(protocol ==1)
		{
			sprintf(addresstemp, "UDP:%s:%d", address.c_str(), port);
			info.destAddress = addresstemp;
		}
		else if(protocol ==2)
		{
			sprintf(addresstemp, "MULITCAST:%s:%d", address.c_str(), port);
			info.destAddress = addresstemp;
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to create datastream with unknown 'protocol'"),destName.c_str());
			throw TianShanIce::InvalidParameter("DataTunnelApp",1,"failed to create datastream with error unknown 'protocol'");

		}
	}	
	else
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL, 
			"[%s]failed to create stream with error missing parameter 'IP'"),destName.c_str());
		throw TianShanIce::InvalidParameter("DataTunnelApp",1,"failed to create datastream with error missing parameter 'IP'");
	}

	itorRsMap = resourceRequirement.find(TianShanIce::SRM::ResourceType::rtServiceGroup);
	if(itorRsMap != resourceRequirement.end())
	{
		const TianShanIce::SRM::Resource& resource = itorRsMap->second;
		itorVmap = resource.resourceData.find("id");
		if(itorVmap != resource.resourceData.end())
		{
			info.groupId  = itorVmap->second.ints[0];
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to create datastream with error missing parameter 'Service groupId'"),destName.c_str());
			throw TianShanIce::InvalidParameter("DataTunnelApp",1,"failed to create datastream with error missing parameter 'Service groupId'");		
		}		
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to create datastream with error missing resource 'ServiceGroup'"),destName.c_str());
		throw TianShanIce::InvalidParameter("DataTunnelApp",1,"failed to create datastream with error missing resource 'ServiceGroup'");		
	}

	itorRsMap = resourceRequirement.find(TianShanIce::SRM::ResourceType::rtTsDownstreamBandwidth);
	if(itorRsMap != resourceRequirement.end())
	{
		const TianShanIce::SRM::Resource& resource = itorRsMap->second;
		itorVmap = resource.resourceData.find("bandwidth");
		if(itorVmap != resource.resourceData.end())
		{
			info.totalBandwidth  = itorVmap->second.ints[0];
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to create datastream with error missing parameter 'Bandwidth'"),destName.c_str());
			throw TianShanIce::InvalidParameter("DataTunnelApp",1,"failed to create datastream with error missing parameter 'Bandwidth'");		
		}
	}	
	else
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to create datastream with error missing resource 'TsDownstreamBandwidth'"),destName.c_str());
		throw TianShanIce::InvalidParameter("DataTunnelApp",1,"failed to create datastream with error missing resource 'TsDownstreamBandwidth'");		
	}

	itorRsMap = resourceRequirement.find(TianShanIce::SRM::ResourceType::rtMpegProgram);
	if(itorRsMap != resourceRequirement.end())
	{
		const TianShanIce::SRM::Resource& resource = itorRsMap->second;
		itorVmap = resource.resourceData.find("PmtPid");
		if(itorVmap != resource.resourceData.end())
		{
			info.pmtPid  = itorVmap->second.ints[0];
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to create datastream with error missing parameter 'PmtPid'"),destName.c_str());
			throw TianShanIce::InvalidParameter("DataTunnelApp",1,"failed to create datastream with error missing parameter 'PmtPid'")	;	

		}
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to create datastream with error missing resource 'MpegProgram'"),destName.c_str());
		throw TianShanIce::InvalidParameter("DataTunnelApp",1,"failed to create datastream with error missing resource 'MpegProgram'");		
	}

	TianShanIce::Application::DataOnDemand::DataStreamExPrx destPrx = 
			TianShanIce::Application::DataOnDemand::DataStreamExPrx::uncheckedCast(
		NameToDataStream(current.adapter)(destName));

	TianShanIce::Application::DataOnDemand::DataStreamExImpl* destObj = new TianShanIce::Application::DataOnDemand::DataStreamExImpl;
	destObj->myInfo = info;
	destObj->resources = resourceRequirement;
	destObj->ident = createDataStreamIdentity(destName);
	destObj->myParent = getThisProxy(current.adapter);
	destObj->properties = props;
	destObj->desc = desc;

	try
	{	
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

		TianShanIce::SRM::SessionPrx sessionprx = _sessManager->createSession(clientResource);

		valuemap.clear();
       	var.type = ::TianShanIce::vtInts;
		var.bRange = false;
		vtints.clear();
		vtints.push_back(gDODAppServiceConfig.lDODAppServiceGroup);
		var.ints = vtints;
		valuemap["id"]=var;

		sessionprx->addResource(::TianShanIce::SRM::rtServiceGroup,::TianShanIce::SRM::raMandatoryNonNegotiable,valuemap);
	
		valuemap.clear();
       	var.type = ::TianShanIce::vtLongs;
		var.bRange = false;
	    var.lints.clear();
        var.lints.push_back(info.totalBandwidth);
		valuemap["bandwidth"]=var;
		
		sessionprx->addResource(::TianShanIce::SRM::rtTsDownstreamBandwidth,::TianShanIce::SRM::raMandatoryNonNegotiable,valuemap);

		var.type = ::TianShanIce::vtStrings;
		var.bRange = false;
		strvalues.clear();
		strvalues.push_back(configSpaceName);
		var.strs = strvalues;
		sessionprx->setPrivateData(RESKEY_SPACENAME, var);

		var.type = ::TianShanIce::vtStrings;
		var.bRange = false;
		strvalues.clear();
		strvalues.push_back(info.name);
		var.strs = strvalues;
        sessionprx->setPrivateData(RESKEY_STREAMNAME, var);

		var.type = ::TianShanIce::vtStrings;
		var.bRange = false;
		strvalues.clear();
		strvalues.push_back(info.destAddress);
		var.strs = strvalues;
        sessionprx->setPrivateData(RESKEY_DESTADDRESS, var);

		var.type = ::TianShanIce::vtInts;
		var.bRange = false;
		vtints.clear();
		vtints.push_back(info.groupId);
		var.ints = vtints;
		sessionprx->setPrivateData(RESKEY_GROUPID, var);

		var.type = ::TianShanIce::vtInts;
		var.bRange = false;
		vtints.clear();
		vtints.push_back(info.pmtPid);
		var.ints = vtints;
        sessionprx->setPrivateData(RESKEY_PMTPID, var);

		var.type = ::TianShanIce::vtInts;
		var.bRange = false;
		vtints.clear();
		vtints.push_back(info.totalBandwidth);
		var.ints = vtints;
		glog(ZQ::common::Log::L_INFO,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s] set privateData"), destName.c_str());
        sessionprx->setPrivateData(RESKEY_TOTALBANDWIDTH, var);
	
		glog(ZQ::common::Log::L_INFO,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s] provision"), destName.c_str());
		sessionprx->provision();

		glog(ZQ::common::Log::L_INFO,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s] serve"), destName.c_str());
		sessionprx->serve();

		glog(ZQ::common::Log::L_INFO,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s] renew (%d)"), destName.c_str() ,gDODAppServiceConfig.lRenewtime * 1000);
		sessionprx->renew(gDODAppServiceConfig.lRenewtime * 1000);


		destObj->weiwooSession = sessionprx;
		glog(ZQ::common::Log::L_INFO,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]get stream proxy"),destName.c_str());

		glog(ZQ::common::Log::L_INFO,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]set privateData"), destName.c_str());
		streamprx = sessionprx->getStream();
		destObj->myState = streamprx->getCurrentState();

/*		glog(ZQ::common::Log::L_INFO,
			"DataPointPublisherImpl::createDestination() checkedCast");
		destObj->_stream = DataOnDemand::DataStreamPrx::checkedCast(streamprx);*/
	}
	catch (TianShanIce::SRM::InvalidResource&ex) 
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to create datastream caught exception[%s]"), destName.c_str(), ex.message.c_str());
		return NULL;
	}
	catch(TianShanIce::NotSupported&ex)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to create datastream caught exception[%s]"), destName.c_str(), ex.message.c_str());
		return NULL;
	}
	catch(TianShanIce::InvalidParameter&ex)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to create datastream caught exception[%s]"), destName.c_str(), ex.message.c_str());
		return NULL;
	}
	catch(TianShanIce::ServerError&ex)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to create datastream caught exception[%s]"), destName.c_str(), ex.message.c_str());
		return NULL;
	}
	catch (const ::Ice::Exception & ex)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to create datastream caught exception[%s]"), destName.c_str(), ex.ice_name().c_str());
		return NULL;
	}

	if (!destObj->init()) {
		glog(ZQ::common::Log::L_INFO,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]failed to init datastream"), destName.c_str());
		delete destObj;
		return NULL;
	}

	glog(ZQ::common::Log::L_INFO,CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]datastream init successfully"), destName.c_str());

	Ice::Int queueSize = _evictor->getSize();
	_evictor->setSize(0);

	_evictor->add(destObj, 
		DataOnDemand::createDataStreamIdentity(destName));
	
	_evictor->setSize(queueSize);
	
	dataStreams.insert(TianShanIce::Application::DataOnDemand::DataStreamDict::value_type(destName, 
		destPrx));

	glog(ZQ::common::Log::L_INFO,  CLOGFMT(DATAPOINTPUBLISHIMPL,"[%s]leave create broadcast datastream"),destName.c_str());
    return destPrx;
}

DataStreamPrx
DataPointPublisherImpl::openDataStream(const ::std::string& name,const Ice::Current& current)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(DATAPOINTPUBLISHIMPL, "[%s]open datastream"),name.c_str());

	DataStreamDict::iterator it = dataStreams.find(name);
	if (it == dataStreams.end())	
	{
		Ice::Identity  ident = createDataStreamIdentity(name);
		if(_evictor->hasObject(ident))
		{
			_evictor->remove(ident);
		}
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(DATAPOINTPUBLISHIMPL,"[%s]failed to open datastream, may be it not exists"),name.c_str());
		throw Ice::ObjectNotExistException(__FILE__, __LINE__);
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"[%s]leave open datastream"),name.c_str());
	return it->second;
}

DataPublishPointInfos
DataPointPublisherImpl::listDataPublishPoints(const ::std::string& searchFor,const Ice::Current& current)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(DATAPOINTPUBLISHIMPL,"enter list datapublishpoints"));
	DataPublishPointInfos result;
	try	{
		MsgQueueDict::const_iterator it;

		for (it = msgQueues.begin(); 
			it != msgQueues.end(); it ++) {
				result.push_back(it->second->getInfo());
		}
		FolderDataDict::const_iterator itor;

		for (itor = shareFolderDatas.begin(); 
			itor != shareFolderDatas.end(); itor ++) {
				result.push_back(itor->second->getInfo());
		}

		for (itor = localFolderDatas.begin(); 
			itor != localFolderDatas.end(); itor ++) {
				result.push_back(itor->second->getInfo());
		}

	} catch(...) {
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"failed to list datapublishpoints caught exception (%d)"),GetLastError());
		throw Ice::ObjectNotExistException(__FILE__, __LINE__);
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"leave list datapublishpoints"));
    return result;
}

DataStreamInfos
DataPointPublisherImpl::listDataStreams(const ::std::string& searchFor,const Ice::Current& current)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(DATAPOINTPUBLISHIMPL,"enter list datastreams"));
    DataStreamInfos result;
	DataStreamDict::const_iterator it;
	result.clear();

	try
	{
		for (it = dataStreams.begin(); it != dataStreams.end(); it ++)
		{
			result.push_back(it->second->getInfo());
		}	
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"failed to list datastreams caught exception(%d)"),GetLastError());
		throw Ice::ObjectNotExistException(__FILE__, __LINE__);
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"leave list datastreams"));
    return result;
}

void
DataPointPublisherImpl::removeDataStream(const ::std::string& name,const Ice::Current& current)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(DATAPOINTPUBLISHIMPL,"[%s]remove datastream"),name.c_str());
    
	try {	
		Ice::Identity ident;
		TianShanIce::Application::DataOnDemand::DataStreamDict::iterator itor;
		itor = dataStreams.find(name);
		if(itor != dataStreams.end())
		{
			dataStreams.erase(name);

			ident = createDataStreamIdentity(name);

			if(_evictor->hasObject(ident))
			{
				_evictor->remove(ident);
			}
		}
		else
		{
			glog(ZQ::common::Log::L_WARNING, CLOGFMT(DATAPOINTPUBLISHIMPL,"failed to get datastream"),name.c_str());
		}

	} catch (const ::Ice::Exception & ex) {

		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"failed to remove datastream caught exception [%s]"),ex.ice_name().c_str());

	} catch (...) {

		glog(ZQ::common::Log::L_ERROR,"failed to remove datastream caught exception (%d)",GetLastError());
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"[%s]leave remove datastream"),name.c_str());
}

void
DataPointPublisherImpl::removeDataPublishPoint(const ::std::string& name,const Ice::Current& current)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(DATAPOINTPUBLISHIMPL,"[%s]enter remove DataPublishPoint"),name.c_str());

	Ice::Identity ident;

	try	{				
			msgQueues.erase(name);
			shareFolderDatas.erase(name);
			localFolderDatas.erase(name);
				
			ident = createDataPublishPointIdentity(name);
				
			if(_evictor->hasObject(ident))
			{
				_evictor->remove(ident);
			}	
			_messagemanage->removeDataPublishPointMessage(name);
	} 
	catch (const ::Ice::Exception & ex) {

		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"failed to remove DataPublishPoint caught exception [%s]"),ex.ice_name().c_str());

	} catch (...) {

		glog(ZQ::common::Log::L_ERROR,"failed to remove DataPublishPoint caught exception (%d)",GetLastError());
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"[%s]leave remove DataPublishPoint"),name.c_str());
}
void
::TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::OnDataEvent_async(const ::TianShanIce::Application::DataOnDemand::AMD_DataPointPublisher_OnDataEventPtr& OnDataEventCB,
																				 ::TianShanIce::Application::DataOnDemand::DataEvent event,
																				 const ::TianShanIce::Properties& params,
																				 const Ice::Current& current)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(DATAPOINTPUBLISHIMPL,"enter OnDataEvent_async()"));
	switch(event)
	{
	case ::TianShanIce::Application::DataOnDemand::onFullUpdate:
		onFolderFullUpdate(params);
		break;
	case TianShanIce::Application::DataOnDemand::onPartlyUpdate:
		onFolderPartlyUpdate(params);
		break;
	case TianShanIce::Application::DataOnDemand::onFolderDeleted:
		onFolderDeleted(params);
		break;
	case TianShanIce::Application::DataOnDemand::onFileAdded:
		onFileAdded(params);
		break;
	case TianShanIce::Application::DataOnDemand::onFileModified:
		onFileModified(params);
		break;
	case TianShanIce::Application::DataOnDemand::onFileDeleted:
		onFileDeleted(params);
		break;
	case TianShanIce::Application::DataOnDemand::onMessageAdded:
		onMessageAdded(params);
		break;
	case TianShanIce::Application::DataOnDemand::onMessageDeleted:
        onMessageDeleted(params);
		break;
	default:
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"unknown DataEvent type(%d)"), event);
		break;		
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"leave OnDataEvent_async()"));

	OnDataEventCB->ice_response();
}
void
DataPointPublisherImpl::onFolderFullUpdate(const ::TianShanIce::Properties&params)
{
	glog( ZQ::common::Log::L_DEBUG, CLOGFMT(DATAPOINTPUBLISHIMPL,"enter onFolderFullUpdate()"));

	int groupId;
	int dataType;
    ::std::string rootPath;
	bool clear;
	int verNumber;

	TianShanIce::Properties::const_iterator itor;

	itor = params.find(SHAREFOLDER_GROUPID);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_GROUPID);
		return;
	}
	groupId = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_DATATYPE);
	if(itor == params.end())
	{		
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_DATATYPE);
		return;
	}
	dataType = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_VERSIONNUM);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_VERSIONNUM);
		return;
	}
	verNumber = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_ROOTPATH);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_ROOTPATH);
		return;
	}
	rootPath = itor->second;

	itor = params.find(SHAREFOLDER_ISDELEDIR);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_ISDELEDIR);
		return;
	}
	clear = atoi((itor->second).c_str());

	FolderDataDict::iterator it;
	DataPublishPointInfo dataPPInfo;	
	FolderExPrx folderExPrx;

	for (it = shareFolderDatas.begin(); it != shareFolderDatas.end(); it ++) 
	{
		
		folderExPrx = it->second;
		dataPPInfo = folderExPrx->getInfo();
		
		if (std::find(dataPPInfo.dataTypes.begin(), 
			dataPPInfo.dataTypes.end(), dataType) != dataPPInfo.dataTypes.end()) {
			it->second->onFullUpdate(groupId, rootPath, clear, verNumber);
		}
	}
	
	if (!DeleteDirectory(rootPath, true)) {
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"failed to delete remote Directory [%s])with error[%d]"), rootPath.c_str(), GetLastError());
		return;
	}

	glog( ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"delete remote directory [%s] successfully"),rootPath.c_str());
}

void
DataPointPublisherImpl::onFolderPartlyUpdate(const ::TianShanIce::Properties&params)
{
	glog( ZQ::common::Log::L_DEBUG, CLOGFMT(DATAPOINTPUBLISHIMPL,"enter onFolderPartlyUpdate()"));
	::Ice::Int groupId;
	::Ice::Int dataType;
	::std::string rootPath;
	::std::string paths;
	int verNumber;

	TianShanIce::Properties::const_iterator itor;

	itor = params.find(SHAREFOLDER_GROUPID);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_GROUPID);
		return;
	}
	groupId = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_DATATYPE);

	if(itor == params.end())
	{		
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_DATATYPE);
		return;
	}
	dataType = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_VERSIONNUM);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_VERSIONNUM);
		return;
	}
	verNumber = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_ROOTPATH);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_ROOTPATH);
		return;
	}
	rootPath = itor->second;

	itor = params.find(SHAREFOLDER_SUBPATH);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_SUBPATH);
		return;
	}
	paths = (itor->second).c_str();

	FolderDataDict::iterator it;
	DataPublishPointInfo dataPPInfo;	
	FolderExPrx folderExPrx;
	
	for (it = shareFolderDatas.begin(); 
		it != shareFolderDatas.end(); it ++) {
			
		folderExPrx = it->second;
		dataPPInfo = folderExPrx->getInfo();

		if (std::find(dataPPInfo.dataTypes.begin(), 
			dataPPInfo.dataTypes.end(), dataType) != dataPPInfo.dataTypes.end()) {

			it->second->onPartlyUpdate(groupId, rootPath, paths, verNumber);
		}
	}

	if (!DeleteDirectory(rootPath, true)) {
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"failed to  delete remote Directory [%s] with error(%d)"), rootPath.c_str(), GetLastError());
		return;
	}

	glog( ZQ::common::Log::L_INFO,  CLOGFMT(DATAPOINTPUBLISHIMPL,"delete remote directory [%s] successfully"),rootPath.c_str());
}

void
DataPointPublisherImpl::onFolderDeleted(const ::TianShanIce::Properties&params)
{
	glog( ZQ::common::Log::L_DEBUG, CLOGFMT(DATAPOINTPUBLISHIMPL,"enter onFolderDeleted()"));
	::Ice::Int groupId;
	::Ice::Int dataType;
	::std::string paths;
	int verNumber;

	TianShanIce::Properties::const_iterator itor;

	itor = params.find(SHAREFOLDER_GROUPID);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_GROUPID);
		return;
	}
	groupId = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_DATATYPE);

	if(itor == params.end())
	{		
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_DATATYPE);
		return;
	}
	dataType = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_VERSIONNUM);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_VERSIONNUM);
		return;
	}
	verNumber = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_SUBPATH);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_SUBPATH);
		return;
	}
	paths = (itor->second).c_str();

	FolderDataDict::iterator it;
	DataPublishPointInfo dataPPInfo;	
	FolderExPrx folderExPrx;

	for (it = shareFolderDatas.begin(); 
		it != shareFolderDatas.end(); it ++) {
			
		folderExPrx = it->second;
		dataPPInfo = folderExPrx->getInfo();

		if (std::find(dataPPInfo.dataTypes.begin(), 
			dataPPInfo.dataTypes.end(), dataType) != dataPPInfo.dataTypes.end()) {

			it->second->onFolderDeleted(groupId, paths, verNumber);
		}
	}

	glog( ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"leave onFolderDeleted()"));
}

void
DataPointPublisherImpl::onFileAdded(const ::TianShanIce::Properties&params)
{
	glog( ZQ::common::Log::L_DEBUG, CLOGFMT(DATAPOINTPUBLISHIMPL,"enter onFileAdded()"));
	int groupId;
	int dataType;
	::std::string rootPath;
	::std::string paths;
	int verNumber;

	TianShanIce::Properties::const_iterator itor;

	itor = params.find(SHAREFOLDER_GROUPID);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_GROUPID);
		return;
	}
	groupId = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_DATATYPE);
	if(itor == params.end())
	{		
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_DATATYPE);
		return;
	}
	dataType = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_VERSIONNUM);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_VERSIONNUM);
		return;
	}
	verNumber = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_ROOTPATH);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_ROOTPATH);
		return;
	}
	rootPath = itor->second;

	itor = params.find(SHAREFOLDER_SUBPATH);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_SUBPATH);
		return;
	}
	paths = (itor->second).c_str();

	FolderDataDict::iterator it;
	DataPublishPointInfo dataPPInfo;	
	FolderExPrx folderExPrx;

	for (it = shareFolderDatas.begin(); 
		it != shareFolderDatas.end(); it ++) {
			
		folderExPrx = it->second;
		dataPPInfo = folderExPrx->getInfo();

		if (std::find(dataPPInfo.dataTypes.begin(), 
			dataPPInfo.dataTypes.end(), dataType) != dataPPInfo.dataTypes.end()) {

			it->second->onFileAdded(groupId,rootPath, paths, verNumber);
		}
	}

	if (!DeleteDirectory(rootPath, true)) {
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"failed to  delete remote Directory [%s]with error[%d]"), rootPath.c_str(), GetLastError());
		return;
	}

	glog( ZQ::common::Log::L_INFO,CLOGFMT(DATAPOINTPUBLISHIMPL,"delete remote directory [%s] successfully"),rootPath.c_str());
}

void
DataPointPublisherImpl::onFileModified(const ::TianShanIce::Properties&params)
{
	glog( ZQ::common::Log::L_DEBUG, CLOGFMT(DATAPOINTPUBLISHIMPL,"enter onFileModified()"));
	int groupId;
    int dataType;
	::std::string rootPath;
	::std::string paths;
	int verNumber;

	FolderDataDict::iterator it;
	DataPublishPointInfo dataPPInfo;	
	FolderExPrx folderExPrx;
	
	TianShanIce::Properties::const_iterator itor;

	itor = params.find(SHAREFOLDER_GROUPID);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_GROUPID);
		return;
	}
	groupId = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_DATATYPE);
	if(itor == params.end())
	{		
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_DATATYPE);
		return;
	}
	dataType = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_VERSIONNUM);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_VERSIONNUM);
		return;
	}
	verNumber = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_ROOTPATH);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_ROOTPATH);
		return;
	}
	rootPath = itor->second;

	itor = params.find(SHAREFOLDER_SUBPATH);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_SUBPATH);
		return;
	}
	paths = (itor->second).c_str();

	for (it = shareFolderDatas.begin(); 
	it != shareFolderDatas.end(); it ++)
	{
		
		folderExPrx = it->second;
		dataPPInfo = folderExPrx->getInfo();
		
		if (std::find(dataPPInfo.dataTypes.begin(), 
			dataPPInfo.dataTypes.end(), dataType) != dataPPInfo.dataTypes.end())
		{
			
			it->second->onFileModified(groupId, rootPath, paths, verNumber);
		}
	}

	if (!DeleteDirectory(rootPath, true)) {
			glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"failed to  delete remote Directory [%s] with error[%d]"), rootPath.c_str(), GetLastError());
			return;
	}

	glog( ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"delete remote directory [%s] successfully"),rootPath.c_str());
}

void
DataPointPublisherImpl::onFileDeleted(const ::TianShanIce::Properties&params)
{
	glog( ZQ::common::Log::L_DEBUG, CLOGFMT(DATAPOINTPUBLISHIMPL,"enter onFileDeleted()"));
	int groupId;
	int dataType;
	::std::string paths;
	int verNumber;

	TianShanIce::Properties::const_iterator itor;

	itor = params.find(SHAREFOLDER_GROUPID);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_GROUPID);
		return;
	}
	groupId = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_DATATYPE);
	if(itor == params.end())
	{		
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_DATATYPE);
		return;
	}
	dataType = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_VERSIONNUM);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_VERSIONNUM);
		return;
	}
	verNumber = atoi((itor->second).c_str());

	itor = params.find(SHAREFOLDER_ROOTPATH);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), SHAREFOLDER_ROOTPATH);
		return;
	}
    paths = itor->second;

	FolderDataDict::iterator it;
	DataPublishPointInfo dataPPInfo;	
	FolderExPrx folderExPrx;

	for (it = shareFolderDatas.begin(); 
		it != shareFolderDatas.end(); it ++) {
			
		folderExPrx = it->second;
		dataPPInfo = folderExPrx->getInfo();

		if (std::find(dataPPInfo.dataTypes.begin(), 
			dataPPInfo.dataTypes.end(), dataType) != dataPPInfo.dataTypes.end()) {

			it->second->onFileDeleted(groupId, paths, verNumber);
		}
	}
	glog( ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"leave onFileDeleted()"));
}

void
DataPointPublisherImpl::onMessageAdded(const ::TianShanIce::Properties&params)
{
	glog( ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"enter onMessageAdded()"));
	int groupId;
	int dataType;
	::std::string messageId;
	::std::string messagedest;
	::std::string messageBody;
	long exprie;

	TianShanIce::Properties::const_iterator itor;

	itor = params.find(MESSAGE_GROUPID);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), MESSAGE_GROUPID);
		return;
	}
	groupId = atoi((itor->second).c_str());

	itor = params.find(MESSAGE_DATATYPE);
	if(itor == params.end())
	{		
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), MESSAGE_DATATYPE);
		return;
	}
	dataType = atoi((itor->second).c_str());

	itor = params.find(MESSAGE_EXPIRATION);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), MESSAGE_EXPIRATION);
		return;
	}
	exprie = atoi((itor->second).c_str());

	itor = params.find(MESSAGE_MESSAGEID);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), MESSAGE_MESSAGEID);
		return;
	}
	messageId = itor->second;

	itor = params.find(MESSAGE_MESSAGEBODY);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), MESSAGE_MESSAGEBODY);
		return;
	}
	messageBody = itor->second;

	itor = params.find(MESSAGE_DESTINATION);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), MESSAGE_DESTINATION);
		return;
	}
	messagedest = itor->second;

	MsgQueueDict::iterator it;
	DataPublishPointInfo dataPPInfo;	
	MessageQueueExPrx msgQueueprx;

	for (it = msgQueues.begin(); 
		it != msgQueues.end(); it ++) {
			
		msgQueueprx = it->second;
		dataPPInfo = msgQueueprx->getInfo();

		if (std::find(dataPPInfo.dataTypes.begin(), 
			dataPPInfo.dataTypes.end(), dataType) != dataPPInfo.dataTypes.end()) {
				TianShanIce::Application::DataOnDemand::Message message;
				message.expiration = exprie;
				message.msgBody = messageBody;
				message.msgDest = messagedest;
			it->second->onMessageAdded(groupId, messageId, message);
		}
	}
	glog( ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"leave onMessageAdded()"));
}

void
DataPointPublisherImpl::onMessageDeleted(const ::TianShanIce::Properties&params)
{
	glog( ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"enter onMessageDeleted()"));
	int groupId;
	int dataType;
	::std::string messageId;

	TianShanIce::Properties::const_iterator itor;

	itor = params.find(MESSAGE_GROUPID);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), MESSAGE_GROUPID);
		return;
	}
	groupId = atoi((itor->second).c_str());

	itor = params.find(MESSAGE_DATATYPE);
	if(itor == params.end())
	{		
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), MESSAGE_DATATYPE);
		return;
	}
	dataType = atoi((itor->second).c_str());

	itor = params.find(MESSAGE_MESSAGEID);
	if(itor == params.end())
	{
		glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"missing parameter[%s]"), MESSAGE_MESSAGEID);
		return;
	}
	messageId = itor->second;

	MsgQueueDict::iterator it;
	DataPublishPointInfo dataPPInfo;	
	MessageQueueExPrx msgQueueprx;

	for (it = msgQueues.begin(); 
		it != msgQueues.end(); it ++) {
			
		msgQueueprx = it->second;
		dataPPInfo = msgQueueprx->getInfo();

		if (std::find(dataPPInfo.dataTypes.begin(), 
			dataPPInfo.dataTypes.end(), dataType) != dataPPInfo.dataTypes.end()) {

			it->second->onMessageDeleted(groupId, messageId);
		}
	}

	glog( ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"leave onMessageDeleted()"));
}

// impl
void 
DataPointPublisherImpl::activate(const ::Ice::Current& current)
{
	try
	{  
		DataStreamDict::iterator it;
		try
		{	
			glog( ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"activate datstream count [%d]"), dataStreams.size());
			
			for (it = dataStreams.begin(); it != dataStreams.end(); it ++) {
				try
				{
					it->second->activate();
					glog( ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"[%s]activate datastream successfully"),it->first.c_str());
				}
				catch(const DataOnDemand::StreamerException& ex)
				{
					glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"[%s]fail to activate datastream  errormessage(%s)"),it->first.c_str(),ex.ice_name().c_str());
                    
				//	_pCreatDestTrd->AddCreatMap(it->first, it->second);
				}
			}
		}
		catch(...)
		{
			glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"[%s]activate datastream  caught unknown exception (%d)"),it->first.c_str(), GetLastError());
				throw Ice::Exception(__FILE__, __LINE__);
		}

		try
		{
			glog( ZQ::common::Log::L_INFO,  CLOGFMT(DATAPOINTPUBLISHIMPL,"activate message datapublishpoint count (%d)"),msgQueues.size());
			{	
				MsgQueueDict::iterator it;
				for (it = msgQueues.begin(); it != msgQueues.end(); it ++) 
				{
					it->second->activate();
					glog( ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"[%s]activate message datapublishpoint successfully"),it->first.c_str());
				}			
			}
			{	
				glog( ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"activate sharefolder datapublishpoint count (%d)"),shareFolderDatas.size());
				FolderDataDict::iterator it;
				for (it = shareFolderDatas.begin(); 
					it != shareFolderDatas.end(); it ++) 
				{
						it->second->activate();
						glog( ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"[%s]activate sharefolder datapublishpoint successfully"),it->first.c_str());
				}
				
				glog( ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"activate localfolder datapublishpoint count (%d)"),localFolderDatas.size());
				
				for (it = localFolderDatas.begin(); 
				it != localFolderDatas.end(); it ++) {
					it->second->activate();
					glog( ZQ::common::Log::L_INFO, CLOGFMT(DATAPOINTPUBLISHIMPL,"[%s]activate localfolder datapublishpoint successfully"),it->first.c_str());					
				}				
			}
		}
		catch(...)
		{
			glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"activate datastream or datapublishpoint caught unknown exception(%d)"),GetLastError());
			throw Ice::Exception(__FILE__, __LINE__);
		}

	} catch (const Ice::Exception & ) {

		// do something		
		throw Ice::Exception(__FILE__, __LINE__);
	}
}

void DataPointPublisherImpl::reconnect(
	const ::Ice::Current& current)
{
   checkSync(true);
}
bool DataPointPublisherImpl::checkSync(bool bReconnect)
{
	return true;
}
inline DataPointPublisherExPrx 
DataPointPublisherImpl::getThisProxy(
		const Ice::ObjectAdapterPtr& adapter)
{
	if (_thisPrx == NULL) {
		try
		{	
			_thisPrx = DataPointPublisherExPrx::uncheckedCast(
				_adapter->createProxy(_adapter->getCommunicator()->
				stringToIdentity(DATA_ONDEMAND_DODAPPNAME)));
		}
		catch (Ice::Exception& ex)
		{
			glog( ZQ::common::Log::L_ERROR, CLOGFMT(DATAPOINTPUBLISHIMPL,"fail to get the DataPointPublisherEx proxy caught ice exception (%s)"),ex.ice_name().c_str());
			_thisPrx = NULL;
		}
	}
	return _thisPrx;
}
SessionRenewThread * 
DataPointPublisherImpl::getSessionTrd(std::string destname)
{
	SessionTrdMap::iterator it = _SessionTrdMap.find(destname);
	if (it == _SessionTrdMap.end())
		return NULL;
	
	return it->second;
}

::TianShanIce::Application::PublishPointPrx
DataPointPublisherImpl::publish(const ::std::string& name,
												   ::Ice::Int maxBitrate,
												   const ::std::string& desc,
												   const Ice::Current& current)
{
	return 0;
}

::TianShanIce::Application::PublishPointPrx
DataPointPublisherImpl::open(const ::std::string& name,
												const Ice::Current& current)
{
	return 0;
}

::TianShanIce::StrValues
DataPointPublisherImpl::list(const Ice::Current& current)
{
	return ::TianShanIce::StrValues();
}

void
DataPointPublisherImpl::listPublishPointInfo_async(const ::TianShanIce::Application::AMD_PointPublisher_listPublishPointInfoPtr& listPublishPointInfoCB,
																		const ::TianShanIce::StrValues& paramNames,
																		const Ice::Current& current) const
{
	::TianShanIce::Application::PublishPointInfos r;
	listPublishPointInfoCB->ice_response(r);
}
} // END DataOnDemand
} // END Application
} // END TianshanICE