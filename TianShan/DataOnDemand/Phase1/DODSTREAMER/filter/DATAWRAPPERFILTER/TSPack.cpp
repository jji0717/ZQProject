

#include "stdafx.h"
#include "TSPack.h"
//#include "Pack.h"
//----------
//vector<UCHAR*> TS_Packets(400);	//400*188=75200
//CTSPack g_tsStream;
//---------------
CTSPack::CTSPack() : m_section_start_flag(1)
{
}

CTSPack::~CTSPack()
{
	ClearTS_Packets();
}

void CTSPack::InitSectionStartFlag( long bufsize, long * outSize )
{
	m_section_start_flag = 1;
	m_continue_counter = 0;
	m_outbuffer_length = bufsize;
}

bool CTSPack::WrapSection( BYTE * pInBuffer, long inSize, BYTE * pOutBuffer, long * outSize, DWS_SubChannelList pSubChannel )
{
	//*outSize = 0;
	int payloadlength = PAYLOADLEN;
	if( m_section_start_flag )
	{		
		payloadlength = PAYLOADLEN - 1;	//pointer_field
	}
	if( inSize > payloadlength )
	{
		if( !WrapPacket( pInBuffer, payloadlength, pOutBuffer, outSize, pSubChannel ) )
			return false;
		WrapSection( pInBuffer+payloadlength, inSize-payloadlength, pOutBuffer, outSize, pSubChannel );
	}
	else
	{
		if( !WrapPacket( pInBuffer, inSize, pOutBuffer, outSize, pSubChannel ) )
			return false;
	}
	return true;
}

bool CTSPack::WrapPacket( BYTE * pInBuffer, long inSize, BYTE * pOutBuffer, long * outSize, DWS_SubChannelList pSubChannel )
{
	if( *outSize+188 > m_outbuffer_length )
		return false;

	UCHAR uSection[188];	
	memset( uSection, 0xFF, 188 );

	//generate header
	TSHeader tsHeader;
	tsHeader.PID = pSubChannel.lChannelPID  & 0xFF;
	tsHeader.PID2 = pSubChannel.lChannelPID >> 8;
	
	if( m_continue_counter > 15 )
		m_continue_counter = 0;
	tsHeader.continuity_counter = m_continue_counter;
	m_continue_counter++;
	
	if( m_section_start_flag )
		tsHeader.payload_unit_start_indicator = 1;
	else
		tsHeader.payload_unit_start_indicator = 0;

	memcpy( uSection, &tsHeader, sizeof(TSHeader) );
	
	int headerlen = sizeof(TSHeader);
	if( m_section_start_flag )
	{
		uSection[4] = 0x00;
		headerlen++;
		m_section_start_flag = 0;
	}

	memcpy( uSection+headerlen, pInBuffer, inSize );
	count++;

	memcpy( pOutBuffer+*outSize, uSection, 188 );
	*outSize += 188;
	//TRACE( "TS PACKET COUNT: %d", count );
	//TS_Packets.push_back( uSection );
	return true;
}

void CTSPack::ClearTS_Packets()
{/*
	UCHAR *pcTmp;
	vector<UCHAR*>::iterator itrPakcet;
	for( itrPakcet = TS_Packets.begin(); itrPakcet != TS_Packets.end(); ++itrPakcet )
	{
		pcTmp = (UCHAR*)(*itrPakcet);
		if( pcTmp )
		{
			delete [] pcTmp;
			pcTmp = NULL;
		}
	}*/
}
//---------------