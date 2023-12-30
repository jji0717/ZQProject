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



#include "PacedIndex.h"


#define PACED_INDEX				"PacedIndex"
#define MOLOG					(*_log)

using namespace	ZQ::common;

void PacedIndexVvc::setIndexWriter( PacedIndexWrite* pWriter )
{
	_pIndexWriter = pWriter;
}

void PacedIndexVvc::setLog( ZQ::common::Log* pLog )
{
	if (pLog)
	{
		_log = pLog;
		_pacingLogic.setLog(pLog);
	}
}

PacedIndexVvc::PacedIndexVvc()
{
	//ZQ::common::setGlogger();
}

void PacedIndexVvc::close()
{
	paceRecords();
	releaseAllRecs();
}

bool PacedIndexVvc::releaseAllRecs()
{
	if (_indexHelper.getAllDataLength())
	{
		_pIndexWriter->write(_indexHelper.getOutputDataPointer(), _indexHelper.getAllDataLength());

		MOLOG(Log::L_INFO, CLOGFMT(PACED_INDEX, "[%s]: %d bytes not releasabled when close"), 
			_strLogHint.c_str(), _indexHelper.getAllDataLength());
	}

	return true;
}

void PacedIndexVvc::release()
{
	delete this;
}

bool PacedIndexVvc::subfileWritten( const char* ext, int size, int64 offset )
{
	_pacingLogic.subFileWritten(ext, size, offset);

	return paceRecords();
}

bool PacedIndexVvc::writeIndex( const char* buf, int size, int offset )
{
	if (_indexHelper.getInputDataOffset() != offset )
	{
		_pIndexWriter->seek(offset);
		_pIndexWriter->write(buf, size);	

		//reset to the last position
		_pIndexWriter->seek(_indexHelper.getOutputDataOffset());

		return true;
	}

	_indexHelper.appendIndexData(buf, size);

	return paceRecords();
}

bool PacedIndexVvc::paceRecords()
{
	while(1)
	{
		VvcIndexHelper::ParseState state = _indexHelper.getParseState();
		if(state == VvcIndexHelper::PS_INDEXREC)	// put it here, because it's the small path
		{
			VVC_INDEX_RECORD_HEADER* pRecHdr = NULL;
			char* pRecData = NULL;
			int nRecLen = 0;
			int nParsedBytes = _indexHelper.parseIndexRecord(&pRecHdr, &pRecData, nRecLen);
			if (nParsedBytes < 0)
			{
				//err
				return false;
			}
			else if (!nParsedBytes)
			{
				//no enough data
				break;
			}

			// read the data, and check if the data if releasable or not
			if (!_pacingLogic.indexReleasable(pRecHdr, pRecData, nRecLen))
				break;

			_indexHelper.moveForwardData(nParsedBytes);
		}
		else if(state == VvcIndexHelper::PS_FILEHEADER)
		{
			VVC_INDEX_FILE_HEADER* pHeader = NULL;
			int nParsedBytes = _indexHelper.parseFileHeader(&pHeader);
			if (nParsedBytes < 0)
			{
				//err
				return false;
			}
			else if (!nParsedBytes)
			{
				//no enough data
				break;
			}

			// save some data?
			_indexHelper.moveForwardData(nParsedBytes);
		}
		else
		{
			char* pIndexHead = NULL;
			int nHeadLen = 0;

			int nParsedBytes = _indexHelper.parseHeaderBlock(&pIndexHead, nHeadLen);
			if (nParsedBytes < 0)
			{
				//err
				return false;
			}
			else if (!nParsedBytes)
			{
				//no enough data
				break;
			}

			_pacingLogic.parseIndexHeadBlock(pIndexHead, nHeadLen);

			_indexHelper.moveForwardData(nParsedBytes);
		}
	}

	if (_indexHelper.getOutputDataLength())
	{
		_pIndexWriter->write(_indexHelper.getOutputDataPointer(), _indexHelper.getOutputDataLength());
		_indexHelper.removeIndexData();
	}

	return true;
}

