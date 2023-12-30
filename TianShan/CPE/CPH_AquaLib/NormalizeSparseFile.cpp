#include "NormalizeSparseFile.h"
#include "SystemUtils.h"
#ifdef ZQ_OS_MSWIN
#include "stdafx.h"
#include <io.h>
#include <share.h>
#include <direct.h>
#include <winioctl.h>
#include <sys/stat.h>
#define CTF_OPEN_SRC_PERMS					( _S_IREAD )
#define CTF_OPEN_SRC_SHFLAGS				( _SH_DENYWR )
#define CTF_OPEN_SRC_FLAGS					( _O_RDONLY | _O_BINARY | _O_SEQUENTIAL )
#define CTF_OPEN_DST_FLAGS					( _O_WRONLY | _O_BINARY | _O_SEQUENTIAL | _O_CREAT | _O_TRUNC )
#define CTF_OPEN_DST_PERMS					( _S_IWRITE |_S_IREAD)
#define CTF_SRC_OPEN(p)						_sopen(p,CTF_OPEN_SRC_FLAGS,CTF_OPEN_SRC_SHFLAGS,CTF_OPEN_SRC_PERMS)
#define CTF_DST_OPEN(p)						_open(p,CTF_OPEN_DST_FLAGS,CTF_OPEN_DST_PERMS)
#define CTF_FDOPEN							_fdopen
#define CTF_CLOSE							_close
#define CTF_READ							_read
#define CTF_WRITE							_write
#define CTF_LSEEK							_lseek
#define CTF_LSEEK64							_lseeki64
#define CTF_STRCPY(dst,dstlen,src)			strcpy_s((dst),(dstlen),(src))
#define CTF_STRICMP							_stricmp
#define CTF_SSCANF							sscanf_s
#define CTF_GETCWD							_getcwd
#define CTF_DELETE_CMD						"del"
#define CTF_COPY_CMD						"copy"
#define CTF_FMTI64							"I64"
#else

#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/stat.h>
#ifdef __KERNEL
#define S_IWRITE							S_IWUSR
#endif
#define CTF_OPEN_SRC_PERMS					( S_IREAD )
#define CTF_OPEN_SRC_FLAGS					( O_RDONLY | O_BINARY | O_SEQUENTIAL )
#define CTF_OPEN_DST_FLAGS					( O_WRONLY | O_BINARY | O_SEQUENTIAL | O_CREAT | O_TRUNC )
#define CTF_OPEN_DST_PERMS					( S_IWRITE | S_IREAD |S_IRGRP |S_IWGRP |S_IROTH)
#define CTF_SRC_OPEN(p)						open(p,CTF_OPEN_SRC_FLAGS,CTF_OPEN_SRC_PERMS)
#define CTF_DST_OPEN(p)						open(p,CTF_OPEN_DST_FLAGS,CTF_OPEN_DST_PERMS)
#define CTF_FDOPEN							fdopen
#define CTF_CLOSE							close
#define CTF_READ							read
#define CTF_WRITE							write
#define CTF_LSEEK							lseek
#define CTF_LSEEK64							lseek64
#define CTF_STRCPY(dst,dstlen,src)			strcpy((dst),(src))
#define CTF_STRICMP							strcasecmp
#define CTF_SSCANF							sscanf
#define CTF_GETCWD							getcwd
#define CTF_DELETE_CMD						"rm"
#define CTF_COPY_CMD						"cp"
#define CTF_FMTI64							"l"
#endif //

#include "CTFLib.h"
#include "vvc.h"

#define NEW_VVC_DEFINE	(VVC_INDEX_MAJOR_VERSION>=1 && VVC_INDEX_MINOR_VERSION>=1)

// define some useful macros

// extract a word from an array of unsigned chars
#define WRD(p)			( ((unsigned short)(*((p)+1))<<8) | (*(p)) )

// extract a long from an array of unsigned chars
#define DWD(p)			( ((((unsigned long)WRD((p)+2)))<<16) | (WRD((p))) )

