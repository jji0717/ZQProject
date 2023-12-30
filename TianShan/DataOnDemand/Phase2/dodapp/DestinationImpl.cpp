#include "stdafx.h"
#include "DODAppImpl.h"
#include "DestinationImpl.h"
#include "DataPublisherImpl.h"
#include "util.h"
#include "global.h"
#include "DODAppThread.h"
#include "..\common\Reskey.h"
using namespace ZQ::common;

DataOnDemand::DestinationImpl::DestinationImpl()
{
	try
	{	
		if(!mySessionId.empty())
		{
			TianShanIce::SRM::SessionPrx sessionprx = 
				DataOnDemand::DataPublisherImpl::_sessManager->
											openSession(mySessionId);
			
			_stream = DataOnDemand::DataStreamPrx::checkedCast(
											sessionprx->getStream());
		}
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_WARNING, "[DestinationName: %s]"
			"get streamPrx error",myInfo.name.c_str());
		mySessionId = "";
		_stream = NULL;
	}

	_thisPrx = NULL;
	myState = DestInit;
}

DataOnDemand::DestinationImpl::~DestinationImpl()
{
/*	glog(Log::L_DEBUG,"[DestinationName: %s]~DestinationImpl():delete Object\
		success!", myInfo.name);*/
	_thisPrx = NULL;
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
	glog( ZQ::common::Log::L_INFO,
		"DestinationImpl::attachChannel() destination (%s)attach channel(%s)", 
		myInfo.name.c_str(), channelName.c_str());

	ChannelPublishPointPrx channelPrx;
	ChannelInfo chInfo;
	MuxItemPrx muxItem;

	try {

		channelPrx = myParent->getChannel(channelName);
		chInfo = channelPrx->getInfo();

	} catch (Ice::ObjectNotExistException&) {
		glog( ZQ::common::Log::L_INFO,
			"DestinationImpl::attachChannel() can't get channel(%s) info", 
			channelName.c_str());
		throw ::TianShanIce::InvalidParameter();
	}


	if (myChannels.find(channelName) != myChannels.end()) {

		glog( ZQ::common::Log::L_INFO,
			"DestinationImpl::attachChannel() channel(%s) object already exist", 
			channelName.c_str());
		throw ObjectExistException();
	}

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

	DataStreamPrx mystream = getStreamPrx();
	if(mystream == NULL)
	{
//		throw StreamerException();
		glog( ZQ::common::Log::L_WARNING,
			"DestinationImpl::attachChannel() [destname: %s]stream porxy is NULL," 
			"retry create this stream!", 
			myInfo.name.c_str());
		return;
	}

	try {
		MuxItemInfo muxItemInfo;
		muxItemInfo.name = chInfo.name;
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
		muxItem = mystream->createMuxItem(muxItemInfo);
		if(	muxItem == NULL)
		{
			glog( ZQ::common::Log::L_ERROR,
				"DestinationImpl::attachChannel() create MuxItem error,please check configration");
				throw Ice::ObjectNotExistException(__FILE__,__LINE__);
		}
	
	} catch (Ice::ObjectNotExistException) {
		 throw StreamerException();
	} catch (DataOnDemand::NameDupException& ) {

		throw ObjectExistException();
	}
	catch(Ice::Exception &e)
	{
		glog( ZQ::common::Log::L_ERROR,
			"DestinationImpl::attachChannel() create MuxItem caught error at Ice::Exception(%s)",
			e.ice_name().c_str());
		throw StreamerException();
	}
	catch(...)
	{		
		glog( ZQ::common::Log::L_ERROR,
			"DestinationImpl::attachChannel() create MuxItem caught unknown exception(%d)",
			GetLastError());
		throw StreamerException();
	}

	std::pair<AttachedInfoDict::iterator, bool> ir;
	AttachedInfo ai;
	ai.channel = channelPrx;
	ai.minBitRate = minBitRate;
	ai.repeatTime = repeatTime;

	ir = myChannels.insert(
		AttachedInfoDict::value_type(channelName, ai));

	if (chType == dodLocalFolder || chType == dodSharedFolder)
	{	
		std::string  myContentName = "";
		std::string  _contentpath = "";
		if(chType == dodSharedFolder)
		{
			try
			{	
				myContentName = folderChannelPrx->
										getContentName(myInfo.groupId);
			}
			catch (TianShanIce::InvalidParameter&)
			{
				folderChannelPrx->setContentName(myInfo.groupId,myContentName);	
			}
		}
		else
		{
			try
			{	
				myContentName = folderChannelPrx->getContentName(0);

				::TianShanIce::Storage::ContentPrx contentprx;
				
				contentprx=  DataOnDemand::DataPublisherImpl::_contentStroe->
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
				 
				 glog(ZQ::common::Log::L_INFO, 
					 "DestinationImpl::attachChannel() [%s,%s]Local folderChannel Update will start", 
					 myInfo.name.c_str(), channelName.c_str());
				 
				 muxItem->notifyFullUpdate(_contentpath);

				 glog(ZQ::common::Log::L_INFO, 
					 "DestinationImpl::attachChannel() [%s,%s]Local folderChannel Update operation end" ,
					 myInfo.name.c_str(), channelName.c_str());				 
			}
			catch (TianShanIce::InvalidParameter&)
			{
				folderChannelPrx->setContentName(0, myContentName);	
			}
			catch (::TianShanIce::InvalidStateOfArt&ex) {
				glog(ZQ::common::Log::L_ERROR,
					"DestinationImpl::attachChannel() caught "
					"TianShanIce::InvalidStateOfArt(%s)",
					ex.message.c_str());
			}
			catch(::TianShanIce::NotImplemented&ex){
				glog(ZQ::common::Log::L_ERROR,
					"DestinationImpl::attachChannel() caught "
					"TianShanIce::NotImplemented(%s)",
					ex.message.c_str());
			}
			catch(const Ice::ObjectNotExistException&) 
			{
				glog(ZQ::common::Log::L_CRIT, 
					"DestinationImpl::attachChannel() can't connect to streamer");
			}
			catch (const ::Ice::Exception & ex) 
			{
				glog(ZQ::common::Log::L_ERROR,
					"DestinationImpl::attachChannel() caught Ice::Exception (%s)",
					ex.ice_name().c_str());
			} 
		}
		try
		{
			folderChannelPrx->linkDest(myInfo.name, 
											getThisProxy(current.adapter), 0);
		} 
		catch(ObjectExistException& )
		{
			throw ObjectExistException();
		}
	} 
	else 
		if (chType == dodMessage)
		{			
			try 
			{
				msgChannelPrx->linkDest(myInfo.name, 
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

		glog( ZQ::common::Log::L_INFO,
			"DestinationImpl::attachChannel() Leave destination (%s)attach channel(%s)",
			myInfo.name.c_str(), channelName.c_str());
}

// impl
void
DataOnDemand::DestinationImpl::detachChannel(const ::std::string& channelName,
											 const Ice::Current& current)
{   
	glog(ZQ::common::Log::L_INFO,
		"DestinationImpl::detachChannel() Enter destination (%s) detach channel(%s) ",
		myInfo.name.c_str(), channelName.c_str());

	ChannelPublishPointPrx channelPrx = myParent->getChannel(channelName);


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

	glog(ZQ::common::Log::L_INFO,
		"DestinationImpl::detachChannel() destination (%s)detach channel(%s) success ",
		myInfo.name.c_str(), channelName.c_str());
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

	DataStreamPrx mystream = getStreamPrx();
	if(mystream == NULL)
	{
		glog( ZQ::common::Log::L_WARNING,
			"DestinationImpl::serve() [DestName: %s]Stream Porxy is NULL ,retry create this stream", 
			myInfo.name.c_str());
		return;
	}

	try {
		state = mystream->getCurrentState();	 
	} catch (Ice::Exception& ) {

		throw StreamerException();				
	}

	if (state == TianShanIce::Streamer::stsStreaming) {
		glog( ZQ::common::Log::L_INFO,
			"DestinationImpl::serve() [DestName: %s]streamer already play", 
			myInfo.name.c_str());

		return; // already streamming

	} else if (state == TianShanIce::Streamer::stsStop) {

		throw StreamInvalidState();
	}

	try {  
		bool bplay = mystream->play();
		
		if(bplay)
		{			
			myState = DestRunning;

			glog(ZQ::common::Log::L_INFO,
				"DestinationImpl::serve() get stream(%s) sessionId (%s)",
				myInfo.name.c_str(), mySessionId.c_str());

			TianShanIce::SRM::SessionPrx sessionprx = 
				DataOnDemand::DataPublisherImpl::_sessManager->
												openSession(mySessionId);

			SessionRenewThread * pThread =  
				new SessionRenewThread(sessionprx, myInfo.name);
			
			if(pThread)
			{
				pThread->start();
				DataOnDemand::DataPublisherImpl::_SessionTrdMap[myInfo.name] = pThread;
				pThread = NULL;
			}
		}
		
		else
		{
			glog(ZQ::common::Log::L_ERROR,
				"DestinationImpl::serve()[DestName: %s] play streamer error."
				"may be sum of channelRate over Port TotalRate or Stream status error",
				myInfo.name.c_str());
			
			stop(Ice::Current());
			destroy(Ice::Current());
			Beep(700, 1000);		
			throw StreamerException();	
		}
	} catch (Ice::Exception& ex) {
		glog(ZQ::common::Log::L_ERROR,
			"DestinationImpl::serve() caught Ice::Exception(%s)",
			ex. ice_name().c_str());

		throw StreamerException();				
	}
 
}

// impl
void
DataOnDemand::DestinationImpl::stop(const Ice::Current& current)
{
	TianShanIce::Streamer::StreamState state;
    
	DataStreamPrx mystream = getStreamPrx();
	if(mystream != NULL)
	{
		try {
			state = _stream->getCurrentState();
			
		} catch (Ice::Exception& ) {
			throw StreamerException();
		}
		
		if (state == TianShanIce::Streamer::stsStop) {
			
			throw StreamInvalidState();
		}
		
		try {
			
			mystream->destroy();
			
		} catch (Ice::Exception& ) {
			
			throw StreamerException();
		}
		catch(...)
		{
			glog(ZQ::common::Log::L_ERROR,
				"DestinationImpl::stop() unknown exception!");
			throw StreamerException();
		}
		myState = DestStopped;
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR,
			"DestinationImpl::stop()  StreamPrx is NULL!");
	}
}

void
DataOnDemand::DestinationImpl::pause(const Ice::Current& current)
{
	throw ::TianShanIce::NotImplemented();
}

void
DataOnDemand::DestinationImpl::destroy(const Ice::Current& current)
{
	TianShanIce::SRM::SessionPrx sessionprx ;
	
	try
	{	
		glog(ZQ::common::Log::L_INFO,
			"DestinationImpl::destroy() get stream(%s) sessionId (%s)",
			myInfo.name.c_str(), mySessionId.c_str());

		sessionprx = 
			DataOnDemand::DataPublisherImpl::_sessManager->openSession(mySessionId);
		if(sessionprx)
		{
			sessionprx->destroy();
		}
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,
			"DestinationImpl::destroy() session destroy failur");	
	}

	SessionRenewThread *pThread = 
		DataOnDemand::DataPublisherImpl::getSessionTrd(myInfo.name);
	if(pThread)
	{
		pThread->stop();
		DataOnDemand::DataPublisherImpl::_SessionTrdMap.erase(myInfo.name);
	}

	myParent->removeDestination(myInfo.name);

	AttachedInfoDict::iterator iter;
	for(iter =myChannels.begin(); iter != myChannels.end(); iter++)
	{	
		ChannelPublishPointPrx channelPrx = myParent->getChannel(iter->first);
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
	}

	myChannels.clear();
	_stream = NULL;
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
	try
	{	
		::DataOnDemand::AttachedInfoDict::iterator itor =  myChannels.find(name);
		if(itor != myChannels.end())
				myChannels.erase(name);
		else
		{
			glog(ZQ::common::Log::L_INFO,
				"DestinationImpl::removeChannel() can't find channel(%s) in destination (%s)",
				name.c_str(), myInfo.name.c_str());
		}
	}
	catch (...)
	{
		
	}

}

void 
DataOnDemand::DestinationImpl::rebuildMuxItem(DataStreamPrx& strm)
{
	AttachedInfoDict::iterator it;
	ChannelInfo chInfo;
	MuxItemPrx muxItem;
	std::string myContentName = "";

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
		try
		{
			muxItem = strm->createMuxItem(muxItemInfo);
		}
		catch (DataOnDemand::DataStreamError &ex)
		{
			glog( ZQ::common::Log::L_ERROR,
				"DestinationImpl::rebuildMuxItem() caught DataOnDemand::DataStreamError (%s)",
				ex.message.c_str());
			throw StreamerException();
		}
		catch (TianShanIce::InvalidParameter &ex)
		{
			glog( ZQ::common::Log::L_ERROR,
				"DestinationImpl::rebuildMuxItem() caught TianShanIce::InvalidParameter(%s)",
				ex.ice_name().c_str());	
			throw StreamerException();
		}
		catch(Ice::Exception &e)
		{
			glog( ZQ::common::Log::L_ERROR,
				"DestinationImpl::rebuildMuxItem() caught Ice::Exception (%s)",
				e.ice_name().c_str());
			throw StreamerException();
		}
	}
}

