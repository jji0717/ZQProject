// VvxParser.cpp: implementation of the VvxParser class.
//
//////////////////////////////////////////////////////////////////////

#include <Log.h>

#if defined(ZQ_OS_MSWIN)
#include <process.h>
#endif

#if defined(ZQ_OS_MSWIN)
#include "vstrmuser.h"
#endif

#if defined (ZQ_OS_LINUX)
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#endif

#if defined(ZQ_OS_LINUX) || !defined(_MSC_VER)
#	include "vvx.h"
#   define BYTE_OFFSET_POINTER(b,o) ((void*) (((uint8*)(b)) + (o)))
#endif

#include <stdio.h>
#include <stdlib.h>
#include "VvxParser.h"


using namespace ZQ::common;

ZQ::common::Log* pParserLog = NULL;
//extern bool
//       LogMsg       ( LPCTSTR       lpszFmt, ... );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VvxParser::VvxParser()
{
	memset(this, 0, sizeof(*this));
}

VvxParser::~VvxParser()
{

}

#if defined(ZQ_OS_MSWIN)
bool VvxParser::getFileTime(SYSTEMTIME* pST)
{
	if (!pST)
		return false;

	return FileTimeToSystemTime(&_fileTime, pST);

}
#endif

bool VvxParser::ntfs_ParseVvxFile(const char* szFileName, VvxParser& parser) 
{

#if defined(ZQ_OS_MSWIN)

	HANDLE fileHandle = INVALID_HANDLE_VALUE;

	fileHandle = CreateFileA(szFileName, 
							 GENERIC_READ, 
							 FILE_SHARE_READ|FILE_SHARE_WRITE,
							 0, 
							 OPEN_EXISTING, 
							 0, 
							 0);
	
	if(INVALID_HANDLE_VALUE == fileHandle)
	{		
		glog(Log::L_ERROR, "(%s) failed to open: %08x", szFileName, GetLastError());
		
		return false;
	}

	DWORD dwHigh=0, dwLow=0;

	dwLow = GetFileSize(fileHandle, &dwHigh);

	if (dwHigh>0||!dwLow)
	{
		glog(Log::L_ERROR, "(%S) GetFileSize failed with code %08x", szFileName, GetLastError());
		return false;
	}

#elif defined(ZQ_OS_LINUX)

	int fd = open(szFileName, O_RDWR);
	if(fd == (-1)) {
		glog(Log::L_ERROR, "(%s) failed to open: (%s)", strerror(errno));
	}

	struct stat info;
	if(fstat(fd, &info) == (-1)) {
		glog(Log::L_ERROR, "(%s) failed to get file info: (%s)", strerror(errno));
		return false;
	}

	long dwLow = info.st_size; 

#else
	/* TODO */	
	return false;
#endif
	
	unsigned long bytesRead = 0;
	PVVX_INDEX_HEADER vvxIndexHeader = (PVVX_INDEX_HEADER)malloc(dwLow);

	if (!vvxIndexHeader)
	{
		glog(Log::L_ERROR, "(%S) Fail to allocate memory, size %d", szFileName, dwLow);
		
		return false;
	}

	unsigned len = dwLow;
	bool bResult = true;

	do{	
#if defined(ZQ_OS_MSWIN)
		if(FALSE == ReadFile(fileHandle,
							 vvxIndexHeader,
							 len,
						     &bytesRead,
							 0))
		{
			glog(Log::L_ERROR, "(%s) ReadFile failed with code %08x", szFileName, GetLastError());
			
			bResult = false;
			break;
		}
#elif defined(ZQ_OS_LINUX)
		if((bytesRead = read(fd, vvxIndexHeader, len)) == (-1)) {
			glog(Log::L_ERROR, "(%s) read failed: (%s)", szFileName, strerror(errno));

			bResult = false;
			break;
		}
#else
	/* TODO */
#endif
		if(bytesRead != len)
		{
			glog(Log::L_ERROR, "(%S) bytes read(%d) != read len requested(%d)",
				szFileName, bytesRead, len);

			bResult = false;
			break;
		}
		
		if(strcmp((char *)vvxIndexHeader->signature, VVX_INDEX_SIGNATURE))
		{
			glog(Log::L_ERROR, "(%S) invalid vvx header signature", szFileName);

			bResult = false;
			break;
		}
		
		if(vvxIndexHeader->majorVersion == 5) {
			glog(Log::L_INFO, "(%s) vvx header version 5", szFileName);

			parser._dwBitrate = vvxIndexHeader->mpegBitRate;
			parser._dwHorizontalSize = vvxIndexHeader->horizontalSize;
			parser._dwVerticalSize = vvxIndexHeader->verticalSize;
			parser._dwVideoBitrate = vvxIndexHeader->videoBitRate;
			parser._dwFrameRateCode = vvxIndexHeader->frameRateCode;
			parser._nSubFileCount = vvxIndexHeader->subFileInformationCount;
	
			PVVX_SUBFILE_INFORMATION subFileInformation = 0;
			subFileInformation = (PVVX_SUBFILE_INFORMATION)BYTE_OFFSET_POINTER(vvxIndexHeader, vvxIndexHeader->subFileInformationOffset);

			for (size_t i = 0; i < vvxIndexHeader->subFileInformationCount; i++) {
				strcpy(parser._subFileExtension[i], (const char*)(subFileInformation[i].fileExtension));
				parser._subFilePlayTimeInMilliSeconds[i] = subFileInformation[i].playTimeInMilliSeconds;
			}

			break;
		}
		else if(vvxIndexHeader->majorVersion == 6) {
			glog(Log::L_INFO, "(%s) vvx header version 6", szFileName);

			PVVX_V6_INDEX_HEADER vvx6Header = (PVVX_V6_INDEX_HEADER)vvxIndexHeader;

#if defined(ZQ_OS_MSWIN)
			*(uint64*)&parser._fileTime = vvx6Header->systemTime;
#endif
			parser._dwBitrate = vvx6Header->streamBitRate;
			parser._dwHorizontalSize = vvx6Header->horizontalSize;
			parser._dwVerticalSize = vvx6Header->verticalSize;
			parser._dwVideoBitrate = vvx6Header->videoBitRate;
//			parser._dwStreamType = vvx6Header->streamType;
			parser._dwFrameRateCode = vvx6Header->frameRateCode;
			parser._nSubFileCount = vvx6Header->subFileInformationCount;
//			parser._tsInformationCount = vvx6Header->tsInformationCount;
//			parser._esInformationCount = vvx6Header->esInformationCount;

//			PVVX_TS_INFORMATION			tsInformation;
//			PVVX_ES_INFORMATION			esInformation;
			PVVX_V6_SUBFILE_INFORMATION	subFileInformation;		
			
//			tsInformation = (PVVX_TS_INFORMATION)BYTE_OFFSET_POINTER (vvx6Header, vvx6Header->tsInformationOffset);
//			for (ULONG i = 0; i < vvx6Header->tsInformationCount; i++)
//			{
//				parser._tsProgramNumber[i] = tsInformation[i].programNumber;
//				parser._tsPMTPid[i] = tsInformation[i].pmtPid;
//				parser._tsPCRPid[i] = tsInformation[i].pcrPid;
//				parser._tsVideoPid[i] = tsInformation[i].videoPid;
//			}
//			
//			esInformation = (PVVX_ES_INFORMATION)BYTE_OFFSET_POINTER (vvx6Header, vvx6Header->esInformationOffset);
//			for (i = 0; i < vvx6Header->esInformationCount; i++)
//			{
//				parser._esStreamType[i] = esInformation[i].streamType;
//				parser._esStreamFlags[i] = esInformation[i].streamFlags;
//				parser._esPID[i] = esInformation[i].pid;		
//			}

			subFileInformation = (PVVX_V6_SUBFILE_INFORMATION)BYTE_OFFSET_POINTER (vvx6Header, vvx6Header->subFileInformationOffset);
			for (unsigned long i = 0; i < vvx6Header->subFileInformationCount; i++)
			{
				strcpy(parser._subFileExtension[i], (const char*)(subFileInformation[i].fileExtension));
//				parser._subFileType[i] = subFileInformation[i].fileType;
//				parser._subFileSpeed_denominator[i] = subFileInformation[i].speed.denominator;
//				parser._subFileSpeed_numerator[i] = subFileInformation[i].speed.numerator;
				parser._subFilePlayTimeInMilliSeconds[i] = subFileInformation[i].playTimeInMilliSeconds;
			}		
		}
		else if(vvxIndexHeader->majorVersion == 7) {
			glog(Log::L_INFO, "(%s) vvx header version 7", szFileName);

			PVVX_V7_INDEX_HEADER vvx7Header = (PVVX_V7_INDEX_HEADER)vvxIndexHeader;

#if defined(ZQ_OS_MSWIN)
			*(uint64*)&parser._fileTime = vvx7Header->systemTime;
#endif
			parser._dwBitrate = vvx7Header->streamBitRate;
			parser._dwHorizontalSize = vvx7Header->horizontalSize;
			parser._dwVerticalSize = vvx7Header->verticalSize;
			parser._dwVideoBitrate = vvx7Header->videoBitRate;
//			parser._dwStreamType = vvx7Header->streamType;
			parser._dwFrameRateCode = vvx7Header->frameRateCode;
			parser._nSubFileCount = vvx7Header->subFileInformationCount;
//			parser._tsInformationCount = vvx7Header->tsInformationCount;
//			parser._esInformationCount = vvx7Header->esInformationCount;

//			PVVX_TS_INFORMATION			tsInformation;
//			PVVX_ES_INFORMATION			esInformation;
			PVVX_V6_SUBFILE_INFORMATION	subFileInformation;		
			
//			tsInformation = (PVVX_TS_INFORMATION)BYTE_OFFSET_POINTER (vvx7Header, vvx7Header->tsInformationOffset);
//			for (ULONG i = 0; i < vvx7Header->tsInformationCount; i++)
//			{
//				parser._tsProgramNumber[i] = tsInformation[i].programNumber;
//				parser._tsPMTPid[i] = tsInformation[i].pmtPid;
//				parser._tsPCRPid[i] = tsInformation[i].pcrPid;
//				parser._tsVideoPid[i] = tsInformation[i].videoPid;
//			}
//			
//			esInformation = (PVVX_ES_INFORMATION)BYTE_OFFSET_POINTER (vvx7Header, vvx7Header->esInformationOffset);
//			for (i = 0; i < vvx7Header->esInformationCount; i++)
//			{
//				parser._esStreamType[i] = esInformation[i].streamType;
//				parser._esStreamFlags[i] = esInformation[i].streamFlags;
//				parser._esPID[i] = esInformation[i].pid;		
//			}

			subFileInformation = (PVVX_V6_SUBFILE_INFORMATION)BYTE_OFFSET_POINTER (vvx7Header, vvx7Header->subFileInformationOffset);
			for (unsigned long i = 0; i < vvx7Header->subFileInformationCount; i++)
			{
				strcpy(parser._subFileExtension[i], (const char*)(subFileInformation[i].fileExtension));
//				parser._subFileType[i] = subFileInformation[i].fileType;
//				parser._subFileSpeed_denominator[i] = subFileInformation[i].speed.denominator;
//				parser._subFileSpeed_numerator[i] = subFileInformation[i].speed.numerator;
				parser._subFilePlayTimeInMilliSeconds[i] = subFileInformation[i].playTimeInMilliSeconds;
			}		
		}
		else {
			glog(Log::L_ERROR, "(%S) invalid vvx header major_version (%d)",
				szFileName, vvxIndexHeader->majorVersion);

			bResult = false;
			break;
		}
		
	}while(0);

	free(vvxIndexHeader);
#if defined (ZQ_OS_MSWIN)
	CloseHandle(fileHandle);
#elif defined(ZQ_OS_LINUX)
	close(fd);
#else
	/* TODO */
#endif

	return bResult;
}

