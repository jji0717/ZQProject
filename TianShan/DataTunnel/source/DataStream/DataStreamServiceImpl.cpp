// DataStreamServiceImpl.cpp: implementation of the DataStreamServiceImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataStreamServiceImpl.h"
#include "DataStreamImpl.h"
#include "ResKey.h"
#include "TsEncoder.h"
#include "datastreamcfg.h"
extern ZQ::common::Config::Loader<DataStreamCfg > gDataStreamConfig;
using namespace ZQLIB;
namespace TianShanIce  {
namespace Streamer     {
namespace DataOnDemand {

DataStreamServiceImpl::DataStreamServiceImpl(ZQADAPTER_DECLTYPE& adapter, ::Freeze::DSServantLocatorPtr& dsServantLocator):
	_adapter(adapter),_dsservantlocator(dsServantLocator)
{
	_destroy = false;
	_rateCount = 0;
}

DataStreamServiceImpl::~DataStreamServiceImpl()
{

}

bool DataStreamServiceImpl::init()
{

	return true;
}

// implemented
std::string
DataStreamServiceImpl::getAdminUri(const ::Ice::Current& )
{
	return "unknown://no-uri";
}

// implemented
::TianShanIce::State
DataStreamServiceImpl::getState(const ::Ice::Current& )
{
	return ::TianShanIce::stInService;
}

::TianShanIce::Streamer::StreamPrx 
DataStreamServiceImpl::createStream(
	const ::TianShanIce::Transport::PathTicketPrx& pathTicket, 
	const Ice::Current& current)
{

	glog(ZQLIB::Log::L_DEBUG, 
		"enter DataStreamServiceImpl::createStream()" LOG_FMT, LOG_ARG);

	::std::string nameSpace;
	StreamInfo info;

	::TianShanIce::Variant var;

	try {
		::TianShanIce::ValueMap valMap = pathTicket->getPrivateData();

		glog(ZQLIB::Log::L_DEBUG, 
			"DataStreamServiceImpl::createStream() privateData.size() = %d" 
			LOG_FMT, valMap.size(), LOG_ARG);

		var = valMap[RESKEY_SPACENAME];
		nameSpace = var.strs[0];

		var = valMap[RESKEY_STREAMNAME];
		info.name = var.strs[0];

		var = valMap[RESKEY_DESTADDRESS];
		info.destAddress = var.strs[0];

		var = valMap[RESKEY_PMTPID];
		info.pmtPid = var.ints[0];

		var = valMap[RESKEY_TOTALBANDWIDTH];
		info.totalBandwidth = var.ints[0];

	} catch (::Ice::Exception& e) {

		glog(ZQLIB::Log::L_ERROR, 
			"DataStreamServiceImpl::createStream() "
			"pathTicket->getPrivateData() failed. exception name is: %s" 
			LOG_FMT, e.ice_name().c_str(), LOG_ARG);

		glog(ZQLIB::Log::L_DEBUG, 
			"leave DataStreamServiceImpl::createStream()");

		return NULL;
	}

	glog(ZQLIB::Log::L_DEBUG, 
			"DataStreamServiceImpl::createStream()\tspace = %s, strmName = %s", 
			nameSpace.c_str(), info.name.c_str());

	::TianShanIce::Streamer::StreamPrx result;

	try {
		result = createStreamByApp(pathTicket, nameSpace, info, current);

	} catch(Ice::Exception& e) {
		glog(ZQLIB::Log::L_ERROR, 
			"DataStreamServiceImpl::createStream() "
			"createStreamByApp failed. exception name is: %s" 
			LOG_FMT, e.ice_name().c_str(), LOG_ARG);

	} catch(...) {

		glog(ZQLIB::Log::L_ERROR, 
			"DataStreamServiceImpl::createStream() "
			"createStreamByApp failed. occurred a unknown exception" 
			LOG_FMT, LOG_ARG);
	}

	glog(ZQLIB::Log::L_DEBUG, 
		"leave DataStreamServiceImpl::createStream()");

	return result;
}

::TianShanIce::Streamer::StreamerDescriptors 
DataStreamServiceImpl::listStreamers(const Ice::Current& current)
{
	::TianShanIce::Streamer::StreamerDescriptor desc;
	desc.deviceId = "0";
	desc.type = "IPEdge";
	::TianShanIce::Streamer::StreamerDescriptors res;
	res.push_back(desc);
	return res;
}

::std::string 
DataStreamServiceImpl::getNetId(const Ice::Current& current) const
{
	return gDataStreamConfig.netId;
}

DataStreamPrx 
DataStreamServiceImpl::getStream(
  const ::std::string& space, 
	const ::std::string& name,
	const Ice::Current& current) const
{
	Lock sync(*this);
	
	SpaceInfoMap::iterator it = _spaceInfos.find(space);
	if (it == _spaceInfos.end()) {

		// log warning
		return NULL;
	}

	DataStreamDict& streams = it->second.streams;
	if (streams.find(name) == streams.end()) {
		// log warning
		return NULL;
	}

	DataStreamPrx strm;
	strm = NameToStream(_adapter)(space, name);
	strm->ice_ping();
	return strm;
}

TianShanIce::StrValues 
DataStreamServiceImpl::listStreams(const std::string& space, 
								  const Ice::Current&)const
{
	Lock sync(*this);
	TianShanIce::StrValues result;

	DataStreamDict* streams = getStreamDict(space);
	if (streams == NULL) {
		// log warning
		return result;
	}
	
	DataStreamDict::iterator it;
	for (it = streams->begin(); it != streams->end(); it ++) {
		result.push_back(it->first);
	}

	return result;
}

void 
DataStreamServiceImpl::clearStreamDict(DataStreamDict& streams)
{
	DataStreamDict strmsDup = streams;
	DataStreamDict::iterator it;
	for (it = strmsDup.begin(); it != strmsDup.end(); it ++) {

		DataStreamImpl* strm = it->second;
		strm->destroy();
	}
}

void
DataStreamServiceImpl::clear(const ::std::string& space, 
							 const Ice::Current&)
{
	Lock sync(*this);
	SpaceInfoMap::iterator spaceIt;

	glog(ZQLIB::Log::L_DEBUG, 
		"DataStreamServiceImpl::clear(%s)", space.c_str());

	if (space.length() <= 0) {

		for (spaceIt = _spaceInfos.begin(); spaceIt != _spaceInfos.end();
			spaceIt ++) {

			DataStreamDict& streams = spaceIt->second.streams;
			clearStreamDict(streams);
		}

		_spaceInfos.clear();

	} else {

		DataStreamDict* streams = getStreamDict(space);
		if (streams == NULL) {
			// log warning
			return;
		}

		clearStreamDict(*streams);
	}
}

DataStreamServiceImpl::SpaceInfoMap::iterator 
DataStreamServiceImpl::createSpace(const std::string& space)
{
	SpaceInfo spaceInfo;
	spaceInfo.lastUpdate = timeGetTime();
	spaceInfo.updateCount = 1;

	std::pair<SpaceInfoMap::iterator, bool> r;
	r = _spaceInfos.insert(SpaceInfoMap::value_type(
		space, spaceInfo));
	if (!r.second) {
		// log error
		return _spaceInfos.end();
	}

	std::string spaceDir = gDataStreamConfig.catchDir;
	spaceDir = spaceDir + "\\" + space;
	CreateDirectoryA(spaceDir.c_str(), NULL);

	return r.first;
}

DataStreamPrx
DataStreamServiceImpl::createStreamByApp(
	const ::TianShanIce::Transport::PathTicketPrx&, 
	const ::std::string& space, 
	const StreamInfo& info, 
	const ::Ice::Current&)
{
	Lock sync(*this);

	// no extra distributer now, cannot apply a too small rate
	if ((unsigned int )info.totalBandwidth < 
		TS_PACKET_SIZE * (MS_PER_SEC / gDataStreamConfig.stdPeriod)) {

		glog(ZQLIB::Log::L_ERROR,
			"DataStreamServiceImpl::createStreamByApp: strmName = %s::%s "
			"invalid totalBandwidth(%d)" LOG_FMT, 
			space.c_str(), info.name.c_str(), info.totalBandwidth, LOG_ARG);

		return NULL;	
	}

	if (_rateCount + info.totalBandwidth > gDataStreamConfig.totalRate) {

		glog(Log::L_ERROR, 
			"createStreamByApp() %s::%s, out of bandwidth" LOG_FMT, 
			space.c_str(), info.name.c_str(), LOG_ARG);
		return NULL;
	}

	SpaceInfoMap::iterator spaceIt = _spaceInfos.find(space);
	if (spaceIt == _spaceInfos.end()) {

		spaceIt = createSpace(space);
		if (spaceIt == _spaceInfos.end()) {

			glog(Log::L_ERROR, 
				"createStreamByApp() createSpace(%s) failed. strmName = %s" 
				LOG_FMT, space.c_str(), info.name.c_str(), LOG_ARG);

			return NULL;
		}
	}

	DataStreamDict& streams = spaceIt->second.streams;

	Ice::Identity strmIdentity = createStreamIdentity(space, info.name);

/*	if (_adapter->find(strmIdentity) != NULL)
		throw NameDupException();*/
	if(_dsservantlocator->hasObject(strmIdentity) != NULL)
		throw NameDupException();

	DataStreamPrx strmPrx = 
		NameToStream(_adapter)(space, info.name);

/*	try {
		strmPrx->ice_ping();
		// strmPrx->destroy();
	} catch(Ice::ObjectNotExistException) {

	} catch(::Ice::Exception& e) {
		glog(Log::L_ERROR, 
			"createStreamByApp() occurred a exception: %s." LOG_FMT, 
			e.ice_name().c_str(), LOG_ARG);
	}*/

	TianShanIce::Streamer::DataOnDemand::DataStreamImpl* strm = 
		new TianShanIce::Streamer::DataOnDemand::DataStreamImpl(_adapter, _dsservantlocator,this, space);

	if (!strm->init(info)) {

		glog(Log::L_ERROR, 
			"createStreamByApp() failed, strm->init() == false." LOG_FMT, 
			LOG_ARG);

		delete strm;
		throw ::TianShanIce::InvalidParameter();
	}

	try {

//		if (_adapter->add(strm, strmIdentity) == NULL) 
		if((_dsservantlocator->add(strm, strmIdentity)) ==NULL)
		{
			glog(Log::L_ERROR, 
				"createStreamByApp() failed. adapter.add() failed." LOG_FMT, 
				LOG_ARG);

			delete strm;
			return NULL;
		}

	} catch(Ice::Exception& e) {

		glog(Log::L_ERROR, 
			"createStreamByApp() failed(%s) adapter.add() failed." LOG_FMT, 
			LOG_ARG, e.ice_name());

		delete strm;
		return NULL;
	}

	{

		std::pair<DataStreamDict::iterator, bool> r;
		r = streams.insert(DataStreamDict::value_type(
			info.name, strm));

		if (!r.second) {

			glog(Log::L_DEBUG, 
				"createStreamByApp() _streams.insert() failed." LOG_FMT, 
				LOG_ARG);

			try {
			//	_adapter->remove(strmIdentity);
				_dsservantlocator->remove(strmIdentity);

			} catch(Ice::Exception& e) {

				::Ice::CommunicatorPtr ic = _adapter->getCommunicator();
				glog(Log::L_WARNING, "DataStreamServiceImpl::createStreamByApp():\t" 
					"_adapter->remove() failed. exception name: %s, "
					"object identity: %s", e.ice_name().c_str(), 
					ic->identityToString(strmIdentity).c_str());
			}

			delete strm;
			return NULL;
		} else {

			_rateCount += info.totalBandwidth;
		}
	}

	return strmPrx;
}

void 
DataStreamServiceImpl::setProperties(
	const ::TianShanIce::Properties& props, 
	const ::Ice::Current& current)
{
	_props = props;
}

::TianShanIce::Properties 
DataStreamServiceImpl::getProperties(
	const ::Ice::Current& current)const
{
	return _props;
}

void 
DataStreamServiceImpl::ping(const std::string& space, 
							const ::Ice::Current& )const
{
	Lock sync(*this);

#if defined(_DEBUG) && 0
	glog(ZQLIB::Log::L_DEBUG, 
		"DataStreamServiceImpl::ping()\t space = %s", space.c_str());
#endif

	SpaceInfoMap::iterator it = _spaceInfos.find(space);
	if (it == _spaceInfos.end()) {

		SpaceInfo spaceInfo;
		spaceInfo.lastUpdate = timeGetTime();
		spaceInfo.updateCount = 1;

		std::pair<SpaceInfoMap::iterator, bool> r;
		r = _spaceInfos.insert(
			SpaceInfoMap::value_type(space, spaceInfo));

		if (!r.second) {
			throw Ice::UnknownException(__FILE__, __LINE__);
		}
        
	} else {
		SpaceInfo& spaceInfo = it->second;
		spaceInfo.lastUpdate = timeGetTime();
		spaceInfo.updateCount += 1;
	}
}

void DataStreamServiceImpl::removeStream(
	const ::std::string& space, const ::std::string& name)
{
	{
		Lock sync(*this);

		DataStreamDict* streams = getStreamDict(space);
		if (streams == NULL) {
			glog(ZQLIB::Log::L_WARNING, 
				"DataStreamServiceImpl::removeStream()\t cannot found space(%s_", 
				space.c_str());
			return;
		}

		DataStreamDict::iterator it = streams->find(name);
		if (it == streams->end()) {
			glog(ZQLIB::Log::L_WARNING, 
				"DataStreamServiceImpl::removeStream()\t cannot found stream(%s:%s)", 
				space.c_str(), name.c_str());
			return;
		}		

		int rate;

		try {
			DataStreamImpl* dataStrm = it->second;
			StreamInfo strmInfo = dataStrm->getInfo();
			rate = strmInfo.totalBandwidth;

			streams->erase(it);
			_rateCount -= rate;
			
		} catch(Ice::Exception& e) {

			glog(Log::L_ERROR, "DataStreamServiceImpl::removeStream():\t" 
				"occurred a exception. exception name: %s, ", e.ice_name().c_str());

			assert(false);
			return;
		}

		// DataStreamImpl* strm = it->second;
		// delete strm;
		
	}

	try {

		::Ice::Identity ident = createStreamIdentity(space, name);
		_dsservantlocator->remove(ident);
	//	_adapter->remove(ident);

	} catch(Ice::Exception& e) {

		::Ice::CommunicatorPtr ic = _adapter->getCommunicator();
		glog(Log::L_ERROR, "DataStreamServiceImpl::removeStream():\t" 
			"_adapter->remove() failed. exception name: %s, "
			"object identity: %s::%s", e.ice_name().c_str(), 
			space.c_str(), name.c_str());

		assert(false);
		return;
	}
}

void 
DataStreamServiceImpl::destroy(const ::Ice::Current& current)
{
	Lock sync(*this);

	clear(std::string());
	try {
		/*
		::Ice::CommunicatorPtr ic = _adapter->getCommunicator();
		_adapter->remove(ic->stringToIdentity(ICE_SERVICE_NAME));
		*/
	} catch(Ice::Exception& e) {

		::Ice::CommunicatorPtr ic = _adapter->getCommunicator();
		glog(Log::L_WARNING, "DataStreamServiceImpl::destroy():\t" 
			"_adapter->remove() failed. exception name: %s, "
			"object identity: %s", e.ice_name().c_str(), 
			ic->identityToString(current.id).c_str());
	}

	_destroy = true;
}

void 
DataStreamServiceImpl::checkTimeout(Ice::Int msec)
{
	Lock sync(*this);

	SpaceInfoMap::iterator it = _spaceInfos.begin();
	SpaceInfoMap::iterator itNext = it;

	for (; it != _spaceInfos.end(); it ++) {
		SpaceInfo& spaceInfo = it->second;
		if (timeGetTime() - spaceInfo.lastUpdate > (unsigned int)msec) {
			
			glog(ZQLIB::Log::L_DEBUG, 
				"DataStreamServiceImpl::checkTimeout() expired, space = %s", 
				it->first.c_str());

			try {
				clear(it->first);
			} catch(Ice::Exception& e) {
				glog(Log::L_ERROR, "%s:\tclear() occurred a exception(%s).", 
					__FUNCTION__, e.ice_name().c_str());
			}

			itNext = it;
			itNext ++;

			_spaceInfos.erase(it);
			it = itNext;
		}
	}
}
void DataStreamServiceImpl::queryReplicas_async(const TianShanIce::AMD_ReplicaQuery_queryReplicasPtr &,
													const std::string &,const std::string &,bool,const Ice::Current &)
{

}
DataStreamFactory::DataStreamFactory(ZQADAPTER_DECLTYPE& adapter, Freeze::EvictorPtr& evictor):
_adapter(adapter), _evictor(evictor)
{

}
DataStreamFactory::~DataStreamFactory()
{

}
Ice::ObjectPtr 
DataStreamFactory::create(const std::string& type)
{
	if (type == "TianShanIce::Streamer::DataOnDemand::DataStream") {

		//return new TianShanIce::Streamer::DataOnDemand::DataStreamImpl(_evictor);

	} else if (type == "TianShanIce::Streamer::DataOnDemand::MuxItem") {

	//	return new TianShanIce::Streamer::DataOnDemand::MuxItemImpl(_adapter);

	}
	assert(false);
	return NULL;
}

} // namespace DataOnDemand {
}
}
