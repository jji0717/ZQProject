#include "stdafx.h"
#include "DODAppImpl.h"
#include "DestinationImpl.h"
#include "DataPublisherImpl.h"
#include "util.h"
#include "global.h"

using namespace ZQ::common;

DataOnDemand::DestinationImpl::DestinationImpl()
{
	_thisPrx = NULL;
	_stream = NULL;
	myState = DestInit;
}

DataOnDemand::DestinationImpl::~DestinationImpl()
{
/*	glog(Log::L_DEBUG,"[destinationName = %s]~DestinationImpl():delete Object\
		success!", myInfo.name);*/

	_thisPrx = NULL;
	_stream = NULL;

}
bool 
DataOnDemand::DestinationImpl::init()
{
	return true;
}

::std::string
DataOnDemand::DestinationImpl::getName(const Ice::Current& current) const
{
    return myInfo.name;
}

::DataOnDemand::DestState
DataOnDemand::DestinationImpl::getState(const Ice::Current& current) const
{
    return myState;
}


// impl
void
DataOnDemand::DestinationImpl::attachChannel(const ::std::string& channelName,
					  ::Ice::Int minBitRate,
					  ::Ice::Int repeatTime,
					  const Ice::Current& current)
{
	
	ChannelPublishPointPrx channelPrx;
	ChannelInfo chInfo;
	MuxItemPrx muxItem;

	try {

		channelPrx = myParent->getChannel(channelName);
		chInfo = channelPrx->getInfo();

	} catch (Ice::ObjectNotExistException&) {

		throw ::TianShanIce::InvalidParameter();
	}

	/*
	if (myChannels.find(channelName) != myChannels.end()) {

		throw ObjectExistException();
	}
	*/

	FolderChannelExPrx folderChannelPrx;
	MessageChannelExPrx msgChannelPrx;

	CacheType ctype;
	std::string addr;

	ChannelType chType = channelPrx->getType();

	if (chType == dodLocalFolder || chType == dodSharedFolder) {
		
		folderChannelPrx = FolderChannelExPrx::checkedCast(channelPrx);
		folderChannelPrx->getCacheInfo(ctype, addr);

	} else {

		msgChannelPrx = MessageChannelExPrx::checkedCast(channelPrx);
		msgChannelPrx->getCacheInfo(ctype, addr);
	}

	try {
		
		DataStreamPrx strm;
		strm = getStream();

		MuxItemInfo muxItemInfo;
		muxItemInfo.name = channelName;
		muxItemInfo.streamId = chInfo.streamId;
		muxItemInfo.streamType = chInfo.streamType;
		muxItemInfo.bandWidth = minBitRate;
		muxItemInfo.tag = chInfo.tag;
		muxItemInfo.expiration = 0;
		muxItemInfo.repeatTime = repeatTime;
		muxItemInfo.ctype = dodCacheTypeSmb;
		muxItemInfo.cacheAddr = addr;
		muxItemInfo.encryptMode = chInfo.encrypt;
		muxItemInfo.subchannelCount = chInfo.subchannelCount;
		muxItem = strm->createMuxItem(muxItemInfo);

#ifdef _PHASE1
		::TianShanIce::Properties muxItemProps;
		::TianShanIce::Properties chItemProps;
		chItemProps = channelPrx->getProperties();
		muxItemProps = muxItem->getProperties();
		muxItemProps["Detect"] = chItemProps["Detect"];
		muxItemProps["DetectInterval"] = chItemProps["DetectInterval"];
		muxItemProps["DataExchangeType"] = 
			chItemProps["DataExchangeType"];

		muxItemProps["PackagingMode"] = chItemProps["PackagingMode"];
		muxItemProps["Path"] = chItemProps["Path"];
		muxItem->setProperies(muxItemProps);
#endif
	
	} catch (Ice::ObjectNotExistException) {
		// throw StreamerException();
		muxItem = NameToMuxItem(DataPublisherImpl::_ic)(
			DataPublisherImpl::_strmSvcEndPoint, configSpaceName, 
			myInfo.name, channelName);
	} catch (DataOnDemand::NameDupException& ) {

		throw ObjectExistException();
	}

	std::pair<AttachedInfoDict::iterator, bool> ir;
	AttachedInfo ai;
	ai.channel = channelPrx;
	ai.minBitRate = minBitRate;
	ai.repeatTime = repeatTime;

	ir = myChannels.insert(
		AttachedInfoDict::value_type(channelName, ai));

	/*
	if (!ir.second) {
		muxItem->destroy();
		throw ObjectExistException();
	}
	*/

	if (chType == dodLocalFolder || chType == dodSharedFolder) {
		
		try {
			folderChannelPrx->linkDest(myInfo.name, 
				getThisProxy(current.adapter), 0);
		} catch(ObjectExistException& ) {

			throw ObjectExistException();
/*			try {

				// the data structure isn't consistent
				assert(false);
				muxItem->destroy();

			} catch (Ice::Exception) {
				assert(false);
				// print warning;
			}
			myChannels.erase(channelName);*/
		}
	} else if (chType == dodMessage){

		try {
			msgChannelPrx->linkDest(myInfo.name, 
				getThisProxy(current.adapter), 0);
		} catch(ObjectExistException& ) {
			
			throw ObjectExistException();
		/*	try {

				// the data structure isn't consistent
				assert(false);
				muxItem->destroy();

			} catch (Ice::Exception) {
				assert(false);
				// print warning;
			}

			myChannels.erase(channelName);*/
		}
	} else {
		assert(false);
		throw Ice::UnknownException(__FILE__, __LINE__);
	}
}

