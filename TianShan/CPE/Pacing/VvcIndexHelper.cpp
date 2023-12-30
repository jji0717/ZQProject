// ===========================================================================
// Copyright (c) 2010 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================



#include "VvcIndexHelper.h"

#define PRINTF	printf

#define PARSE_START_OFFSET		(_buf + _nParseStartBytes)
#define PARSE_DATA_LENGTH		(_nDataLen - _nParseStartBytes)

VvcIndexHelper::VvcIndexHelper()
{
	_nDataLen = 0;

	_nParseStartBytes = 0;
//	_nParsedDataBytes = 0;		//related to moveforward

	_nParseState = PS_FILEHEADER;
	_nInputDataOffset = 0;

	_nTotalBytes = 0;
}

VvcIndexHelper::ParseState VvcIndexHelper::getParseState()
{
	return (ParseState)_nParseState;
}

// generate an aligned pointer to a TLV. skip any
// leading zeros. advance index past the TLV header.
CTF_TLV* VvcIndexHelper::alignTLV(char *pBuf, int *pIndex )
{
	CTF_TLV *pTLV;

	while( pBuf[ *pIndex ] == 0 )
	{
		++( *pIndex );
	}
	pTLV = (CTF_TLV *)( pBuf + ( *pIndex ) );
	*pIndex += (int)sizeof(CTF_TLV);
	return pTLV;
}

int VvcIndexHelper::parseIndexRecord( VVC_INDEX_RECORD_HEADER** ppRecHeader, char** ppRecord, int& nRecordLen )
{
	// read the rest of the index file and write it to the temp file
	// NOTE: most of the data records of a VVC index file do not require modification
	// during normalization. Most of the records pertain to the main file, which is
	// never sparse. Likewise the records that apply to FF files. Of the records that
	// pertain to FR files, only the trick speed I frame records have an absolute packet
	// number. The rest of the trick file records  use packet offsets from the "zero byte"
	// (i.e. the first real packet in the file, NOT from offset zero )
	int adjust = 0;
	while(!PARSE_START_OFFSET[adjust])
		adjust++;

	// read the next index record header
	int request = (int)sizeof(VVC_INDEX_RECORD_HEADER);
	if (PARSE_DATA_LENGTH < adjust + request)
		return 0;

	VVC_INDEX_RECORD_HEADER* pRec = (VVC_INDEX_RECORD_HEADER *)(PARSE_START_OFFSET + adjust);
	// read the body of the TLV, plus any trailing alignment zeroes
	
	if (PARSE_DATA_LENGTH < adjust + request + pRec->recLength)
		return 0;

	*ppRecHeader = pRec;
	*ppRecord = PARSE_START_OFFSET + adjust + sizeof(VVC_INDEX_RECORD_HEADER);
	nRecordLen = (int)pRec->recLength;

	return adjust + request + nRecordLen;
}

int VvcIndexHelper::parseHeaderBlock(char** ppHeadBlock, int& nBlockLen)
{
	if (_nParseState != PS_INDEXHEADER)
		return 0;

	// read the index header block tag and length field
	// NOTE: index header TLV is always aligned,
	// but is always followed by 2 alignment bytes
	int hdrBytes = (int)sizeof(CTF_TLV) + 2;
	if (PARSE_DATA_LENGTH < hdrBytes)
		return false;

	// make sure the tag is correct
	CTF_TLV* pHdrTLV = (CTF_TLV *)PARSE_START_OFFSET;
	if( pHdrTLV->tag != VVC_TAG_INDEX_HEADER_BLOCK )
	{
		PRINTF( "Incorrect index header block tag (read 0x%x, expected 0x%x) during normalization\n", pHdrTLV->tag, VVC_TAG_INDEX_HEADER_BLOCK );
		return false;
	}
	
	// read the rest of the index header block, plus any trailing alignment zeros
	int request = (int)pHdrTLV->len;
	int adjust = ( 4 - ( hdrBytes + request ) ) & 0x03;
	request += adjust;
	if (PARSE_DATA_LENGTH - hdrBytes < request)
	{
		return false;
	}

	_nParseState = PS_INDEXREC;

	*ppHeadBlock = PARSE_START_OFFSET + hdrBytes;
	nBlockLen = (int)pHdrTLV->len;
	return hdrBytes + request;
}