// extract a long long from an array of unsigned char
#define BY5(p)			( ((((unsigned long long)*((p)+4)))<<32) | (DWD((p))) )

// locally defined structures

#define TEMPEXT "~"
#pragma pack(push, ctf_types, 1)
typedef struct _CTF_TLV
{
	VVC_TAG tag;
	UINT16 len;
} CTF_TLV;

// some configuration constants

#define MAX_OUTFILE_COUNT				8
#define MLOG if(_log)(*_log)

namespace ZQTianShan {
	namespace ContentProvision {

CTF_TLV *alignTLV( unsigned char *pBuf, int *pIndex )
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
void encode(int64 nEncode, int len,  unsigned char * pChar)
{
	int64 * pTemp = &nEncode; 
	for(int i = 0 ; i < len; i++)
	{
		*(pChar + i) =  *((unsigned char *)pTemp + i);
	}
}
NormalizeSparseFile::NormalizeSparseFile(ZQ::common::Log* log, int outputFileCount, FilesLists& outputFileNames, int indexType, bool bGenerateTTS):
	_log(log), _outputFileCount(outputFileCount), _outputFileNames(outputFileNames), _indexType(indexType), _bGenerateTTS(bGenerateTTS)
{
	_indexFilepath = outputFileNames[0];

// #ifdef ZQ_OS_MSWIN
// 	int npos = _indexFilepath.rfind("\\");
// 	if(npos >= 0)
// 		_cacheDir = _indexFilepath.substr(0, npos);
// #else
// 	int npos = _indexFilepath.rfind("/");
// 	if(npos >= 0)
// 		_cacheDir = _indexFilepath.substr(0, npos);
// #endif
	
}

NormalizeSparseFile::~NormalizeSparseFile(void)
{
}
// normalize a VVX index that contains sparse files
// return the original first and final offsets of all output files
int NormalizeSparseFile::normalizeSparseIndexFileVVX( int indexFile, int tempFile, int *pFileCount,
													 int64 *pFirstOffsets, int64 *pFinalOffsets )
{
	int result = 0;

	do				// begin error escape wrapper
	{

	} while( 0 );			// end error escape wrapper

	return result;
}

// normalize a VV2 index that contains sparse files
// return the original first and final offsets of all output files
int NormalizeSparseFile::normalizeSparseIndexFileVV2( int indexFile, int tempFile, int *pFileCount,
									   int64 *pFirstOffsets, int64 *pFinalOffsets )
{
	FILE *pIndexFile;
	FILE *pTempFile;
	int64 firstOffset;
	int64 finalOffset;
	int result = 0;
	int fileNumber;
	int seqNumber;
	int pktCount;
	int nts;
	char cTemp;
	char dirChar[ MAX_OUTFILE_COUNT ];
	char buf[ 256 ];

	// reset some variables
	memset( (void *)dirChar, 0, sizeof(dirChar) );
	// open a file descriptor for the index file
	pIndexFile = CTF_FDOPEN( indexFile, "r" );
	if( pIndexFile == (FILE *)NULL )
	{
		MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Failed to open file descriptor for index file during normalization" ));
		return -1;
	}
	// open a file descriptor for the temp file
	pTempFile = CTF_FDOPEN( tempFile, "w" );
	if( pTempFile == (FILE *)NULL )
	{
		MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Failed to open file descriptor for temp file during normalization" ));
		return -1;
	}

	do				// begin error escape wrapper
	{

		// scan forward. count the number of FILE lines before
		// the first IN_POINT_COUNT_HEX line is encountered
		// record the initial starting offset of the file and the direction
		// NOTE: some speed files may have been stubbed out
		*pFileCount = 0;
		while( fgets( buf, (int)sizeof(buf), pIndexFile ) != (char *)NULL )
		{
			// is this a FILE line?
			if( strncmp( buf, "FILE:#.F", strlen("FILE:#.F") ) == 0 )
			{
				// yes - get the file number and direction character
				if( sscanf( buf, "FILE:#.F%c:%x", &cTemp, &fileNumber ) != 2 )
				{
					MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Unable to parse FILE line (%s) in index header during normalization"), buf );
					result = -1;
					break;
				}
				if( fileNumber >= MAX_OUTFILE_COUNT )
				{
					MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Invalid file number (%x) in index header during normalization"), fileNumber );
					result = -1;
					break;
				}
				// record the direction of the file
				dirChar[ fileNumber ] = cTemp;
			}
			// is this a file allocation record?
			if( strncmp( buf, "FILE_ALLOCATION_HEX", strlen("FILE_ALLOCATION_HEX") ) == 0 )
			{
				// yes - bump the file count and record the first and last offsets of the file
				++(*pFileCount);
				if( sscanf( buf, "FILE_ALLOCATION_HEX:%x:%" CTF_FMTI64 "x:%" CTF_FMTI64 "x",
					&fileNumber, &firstOffset, &finalOffset ) != 3 )
				{
					MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Failed to parse FILE_ALLOCATION line in index during normalization" ));
					result = -1;
					break;
				}
				// record the first and last offsets
				pFirstOffsets[ fileNumber ] = firstOffset;
				pFinalOffsets[ fileNumber ] = finalOffset;
				// is this is a reverse trick file?
				if( dirChar[ fileNumber ] == 'R' )
				{
					// yes - adjust the offsets (value and text)
					finalOffset -= firstOffset;
					firstOffset = 0;
					sprintf( buf, "FILE_ALLOCATION_HEX:%x:%016" CTF_FMTI64 "x:%016" CTF_FMTI64 "x\n",
						fileNumber, firstOffset, finalOffset );
				}
			}
			// write this line to the temp file
			fputs( buf, pTempFile );
			// escape when the start-of-data line has been encountered
			if( strncmp( buf, "SOD", strlen("SOD") ) == 0 )
			{
				break;
			}
		};
		// make sure we found some files
		if( *pFileCount == 0 )
		{
			MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Premature end of index file during normalization" ));
			result = -1;
			break;
		}
		// process the data section
		while( fgets( buf, (int)sizeof(buf), pIndexFile ) != (char *)NULL )
		{
			// if this is the "EOD" line, don't attempt to treat it like a file record
			if( strncmp( buf, "EOD", strlen("EOD") ) != 0 )
			{
				// this is a file record - parse the fixed fields
				if( sscanf( buf, "%x:%x:%x:%" CTF_FMTI64 "x:%x",
					&seqNumber, &nts, &fileNumber, &firstOffset, &pktCount ) != 5 )
				{
					MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Invalid index record (%s) encountered during normalization"), buf );
					result = -1;
					break;
				}
				if( fileNumber >= MAX_OUTFILE_COUNT )
				{
					MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Invalid file number in index record (%s) during normalization"), buf );
					result = -1;
					break;
				}
				// is this a reverse trick file?
				if( dirChar[ fileNumber ] == 'R' )
				{
					// yes - adjust the offset (value and text)
					firstOffset -= pFirstOffsets[ fileNumber ];
					sprintf( buf, "%04x:%08x:%x:%09" CTF_FMTI64 "x:%08x\n",
						seqNumber, nts, fileNumber, firstOffset, pktCount );
				}
			}
			// write this line to the temp file
			fputs( buf, pTempFile );
		}

	} while( 0 );			// end error escape wrapper

	// close the index file descriptor opened above
	fclose( pIndexFile );
	// close the temp file descriptor opened above
	fclose( pTempFile );

	return result;
}
// normalize a VVC index that contains sparse files
// return the original first and final offsets of all output files
int NormalizeSparseFile::normalizeSparseIndexFileVVC( int indexFile, int tempFile, int *pFileCount,
									   int64 *pFirstOffsets, int64 *pFinalOffsets )
{
	CTF_TLV *pHdrTLV;
	CTF_TLV *pSubTLV;
	VVC_INDEX_RECORD_HEADER *pRec;
#if NEW_VVC_DEFINE
		VVC_INDEX_RECORD_MP2_TRICK_IFRAME *pTIPIC;
#else
        VVC_INDEX_RECORD_TRICK_RATE_IFRAME *pTIPIC;
#endif
	unsigned char *pTmp;
	int64 firstOffset;
	int64 finalOffset;
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
	int dir[ MAX_OUTFILE_COUNT ];
	char unsigned buf[ 0x10000 ];

	do				// begin error escape wrapper
	{

		// read the index file header
		totalBytes = 0;
		request = (int)sizeof(VVC_INDEX_FILE_HEADER);
		bytes = CTF_READ( indexFile, (void *)buf, request );
		if( bytes != request )
		{
			MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Error attempting to read index file header block during normalization") );
			result = -1;
			break;
		}
		totalBytes += bytes;
		// read the index header block tag and length field
		// NOTE: index header TLV is always aligned,
		// but is always followed by 2 alignment bytes
		hdrBytes = (int)sizeof(CTF_TLV) + 2;
		bytes = CTF_READ( indexFile, buf, hdrBytes );
		if( bytes != hdrBytes )
		{
			MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Error attempting to read index header block tag and length during normalization" ));
			result = -1;
			break;
		}
		totalBytes += bytes;
		hdrIndex = bytes;
		// make sure the tag is correct
		pHdrTLV = (CTF_TLV *)buf;
		if( pHdrTLV->tag != VVC_TAG_INDEX_HEADER_BLOCK )
		{
			MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Incorrect index header block tag (read 0x%x, expected 0x%x) during normalization"), pHdrTLV->tag, VVC_TAG_INDEX_HEADER_BLOCK );
			result = -1;
			break;
		}
		// read the rest of the index header block, plus any trailing alignment zeros
		request = (int)pHdrTLV->len;
		hdrLimit = hdrIndex + request;
		adjust = ( 4 - ( totalBytes + request ) ) & 0x03;
		request += adjust;
		bytes = CTF_READ( indexFile, (void *)( buf + hdrIndex ), request );
		if( bytes != request )
		{
			MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Error attempting to read index header block during normalization" ));
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
					MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Subfile info block not contained within index header limits during normalization" ));
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
				unsigned char *pFirst, *pFinal;
				int nLenFirst, nLenFinal;
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
						pFirst = pTmp;
						nLenFirst = pSubTLV->len;
						break;
					case VVC_TAG_SUB_FILE_ENDING_BYTE:
						finalOffset = BY5( pTmp );
						pFinal = pTmp;
						nLenFinal = pSubTLV->len;
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
					MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Subfile info block did not contain file number, start byte, end byte, or direction during normalization" ));
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
				MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Error attempting to read index record header during normalization" ));
				result = -1;
				break;
			}
			totalBytes += bytes;
			// discard any leading zeros
			while( buf[ 0 ] == 0 )
			{
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
					MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Error attempting to read index record header during normalization" ));
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
					MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Error attempting to read index record body during normalization") );
					result = -1;
					break;
				}
				totalBytes += bytes;
			}
			// make sure the TLV tag is legit
			switch( pRec->recType )
			{
			case INDEX_REC_TYPE_PCR:
#if NEW_VVC_DEFINE
			case INDEX_REC_TYPE_MP2_NORMAL_IFRAME:
#else
			case INDEX_REC_TYPE_NORM_RATE_IFRAME:
#endif
#if NEW_VVC_DEFINE
			case INDEX_REC_TYPE_MP2_TRICK_IFRAME:
#else
			case INDEX_REC_TYPE_TRICK_RATE_IFRAME:
#endif
#if NEW_VVC_DEFINE
			case INDEX_REC_TYPE_MP2_PFRAME:
#else
			case INDEX_REC_TYPE_PFRAME:
#endif

#if NEW_VVC_DEFINE
			case INDEX_REC_TYPE_MP2_BFRAME:
#else
			case INDEX_REC_TYPE_BFRAME:
#endif
			case INDEX_REC_TYPE_AUDIO:
			case INDEX_REC_TYPE_SCTE35:
			case INDEX_REC_TYPE_GAP:
			case INDEX_REC_TYPE_ENDOFGOP:
#if NEW_VVC_DEFINE
			case INDEX_REC_TYPE_MP2_ENDOFSEQ:
#else
			case INDEX_REC_TYPE_ENDOFSEQ:
#endif
			case INDEX_REC_TYPE_PRIVATE:
			case INDEX_REC_TYPE_TERMINATOR:
			case INDEX_REC_TYPE_EXTENDED:
				break;
			case INDEX_REC_TYPE_INVALID:
			default:
				MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Invalid record tag 0x%x encountered during normalization"), (int)pRec->recType );
				result = -1;
				break;
			}
			if( result < 0 )
			{
				break;
			}
			// is this a trick file I-frame?
#if NEW_VVC_DEFINE
			if(pRec->recType == INDEX_REC_TYPE_MP2_TRICK_IFRAME)
#else
			if( pRec->recType == INDEX_REC_TYPE_TRICK_RATE_IFRAME )
#endif

			{
				// yes - is this a fast reverse file?
#if NEW_VVC_DEFINE
				pTIPIC = (VVC_INDEX_RECORD_MP2_TRICK_IFRAME*)pTmp;
#else
				pTIPIC = (VVC_INDEX_RECORD_TRICK_RATE_IFRAME *)pTmp;
#endif
				if( dir[ pTIPIC->subFileIndex ] == VVC_PERCEIVED_DIRECTION_REVERSE )
				{
					// yes - normalize the packet number
					unsigned long pktBytes = ( _bGenerateTTS == FALSE ) ? TRANSPORT_PACKET_BYTES : TTS_PACKET_BYTES;
					unsigned long firstPkt = (unsigned long)( pFirstOffsets[ pTIPIC->subFileIndex ] / pktBytes );
					pTIPIC->packetOffset = pTIPIC->packetOffset - firstPkt;
				}
			}
			// write the data that was just read to the temp file
			request = recBytes;
			//totalBytes += bytes;
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
	} while( 0 );			// end error escape wrapper

