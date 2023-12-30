// DataQueue.h: interface for the DataQueue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAQUEUE_H__8C93043C_A946_4FFF_A39A_CAFE7D9BA935__INCLUDED_)
#define AFX_DATAQUEUE_H__8C93043C_A946_4FFF_A39A_CAFE7D9BA935__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SyncUtil.h"

namespace DataStream {

class BufferBlock;

typedef ZQLIB::WaitableQueue2<BufferBlock* > DataQueueBase;

class DataQueue: protected DataQueueBase {
public:
	DataQueue(size_t maxSize);
	virtual ~DataQueue();

	void enter(BufferBlock* p);
	BufferBlock* leave();

	size_t getSize();
};

} // namespace DataStream {

#endif // !defined(AFX_DATAQUEUE_H__8C93043C_A946_4FFF_A39A_CAFE7D9BA935__INCLUDED_)
