// DataPusher.cpp: implementation of the DataPusher class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataDef.h"
#include "TsEncoder.h"
#include "DataPusher.h"
#include "DataSource.h"
#include "DataQueue.h"
#include "DataReader.h"
#include "DataSender.h"
#include "DataDistributer.h"
#include "NullPacketSource.h"
#include "BufferManager.h"
#include "DataDebug.h"
#include "datastreamcfg.h"
extern ZQ::common::Config::Loader<DataStreamCfg> gDataStreamConfig;
namespace DataStream {

class StdDistItem: public DistItem {
public:
	virtual void distribute(void* param)
	{
		DataPusher* thisPtr = (DataPusher* )param;
		thisPtr->requestStdSend();
	}
};

class ExtraDistItem: public DistItem {
public:
	virtual void distribute(void* param)
	{
		DataPusher* thisPtr = (DataPusher* )param;
		thisPtr->requestExtraSend();
	}
};

//////////////////////////////////////////////////////////////////////////

class StdSenderWorkItem: public ZQLIB::ZQWorkItem {
public:
	StdSenderWorkItem(ZQLIB::ZQThreadPool& threadPool, 
		DataPusher& pusher): ZQWorkItem(threadPool), _pusher(pusher)
	{
		pusher.refer();
	}

	~StdSenderWorkItem()
	{
		_pusher.release();
	}

#ifdef _DEBUG
	virtual bool init()
	{
		// _initTime = GetTickCount();
		dataDebug.incCounter(DBG_STDSENDER_COUNTER);
		return true;
	}
	
#endif

	int run()
	{
		if (_pusher.doStdSend()) {

			// log error
			return 0;
		}

		return -1;
	}

	void final(int )
	{

#ifdef _DEBUG
		/*
		glog(ZQLIB::Log::L_DEBUG, "StdSenderWorkItem::final()\tduration = %u", 
			GetTickCount() - _initTime);
		*/
		
		dataDebug.decCounter(DBG_STDSENDER_COUNTER);
#endif

		delete this;
	}

protected:
	DataPusher&		_pusher;

#ifdef _DEBUG
	// unsigned long	_initTime;
#endif
};

class ExtraSenderWorkItem: public ZQLIB::ZQWorkItem {
public:
	ExtraSenderWorkItem(ZQLIB::ZQThreadPool& threadPool, 
		DataPusher& pusher): ZQWorkItem(threadPool), _pusher(pusher)
	{
		pusher.refer();
	}

	~ExtraSenderWorkItem()
	{
		_pusher.release();
	}

	int run()
	{
		if (_pusher.doExtraSend()) {
			
			// log error
			return 0;
		}

		return -1;
	}

	void final(int )
	{
		delete this;
	}

protected:
	DataPusher&		_pusher;
};

//////////////////////////////////////////////////////////////////////////

class StdReaderWorkItem: public ZQLIB::ZQWorkItem {
public:
	StdReaderWorkItem(ZQLIB::ZQThreadPool& threadPool, 
		DataPusher& pusher): ZQWorkItem(threadPool), _pusher(pusher)
	{
		pusher.refer();
	}

	~StdReaderWorkItem()
	{
		_pusher.release();
	}

#ifdef _DEBUG

	virtual bool init()
	{
		// _initTime = GetTickCount();
		dataDebug.incCounter(DBG_STDREADER_COUNTER);
		return true;
	}

#endif

	virtual int run()
	{
		if (_pusher.doStdRead())
			return true;

		return false;
	}

	virtual void final(int )
	{
#ifdef _DEBUG
		/*
		glog(ZQLIB::Log::L_DEBUG, "StdReaderWorkItem::final()\tduration = %u", 
			GetTickCount() - _initTime);
		*/
		dataDebug.decCounter(DBG_STDREADER_COUNTER);
#endif
		delete this;
	}

protected:
	DataPusher&		_pusher;

#ifdef _DEBUG
	// unsigned long	_initTime;
#endif

};

class ExtraReaderWorkItem: public ZQLIB::ZQWorkItem {
public:
	ExtraReaderWorkItem(ZQLIB::ZQThreadPool& threadPool, DataPusher& pusher): 
	  ZQWorkItem(threadPool), _pusher(pusher)
	{
		pusher.refer();
	}

