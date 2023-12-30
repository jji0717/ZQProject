
// for #include "DODServerController.h"
#include <afx.h>
#include "scqueue.h"
#include <list>
#include "DODServerController.h"
#include "DeviceInfoParser.h"
#include "ListenManager.h"
#include "mmsystem.h"
#pragma comment(lib, "Winmm.lib")

//////////////////////////////////////////////////////////////////////////

inline CDODDeviceController* getDeviceManager() 
{
	return &CDeviceInfoParser::s_pDeviceInfoParser->m_DeviceManager;
}

void split( const std::string& src, char delimiter, std::vector<std::string>& result)
{
	std::string::const_iterator it, beginPos = src.begin();
	for (it = src.begin(); it != src.end(); it ++) {
		if (*it == delimiter) {
			std::string str(beginPos, it);
			beginPos = it + 1;
			result.push_back(str);
		}
	}

	std::string str(beginPos, it);
	result.push_back(str);
}

//////////////////////////////////////////////////////////////////////////

#include <DataStreamImpl.h>
#include "svclog.h"
void glog(ISvcLog::LogLevel level, const char* fmt, ...);

//////////////////////////////////////////////////////////////////////////

Ice::ObjectPtr DataOnDemand::StreamerFactory::create(
	const std::string& type)
{
	if (type == "::DataOnDemand::DataStreamServiceEx") {
		DataStreamServiceImpl* service = 
			new DataOnDemand::DataStreamServiceImpl();

		return service;

	} if (type == "::DataOnDemand::DataStreamEx") {

		DataStreamImpl* strm = new DataStreamImpl(DataStreamServiceImpl::_evictor);
		return strm;

	} else if (type == "::DataOnDemand::MuxItemEx") {
		MuxItemImpl* muxItem = new MuxItemImpl(DataStreamServiceImpl::_evictor);
		return muxItem;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////

::Ice::Identity DataOnDemand::createStreamIdentity(const std::string& space, 
                                                   const std::string& name)
{
	Ice::Identity ident;
	ident.category = space;
	ident.name = name;
	return ident;
}

::Ice::Identity DataOnDemand::createMuxItemIdentity(
	const std::string& space, 
	const std::string& strmName, 
	const std::string& itemName)
{
	Ice::Identity ident;
	ident.category = space;
	ident.name = itemName + "\\/" + strmName;
	return ident;
}

//////////////////////////////////////////////////////////////////////////

DataOnDemand::MuxItemImpl::MuxItemImpl(::Freeze::EvictorPtr evictor):
	_evictor(evictor)
{

}

DataOnDemand::MuxItemImpl::~MuxItemImpl()
{

}

bool DataOnDemand::MuxItemImpl::init(const MuxItemInfo& info)
{
	return true;
}

::std::string
DataOnDemand::MuxItemImpl::getName(const Ice::Current& current) const
{
    return myInfo.name;
}

void
DataOnDemand::MuxItemImpl::notifyFullUpdate(const ::std::string&, 
											const Ice::Current& current)
{
	::TianShanIce::Properties props = myParent->getProperties();
	unsigned long sessionId = atoi(props["SessionId"].c_str());
	getDeviceManager()->UpdateCatalog(sessionId, myInfo.streamId);
}

void
DataOnDemand::MuxItemImpl::notifyFileAdded(const ::std::string& fileName,
					const Ice::Current& current)
{
	throw TianShanIce::NotImplemented(
		"DODStreamer don't support this operation");
}

void
DataOnDemand::MuxItemImpl::notifyFileDeleted(const ::std::string& fileName,
					  const Ice::Current& current)
{
	throw TianShanIce::NotImplemented(
		"DODStreamer don't support this operation");
}

::DataOnDemand::MuxItemInfo
DataOnDemand::MuxItemImpl::getInfo(const Ice::Current& current) const
{
    return myInfo;
}

void
DataOnDemand::MuxItemImpl::destroy(const Ice::Current& current)
{
	myParent->removeMuxItem(myInfo.name);
	_evictor->remove(current.id);
}

void 
DataOnDemand::MuxItemImpl::setProperies(const ::TianShanIce::Properties& props, 
										const ::Ice::Current& current)
{
	myProps = props;
}

::TianShanIce::Properties 
DataOnDemand::MuxItemImpl::getProperties(const ::Ice::Current& current) const
{
	return myProps;
}


void 
DataOnDemand::MuxItemImpl::activate(const ::Ice::Current& current)
{

}

//////////////////////////////////////////////////////////////////////////

DataOnDemand::DataStreamImpl::DataStreamImpl(
	::Freeze::EvictorPtr evictor):
	_evictor(evictor)
{

}

DataOnDemand::DataStreamImpl::~DataStreamImpl()
{

}

bool 
DataOnDemand::DataStreamImpl::init(const ::DataOnDemand::StreamInfo& info, unsigned long sessionId)
{
	myInfo = info;
	setSessionId(sessionId);
	myState = ::TianShanIce::Streamer::stsSetup;
	return true;
}

void 
DataOnDemand::DataStreamImpl::setSessionId(unsigned long sessionId)
{
	char buf[128];
	sprintf(buf, "%ul", sessionId);
	myProps["SessionId"] = buf;
}

unsigned long 
DataOnDemand::DataStreamImpl::getSessionId()
{
	return atol(myProps["SessionId"].c_str());
}

// implemented
void
DataOnDemand::DataStreamImpl::allotPathTicket(
	const TianShanIce::Transport::PathTicketPrx& ticket,
	const Ice::Current& current)
{
	throw TianShanIce::NotImplemented(
		"DODStreamer don't support this operation");
}

// implemented
void
DataOnDemand::DataStreamImpl::destroy(const Ice::Current& current)
{
	if (myState == ::TianShanIce::Streamer::stsStreaming) {
	
		DWORD result = getDeviceManager()->StopPort( getSessionId());
		result = getDeviceManager()->ClosePort(getSessionId());

		if (result) {
			glog(ISvcLog::L_ERROR, "%s:\tStopPort() failed.\n", __FUNCTION__);
			throw DataStreamError("StopPort failed.");
		}

		MuxItemExDict muxItems = myMuxItems;
		MuxItemExDict::iterator it;
		for (it =  muxItems.begin(); it != muxItems.end(); it ++) {

			it->second->destroy();
		}

		// myState = ::TianShanIce::Streamer::stsStop;

		myParent->removeStream(mySpace, myInfo.name);
		_evictor->remove(current.id);
	} else {
		throw Ice::Exception(__FILE__, __LINE__);
	}
}

// implemented
::std::string
DataOnDemand::DataStreamImpl::lastError(const Ice::Current& current) const
{
	return _lastError;
}

// implemented
::Ice::Identity
DataOnDemand::DataStreamImpl::getIdent(const Ice::Current& current) const
{
	return ident; // id is member of class Stream;
}

void DataOnDemand::DataStreamImpl::setConditionalControl(
	const ::TianShanIce::Streamer::ConditionalControlMask& mask,
	const ::TianShanIce::Streamer::ConditionalControlPrx& condCtrl, 
	const ::Ice::Current& current)
{

}

bool
DataOnDemand::DataStreamImpl::play(const Ice::Current& current)
{
	PPortInfo portInfo;
	int destCount = 0;

	portInfo.wPmtPID = myInfo.pmtPid;
	portInfo.wTotalRate = myInfo.totalBandwidth;
	portInfo.szTempPath[0] = 0;
	// strcpy(portInfo.szTempPath, myProps["TmpPath"].c_str());

	// process address

	std::vector<std::string> addressList;
	split(myInfo.destAddress, ';', addressList);
    
	if (addressList.size() <= 0) {
		glog(ISvcLog::L_ERROR, "%s invalid destAddress %s", __FUNCTION__, 
			myInfo.destAddress.c_str());
		throw ::TianShanIce::InvalidParameter();
	}

  /*
	::TianShanIce::Properties svcProps;
	svcProps = myParent->getProperties();
  */

	{
		std::vector<std::string>::iterator it;
		std::string ip, port;
		std::vector<std::string> addr;
		ZQSBFIPPORTINF tmpIPPort;
		
    /*
		std::string srcIP = svcProps["SourceIP"];
		unsigned short srcPort = atoi(svcProps["SourcePort"].c_str());
    */

		for (it = addressList.begin(); it != addressList.end(); it ++) {
			destCount ++;

			strncpy(tmpIPPort.cSourceIp, 
				CListenManager::s_ListenManager->m_sysConfig.cAddr,
				sizeof(tmpIPPort.cSourceIp));

			tmpIPPort.wSourcePort = 0;

			addr.clear();
			split(*it, ':', addr);

			if (addr.size() != 3) {
				continue;
				// throw ::TianShanIce::InvalidParameter();
			}

			if (addr[0] == "TCP") {
				tmpIPPort.wSendType = 0;

			} else if (addr[0] == "UDP") {

				tmpIPPort.wSendType = 1;
			} else if (addr[0] == "MULTICAST") {

				tmpIPPort.wSendType = 2;
			
			} else if (addr[0] == "BROADCAST") {

				tmpIPPort.wSendType = 3;
			}

			strncpy(tmpIPPort.cDestIp, addr[1].c_str(), 
				sizeof(tmpIPPort.cDestIp));
			tmpIPPort.wDestPort = atoi(addr[2].c_str());
			
			portInfo.lstIPPortInfo.push_back(tmpIPPort);
		}
	}

	int index = 0;

	// process items
	{
		DataOnDemand::MuxItemExDict::iterator it;
		MuxItemInfo itemInfo;
		PPChannelInfo tmpChannel;
		::TianShanIce::Properties itemProps;

		for (it = myMuxItems.begin(); it != myMuxItems.end(); it ++) {

			itemInfo = it->second->getInfo();			
			itemProps = it->second->getProperties();			

			tmpChannel.wPID = itemInfo.streamId;
			tmpChannel.wRepeatTime = itemInfo.repeatTime;
			strncpy(tmpChannel.cDescriptor, (char* )&itemInfo.tag, 
				sizeof(tmpChannel.cDescriptor));
			tmpChannel.wRate = itemInfo.bandWidth;
			tmpChannel.nStreamType = itemInfo.streamType;

			tmpChannel.bBeDetect = atoi(itemProps["Detect"].c_str());
			tmpChannel.wBeDetectInterval = atoi(itemProps["DetectInterval"].c_str());
			tmpChannel.wBeEncrypted = itemInfo.encryptMode;

			tmpChannel.wDataExchangeType = atoi(itemProps["DataExchangeType"].c_str());
			tmpChannel.wStreamCount = itemInfo.subchannelCount;

			if (tmpChannel.wStreamCount == 0)
				tmpChannel.wChannelType = 0;
			else
				tmpChannel.wChannelType = 1;

			tmpChannel.wPackagingMode = atoi(itemProps["PackagingMode"].c_str());;

			strcpy(tmpChannel.szPath, itemProps["Path"].c_str());
			tmpChannel.bEnable = TRUE;
			tmpChannel.nIndex = ++ index;
			portInfo.lstChannelInfo.push_back(tmpChannel);
		}

	}

	portInfo.nCastCount = destCount;
	portInfo.wChannelCount = index;

	portInfo.m_nSessionID = getSessionId();
	DWORD result = getDeviceManager()->OpenPort(portInfo.m_nSessionID, &portInfo);
	if (result == 0) {

		myState = ::TianShanIce::Streamer::stsSetup;
		result = getDeviceManager()->RunPort(getSessionId());
		if (result == 0) {
			myState = TianShanIce::Streamer::stsStreaming;
		} else {

			glog(ISvcLog::L_ERROR, "%s:\tRunPort() failed", __FUNCTION__);
		}
	} else {
		glog(ISvcLog::L_ERROR, "%s:\tOpenPort() failed", __FUNCTION__);
	}

	return true;
}

// implemented
bool
DataOnDemand::DataStreamImpl::setSpeed(::Ice::Float newSpeed,
					   const Ice::Current& current)
{
	throw TianShanIce::NotImplemented(
		"DODStreamer don't support this operation");
	return false;
}

// implemented
bool
DataOnDemand::DataStreamImpl::pause(const Ice::Current& current)
{
	throw TianShanIce::NotImplemented(
		"DODStreamer don't support this operation");
	return false;
}

// implemented
bool
DataOnDemand::DataStreamImpl::resume(const Ice::Current& current)
{
	throw TianShanIce::NotImplemented(
		"DODStreamer don't support this operation");
	return false;
}

TianShanIce::Streamer::StreamState
DataOnDemand::DataStreamImpl::getCurrentState(const Ice::Current& current) const
{
	return myState;
}

// implemented
TianShanIce::SRM::SessionPrx
DataOnDemand::DataStreamImpl::getSession(const Ice::Current& current)
{
	return _session;
}

void DataOnDemand::DataStreamImpl::setMuxRate(::Ice::Int nowRate, 
											  ::Ice::Int maxRate, 
											  ::Ice::Int minRate, 
											  const ::Ice::Current& current)
{
	throw TianShanIce::NotImplemented(
		"DODStreamer don't support this operation");
}

bool DataOnDemand::DataStreamImpl::allocDVBCResource(::Ice::Int serviceGroupID, 
													 ::Ice::Int bandWidth, 
													 const ::Ice::Current& current)
{
	throw TianShanIce::NotImplemented(
		"DODStreamer don't support this operation");
}

::std::string
DataOnDemand::DataStreamImpl::getName(const Ice::Current& current) const
{
    return myInfo.name;
}

::DataOnDemand::StreamInfo
DataOnDemand::DataStreamImpl::getInfo(const Ice::Current& current) const
{
    return myInfo;
}

::Ice::Int DataOnDemand::DataStreamImpl::control(::Ice::Int code, 
												   const ::std::string& param, 
												   const ::Ice::Current& current)
{
	return 0;
}

::DataOnDemand::MuxItemPrx
DataOnDemand::DataStreamImpl::createMuxItem(
	const ::DataOnDemand::MuxItemInfo& info,
	const Ice::Current& current)
{

	::DataOnDemand::MuxItemExPrx muxItemPrx = NameToMuxItem(current.adapter)(
		mySpace, myInfo.name, info.name);

	try {
		muxItemPrx->ice_ping();
		muxItemPrx->destroy();
		// throw DataOnDemand::NameDupException();


	} catch(const Ice::ObjectNotExistException&) {

	}

	DataOnDemand::MuxItemImpl* muxItem = new DataOnDemand::MuxItemImpl(
		_evictor);
	muxItem->myParent = NameToStream(current.adapter)(mySpace, myInfo.name);

	if (!muxItem->init(info)) {
		delete muxItem;
		throw ::TianShanIce::InvalidParameter();
	}

	muxItem->myInfo = info;
	// muxItem->myInfo.name = name;

	_evictor->add(muxItem, DataOnDemand::createMuxItemIdentity(
		mySpace, myInfo.name, info.name));

	myMuxItems.insert(MuxItemExDict::value_type(info.name, muxItemPrx));
    return muxItemPrx;
}

::DataOnDemand::MuxItemPrx
DataOnDemand::DataStreamImpl::getMuxItem(const ::std::string& name,
				      const Ice::Current& current)
{
	::DataOnDemand::MuxItemPrx muxItemPrx = NameToMuxItem(current.adapter)(
    mySpace, myInfo.name, name);

	muxItemPrx->ice_ping();
    return muxItemPrx;
}

::Ice::StringSeq
DataOnDemand::DataStreamImpl::listMuxItems(const Ice::Current& current) const
{
	// return myMuxItems;
	::Ice::StringSeq items;
	MuxItemExDict::const_iterator it;

	for (it = myMuxItems.begin(); it != myMuxItems.end(); it ++) {
        items.push_back(it->first);
	}

	return items;
}

void 
DataOnDemand::DataStreamImpl::setProperies(const ::TianShanIce::Properties& props, 
										const ::Ice::Current& current)
{
	myProps = props;
}

::TianShanIce::Properties 
DataOnDemand::DataStreamImpl::getProperties(const ::Ice::Current& current) const
{
	return myProps;
}

void DataOnDemand::DataStreamImpl::removeMuxItem(const ::std::string& name, 
												 const ::Ice::Current& current )
{
	myMuxItems.erase(name);
}

void DataOnDemand::DataStreamImpl::activate(const ::Ice::Current& current)
{
	MuxItemExDict::iterator it;
	for (it = myMuxItems.begin(); it != myMuxItems.end(); it ++) {
		try {
			it->second->activate();
		} catch (DataStreamError& ) {

			glog(ISvcLog::L_ERROR,"%s:\tmuxIte(%s) activate failed", 
				__FUNCTION__, it->first.c_str());
		} catch (Ice::ObjectNotExistException& e) {

			glog(ISvcLog::L_ERROR,"%s:\tmuxItem(%s) activate failed(%s)", 
				__FUNCTION__, it->first.c_str(), e.ice_name().c_str());
			it = myMuxItems.erase(it);
		}
	}

	if (myState == TianShanIce::Streamer::stsStreaming) {
        play(current);
	}
}

//////////////////////////////////////////////////////////////////////////

::Freeze::EvictorPtr DataOnDemand::DataStreamServiceImpl::_evictor;
::Ice::ObjectAdapterPtr	DataOnDemand::DataStreamServiceImpl::_adapter;

DataOnDemand::DataStreamServiceImpl::DataStreamServiceImpl()
{

}

DataOnDemand::DataStreamServiceImpl::~DataStreamServiceImpl()
{

}

bool DataOnDemand::DataStreamServiceImpl::init()
{

	return true;
}

// implemented
std::string
DataOnDemand::DataStreamServiceImpl::getAdminUri(const ::Ice::Current& )
{
	throw TianShanIce::NotImplemented(
		"DODStreamer don't support this operation");
}

// implemented
::TianShanIce::State
DataOnDemand::DataStreamServiceImpl::getState(const ::Ice::Current& )
{
	throw TianShanIce::NotImplemented(
		"DODStreamer don't support this operation");
}

::TianShanIce::Streamer::StreamPrx 
DataOnDemand::DataStreamServiceImpl::createStream(
	const ::TianShanIce::Transport::PathTicketPrx&, 
	const Ice::Current& current)
{
	throw TianShanIce::NotImplemented(
		"DODStreamer don't support this operation");
}

::TianShanIce::Streamer::StreamerDescriptors 
DataOnDemand::DataStreamServiceImpl::listStreamers(const Ice::Current& current)
{
	throw TianShanIce::NotImplemented(
		"DODStreamer don't support this operation");
}

::std::string 
DataOnDemand::DataStreamServiceImpl::getNetId(const Ice::Current& current) const
{
	return std::string();
}

::DataOnDemand::DataStreamPrx 
DataOnDemand::DataStreamServiceImpl::getStream(
  const ::std::string& space, 
	const ::std::string& name,
	const Ice::Current& current)
{
	DataOnDemand::DataStreamPrx strm;
	strm = NameToStream(_adapter)(space, name);
	strm->ice_ping();
	return strm;
}

TianShanIce::StrValues 
DataOnDemand::DataStreamServiceImpl::listStrems(const std::string& space, 
												const Ice::Current&) const
{
	TianShanIce::StrValues result;
	Ice::Identity ident;

	DataStreamExDict::const_iterator it;
	for (it = myStreams.begin(); it != myStreams.end(); it ++) {
		ident = it->first;
		result.push_back(ident.name);
	}

	return result;
}

void
DataOnDemand::DataStreamServiceImpl::clear(const ::std::string& space, const Ice::Current&)
{
	DataStreamExDict::iterator it;
	DataStreamExDict streams = myStreams;

	if (space.length() <= 0) {
		for (it = streams.begin(); it != streams.end(); it ++) {

			DataStreamPrx strm = it->second;
			strm->destroy();
		}

		// myStreams.clear();

	} else {

		for (it = streams.begin(); it != streams.end(); it ++) {

			DataStreamPrx strm = it->second;
			Ice::Identity identity = strm->ice_getIdentity();

			if (identity.category == space) {

			strm->destroy();
			// it = myStreams.erase(it);
			}
		}
	}
}

::DataOnDemand::DataStreamPrx
DataOnDemand::DataStreamServiceImpl::createStreamByApp(
	const ::TianShanIce::Transport::PathTicketPrx& pathTicket,
	const ::std::string& space, 
	const ::DataOnDemand::StreamInfo& info,
	const Ice::Current& current)
{

	Ice::Identity strmIdentity = DataOnDemand::createStreamIdentity(space, info.name);
	if (_evictor->hasObject(strmIdentity))
		throw DataOnDemand::NameDupException();

	::DataOnDemand::DataStreamExPrx strmPrx = NameToStream(current.adapter)(space, info.name);
	try {
		strmPrx->ice_ping();
		strmPrx->destroy();
	} catch(Ice::ObjectNotExistException) {

	}

	DataOnDemand::DataStreamImpl* strm = new DataOnDemand::DataStreamImpl(_evictor);
	strm->myParent = getThisPrx();

	if (!strm->init(info, generateSessionId())) {
		delete strm;
		throw ::TianShanIce::InvalidParameter();
	}

	strm->mySpace = space;
	// strm->myInfo.name = info.name;

	try {
		_evictor->add(strm, strmIdentity);
	} catch(Ice::Exception& e) {
		glog(ISvcLog::L_DEBUG, "createStreamByApp() failed(%s).", e.ice_name());
		delete strm;
		return NULL;
	}

	{
		std::pair<DataStreamExDict::iterator, bool> r;
		r = myStreams.insert(DataStreamExDict::value_type(
			strmIdentity, strmPrx));

		if (!r.second) {

			glog(ISvcLog::L_DEBUG, "createStreamByApp() myStreams.insert() failed.");
            _evictor->remove(strmIdentity);
			delete strm;
			return NULL;
		}
	}

	
	SpaceInfo spaceInfo;
	spaceInfo.lastUpdate = timeGetTime();
	spaceInfo.updateCount = 1;

	_spaceInfoMap.insert(SpaceInfoMap::value_type(space, spaceInfo));

	return strmPrx;
}

void 
DataOnDemand::DataStreamServiceImpl::setProperies(
	const ::TianShanIce::Properties& props, 
	const ::Ice::Current& current)
{
	myProps = props;
}

::TianShanIce::Properties 
DataOnDemand::DataStreamServiceImpl::getProperties(
	const ::Ice::Current& current) const
{
	return myProps;
}

void 
DataOnDemand::DataStreamServiceImpl::ping(const std::string& space, 
										  const Ice::Current& current) const
{
	SpaceInfoMap::iterator it = _spaceInfoMap.find(space);
	if (it == _spaceInfoMap.end()) {

		SpaceInfo spaceInfo;
		spaceInfo.lastUpdate = timeGetTime();
		spaceInfo.updateCount = 1;

		std::pair<SpaceInfoMap::iterator, bool> r;
		r = _spaceInfoMap.insert(SpaceInfoMap::value_type(space, spaceInfo));
		if (!r.second) {
			throw Ice::UnknownException(__FILE__, __LINE__);
		}
        
	} else {
		SpaceInfo& spaceInfo = it->second;
		spaceInfo.lastUpdate = timeGetTime();
		spaceInfo.updateCount += 1;
	}
}

void DataOnDemand::DataStreamServiceImpl::removeStream(
	const ::std::string& space, const ::std::string& name, 
	const ::Ice::Current& current)
{
	Ice::Identity ident = createStreamIdentity(space, name);
	myStreams.erase(ident);
}

void DataOnDemand::DataStreamServiceImpl::activate(
	const ::Ice::Current& current)
{
	Freeze::EvictorIteratorPtr evictor_it = _evictor->getIterator(
		"::DataOnDemand::DataStreamEx", 3);

	/*
	while (evictor_it->hasNext()) {
		Ice::Identity ident = evictor_it->next();
		if (myStreams.find(ident) == myStreams.end()) {
			_evictor->remove(ident);
		}
	}
	*/

	DataStreamExDict::iterator it;
	for (it = myStreams.begin(); it != myStreams.end(); it ++) {
		try {
			it->second->activate();
		} catch (DataStreamError& ) {

			glog(ISvcLog::L_ERROR,"%s:\tstream(%s) activate failed", 
				__FUNCTION__, 
				current.adapter->getCommunicator()->identityToString(
				it->first).c_str());
		} catch (Ice::ObjectNotExistException& e) {

			glog(ISvcLog::L_ERROR,"%s:\tstream(%s) activate failed(%s)", 
				__FUNCTION__, 
				current.adapter->getCommunicator()->identityToString(
				it->first).c_str(), e.ice_name().c_str());
			it = myStreams.erase(it);
		}
	}
}

unsigned long 
DataOnDemand::DataStreamServiceImpl::generateSessionId()
{
	unsigned long sessionId = atol(myProps["NextSessionId"].c_str());
	char buf[128];
	sprintf(buf, "%ld", sessionId + 1);
	myProps["NextSessionId"] = buf;
	return sessionId;
}

DataOnDemand::DataStreamServiceExPrx 
DataOnDemand::DataStreamServiceImpl::getThisPrx()
{
	return DataStreamServiceExPrx::uncheckedCast(
		_adapter->createProxy(_adapter->getCommunicator()->
		stringToIdentity("DODStreamer")));
}

void 
DataOnDemand::DataStreamServiceImpl::checkTimeout(Ice::Int msec, 
												  const ::Ice::Current& current)
{
	SpaceInfoMap::iterator it;
	for (it = _spaceInfoMap.begin(); it != _spaceInfoMap.end(); it ++) {
		SpaceInfo& spaceInfo = it->second;
		if (timeGetTime() - spaceInfo.lastUpdate > (unsigned int)msec) {
			try {
				clear(it->first, current);
			} catch(Ice::Exception& e) {
				glog(ISvcLog::L_ERROR, "%s:\tclear() occurred a exception(%s).", 
					__FUNCTION__, e.ice_name().c_str());
			}
			it = _spaceInfoMap.erase(it);
		}
	}
}