PacedIndexVvc::~PacedIndexVvc()
{

}

void PacedIndexVvc::setLogHint( const char* strHint )
{
	_strLogHint = strHint;
	_pacingLogic.setLogHint(strHint);
}

void PacedIndexVvcFactory::setConfig( const char* szName, const char* szValue )
{

}

void PacedIndexVvcFactory::setLog( ZQ::common::Log* pLog )
{
	if (pLog)
		_log = pLog;
}

PacedIndex* PacedIndexVvcFactory::create( const char* type )
{
	if (strncmp(type, "vvc", 3))
		return NULL;

	PacedIndexVvc* pPacing = new PacedIndexVvc();
	pPacing->setLog(_log);
	return pPacing;
}

PacedIndexVvcFactory::PacedIndexVvcFactory()
{
	//ZQ::common::setGlogger();
}

PacedIndexVvcFactory::~PacedIndexVvcFactory()
{

}

/*
class ParseStateMachine
{
public:
	void setIndexWriter( PacedIndexWrite* pWriter )
	{
		_pIndexWriter = pWriter;
	}

	void movedata(char* buf, int& len, int parse)
	{
		if (len > parse)
		{
			for(int i=parse;i<len;i++)
			{
				buf[i-parse] = buf[i];
			}

			len -= parse;
		}
		else
		{
			len = 0;
		}
	}

	// generate an aligned pointer to a TLV. skip any
	// leading zeros. advance index past the TLV header.
	CTF_TLV *alignTLV(char *pBuf, int *pIndex )
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

	PacedIndexWrite* _pIndexWriter; 
	int				 totalBytes;
};

class ParseState1 : public ParseStateMachine
{
public:

	bool parse(char* buf, int& len)
	{
		int request = (int)sizeof(VVC_INDEX_FILE_HEADER);
		if (len < request)
		{
			// not enough data, return
			return false;
		}

		// write the index file header
		_pIndexWriter->write(buf, request);

		movedata(buf, len, request);
		return true;
	}
};


class PacingLogic
{
public:
	///@return, if ext is not correct, return false, else return true
	bool subFileWritten(const char* ext, int size, int64 offset);

	///@return, if the data is releasable, return true
	bool indexReleasable(VVC_INDEX_RECORD_HEADER* ppRecHeader, char* ppRecord);
	

protected:
	enum
	{
		MAX_SUBFILE_COUNT = (1 + 2 * 12);
	};

	///@return <0 if error, if success, value <MAX_SUBFILE_COUNT
	int extToSubfileId(const char* ext);

	int64	_startingOffset[MAX_SUBFILE_COUNT];
	int64	_endingOffset[MAX_SUBFILE_COUNT];
};

class VvcIndexHelper
{
public:
	struct VVCIndexHeader
	{
		int a;
	};

	enum ParseState
	{
		PS_FILEHEADER,
		PS_INDEXHEADER,
		PS_INDEXREC
	};

	void appendIndexData(const char* buf, int size);	
	bool removeIndexData(int nParsedBytes);
	const char* getDataPointer();


	//
	// return value, 0 for data not enough, <0 for error, >0 is the parsed bytes
	//

	int parseFileHeader(VVC_INDEX_FILE_HEADER** ppFileHeader);
	int parseHeaderBlock(VVCIndexHeader& hd);
	int parseIndexRecord(VVC_INDEX_RECORD_HEADER** ppRecHeader, char** ppRecord);

	ParseState getParseState();


	
protected:

};
*/

