// DataSource.h: interface for the DataSource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATASOURCE_H__36E7534A_0B4A_45B4_B2CD_9B82D8BC03A6__INCLUDED_)
#define AFX_DATASOURCE_H__36E7534A_0B4A_45B4_B2CD_9B82D8BC03A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZQThreadPool.h"
#include "DataQueue.h"
#include "BufferManager.h"

namespace DataStream {

class BufferBlock;
class BufferPool;
class DataQueue;
class DataReader;
class ReadBlocksWorkItem;

class DataSource: protected ZQLIB::LightLock {

	friend class DataPusher;
	
public:

	DataSource(unsigned int rate, unsigned int repeatDelay = 0);

	virtual ~DataSource();

	virtual void switchReader(DataReader* reader);
	virtual bool nextBlock(BufferBlock& block);

	unsigned int getRate() const 
	{
		return _rate;
	}

	size_t getBlockSize() const
	{
		return _blockSize;
	}

	unsigned int getRepeatDelay() const
	{
		return _repeatDelay;
	}

	unsigned int getFrequence() const
	{
		return _frequence;
	}

	void setParameter(size_t blockSize, unsigned int freq);

protected:
	virtual bool init();
	virtual void newCycle();
	
	virtual bool fixPacket(unsigned char* ptr, size_t size);
	unsigned char getPacketCounter(unsigned short pid);

	void fillNullPackets(unsigned char* buf, size_t len);
	
protected:	
	unsigned int		_rate;
	unsigned int		_repeatDelay;
	unsigned int		_frequence;
	int					_delayPackets;
	int					_delayCurrent;
	size_t				_blockSize;
	DataReader*			_reader;
	DataReader*			_nextReader;
	bool				_switched;

	typedef std::map<unsigned short, unsigned char> PidCountMap;
	PidCountMap			_pidCountMap;
};

} // namespace DataStream {

#endif // !defined(AFX_DATASOURCE_H__36E7534A_0B4A_45B4_B2CD_9B82D8BC03A6__INCLUDED_)
