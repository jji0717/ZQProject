// DataSource.cpp: implementation of the DataSource class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataDef.h"
#include "TsEncoder.h"
#include "DataSource.h"
#include "DataQueue.h"
#include "DataReader.h"
#include "ZQThreadPool.h"
#include "BufferManager.h"

namespace DataStream {

DataSource::DataSource(unsigned int rate, unsigned int repeatDelay): 
	_rate(rate), _repeatDelay(repeatDelay)
{
	_delayCurrent = 0;
	_delayPackets = 0;
	_blockSize = 0;
	_frequence = 0;
	_reader = NULL;
	_nextReader = NULL;
	_switched = false;
}

DataSource::~DataSource()
{
	if (_reader) {
		_reader->release();
	}

	if (_nextReader && _nextReader != _reader)
		_nextReader->release();
}

unsigned char DataSource::getPacketCounter(unsigned short pid)
{
	PidCountMap::iterator it = _pidCountMap.find(pid);
	if (it == _pidCountMap.end()) {
		std::pair<PidCountMap::iterator, bool> r;
		r = _pidCountMap.insert(PidCountMap::value_type(pid, 0));
		
		if (!r.second) {
			// log error
			assert(false);
			return 0;
		}

		it = r.first;;
	}

	unsigned char res = it->second;
	it->second = (it->second + 1) % 0x10;
	return res;
}

void DataSource::setParameter(size_t blockSize, unsigned int freq)
{
	_blockSize = blockSize;
	_frequence = freq;
}

bool DataSource::init()
{
	_delayPackets = _repeatDelay / _frequence * 
		_blockSize / TS_PACKET_SIZE;

	_delayCurrent = _delayPackets - 1;
	return true;
}

bool DataSource::fixPacket(unsigned char* ptr, size_t size)
{
	for(size_t i = 0; i < size / TS_PACKET_SIZE; i ++) {
		TsHeader* tsHdr = (TsHeader* )(ptr + i * TS_PACKET_SIZE);

		if (getTsHeaderPid(tsHdr) != TS_NULL_PID) {

			setTsHeaderCounter(tsHdr, 
				getPacketCounter(getTsHeaderPid(tsHdr)));
		}
	}

	return true;
}

void DataSource::switchReader(DataReader* reader)
{
	ZQLIB::AbsAutoLock autoLock(*this);

	if (_reader == NULL)
		_reader = reader;
	else {
		if (_nextReader)
			_nextReader->release();

		_nextReader = reader;
		_switched = true;
	}	
}

bool DataSource::nextBlock(BufferBlock& block)
{
	ZQLIB::AbsAutoLock autoLock(*this);

	unsigned char* blockPtr = (unsigned char* )block.getPtr();
	size_t blockSize = block.size();
	
	assert(blockSize == 0 || TS_VALID_SIZE(blockSize));

	if (_reader == NULL) {
		fillNullPackets(blockPtr, blockSize);
		return true;
	}

	if (_delayCurrent < _delayPackets - 1) {

		int blockPkts = blockSize / TS_PACKET_SIZE;
		int fillPkgs;

		if (blockPkts < _delayPackets - _delayCurrent - 1) {
			fillPkgs = blockPkts;
		} else {
			fillPkgs = _delayPackets - _delayCurrent - 1;
		}

		const size_t fileBytes = fillPkgs * TS_PACKET_SIZE;
		assert(fileBytes <= blockSize);

		fillNullPackets(blockPtr, fileBytes);

		_delayCurrent += fillPkgs;
		assert(_delayCurrent < _delayPackets);

		blockPtr += fileBytes;
		blockSize -= fileBytes;
	}

	if (blockSize) {

		size_t readSize = _reader->read(blockPtr, blockSize);
		assert(readSize == 0 || TS_VALID_SIZE(readSize));

		if (readSize < blockSize) {
			fillNullPackets(blockPtr + readSize, blockSize - readSize);
		}

		if (_reader->isEnd()) {
			newCycle();
			_delayCurrent = 0;
		}

		if (!fixPacket(blockPtr, blockSize)) {
			// log error
			return false;
		}
	}

	return true;
}

void DataSource::newCycle()
{
	ZQLIB::AbsAutoLock autoLock(*this);
	assert(_reader);

	if (_switched) {
		
		_reader->release();
		_reader = _nextReader;
		_nextReader = NULL;
		_pidCountMap.clear();

		_switched = false;

	} else {
		
		if (_reader)
			_reader->reset();

		// ?? 重新计数
		// _pidCountMap.clear();
	}
}

void DataSource::fillNullPackets(unsigned char* buf, size_t len)
{
	assert(TS_VALID_SIZE(len));
	size_t pkts = len / TS_PACKET_SIZE;
	for (size_t i = 0; i < pkts; i ++) {
		buildNullPacket(buf + i * TS_PACKET_SIZE);
	}
}

} // namespace DataStream {