/*
void test1(const char* buf, int size, int offset)
{
	VvcIndexHelper vvcHelper;
	PacedIndexWrite* _pIndexWriter; 
	VvcIndexHelper::VVCIndexHeader	_vvcIndexHeader;
	PacingLogic	_pLogic;

	bool no_seek = true;
	if (!no_seek)
	{

		//seek
		//write
		return;
	}

	vvcHelper.appendIndexData(buf, size);

	int nDataSize = 0;

	while(1)
	{
		VvcIndexHelper::ParseState state = vvcHelper.getParseState();
		if(state == VvcIndexHelper::PS_INDEXREC)	// put it here, because it's the small path
		{
			VVC_INDEX_RECORD_HEADER* pRecHdr = NULL;
			char* pRecData = NULL;
			int nParsedBytes = vvcHelper.parseIndexRecord(&pRecHdr, &pRecData);
			if (nParsedBytes < 0)
			{
				//err
				return;
			}
			else if (!nParsedBytes)
			{
				//no enough data
				break;
			}

			// read the data, and check if the data if releasable or not
			if (!_pLogic.indexReleasable(pRecHdr, pRecData))
				break;

			nDataSize += nParsedBytes;
		}
		else if(state == VvcIndexHelper::PS_FILEHEADER)
		{
			VVC_INDEX_FILE_HEADER* pHeader = NULL;
			int nParsedBytes = VvcIndexHelper.parseFileHeader(&pHeader);
			if (nParsedBytes < 0)
			{
				//err
				return;
			}
			else if (!nParsedBytes)
			{
				//no enough data
				break;
			}

			// save some data?
			nDataSize += nParsedBytes;
		}
		else
		{
			int nParsedBytes = vvcHelper.parseHeaderBlock(_vvcIndexHeader);
			if (nParsedBytes < 0)
			{
				//err
				return;
			}
			else if (!nParsedBytes)
			{
				//no enough data
				break;
			}

			nDataSize += nParsedBytes;
		}
	}

	if (nDataSize)
	{
		_pIndexWriter->write(vvcHelper.getDataPointer(), nDataSize);
		vvcHelper.removeIndexData(nDataSize);
	}
}*/

