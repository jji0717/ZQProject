// DataPusher.h: interface for the DataPusher class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAPUSHER_H__768F6971_E2F3_4FE7_B543_C386E5B9AA3F__INCLUDED_)
#define AFX_DATAPUSHER_H__768F6971_E2F3_4FE7_B543_C386E5B9AA3F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataDef.h"
#include "ZQThreadPool.h"

#define MAX_STDQUEUE_SIZE		64
#define MAX_EXTRAQUEUE_SIZE		64

#define MIN_STDQUEUE_SIZE		16
#define MIN_EXTRAQUEUE_SIZE		16

namespace DataStream {

class DataSender;
class DataSource;
class DataDistributer;
class BufferManager;
class DataSender;
class StdDistItem;
class ExtraDistItem;
class StdSenderWorkItem;
class ExtraSenderWorkItem;
class Transfer;
class NullPacketSource;
class DataQueue;
class BufferPool;

class StdReaderWorkItem;
class ExtraReaderWorkItem;

struct OrganizeInfo {
	unsigned int	period;
	size_t			blockSize;
	size_t			nullPacketSize;
	size_t			queueMinSize;
	size_t			queueMaxSize;
};

class DataPusher: protected ZQLIB::LightLock {

	friend class StdDistItem;
	friend class ExtraDistItem;
	friend class StdSenderWorkItem;
	friend class ExtraSenderWorkItem;
	friend class StdReaderWorkItem;
	friend class ExtraReaderWorkItem;

public:
	DataPusher(ZQLIB::ZQThreadPool& senderThreadPool, 
		ZQLIB::ZQThreadPool& readerThreadPool,  BufferManager& bufferMgr, 
		const TransferAddress& addr, unsigned int rate, DataSender& sender);

	bool setSources(DataSource* srcs[], size_t count);

	const TransferAddress& getTransferAddress() const
	{
		return _addr;
	}

	unsigned int getRate() const 
	{
		return _rate;
	}

	DataSender& getSender() const
	{
		return _sender;
	}

	bool run();
	void stop();
	void pause();

	typedef enum {
		StateRunning,
		StateStopped,
		StatePaused
	} State;

	State getState() const
	{
		return _state;
	}

	long refer()
	{
		return InterlockedIncrement(&_ref);
	}

	long release()
	{
		long r = InterlockedDecrement(&_ref);
		if (r == 0) {
			delete this;
		}

		return r;
	}

protected:
	virtual ~DataPusher();
	
	unsigned int calcSumRate(DataSource* srcs[], size_t count) const;
	
	bool prepare();

	bool prepareStdDist();
	bool prepareExtraDist();

	void requestStdSend();
	void requestExtraSend();

	void requestStdRead();
	void requestExtraRead();

	bool doStdSend();
	bool doExtraSend();

	bool doStdRead();
	bool doExtraRead();

	bool doStdReadOnce();
	bool doExtraReadOnce();

	void stdDist();
	void extraDist();
	bool organizeSource();

	void reset();
	void resetStdDist();
	void resetExtraDist();
	
	void clearStdQueue();
	void clearExtraQueue();

protected:
	ZQLIB::ZQThreadPool&	_senderThreadPool;
	ZQLIB::ZQThreadPool&	_readerThreadPool;
	DataSource**			_sources;
	size_t					_srcCount;
	DataDistributer*		_stdDist;
	DataDistributer*		_extraDist;
	unsigned int			_rate;
	BufferManager&			_bufferMgr;
	DataSender&				_sender;
	TransferAddress			_addr;
	Transfer*				_transfer;
	volatile State			_state;					
	StdDistItem*			_stdDistItem;
	ExtraDistItem*			_extraDistItem;
	NullPacketSource*		_nullPecketSrc;
	size_t					_stdSrcCount;
	BufferPool*				_stdBufferPool;
	BufferPool*				_extraBufferPool;
	DataQueue*				_stdQueue;
	DataQueue*				_extraQueue;
	OrganizeInfo			_stdOrgInfo;
	OrganizeInfo			_extraOrgInfo;
	long					_ref;
};

} // namespace DataStream {

#endif // !defined(AFX_DATAPUSHER_H__768F6971_E2F3_4FE7_B543_C386E5B9AA3F__INCLUDED_)