int VvcIndexHelper::parseFileHeader( VVC_INDEX_FILE_HEADER** ppFileHeader )
{
	if (_nParseState != PS_FILEHEADER)
		return 0;

	int request = (int)sizeof(VVC_INDEX_FILE_HEADER);
	if (PARSE_DATA_LENGTH < request)
	{
		// not enough data, return
		return 0;
	}

	_nTotalBytes += request;
	_nParseState = PS_INDEXHEADER;
	*ppFileHeader = (VVC_INDEX_FILE_HEADER*)_buf;
	return request;
}

bool VvcIndexHelper::removeIndexData()
{
	//_nParseStartBytes 
	if (_nDataLen > _nParseStartBytes)
	{
		for(int i=_nParseStartBytes;i<_nDataLen;i++)
		{
			_buf[i-_nParseStartBytes] = _buf[i];
		}

		_nDataLen -= _nParseStartBytes;
	}
	else
	{
		_nDataLen = 0;
	}

	_nParseStartBytes = 0;
	return true;
}

bool VvcIndexHelper::appendIndexData( const char* buf, int size )
{
	if (_nDataLen + size > INDEX_BUFFER_SIZE)
	{
		// buffer overflow
		return false;
	}

	memcpy(_buf + _nDataLen, buf, size);
	_nDataLen += size;
	_nInputDataOffset += size;

	return true;
}

int VvcIndexHelper::getInputDataOffset()
{
	return _nInputDataOffset;
}

bool VvcIndexHelper::moveForwardData( int nParsedBytes )
{
	_nParseStartBytes += nParsedBytes;
	return true;
}

const char* VvcIndexHelper::getOutputDataPointer()
{
	return _buf;
}

int VvcIndexHelper::getOutputDataLength()
{
	return _nParseStartBytes;
}

int VvcIndexHelper::getAllDataLength()
{
	return _nDataLen;
}

int VvcIndexHelper::getOutputDataOffset()
{
	return _nInputDataOffset - _nDataLen;
}

/*
hdrBytes += request;
// scan the index header TLVs
int nFileCount = 0;
while( hdrIndex < hdrLimit )
{
pHdrTLV = alignTLV( PARSE_START_OFFSET, &hdrIndex );
// is this a subfile info subblock?
if( pHdrTLV->tag == VVC_TAG_SUB_FILE_INFO_BLOCK )
{
// yes - set up to scan the subfile info block
int subLen = (int)pHdrTLV->len;
int subIndex = hdrIndex;
int subLimit = subIndex + subLen;
// make sure that the end of the block is in the header
if( subLimit > hdrLimit )
{
PRINTF( "Subfile info block not contained within index header limits during normalization\n" );
break;
}
// scan the subfile info TLVs. Count the number of FILE entries.
// record the initial starting offset of each file and the direction
// NOTE: some speed files may have been stubbed out
int fileNumber  = -1;
int firstOffset = -1;
int finalOffset = -1;
int direction   = -1;
while( subIndex < subLimit )
{ 
CTF_TLV* pSubTLV = alignTLV( PARSE_START_OFFSET, &subIndex );
char* pTmp = PARSE_START_OFFSET + subIndex;
switch( pSubTLV->tag )
{
case VVC_TAG_SUB_FILE_INDEX:
fileNumber = DWD( pTmp );
break;
case VVC_TAG_SUB_FILE_STARTING_BYTE:
firstOffset = BY5( pTmp );
break;
case VVC_TAG_SUB_FILE_ENDING_BYTE:
finalOffset = BY5( pTmp );
break;
case VVC_TAG_SUB_FILE_PLAY_DIRECTION:
direction = *pTmp;
break;
default:
break;
}
subIndex += pSubTLV->len;
}; // while( subIndex < subLimit )
// did we fail to get a file number, start byte, end byte. or direction?
if( ( fileNumber  == -1 ) ||
( firstOffset == -1 ) ||
( finalOffset == -1 ) ||
( direction   == -1 ) )
{
PRINTF( "Subfile info block did not contain file number, start byte, end byte, or direction during normalization\n" );
break;
}
// record the start and end byte offsets
//pFirstOffsets[ fileNumber ] = firstOffset;
//pFinalOffsets[ fileNumber ] = finalOffset;
// record the direction
//dir[ fileNumber ] = direction;
// increment the file count

} // if( pHdrTLV->tag == VVC_TAG_SUB_FILE_INFO_BLOCK )
// advance to the next TLV
hdrIndex += pHdrTLV->len;
}; // while( hdrIndex < hdrLimit )

*/