// reserve for seek in file instead of load all file in memory
//bool VvxParser::ntfs_ParseVvxFile(const char* szFileName, VvxParser& parser)
//{
//	HANDLE fileHandle = INVALID_HANDLE_VALUE;
//	fileHandle = CreateFileA(szFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
//		0, OPEN_EXISTING, 0, 0);
//	
//	if(INVALID_HANDLE_VALUE == fileHandle)
//	{		
//		glog(Log::L_DEBUG, "(%S) CreateFile failed with code %08x", szFileName, GetLastError());
//		return false;
//	}
//		
//	DWORD bytesRead = 0;	
//	VVX_V7_INDEX_HEADER hdr = {0};
//	DWORD len = sizeof(hdr);
//	bool bResult = false;
//
//	do{	
//
//		if(FALSE == ReadFile(fileHandle,
//			&hdr,
//			len,
//			&bytesRead,
//			0))
//		{
//			glog(Log::L_DEBUG, "(%S) ReadFile failed with code %08x", szFileName, GetLastError());
//			break;
//		}
//		
//		if(bytesRead != len)
//		{
//			glog(Log::L_DEBUG, "(%S) bytes read(%d) != read len requested(%d)",
//				szFileName, bytesRead, len);
//			break;
//		}
//		
//		// get head data;
//		if(strcmp((char *)hdr.signature, VVX_INDEX_SIGNATURE))
//		{
//			glog(Log::L_DEBUG, "(%S) invalid vvx header signature", szFileName);
//			break;
//		}
//		
//		if(hdr.majorVersion != 7)
//		{
//			glog(Log::L_DEBUG, "(%S) invalid vvx header major_version (%d)",
//				szFileName, hdr.majorVersion);
//			break;
//		}
//		
//		*(UQUADWORD*)&parser._fileTime = hdr.systemTime;
//		parser._dwBitrate = hdr.streamBitRate;
//		parser._dwHorizontalSize = hdr.horizontalSize;
//		parser._dwVerticalSize = hdr.verticalSize;
//		parser._dwVideoBitrate = hdr.videoBitRate;
//		parser._dwStreamType = hdr.streamType;
//		parser._dwFrameRateCode = hdr.frameRateCode;
//		parser._nSubFileCount = hdr.subFileInformationCount;
//		parser._tsInformationCount = hdr.tsInformationCount;
//		parser._esInformationCount = hdr.esInformationCount;
//		
//		//
//		// parse content
//		//
//		PVVX_TS_INFORMATION			tsInformation;
//		PVVX_ES_INFORMATION			esInformation;
//		PVVX_V6_SUBFILE_INFORMATION	subFileInformation;		
//			
//		tsInformation = (PVVX_TS_INFORMATION)BYTE_OFFSET_POINTER (&hdr, hdr.tsInformationOffset);
//		for (ULONG i = 0; i < hdr.tsInformationCount; i++)
//		{
//			parser._tsProgramNumber[i] = tsInformation[i].programNumber;
//			parser._tsPMTPid[i] = tsInformation[i].pmtPid;
//			parser._tsPCRPid[i] = tsInformation[i].pcrPid;
//			parser._tsVideoPid[i] = tsInformation[i].videoPid;
//		}
//		
//		esInformation = (PVVX_ES_INFORMATION)BYTE_OFFSET_POINTER (&hdr, hdr.esInformationOffset);
//		for (i = 0; i < hdr.esInformationCount; i++)
//		{
//			parser._esStreamType[i] = esInformation[i].streamType;
//			parser._esStreamFlags[i] = esInformation[i].streamFlags;
//			parser._esPID[i] = esInformation[i].pid;		
//		}
//
//		subFileInformation = (PVVX_V6_SUBFILE_INFORMATION)BYTE_OFFSET_POINTER (&hdr, hdr.subFileInformationOffset);
//		for (i = 0; i < hdr.subFileInformationCount; i++)
//		{
//			strcpy(parser._subFileExtension[i], (const char*)(subFileInformation[i].fileExtension));
//			parser._subFileType[i] = subFileInformation[i].fileType;
//			parser._subFileSpeed_denominator[i] = subFileInformation[i].speed.denominator;
//			parser._subFileSpeed_numerator[i] = subFileInformation[i].speed.numerator;
//			parser._subFilePlayTimeInMilliSeconds[i] = subFileInformation[i].playTimeInMilliSeconds;
//		}		
//
//		bResult = true;
//
//	}while(0);
//	
//	CloseHandle(fileHandle);
//
//	return true;
//}


