
#ifndef TSPackH
#define TSPackH

#include "common.h"
#include "scqueue.h"

#define PAYLOADLEN	184

#define GET_12(p, o)	(((p[o] & 0x0f) << 8) | p[(o)+1])
#define GET_13(p, o)	(((p[o] & 0x1f) << 8) | p[(o)+1])
#define GET_16(p, o)	((p[o] << 8) | p[(o)+1])

struct TransportPacketHeader
{
	UCHAR	sync_byte;							//8 bit =0x47

	UCHAR	PID2:5;
	UCHAR	transport_priority:1;				//1 bit
	UCHAR	payload_unit_start_indicator:1;		//1 bit ='0'
	UCHAR	ts_error_indicator:1;				//1 bit ='0'

	UCHAR	PID:8;								//5+8 bit PID values 0x0000-0x000F are reserved. PID value 0x1FFF is reserved for null packets.
	
	UCHAR	continuity_counter:4;				//4 bit increment 0~max:15 随着每一个具有相同PID的传送流分组而增加，复制分组不变
	UCHAR	adaptation_field_control:2;			//2 bit 
	UCHAR	ts_scrambling_control:2;			//2 bit scrambling mode, no set
	
	TransportPacketHeader()
	{
		sync_byte = 0x47;
		ts_error_indicator = 0;
		payload_unit_start_indicator = 0;
		transport_priority = 0;
		ts_scrambling_control = 0;		
		adaptation_field_control = 1;
		continuity_counter = 0;
	}
};
typedef struct TransportPacketHeader TSHeader;

class CTSPack 
{
public:
	CTSPack();
	~CTSPack();
	void InitSectionStartFlag(  long bufsize, long * outSize );
	bool WrapSection( BYTE * pInBuffer, long inSize, BYTE * pOutBuffer, long * outSize, DWS_SubChannelList pSubChannel );
	bool WrapPacket( BYTE * pInBuffer, long inSize, BYTE * pOutBuffer, long * outSize, DWS_SubChannelList pSubChannel );
	void ClearTS_Packets();
public:
	//vector<UCHAR*> TS_Packets;
	int count;
private:
	int m_section_start_flag;		
	int m_continue_counter;
	int m_outbuffer_length;
};

//extern CTSPack g_tsStream;

#endif