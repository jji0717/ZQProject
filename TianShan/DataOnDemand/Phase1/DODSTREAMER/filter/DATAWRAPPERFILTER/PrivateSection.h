
#ifndef PrivateSectionH
#define PrivateSectionH

#include "scqueue.h"
#include "common.h"

struct PrivateSectionHeader
{
	UCHAR	tableid;					//8 bit

	UCHAR	section_length_2:4;			//4 bit high
	UCHAR	reserved:2;					//2 bit
	UCHAR	INVALID:1;					//1 bit ='0'
	UCHAR	indicator:1;				//1 bit ='1'
	
	UCHAR	section_length:8;			//12 bit include CRC header='00' only 10 bit valid max:1023

	UCHAR	table_id_ext_2:8;				// high 16 bit tag transport stream
	UCHAR	table_id_ext:8;			// low
	
	UCHAR	current_next_indicator:1;	//1 bit 1: current valid 0:next valid
	UCHAR	version_number:5;			//5 bit 0~31 PAT Table version
	UCHAR	reserved_next:2;			//2 bit

	UCHAR	section_number;				//8
	UCHAR	last_section_number;		//8
	PrivateSectionHeader()
	{
		indicator = 1;
		INVALID = 0;
		reserved = 3;
		reserved_next = 3;
	}
};
typedef struct PrivateSectionHeader PSHeader;	// 8 bytes

class CPrivateSection
{
public:
	CPrivateSection(int nPID);
	~CPrivateSection();
	void InitSectionNumber( int sectionlen, int total, DWS_SubChannelList pSubChannel );
	bool ChipSection( BYTE * pInBuffer, long inSize, BYTE * pOutBuffer, long bufsize, long * outSize, DWS_SubChannelList pSubChannel );
	bool PackSection( BYTE * pInBuffer, long inSize, BYTE * pOutBuffer, long bufsize, long * outSize, DWS_SubChannelList pSubChannel );
	int  Set_CRC32( UCHAR * m_buf );
private:
	int m_nSectionNumber;
	int m_last_section_number;
	int m_nPID;
};

#endif