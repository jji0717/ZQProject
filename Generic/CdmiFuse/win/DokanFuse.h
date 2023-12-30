// ===========================================================================
// Copyright (c) 2004 by
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
// Ident : $Id: DokanFuse.h Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : FUSE implementation for Windows based on Dokan
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/CdmiFuse/win/DokanFuse.h $
// 
// 17    7/19/13 3:06p Hui.shao
// impl install/remove dokan driver
// 
// 16    5/22/13 11:17a Li.huang
// 
// 15    4/19/13 2:22p Li.huang
// 
// 14    4/19/13 11:30a Hui.shao
// 
// 13    4/19/13 9:32a Hui.shao
// thread-ized fuse to support multiple threads
// 
// 12    4/19/13 9:24a Li.huang
// 
// 11    2/25/13 8:33p Hui.shao
// skip guess if getFileInfo() is certain whether the input path is a dir
// or file
// 
// 10    1/09/13 3:03p Hui.shao
// 
// 9     1/07/13 12:03p Hui.shao
// 
// 8     1/06/13 4:22p Hui.shao
// drafted some entries for CdmiDokan
// 
// 7     12/28/12 11:40a Hui.shao
// disk space
// 
// 6     12/27/12 5:06p Hui.shao
// splitted
// 
// 5     12/27/12 4:04p Hui.shao
// 
// 4     12/26/12 8:06p Hui.shao
// smoketest
// 
// 3     12/26/12 6:18p Hui.shao
// log and DokanNest
// 
// 2     12/26/12 12:24p Hui.shao
// 
// 1     12/24/12 5:10p Hui.shao
// created
// ===========================================================================

#ifndef __DokanFuse_H__
#define __DokanFuse_H__

// #define DOKANX 

#include "../CdmiFuseOps.h"

extern "C" {
#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include <stdlib.h>
}

#include <locale>

#ifdef DOKANX

extern "C" {
#include "dokanx/dokan.h"
#include "dokanx/fileinfo.h"
}

#ifdef _USER_GUI
#pragma comment(lib, "../dokanx/bin/dokanx.lib")
#else
#pragma comment(lib, "dokanx/bin/dokanx.lib")
#endif

#else
extern "C" {
#include "dokan/dokan.h"
#include "dokan/fileinfo.h"
}

#ifdef _USER_GUI
#pragma comment(lib, "../dokan/bin/dokan.lib")
#else
#pragma comment(lib, "dokan/bin/dokan.lib")
#endif

#endif // DOKANX

// -----------------------------
// class DokanFuse
// -----------------------------
/// A FUSE-like implementation for Windows with the support of open source project Dokan
/// dokan homepage: http://dokan-dev.net 
class DokanFuse : public ZQ::common::NativeThread
{
public:
	DokanFuse(ZQ::common::Log& log, const std::string& mountPoint, const std::string& rootPath, uint32 options, const std::string& locale="", uint threadCount=0);
	virtual ~DokanFuse();

	void stop();

	static uint runningCount() { return _cRunning; }

protected: // overwrites of NativeThread
	virtual int run(void);

protected:

	// the variaables to initialize dokan env
	// TODO: we may borrow _dokanOps.GlobalContext and DOKAN_FILE_INFO::Context or DokanContext to cover
	// multiple mountpoints by one instance
	DOKAN_OPERATIONS _dokanOperations;
	DOKAN_OPTIONS    _dokanOptions;

	std::string      _locale;
	std::wstring     _wMountPoint, _wRootPath;
	std::string      _desc;

	DWORD            _volSN;

	ZQ::common::Log& _log;
	HANDLE _waitStop;

protected:

	// the operation entries of dokan
	// ----------------------------------
	virtual int doCreateFile(PDOKAN_FILE_INFO DokanFileInfo,
		LPCWSTR					FileName,
		DWORD					AccessMode,
		DWORD					ShareMode,
		DWORD					CreationDisposition,
		DWORD					FlagsAndAttributes);

