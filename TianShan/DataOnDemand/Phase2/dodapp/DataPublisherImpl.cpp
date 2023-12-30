#include "stdafx.h"
#include "DODAppImpl.h"
#include "DataPublisherImpl.h"
#include "FolderChannelImpl.h"
#include "MessageChannelImpl.h"
#include "DestinationImpl.h"
#include "global.h"
#include "..\common\Reskey.h"
#include <algorithm>
//////////////////////////////////////////////////////////////////////////

Ice::ObjectAdapterPtr DataOnDemand::DataPublisherImpl::_adapter;
::Freeze::EvictorPtr DataOnDemand::DataPublisherImpl::_evictor;
DataOnDemand::DODAppServicePtr DataOnDemand::DataPublisherImpl::_dodappservice;
TianShanIce::SRM::SessionManagerPrx DataOnDemand::DataPublisherImpl::_sessManager;
TianShanIce::Storage::ContentStorePrx  DataOnDemand::DataPublisherImpl::_contentStroe;
SessionTrdMap	DataOnDemand::DataPublisherImpl::_SessionTrdMap;
CreatDestionTrd*	DataOnDemand::DataPublisherImpl::_pCreatDestTrd;
/*
DataOnDemand::ChannelTypeIndexPtr 
	DataOnDemand::DataPublisherImpl::_channelTypeIndex;
*/

Ice::CommunicatorPtr DataOnDemand::DataPublisherImpl::_ic;

//////////////////////////////////////////////////////////////////////////

DataOnDemand::DataPublisherImpl::DataPublisherImpl()
{
	_thisPrx = NULL;
}

DataOnDemand::DataPublisherImpl::~DataPublisherImpl()
{

}

bool DataOnDemand::DataPublisherImpl::init()
{	
	return true;
}

::DataOnDemand::FolderChannelPrx
DataOnDemand::DataPublisherImpl::createLocalFolderChannel(
							   const ::std::string& name,
						       const ::DataOnDemand::ChannelInfo& info,
						       const ::std::string& path,
						       const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::createLocalFolderChannel(): Enter create local folder channel (%s)",
		name.c_str());
	FolderChannelExPrx channel = FolderChannelExPrx::uncheckedCast(
		NameToChannel(current.adapter)(name));
    TianShanIce::Properties props;
	FolderChannelImpl* folderChannel = new FolderChannelImpl;
    
	folderChannel->myInfo = info;
	folderChannel->myInfo.name = name;
	folderChannel->myParent = getThisProxy(current.adapter);
	folderChannel->myType = dodLocalFolder;
	
	props = folderChannel->getProperties(Ice::Current());	
	props["Path"] =  path;	
	folderChannel->setProperties(props,Ice::Current());

	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::createLocalFolderChannel() channel (%s) init",
		name.c_str());

	if (!folderChannel->init()) {
		delete folderChannel;
		return false;
	}
	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::createLocalFolderChannel() channel (%s) init success",
		name.c_str());
	Ice::Int queueSize = _evictor->getSize();
	_evictor->setSize(0);

	_evictor->add(folderChannel, 
		DataOnDemand::createChannelIdentity(name));
		
	_evictor->setSize(queueSize);	
	
	myLocFodChannels.insert(
		DataOnDemand::FodChannelDict::value_type(name, channel));
	
	activeChannelManager.create(
		ChannelPublishPointPrx::checkedCast(channel));
	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::createLocalFolderChannel() Leave create local folder channel (%s)",
		name.c_str());
    return channel;
}

