#include "stdafx.h"
#include "log.h"
#include "DataStreamImpl.h"
#include "DataSource.h"
#include "DataPusher.h"
#include "PsiPusher.h"
#include "DataStreamServiceImpl.h"
#include "MuxItemImpl.h"
#include "TsEncoder.h"
#include "DataStreamRequest.h"
using namespace ZQLIB;
using namespace DataStream;
//////////////////////////////////////////////////////////////////////////

void split( const std::string& src, char delimiter, 
		   std::vector<std::string>& result)
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

namespace DataOnDemand {

::Ice::Identity createMuxItemIdentity(const std::string& space, 
									  const std::string& strmName, 
									  const std::string& itemName)
{
	Ice::Identity ident;
	ident.category = space;
	ident.name = itemName + "\\/" + strmName;
	return ident;
}

::Ice::Identity createStreamIdentity(const std::string& space, 
									 const std::string& name)
{
	Ice::Identity ident;
	ident.category = space;
	ident.name = name;
	return ident;
}

//////////////////////////////////////////////////////////////////////////

DataStreamImpl::DataStreamImpl(Ice::ObjectAdapterPtr& adapter, 
							   DataStreamServiceImpl* parent, 
							   const std::string& space):
	_adapter(adapter), _parent(parent), _space(space)
{
	_state = ::TianShanIce::Streamer::stsSetup;
	_destroy = false;
	_dataPusher = NULL;
	_dataSources = NULL;
}

DataStreamImpl::~DataStreamImpl()
{

}

bool 
DataStreamImpl::init(const StreamInfo& info)
{
	_info = info;
	_state = ::TianShanIce::Streamer::stsSetup;

	std::string strmDir = getCacheDir();
	// CreateDirectoryA(strmDir.c_str(), NULL);
	ident = createStreamIdentity(_space, info.name);
	return true;
}

// implemented
void
DataStreamImpl::allotPathTicket(
	const TianShanIce::Transport::PathTicketPrx& ticket,
	const Ice::Current& current)
{
	throw TianShanIce::NotImplemented("DataStreamImpl", 0, 
		"DODStream don't support this operation");
}

// implemented
void
DataStreamImpl::destroy(const Ice::Current& current)
{
	std::string space = _space;
	std::string name = _info.name;

	glog(ZQLIB::Log::L_DEBUG,
		"enter DataStreamImpl::destroy(): strmName = %s::%s" LOG_FMT, 
		space.c_str(), name.c_str(), LOG_ARG);

	lock();

	if (_destroy) {

		unlock();
		glog(ZQLIB::Log::L_WARNING,
			"DataStreamImpl::destroy(): strmName = %s::%s invalid state(%d)" 
			LOG_FMT, _space.c_str(), _info.name.c_str(), _state, LOG_ARG);

	} else {

		if (_state != ::TianShanIce::Streamer::stsSetup) {

			psiPusher->stop();
			if (!psiPusher->unregisterProgram(_transferAddr, _info.pmtPid)) {

				glog(ZQLIB::Log::L_ERROR, 
					"DataStreamImpl::destroy() PsiPusher::unregisterProgram() = 0"
					"strmName = %s::%s" LOG_FMT , _space.c_str(), 
					_info.name.c_str(), LOG_ARG);
				unlock();
				assert(false);
				goto L_Exit;
			}

			if (!psiPusher->run()) {
				
				glog(ZQLIB::Log::L_DEBUG, 
					"DataStreamImpl::destroy() PsiPusher::run() == false:"
					"strmName = %s::%s" LOG_FMT , _space.c_str(), 
					_info.name.c_str(), LOG_ARG);

			}

			if (_dataPusher) {
				_dataPusher->stop();
				_dataPusher->release();
				_dataPusher = NULL;
			}

			clearDataSource();
		}		

		MuxItemDict::iterator it = _muxItems.begin();
		for (; it != _muxItems.end(); it ++) {

			::Ice::Identity ident = createMuxItemIdentity(_space, 
				_info.name, it->first);

			try {

				_adapter->remove(ident);
			} catch(Ice::Exception& e) {

				glog(Log::L_WARNING, "DataStreamImpl::destroy():\t" 
					"_adapter->remove() failed. exception name: %s, "
					"object identity: %s:%s:%s", e.ice_name().c_str(), 
					_space.c_str(), _info.name.c_str(), it->first.c_str());
			}
		}

		_muxItems.clear();		
		_state = ::TianShanIce::Streamer::stsStop;
		_destroy = true;
		unlock();

		_parent->removeStream(_space, _info.name);
	}

L_Exit:

	glog(ZQLIB::Log::L_DEBUG,  
		"leave DataStreamImpl::destroy(): strmName = %s::%s" LOG_FMT, 
		space.c_str(), name.c_str(), LOG_ARG);
}

// implemented
::std::string
DataStreamImpl::lastError(const Ice::Current& current) const
{
	return _lastError;
}

// implemented
::Ice::Identity
DataStreamImpl::getIdent(const Ice::Current& current) const
{
	return ident; // id is member of class Stream;
}

void DataStreamImpl::setConditionalControl(
	const ::TianShanIce::Streamer::ConditionalControlMask& mask,
	const ::TianShanIce::Streamer::ConditionalControlPrx& condCtrl, 
	const ::Ice::Current& current)
{

}
void 
DataStreamImpl::playEx(::Ice::Long&, ::Ice::Float&, 
					const Ice::Current& current)
{
	throw TianShanIce::NotImplemented("DataStreamImpl", 0, 
		"DataStream don't support this operation");
}

void 
DataStreamImpl::pauseEx(::Ice::Long&, const Ice::Current& current)
{
	throw TianShanIce::NotImplemented("DataStreamImpl", 0, 
		"DataStream don't support this operation");
}

void 
DataStreamImpl::setSpeedEx(::Ice::Float, ::Ice::Long&, ::Ice::Float&,
							const Ice::Current& current)
{
	throw TianShanIce::NotImplemented("DataStreamImpl", 0, 
		"DataStream don't support this operation");
}

bool
DataStreamImpl::play(const Ice::Current& current)
{
	glog(ZQLIB::Log::L_DEBUG,  
		"enter DataStreamImpl::play(): strmName = %s::%s" LOG_FMT, 
		_space.c_str(), _info.name.c_str(), LOG_ARG);

	Lock sync(*this);
	bool result = false;
	size_t muxItemCount;
	std::vector<std::string> addr;
	unsigned int proto;
	ProgMapInfo progMapInfo;

	if (_state == ::TianShanIce::Streamer::stsStreaming || _destroy) {
		glog(ZQLIB::Log::L_WARNING,
			"DataStreamImpl::play(): strmName = %s::%s invalid state(%d)" 
			LOG_FMT, _space.c_str(), _info.name.c_str(), _state, LOG_ARG);
		goto L_Exit;
	}

	if (_state == ::TianShanIce::Streamer::stsPause) {
		assert(_dataPusher);
		_dataPusher->run();
		_state = ::TianShanIce::Streamer::stsStreaming;
		result = true;
		goto L_Exit;
	}
	split(_info.destAddress, ':', addr);
	
	if (addr[0] == "UDP") {
		proto = TRANSFER_UDP;
	} 
	else 
		if (addr[0] == "MULITCAST") {
			proto = TRANSFER_MULTICAST;
		} 
		else {

		glog(ZQLIB::Log::L_ERROR, "proto MUST be UDP or MULITCAST" LOG_FMT, LOG_ARG);
		glog(ZQLIB::Log::L_DEBUG, 
			"leave DataStreamImpl::play(): strmName = %s::%s" LOG_FMT , 
			_space.c_str(), _info.name.c_str(), LOG_ARG);
		
		goto L_Exit;
	}

	glog(ZQLIB::Log::L_INFO,  
		"DataStreamImpl::play(): strmName = %s::%s "
		"proto = %s,  ip = %s, port = %s" LOG_FMT, _space.c_str(), 
		_info.name.c_str(), addr[0].c_str(), addr[1].c_str(), 
		addr[2].c_str(), LOG_ARG);
	
	_transferAddr = TransferAddress(proto, inet_addr(addr[1].c_str()), 
		htons(atoi(addr[2].c_str())));

	_dataPusher = new DataPusher(*senderThreadPool, *readerThreadPool, 
		*bufferManager, _transferAddr, _info.totalBandwidth, *dataSender);
	
	progMapInfo.pmtPid = _info.pmtPid;
	progMapInfo.progNum = 0; // auto generate

	muxItemCount = _muxItems.size();
	if (muxItemCount) {

		_dataSources = new DataSource* [muxItemCount];

		MuxItemDict::iterator it;
		int i = 0;

		ProgMapEntry progMapEntry;
		
		for (it = _muxItems.begin(); it != _muxItems.end(); it ++) {

			MuxItemImpl* muxItem = it->second;
			MuxItemInfo& itemInfo = muxItem->getInfo();	
			::TianShanIce::Properties& itemProps = muxItem->getProperties();	

			_dataSources[i] = new DataSource(itemInfo.bandWidth, 
				itemInfo.repeatTime);
			muxItem->setDataSource(_dataSources[i]);
			i ++;

			progMapEntry.extraInfoLen = 0;
			int n = 0;
			
			do {
				
				progMapEntry.streamType = itemInfo.streamType + n;
				progMapEntry.elementId = itemInfo.streamId + n;
				progMapInfo.progMapEntrys.push_back(progMapEntry);
				n ++;

			} while (n <= itemInfo.subchannelCount);
		}

		if (!_dataPusher->setSources(_dataSources, muxItemCount)) {
			glog(ZQLIB::Log::L_ERROR,
				"DataStreamImpl::play(): strmName = %s::%s setSources() falied. " 
				"invalid state or out of total bandwidth"
				LOG_FMT, _space.c_str(), _info.name.c_str(), _state, LOG_ARG);

			clearDataSource();

			_dataPusher->release();
			_dataPusher = NULL;

			goto L_Exit;
		}

	} else {

		glog(ZQLIB::Log::L_ERROR,
			"DataStreamImpl::play(): strmName = %s::%s have no mux items" 
			LOG_FMT, _space.c_str(), _info.name.c_str(), LOG_ARG);	

		goto L_Exit;
	}

	psiPusher->stop();
	if (!psiPusher->registerProgram(_transferAddr, progMapInfo)) {

		glog(ZQLIB::Log::L_ERROR, 
			"DataStreamImpl::play() PsiPusher::registerProgram() = 0 "
			"strmName = %s::%s" LOG_FMT , _space.c_str(), 
			_info.name.c_str(), LOG_ARG);

		clearDataSource();
		_dataPusher->release();
		_dataPusher = NULL;

		psiPusher->run();
		goto L_Exit;
	}

	if (!psiPusher->run()) {

		psiPusher->unregisterProgram(_transferAddr, progMapInfo.pmtPid);
		if (!psiPusher->run()) {

			glog(ZQLIB::Log::L_ERROR, 
				"DataStreamImpl::play() PsiPusher::run() = 0 "
				"strmName = %s::%s" LOG_FMT , _space.c_str(), 
				_info.name.c_str(), LOG_ARG);

			assert(false);
		}

		glog(ZQLIB::Log::L_ERROR, 
			"DataStreamImpl::play() PsiPusher::run() == false "
			"strmName = %s::%s" LOG_FMT , _space.c_str(), 
			_info.name.c_str(), LOG_ARG);

		clearDataSource();
		_dataPusher->release();
		_dataPusher = NULL;

		goto L_Exit;
	}
	
	if (_dataPusher->run()) {

		_state = ::TianShanIce::Streamer::stsStreaming;
		result = true;

	} else {

		glog(ZQLIB::Log::L_ERROR, 
			"DataStreamImpl::play() dataPusher::run() == false "
			"strmName = %s::%s" LOG_FMT , _space.c_str(), 
			_info.name.c_str(), LOG_ARG);

		clearDataSource();
		_dataPusher->release();
		_dataPusher = NULL;
	}

L_Exit:

	glog(ZQLIB::Log::L_DEBUG, 
		"leave DataStreamImpl::play(): strmName = %s::%s" LOG_FMT, 
		_space.c_str(), _info.name.c_str(), LOG_ARG);

	return result;
}

// implemented
bool
DataStreamImpl::setSpeed(::Ice::Float newSpeed,
						 const Ice::Current& current)
{
	throw TianShanIce::NotImplemented("DataStreamImpl", 0, 
		"DataStream don't support this operation");
	return false;
}

// implemented
bool
DataStreamImpl::pause(const Ice::Current& current)
{
	Lock sync(*this);

	if (_state != ::TianShanIce::Streamer::stsStreaming) {
		glog(ZQLIB::Log::L_WARNING,
			"DataStreamImpl::play(): strmName = %s::%s invalid state(%d)" 
			LOG_FMT, _space.c_str(), _info.name.c_str(), _state, LOG_ARG);

		return false;
	}

	if (_dataPusher == NULL) {

		assert(false);
		return false;
	}

	_dataPusher->pause();

	_state = ::TianShanIce::Streamer::stsPause;
	
	return true;
}

// implemented
bool
DataStreamImpl::resume(const Ice::Current& current)
{
	Lock sync(*this);

	if (_state != ::TianShanIce::Streamer::stsPause) {

		glog(ZQLIB::Log::L_WARNING,
			"DataStreamImpl::resume(): strmName = %s::%s invalid state(%d)" 
			LOG_FMT, _space.c_str(), _info.name.c_str(), _state, LOG_ARG);
		return false;
	}

	if (_dataPusher == NULL) {

		assert(false);
		return false;
	}

	_dataPusher->run();
	_state = ::TianShanIce::Streamer::stsStreaming;

	return true;
}

TianShanIce::Streamer::StreamState
DataStreamImpl::getCurrentState(const Ice::Current& current) const
{
	return _state;
}

// implemented
TianShanIce::SRM::SessionPrx
DataStreamImpl::getSession(const Ice::Current& current)
{
	return _session;
}

void DataStreamImpl::setMuxRate(::Ice::Int nowRate, 
								::Ice::Int maxRate, 
								::Ice::Int minRate, 
								const ::Ice::Current& current)
{
	throw TianShanIce::NotImplemented("DataStreamImpl", 0, 
		"DataStream don't support this operation");
}

bool DataStreamImpl::allocDVBCResource(::Ice::Int serviceGroupID, 
									   ::Ice::Int bandWidth, 
									   const ::Ice::Current& current)
{
	throw TianShanIce::NotImplemented("DataStreamImpl", 0, 
		"DataStream don't support this operation");
}

::Ice::Long 
DataStreamImpl::seekStream(::Ice::Long, ::Ice::Int, const ::Ice::Current&)
{
	throw TianShanIce::NotImplemented("DataStreamImpl", 0, 
		"DataStream don't support this operation");
}

::std::string
DataStreamImpl::getName(const Ice::Current& current) const
{
    return _info.name;
}

StreamInfo
DataStreamImpl::getInfo(const Ice::Current& current) const
{
    return _info;
}

::Ice::Int DataStreamImpl::control(::Ice::Int code, 
								   const ::std::string& param, 
								   const ::Ice::Current& current)
{
	throw TianShanIce::NotImplemented("DataStreamImpl", 0, 
		"DataStream don't support this operation");

	return 0;
}

bool 
DataStreamImpl::findMuxItemByStreamId(int streamId)
{
	MuxItemDict::iterator it = _muxItems.begin();
	for (;it != _muxItems.end(); it ++) {
		MuxItemImpl* muxItem = it->second;
		MuxItemInfo muxItemInfo = muxItem->getInfo();
		if (muxItemInfo.streamId == streamId)
			return true;
	}

	return false;
}

MuxItemPrx
DataStreamImpl::createMuxItem(const MuxItemInfo& info,
							  const Ice::Current& current)
{
	glog(ZQLIB::Log::L_DEBUG,  
		"enter DataStreamImpl::createMuxItem(): strmName = %s::%s, "
		"muxItemName = %s" LOG_FMT, _space.c_str(), 
		_info.name.c_str(), info.name.c_str(), LOG_ARG);

	Lock sync(*this);
	MuxItemPrx muxItemPrx;

	// no extra distributer now, cannot apply a too small rate
	if ((unsigned int )info.bandWidth < 
		TS_PACKET_SIZE * (MS_PER_SEC / gDataStreamConfig.stdPeriod)) {

		glog(ZQLIB::Log::L_ERROR,
			"DataStreamImpl::createMuxItem(): strmName = %s::%s "
			"invalid bandwidth(%d)" LOG_FMT, 
			_space.c_str(), _info.name.c_str(), info.bandWidth, LOG_ARG);

		goto L_Exit;
	
	}

	if (_state != ::TianShanIce::Streamer::stsSetup) {
		glog(ZQLIB::Log::L_WARNING,
			"DataStreamImpl::createMuxItem(): strmName = %s::%s invalid state(%d)" 
			LOG_FMT, _space.c_str(), _info.name.c_str(), _state, LOG_ARG);

		goto L_Exit;
	}	

	if (findMuxItemByStreamId(info.streamId)) {

		glog(ZQLIB::Log::L_ERROR,  
			"DataStreamImpl::createMuxItem(): muxItemName = %s::%s::%s " 
			"streamId = %d is already existing" LOG_FMT, 
			_space.c_str(), _info.name.c_str(), info.name.c_str(), 
			info.streamId, LOG_ARG);

		goto L_Exit;
	}	

	if (_muxItems.find(info.name) == _muxItems.end()) {

		muxItemPrx = NameToMuxItem(current.adapter)(
			_space, _info.name, info.name);

		try {
			muxItemPrx->ice_ping();
			// muxItemPrx->destroy();
			throw NameDupException();

		} catch(const Ice::ObjectNotExistException&) {

		}

		MuxItemImpl* muxItem = new MuxItemImpl(
			_adapter);
		muxItem->_parent = this;

		if (!muxItem->init(info)) {
			
			delete muxItem;
			muxItemPrx = NULL;

			glog(ZQLIB::Log::L_ERROR, "DataStreamImpl::createMuxItem() failed, "
				"muxItem->init() == false: strmName = %s::%s, "
				"muxItemName = %s" LOG_FMT, _space.c_str(), 
				_info.name.c_str(), info.name.c_str(), LOG_ARG);
			
			goto L_Exit;
		}

		if (_adapter->add(muxItem, createMuxItemIdentity(
			_space, _info.name, info.name)) == NULL) {

			delete muxItem;
			muxItemPrx = NULL;

			glog(ZQLIB::Log::L_ERROR, "DataStreamImpl::createMuxItem() failed, "
				"adapter->add() == NULL: strmName = %s::%s, "
				"muxItemName = %s" LOG_FMT, _space.c_str(), 
				_info.name.c_str(), info.name.c_str(), LOG_ARG);

			goto L_Exit;
		}

		std::pair<MuxItemDict::iterator, bool> r;
		r = _muxItems.insert(MuxItemDict::value_type(info.name, muxItem));

		if (!r.second) {

			delete muxItem;
			muxItemPrx = NULL;

			glog(ZQLIB::Log::L_ERROR, "DataStreamImpl::createMuxItem() failed, "
				"muxItemDict.insert() == false: strmName = %s::%s, "
				"muxItemName = %s" LOG_FMT, _space.c_str(), 
				_info.name.c_str(), info.name.c_str(), LOG_ARG);
			goto L_Exit;
		}

	} else {

		glog(ZQLIB::Log::L_WARNING, "DataStreamImpl::createMuxItem() failed, "
			"object is existing: strmName = %s::%s, "
			"muxItemName = %s" LOG_FMT, _space.c_str(), 
			_info.name.c_str(), info.name.c_str(), LOG_ARG);
	}

L_Exit:
	glog(ZQLIB::Log::L_DEBUG,  
		"leave DataStreamImpl::createMuxItem(): strmName = %s::%s, "
		"muxItemName = %s" LOG_FMT, _space.c_str(), 
		_info.name.c_str(), info.name.c_str(), LOG_ARG);

    return muxItemPrx;
}

MuxItemPrx
DataStreamImpl::getMuxItem(const ::std::string& name,
						   const Ice::Current& current)
{
	Lock sync(*this);

	MuxItemDict::iterator it = _muxItems.find(name);
	if (it == _muxItems.end())
		return NULL;
	
	MuxItemPrx muxItemPrx = NameToMuxItem(current.adapter)(
		_space, _info.name, name);

	muxItemPrx->ice_ping();
    return muxItemPrx;
}

::Ice::StringSeq
DataStreamImpl::listMuxItems(const Ice::Current& current) const
{
	Lock sync(*this);

	::Ice::StringSeq items;
	MuxItemDict::const_iterator it;

	for (it = _muxItems.begin(); it != _muxItems.end(); it ++) {
        items.push_back(it->first);
	}

	return items;
}

void            
DataStreamImpl::setProperties(const ::TianShanIce::Properties& props, 
							 const ::Ice::Current& current)
{
	Lock sync(*this);
	
	_props = props;
}

::TianShanIce::Properties 
DataStreamImpl::getProperties(const ::Ice::Current& current) const
{
	return _props;
}

void DataStreamImpl::ping(const ::Ice::Current& current) const
{
	assert(_parent);
	
#ifdef _DEBUG
	glog(ZQLIB::Log::L_DEBUG, 
		"DataStreamImpl::ping()\t space = %s, strmName = %s", 
		_space.c_str(), _info.name.c_str());
#endif

	_parent->ping(_space);
}

bool DataStreamImpl::removeMuxItem(const ::std::string& name)
{
	{
		Lock sync(*this);
		
		if (_state != ::TianShanIce::Streamer::stsSetup && 
			_state != ::TianShanIce::Streamer::stsStop) {
			
			glog(ZQLIB::Log::L_WARNING,
				"DataStreamImpl::removeMuxItem(): strmName = %s::%s "
				"invalid state(%d)" LOG_FMT, _space.c_str(), 
				_info.name.c_str(), _state, LOG_ARG);

			return false;
		}

		MuxItemDict::iterator it = _muxItems.find(name);
		if (it == _muxItems.end()) {
			glog(ZQLIB::Log::L_WARNING,
				"DataStreamImpl::removeMuxItem(): strmName = %s::%s not found" 
				LOG_FMT, _space.c_str(), _info.name.c_str(), _state, LOG_ARG);

			return false;
		}
		
		Ice::Identity ident = createMuxItemIdentity(_space, 
			_info.name, name);

		_muxItems.erase(name);
	}

	try {
		_adapter->remove(ident);
	} catch(Ice::Exception& e) {
		
		glog(Log::L_WARNING, "DataStreamImpl::removeMuxItem():\t" 
			"_adapter->remove() failed. exception name: %s, "
			"object identity: %s:%s:%s", e.ice_name().c_str(), 
			_space.c_str(), _info.name.c_str(), name.c_str());

		return false;
	}
	
	return true;
}

void DataStreamImpl::clearDataSource()
{
	if (_dataSources) {
		size_t i;
		for (i = 0; i <  _muxItems.size(); i ++)
			if (_dataSources[i])
				delete _dataSources[i];

		delete _dataSources;	
		_dataSources = NULL;
	}
}
void 
DataStreamImpl::commit_async(const ::TianShanIce::Streamer::AMD_Stream_commitPtr& amdstreamcommit, 
							 const ::Ice::Current& current)
{
/*	throw TianShanIce::NotImplemented("DataStreamImpl", 0, 
		"DataStream don't support this operation");*/
	glog(Log::L_INFO, "enter DataStreamImpl::commit_async()");
    amdstreamcommit ->ice_response();
   	glog(Log::L_INFO, "leaving DataStreamImpl::commit_async()");

}
::TianShanIce::Streamer::StreamInfo 
DataStreamImpl::playEx(::Ice::Float newSpeed, ::Ice::Long offset, ::Ice::Short from, 
					   const ::TianShanIce::StrValues& expectedProps, 
					   const ::Ice::Current& current)
{
	::TianShanIce::Streamer::StreamInfo streaminfo;
	throw TianShanIce::NotImplemented("DataStreamImpl", 0, 
		"DataStream don't support this operation");

	return streaminfo;
}
::TianShanIce::Streamer::StreamInfo 
DataStreamImpl::pauseEx(const ::TianShanIce::StrValues& expectedProps, 
						const ::Ice::Current& current)
{
	::TianShanIce::Streamer::StreamInfo streaminfo;
	throw TianShanIce::NotImplemented("DataStreamImpl", 0, 
		"DataStream don't support this operation");

	return streaminfo;
}

void DataStreamImpl::play_async(const TianShanIce::Streamer::AMD_Stream_playPtr &amdCB,const Ice::Current &)
{
	try 
	{	
		PlayStreamRequest* plRequest = (new PlayStreamRequest(amdCB, *this));
		if(plRequest)
			plRequest->start();
		else 
		{
			glog(ZQ::common::Log::L_ERROR,"create play request for play stream failed");
			amdCB->ice_exception(::TianShanIce::ServerError("DataStreamImpl", 504, "create play request for play stream failed"));
		}
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, "play_async() failed to play, errorcode=%d", GetLastError());
		amdCB->ice_exception(::TianShanIce::ServerError("DataStreamImpl", 500, "failed to play"));
	}
	return ;
}
void DataStreamImpl::seekStream_async(const TianShanIce::Streamer::AMD_Stream_seekStreamPtr &,Ice::Long,Ice::Int,const Ice::Current &)
{
	::TianShanIce::Streamer::StreamInfo streaminfo;
	throw TianShanIce::NotImplemented("DataStreamImpl", 0, 
		"DataStream don't support this operation");

	return ;
}
void DataStreamImpl::playEx_async(const TianShanIce::Streamer::AMD_Stream_playExPtr &,Ice::Float,Ice::Long,Ice::Short,const TianShanIce::StrValues &,const Ice::Current &)
{
	::TianShanIce::Streamer::StreamInfo streaminfo;
	throw TianShanIce::NotImplemented("DataStreamImpl", 0, 
		"DataStream don't support this operation");

	return ;
}


} // namespace DataOnDemand {
