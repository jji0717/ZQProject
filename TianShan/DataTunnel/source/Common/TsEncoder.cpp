// TsWrapper.cpp: implementation of the TsWrapper class.
//
//////////////////////////////////////////////////////////////////////

#include "TsEncoder.h"

// author:	XiaoTao
// date:	2007-03-16

namespace DataStream {

//////////////////////////////////////////////////////////////////////////

TsPacketEncoder::TsPacketEncoder(unsigned short packetId):
								 _packetId(packetId)								 
{
	
}

TsPacketEncoder::~TsPacketEncoder()
{

}

size_t TsPacketEncoder::encode(unsigned char* data, size_t dataLen, 
							   unsigned char* outBuf)
{
	const size_t pktPolyloadSize = TS_PACKET_SIZE - sizeof(TsHeader);
	size_t pktCount = ((dataLen - 1) + (pktPolyloadSize - 1)) / 
		pktPolyloadSize;

	_outBufPos = outBuf;

	size_t remainSize = dataLen;
	unsigned char* inPos = data;
	for (size_t i = 0; i < pktCount; i ++) {
	
		size_t encodeLen = encodePacket(inPos, 
			remainSize, i == 0);

		remainSize -= encodeLen;
		inPos += encodeLen;
	}

	return pktCount * TS_PACKET_SIZE;
}

size_t TsPacketEncoder::encodePacket(unsigned char* data, size_t dataLen, 
									 unsigned char startIndicator)
{
	TsHeaderEx tsHdr;
	
	initializeTsHeaderEx(&tsHdr, 
		_packetId,		// pid
		0,				// transportPriority
		startIndicator,	// payloadUnitStartIndicator
		0,				// transportErrorIndicator
		0,				// continuityCounter
		1,				// payloadPresentFlag
		0,				// adaptationFieldPresentFlag
		0,				// transportScramblingControl
		0);				// pointField
	
	size_t tsHdrLen;

	if (startIndicator) {
		tsHdrLen = sizeof(TsHeaderEx);
	} else {
		tsHdrLen = sizeof(TsHeader);
	}
	
	memcpy(_outBufPos, &tsHdr, tsHdrLen);
	_outBufPos += tsHdrLen;

	const size_t pktPolyloadSize = TS_PACKET_SIZE - tsHdrLen;

	size_t actualSize = pktPolyloadSize < dataLen ? pktPolyloadSize : dataLen;
	memcpy(_outBufPos, data, actualSize);

	if (actualSize < pktPolyloadSize) {
		memset(_outBufPos + dataLen, TS_NULL_PADDING, 
			pktPolyloadSize - dataLen);
	}

	_outBufPos += pktPolyloadSize;
	return actualSize;
}

//////////////////////////////////////////////////////////////////////////

TsSectionEncoder::TsSectionEncoder(unsigned char tableId, 
								   unsigned short tableExtId, 
								   unsigned char verNum /* = 0 */): 
	_tableId(tableId), _tableExtId(tableExtId), _verNum(verNum)
{
	size_t bufSize = TS_MAX_SECTION_LENGTH + getSectionHeaderSize();

	_secBuf = (unsigned char* )malloc(bufSize);
	memset(_secBuf, 0xff, bufSize);
	_packetEncoder = NULL;
}

TsSectionEncoder::~TsSectionEncoder()
{
	free(_secBuf);
	if (_packetEncoder) {
		delete _packetEncoder;
#ifdef _DEBUG
		_packetEncoder = NULL;
#endif
	}
}

TsPacketEncoder* TsSectionEncoder::createPacketEncoder()
{
	unsigned short pid = _tsEncoder->getPacketId();
	return new TsPacketEncoder(pid);
}

size_t TsSectionEncoder::encode(unsigned char* outBuf, 
								TsData& data, 
								size_t dataLen)
{
	if (_tsEncoder == NULL) {
		// log error
		assert(false);
		return 0;
	}

	if (_packetEncoder == NULL)
		_packetEncoder = createPacketEncoder();

	_secPolyloadSize = TS_MAX_SECTION_LENGTH - 
		getSectionHeaderSize() - /* crc */ 4;

	size_t sectionCount = (dataLen + (_secPolyloadSize - 1)) / 
		_secPolyloadSize;

	assert(sectionCount <= 0x100);

	unsigned char lastSecNum = (unsigned char)(sectionCount - 1);	
	unsigned char* bufPos = outBuf;

	size_t secLen = _secPolyloadSize;
	for (size_t i = 0; i < sectionCount; i ++) {
		
		if (i == sectionCount - 1 && dataLen % _secPolyloadSize)
			secLen = dataLen % _secPolyloadSize;

		size_t encodedSecLen = encodeSection(_secBuf, data, 
			(unsigned char )i, secLen, lastSecNum);

		size_t tsLen = _packetEncoder->encode(_secBuf, 
			encodedSecLen, bufPos);
		
		// assert(tsLen % TS_PACKET_SIZE == 0);
		bufPos += tsLen;
	}

	return bufPos - outBuf;
}

bool TsSectionEncoder::initSecHdr(unsigned char* buf, 
								  unsigned char secNum, 
								  unsigned short secLen, 
								  unsigned char lastSecNum)
{
	GenericSectionHeader* secHdr = (GenericSectionHeader* )buf;
	size_t encodedLen = secLen + getSectionHeaderSize() - 
		3 /* section_hdr_part */ + 4 /* (crc) */;

	initializePrivateHeader(secHdr, 
		_tableId,			// tableId
		encodedLen,			// sectionLen
		1,					// sectionSyntaxIndicator
		getTableExtId(),	// tableExtensionId
		1,					// currentNextIndicator
		_verNum,			// versinNumber
		secNum,				// sectionNumber
		lastSecNum);		// lastSectionNumber

	return true;
}

size_t TsSectionEncoder::encodeSection(unsigned char* buf, 
									   TsData& data, 
									   unsigned char secNum, 
									   unsigned short secLen, 
									   unsigned char lastSecNum)
{
	if (!initSecHdr(buf, secNum, secLen, lastSecNum)) {
		// log error
		return 0;
	}

	size_t secHdrLen = getSectionHeaderSize();
	size_t encodedLen = secLen + secHdrLen - 
		3 /* section_hdr_part */ + 4 /* (crc) */;
	
	size_t readLen = data.read(buf + secHdrLen, secLen);
	assert(readLen == secLen);
	setCRC32(buf, encodedLen);
	return secHdrLen + secLen + 4 /* crc */;
}

unsigned int TsSectionEncoder::setCRC32(unsigned char* sec, 
								  unsigned short secLen)
{
	int  i, j, k;
	unsigned int z[32], bit, crc = 0;
	unsigned short len = secLen + 3 -4;
	unsigned char * p = sec;
	unsigned int CRCFlag = 0x04c11db6;

	for (i = 0; i < 32; i ++)
		z[i] = 1;

	for (k = 0; k < len; k ++) {
		for (j = 0; j < 8; j ++) {

			bit = (*(p + k) >> (7 - j)) & 0x01;
			bit ^= z[31];                              
			for(i = 31; i > 0; i--) {
				if(CRCFlag & (1 << i))  
					z[i] = z[i-1] ^ bit;
				else
					z[i] = z[i-1];
			}

			z[0] = bit;
		}
	}

	for(i = 31; i >= 0; i --)  
		crc |= ((z[i] & 0x01) << i);

	sec[len + 4 - 1] = (unsigned char)crc;
	sec[len + 4 - 2] = (unsigned char)(crc >> 8);
	sec[len + 4 - 3] = (unsigned char)(crc >> 16);
	sec[len + 4 - 4] = (unsigned char)(crc >> 24);
	return crc;
}

//////////////////////////////////////////////////////////////////////////

TsEncoder::TsEncoder()
{
	_tablePolyloadSize = 0;
	_tableCount = 0;
	_buf = NULL;
	_data = NULL;
	_nextTable = 0xffffffff;
	_tableBufferSize = 0;
}

TsEncoder::~TsEncoder()
{
	if (_buf)
		free(_buf);
}

bool TsEncoder::encode(unsigned short pid, 
					   TsSectionEncoder* sectionEncoder, 
					   TsData& data)
{
	_pid = pid;
	_sectionEncoder = sectionEncoder;
	sectionEncoder->setEncoder(this);

	_tablePolyloadSize = (TS_MAX_SECTION_LENGTH - 
		_sectionEncoder->getSectionHeaderSize() - /* crc */ 4) * 256;

	_data = &data;
	size_t dataLen = data.getSize();
	_tableCount = (dataLen + (_tablePolyloadSize - 1)) / 
		_tablePolyloadSize;

	allocateBuffer(dataLen, _tablePolyloadSize);
	_nextTable = 0;

	return true;
}

unsigned char* TsEncoder::nextTable(size_t& len)
{
	if (_data == NULL) {
		// log error;
		assert(false);		
		return NULL;
	}

	if (_buf == NULL) {
		assert(false);
		// log error
		return NULL;
	}

	if (_nextTable >= _tableCount) {
		len = 0;
		return NULL;
	}

	size_t dataLen = _data->getSize();

	size_t tableLen;
	if (_nextTable == _tableCount - 1 && 
		dataLen % _tablePolyloadSize) {

		tableLen = dataLen % _tablePolyloadSize;
	} else {

		tableLen = _tablePolyloadSize;
	}

	size_t encodeLen = encodeTable(*_data, tableLen, _buf);
	assert(encodeLen <= _tableBufferSize);
	_nextTable ++;
	len = encodeLen;
	_sectionEncoder->updateTableExtId();
	return _buf;
}

unsigned char* TsEncoder::allocateBuffer(size_t dataLen, 
										 size_t tablePolyloadSize)
{
	if (_buf) {
		free(_buf);
	}

	// 256 个 section 乘以 每个节的包数 乘以 包的大小, 得到每张表最大字节数
	_tableBufferSize = 256 * (TS_MAX_SECTION_LENGTH / 184 + 1) * 
		TS_PACKET_SIZE;

	_buf = (unsigned char* )malloc(_tableBufferSize); // ~= 1M
	
	return _buf;
}

size_t TsEncoder::encodeTable(TsData& data, 
							size_t dataLen, 
							unsigned char* outBuf)
{
	assert(_sectionEncoder);
	return _sectionEncoder->encode(outBuf, data, dataLen);
}

//////////////////////////////////////////////////////////////////////////

TsPatSectionEncoder::TsPatSectionEncoder(unsigned short tableExtId, 
										 unsigned char verNum /* = 1 */):
	TsSectionEncoder(TS_PAT_TABLEID, tableExtId, verNum)
{

}

bool TsPatSectionEncoder::initSecHdr(unsigned char* buf, 
									 unsigned char secNum, 
									 unsigned short secLen, 
									 unsigned char lastSecNum)
{
	PatSectionHeader* secHdr = (PatSectionHeader* )buf;
	size_t encodedLen = secLen + getSectionHeaderSize() - 
		3 /* section_hdr_part */ + 4 /* (crc) */;

	initializePatHeader(secHdr, 
		_tableId,			// tableId
		encodedLen,			// sectionLen
		1,					// sectionSyntaxIndicator
		getTableExtId(),	// tableExtensionId
		1,					// currentNextIndicator
		_verNum,			// versinNumber
		secNum,				// sectionNumber
		lastSecNum);		// lastSectionNumber

	return true;
}

//////////////////////////////////////////////////////////////////////////

TsPmtSectionEncoder::TsPmtSectionEncoder(unsigned short tableExtId, 
										 unsigned char verNum /* = 1 */):
	TsSectionEncoder(TS_PMT_TABLEID, tableExtId, verNum)
{

}

bool TsPmtSectionEncoder::initSecHdr(unsigned char* buf, 
									 unsigned char secNum, 
									 unsigned short secLen, 
									 unsigned char lastSecNum)
{
	PmtSectionHeader* secHdr = (PmtSectionHeader* )buf;
	size_t encodedLen = secLen + getSectionHeaderSize() - 
		3 /* section_hdr_part */ + 4 /* (crc) */;

	initializePmtHeader(secHdr, 
		_tableId,			// tableId
		encodedLen,			// sectionLen
		1,					// sectionSyntaxIndicator
		getTableExtId(),	// tableExtensionId
		1,					// currentNextIndicator
		_verNum,			// versinNumber
		secNum,				// sectionNumber
		lastSecNum,			// lastSectionNumber
		TS_NULL_PID,		// pcrPid
		0);					// progInfoLen

	return true;
}

} // namespace DataStream {