::DataOnDemand::FolderChannelPrx
DataOnDemand::DataPublisherImpl::createShareFolderChannel(const ::std::string& name,
						       const ::DataOnDemand::ChannelInfo& info,
						       const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::createShareFolderChannel() Enter create share folder channel (%s)",
		name.c_str());
	FolderChannelExPrx channel = FolderChannelExPrx::uncheckedCast(
		NameToChannel(current.adapter)(name));

	FolderChannelImpl* folderChannel = new FolderChannelImpl;

	folderChannel->myInfo = info;
	folderChannel->myInfo.name = name;
	folderChannel->myParent = getThisProxy(current.adapter);
	folderChannel->myType = dodSharedFolder;

	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::createShareFolderChannel() channel (%s) init",
		name.c_str());

	if (!folderChannel->init()) {
		delete folderChannel;
		return NULL;
	}
	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::createShareFolderChannel(): channel (%s) init success",
		name.c_str());
	Ice::Int queueSize = _evictor->getSize();
	_evictor->setSize(0);

	_evictor->add(folderChannel, 
		DataOnDemand::createChannelIdentity(name));

	_evictor->setSize(queueSize);
	
	myShaFodChannels.insert(
		DataOnDemand::FodChannelDict::value_type(name, channel));

	activeChannelManager.create(
		ChannelPublishPointPrx::checkedCast(channel));

	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::createShareFolderChannel() Leave create local folder channel (%s)",
		name.c_str());
    return channel;
}

::DataOnDemand::MessageChannelPrx
DataOnDemand::DataPublisherImpl::createMsgChannel(const ::std::string& name,
					       const ::DataOnDemand::ChannelInfo& info,
					       const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::createMsgChannel() Enter create message channel (%s)",
		name.c_str());
	MessageChannelExPrx channel = MessageChannelExPrx::uncheckedCast(
		NameToChannel(current.adapter)(name));

	MessageChannelImpl* msgChannel = new MessageChannelImpl;

	msgChannel->myInfo = info;
	msgChannel->myInfo.name = name;
	msgChannel->myParent = getThisProxy(current.adapter);
	msgChannel->myType = dodMessage;

	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::createMsgChannel() channel (%s) init success",
		name.c_str());

	if (!msgChannel->init()) {
		delete msgChannel;
		return NULL;
	}
	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::createMsgChannel() channel (%s) init success",
		name.c_str());

	Ice::Int queueSize = _evictor->getSize();
	_evictor->setSize(0);

	_evictor->add(msgChannel, 
		DataOnDemand::createChannelIdentity(name));

	_evictor->setSize(queueSize);
	
	myMsgChannels.insert(
		DataOnDemand::MsgChannelDict::value_type(name, 
		channel));

	activeChannelManager.create(
		ChannelPublishPointPrx::checkedCast(channel));
	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::createMsgChannel() Leave create message channel (%s)",
		name.c_str());
    return channel;
}

::DataOnDemand::ChannelPublishPointPrx
DataOnDemand::DataPublisherImpl::getChannel(const ::std::string& name,
					 const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::getChannel() Enter get Channel (%s)",
		name.c_str());
	{
		MsgChannelDict::iterator it = myMsgChannels.find(name);
		if (it != myMsgChannels.end())
			return ChannelPublishPointPrx::checkedCast(it->second);
	}

	{
		FodChannelDict::iterator it = myShaFodChannels.find(name);
		if (it != myShaFodChannels.end())
			return ChannelPublishPointPrx::checkedCast(it->second);
	}

	{
		FodChannelDict::iterator it = myLocFodChannels.find(name);
		if (it != myLocFodChannels.end())
			return ChannelPublishPointPrx::checkedCast(it->second);
	}

	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::getChannel() can't get Channel (%s) info",
		name.c_str());

	throw Ice::ObjectNotExistException(__FILE__, __LINE__);

	return NULL;
}


