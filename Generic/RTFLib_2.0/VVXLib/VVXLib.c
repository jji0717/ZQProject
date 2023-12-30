// $Header: /ZQProjs/Generic/RTFLib_2.0/VVXLib/VVXLib.c 1     10-11-12 15:59 Admin $
//
// Copyright (c) 2006 SeaChange International
//
// Module Name:
//
//    	VVXLib.c
//
// Abstract:
//
//		A set of subroutines to create an VOD VVX style index file
//
//		SetupInitialIndexHeader
//			Initialize a VOD VVX index file header
//
//		WriteIndexHeader
//			Format output buffer with VVX VVX header data
//
//		WriteFirstRecord
//			Write a single index record which contains
//
//		WriteIndexRecord
//			Write two index records overwriting 1 prior record entry.
//
// Author:
//
//		Philip Wells
//
// Environment:
//
//		Any
//
// Notes:
//
//		Module written to run in user mode and kernel mode 
//
// Revision History:
//
// $Log: /ZQProjs/Generic/RTFLib_2.0/VVXLib/VVXLib.c $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 1     08-02-26 11:02 Ken.qian
//

#include "RTFPrv.h"

//
// verify that the indicated mpeg descriptor is a AC3 registration descriptor
//
#define IS_AC3_DESCRIPTOR(a)	(a[1] == 4 && a[2] == 'A' && a[3] == '-' && a[4] == '3')
//
//  macro used to compute a memory pointer from a base address and byte offset
//
#define BYTE_OFFSET_POINTER(Address, Offset)		((PVOID) (((PUCHAR)(Address)) + (Offset)))
//
// VOD VVX index headers
//
typedef struct _extension_headers
{
	VVX_TS_INFORMATION			tsInfo;
	VVX_ES_DESCRIPTOR_DATA		esDescriptors[VVX_MAX_SUBFILE_COUNT];
	VVX_ES_INFORMATION			esInfo[VVX_MAX_SUBFILE_COUNT];
	VVX_V7_SUBFILE_INFORMATION	si[VVX_MAX_SUBFILE_COUNT];
} EXTENSION_HEADERS, *PEXTENSION_HEADERS;

#ifdef _LINUX

#define LINUX_TO_WIN32_DELTA_EPOCH_SECONDS_HI 116444
#define LINUX_TO_WIN32_DELTA_EPOCH_SECONDS_LO 73600

static void GetSystemTimeAsFileTime( PFILETIME pTime )
{
	time_t rawTime;
	INT64 winFileTime;
	INT64 adjust;

#ifdef __KERNEL__
	struct timeval tv;
	do_gettimeofday( &tv );
	rawTime = tv.tv_sec;
#else
	// get seconds since the Linux Epoch (midnight GMT 1/1/1970)
	time( &rawTime );
#endif
	// convert to Windows Epoch (midnight GMT 1/1/1601)
	winFileTime = (INT64)rawTime;
	// RATS! NO 64 BIT CONSTANTS!
	adjust = LINUX_TO_WIN32_DELTA_EPOCH_SECONDS_HI;
	adjust *= 100000;
	adjust += LINUX_TO_WIN32_DELTA_EPOCH_SECONDS_LO;
	winFileTime += adjust;
	// FILETIME is 100 ns intervals, so multiply by 10E7
	winFileTime *= 10000000;
	// fill in the FILETIME structure
	pTime->dwHighDateTime = (DWORD)(winFileTime>>32);
	pTime->dwLowDateTime  = (DWORD)(winFileTime);
}
#endif