	~ExtraReaderWorkItem()
	{
		_pusher.release();
	}

	int run()
	{
		if (_pusher.doExtraRead())
			return 0;
		return -1;
	}

	void final(int )
	{
		delete this;
	}

protected:
	DataPusher&		_pusher;
};

//////////////////////////////////////////////////////////////////////////

DataPusher::DataPusher(ZQLIB::ZQThreadPool& senderThreadPool, 
					   ZQLIB::ZQThreadPool& readerThreadPool, 
					   BufferManager& bufferMgr, 
					   const TransferAddress& addr, 
					   unsigned int rate, DataSender& sender):
						_senderThreadPool(senderThreadPool), 
						_readerThreadPool(readerThreadPool), 
						_bufferMgr(bufferMgr), _addr(addr), 
						_rate(rate), _sender(sender)
{
	_state = StateStopped;
	_extraDist = NULL;
	_stdDist = NULL;
	_sources = NULL;
	_srcCount = 0;
	_stdDistItem = NULL;
	_extraDistItem = NULL;
	_nullPecketSrc = NULL;
	_stdQueue = NULL;
	_extraQueue = NULL;
	_stdBufferPool = NULL;
	_extraBufferPool = NULL;
	_ref = 1;
	_transfer = NULL;
}

DataPusher::~DataPusher()
{
	reset();

	if (_stdDistItem) {
		delete _stdDistItem;
		_stdDistItem = NULL;
	}

	if (_extraDistItem) {
		delete _extraDistItem;
		_extraDistItem = NULL;
	}

	if (_transfer) {
		_transfer->release();
		_transfer = NULL;
	}
}

inline unsigned int DataPusher::calcSumRate(DataSource* srcs[], 
											size_t count) const
{
	size_t rateSum = 0;
	for (size_t i = 0; i < count; i ++) {
		rateSum += srcs[i]->getRate();
	}

	return rateSum;
}

bool DataPusher::setSources(DataSource* srcs[], size_t count)
{
	ZQLIB::AbsAutoLock lock(*this);

	if (_state != StateStopped) {
		// log error
		assert(false);
		return false;
	}
	
	if (srcs == NULL || count == 0) {
		// log error
		assert(false);
		return false;
	}

	if (calcSumRate(srcs, count) > _rate) {
		// log error for invalid configuration
		return false;
	}

	_sources = srcs;
	_srcCount = count;
	return true;
}

bool DataPusher::run()
{
	ZQLIB::AbsAutoLock lock(*this);

	if (_state != StatePaused) {
		if (!prepare()) {
			return false;
		}

		_state = StateRunning;
		doStdRead();

	} else {
		_state = StateRunning;
	}

	_stdDist->start(this);
	if (_extraDist) {
		_extraDist->start(this);
	}
	
	return true;
}

void DataPusher::stop()
{
	ZQLIB::AbsAutoLock lock(*this);
	_state = StateStopped;
	reset();
}

void DataPusher::pause()
{
	ZQLIB::AbsAutoLock lock(*this);
	if (_state == StatePaused) {

		// log waring
		return;
	}

	_stdDist->stop();
	if (_extraDist) {
		_extraDist->stop();
	}
	
	_state = StatePaused;
}

bool DataPusher::prepare()
{
	_transfer = _sender.getTransfer(_addr);
	if (_transfer == NULL) {

		// log error
		return false;
	}

	// organize the data source
	if (!organizeSource()) {
		return false;
	}

	if (!prepareStdDist())
		return false;

	if (!prepareExtraDist())
		return false;

	return true;
}

bool DataPusher::prepareStdDist()
{
	assert(_stdOrgInfo.period);

	if (!_stdDistItem)
		_stdDistItem = new StdDistItem;

	_stdDist = new DataDistributer(_stdOrgInfo.period, _stdDistItem);

	_stdQueue = new DataQueue(_stdOrgInfo.queueMaxSize);
	_stdBufferPool = _bufferMgr.createBufferPool(
		_stdOrgInfo.blockSize, 30, 100);
	
	return true;
}

bool DataPusher::prepareExtraDist()
{
	if (_extraOrgInfo.period != 0) {

		if (!_extraDistItem)
			_extraDistItem = new ExtraDistItem;

		_extraDist = new DataDistributer(_extraOrgInfo.period,
			_extraDistItem);

		_extraBufferPool = _bufferMgr.createBufferPool(
			_extraOrgInfo.blockSize, 30, 100);
	}

	return true;
}

bool DataPusher::doStdSend()
{
	// work within thread pool.

	BufferBlock* block;

	{
		ZQLIB::AbsAutoLock lock(*this);

		if (_state == StateStopped) {
			return true;
		}		
	}

	block = _stdQueue->leave();

	if (block == NULL)
		return false;	
	
	_sender.directSendData(*_transfer, *block);

	{
		ZQLIB::AbsAutoLock lock(*this);
		if (_state == StateStopped) {
			return true;
		}

		if (_stdQueue->getSize() < _stdOrgInfo.queueMinSize) {
			requestStdRead();
		}
	}

	return true;
}

bool DataPusher::doExtraSend()
{
	// work with thread pool.

	BufferBlock* block;

	{
		ZQLIB::AbsAutoLock lock(*this);

		if (_state == StateStopped) {
			return true;
		}		

		block = _extraQueue->leave();
	}

	_sender.directSendData(*_transfer, *block);

	{
		ZQLIB::AbsAutoLock lock(*this);

		if (_state == StateStopped) {
			return true;
		}

		if (_extraQueue->getSize() < _extraOrgInfo.queueMinSize) {
			requestExtraRead();		
		}
	}

	return true;
}

inline bool DataPusher::doStdReadOnce()
{
	DataSource* ds;
	size_t subblockSize;
	BufferBlock* block = _stdBufferPool->allocate();
	BufferBlock* subblock;

	for (size_t i = 0; i < _stdSrcCount; i ++) {
		ds = _sources[i];
		subblockSize = ds->getBlockSize();
		assert(subblockSize);
		subblock = block->getSubBlock(subblockSize);
		if (!ds->nextBlock(*subblock)) {
			block->release();
			assert(false);
			return false;
		}

		subblock->release();
	}

	if (_nullPecketSrc) {

		subblockSize = _nullPecketSrc->getBlockSize();
		assert(subblockSize);
		subblock = block->getSubBlock(subblockSize);
		if (!_nullPecketSrc->nextBlock(*subblock)) {
			block->release();
			assert(false);
			return false;
		}

		subblock->release();
	}

	_stdQueue->enter(block);
	return true;
}

inline bool DataPusher::doExtraReadOnce()
{
	DataSource* ds;
	size_t subblockSize;
	BufferBlock* block = _extraBufferPool->allocate();
	BufferBlock* subblock;

	for (size_t i = _stdSrcCount; i < _srcCount; i ++) {
		ds = _sources[i];
		subblockSize = ds->getBlockSize();
		subblock = block->getSubBlock(subblockSize);
		if (!ds->nextBlock(*subblock)) {
			block->release();
			assert(false);
			return false;
		}
	}

	_extraQueue->enter(block);

	return true;
}

bool DataPusher::doStdRead()
{
	size_t times;

	{
		ZQLIB::AbsAutoLock lock(*this);

		if (_state == StateStopped) {
			return true;
		}

		// 有多少空就读多少, hoho
		times = _stdOrgInfo.queueMaxSize - _stdQueue->getSize();
	}

	for (size_t i = 0; i < times; i ++) {

		if (_state == StateStopped)
			return true;

		if (!doStdReadOnce())
			return false;
	}

	return true;
}

bool DataPusher::doExtraRead()
{
	size_t times;

	{

		ZQLIB::AbsAutoLock lock(*this);

		if (_state == StateStopped) {
			return true;
		}
		
		times = _extraOrgInfo.queueMaxSize - _extraQueue->getSize();
	}
	
	for (size_t i = 0; i < times; i ++) {
		if (!doExtraReadOnce())
			return false;
	}

	return true;
}

void DataPusher::requestStdRead()
{
	StdReaderWorkItem* item = new StdReaderWorkItem(
		_readerThreadPool, *this);
	item->start();
}

void DataPusher::requestExtraRead()
{
	ExtraReaderWorkItem* item = new ExtraReaderWorkItem(
		_readerThreadPool, *this);
	item->start();
}

void DataPusher::requestStdSend()
{
	StdSenderWorkItem* item = new StdSenderWorkItem(
		_senderThreadPool, *this);
	item->start();
}

void DataPusher::requestExtraSend()
{
	ExtraSenderWorkItem* item = new ExtraSenderWorkItem(
		_senderThreadPool, *this);
	item->start();
}

bool srcCmp ( DataSource* elem1, DataSource* elem2 )
{
   return elem1->getRate() > elem2->getRate();
}

bool DataPusher::organizeSource()
{
	// no extra distributer
	// std::sort(&_sources[0], &_sources[_srcCount - 1], srcCmp);
	
	memset(&_extraOrgInfo, 0, sizeof(_extraOrgInfo));

	_stdOrgInfo.period = gDataStreamConfig.stdPeriod;
	_stdOrgInfo.queueMaxSize = gDataStreamConfig.stdMaxQueue;
	_stdOrgInfo.queueMinSize = gDataStreamConfig.stdMinQueue;

	const int timesPreSec = MS_PER_SEC / _stdOrgInfo.period;
	_stdOrgInfo.blockSize = TS_ADJUST_PACKET_SIZE(_rate / timesPreSec / 8);

	_stdSrcCount = _srcCount;

	unsigned int sumBlkSize= 0;
	for (size_t i = 0; i < _srcCount; i ++) {
		DataSource* src = _sources[i];
		size_t blockSize = TS_ADJUST_PACKET_SIZE(
			src->getRate() / timesPreSec / 8);

		src->setParameter(blockSize, _stdOrgInfo.period);
		if (!src->init()) {

			// log error
			return false;
		}

		sumBlkSize += blockSize;
	}
	
	if (sumBlkSize <= _stdOrgInfo.blockSize - TS_PACKET_SIZE) {

		_stdOrgInfo.nullPacketSize = TS_ADJUST_PACKET_SIZE(
			_stdOrgInfo.blockSize - sumBlkSize);

		assert(_nullPecketSrc == NULL);

		// create null packet source
		_nullPecketSrc = new NullPacketSource();

		_nullPecketSrc->setParameter(_stdOrgInfo.nullPacketSize, 
			_stdOrgInfo.period);
		_nullPecketSrc->init();

	} else {
		_stdOrgInfo.nullPacketSize = 0;
		assert(false);
	}

	_stdOrgInfo.blockSize = sumBlkSize + _stdOrgInfo.nullPacketSize;
	
	glog(ZQLIB::Log::L_DEBUG, "DataPusher::organizeSource()\t "
		"blockSize = %d, sumBlkSize = %d, nullPacketSize = %d" LOG_FMT, 
		_stdOrgInfo.blockSize, sumBlkSize, _stdOrgInfo.nullPacketSize, 
		LOG_ARG);

	return true;
}

void DataPusher::clearStdQueue()
{
	for (int i = 0; i < MAX_STDQUEUE_SIZE; i ++) {
		if (_stdQueue->leave() == NULL)
			break;
	}
}

void DataPusher::clearExtraQueue()
{
	// no extra queue
}

void DataPusher::resetStdDist()
{
	if (_stdDist) {
		_stdDist->stop();
		delete _stdDist;
		_stdDist = NULL;
	}

	if (_stdQueue) {
		clearStdQueue();
		delete _stdQueue;
		_stdQueue = NULL;
	}

	if (_stdBufferPool) {
		_bufferMgr.destroyBufferPool(_stdBufferPool);
		_stdBufferPool = NULL;
	}
}

void DataPusher::resetExtraDist()
{
	if (_extraDist) {
		_extraDist->stop();
		delete _extraDist;
		_extraDist = NULL;
	}
	
	if (_extraQueue) {
		clearExtraQueue();
		delete _extraQueue;
		_extraQueue = NULL;
	}

	if (_extraBufferPool) {
		_bufferMgr.destroyBufferPool(_extraBufferPool);
		_extraBufferPool = NULL;
	}
}

void DataPusher::reset()
{
	resetStdDist();
	resetExtraDist();
	if (_nullPecketSrc) {
		delete _nullPecketSrc;
		_nullPecketSrc = NULL;
	}
	
	_sources = NULL;
	_srcCount = 0;
}

} // namespace DataStream {