::DataOnDemand::DestinationPrx
DataOnDemand::DataPublisherImpl::createDestination(
						const ::std::string& destName,
						const ::DataOnDemand::DestInfo& info,
						const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::createDestination() Enter create destination(%s)",
		destName.c_str());

	DestinationExPrx destPrx = DestinationExPrx::uncheckedCast(
		NameToDestination(current.adapter)(destName));
	
	DestinationImpl* destObj = new DestinationImpl;
	
	destObj->myInfo = info;
	destObj->myInfo.name = destName;
	destObj->myParent = getThisProxy(current.adapter);

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

		TianShanIce::SRM::SessionPrx sessionprx = 
								_sessManager->createSession(clientResource);

		valuemap.clear();
       	var.type = ::TianShanIce::vtInts;
		var.bRange = false;
		vtints.clear();
		vtints.push_back(gDODAppServiceConfig.lDODAppServiceGroup);
		var.ints = vtints;
		valuemap["id"]=var;

		sessionprx->addResource(::TianShanIce::SRM::rtServiceGroup,
			::TianShanIce::SRM::raMandatoryNonNegotiable,
			valuemap);
	
		valuemap.clear();
       	var.type = ::TianShanIce::vtLongs;
		var.bRange = false;
	    var.lints.clear();
        var.lints.push_back(info.totalBandwidth);
		valuemap["bandwidth"]=var;
		
		sessionprx->addResource(::TianShanIce::SRM::rtTsDownstreamBandwidth,
			::TianShanIce::SRM::raMandatoryNonNegotiable,
			valuemap);

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
		glog(ZQ::common::Log::L_INFO,
			"DataPublisherImpl::createDestination() set privateData");
        sessionprx->setPrivateData(RESKEY_TOTALBANDWIDTH, var);
	
		glog(ZQ::common::Log::L_INFO,
			"DataPublisherImpl::createDestination() provision");
		sessionprx->provision();

		glog(ZQ::common::Log::L_INFO,
			"DataPublisherImpl::createDestination() serve");
		sessionprx->serve();

		glog(ZQ::common::Log::L_INFO,
			"DataPublisherImpl::createDestination() renew (%d)", gDODAppServiceConfig.lRenewtime * 1000);
		sessionprx->renew(gDODAppServiceConfig.lRenewtime * 1000);


		destObj->mySessionId = sessionprx->getId();
		glog(ZQ::common::Log::L_INFO,
			"DataPublisherImpl::createDestination() get stream(%s) sessionID (%s)",
			destName.c_str(), destObj->mySessionId.c_str());

		glog(ZQ::common::Log::L_INFO,
			"DataPublisherImpl::createDestination() set privateData");
		streamprx = sessionprx->getStream();

		glog(ZQ::common::Log::L_INFO,
			"DataPublisherImpl::createDestination() checkedCast");
		destObj->_stream = DataOnDemand::DataStreamPrx::checkedCast(streamprx);
	}
	catch (TianShanIce::SRM::InvalidResource&ex) 
	{
		glog(ZQ::common::Log::L_ERROR,
			"DataPublisherImpl::createDestination() TianShanIce::SRM::InvalidResource(%s)",
			ex.message.c_str());
		return NULL;
	}
	catch(TianShanIce::NotSupported&ex)
	{
		glog(ZQ::common::Log::L_ERROR,
			"DataPublisherImpl::createDestination() TianShanIce::NotSupported(%s)",
			ex.message.c_str());
		return NULL;
	}
	catch(TianShanIce::InvalidParameter&ex)
	{
		glog(ZQ::common::Log::L_ERROR,
			"DataPublisherImpl::createDestination() TianShanIce::InvalidParameter(%s)",
			ex.message.c_str());
		return NULL;
	}
	catch(TianShanIce::ServerError&ex)
	{
		glog(ZQ::common::Log::L_ERROR,
			"DataPublisherImpl::createDestination() TianShanIce::ServerError(%s)",
			ex.message.c_str());
		return NULL;
	}
	catch (const ::Ice::Exception & ex)
	{
		glog(ZQ::common::Log::L_ERROR,
			"DataPublisherImpl::createDestination() Ice::Exception (%s)",
			ex.ice_name().c_str());
		return NULL;
	}

	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::createDestination() destination (%s) init", destName.c_str());

	if (!destObj->init()) {
		glog(ZQ::common::Log::L_INFO,
			"DataPublisherImpl::createDestination() destination (%s) init failur", destName.c_str());
		delete destObj;
		return NULL;
	}

	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::createDestination() destination (%s) init success", destName.c_str());

	Ice::Int queueSize = _evictor->getSize();
	_evictor->setSize(0);

	_evictor->add(destObj, 
		DataOnDemand::createDestinationIdentity(destName));
	
	_evictor->setSize(queueSize);
	
	myDests.insert(DataOnDemand::DestDict::value_type(destName, 
		destPrx));

	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::createDestination() Leave  create destination (%s)", destName.c_str());
    return destPrx;
}