	return result;
}

// normalize an index that contains sparse files
// return the original first offsets of all output files
int  NormalizeSparseFile::normalizeSparseIndexFile(int *pFileCount,
									int64 *pFirstOffsets, int64 *pFinalOffsets )
{
	int indexFile;
	int tempFile;
	int result = 0;
	std::string tempFileName;
	char buf[ 0x10000 ];

	do				// begin error escape wrapper
	{

		// re-open the index file for reading
		indexFile = CTF_SRC_OPEN( _indexFilepath.c_str() );
		if( indexFile < 0 )
		{
			MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Failed to re-open index file %s for normalization"),
				_indexFilepath.c_str());
			result = -1;
			break;
		}
		// create a unique name for the temporary output file
		tempFileName = _indexFilepath + TEMPEXT;
		_tempIndexFile = tempFileName;

		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(NormalizeSparseFile, "normalize Sparse Index File (%s)"),
			_indexFilepath.c_str());

		// dispatch to the specific index type handler
		switch( _indexType )
		{
		case CTF_INDEX_TYPE_VVX:
// 			result = normalizeSparseIndexFileVVX( indexFile, tempFile, pFileCount,
// 				pFirstOffsets, pFinalOffsets );
			// close the index file
			CTF_CLOSE( indexFile );
			break;
		case CTF_INDEX_TYPE_VV2:
// 			result = normalizeSparseIndexFileVV2( indexFile, tempFile, pFileCount,
// 				pFirstOffsets, pFinalOffsets );
			// note: don't close the index or temp files; already closed in above function
			break;
#ifndef ZQ_OS_MSWIN
		case CTF_INDEX_TYPE_VVC:
			result = normalizeSparseIndexFileVVC( indexFile, tempFile, pFileCount,
				pFirstOffsets, pFinalOffsets );
			// close the index file
			CTF_CLOSE( indexFile );
			break;
#endif
		default:
			MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"unrecognized index type (%d)"),
				 (int)_indexType );
			result = -1;
		}
	} while( 0 );			// end error escape wrapper

	return result;
}

