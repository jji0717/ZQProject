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



#include "PacingLogic.h"

#define TRANSPORT_PACKET_BYTES			188
#define PRINTF printf

#define PACING_LOGIC			"PacingLogic"
#define MOLOG					(*_log)

#define MAINFILE_EXT			"mpg"


using namespace ZQ::common;

bool PacingLogic::subFileWritten( const char* ext, int size, int64 offset )
{
	int nFileId = extToSubfileId(ext);
	if (nFileId < 0)
		return false;

	if (offset < _subFiles[nFileId].startingOffset)
		_subFiles[nFileId].startingOffset = offset;

	if (offset + size > _subFiles[nFileId].endingOffset)
		_subFiles[nFileId].endingOffset = offset + size;

	return true;
}

bool PacingLogic::indexReleasable( VVC_INDEX_RECORD_HEADER* ppRecHeader, char* pRecord, int nRecordLen )
{
#if NEW_VVC_DEFINE
	if (ppRecHeader->recType != INDEX_REC_TYPE_MP2_NORMAL_IFRAME && ppRecHeader->recType != INDEX_REC_TYPE_MP2_TRICK_IFRAME)
	{
		return true;
	}

	// is this a trick file I-frame?
	if( ppRecHeader->recType == INDEX_REC_TYPE_MP2_TRICK_IFRAME )
	{
		return trickIndexReleasable((VVC_INDEX_RECORD_MP2_TRICK_IFRAME *)pRecord);
	}
	else
	{
		return normalIndexReleasable((VVC_INDEX_RECORD_MP2_NORMAL_IFRAME*)pRecord);
	}

#else
	if (ppRecHeader->recType != INDEX_REC_TYPE_NORM_RATE_IFRAME && ppRecHeader->recType != INDEX_REC_TYPE_TRICK_RATE_IFRAME)
	{
		return true;
	}

	// is this a trick file I-frame?
	if( ppRecHeader->recType == INDEX_REC_TYPE_TRICK_RATE_IFRAME )
	{
		return trickIndexReleasable((VVC_INDEX_RECORD_TRICK_RATE_IFRAME *)pRecord);
	}
	else
	{
		return normalIndexReleasable((VVC_INDEX_RECORD_NORMAL_RATE_IFRAME*)pRecord);
	}
#endif
}


// generate an aligned pointer to a TLV. skip any
// leading zeros. advance index past the TLV header.
CTF_TLV* PacingLogic::alignTLV(char *pBuf, int *pIndex )
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

bool PacingLogic::parseIndexHeadBlock( char* pHeadBlock, int nLen )
{
	_nSubFiles = 0;

	// scan the index header TLVs
	int hdrIndex = 0;
	int hdrLimit = nLen;
	//int nFileCount = 0;
	while( hdrIndex < hdrLimit )
	{
		CTF_TLV* pHdrTLV = alignTLV(pHeadBlock, &hdrIndex );
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
			int64 firstOffset = -1;
			int64 finalOffset = -1;
			int direction   = -1;
			std::string strName;

			while( subIndex < subLimit )
			{ 
				CTF_TLV* pSubTLV = alignTLV((char*) pHeadBlock, &subIndex );
				char* pTmp = pHeadBlock + subIndex;
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
				case VVC_TAG_SUB_FILE_NAME:
					strName = pTmp;
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

			_subFiles[fileNumber].strExt = getExtFromName(strName.c_str());
			_subFiles[fileNumber].direction = direction;

			_nSubFiles++;
			// record the direction
			//dir[ fileNumber ] = direction;
			// increment the file count

		} // if( pHdrTLV->tag == VVC_TAG_SUB_FILE_INFO_BLOCK )
		// advance to the next TLV
		hdrIndex += pHdrTLV->len;
	}; // while( hdrIndex < hdrLimit )

	return true;
}