::DataOnDemand::DestinationPrx
DataOnDemand::DataPublisherImpl::getDestination(const ::std::string& name,
					     const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::getDestination() Enter  get destination (%s)",
		name.c_str());

	DestDict::iterator it = myDests.find(name);
	if (it == myDests.end())	
	{
		glog(ZQ::common::Log::L_INFO,
			"DataPublisherImpl::getDestination(): can't find destination (%s)",
			name.c_str());
		throw Ice::ObjectNotExistException(__FILE__, __LINE__);
	}
	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::getDestination() get destination (%s) info success",
		name.c_str());
	return it->second;
}

::TianShanIce::StrValues
DataOnDemand::DataPublisherImpl::listChannels(const Ice::Current& current) const
{
	::TianShanIce::StrValues result;
	try	{

		{
			MsgChannelDict::const_iterator it;
			
			for (it = myMsgChannels.begin(); 
			    it != myMsgChannels.end(); it ++) {
				result.push_back(it->first);
			}
		}
		
		{
			FodChannelDict::const_iterator it;
			
			for (it = myShaFodChannels.begin(); 
			it != myShaFodChannels.end(); it ++) {
				
				result.push_back(it->first);
			}
			
			for (it = myLocFodChannels.begin(); 
			it != myLocFodChannels.end(); it ++) {
				
				result.push_back(it->first);
			}
		}

	} catch(...) {
       glog(ZQ::common::Log::L_ERROR,
			"DataPublisherImpl::listChannels() unknown exception(%d)",
			GetLastError());
	}

    return result;
}

::TianShanIce::StrValues
DataOnDemand::DataPublisherImpl::listDestinations(const Ice::Current& current) const
{
    ::TianShanIce::StrValues result;
	DestDict::const_iterator it;
	result.clear();

	try	{

		for (it = myDests.begin(); it != myDests.end(); it ++)
		{
			result.push_back(it->first);
		}
		
	} catch(...) {

       glog(ZQ::common::Log::L_ERROR,
		   "DataPublisherImpl::listChannels() unknown exception(%d)",
			GetLastError());
	}
	
    return result;
}

void
DataOnDemand::DataPublisherImpl::removeDestination(
						  const ::std::string& name,
						  const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::removeDestination() Enter  remove destination (%s)",
		name.c_str());
	try {	

		myDests.erase(name);
		_evictor->remove(createDestinationIdentity(name));

	} catch (const ::Ice::Exception & ex) {

		glog(ZQ::common::Log::L_ERROR,
			"DataPublisherImpl::removeDestination() Ice::Exception (%s)",
			ex.ice_name().c_str());

	} catch (...) {

		glog(ZQ::common::Log::L_ERROR,
			"DataPublisherImpl::removeDestination() cauht unknown Exception (%d)",
			GetLastError());

	}
	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::removeDestination() Leave remove destination (%s)",
		name.c_str());

}

void
DataOnDemand::DataPublisherImpl::removeChannel(const ::std::string& name,
						  const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::removeChannel() Enter  remove channel(%s)",
		name.c_str());

	Ice::Identity ident;

	try	{				
			myMsgChannels.erase(name);
			myLocFodChannels.erase(name);
			myShaFodChannels.erase(name);
				
			ident = createChannelIdentity(name);
				
			if(_evictor->hasObject(ident))
			{
				_evictor->remove(ident);
			}				
	} 
	catch (const ::Ice::Exception & ex) 
	{
		
		glog(ZQ::common::Log::L_ERROR,
			"DataPublisherImpl::removeChannel() caught Ice::Exception(%s)",
			ex.ice_name().c_str());
		
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,
			"DataPublisherImpl::removeChannel() caught unknown exception (%d)",
			GetLastError());	
	}
	glog(ZQ::common::Log::L_INFO,
		"DataPublisherImpl::removeChannel() Leave remove channel(%s)",
		name.c_str());
}

