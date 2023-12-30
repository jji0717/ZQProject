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
// Ident : $Id: CdmiDokan.h Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : FUSE implementation for Windows based on Dokan
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/CdmiFuse/win/CdmiDokan.h $
// 
// 22    11/19/14 8:08p Hui.shao
// userdomain
// 
// 21    6/19/13 3:36p Li.huang
// 
// 20    6/13/13 6:10p Hui.shao
// flags
// 
// 19    6/03/13 7:37p Hui.shao
// ChildReader dispatch to searchByParent() and readByEnumeration() 
// 
// 18    5/30/13 11:43a Hui.shao
// FindFiles() takes ChildReaders, and sunk fileInfoCache into CdmiFuseOps
// 
// 17    5/09/13 3:47p Li.huang
// 
// 16    3/01/13 10:20a Hui.shao
// 
// 15    2/25/13 8:33p Hui.shao
// skip guess if getFileInfo() is certain whether the input path is a dir
// or file
// 
// 14    2/25/13 4:11p Hui.shao
// separated rootUrl and homeContainer (a sub container under rootUrl)
// 
// 13    2/07/13 2:54p Hui.shao
// blacklist of CreateDirectory()
// 
// 12    1/25/13 3:15p Hui.shao
// 
// 11    1/24/13 4:00p Hui.shao
// supported files in subdir
// 
// 10    1/23/13 10:15a Hui.shao
// wrapperd fstat into fileinfo
// 
// 9     1/10/13 5:16p Hui.shao
// all entries enumerated
// 
// 8     1/10/13 2:15p Hui.shao
// doReadFile()
// 
// 7     1/09/13 3:13p Hui.shao
// covered initMount + copyInto
// 
// 6     1/07/13 10:54p Hui.shao
// WriteFile()
// 
// 5     1/07/13 8:36p Hui.shao
// findfiles and getfileinfo
// 
// 4     1/07/13 12:03p Hui.shao
// 
// 3     1/06/13 4:22p Hui.shao
// drafted some entries for CdmiDokan
// 
// 2     12/28/12 11:40a Hui.shao
// disk space
// 
// 1     12/27/12 5:06p Hui.shao
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

#ifndef __CdmiDokan_H__
#define __CdmiDokan_H__

#define DEFAULT_LOG_FLAGS (0x0000f)

#include "DokanFuse.h"

// -----------------------------
// class CdmiDokan
// -----------------------------
/// A FUSE-like implementation for Windows with the support of open source project Dokan
/// dokan homepage: http://dokan-dev.net 
class CdmiDokan : public CdmiFuseOps, public DokanFuse
{
public:
	CdmiDokan(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, const std::string& mountPoint, const std::string& url, const std::string& userDomain, const std::string& homeContainer, uint32 options, uint32 flags =DEFAULT_LOG_FLAGS, const std::string& locale="", uint threadCount=0);
	virtual ~CdmiDokan();

	// typedef std::vector< std::string > StringList;

	HANDLE newHandle(const std::string& pathname);
	void closeHandle(HANDLE handle);

	void fixupPathname(std::string& pathname);
	static void timeToFileTime(int64 t, FILETIME& ft);
	static int cdmiErrToDokanErr(CdmiRetCode cdmirc);

protected: // overwrites of utilities of DokanFuse
	virtual std::string toFilePath(std::wstring& wFilePath, LPCWSTR fileName);

	typedef std::map< uint64, std::string > HandleMap; // map from handle to pathname, it would be a dir if the pathname ends with a '\'
	HandleMap  _handleMap;
	uint64     _lastHandle;
	ZQ::common::Mutex _lkHandles;

protected: // owverwrites of CdmiFuseOps
	virtual void OnChildInfo(const std::string& pathName, FileInfo& fi, const std::string& txnId);
	virtual void OnChildReaderStopped(int readerId, bool bCancelled, const std::string& txnId);

protected:

	// overwrites of the operation entries of dokan
	// ----------------------------------
	virtual int doGetVolumeInfo(PDOKAN_FILE_INFO DokanFileInfo,
		LPWSTR		VolumeNameBuffer,     DWORD VolumeNameSize,
		LPWSTR		FileSystemNameBuffer, DWORD FileSystemNameSize,
		LPDWORD		VolumeSerialNumber,   LPDWORD MaximumComponentLength, LPDWORD FileSystemFlags);

	virtual int doUnmount(PDOKAN_FILE_INFO DokanFileInfo);

	virtual int doGetFileInfo(PDOKAN_FILE_INFO DokanFileInfo,
		LPCWSTR							FileName,
		LPBY_HANDLE_FILE_INFORMATION	HandleFileInformation, bool skipGuessContainer=false);

	virtual int doCreateDirectory(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR DirName);
	virtual int doOpenDirectory(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR DirName);
	virtual int doDeleteDirectory(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName);
	virtual int doFindFiles(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR PathName, PFillFindData FillFindData);

	virtual int doSetAllocationSize(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName, LONGLONG AllocSize);

	virtual int doSetEndOfFile(PDOKAN_FILE_INFO  DokanFileInfo, LPCWSTR FileName, LONGLONG ByteOffset);

	virtual int doFlushFileBuffers(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName);

	virtual int doCreateFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName,
		DWORD AccessMode, DWORD ShareMode, DWORD CreationDisposition, DWORD FlagsAndAttributes);

	virtual int doCloseFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName);
	virtual int doCleanup(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName);

	virtual int doWriteFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName,
		LPCVOID Buffer, DWORD NumberOfBytesToWrite, LPDWORD NumberOfBytesWritten, LONGLONG Offset);

	virtual int doSetFileAttrs(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName, DWORD FileAttributes);
	virtual int doSetFileTime(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName,
		CONST FILETIME* CreationTime, CONST FILETIME* LastAccessTime, CONST FILETIME* LastWriteTime);

	virtual int doReadFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName,
		LPVOID Buffer, DWORD Length, LPDWORD pReadLength, LONGLONG Offset);

	virtual int doDeleteFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName);
	virtual int doMoveFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName, // existing file name
		LPCWSTR NewFileName, BOOL ReplaceIfExisting);

	virtual int doLockFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName,
		LONGLONG ByteOffset, LONGLONG Length);
	virtual int CdmiDokan::doUnlockFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName,
		LONGLONG ByteOffset, LONGLONG Length);

	virtual int doGetFileSecurity(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName,
		PSECURITY_INFORMATION SecurityInfo, PSECURITY_DESCRIPTOR SecurityDcptr,
		ULONG BufferLength, PULONG LengthNeeded);
	virtual int doSetFileSecurity(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName,
		PSECURITY_INFORMATION SecurityInfo, PSECURITY_DESCRIPTOR SecurityDcptr,
		ULONG SecurityDescriptorLength);

	virtual int doGetDiskFreeSpace(PDOKAN_FILE_INFO DokanFileInfo,
		PULONGLONG FreeBytesAvailable, PULONGLONG TotalNumberOfBytes, PULONGLONG TotalNumberOfFreeBytes);
};

#endif // __CdmiDokan_H__