// impl
void
DataOnDemand::DestinationImpl::detachChannel(const ::std::string& channelName,
					  const Ice::Current& current)
{
	ChannelPublishPointPrx channelPrx = myParent->getChannel(channelName);
	// ChannelType type = channelPrx->getType();

	try {

		DataStreamPrx strm;
		strm = getStream();
		MuxItemPrx muxItem = strm->getMuxItem(channelName);
		muxItem->destroy();

	} catch (Ice::ObjectNotExistException) {

		throw ::TianShanIce::InvalidParameter();
	}

	ChannelType chType = channelPrx->getType();
	
	if (chType == dodLocalFolder || chType == dodSharedFolder) {

		FolderChannelExPrx folderChannelPrx = 
			FolderChannelExPrx ::checkedCast(channelPrx);

		folderChannelPrx->unlinkDest(myInfo.name);

	} else if (chType == dodMessage) {

		MessageChannelExPrx msgChannelPrx = 
			MessageChannelExPrx ::checkedCast(channelPrx);

		msgChannelPrx->unlinkDest(myInfo.name);
	}

	myChannels.erase(channelName);
}

::DataOnDemand::DestInfo
DataOnDemand::DestinationImpl::getInfo(const Ice::Current& current) const
{
    return myInfo;
}

// impl
void
DataOnDemand::DestinationImpl::serve(const Ice::Current& current)
{
	TianShanIce::Streamer::StreamState state;
	DataStreamPrx stream;

	try {

		stream = getStream();
		state = stream->getCurrentState();
	 
	} catch (Ice::Exception& ) {

		throw StreamerException();				
	}

	if (state == TianShanIce::Streamer::stsStreaming) {

		return; // already streamming

	} else if (state == TianShanIce::Streamer::stsStop) {

		throw StreamInvalidState();
	}

	try {

		stream->play();		

	} catch (Ice::Exception& ) {

		throw StreamerException();				
	}

	myState = DestRunning;
}

// impl
void
DataOnDemand::DestinationImpl::stop(const Ice::Current& current)
{
	TianShanIce::Streamer::StreamState state;
	DataStreamPrx stream;

	try {

		stream = getStream();
		state = stream->getCurrentState();

	} catch (Ice::Exception& ) {
		throw StreamerException();
	}

	if (state == TianShanIce::Streamer::stsStop) {

		throw StreamInvalidState();
	}

	try {

		stream->destroy();

	} catch (Ice::Exception& ) {

		throw StreamerException();
	}

	myState = DestStopped;
}

void
DataOnDemand::DestinationImpl::pause(const Ice::Current& current)
{
	throw ::TianShanIce::NotImplemented();
}

void
DataOnDemand::DestinationImpl::destroy(const Ice::Current& current)
{
	DataStreamPrx stream;
	TianShanIce::Streamer::StreamState state;

	/*
	try {

		stream = getStream();
		state = stream->getCurrentState();

	} catch (Ice::Exception& ) {

		throw StreamerException();				
	}

	if (state != TianShanIce::Streamer::stsStop) {

		throw StreamInvalidState();
	}
	*/

	myParent->removeDestination(myInfo.name);
	// stop(current); // must be stopped before destroy
}

void
DataOnDemand::DestinationImpl::setProperies(const ::TianShanIce::Properties& props,
					 const Ice::Current& current)
{
	myProps = props;
}

::TianShanIce::Properties
DataOnDemand::DestinationImpl::getProperties(const Ice::Current& current) const
{
    return myProps;
}

void
DataOnDemand::DestinationImpl::removeChannel(const ::std::string& name,
					    const Ice::Current& current)
{
	myChannels.erase(name);
}