void
DataOnDemand::DataPublisherImpl::notifyFolderFullUpdate(::Ice::Int groupId,
						     ::Ice::Int dataType,
						     const ::std::string& rootPath,
						     bool clear,
							 int verNumber,
						     const Ice::Current& current)
{
	Ice::ObjectAdapterPtr adapter = current.adapter;

	FodChannelDict::iterator it;
	ChannelInfo chInfo;	
	FolderChannelExPrx channel;

	for (it = myShaFodChannels.begin(); it != myShaFodChannels.end(); it ++) 
	{
		
		channel = it->second;
		chInfo = channel->getInfo();
		
		if (std::find(chInfo.dataTypes.begin(), 
			chInfo.dataTypes.end(), dataType) != chInfo.dataTypes.end()) {
			it->second->notifyFullUpdate(rootPath, clear, groupId,verNumber);
		}
	}
	
	if (!DeleteDirectory(rootPath, true)) {
		glog( ZQ::common::Log::L_ERROR,
			"DataPublisherImpl::notifyFolderFullUpdate()Delete remote Directory (%s) error", 
			rootPath.c_str());
		return;
	}

	glog( ZQ::common::Log::L_INFO,
		"DataPublisherImpl::notifyFolderFullUpdate()Delete remote Directory (%s) OK",
		rootPath.c_str());
}

void
DataOnDemand::DataPublisherImpl::notifyFolderPartlyUpdate(::Ice::Int groupId,
						       ::Ice::Int dataType,
						       const ::std::string& rootPath,
						       const ::std::string& paths,
							   int verNumber,
						       const Ice::Current& current)
{
	Ice::ObjectAdapterPtr adapter = current.adapter;

	FodChannelDict::iterator it;
	ChannelInfo chInfo;	
	FolderChannelExPrx channel;
	
	for (it = myShaFodChannels.begin(); 
		it != myShaFodChannels.end(); it ++) {
			
		channel = it->second;
		chInfo = channel->getInfo();

		if (std::find(chInfo.dataTypes.begin(), 
			chInfo.dataTypes.end(), dataType) != chInfo.dataTypes.end()) {

			it->second->notifyPartlyUpdate(rootPath, paths, groupId,verNumber);
		}
	}

	if (!DeleteDirectory(rootPath, true)) {
		glog( ZQ::common::Log::L_ERROR,
			"DataPublisherImpl::notifyFolderPartlyUpdate() Delete remote Directory (%s) error", 
			rootPath.c_str());
		return;
	}

	glog( ZQ::common::Log::L_INFO,
		"DataPublisherImpl::notifyFolderFullUpdate() Delete remote Directory (%s) OK",
		rootPath.c_str());
}

void
DataOnDemand::DataPublisherImpl::notifyFolderDeleted(::Ice::Int groupId,
						  ::Ice::Int dataType,
						  const ::std::string& paths,
						  int verNumber,
						  const Ice::Current& current)
{
	Ice::ObjectAdapterPtr adapter = current.adapter;

	FodChannelDict::iterator it;
	ChannelInfo chInfo;	
	FolderChannelExPrx channel;

	for (it = myShaFodChannels.begin(); 
		it != myShaFodChannels.end(); it ++) {
			
		channel = it->second;
		chInfo = channel->getInfo();

		if (std::find(chInfo.dataTypes.begin(), 
			chInfo.dataTypes.end(), dataType) != chInfo.dataTypes.end()) {

			it->second->notifyFolderDeleted(paths, groupId,verNumber);
		}
	}

}

void
DataOnDemand::DataPublisherImpl::notifyFileAdded(::Ice::Int groupId,
					      ::Ice::Int dataType,
					      const ::std::string& rootPath,
					      const ::std::string& paths,
						  int verNumber,
					      const Ice::Current& current)
{
	Ice::ObjectAdapterPtr adapter = current.adapter;

	FodChannelDict::iterator it;
	ChannelInfo chInfo;	
	FolderChannelExPrx channel;

	for (it = myShaFodChannels.begin(); 
		it != myShaFodChannels.end(); it ++) {
			
		channel = it->second;
		chInfo = channel->getInfo();

		if (std::find(chInfo.dataTypes.begin(), 
			chInfo.dataTypes.end(), dataType) != chInfo.dataTypes.end()) {

			it->second->notifyFileAdded(rootPath, paths, groupId,verNumber);
		}
	}

	if (!DeleteDirectory(rootPath, true)) {
		glog( ZQ::common::Log::L_ERROR,
			"DataPublisherImpl::notifyFileAdded() Delete remote Directory (%s) error", 
			rootPath.c_str());
		return;
	}


	glog( ZQ::common::Log::L_INFO,
		"DataPublisherImpl::notifyFolderFullUpdate() Delete remote Directory (%s) success",
		rootPath.c_str());
}

