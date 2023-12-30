#include "stdafx.h"
#include "DODAppImpl.h"
#include "DataPublisherImpl.h"
#include "FolderChannelImpl.h"
#include "MessageChannelImpl.h"
#include "DestinationImpl.h"
#include "global.h"
// #include "ChannelTypeIndex.h"
#include <algorithm>
//////////////////////////////////////////////////////////////////////////

Ice::ObjectAdapterPtr DataOnDemand::DataPublisherImpl::_adapter;
::Freeze::EvictorPtr DataOnDemand::DataPublisherImpl::_evictor;

DataOnDemand::DataStreamServicePrx	
	DataOnDemand::DataPublisherImpl::_dataStreamSvc;

std::string	DataOnDemand::DataPublisherImpl::_strmSvcEndPoint;
/*
DataOnDemand::ChannelTypeIndexPtr 
	DataOnDemand::DataPublisherImpl::_channelTypeIndex;
*/

Ice::CommunicatorPtr DataOnDemand::DataPublisherImpl::_ic;

//////////////////////////////////////////////////////////////////////////

DataOnDemand::DataPublisherImpl::DataPublisherImpl()
{
	_thisPrx = NULL;
	_pNMC = NULL;
}

DataOnDemand::DataPublisherImpl::~DataPublisherImpl()
{

}

bool DataOnDemand::DataPublisherImpl::init()
{
	
/*	Ice::CommunicatorPtr ic = Ice::Application::communicator();

	Ice::PropertiesPtr properties = ic ->getProperties();

	long updateInterVal = properties->getPropertyAsInt(
		"DODAppService.UpdateInterval");

	// 加入属性校正代码
	// if(updateInterVal)
	//	;
	
    _pNMC = new CNotifyUpdateMsgChannel(updateInterVal);

	if(_pNMC = NULL)
		return false;*/
	
	return true;
}

::DataOnDemand::FolderChannelPrx
DataOnDemand::DataPublisherImpl::createLocalFolderChannel(
							   const ::std::string& name,
						       const ::DataOnDemand::ChannelInfo& info,
						       const ::std::string& path,
						       const Ice::Current& current)
{
	//! 请加入参数检查代码, 关键点加入log输出
	FolderChannelExPrx channel = FolderChannelExPrx::uncheckedCast(
		NameToChannel(current.adapter)(name));

	FolderChannelImpl* folderChannel = new FolderChannelImpl;

	folderChannel->myInfo = info;
	folderChannel->myInfo.name = name;
	folderChannel->myParent = getThisProxy(current.adapter);
	folderChannel->myType = dodLocalFolder;


	if (!folderChannel->init()) {
		delete folderChannel;
		return false;
	}
	
	Ice::Int queueSize = _evictor->getSize();
	_evictor->setSize(0);

	_evictor->add(folderChannel, 
		DataOnDemand::createChannelIdentity(name));
		
	_evictor->setSize(queueSize);	
	
	myLocFodChannels.insert(
		DataOnDemand::FodChannelDict::value_type(name, channel));
	
	activeChannelManager.create(
		ChannelPublishPointPrx::checkedCast(channel));

    return channel;
}

::DataOnDemand::FolderChannelPrx
DataOnDemand::DataPublisherImpl::createShareFolderChannel(const ::std::string& name,
						       const ::DataOnDemand::ChannelInfo& info,
						       const Ice::Current& current)
{
    //! 请加入参数检查代码, 关键点加入log输出
	FolderChannelExPrx channel = FolderChannelExPrx::uncheckedCast(
		NameToChannel(current.adapter)(name));

	FolderChannelImpl* folderChannel = new FolderChannelImpl;

	folderChannel->myInfo = info;
	folderChannel->myInfo.name = name;
	folderChannel->myParent = getThisProxy(current.adapter);
	folderChannel->myType = dodSharedFolder;

	
	if (!folderChannel->init()) {
		delete folderChannel;
		return false;
	}

	Ice::Int queueSize = _evictor->getSize();
	_evictor->setSize(0);

	_evictor->add(folderChannel, 
		DataOnDemand::createChannelIdentity(name));

	_evictor->setSize(queueSize);
	
	myShaFodChannels.insert(
		DataOnDemand::FodChannelDict::value_type(name, channel));

	activeChannelManager.create(
		ChannelPublishPointPrx::checkedCast(channel));

    return channel;
}

