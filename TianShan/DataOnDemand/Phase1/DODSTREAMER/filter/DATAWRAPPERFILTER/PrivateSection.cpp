
#include "stdafx.h"
#include "PrivateSection.h"
#include "TSPack.h"

CPrivateSection::CPrivateSection(int nPID) : m_nSectionNumber(0)
{
	m_nPID = nPID;
}

CPrivateSection::~CPrivateSection()
{
}

void CPrivateSection::InitSectionNumber( int sectionlen, int total, DWS_SubChannelList pSubChannel )
{
	m_nSectionNumber = 0;
	m_last_section_number = total/sectionlen + ( total%sectionlen?1:0 ) - 1;	// -1: because start from 0
}

int CPrivateSection::Set_CRC32( UCHAR * m_buf )
{
 int sectionLen =((m_buf[1] & 0x0f) << 8) | m_buf[2]; //-liqing-

 int  i, j, k;
 UINT z[32], bit, crc = 0;
 USHORT len = sectionLen+3-4;
 UCHAR * p = m_buf;
 ULONG CRCFlag = 0x04c11db6;
 for(i=0; i<32; i++) z[i] = 1;
 for(k=0; k<len; k++)
 {
  for(j=0; j<8; j++)
  {
   bit = (*(p+k) >>(7-j)) & 0x01;
   bit ^= z[31];                              
   for(i=31;i>0;i--)
   {
    if(CRCFlag & (1<<i))  z[i] = z[i-1] ^ bit;
    else  z[i] = z[i-1];
   }
   z[0] = bit;
  }
 }
 for(i=31; i>=0; i--)  crc |= ((z[i]&0x01) << i);
 m_buf[len+4-1] = (UCHAR)crc;
 m_buf[len+4-2] = (UCHAR)(crc >> 8);
 m_buf[len+4-3] = (UCHAR)(crc >> 16);
 m_buf[len+4-4] = (UCHAR)(crc >> 24);
 return crc;
}


bool CPrivateSection::ChipSection( BYTE * pInBuffer, long inSize, BYTE * pOutBuffer, long bufsize, long * outSize, DWS_SubChannelList pSubChannel )
{//recursion call
		int unitlength = LONGSECTION; //pSubChannel.wReserve;	// SHORTSECTION or LONGSECTION

		if( inSize > unitlength )
		{
			if( !PackSection( pInBuffer, unitlength, pOutBuffer, bufsize, outSize, pSubChannel ) )
				return false;
			ChipSection( pInBuffer+unitlength, inSize-unitlength, pOutBuffer, bufsize, outSize, pSubChannel );
		}
		else
		{
			if( !PackSection( pInBuffer, inSize, pOutBuffer, bufsize, outSize, pSubChannel ) )
				return false;
		}
		return true;
}

bool CPrivateSection::PackSection( BYTE * pInBuffer, long inSize, BYTE * pOutBuffer, long bufsize, long * outSize, DWS_SubChannelList pSubChannel )
{	
	UCHAR uSection[PRISECTIONMAXLEN];	// assign local memory
	char chLogMsg[MAX_PATH];

	PSHeader psHeader;
	psHeader.tableid = pSubChannel.byTableID;
	//psHeader.indicator = 1;
	//psHeader.INVALID = 0;
	//psHeader.reserved = ;
	USHORT length = (USHORT)(inSize+9);			// 5: LONGSectionheader-SHORTSectionheader 4:CRC
	psHeader.section_length = length & 0xFF;	
	psHeader.section_length_2 = length >> 8;	
	psHeader.table_id_ext = pSubChannel.wTableIDExtension & 0xFF;  // tag transport_stream number
	psHeader.table_id_ext_2 = pSubChannel.wTableIDExtension >> 8;
	//psHeader.reserved_next = ;
	psHeader.version_number = pSubChannel.wReserve;
	psHeader.current_next_indicator = 1;
	psHeader.section_number = m_nSectionNumber++;
	psHeader.last_section_number = m_last_section_number;	

	wsprintf(chLogMsg,"PID=%d:PackSection() - sec_num:%d last_sec_num:%d.",m_nPID, m_nSectionNumber, m_last_section_number);LogMyEvent(2,0,chLogMsg);

	memcpy( uSection, &psHeader, sizeof(PSHeader) );
	memcpy( uSection+sizeof(PSHeader), pInBuffer, inSize );
	
	Set_CRC32( uSection );

	// wrap a section to multiple ts packets.
	CTSPack tsPacket;
	tsPacket.InitSectionStartFlag( bufsize, outSize );
	if( !tsPacket.WrapSection( uSection, inSize+sizeof(PSHeader)+4, pOutBuffer, outSize, pSubChannel ) )
		return false;
	return true;
}