void
DataOnDemand::DataPublisherImpl::notifyFileModified(::Ice::Int groupId,
						 ::Ice::Int dataType,
						 const ::std::string& rootPath,
						 const ::std::string& paths,
						 int verNumber,
						 const Ice::Current& current)
{
	Ice::ObjectAdapterPtr adapter = current.adapter;

	FodChannelDict::iterator it;
	ChannelInfo chInfo;	
	FolderChannelExPrx channel;
	
	for (it = myShaFodChannels.begin(); 
	it != myShaFodChannels.end(); it ++)
	{
		
		channel = it->second;
		chInfo = channel->getInfo();
		
		if (std::find(chInfo.dataTypes.begin(), 
			chInfo.dataTypes.end(), dataType) != chInfo.dataTypes.end())
		{
			
			it->second->notifyFileModified(rootPath, paths, groupId,verNumber);
		}
	}

	if (!DeleteDirectory(rootPath, true)) {
			glog( ZQ::common::Log::L_ERROR,
				"DataPublisherImpl::notifyFileAdded()Delete remote Directory (%s) Error", 
				rootPath.c_str());
			return;
	}

	glog( ZQ::common::Log::L_INFO,
		"DataPublisherImpl::notifyFileAdded() Delete remote Directory (%s) success",
		rootPath.c_str());

}

void
DataOnDemand::DataPublisherImpl::notifyFileDeleted(::Ice::Int groupId,
						::Ice::Int dataType,
						const ::std::string& paths,
						int verNumber,
						const Ice::Current& current)
{
	Ice::ObjectAdapterPtr adapter = current.adapter;

	FodChannelDict::iterator it;
	ChannelInfo chInfo;	
	FolderChannelExPrx channel;

	for (it = myShaFodChannels.begin(); 
		it != myShaFodChannels.end(); it ++) {
			
		channel = it->second;
		chInfo = channel->getInfo();

		if (std::find(chInfo.dataTypes.begin(), 
			chInfo.dataTypes.end(), dataType) != chInfo.dataTypes.end()) {

			it->second->notifyFileDeleted(paths, groupId,verNumber);
		}
	}

}

void
DataOnDemand::DataPublisherImpl::notifyMessageAdded(::Ice::Int groupId,
						 ::Ice::Int dataType,
						 const ::std::string& messageId,
						 const ::std::string& dest,
						 const ::std::string& messageBody,
						 ::Ice::Long exprie,
						 const Ice::Current& current)
{
	Ice::ObjectAdapterPtr adapter = current.adapter;

	MsgChannelDict::iterator it;
	ChannelInfo chInfo;	
	MessageChannelExPrx channel;

	for (it = myMsgChannels.begin(); 
		it != myMsgChannels.end(); it ++) {
			
		channel = it->second;
		chInfo = channel->getInfo();

		if (std::find(chInfo.dataTypes.begin(), 
			chInfo.dataTypes.end(), dataType) != chInfo.dataTypes.end()) {

			it->second->notifyMessageAdded(messageId, dest, messageBody, 
				exprie, groupId);
		}
	}

}

void
DataOnDemand::DataPublisherImpl::notifyMessageDeleted(::Ice::Int groupId,
						   ::Ice::Int dataType,
						   const ::std::string& messageId,
						   const Ice::Current& current)
{
	Ice::ObjectAdapterPtr adapter = current.adapter;

	MsgChannelDict::iterator it;
	ChannelInfo chInfo;	
	MessageChannelExPrx channel;

	for (it = myMsgChannels.begin(); 
		it != myMsgChannels.end(); it ++) {
			
		channel = it->second;
		chInfo = channel->getInfo();

		if (std::find(chInfo.dataTypes.begin(), 
			chInfo.dataTypes.end(), dataType) != chInfo.dataTypes.end()) {

			it->second->notifyMessageDeleted(messageId, groupId);
		}
	}

}

