// TsWrapper.h: interface for the TsWrapper class.
//
//////////////////////////////////////////////////////////////////////

// author:	XiaoTao
// date:	2007-03-16

#if !defined(AFX_TSWRAPPER_H__39542625_849F_4577_B7C1_05CE42844836__INCLUDED_)
#define AFX_TSWRAPPER_H__39542625_849F_4577_B7C1_05CE42844836__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#define TS_SYNC_BYTE					0x47
#define TS_PACKET_SIZE					188
#define	TS_PAT_PID						0
#define	TS_PAT_TABLEID					0
#define	TS_PMT_TABLEID					2
#define	TS_NULL_PID						0x1fff
#define TS_MAX_PID						0x1fff
#define TS_MAX_SECTION_LENGTH			4096
#define TS_NULL_PADDING					0xff

#define TS_MAX_SECTION_NUMBER			255
#define TS_MAX_TABLE_NMUBER				0xffff
#define TS_MAX_TABLE_LENGTH				((TS_MAX_SECTION_LENGTH + 1) * \
											(TS_MAX_SECTION_NUMBER + 1))

#define TS_VALID_SIZE(theSize)			((theSize) && (theSize) % TS_PACKET_SIZE == 0)

#define MS_PER_SEC				1000

#define _ADJUST_SIZE(unit_size, size)		\
	((((size) + ((unit_size) - 1)) / (unit_size)) * (unit_size))

#define TS_ADJUST_PACKET_SIZE(size)	\
	_ADJUST_SIZE(TS_PACKET_SIZE, size)

namespace DataStream {

/*
	void initializeTsHeader(TsHeader* tsHdr, Word pid, 
		Byte transport_priority, Byte  payload_unit_start_indicator, 
		Byte transport_error_indicator, Byte continuity_counter, 
		Byte _payload_present_flag, Byte _adaptation_field_present_flag, 
		Byte transport_scrambling_control);
*/
#define initializeTsHeader(tsHdr, pid, transportPriority, \
	payloadUnitStartIndicator, transportErrorIndicator, \
	continuityCounter, payloadPresentFlag, \
	adaptationFieldPresentFlag, transportScramblingControl) \
	{ \
		(tsHdr)->sync_byte = TS_SYNC_BYTE; \
		(tsHdr)->pid_high = ((pid) >> 8) & 0x1f; \
		(tsHdr)->pid_low = (pid) & 0xff; \
		(tsHdr)->transport_priority = transportPriority; \
		(tsHdr)->payload_unit_start_indicator = payloadUnitStartIndicator; \
		(tsHdr)->transport_error_indicator = transportErrorIndicator; \
		(tsHdr)->continuity_counter = continuityCounter; \
		(tsHdr)->_payload_present_flag = payloadPresentFlag; \
		(tsHdr)->_adaptation_field_present_flag = adaptationFieldPresentFlag; \
		(tsHdr)->transport_scrambling_control = transportScramblingControl; \
	}


/*
	void initializeTsHeaderEx(TsHeader* tsHdr, Word pid, 
		Byte transport_priority, Byte  payload_unit_start_indicator, 
		Byte transport_error_indicator, Byte continuity_counter, 
		Byte _payload_present_flag, Byte _adaptation_field_present_flag, 
		Byte transport_scrambling_control, Byte point_field);
*/
#define initializeTsHeaderEx(tsHdr, pid, transportPriority, \
	payloadUnitStartIndicator, transportErrorIndicator, \
	continuityCounter, payloadPresentFlag, \
	adaptationFieldPresentFlag, transportScramblingControl, pointField) \
	{ \
		(tsHdr)->sync_byte = TS_SYNC_BYTE; \
		(tsHdr)->pid_high = ((pid) >> 8) & 0x1f; \
		(tsHdr)->pid_low = (pid) & 0xff; \
		(tsHdr)->transport_priority = transportPriority; \
		(tsHdr)->payload_unit_start_indicator = payloadUnitStartIndicator; \
		(tsHdr)->transport_error_indicator = transportErrorIndicator; \
		(tsHdr)->continuity_counter = continuityCounter; \
		(tsHdr)->_payload_present_flag = payloadPresentFlag; \
		(tsHdr)->_adaptation_field_present_flag = adaptationFieldPresentFlag; \
		(tsHdr)->transport_scrambling_control = transportScramblingControl; \
		(tsHdr)->point_field = pointField; \
	}