void 
DataOnDemand::DestinationImpl::rebuildMuxItem(DataStreamPrx& strm)
{
	AttachedInfoDict::iterator it;
	ChannelInfo chInfo;
	MuxItemPrx muxItem;

	for (it = myChannels.begin(); it != myChannels.end(); it ++) {
		
		ChannelPublishPointPrx channelPrx;
		AttachedInfo& ai = it->second;
		channelPrx = ai.channel;

		FolderChannelExPrx folderChannelPrx;
		MessageChannelExPrx msgChannelPrx;

		CacheType ctype;
		std::string addr;

		ChannelType chType = channelPrx->getType();

		if (chType == dodLocalFolder || chType == dodSharedFolder) {
			
			folderChannelPrx = FolderChannelExPrx::checkedCast(channelPrx);
			folderChannelPrx->getCacheInfo(ctype, addr);

		} else {

			msgChannelPrx = MessageChannelExPrx::checkedCast(channelPrx);
			msgChannelPrx->getCacheInfo(ctype, addr);
		}

		chInfo = channelPrx->getInfo();

		MuxItemInfo muxItemInfo;
		muxItemInfo.name = chInfo.name;
		muxItemInfo.streamId = chInfo.streamId;
		muxItemInfo.streamType = chInfo.streamType;
		muxItemInfo.bandWidth = ai.minBitRate;
		muxItemInfo.tag = chInfo.tag;
		muxItemInfo.expiration = 0;
		muxItemInfo.repeatTime = ai.repeatTime;

		muxItemInfo.ctype = ctype;
		muxItemInfo.cacheAddr = addr;
		muxItemInfo.encryptMode = chInfo.encrypt;
		muxItemInfo.subchannelCount = chInfo.subchannelCount;
		muxItem = strm->createMuxItem(muxItemInfo);

#ifdef _PHASE1
		::TianShanIce::Properties muxItemProps;
		::TianShanIce::Properties chItemProps;
		chItemProps = channelPrx->getProperties();
		muxItemProps = muxItem->getProperties();
		muxItemProps["Detect"] = chItemProps["Detect"];
		muxItemProps["DetectInterval"] = chItemProps["DetectInterval"];
		muxItemProps["DataExchangeType"] = 
			chItemProps["DataExchangeType"];

		muxItemProps["PackagingMode"] = chItemProps["PackagingMode"];
		muxItemProps["Path"] = chItemProps["Path"];
		muxItem->setProperies(muxItemProps);
#endif

	}
}

void 
DataOnDemand::DestinationImpl::activate(const ::Ice::Current& current)
{
	DataStreamPrx strm;

	try {

		strm = getStream();
		strm->ice_ping();

	} catch(Ice::ObjectNotExistException& ) {

		try {

			StreamInfo strmInfo;
             
			strmInfo.name = myInfo.name;
			strmInfo.totalBandwidth = myInfo.totalBandwidth;
			strmInfo.destAddress = myInfo.destAddress;
			strmInfo.pmtPid = myInfo.pmtPid;
			
			DataOnDemand::DataStreamServicePrx strmService = 
				DataOnDemand::DataPublisherImpl::_dataStreamSvc;

			strm = strmService->createStreamByApp(NULL, 
				configSpaceName, strmInfo);



		} catch(Ice::Exception& e) {

			glog( ZQ::common::Log::L_ERROR,
				"DestinationImpl::activate()\tError:%s", e.ice_name().c_str());
		}
	}

	try {
		rebuildMuxItem(strm);
		if (myState == DestRunning) {
			strm->play();
		}

	} catch (Ice::Exception& ) {

		throw StreamerException();				
	}
}

inline DataOnDemand::DestinationExPrx 
DataOnDemand::DestinationImpl::getThisProxy(
	const Ice::ObjectAdapterPtr& adapter)
{
	if (_thisPrx == NULL) {
		_thisPrx = NameToDestination(adapter)(myInfo.name);
	}
	
	return _thisPrx;
}

inline DataOnDemand::DataStreamPrx 
DataOnDemand::DestinationImpl::getStream()
{
	if (_stream == NULL) {

		Ice::Identity ident = createStreamIdentity(configSpaceName, 
			myInfo.name);

		Ice::ObjectPrx obj = createObjectWithEndPoint(
			DataPublisherImpl::_ic, ident, 
			DataPublisherImpl::_strmSvcEndPoint);

		_stream = DataStreamPrx::checkedCast(obj);
	}

	return _stream;
}

::TianShanIce::StrValues 
DataOnDemand::DestinationImpl::listChannels(const ::Ice::Current&)const
{
	::TianShanIce::StrValues result;
	AttachedInfoDict::const_iterator it;
	
	for (it = myChannels.begin(); it != myChannels.end(); it ++) {
		result.push_back(it->first);
	}
	
    return result;
}

void 
DataOnDemand::DestinationImpl::getAttachedInfo(const ::std::string& chName, 
		::DataOnDemand::AttachedInfo& info, 
		const ::Ice::Current& current)
{
	AttachedInfoDict::iterator it = myChannels.find(chName);
	if (it == myChannels.end())
		throw TianShanIce::InvalidParameter();
	info = it->second;
		
}
void 
DataOnDemand::DestinationImpl::getChannelAttachedInfo(const ::std::string& chName, 
                                 ::Ice::Int& minBitRate, ::Ice::Int& repeatTime, const Ice::Current&)
{
  ::DataOnDemand::AttachedInfo attachedinfo;
  try
  {  
	  getAttachedInfo(chName, attachedinfo, ::Ice::Current());
  
      minBitRate = attachedinfo.minBitRate;
      repeatTime = attachedinfo.repeatTime;
  }
  catch (TianShanIce::InvalidParameter&)
  {
  	  minBitRate = 0;
      repeatTime = 0;
  }

}