// impl
void 
DataOnDemand::DataPublisherImpl::activate(const ::Ice::Current& current)
{
	try
	{   DestDict::iterator it;
		try
		{	
			glog( ZQ::common::Log::L_INFO,
				"DataPublisherImpl::activate() Destination count(%d)",
				myDests.size());
			
			for (it = myDests.begin(); it != myDests.end(); it ++) {
				try
				{
					it->second->activate();
					glog( ZQ::common::Log::L_INFO,
						"DataPublisherImpl::activate() activate destination (%s) success",
						it->first.c_str());
				}
				catch(const DataOnDemand::StreamerException& ex)
				{
					glog( ZQ::common::Log::L_ERROR,
						"DataPublisherImpl::activate():activate destination (%s)  error(%s)",
						it->first.c_str(),ex.ice_name().c_str());
                    
				//	_pCreatDestTrd->AddCreatMap(it->first, it->second);
				}
			}
		}
		catch(...)
		{
			glog( ZQ::common::Log::L_ERROR,
				"DataPublisherImpl::activate() activate destination (%s) unknow exception(%d)",
				it->first.c_str(), GetLastError());
				throw Ice::Exception(__FILE__, __LINE__);
		}

		try
		{
			glog( ZQ::common::Log::L_INFO,
				"DataPublisherImpl::activate()Message Channels count(%d)",
				myMsgChannels.size());
			{	
				MsgChannelDict::iterator it;
				for (it = myMsgChannels.begin(); it != myMsgChannels.end(); 
				it ++) {
					it->second->activate();
					glog( ZQ::common::Log::L_INFO,
					"DataPublisherImpl::activate() activate channel (%s)  success",
					it->first.c_str());
				}			
			}
			
			glog( ZQ::common::Log::L_INFO,
				"DataPublisherImpl::activate() Sharefold Channels count(%d)",
				myShaFodChannels.size());

			{
				FodChannelDict::iterator it;
				for (it = myShaFodChannels.begin(); 
				it != myShaFodChannels.end(); it ++) {
					it->second->activate();
					glog( ZQ::common::Log::L_INFO,
						"DataPublisherImpl::activate() activate channel (%s) success",
						it->first.c_str());
				}
				
				glog( ZQ::common::Log::L_INFO,
					"DataPublisherImpl::activate() LocalFold Channels count (%d)",
					myLocFodChannels.size());
				
				for (it = myLocFodChannels.begin(); 
				it != myLocFodChannels.end(); it ++) {
					it->second->activate();
					glog( ZQ::common::Log::L_INFO,
					"DataPublisherImpl::activate() activate channel (%s) success",
					it->first.c_str());					
				}				
			}
		}
		catch(...)
		{
			glog( ZQ::common::Log::L_ERROR,
				"DataPublisherImpl::activate() activate Destination or Channel error(%d)",
				GetLastError());
			throw Ice::Exception(__FILE__, __LINE__);
		}

	} catch (const Ice::Exception & ) {

		// do something		
		throw Ice::Exception(__FILE__, __LINE__);
	}
}

void DataOnDemand::DataPublisherImpl::reconnect(
	const ::Ice::Current& current)
{
   checkSync(true);
}
bool DataOnDemand::DataPublisherImpl::checkSync(bool bReconnect)
{
	return true;
}
inline DataOnDemand::DataPublisherExPrx 
DataOnDemand::DataPublisherImpl::getThisProxy(
		const Ice::ObjectAdapterPtr& adapter)
{
	if (_thisPrx == NULL) {

		_thisPrx = DataPublisherExPrx::uncheckedCast(
			_adapter->createProxy(_adapter->getCommunicator()->
			stringToIdentity(DATA_ONDEMAND_DODAPPNAME)));

	}

	return _thisPrx;
}
SessionRenewThread * 
DataOnDemand::DataPublisherImpl::getSessionTrd(std::string destname)
{
	SessionTrdMap::iterator it = _SessionTrdMap.find(destname);
	if (it == _SessionTrdMap.end())
		return NULL;
	
	return it->second;
}