/*
	Byte setTsHeaderCounter(TsHeader* tsHdr, Byte count);
*/
#define setTsHeaderCounter(tsHdr, counter) \
		((tsHdr)->continuity_counter = counter)

/*
	Byte getTsHeaderCounter(TsHeader* tsHdr);
*/
#define getTsHeaderCounter(tsHdr)			((tsHdr)->continuity_counter)

/*
	Word getTsHeaderPid(TsHeader* tsHdr);
*/
#define getTsHeaderPid(tsHdr)				\
	(((unsigned short)(tsHdr)->pid_high) << 8 | (tsHdr)->pid_low)

/*
	void setTsHeaderPid(TsHeader* tsHdr, Word pid);
*/
#define setTsHeaderPid(tsHdr, pid)			\
	{ \
		(tsHdr)->pid_high = ((pid) >> 8) & 0x1f; \
		(tsHdr)->pid_low = (pid) & 0xff; \
	}

/*
	Byte setPayloadUnitStartIndicator(TsHeader* tsHdr);
	Byte clearPayloadUnitStartIndicator(TsHeader* tsHdr);
*/
#define setPayloadUnitStartIndicator(tsHdr) \
		((tsHdr)->payload_unit_start_indicator = 1)
#define clearPayloadUnitStartIndicator(tsHdr) \
		((tsHdr)->payload_unit_start_indicator = 0)

typedef struct _TsHeader // 4 BYTES
{
	unsigned char sync_byte;	// First byte
	unsigned char pid_high : 5;	// Second byte - Notice the reverse declaration
	unsigned char transport_priority : 1;
	unsigned char payload_unit_start_indicator : 1;
	unsigned char transport_error_indicator : 1;
	unsigned char pid_low;		// Third byte
	unsigned char continuity_counter : 4;	// Fourth byte
	unsigned char _payload_present_flag : 1;
	unsigned char _adaptation_field_present_flag : 1;
	unsigned char transport_scrambling_control : 2;
	// unsigned char point_field; // if payload_unit_start_indicator == 1

} TsHeader;

typedef struct _TsHeaderEx // 4 BYTES
{
	unsigned char sync_byte;	// First byte
	unsigned char pid_high : 5;	// Second byte - Notice the reverse declaration
	unsigned char transport_priority : 1;
	unsigned char payload_unit_start_indicator : 1;
	unsigned char transport_error_indicator : 1;
	unsigned char pid_low;		// Third byte
	unsigned char continuity_counter : 4;	// Fourth byte
	unsigned char _payload_present_flag : 1;
	unsigned char _adaptation_field_present_flag : 1;
	unsigned char transport_scrambling_control : 2;
	unsigned char point_field; // if payload_unit_start_indicator == 1

} TsHeaderEx;

	
/*
	void initializePatHeader(PatHeader* patHdr,	Byte tableId, Word sectionLen, 
		Byte sectionSyntaxIndicator, Word transportStreamId, Byte currentNextIndicator, 
		Byte versinNumber, Byte sectionNumber, Byte lastSectionNumber );
*/
#define initializePatHeader(patHdr, tableId, sectionLen, \
	sectionSyntaxIndicator, transportStreamId, currentNextIndicator, \
	versinNumber, sectionNumber, lastSectionNumber ) \
	{ \
		(patHdr)->table_id = tableId; \
		(patHdr)->section_length_highbits = ((sectionLen) >> 8) & 0x0f; \
		(patHdr)->section_length_lowbits = (sectionLen) & 0xff; \
		(patHdr)->reserved_twobits = 0; \
		(patHdr)->must_be_zero = 0; \
		(patHdr)->section_syntax_indicator = sectionSyntaxIndicator; \
		(patHdr)->transport_stream_id_hi = ((transportStreamId) >> 8) & 0xff; \
		(patHdr)->transport_stream_id_lo = (transportStreamId) & 0xff; \
		(patHdr)->current_next_indicator = currentNextIndicator; \
		(patHdr)->version_number = versinNumber; \
		(patHdr)->reserved_twobits_2 = 0; \
		(patHdr)->section_number = sectionNumber; \
		(patHdr)->last_section_number = lastSectionNumber; \
	}