::DataOnDemand::MessageChannelPrx
DataOnDemand::DataPublisherImpl::createMsgChannel(const ::std::string& name,
					       const ::DataOnDemand::ChannelInfo& info,
					       const Ice::Current& current)
{
    //! 请加入参数检查代码, 关键点加入log输出
	MessageChannelExPrx channel = MessageChannelExPrx::uncheckedCast(
		NameToChannel(current.adapter)(name));

	MessageChannelImpl* msgChannel = new MessageChannelImpl;

	msgChannel->myInfo = info;
	msgChannel->myInfo.name = name;
	msgChannel->myParent = getThisProxy(current.adapter);
	msgChannel->myType = dodMessage;

	if (!msgChannel->init()) {
		delete msgChannel;
		return false;
	}

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

    return channel;
}

::DataOnDemand::ChannelPublishPointPrx
DataOnDemand::DataPublisherImpl::getChannel(const ::std::string& name,
					 const Ice::Current& current)
{
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

	throw Ice::ObjectNotExistException(__FILE__, __LINE__);

	return NULL;
}


::DataOnDemand::DestinationPrx
DataOnDemand::DataPublisherImpl::createDestination(
						const ::std::string& destName,
						const ::DataOnDemand::DestInfo& info,
						const Ice::Current& current)
{
	try {
		StreamInfo strmInfo;

		strmInfo.name = destName;
		strmInfo.totalBandwidth = info.totalBandwidth;
		strmInfo.destAddress = info.destAddress;
		strmInfo.pmtPid = info.pmtPid;
		
		DataStreamPrx strm = _dataStreamSvc->createStreamByApp(NULL, 
			configSpaceName, strmInfo);
		
	} catch (NameDupException& ) {
		// the streamer is existent
		// I'll do nothing
	}

	//! 请加入参数检查代码, 关键点加入log输出
	DestinationExPrx destPrx = DestinationExPrx::uncheckedCast(
		NameToDestination(current.adapter)(destName));

	DestinationImpl* destObj = new DestinationImpl;

	destObj->myInfo = info;
	destObj->myInfo.name = destName;
	destObj->myParent = getThisProxy(current.adapter);

	if (!destObj->init()) {
		delete destObj;
		return false;
	}

	Ice::Int queueSize = _evictor->getSize();
	_evictor->setSize(0);

	_evictor->add(destObj, 
		DataOnDemand::createDestinationIdentity(destName));
	
	_evictor->setSize(queueSize);
	
	myDests.insert(DataOnDemand::DestDict::value_type(destName, 
		destPrx));

    return destPrx;

}

::DataOnDemand::DestinationPrx
DataOnDemand::DataPublisherImpl::getDestination(const ::std::string& name,
					     const Ice::Current& current)
{
	DestDict::iterator it = myDests.find(name);
	if (it == myDests.end())
		throw Ice::ObjectNotExistException(__FILE__, __LINE__);

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
			"listChannels: Exception errorcode = %d",
			GetLastError());
	}

    return result;
}

::TianShanIce::StrValues
DataOnDemand::DataPublisherImpl::listDestinations(const Ice::Current& current) const
{
    ::TianShanIce::StrValues result;
	DestDict::const_iterator it;

	try	{

		for (it = myDests.begin(); it != myDests.end(); it ++)
		{
			result.push_back(it->first);
		}
		
	} catch(...) {

       glog(ZQ::common::Log::L_DEBUG,
			"listChannels: Exception errorcode = %d",
			GetLastError());
	}
	
    return result;
}