// normalize a sparse speed file
int  NormalizeSparseFile::normalizeSparseSpeedFile( int fileNumber,
									int64 *pFirstOffsets, int64 *pFinalOffsets )
{

	MLOG(ZQ::common::Log::L_INFO,  CLOGFMT(NormalizeSparseFile, "normalize Sparse Speed File (%s) FirstOffsets [%lld] FinalOffsets[%lld] "),
		_outputFileNames[ fileNumber ].c_str(), pFirstOffsets[ fileNumber ], pFinalOffsets[ fileNumber ]);
	int speedFile;
	int tempFile;
	int bytes;
	int result = 0;
	int64 totalBytes;
	std::string tempFileName;
	char buf[ 0x10000 ];

	do				// begin error escape wrapper
	{

		// do nothing if the initial offset of this speed file is zero (i.e. not sparse)
		if( pFirstOffsets[ fileNumber ] == 0 )
		{
			break;
		}
		// re-open the speed file for reading
		speedFile = CTF_SRC_OPEN( _outputFileNames[ fileNumber ].c_str() );
		if( speedFile < 0 )
		{
			MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Failed to re-open speed file %s for normalization"),
				_outputFileNames[ fileNumber ].c_str());
			result = -1;
			break;
		}
		// seek to the lowest offset of the speed file
		if( CTF_LSEEK64( speedFile, pFirstOffsets[ fileNumber ], SEEK_SET ) != pFirstOffsets[ fileNumber ] )
		{
			MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Failed to seek to lowest active offset in speed file %s for normalization"),
				_outputFileNames[ fileNumber ].c_str() );
			result = -1;
			break;
		}
		// create a unique name for the temporary output file   
		tempFileName = _outputFileNames[ fileNumber ] + TEMPEXT;

		MLOG(ZQ::common::Log::L_INFO,  CLOGFMT(NormalizeSparseFile, "normalize Sparse Speed File (%s) tempfile(%s) "),
			_outputFileNames[ fileNumber ].c_str(), tempFileName.c_str());

		// create the temporary output file
		tempFile = CTF_DST_OPEN( tempFileName.c_str());
		if( tempFile < 0 )
		{
			MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Failed to open temp file %s for normalization"), tempFileName.c_str() );
			result = -1;
			break;
		}
		// copy the active data from the speed file to the temp file
		totalBytes = 0;
		while( ( bytes = CTF_READ( speedFile, (void *)buf, (int)sizeof(buf) ) ) > 0 )
		{
			if( CTF_WRITE( tempFile, (void *)buf, bytes ) != bytes )
			{
				MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Failed to write %d bytes to temp file %s during normalization"),
					bytes, tempFileName.c_str() );
				result = -1;
				break;
			}
			totalBytes += bytes;
		}
		// was the whole file written?
		MLOG(ZQ::common::Log::L_INFO,  CLOGFMT(NormalizeSparseFile,"normalize Sparse Speed File %s, read totalbypes[%lld],[%lld]"),
			_outputFileNames[ fileNumber ].c_str(), totalBytes,  (int64)(pFinalOffsets[ fileNumber ] - pFirstOffsets[ fileNumber ]));
		if( totalBytes != 1 + ( pFinalOffsets[ fileNumber ] - pFirstOffsets[ fileNumber ]))
		{
			MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Failed to copy entire speed file %s during normalization"),
				_outputFileNames[ fileNumber ].c_str() );
			result = -1;
			// close both files
			CTF_CLOSE( speedFile );
			CTF_CLOSE( tempFile );
			// delete temp file
			MLOG(ZQ::common::Log::L_INFO, CLOGFMT(NormalizeSparseFile, "delete speed temp file %s"),tempFileName.c_str());
			if(unlink(tempFileName.c_str()))
			{
				MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Failed to delete speed temp file %s during normalization(%d,%s)"),
					tempFileName.c_str() , SYS::getLastErr(), SYS::getErrorMessage().c_str());
			}
			break;
		}
		// close both files
		CTF_CLOSE( speedFile );
		CTF_CLOSE( tempFile );

		// delete the original speed file
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(NormalizeSparseFile, "delete original speed file %s"),_outputFileNames[ fileNumber ].c_str());
		if(unlink(_outputFileNames[ fileNumber ].c_str()))
		{
			MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Failed to delete speed file %s during normalization(%d, %s)"),
				_outputFileNames[ fileNumber ].c_str() , SYS::getLastErr(), SYS::getErrorMessage().c_str());
		}

		// rename the temp speed file to replace the original speed file
		MLOG(ZQ::common::Log::L_INFO,  CLOGFMT(NormalizeSparseFile, "rename temp speed file %s as speed file %s"), 
			tempFileName.c_str(), _outputFileNames[ fileNumber ].c_str());
		bool bRet = rename(tempFileName.c_str(), _outputFileNames[ fileNumber ].c_str());
		if(bRet)
		{
			MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Failed to rename temp speed file %s as speed file %s during normalization(%d, %s)"),
				tempFileName.c_str(), _outputFileNames[ fileNumber ].c_str(), SYS::getLastErr(), SYS::getErrorMessage().c_str());
			result = -1;
			break;
		}

	} while( 0 );			// end error escape wrapper

	if(result == 0)
	{
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(NormalizeSparseFile, "normalize Sparse Speed File (%s) successful"), _outputFileNames[ fileNumber ].c_str());
	}
	else
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NormalizeSparseFile, "Failed to normalize Sparse Speed File (%s)"), _outputFileNames[ fileNumber ].c_str());
	}
	return result;
}