typedef struct _PatSectionHeader { // 8 BYTES

	unsigned char table_id;
	unsigned char section_length_highbits : 4;
	unsigned char reserved_twobits : 2;
	unsigned char must_be_zero : 1;
	unsigned char section_syntax_indicator : 1;
	unsigned char section_length_lowbits;
	unsigned char transport_stream_id_hi;
	unsigned char transport_stream_id_lo;
	unsigned char current_next_indicator : 1;
	unsigned char version_number : 5;
	unsigned char reserved_twobits_2 : 2;
	unsigned char section_number;
	unsigned char last_section_number;

} PatSectionHeader, PatHeader;

/*
  void initializePatEntry(PatEntry* patEntry, Word programNumber, Word pid);
*/
#define initializePatEntry(patEntry, programNumber, pid) \
	{ \
		(patEntry)->program_number_hi = ((programNumber) >> 8) & 0xff; \
		(patEntry)->program_number_lo = (programNumber) & 0xff; \
		(patEntry)->program_map_PID_highbits = ((pid) >> 8) & 0x1f; \
		(patEntry)->reserved = 0; \
		(patEntry)->program_map_PID_lo = (pid)  & 0xff; \
	}

typedef struct _PatSectionProgramMap { // 4 BYTES

	unsigned char program_number_hi;
	unsigned char program_number_lo;
	unsigned char program_map_PID_highbits : 5;
	unsigned char reserved : 3;
	unsigned char program_map_PID_lo;

} PatSectionProgramMap, PatEntry;

/*
void initializePmtHeader(PatHeader* pmtHdr, Byte tableId, Word sectionLen, 
	Byte sectionSyntaxIndicator, Word programNumber, Byte currentNextIndicator, 
	Byte versinNumber, Byte sectionNumber, Byte lastSectionNumber, Word pcrPid,  
	Word progInfoLen);

*/
#define initializePmtHeader(pmtHdr, tableId, sectionLen, \
	sectionSyntaxIndicator, programNumber, currentNextIndicator, \
	versinNumber, sectionNumber, lastSectionNumber, pcrPid,  progInfoLen) \
	{ \
		(pmtHdr)->table_id = tableId; \
		(pmtHdr)->section_length_highbits = ((sectionLen) >> 8) & 0x0f; \
		(pmtHdr)->section_length_lowbits = (sectionLen) & 0xff; \
		(pmtHdr)->reserved_twobits = 0; \
		(pmtHdr)->must_be_zero = 0; \
		(pmtHdr)->section_syntax_indicator = sectionSyntaxIndicator; \
		(pmtHdr)->program_number_hi = ((programNumber) >> 8) & 0xff; \
		(pmtHdr)->program_number_lo = (programNumber) & 0xff; \
		(pmtHdr)->current_next_indicator = currentNextIndicator; \
		(pmtHdr)->version_number = versinNumber; \
		(pmtHdr)->reserved_twobits_2 = 0; \
		(pmtHdr)->section_number = sectionNumber; \
		(pmtHdr)->last_section_number = lastSectionNumber; \
		(pmtHdr)->PCR_PID_highbits = ((pcrPid) >> 8) & 0x1f; \
		(pmtHdr)->reserved_threebits = 0; \
		(pmtHdr)->PCR_PID_lo = (pcrPid) & 0xff; \
		(pmtHdr)->program_info_length_highbits = ((progInfoLen) >> 8) & 0x0f; \
		(pmtHdr)->reserved_fourbits = 0; \
		(pmtHdr)->program_info_length_lo = (progInfoLen) & 0xff; \
	}

		
