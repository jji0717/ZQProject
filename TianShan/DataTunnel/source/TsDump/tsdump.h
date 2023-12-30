#ifndef _TSDUMP_H_
#define _TSDUMP_H_

#define TSDUMP_MAJOR_VERION			1
#define TSDUMP_MINOR_VERION			8

#define TS_PACKAGE_SIZE				188
#define TS_PACKAGE_LEAD				0x47

#define DEFAULTTAGLENGTH			4
#define DEFAULTOBJECTKEYLENGTH		4

#include <vector>
#include <string>

#include <pshpack1.h>

typedef struct _TsHeader // 4 BYTES
{
	unsigned char sync_byte;		// First byte
	unsigned char pid_high : 5;	// Second byte - Notice the reverse declaration
	unsigned char transport_priority : 1;
	unsigned char payload_unit_start_indicator : 1;
	unsigned char transport_error_indicator : 1;
	unsigned char pid_low;	// Third byte
	unsigned char continuity_counter : 4;	// Fourth byte
	unsigned char _payload_present_flag : 1;
	unsigned char _adaptation_field_present_flag : 1;
	unsigned char transport_scrambling_control : 2;
	// unsigned char point_field; // if payload_unit_start_indicator == 1
} TsHeader;

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
} PatSectionHeader;

typedef struct _PatSectionProgramMap { // 4 BYTES
	unsigned char program_number_hi;
	unsigned char program_number_lo;
	unsigned char program_map_PID_highbits : 5;
	unsigned char reserved : 3;
	unsigned char program_map_PID_lo;
} PatSectionProgramMap;

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
} PmtSectionHeader;

typedef struct _PmtSectionStreamMap // 5 BYTES
{
	unsigned char stream_type; // 0x01 mpeg1 video; 0x02 mpeg2 video; 0x03 mpeg1 audio; 0x04 mpeg2 audio;
	unsigned char elementary_PID_highbits : 5;
	unsigned char reserved : 3;
	unsigned char elementary_PID_lo;
	unsigned char ES_info_length_hibits : 4;
	unsigned char reserved2 : 4;
	unsigned char ES_info_length_lo;
} PmtSectionStreamMap;


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
} PrivateSectionHeader;

typedef PrivateSectionHeader PSHeader;

//////////////////////////////////////////////////////////////////////////
// cut from dod server project

typedef struct StrObjectHeader{
	char		sObjectTag[DEFAULTTAGLENGTH];
	char		dwReserved[4];
	char		nObjectKeyLength;
	char		sObjectKey[DEFAULTOBJECTKEYLENGTH];
	char		dwObjectContentLength[4];
	//char		*pchObjectContent;
	StrObjectHeader()
	{
		memset( sObjectTag, 0, DEFAULTTAGLENGTH );
		memset( dwReserved, 0xFF, 4 );
		nObjectKeyLength = DEFAULTOBJECTKEYLENGTH;
		memset( sObjectKey, 0, DEFAULTOBJECTKEYLENGTH );
		memset( dwObjectContentLength, 0, 4 );
	}
} ObjectHeader;		// by test, sizeof(ObjectHeader) = 17

#define DEFAULTINDEXDESCLENGTH		16

typedef struct ObjectIndexDescriptor
{
	BYTE	byTag;
	BYTE	byLength;
	BYTE	byTableCount;
	BYTE	byIDLength;									// = DEFAULTOBJECTKEYLENGTH
	char	pchID[DEFAULTOBJECTKEYLENGTH];
	BYTE	byDescLength;								// = DEFAULTINDEXDESCLENGTH
	char	pchDesc[DEFAULTINDEXDESCLENGTH];
	ObjectIndexDescriptor()
	{
		byTag = 0x80;
		byLength = sizeof(ObjectIndexDescriptor) - 2;
		byTableCount = 0;
		byIDLength = DEFAULTOBJECTKEYLENGTH;
		byDescLength = DEFAULTINDEXDESCLENGTH;
	}
} ObjectDescriptor;

#include <poppack.h>

//////////////////////////////////////////////////////////////////////////

class TSSource {
public:
	virtual bool open(const char* srcName) = 0;
	virtual size_t recv(void* buf, size_t len) = 0;
	virtual bool close() = 0;

	virtual bool listItems(std::vector<std::string>& items)
	{
		return false;
	}
};

#define PROCESS_FINISH			0
#define PROCESS_OK				1
#define PROCESS_ERROR			2
#define PROCESS_FAULT			3

class TSWorker {
public:
	virtual bool init()
	{
		return true;
	}

	virtual int process(ULONG index, char package[TS_PACKAGE_SIZE]) = 0;
	
	virtual void final()
	{

	}
};

#endif // #ifndef _TSDUMP_H_