//
// Here to check PMT registration descriptor for AC-3 stream.  It is assumed that a stream 
// type of 0x81 is AC3
//
// Looks for a registration descriptor for the stream type 0x81 that indicates it is not 
// an AC3 stream or a AC3 descriptor for a non 0x81 stream type.
//
static void CheckRegistrationDescriptor(VVX_INDEX_CONTEXT *pContext)
{
	//
	// form a pointer to the elemtary stream info section of the pmt
	//
	PBYTE esData = pContext->pmt+pContext->patEsInfoOffset;
	//
	// get a pointer to our extension headers
	//
	PEXTENSION_HEADERS eh = (PEXTENSION_HEADERS)pContext->extensionHeaders;

	int i;
	int esInfoBytes = pContext->pmtEsInfoLength;
	while(esInfoBytes > 0)
	{
		//
		// load the pid in the descriptor
		//
		int streamType = esData[0];
		int pid = ((esData[1] & 0x1f) << 8) | esData[2];
		int esInfoLength = ((esData[3] & 0x0f) << 8) | esData[4];
		PBYTE descriptor = esData+5;
		//
		// setup for next stream
		//
		esData += esInfoLength + 5;
		esInfoBytes -= esInfoLength + 5;
		//
		// locate entry in our es info table for this pid
		//
		for (i = 0 ; i < pContext->pmtEsInfoCount ; i++)
		{
			if (eh->esInfo[i].pid == pid)
			{
				break;
			}
		}

		if (i != pContext->pmtEsInfoCount)
		{
			//
			// dafault to ac3 audio if stream type is 81
			//
			if (streamType == 0x81)
			{
				eh->esInfo[i].streamFlags |= VVX_ESFLAG_AC3_AUDIO;
			}
			//
			// process each descriptor for this stream
			//
			while(esInfoLength)
			{
				if (descriptor[0] == 5)		// if registration descriptor
				{
					if (IS_AC3_DESCRIPTOR(descriptor) && streamType != 0x81)
					{
						// AC-3 reg desc found, set audio flag in flags byte
						//
						eh->esInfo[i].streamFlags |= VVX_ESFLAG_AC3_AUDIO;
					}
					else if (esData[0] == 0x81)
					{
						// A non-AC-3 reg descriptor found for stream type 0x81
						// Clear the flag if it was set before
						//
						eh->esInfo[i].streamFlags &= ~VVX_ESFLAG_AC3_AUDIO;
					}

					break;
				}

				esInfoLength -= 2 + descriptor[1];
				descriptor += 2 + descriptor[1];
			}
		}
	}
}
//
// get byte offset of elementary stream data in pmt
//
static int SniffPmt(VVX_INDEX_CONTEXT *pContext, PBYTE packet, int byteOffset, int sectionLength)
{
	int					status = TRUE;
	int					bytesRemaining;
	int					programInfoLength;
	int					programNumber;
	PBYTE				esData;
	//
	// initialize count of elementary stream descriptors
	//
	pContext->pmtEsInfoCount = 0;
	//
	// Calculate the number of bytes remaining in the PMT (4 is the crc32 length)
	//
	bytesRemaining = (sectionLength - 4);
	//
	// Pick up program_number
	//
	programNumber = (packet[byteOffset] << 8) | packet[byteOffset+1];
	byteOffset += 2;
	//
	// Process the pmt 
	//
	if (programNumber != 0)
	{
		//
		// Skip over reserved					(2 bits)
		// Skip over version_number				(5 bits)
		// Skip over current_next_indicator		(1 bit)
		// Skip over section_number				(8 bits)
		// Skip over last_section_number		(8 bits)
		//
		byteOffset += 3;
		//
		// skip PCR_PID
		//
		byteOffset += 2;
		//
		// Pick up program_info_length
		// Skip over program descriptors
		//
		programInfoLength = ((packet[byteOffset] & 0x0f) << 8) | packet[byteOffset+1];
		byteOffset +=  programInfoLength + 2;
		pContext->patEsInfoOffset = byteOffset;
		pContext->pmtEsInfoCount = 0;
		pContext->pmtEsInfoLength = 0;
		esData = BYTE_OFFSET_POINTER(pContext->pmt, byteOffset);
		//
		// syntax check es info section and compute count of pids described
		//
		bytesRemaining -= programInfoLength + 9;	// 9 = fixed section header size 
		while (bytesRemaining > 0)
		{
			//
			// accumulate sum of esinfolengths
			//
			int esInfoLength = ((packet[byteOffset+pContext->pmtEsInfoLength+3] & 0x0f) << 8) | packet[byteOffset+pContext->pmtEsInfoLength+4];
			if (bytesRemaining >= esInfoLength + 5)
			{
				bytesRemaining -= esInfoLength + 5; 
				pContext->pmtEsInfoLength += esInfoLength + 5;
				pContext->pmtEsInfoCount++;
			}
			else
			{
				PRINTF("Syntax error parsing PMT ES info section\n");
				status = FALSE;
				break;
			}
		}
	}

	return status;
}
//
// scan the PSI tables for the program info table
//
static int SniffPmtPid(VVX_INDEX_CONTEXT *pContext)
{
	int		transportTableFound = 0;
	PBYTE	packet = pContext->pmt;
	int		status = NO_ERROR;
	int		adaptationFieldControl;
	int		byteOffset = 0;
	int		tableId;
	int		bytesRemaining;
	int		bytesThisTable;
	//
	// skip over transport packet header
	//
	bytesRemaining = TRANSPORT_PACKET_SIZE - 4;
	byteOffset = 4;
	//
	// check for adaptation field
	//
	adaptationFieldControl = (packet[3] & 0x30) >> 4;
	if ((adaptationFieldControl == 0x2) || (adaptationFieldControl == 0x3))
	{
		//
		// skip over adaptation field
		//
		byteOffset += packet[byteOffset] + 1;
	}
	//
	// Parse PMT table contents
	//
	if ((adaptationFieldControl == 0x01) || (adaptationFieldControl == 0x03))
	{
		//
		// skip to start of table
		//
		byteOffset += packet[byteOffset] + 1;
		//
		// parse pmt tables
		//
		while(!transportTableFound && byteOffset < bytesRemaining)
		{
			int sectionLength;
			//
			// Pick up table id
			//
			tableId = packet[byteOffset];
			byteOffset++;
			//
			// Pick up section_length
			//
			// section_length is the number of bytes following the section_length byte
			// including the crc32 at the end
			//
			sectionLength = ((packet[byteOffset] & 0x0f) << 8) | packet[byteOffset+1];
			byteOffset += 2;
			//
			// dispatch on table type
			//
			switch(tableId)
			{
			case 0xff:
				//
				// filler byte is end of table data
				//
				byteOffset = bytesRemaining;
				break;

			default:
				//
				// conditional access table or user private table
				//
				// don't overflow a single packet
				//
				bytesThisTable = sectionLength;
				if (bytesThisTable + byteOffset > bytesRemaining)
				{
					bytesThisTable = bytesRemaining - byteOffset;
				}

				byteOffset += bytesThisTable;
				break;

			case MPEG2_TRANSPORT_PMT_TABLE_ID:
				transportTableFound = 1;
				//
				// process pmt
				//
				status = SniffPmt(pContext, packet, byteOffset, sectionLength);
				break;
			}

			if (status != NO_ERROR)
				break;
		}
	}

	return status;
}