typedef struct _PmtSectionHeader { //12 BYTES

	unsigned char table_id;
	unsigned char section_length_highbits : 4;
	unsigned char reserved_twobits : 2;
	unsigned char must_be_zero : 1;
	unsigned char section_syntax_indicator : 1;
	unsigned char section_length_lowbits;
	unsigned char program_number_hi;
	unsigned char program_number_lo;
	unsigned char current_next_indicator : 1;
	unsigned char version_number : 5;
	unsigned char reserved_twobits_2 : 2;
	unsigned char section_number;
	unsigned char last_section_number;
	unsigned char PCR_PID_highbits : 5;
	unsigned char reserved_threebits : 3;
	unsigned char PCR_PID_lo;
	unsigned char program_info_length_highbits : 4;
	unsigned char reserved_fourbits : 4;
	unsigned char program_info_length_lo;

} PmtSectionHeader, PmtHeader;


/*
	void initializePmtEntry(PmtEntry* pmtEntry, Byte streamType, 
		Word elememtaryPid, Word esInfoLen);
*/
#define initializePmtEntry(pmtEntry, streamType, elememtaryPid, esInfoLen) \
	{ \
		(pmtEntry)->stream_type = streamType; \
		(pmtEntry)->elementary_PID_highbits = ((elememtaryPid) >> 8) & 0x1f; \
		(pmtEntry)->reserved = 0; \
		(pmtEntry)->elementary_PID_lo = (elememtaryPid) & 0xff; \
		(pmtEntry)->ES_info_length_hibits = ((esInfoLen) >> 8) & 0x0f; \
		(pmtEntry)->reserved2 = 0; \
		(pmtEntry)->ES_info_length_lo = (esInfoLen) & 0xff; \
	}

typedef struct _PmtSectionStreamMap // 5 BYTES
{
	
	unsigned char stream_type;	// 0x01 mpeg1 video; 0x02 mpeg2 video; 
								// 0x03 mpeg1 audio; 0x04 mpeg2 audio;

	unsigned char elementary_PID_highbits : 5;
	unsigned char reserved : 3;
	unsigned char elementary_PID_lo;
	unsigned char ES_info_length_hibits : 4;
	unsigned char reserved2 : 4;
	unsigned char ES_info_length_lo;

} PmtSectionStreamMap, PmtEntry;


/*
	void initializePrivateHeader(PatHeader* patHdr,	Byte tableId, Word sectionLen, 
		Byte sectionSyntaxIndicator, Word tableIdExt, Byte currentNextIndicator, 
		Byte versinNumber, Byte sectionNumber, Byte lastSectionNumber );
*/
#define initializePrivateHeader(priHdr, tableId, sectionLen, \
	sectionSyntaxIndicator, tableIdExt, currentNextIndicator, \
	versinNumber, sectionNumber, lastSectionNumber ) \
	{ \
		(priHdr)->table_id = tableId; \
		(priHdr)->section_length_highbits = ((sectionLen) >> 8) & 0x0f; \
		(priHdr)->section_length_lowbits = (sectionLen) & 0xff; \
		(priHdr)->reserved_twobits = 0; \
		(priHdr)->must_be_zero = 0; \
		(priHdr)->section_syntax_indicator = sectionSyntaxIndicator; \
		(priHdr)->table_id_extension_hi = ((tableIdExt) >> 8) & 0xff; \
		(priHdr)->table_id_extension_lo = (tableIdExt) & 0xff; \
		(priHdr)->current_next_indicator = currentNextIndicator; \
		(priHdr)->version_number = versinNumber; \
		(priHdr)->reserved_twobits_2 = 0; \
		(priHdr)->section_number = sectionNumber; \
		(priHdr)->last_section_number = lastSectionNumber; \
	}