void
DataOnDemand::DataPublisherImpl::removeDestination(
						  const ::std::string& name,
						  const Ice::Current& current)
{
	try {	

		myDests.erase(name);
		_evictor->remove(createDestinationIdentity(name));

	} catch (const ::Ice::Exception & ex) {

		glog(ZQ::common::Log::L_DEBUG,
			"removeDestination: Ice::Exception errorcode = %s",
			ex.ice_name().c_str());

	} catch (...) {

		glog(ZQ::common::Log::L_DEBUG,
			"listChannels: Exception errorcode = %d",
			GetLastError());

	}

}

void
DataOnDemand::DataPublisherImpl::removeChannel(const ::std::string& name,
						  const Ice::Current& current)
{
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
		glog(ZQ::common::Log::L_DEBUG,
			"removeChannel: Ice::Exception errorcode = %s",
			ex.ice_name().c_str());		
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_DEBUG,
			"listChannels: Exception errorcode = %d",
			GetLastError());	
	}
}

void
DataOnDemand::DataPublisherImpl::notifyFolderFullUpdate(::Ice::Int groupId,
						     ::Ice::Int dataType,
						     const ::std::string& rootPath,
						     bool clear,
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
			chInfo.dataTypes.end(), dataType) != chInfo.dataTypes.end()) {
			it->second->notifyFullUpdate(rootPath, clear, groupId);
		}
	}
	
	if (!DeleteDirectory(rootPath, true)) {
		glog( ZQ::common::Log::L_ERROR,
			"notifyFolderFullUpdate::Delete remote Directory dir = %s Error", 
			rootPath.c_str());
		return;
	}

	glog( ZQ::common::Log::L_INFO,
		"notifyFolderFullUpdate::Delete remote Directory dir = %s OK",
		rootPath.c_str());
}

void
DataOnDemand::DataPublisherImpl::notifyFolderPartlyUpdate(::Ice::Int groupId,
						       ::Ice::Int dataType,
						       const ::std::string& rootPath,
						       const ::std::string& paths,
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

			it->second->notifyPartlyUpdate(rootPath, paths, groupId);
		}
	}

	if (!DeleteDirectory(rootPath, true)) {
		glog( ZQ::common::Log::L_ERROR,
			"notifyFolderPartlyUpdate::Delete remote Directory dir = %s Error", 
			rootPath.c_str());
		return;
	}

	glog( ZQ::common::Log::L_INFO,
		"notifyFolderFullUpdate::Delete remote Directory dir = %s OK",
		rootPath.c_str());
}

void
DataOnDemand::DataPublisherImpl::notifyFolderDeleted(::Ice::Int groupId,
						  ::Ice::Int dataType,
						  const ::std::string& paths,
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

			it->second->notifyFolderDeleted(paths, groupId);
		}
	}

}

void
DataOnDemand::DataPublisherImpl::notifyFileAdded(::Ice::Int groupId,
					      ::Ice::Int dataType,
					      const ::std::string& rootPath,
					      const ::std::string& paths,
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

			it->second->notifyFileAdded(rootPath, paths, groupId);
		}
	}

	if (!DeleteDirectory(rootPath, true)) {
		glog( ZQ::common::Log::L_ERROR,
			"notifyFileAdded::Delete remote Directory dir = %s Error", 
			rootPath.c_str());
		return;
	}

	glog( ZQ::common::Log::L_INFO,
		"notifyFolderFullUpdate::Delete remote Directory dir = %s OK",
		rootPath.c_str());

}

