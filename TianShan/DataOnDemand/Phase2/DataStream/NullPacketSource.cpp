// NullPacketSource.cpp: implementation of the NullPacketSource class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NullPacketSource.h"
#include "BufferManager.h"
#include "TsEncoder.h"

namespace DataStream {

NullPacketSource::NullPacketSource():
	DataSource(0)
{

}

NullPacketSource::~NullPacketSource()
{

}

bool NullPacketSource::nextBlock(BufferBlock& block)
{
	size_t blockSize = block.size();
	unsigned char* ptr = (unsigned char* )block.getPtr();
	assert(TS_VALID_SIZE(blockSize));

	size_t blks = blockSize / TS_PACKET_SIZE;
	for (size_t i = 0; i < blks; i ++) {
		unsigned char* packet = ptr + i * TS_PACKET_SIZE;
		buildNullPacket(packet);
	}

	return true;
}

void NullPacketSource::newCycle()
{

}

} // namespace DataStream {