/*
	Byte increaseSectionHeaderSectionNumber(SectionHeader* secHdr);
*/
#define increaseSectionHeaderSectionNumber(secHdr) \
		(++ (secHdr)->section_number)

/*
	Byte increaseSectionHeaderVersionNumber(SectionHeader* secHdr);
*/
#define increaseSectionHeaderVersionNumber(secHdr) \
		(++ (secHdr)->version_number)

/*
	Byte getSectionHeaderTableId(SectionHeader* secHdr);
*/
#define getSectionHeaderTableId(secHdr)		((secHdr)->table_id)

/*
	Byte setSectionHeaderTableId(SectionHeader* secHdr, tableId);
*/
#define setSectionHeaderTableId(secHdr, tableId)	\
	((secHdr)->table_id = tableId)

/*
	Word getSectionHeaderTableExtId(SectionHeader* secHdr);
*/
#define getSectionHeaderTableExtId(secHdr) \
	((((unsigned short)((secHdr)->table_id_extension_hi)) << 8) | \
	((secHdr)->table_id_extension_lo))

/*
	void setSectionHeaderTableExtId(SectionHeader* secHdr, Word tableExtId);
*/
#define setSectionHeaderTableExtId(secHdr, tableExtId) \
	{ \
		(secHdr)->table_id_extension_hi = ((tableExtId) >> 8) & 0xff; \
		(secHdr)->table_id_extension_lo = (tableExtId) & 0xff; \
	}

typedef struct _PrivateSectionHeader { // 8 BYTES

	unsigned char table_id;
	unsigned char section_length_highbits : 4;
	unsigned char reserved_twobits : 2;
	unsigned char must_be_zero : 1;
	unsigned char section_syntax_indicator : 1;
	unsigned char section_length_lowbits;
	unsigned char table_id_extension_hi;
	unsigned char table_id_extension_lo;
	unsigned char current_next_indicator : 1;
	unsigned char version_number : 5;
	unsigned char reserved_twobits_2 : 2;
	unsigned char section_number;
	unsigned char last_section_number;

} PrivateSectionHeader, PrivateHeader, GenericSectionHeader, GenericHeader;

//////////////////////////////////////////////////////////////////////////

inline void buildNullPacket(unsigned char* packet)
{
	initializeTsHeader((TsHeader* )packet, TS_NULL_PID, 0, 0, 
		0, 0, 1, 0, 0);
	memset(packet + sizeof(TsHeader), TS_NULL_PADDING, 
		TS_PACKET_SIZE - sizeof(TsHeader));
}

//////////////////////////////////////////////////////////////////////////

class TsData {

protected:
	TsData()
	{
		_totalSize = 0;
	}
	
public:

	TsData(size_t size)
	{
		_totalSize = size;
	}
	
	virtual size_t read(unsigned char* buf, size_t size) = 0;

	virtual size_t getSize()
	{
		return _totalSize;
	}

protected:
	size_t		_totalSize;
};

class TsMemData: public TsData {
public:
	TsMemData(unsigned char* ptr, size_t size): TsData(size), _ptr(ptr)
	{
		_pos = 0;
	}

	size_t read(unsigned char* buf, size_t size)
	{
		if (_pos >= _totalSize)
			return 0;

		size_t actualSize = _totalSize - _pos > size ? size : 
			_totalSize - _pos;
		size = actualSize;
		memcpy(buf, _ptr + _pos, actualSize);
		_pos += actualSize;
		return actualSize;
	}

protected:
	unsigned char*	_ptr;
	size_t			_pos;
};

class TsPacketEncoder {
public:
	TsPacketEncoder(unsigned short packetId);
	virtual ~TsPacketEncoder();

	size_t encode(unsigned char* data, size_t dataLen, unsigned char* outBuf);

protected:
	virtual size_t encodePacket(unsigned char* data, size_t dataLen, 
		unsigned char startIndicator);

protected:
	unsigned short			_packetId;
	unsigned char*			_outBufPos;
};

