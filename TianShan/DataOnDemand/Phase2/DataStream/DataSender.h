// DataSender.h: interface for the DataSender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATASENDER_H__275D4229_4AE9_4CFA_99EC_8A499639D5CF__INCLUDED_)
#define AFX_DATASENDER_H__275D4229_4AE9_4CFA_99EC_8A499639D5CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZQThreadPool.h"
#include "DataDef.h"

namespace DataStream {

class BufferBlock;
class TransferAddress;
class DataSender;
class SendDataWorkItem;
class Transfer;

struct PacketRate {
	unsigned short	pid;
	unsigned long	rate;
	unsigned long	totalPackets;
	unsigned long	lastUpdate;
};


class TransferProfile {

	friend class Transfer;
public:
	unsigned long getTotalRate() {return 0;}
	size_t getPacketCount() {return 0;}
	PacketRate* getPacketRate(int index) {return 0;}

protected:
	void report(unsigned short pid, unsigned short packets){return;}

protected:
	typedef std::vector<PacketRate> PacketRateVec;

	PacketRateVec	_packetRates;
};

class Transfer: public ZQLIB::LightLock {

	friend class DataSender;
	friend class SendDataWorkItem;

protected:
	Transfer();
	virtual bool init();
	virtual size_t send(void* buf, size_t len);
	virtual long refer();

public:
	virtual long release();
	static bool InitSocket();

protected:
	long	_ref;
};

class DataSender: protected ZQLIB::LightLock {
public:
	DataSender(ZQLIB::ZQThreadPool&	sendPool);
	virtual ~DataSender();

	Transfer* getTransfer(const TransferAddress& addr);
	
	void sendData(Transfer& tran, BufferBlock& block);
	size_t directSendData(Transfer& tran, BufferBlock& block);

protected:
	Transfer* createTransfer(const TransferAddress& addr);

protected:
	ZQLIB::ZQThreadPool&	_sendPool;
	typedef std::map<TransferAddress, Transfer* > TranMap;
	TranMap					_tranMap;
};

} // namespace DataStream {

#endif // !defined(AFX_DATASENDER_H__275D4229_4AE9_4CFA_99EC_8A499639D5CF__INCLUDED_)