bool VvxParser::parse(const char* szFilename, bool bNTFS/* = FALSE*/)
{
	char szName[MAX_PATH];
	sprintf(szName, "%s.vvx", szFilename);

	if (bNTFS)
		return ntfs_ParseVvxFile(szName, *this);
	else {
#ifdef NAS 
		return false;
#else
		return vsm_ParseVvxFile(szName, *this);	
#endif
	}
	
}


//	bool result = false;
//
//	// get the vvx file size
//	LONGLONG size = 0;
//	VVX_V7_INDEX_HEADER hdr = {0};
//
//	string name = szFilename;
//	name += ".vvx";
//
//	do {
//		if(false == OpenAndRead(name.c_str(), sizeof(hdr), &hdr))
//			break;
//
//		if(strcmp((char *)hdr.signature, VVX_INDEX_SIGNATURE))
//		{
//			glog(Log::L_DEBUG, "(%S) invalid vvx header signature", name.c_str());
//			break;
//		}
//
//		if(hdr.majorVersion != 7)
//		{
//			glog(Log::L_DEBUG, "(%S) invalid vvx header major_version (%d)",
//				name.c_str(), hdr.majorVersion);
//			break;
//		}
//
//		_dwBitrate = hdr.streamBitRate;
//		_dwHorizontalSize = hdr.horizontalSize;
//		_dwVerticalSize = hdr.verticalSize;
//		
//		result = true;
//			
//	} while (0);
//	