class TsSectionEncoder {

	friend class TsEncoder;

public:
	TsSectionEncoder(unsigned char tableId, unsigned short tableExtId, 
		unsigned char verNum = 1);
	virtual ~TsSectionEncoder();

	size_t encode(unsigned char* outBuf, TsData& data, size_t dataLen);

protected:

	TsEncoder* getEncoder()
	{
		return _tsEncoder;
	}

	void setEncoder(TsEncoder* tsEncoder) 
	{
		_tsEncoder = tsEncoder;
	}

	unsigned char getTableId()
	{
		return _tableId;
	}

	unsigned char setTableId(unsigned char tableId)
	{
		return _tableId = tableId;
	}

	unsigned short setTableExtId(unsigned short tableExtId)
	{
		return _tableExtId = tableExtId;
	}

	unsigned short getTableExtId()
	{
		return _tableExtId;
	}

	size_t encodeSection(unsigned char* buf, TsData& data, 
		unsigned char secNum, unsigned short secLen, 
		unsigned char lastSecNum);
	
	unsigned int setCRC32(unsigned char* sec, unsigned short secLen);

protected:
	virtual size_t getSectionHeaderSize()
	{
		return sizeof(GenericSectionHeader);
	}

	virtual bool initSecHdr(unsigned char* buf, unsigned char secNum, 
		unsigned short secLen, unsigned char lastSecNum);

	virtual unsigned short updateTableExtId()
	{
		return _tableExtId ++;
	}

	virtual TsPacketEncoder* createPacketEncoder();

protected:
	TsEncoder*			_tsEncoder;
	TsPacketEncoder*	_packetEncoder;
	unsigned char*		_secBuf;
	size_t				_secPolyloadSize;	
	unsigned char		_tableId;
	unsigned short		_tableExtId;
	unsigned char		_verNum;
};

class TsEncoder {
public:
	TsEncoder();
	virtual ~TsEncoder();
	
	bool encode(unsigned short pid, TsSectionEncoder* sectionEncoder, 
		TsData& data);
	
	unsigned char* nextTable(size_t& len);

	unsigned short getPacketId()
	{
		return _pid;
	}

	unsigned short getTableCount()
	{
		return _tableCount;
	}
	
protected:
	unsigned char* allocateBuffer(size_t dataLen, 
		size_t tablePolyloadSize);

	size_t encodeTable(TsData& data, size_t dataLen, 
		unsigned char* outBuf);

protected:
	size_t				_tablePolyloadSize;
	size_t				_tableCount;
	size_t				_nextTable;
	unsigned short		_pid;
	unsigned char*		_buf;
	size_t				_bufSize;
	unsigned long		_outBufSize;
	TsSectionEncoder*	_sectionEncoder;
	TsData*				_data;
	size_t				_tableBufferSize;
};

class TsPatSectionEncoder: public TsSectionEncoder {
public:
	TsPatSectionEncoder(unsigned short tableExtId, 
		unsigned char verNum = 1);

	virtual size_t getSectionHeaderSize()
	{
		return sizeof(PatSectionHeader);
	}

	virtual bool initSecHdr(unsigned char* buf, unsigned char secNum, 
		unsigned short secLen, unsigned char lastSecNum);

};

class TsPmtSectionEncoder: public TsSectionEncoder {
public:
	TsPmtSectionEncoder(unsigned short tableExtId, 
		unsigned char verNum = 1);

	virtual size_t getSectionHeaderSize()
	{
		return sizeof(PmtSectionHeader);
	}

	virtual bool initSecHdr(unsigned char* buf, unsigned char secNum, 
		unsigned short secLen, unsigned char lastSecNum);

};

} // namespace DataStream {

#endif // !defined(AFX_TSWRAPPER_H__39542625_849F_4577_B7C1_05CE42844836__INCLUDED_)