	virtual int doCloseFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName);

	virtual int doCreateDirectory(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName);
	virtual int doOpenDirectory(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName);
	virtual int doDeleteDirectory(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName);

	virtual int doCleanup(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName);
	virtual int doReadFile(PDOKAN_FILE_INFO DokanFileInfo,
		LPCWSTR				FileName,
		LPVOID				Buffer,
		DWORD				BufferLength,
		LPDWORD				ReadLength,
		LONGLONG			Offset);

	virtual int doWriteFile(PDOKAN_FILE_INFO DokanFileInfo,
		LPCWSTR		FileName,
		LPCVOID		Buffer,
		DWORD		NumberOfBytesToWrite,
		LPDWORD		NumberOfBytesWritten,
		LONGLONG			Offset);

	virtual int doFlushFileBuffers(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName);

	virtual int doGetFileInfo(PDOKAN_FILE_INFO DokanFileInfo,
		LPCWSTR							FileName,
		LPBY_HANDLE_FILE_INFORMATION	HandleFileInformation, bool skipGuessContainer=false);

	virtual int doFindFiles(PDOKAN_FILE_INFO DokanFileInfo,
		LPCWSTR				FileName,
		PFillFindData		FillFindData);

	virtual int doDeleteFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName);

	virtual int doMoveFile(PDOKAN_FILE_INFO DokanFileInfo,
		LPCWSTR				FileName, // existing file name
		LPCWSTR				NewFileName,
		BOOL				ReplaceIfExisting);

	virtual int doLockFile(PDOKAN_FILE_INFO DokanFileInfo,
		LPCWSTR				FileName,
		LONGLONG			ByteOffset,
		LONGLONG			Length);

	virtual int doUnlockFile(PDOKAN_FILE_INFO DokanFileInfo,
		LPCWSTR				FileName,
		LONGLONG			ByteOffset,
		LONGLONG			Length);

	virtual int doSetEndOfFile(PDOKAN_FILE_INFO DokanFileInfo,
		LPCWSTR				FileName,
		LONGLONG			ByteOffset);

	virtual int doSetAllocationSize(PDOKAN_FILE_INFO DokanFileInfo,
		LPCWSTR				FileName,
		LONGLONG			AllocSize);

	virtual int doSetFileAttrs(PDOKAN_FILE_INFO DokanFileInfo,
		LPCWSTR				FileName,
		DWORD				FileAttributes);

	virtual int doSetFileTime(PDOKAN_FILE_INFO DokanFileInfo,
		LPCWSTR				FileName,
		CONST FILETIME*		CreationTime,
		CONST FILETIME*		LastAccessTime,
		CONST FILETIME*		LastWriteTime);

	virtual int doGetFileSecurity(PDOKAN_FILE_INFO DokanFileInfo,
		LPCWSTR					FileName,
		PSECURITY_INFORMATION	SecurityInformation,
		PSECURITY_DESCRIPTOR	SecurityDescriptor,
		ULONG				BufferLength,
		PULONG				LengthNeeded);

	virtual int doSetFileSecurity(PDOKAN_FILE_INFO DokanFileInfo,
		LPCWSTR					FileName,
		PSECURITY_INFORMATION	SecurityInformation,
		PSECURITY_DESCRIPTOR	SecurityDescriptor,
		ULONG				SecurityDescriptorLength);

	virtual int  doGetDiskFreeSpace(PDOKAN_FILE_INFO DokanFileInfo,
		PULONGLONG FreeBytesAvailable,
		PULONGLONG TotalNumberOfBytes,
		PULONGLONG TotalNumberOfFreeBytes);

	virtual int doGetVolumeInfo(PDOKAN_FILE_INFO DokanFileInfo,
		LPWSTR		VolumeNameBuffer,     DWORD VolumeNameSize,
		LPWSTR		FileSystemNameBuffer, DWORD FileSystemNameSize,
		LPDWORD		VolumeSerialNumber,   LPDWORD MaximumComponentLength, LPDWORD FileSystemFlags);

	virtual int doUnmount(PDOKAN_FILE_INFO DokanFileInfo);

private:
	friend class DokanNest;
	DokanNest* _nest;
	static uint  _cRunning;

protected:
	virtual void getOpUserName(PDOKAN_FILE_INFO DokanFileInfo, std::string& user, std::string& domain);
	virtual std::string toFilePath(std::wstring& wFilePath, LPCWSTR fileName);

public: // utility functions
	// normal string to wide-string
	static std::wstring mbs2wcs(const std::string& mbs, const char* locale=NULL);
	static std::string wcs2mbs(const std::wstring& wcs, const char* locale=NULL);

	static bool installDriver(void);
	static bool removeDriver(void);
};

#endif // __DokanFuse_H__