void 
DataOnDemand::DestinationImpl::activate(const ::Ice::Current& current)
{
	TianShanIce::Streamer::StreamPrx strmprx;
	TianShanIce::SRM::SessionPrx sessionprx = NULL;
	try
	{
		glog(ZQ::common::Log::L_INFO,
			"DestinationImpl::activate() get stream(%s) sessionId(%s)",
			myInfo.name.c_str(), mySessionId.c_str());

		if(!mySessionId.empty())
      	sessionprx = DataOnDemand::DataPublisherImpl::_sessManager->
													openSession(mySessionId);
	}
    catch (...)
	{
		glog( ZQ::common::Log::L_WARNING,
			"DestinationImpl::activate() [DestName: %s] can't get session proxy, try to re-create stream",
			myInfo.name.c_str());
		sessionprx = NULL;
		mySessionId="";
    }
	if(sessionprx)
	{
		try
		{	
			_stream = DataOnDemand::DataStreamPrx::checkedCast(
				sessionprx->getStream());
			//if(myChannels.size()  >0)
            if(_stream)
			{				
				if (myState == DestRunning)
				{
					glog( ZQ::common::Log::L_INFO,
						"DestinationImpl::activate()[DestName: %s] Streamer already play", 
						myInfo.name.c_str());
					//    _stream->play();
				}
				
				SessionRenewThread * pThread =  
					new SessionRenewThread(sessionprx, myInfo.name);
				
				if(pThread)
				{
					pThread->start();
					DataOnDemand::DataPublisherImpl::_SessionTrdMap[myInfo.name] = pThread;
					pThread = NULL;
				}
				return;	
			}
			else
			{
				sessionprx->destroy();
				glog( ZQ::common::Log::L_WARNING,
					"DestinationImpl::activate() can't get stream proxy, retry create stream" );
				
				sessionprx = NULL;
				mySessionId="";
			}
		}
		catch (TianShanIce::ServerError &e)
		{
			sessionprx->destroy();
			glog( ZQ::common::Log::L_WARNING,
			"DestinationImpl::activate() caught error at TianShanIce::ServerError(%s)", 
			e.ice_name().c_str());

			sessionprx = NULL;
			mySessionId="";
		}
		catch(Ice::Exception &e)
		{
			sessionprx->destroy();
			glog( ZQ::common::Log::L_WARNING,
				"DestinationImpl::activate() caught error at Ice::Exception (%s)",
				e.ice_name().c_str());
			sessionprx = NULL;
			mySessionId="";
		}
	}
	try
	{
		glog( ZQ::common::Log::L_WARNING,
			"DestinationImpl::activate() [DestName: %s] create stream",
			myInfo.name.c_str());
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

        sessionprx = DataOnDemand::DataPublisherImpl::_sessManager->
												createSession(clientResource);

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
        var.lints.push_back(myInfo.totalBandwidth);
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
		strvalues.push_back(myInfo.name);
		var.strs = strvalues;
        sessionprx->setPrivateData(RESKEY_STREAMNAME, var);

		var.type = ::TianShanIce::vtStrings;
		var.bRange = false;
		strvalues.clear();
		strvalues.push_back(myInfo.destAddress);
		var.strs = strvalues;
        sessionprx->setPrivateData(RESKEY_DESTADDRESS, var);

		var.type = ::TianShanIce::vtInts;
		var.bRange = false;
		vtints.clear();
		vtints.push_back(myInfo.groupId);
		var.ints = vtints;
		sessionprx->setPrivateData(RESKEY_GROUPID, var);

		var.type = ::TianShanIce::vtInts;
		var.bRange = false;
		vtints.clear();
		vtints.push_back(myInfo.pmtPid);
		var.ints = vtints;

       sessionprx->setPrivateData(RESKEY_PMTPID, var);

		var.type = ::TianShanIce::vtInts;
		var.bRange = false;
		vtints.clear();
		vtints.push_back(myInfo.totalBandwidth);
		var.ints = vtints;

		glog(ZQ::common::Log::L_INFO,
			"DestinationImpl::activate() set privateData");

		sessionprx->setPrivateData(RESKEY_TOTALBANDWIDTH, var);

		glog(ZQ::common::Log::L_INFO,
			"DestinationImpl::activate() provision");
		sessionprx->provision();

		glog(ZQ::common::Log::L_INFO,
			"DestinationImpl::activate() serve");
		sessionprx->serve();

		glog(ZQ::common::Log::L_INFO,
			"DestinationImpl::activate() renew (%d)", gDODAppServiceConfig.lRenewtime * 1000);
		sessionprx->renew(gDODAppServiceConfig.lRenewtime * 1000);

		mySessionId = sessionprx->getId();
		glog(ZQ::common::Log::L_INFO,
			"DestinationImpl::activate() get stream(%s)  new sessionId (%s)",
			myInfo.name.c_str(), mySessionId.c_str());
		glog(ZQ::common::Log::L_INFO,
			"DestinationImpl::activate() set privateData");
		streamprx = sessionprx->getStream();

		glog(ZQ::common::Log::L_INFO,
			"DestinationImpl::activate() checkedCast");
		_stream = DataOnDemand::DataStreamPrx::checkedCast(streamprx);

		glog( ZQ::common::Log::L_INFO,
			"DestinationImpl::activate() [DestName: %s] create stream success",
			myInfo.name.c_str());
	   }
	catch (TianShanIce::SRM::InvalidResource&ex) 
	{
		glog(ZQ::common::Log::L_WARNING,
			"DestinationImpl::activate() TianShanIce::SRM::InvalidResource(%s)",
			ex.message.c_str());
		throw StreamerException();
	}
	catch(TianShanIce::NotSupported&ex)
	{
		glog(ZQ::common::Log::L_WARNING,
			"DestinationImpl::activate() TianShanIce::NotSupported(%s)",
			ex.message.c_str());
		throw StreamerException();
	}
	catch(TianShanIce::InvalidParameter&ex)
	{
		glog(ZQ::common::Log::L_WARNING,
			"DestinationImpl::activate() TianShanIce::InvalidParameter(%s)",
			ex.message.c_str());
		throw StreamerException();
	}
	catch(TianShanIce::ServerError&ex)
	{
		glog(ZQ::common::Log::L_WARNING,
			"DestinationImpl::activate() TianShanIce::ServerError(%s)",
			ex.message.c_str());
		throw StreamerException();
	}
	
	catch(Ice::Exception& ex)
	{
		
		glog( ZQ::common::Log::L_WARNING,
			"DestinationImpl::activate() caught error at ICE exception (%s)", ex.ice_name().c_str());
		throw StreamerException();
	}

	try
	{
		rebuildMuxItem(_stream);
	}
	catch (DataOnDemand::StreamerException& )
	{	
		try
		{
			_stream->destroy();
			sessionprx->destroy();	
		}
		catch (Ice::Exception&ex) 
		{
			glog(ZQ::common::Log::L_WARNING,
				"DestinationImpl::activate() [DestName: %s] rebuildMuxItem (%s) error",
				myInfo.name.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			glog(ZQ::common::Log::L_WARNING,
				"DestinationImpl::activate() [DestName: %s] rebuildMuxItem (%d) error",
				myInfo.name.c_str(), GetLastError());
		}
		throw StreamerException();	
	}
	try 
	{
		if (myState == DestRunning)
		{
			bool bplay = _stream->play();
			if(bplay)
			{
				SessionRenewThread * pThread =  
					new SessionRenewThread(sessionprx, myInfo.name);
				
				if(pThread)
				{
					pThread->start();
					DataOnDemand::DataPublisherImpl::_SessionTrdMap[myInfo.name] = pThread;
					pThread = NULL;
				}
			}
			else
			{	
				try
				{
					_stream->destroy();
					sessionprx->destroy();	
				}
				catch (Ice::Exception&ex) 
				{
					glog(ZQ::common::Log::L_WARNING,
						"DestinationImpl::activate() [DestName: %s] play stream error(%s)",
						myInfo.name.c_str(), ex.ice_name().c_str());
				}
				catch(...)
				{
					glog(ZQ::common::Log::L_WARNING,
						"DestinationImpl::activate() [DestName: %s] play stream error(%d)",
						myInfo.name.c_str(), GetLastError());
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

inline DataOnDemand::DestinationExPrx 
DataOnDemand::DestinationImpl::getThisProxy(
	const Ice::ObjectAdapterPtr& adapter)
{
	if (_thisPrx == NULL) {
		_thisPrx = NameToDestination(adapter)(myInfo.name);
	}
	
	return _thisPrx;
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
::std::string 
DataOnDemand::DestinationImpl::getSessionId(const Ice::Current&)
{
	return mySessionId;
}

void DataOnDemand::DestinationImpl::notifyMuxItem()
{
	AttachedInfoDict::iterator it;
	ChannelInfo chInfo;
	MuxItemPrx muxItem;
	std::string myContentName = "";
	std::string _contentpath;

	DataStreamPrx mystream = getStreamPrx();
	if(mystream == NULL)
	{
		throw StreamerException();
	}

	for (it = myChannels.begin(); it != myChannels.end(); it ++)
	{		
		ChannelPublishPointPrx channelPrx;
		AttachedInfo& ai = it->second;
		channelPrx = ai.channel;
		
		FolderChannelExPrx folderChannelPrx;
		
		ChannelType chType = channelPrx->getType();

		folderChannelPrx = FolderChannelExPrx::checkedCast(channelPrx);

		if (chType == dodSharedFolder)
		{	
			try
			{
				myContentName = folderChannelPrx->getContentName(myInfo.groupId);
			}
			catch (TianShanIce::InvalidParameter&)
			{
				glog(ZQ::common::Log::L_ERROR, "DestinationImpl::notifyMuxItem()[DestinationName: %s]"
					" ContentName is not exist!",
					myInfo.name.c_str());
				continue;
			}
			catch (Ice::Exception & ex)
			{
				glog(ZQ::common::Log::L_ERROR, "DestinationImpl::notifyMuxItem()[DestinationName: %s]"
					" Ice::Exception (%s)!",
					ex.ice_name().c_str());
				continue;
			}
		}
		else
			if(chType == dodLocalFolder)
			{
				try
				{
					myContentName = folderChannelPrx->getContentName(0);
				}
				catch (TianShanIce::InvalidParameter&)
				{
					glog(ZQ::common::Log::L_ERROR, "DestinationImpl::notifyMuxItem()[DestinationName: %s]"
						" ContentName is not exist!",
						myInfo.name.c_str());
					continue;
				}
			}
			else
			{
			/*	ActiveChannel* activeChannel = activeChannelManager.get(chInfo.name);
				if(activeChannel)
				{

				}*/
				continue;
			}
			
			if(!myContentName.empty())
			{
				::TianShanIce::Storage::ContentPrx contentprx;
				
				try
				{
					contentprx=  DataOnDemand::DataPublisherImpl::_contentStroe->
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
					DataOnDemand::MuxItemPrx muxitemprx ;
					muxitemprx = mystream->getMuxItem((*it).first);
					
					glog(ZQ::common::Log::L_INFO, 
						"DestinationImpl::notifyMuxItem()[%s,%s]UpdateChannel will start!", 
						myInfo.name.c_str(), (*it).first.c_str());

					glog(ZQ::common::Log::L_INFO, 
						"DestinationImpl::notifyMuxItem():contentpath = %s!", _contentpath.c_str());
					muxitemprx->notifyFullUpdate(_contentpath);
					
					glog(ZQ::common::Log::L_INFO, 
						"DestinationImpl::notifyMuxItem()[%s, %s]Update operation end!",
						myInfo.name.c_str(),(*it).first.c_str());					
				}
				catch (::TianShanIce::InvalidParameter&ex){	
					glog(ZQ::common::Log::L_DEBUG,
						"DestinationImpl::notifyMuxItem() caught "
						"TianShanIce::InvalidParameter(%s)",
						ex.message.c_str());
				}
				catch (::TianShanIce::InvalidStateOfArt&ex) {
					glog(ZQ::common::Log::L_DEBUG,
						"DestinationImpl::notifyMuxItem() caught "
						"TianShanIce::InvalidStateOfArt(%s)",
						ex.message.c_str());
				}
				catch(::TianShanIce::NotImplemented&ex){
					glog(ZQ::common::Log::L_DEBUG,
						"DestinationImpl::notifyMuxItem() caught "
						"TianShanIce::NotImplemented(%s)",
						ex.message.c_str());
				}
				catch(const Ice::ObjectNotExistException&) 
				{
					glog(ZQ::common::Log::L_CRIT, 
						"DestinationImpl::notifyMuxItem() can't connect to streamer");
				}
				catch (const ::Ice::Exception & ex) 
				{
					glog(ZQ::common::Log::L_ERROR,
						"DestinationImpl::notifyMuxItem() caught Ice::Exception (%s)",
						ex.ice_name().c_str());
				} 
			}
		}			
}
DataOnDemand::DataStreamPrx 
DataOnDemand::DestinationImpl::getStreamPrx()
{
	try
	{	
		if(!mySessionId.empty())
		{		
			glog(ZQ::common::Log::L_INFO,
				"DestinationImpl::getStreamPrx() get destination (%s) sessionID(%s)",
				myInfo.name.c_str(), mySessionId.c_str());

			TianShanIce::SRM::SessionPrx sessionprx = 
				DataOnDemand::DataPublisherImpl::_sessManager->
				openSession(mySessionId);
			
			_stream = DataOnDemand::DataStreamPrx::checkedCast(
				sessionprx->getStream());
		}
		else
			_stream = NULL;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, "DestinationImpl::getStreamPrx() [DestName: %s]"
			"fail to  get stream proxy",
			myInfo.name.c_str());
		return NULL;
	}
	return _stream;
}