void
DataOnDemand::DataPublisherImpl::notifyFileModified(::Ice::Int groupId,
						 ::Ice::Int dataType,
						 const ::std::string& rootPath,
						 const ::std::string& paths,
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
			
			it->second->notifyFileModified(rootPath, paths, groupId);
		}
	}

	if (!DeleteDirectory(rootPath, true)) {
			glog( ZQ::common::Log::L_ERROR,
				"notifyFileAdded::Delete remote Directory dir = %s Error", 
				rootPath.c_str());
			return;
	}

	glog( ZQ::common::Log::L_INFO,
		"notifyFolderFullUpdate::Delete remote Directory dir = %s OK",
		rootPath.c_str());

}

void
DataOnDemand::DataPublisherImpl::notifyFileDeleted(::Ice::Int groupId,
						::Ice::Int dataType,
						const ::std::string& paths,
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

			it->second->notifyFileDeleted(paths, groupId);
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
			glog( ZQ::common::Log::L_DEBUG,
				"DataPublisherImpl::activate():Dstination size = %d!",
				myDests.size());
			
			for (it = myDests.begin(); it != myDests.end(); it ++) {
				it->second->activate();
				glog( ZQ::common::Log::L_DEBUG,
					"DataPublisherImpl::activate Destination Name = %s success!",
					it->first.c_str());
			}
		}
		catch(const DataOnDemand::StreamerException& ex)
		{
			glog( ZQ::common::Log::L_DEBUG,
				"DataPublisherImpl::activate destination Name = %s, errorMsg = %s",
				it->first.c_str(),ex.ice_name().c_str());
		}
		try
		{
			glog( ZQ::common::Log::L_DEBUG,
				"DataPublisherImpl::activate():Msg Channels size = %d!",
				myMsgChannels.size());
			{	
				MsgChannelDict::iterator it;
				for (it = myMsgChannels.begin(); it != myMsgChannels.end(); 
				it ++) {
					it->second->activate();
					glog( ZQ::common::Log::L_DEBUG,
					"DataPublisherImpl::activate Channel Name = %s success!",
					it->first.c_str());
				}			
			}
			
			glog( ZQ::common::Log::L_DEBUG,
				"DataPublisherImpl::activate():Sharefold Channels size = %d!",
				myShaFodChannels.size());

			{
				FodChannelDict::iterator it;
				for (it = myShaFodChannels.begin(); 
				it != myShaFodChannels.end(); it ++) {
					it->second->activate();
					glog( ZQ::common::Log::L_DEBUG,
						"DataPublisherImpl::activate Channel Name = %s success!",
						it->first.c_str());
				}
				
				glog( ZQ::common::Log::L_DEBUG,
					"DataPublisherImpl::activate():LocalFold Channels size = %d!",
					myLocFodChannels.size());
				
				for (it = myLocFodChannels.begin(); 
				it != myLocFodChannels.end(); it ++) {
					it->second->activate();
					glog( ZQ::common::Log::L_DEBUG,
						"DataPublisherImpl::activate Channel Name = %s success!",
						it->first.c_str());					
				}				
			}
		}
		catch(...)
		{
			glog( ZQ::common::Log::L_DEBUG,
				"DataPublisherImpl::activate Destination or Channel error!");	
		}

		checkSync();

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
	::TianShanIce::StrValues streams;
	::TianShanIce::StrValues dests;
	::TianShanIce::StrValues muxitems;
	::TianShanIce::StrValues channels;
    
	::TianShanIce::StrValues::iterator destsitor;
	::TianShanIce::StrValues::iterator muxitemitor;
	::TianShanIce::StrValues::iterator streamsitor;
	::TianShanIce::StrValues::iterator channelsitor;

	::DataOnDemand::DestinationPrx destprx;
	::DataOnDemand::DestinationExPrx destexprx;
	DestDict::iterator it;

	::DataOnDemand::MuxItemPrx muxitemprx;

	::std::string streamName;
    ::DataOnDemand::DataStreamPrx datastreamprx;

	streams = _dataStreamSvc->listStrems(configSpaceName);

	glog( ZQ::common::Log::L_DEBUG,
		"DataPublisherImpl::checkSync():streams size = %d",
		streams.size());

	for(streamsitor = streams.begin();streamsitor != streams.end(); streamsitor++) 
	{
		glog( ZQ::common::Log::L_DEBUG,
		"DataPublisherImpl::checkSync():stream name  = %s",
		(*streamsitor).c_str());
	}

	dests   =  listDestinations(::Ice::Current());
	glog( ZQ::common::Log::L_DEBUG,
		"DataPublisherImpl::checkSync():destination size = %d",
		dests.size());

	for(destsitor = dests.begin();destsitor != dests.end(); destsitor++) 
	{
		glog( ZQ::common::Log::L_DEBUG,
		"DataPublisherImpl::checkSync():destination name  = %s",
		(*destsitor).c_str());
	}

	for(destsitor = dests.begin();destsitor != dests.end(); destsitor++) 
	{
		try
		{	
			destprx = getDestination(*destsitor,::Ice::Current());
		}
		catch (const Ice::ObjectNotExistException&)
		{
			glog(ZQ::common::Log::L_DEBUG, 
				"[Name = %s]destination not exist",
				(*destsitor).c_str());
			return false;
		}
        	
        streamsitor = std::find(streams.begin(), 
			streams.end(), *destsitor);
		
		if( streamsitor == streams.end())
		{
			if(bReconnect)
			{
				glog(ZQ::common::Log::L_DEBUG, 
					"checkSync(true):[Name = %s]stream not exist! create this stream!",
					(*destsitor).c_str());

				destexprx = ::DataOnDemand::DestinationExPrx::
					checkedCast(destprx);
				
				destexprx->activate();

				glog(ZQ::common::Log::L_DEBUG, 
					"checkSync(true):[Name = %s]create this stream success!",
					(*destsitor).c_str());
			}
         	continue;
		}
				
        streams.erase(streamsitor);

		try
		{	
			datastreamprx = _dataStreamSvc->getStream(
				                    configSpaceName, *destsitor);
		}
		catch (const Ice::ObjectNotExistException&)
		{
			glog(ZQ::common::Log::L_DEBUG,
				"checkSync:Ice::ObjectNotExistException,streamName = %s",
				(*destsitor).c_str());
			return false;
		}
		
		muxitems = datastreamprx->listMuxItems();

		glog( ZQ::common::Log::L_DEBUG,
			"DataPublisherImpl::checkSync():stream name = %s,muxitems size = %d!",
			(*destsitor).c_str(),muxitems.size());

		for(muxitemitor = muxitems.begin();muxitemitor != 
			                     muxitems.end(); muxitemitor++) 
		{
			glog( ZQ::common::Log::L_DEBUG,
				"DataPublisherImpl::checkSync():MuxItem name  = %s!",
				(*muxitemitor).c_str());
		}

		channels = destprx->listChannels();

		glog( ZQ::common::Log::L_DEBUG,
			"DataPublisherImpl::checkSync():destination name = %s,\
			channels size = %d!",(*destsitor).c_str(),channels.size());

		for(channelsitor = channels.begin();channelsitor != 
			                        channels.end(); channelsitor++) 
		{
			glog( ZQ::common::Log::L_DEBUG,
				"DataPublisherImpl::checkSync():channel name  = %s!",
				(*channelsitor).c_str());
		}

		for(channelsitor= channels.begin(); channelsitor != channels.end(); 
		                                            channelsitor++) 
		{			
			muxitemitor = std::find(muxitems.begin(), 
				muxitems.end(), *channelsitor);
			
            if( muxitemitor == muxitems.end())	
			{
				if(bReconnect)
				{
					::DataOnDemand::AttachedInfo attinfo;
					try
					{
						glog(ZQ::common::Log::L_DEBUG, 
						"checkSync(true):[Name = %s]MuxItem not exist!\
						create this MuxItem!",(*channelsitor).c_str());
						
						destexprx = ::DataOnDemand::DestinationExPrx::
							checkedCast(destprx);
						
						destexprx->getAttachedInfo(
							*channelsitor,attinfo);
						
						destprx->attachChannel(*channelsitor,attinfo.minBitRate,
							attinfo.repeatTime);
						
						glog(ZQ::common::Log::L_DEBUG, 
						"checkSync(true):[Name = %s]create this MuxItem success",
						(*channelsitor).c_str());
					}
					catch (const DataOnDemand::ObjectExistException &e)
					{
						glog(ZQ::common::Log::L_DEBUG,
							"checkSync:attachChannel Object exist! errorcode  = %s",
							e.ice_name().c_str());
					}					
					catch (const ::TianShanIce::InvalidParameter & ex) 
					{
						glog(ZQ::common::Log::L_DEBUG,
							"checkSync:TianShanIce::InvalidParameter errorcode = %s",
							ex.message);
						return false;
					} 
					catch (const Ice::Exception & ex) 
					{
						glog(ZQ::common::Log::L_DEBUG,
							"checkSync:Ice::UnknownException errorcode = %s",
							ex.ice_name().c_str());
						return false;
					} 									
				}
				continue;
			}
			
			muxitems.erase(muxitemitor);
		}
		
		//deal with extern muxitems;
		try
		{
			if(!muxitems.empty())           
			{
				
				for(muxitemitor = muxitems.begin(); muxitemitor != muxitems.end(); 
				muxitemitor++) 
				{
					
					glog(ZQ::common::Log::L_DEBUG, 
						"checkSync:[Name = %s]destroy MuxItem!",
						
						(*muxitemitor).c_str());
					muxitemprx =  datastreamprx->getMuxItem(*muxitemitor);
					muxitemprx->destroy();
					
					glog(ZQ::common::Log::L_DEBUG, 
						"checkSync:[Name = %s]destroy MuxItem success!",
						(*muxitemitor).c_str());
				}
			} 
		}
		catch (const DataOnDemand::DataStreamError& ex)
		{
			glog(ZQ::common::Log::L_DEBUG,
				"checkSync:DataOnDemand::DataStreamError errorcode = %s",
				ex.ice_name().c_str());			
		}
		catch(const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_DEBUG,
				"checkSync:Ice::Exception errorcode = %s",
				ex.ice_name().c_str());
		}
		if(bReconnect)
		{        
			try
			{
				destprx->serve();
			}
			//			catch(DataOnDemand::StreamInvalidState)
			catch (DataOnDemand::StreamerException* e)
			{
				glog(ZQ::common::Log::L_DEBUG, 
					"checkSync:DataOnDemand::StreamerException errorcode = %s",
					e->ice_name().c_str());
				return false;
			}
		}		
	}
////deal with extern muxitems and streams;
	try
	{                
		if (!streams.empty())
		{
			for(streamsitor = streams.begin();
			streamsitor != streams.end(); streamsitor++)
			{
					
				glog(ZQ::common::Log::L_DEBUG, 
					"checkSync:[Name = %s]destroy stream!",
					(*streamsitor).c_str());

				datastreamprx = _dataStreamSvc->getStream
					(configSpaceName, *streamsitor);
				datastreamprx->destroy();

				
				glog(ZQ::common::Log::L_DEBUG, 
					"checkSync:[Name = %s]destroy stream!",
					(*streamsitor).c_str());
			}
		}
	}
	catch (const DataOnDemand::DataStreamError& ex)
	{
		glog(ZQ::common::Log::L_DEBUG,
			"checkSync:DataOnDemand::DataStreamError errorcode = %s",
			ex.ice_name().c_str());	
		return false;
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_DEBUG,
			"checkSync:Ice::Exception errorcode = %s",
			ex.ice_name().c_str());
		return false;
	}

	glog(ZQ::common::Log::L_DEBUG,
		"DataOnDemand::DataPublisherExPrx: checkSync success!");
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
