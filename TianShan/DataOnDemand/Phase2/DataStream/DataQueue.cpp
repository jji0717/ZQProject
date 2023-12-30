// DataQueue.cpp: implementation of the DataQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataQueue.h"
#include "BufferManager.h"

namespace DataStream {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DataQueue::DataQueue(size_t maxSize): 
	DataQueueBase(maxSize)
{

}

DataQueue::~DataQueue()
{

}

void DataQueue::enter(BufferBlock* block)
{
	push(block);
}

BufferBlock* DataQueue::leave()
{
	BufferBlock* block;
	if (!pop(block, 100))
		return NULL;

	return block;
}

size_t DataQueue::getSize()
{
	return size();
}

} // namespace DataStream {