// normalize a sparse fileset
int NormalizeSparseFile::normalizeSparseFileSet( )
{
	int i;
	int fileCount;
	int result = 0;
	int64 firstOffsets[ MAX_OUTFILE_COUNT ];
	int64 finalOffsets[ MAX_OUTFILE_COUNT ];

	do				// begin error escape wrapper
	{

		// first, reset the speed file offset arrays
		memset( (void *)firstOffsets, 0, sizeof(firstOffsets) );
		memset( (void *)finalOffsets, 0, sizeof(finalOffsets) );
		// normalize the index file
		result = normalizeSparseIndexFile(&fileCount, firstOffsets, finalOffsets );
		if( result != 0 )
		{
			break;
		}
// 		for(i = 0; i < fileCount; i++)
// 		{
// 			MLOG(ZQ::common::Log::L_INFO, CLOGFMT(NormalizeSparseFile, "normalizeSparseFileSet [%d] firstOffsets [%lld], finalOffsets[%lld]"),
// 				i , firstOffsets[i],  finalOffsets[i]);
// 		}
		// iterate over the output speed files and normalize them
		for( i = 1; i<_outputFileCount; ++i )
		{
			result = normalizeSparseSpeedFile(i, firstOffsets, finalOffsets );
			if( result != 0 )
			{
				break;
			} 
		}
	} while( 0 );			// end error escape wrapper

	return result;
}

int  NormalizeSparseFile::deleteIndexFile()
{
	int result = 0;
	// delete the original index file
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(NormalizeSparseFile, "delete original index file %s"),_indexFilepath.c_str());
	if(unlink(_indexFilepath.c_str()))
	{
		MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Failed to delete original index file %s during normalization(%d, %s)"),
			_indexFilepath.c_str() , SYS::getLastErr(), SYS::getErrorMessage().c_str());
		result = -1;
		return result;
	}

	// copy the temp index file to replace the original index file
	MLOG(ZQ::common::Log::L_INFO,  CLOGFMT(NormalizeSparseFile, "rename temp index file %s as index file %s"), 
		_tempIndexFile.c_str(), _indexFilepath.c_str());

	bool bRet = rename(_tempIndexFile.c_str(), _indexFilepath.c_str());
	if(bRet)
	{
		MLOG(ZQ::common::Log::L_ERROR,  CLOGFMT(NormalizeSparseFile,"Failed to rename temp index file %s as index file %s, (%d, %s)"),
			_tempIndexFile.c_str(), _indexFilepath.c_str() , SYS::getLastErr(), SYS::getErrorMessage().c_str());
		result = -1;
		return result;
	}
	return result;
}
}}