/*
class ParseState2 : public ParseStateMachine
{
public:


	
	

	bool parse(char* buf, int& len)
	{
		int request;

		// read the index header block tag and length field
		// NOTE: index header TLV is always aligned,
		// but is always followed by 2 alignment bytes
		int hdrBytes = (int)sizeof(CTF_TLV) + 2;
		if (len < hdrBytes)
			return false;

		totalBytes += hdrBytes;

		int hdrIndex = hdrBytes;

		// make sure the tag is correct
		CTF_TLV* pHdrTLV = (CTF_TLV *)buf;
		if( pHdrTLV->tag != VVC_TAG_INDEX_HEADER_BLOCK )
		{
			PRINTF( "Incorrect index header block tag (read 0x%x, expected 0x%x) during normalization\n", pHdrTLV->tag, VVC_TAG_INDEX_HEADER_BLOCK );
			return false;
		}

		// read the rest of the index header block, plus any trailing alignment zeros
		request = (int)pHdrTLV->len;
		int hdrLimit = hdrIndex + request;
		int adjust = ( 4 - ( totalBytes + request ) ) & 0x03;
		request += adjust;
		if (len - hdrBytes < request)
		{
			return false;
		}

		totalBytes += request;
		hdrBytes += request;
		// scan the index header TLVs
		int nFileCount = 0;
		while( hdrIndex < hdrLimit )
		{
			pHdrTLV = alignTLV( buf, &hdrIndex );
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
					CTF_TLV* pSubTLV = alignTLV( buf, &subIndex );
					char* pTmp = buf + subIndex;
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

		// write the modified index header 
		
		_pIndexWriter->write(buf, hdrBytes + request);
		movedata(buf, len, hdrBytes + request);
		return true;
	}
};


class ParseState3 : public ParseStateMachine
{
public:

	bool parse(char* buf, int& len)
	{
		// read the rest of the index file and write it to the temp file
		// NOTE: most of the data records of a VVC index file do not require modification
		// during normalization. Most of the records pertain to the main file, which is
		// never sparse. Likewise the records that apply to FF files. Of the records that
		// pertain to FR files, only the trick speed I frame records have an absolute packet
		// number. The rest of the trick file records  use packet offsets from the "zero byte"
		// (i.e. the first real packet in the file, NOT from offset zero )
		for( int i=0; ; ++i )
		{
			// read the next index record header
			int request = (int)sizeof(VVC_INDEX_RECORD_HEADER);
			int bytes = CTF_READ( indexFile, (void *)buf, request );
			// end of file?
			if( bytes == 0 )
			{
				// yes - escape
				break;
			}
			// error?
			if( bytes != request )
			{
				PRINTF( "Error attempting to read index record header during normalization\n" );
				result = -1;
				break;
			}
			totalBytes += bytes;
			// discard any leading zeros
			while( buf[ 0 ] == 0 )
			{
				bytes = CTF_WRITE( tempFile, (void *)buf, 1 );
				if( bytes != 1 )
				{
					PRINTF( "Error attempting to write index record header during normalization\n" );
					result = -1;
					break;
				}
				buf[ 0 ] = buf[ 1 ];
				bytes = CTF_READ( indexFile, (void *)(buf+1), 1 );
				// end of file?
				if( bytes == 0 )
				{
					// yes - escape
					break;
				}
				// error?
				if( bytes != 1 )
				{
					PRINTF( "Error attempting to read index record header during normalization\n" );
					result = -1;
					break;
				}
				totalBytes += bytes;
			}
			if( ( bytes == 0 ) || ( result < 0 ) )
			{
				break;
			}
			VVC_INDEX_RECORD_HEADER* pRec = (VVC_INDEX_RECORD_HEADER *)buf;
			// read the body of the TLV, plus any trailing alignment zeroes
			pTmp = buf + 2;
			request = (int)pRec->recLength;
			recBytes = request + 2;
			if( request > 0 )
			{
				bytes = CTF_READ( indexFile, (void *)pTmp, request );
				// error or end of file?
				if( bytes != request )
				{
					PRINTF( "Error attempting to read index record body during normalization\n" );
					result = -1;
					break;
				}
				totalBytes += bytes;
			}
			// make sure the TLV tag is legit
			switch( pRec->recType )
			{
			case INDEX_REC_TYPE_PCR:
			case INDEX_REC_TYPE_NORM_RATE_IFRAME:
			case INDEX_REC_TYPE_TRICK_RATE_IFRAME:
			case INDEX_REC_TYPE_PFRAME:
			case INDEX_REC_TYPE_BFRAME:
			case INDEX_REC_TYPE_AUDIO:
			case INDEX_REC_TYPE_SCTE35:
			case INDEX_REC_TYPE_GAP:
			case INDEX_REC_TYPE_ENDOFGOP:
			case INDEX_REC_TYPE_ENDOFSEQ:
			case INDEX_REC_TYPE_PRIVATE:
			case INDEX_REC_TYPE_TERMINATOR:
			case INDEX_REC_TYPE_EXTENDED:
				break;
			case INDEX_REC_TYPE_INVALID:
			default:
				PRINTF( "Invalid record tag 0x%x encountered during normalization\n", (int)pRec->recType );
				result = -1;
				break;
			}
			if( result < 0 )
			{
				break;
			}
			// is this a trick file I-frame?
			if( pRec->recType == INDEX_REC_TYPE_TRICK_RATE_IFRAME )
			{
				// yes - is this a fast reverse file?
				VVC_INDEX_RECORD_TRICK_RATE_IFRAME *pTIPIC = (VVC_INDEX_RECORD_TRICK_RATE_IFRAME *)pTmp;
				if( dir[ pTIPIC->subFileIndex ] == VVC_PERCEIVED_DIRECTION_REVERSE )
				{
					// yes - normalize the packet number
					uint32_t pktBytes = ( generateTTS == FALSE ) ? TRANSPORT_PACKET_BYTES : TTS_PACKET_BYTES;
					uint32_t firstPkt = (uint32_t)( pFirstOffsets[ pTIPIC->subFileIndex ] / pktBytes );
					pTIPIC->packetOffset = pTIPIC->packetOffset - firstPkt;
				}
			}
			// write the data that was just read to the temp file
			request = recBytes;
			bytes = CTF_WRITE( tempFile, (void *)buf, request );
			if( bytes != request )
			{
				PRINTF( "Error attempting to write index records during normalization\n" );
				result = -1;
				break;
			}
			totalBytes += bytes;
			// check the TLV tag - did we just write the terminator record?
			if( pRec->recType == INDEX_REC_TYPE_TERMINATOR )
			{
				// yes - escape
				break;
			}
		} // for( i=0; ; ++i )
		// pad the index with zeros to the next 4-byte boundary
		buf[ 0 ] = buf[ 1 ] = buf[ 2 ] = 0;
		request = ( 4 - totalBytes ) & 0x03;
		bytes = CTF_WRITE( tempFile, (void *)buf, request );
		if( bytes != request )
		{
			PRINTF( "Error attempting to write final padding zeros during normalization\n" );
			result = -1;
			break;
		}

	} while( 0 );			// end error escape wrapper
	}
protected:
private:
};


void PacedIndexVvc::writeIndex( const char* buf, int size, int offset )
{
	if (_nDataOffset != offset )
	{
		_pIndexWriter->seek(offset);
		_pIndexWriter->write(buf, size);	

		return;
	}

	memcpy(_buf + _nBufLen, buf, size);		
	_nBufLen += size;

	_nDataOffset += size;


	//read buffer
	CTF_TLV *pHdrTLV;
	CTF_TLV *pSubTLV;
	VVC_INDEX_RECORD_HEADER *pRec;
	VVC_INDEX_RECORD_TRICK_RATE_IFRAME *pTIPIC;
	unsigned char *pTmp;
	INT64 firstOffset;
	INT64 finalOffset;
	int result = 0;
	int fileNumber;
	int hdrIndex;
	int hdrLimit;
	int subIndex;
	int subLen;
	int subLimit;
	int direction;
	int request;
	int totalBytes;
	int hdrBytes;
	int recBytes;
	int adjust;
	int bytes;
	int i;
	int dir[ 8 ];
	char unsigned buf[ 0x10000 ];

	int nLeftBytes = _nBufLen;

#define BUFFER_START	(_buf + _nBufLen - nLeftBytes)


	do				// begin error escape wrapper
	{

		do
		{
			// if not this state, skip
			if (_nParseState != 0)
				break;

				// read the index file header
			totalBytes = 0;
			request = (int)sizeof(VVC_INDEX_FILE_HEADER);

			if (_nBufLen < request)
			{
				// not enough data, return
				return;
			}

			memcpy( buf, BUFFER_START, request );
			totalBytes += bytes;
			// write the index file header
			_pIndexWriter->write(buf, request);

			_nParseState++;
			nLeftBytes -= request;

			//move
		}while(0);

		if (nLeftBytes <= 0)
			return;

		do
		{
			if (_nParseState != 1)
				break;

			// read the index header block tag and length field
			// NOTE: index header TLV is always aligned,
			// but is always followed by 2 alignment bytes
			hdrBytes = (int)sizeof(CTF_TLV) + 2;
			if (nLeftBytes < hdrBytes)
				return;

			memcpy(buf, BUFFER_START, hdrBytes);
			totalBytes += hdrBytes;
	
			hdrIndex = hdrBytes;

			// make sure the tag is correct
			pHdrTLV = (CTF_TLV *)buf;
			if( pHdrTLV->tag != VVC_TAG_INDEX_HEADER_BLOCK )
			{
				PRINTF( "Incorrect index header block tag (read 0x%x, expected 0x%x) during normalization\n", pHdrTLV->tag, VVC_TAG_INDEX_HEADER_BLOCK );
				return;
			}

			// read the rest of the index header block, plus any trailing alignment zeros
			request = (int)pHdrTLV->len;
			hdrLimit = hdrIndex + request;
			adjust = ( 4 - ( totalBytes + request ) ) & 0x03;
			request += adjust;
			if (nLeftBytes - hdrBytes < request)
			{

				return;
			}

		bytes = CTF_READ( indexFile, (void *)( buf + hdrIndex ), request );
		if( bytes != request )
		{
			PRINTF( "Error attempting to read index header block during normalization\n" );
			result = -1;
			break;
		}
		totalBytes += bytes;
		hdrBytes += bytes;
		// scan the index header TLVs
		*pFileCount = 0;
		while( hdrIndex < hdrLimit )
		{
			pHdrTLV = alignTLV( buf, &hdrIndex );
			// is this a subfile info subblock?
			if( pHdrTLV->tag == VVC_TAG_SUB_FILE_INFO_BLOCK )
			{
				// yes - set up to scan the subfile info block
				subLen = (int)pHdrTLV->len;
				subIndex = hdrIndex;
				subLimit = subIndex + subLen;
				// make sure that the end of the block is in the header
				if( subLimit > hdrLimit )
				{
					PRINTF( "Subfile info block not contained within index header limits during normalization\n" );
					result = -1;
					break;
				}
				// scan the subfile info TLVs. Count the number of FILE entries.
				// record the initial starting offset of each file and the direction
				// NOTE: some speed files may have been stubbed out
				fileNumber  = -1;
				firstOffset = -1;
				finalOffset = -1;
				direction   = -1;
				while( subIndex < subLimit )
				{ 
					pSubTLV = alignTLV( buf, &subIndex );
					pTmp = buf + subIndex;
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
					result = -1;
					break;
				}
				// record the start and end byte offsets
				pFirstOffsets[ fileNumber ] = firstOffset;
				pFinalOffsets[ fileNumber ] = finalOffset;
				// record the direction
				dir[ fileNumber ] = direction;
				// increment the file count
				++( *pFileCount );
			} // if( pHdrTLV->tag == VVC_TAG_SUB_FILE_INFO_BLOCK )
			// advance to the next TLV
			hdrIndex += pHdrTLV->len;
		}; // while( hdrIndex < hdrLimit )
		if( result < 0 )
		{
			break;
		}
		// write the modified index header to the temp file
		bytes = CTF_WRITE( tempFile, (void *)buf, hdrBytes );
		if( bytes != hdrBytes )
		{
			PRINTF( "Error attempting to write index header block during normalization\n" );
			result = -1;
			break;
		}




		// read the rest of the index file and write it to the temp file
		// NOTE: most of the data records of a VVC index file do not require modification
		// during normalization. Most of the records pertain to the main file, which is
		// never sparse. Likewise the records that apply to FF files. Of the records that
		// pertain to FR files, only the trick speed I frame records have an absolute packet
		// number. The rest of the trick file records  use packet offsets from the "zero byte"
		// (i.e. the first real packet in the file, NOT from offset zero )
		for( i=0; ; ++i )
		{
			// read the next index record header
			request = (int)sizeof(VVC_INDEX_RECORD_HEADER);
			bytes = CTF_READ( indexFile, (void *)buf, request );
			// end of file?
			if( bytes == 0 )
			{
				// yes - escape
				break;
			}
			// error?
			if( bytes != request )
			{
				PRINTF( "Error attempting to read index record header during normalization\n" );
				result = -1;
				break;
			}
			totalBytes += bytes;
			// discard any leading zeros
			while( buf[ 0 ] == 0 )
			{
				bytes = CTF_WRITE( tempFile, (void *)buf, 1 );
				if( bytes != 1 )
				{
					PRINTF( "Error attempting to write index record header during normalization\n" );
					result = -1;
					break;
				}
				buf[ 0 ] = buf[ 1 ];
				bytes = CTF_READ( indexFile, (void *)(buf+1), 1 );
				// end of file?
				if( bytes == 0 )
				{
					// yes - escape
					break;
				}
				// error?
				if( bytes != 1 )
				{
					PRINTF( "Error attempting to read index record header during normalization\n" );
					result = -1;
					break;
				}
				totalBytes += bytes;
			}
			if( ( bytes == 0 ) || ( result < 0 ) )
			{
				break;
			}
			pRec = (VVC_INDEX_RECORD_HEADER *)buf;
			// read the body of the TLV, plus any trailing alignment zeroes
			pTmp = buf + 2;
			request = (int)pRec->recLength;
			recBytes = request + 2;
			if( request > 0 )
			{
				bytes = CTF_READ( indexFile, (void *)pTmp, request );
				// error or end of file?
				if( bytes != request )
				{
					PRINTF( "Error attempting to read index record body during normalization\n" );
					result = -1;
					break;
				}
				totalBytes += bytes;
			}
			// make sure the TLV tag is legit
			switch( pRec->recType )
			{
			case INDEX_REC_TYPE_PCR:
			case INDEX_REC_TYPE_NORM_RATE_IFRAME:
			case INDEX_REC_TYPE_TRICK_RATE_IFRAME:
			case INDEX_REC_TYPE_PFRAME:
			case INDEX_REC_TYPE_BFRAME:
			case INDEX_REC_TYPE_AUDIO:
			case INDEX_REC_TYPE_SCTE35:
			case INDEX_REC_TYPE_GAP:
			case INDEX_REC_TYPE_ENDOFGOP:
			case INDEX_REC_TYPE_ENDOFSEQ:
			case INDEX_REC_TYPE_PRIVATE:
			case INDEX_REC_TYPE_TERMINATOR:
			case INDEX_REC_TYPE_EXTENDED:
				break;
			case INDEX_REC_TYPE_INVALID:
			default:
				PRINTF( "Invalid record tag 0x%x encountered during normalization\n", (int)pRec->recType );
				result = -1;
				break;
			}
			if( result < 0 )
			{
				break;
			}
			// is this a trick file I-frame?
			if( pRec->recType == INDEX_REC_TYPE_TRICK_RATE_IFRAME )
			{
				// yes - is this a fast reverse file?
				pTIPIC = (VVC_INDEX_RECORD_TRICK_RATE_IFRAME *)pTmp;
				if( dir[ pTIPIC->subFileIndex ] == VVC_PERCEIVED_DIRECTION_REVERSE )
				{
					// yes - normalize the packet number
					uint32_t pktBytes = ( generateTTS == FALSE ) ? TRANSPORT_PACKET_BYTES : TTS_PACKET_BYTES;
					uint32_t firstPkt = (uint32_t)( pFirstOffsets[ pTIPIC->subFileIndex ] / pktBytes );
					pTIPIC->packetOffset = pTIPIC->packetOffset - firstPkt;
				}
			}
			// write the data that was just read to the temp file
			request = recBytes;
			bytes = CTF_WRITE( tempFile, (void *)buf, request );
			if( bytes != request )
			{
				PRINTF( "Error attempting to write index records during normalization\n" );
				result = -1;
				break;
			}
			totalBytes += bytes;
			// check the TLV tag - did we just write the terminator record?
			if( pRec->recType == INDEX_REC_TYPE_TERMINATOR )
			{
				// yes - escape
				break;
			}
		} // for( i=0; ; ++i )
		// pad the index with zeros to the next 4-byte boundary
		buf[ 0 ] = buf[ 1 ] = buf[ 2 ] = 0;
		request = ( 4 - totalBytes ) & 0x03;
		bytes = CTF_WRITE( tempFile, (void *)buf, request );
		if( bytes != request )
		{
			PRINTF( "Error attempting to write final padding zeros during normalization\n" );
			result = -1;
			break;
		}

	} while( 0 );			// end error escape wrapper

	return result;


*/





















