//
// compute a checksum for the vvx index.  Checksum is valid for V6.1 vvx files 
// and later
//
// Returns:
//
//	!= 0	valid checksum
//	== 0	checksum not supported in vvx version
//
static ULONG TrickFilesComputeVvxChecksum(PVOID vvxHeader)
{
	register ULONG checksum = 0;
	PBYTE vvxIndexHeader = (PBYTE)vvxHeader;
	int len 			= GETLONG(vvxIndexHeader+VVX_V7IH_HEADERLENGTH_OFFSET) - sizeof(ULONG);
	
	int alignedCount 	= len % sizeof(ULONG);
	int mask 			= (ULONG)(-1) >> ((sizeof(ULONG) - alignedCount) * 8);

	register int l1 	= (len - alignedCount) / sizeof(ULONG);
	register PULONG ptr = (PULONG) vvxIndexHeader;

	while(l1)
	{
		PULONG p = ptr;
		switch(l1 % 10)
		{
			case 0: checksum += *ptr++;
			case 9: checksum += *ptr++;
			case 8: checksum += *ptr++;
			case 7: checksum += *ptr++;
			case 6: checksum += *ptr++;
			case 5: checksum += *ptr++;
			case 4: checksum += *ptr++;
			case 3: checksum += *ptr++;
			case 2: checksum += *ptr++;
			case 1: checksum += *ptr++;
		}
		l1 -= (ptr - p);
	}
	//
	// fold in unaligned bytes
	//
	if (alignedCount)
		checksum += (*ptr & mask);

	if (checksum == 0) 
		checksum = 0xffffffff;
		
	return checksum;
}
//
// This function is called to collect the information required to construct a VOD VVX index
// header and extension headers
//
// Return errno, 0 == success
//
EXPORT int vvxSetupInitialIndexHeader(VVX_INDEX_CONTEXT *pContext)
{
	int						status = 0;		// no error
	int						i;
	PEXTENSION_HEADERS		eh = (PEXTENSION_HEADERS)pContext->extensionHeaders;
	int						pid;
	//
	// sniff the pmt
	//
	SniffPmtPid(pContext);
	//
	// record current time
	//
	GetSystemTimeAsFileTime(&pContext->systemTime);
	//
	// compute the number of files being created
	//
	pContext->fileCount = 1 + pContext->ffSpeedCount + pContext->frSpeedCount;
	//
	// format subfile info section
	//
	for (i = 0 ; i < (int)pContext->fileCount ; i++)
	{
		strcpy((char *)(eh->si[i].fileExtension), pContext->outputFiles[i].extension);
		eh->si[i].fileType						= VVX_SUBFILE_TYPE_SPTS;
		eh->si[i].startingByte					= 0;
		eh->si[i].endingByte					= pContext->outputFiles[i].fileSize;
		eh->si[i].playTimeInMilliSeconds		= (ULONG)RTF_DIV64((pContext->outputFiles[i].fileSize * 8000), pContext->transportBitRate);
		//
		// record the speed of this file
		//
		eh->si[i].speed.numerator				= pContext->outputFiles[i].numerator;
		eh->si[i].speed.denominator				= pContext->outputFiles[i].denominator;
		eh->si[i].recordIndex					= pContext->outputFiles[i].index;
	}
	//
	// format transport stream information data
	//
	eh->tsInfo.programNumber					= pContext->program_number;
	eh->tsInfo.pmtPid							= pContext->pmtPid;
	eh->tsInfo.pcrPid							= pContext->pcrPid;
	eh->tsInfo.videoPid							= pContext->videoPid;
	//
	// insert elementary stream information data
	//
	if (pContext->pmtEsInfoLength)
	{
		int index = 0;
		PBYTE esData = pContext->pmt + pContext->patEsInfoOffset;
		int esInfoBytesRemaining = pContext->pmtEsInfoLength;
		esData = BYTE_OFFSET_POINTER(pContext->pmt, pContext->patEsInfoOffset);
		while(esInfoBytesRemaining > 0)
		{
			int esInfoLength = ((esData[3] & 0x0f) << 8) | esData[4];
			pid = ((esData[1] & 0x1f) << 8) | esData[2];
			//
			// Loop through existing es table to see if we already have this stream
			//
			for (i = 0; i < pContext->pmtEsInfoCount; i++)
			{
				if (eh->esInfo[i].pid == pid)
				{
					break;
				}
			}
			//
			// Add this stream to table if not currently known
			//
			if (i == pContext->pmtEsInfoCount)
			{
				eh->esInfo[index].streamType			= esData[0];
				eh->esInfo[index].streamFlags			= 0;
				eh->esInfo[index].pid					= pid;
				index++;
			}
			esData += esInfoLength + 5;
			esInfoBytesRemaining -= esInfoLength + 5;
		}
		//
		// fixup AC-3 audio streams if needed
		//
		CheckRegistrationDescriptor(pContext);
		//
		// reset the current index file position
		//
		pContext->currentIndexFilePos = 0;
	}
	return status;
}
//
// call this function to write a leading index record to the index file.  This must be called
// after WriteIndexHeader so that the byte offset is correct.  A leading record is written in case
// the video is to be played while it is being created.  In this case, the record acts as an "indicator"
// record where the file byte offsets contain file size data.  Since this is the first record, the 
// file sizes are all zero.
//
//	Returns errno, 0 == success
//
EXPORT int vvxWriteFirstRecord(VVX_INDEX_CONTEXT *pContext, PBYTE buffer, int bufferLen)
{
	int status = 0;
	if (bufferLen >= VVX_V7R_SIZE)
	{
		int offset = VVX_V7R_TRICK_FRAMEBYTEOFFSET_OFFSET;
		//
		// zero the record
		//
		memset(buffer, 0, VVX_V7R_SIZE);
		SETLONG(buffer+VVX_V7R_RECORDTYPE_OFFSET, TRICK_INDEX);	// recordType
		SETLONG(buffer+VVX_V7R_TRICK_TIMECODE_OFFSET, 0);		// trick.timeCode.time

		status = pContext->writeRtn(pContext->writeContext, 0, buffer, VVX_V7R_SIZE);
		if (status)
		{
			status = 0;
		}
		else
		{
			status = EIO;
		}
		//
		// flag record written
		//
		pContext->firstRecordWritten = 1;
		//
		// update current index file position
		//
		pContext->currentIndexFilePos += VVX_V7R_SIZE;
	}

	return status;
}
//
// use this function to when a valid index has been discovered.
//
// timeCode		- GOP time code associated with the I-Frame being written
// indexTable	- array of byte offsets in the various files for the I-Frame
// sizeTable	- array of current file sizes
// buffer		- a buffer to write the record data into
// bufferLen	- the size of the buffer supplied
//
//	Returns
//		error from errno, 0 == success
//
EXPORT int vvxWriteIndexRecord(VVX_INDEX_CONTEXT *pContext, TIME_CODE timeCode, INT64 *indexTable, INT64 *sizeTable, PBYTE buffer, int bufferLen)
{
	int status = ENOMEM;
	PVVX_V7_RECORD rec = (PVVX_V7_RECORD)buffer;
	PEXTENSION_HEADERS eh = (PEXTENSION_HEADERS)pContext->extensionHeaders;
	if (bufferLen >= 2*VVX_V7R_SIZE)
	{
		// buffer is large enough
		//
		PBYTE frameData = buffer;
		int offset = VVX_V7R_TRICK_FRAMEBYTEOFFSET_OFFSET;
		int offsetFromCurrentPosition = 0;
		int j;
		//
		// format the record as a trick index record
		// copy the time code
		//
		SETLONG(frameData+VVX_V7R_RECORDTYPE_OFFSET, TRICK_INDEX);
		SETCHAR(frameData+VVX_V7R_TRICK_TIMECODE_FRAMES_OFFSET, timeCode.frames);
		SETCHAR(frameData+VVX_V7R_TRICK_TIMECODE_HOURS_OFFSET, timeCode.hours);
		SETCHAR(frameData+VVX_V7R_TRICK_TIMECODE_MINUTES_OFFSET, timeCode.minutes);
		SETCHAR(frameData+VVX_V7R_TRICK_TIMECODE_SECONDS_OFFSET, timeCode.seconds);
		//
		// copy the index pointer values
		//
		for (j = 0; j < 1 + pContext->ffSpeedCount + pContext->frSpeedCount ; j++)
		{
			SETLONGLONG(frameData+offset, indexTable[eh->si[j].recordIndex]);
			offset += VVX_V7R_TRICK_FRAMEBYTEOFFSET_SIZE;
		}
		//
		// should we generate an "indicator" index record?
		//
		if( pContext->useIndicatorRecords == 0 )
		{
			//
			// no - write the record with zero offset from the current file position
			//
			status = pContext->writeRtn(pContext->writeContext, 0, buffer, VVX_V7R_SIZE);
		}
		else
		{
			//
			// yes - the next record is an "indicator" record containing the file sizes
			//
			// point at next index record
			//
			frameData += VVX_V7R_SIZE;
			
			offset = VVX_V7R_TRICK_FRAMEBYTEOFFSET_OFFSET;
			//
			// set up record type and timecode to zero
			//
			SETLONG(frameData+VVX_V7R_RECORDTYPE_OFFSET, TRICK_INDEX);
			SETLONG(frameData+VVX_V7R_TRICK_TIMECODE_OFFSET, 0);
			//
			// copy file sizes from size table
			//
			for ( j = 0; j < 1 + pContext->ffSpeedCount + pContext->frSpeedCount; j++ )
			{
				SETLONGLONG(frameData+offset, sizeTable[eh->si[j].recordIndex]);
				offset += VVX_V7R_TRICK_FRAMEBYTEOFFSET_SIZE;
			}

			status = 0;
			//
			// compute a byte offset variance.  It's most likely that we'll create a negative offset
			//
			if (pContext->firstRecordWritten)
			{
				offsetFromCurrentPosition = -(int)VVX_V7R_SIZE;
			}
			//
			// write the record and indicate an offset from the current file position
			//
			status = pContext->writeRtn(pContext->writeContext, offsetFromCurrentPosition, buffer, 2*VVX_V7R_SIZE);
		}
		if (status)
		{
			// convert bool to status code
			//
			status = 0;
		}
		else
		{
			status = EIO;
		}
		//
		// count a record and frame
		//
		pContext->trickIndexRecordCount++;
		pContext->frameDataCount++;
		//
		// update current index file position
		//
		pContext->currentIndexFilePos += offsetFromCurrentPosition;
		pContext->currentIndexFilePos += (pContext->useIndicatorRecords == 0 ) ? VVX_V7R_SIZE : 2*VVX_V7R_SIZE;
	}

	return status;
}
//
// use this function to re-write the final terminator with the input file byte count.
//
// byteCount	- final byte count of input file
// buffer		- a buffer to write the record data into
// bufferLen	- the size of the buffer supplied
//
//	Returns
//		error from errno, 0 == success
//
EXPORT int vvxWriteFinalTerminator(VVX_INDEX_CONTEXT *pContext, INT64 *sizeTable, PBYTE buffer, int bufferLen)
{
	int status = ENOMEM;
	PVVX_V7_RECORD rec = (PVVX_V7_RECORD)buffer;
	PEXTENSION_HEADERS eh = (PEXTENSION_HEADERS)pContext->extensionHeaders;
	if (bufferLen >= VVX_V7R_SIZE)
	{
		// buffer is large enough
		//
		PBYTE frameData = buffer;
		int offset;
		int offsetFromCurrentPosition = 0;
		int j;
		//
		// the terminator record contains the file sizes
		//
		// point at next index record
		//
		offset = VVX_V7R_TRICK_FRAMEBYTEOFFSET_OFFSET;
		//
		// set up record type and timecode to zero
		//
		SETLONG(frameData+VVX_V7R_RECORDTYPE_OFFSET, TRICK_INDEX);
		SETLONG(frameData+VVX_V7R_TRICK_TIMECODE_OFFSET, 0);
		//
		// copy file sizes from size table
		//
		for ( j = 0; j < 1 + pContext->ffSpeedCount + pContext->frSpeedCount; j++ )
		{
			SETLONGLONG(frameData+offset, sizeTable[eh->si[j].recordIndex]);
			offset += VVX_V7R_TRICK_FRAMEBYTEOFFSET_SIZE;
		}
		status = 0;
		//
		// compute a byte offset variance.  It's most likely that we'll create a negative offset
		//
		if (pContext->firstRecordWritten)
		{
			offsetFromCurrentPosition = -(int)VVX_V7R_SIZE;
		}
		//
		// write the terminator and indicate an offset from the current file position
		//
		status = pContext->writeRtn(pContext->writeContext, offsetFromCurrentPosition, buffer, VVX_V7R_SIZE);
		// convert bool to status code
		status = (status) ? 0 : EIO;
	}

	return status;
}
//
// this function is called to format and write the index headers.
//
// Returns errno, 0 == success
//
EXPORT int vvxWriteIndexHeader(VVX_INDEX_CONTEXT *pContext, PBYTE buffer,
							   int bufferLen, char *pProgramVersionStr)
{
	int status = ENOMEM;
	PEXTENSION_HEADERS eh = (PEXTENSION_HEADERS)pContext->extensionHeaders;
	if (eh)
	{
		LARGE_INTEGER li;
		PBYTE esData;
		int nextOffset;
		int i, headerLength;
		ULONG checksum;
		PULONG pChecksum = &checksum;
		int required;
		int unusedOffset = VVX_V7IH_SIZE;

		//
		// compute the required buffer size
		//
		required = ( VVX_MAX_SUBFILE_COUNT * VVX_V7IH_SIZE ) +
				   ( pContext->pmtEsInfoCount * VVX_V7EI_SIZE ) +
				   ( pContext->pmtEsInfoLength ) +
				   ( pContext->lenZeroMotionB ) +
				   ( pContext->lenZeroMotionP ) +
				   ( 4 );
		if( bufferLen  < required )
		{
			return EIO;
		}

		//
		// start off with a clean slate
		//
		memset(buffer, 0, bufferLen);
		//
		// setup signature, version and filename
		//
		strcpy((char *)(buffer+VVX_V7IH_SIGNATURE_OFFSET), VVX_INDEX_SIGNATURE);
		strcpy((char *)(buffer+VVX_V7IH_PROGRAMVERSION_OFFSET), pProgramVersionStr);
		strcpy((char *)(buffer+VVX_V7IH_FILENAME_OFFSET), pContext->filename);
		//
		// set version numbers
		//
		SETSHORT(buffer+VVX_V7IH_MAJORVERSION_OFFSET, 7);
		SETSHORT(buffer+VVX_V7IH_MINORVERSION_OFFSET, (unsigned short)pContext->minorVersion);
		//
		// convert FILETIME to a INT64
		//
		li.HighPart = pContext->systemTime.dwHighDateTime;
		li.LowPart = pContext->systemTime.dwLowDateTime;
		SETLONGLONG(buffer+VVX_V7IH_SYSTEMTIME_OFFSET, li.QuadPart);
		//
		// dts/pts time offset is the decoding delay of the first I-Frame
		//
		SETLONGLONG(buffer+VVX_V7IH_DTSPTSTIMEOFFSET_OFFSET, pContext->dtsPtsTimeOffset);
		//
		// set flags
		//
		SETCHAR(buffer+VVX_V7IH_REVERSEREAD_OFFSET, 1);
		SETCHAR(buffer+VVX_V7IH_USEPRIVATEDATA_OFFSET, 0);
		SETCHAR(buffer+VVX_V7IH_AVAILABLE_BOOLEAN_1_OFFSET, 0);
		SETCHAR(buffer+VVX_V7IH_AVAILABLE_BOOLEAN_OFFSET, 0);
		//
		// transport bitrate and stream type
		//
		SETLONG(buffer+VVX_V7IH_STREAMBITRATE_OFFSET, pContext->transportBitRate);
		SETLONG(buffer+VVX_V7IH_STREAMTYPE_OFFSET, 1);
		//
		// force record count to 1 on first pass
		//
		if (pContext->trickIndexRecordCount != 0)
		{
			SETLONG(buffer+VVX_V7IH_TRICKINDEXRECORDCOUNT_OFFSET, pContext->trickIndexRecordCount);
			SETLONG(buffer+VVX_V7IH_FRAMEDATACOUNT_OFFSET, pContext->frameDataCount);
		}
		else
		{
			// don't force if this is the final update on an audio-only file (no index records)
			if( pContext->currentIndexFilePos == 0 )
			{
				SETLONG(buffer+VVX_V7IH_TRICKINDEXRECORDCOUNT_OFFSET, 1);		// needed for voddrv
				SETLONG(buffer+VVX_V7IH_FRAMEDATACOUNT_OFFSET, 1);				// needed for voddrv
			}
		}
		//
		// set other video header elements
		//
		SETLONG(buffer+VVX_V7IH_FRAMERATECODE_OFFSET, pContext->frame_rate_code);
		SETLONG(buffer+VVX_V7IH_HORIZONTALSIZE_OFFSET, pContext->horizontal_size);
		SETLONG(buffer+VVX_V7IH_VERTICALSIZE_OFFSET, pContext->vertical_size);
		SETLONG(buffer+VVX_V7IH_VIDEOBITRATE_OFFSET, pContext->videoBitRate);
		SETLONG(buffer+VVX_V7IH_ZEROMOTIONPICTURETYPE_OFFSET, pContext->zeroMotionFrameType);
		SETLONG(buffer+VVX_V7IH_EXTENDEDHEADERLENGTH_OFFSET, 0);	//no extended header
		//
		// copy subfile info
		//
		SETLONG(buffer+VVX_V7IH_SUBFILEINFORMATIONMAXCOUNT_OFFSET, VVX_MAX_SUBFILE_COUNT);
		SETLONG(buffer+VVX_V7IH_SUBFILEINFORMATIONCOUNT_OFFSET, pContext->fileCount);
		SETLONG(buffer+VVX_V7IH_SUBFILEINFORMATIONLENGTH_OFFSET, VVX_V7SI_SIZE * VVX_MAX_SUBFILE_COUNT);
		SETLONG(buffer+VVX_V7IH_SUBFILEINFORMATIONOFFSET_OFFSET, unusedOffset);
		for (i = 0 ; i < (int)pContext->fileCount ; i++)
		{
			// update the ending byte with the file size
			eh->si[i].endingByte = pContext->outputFiles[i].fileSize;
			// update the playtime with the new file size
			eh->si[i].playTimeInMilliSeconds = (ULONG)RTF_DIV64((pContext->outputFiles[i].fileSize * 8000), pContext->transportBitRate);
			memcpy(buffer+unusedOffset+VVX_V7SI_FILEEXTENSION_OFFSET,		eh->si[i].fileExtension, VVX_V7SI_FILEEXTENSION_SIZE);
			SETLONG(buffer+unusedOffset+VVX_V7SI_FILETYPE_OFFSET,			eh->si[i].fileType);
			SETLONGLONG(buffer+unusedOffset+VVX_V7SI_STARTINGBYTE_OFFSET,	eh->si[i].startingByte);
			SETLONGLONG(buffer+unusedOffset+VVX_V7SI_ENDINGBYTE_OFFSET,		eh->si[i].endingByte);
			SETLONG(buffer+unusedOffset+VVX_V7SI_PLAYTIMEINMILLISECONDS_OFFSET, eh->si[i].playTimeInMilliSeconds);
			SETLONG(buffer+unusedOffset+VVX_V7SI_SPEED_NUMERATOR_OFFSET,	eh->si[i].speed.numerator);
			SETLONG(buffer+unusedOffset+VVX_V7SI_SPEED_DENOMINATOR_OFFSET,	eh->si[i].speed.denominator);
			SETLONG(buffer+unusedOffset+VVX_V7SI_RECORDINDEX_OFFSET,		eh->si[i].recordIndex);
			unusedOffset += VVX_V7SI_SIZE;
		}
		for ( ; i < VVX_MAX_SUBFILE_COUNT; i++)
		{
			unusedOffset += VVX_V7SI_SIZE;
		}
		//
		// copy transport stream info
		//
		SETLONG(buffer+VVX_V7IH_TSINFORMATIONCOUNT_OFFSET, 1);
		SETLONG(buffer+VVX_V7IH_TSINFORMATIONLENGTH_OFFSET, VVX_V7TI_SIZE);
		SETLONG(buffer+VVX_V7IH_TSINFORMATIONOFFSET_OFFSET, unusedOffset);
		SETLONG(buffer+unusedOffset+VVX_V7TI_PROGRAMNUMBER_OFFSET, eh->tsInfo.programNumber);
		SETLONG(buffer+unusedOffset+VVX_V7TI_PMTPID_OFFSET, eh->tsInfo.pmtPid);
		SETLONG(buffer+unusedOffset+VVX_V7TI_PCRPID_OFFSET, eh->tsInfo.pcrPid);
		SETLONG(buffer+unusedOffset+VVX_V7TI_VIDEOPID_OFFSET, eh->tsInfo.videoPid);
		unusedOffset += VVX_V7TI_SIZE;
		//
		// copy elementary stream info
		//
		SETLONG(buffer+VVX_V7IH_ESINFORMATIONCOUNT_OFFSET, pContext->pmtEsInfoCount);
		SETLONG(buffer+VVX_V7IH_ESINFORMATIONLENGTH_OFFSET, VVX_V7EI_SIZE * pContext->pmtEsInfoCount);
		SETLONG(buffer+VVX_V7IH_ESINFORMATIONOFFSET_OFFSET, unusedOffset);
		for (i = 0 ; i < (int)pContext->pmtEsInfoCount ; i++)
		{
			SETCHAR(buffer+unusedOffset+VVX_V7EI_STREAMTYPE_OFFSET, eh->esInfo[i].streamType);
			SETCHAR(buffer+unusedOffset+VVX_V7EI_STREAMFLAGS_OFFSET, eh->esInfo[i].streamFlags);
			SETSHORT(buffer+unusedOffset+VVX_V7EI_PID_OFFSET, eh->esInfo[i].pid);
			unusedOffset += VVX_V7EI_SIZE;
		}
		//
		// copy elementary stream descriptor data
		//
		if (pContext->pmtEsInfoLength)
		{
			int esDescriptorInfoRemaining = pContext->pmtEsInfoLength;
			SETLONG(buffer+VVX_V7IH_ESDESCRIPTORDATACOUNT_OFFSET, pContext->pmtEsInfoCount);
			SETLONG(buffer+VVX_V7IH_ESDESCRIPTORDATALENGTH_OFFSET, VVX_V7DD_SIZE * pContext->pmtEsInfoCount);
			SETLONG(buffer+VVX_V7IH_ESDESCRIPTORDATAOFFSET_OFFSET, unusedOffset);
			esData = pContext->pmt+pContext->patEsInfoOffset;
			nextOffset = unusedOffset;
			unusedOffset += VVX_V7DD_SIZE*pContext->pmtEsInfoCount;
			while(esDescriptorInfoRemaining > 0)
			{
				//
				// copy the es data
				//
				int esInfoLength = ((esData[3] & 0x0f) << 8) | esData[4];
				if (esInfoLength != 0)
				{
					int pid = ((esData[1] & 0x1f) << 8) | esData[2];
					for (i = 0 ; i < (int)pContext->pmtEsInfoCount ; i++)
					{
						if (pid = eh->esInfo[i].pid)
						{
							SETLONG(buffer+nextOffset+VVX_V7DD_DESCRIPTORDATALENGTH_OFFSET, esInfoLength);
							SETLONG(buffer+nextOffset+VVX_V7DD_DESCRIPTORDATAOFFSET_OFFSET, unusedOffset);
							memcpy(buffer+unusedOffset, esData+5, esInfoLength);
							esData += esInfoLength+5;
							unusedOffset += esInfoLength;
							break;
						}
					}
					esDescriptorInfoRemaining -= esInfoLength+5;
				}
				else
				{
					SETLONG(buffer+nextOffset+VVX_V7DD_DESCRIPTORDATALENGTH_OFFSET, 0);
					SETLONG(buffer+nextOffset+VVX_V7DD_DESCRIPTORDATAOFFSET_OFFSET, 0);
					esData += 5;
					esDescriptorInfoRemaining -= 5;
				}
				nextOffset += VVX_V7DD_SIZE;
			}
		}
		else
		{
			SETLONG(buffer+VVX_V7IH_ESDESCRIPTORDATACOUNT_OFFSET, 0);
			SETLONG(buffer+VVX_V7IH_ESDESCRIPTORDATALENGTH_OFFSET, 0);
		}
		//
		// insert zero motion B picture
		//
		SETLONG(buffer+VVX_V7IH_ZEROMOTIONBFRAMEPESLENGTH_OFFSET, pContext->lenZeroMotionB);
		SETLONG(buffer+VVX_V7IH_ZEROMOTIONBFRAMEPESOFFSET_OFFSET, unusedOffset);
		memcpy(buffer+unusedOffset, pContext->zeroMotionB, pContext->lenZeroMotionB);
		unusedOffset += pContext->lenZeroMotionB;
		//
		// copy zero motion frame
		//
		SETLONG(buffer+VVX_V7IH_ZEROMOTIONFRAMEPESLENGTH_OFFSET, pContext->lenZeroMotionP);
		SETLONG(buffer+VVX_V7IH_ZEROMOTIONFRAMEPESOFFSET_OFFSET, unusedOffset);
		memcpy(buffer+unusedOffset, pContext->zeroMotionP, pContext->lenZeroMotionP);
		unusedOffset += pContext->lenZeroMotionP;
		//
		// set splice info
		//
		SETLONG(buffer+VVX_V7IH_SPLICEINDEXRECORDCOUNT_OFFSET, 0);
		SETLONG(buffer+VVX_V7IH_SPLICINGPARAMETERSLENGTH_OFFSET, 0);
		SETLONG(buffer+VVX_V7IH_SPLICINGPARAMETERSOFFSET_OFFSET, 0);
		//
		// now set the header length
		//
		headerLength = unusedOffset+4;
		SETLONG(buffer+VVX_V7IH_HEADERLENGTH_OFFSET, headerLength);
		//
		// copy frame data info
		//
		SETLONG(buffer+VVX_V7IH_FRAMEDATALENGTH_OFFSET, pContext->trickIndexRecordCount * VVX_V7R_SIZE);
		SETLONG(buffer+VVX_V7IH_FRAMEDATAOFFSET_OFFSET, headerLength);
		//
		// finally compute checksum
		//
		*pChecksum = TrickFilesComputeVvxChecksum((PVVX_INDEX_HEADER)buffer);
		memcpy(buffer+headerLength-sizeof(ULONG), pChecksum, sizeof(ULONG));

		status = pContext->writeRtn(pContext->writeContext, -(pContext->currentIndexFilePos), buffer, headerLength);
		if (status)
		{
			status = 0;
		}
		else
		{
			status = EIO;
		}
		//
		// update current index file position
		//
		pContext->currentIndexFilePos = headerLength;
	}

	return status;
}