#if NEW_VVC_DEFINE
bool PacingLogic::trickIndexReleasable( VVC_INDEX_RECORD_MP2_TRICK_IFRAME* pFrame )
#else
bool PacingLogic::trickIndexReleasable( VVC_INDEX_RECORD_TRICK_RATE_IFRAME* pFrame )
#endif
{
	int64 nStartPos = ((int64)pFrame->packetOffset) * TRANSPORT_PACKET_BYTES;
	int64 nEndPos = ((int64)(pFrame->packetOffset + pFrame->lengthInPackets)) * TRANSPORT_PACKET_BYTES;

	if (_subFiles[pFrame->subFileIndex].direction == 2)
	{
		//reserse
		if (nStartPos < _subFiles[pFrame->subFileIndex].startingOffset)
		{
			MOLOG(Log::L_DEBUG, CLOGFMT(PACING_LOGIC, "[%s]: [%s] packetOffset[0x%08x] is not releasable at the moment"), 
				_strLogHint.c_str(), _subFiles[pFrame->subFileIndex].strExt.c_str(), pFrame->packetOffset);
			return false;
		}
		
	}
	else
	{
		if (nEndPos > _subFiles[pFrame->subFileIndex].endingOffset)
		{
			MOLOG(Log::L_DEBUG, CLOGFMT(PACING_LOGIC, "[%s]: [%s] packetOffset[0x%08x] is not releasable at the moment"), 
				_strLogHint.c_str(), _subFiles[pFrame->subFileIndex].strExt.c_str(), pFrame->packetOffset);
			return false;
		}
	}

	return true;
}

#if NEW_VVC_DEFINE
bool PacingLogic::normalIndexReleasable( VVC_INDEX_RECORD_MP2_NORMAL_IFRAME* pFrame )
#else
bool PacingLogic::normalIndexReleasable( VVC_INDEX_RECORD_NORMAL_RATE_IFRAME* pFrame )
#endif
{
	int64 nDataLen = ((int64)(pFrame->packetOffset + pFrame->lengthInPackets)) * TRANSPORT_PACKET_BYTES;

	if (_subFiles[0].endingOffset < nDataLen)
	{
		MOLOG(Log::L_DEBUG, CLOGFMT(PACING_LOGIC, "[%s]: [mpg] packetOffset[0x%08x] is not releasable at the moment"), 
			_strLogHint.c_str(), pFrame->packetOffset);
		return false;
	}

	return true;
}

PacingLogic::PacingLogic()
{
	_nSubFiles = 0;
	_log = &glog;
}

/*
* <Name><AssetID>cdntest0000000000020</AssetID><ProviderID>cdntest0000000000020schange</ProviderID></Name><SubType></SubType>
* <Name><AssetID>cdntest0000000000020</AssetID><ProviderID>cdntest0000000000020schange</ProviderID></Name><SubType>FF</SubType>
* <Name><AssetID>cdntest0000000000020</AssetID><ProviderID>cdntest0000000000020schange</ProviderID></Name><SubType>FR</SubType>
*/

std::string PacingLogic::getExtFromName( const char* name )
{
	std::string strRet;
	const char* szSubType = "<SubType>";
	char* pPtr = strstr((char*)name, szSubType);
	if (!pPtr)
		return strRet;

	char tmp[256], *p=tmp;
	pPtr += strlen(szSubType);
	while(*pPtr && *pPtr != '<')
		*p++ = *pPtr++;
	*p = '\0';

	strRet = tmp;
	return strRet;
}

std::string PacingLogic::convertExt( const char* ext )
{
	char tmp[256];
	const char* p1 = ext;
	char* p2 = tmp;

	//skip the .
	while(*p1 && *p1 == '.')
		p1++;

	while(*p1)
		*p2++ = toupper(*p1++);

	*p2 = '\0';

	if (!tmp[0])
		return MAINFILE_EXT;
	else
		return tmp;
}

int PacingLogic::extToSubfileId( const char* ext )
{
	std::string strExt = convertExt(ext);

	// at the first, the main file would write before the index generated, so
	if (strExt == MAINFILE_EXT)
		return 0;

	for(int i=0;i<_nSubFiles;i++)
	{
		if (strExt == _subFiles[i].strExt)
			return i;
	}

	return -1;
}

void PacingLogic::setLog( ZQ::common::Log* pLog )
{
	if (pLog)
		_log = pLog;
}

void PacingLogic::setLogHint( const char* strHint )
{
	_strLogHint = strHint;
}

